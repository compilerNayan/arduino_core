#ifndef MYENTITY_H
#define MYENTITY_H

#include <StandardDefines.h>

/* @Entity */
class MyEntity {
public:
    /* @Id */
    optional<int> id;
    optional<StdString> username;
    optional<StdString> password;
};

#endif

