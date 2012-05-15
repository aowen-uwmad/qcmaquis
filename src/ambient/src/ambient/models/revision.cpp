#include "ambient/ambient.h"
#include "ambient/models/v_model.h"

namespace ambient { namespace models {

    void null_i(models::v_model::object& a){
        dim2 idx = ctxt.get_block_id();
        dim2 dim = a.revision(0).get_layout().get_mem_dim();
        memset((double*)a.revision(0)(idx.y,idx.x), 0, dim.y*dim.x*a.get_t_size());
    }

    v_model::revision::revision(v_model::object* o, v_model::layout* l)
    : object(o), layout(l), reduction(NULL), placement(NULL), generator(NULL)
    {
        this->number = o->get_revision_base(); // debug
        this->layout->set_revision(this);
    };

    v_model::revision::~revision(){
        delete this->layout;
    }

    void v_model::revision::set_dim(dim2 dim){
        this->layout->set_dim(dim);
    }

    dim2 v_model::revision::get_dim(){
        return this->layout->get_dim();
    }

    std::pair<size_t*,size_t> v_model::revision::id(){
        return this->layout->id();
    }

    v_model::object& v_model::revision::get_object(){
        return *this->object;
    }

    v_model::layout& v_model::revision::get_layout(){
        return *this->layout;
    }

    v_model::layout::entry* v_model::revision::block(size_t i, size_t j){
        return this->layout->get(i, j);
    }

    v_model::layout::entry& v_model::revision::operator()(size_t i, size_t j){
        return controller.ufetch_block(*this, i, j);
    }

    void v_model::revision::add_modifier(models::v_model::modifier* m){
        this->modifiers.push_back(m);
    }

    std::list<v_model::modifier*>& v_model::revision::get_modifiers(){
        return this->modifiers;
    }

    channels::group* v_model::revision::get_placement(){
        return this->placement;
    }

    void v_model::revision::set_placement(channels::group* grp){
        this->placement = grp;
    }

    void v_model::revision::set_generator(v_model::modifier* m){
        this->generator = m;
    }

    v_model::modifier* v_model::revision::get_generator(){
        return this->generator;
    }

    v_model::reduction* v_model::revision::get_reduction(){
         return this->reduction;
    }

    void v_model::revision::set_reduction(){
         this->reduction = new v_model::reduction(this);
    }

} }
