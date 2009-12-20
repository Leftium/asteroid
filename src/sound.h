#pragma once

#include "object.h"

class Sound;

typedef std::tr1::shared_ptr<Sound> SoundPtr;

enum SoundType { ENGINE, SHOOT, BOOM };

class Sound: public CObject
{
protected:
    SoundType soundType;

    bool beenUpdated;
    
public:
    // constructors
    Sound(SoundType type, double x, double y);

    CollisionFlags collidesWith(CObject *o);
    bool update();

    friend class World;
};
