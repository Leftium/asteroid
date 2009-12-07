// asteroid.cpp /////////////////////////////////////////////////////////////

// #INCLUDES ////////////////////////////////////////////////////////////////
#include <allegro.h>
#include <stdlib.h>    // for rand()
#include "asteroid.h"
#include "object.h"

#include <list>

// #DEFINES /////////////////////////////////////////////////////////////////
#define NUM_ROCKS   5

#define WORLD_W 320
#define WORLD_H 240

#define PAN(x)      (int((x) * 256) / WORLD_W)

// TODO: refactor global variables
std::list<CObject*> objects;
BITMAP *ship1, *ship2, *rock, *ammo1, *ammo2, *buf, *explode, *bar1, *bar2;

int gfx_card = GFX_AUTODETECT_WINDOWED;
int gfx_w = 640;
int gfx_h = 480;
int gfx_bpp = 8;

// MAIN /////////////////////////////////////////////////////////////////////
int main()
{
  // init stuff ///////////////////////////////////////////////////////////
    srand(7942);
    allegro_init();
    install_timer();
    install_mouse();    // added for screen resolution selection dialog
    install_keyboard();
    install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, "SOUND.CFG");

    // START NEW GFX MODE
    /* set a graphics mode sized 320x200 */
    if (set_gfx_mode(GFX_SAFE, 320, 200, 0, 0)!=0) {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        allegro_message("Unable to set any graphic mode\n%s\n", allegro_error);
        return 1;
    }

    RGB grey = { 52, 51, 49 };

    vsync();
    set_color(0, &grey);
    set_color(1, &black_palette[0]);

    gui_fg_color = 1;
    gui_bg_color = 0;

    gfx_bpp = desktop_color_depth();
    if (false && !gfx_mode_select_ex(&gfx_card, &gfx_w, &gfx_h, &gfx_bpp)) {
        return -1;
    }
    if (gfx_bpp != 0) {
        set_color_depth(gfx_bpp);
    }

    if (set_gfx_mode(gfx_card, gfx_w, gfx_h, 0, 0) != 0) return 1;
    // END NEW GFX MODE

    // enable running in background if possible
    if (set_display_switch_mode(SWITCH_BACKGROUND) != 0 &&
        set_display_switch_mode(SWITCH_BACKAMNESIA) != 0)
    {
        // fail silently: just run paused when switched to background

        // set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        // allegro_message("Unable to set switch mode\n%s\n", allegro_error);
        // return 1;
    }

    // work-around to get mouse pointer out of game area
    // causes problems in fullscreen mode
    if (is_windowed_mode())
    {
        show_mouse(screen);
        position_mouse(SCREEN_W+10, SCREEN_H+10);
    }

    set_volume(255,255);

    // Load data ////////////////////////////////////////////////////////////
    DATAFILE *data, *palette_object;

    // Set palette before loading data file so color conversion correct
    palette_object = load_datafile_object("asteroid.dat", "Game_palette");

    if (!palette_object)
    {
        allegro_message("Error loading PAL data!\n");
        exit(2);
    }

    set_palette((RGB *) palette_object->dat);

    // Destroy unneeded palette
    unload_datafile_object(palette_object);

    set_color_conversion(COLORCONV_KEEP_TRANS | COLORCONV_TOTAL);
    data = load_datafile("asteroid.dat");

    if (!data)
    {
        allegro_message("Error loading game data!\n");
        exit(2);
    }

    // load MIDI ////////////////////////////////////////////////////////////
    MIDI *midi;
    midi = (MIDI *)data[__music].dat;
    play_midi(midi, 1);

    // load SFX /////////////////////////////////////////////////////////////
    SAMPLE *boom, *engine, *shoot;
    boom = (SAMPLE *)data[__Boom].dat;
    engine = (SAMPLE *)data[__Engine].dat;
    shoot = (SAMPLE *)data[__Shoot].dat;

    // load bitmaps /////////////////////////////////////////////////////////

    explode = (BITMAP *)data[__Explode].dat;
    rock = (BITMAP *)data[__Rock].dat;
    ammo1 = (BITMAP *)data[__Ammo1].dat;
    ship1 = (BITMAP *)data[__Ship1].dat;
    ammo2 = (BITMAP *)data[__Ammo2].dat;
    ship2 = (BITMAP *)data[__Ship2].dat;
    bar1 = (BITMAP *)data[__Bar1].dat;
    bar2 = (BITMAP *)data[__Bar2].dat;

    buf = create_bitmap(WORLD_W, WORLD_H);

    int Ship1Color = makecol(97, 207, 207);
    int Ship2Color = makecol(97, 255, 190);
    int StarColor[8];
    for (int i=0; i<8; i++)
    {
        StarColor[i] = makecol(255*i/7, 255*i/7, 255*i/7);
    }


    // create objects ///////////////////////////////////////////////////////
    CObject *Ship1 = new CObject(SHIP, NULL);
    objects.push_front(Ship1);

    int ShotDelay1 = 0;
    int Energy1 = 1000;
    bool fPlayEngine1 = FALSE;

    CObject *Ship2 = new CObject(SHIP, Ship1);
    objects.push_front(Ship2);

    int ShotDelay2 = 0;
    int Energy2 = 1000;
    bool fPlayEngine2 = FALSE;

    for (int i = 0; i<NUM_ROCKS; i++)
    {
        objects.push_front(new CObject(ROCK, NULL));
    }

// STAR FIELD STUFF BY SHAWN HARGRAEVES //////////////////////////////////////
int x, y, ix, iy, c2, star_count = 0, star_count_count = 0;
#define MAX_STARS       128

    volatile struct {
       fixed x, y, z;
       int ox, oy;
    } star[MAX_STARS];


    std::list<CObject*>::iterator iter_i, iter_j;

    // main loop ////////////////////////////////////////////////////////////
    while(!key[KEY_ESC])
    {
        // erase buf //
        clear(buf);

        // HANDLE KEYPRESSES ////////////////////////////////////////////////
// action: ship: handle input

        // player 1 //
        if (Ship1->GetData())
        {
            if(key[KEY_W])
            {
                Ship1->addForce( 7, Ship1->bearing);
                fPlayEngine1 = TRUE;
            }
            if(key[KEY_S])
            {
                Ship1->addForce(-7, Ship1->bearing);
                fPlayEngine1 = TRUE;
            }

            if(key[KEY_A])    Ship1->Rotate( 3 * FIX_PER_RAD);
            if(key[KEY_D])    Ship1->Rotate(-3 * FIX_PER_RAD);

            if((key[KEY_R]))
            {
                if (Energy1 > -10)
                {
                    Energy1 -= 30;
                }

                if (ShotDelay1 == 0 && Energy1 > 100)
                {
                    objects.push_front(new CObject(SHOT, Ship1));

                    ShotDelay1 = 4;
                    play_sample(shoot, 64, PAN(Ship1->GetX()), 1000, 0);
                }
            }
            if (ShotDelay1 > 0) ShotDelay1--;
            if (Energy1 < 1000) Energy1 += abs(Energy1/100)+2;
        }

        if (Ship2->GetData())
        {
            // player 2 //
            if(key[KEY_UP])
            {
                Ship2->addForce(7, Ship2->bearing);
                fPlayEngine2 = TRUE;
            }

            if(key[KEY_DOWN])
            {
                Ship2->addForce(-7, Ship2->bearing);
                fPlayEngine2 = TRUE;
            }

            if(key[KEY_LEFT])    Ship2->Rotate( 3 * FIX_PER_RAD);
            if(key[KEY_RIGHT])   Ship2->Rotate(-3 * FIX_PER_RAD);

            if(key[KEY_KANJI] || key[KEY_ALTGR] || key[KEY_LCONTROL] || key[KEY_SPACE])
            {

                if (Energy2 > -10)
                {
                    // Commmented out for dev purposes
                    // Energy2 -= 30;
                }

                if (ShotDelay2 == 0 && Energy2 > 10)
                {
                    objects.push_front(new CObject(SHOT, Ship2));
                    // action: ship: spawn projectile
                    ShotDelay2 = 4;
                    play_sample(shoot, 64, PAN(Ship2->GetX()), 1000, 0);
                }
            }
            if (ShotDelay2 > 0) ShotDelay2--;
            if (Energy2 < 1000) Energy2 += abs(Energy2/100)+2;

        }

// action: ship: render engine sound
        if(fPlayEngine1)
        {
            if (Ship1->GetData())
            {
                play_sample(engine, 64, PAN(Ship1->GetX()), 1000, 0);
            }
            fPlayEngine1 = FALSE;
        }

        else if(fPlayEngine2)
        {
            if (Ship2->GetData())
            {
                play_sample(engine, 64, PAN(Ship2->GetX()), 1000, 0);
            }
            fPlayEngine2 = FALSE;
        }
        else
        {
            stop_sample(engine);
        }

        iter_i = objects.begin();
        while(iter_i != objects.end())
        {
            CObject *o = *iter_i;
            if (o->update())
            {
                objects.erase(iter_i++);
                delete o;
            }
            else iter_i++;
        }

        // HANDLE COLLISIONS ////////////////////////////////////////////////

        iter_i = objects.begin();
        while (iter_i != objects.end())
        {
            iter_j = iter_i;
            iter_j++;

            while (iter_j != objects.end())
            {
                CObject *p = *iter_i;
                CObject *q = *iter_j;

                CObject::handleCollision(p, q);
                iter_j++;
            }
            iter_i++;
        }

        // DRAW EVERYTHING //////////////////////////////////////////////////

                // starfield ( by Shawn Hargraeves)

     /* animate the starfield */
      for (int c=0; c<MAX_STARS; c++) {
     if (star[c].z <= itofix(1)) {
        x = itofix(rand()&0xff);
        y = itofix(((rand()&3)+1)*buf->w);
        star[c].x = fmul(fcos(x), y);
        star[c].y = fmul(fsin(x), y);
        star[c].z = itofix((rand() & 0x1f) + 0x20);
     }

     x = fdiv(star[c].x, star[c].z);
     y = fdiv(star[c].y, star[c].z);
     ix = (int)(x>>16) + buf->w/2;
     iy = (int)(y>>16) + buf->h/2;
     // putpixel(screen, star[c].ox, star[c].oy, 0);
     if ((ix >= 0) && (ix < buf->w) && (iy >= 0) && (iy <= buf->h)) {
        // if (getpixel(screen, ix, iy) == 0) {
           if (c < star_count) {
          c2 = 7-(int)(star[c].z>>18);
          putpixel(buf, ix, iy, StarColor[MID(0, c2, 7)]);
           }
           star[c].ox = ix;
           star[c].oy = iy;
        // }
        star[c].z -= 4096;
     }
     else
        star[c].z = 0;
      }

      if (star_count < MAX_STARS) {
     if (star_count_count++ >= 32) {
        star_count_count = 0;
        star_count++;
     }
      }

        // life bars
        if (Ship1->GetData())
        {
            for (int i=Ship1->GetHealth(); i>0; i--)
            {
                blit(bar1, buf, 0, 0, 10, buf->h-1-i*2, 14, 1);
            }

            if (Energy1 > 0)
            {
                rectfill(buf, 25, buf->h-2, 30 , int(buf->h-2 - (double(Energy1)/1000.0) * 50.0), Ship1Color);
            }
        }

        if (Ship2->GetData())
        {
            for (int i=Ship2->GetHealth(); i>0; i--)
            {
                blit(bar2, buf, 0, 0, 300, buf->h-1-i*2, 14, 1);
            }

            if (Energy2 > 0)
            {
                rectfill(buf, 293, buf->h-2, 298 , int(buf->h-2 - (double(Energy2)/1000.0) * 50.0), Ship2Color);
            }
        }

        iter_i = objects.begin();
        while(iter_i != objects.end())
        {
            CObject *o = *iter_i;
            if(o->GetData())
            {
                o->Draw(buf);
            }
            iter_i++;
        }

        if (Ship2->GetData())
        {
            Ship2->ShowStats(screen);
        }

        vsync();
        stretch_blit(buf, screen, 0, 0, WORLD_W, WORLD_H, 0, 0, SCREEN_W, SCREEN_H);

    }

    allegro_exit();

    return 0;
}
END_OF_MAIN()
