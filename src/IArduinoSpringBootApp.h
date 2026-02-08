#ifndef IARDUINOAPP_H
#define IARDUINOAPP_H

#include <StandardDefines.h>

DefineStandardPointers(IArduinoSpringBootApp)
class IArduinoSpringBootApp {
    Public Virtual ~IArduinoSpringBootApp() = default;

    /**
     * @brief Starts the Arduino application
     * Connects to network and starts the Spring Boot application
     * @return true if application started successfully, false otherwise
     */
    Public Virtual Bool StartApp() = 0;

    /**
     * @brief Stops the Arduino application
     * Disconnects from network and stops the Spring Boot application
     */
    Public Virtual Void StopApp() = 0;

    /**
     * @brief Restarts the Arduino application
     * Stops and then starts the application
     * @return true if application restarted successfully, false otherwise
     */
    Public Virtual Bool RestartApp() = 0;

    /**
     * @brief Listens for and processes HTTP requests
     * Delegates to Spring Boot application's ListenToRequest method
     */
    Public Virtual Void ListenToRequest() = 0;
};

#endif // IARDUINOAPP_H

