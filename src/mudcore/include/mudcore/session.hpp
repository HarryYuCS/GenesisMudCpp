#ifndef MUDCORE_SESSION_HPP
#define MUDCORE_SESSION_HPP

#include <mudcore/input_pipeline.hpp>
#include <mudcore/output_pipeline.hpp>
#include <mudcore/event_bus.hpp>
#include <mudcore/game_state.hpp>
#include <mudcore/event.hpp>
#include <network/network_types.hpp>
#include <network/tcp_session.hpp>
#include <network/telnet_codec.hpp>

namespace genesis::mudcore {

/**
 * @brief The Session class is responsible for managing the session with the server.
 */
class Session {
public:
    Session();
    ~Session();

    void connect(const std::string& host, uint16_t port);
    void disconnect();

    void sendCommand(const std::string& command);
    void sendText(const std::string& text);

private:
    void drainClientEventBus();
    void handleClientEvent(const ClientEvent& event);

    void sendCoreSupports();
    bool sentCoreSupports;

    genesis::network::TelnetCodec telnetCodec;
    genesis::network::TcpSession tcpSession;

    InputPipeline inputPipeline;
    OutputPipeline outputPipeline;

    EventBus eventBus;

    GameState gameState;
};

} // namespace genesis::mudcore

#endif // MUDCORE_SESSION_HPP