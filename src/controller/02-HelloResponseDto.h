#ifndef HELLORESPONSEDTO_H
#define HELLORESPONSEDTO_H

#include <StandardDefines.h>

/* @Serializable */
class HelloResponseDto {
public:
    optional<StdString> randomString1;
    optional<StdString> randomString2;
};

#endif

