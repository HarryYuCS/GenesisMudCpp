#pragma once

#include <mudcore/gmcp_parser.hpp>

#include <string>
#include <string_view>

namespace genesis::test {

inline std::string_view charVitalsJson() {
    return R"({"health":"very hurt","mana":"in full vigour","food":"satisfied","drink":"quenched","fatigue":"rested","intoxication":"sober"})";
}

inline std::string_view roomInfoJson() {
    return R"({"id":"iX5PnH","short":"A busy plaza.","exits":["north","east"],"doors":["north"],"x":10,"y":20})";
}

inline std::string_view roomMapJson() {
    return R"({"map":"map graphics","zoom":"zoomed map graphics"})";
}

inline std::string_view commChannelJson() {
    return R"({"line":"say","name":"You","body":"How are you?","message":"You say: How are you?"})";
}

inline std::string_view coreSupportsSetJson() {
    return R"(["Char 1","Room 1","Comm 1","Core 1"])";
}

inline std::string_view charLoginJson() {
    return R"({"name":"eowul","uid":12345})";
}

inline std::string_view unknownPackageJson() {
    return R"({"value":1})";
}

/** @brief Build a raw GMCP body ("Package.Name {...}") as received from telnet. */
inline std::string rawGmcp(std::string_view package, std::string_view json) {
    return std::string(package) + " " + std::string(json);
}

inline genesis::mudcore::GmcpMessage charVitalsMessage() {
    return genesis::mudcore::GmcpMessage{"Char.Vitals", std::string(charVitalsJson())};
}

inline genesis::mudcore::GmcpMessage roomInfoMessage() {
    return genesis::mudcore::GmcpMessage{"Room.Info", std::string(roomInfoJson())};
}

inline genesis::mudcore::GmcpMessage roomMapMessage() {
    return genesis::mudcore::GmcpMessage{"Room.Map", std::string(roomMapJson())};
}

inline genesis::mudcore::GmcpMessage charLoginMessage() {
    return genesis::mudcore::GmcpMessage{"Char.Login", std::string(charLoginJson())};
}

inline genesis::mudcore::GmcpMessage unknownPackageMessage() {
    return genesis::mudcore::GmcpMessage{"Custom.Unknown", std::string(unknownPackageJson())};
}

inline std::string charVitalsRaw() {
    return rawGmcp("Char.Vitals", charVitalsJson());
}

inline std::string roomInfoRaw() {
    return rawGmcp("Room.Info", roomInfoJson());
}

inline std::string commChannelRaw() {
    return rawGmcp("Comm.Channel", commChannelJson());
}

inline std::string unknownPackageRaw() {
    return rawGmcp("Custom.Unknown", unknownPackageJson());
}

} // namespace genesis::test
