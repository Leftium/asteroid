#pragma once

#include "ship.h"
#include "object.h"

class Rock: public CObject
{
public:
    // constructors
    Rock::Rock(Rock *parent=NULL);
    Rock::Rock(Ship *parent);

    virtual CollisionFlags collidesWith(CObject *o);
    virtual bool update();
    virtual void bumpedInto(CObject *o, vector2f v_delta);
};
