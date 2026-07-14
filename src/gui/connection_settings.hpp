#ifndef GENESIS_GUI_CONNECTION_SETTINGS_HPP
#define GENESIS_GUI_CONNECTION_SETTINGS_HPP

#include <cstdint>
#include <string>

namespace genesis::gui {

struct ConnectionSettings {
    std::string host{"mud.genesismud.org"};
    std::uint16_t port{3011};
    bool debugLogging{false};
};

ConnectionSettings loadConnectionSettings();
void saveConnectionSettings(const ConnectionSettings& settings);

} // namespace genesis::gui

#endif // GENESIS_GUI_CONNECTION_SETTINGS_HPP
