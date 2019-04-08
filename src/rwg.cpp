#include <stdlib.h>
#include "rwg.h"

namespace rwg {

class object * object_list[RWG_MAX_KEY_HASH][RWG_MAX_OBJ_TYPE]; // All active objects are held here
class object * object_free[RWG_MAX_OBJ_TYPE];                   // All objects waiting for recycling are here
class object * object_new;                                      // All objects created last step
std::vector< class object *> object_del;                        // All objects destroyed last step

int object_count = 0;

int flags;

const int whittaker_table[RWG_WHITTAKER_TABLE_SIZE][RWG_WHITTAKER_TABLE_SIZE] =
{
    { 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,1,1,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,1,1,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,1,1,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 1,1,1,1,1,1,1,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 3,1,1,1,1,1,1,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 3,3,1,1,1,1,1,1,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 3,3,3,3,1,1,1,1,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 3,3,3,3,3,3,1,1,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 3,3,3,3,3,3,3,3,4,4,4,2,2,2,2,2,2,2,2,5,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 3,3,3,3,3,3,3,3,4,4,4,4,4,2,2,2,2,2,2,5,5,0,0,0,0,0,0,0,0,0,0,0 },
    { 3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,2,2,2,2,5,5,5,0,0,0,0,0,0,0,0,0,0 },
    { 3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,2,2,5,5,5,5,0,0,0,0,0,0,0,0,0 },
    { 7,7,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,2,5,5,5,5,0,0,0,0,0,0,0,0,0 },
    { 7,7,7,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,0,0,0,0,0,0,0,0 },
    { 7,7,7,7,7,7,7,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,0,0,0,0,0,0,0 },
    { 7,7,7,7,7,7,7,7,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,6,0,0,0,0,0,0 },
    { 7,7,7,7,7,7,7,8,8,8,8,4,4,4,4,4,4,4,4,4,5,5,5,5,5,6,6,0,0,0,0,0 },
    { 7,7,7,7,7,7,7,7,8,8,8,8,8,4,4,4,4,4,4,4,5,5,5,5,5,6,6,6,0,0,0,0 },
    { 9,7,7,7,7,7,7,7,8,8,8,8,8,8,8,4,4,4,4,4,5,5,5,5,5,6,6,6,6,0,0,0 },
    { 9,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,4,4,4,5,5,5,5,5,6,6,6,6,6,0,0 },
    { 9,9,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,4,4,5,5,5,5,6,6,6,6,6,6,0 },
    { 9,9,9,9,9,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,5,5,5,5,5,6,6,6,6,6,6 },
    { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,5,6,6,6,6,6,6 },
    { 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,6,6,6,6,6 }
};

void init(int rwg_flags) {
    flags = rwg_flags;
}

double double_range(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

int number_range(int a, int b) {
    int N = b-a+1;
    int ret = a+rand()/(int)(((unsigned)RAND_MAX + 1) / N);
    return RWG_URANGE(a,ret,b);
}

double cubic_interpolate(double v0, double v1, double v2, double v3,double x) {
    double P = (v3 - v2) - (v0 - v1);
    double Q = (v0 - v1) - P;
    double R = v2 - v0;
    double S = v1;

    return P*x*x*x + Q*x*x + R*x + S;
}

void destroy_map(class rwg::map *m) {
    delete m;
}

class rwg::map * create_map(size_t w, size_t h) {
    return new rwg::map(w,h);
}

void destroy_world(class rwg::world *w) {
    delete w;
}

class rwg::world * create_world(size_t w, size_t h) {
    return new rwg::world(w,h);
}

double seamless_value(double ** field, int x, int y, size_t w, size_t h) {
    size_t xx,yy;

    if (x < (-1*w)) x-= (x/w)*w;
    if (y < (-1*h)) y-= (y/h)*h;

    if (x<0) x = w+x;
    if (y<0) y = h+y;

    xx = x % w;
    yy = y % h;

    return field[xx][yy];
}

int seamless_posx(double ** field, int x, int y, size_t w, size_t h) {
    size_t xx,yy;

    if (x < (-1*w)) x-= (x/w)*w;
    if (y < (-1*h)) y-= (y/h)*h;

    if (x<0) x = w+x;
    if (y<0) y = h+y;

    xx = x % w;
    yy = y % h;

    return (int)xx;
}

int seamless_posy(double ** field, int x, int y, size_t w, size_t h) {
    size_t xx,yy;

    if (x < (-1*w)) x-= (x/w)*w;
    if (y < (-1*h)) y-= (y/h)*h;

    if (x<0) x = w+x;
    if (y<0) y = h+y;

    xx = x % w;
    yy = y % h;

    return (int)yy;
}

void seamless_pos(double ** field, int x, int y, size_t w, size_t h, int * ret_x, int * ret_y) {
    size_t xx,yy;

    if (x < (-1*w)) x-= (x/w)*w;
    if (y < (-1*h)) y-= (y/h)*h;

    if (x<0) x = w+x;
    if (y<0) y = h+y;

    xx = x % w;
    yy = y % h;

    *ret_x = xx;
    *ret_y = yy;

    return;
}

void floodfill(double **field, int x, int y, size_t w, size_t h, double depo, double strength, int ttl) {
    double depo_now = depo * strength;
    depo = depo_now /5.0;
    field[x][y]+=depo_now;

    if (ttl>0) {
        for (int i=0; i<RWG_MAX_DIR;i++) {
            int dx = RWG_DIR_XPOS(i);
            int dy = RWG_DIR_YPOS(i);
            int nx,ny;

            seamless_pos(field,x+dx, y+dy, w, h, &nx, &ny);
            floodfill(field,nx,ny,w,h,depo_now,strength,ttl-1);
        }
    }
}

void deinit(void) {
    {
        for (size_t j=0;j<RWG_MAX_OBJ_TYPE;j++) {
            for (size_t i=0;i<RWG_MAX_KEY_HASH;i++) {
                object *obj_next=NULL;
                for (object *obj=object_list[i][j];obj!=NULL;obj=obj_next) {
                    obj_next = obj->next;
                    destroy_object(obj);
                }
            }
        }
        tidy();
    }
    {
        for (size_t j=0;j<RWG_MAX_OBJ_TYPE;j++) {
            for (size_t i=0;i<RWG_MAX_KEY_HASH;i++) {
                object *obj_next=NULL;
                for (object *obj=object_list[i][j];obj!=NULL;obj=obj_next) {
                    obj_next = obj->next;
                    delete obj;
                }
            }

            object *obj_next=NULL;
            for (object *obj=object_free[j];obj!=NULL;obj=obj_next) {
                obj_next = obj->next;
                delete obj;
            }
        }

        object *obj_next=NULL;
        for (object *obj=object_new;obj!=NULL;obj=obj_next) {
            obj_next = obj->next;
            delete obj;
        }

        while (!object_del.empty()) {
            if (object_del.back()!=NULL) delete object_del.back();
            object_del.pop_back();
        }
    }
}

void tidy(void) {
    // Recycling:
    {
        // Destroyed objects will be set ready for recycling
        for (size_t i=0;i<object_del.size();i++) {
            if (object_del[i]==NULL) continue;

            object *obj = object_del[i];

            // We must check the list pointers as well!
            if (obj == object_new) {
                object_new = obj->next;
                if (object_new!=NULL) object_new->prev = NULL;
            }
            else {
                for (size_t i=0;i<RWG_MAX_KEY_HASH;i++) {
                    for (size_t j=0;j<RWG_MAX_OBJ_TYPE;j++) {
                        if (object_list[i][j] == obj) {
                            object_list[i][j] = obj->next;
                            if (object_list[i][j]!=NULL) object_list[i][j]->prev = NULL;
                        }
                    }
                }
            }

            if (obj->prev!=NULL) obj->prev->next = obj->next;
            if (obj->next!=NULL) obj->next->prev = obj->prev;
            obj->prev=NULL;
            obj->next = object_free[obj->type];
            object_free[obj->type]=obj;
            obj->clear();
        }
        object_del.clear();

        // Created objects will be inited
        object *obj_next=NULL;
        for (object *obj = object_new; obj!=NULL; obj=obj_next) {
            obj_next = obj->next;
            init_object(obj);
        }
        object_new=NULL;
    }

    // Call tidy for all objects:
    {
        object *obj=NULL;

        for (size_t j=0;j<RWG_MAX_OBJ_TYPE;j++) {
            for (size_t i=0;i<RWG_MAX_KEY_HASH;i++)
            for (obj = object_list[i][j];obj!=NULL;obj=obj->next) {
                if (obj->is_broken()) continue;
                obj->tidy();
            }
        }
    }
}

class object *create_object(int type) {
    object *obj=NULL;

    if (type>=0 && type<RWG_MAX_OBJ_TYPE && object_free[type]!=NULL) {
        obj = object_free[type];
        object_free[type]=obj->next;
        if (object_free[type]!=NULL) object_free[type]->prev=NULL;
        obj->clear();
        obj->give_new_id();
    }
    else {
        switch (type) {
            case RWG_OBJ_DEFAULT  : obj=new object;  break;
            case RWG_OBJ_DROPLET  : obj=new droplet; break;
            case RWG_OBJ_TASK     : obj=new task;    break;
            case RWG_OBJ_LAKE     : obj=new lake;    break;
            case RWG_OBJ_RIVER    : obj=new river;   break;
            case RWG_OBJ_LANDMARK : obj=new landmark;break;
            default: break;
        }
    }
    if (obj) {
        obj->next = object_new;
        obj->prev = NULL;
        if (object_new!=NULL) object_new->prev = obj;
        object_new = obj;
    }
    return obj;
}

void destroy_object(class object *obj) {
    if (!obj->is_broken()) {
        obj->paralyze();
    }

    for (size_t i = 0;i< object_del.size();i++) {
        if (object_del[i] == obj) return;
    }

    object_del.push_back(obj);
}

void init_object(class object *obj) {
    object *o_next=NULL;
    bool found=false;

    if (obj==NULL) {
        return;
    }


    if (object_new==obj) {
        object_new=obj->next;
        if (object_new!=NULL) object_new->prev=NULL;
        found=true;
    }
    else {
        for (object *o=object_new;o!=NULL;o=o_next) {
            o_next = o->next;
            if (o == obj) {
                obj->prev->next=obj->next;
                if (obj->next!=NULL) obj->next->prev=obj->prev;
                found=true;
                break;
            }
        }
    }

    if (found) {
        if (!obj->create()) {
            // Failed creating object.
        }

        int key = obj->get_id()%RWG_MAX_KEY_HASH;
        obj->prev=NULL;
        if (object_list[key][obj->type]!=NULL) object_list[key][obj->type]->prev = obj;
        obj->next=object_list[key][obj->type];
        object_list[key][obj->type]=obj;
    }
}

class object * find_object(int id) {
    object *obj=NULL;
    for (size_t i=0;i<RWG_MAX_OBJ_TYPE;i++) {
        obj = find_object(id,i);
        if (obj!=NULL) return obj;
    }
    return NULL;
}

class object * find_object(int id, int type) {
    int key = id % RWG_MAX_KEY_HASH;
    object *obj=NULL;

    for (obj = object_list[key][type];obj!=NULL;obj=obj->next) {
        if (obj->get_id() == id) {
            if (obj->is_broken()) return NULL;

            return obj;
        }
    }
    return NULL;
}

}
