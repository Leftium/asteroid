/// object.h
// CObject header

// this is the base class for all objects in asteroids
#pragma once

#include <stdio.h>
#include <list>
#include "util.h"

class CObject;

typedef std::tr1::shared_ptr<CObject>      objectPtr;
typedef std::tr1::weak_ptr<CObject>        objectPtrWeak;
typedef std::list<objectPtr>::iterator     objectIter;
typedef std::list<objectPtrWeak>::iterator objectIterWeak;

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

enum ObjectType { GENERIC, SHIP, SHOT, ROCK, EXPLOSION, SOUND, STARFIELD, FLASH};


class CObject
{
protected:
    ObjectType type_;

    int    id;        // uniquely identifies object
    double px, py;    // position
    double vx, vy;    // velocity
    double fx, fy;    // net force on object
    double m;         // mass of object
    double azimuth;   // direction object is facing
    double radius;    // Radius of object

    // TODO: refactor into subclasses?
    int    health;    // Amount of hits left
    int    maxHealth; // Starting value of health
    int    team_;

    std::list<objectPtrWeak> dependedObjects;

    void wrapPosition();
    void setEverything(ObjectType _type, double _px, double _py, double _speed, double _radius, int _health, double _heading, double _bearing, double mass=100);
    static void elasticCollide(double &v1, double m1, double &v2, double m2);

public:
    // TODO: move clipping logic outside of object class
    static const int MAX_X = 320;
    static const int MAX_Y = 240;

    static bool isCollision(CObject *p1, CObject *p2);
    static bool CObject::handleCollision(CObject *p, CObject *q);

    virtual CollisionFlags collidesWith(CObject *o);

    bool checkDependencies();
    virtual bool update();
    void addForce(double magnitude, double angle);
    void applyForces();
    virtual void bumpedInto(CObject *o);

    // constructors
    CObject::CObject(ObjectType _type=GENERIC, CObject *parent=NULL);

    void addDependency(objectPtr obj);

    void Rotate(double angle);

    // TODO: Move debugging outside of object class?
    void ShowStats(BITMAP *pDest);

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

    __declspec ( property ( get=getteam ) ) int team;
    int getteam() { return team_; }

    __declspec ( property ( get=gettype ) ) ObjectType type;
    ObjectType gettype() { return type_; }

    int        GetHealth() { return health; };

    friend void render(objectPtr o);
};

extern std::list< objectPtr > objects;
extern BITMAP *buf;
