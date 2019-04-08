

class map {
    public:

    map(size_t w, size_t h);
    ~map();

    size_t get_w   (void) {return w;}
    size_t get_h   (void) {return h;}

    void   set_smooth         (int type)    {smooth_type = type;}
    int    get_smooth         (void)        {return smooth_type;}
    void   set_texture        (int texture) {texture_type = texture;}
    int    get_texture        (void)        {return texture_type;}
    void   set_invert         (bool invert) {this->invert = invert;}
    bool   get_invert         (void)        {return invert;}
    void   set_threshold      (double t)    {abs_threshold = t;}
    double get_threshold      (void)        {return abs_threshold;}
    void   set_marble_strength(double s)    {marble_strength = s;}
    double get_marble_strength(void)        {return marble_strength;}
    void   set_marble_period  (double p)    {marble_period = p;}
    double get_marble_period  (void)        {return marble_period;}

    void   init(void);
    void   scramble(void);
    void   clear(void);
    double value(int x, int y);
    void   generate(class map * target, int octs);
    void   multiply(double multiplier);
    void   add     (double plus);

    // iterative versions:
    double iscramble(void);
    double igenerate(class map * target, int octs);
    double imultiply(double multiplier);
    double imap_multiply(class map * arg);
    double iadd     (double plus);
    double imix     (class map * relief, class map *rocks, double min, double blend);

    double **field;

    private:

    double turbulence     (double x, double y, double size);
    double smooth_bilinear(double x, double y, int zoom_w, int zoom_h);
    double smooth_bicubic (double x, double y, int zoom_w, int zoom_h);

    size_t w;
    size_t h;
    size_t ibuf;
    //unsigned int seed;

    // Texture genration parameters:
    int smooth_type;
    int texture_type;
    bool invert;
    double abs_threshold;
    double marble_period;
    double marble_strength;
};
