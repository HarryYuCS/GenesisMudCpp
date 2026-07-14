/**
 * @file game_state.hpp
 * @brief Structured game snapshot updated from GMCP on the main thread.
 *
 * GUI panels (magic map, status bars) read from GameState; they do not parse GMCP directly.
 */

#ifndef MUDCORE_GAME_STATE_HPP
#define MUDCORE_GAME_STATE_HPP

#include <mudcore/gmcp_parser.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace genesis::mudcore {

/**
 * @brief Room data derived from GMCP Room.Info and Room.Map packages.
 */
struct RoomInfo {
    std::string roomId;
    std::string shortDescription;
    std::vector<std::string> exits;
    std::vector<std::string> doors;
    std::optional<int> x;
    std::optional<int> y;
    std::string map;   ///< Map graphics from Room.Map.
    std::string zoom;  ///< Zoomed map graphics from Room.Map.
};

/**
 * @brief Mutable game snapshot owned by Session; updated only during poll().
 */
class GameState {
public:
    /**
     * @brief Apply a parsed GMCP message to internal state.
     *
     * Package-specific handlers (Char.Vitals, Room.Info, etc.) are implemented here.
     * Package names are matched case-insensitively; JSON keys are case-sensitive.
     *
     * @param message Parsed GMCP message from GmcpParser.
     * @return true if any tracked game state field was updated.
     */
    bool applyGmcp(const GmcpMessage& message);

    /** @brief Reset all tracked fields to their initial empty/disconnected values. */
    void reset();

    /**
     * @brief Current room information for the magic map panel.
     * @return Read-only reference to the current RoomInfo.
     */
    const RoomInfo& room() const noexcept;

    /**
     * @brief Whether the player is logged in to the MUD.
     * @return true after a Char.Login broadcast with a name field.
     */
    bool loggedIn() const noexcept;

    /**
     * @brief Player name from the Char.Login broadcast.
     * @return Name string, or empty if not logged in.
     */
    const std::string& playerName() const noexcept;

    /**
     * @brief Store an arbitrary named variable.
     * @param name Variable key.
     * @param value Variable value.
     */
    void setVariable(const std::string& name, const std::string& value);

    /**
     * @brief Retrieve a named variable.
     * @param name Variable key.
     * @return Stored value, or empty string if not found.
     */
    std::string getVariable(const std::string& name) const;

    /** @brief Current health vital as reported by Char.Vitals (textual level). */
    const std::string& healthLevel() const noexcept;

    /** @brief Current mana vital as reported by Char.Vitals (textual level). */
    const std::string& manaLevel() const noexcept;

    /** @brief Current food vital as reported by Char.Vitals (textual level). */
    const std::string& foodLevel() const noexcept;

    /** @brief Current drink vital as reported by Char.Vitals (textual level). */
    const std::string& drinkLevel() const noexcept;

    /** @brief Current fatigue vital as reported by Char.Vitals (textual level). */
    const std::string& fatigueLevel() const noexcept;

    /** @brief Current intoxication vital as reported by Char.Vitals (textual level). */
    const std::string& intoxicationLevel() const noexcept;

private:
    struct GmcpApplier;

    std::unordered_map<std::string, std::string> variables_;

    std::string healthLevel_;
    std::string manaLevel_;
    std::string foodLevel_;
    std::string drinkLevel_;
    std::string fatigueLevel_;
    std::string intoxicationLevel_;

    RoomInfo currentRoomInfo_;
    std::string playerName_;
    bool loggedIn_{false};
};

} // namespace genesis::mudcore

#endif // MUDCORE_GAME_STATE_HPP
