#ifndef _WIFI_CONNECTION_REPOSITORY_H_
#define _WIFI_CONNECTION_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/WifiConnection.h"

/* @Repository */
DefineStandardPointers(WifiConnectionRepository)
class WifiConnectionRepository : public CpaRepository<WifiConnection, int> {
    Public Virtual ~WifiConnectionRepository() = default;
    
};

#endif // _WIFI_CONNECTION_REPOSITORY_H_

