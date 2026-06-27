#include <mudcore/game_state.hpp>

namespace genesis::mudcore {

void GameState::applyGmcp(const GmcpMessage& message) {
    // Package-specific handlers will be filled in as GMCP support grows.
    (void)message;
}

const RoomInfo& GameState::room() const noexcept {
    return currentRoomInfo_;
}

bool GameState::loggedIn() const noexcept {
    return loggedIn_;
}

void GameState::setVariable(const std::string& name, const std::string& value) {
    variables_[name] = value;
}

std::string GameState::getVariable(const std::string& name) const {
    const auto it = variables_.find(name);
    return it != variables_.end() ? it->second : std::string{};
}

const std::string& GameState::manaLevel() const noexcept { return manaLevel_; }
const std::string& GameState::healthLevel() const noexcept { return healthLevel_; }
const std::string& GameState::staminaLevel() const noexcept { return staminaLevel_; }
const std::string& GameState::encumberanceLevel() const noexcept { return encumberanceLevel_; }
const std::string& GameState::hungerLevel() const noexcept { return hungerLevel_; }
const std::string& GameState::thirstLevel() const noexcept { return thirstLevel_; }

} // namespace genesis::mudcore
