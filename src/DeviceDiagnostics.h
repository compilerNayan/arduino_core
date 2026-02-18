#ifdef ARDUINO
#ifndef DEVICEDIAGNOSTICS_H
#define DEVICEDIAGNOSTICS_H

#include <StandardDefines.h>
#include "IDeviceDiagnostics.h"

#if defined(ESP32)
#include <esp_system.h>
#endif

/* @Component */
class DeviceDiagnostics : public IDeviceDiagnostics {
    Public Virtual ~DeviceDiagnostics() = default;

    Public Virtual Bool HadPreviousCrash() const override {
#if defined(ESP32)
        return (esp_reset_reason() == ESP_RST_PANIC);
#else
        return false;
#endif
    }
};

#endif // DEVICEDIAGNOSTICS_H
#endif // ARDUINO
