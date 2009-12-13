#pragma once

#include <allegro.h>
#include <math.h>
#include <float.h>

#define RAD_PER_FIX (128.0 / M_PI)
#define FIX_PER_RAD (M_PI / 128.0)

int Rnd(int max);

// convert radians (0 == East, O to 2PI)
// to Allegro fixed point binary angle (0 == North, 0 to 256)
fixed RAD2FIX(double r);
double squareDistance(double x1, double y1, double x2, double y2);
double relativeAngle(double x1, double y1, double x2, double y2);