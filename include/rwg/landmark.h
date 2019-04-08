
class landmark : public object {
    friend class world;
    public:

    landmark();

    class landmark *to_landmark (void) {return this;}

    private:

    bool create (void);
    bool destroy(void);
    void clear  (void);
    void update (void);

    ~landmark();
};
