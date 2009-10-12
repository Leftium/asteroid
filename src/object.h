// object.h
// CObject header

// this is the base class for all objects in asteroids

#ifndef OBJECT_H
#define OBJECT_H

#include <allegro.h>
#include <stdio.h>
#include "distance.h"
#include "bearing.h"

class CObject
{
protected:
    double    dX;            // X loc of object
    double    dY;            // Y loc of object

    double    dOldX;        // last X loc of object
    double    dOldY;        // last Y loc of object

    int        nHeading;    // Angle object is moving
    int        nBearing;    // Angle object is facing
    int        nRadius;    // Radius of object
    double    dVelocity;    // Speed object is moving

    int     nHealth;    // Amount of hits left
    int        nData;        // all-purpose variable

public:
    static const int MAX_X = 320;
    static const int MAX_Y = 200;

    inline CObject(double dInitX, double dInitY, double dInitVelocity,
            int nInitRadius, int nInitHealth, int nData = 0,
            int nInitHeading = 0, int nInitBearing = 0);

    inline void Move(double dPower = 0, int nAngle = 0);
    inline void Rotate(int nAngle);
    inline void Draw(BITMAP *pSprite, BITMAP *pTarget);

    inline void ShowStats(BITMAP *pDest);

    double     GetX() { return dX; };
    double     GetY() { return dY; };

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

CObject::CObject(double dInitX, double dInitY, double dInitVelocity,
                 int nInitRadius, int nInitHealth, int nInitData,
                 int nInitHeading, int nInitBearing)
{
    dX = dInitX;
    dY = dInitY;
    dVelocity = dInitVelocity;
    nRadius = nInitRadius;
    nHealth = nInitHealth;
    nData = nInitData;
    nHeading = nInitHeading;
    nBearing = nInitBearing;

    dOldX = dX;
    dOldY = dY;
}

inline void CObject::Move(double dPower, int nAngle)
{
    if (dPower != 0)    // moved by outside force (thrusters or collision)
    {
        // move
        dX += (fixtof( fsin(itofix(nAngle))) * dPower);
        dY += (fixtof(-fcos(itofix(nAngle))) * dPower);

        nHeading = Bearing(dOldX, dOldY, dX, dY);
    }

    else    // not from outside force (moved by momentum)
    {
        dOldX = dX; dOldY = dY;

        // move
        dX += (fixtof( fsin(itofix(nHeading))) * dVelocity);
        dY += (fixtof(-fcos(itofix(nHeading))) * dVelocity);

        nHeading = Bearing(dOldX, dOldY, dX, dY);
    }

    SetVelocity(Distance(dOldX, dOldY, dX, dY));

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
                  (int)dY-(pSprite->h>>1), itofix(nBearing));
}

inline void CObject::ShowStats(BITMAP *pDest)
{
    char szBuf[80];

        sprintf(szBuf, "  x:% 010.5f", dX);
        textout(pDest, font, szBuf, 0, 0, 255);

        sprintf(szBuf, "  y:% 010.5f", dY);
        textout(pDest, font, szBuf, 0, 10, 255);

        sprintf(szBuf, "  h:% 04d", nHeading);
        textout(pDest, font, szBuf, 0, 60, 255);

        sprintf(szBuf, "  b:% 04d", nBearing);
        textout(pDest, font, szBuf, 0, 70, 255);

        sprintf(szBuf, "  v:% 010.5f", dVelocity);
        textout(pDest, font, szBuf, 0, 90, 255);
}

#endif
