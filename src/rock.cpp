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

        double parentRatio = parent->m / stdMass;
        parent->m         = parentRatio * stdMass;
        parent->radius    = sqrt(parentRatio) * stdRadius;
        parent->health    = parentRatio * stdHealth;
        parent->maxHealth = parent->health;

        ratio = m/stdMass;

        vx += (randf() * 2) - 1;
        vy += (randf() * 2) - 1;
    }
    else
    {
        ratio = 16;
    }
    m         = ratio * stdMass;
    radius    = sqrt(ratio) * stdRadius;
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

    if (health > 50)
    {
        if (pow(double(health) / double(maxHealth), 2) < randf())
        {
            objects.push_back(objectPtr(new Rock(this)));
        }
    }
}
