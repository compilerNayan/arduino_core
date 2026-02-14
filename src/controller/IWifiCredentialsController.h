#ifndef IWIFI_CREDENTIALS_CONTROLLER_H
#define IWIFI_CREDENTIALS_CONTROLLER_H

#include <StandardDefines.h>
#include "../entity/WifiCredentials.h"

DefineStandardPointers(IWifiCredentialsController)
class IWifiCredentialsController {
    Public Virtual ~IWifiCredentialsController() = default;

    // Create - Add new WiFi credentials
    Public Virtual WifiCredentials CreateWifiCredentials(WifiCredentials credentials) = 0;

    // Read - Get WiFi credentials by SSID
    Public Virtual WifiCredentials GetWifiCredentials(StdString ssid) = 0;

    // Read All - Get all WiFi credentials
    Public Virtual StdVector<WifiCredentials> GetAllWifiCredentials() = 0;

    // Update - Update existing WiFi credentials
    Public Virtual WifiCredentials UpdateWifiCredentials(WifiCredentials credentials) = 0;

    // Delete - Delete WiFi credentials by SSID
    Public Virtual Void DeleteWifiCredentials(StdString ssid) = 0;

    // Get last connected WiFi
    Public Virtual optional<WifiCredentials> GetLastConnectedWifi() = 0;
};

#endif // IWIFI_CREDENTIALS_CONTROLLER_H

