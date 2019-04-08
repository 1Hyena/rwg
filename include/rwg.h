#include <string>
#include <vector>
#include <map>
#include <utility>

#define RWG_PI 3.14159265

#define RWG_TEXTURE_DEFAULT      0
#define RWG_TEXTURE_FLAMES       1
#define RWG_TEXTURE_MARBLE       2

#define RWG_LAYER_ELEVATION      0
#define RWG_LAYER_RAINFALL       1
#define RWG_LAYER_DRAINAGE       2
#define RWG_LAYER_DENSITY        3
#define RWG_LAYER_TEMPERATURE    4
#define RWG_LAYER_RIVERS         5
#define RWG_LAYER_COASTLINE      6
#define RWG_LAYER_RELIEF         7
#define RWG_LAYER_ROCKS          8
#define RWG_LAYER_NOISE          9
#define RWG_LAYER_TECTONICS     10
#define RWG_MAX_LAYER           11

#define RWG_LAYER_NAME(l) ( l == RWG_LAYER_ELEVATION   ? "elevation" : \
                            l == RWG_LAYER_RAINFALL    ? "rainfall" : \
                            l == RWG_LAYER_DRAINAGE    ? "drainage" : \
                            l == RWG_LAYER_DENSITY     ? "density" : \
                            l == RWG_LAYER_TEMPERATURE ? "temperature" : \
                            l == RWG_LAYER_COASTLINE   ? "coastline" : \
                            l == RWG_LAYER_RELIEF      ? "relief" : \
                            l == RWG_LAYER_ROCKS       ? "rocks" : \
                            l == RWG_LAYER_TECTONICS   ? "tectonics" : \
                            l == RWG_LAYER_NOISE       ? "noise" : \
                            l == RWG_LAYER_RIVERS      ? "rivers" : "unknown" )

#define RWG_STATE_INIT   0
#define RWG_STATE_META   1
#define RWG_STATE_POLISH 2
#define RWG_STATE_FINAL  3

#define RWG_SUPEREON(a) ( (a) <=(             0.0 ) ? "Singularium"      : \
                          (a) < ( 4570.0 -  542.0 ) ? "Precambrian"      : "N/A" )

#define RWG_EON(a)      ( (a) <=(            0.0 )  ? "Singularium"      : \
                          (a) < ( 4570.0 - 3800.0 ) ? "Hadean"           : \
                          (a) < ( 4570.0 - 2500.0 ) ? "Archaen"          : \
                          (a) < ( 4570.0 -  542.0 ) ? "Proterozoic"      : "Phanerozoic" )

#define RWG_ERA(a)      ( (a) <=(             0.0 ) ? "Singularium"      : \
                          (a) < ( 4570.0 - 4150.0 ) ? "Cryptic"          : \
                          (a) < ( 4570.0 - 3920.0 ) ? "Basin Groups"     : \
                          (a) < ( 4570.0 - 3850.0 ) ? "Nectarian"        : \
                          (a) < ( 4570.0 - 3800.0 ) ? "Early Imbrian"    : \
                          (a) < ( 4570.0 - 3600.0 ) ? "Eoarchean"        : \
                          (a) < ( 4570.0 - 3200.0 ) ? "Paleoarchean"     : \
                          (a) < ( 4570.0 - 2800.0 ) ? "Mesoarchean"      : \
                          (a) < ( 4570.0 - 2500.0 ) ? "Neoarchean"       : \
                          (a) < ( 4570.0 - 1600.0 ) ? "Paleoproterozoic" : \
                          (a) < ( 4570.0 - 1000.0 ) ? "Mesoproterozoic"  : \
                          (a) < ( 4570.0 -  542.0 ) ? "Neoproterozoic"   : \
                          (a) < ( 4570.0 -  251.0 ) ? "Paleozoic"        : \
                          (a) < ( 4570.0 -   65.5 ) ? "Mesozoic"         : "Cenozoic"  )

#define RWG_DIR_N   0
#define RWG_DIR_S   1
#define RWG_DIR_E   2
#define RWG_DIR_W   3
#define RWG_MAX_DIR 4

#define RWG_DIR_OPPOSITE(d) ( (d) == RWG_DIR_N ? RWG_DIR_S : \
                              (d) == RWG_DIR_S ? RWG_DIR_N : \
                              (d) == RWG_DIR_E ? RWG_DIR_W : \
                              (d) == RWG_DIR_W ? RWG_DIR_E : RWG_DIR_N )

#define RWG_SEAMLESS_V 1
#define RWG_SEAMLESS_H 2

#define RWG_SMOOTH_BILINEAR 1
#define RWG_SMOOTH_BICUBIC  2

#define RWG_UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define RWG_UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define RWG_URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b))) // Look carefully! Lowest value must be the leftmost argument
#define RWG_MAX_KEY_HASH 1024

#define RWG_OBJ_DEFAULT    0
#define RWG_OBJ_DROPLET    1
#define RWG_OBJ_TASK       2
#define RWG_OBJ_LAKE       3
#define RWG_OBJ_RIVER      4
#define RWG_OBJ_LANDMARK   5
#define RWG_MAX_OBJ_TYPE   6

#define RWG_IS_SET(flag, bit)       ((flag) & (bit))
#define RWG_SET_BIT(var, bit)       ((var) |= (bit))
#define RWG_REMOVE_BIT(var, bit)    ((var) &= ~(bit))

#define RWG_DIR_XPOS(d) (d == RWG_DIR_N ?  0 : \
                         d == RWG_DIR_S ?  0 : \
                         d == RWG_DIR_E ?  1 : \
                         d == RWG_DIR_W ? -1 : 0)

#define RWG_DIR_YPOS(d) (d == RWG_DIR_N ? -1 : \
                         d == RWG_DIR_S ?  1 : \
                         d == RWG_DIR_E ?  0 : \
                         d == RWG_DIR_W ?  0 : 0)


#define RWG_BIOME_NONE                       0
#define RWG_BIOME_TROPICAL_RAIN_FOREST       1
#define RWG_BIOME_TEMPERATE_RAIN_FOREST      2
#define RWG_BIOME_TROPICAL_SEASONAL_FOREST   3
#define RWG_BIOME_TEMPERATE_DECIDUOUS_FOREST 4
#define RWG_BIOME_TAIGA                      5
#define RWG_BIOME_TUNDRA                     6
#define RWG_BIOME_SAVANNA                    7
#define RWG_BIOME_TEMPERATE_GRASSLAND        8
#define RWG_BIOME_SUBTROPICAL_DESERT         9

#define RWG_WHITTAKER_TABLE_SIZE            32

namespace rwg {

#include "rwg/map.h"
#include "rwg/object.h"
#include "rwg/droplet.h"
#include "rwg/task.h"
#include "rwg/river.h"
#include "rwg/lake.h"
#include "rwg/landmark.h"
#include "rwg/world.h"

void init(int rwg_flags);
void deinit(void);
void tidy(void);
double double_range(double fMin, double fMax);
int    number_range(int min, int max);
double cubic_interpolate(double v0, double v1, double v2, double v3,double x);

void destroy_map(class rwg::map *);
class rwg::map * create_map(size_t w, size_t h);

void destroy_world(class rwg::world *);
class rwg::world * create_world(size_t w, size_t h);

class object *create_object(int type);
void init_object(class object *obj);
void destroy_object(class object *obj);

class object * find_object(int id);
class object * find_object(int id, int type);

double seamless_value(double ** field, int x, int y, size_t w, size_t h);
int seamless_posx(double ** field, int x, int y, size_t w, size_t h);
int seamless_posy(double ** field, int x, int y, size_t w, size_t h);
void seamless_pos(double ** field, int x, int y, size_t w, size_t h, int * ret_x, int * ret_y);

void floodfill(double **field, int x, int y, size_t w, size_t h, double depo, double strength, int ttl);

extern class object * object_list[RWG_MAX_KEY_HASH][RWG_MAX_OBJ_TYPE];
extern class object * object_free[RWG_MAX_OBJ_TYPE];
extern class object * object_new;
extern std::vector< class object *> object_del;

extern int object_count;

extern const int whittaker_table[RWG_WHITTAKER_TABLE_SIZE][RWG_WHITTAKER_TABLE_SIZE];

}

