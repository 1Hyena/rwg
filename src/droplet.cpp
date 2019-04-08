#include <stdlib.h>

#include "rwg.h"

using namespace rwg;

droplet::droplet() {
    type  =RWG_OBJ_DROPLET;
    clear();
}

droplet::~droplet() {
    destroy();
}

void droplet::clear(void) {
    object::clear();

    water    = 2.0;
    sediments= 0.0;
    speed    = 0.0;
    direction= number_range(0,RWG_MAX_DIR-1);
    trail.clear();
}

bool droplet::create(void) {
    if (object::create()==false) return false;

    return true;
}

bool droplet::destroy(void) {
    if (object::destroy()==false) return false;
    clear();
    return true;
}

// http://ranmantaru.com/blog/2011/10/08/water-erosion-on-heightmap-terrain/
void droplet::update(void) {
    object::update();

    if (in_world == NULL) return;
    rwg::map *map = in_world->get_elevation();
    if (!map) return;

    size_t w = map->get_w();
    size_t h = map->get_h();
    double ocean = in_world->get_ocean();

    if (x >= w || y>= w) return;

    double **field     = in_world->get_elevation()->field;
    double **density   = in_world->get_density()->field;
    double **coastline = in_world->get_coastline()->field;
    double **rivers    = in_world->get_rivers()->field;

    int    dir_low = number_range(0,RWG_MAX_DIR);
    double val_low = 0.0;
    bool first=true;
    for (size_t i=0; i<RWG_MAX_DIR; i++) {
        if (trail.size()>0
        &&  i == RWG_DIR_OPPOSITE(direction) )
            continue;

        int dx = RWG_DIR_XPOS(i);
        int dy = RWG_DIR_YPOS(i);

        double val   = rwg::seamless_value(field, x+dx, y+dy, w, h);
        if (val < val_low || first) {
            val_low = val;
            dir_low = i;
        }

        first = false;
    }

    if (water <= 0.0) {
        die();
        return;
    }
    else {
        double evaporate = 0.075*in_world->get_drainage()->field[x][y]
                                *in_world->get_temperature()->field[x][y];
        water-=evaporate;
        // Actually not all the water evaporates. Instead some is left
        // behind, so that next droplets could consume it, allowing
        // really long rivers to still reach the sea.
        rivers[x][y]+=evaporate/2.0;
    }

    //if (in_world->get_state() != RWG_STATE_POLISH) {
        if (val_low > (field[x][y] + sediments )) {
            bool break_through=false;
            if (sediments == 0.0) {
                if ( val_low > ( field[x][y] + water ) )
                    die();
                else {
                    water -= (val_low - field[x][y]);
                    break_through = true;
                }
            }
            else {
                field[x][y] += sediments;
            }
            if (!break_through) return;
        }

        speed = (speed + (field[x][y] - val_low))/2.0;

        // Flowing to the same direction halves the speed,
        // therefore we support water to change direction.
        // When speed is halved, water cannot erode that much
        // and therefore we get rid of the artifacts of having
        // only 4 directions.
        if (dir_low == direction) speed = speed/2.0;

        int odx;
        int ody;
        seamless_pos(field, x+RWG_DIR_XPOS(direction), y+RWG_DIR_YPOS(direction), w, h, &odx, &ody);

        if (direction != dir_low
        &&  sediments < water*speed
        &&  field[x][y] < field[odx][ody]
        && (in_world->get_state() == RWG_STATE_POLISH
         /*|| field[x][y] < (ocean+1.0) /2.0*/ ) ) {
            double erode = (field[odx][ody] - field[x][y]);
            erode*=(1.0-density[odx][ody]);

            if (coastline[odx][ody]>0.5) {
                if (density[odx][ody]>0.5)   erode/=2.0;
                if (density[odx][ody]>0.75)  erode/=2.0;
            }
            erode = RWG_UMIN(erode, field[odx][ody] - field[x][y]);
            sediments  += erode;

            field[odx][ody] -= erode;
        }
        else if (sediments < water*speed
        &&  val_low   < field[x][y]) {
            double erode = (field[x][y] - val_low) / 2.0;
            erode*=(1.0-density[x][y]);

            if (coastline[x][y]>0.5) {
                if (density[x][y]>0.5)   erode/=2.0;
                if (density[x][y]>0.75)  erode/=2.0;
            }
            erode = RWG_UMIN(erode, field[x][y] - val_low);
            sediments  += erode;

            erode_map(field, x, y, w, h, dir_low, erode);
        }
        else {
            double deposit = sediments - water*speed;
            if (deposit > 0.0) {
                sediments   -= deposit/2.0;

                sediments = RWG_UMAX(sediments, 0.0);
                deposit   = RWG_UMAX(deposit,   0.0);
                if (deposit > 0.0)
                    rwg::floodfill(field, x, y, w, h, deposit/2.0, 0.25, 2);
            }
        }
    //}

    double val_old = field[x][y];

    add_trail(x,y);
    /*
    size_t ts = trail.size();
    if (ts >= 3) {
        if (trail[ts-1] == trail[ts-3]) {
            die();
            return;
        }
    }
    if (ts >= 5) {
        if (trail[ts-1] == trail[ts-5]) {
            die();
            return;
        }
    }
    */
    int dx = RWG_DIR_XPOS(dir_low);
    int dy = RWG_DIR_YPOS(dir_low);

    // Before changing direction, erode from old direction:
    /*int odx = RWG_DIR_XPOS(direction);
    int ody = RWG_DIR_YPOS(direction);
    double o_erode  = (field[odx][ody] - field[x][y])/2.0;
    if (o_erode > 0.0
    &&  direction != dir_low) {
        field[odx][ody] -= o_erode;
        sediments       += o_erode;
    }*/

         if (dx > 0 && x  < w-1) x++;
    else if (dx > 0 && x == w-1) x = 0;
    else if (dx < 0 && x  >   0) x--;
    else if (dx < 0 && x ==   0) x = w-1;

         if (dy > 0 && y  < h-1) y++;
    else if (dy > 0 && y == h-1) y = 0;
    else if (dy < 0 && y  >   0) y--;
    else if (dy < 0 && y ==   0) y = h-1;

    direction = dir_low;

    if (ocean > field[x][y]) {
        double max_depo = val_old - field[x][y] + 0.005;
        double depo = RWG_UMIN(max_depo, sediments);

        // Coastline is more prone to be filled with sediments:
        if (coastline[x][y]<0.15) {
            rwg::floodfill(field, x, y, w, h, depo, 0.25, 4);
        }
        else if (coastline[x][y]<0.5) {
            rwg::floodfill(field, x, y, w, h, depo, 0.25, 3);
        }

        sediments   -= depo;
        if (sediments == 0.0) water = 0.0;
        water /= 2.0;
    }
    else {
        // Entered a new area, consume the water that is left there:
        water+=rivers[x][y];
        rivers[x][y]=0.0;
    }
}

void droplet::die(void) {
    if (trail.size()>5
    && in_world->get_state() == RWG_STATE_POLISH) {
        in_world->create_river(id);
    }
    destroy_object(this);
}

void droplet::erode_map(double **field, int x, int y, size_t w, size_t h, int dir_to, double erode) {
    double take = erode/2.0;

    field[x][y] -= take;
    erode -= take;

    erode/=3.0;
    for (size_t i=0; i<RWG_MAX_DIR; i++) {
        if (dir_to == (int)i) continue;
        int dx = RWG_DIR_XPOS(i);
        int dy = RWG_DIR_YPOS(i);

        //int px = rwg::seamless_posx(field, x+dx, y+dy, w, h);
        //int py = rwg::seamless_posy(field, x+dx, y+dy, w, h);

        int px,py;
        rwg::seamless_pos(field,x+dx,y+dy,w,h,&px,&py);

        double _erode = erode;
        double _take = _erode/2.0;
        field[px][py]-=_take;

        _erode-=_take;
        _erode/=3.0;

        for (size_t j=0; j<RWG_MAX_DIR; j++) {
            if (dir_to == (int)j) continue;
            int _dx = RWG_DIR_XPOS(j);
            int _dy = RWG_DIR_YPOS(j);

            int _px, _py;

            rwg::seamless_pos(field,px+_dx,py+_dy,w,h,&_px,&_py);

            field[_px][_py]-=_erode;
        }
    }
}

void droplet::add_trail(void) {
    add_trail(x,y);
}

void droplet::add_trail(size_t x, size_t y) {
    size_t w = in_world->get_w();
    size_t pos = y*w + x;
    size_t sz = trail.size();
    if (sz == 0) {
        trail.push_back(pos);
    }
    else {
        if (sz>=32) {
            for (size_t i=1; i<=32; i++) {
                if (trail[sz-i] == pos) return;
            }
        }
        if (trail[sz-1] != pos) {
            trail.push_back(pos);
        }
    }
}

