#ifdef ARDUINO
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <StandardDefines.h>
#include "INetworkManager.h"
#include "service/IWifiService.h"
#include "entity/WifiCredentials.h"
#include <INetworkStatusProvider.h>
#include <ILogger.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#include <WiFiClient.h>
#endif
#include <Arduino.h>

namespace {
struct InternetCheckPair {
    const char* ip1;
    const char* ip2;
};
const InternetCheckPair kInternetCheckPairs[] = {
    { "208.67.222.123", "1.0.0.1" },       // 1: FamilyShield, Cloudflare
    { "8.8.4.4", "208.67.220.220" },       // 2: Google, OpenDNS
    { "156.154.70.1", "64.6.64.6" },       // 3: Neustar, Verisign
    { "195.46.39.39", "199.85.127.10" },   // 4: SafeDNS, Norton
    { "76.76.2.0", "94.140.15.15" },       // 5: Control D, AdGuard
    { "208.67.220.123", "156.154.71.1" },  // 6: FamilyShield, Neustar
    { "1.1.1.1", "208.67.222.222" },      // 7: Cloudflare, OpenDNS
    { "8.8.8.8", "64.6.65.6" },            // 8: Google, Verisign
    { "195.46.39.40", "76.76.10.0" },      // 9: SafeDNS, Control D
    { "199.85.126.10", "76.76.19.19" },    // 10: Norton, Alternate
};
const size_t kNumInternetCheckPairs = sizeof(kInternetCheckPairs) / sizeof(kInternetCheckPairs[0]);
}

/* @Component */
class NetworkManager : public INetworkManager {
    Public Virtual ~NetworkManager() = default;

    /* @Autowired */
    IWifiServicePtr wifiService;
    /* @Autowired */
    INetworkStatusProviderPtr networkStatusProvider_;
    /* @Autowired */
    ILoggerPtr logger;

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

    // Round-robin index for internet check pairs (no same provider in consecutive pairs)
    Private size_t nextInternetCheckPairIndex_ = 0;

    Private Bool HasInternet() {
        const InternetCheckPair& pair = kInternetCheckPairs[nextInternetCheckPairIndex_];
        nextInternetCheckPairIndex_ = (nextInternetCheckPairIndex_ + 1) % kNumInternetCheckPairs;

        WiFiClient client;
        if (client.connect(pair.ip1, 53, 2000)) {
            client.stop();
            return true;
        }
        if (client.connect(pair.ip2, 53, 2000)) {
            client.stop();
            return true;
        }
        return false;
    }

    // Helper method to attempt WiFi connection
    Private Bool TryConnectToWifi(const StdString& ssid, const StdString& password) {
        StdString msg = "[NetworkManager] Attempting to connect to WiFi - SSID: " + ssid;
        logger->Info(Tag::Untagged, msg);
        if (!password.empty()) {
            logger->Info(Tag::Untagged, StdString("[NetworkManager] Using password"));
        } else {
            logger->Info(Tag::Untagged, StdString("[NetworkManager] No password (open network)"));
        }

        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_STA);

        WiFi.begin(ssid.c_str(), password.empty() ? nullptr : password.c_str());
        currentMode = "wifi";

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            wifiConnectionId_ = random(1, 2147483647);
            logger->Info(Tag::Untagged, StdString("[NetworkManager] WiFi connected successfully! IP Address: " + StdString(WiFi.localIP().toString().c_str())));
            return true;
        } else {
            logger->Error(Tag::Untagged, StdString("[NetworkManager] WiFi connection failed or timeout for SSID: " + ssid));
            return false;
        }
    }

    // Connect to network (WiFi or start hotspot)
    Public Virtual Void ConnectNetwork() override {
        logger->Info(Tag::Untagged, StdString("[NetworkManager] ConnectNetwork() called"));

        Bool connected = false;

        logger->Info(Tag::Untagged, StdString("[NetworkManager] Step 1: Checking for last connected WiFi..."));
        optional<WifiCredentials> lastWifi = wifiService->GetLastConnectedWifi();

        if (lastWifi.has_value() &&
            lastWifi.value().ssid.has_value() &&
            !lastWifi.value().ssid.value().empty()) {
            StdString ssid = lastWifi.value().ssid.value();
            StdString password = "";

            if (lastWifi.value().password.has_value()) {
                password = lastWifi.value().password.value();
            }

            logger->Info(Tag::Untagged, StdString("[NetworkManager] Last connected WiFi found - SSID: " + ssid));

            connected = TryConnectToWifi(ssid, password);

            if (connected) {
                wifiService->UpdateLastConnectedSsid(ssid);
                networkStatusProvider_->SetWifiConnectionId(wifiConnectionId_);
                networkStatusProvider_->SetWiFiConnected(true);
                logger->Info(Tag::Untagged, StdString("[NetworkManager] Successfully connected to last connected WiFi"));
                return;
            } else {
                logger->Warning(Tag::Untagged, StdString("[NetworkManager] Failed to connect to last connected WiFi, trying other credentials..."));
            }
        } else {
            logger->Info(Tag::Untagged, StdString("[NetworkManager] No last connected WiFi found"));
        }

        logger->Info(Tag::Untagged, StdString("[NetworkManager] Step 2: Getting all WiFi credentials from database..."));
        StdVector<WifiCredentials> allCredentials = wifiService->GetAllWifiCredentials();

        if (!allCredentials.empty()) {
            logger->Info(Tag::Untagged, StdString("[NetworkManager] Found " + std::to_string(allCredentials.size()) + " WiFi credential(s) in database"));

            for (size_t i = 0; i < allCredentials.size(); i++) {
                const WifiCredentials& cred = allCredentials[i];

                if (!cred.ssid.has_value() || cred.ssid.value().empty()) {
                    logger->Warning(Tag::Untagged, StdString("[NetworkManager] Skipping credential with empty SSID"));
                    continue;
                }

                StdString ssid = cred.ssid.value();
                StdString password = "";

                if (cred.password.has_value()) {
                    password = cred.password.value();
                }

                logger->Info(Tag::Untagged, StdString("[NetworkManager] Trying credential " + std::to_string(i + 1) + " of " + std::to_string(allCredentials.size()) + " - SSID: " + ssid));

                connected = TryConnectToWifi(ssid, password);

                if (connected) {
                    wifiService->UpdateLastConnectedSsid(ssid);
                    networkStatusProvider_->SetWifiConnectionId(wifiConnectionId_);
                    networkStatusProvider_->SetWiFiConnected(true);
                    logger->Info(Tag::Untagged, StdString("[NetworkManager] Successfully connected to WiFi: " + ssid));
                    logger->Info(Tag::Untagged, StdString("[NetworkManager] Updated last connected WiFi"));
                    return;
                }
            }

            logger->Warning(Tag::Untagged, StdString("[NetworkManager] All WiFi credentials failed to connect"));
        } else {
            logger->Warning(Tag::Untagged, StdString("[NetworkManager] No WiFi credentials found in database"));
        }

        logger->Info(Tag::Untagged, StdString("[NetworkManager] Step 3: Starting hotspot (no WiFi connections available or all failed)"));
        logger->Info(Tag::Untagged, StdString("[NetworkManager] Hotspot SSID: Mishulika (open, no password)"));

        WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_AP);

        Bool apStarted = WiFi.softAP("SmartBoard", nullptr);
        if (apStarted) {
            currentMode = "hotspot";
            networkStatusProvider_->SetWiFiConnected(false);
            networkStatusProvider_->SetInternetConnected(false);
            networkStatusProvider_->SetWifiConnectionId(0);
            logger->Info(Tag::Untagged, StdString("[NetworkManager] Hotspot started successfully! AP IP Address: " + StdString(WiFi.softAPIP().toString().c_str())));
        } else {
            logger->Error(Tag::Untagged, StdString("[NetworkManager] Failed to start hotspot"));
        }
    }

    // Disconnect from network (stop WiFi or hotspot)
    Public Virtual Void DisconnectNetwork() override {
        logger->Info(Tag::Untagged, StdString("[NetworkManager] DisconnectNetwork() called"));

        WiFiMode_t mode = WiFi.getMode();

        if (mode == WIFI_AP || mode == WIFI_AP_STA) {
            logger->Info(Tag::Untagged, StdString("[NetworkManager] Stopping hotspot"));
            WiFi.softAPdisconnect(true);
        }

        if (mode == WIFI_STA || mode == WIFI_AP_STA) {
            if (WiFi.status() == WL_CONNECTED) {
                logger->Info(Tag::Untagged, StdString("[NetworkManager] Disconnecting WiFi - Previous IP: " + StdString(WiFi.localIP().toString().c_str())));
            }
            WiFi.disconnect();
        }

        networkStatusProvider_->SetWiFiConnected(false);
        networkStatusProvider_->SetInternetConnected(false);
        networkStatusProvider_->SetWifiConnectionId(0);

        currentMode = "";
        logger->Info(Tag::Untagged, StdString("[NetworkManager] Network disconnected"));
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
            logger->Info(Tag::Untagged, StdString(w ? "[NetworkManager] State: WiFi connected" : "[NetworkManager] State: WiFi disconnected"));
        }
        if (inet != internetConnected_) {
            internetConnected_ = inet;
            logger->Info(Tag::Untagged, StdString(inet ? "[NetworkManager] State: Internet connected" : "[NetworkManager] State: Internet disconnected"));
        }
        if (hot != hotspotActive_) {
            hotspotActive_ = hot;
            logger->Info(Tag::Untagged, StdString(hot ? "[NetworkManager] State: Hotspot active" : "[NetworkManager] State: Hotspot inactive"));
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
        logger->Info(Tag::Untagged, StdString("[NetworkManager] RestartNetwork() called"));
        DisconnectNetwork();
        delay(1000);
        logger->Info(Tag::Untagged, StdString("[NetworkManager] Reconnecting..."));
        ConnectNetwork();
    }
};

#endif // NETWORKMANAGER_H
#endif // ARDUINO   
