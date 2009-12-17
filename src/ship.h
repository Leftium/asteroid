#pragma once

#include "object.h"

class Ship;

typedef std::tr1::shared_ptr<Ship> ShipPtr;
typedef std::tr1::weak_ptr<Ship>   ShipPtrWeak;

class Ship: public CObject
{
protected:
    int energy_;
    int reloadTime_;

    bool isEngineOn_;

public:
    // constructors
    Ship::Ship(double x, double y, int _team, double _bearing, int _health);

    virtual CollisionFlags collidesWith(CObject *o);
    virtual bool update();
    virtual void bumpedInto(CObject *o, vector2f v_delta);

    __declspec ( property ( get=getenergy ) ) int energy;
    int getenergy() { return energy_; }

    void fire();
    void thrust(int power);

    friend void render(objectPtr o);
};
