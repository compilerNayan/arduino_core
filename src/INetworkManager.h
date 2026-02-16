#ifndef INETWORKMANAGER_H
#define INETWORKMANAGER_H

#include <StandardDefines.h>

DefineStandardPointers(INetworkManager)
class INetworkManager {
    Public Virtual ~INetworkManager() = default;

    // Connect to network (WiFi or start hotspot)
    Public Virtual Void ConnectNetwork() = 0;

    // Disconnect from network (stop WiFi or hotspot)
    Public Virtual Void DisconnectNetwork() = 0;

    // Check if network is connected (WiFi or hotspot)
    Public Virtual Bool IsNetworkConnected() = 0;

    // Check if device has internet connectivity
    Public Virtual Bool IsInternetConnected() = 0;

    // Get current WiFi connection id (set when connecting to WiFi; 0 if not applicable)
    Public Virtual Int GetWifiConnectionId() = 0;

    // Ensure network connectivity: reconnect if WiFi down, then update network status provider (if set).
    Public Virtual Bool EnsureNetworkConnectivity() = 0;

    // Restart network (disconnect and reconnect)
    Public Virtual Void RestartNetwork() = 0;
};

#endif // INETWORKMANAGER_H

