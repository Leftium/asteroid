#ifndef KINPUT_H
#define KINPUT_H

#include "object.h"

class kinput: public CObject
{
protected:
    char k_forward, k_back, k_left, k_right, k_fire;
public:
    // constructors
    kinput() {};
    kinput(objectPtr ship, char forward, char back, char left, char right, char fire);

    CollisionFlags collidesWith(CObject *o);
    bool update();
};

#endif
