#include "starfield.h"

Starfield::Starfield(): CObject(STARFIELD)
{
    star_count = 0;
    star_count_count = 0;

    for (int i=0; i<8; i++)
    {
        StarColor[i] = makecol(255*i/7, 255*i/7, 255*i/7);
    }

   for (int c = 0; c < MAX_STARS; c++) {
      star[c].z = 0;
      star[c].ox = star[c].oy = -1;
   }
}

CollisionFlags Starfield::collidesWith(CObject *o)
{
    return NONE;
}

bool Starfield::update()
{
    int c;
    fixed x, y;
    int ix, iy;
    for (c = 0; c < star_count; c++) {
        if (star[c].z <= itofix(1)) {
            x = itofix(AL_RAND() & 0xff);
            y = itofix(((AL_RAND() & 3) + 1) * SCREEN_W);
            star[c].x = fixmul(fixcos(x), y);
            star[c].y = fixmul(fixsin(x), y);
            star[c].z = itofix((AL_RAND() & 0x1f) + 0x20);
        }

        x = fixdiv(star[c].x, star[c].z);
        y = fixdiv(star[c].y, star[c].z);
        ix = (int)(x>>16) + buf->w/2;
        iy = (int)(y>>16) + buf->h/2;
        if ((ix >= 0) && (ix < buf->w) && (iy >= 0)
                && (iy <= buf->h)) {
            // point is inside buffer
            star[c].ox = ix;
            star[c].oy = iy;
            star[c].z -= 4096;
        }
        else {
            // point is outside buffer
            star[c].ox = -1;
            star[c].oy = -1;
            star[c].z = 0;
        }
    }

   /* wake up new star */
    if (star_count < MAX_STARS) {
        if (star_count_count++ >= 32) {
            star_count_count = 0;
            star_count++;
        }
    }
    return false;
}

void Starfield::draw_starfield_3d(BITMAP *bmp)
{
    int c, c2;
    for (c = 0; c < star_count; c++) {
        c2 = 7 - (int)(star[c].z >> 18);
        putpixel(bmp, star[c].ox, star[c].oy, StarColor[MID(0, c2, 7)]);
    }
}
