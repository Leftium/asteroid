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
    if (objectPtr ship = (dependedObjects.front()).lock())
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
            // TODO: move this logic to ship object
            objects.push_front(objectPtr(new CObject(SHOT, ship.get())));
            /*
            if (Energy2 > -10)
            {
                // Commmented out for dev purposes
                // Energy2 -= 30;
            }

            if (ShotDelay2 == 0 && Energy2 > 10)
            {
                objects.push_front(objectPtr(new CObject(SHOT, Ship2.get())));
                // action: ship: spawn projectile
                ShotDelay2 = 4;
                play_sample(shoot, 64, PAN(Ship2->GetX()), 1000, 0);
            }
            */
        }
        //  if (ShotDelay2 > 0) ShotDelay2--;
        // if (Energy2 < 1000) Energy2 += abs(Energy2/100)+2;
        return false;
    }
    return true;
}
