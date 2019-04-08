

class world {
    public:

    world(size_t w, size_t h);
    ~world();

    size_t get_w   (void) {return w;}
    size_t get_h   (void) {return h;}

    double       get_speed(void)             {return speed;}
    void         set_speed(double speed)     {this->speed = speed;}
    double       get_ocean(void)             {return ocean_level;}
    void         set_ocean(double level)     {this->ocean_level = level;}
    double       get_age  (void)             {return age;}
    int get_state(void) {return state;}
    int get_level(void) {return level;}

    int get_biome(int x, int y);

    bool init(void);
    void clear(void);
    void generate(int iterations);

    int sector_type(int x, int y);

    rwg::map *get_elevation  (void) {return elevation;}
    rwg::map *get_density    (void) {return density;}
    rwg::map *get_rainfall   (void) {return rainfall;}
    rwg::map *get_drainage   (void) {return drainage;}
    rwg::map *get_temperature(void) {return temperature;}
    rwg::map *get_rivers     (void) {return rivers;}
    rwg::map *get_coastline  (void) {return coastline;}

    rwg::map *get_relief     (void) {return relief;}
    rwg::map *get_rocks      (void) {return rocks;}
    rwg::map *get_noise      (void) {return noise;}
    rwg::map *get_tectonics  (void) {return tectonics;}

    rwg::river   * create_river(int id);

    void get_droplets(std::vector<rwg::droplet *> *drops);

    void remove_object(int id, int x, int y);
    void add_object   (int id, int x, int y);

    std::string current_job;
    double      current_done;
    int         current_layer;

    private:

    void step(void);
    void tidy(void);

    void step_init  (void);
    void step_meta  (void);
    void step_polish(void);
    void step_final (void);

    rwg::droplet * create_droplet(size_t x, size_t y);
    rwg::task    * create_task(void);

    rwg::river   * check_tributary(rwg::droplet *drop);

    class rwg::map *elevation;
    class rwg::map *tectonics;
    class rwg::map *density;
    class rwg::map *rainfall;
    class rwg::map *drainage;
    class rwg::map *temperature;
    class rwg::map *rivers;
    class rwg::map *coastline;

    // Helper maps:
    class rwg::map *relief;
    class rwg::map *rocks;
    class rwg::map *noise;

    std::vector<int> objects;

    size_t w;
    size_t h;
    //unsigned int seed;
    double speed;

    double age;
    int tidy_timer;

    double ocean_level;
    int state;
    int level;

    size_t ibuf;
};
