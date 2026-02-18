#ifndef ARDUINOAPP_H
#define ARDUINOAPP_H

#include <StandardDefines.h>
#include "IArduinoSpringBootApp.h"
#include "INetworkManager.h"
#include "ISpringBootCppApp.h"
#include <IArduinoRemoteStorage.h>
#include <IThreadPool.h>
#include <IDeviceTime.h>
#include <ILogger.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <atomic>
// Define print macros for Arduino
#define std_print(x) Serial.print(x)
#define std_println(x) Serial.println(x)
#endif

/* @Component */
class ArduinoSpringBootApp : public IArduinoSpringBootApp {
    Public Virtual ~ArduinoSpringBootApp() = default;

    /* @Autowired */
    Private INetworkManagerPtr networkManager;

    /* @Autowired */
    Private ISpringBootCppAppPtr springBootCppApp;

    /* @Autowired */
    Private IArduinoRemoteStoragePtr remoteStorage;

    /* @Autowired */
    Private IThreadPoolPtr threadPool;

    /* @Autowired */
    Private IDeviceTimePtr deviceTimeSync;

    /* @Autowired */
    Private ILoggerPtr logger;

    Private Static const ULong kPublishLogsIntervalMs = 120000;  // 2 minutes
    Private std::atomic<ULong> lastPublishLogsMillis_{0};
    /** True while a PublishLogs() call is in flight; prevents starting another until we get a result. */
    Private std::atomic<bool> publishInProgress_{false};

    Private Void TryPublishLogs() {
        if (millis() - lastPublishLogsMillis_.load(std::memory_order_relaxed) < kPublishLogsIntervalMs) return;
        if (publishInProgress_.exchange(true)) return;  // another publish already in flight; wait for next call

        Bool submitted = threadPool->Submit([this]() {
            FirebaseOperationResult res = remoteStorage->PublishLogs();
            publishInProgress_.store(false);
            if (res == FirebaseOperationResult::OperationSucceeded) {
                lastPublishLogsMillis_.store((ULong)millis(), std::memory_order_release);
            }
        });
        if (!submitted) {
            publishInProgress_.store(false);
        } else {
            publishInProgress_.store(true);  // task in flight; cleared when task completes
        }
    }

    Public Virtual Bool StartApp() override {
        logger->Info(Tag::Untagged, StdString("[ArduinoSpringBootApp] Starting app..."));
        // First connect to network
        networkManager->EnsureNetworkConnectivity();

        // Let WiFi and DNS stabilize before NTP (avoids 1970 when sync runs too early)
        delay(2000);

        // Set device time from NTP while network is up
        deviceTimeSync->SyncTimeFromNetwork();

        // Then start the Spring Boot application
        return springBootCppApp->StartApp();
    }

    Public Virtual Void StopApp() override {
        logger->Info(Tag::Untagged, StdString("[ArduinoSpringBootApp] Stopping app..."));
        // First disconnect from network
        networkManager->DisconnectNetwork();
        
        // Then stop the Spring Boot application
        springBootCppApp->StopApp();
    }

    Public Virtual Bool RestartApp() override {
        logger->Info(Tag::Untagged, StdString("[ArduinoSpringBootApp] Restarting app..."));
        // Stop the application (calls StopApp of this class)
        StopApp();
        
        // Start the application (calls StartApp of this class)
        Bool result = StartApp();
        
        return result;
    }

    Public Virtual Void ListenToRequest() override {
        networkManager->EnsureNetworkConnectivity();
        springBootCppApp->ListenToRequest();
        TryPublishLogs();
    }
};

#endif // ARDUINOAPP_H

