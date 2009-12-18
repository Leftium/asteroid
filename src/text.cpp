#include "text.h"

Text::Text(double x, double y, std::string m, CObject *parent): CObject(TEXT)
{
    message = m;
    p(x,y);

    health = maxHealth = 50;

    // suppress collision hull/health overlay
    radius = -1;
}

bool Text::update()
{
    if (health-- > 0)
    {
        p.y += 1;
        applyForces();
        return false;
    }
    else
    {
        return true;
    }
}
