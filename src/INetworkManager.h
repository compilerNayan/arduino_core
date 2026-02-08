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

    // Restart network (disconnect and reconnect)
    Public Virtual Void RestartNetwork() = 0;
};

#endif // INETWORKMANAGER_H

