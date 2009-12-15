#include "util.h"

int Rnd(int max)
{
    return AL_RAND() / (RAND_MAX / max + 1);
}

double randf()
{
    return double(AL_RAND()) / (double(RAND_MAX) + 1.0);
}

// convert radians (0 == E, O to 2PI)
// to Allegro fixed point binary angle (0 == N, 0 to 256)
fixed RAD2FIX(double r)
{
    return itofix((int(256+64 - (r * RAD_PER_FIX))) % 256);
}

double squareDistance(double x1, double y1, double x2, double y2)
{
    return pow(x1 - x2, 2) + pow(y1 - y2, 2);
}

double relativeAngle(double x1, double y1, double x2, double y2)
{
    double angle = atan2(y2 - y1, x2 - x1 + DBL_MIN);
    if (angle >= 0)
    {
        return angle;
    }
    else
    {
        return angle + (2 * M_PI);
    }
}

