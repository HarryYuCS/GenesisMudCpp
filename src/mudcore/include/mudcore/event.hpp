/**
 * @file event.hpp
 * @brief Thread-bridge events passed from the io thread to the main thread via EventBus.
 *
 * These types are internal to mudcore. The GUI consumes DisplayLine and GameState via Session::poll().
 */

#ifndef MUDCORE_EVENT_HPP
#define MUDCORE_EVENT_HPP

#include <network/network_types.hpp>

#include <string>
#include <variant>

namespace genesis::mudcore {

/**
 * @brief Kind of inbound event produced by Session on the io thread.
 */
enum class EventType {
    Connected,    ///< TCP connection established.
    Disconnected, ///< TCP connection closed or lost.
    Error,        ///< Network or protocol error; payload is NetworkError.
    MudText,      ///< Player-visible MUD text; payload is std::string.
    GmcpRaw,      ///< Raw GMCP body ("Package.Name {...}"); payload is std::string.
};

/**
 * @brief Inbound event queued on the io thread and drained in Session::poll() on the main thread.
 *
 * Payload interpretation depends on @p type:
 * - MudText, GmcpRaw: std::string
 * - Error: genesis::network::NetworkError
 * - Connected, Disconnected: std::monostate
 */
struct Event {
    EventType type{EventType::MudText};
    std::variant<std::monostate, std::string, genesis::network::NetworkError> payload;
};

} // namespace genesis::mudcore

#endif // MUDCORE_EVENT_HPP
