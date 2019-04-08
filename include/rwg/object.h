
class object {
    friend class world;

    public:

    virtual ~object();
    object();

    virtual class droplet  *to_droplet (void) {return NULL;}
    virtual class task     *to_task    (void) {return NULL;}
    virtual class river    *to_river   (void) {return NULL;}
    virtual class lake     *to_lake    (void) {return NULL;}
    virtual class landmark *to_landmark(void) {return NULL;}

    bool is_broken(void) {return broken;}
    void paralyze(void)  {broken = true;}

    int get_id(void)      { return id;}
    int get_type(void)    { return type;}
    int give_new_id(void) { return (id = next_id++);}

    size_t x;
    size_t y;
    class world * in_world;
    int type;

    class object *next;
    class object *prev;

    virtual void clear  (void);
    virtual void tidy   (void);
    virtual bool create (void);

    protected:
    virtual bool destroy(void);
    virtual void update (void);

    int id;

    private:
    static int next_id;

    bool broken;
};

