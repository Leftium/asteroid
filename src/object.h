// object.h
// CObject header

// this is the base class for all objects in asteroids

#ifndef OBJECT_H
#define OBJECT_H

#include <allegro.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#define RAD_PER_FIX (128.0 / M_PI)
#define FIX_PER_RAD (M_PI / 128)

// convert between Allegro binary angle (0 == N, 0 to 256)
// and radians (0 == E, O to 2PI)
double FIX2RAD(int f) { return (((256+64) - (f)) % 256 * FIX_PER_RAD); }
int RAD2FIX(double r) { return (int(256+64 - (r * RAD_PER_FIX))) % 256; }

inline double Distance(double x1, double y1, double x2, double y2)
{
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

// calculate Allegro binary angle of vector from (x1,y1) to (x2,y2)
inline int Bearing(double x1, double y1, double x2, double y2)
{
    // x and y args to atan2 swapped to rotate resulting angle 90 degrees
    // (thus angle in respect to +Y axis instead of +X axis)
    int angle = atan2(x1 - x2,
                      y2 - y1 + DBL_MIN)  // DBL_MIN added to avoid division by zero
                * RAD_PER_FIX;            // convert radians to Allegro binary angle

    // ensure result in interval [0,256)
	// subtract because positive Allegro angles go clockwise
    return (256 - angle) % 256;
}

double radbearing(double x1, double y1, double x2, double y2)
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
    double    dX;            // X loc of object
    double    dY;            // Y loc of object
    double vx, vy;    // velocity


    // TODO: replace with velocity vector
    double    dOldX;        // last X loc of object
    double    dOldY;        // last Y loc of object
    int        nHeading;    // Angle object is moving
    double    dVelocity;    // Speed object is moving

    // TODO: change to radians in double
    int        nBearing;    // Angle object is facing
    double bearing;

    int        nRadius;    // Radius of object


    // TODO: refactor into subclasses?
    int     nHealth;    // Amount of hits left
    int        nData;        // all-purpose variable

public:
    // TODO: move clipping logic outside of object class
    static const int MAX_X = 320;
    static const int MAX_Y = 240;

    inline CObject(double dInitX, double dInitY, double dInitVelocity,
            int nInitRadius, int nInitHealth, int nData = 0,
            int nInitHeading = 0, int nInitBearing = 0);

    // TODO: split into impulse and one "apply forces" function
    inline void Move(double dPower = 0, int nAngle = 0);
    inline void Rotate(int nAngle);

    // TODO: Move rendering outside object class?
    inline void Draw(BITMAP *pSprite, BITMAP *pTarget);

    // TODO: Move debugging outside of object class?
    inline void ShowStats(BITMAP *pDest);

    // TODO: change to properties
    double     GetX() { return dX; };
    double     GetY() { return dY; };

    __declspec ( property ( get=getspeed ) ) double speed;
    double getspeed() { return Distance(0, 0, vx, vy); }

    __declspec ( property ( get=getheading ) ) double heading;
    double getheading() { return radbearing(0, 0, vx, vy); }

    int     GetRadius() { return nRadius; };

    int     GetBearing() { return nBearing; };
    int     GetHeading() { return nHeading; };

    int        GetHealth() { return nHealth; };
    void    SetHealth(int nNewHealth ) { nHealth = nNewHealth; };

    int        GetData() { return nData; };
    void    SetData(int nNewData) { nData= nNewData; };

    double    GetVelocity() { return dVelocity; };
    void    SetVelocity(double dNewVelocity) { dVelocity = dNewVelocity; };
};

CObject::CObject(double dInitX, double dInitY, double _speed,
                 int nInitRadius, int nInitHealth, int nInitData,
                 int _heading, int nInitBearing)
{
    dX = dInitX;
    dY = dInitY;
    dVelocity = _speed; ///
    nRadius = nInitRadius;
    nHealth = nInitHealth;
    nData = nInitData;
    nHeading = _heading;
    nBearing = nInitBearing; ///

    bearing = nInitBearing * FIX_PER_RAD;

    vx = cos( FIX2RAD( _heading ) ) * _speed;
    vy = sin( FIX2RAD( _heading ) ) * _speed;

    dOldX = dX;
    dOldY = dY;
}

inline void CObject::Move(double dPower, int nAngle)
{
    if (dPower != 0)    // moved by outside force (thrusters or collision)
    {
        // move
        dX += (fixtof( fsin(itofix(nAngle))) * dPower);
        dY += (fixtof( fcos(itofix(nAngle))) * dPower);

        nHeading = Bearing(dOldX, dOldY, dX, dY);
    }

    else    // not from outside force (moved by momentum)
    {
        dOldX = dX; dOldY = dY;

        // move
        dX += (fixtof( fsin(itofix(nHeading))) * dVelocity);
        dY += (fixtof( fcos(itofix(nHeading))) * dVelocity);

        nHeading = Bearing(dOldX, dOldY, dX, dY);
    }

    SetVelocity(Distance(dOldX, dOldY, dX, dY));
    vx = GetVelocity() * cos( FIX2RAD( GetBearing() ) ); 
    vy = GetVelocity() * sin( FIX2RAD( GetBearing() ) );

    // wrap around
    if (dX > MAX_X)
    {
        dX = dX - MAX_X;
        dOldX = dOldX - MAX_X;
    }
    if (dX < 0)
    {
        dX = dX + MAX_X;
        dOldX = dOldX + MAX_X;
    }

    if (dY > MAX_Y)
    {
        dY = dY - MAX_Y;
        dOldY = dOldY - MAX_Y;
    }
    if (dY < 0)
    {
        dY = dY + MAX_Y;
        dOldY = dOldY + MAX_Y;
    }
}

inline void CObject::Rotate(int nAngle)
{
    nBearing += nAngle;
    if (nBearing > 255) nBearing = (nBearing - 255);
    if (nBearing < 0 ) nBearing = 255 + nBearing;
}

inline void CObject::Draw(BITMAP *pSprite, BITMAP *pDest)
{
    rotate_sprite(pDest, pSprite, (int)dX-(pSprite->w>>1),
                  MAX_Y-((int)dY+(pSprite->h>>1)), itofix(nBearing));
}

inline void CObject::ShowStats(BITMAP *pDest)
{
    int y = 1;
    char szBuf[80];


        sprintf(szBuf, "     x:% 010.5f", dX);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        sprintf(szBuf, "     y:% 010.5f", dY);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        y++;

        sprintf(szBuf, "     h:% 04d", nHeading);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        double rad = FIX2RAD(nHeading);
        sprintf(szBuf, "f2r(h):% 010.5f", rad);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        sprintf(szBuf, "r2f(h):% 04d", RAD2FIX(rad));
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        sprintf(szBuf, "     b:% 04d", nBearing);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        sprintf(szBuf, "     H:% 010.5f", this->heading);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        y++;

        sprintf(szBuf, "     v:% 010.5f", dVelocity);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        sprintf(szBuf, " speed:% 010.5f", this->speed);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

}

#endif
