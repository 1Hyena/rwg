
class task : public object {
    friend class world;
    public:

    task();

    class task *to_task (void) {return this;}

    protected:
    std::map< std::string, class rwg::map * > maps;

    std::string job;
    int arg_int;
    double arg_double;
    double arg_double2;

    double done;

    private:

    bool create (void);
    bool destroy(void);
    void clear  (void);
    void update (void);

    ~task();
};
