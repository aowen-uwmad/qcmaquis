#ifndef AMBIENT_CONTROLLERS_VELVET_CONTROLLER
#define AMBIENT_CONTROLLERS_VELVET_CONTROLLER

#include "ambient/utils/touchstack.h"
#include "ambient/controllers/velvet/cfunctor.h"
#include "ambient/controllers/velvet/context.h"
#include "ambient/controllers/velvet/iteratable.h"
#include "ambient/utils/tasklist.hpp"
#include "ambient/utils/collector.hpp"

#include <cilk/cilk.h>

namespace ambient { namespace controllers { namespace velvet {

    using ambient::models::velvet::history;
    using ambient::models::velvet::revision;
    using ambient::models::velvet::memspec;

    class controller : public singleton< controller >
    {
    public:
        controller();
        inline void   acquire(channels::mpi::channel* channel);
        inline void   schedule(cfunctor* op);

        inline void alloc (revision& r);
        inline void calloc(revision& r);
        inline revision& ufetch(revision& r);
        inline void ifetch(revision& r);
        inline void unlock_revision(revision* arg);
        inline void unlink_revision(revision* arg);

        template<typename T> inline void destroy(T* o);

        inline void flush();
        inline void atomic_receive(revision& r);
        inline ~controller();
    public:
        bool muted;
        collector garbage;
    private:
        touchstack< cfunctor* > chains;
        touchstack< cfunctor* > mirror;
        int arity;
    };
    
} } }

namespace ambient {
    extern controllers::velvet::controller& controller;
}

#include "ambient/controllers/velvet/controller.hpp"
#include "ambient/controllers/velvet/context.hpp"
#include "ambient/controllers/velvet/iteratable.hpp"
#include "ambient/controllers/velvet/cfunctor.hpp"
#endif
