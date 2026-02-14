#ifndef IWIFISERVICE_H
#define IWIFISERVICE_H

#include <StandardDefines.h>
#include "../entity/WifiCredentials.h"

DefineStandardPointers(IWifiService)
class IWifiService {
    Public Virtual ~IWifiService() = default;

    // Add WiFi credentials
    Public Virtual WifiCredentials AddWifiCredentials(const WifiCredentials& credentials) = 0;

    // Update WiFi credentials
    Public Virtual WifiCredentials UpdateWifiCredentials(const WifiCredentials& credentials) = 0;

    // Delete WiFi credentials by SSID
    Public Virtual Void DeleteWifiCredentials(const StdString& ssid) = 0;

    // Read WiFi credentials by SSID
    Public Virtual WifiCredentials GetWifiCredentials(const StdString& ssid) = 0;

    // Get all WiFi credentials
    Public Virtual StdVector<WifiCredentials> GetAllWifiCredentials() = 0;

    // Get last connected WiFi details
    Public Virtual optional<WifiCredentials> GetLastConnectedWifi() = 0;

    // Update last connected SSID
    Public Virtual Void UpdateLastConnectedSsid(const StdString& ssid) = 0;
};

#endif // IWIFISERVICE_H

