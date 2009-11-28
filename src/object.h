/// object.h
// CObject header

// this is the base class for all objects in asteroids

#ifndef OBJECT_H
#define OBJECT_H

#include <allegro.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#define RAD_PER_FIX (128.0 / M_PI)
#define FIX_PER_RAD (M_PI / 128.0)

#define Rnd(x)      ((rand() % (x)))

enum objectType { GENERIC, SHIP, SHOT, ROCK, EXPLOSION };

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
    objectType type;

    double px, py;    // position
    double vx, vy;    // velocity
    double fx, fy;    // net force on object
    double m;         // mass of object
    double azimuth;   // direction object is facing
    double radius;    // Radius of object

    // TODO: refactor into subclasses?
    int    nHealth;   // Amount of hits left
    int    nData;     // all-purpose variable

    void wrapPosition();
    void setEverything(objectType _type, double _px, double _py, double _speed, double _radius, int _health, int _data, double _heading, double _bearing, double mass=100);

public:
    // TODO: move clipping logic outside of object class
    static const int MAX_X = 320;
    static const int MAX_Y = 240;

    static bool isCollision(CObject *p1, CObject *p2);

    void addForce(double magnitude, double angle);
    void applyForces();

    // constructors
    CObject::CObject(objectType _type, CObject *parent);

    inline void Rotate(double angle);

    // TODO: Move rendering outside object class?
    inline void Draw(BITMAP *pSprite, BITMAP *pTarget);

    // TODO: Move debugging outside of object class?
    inline void ShowStats(BITMAP *pDest);

    // TODO: change to properties... or get rid of them
    double     GetX() { return px; };
    double     GetY() { return py; };

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

    int        GetHealth() { return nHealth; };
    void    SetHealth(int nNewHealth ) { nHealth = nNewHealth; };

    int        GetData() { return nData; };
    void    SetData(int nNewData) { nData= nNewData; };
};

// ISCOLLISION //////////////////////////////////////////////////////////////////
bool CObject::isCollision(CObject *p1, CObject *p2)
{
    return ( ( squareDistance(p1->px, p1->py, p2->px, p2->py ) ) <
             ( pow(p1->radius + p2->radius, 2) ) );
}

CObject::CObject(objectType _type, CObject *parent)
{
    int randomHeading = (rand()/(double)RAND_MAX)*2*M_PI;
    int randomBearing = (rand()/(double)RAND_MAX)*2*M_PI;

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
                px = 80;
            }
            else
            {
                // this is ship2
                px = 240;
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
                    parent->bearing); // bearing
            addForce(300, parent->bearing);
            break;

        case ROCK:
            setEverything(
                    ROCK,
                    Rnd(MAX_X),
                    Rnd(MAX_Y),
                    0.1,            // speed
                    5,              // radius
                    100,            // health
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
        objectType _type,
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
    nHealth = _health;
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

inline void CObject::Draw(BITMAP *pSprite, BITMAP *pDest)
{
    rotate_sprite(pDest, pSprite, (int)px-(pSprite->w>>1),
                  MAX_Y-((int)py+(pSprite->h>>1)), RAD2FIX( azimuth ));

    // rect(pDest, px-1, MAX_Y-py-1, px+1, MAX_Y-py+1, makecol(255, 255, 255));
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
