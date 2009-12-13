#include "sound.h"

Sound::Sound(SoundType type, double x, double y)
{
    type_ = SOUND;
    soundType = type;

    px = x;
    py = y;

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
