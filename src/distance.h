// distance.h										//
// distance(double, double, double, double)	 		//

#ifndef DISTANCE_H
#define DISTANCE_H

#include <math.h>

inline double Distance(double x1, double y1, double x2, double y2)
{
	return sqrt(pow(x1-x2, 2) + pow(y1-y2, 2));
}

#endif
