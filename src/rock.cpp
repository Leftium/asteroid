#include "rock.h"
#include "explosion.h"

Rock::Rock(Rock *parent): CObject(ROCK, parent)
{
    const double stdMass   = 100;
    const double stdRadius = 7;
    const double stdHealth = 100;

    double ratio = 1;
    if (parent != NULL)
    {
        m = parent->m / 2;
        parent->m -= m;

        ratio = m/stdMass;

        vx += (double(AL_RAND()) / RAND_MAX * 2) - 1;
        vy += (double(AL_RAND()) / RAND_MAX * 2) - 1;
    }
    else
    {
        ratio = 4;
    }
    m         = ratio * stdMass;
    radius    = ratio * stdRadius;
    health    = ratio * stdHealth;
    maxHealth = health;
}

CollisionFlags Rock::collidesWith(CObject *o)
{
  if ((o->type == SHIP && o->team != team) ||
      (o->type == ROCK))
  {
      return ALL;
  }
  else
  {
    return NONE;
  }  
}

bool Rock::update()
{
    if (health > 0)
    {
        Rotate(1 * FIX_PER_RAD);
        applyForces();
        return false;
    }
    else
    {
        // explosion
        objects.push_back(objectPtr(new Explosion(this)));

        // spawn some smaller rocks
        while (m > 50)
        {
           objects.push_back(objectPtr(new Rock(this)));
        }
        return true;
    }
}

void Rock::bumpedInto(CObject *o)
{
    switch(o->type)
    {
        case SHOT:
            health -= 34;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);
            break;

        case ROCK:
        case SHIP:
            health -= 5;
            Rnd(2) ? Rotate(20 * FIX_PER_RAD) : Rotate(-20 * FIX_PER_RAD);
            break;

        default:
            break;
    } 
}
