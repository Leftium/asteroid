#include "shot.h"
#include "flash.h"

Shot::Shot(CObject *parent): CObject(SHOT, parent)
{
    radius = 4;
    m      = 10;
    maxHealth = health = 50;
    // v(0,0);

    addForce(6*m, parent->bearing);

}

CollisionFlags Shot::collidesWith(CObject *o)
{
  if ((o->type == SHIP && o->team != team) ||
      (o->type == ROCK))
  {
      return CollisionFlags(PHYSICS_GIVE | LOGIC_TAKE);
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
        return false;
    }
    else
    {
        return true;
    }
}

void Shot::bumpedInto(CObject *o, vector2f v_delta)
{
    health = 0;
    world.addObject(new Flash(this));
}
