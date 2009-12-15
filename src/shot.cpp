#include "shot.h"
#include "flash.h"

Shot::Shot(CObject *parent): CObject(SHOT, parent)
{
    radius = 4;
    m      = 10;
    maxHealth = health = 50;

    addForce(4*m, parent->bearing);
}

CollisionFlags Shot::collidesWith(CObject *o)
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

bool Shot::update()
{
    if (health-- > 0)
    {
        applyForces();
        Rotate(6 * FIX_PER_RAD);
        return false;
    }
    else
    {
        return true;
    }
}

void Shot::bumpedInto(CObject *o)
{
    health = 0;
    objects.push_back(objectPtr(new Flash(this)));
}
