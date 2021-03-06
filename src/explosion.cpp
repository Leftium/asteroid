#include "explosion.h"

Explosion::Explosion(CObject *source): CObject(EXPLOSION, source)
{
    maxHealth = health = 30;
    maxRadius = radius * 2;
    radius = 1;
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
        if (health  > 15)
        {
            if (radius < maxRadius)
            {
                radius *= 1.5;
            }
        }
        else
        {
            radius *= 0.5;
        }
        return false;
    }
    else
    {
        return true;
    }
}
