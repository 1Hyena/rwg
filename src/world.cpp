#include <stdlib.h>
#include <math.h>

#include "rwg.h"

using namespace rwg;

world::~world() {
    clear();
}

world::world(size_t w, size_t h) {
    this->w = w;
    this->h = h;

    elevation   =NULL;
    tectonics   =NULL;
    density     =NULL;
    rainfall    =NULL;
    drainage    =NULL;
    temperature =NULL;
    rivers      =NULL;
    coastline   =NULL;

    noise       =NULL;
    relief      =NULL;
    rocks       =NULL;

    age   = 0.0;
    state = 0;
    level = 0;
    ibuf  = 0;

    current_job  = "";
    current_done = 0.0;
    current_layer= RWG_LAYER_ELEVATION;
}

#define RWG_NEW_TASK(task)     ( {(task) = create_task(); if (!(task)) {return false;}} )
#define RWG_SCRAMBLE(task,map) ( { RWG_NEW_TASK((task)); (task)->maps["target"] = (map); \
                                                         (task)->job="scramble"; } )
#define RWG_GENERATE(task,map,noise,size) ( { RWG_NEW_TASK((task)); \
                                              (task)->maps["target"] = (map); \
                                              (task)->maps["noise"] = (noise); \
                                              (task)->job="generate"; \
                                              (task)->arg_int = (size); } )

bool world::init(void) {
    rwg::task *t = NULL;

    noise      = create_map(w,h); if (noise)      noise->init();      else return false;
    elevation  = create_map(w,h); if (elevation)  elevation->init();  else return false;
    tectonics  = create_map(w,h); if (tectonics)  tectonics->init();  else return false;
    rocks      = create_map(w,h); if (rocks)      rocks->init();      else return false;
    relief     = create_map(w,h); if (relief)     relief->init();     else return false;
    density    = create_map(w,h); if (density)    density->init();    else return false;
    drainage   = create_map(w,h); if (drainage)   drainage->init();   else return false;
    rainfall   = create_map(w,h); if (rainfall)   rainfall->init();   else return false;
    rivers     = create_map(w,h); if (rivers)     rivers->init();     else return false;
    temperature= create_map(w,h); if (temperature)temperature->init();else return false;
    coastline  = create_map(w,h); if (coastline)  coastline->init();  else return false;

    rocks  ->set_texture(RWG_TEXTURE_FLAMES);
    //relief ->set_texture(RWG_TEXTURE_MARBLE);
    //relief ->set_marble_strength(0.25);
    //relief ->set_marble_period(4.0);
    density->set_texture(RWG_TEXTURE_FLAMES);
    density->set_threshold(0.33);
    density->set_invert(true);

    RWG_NEW_TASK(t);
    t->maps["target"] = noise;
    t->job="set_smooth";
    t->arg_int=RWG_SMOOTH_BILINEAR;

    RWG_SCRAMBLE(t,noise);

    RWG_NEW_TASK(t);
    t->maps["target"] = elevation;
    t->maps["noise"]  = noise;
    t->job="generate";
    t->arg_int = 4;

    RWG_NEW_TASK(t);
    t->maps["target"] = noise;
    t->job="set_smooth";
    t->arg_int = RWG_SMOOTH_BICUBIC;

    //RWG_SCRAMBLE(t,noise); // Tectonics and elevation should be similar

    RWG_NEW_TASK(t);
    t->maps["target"] = tectonics;
    t->maps["noise"]  = noise;
    t->job="generate";
    t->arg_int = 4;

    {
        // Enhance tectonics:
        RWG_SCRAMBLE(t,noise);

        RWG_NEW_TASK(t);
        t->maps["target"] = rocks;
        t->job="set_smooth";
        t->arg_int = RWG_SMOOTH_BILINEAR;

        RWG_NEW_TASK(t);
        t->maps["target"] = rocks;
        t->maps["noise"]  = noise;
        t->job="generate";
        t->arg_int = 32;

        RWG_NEW_TASK(t);
        t->maps["target"] = rocks;
        t->job="multiply";
        t->arg_double = 0.1;

        RWG_NEW_TASK(t);
        t->maps["target"] = rocks;
        t->job="add";
        t->arg_double = 0.9;

        RWG_SCRAMBLE(t,noise);

        RWG_NEW_TASK(t);
        t->maps["target"] = relief;
        t->job="set_smooth";
        t->arg_int = RWG_SMOOTH_BICUBIC;

        RWG_GENERATE(t,relief,noise,8);

        RWG_NEW_TASK(t);
        t->maps["target"] = tectonics;
        t->maps["relief"] = relief;
        t->maps["rocks"]  = rocks;
        t->job="mix";
        t->arg_double = 0.40;
        t->arg_double2= 0.40;
    }

    RWG_SCRAMBLE(t,noise);
    RWG_GENERATE(t,density,noise,64);
    RWG_SCRAMBLE(t,noise);
    RWG_GENERATE(t,coastline,noise,64);
    RWG_SCRAMBLE(t,noise);
    RWG_GENERATE(t,drainage,noise,4);

    RWG_SCRAMBLE(t,noise);
    RWG_GENERATE(t,rainfall,noise,16);
    RWG_SCRAMBLE(t,noise);
    RWG_GENERATE(t,temperature,noise,8);

    RWG_NEW_TASK(t);
    t->maps["target"] = rainfall;
    t->job="map_multiply";
    t->maps["argument"] = temperature;

    RWG_NEW_TASK(t);
    t->maps["target"] = rivers;
    t->job="multiply";
    t->arg_double = 0.0;


    return true;
/*
    rwg::map noise(w,h);
    noise.init();
    //noise.set_seed(get_seed());
    noise.set_smooth(RWG_SMOOTH_BILINEAR);
    noise.scramble();

    elevation = create_map(w,h);
    elevation->init();
    elevation->generate(&noise, 4);

    noise.set_smooth(RWG_SMOOTH_BICUBIC);
    noise.scramble();

    tectonics = create_map(w,h);
    tectonics->init();
    tectonics->generate(&noise, 4);

    {
        // Enhance tectonics:
        noise.scramble();
        rwg::map rocks(w,h);
        rocks.init();
        rocks.set_smooth(RWG_SMOOTH_BILINEAR);
        rocks.generate(&noise, 32);
        rocks.multiply(0.1);
        rocks.add(0.9);

        noise.scramble();
        rwg::map relief(w,h);
        relief.init();
        relief.set_smooth(RWG_SMOOTH_BICUBIC);
        relief.generate(&noise,8);

        for (size_t j=0; j<h; j++) {
            for (size_t i=0; i<w; i++) {
                if (relief.field[i][j] > 0.6) {
                    double s = RWG_URANGE(0.0, relief.field[i][j] - 0.6, 0.2);
                    s *= 5.0;

                    double rock = rocks.field[i][j];
                    tectonics->field[i][j] = (1.0-s)*tectonics->field[i][j] + s*rock;
                }
            }
        }
    }

    noise.scramble();
    density   = create_map(w,h);
    density->init();
    density->generate(&noise, 32);

    noise.scramble();
    coastline   = create_map(w,h);
    coastline->init();
    coastline->generate(&noise, 64);

    noise.scramble();
    drainage   = create_map(w,h);
    drainage->init();
    drainage->generate(&noise, 4);

    noise.scramble();
    rainfall   = create_map(w,h);
    rainfall->init();
    rainfall->generate(&noise, 4);

    noise.scramble();
    temperature= create_map(w,h);
    temperature->init();
    temperature->generate(&noise, 2);

    rivers = create_map(w,h);
    rivers->init();
    rivers->multiply(0.0);
*/
}

void world::clear(void) {
    while (!objects.empty()) {
        int id = objects.back();

        object *obj = find_object(id);
        if (obj && !obj->is_broken()) {
            destroy_object(obj);
        }

        objects.pop_back();
    }

    if (elevation)   {destroy_map(elevation);    elevation   =NULL;}
    if (tectonics)   {destroy_map(tectonics);    tectonics   =NULL;}
    if (density)     {destroy_map(density);      density     =NULL;}
    if (rainfall)    {destroy_map(rainfall);     rainfall    =NULL;}
    if (drainage)    {destroy_map(drainage);     drainage    =NULL;}
    if (temperature) {destroy_map(temperature);  temperature =NULL;}
    if (rivers)      {destroy_map(rivers);       rivers      =NULL;}
    if (coastline)   {destroy_map(coastline);    coastline   =NULL;}
    if (noise)       {destroy_map(noise);        noise       =NULL;}
    if (relief)      {destroy_map(relief);       relief      =NULL;}
    if (rocks)       {destroy_map(rocks);        rocks       =NULL;}
}

void world::generate(int iterations) {
    for (int i=0; i<iterations;i++) {
        step();
    }
    tidy();
}

void world::step(void) {
    if (state == RWG_STATE_INIT) {
        step_init();
    }
    else if (state == RWG_STATE_META) {
        step_meta();
    }
    else if (state == RWG_STATE_POLISH) {
        step_polish();
    }
    else if (state == RWG_STATE_FINAL) {
        step_final();
    }
}

void world::step_init(void) {
    for (size_t i = 0; i<objects.size(); i++) {
        int id = objects[i];
        object *obj = find_object(id);
        // Tasks can only be processed one at time:
        if (obj) {
            obj->update();
            task *t = obj->to_task();
            if (t) {
                current_job = t->job;
                current_done= t->done;

                if (t->maps.count("target") == 1) {
                    map *m = t->maps["target"];

                         if (m == elevation  ) current_layer = RWG_LAYER_ELEVATION;
                    else if (m == tectonics  ) current_layer = RWG_LAYER_TECTONICS;
                    else if (m == rainfall   ) current_layer = RWG_LAYER_RAINFALL;
                    else if (m == drainage   ) current_layer = RWG_LAYER_DRAINAGE;
                    else if (m == temperature) current_layer = RWG_LAYER_TEMPERATURE;
                    else if (m == density    ) current_layer = RWG_LAYER_DENSITY;
                    else if (m == rivers     ) current_layer = RWG_LAYER_RIVERS;
                    else if (m == coastline  ) current_layer = RWG_LAYER_COASTLINE;
                    else if (m == relief     ) current_layer = RWG_LAYER_RELIEF;
                    else if (m == rocks      ) current_layer = RWG_LAYER_ROCKS;
                    else if (m == noise      ) current_layer = RWG_LAYER_NOISE;

                }
            }
            return;
        }
    }
    state = RWG_STATE_META;
    ibuf  = 0;

    /*
    for (size_t j=0; j<h; j++) {
        for (size_t i=0; i<w; i++) {
            double pw = (i*1.0)/w;
            double ph = (j*1.0)/h;
            temperature->field[i][j] = pw;
            rainfall->field[i][j] = ph;
        }
    }
    */
}

void world::step_meta(void) {
    double **field   = elevation->field;

    size_t i,j,fw,fh;
    fw = elevation->get_w();
    fh = elevation->get_h();

    if (field != NULL) {
        for (j=ibuf;j<fh;j++) {
            for (i=0;i<fw;i++) {
                /*double x = tectonics->field[i][j]; //x = x*x*x*x;
                double f_x = -pow(x-0.5,4)*16.0;
                double g_x = 1.0 + f_x/50.0;*/

                double str = 1.0 - 0.002*speed
                    *(0.5*density->field[i][j]+0.5*temperature->field[i][j]);//g_x;
                field[i][j] = (field[i][j]*(str)) + (tectonics->field[i][j]*(1.0 - str));

                rivers->field[i][j] = RWG_UMAX(rivers->field[i][j]-0.01, 0.0);
            }
            ibuf++;
            if (ibuf<fh) return;
            else {
                ibuf = 0;
                break;
            }
        }
    }


    if (age > 4500.0) {
        state = RWG_STATE_POLISH;
        ibuf  = 0;
        rivers->multiply(0.0);
    }
    else age = age+5.0;

    for (i = 0; i < fw; i++) {
        int xx = number_range(0,fw-1);
        int yy = number_range(0,fh-1);
        if (double_range(0.0,0.5) < rainfall->field[xx][yy]
        &&  field[xx][yy]>ocean_level
        &&  objects.size()<10000){
            create_droplet(xx,yy);
        }
    }

    for (size_t i = 0; i<objects.size(); i++) {
        int id = objects[i];
        object *obj = find_object(id);
        if (obj) {
            if (state == RWG_STATE_POLISH) {
                destroy_object(obj);
                continue;
            }
            obj->update();
        }
    }
}

void world::step_polish(void) {
    double **field   = elevation->field;

    size_t i,j,fw,fh;
    fw = elevation->get_w();
    fh = elevation->get_h();

    if (level == 0) {
        for (i = 0; i<objects.size(); i++) {
            int id = objects[i];
            object *obj = find_object(id);
            if (obj) {
                droplet *drop = obj->to_droplet();
                if (drop) {
                    rwg::river *r = check_tributary(drop);

                    if (r) {
                        drop->add_trail();
                        rwg::river * tributary = create_river(drop->get_id());

                        r->tributaries.push_back(tributary->get_id());
                        destroy_object(obj);
                        return;
                    }
                    obj->update();
                    return;
                }
            }
        }

        for (j=ibuf;j<fh;j++) {
            for (i=0;i<fw;i++) {
                if (double_range(0.0,250.0) < rainfall->field[i][j]
                &&  field[i][j]>ocean_level
                &&  field[i][j]<0.85){
                    rwg::droplet * d = create_droplet(i,j);
                    if (d) {
                        d->water*=10.0;
                    }
                }
            }
            ibuf++;
            if (ibuf<fh) return;
            else {
                ibuf = 0;
                break;
            }
        }
        level++;
        rivers->multiply(0.0);
        for (i = 0; i<objects.size(); i++) {
            int id = objects[i];
            object *obj = find_object(id);
            if (obj) {
                river *r = obj->to_river();
                if (r) {
                    double color = double_range(0.001,1.0);
                    size_t sz = r->trail.size();
                    for (j=0; j<sz; j++) {
                        size_t ty = r->trail[j] / w;
                        size_t tx = r->trail[j] % w;

                        rivers->field[tx][ty] = color;
                    }
                }
            }
        }
    }
    //
    else if (level == 1) {

    }
}

void world::step_final(void) {

}

void world::tidy(void) {
    size_t sz = objects.size();
    std::vector<int> bufv;
    for (size_t i=0;i<sz;i++) {
        int oid = objects[i];
        if (oid == 0
        ||  find_object(oid) == NULL)
            continue;
        bufv.push_back(oid);
    }
    objects = bufv;
}

rwg::droplet *world::create_droplet(size_t x, size_t y) {
    rwg::object * obj = create_object(RWG_OBJ_DROPLET);
    droplet * drop = (obj ? obj->to_droplet() : NULL);

    if (!drop) return NULL;

    drop->x = x;
    drop->y = y;
    drop->in_world = this;
    drop->water *= speed;

    init_object(obj);
    objects.push_back(drop->get_id());

    return drop;
}

rwg::task *world::create_task(void) {
    rwg::object * obj = create_object(RWG_OBJ_TASK);
    task * t = (obj ? obj->to_task() : NULL);
    if (!t) return NULL;

    t->in_world = this;

    init_object(t);
    objects.push_back(t->get_id());

    return t;
}

rwg::river *world::create_river(int drop_id) {
    rwg::object * obj  = find_object(drop_id, RWG_OBJ_DROPLET);
    rwg::droplet *drop = (obj ? obj->to_droplet() : NULL);

    if (!drop) return NULL;

    obj = create_object(RWG_OBJ_RIVER);
    rwg::river  *   r = ( obj ? obj->to_river() : NULL);

    if (!r) return NULL;

    r->in_world = this;
    r->trail = drop->trail;

    r->x = drop->x;
    r->y = drop->y;

    init_object(obj);
    objects.push_back(r->get_id());

    return r;
}

int world::sector_type(int x, int y) {
    return 0;
}

void world::get_droplets(std::vector<rwg::droplet *> *drops) {
    for (size_t i = 0; i<objects.size(); i++) {
        object * obj = find_object(objects[i],RWG_OBJ_DROPLET);
        if (!obj) continue;
        droplet * drop = obj->to_droplet();
        if (drop) drops->push_back(drop);
    }

    return;
}

rwg::river * world::check_tributary(rwg::droplet *drop) {
    size_t sz = objects.size();

    for (size_t i=0; i< sz; i++) {
        object * obj = find_object(objects[i],RWG_OBJ_RIVER);
        if (!obj) continue;

        river * r = obj->to_river();
        if (r) {
            size_t tsz = r->trail.size();
            for (size_t j=0; j<tsz; j++) {
                size_t ty = r->trail[j] / w;
                size_t tx = r->trail[j] % w;
                size_t dx = drop->x;
                size_t dy = drop->y;

                if (tx == dx
                &&  ty == dy) {
                    return r;
                }

                bool h = false;
                if ( tx+1 == dx
                ||   dx+1 == tx) h = true;

                bool v = false;
                if ( ty+1 == dy
                ||   dy+1 == ty) v = true;

                if ((h && dy == ty)
                ||  (v && dx == tx)) return r;
            }
        }
    }

    return NULL;
}

int world::get_biome(int x, int y) {
    double t = temperature->field[x][y];
    double p = rainfall->field[x][y];
    double e = elevation->field[x][y];

    if (e > 0.7) return RWG_BIOME_NONE;

    t*=32.0;
    p*=32.0;

    int wx = 31 - ((int) t);
    int wy = 31 - ((int) p);

    if (wx >= 0
    &&  wy >= 0
    &&  wx < RWG_WHITTAKER_TABLE_SIZE
    &&  wy < RWG_WHITTAKER_TABLE_SIZE) {
        return whittaker_table[wy][wx];
    }

    return RWG_BIOME_NONE;
}
