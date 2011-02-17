#include "ambient/ambient.h"
#include "ambient/core/operation.h"

namespace ambient{ namespace core{

    operation::operation(void(*fp)(void_pt&, void_pt&, void_spt&), void_pt* arg1, void_pt* arg2, void_spt* arg3):scope(NULL){
        this->operation_ptr = (void(*)())fp;
        this->arg_count = 3;
        this->arguments = (void_pt**)malloc(sizeof(void_pt*)*this->arg_count);
        this->arguments[0] = arg1;
        this->arguments[1] = arg2;
        this->arguments[2] = (void_pt*)arg3;
        this->pin = arg3;
        this->prototype = &operation::prototype_triplet;
    }
    void operation::prototype_triplet(){ ((void(*)(void_pt&,void_pt&,void_pt&))this->operation_ptr)(*this->arguments[0], *this->arguments[1], *this->arguments[2]); }
    void operation::perform()
    {
        for(size_t i=0; i < this->arg_count; i++)
            this->arguments[i] = this->arguments[i]->dereference();
        this->pin = this->pin->dereference();
        asmp.op = this;
        asmp.set_scope((groups::group*)NULL);
        (this->*prototype)();
        if(!asmp.compute_mode){
            for(size_t i=0; i < this->arg_count; i++)
                this->arguments[i]->postprocess();
        }
    }
    void operation::set_ids()
    {
        for(size_t i=0; i < this->arg_count; i++){
            if(this->arguments[i]->id == 0)
                this->arguments[i]->set_id(ambient::asmp.get_scope()->id);
        }
    }
    void operation::set_scope(groups::group* scope)
    {
        this->scope = scope;
    }
    groups::group* operation::get_scope()
    {
        return this->scope;
    }
} }
