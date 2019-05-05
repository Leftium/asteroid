#include "rock.h"
#include "ship.h"
#include "sound.h"
#include "explosion.h"
#include "shot.h"

Ship::Ship(double x, double y, int team, double _bearing, int _health): CObject(SHIP)
{
    p.x     = x;
    p.y     = y;
    team_   = team;
    bearing = _bearing;
    health  = _health;
    radius  = 10;
    m       = 100;
    maxHealth = 100;

    energy_     = 1000;
    reloadTime_ = 0;
    isEngineOn_ = false;
    shootRocks  = false;
}

CollisionFlags Ship::collidesWith(CObject *o)
{
    if (o->type == ROCK || o->type == SHIP ||  (o->type == SHOT && team != o->team))
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

        w *= 0.995;
        
        if (reloadTime_ > 0) reloadTime_--;
        if (energy_ < 1000) energy_ += 10;
    }
    else
    {
        world.addObject(new Explosion(this));
        return true;
    }
    return false;
}

void Ship::bumpedInto(CObject *o, vector2f v_delta)
{
    CObject::bumpedInto(o, v_delta);
}

void Ship::fire()
{
    if (reloadTime_ == 0)
    {
        if (shootRocks)
        {
            world.addObject(new Rock(this));
        }
        else
        {
            world.addObject(new Shot(this));
        }
        world.addObject(new Sound(SHOOT, p.x, p.y));

        energy_     = MAX(energy_ - 100, 0);
        reloadTime_ = MID(0, 20 - 50*energy_/1000, 15);
    }
}

void Ship::thrust(int power)
{
    if (power != 0)
    {
        shootRocks = (power < 0);
        // addForce(power, bearing);
        vector2f ef;

        ef.x = cos(bearing) * power;
        ef.y = sin(bearing) * power;

        if (energy_ < 100)
        {
            ef *= 0.5;
        }

        vector2f dv = ef/m;   // add velocity from impulse

        vector2f new_v = v + dv;

        if (type != SHOT && (new_v.length() > v.length()))
        {
            // apply lorentz factor
            double c = 2;

            if (reloadTime_ == 0)
            {
                c = 4;
            }
            
            double b = 1 - v.length_squared()/(c*c);
            if (b <= 0) b = DBL_MIN;

            double lorentz_factor = 1/sqrt(b);

            dv /= lorentz_factor;
        }

        v += dv;
        // v *= 0.99;

        

        // Only apply the Lorentz factor to the resulting velocity
        // vector's magnitude; not it's direction
        if (v.length() > 0)
        {
            v = new_v.normalized() * v.length();
        }

        if (reloadTime_ != 0)
        {
        //    v *= 0.95;
        }
        



        isEngineOn_ = true;
        energy_     = MAX(energy_ - 5, 0);

        w *= 0.90; // dampen angular motion
    }
    else
    {
        isEngineOn_ = false;
    }
}

void Ship::steer(int direction)
{
    double dw = direction;

    if (speed < 0.025 && ((direction > 0) == (w > 0)))
    {
        dw *= 0.0015;
    }
    else
    {
        dw *= 0.009;
    }

    if (reloadTime_ != 0)
    {
        dw *= 0.5;
    }

    double w_new = w + dw;

    if (abs(w_new) > abs(w))
    {
        // apply lorentz factor
        double max_w = 0.05;

        if (reloadTime_ == 0)
        {
                max_w = 0.2;
        }

        double b = 1 - w*w/(max_w*max_w);
        if (b <= 0) b = DBL_MIN;

        double lorentz_factor = 1/sqrt(b);

        dw /= lorentz_factor;
    }

    w += dw;
}
