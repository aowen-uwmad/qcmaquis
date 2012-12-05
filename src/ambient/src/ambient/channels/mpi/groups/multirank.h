#ifndef AMBIENT_CHANNELS_MPI_GROUPS_MULTIRANK
#define AMBIENT_CHANNELS_MPI_GROUPS_MULTIRANK

namespace ambient { namespace channels { namespace mpi {

    class multirank : public singleton< multirank >
    {
    public:
        multirank() : verbose(true) {}
        int operator()() const;
        int operator()(const group* grp) const;
        int translate(int rank, const group* source) const; // default: world
        int translate(int rank, const group* source, const group* target) const;
        int cast_to_parent(int rank, const group* source, const group* target) const;
        int cast_to_child(int rank, const group* source, const group* target) const;
        bool belongs(const group* target) const;
        bool masters(const group* target) const;
        bool verbose;
    };

} } }

namespace ambient {
    extern channels::mpi::multirank& rank;
}

#endif
