#ifndef AMBIENT_INTERFACE_CONTROLLER
#define AMBIENT_INTERFACE_CONTROLLER
#include "ambient/models/v_model.h"
#include "ambient/channels/ichannel.h"

namespace ambient { namespace controllers { 

    class icontroller {
    public:
        virtual void acquire(channels::ichannel* channel) = 0;
        virtual models::v_model::layout::entry* alloc_block(models::v_model::revision& r) = 0;
        virtual models::v_model::layout::entry& alloc_block(models::v_model::revision& r, size_t i, size_t j) = 0;
        virtual models::v_model::layout::entry& ufetch_block(models::v_model::revision& r, size_t i, size_t j) = 0;
        virtual models::v_model::layout::entry& ifetch_block(models::v_model::revision& r, size_t i, size_t j) = 0;
        virtual void push(models::v_model::modifier* op) = 0;
        virtual void execute_mod(models::v_model::modifier* op, dim2) = 0;
        virtual void execute_free_mod(models::v_model::modifier* op) = 0;
        virtual void atomic_complete() = 0;
        virtual void atomic_receive(models::v_model::revision& r, size_t i, size_t j) = 0;
        virtual void flush() = 0;
        virtual void set_num_threads(size_t n) = 0;
        virtual size_t get_num_threads() const = 0;

        virtual pthread_mutex_t* get_pool_control_mutex() = 0;
    };
    
    void forward_block(channels::ichannel::packet&);
    void accept_block(channels::ichannel::packet&);
} }

namespace ambient {
    extern controllers::icontroller& controller;
}

#endif
