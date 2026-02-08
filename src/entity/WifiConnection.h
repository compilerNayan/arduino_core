#ifndef WIFICONNECTION_H
#define WIFICONNECTION_H

#include <StandardDefines.h>

/* @Entity */
class WifiConnection {

    /* @Id */
    /* @NotNull */
    Public optional<int> id;

    Public optional<StdString> lastConnectedSsid;
};

#endif

