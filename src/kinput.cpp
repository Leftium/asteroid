#include "kinput.h"

kinput::kinput(objectPtr ship, char forward, char back, char left, char right, char fire): CObject(GENERIC, ship.get())
{
    addDependency(ship);

    k_forward = forward;
    k_back    = back;
    k_left    = left;
    k_right   = right;
    k_fire    = fire;
}

CollisionFlags kinput::collidesWith(CObject *o)
{
    return NONE;
}

bool kinput::update()
{
    if ( ShipPtr ship = std::tr1::dynamic_pointer_cast<Ship>(dependedObjects.front().lock()) )
    {
        if (!key[k_forward] && !key[k_back])
        {
            // ensure engine sound doesn't play
            ship->thrust(0);
        }

        if(key[k_forward])
        {
            ship->thrust(7);
        }

        if(key[k_back])
        {
            ship->thrust(-3);
        }

        if(key[k_left])
        {
            ship->steer(1);
        }
        else if(key[k_right])
        {
            ship->steer(-1);
        }

        if(key[k_fire])
        {
            ship->fire();
        }
        return false;
    }
    return true;
}
