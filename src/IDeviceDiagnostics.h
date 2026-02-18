#ifndef IDEVICEDIAGNOSTICS_H
#define IDEVICEDIAGNOSTICS_H

#include <StandardDefines.h>

/**
 * Device diagnostics and analytics: crash state, and extensible for
 * temperature, RAM, and other device health info later.
 */
DefineStandardPointers(IDeviceDiagnostics)
class IDeviceDiagnostics {
    Public Virtual ~IDeviceDiagnostics() = default;

    /** Returns true if the previous run ended in a panic/exception (core dump). */
    Public Virtual Bool HadPreviousCrash() const = 0;
};

#endif // IDEVICEDIAGNOSTICS_H
