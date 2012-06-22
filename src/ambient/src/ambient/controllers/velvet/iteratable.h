#ifndef AMBIENT_CONTROLLERS_VELVET_ITERATABLE
#define AMBIENT_CONTROLLERS_VELVET_ITERATABLE

extern pthread_key_t pthread_tid;
#define GET_TID 0 //*(size_t*)pthread_getspecific(pthread_tid)

namespace ambient { namespace controllers { namespace velvet {

    using ambient::models::velvet::revision;

    struct revision_sub {
        inline revision* get_parent();
    };

    struct c_revision : public revision_sub { inline revision::entry& operator()(size_t, size_t); }; // check
    struct w_revision : public revision_sub { inline revision::entry& operator()(size_t, size_t); }; // weak
    struct p_revision : public revision_sub { inline revision::entry& operator()(size_t, size_t); }; // purge
    struct r_revision : public revision_sub { inline revision::entry& operator()(size_t, size_t); }; // reuse

    template<class T>
    class iteratable : public T
    {
    protected:
        inline iteratable(dim2);
    public:
        inline revision&   ui_l_revision_0() const { return *this->content[this->thread_revision_base[GET_TID]];                  }
        inline c_revision& ui_c_revision_0() const { return *(c_revision*)this->content[this->thread_revision_base[GET_TID]];     }
        inline w_revision& ui_w_revision_1() const { return *(w_revision*)this->content[this->thread_revision_base[GET_TID] + 1]; }
        inline p_revision& ui_p_revision_1() const { return *(p_revision*)this->content[this->thread_revision_base[GET_TID] + 1]; }
        inline r_revision& ui_r_revision_1() const { return *(r_revision*)this->content[this->thread_revision_base[GET_TID] + 1]; }
        inline size_t get_thread_revision_base() const;
        inline void set_thread_revision_base(size_t);
        size_t thread_revision_base[NUM_THREADS];
    };

} } }

#endif
