#pragma once

#include "objectTypes.h"

class World;

class Camera
{
protected:
    double x,y;
    int    w,h;
    double ratio;

    objectPtrWeak target1, target2;

public:
    Camera(objectPtrWeak t1, objectPtrWeak t2);

    void adjust();
    void fitInsideWorld(World &world);

    friend class World;
};
