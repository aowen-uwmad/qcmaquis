#ifndef AMBIENT_CORE_SMP_H
#define AMBIENT_CORE_SMP_H

#include "ambient/core/operation/operation.h"
#include "ambient/core/p_object.h"
#include "ambient/core/layout.h"

namespace ambient {

    class scope_context { // scalable multiprocessor
    private:
        scope_context();
        scope_context(scope_context const&);
        scope_context& operator=(scope_context const&);
    public:
        static scope_context& instance();

    public:
// proxy functionality //
        scope_context& operator()(const int rank);
        void reset_group();
        void set_group(groups::group* grp);
        groups::group* get_group();
        void set_op(core::operation* op);
        core::operation* get_op();
    private:
        groups::group* grp;
        core::operation* op;
// proxy functionality //
// group class method duplicates
    public:
	int np,nq; //mask of the two cyclic distribution
        int get_master_g();
        groups::packet_manager* get_manager();
        void spin();
        void spin_loop();
        int get_rank();
        int get_size();
        const char* get_name();
        bool involved();
        bool is_master();
    };

}
#endif
