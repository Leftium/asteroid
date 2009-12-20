#pragma once

#include "objectTypes.h"

class World;

class Camera
{
protected:
    double x,y;
    double w,h;

    double screenHeight;

    objectPtrWeak target1, target2;

public:
    Camera(double height, objectPtrWeak t1, objectPtrWeak t2);

    vector2f Camera::World2Screen(double px, double py);
    vector2f Camera::World2Screen(vector2f p);

    double toScreenLength(double length);

    void adjust();
    void fitInsideWorld(World &world);

    friend class World;
};
