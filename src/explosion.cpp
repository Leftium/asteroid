#include "explosion.h"

Explosion::Explosion(CObject *source): CObject(EXPLOSION, source)
{
    maxHealth = health = 30;
    radius = 12;
}

CollisionFlags Explosion::collidesWith(CObject *o)
{
    return NONE;
}

bool Explosion::update()
{
    if (health-- > 0)
    {
        applyForces();
        Rotate(10 * FIX_PER_RAD);
        return false;
    }
    else
    {
        return true;
    }
}
