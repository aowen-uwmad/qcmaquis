#include "ambient/core/operation.h"

namespace ambient{ namespace core{

    operation::operation(void(*fp)(void_pt*, void_pt*, void_spt*), void_pt* arg1, void_pt* arg2, void_spt* arg3){
        this->operation_ptr = (void(*)())fp;
        this->arg_count = 3;
        this->arguments = (void_pt**)malloc(sizeof(void_pt*)*this->arg_count);
        this->arguments[0] = arg1;
        this->arguments[1] = arg2;
        this->arguments[2] = (void_pt*)arg3;
        this->prototype = &operation::prototype_triplet;
    }
    void operation::prototype_triplet(){ ((void(*)(void_pt*,void_pt*,void_pt*))this->operation_ptr)(this->arguments[0], this->arguments[1], this->arguments[2]); }
    void operation::perform(){
        for(size_t i=0; i < this->arg_count; i++)
            this->arguments[i] = this->arguments[i]->dereference();
        (this->*prototype)(); 
    }
} }
