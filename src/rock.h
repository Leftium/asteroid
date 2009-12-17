#pragma once

#include "object.h"

class Rock: public CObject
{
public:
    // constructors
    Rock::Rock(Rock *parent=NULL);

    virtual CollisionFlags collidesWith(CObject *o);
    virtual bool update();
    virtual void bumpedInto(CObject *o, vector2f v_delta);
};
