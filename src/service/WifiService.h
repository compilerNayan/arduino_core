#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include <StandardDefines.h>
#include "IWifiService.h"
#include "../repository/WifiCredentialsRepository.h"
#include "../repository/WifiConnectionRepository.h"
#include "../entity/WifiCredentials.h"
#include "../entity/WifiConnection.h"
//#include "../IArduinoSpringBootApp.h"

/* @Service */
class WifiService : public IWifiService {
    Public Virtual ~WifiService() = default;

    /* @Autowired */
    WifiCredentialsRepositoryPtr wifiCredentialsRepository;

    /* @Autowired */
    WifiConnectionRepositoryPtr wifiConnectionRepository;

    /* -- @Autowired */
    //IArduinoSpringBootAppPtr arduinoSpringBootApp;

    // Helper method to check last connected WiFi and restart app if invalid
    Private Void CheckAndRestartIfNeeded() {
        optional<WifiCredentials> lastWifi = GetLastConnectedWifi();
        
        // Check if last connected WiFi is not present or SSID is null/empty
        if (!lastWifi.has_value() || 
            !lastWifi.value().ssid.has_value() || 
            lastWifi.value().ssid.value().empty()) {
            // Last connected WiFi is invalid, restart the app
            /*if (arduinoSpringBootApp != nullptr) {
                arduinoSpringBootApp->RestartApp();
            } */
        }
    }

    // Add WiFi credentials
    Public Virtual WifiCredentials AddWifiCredentials(const WifiCredentials& credentials) override {
        // Save credentials
        WifiCredentials creds = credentials;
        WifiCredentials saved = wifiCredentialsRepository->Save(creds);
        
        // Check if last connected WiFi is still valid and restart if needed
        CheckAndRestartIfNeeded();
        
        return saved;
    }

    // Update WiFi credentials
    Public Virtual WifiCredentials UpdateWifiCredentials(const WifiCredentials& credentials) override {
        // Update credentials
        WifiCredentials creds = credentials;
        WifiCredentials updated = wifiCredentialsRepository->Update(creds);
        
        // Check if last connected WiFi is still valid and restart if needed
        CheckAndRestartIfNeeded();
        
        return updated;
    }

    // Delete WiFi credentials by SSID
    Public Virtual Void DeleteWifiCredentials(const StdString& ssid) override {
        // Check if this SSID is the last connected one
        StdVector<WifiConnection> connections = wifiConnectionRepository->FindAll();
        if (!connections.empty()) {
            WifiConnection connection = connections[0];
            if (connection.lastConnectedSsid.has_value() && 
                connection.lastConnectedSsid.value() == ssid) {
                // Clear the last connected SSID since we're deleting it
                connection.lastConnectedSsid = std::nullopt;
                wifiConnectionRepository->Update(connection);
            }
        }
        
        // Delete the credentials
        wifiCredentialsRepository->DeleteById(ssid);
        
        // Check if last connected WiFi is still valid and restart if needed
        CheckAndRestartIfNeeded();
    }

    // Read WiFi credentials by SSID
    Public Virtual WifiCredentials GetWifiCredentials(const StdString& ssid) override {
        optional<WifiCredentials> result = wifiCredentialsRepository->FindById(ssid);
        if (result.has_value()) {
            return result.value();
        }
        // Return empty WifiCredentials if not found
        return WifiCredentials();
    }

    // Get all WiFi credentials
    Public Virtual StdVector<WifiCredentials> GetAllWifiCredentials() override {
        return wifiCredentialsRepository->FindAll();
    }

    // Get last connected WiFi details
    Public Virtual optional<WifiCredentials> GetLastConnectedWifi() override {
        // Get all connection records (should only be one, but we'll check all)
        StdVector<WifiConnection> connections = wifiConnectionRepository->FindAll();
        
        if (connections.empty()) {
            return std::nullopt;
        }
        
        // Get the first connection record (assuming single record for last connected)
        // If multiple exist, we'll use the first one
        WifiConnection connection = connections[0];
        
        if (!connection.lastConnectedSsid.has_value() || connection.lastConnectedSsid.value().empty()) {
            return std::nullopt;
        }
        
        // Fetch credentials using the last connected SSID
        optional<WifiCredentials> credentials = wifiCredentialsRepository->FindById(connection.lastConnectedSsid.value());
        
        // Return nullopt if credentials don't exist
        if (!credentials.has_value()) {
            return std::nullopt;
        }
        
        return credentials;
    }

    // Update last connected SSID
    Public Virtual Void UpdateLastConnectedSsid(const StdString& ssid) override {
        // Get all connection records
        StdVector<WifiConnection> connections = wifiConnectionRepository->FindAll();
        
        WifiConnection connection;
        
        if (connections.empty()) {
            // Create new connection record
            connection.id = 1;
            connection.lastConnectedSsid = ssid;
            wifiConnectionRepository->Save(connection);
        } else {
            // Update existing connection record (use first one)
            connection = connections[0];
            connection.lastConnectedSsid = ssid;
            wifiConnectionRepository->Update(connection);
        }
    }
};

#endif // WIFISERVICE_H

