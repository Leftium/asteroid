#include "world.h"
#include "camera.h"

extern World world;

Camera::Camera(double height, objectPtrWeak t1, objectPtrWeak t2)
{
    target1 = t1;
    target2 = t2;

    x = world.w/2;
    y = world.h/2;
    w = 10;
    h = 10 * world.h/world.w;

    screenHeight = height;

    adjust();
}

double smoothAdjust(double current, double target, double range, double steps)
{
    if ((target - current) > 1)
    {
        return current + MAX((target - current) / steps, 1);
    }
    else if ((target - current) < -1)
    {
        return current + MIN((target - current) / steps, -1);
    }
    else return target;
}

void Camera::adjust()
{
    objectPtr t1 = target1.lock();
    objectPtr t2 = target2.lock();

    if (t1 || t2)
    {
        double wTarget, hTarget, xTarget, yTarget;
        if (t1 && t2)
        {
            double idealWidth  = MID(world.w/3, fabs(t1->p.x - t2->p.x) * 3 ,world.w);
            double idealHeight = MID(world.h/3, fabs(t1->p.y - t2->p.y) * 3 ,world.h);
            double ratio = MAX(idealWidth/world.w, idealHeight/world.h);

            wTarget = ratio*world.w;
            hTarget = ratio*world.h;

            xTarget = (t1->p.x + t2->p.x) / 2;
            yTarget = (t1->p.y + t2->p.y) / 2;
        }
        else
        {
            if (objectPtr t = (t1 ? t1 : t2))
            {
                wTarget = world.w / 3;
                hTarget = world.h / 3;
                xTarget = t->p.x / 2;
                yTarget = t->p.y / 2;
            }
        }

        w = smoothAdjust(w, wTarget, world.w, 10);
        h = smoothAdjust(h, hTarget, world.h, 10);
        x = smoothAdjust(x, xTarget, world.w, 10);
        y = smoothAdjust(y, yTarget, world.h, 10);

        fitInsideWorld(world);
    }
}

void Camera::fitInsideWorld(World &world)
{
    double delta;
    if (x - w/2 < 0)
    {
        x = w/2;
    }
    else if ((delta = ((x + w/2) - world.w)) > 0)
    {
        x -= delta;
    }

    if (y - h/2 < 0)
    {
        y = h/2;
    }
    else if ((delta = ((y + h/2) - world.h)) > 0)
    {
        y -= delta;
    }
}

vector2f Camera::World2Screen(double px, double py)
{
    return vector2f(               toScreenLength(px - (x - w/2)),
                    screenHeight - toScreenLength(py - (y - h/2)));
}

vector2f Camera::World2Screen(vector2f p)
{
    return World2Screen(p.x, p.y);
}

double Camera::toScreenLength(double length)
{
    return length * screenHeight/h;
}
