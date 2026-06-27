/**
 * @file event.hpp
 * @brief Thread-bridge events (io thread -> main thread). Not exposed to GUI.
 */

#ifndef MUDCORE_EVENT_HPP
#define MUDCORE_EVENT_HPP

#include <network/network_types.hpp>

#include <string>
#include <variant>

namespace genesis::mudcore {

enum class EventType {
    Connected,
    Disconnected,
    Error,
    MudText,
    GmcpRaw,
};

/**
 * @brief Inbound event queued by Session on the io thread, drained in poll() on the main thread.
 */
struct Event {
    EventType type{EventType::MudText};
    std::variant<std::monostate, std::string, genesis::network::NetworkError> payload;
};

} // namespace genesis::mudcore

#endif // MUDCORE_EVENT_HPP
