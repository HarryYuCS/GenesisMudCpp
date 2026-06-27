#ifndef MUDCORE_GAME_STATE_HPP
#define MUDCORE_GAME_STATE_HPP

#include <mudcore/gmcp_parser.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace genesis::mudcore {

struct RoomInfo {
    std::string roomId;
    std::string map;
    std::string description;
    std::vector<std::string> exits;
};

class GameState {
public:
    void applyGmcp(const GmcpMessage& message);

    const RoomInfo& room() const noexcept;
    bool loggedIn() const noexcept;

    void setVariable(const std::string& name, const std::string& value);
    std::string getVariable(const std::string& name) const;

    const std::string& manaLevel() const noexcept;
    const std::string& healthLevel() const noexcept;
    const std::string& staminaLevel() const noexcept;
    const std::string& encumberanceLevel() const noexcept;
    const std::string& hungerLevel() const noexcept;
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
