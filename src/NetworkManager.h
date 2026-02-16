#ifdef ARDUINO
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <StandardDefines.h>
#include "INetworkManager.h"
#include "service/IWifiService.h"
#include "entity/WifiCredentials.h"
#include "INetworkStatusProvider.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>

// Define print macros for Arduino
#define std_print(x) Serial.print(x)
#define std_println(x) Serial.println(x)

/* @Component */
class NetworkManager : public INetworkManager {
    Public Virtual ~NetworkManager() = default;

    /* @Autowired */
    IWifiServicePtr wifiService;
    /* @Autowired */
    INetworkStatusProviderPtr networkStatusProvider_;

    // Track current mode: "wifi" or "hotspot"
    Private StdString currentMode;
    // WiFi connection id (random when connecting to WiFi; 0 when not on WiFi)
    Private Int wifiConnectionId_ = 0;
    // Cached state for change detection (log only when state changes)
    Private Bool wifiConnected_ = false;
    Private Bool internetConnected_ = false;
    Private Bool hotspotActive_ = false;
    // Throttle internet check to every 5 seconds when WiFi connected
    Private ULong lastInternetCheckMillis_ = 0;
    static const ULong kInternetCheckIntervalMs = 5000;

    Private Bool HasInternet() {
        WiFiClient client;
        if (!client.connect("8.8.8.8", 53, 2000)) {
            return false;
        }
        client.stop();
        return true;
    }

    // Helper method to attempt WiFi connection
    Private Bool TryConnectToWifi(const StdString& ssid, const StdString& password) {
        std_print("[NetworkManager] Attempting to connect to WiFi - SSID: ");
        std_println(ssid.c_str());
        if (!password.empty()) {
            std_println("[NetworkManager] Using password");
        } else {
            std_println("[NetworkManager] No password (open network)");
        }
        
        // Disconnect any existing connection first
        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_STA);
        
        // Connect to WiFi
        WiFi.begin(ssid.c_str(), password.empty() ? nullptr : password.c_str());
        currentMode = "wifi";
        
        // Wait for connection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnectionId_ = random(1, 2147483647);
            std_print("[NetworkManager] WiFi connected successfully! IP Address: ");
            std_println(WiFi.localIP().toString().c_str());
            return true;
        } else {
            std_print("[NetworkManager] WiFi connection failed or timeout for SSID: ");
            std_println(ssid.c_str());
            return false;
        }
    }

    // Connect to network (WiFi or start hotspot)
    Public Virtual Void ConnectNetwork() override {
        std_println("[NetworkManager] ConnectNetwork() called");
        
        Bool connected = false;
        
        // Step 1: Check if there's a last connected WiFi
        std_println("[NetworkManager] Step 1: Checking for last connected WiFi...");
        optional<WifiCredentials> lastWifi = wifiService->GetLastConnectedWifi();
        
        if (lastWifi.has_value() && 
            lastWifi.value().ssid.has_value() && 
            !lastWifi.value().ssid.value().empty()) {
            // Try connecting to last connected WiFi
            StdString ssid = lastWifi.value().ssid.value();
            StdString password = "";
            
            if (lastWifi.value().password.has_value()) {
                password = lastWifi.value().password.value();
            }
            
            std_print("[NetworkManager] Last connected WiFi found - SSID: ");
            std_println(ssid.c_str());
            
            connected = TryConnectToWifi(ssid, password);
            
            if (connected) {
                // Update last connected SSID (in case it changed)
                wifiService->UpdateLastConnectedSsid(ssid);
                networkStatusProvider_->SetWifiConnectionId(wifiConnectionId_);
                networkStatusProvider_->SetWiFiConnected(true);
                std_println("[NetworkManager] Successfully connected to last connected WiFi");
                return;
            } else {
                std_println("[NetworkManager] Failed to connect to last connected WiFi, trying other credentials...");
            }
        } else {
            std_println("[NetworkManager] No last connected WiFi found");
        }
        
        // Step 2: If last WiFi not present or connection failed, try all WiFi credentials
        std_println("[NetworkManager] Step 2: Getting all WiFi credentials from database...");
        StdVector<WifiCredentials> allCredentials = wifiService->GetAllWifiCredentials();
        
        if (!allCredentials.empty()) {
            std_print("[NetworkManager] Found ");
            std_print(std::to_string(allCredentials.size()).c_str());
            std_println(" WiFi credential(s) in database");
            
            // Try connecting to each credential one by one
            for (size_t i = 0; i < allCredentials.size(); i++) {
                const WifiCredentials& cred = allCredentials[i];
                
                if (!cred.ssid.has_value() || cred.ssid.value().empty()) {
                    std_println("[NetworkManager] Skipping credential with empty SSID");
                    continue;
                }
                
                StdString ssid = cred.ssid.value();
                StdString password = "";
                
                if (cred.password.has_value()) {
                    password = cred.password.value();
                }
                
                std_print("[NetworkManager] Trying credential ");
                std_print(std::to_string(i + 1).c_str());
                std_print(" of ");
                std_print(std::to_string(allCredentials.size()).c_str());
                std_print(" - SSID: ");
                std_println(ssid.c_str());
                
                connected = TryConnectToWifi(ssid, password);
                
                if (connected) {
                    // Update last connected SSID with the successful one
                    wifiService->UpdateLastConnectedSsid(ssid);
                    networkStatusProvider_->SetWifiConnectionId(wifiConnectionId_);
                    networkStatusProvider_->SetWiFiConnected(true);
                    std_print("[NetworkManager] Successfully connected to WiFi: ");
                    std_println(ssid.c_str());
                    std_println("[NetworkManager] Updated last connected WiFi");
                    return;
                }
            }
            
            std_println("[NetworkManager] All WiFi credentials failed to connect");
        } else {
            std_println("[NetworkManager] No WiFi credentials found in database");
        }
        
        // Step 3: If no entries or all connections failed, start hotspot
        std_println("[NetworkManager] Step 3: Starting hotspot (no WiFi connections available or all failed)");
        std_println("[NetworkManager] Hotspot SSID: Mishulika (open, no password)");
        
        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_AP);
        
        // Start hotspot with SSID "SmartBoard" and no password (open)
        Bool apStarted = WiFi.softAP("SmartBoard", nullptr);
        if (apStarted) {
            currentMode = "hotspot";
            networkStatusProvider_->SetWiFiConnected(false);
            networkStatusProvider_->SetInternetConnected(false);
            networkStatusProvider_->SetWifiConnectionId(0);
            std_print("[NetworkManager] Hotspot started successfully! AP IP Address: ");
            std_println(WiFi.softAPIP().toString().c_str());
        } else {
            std_println("[NetworkManager] Failed to start hotspot");
        }
    }

    // Disconnect from network (stop WiFi or hotspot)
    Public Virtual Void DisconnectNetwork() override {
        std_println("[NetworkManager] DisconnectNetwork() called");
        
        // Check actual WiFi mode to determine what to disconnect
        WiFiMode_t mode = WiFi.getMode();
        
        if (mode == WIFI_AP || mode == WIFI_AP_STA) {
            // Stop hotspot
            std_println("[NetworkManager] Stopping hotspot");
            WiFi.softAPdisconnect(true);
        }
        
        if (mode == WIFI_STA || mode == WIFI_AP_STA) {
            // Disconnect WiFi
            if (WiFi.status() == WL_CONNECTED) {
                std_print("[NetworkManager] Disconnecting WiFi - Previous IP: ");
                std_println(WiFi.localIP().toString().c_str());
            }
            WiFi.disconnect();
        }

        networkStatusProvider_->SetWiFiConnected(false);
        networkStatusProvider_->SetInternetConnected(false);
        networkStatusProvider_->SetWifiConnectionId(0);        
        
        currentMode = "";
        std_println("[NetworkManager] Network disconnected");
    }

    // Check if network is connected (WiFi or hotspot)
    Public Virtual Bool IsNetworkConnected() override {
        // Check if WiFi is connected
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        
        // Check if hotspot is active
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            return true;
        }
        
        return false;
    }

    Public Virtual Bool IsInternetConnected() override {
        if (WiFi.status() != WL_CONNECTED) {
            return false;
        }
        return HasInternet();
    }

    Public Virtual Int GetWifiConnectionId() override {
        return wifiConnectionId_;
    }

    Public Virtual Bool EnsureNetworkConnectivity() override {
        if (WiFi.status() != WL_CONNECTED) {
            DisconnectNetwork();
            ConnectNetwork();
        }

        Bool w = (WiFi.status() == WL_CONNECTED);
        Bool inet = false;
        if (w) {
            ULong now = (ULong)millis();
            Bool wasConnected = internetConnected_;
            Bool throttle = wasConnected && (lastInternetCheckMillis_ != 0) && (now - lastInternetCheckMillis_ < kInternetCheckIntervalMs);
            Bool shouldCheck = !throttle;
            if (shouldCheck) {
                inet = HasInternet();
                lastInternetCheckMillis_ = now;
            } else {
                inet = internetConnected_;
            }
        } else {
            lastInternetCheckMillis_ = 0;
        }
        Bool hot = (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA);

        if (w != wifiConnected_) {
            wifiConnected_ = w;
            std_println(w ? "[NetworkManager] State: WiFi connected" : "[NetworkManager] State: WiFi disconnected");
        }
        if (inet != internetConnected_) {
            internetConnected_ = inet;
            std_println(inet ? "[NetworkManager] State: Internet connected" : "[NetworkManager] State: Internet disconnected");
        }
        if (hot != hotspotActive_) {
            hotspotActive_ = hot;
            std_println(hot ? "[NetworkManager] State: Hotspot active" : "[NetworkManager] State: Hotspot inactive");
        }

        if (networkStatusProvider_) {
            if (w) {
                networkStatusProvider_->SetWiFiConnected(true);
                networkStatusProvider_->SetWifiConnectionId(wifiConnectionId_);
                networkStatusProvider_->SetInternetConnected(inet);
            } else if (hot) {
                networkStatusProvider_->SetWiFiConnected(false);
                networkStatusProvider_->SetInternetConnected(false);
                networkStatusProvider_->SetWifiConnectionId(0);
            } else {
                networkStatusProvider_->SetWiFiConnected(false);
                networkStatusProvider_->SetInternetConnected(false);
            }
        }

        return IsNetworkConnected();
    }

    // Restart network (disconnect and reconnect)
    Public Virtual Void RestartNetwork() override {
        std_println("[NetworkManager] RestartNetwork() called");
        DisconnectNetwork();
        delay(1000); // Small delay before reconnecting
        std_println("[NetworkManager] Reconnecting...");
        ConnectNetwork();
    }
};

#endif // NETWORKMANAGER_H
#endif // ARDUINO   
