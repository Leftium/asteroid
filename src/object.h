/// object.h
// CObject header

// this is the base class for all objects in asteroids

#ifndef OBJECT_H
#define OBJECT_H

#include <allegro.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <list>

#define RAD_PER_FIX (128.0 / M_PI)
#define FIX_PER_RAD (M_PI / 128.0)

#define Rnd(x)      ((rand() % (x)))

// Possible collision detection levels
enum CollisionFlags
{
    NONE           = 0,
    PHYSICS_SELF   = 1,
    PHYSICS_TARGET = 2,
    LOGIC_SELF     = 4,
    LOGIC_TARGET   = 8,
    ALL            = PHYSICS_SELF | PHYSICS_TARGET | LOGIC_SELF | LOGIC_TARGET,
    ONLY_SELF      = PHYSICS_SELF                  | LOGIC_SELF,
    ONLY_TARGET    =                PHYSICS_TARGET |              LOGIC_TARGET
};

enum ObjectType { GENERIC, SHIP, SHOT, ROCK, EXPLOSION };

// convert radians (0 == E, O to 2PI)
// to Allegro fixed point binary angle (0 == N, 0 to 256)
fixed RAD2FIX(double r) { return itofix((int(256+64 - (r * RAD_PER_FIX))) % 256); }

inline double squareDistance(double x1, double y1, double x2, double y2)
{
    return pow(x1 - x2, 2) + pow(y1 - y2, 2);
}

double relativeAngle(double x1, double y1, double x2, double y2)
{
    double angle = atan2(y2 - y1, x2 - x1 + DBL_MIN);
    if (angle >= 0)
    {
        return angle;
    }
    else
    {
        return angle + (2 * M_PI);
    }
}


class CObject
{
protected:
    ObjectType type;

    double px, py;    // position
    double vx, vy;    // velocity
    double fx, fy;    // net force on object
    double m;         // mass of object
    double azimuth;   // direction object is facing
    double radius;    // Radius of object

    // TODO: refactor into subclasses?
    int    health;   // Amount of hits left
    int    nData;     // all-purpose variable
    int    team;

    void wrapPosition();
    void setEverything(ObjectType _type, double _px, double _py, double _speed, double _radius, int _health, int _data, double _heading, double _bearing, double mass=100);
    static void elasticCollide(double &v1, double m1, double &v2, double m2);

public:
    // TODO: move clipping logic outside of object class
    static const int MAX_X = 320;
    static const int MAX_Y = 240;

    static bool isCollision(CObject *p1, CObject *p2);
    static bool CObject::handleCollision(CObject *p, CObject *q);

    CollisionFlags collidesWith(CObject *o);

    bool update();
    void addForce(double magnitude, double angle);
    void applyForces();
    void bumpedInto(CObject *o);

    // constructors
    CObject::CObject(ObjectType _type, CObject *parent);

    inline void Rotate(double angle);

    // TODO: Move debugging outside of object class?
    inline void ShowStats(BITMAP *pDest);

    // TODO: change to properties... or get rid of them
    double     GetX() { return px; };

    __declspec ( property ( get=getspeed ) ) double speed;
    double getspeed() { return sqrt(squareDistance(0, 0, vx, vy)); }

    __declspec ( property ( get=getheading ) ) double heading;
    double getheading() { return relativeAngle(0, 0, vx, vy); }

    __declspec ( property ( get=getbearing, put=setbearing ) ) double bearing;
    double getbearing() { return azimuth; }
    void setbearing(double b)
    {
        azimuth = fmod(b, 2 * M_PI);
        if (azimuth < 0)
        {
            azimuth += 2 * M_PI;
        }
    }

    int        GetHealth() { return health; };
    int        GetData() { return nData; };

    friend void render(CObject* o);
};

extern std::list<CObject*> objects;
extern BITMAP *buf;


CollisionFlags CObject::collidesWith(CObject *o)
{
    if (type == ROCK && o->type == ROCK)
    {
        return ALL;
    }
    else if (type == EXPLOSION || o->type == EXPLOSION)
    {
        return NONE;
    }
    else if ((type == SHIP && nData == 0) || (o->type == SHIP && o->nData == 0))
    {
        return NONE;
    }
    else if (team != o->team)
    {
        return ALL;
    }
    else
    {
        return NONE;
    }
}

bool CObject::update()
{
    switch(type)
    {
        case SHIP:
            if (health > 0)
            {
                // TODO: Move limits to Ship object subclass
                /// if (Ship2->speed >  2) Ship2->SetVelocity( 2);
                /// if (Ship2->speed < -2) Ship2->SetVelocity(-2);
                applyForces();
                /// Ship2->SetVelocity(Ship2->speed * .99);
            }
            else if (nData)
            {
                objects.push_front(new CObject(EXPLOSION, this));
                nData = 0;
            }
            return false;
            break;

        case SHOT:
            nData--;
            if (nData > 0)
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

        case EXPLOSION:
            nData--;
            if (nData > 0)
            {
                applyForces();
                Rotate(10 * FIX_PER_RAD);
                return false;
            }
            else
            {
                return true;
            }
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
                objects.push_front(new CObject(EXPLOSION, this));

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
            }
        }
    }
    return true;
}

CObject::CObject(ObjectType _type, CObject *parent)
{
    double randomHeading = (rand()/(double)RAND_MAX)*2*M_PI;
    double randomBearing = (rand()/(double)RAND_MAX)*2*M_PI;

    fx = 0;
    fy = 0;

    if (parent != NULL)
    {
        team = parent->team;
    }
    else
    {
        team = 0;
    }

    switch (_type)
    {
        case SHIP:
            setEverything(
                    SHIP,
                    0,
                    100,
                    0,         // speed
                    10,        // radius
                    100,       // health
                    1,         // data
                    0,         // heading
                    (M_PI_2)); // bearing

            if (parent == NULL)
            {
                // this is ship1: no ships created, yet.
                health = 50;
                px = 80;
                bearing = 0;
                team = 1;
            }
            else
            {
                // this is ship2
                px = 240;
                bearing = M_PI;
                team = 2;
            }
            break;

        case SHOT:
            setEverything(
                    SHOT,
                    parent->px,
                    parent->py,
                    parent->speed,    // speed
                    4,                // radius
                    0,                // health
                    25,               // data
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
                    1,              // data
                    randomHeading,  // heading
                    randomBearing,  // bearing
                    200);           // mass
            break;

        case EXPLOSION:
            setEverything(
                    EXPLOSION,
                    parent->px,
                    parent->py,
                    parent->speed,   // speed
                    0,               // radius
                    0,               // health
                    30,              // data
                    parent->heading, // heading
                    randomBearing);  // bearing
            break;

        case GENERIC:
        default:
            break;
    }
}

void CObject::setEverything(
        ObjectType _type,
        double _px,
        double _py,
        double _speed,
        double _radius,
        int    _health,
        int    _data,
        double _heading,
        double _bearing,
        double _mass)
{
    type    = _type;
    px      = _px;
    py      = _py;
    radius  = _radius;
    health = _health;
    nData   = _data;
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
        case SHIP:
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
            break;
        case SHOT:
            nData = 0;
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
        case EXPLOSION:
            // don't do anything
            break;
        case GENERIC:
        default:
            break;
    }
    return;
}

inline void CObject::ShowStats(BITMAP *pDest)
{
    int y = 1;
    char szBuf[80];


        sprintf(szBuf, "     x:% 010.5f", px);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        sprintf(szBuf, "     y:% 010.5f", py);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        y++;

        sprintf(szBuf, "     b:% 010.5f", azimuth);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        sprintf(szBuf, "     H:% 010.5f", heading);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        y++;

        sprintf(szBuf, " speed:% 010.5f", speed);
        textout(pDest, font, szBuf, 0, 10*y++, 255);
}

#endif
