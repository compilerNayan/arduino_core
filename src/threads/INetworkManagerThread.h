#ifndef INETWORKMANAGERTHREAD_H
#define INETWORKMANAGERTHREAD_H

#include <StandardDefines.h>
#include <IRunnable.h>

/**
 * Runnable for the network manager thread (e.g. connectivity check loop).
 * Extends IRunnable for use with Thread or IThreadPool::Execute.
 */
DefineStandardPointers(INetworkManagerThread)
class INetworkManagerThread : public IRunnable {
    Public Virtual ~INetworkManagerThread() = default;
};

#endif // INETWORKMANAGERTHREAD_H
