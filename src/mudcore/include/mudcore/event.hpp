/**
 * @file event.hpp
 * @brief Event class and related enums representing communication interface between mudcore and client
 */

#ifndef MUDCORE_EVENT_HPP
#define MUDCORE_EVENT_HPP

#include <unordered_map>
#include <string>

namespace genesis::mudcore {

/**
 * @brief ServerEvent is a class that represents an event that is sent from the server.
 */
struct ServerEvent : public Event {
    enum class ServerEventType {
        CONNECTED,
        DISONNECTED,
        STATUS,
        COMMS,
        ERROR,
        TEXT,
        MAP
    };

    ServerEvent(ServerEventType type, const std::unordered_map<std::string, std::string>& data);

    std::unordered_map<std::string, std::string> data;
    std::string text;
    ServerEventType type;
};

/**
 * @brief ClientEvent is a class that represents an event that is sent from the client.
 */
struct ClientEvent : public Event {
    enum class ClientEventType {
        CONNECT_REQUEST,
        DISCONNECT_REQUEST,
        COMMAND
    };

    ClientEvent(ClientEventType type, const std::unordered_map<std::string, std::string>& data);

    std::unordered_map<std::string, std::string> data;
    std::string text;
    ClientEventType type;
};

} // namespace genesis::mudcore

#endif // MUDCORE_EVENT_HPP