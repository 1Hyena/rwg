#include <stdlib.h>
#include <math.h>

#include "rwg.h"

using namespace rwg;

map::map(size_t w, size_t h) {
    this->w = w;
    this->h = h;

    field       = NULL;
    smooth_type = RWG_SMOOTH_BILINEAR;

    texture_type    = RWG_TEXTURE_DEFAULT;
    invert          = false;
    abs_threshold   = 0.5;
    marble_period   = 2.0;
    marble_strength = 0.25;
}

map::~map() {
    clear();
}

void map::clear(void) {
    if (field) {
        for ( size_t i = 0; i < w; ++i) {
            free (field[i]);
        }
        free(field);
    }
    field = NULL;
}

void map::init(void) {
    if (field != NULL) return;

    size_t i;

    field = (double **) malloc( w * sizeof( double *) );

    for (i = 0 ; i < w ; ++i ) {
        field[i] = (double *) malloc( h * sizeof(double) );
    }
    ibuf = 0;
}

void map::scramble(void) {
    size_t i,j;
    if (field != NULL) {
        //srand(seed);

        for (j=0;j<h;j++) {
            for (i=0;i<w;i++) {
                field[i][j] = double_range(0.0, 1.0);
            }
        }
    }
}

double map::iscramble(void) {
    size_t i,j;
    if (field != NULL) {
        for (j=ibuf;j<h;j++) {
            for (i=0;i<w;i++) {
                field[i][j] = double_range(0.0, 1.0);
            }
            ibuf++;
            double r = ibuf/(double)h;
            if (r == 1.0) ibuf=0;
            return r;
        }
    }
    ibuf=0;
    return 1.0;
}

void map::multiply(double multiplier) {
    size_t i,j;
    if (field != NULL) {
        for (j=0;j<h;j++) {
            for (i=0;i<w;i++) {
                field[i][j] *= multiplier;
            }
        }
    }
}

double map::imultiply(double multiplier) {
    size_t i,j;
    if (field != NULL) {
        for (j=ibuf;j<h;j++) {
            for (i=0;i<w;i++) {
                field[i][j] *= multiplier;
            }
            ibuf++;
            double r = ibuf/(double)h;
            if (r == 1.0) ibuf=0;
            return r;
        }
    }
    ibuf=0;
    return 1.0;
}

void map::add(double plus) {
    size_t i,j;
    if (field != NULL) {
        for (j=0;j<h;j++) {
            for (i=0;i<w;i++) {
                field[i][j] += plus;
            }
        }
    }
}

double map::iadd(double plus) {
    size_t i,j;
    if (field != NULL) {
        for (j=ibuf;j<h;j++) {
            for (i=0;i<w;i++) {
                field[i][j] += plus;
            }
            ibuf++;
            double r = ibuf/(double)h;
            if (r == 1.0) ibuf=0;
            return r;
        }
    }
    ibuf=0;
    return 1.0;
}

double map::value(int x, int y) {
    size_t xx,yy;

    if (x < (-1*w)) x-= (x/w)*w;
    if (y < (-1*h)) y-= (y/h)*h;

    if (x<0) x = w+x;
    if (y<0) y = h+y;

    xx = x % w;
    yy = y % h;

    return field[xx][yy];
}

void map::generate(map * noise, int octs) {
    size_t i,j;
    double div = (octs < 1 ? 1.0 : 1.0*octs);

    noise->set_invert(invert);
    noise->set_marble_period(marble_period);
    noise->set_marble_strength(marble_strength);
    noise->set_texture(texture_type);
    noise->set_threshold(abs_threshold);

    for (j=0;j<h;j++) {
        for (i=0;i<w;i++) {
            field[i][j] = noise->turbulence(i,j,w/div);
        }
    }
}

double map::igenerate(map * noise, int octs) {
    size_t i,j;
    double div = (octs < 1 ? 1.0 : 1.0*octs);

    for (j=ibuf;j<h;j++) {
        noise->set_invert(invert);
        noise->set_marble_period(marble_period);
        noise->set_marble_strength(marble_strength);
        noise->set_texture(texture_type);
        noise->set_threshold(abs_threshold);

        for (i=0;i<w;i++) {
            field[i][j] = noise->turbulence(i,j,RWG_UMIN(w/div,h));
        }
        ibuf++;
        double r = ibuf/(double)h;
        if (r == 1.0) ibuf=0;
        return r;
    }
    ibuf=0;
    return 1.0;
}

double map::imap_multiply(map * arg) {
    size_t i,j;

    for (j=ibuf;j<h;j++) {
        for (i=0;i<w;i++) {
            field[i][j] *= arg->field[i][j];
        }
        ibuf++;
        double r = ibuf/(double)h;
        if (r == 1.0) ibuf=0;
        return r;
    }
    ibuf=0;
    return 1.0;
}

double map::imix(map * relief, map *rocks, double min, double blend) {
    for (size_t j=ibuf; j<h; j++) {
        for (size_t i=0; i<w; i++) {
            double f = field[i][j];

            f = pow(-(-f+1.0),3)+1.0;
            f = pow(f,8);

            if (field[i][j] > 0.5) {
                field[i][j]= (0.5 + 0.5 + field[i][j])/3.0;
            }
            //double f = field[i][j];

            if (relief->field[i][j] > min) {
                double s = RWG_URANGE(0.0, relief->field[i][j] - min, blend);

                s *= (1.0 / blend);
                s *= s*s*f;

                double rock = rocks->field[i][j];
                field[i][j] = (1.0-s)*field[i][j] + s*rock;
            }

            if (field[i][j] > 0.8) {
                field[i][j]= 0.25 + 0.75*field[i][j];
            }

            //double f_x = (pow(2.0*field[i][j]-1.0),3)/2.0 +0.5;

            //field[i][j] = f_x; // ( f_x + field[i][j] )/2.0;
        }
        ibuf++;
        double r = ibuf/(double)h;
        if (r == 1.0) ibuf=0;
        return r;
    }
    ibuf=0;
    return 1.0;
}

double map::turbulence(double x, double y, double size) {
    std::vector<double> turbs;
    double valuex = 0.0;

    double strength = 0.5;
    double total    = 0.0;

    double turb = 0.0;
    while(size >= 1.0) {
        if (smooth_type == RWG_SMOOTH_BICUBIC)
             turb = smooth_bicubic (x / size, y / size, (int)(w/size),(int)(h/size));
        else turb = smooth_bilinear(x / size, y / size, (int)(w/size),(int)(h/size));
        size /= 2.0;

        valuex  += turb*strength;

        total   += strength;
        strength/=2.0;

        turbs.push_back(turb);
    }
    /*
    // First in turbs has the greatest effect
    size_t sz = turbs.size();

    for (size_t i=0; i<sz; ++i) {
        valuex     +=turbs[i]*strength;
        total      +=strength;
        strength/=2.0;
    }
    */

    double mul = 1.0 / total;
    double v = valuex*mul;

    /*double dong   = 0.0;

    while(size >= 1.0) {
        if (smooth_type == RWG_SMOOTH_BICUBIC)
             valuex += smooth_bicubic (x / size, y / size, (int)(w/size),(int)(h/size)) * size;
        else valuex += smooth_bilinear(x / size, y / size, (int)(w/size),(int)(h/size)) * size;

        dong += size;
        size /= 2.0;
    }
    double v = valuex / dong;*/

    // FLAME LIKE:
    if (texture_type == RWG_TEXTURE_FLAMES
    ||  texture_type == RWG_TEXTURE_MARBLE) {
        double level  = abs_threshold;
        bool   invert = this->invert;

        level = RWG_URANGE(0.001,level,0.999);

        v -= level; if (v<0.0) v*=-1.0;
        double abs_level = ( level < 0.5 ? (1.0 - level) : level );
        double mod = 1.0 / abs_level;
        v *= mod;

        // MARBLE:
        if (texture_type == RWG_TEXTURE_MARBLE) {
            double m1 = (sin( (x/(((double)w/marble_period) )*RWG_PI) + v) + 1.0)/2.0;
            double m2 = (cos( (y/(((double)h/marble_period) )*RWG_PI) + v) + 1.0)/2.0;
            v = (marble_strength)*((m1+m2)/2.0)+(1.0-marble_strength)*v;
        }
        // END OF MARBLE

        if (invert) v = (1.0 - v);

        // END OF FLAME LIKE
    }

    return RWG_URANGE(0.0,v,1.0);
}

double map::smooth_bilinear(double x, double y, int zoom_w, int zoom_h) {
   double **noise     = field;
   //get fractional part of x and y
   double fractX = x - int(x);
   double fractY = y - int(y);

   if (zoom_w==0) zoom_w=1;
   if (zoom_h==0) zoom_h=1;

   if (zoom_w>(int)w) zoom_w = (int)w;
   if (zoom_h>(int)h) zoom_h = (int)h;

   int ww = (int)zoom_w;
   int hh = (int)zoom_h;

   //wrap around
   int x1 = (int(x) + ww) % ww;
   int y1 = (int(y) + hh) % hh;

   //neighbor values
   int x2 = (x1 + ww - 1) % ww;
   int y2 = (y1 + hh - 1) % hh;

   //smooth the noise with bilinear interpolation
   double value = 0.0;

   if (x1==0) x1 = zoom_w-1;
   if (x2==0) x2 = zoom_w-1;
   if (y1==0) y1 = zoom_h-1;
   if (y2==0) y2 = zoom_h-1;

   value += fractX       * fractY       * noise[x1][y1];
   value += fractX       * (1 - fractY) * noise[x1][y2];
   value += (1 - fractX) * fractY       * noise[x2][y1];
   value += (1 - fractX) * (1 - fractY) * noise[x2][y2];

   if (value<  0.0) value = 0.0;
   if (value>  1.0) value = 1.0;

   return value;
}

double map::smooth_bicubic(double x, double y, int zoom_w, int zoom_h) {
    double **noise = field;
    int i;
    double v0,v1,v2,v3;
    double v[4];
    int vx[4];
    int vy[4];

    int X = (int)x;
    int Y = (int)y;

    vx[0] = X-1; vy[0] = Y-1;
    vx[1] = X;   vy[1] = Y;
    vx[2] = X+1; vy[2] = Y+1;
    vx[3] = X+2; vy[3] = Y+2;

    for (i=0;i<4;i++) {
        if (vx[i]< 0) vx[i]-=(zoom_w*(vx[i]/zoom_w));
        if (vx[i]< 0) vx[i] = zoom_w + vx[i];
        if (vx[i]< 0) vx[i] = 0;
        if (vx[i]>= zoom_w) vx[i] =vx[i]%zoom_w;

        if (vy[i]< 0) vy[i]-=(zoom_h*(vy[i]/zoom_h));
        if (vy[i]< 0) vy[i] = zoom_h + vy[i];
        if (vy[i]< 0) vy[i] = 0;
        if (vy[i]>= zoom_h) vy[i] =vy[i]%zoom_h;
    }

    for (i=0;i<4;i++) {
        //ZOOM LEVEL TILES:
        v0 = noise[vx[0]][vy[i]];
        v1 = noise[vx[1]][vy[i]];
        v2 = noise[vx[2]][vy[i]];
        v3 = noise[vx[3]][vy[i]];

        v[i] = cubic_interpolate(v0,v1,v2,v3,x-1.0*X);
    }

    double value = cubic_interpolate(v[0],v[1],v[2],v[3],y-1.0*Y);

    if (value < 0.0) value = 0.0;
    if (value > 1.0) value = 1.0;

    return value;
}
