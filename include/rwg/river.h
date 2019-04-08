
class river : public object {
    friend class world;
    public:

    river();

    class river *to_river (void) {return this;}

    private:

    bool create (void);
    bool destroy(void);
    void clear  (void);
    void update (void);

    ~river();

    std::vector<size_t> trail;
    std::vector<int> tributaries;
};
