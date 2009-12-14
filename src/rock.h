#pragma once

#include "object.h"

class Rock: public CObject
{
protected:

public:
    // constructors
    Rock::Rock();

    virtual CollisionFlags collidesWith(CObject *o);
    virtual bool update();
    virtual void bumpedInto(CObject *o);
};
