#ifdef ARDUINO
#ifndef NETWORKMANAGERTHREAD_H
#define NETWORKMANAGERTHREAD_H

#include <StandardDefines.h>
#include "INetworkManagerThread.h"

/* @Component */
class NetworkManagerThread : public INetworkManagerThread {
    Public Virtual ~NetworkManagerThread() = default;

    Public Virtual Void Run() override {
        (void)0;  // dummy for now
    }
};

#endif // NETWORKMANAGERTHREAD_H
#endif // ARDUINO
