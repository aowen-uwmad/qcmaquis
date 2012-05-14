#include "ambient/ambient.h"
#include "ambient/models/v_model.h"
#include "ambient/utils/hashmap.h"

namespace ambient { namespace models {

    v_model::v_model()
    : mem_dim(dim2(16,16)), item_dim(dim2(16,16))
    {
    }

    v_model::~v_model(){
    }

    void v_model::add_revision(imodel::object* obj){
        v_model::layout* l = new v_model::layout(obj->get_dim(), obj->get_t_size());
        size_t size = obj->get_dim().max();
        //if(this->mem_dim < size) 
            l->set_dimensions(this->mem_dim, this->item_dim);
        //else
        //    l->set_dimensions(dim2(size,size), this->item_dim);
        *const_cast<size_t*>(&l->sid) = this->map.insert(l);
        *const_cast<const size_t **>(&l->gid) = channel.id().first;
        obj->add_revision(l);
    }

    void v_model::update_revision(imodel::revision* r, channels::group* placement){

    }

    v_model::revision* v_model::get_revision(size_t* hash, size_t hash_len, size_t id) const {
        return ((layout*)this->map.find(id))->revision;
    }

    v_model& v_model::operator>>(dim2 mem_dim){
        this->mem_dim  = mem_dim;
        this->item_dim = NULL;
        return *this;
    }

    v_model& v_model::operator,(dim2 dim){
        if(this->item_dim == NULL)
            this->item_dim = dim;
        return *this;
    }

    // {{{ free functions for mangling the data //

    void* solidify(const imodel::object& o){
        imodel::revision& r = o.revision(0);
        imodel::layout& l = r.get_layout();
        size_t iterator = 0;
        char* memory = NULL;

        for(size_t j=0; j < l.get_mem_grid_dim().x; j++)
            for(size_t jj=0; jj < l.get_mem_dim().x; jj++)
                for(size_t i=0; i < l.get_mem_grid_dim().y; i++){
                    memory = (char*)realloc(memory, (iterator+1)*l.get_mem_lda());
                    memcpy(memory+iterator*l.get_mem_lda(),                         // copy to
                           &((char*)r(i,j))[jj*l.get_mem_lda()],                    // copy from
                           l.get_mem_lda());                                        // of size
                    iterator++;
                }
        return memory;
    }

    void disperse(void* data, imodel::object& o){
        imodel::revision& current = o.revision(0);
        imodel::revision& updated = o.revision(1);
        imodel::layout& lc = current.get_layout();
        imodel::layout& lu = updated.get_layout();
        size_t iterator = 0;
        char* memory = (char*)data;

        size_t lda = lc.get_mem_dim().y*lc.get_mem_grid_dim().y;
        size_t sda = lc.get_mem_dim().x*lc.get_mem_grid_dim().x;
        size_t t_size = o.get_t_size();

        for(size_t j=0; j < lu.get_mem_grid_dim().x; j++){
            size_t sizex =  std::min(lu.get_mem_dim().x,sda - j*lu.get_mem_dim().x);
            for(size_t jj=0; jj < sizex; jj++)
                for(size_t i=0; i < lu.get_mem_grid_dim().y; i++){
                    size_t sizey = std::min(lu.get_mem_dim().y,lda - i*lu.get_mem_dim().y);
                    memcpy(&((char*)updated(i,j))[jj*lu.get_mem_lda()],  // copy to
                           memory,                                       // copy from
                           sizey*t_size);                                // of size
                    memory += sizey*t_size;
                }
        }
        free(data);
    }

    // }}}

} }
