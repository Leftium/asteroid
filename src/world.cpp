#include "world.h"
#include "kinput.h"
#include "ship.h"
#include "sound.h"
#include "starfield.h"
#include "rock.h"
#include "text.h"

#define PAN(x)      (int((x) * 256) / w)

extern BITMAP *ship1, *ship2, *rock, *ammo1, *ammo2, *explode, *bar1, *bar2;
extern SAMPLE *boom, *engine, *shoot;
extern bool DEBUG;

World::World(int width, int height): w(width), h(height) {}

void World::addObject(CObject  *o)
{
    objects.push_back(objectPtr(o));
}

objectPtr World::lastObject()
{
    return objects.back();
}

void World::wrapToWorld(objectPtr o)
{
    while (o->p.x < 0) o->p.x += w;
    while (o->p.y < 0) o->p.y += h;

    while (o->p.x >= w) o->p.x -= w;
    while (o->p.y >= h) o->p.y -= h;
}

void World::update()
{
    objectIter iter_i, iter_j;

    // update each object
    iter_i = objects.begin();
    while(iter_i != objects.end())
    {
        objectPtr o = *iter_i;
        if (o->update())
        {
            // object requested to be removed
            o.reset();
            iter_i = objects.erase(iter_i);
        }
        else iter_i++;
    }


    // HANDLE COLLISIONS
    iter_i = objects.begin();
    while (iter_i != objects.end())
    {
        iter_j = iter_i;
        iter_j++;

        while (iter_j != objects.end())
        {
            objectPtr p = *iter_i;
            objectPtr q = *iter_j;

            CObject::handleCollision(p.get(), q.get());
            iter_j++;
        }
        iter_i++;
    }

    // remove all dependents of expired objects, as well as their dependents
    bool removedObject = false;
    do
    {
        removedObject = false;
        iter_i = objects.begin();
        while(iter_i != objects.end())
        {
            objectPtr o = *iter_i;
            if (o->checkDependencies())
            {
                o.reset();
                iter_i = objects.erase(iter_i);
                removedObject = true;
            }
            else iter_i++;
        }
    } while (removedObject == true);
}


void World::renderCamera(BITMAP *buf, Camera camera)
{
    objectIter iter_i = objects.begin();
    while(iter_i != objects.end())
    {
        objectPtr o = *iter_i;
        render(buf, o, camera);
        iter_i++;
    }
}


void World::render(BITMAP *buf, Camera c)
{
    rect(buf, c.x - c.w/2, h - (c.y - c.h/2), c.x + c.w/2, h - (c.y + c.h/2), makecol(255, 255, 0));
}

void World::render(BITMAP *buf, objectPtr o, Camera &camera)
{
    const int white = makecol(255, 255, 255);
    const int red   = makecol(255,   0,   0);
    const int green = makecol(  0, 255,   0);
    const int blue  = makecol(  0,   0, 255);

    static int ship1EngineVoice = -1;
    static int ship2EngineVoice = -1;

    BITMAP* bmp      = NULL;
    SAMPLE* sample   = NULL;
    int *engineVoice = NULL;

    SoundPtr sound;
    ShipPtr s;
    StarfieldPtr starfield;
    TextPtr text;

    int c;

    double scale = 0;

    wrapToWorld(o);

    vector2f center = camera.World2Screen(o->p.x, o->p.y);
    vector2f velocity = camera.World2Screen(o->p.x + o->v.x * 10, o->p.y + o->v.y * 10);
    vector2f bearing = camera.World2Screen(o->p.x + cos(o->bearing) * (o->radius-1),
                                           o->p.y + sin(o->azimuth) * (o->radius-1));

    if (center.x < 0 || center.x >= buf->w || center.y < 0 || center.y >= buf->h)
    {
        if (o->type != STARFIELD) return;
    }

    int pan = 255 * center.x/buf->w;
    vector2f topleft;

    switch (o->type)
    {
        case STARFIELD:
            starfield = std::tr1::dynamic_pointer_cast<Starfield>(o);
            starfield->draw_starfield_3d(buf);
            break;

        case FLASH:
            circlefill(buf, center.x, center.y, camera.toScreenLength(o->radius), white);
            break;

        case SHIP:
            s = std::tr1::dynamic_pointer_cast<Ship>(o);
            if (o->team == 1)
            {
                bmp = ship1;
                engineVoice = &ship1EngineVoice;
            }
            else
            {
                bmp = ship2;
                engineVoice = &ship2EngineVoice;
            }

            if (s->isEngineOn_)
            {
                int freq = 0;

                if (*engineVoice >= 0 && (voice_get_position(*engineVoice) == -1 || voice_check(*engineVoice) == NULL))
                {
                    deallocate_voice(*engineVoice);
                    *engineVoice = -1;
                }

                if (*engineVoice < 0)
                {
                    freq = (engineVoice == &ship1EngineVoice ? engine->freq : engine->freq * 2);

                    *engineVoice = allocate_voice(engine);
                    if (*engineVoice >= 0)
                    {
                        voice_set_frequency(*engineVoice, freq);
                        voice_set_playmode(*engineVoice, false);
                        voice_start(*engineVoice);
                    }

                }

                if (*engineVoice >= 0)
                {
                    voice_set_pan(*engineVoice, pan);
                    voice_set_volume(*engineVoice, 64);
                }
            }
            else
            {
                if (*engineVoice >= 0)
                {
                    voice_set_volume(*engineVoice, 0);
                }
            }
            break;

        case SHOT:
            bmp = ((o->team == 1) ? ammo1 : ammo2);
            break;

        case ROCK:
            scale = o->radius/7;
            topleft = camera.World2Screen(o->p.x - rock->w/2*scale, o->p.y + rock->h/2*scale);
            rotate_scaled_sprite(buf, rock, topleft.x,
                                            topleft.y, RAD2FIX( o->azimuth ), ftofix(scale*buf->w/camera.w));
            break;

        case EXPLOSION:
            scale = o->radius/12;
            topleft = camera.World2Screen(o->p.x - explode->w/2*scale, o->p.y + explode->h/2*scale);
            rotate_scaled_sprite(buf, explode, topleft.x,
                                               topleft.y, RAD2FIX( o->azimuth ), ftofix(scale*buf->w/camera.w));
            break;

        case SOUND:
            sound = std::tr1::dynamic_pointer_cast<Sound>(o);
            switch (sound->soundType)
            {
                case SHOOT:
                    sample = shoot;
                    break;
                case BOOM:
                    sample = boom;
                    break;
                default:
                    break;
            }
            break;

        case TEXT:
            text = std::tr1::dynamic_pointer_cast<Text>(o);
            c = 128 * text->health/text->maxHealth + 128;
            c = makecol(c, 0, 0);
            textprintf_ex(buf, font, center.x, center.y, c, -1, "%s", text->message.c_str());

            break;

        default:
            break;
    }

    if (bmp != NULL)
    {
        topleft = camera.World2Screen(o->p.x - bmp->w/2, o->p.y + bmp->h/2);
        rotate_scaled_sprite(buf, bmp, topleft.x,
                                       topleft.y, -RAD2FIX( o->azimuth ), ftofix(buf->w/camera.w));
    }
    else if (sample != NULL)
    {
        play_sample(sample, 64, pan, 1000, 0);
    }

    if (DEBUG && o->radius > 0)
    {
        // collision hull/health

        circle(buf, center.x, center.y, camera.toScreenLength(o->radius), red);
        int arcLength = 255 * double(o->health)/o->maxHealth/2;
        arc(buf, center.x, center.y, itofix(-64 - arcLength), itofix(-64 + arcLength), camera.toScreenLength(o->radius), green);

        // velocity indicator
        line(buf, center.x, center.y, velocity.x, velocity.y, blue);

        // bearing indicator
        circle(buf, bearing.x, bearing.y, 1, white);

        // center
        circle(buf, center.x, center.y, 1, white);

        // object id
        // textprintf_ex(buf, font, (o->p.x + o->radius), h - (o->p.y - o->radius), white, -1, "%X", o->id);
    }
}
