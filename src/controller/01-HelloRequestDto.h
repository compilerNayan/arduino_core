#ifndef HELLOREQUESTDTO_H
#define HELLOREQUESTDTO_H

#include <StandardDefines.h>

/* @Serializable */
class HelloRequestDto {
public:
    optional<int> number;
    optional<StdString> message;
};

#endif

