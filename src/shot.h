#pragma once

#include "object.h"

class Shot: public CObject
{
protected:

public:
    // constructors
    Shot::Shot(CObject *parent);

    virtual CollisionFlags collidesWith(CObject *o);
    virtual bool update();
    virtual void bumpedInto(CObject *o, vector2f v_delta);
};
