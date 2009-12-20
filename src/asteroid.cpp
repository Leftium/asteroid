// asteroid.cpp /////////////////////////////////////////////////////////////

// #INCLUDES ////////////////////////////////////////////////////////////////
#include <allegro.h>
#include <stdlib.h>    // for rand()
#include "asteroid.h"
#include "object.h"
#include "world.h"
#include "kinput.h"
#include "ship.h"
#include "sound.h"
#include "starfield.h"
#include "rock.h"
#include "text.h"

#include <memory>

// #DEFINES /////////////////////////////////////////////////////////////////
#define NUM_ROCKS   5

#define WORLD_W 640
#define WORLD_H 480

World world(WORLD_W, WORLD_H);

bool DEBUG = true;

// TODO: refactor global variables
std::list< objectPtr > objects;
BITMAP *ship1, *ship2, *rock, *ammo1, *ammo2, *buf, *explode, *bar1, *bar2;
SAMPLE *boom, *engine, *shoot;

int gfx_card = GFX_AUTODETECT_WINDOWED;
int gfx_w = 640;
int gfx_h = 480;
int gfx_bpp = 8;



int initialize()
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
        return 2;
    }

    set_palette((RGB *) palette_object->dat);

    // Destroy unneeded palette
    unload_datafile_object(palette_object);

    set_color_conversion(COLORCONV_KEEP_TRANS | COLORCONV_TOTAL);
    data = load_datafile("asteroid.dat");

    if (!data)
    {
        allegro_message("Error loading game data!\n");
        return 2;
    }

    // load MIDI ////////////////////////////////////////////////////////////
    MIDI *midi;
    midi = (MIDI *)data[__music].dat;
    // play_midi(midi, 1);

    // load SFX /////////////////////////////////////////////////////////////
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

    return 0;
}

// MAIN /////////////////////////////////////////////////////////////////////
int main()
{
    if (int error = (initialize() != 0))
    {
        exit(error);
    }

    int Ship1Color = makecol(97, 207, 207);
    int Ship2Color = makecol(97, 255, 190);

    // create objects ///////////////////////////////////////////////////////
    world.addObject(new Starfield());

    world.addObject(new Ship(world.w/3, world.h/2, 1, 0, 100));
    ShipPtrWeak Ship1Weak = std::tr1::dynamic_pointer_cast<Ship>(world.lastObject());
    world.addObject(new kinput(world.lastObject(), KEY_W, KEY_S, KEY_A, KEY_D, KEY_H));

    world.addObject(new Ship(world.w*2/3, world.h/2 + 5, 2, M_PI, 100));
    ShipPtrWeak Ship2Weak = std::tr1::dynamic_pointer_cast<Ship>(world.lastObject());
    world.addObject(new kinput(world.lastObject(), KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_SPACE));

    for (int i = 0; i<NUM_ROCKS; i++)
    {
        world.addObject(new Rock());
    }

    objectIter iter_i, iter_j;

    bool debugKeyPressedLastFrame = false;

    // main loop ////////////////////////////////////////////////////////////
    while(!key[KEY_ESC])
    {
        if (!key[KEY_BACKSPACE] && debugKeyPressedLastFrame)
        {
            DEBUG = !DEBUG;
        }
        debugKeyPressedLastFrame = (key[KEY_BACKSPACE] != 0);

        world.update();

        // erase buf //
        clear(buf);

        // DRAW EVERYTHING //////////////////////////////////////////////////

        // life bars
        if (ShipPtr Ship1 = Ship1Weak.lock())
        {
            for (int i=Ship1->GetHealth(); i>0; i--)
            {
                blit(bar1, buf, 0, 0, 10, buf->h-1-i*2, 14, 1);
            }

            if (Ship1->energy > 0)
            {
                rectfill(buf, 25, buf->h-2, 30 , int(buf->h-2 - (double(Ship1->energy)/1000.0) * 50.0), Ship1Color);
            }
        }

        if (ShipPtr Ship2 = Ship2Weak.lock())
        {
            for (int i=Ship2->GetHealth(); i>0; i--)
            {
                blit(bar2, buf, 0, 0, buf->w - 20, buf->h-1-i*2, 14, 1);
            }

            if (Ship2->energy > 0)
            {
                rectfill(buf, buf->w - 27, buf->h-2, buf->w - 22 , int(buf->h-2 - (double(Ship2->energy)/1000.0) * 50.0), Ship2Color);
            }
        }

        world.render(buf);

        vsync();
        acquire_screen();
        stretch_blit(buf, screen, 0, 0, WORLD_W, WORLD_H, 0, 0, SCREEN_W, SCREEN_H);
        if (objectPtr Ship2 = Ship2Weak.lock())
        {
            Ship2->ShowStats(screen, Ship1Weak.lock().get());
        }
        release_screen();
    }

    allegro_exit();

    return 0;
}
END_OF_MAIN()
