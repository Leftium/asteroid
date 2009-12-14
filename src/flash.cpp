#include "flash.h"

Flash::Flash(CObject *source): CObject(FLASH, source)
{
    maxHealth = health = 10;
    radius = 1;
}

CollisionFlags Flash::collidesWith(CObject *o)
{
    return NONE;
}

bool Flash::update()
{
    if (health-- > 0)
    {
        if (health > 5)
        {
            radius *= 1.4;
        }
        else
        {
            radius *= 0.6;
        }

        return false;
    }
    else
    {
        return true;
    }
}

