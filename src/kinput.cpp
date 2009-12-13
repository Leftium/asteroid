#include "kinput.h"

kinput::kinput(objectPtr ship, char forward, char back, char left, char right, char fire)
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
        if(key[k_forward])
        {
            ship->addForce(7, ship->bearing);
            // TODO: move outside kinput object
            // fPlayEngine2 = TRUE;
        }

        if(key[k_back])
        {
            ship->addForce(-7, ship->bearing);
            // fPlayEngine2 = TRUE;
        }

        if(key[k_left])
        {
           ship->Rotate( 3 * FIX_PER_RAD);
        }

        if(key[k_right])
        {
            ship->Rotate(-3 * FIX_PER_RAD);
        }

        if(key[k_fire])
        {
            ship->fire();
        }
        return false;
    }
    return true;
}
