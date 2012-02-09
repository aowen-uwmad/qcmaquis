#ifndef AMBIENT_CORE_SMP_H
#define AMBIENT_CORE_SMP_H

#include "ambient/models/operation/operation.h"
#include "ambient/models/v_model.h"
#include "ambient/utils/singleton.hpp"

namespace ambient { namespace controllers {

    class context : public singleton< context > 
    { // scalable multiprocessor
    public:
        context();
    public:
// proxy functionality //
        context& operator()(const int rank);
        void set_group(channels::group* grp);
        channels::group* get_group();
        void set_op(models::imodel::modifier* op);
        models::imodel::modifier* get_op();
    private:
        channels::group* grp;
        models::imodel::modifier* op;
// proxy functionality //
// group class method duplicates
    public:
        enum { MARKUP, EXECUTE } state;
        int np,nq; //mask of the two cyclic distribution
        int get_master_g();
        int get_rank();
        int get_size();
        dim2 get_block_id();
        void set_block_id(dim2);
        const char* get_name();
        bool involved();
        bool is_master();
    };

} }

namespace ambient {
    extern controllers::context& ctxt;
}

#endif
