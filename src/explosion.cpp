#include "explosion.h"

Explosion::Explosion(CObject *source): CObject(EXPLOSION, source)
{
    maxHealth = health = 30;
    radius = 5;
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

        if (health  > 15)
        {
            if (radius < 50)
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
