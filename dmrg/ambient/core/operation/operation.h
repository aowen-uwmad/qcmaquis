#ifndef AMBIENT_CORE_OPERATION_H
#define AMBIENT_CORE_OPERATION_H
#include <stdlib.h>
#define pinned ambient::core::ambient_pin* , 
#define marked NULL,

#include "ambient/auxiliary.h"

namespace ambient{
    class p_profile; 
    namespace groups { class group; }
}
namespace ambient{ namespace core{

    class ambient_pin{};

    class operation{
    public:
        #include "ambient/core/operation/operation.pp.h"
        void init();     // some init that doesn't depend upon arguments
        void perform();  // executes operation
        void invoke();   // executes operation (clean way)
        void set_scope(groups::group* scope);
        groups::group* get_scope();
        void extract_profiles();
        void preprocess();
        void finalize();
        void add_dependant(operation* dep);
        void resolve_dependencies();

        bool executed;
        one_touch_stack<operation*>* dependants;
        size_t dependency_count;
        void(operation::*extract)();
        void(operation::*prototype)();
        void(*operation_ptr)();
        groups::group* scope;
        bool* constness;
        void** arguments;
        p_profile** profiles;
        p_profile* pin;
        size_t count;
    };

    class out_of_scope_e{};

} }

#endif
