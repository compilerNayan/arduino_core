#ifndef _IHELLOCONTROLLER_H_
#define _IHELLOCONTROLLER_H_

#include <StandardDefines.h>
#include "01-HelloRequestDto.h"
#include "02-HelloResponseDto.h"

DefineStandardPointers(IHelloController)
class IHelloController {
public:
    virtual ~IHelloController() = default;

    virtual HelloResponseDto hello(HelloRequestDto request) = 0;
};

#endif

