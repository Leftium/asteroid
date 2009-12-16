#include "sound.h"

Sound::Sound(SoundType type, double x, double y): CObject(SOUND)
{
    p.x = x;
    p.y = y;

    soundType   = type;
    beenUpdated = false;
}

CollisionFlags Sound::collidesWith(CObject *o)
{
    return NONE;
}

bool Sound::update()
{
    if (beenUpdated)
    {
        // has been updated once; thus has been rendered
        return true;
    }
    else
    {
        // delay deletion until rendered once 
        beenUpdated = true;
        return false;
    }
}
