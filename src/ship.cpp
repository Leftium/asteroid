#include "ship.h"

Ship::Ship(double x, double y, int team, double _bearing, int _health)
{
    type_ = SHIP;

    fx = 0;
    fy = 0;
    vx = 0;
    vy = 0;

    px = x;
    py = y;


    team_   = team;
    bearing = _bearing;
    health  = _health;

    radius  = 10;
    m       = 100;

    energy_     = 1000;
    reloadTime_ = 0;
}

CollisionFlags Ship::collidesWith(CObject *o)
{    
    if (team != o->team)
    {
        return ALL;
    }
    else
    {
        return NONE;
    }
}

bool Ship::update()
{
    if (health > 0)
    {
        // TODO: Move limits to Ship object subclass
        /// if (Ship2->speed >  2) Ship2->SetVelocity( 2);
        /// if (Ship2->speed < -2) Ship2->SetVelocity(-2);
        applyForces();
        /// Ship2->SetVelocity(Ship2->speed * .99);
        
        if (reloadTime_ > 0) reloadTime_--;
        if (energy_ < 1000) energy_ += abs(energy_/100)+2;
    }
    else
    {
        objects.push_front(objectPtr(new CObject(EXPLOSION, this) ));
        return true;
    }
    return false;
}

void Ship::bumpedInto(CObject *o)
{
    switch(o->type)
    {
        case SHIP:
            health -= 3;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);
            break;

        case SHOT:
            health--;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);

            break;
        case ROCK:
            health -= 5;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);
            break;

        default:
            break;
    }
    return;
}

void Ship::fire()
{
    if (energy_ > -10)
    {
        energy_ -= 30;
    }

    if (reloadTime_ == 0 && energy_ > 10)
    {
        objects.push_front(objectPtr(new CObject(SHOT, this)));
        reloadTime_ = 4;
        // play_sample(shoot, 64, PAN(Ship2->GetX()), 1000, 0);
    }
}
