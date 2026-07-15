#include <mudcore/game_state.hpp>
#include <mudcore/string_utils.hpp>

#include <nlohmann/json.hpp>

namespace genesis::mudcore {

namespace {

using json = nlohmann::json;

/** @brief Assign a string value to a string field if it is different from the current value. */
bool assignIfChanged(std::string& field, const std::string& value) {
    if (field == value) {
        return false;
    }
    field = value;
    return true;
}

/** @brief Assign payload[key] to field if present and a string; report whether field changed. */
bool applyStringField(const json& payload, const char* key, std::string& field) {
    if (!payload.contains(key) || !payload[key].is_string()) {
        return false;
    }
    return assignIfChanged(field, payload[key].get<std::string>());
}

/** @brief Assign int payload[key] to field if present and an integer; report whether field changed. */
bool applyIntField(const json& payload, const char* key, std::optional<int>& field) {
    if (!payload.contains(key) || !payload[key].is_number_integer()) {
        return false;
    }
    const int value = payload[key].get<int>();
    if (field.has_value() && *field == value) {
        return false;
    }
    field = value;
    return true;
}

/** @brief Replace target with the string array from payload[key], if present. */
bool applyStringArrayField(const json& payload, const char* key, std::vector<std::string>& target) {
    if (!payload.contains(key) || !payload[key].is_array()) {
        return false;
    }

    std::vector<std::string> updated;
    for (const auto& entry : payload[key]) {
        if (!entry.is_string()) {
            continue;
        }
        updated.push_back(entry.get<std::string>());
    }

    if (target == updated) {
        return false;
    }
    target = std::move(updated);
    return true;
}

} // namespace

/** @brief wraps the separate GMCP package applier functions */
struct GameState::GmcpApplier {
    static bool applyCharVitals(GameState& state, const json& payload) {
        bool changed = false;
        changed |= applyStringField(payload, "health", state.healthLevel_);
        changed |= applyStringField(payload, "mana", state.manaLevel_);
        changed |= applyStringField(payload, "food", state.foodLevel_);
        changed |= applyStringField(payload, "drink", state.drinkLevel_);
        changed |= applyStringField(payload, "fatigue", state.fatigueLevel_);
        changed |= applyStringField(payload, "intoxication", state.intoxicationLevel_);
        return changed; // return true if any field changed
    }

    static bool applyRoomInfo(RoomInfo& room, const json& payload) {
        bool changed = false;
        changed |= applyStringField(payload, "id", room.roomId);
        changed |= applyStringField(payload, "short", room.shortDescription);
        changed |= applyStringArrayField(payload, "exits", room.exits);
        changed |= applyStringArrayField(payload, "doors", room.doors);
        changed |= applyIntField(payload, "x", room.x);
        changed |= applyIntField(payload, "y", room.y);
        changed |= applyIntField(payload, "zoomx", room.zoomX);
        changed |= applyIntField(payload, "zoomy", room.zoomY);

        if (!payload.contains("x") || !payload["x"].is_number_integer()) {
            if (room.x.has_value()) {
                room.x.reset();
                changed = true;
            }
        }
        if (!payload.contains("y") || !payload["y"].is_number_integer()) {
            if (room.y.has_value()) {
                room.y.reset();
                changed = true;
            }
        }
        if (!payload.contains("zoomx") || !payload["zoomx"].is_number_integer()) {
            if (room.zoomX.has_value()) {
                room.zoomX.reset();
                changed = true;
            }
        }
        if (!payload.contains("zoomy") || !payload["zoomy"].is_number_integer()) {
            if (room.zoomY.has_value()) {
                room.zoomY.reset();
                changed = true;
            }
        }

        return changed;
    }

    static bool applyRoomMap(RoomInfo& room, const json& payload) {
        bool changed = false;
        changed |= applyStringField(payload, "map", room.map);
        changed |= applyStringField(payload, "zoom", room.zoom);
        return changed; // return true if any field changed
    }

    static bool applyCharLogin(GameState& state, const json& payload) {
        if (!payload.contains("name") || !payload["name"].is_string()) {
            return false;
        }

        const std::string name = payload["name"].get<std::string>();
        bool changed = assignIfChanged(state.playerName_, name);
        if (!state.loggedIn_) {
            state.loggedIn_ = true;
            changed = true;
        }
        return changed;
    }
};

bool GameState::applyGmcp(const GmcpMessage& message) {
    if (message.jsonBody.empty()) {
        return false;
    }

    json payload;
    try {
        payload = json::parse(message.jsonBody);
    } catch (const json::parse_error&) {
        return false;
    }

    if (equalsIgnoreCase(message.package, "Char.Vitals")) {
        return GmcpApplier::applyCharVitals(*this, payload);
    }

    if (equalsIgnoreCase(message.package, "Room.Info")) {
        return GmcpApplier::applyRoomInfo(currentRoomInfo_, payload);
    }

    if (equalsIgnoreCase(message.package, "Room.Map")) {
        return GmcpApplier::applyRoomMap(currentRoomInfo_, payload);
    }

    if (equalsIgnoreCase(message.package, "Char.Login")) {
        return GmcpApplier::applyCharLogin(*this, payload);
    }

    return false;
}

void GameState::reset() {
    variables_.clear();
    healthLevel_.clear();
    manaLevel_.clear();
    foodLevel_.clear();
    drinkLevel_.clear();
    fatigueLevel_.clear();
    intoxicationLevel_.clear();
    currentRoomInfo_ = RoomInfo{};
    playerName_.clear();
    loggedIn_ = false;
}

// getters

const RoomInfo& GameState::room() const noexcept {
    return currentRoomInfo_;
}

bool GameState::loggedIn() const noexcept {
    return loggedIn_;
}

const std::string& GameState::playerName() const noexcept {
    return playerName_;
}

void GameState::setVariable(const std::string& name, const std::string& value) {
    variables_[name] = value;
}

std::string GameState::getVariable(const std::string& name) const {
    const auto it = variables_.find(name);
    return it != variables_.end() ? it->second : std::string{};
}

const std::string& GameState::healthLevel() const noexcept { return healthLevel_; }
const std::string& GameState::manaLevel() const noexcept { return manaLevel_; }
const std::string& GameState::foodLevel() const noexcept { return foodLevel_; }
const std::string& GameState::drinkLevel() const noexcept { return drinkLevel_; }
const std::string& GameState::fatigueLevel() const noexcept { return fatigueLevel_; }
const std::string& GameState::intoxicationLevel() const noexcept { return intoxicationLevel_; }

} // namespace genesis::mudcore
