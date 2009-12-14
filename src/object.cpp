#include "object.h"
#include "sound.h"
#include "explosion.h"

CollisionFlags CObject::collidesWith(CObject *o)
{
    if (type == ROCK && o->type == ROCK)
    {
        return ALL;
    }
    else if (type == EXPLOSION || o->type == EXPLOSION || type == GENERIC || o->type == GENERIC || o->type == STARFIELD)
    {
        return NONE;
    }
    else if (((type == SHOT) || (type == SHIP)) && (team != o->team))
    {
        return ALL;
    }
    else
    {
        return NONE;
    }
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
    switch(type)
    {
        case SHOT:
            health--;
            if (health > 0)
            {
                // action: projectile: move
                applyForces();
                Rotate(6 * FIX_PER_RAD);
            }
            else
            {
                return true;
            }
            return false;
            break;

        case ROCK:
            if (health > 0)
            {
                Rotate(1 * FIX_PER_RAD);
                // action: rock: move
                applyForces();
                return false;
            }
            else
            {
                // action: rock: explosion animation
                objects.push_back(objectPtr(new Explosion(this)));

                return true;
            }
            break;

        default:
            return false;
            break;
    }
}

// ISCOLLISION //////////////////////////////////////////////////////////////////
bool CObject::isCollision(CObject *p1, CObject *p2)
{
    return ( ( squareDistance(p1->px, p1->py, p2->px, p2->py ) ) <
             ( pow(p1->radius + p2->radius, 2) ) );
}


bool CObject::handleCollision(CObject *p, CObject *q)
{
    // check if collision matters
    if (p->collidesWith(q) == NONE &&
        q->collidesWith(p) == NONE)
    {
        return false;
    }
    else
    {
        // check if on collision course
        double h, j, k, l, d, a, b, c, discriminantSquared;

        h = q->px - p->px;  // relative x location
        j = q->vx - p->vx;  // relative x velocity
        k = q->py - p->py;  // relative y location
        l = q->vy - p->vy;  // relative y velocity
        d = q->radius + p->radius;

        a = j*j + l*l + DBL_MIN;
        b = 2 * (h*j + k*l);
        c = h*h + k*k - d*d;

        discriminantSquared = b*b - 4*a*c;

        if (discriminantSquared < 0)
        {
            return false;
        }
        else
        {
            // determine time of collision
            double discriminant = sqrt(discriminantSquared);
            double t1 = (-b + discriminant) / (2*a);
            double t2 = (-b - discriminant) / (2*a);
            double t  = -999;

            if (isCollision(p, q))
            {
                // collision in past: t = negative time
                if ((t1 < 0))
                {
                    t = t1;
                }
                else if ((t2 < 0))
                {
                    t = t2;
                }
            }
            else
            {
                if ((t1 >= 0) && (t1 < t2))
                {
                    t = t1;
                }
                else if ((t2 >= 0) && (t2 < t1))
                {
                    t = t2;
                }
            }

            if (t >= 0 && t < 1)
            {
                bool doPhysicsP = (p->collidesWith(q) & PHYSICS_SELF) ||
                                  (q->collidesWith(p) & PHYSICS_TARGET);
                bool doPhysicsQ = (q->collidesWith(p) & PHYSICS_SELF) ||
                                  (p->collidesWith(q) & PHYSICS_TARGET);

                if (doPhysicsP || doPhysicsQ)
                {
                    // calculate physics for elastic collision
                    double vxpf = p->vx;
                    double vypf = p->vy;

                    double vxqf = q->vx;
                    double vyqf = q->vy;

                    elasticCollide(vxpf, p->m, vxqf, q->m);
                    elasticCollide(vypf, p->m, vyqf, q->m);

                    if (doPhysicsP)
                    {
                        // place at point of collision
                        p->px += t * p->vx;
                        p->py += t * p->vy;

                        // adjust velocity
                        p->vx = vxpf;
                        p->vy = vypf;

                        // move ships ships after collision
                        p->px += -t * p->vx;
                        p->py += -t * p->vy;
                        p->wrapPosition();
                    }

                    if (doPhysicsQ)
                    {
                        // place at point of collision
                        q->px += t * q->vx;
                        q->py += t * q->vy;

                        // adjust velocity
                        q->vx = vxqf;
                        q->vy = vyqf;

                        // move ships ships after collision
                        q->px += -t * q->vx;
                        q->py += -t * q->vy;
                        q->wrapPosition();
                    }
                }

                // do final collision logic
                bool doLogicP = (p->collidesWith(q) & LOGIC_SELF) ||
                                (q->collidesWith(p) & LOGIC_TARGET);
                bool doLogicQ = (q->collidesWith(p) & LOGIC_SELF) ||
                                (p->collidesWith(q) & LOGIC_TARGET);

                if (doLogicP)
                {
                    p->bumpedInto(q);
                }

                if (doLogicQ)
                {
                    q->bumpedInto(p);
                }
                objects.push_back(objectPtr(new Sound(BOOM, (p->px + q->px) / 2, (p->py + q->py) / 2 )));
            }
        }
    }
    return true;
}

CObject::CObject(ObjectType _type, CObject *parent)
{
    static int currentId    = 0;
    static int nextGenericX = 4;

    double randomHeading = (rand()/(double)RAND_MAX)*2*M_PI;
    double randomBearing = (rand()/(double)RAND_MAX)*2*M_PI;

    id = currentId++;
    fx = 0;
    fy = 0;

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
        case SHOT:
            setEverything(
                    SHOT,
                    parent->px,
                    parent->py,
                    parent->speed,    // speed
                    4,                // radius
                    25,               // health
                    parent->heading,  // heading
                    parent->bearing,  // bearing
                    100);             // mass
            addForce(3*m, parent->bearing);
            break;

        case ROCK:
            setEverything(
                    ROCK,
                    Rnd(MAX_X),
                    Rnd(MAX_Y),
                    0.1,            // speed
                    5,              // radius
                    10,             // health
                    randomHeading,  // heading
                    randomBearing,  // bearing
                    200);           // mass
            break;

        case GENERIC:
            setEverything(
                    GENERIC,
                    nextGenericX,
                    12,
                    0,               // speed
                    3,               // radius
                    99,              // health
                    1.1,             // heading
                    1.1);            // bearing
            nextGenericX += 12;
            break;
        default:
            if (parent != NULL)
            {
                setEverything( _type, parent->px, parent->py, parent->speed, parent->radius, parent->health, parent->heading, parent->bearing);
            }
            else
            {
                setEverything( _type, 40, 40, 0, 3, 97, 0, 0);
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
        int    _health,
        double _heading,
        double _bearing,
        double _mass)
{
    type_   = _type;
    px      = _px;
    py      = _py;
    radius  = _radius;
    health = _health;
    vx      = cos(_heading) * _speed;
    vy      = sin(_heading) * _speed;
    bearing = _bearing;
    m       = _mass;
}


void CObject::addForce(double magnitude, double angle)
{
    fx = cos(angle) * magnitude;
    fy = sin(angle) * magnitude;
}

// apply net force and convert to delta velocity
void CObject::applyForces()
{
    // add velocity from impulse
    vx += fx / m;
    vy += fy / m;

    // apply some friction
    vx *= 0.995;
    vy *= 0.995;

    // reset net force
    fx = 0;
    fy = 0;

    // update position
    px += vx;
    py += vy;

    wrapPosition();
}

void CObject::wrapPosition()
{
    if (px < 0) px += MAX_X;
    if (py < 0) py += MAX_Y;

    if (px > MAX_X) px -= MAX_X;
    if (py > MAX_Y) py -= MAX_Y;
}

inline void CObject::Rotate(double angle)
{
    bearing += angle;
}

// TODO: fix elastic collision so collision point is accounted for in final velecities
// Compute final velocities after elastic collision
// Results returned via reference variables holding initial velocities
// Based on: http://farside.ph.utexas.edu/teaching/301/lectures/node76.html
void CObject::elasticCollide(double &v1, double m1, double &v2, double m2)
{
   double v1f;

   v1f = ((m1 - m2) / (m1 + m2) * v1) + ((2 * m2) / (m1 + m2)  * v2);
   v2  = ((2 * m1) / (m1 + m2)  * v1) + ((m1 - m2) / (m1 + m2) * v2);

   v1 = v1f;
}

void CObject::bumpedInto(CObject *o)
{
    switch(type)
    {
        case SHOT:
            health = 0;
            // render flash
            circlefill(buf, px, MAX_Y - py, 5, makecol(255, 255, 255));
            break;

        case ROCK:
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
            break;

        case GENERIC:
        default:
            break;
    }
    return;
}

void CObject::ShowStats(BITMAP *pDest)
{
    const int white = makecol(255, 255, 255);
    const int red   = makecol(255,   0,   0);
    const int green = makecol(  0, 255,   0);
    const int blue  = makecol(  0,   0, 255);

    int y = 1;

    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     x:% 010.5f", px);
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     y:% 010.5f", py);
    y++;
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     b:% 010.5f", azimuth);
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, "     H:% 010.5f", heading);
    y++;
    textprintf_ex(pDest, font, 0, 10*y++, blue, -1, " speed:% 010.5f", speed);
}
