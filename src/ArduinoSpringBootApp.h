#ifndef ARDUINOAPP_H
#define ARDUINOAPP_H

#include <StandardDefines.h>
#include "IArduinoSpringBootApp.h"
#include "INetworkManager.h"
#include "ISpringBootCppApp.h"

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

    Public Virtual Bool StartApp() override {
        // First connect to network
        if (networkManager != nullptr) {
            networkManager->ConnectNetwork();
        }
        
        // Then start the Spring Boot application
        if (springBootCppApp != nullptr) {
            return springBootCppApp->StartApp();
        }
        
        return false;
    }

    Public Virtual Void StopApp() override {
        // First disconnect from network
        if (networkManager != nullptr) {
            networkManager->DisconnectNetwork();
        }
        
        // Then stop the Spring Boot application
        if (springBootCppApp != nullptr) {
            springBootCppApp->StopApp();
        }
    }

    Public Virtual Bool RestartApp() override {
        // Stop the application (calls StopApp of this class)
        StopApp();
        
        // Start the application (calls StartApp of this class)
        Bool result = StartApp();
        
        return result;
    }

    Public Virtual Void ListenToRequest() override {
        // Delegate to Spring Boot application's ListenToRequest
        if (springBootCppApp != nullptr) {
            springBootCppApp->ListenToRequest();
        }
    }
};

#endif // ARDUINOAPP_H

