#include <string>
#include <sstream>

#include "object.h"
#include "sound.h"
#include "explosion.h"
#include "flash.h"
#include "text.h"

bool SHOW_DAMAGE = true;

CollisionFlags CObject::collidesWith(CObject *o)
{
    return NONE;
}

void CObject::addDependency(objectPtr obj)
{
    if (obj)
    {
        dependedObjects.push_back(objectPtrWeak(obj));
    }
}

bool CObject::checkDependencies()
{
    objectIterWeak iter_i = dependedObjects.begin();
    while(iter_i != dependedObjects.end())
    {
        objectPtrWeak o = *iter_i;
        if (o.expired())
        {
            return true;
        }
        iter_i++;
    }
    return false;
}

bool CObject::update()
{
    // derived class should handle any action required
    return false;
}

void CObject::resolveCollision(CObject *obj2, CObject *obj1, vector2f n)
{
    // n is the normal vector to the collision
    double   r  = obj1->m / obj2->m;
    vector2f u  = obj1->v - obj2->v;
    vector2f un = u.projected(n);
    vector2f ut = u - un;
    vector2f vn = un * (r-1)/(r+1);
    vector2f wn = un * 2 * r / (r+1);

    obj1->v = ut + vn + obj2->v;
    obj2->v = wn +      obj2->v;
}

// return time to collision
double CObject::circleCircleCollision(CObject *cir1, CObject *cir2)
{
    vector2f w = cir1->p - cir2->p;
    double   r = cir1->radius + cir2->radius;
    double   ww = w*w;

    if (ww < r*r)
    {
        // objects already overlap; ignore
        return 666;
    }

    vector2f v = cir1->v - cir2->v;

    double a = v*v + DBL_MIN;
    double b = w*v;
    double c = ww - r*r;

    double root = b*b - a*c;

    if (root < 0)
    {
        // objects not on collision course
        return 999;
    }

    double t = (-b - sqrt(root)) / a;

    return t;
}

void CObject::resolveCollisionRadialVelocity(CObject *p, vector2f qv_orig, vector2f n)
{
    double deltaW = n.clockwise(qv_orig) / p->m;
    p->w -= deltaW;

    if (SHOW_DAMAGE)
    {
        std::stringstream ss;
        ss << int(-deltaW * 360 / (2 * M_PI) * 100);
        world.addObject(new Text(p->p.x+20, p->p.y, ss.str(), p));
    }
}

bool CObject::handleCollision(CObject *p, CObject *q)
{
    // check if collision matters
    if (p->collidesWith(q) == NONE &&
        q->collidesWith(p) == NONE)
    {
        return false;
    }

    double t = circleCircleCollision(p, q);
    if (t < 0 || t > 1)
    {
        // collision not in this time frame
        return false;
    }

    bool doPhysicsP = (p->collidesWith(q) & PHYSICS_TAKE);
    bool doPhysicsQ = (q->collidesWith(p) & PHYSICS_TAKE);

    vector2f pv_orig = p->v;
    vector2f qv_orig = q->v;

    // move objects to point of collision
    p->p += p->v * t;
    q->p += q->v * t;

    // find point of collision & normal
    vector2f collisionNormal = (q->p - p->p);
    collisionNormal.normalize();

    // resolve collision
    resolveCollision(q, p, collisionNormal);

    if (doPhysicsP)
    {
        resolveCollisionRadialVelocity(p, qv_orig, collisionNormal);
    }
    else
    {
        p->v = pv_orig;
    }

    if (doPhysicsQ)
    {
        resolveCollisionRadialVelocity(q, pv_orig, collisionNormal);
    }
    else
    {
        q->v = qv_orig;
    }

    // do final collision logic
    bool doLogicP = (p->collidesWith(q) & LOGIC_TAKE) != 0;
    bool doLogicQ = (q->collidesWith(p) & LOGIC_TAKE) != 0;

    if (doLogicP)
    {
        p->bumpedInto(q, p->v - pv_orig);
    }

    if (doLogicQ)
    {
        q->bumpedInto(p, q->v - qv_orig);
    }
    world.addObject(new Sound(BOOM, (p->p.x + q->p.x) / 2, (p->p.y + q->p.y) / 2 ));
    return true;
}

CObject::CObject(ObjectType _type, CObject *parent)
{
    static int currentId    = 0;
    static int nextGenericX = 4;

    double randomHeading = randf()*2*M_PI;
    double randomBearing = randf()*2*M_PI;

    id = currentId++;
    f.x = 0;
    f.y = 0;

    if (parent != NULL)
    {
        team_ = parent->team;
    }
    else
    {
        team_ = 0;
    }

    switch (_type)
    {
        case ROCK:
            if (parent == NULL)
            {
                setEverything(
                        ROCK,
                        Rnd(world.w),
                        Rnd(world.h),
                        0.1,            // speed
                        5,              // radius
                        0,              // radial velocity
                        10,             // health
                        randomHeading,  // heading
                        randomBearing,  // bearing
                        200);           // mass
            }
            else
            {
                setEverything( _type, parent->p.x, parent->p.y, parent->speed, parent->radius, parent->w, parent->health, parent->heading, parent->bearing, parent->m);
            }
            break;

        case GENERIC:
            setEverything(
                    GENERIC,
                    nextGenericX,
                    12,
                    0,               // speed
                    3,               // radius
                    0,               // radial velocity
                    99,              // health
                    1.1,             // heading
                    1.1);            // bearing
            nextGenericX += 12;
            break;

        default:
            if (parent != NULL)
            {
                setEverything( _type, parent->p.x, parent->p.y, parent->speed, parent->radius, parent->w, parent->health, parent->heading, parent->bearing, parent->m);
            }
            else
            {
                setEverything( _type, 40, 40, 0, 3, 0, 97, 0, 0);
            }
            break;
    }
    maxHealth = health;
}

void CObject::setEverything(
        ObjectType _type,
        double _px,
        double _py,
        double _speed,
        double _radius,
        double _w,
        int    _health,
        double _heading,
        double _bearing,
        double _mass)
{
    type_   = _type;
    p.x     = _px;
    p.y     = _py;
    radius  = _radius;
    w       = _w;
    health  = _health;
    v.x     = cos(_heading) * _speed;
    v.y     = sin(_heading) * _speed;
    bearing = _bearing;
    m       = _mass;
}


void CObject::addForce(double magnitude, double angle)
{
    f.x = cos(angle) * magnitude;
    f.y = sin(angle) * magnitude;
}

// apply net force and convert to delta velocity
void CObject::applyForces()
{
    v += f/m;   // add velocity from impulse
    v *= 0.995; // apply some friction
    f(0,0);     // reset net force
    p += v;     // update position

    w       *= 0.995;
    bearing += w;
}

void CObject::bumpedInto(CObject *o, vector2f v_delta)
{
    // use this in derived classes to get radius-based damage
    double damage = v_delta.length() * sqrt(o->radius);

    // give bonus damage to SHOT vs ROCK
    // otherwise SHOT too ineffective vs ROCK
    if (type == ROCK && o->type == SHOT)
    {
        damage *= 100;
    }

    health -= damage;

    std::stringstream ss;
    ss << int(damage);

    if (SHOW_DAMAGE)
    {
        world.addObject( new Text(p.x, p.y, ss.str(), this));
    }
}

void CObject::ShowStats(BITMAP *pDest, CObject *obj)
{
    const int white = makecol(255, 255, 255);
    const int red   = makecol(255,   0,   0);
    const int green = makecol(  0, 255,   0);
    const int blue  = makecol(  0,   0, 255);

    int y = 1;

    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     x:% 010.5f", p.x);
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     y:% 010.5f", p.y);
    y++;
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     b:% 010.5f", azimuth);
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     w:% 010.5f", w);
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     H:% 010.5f", heading);
    y++;
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, " speed:% 010.5f", speed);

    if (obj != NULL)
    {
        textprintf_ex(pDest, font, 0, 10*y++, red, -1, "   TtC:% 010.5f", CObject::circleCircleCollision(this, obj));
    }
}
