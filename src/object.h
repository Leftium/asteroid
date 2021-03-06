/// object.h
// CObject header

// this is the base class for all objects in asteroids
#pragma once

#include <stdio.h>
#include <list>
#include "util.h"
#include "vector2.h"
#include "objecttypes.h"
#include "world.h"

// Possible collision detection levels
enum CollisionFlags
{
    NONE           = 0,
    PHYSICS_TAKE   = 1,
    PHYSICS_GIVE   = 2,
    LOGIC_GIVE     = 4,
    LOGIC_TAKE     = 8,
    ALL            = PHYSICS_TAKE | PHYSICS_GIVE | LOGIC_TAKE | LOGIC_GIVE,
    ONLY_SELF      = PHYSICS_TAKE                | LOGIC_TAKE,
    ONLY_TARGET    =                PHYSICS_GIVE |              LOGIC_GIVE
};

enum ObjectType { GENERIC, SHIP, SHOT, ROCK, EXPLOSION, SOUND, STARFIELD, FLASH, TEXT};


class CObject
{
protected:
    ObjectType type_;

    int      id;      // uniquely identifies object
    vector2f p;       // position
    vector2f v;       // velocity
    vector2f f;       // net force on object
    double   m;       // mass of object
    double   azimuth; // direction object is facing
    double   radius;  // Radius of object
    double   w;        // radial velocity

    // TODO: refactor into subclasses?
    int    health;    // Amount of hits left
    int    maxHealth; // Starting value of health
    int    team_;

    std::list<objectPtrWeak> dependedObjects;

    void setEverything(ObjectType _type, double _px, double _py, double _speed, double _radius, double _w, int _health, double _heading, double _bearing, double mass=100);

    static void CObject::resolveCollision(CObject *obj1, CObject *obj2, vector2f n);
    static void CObject::resolveCollisionRadialVelocity(CObject *p, vector2f qv_orig, vector2f n);

public:
    static double CObject::circleCircleCollision(CObject *cir1, CObject *cir2);
    static bool CObject::handleCollision(CObject *p, CObject *q);

    virtual CollisionFlags collidesWith(CObject *o);

    bool checkDependencies();
    virtual bool update();
    void addForce(double magnitude, double angle);
    void applyForces();
    virtual void bumpedInto(CObject *o, vector2f v_delta);

    // constructors
    CObject::CObject(ObjectType _type=GENERIC, CObject *parent=NULL);

    void addDependency(objectPtr obj);

    // TODO: Move debugging outside of object class?
    void CObject::ShowStats(BITMAP *pDest, CObject *obj);

    __declspec ( property ( get=getspeed ) ) double speed;
    double getspeed() { return v.length(); }

    __declspec ( property ( get=getheading ) ) double heading;
    double getheading() { return relativeAngle(0, 0, v.x, v.y); }

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

    friend class World;
    friend class Camera;
};

extern World world;