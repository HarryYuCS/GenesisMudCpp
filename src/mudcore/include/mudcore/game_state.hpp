/**
 * @file game_state.hpp
 * @brief Structured game snapshot updated from GMCP on the main thread.
 *
 * GUI panels (magic map, status bars) read from GameState; they do not parse GMCP directly.
 */

#ifndef MUDCORE_GAME_STATE_HPP
#define MUDCORE_GAME_STATE_HPP

#include <mudcore/gmcp_parser.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace genesis::mudcore {

/**
 * @brief Room data derived from GMCP Room.* packages.
 */
struct RoomInfo {
    std::string roomId;
    std::string map;          ///< Map identifier or layout data from the server.
    std::string description;
    std::vector<std::string> exits;
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
     *
     * @param message Parsed GMCP message from GmcpParser.
     */
    void applyGmcp(const GmcpMessage& message);

    /**
     * @brief Current room information for the magic map panel.
     * @return Read-only reference to the current RoomInfo.
     */
    const RoomInfo& room() const noexcept;

    /**
     * @brief Whether the player is logged in to the MUD.
     * @return true if a successful login has been recorded.
     */
    bool loggedIn() const noexcept;

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

    /** @brief Current mana level as reported by the server (string abstraction). */
    const std::string& manaLevel() const noexcept;

    /** @brief Current health level as reported by the server. */
    const std::string& healthLevel() const noexcept;

    /** @brief Current stamina level as reported by the server. */
    const std::string& staminaLevel() const noexcept;

    /** @brief Current encumbrance level as reported by the server. */
    const std::string& encumberanceLevel() const noexcept;

    /** @brief Current hunger level as reported by the server. */
    const std::string& hungerLevel() const noexcept;

    /** @brief Current thirst level as reported by the server. */
    const std::string& thirstLevel() const noexcept;

private:
    std::unordered_map<std::string, std::string> variables_;

    std::string manaLevel_;
    std::string healthLevel_;
    std::string staminaLevel_;
    std::string encumberanceLevel_;
    std::string hungerLevel_;
    std::string thirstLevel_;

    RoomInfo currentRoomInfo_;
    bool loggedIn_{false};
};

} // namespace genesis::mudcore

#endif // MUDCORE_GAME_STATE_HPP
