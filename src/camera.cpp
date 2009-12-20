#include "world.h"
#include "camera.h"

extern World world;

Camera::Camera(objectPtrWeak t1, objectPtrWeak t2)
{
    target1 = t1;
    target2 = t2;

    adjust();
}

void Camera::adjust()
{
    objectPtr t1 = target1.lock();
    objectPtr t2 = target2.lock();

    if (t1 && t2)
    {
        double idealWidth  = MID(world.w/3, fabs(t1->p.x - t2->p.x) * 2 ,world.w);
        double idealHeight = MID(world.h/3, fabs(t1->p.y - t2->p.y) * 2 ,world.h);
        ratio = MAX(idealWidth/world.w, idealHeight/world.h);

        w = ratio*world.w;
        h = ratio*world.h;

        x = (t1->p.x + t2->p.x - w)/2;
        y = (t1->p.y + t2->p.y - h)/2;
    }
    else
    {
        if (objectPtr t = (t1 ? t1 : t2))
        {
            w = world.w/3;
            h = world.h/3;
            x = t->p.x - w/2;
            y = t->p.y - h/2;
        }
    }
    fitInsideWorld(world);
}

void Camera::fitInsideWorld(World &world)
{
    double delta;
    if (x < 0)
    {
        x = 0;
    }
    else if ((delta = ((x + w) - world.w)) > 0)
    {
        x -= delta;
    }

    if (y < 0)
    {
        y = 0;
    }
    else if ((delta = ((y + h) - world.h)) > 0)
    {
        y -= delta;
    }
}
