#include <stdio.h>
#include <math.h>

#define ALLEGRO_STATICLINK

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_color.h>

#include <rwg.h>

const float FPS = 60.0;
const int SCREEN_W = 800;
const int SCREEN_H = 600;

struct crust_type
{
    double age;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    double ocean_level;
    unsigned char wr;
    unsigned char wg;
    unsigned char wb;
};

const struct crust_type crust_table[] =
{
    { 4028.0,   0, 128,   0,  1.00,   0,   0,255  },
    { 2300.0, 188, 160, 148,  0.50,   0,   0,255  },
    { 2070.0, 138, 106,  83,  0.00,   0,   0,255  },
    { 2000.0, 153, 145, 129,  0.00,   0,   0,  0  },
    { 1800.0, 153, 145, 129,  0.50, 255,  64,  0  },
    {  770.0, 153, 145, 129,  0.80, 255, 128,  0  },
    {    0.0,   0,   0,   0,  0.90, 255, 255,  0  }
};

void draw_world(rwg::world *world, ALLEGRO_BITMAP *to, int x, int y);
void save_world(rwg::world *world, ALLEGRO_BITMAP *map);

int find_crust(double age);
void interpolate_crust(double age, int ct_1, int ct_2,
                       double *r, double *g, double *b,
                       double *ocean_level,
                       double *wr, double *wg, double *wb);

double zoom   = 1.0;
double view_x = 0.0;
double view_y = 0.0;
int    map_type = 0;
bool   redraw_world = true;

int main(int argc, char **argv) {
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    bool redraw = true;
    bool doexit = false;

    if(!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    if(!al_install_keyboard()) {
        fprintf(stderr, "failed to initialize the keyboard!\n");
        return -1;
    }

    al_install_mouse();
    al_init_image_addon();
    al_init_font_addon();

    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    al_set_new_bitmap_flags(ALLEGRO_MAG_LINEAR|ALLEGRO_MIN_LINEAR|ALLEGRO_MIPMAP);

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    ALLEGRO_FONT *font = al_load_font("data/fixed_font.tga", 0, 0);
    if (font==NULL) {
        fprintf(stderr, "failed to load font!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);

        return -1;
    }

    //ALLEGRO_BITMAP *whittaker  = al_load_bitmap("data/whittaker.png");
    ALLEGRO_BITMAP *map = al_create_bitmap(320,320);
    double map_w = (double) al_get_bitmap_width(map);
    double map_h = (double) al_get_bitmap_height(map);

    srand(time(NULL));
    rwg::init(0);
    rwg::world *world = rwg::create_world(320,320);
    world->set_ocean(0.50);
    world->set_speed(1.50);
    if (!world->init()) {
        fprintf(stderr, "failed to initialize world!\n");
        return -1;
    }

    al_set_target_bitmap(al_get_backbuffer(display));

    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(0,0,0));
    al_flip_display();
    al_start_timer(timer);

    double old_time = al_get_time();

    bool key_pad_minus_down = false;
    bool key_pad_plus_down  = false;
    bool key_up_down        = false;
    bool key_down_down      = false;
    bool key_left_down      = false;
    bool key_right_down     = false;
    bool paused             = true;

    while(!doexit) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER && redraw==false) {
            redraw = true;

            int iterate = 1;
                 if (world->get_state() == RWG_STATE_INIT)   iterate = 128;
            else if (world->get_state() == RWG_STATE_META)   iterate = 128;
            else if (world->get_state() == RWG_STATE_POLISH) iterate = 32;

            if (!redraw_world) iterate*=4;

            if (!paused) world->generate(iterate);

            rwg::tidy();
            if (key_pad_plus_down) {
                zoom += 0.1;
                if (zoom > 16.0) zoom = 16.0;
            }
            if (key_pad_minus_down) {
                zoom -= 0.1;
                if (zoom < 1.0) zoom = 1.0;
            }
            if (key_down_down  && view_y > map_h/(-1.0)) view_y-=2.0;
            if (key_up_down    && view_y < map_h/  1.0 ) view_y+=2.0;
            if (key_left_down  && view_x < map_w/( 1.0)) view_x+=2.0;
            if (key_right_down && view_x > map_w/ -1.0 ) view_x-=2.0;
        }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_PAD_PLUS:
                    key_pad_plus_down = true;
                    break;
                case ALLEGRO_KEY_PAD_MINUS:
                    key_pad_minus_down = true;
                    break;
                case ALLEGRO_KEY_LEFT:
                    key_left_down = true;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    key_right_down = true;
                    break;
                case ALLEGRO_KEY_UP:
                    key_up_down = true;
                    break;
                case ALLEGRO_KEY_DOWN:
                    key_down_down = true;
                    break;
                case ALLEGRO_KEY_PGDN:
                    map_type--;
                    if (map_type < 0) map_type = 0;
                    break;
                case ALLEGRO_KEY_PGUP:
                    map_type++;
                    if (map_type >= RWG_MAX_LAYER) map_type = RWG_MAX_LAYER-1;
                    break;
                case ALLEGRO_KEY_F12:
                    save_world(world, map);
                    break;
                case ALLEGRO_KEY_F11:
                    redraw_world = !redraw_world;
                    break;
                case ALLEGRO_KEY_SPACE:
                    paused = !paused;
                    break;
                default: break;
            }
        }
        else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_ESCAPE:
                    doexit = true;
                    break;
                case ALLEGRO_KEY_PAD_PLUS:
                    key_pad_plus_down = false;
                    break;
                case ALLEGRO_KEY_PAD_MINUS:
                    key_pad_minus_down = false;
                    break;
                case ALLEGRO_KEY_LEFT:
                    key_left_down = false;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    key_right_down = false;
                    break;
                case ALLEGRO_KEY_UP:
                    key_up_down = false;
                    break;
                case ALLEGRO_KEY_DOWN:
                    key_down_down = false;
                    break;
                default: break;
            }
        }

        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;

            if (world->get_state() == RWG_STATE_INIT) {
                map_type = world->current_layer;
            }
            if (redraw_world) draw_world(world,map,0,0);

            al_set_target_bitmap(al_get_backbuffer(display));
            al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0,0.0));

            float sw = (float) al_get_bitmap_width(map);
            float sh = (float) al_get_bitmap_height(map);
            float dw = sw*zoom;
            float dh = sh*zoom;
            float dx = (SCREEN_W / 2.0) - dw/2.0 + view_x;
            float dy = (SCREEN_H / 2.0) - dh/2.0 + view_y;

            al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx, dy, dw, dh, 0);
            if (sw <= 256) {
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx + dw, dy + dh, dw, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx - dw, dy + dh, dw, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx + dw, dy - dh, dw, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx - dw, dy - dh, dw, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx + dw, dy     , dw, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx - dw, dy     , dw, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx     , dy + dh, dw, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh, dx     , dy - dh, dw, dh, 0);
            }
            else {
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw/10.0, sh, dx + dw, dy, dw/10.0, dh, 0);
                al_draw_scaled_bitmap(map, sw - sw/10.0, 0.0, sw/10.0, sh, dx - dw/10.0, dy, dw/10.0, dh, 0);
                al_draw_scaled_bitmap(map, 0.0, 0.0, sw, sh/10.0, dx, dy + dh, dw, dh/10.0, 0);
                al_draw_scaled_bitmap(map, 0.0, sh - sh /10.0, sw, sh/10.0, dx, dy - dh/10.0, dw, dh/10.0, 0);
            }
            double new_time = al_get_time();
            double delta = new_time - old_time;
            double fps = 1.0/delta;
            old_time = new_time;

            if (font!=NULL) {
                al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
                if (!paused) {
                    al_draw_textf(font, al_map_rgb(0,255,0), 0, 0, 0, "FPS: %d (%d obj) - %s;  TASK: %s - %d%%",
                        (int)round(fps),
                        rwg::object_count,
                        RWG_LAYER_NAME(map_type),
                        world->current_job.c_str(), (int)(world->current_done*100.0));
                }
                else {
                    al_draw_textf(font, al_map_rgb(255,255,0), 0, 0, 0, "PAUSED - PRESS SPACE TO CONTINUE.");
                }
                int fh = al_get_font_line_height(font);
                double age = world->get_age();
                al_draw_textf(font, al_map_rgb(255,0,0), 0.0, 1.0*(SCREEN_H - fh), 0,
                              "SuperEon: %s;  Eon: %s;   Era: %s;   Age: %d MY",
                              RWG_SUPEREON(age),
                              RWG_EON(age),
                              RWG_ERA(age),
                              (int)age);
            }

            al_flip_display();
        }
    }

    rwg::destroy_world(world);
    rwg::deinit();

/*
    al_lock_bitmap(whittaker, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

    int w   = al_get_bitmap_width(whittaker);
    int h   = al_get_bitmap_height(whittaker);
    al_unlock_bitmap(whittaker);

    al_destroy_bitmap(whittaker);
*/

    al_destroy_bitmap(map);

    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return 0;
}

void draw_world(rwg::world *world, ALLEGRO_BITMAP *to, int x, int y) {
    size_t w = world->get_w();
    size_t h = world->get_h();
    int tw   = al_get_bitmap_width(to);
    int th   = al_get_bitmap_height(to);
    double ocean = world->get_ocean();
    double age   = world->get_age();

    al_lock_bitmap(to, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
    al_set_target_bitmap(to);
    for (size_t j=0;j<h;j++) {
        if (y+(int)j >= th) continue;
        for (size_t i=0; i<w; i++) {
            if (x+(int)i >= tw) continue;
            float val = world->get_elevation()->field[i][j];

            float vr,vg,vb;
            double cvr,cvg,cvb;
            double ovr,ovg,ovb;
            if (map_type == RWG_LAYER_ELEVATION
            ||  map_type == RWG_LAYER_RIVERS) {
                int ct_1 = find_crust(age);
                int ct_2 = RWG_UMAX(ct_1-1,0);
                double obuf;
                interpolate_crust(age,ct_1,ct_2,&cvr,&cvg,&cvb,&obuf,&ovr,&ovg,&ovb);

                if (val < (ocean * obuf)) {
                    vr = ovr;
                    vg = ovg;
                    vb = ovb;
                }
                else {
                    vr = cvr;
                    vg = cvg;
                    vb = cvb;
                }


                if (world->get_state() == RWG_STATE_POLISH) {
                         if (val < ocean){vr = 0.0; vg = 0.0; vb = val;}
                    else if (val < 0.60) {vr = 0.0; vg = val; vb = 0.0;}
                    else if (val < 0.65) {vr = val/3.0; vg = val; vb = val/10.0;}
                    else if (val < 0.75) {vr = val/2.0; vg = val; vb = val/20.0;}
                    else                 {vr = val; vg = val; vb = val;}
                }

                if (map_type == RWG_LAYER_RIVERS && obuf == 1.0) {
                    double v = 0.0;
                    v = world->get_rivers()->field[i][j];
                    if (v>0.0) {
                        vr = 0.0;
                        vg = 0.0;
                        vb = 1.0;
                    }
                    if (world->get_state() == RWG_STATE_POLISH
                    &&  world->get_level() == 1
                    &&  v>0.0) {
                        al_color_hsl_to_rgb((0.75-v/4.0)*360.0, 0.5, 0.5, &vr, &vg, &vb);
                    }
                }
            }
            else {
                if (val > ocean-0.0025
                &&  val < ocean+0.0025) {
                    vr = 0.0;
                    vg = 0.0;
                    vb = 0.0;
                }
                else {
                    double v = 0.0;
                    if (map_type == RWG_LAYER_TEMPERATURE) {
                        v = world->get_temperature()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_DRAINAGE) {
                        v = world->get_drainage()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_RAINFALL) {
                        v = world->get_rainfall()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_DENSITY) {
                        v = world->get_density()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_COASTLINE) {
                        v = world->get_coastline()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_ROCKS) {
                        v = world->get_rocks()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_RELIEF) {
                        v = world->get_relief()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_NOISE) {
                        v = world->get_noise()->field[i][j];
                    }
                    else if (map_type == RWG_LAYER_TECTONICS) {
                        v = world->get_tectonics()->field[i][j];
                    }
                    al_color_hsl_to_rgb((1.0-v)*360.0, val > ocean ? 0.25 : 0.5, 0.5, &vr, &vg, &vb);
                }
            }
            if (world->get_state() == RWG_STATE_POLISH)  {
                al_put_pixel(x+(int)i, y+(int)j, al_map_rgb_f((vr)/1.0,
                                                              (vg)/1.0,
                                                              (vb)/1.0));
            }
            else {
                al_put_pixel(x+(int)i, y+(int)j, al_map_rgb_f((vr+val)/2.0,
                                                              (vg+val)/2.0,
                                                              (vb+val)/2.0));
            }
        }
    }
    if (map_type != RWG_LAYER_RIVERS) {
        std::vector<rwg::droplet *> drops;
        world->get_droplets(&drops);
        for (size_t i =0; i < drops.size(); i++) {
            int dx = drops[i]->x;
            int dy = drops[i]->y;
            if (x + dx >= tw
            ||  y + dy >= th) continue;

            al_put_pixel(x+dx, y+dy, al_map_rgba_f(0.0, 0.0, 1.0,0.5));
        }
    }

    al_unlock_bitmap(to);

}

int find_crust(double age) {
    int i;
    for (i =0;;i++) {
        if (crust_table[i].age <= age) {
            return i;
        }
        if (crust_table[i].age == 0.0) break;
    }
    return i;
}

void interpolate_crust(double age, int ct_1, int ct_2,
                       double *r, double *g, double *b,
                       double *ocean_level,
                       double *wr, double *wg, double *wb) {
    if (ct_1 == ct_2) {
        *ocean_level = crust_table[ct_1].ocean_level;
        *r           = crust_table[ct_1].r/255.0;
        *g           = crust_table[ct_1].g/255.0;
        *b           = crust_table[ct_1].b/255.0;
        *wr          = crust_table[ct_1].wr/255.0;
        *wg          = crust_table[ct_1].wg/255.0;
        *wb          = crust_table[ct_1].wb/255.0;
        return;
    }
    if (ct_1 < ct_2) {
        int buf = ct_1;
        ct_1 = ct_2;
        ct_2 = buf;
    }
    double age_1 = crust_table[ct_1].age;
    double age_2 = crust_table[ct_2].age;
    double max_a = age_2 - age_1;
    double p     = (age - age_1)/max_a;

    *ocean_level = (1.0-p)*crust_table[ct_1].ocean_level + p*crust_table[ct_2].ocean_level;
    *r           = ((1.0-p)*crust_table[ct_1].r + p*crust_table[ct_2].r)/255.0;
    *g           = ((1.0-p)*crust_table[ct_1].g + p*crust_table[ct_2].g)/255.0;
    *b           = ((1.0-p)*crust_table[ct_1].b + p*crust_table[ct_2].b)/255.0;
    *wr          = ((1.0-p)*crust_table[ct_1].wr + p*crust_table[ct_2].wr)/255.0;
    *wg          = ((1.0-p)*crust_table[ct_1].wg + p*crust_table[ct_2].wg)/255.0;
    *wb          = ((1.0-p)*crust_table[ct_1].wb + p*crust_table[ct_2].wb)/255.0;
    return;
}

void save_world(rwg::world *world, ALLEGRO_BITMAP *map) {
    ALLEGRO_BITMAP *final = al_create_bitmap(1024,1024);

    size_t w = world->get_w();
    size_t h = world->get_h();
    int tw   = al_get_bitmap_width(map);
    int th   = al_get_bitmap_height(map);
    double ocean = world->get_ocean();

    al_lock_bitmap(map, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
    al_set_target_bitmap(map);

    for (size_t j=0;j<h;j++) {
        for (size_t i=0; i<w; i++) {
            float val = world->get_elevation()->field[i][j];

            float vr,vg,vb;
/*
                 if (val < ocean){vr = 0.0; vg = 0.0; vb = val;}
            else if (val < 0.60) {vr = 0.0; vg = val; vb = 0.0;}
            else if (val < 0.65) {vr = val/3.0; vg = val; vb = val/10.0;}
            else if (val < 0.75) {vr = val/2.0; vg = val; vb = val/20.0;}
            else                 {vr = val; vg = val; vb = val;}
*/
            vr = val;
            vg = val;
            vb = val;
            if (val >= ocean) {
                int biome = world->get_biome(i,j);
                if (biome != RWG_BIOME_NONE) {
                    switch (biome) {
                        case RWG_BIOME_SAVANNA:                    vr=0.7; vg=0.7; vb=0.0; break;
                        case RWG_BIOME_SUBTROPICAL_DESERT:         vr=1.0; vg=0.5; vb=0.0; break;
                        case RWG_BIOME_TAIGA:                      vr=0.0; vg=0.3; vb=0.0; break;
                        case RWG_BIOME_TEMPERATE_DECIDUOUS_FOREST: vr=0.0; vg=0.5; vb=0.0; break;
                        case RWG_BIOME_TEMPERATE_GRASSLAND:        vr=1.0; vg=1.0; vb=0.3; break;
                        case RWG_BIOME_TEMPERATE_RAIN_FOREST:      vr=0.0; vg=0.7; vb=0.0; break;
                        case RWG_BIOME_TROPICAL_RAIN_FOREST:       vr=0.0; vg=1.0; vb=0.0; break;
                        case RWG_BIOME_TROPICAL_SEASONAL_FOREST:   vr=0.5; vg=0.5; vb=0.0; break;
                        case RWG_BIOME_TUNDRA:                     vr=0.0; vg=0.3; vb=0.3; break;
                        default :                                  vr=0.0; vg=0.0; vb=0.0; break;
                    }
                }
            }
            else {
                vr = 0.0;
                vg = 0.0;
                vb = val;
            }

            double river = world->get_rivers()->field[i][j];
            if (river > 0.0) {
                vr = 0.0;
                vg = ((river)+1.0)/2.0;
                vb = ((river)+1.0)/2.0;
            }

            al_put_pixel((int)i, (int)j, al_map_rgb_f(vr, vg, vb) );
        }
    }

    al_unlock_bitmap(map);

    al_set_target_bitmap(final);
    al_draw_scaled_bitmap(map, 0.0, 0.0, (float)tw, (float)th,
                          0.0, 0.0, 1024.0, 1024.0, 0);

    al_save_bitmap("texture.png", final);

    // SAVE HEIGHTMAP:
    al_lock_bitmap(map, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
    al_set_target_bitmap(map);

    for (size_t j=0;j<h;j++) {
        for (size_t i=0; i<w; i++) {
            float val = world->get_elevation()->field[i][j];

            if (val <= ocean) val = ocean;

            al_put_pixel((int)i, (int)j, al_map_rgb_f(val, val, val) );
        }
    }

    al_unlock_bitmap(map);

    al_set_target_bitmap(final);
    al_draw_scaled_bitmap(map, 0.0, 0.0, (float)tw, (float)th,
                          0.0, 0.0, 1024.0, 1024.0, 0);

    al_save_bitmap("heightmap.png", final);

    al_destroy_bitmap(final);
}
