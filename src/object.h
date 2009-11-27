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
    double px, py;    // position
    double vx, vy;    // velocity
    double fx, fy;    // net force on object
    double m;         // mass of object


    // TODO: replace with velocity vector
    double    dOldX;        // last X loc of object
    double    dOldY;        // last Y loc of object

    double azimuth;

    int        radius;    // Radius of object


    // TODO: refactor into subclasses?
    int     nHealth;    // Amount of hits left
    int        nData;        // all-purpose variable

public:
    static bool isCollision(CObject *p1, CObject *p2);

    // TODO: move clipping logic outside of object class
    static const int MAX_X = 320;
    static const int MAX_Y = 240;

    inline CObject(double dInitX, double dInitY, double dInitVelocity,
            int nInitRadius, int nInitHealth, int nData = 0,
            double nInitHeading = 0, double nInitBearing = 0);

    // TODO: split into impulse and one "apply forces" function
    inline void Move(double dPower = 0, double nAngle = 0);
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
    return (squareDistance(p1->px, p1->py, p2->px, p2->py)) <
           pow(double(p1->radius + p2->radius), 2);
}

CObject::CObject(double dInitX,
                 double dInitY,
                 double _speed,
                 int nInitRadius,
                 int nInitHealth,
                 int nInitData,
                 double _heading,
                 double _bearing)
{
    px = dInitX;
    py = dInitY;
    radius = nInitRadius;
    nHealth = nInitHealth;
    nData = nInitData;

    this->bearing = _bearing;

    vx = cos(_heading) * _speed;
    vy = sin(_heading) * _speed;

    dOldX = px;
    dOldY = py;
}

inline void CObject::Move(double dPower, double angle)
{
    if (dPower != 0)    // moved by outside force (thrusters or collision)
    {
        // move
        px += cos(angle) * dPower;
        py += sin(angle) * dPower;
    }
    else    // not from outside force (moved by momentum)
    {
        // move
        px += vx;
        py += vy;

        double newVelocity = sqrt(squareDistance(dOldX, dOldY, px, py)) * 0.995;
        vx = cos( relativeAngle( dOldX, dOldY, px, py ) ) * newVelocity;
        vy = sin( relativeAngle( dOldX, dOldY, px, py ) ) * newVelocity;

        dOldX = px; dOldY = py;
    }

    // wrap around
    if (px > MAX_X)
    {
        px = px - MAX_X;
        dOldX = dOldX - MAX_X;
    }
    if (px < 0)
    {
        px = px + MAX_X;
        dOldX = dOldX + MAX_X;
    }

    if (py > MAX_Y)
    {
        py = py - MAX_Y;
        dOldY = dOldY - MAX_Y;
    }
    if (py < 0)
    {
        py = py + MAX_Y;
        dOldY = dOldY + MAX_Y;
    }
}

inline void CObject::Rotate(double angle)
{
    this->bearing += angle;
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

        sprintf(szBuf, "     H:% 010.5f", this->heading);
        textout(pDest, font, szBuf, 0, 10*y++, 255);

        y++;

        sprintf(szBuf, " speed:% 010.5f", this->speed);
        textout(pDest, font, szBuf, 0, 10*y++, 255);
}

#endif
