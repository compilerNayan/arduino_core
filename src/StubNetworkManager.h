#ifndef ARDUINO
#ifndef STUBNETWORKMANAGER_H
#define STUBNETWORKMANAGER_H

#include <StandardDefines.h>
#include "INetworkManager.h"

/* @Component */
class StubNetworkManager : public INetworkManager {
    Public Virtual ~StubNetworkManager() = default;

    // Connect to network (WiFi or start hotspot) - Stub implementation
    Public Virtual Void ConnectNetwork() override {
        // Stub: Do nothing on desktop
    }

    // Disconnect from network (stop WiFi or hotspot) - Stub implementation
    Public Virtual Void DisconnectNetwork() override {
        // Stub: Do nothing on desktop
    }

    // Check if network is connected (WiFi or hotspot) - Stub implementation
    Public Virtual Bool IsNetworkConnected() override {
        // Stub: Always return false on desktop
        return false;
    }

    // Restart network (disconnect and reconnect) - Stub implementation
    Public Virtual Void RestartNetwork() override {
        // Stub: Do nothing on desktop
    }
};

#endif // STUBNETWORKMANAGER_H
#endif // ARDUINO

