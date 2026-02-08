#ifndef _WIFI_CREDENTIALS_REPOSITORY_H_
#define _WIFI_CREDENTIALS_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/WifiCredentials.h"

/* @Repository */
DefineStandardPointers(WifiCredentialsRepository)
class WifiCredentialsRepository : public CpaRepository<WifiCredentials, StdString> {
    Public Virtual ~WifiCredentialsRepository() = default;
    
    // Additional custom repository methods can be added here
    // The base CRUD methods are inherited from CpaRepository
    // Note: SSID is the ID field, so FindById(ssid) can be used to find by SSID
};

#endif // _WIFI_CREDENTIALS_REPOSITORY_H_

