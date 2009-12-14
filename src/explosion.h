#pragma once

#include "object.h"

class Explosion: public CObject
{
protected:

public:
    // constructor
    Explosion(CObject *source=NULL);

    CollisionFlags collidesWith(CObject *o);
    bool update();
};
