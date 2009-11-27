// asteroid.cpp /////////////////////////////////////////////////////////////

// #INCLUDES ////////////////////////////////////////////////////////////////
#include <allegro.h>
// #include <random.h> // not available in MSVC
#include <stdlib.h>    // for rand()
#include "asteroid.h"
#include "object.h"


// #DEFINES /////////////////////////////////////////////////////////////////
#define Rnd(x)      ((rand() % (x)))
#define PAN(x)      (int((x) * 256) / SCREEN_W)
#define NUM_ROCKS    1
#define MAX_SHOTS   10
#define MAX_EXPLODE 10

#define WORLD_W 320
#define WORLD_H 240

// COLLIDE //////////////////////////////////////////////////////////////////
int Collide(CObject *p1, CObject *p2)
{
    if (Distance(p1->GetX(), p1->GetY(), p2->GetX(), p2->GetY()) <
        (p1->GetRadius() + p2->GetRadius())) return 1;

    return 0;
}

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
    BITMAP *ship1, *ship2, *rock, *ammo1, *ammo2, *buf, *explode, *bar1, *bar2;
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
    int WhiteColor = makecol(255, 255, 255);
    for (int i=0; i<8; i++)
    {
        StarColor[i] = makecol(255*i/7, 255*i/7, 255*i/7);
    }


    // create objects ///////////////////////////////////////////////////////
    CObject *Ship1;
    CObject *Shot1[MAX_SHOTS];
    Ship1 = new CObject(80, 100, 0, 10, 100, 1, 0, 0);
    for (int i=0; i<MAX_SHOTS;i++)
    {
        Shot1[i] = NULL;
    }

    int ShotDelay1 = 0;
    int Energy1 = 1000;
    bool fPlayEngine1 = FALSE;

    CObject *Ship2;
    CObject *Shot2[MAX_SHOTS];
    Ship2 = new CObject(240, 100, 0, 10, 100, 1, 0, 0);
    for (int i=0; i<MAX_SHOTS;i++)
    {
        Shot2[i] = NULL;
    }
    int ShotDelay2 = 0;
    int Energy2 = 1000;
    bool fPlayEngine2 = FALSE;

    CObject *Rocks[NUM_ROCKS];
    for (int i = 0; i<NUM_ROCKS; i++)
    {
        Rocks[i] = new CObject (Rnd(WORLD_W), Rnd(WORLD_H), .1,
                                5, 100, 1, Rnd(256), Rnd(256));
    }

    CObject *Explode[MAX_EXPLODE];
    for (int i =0; i< MAX_EXPLODE; i++)
    {
        Explode[i] = NULL;
    }

// STAR FIELD STUFF BY SHAWN HARGRAEVES //////////////////////////////////////
int x, y, ix, iy, c2, star_count = 0, star_count_count = 0;
#define MAX_STARS       128

    volatile struct {
       fixed x, y, z;
       int ox, oy;
    } star[MAX_STARS];


    // main loop ////////////////////////////////////////////////////////////
    while(!key[KEY_ESC])
    {
        // erase buf //
        clear(buf);

        // HANDLE KEYPRESSES ////////////////////////////////////////////////

        // player 1 //
        if (Ship1 != NULL)
        {
            if(key[KEY_W])
            {
                Ship1->Move( .07, Ship1->GetBearing());
                fPlayEngine1 = TRUE;
            }
            if(key[KEY_S])
            {
                Ship1->Move(-.07, Ship1->GetBearing());
                fPlayEngine1 = TRUE;
            }

            if(key[KEY_A])    Ship1->Rotate(-3);
            if(key[KEY_D])    Ship1->Rotate(3);

            if((key[KEY_R]))
            {
                if (Energy1 > -10)
                {
                    Energy1 -= 30;
                }

                if (ShotDelay1 == 0 && Energy1 > 100)
                {

                    for(int i = 0; i<MAX_SHOTS; i++)
                    {
                        if (Shot1[i] == NULL)
                        {
                            Shot1[i] = new CObject(Ship1->GetX(), Ship1->GetY(),
                                           3, 4, 0, 25,
                                           Ship1->GetBearing(), Ship1->GetBearing());

                            Shot1[i]->Move();
                            Shot1[i]->Move(Ship1->speed, Ship1->heading);
                            ShotDelay1 = 4;
                            play_sample(shoot, 64, PAN(Ship1->GetX()), 1000, 0);
                            break;
                        }
                    }
                }
            }
            if (ShotDelay1 > 0) ShotDelay1--;
            if (Energy1 < 1000) Energy1 += abs(Energy1/100)+2;
        }

        if (Ship2 != NULL)
        {
            // player 2 //
            if(key[KEY_UP])
            {
                Ship2->Move( .07, Ship2->GetBearing());
                fPlayEngine2 = TRUE;
            }

            if(key[KEY_DOWN])
            {
                Ship2->Move(-.07, Ship2->GetBearing());
                fPlayEngine2 = TRUE;
            }

            if(key[KEY_LEFT])    Ship2->Rotate(-3);
            if(key[KEY_RIGHT])    Ship2->Rotate( 3);

            if(key[KEY_KANJI] || key[KEY_ALTGR] || key[KEY_LCONTROL] || key[KEY_SPACE])
            {

                if (Energy2 > -10)
                {
                    // Commmented out for dev purposes
                    // Energy2 -= 30;
                }

                if (ShotDelay2 == 0 && Energy2 > 10)
                {
                    for(int i = 0; i<MAX_SHOTS; i++)
                    {
                        if (Shot2[i] == NULL)
                        {
                            Shot2[i] = new CObject(Ship2->GetX(),
                                                   Ship2->GetY(),
                                                   Ship2->speed,               // speed
                                                   4,                          // radius
                                                   0,                          // health
                                                   25,                         // data
                                                   RAD2FIX( Ship2->heading ),  // heading
                                                   RAD2FIX( Ship2->heading )); // bearing

                            Shot2[i]->Move(3, Ship2->GetBearing());

                            ShotDelay2 = 4;
                            play_sample(shoot, 64, PAN(Ship2->GetX()), 1000, 0);
                            break;
                        }
                    }
                }
            }
            if (ShotDelay2 > 0) ShotDelay2--;
            if (Energy2 < 1000) Energy2 += abs(Energy2/100)+2;

        }

        if(fPlayEngine1)
        {
            if (Ship1 != NULL)
            {
                play_sample(engine, 64, PAN(Ship1->GetX()), 1000, 0);
            }
            fPlayEngine1 = FALSE;
        }

        else if(fPlayEngine2)
        {
            if (Ship2 != NULL)
            {
                play_sample(engine, 64, PAN(Ship2->GetX()), 1000, 0);
            }
            fPlayEngine2 = FALSE;
        }
        else
        {
            stop_sample(engine);
        }


        // HANDLE MOVEMENT //////////////////////////////////////////////////

        // player 1 //
        if (Ship1 != NULL)
        {
            if (Ship1->GetHealth() < 0)
            {
                for(int i = 0; i<MAX_EXPLODE; i++)
                {
                    if (Explode[i] == NULL)
                    {
                        Explode[i] = new CObject(Ship1->GetX(), Ship1->GetY(),
                                       Ship1->speed, 0, 0, 30,
                                       Ship1->GetBearing(), Ship1->GetBearing());
                        break;
                    }
                }
                delete Ship1;
                Ship1 = NULL;
            }
            else
            {
                // TODO: Move limits to Ship object subclass
                /// if (Ship1->speed >  2) Ship1->SetVelocity( 2);
                /// if (Ship1->speed < -2) Ship1->SetVelocity(-2);
                Ship1->Move();
                /// Ship1->SetVelocity(Ship1->speed * .99);
            }
        }

        // player 2 //
        if (Ship2 != NULL)
        {
            if (Ship2->GetHealth() < 0)
            {
                for(int i = 0; i<MAX_EXPLODE; i++)
                {
                    if (Explode[i] == NULL)
                    {
                        Explode[i] = new CObject(Ship2->GetX(), Ship2->GetY(),
                                       Ship2->speed, 0, 0, 30,
                                       Ship2->GetBearing(), Ship2->GetBearing());
                        break;
                    }
                }
                delete Ship2;
                Ship2 = NULL;
            }
            else
            {
                // TODO: Move limits to Ship object subclass
                /// if (Ship2->speed >  2) Ship2->SetVelocity( 2);
                /// if (Ship2->speed < -2) Ship2->SetVelocity(-2);
                Ship2->Move();
                /// Ship2->SetVelocity(Ship2->speed * .99);
            }
        }

        // missiles //
        for (int i = 0; i<MAX_SHOTS; i++)
        {
            // player 1 //
            if (Shot1[i] != NULL)
            {
                Shot1[i]->SetData(Shot1[i]->GetData()-1);
                if (Shot1[i]->GetData() < 0)
                {
                    delete Shot1[i];
                    Shot1[i] = NULL;
                }
                else
                {
                    Shot1[i]->Move();
                    Shot1[i]->Rotate(6);
                }
            }

            // player 2 //
            if (Shot2[i] != NULL)
            {
                Shot2[i]->SetData(Shot2[i]->GetData()-1);
                if (Shot2[i]->GetData() < 0)
                {
                    delete Shot2[i];
                    Shot2[i] = NULL;
                }
                else
                {
                    Shot2[i]->Move();
                    Shot2[i]->Rotate(6);
                }
            }
        }

        // asteroids //
        for (int i = 0; i<NUM_ROCKS; i++)
        {
            if (Rocks[i] != NULL)
            {
                if (Rocks[i]->GetHealth() < 0)
                {
                    for(int c = 0; c<MAX_EXPLODE; c++)
                    {
                        if (Explode[c] == NULL)
                        {
                            Explode[c] = new CObject(Rocks[i]->GetX(), Rocks[i]->GetY(),
                                           Rocks[i]->speed, 0, 0, 30,
                                           Rocks[i]->GetBearing(), Rocks[i]->GetBearing());
                            break;
                        }
                    }
                    delete Rocks[i];
                    Rocks[i] = NULL;
                }
                else
                {
                    Rocks[i]->Rotate(1);
                    Rocks[i]->Move();
                }
            }
        }

        // explosions //
        for(int i = 0; i<MAX_EXPLODE; i++)
        {
            if (Explode[i] != NULL)
            {
                Explode[i]->SetData(Explode[i]->GetData()-1);
                if (Explode[i]->GetData() < 0)
                {
                    delete Explode[i];
                    Explode[i] = NULL;
                }
                else
                {
                    Explode[i]->Move();
                    Explode[i]->Rotate(10);
                }
            }
        }

        // HANDLE COLLISIONS ////////////////////////////////////////////////

        // ship1, Ship2 //
        if ((Ship1 != NULL) && (Ship2 !=NULL))
        {
            if (Collide(Ship1, Ship2))
            {
                Ship1->SetHealth(Ship1->GetHealth()-3);
                Rnd(2) ? Ship1->Rotate(20) : Ship1->Rotate(-20);
                Ship1->Move(Ship2->speed,
                            RAD2FIX( bearing(Ship2->GetX(), Ship2->GetY(),
                                    Ship1->GetX(), Ship1->GetY())));

                Ship2->SetHealth(Ship2->GetHealth()-3);
                Rnd(2) ? Ship2->Rotate(20) : Ship2->Rotate(-20);
                Ship2->Move(Ship1->speed,
                            RAD2FIX( bearing(Ship1->GetX(), Ship1->GetY(),
                                       Ship2->GetX(), Ship2->GetY())));

                play_sample(boom, 128, PAN(Ship2->GetX()), 1000, 0);
            }
        }


        // Ship1 Missiles, Ship2 //
        if (Ship2 != NULL)
        {
            for(int i=0; i<MAX_SHOTS; i++)
            {
                if (Shot1[i] != NULL)
                {
                    if (Collide(Shot1[i], Ship2))
                    {
                        Ship2->SetHealth(Ship2->GetHealth()-1);
                        Rnd(2) ? Ship2->Rotate(20) : Ship2->Rotate(-20);
                        Ship2->Move(.06,
                                    RAD2FIX( bearing(Shot1[i]->GetX(), Shot1[i]->GetY(),
                                       Ship2->GetX(), Ship2->GetY())));

                        play_sample(boom, 128, PAN(Ship2->GetX()), 1000, 0);

                        circlefill(buf, (int)Shot1[i]->GetX(), WORLD_H - (int)Shot1[i]->GetY(),
                                5, WhiteColor);
                        delete Shot1[i];
                        Shot1[i] = NULL;
                    }
                }
            }
        }

        // Ship2 Missiles, Ship1 //
        if (Ship1 != NULL)
        {
            for(int i=0; i<MAX_SHOTS; i++)
            {
                if (Shot2[i] != NULL)
                {
                    if (Collide(Shot2[i], Ship1))
                    {
                        Ship1->SetHealth(Ship1->GetHealth()-1);
                        Rnd(2) ? Ship1->Rotate(20) : Ship1->Rotate(-20);
                        Ship1->Move(.06,
                                    RAD2FIX( bearing(Shot2[i]->GetX(), Shot2[i]->GetY(),
                                       Ship1->GetX(), Ship1->GetY())));

                        play_sample(boom, 128, PAN(Ship1->GetX()), 1000, 0);

                        circlefill(buf, (int)Shot2[i]->GetX(), WORLD_H - (int)Shot2[i]->GetY(),
                                5, WhiteColor);

                        delete Shot2[i];
                        Shot2[i] = NULL;
                    }
                }
            }
        }

        // Asteroids, Ship1 //
        if (Ship1 != NULL)
        {
            for (int i = 0; i<NUM_ROCKS; i++)
            {
                if (Rocks[i] != NULL)
                {
                    if (Collide(Rocks[i], Ship1))
                    {
                        Ship1->SetHealth(Ship1->GetHealth()-5);
                        Rnd(2) ? Ship1->Rotate(20) : Ship1->Rotate(-20);
                        Ship1->Move(Rocks[i]->speed * 2,
                                     RAD2FIX( bearing(Rocks[i]->GetX(), Rocks[i]->GetY(),
                                              Ship1->GetX(), Ship1->GetY())));

                        Rocks[i]->SetHealth(Rocks[i]->GetHealth()-10);
                        Rocks[i]->Move(Ship1->speed / 2,
                                    RAD2FIX( bearing(Ship1->GetX(), Ship1->GetY(),
                                               Rocks[i]->GetX(), Rocks[i]->GetY())));

                        play_sample(boom, 128, PAN(Ship1->GetX()), 1000, 0);
                    }

                }
            }
        }

        // Asteroids, Ship2 //
        if (Ship2 != NULL)
        {
            for (int i = 0; i<NUM_ROCKS; i++)
            {
                if (Rocks[i] != NULL)
                {
                    if (Collide(Rocks[i], Ship2))
                    {
                        Ship2->SetHealth(Ship2->GetHealth()-5);
                        Rnd(2) ? Ship2->Rotate(20) : Ship2->Rotate(-20);
                        Ship2->Move(Rocks[i]->speed * 2,
                                     RAD2FIX( bearing(Rocks[i]->GetX(), Rocks[i]->GetY(),
                                              Ship2->GetX(), Ship2->GetY())));

                        Rocks[i]->SetHealth(Rocks[i]->GetHealth()-10);
                        Rocks[i]->Move(Ship2->speed / 2,
                                    RAD2FIX( bearing(Ship2->GetX(), Ship2->GetY(),
                                               Rocks[i]->GetX(), Rocks[i]->GetY())));

                        play_sample(boom, 128, PAN(Ship2->GetX()), 1000, 0);
                    }

                }
            }
        }

        // Asteroids, shot1 //

        for (int i = 0; i<NUM_ROCKS; i++)
        {
            if (Rocks[i] != NULL)
            {
                for(int c=0; c<MAX_SHOTS; c++)
                {
                    if (Shot1[c] != NULL)
                    {

                        if (Collide(Rocks[i], Shot1[c]))
                        {
                            Rocks[i]->SetHealth(Rocks[i]->GetHealth()-2);
                            Rnd(2) ? Rocks[i]->Rotate(20) : Rocks[i]->Rotate(-20);
                            Rocks[i]->Move(.07,
                                        RAD2FIX( bearing(Shot1[c]->GetX(), Shot1[c]->GetY(),
                                           Rocks[i]->GetX(), Rocks[i]->GetY())));

                               play_sample(boom, 128, PAN(Rocks[i]->GetX()), 1000, 0);

                               circlefill(buf, (int)Shot1[c]->GetX(), WORLD_H - (int)Shot1[c]->GetY(),
                                5, WhiteColor);

                            delete Shot1[c];
                            Shot1[c] = NULL;
                        }

                    }
                }
            }
        }

        // Asteroids, shot2 //

        for (int i = 0; i<NUM_ROCKS; i++)
        {
            if (Rocks[i] != NULL)
            {
                for(int c=0; c<MAX_SHOTS; c++)
                {
                    if (Shot2[c] != NULL)
                    {

                        if (Collide(Rocks[i], Shot2[c]))
                        {
                            Rocks[i]->SetHealth(Rocks[i]->GetHealth()-2);
                            Rnd(2) ? Rocks[i]->Rotate(20) : Rocks[i]->Rotate(-20);
                            Rocks[i]->Move(.07,
                                        RAD2FIX( bearing(Shot2[c]->GetX(), Shot2[c]->GetY(),
                                           Rocks[i]->GetX(), Rocks[i]->GetY())));

                               play_sample(boom, 128, PAN(Rocks[i]->GetX()), 1000, 0);

                               circlefill(buf, (int)Shot2[c]->GetX(), WORLD_H - (int)Shot2[c]->GetY(),
                                5, WhiteColor);
                            delete Shot2[c];
                            Shot2[c] = NULL;
                        }

                    }
                }
            }
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
        if (Ship1 != NULL)
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

        if (Ship2 != NULL)
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


        // ships //
        if (Ship1 != NULL)
        {
            Ship1->Draw(ship1, buf);
        }

        if (Ship2 != NULL)
        {
            Ship2->Draw(ship2, buf);
        }

        // missiles //
        for (int i = 0; i<MAX_SHOTS; i++)
        {
            if (Shot1[i] != NULL)
            {
                Shot1[i]->Draw(ammo1, buf);
            }

            if (Shot2[i] != NULL)
            {
                Shot2[i]->Draw(ammo2, buf);
            }
        }

        // asteroids //
        for (int i = 0; i<NUM_ROCKS; i++)
        {
            if (Rocks[i] != NULL)
            {
                Rocks[i]->Draw(rock, buf);
            }
        }

        for(int i = 0; i<MAX_EXPLODE; i++)
        {
            if (Explode[i] != NULL)
            {
                Explode[i]->Draw(explode, buf);
            }
        }

        if (Ship2 != NULL)
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
