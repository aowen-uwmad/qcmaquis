#ifndef AMBIENT_CONTROLLERS_VELVET_CONTROLLER
#define AMBIENT_CONTROLLERS_VELVET_CONTROLLER
#include "ambient/utils/touchstack.h"
#include "ambient/controllers/velvet/cfunctor.h"
#include "ambient/controllers/velvet/iteratable.h"
#include "ambient/controllers/context.h"
#include "ambient/utils/tasklist.hpp"

#ifndef DEFAULT_NUM_THREADS 
#define DEFAULT_NUM_THREADS 1
#endif
#ifndef MAX_NUM_THREADS 
#define MAX_NUM_THREADS 8
#endif
#ifndef NUM_THREADS
#define NUM_THREADS 1 // controller.get_num_threads()
#endif

namespace ambient { namespace controllers { namespace velvet {

    using ambient::models::velvet::revision;
    using ambient::models::velvet::memspec;

    class controller : public singleton< controller >
    {
    public:
        controller();
        static void* stream(void* list);
        inline void   master_stream(void* list); // specialized version for the main thread
        inline void   acquire(channels::mpi::channel* channel);
        inline void   push(cfunctor* op);
        inline void   execute_mod(cfunctor* op, dim2 pin);
        inline void   execute_free_mod(cfunctor* op);

        inline void alloc_block (revision& r, size_t x, size_t y);
        inline void calloc_block(revision& r, size_t x, size_t y);
        inline revision::entry& ufetch_block(revision& r, size_t x, size_t y);
        inline revision::entry& ifetch_block(revision& r, size_t x, size_t y);
        inline bool lock_block  (revision& r, size_t x, size_t y);
        inline void unlock_block(revision& r, size_t x, size_t y);
        inline void unlock_revision(revision* arg);
        inline void unlink_revision(revision* arg);

        inline void flush();
        inline void conditional_flush();
        inline void mute();
        inline void unmute();
        inline void allocate_threads();
        inline void set_num_threads(size_t n);
        inline size_t get_num_threads() const;
        inline void atomic_complete(cfunctor* op);
        inline void atomic_receive(revision& r, size_t x, size_t y);
        inline pthread_mutex_t* get_pool_control_mutex();
        inline ~controller();

    private:
        pthread_mutex_t pool_control_mutex;
        pthread_mutex_t mutex;
        touchstack< cfunctor* > stack;
        pthread_mutex_t mpool[MAX_NUM_THREADS];
        pthread_t pool[MAX_NUM_THREADS];
        tasklist tasks[MAX_NUM_THREADS];
        size_t workload;
        size_t num_threads;
        size_t rrn;
        bool muted;
    };
    
} } }

namespace ambient {
    extern controllers::velvet::controller& controller;
}

#include "ambient/controllers/velvet/controller.hpp"
#include "ambient/controllers/context.hpp"
#include "ambient/controllers/velvet/iteratable.hpp"
#include "ambient/controllers/velvet/cfunctor.hpp"
#endif
