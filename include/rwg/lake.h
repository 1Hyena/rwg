
class lake : public object {
    friend class world;
    public:

    lake();

    class lake *to_lake (void) {return this;}

    private:

    bool create (void);
    bool destroy(void);
    void clear  (void);
    void update (void);

    ~lake();
};
