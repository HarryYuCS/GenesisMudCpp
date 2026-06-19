#ifndef MUDCORE_GAME_STATE_HPP
#define MUDCORE_GAME_STATE_HPP

#include <unordered_map>
#include <string>

namespace genesis::mudcore {

struct GameState {
    std::unordered_map<std::string, std::string> variables;

    std::string currentRoomId;
    // statuses are strings as they are abstracted as such by the game
    std::string manaLevel;
    std::string healthLevel;
    std::string staminaLevel;
    std::string encumberanceLevel;
    std::string hungerLevel;
    std::string thirstLevel;
};

} // namespace genesis::mudcore

#endif // MUDCORE_GAME_STATE_HPP