// bearing.h
// bearing(double, double, double, double)

#ifndef BEARING_H
#define BEARING_H

#include "distance.h"

inline int Bearing(double x1, double y1, double x2, double y2)
{
    int angle;

    angle = fixtoi(fasin(ftofix((x2-x1) / (Distance(x1, y1, x2, y2)+.000000000000000000000000000000000001))));

    if (angle >= 0 )
    {
        if (y1 > y2) return angle;
        else return 128 - angle;
    }
    if (angle < 0)
    {
        if (y1 > y2) return 256 + angle;
        else return 128 - angle;
    }
    return 0;
}

#endif
