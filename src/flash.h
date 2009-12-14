#pragma once

#include "explosion.h"

class Flash: public CObject
{
protected:

public:
    // constructor
    Flash(CObject *source=NULL);

    CollisionFlags collidesWith(CObject *o);
    bool update();
};
