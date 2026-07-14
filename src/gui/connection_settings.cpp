#include <connection_settings.hpp>

#include <wx/config.h>

namespace genesis::gui {

namespace {

constexpr const char* kConfigAppName = "GenesisMUD";
constexpr const char* kHostKey = "Host";
constexpr const char* kPortKey = "Port";
constexpr const char* kDebugLoggingKey = "DebugLogging";

} // namespace

ConnectionSettings loadConnectionSettings() {
    ConnectionSettings settings;
    wxConfig config(kConfigAppName);

    wxString host = config.Read(kHostKey, wxString::FromUTF8(settings.host));
    if (!host.IsEmpty()) {
        settings.host = host.ToUTF8().data();
    }

    const long port = config.ReadLong(kPortKey, settings.port);
    if (port >= 1 && port <= 65535) {
        settings.port = static_cast<std::uint16_t>(port);
    }

    settings.debugLogging = config.ReadBool(kDebugLoggingKey, settings.debugLogging);
    return settings;
}

void saveConnectionSettings(const ConnectionSettings& settings) {
    wxConfig config(kConfigAppName);
    config.Write(kHostKey, wxString::FromUTF8(settings.host.data(), static_cast<int>(settings.host.size())));
    config.Write(kPortKey, static_cast<long>(settings.port));
    config.Write(kDebugLoggingKey, settings.debugLogging);
    config.Flush();
}

} // namespace genesis::gui
