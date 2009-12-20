#pragma once

#include <list>

#include "object.h"
#include "camera.h"

class World
{
protected:

public:
    const int w;    // width
    const int h;    // height

    std::list< objectPtr > objects;

    World(int width, int height);

    void addObject(CObject  *o);
    objectPtr lastObject();

    void wrapToWorld(objectPtr o);

    void update();
    void render(BITMAP *buf);
    void render(BITMAP *buf, objectPtr o);
    void render(BITMAP *buf, Camera c);
};
