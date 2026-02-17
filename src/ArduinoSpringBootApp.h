#ifndef ARDUINOAPP_H
#define ARDUINOAPP_H

#include <StandardDefines.h>
#include "IArduinoSpringBootApp.h"
#include "INetworkManager.h"
#include "ISpringBootCppApp.h"
#include <IArduinoRemoteStorage.h>

#ifdef ARDUINO
#include <Arduino.h>
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

    Private Static const ULong kPublishLogsIntervalMs = 120000;  // 2 minutes
    Private ULong lastPublishLogsMillis_{0};

    Public Virtual Bool StartApp() override {
        // First connect to network
        networkManager->EnsureNetworkConnectivity();

        // Then start the Spring Boot application
        return springBootCppApp->StartApp();
        
        return false;
    }

    Public Virtual Void StopApp() override {
        // First disconnect from network
        networkManager->DisconnectNetwork();
        
        // Then stop the Spring Boot application
        springBootCppApp->StopApp();
    }

    Public Virtual Bool RestartApp() override {
        // Stop the application (calls StopApp of this class)
        StopApp();
        
        // Start the application (calls StartApp of this class)
        Bool result = StartApp();
        
        return result;
    }

    Public Virtual Void ListenToRequest() override {
        networkManager->EnsureNetworkConnectivity();
        springBootCppApp->ListenToRequest();
        if (remoteStorage && (millis() - lastPublishLogsMillis_ >= kPublishLogsIntervalMs)) {
            remoteStorage->PublishLogs();
            lastPublishLogsMillis_ = millis();
        }
    }
};

#endif // ARDUINOAPP_H

