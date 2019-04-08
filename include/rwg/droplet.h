
class droplet : public object {
    friend class world;
    public:

    droplet();

    class droplet *to_droplet (void) {return this;}

    private:

    bool create (void);
    bool destroy(void);
    void clear  (void);
    void update (void);

    void die(void);

    void erode_map(double **field, int x, int y, size_t w, size_t h, int dir_to, double amount);

    void add_trail(size_t x, size_t y);
    void add_trail(void);

    ~droplet();

    double water;
    double sediments;
    double speed;
    int    direction;

    std::vector<size_t> trail;
};

