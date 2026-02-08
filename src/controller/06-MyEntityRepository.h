#ifndef _MYENTITY_REPOSITORY_H_
#define _MYENTITY_REPOSITORY_H_

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "05-MyEntity.h"

/* @Repository */
DefineStandardPointers(MyEntityRepository)
class MyEntityRepository : public CpaRepository<MyEntity, int> {
    Public Virtual ~MyEntityRepository() = default;
    
    // Additional custom repository methods can be added here
    // The base CRUD methods are inherited from CpaRepository
};

#endif // _MYENTITY_REPOSITORY_H_

