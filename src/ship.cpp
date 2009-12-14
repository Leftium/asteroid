#include "ship.h"
#include "sound.h"
#include "explosion.h"
#include "shot.h"

Ship::Ship(double x, double y, int team, double _bearing, int _health): CObject(SHIP)
{
    px      = x;
    py      = y;
    team_   = team;
    bearing = _bearing;
    health  = _health;
    radius  = 10;
    m       = 100;

    energy_     = 1000;
    reloadTime_ = 0;
    isEngineOn_ = false;
}

CollisionFlags Ship::collidesWith(CObject *o)
{    
    if (o->type == ROCK || o->type == SHIP || (o->type == SHOT && team != o->team))
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
        if (energy_ < 1000) energy_ += abs(energy_/100)+1;
    }
    else
    {
        objects.push_back(objectPtr(new Explosion(this)));
        return true;
    }
    return false;
}

void Ship::bumpedInto(CObject *o)
{
    switch(o->type)
    {
        case SHIP:
            health -= 10;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);
            break;

        case SHOT:
            health -= 5;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);

            break;
        case ROCK:
            health -= 20;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);
            break;

        default:
            break;
    }
    return;
}

void Ship::fire()
{
    if (energy_ > 200 && reloadTime_ == 0)
    {
        objects.push_back(objectPtr(new Shot(this)));
        objects.push_back(objectPtr(new Sound(SHOOT, px, py)));

        energy_     = MAX(energy_ - 200, 0);
        reloadTime_ = 12;
    }
}

void Ship::thrust(int power)
{
    if (power != 0 && energy_ >= 50)
    {
        addForce(power, bearing);
        isEngineOn_ = true;
        energy_     = MAX(energy_ - 15, 0);
    }
    else
    {
        isEngineOn_ = false;
    }
}
