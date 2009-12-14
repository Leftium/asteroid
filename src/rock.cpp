#include "rock.h"
#include "explosion.h"

Rock::Rock(): CObject(ROCK)
{
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
        objects.push_back(objectPtr(new Explosion(this)));
        return true;
    }
}

void Rock::bumpedInto(CObject *o)
{
    switch(o->type)
    {
        case SHOT:
            health -= 2;
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
