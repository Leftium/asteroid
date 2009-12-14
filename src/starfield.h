#pragma once

#include "object.h"

class Starfield;
typedef std::tr1::shared_ptr<Starfield> StarfieldPtr;

// Based on star field code by Shawn Hargraeves
class Starfield: public CObject
{
protected:
    static const int MAX_STARS = 128;
    int star_count, star_count_count;

    int StarColor[8];

    volatile struct {
       fixed x, y, z;
       int ox, oy;
    } star[MAX_STARS];

public:
    // constructors
    Starfield();

    CollisionFlags collidesWith(CObject *o);
    bool update();

    void draw_starfield_3d(BITMAP *bmp);
};
