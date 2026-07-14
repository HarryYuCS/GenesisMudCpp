#include <mudcore/inbound_pipeline.hpp>
#include <mudcore/string_utils.hpp>

#include <nlohmann/json.hpp>

namespace genesis::mudcore {

namespace {

/** @brief Extract the comm message from the JSON body of GMCP Comm. messages.*/
std::string extractCommMessage(const std::string_view jsonBody) {
    try {
        const nlohmann::json payload = nlohmann::json::parse(jsonBody);
        if (payload.contains("message") && payload["message"].is_string()) {
            return payload["message"].get<std::string>();
        }
    } catch (const nlohmann::json::parse_error&) {
    }
    return std::string(jsonBody);
}

/** @brief Extract the disconnect reason from a Core.Goodbye JSON body. */
std::string extractGoodbyeMessage(const std::string_view jsonBody) {
    if (jsonBody.empty()) {
        return "Server disconnected.";
    }

    try {
        const nlohmann::json payload = nlohmann::json::parse(jsonBody);
        if (payload.is_string()) {
            return payload.get<std::string>();
        }
    } catch (const nlohmann::json::parse_error&) {
    }

    return std::string(jsonBody);
}

} // namespace

std::optional<DisplayLine> InboundPipeline::processMudText(const std::string_view text) const {
    if (text.empty()) {
        return std::nullopt;
    }
    return DisplayLine{OutputSink::Main, std::string(text)};
}

GmcpInboundResult InboundPipeline::processGmcp(const std::string_view rawBody, GameState& gameState) const {
    const auto message = gmcpParser_.parse(rawBody);
    if (!message.has_value()) {
        return {};
    }
    return processParsedGmcp(*message, gameState);
}

GmcpInboundResult InboundPipeline::processParsedGmcp(const GmcpMessage& message, GameState& gameState) const {
    if (equalsIgnoreCase(message.package, "Core.Goodbye")) {
        return GmcpInboundResult{
            DisplayLine{OutputSink::System, extractGoodbyeMessage(message.jsonBody)},
            false,
            false,
        };
    }

    if (startsWithIgnoreCase(message.package, "Comm.")) {
        return GmcpInboundResult{
            DisplayLine{OutputSink::Comms, extractCommMessage(message.jsonBody)},
            false,
            false,
        };
    }

    const bool stateChanged = gameState.applyGmcp(message);
    const bool playerLoggedIn = equalsIgnoreCase(message.package, "Char.Login") && gameState.loggedIn();
    return GmcpInboundResult{std::nullopt, stateChanged, playerLoggedIn};
}

} // namespace genesis::mudcore
