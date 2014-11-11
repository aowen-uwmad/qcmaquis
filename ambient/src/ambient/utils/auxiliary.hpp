/*
 * Copyright Institute for Theoretical Physics, ETH Zurich 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AMBIENT_UTILS_AUXILIARY
#define AMBIENT_UTILS_AUXILIARY

namespace ambient {

    using ambient::models::ssm::revision;

    void sync();
    
    template<typename T> 
    void sync(const T& t){ 
        ambient::sync(); 
    }

    inline bool isset(const char* env){
        return (std::getenv( env ) != NULL);
    }

    inline int getint(const char* env){
        return std::atoi(std::getenv( env ));
    }

    inline int num_procs(){
        return selector.get_controller().get_num_procs();
    }

    inline int get_tag_ub(){
        return selector.get_controller().get_tag_ub();
    }

    inline rank_t rank(){
        return selector.get_controller().get_rank();
    }

    inline bool master(){
        return (rank() == 0);
    }

    inline bool verbose(){ 
        return selector.get_controller().verbose();
    }

    template<typename T>
    inline void destroy(T* o){ 
        selector.get_controller().collect(o); 
    }

    template<typename V>
    inline bool weak(const V& obj){
        return obj.ambient_rc.desc->weak();
    }

    template<typename V>
    inline void merge(const V& src, V& dst){
        assert(dst.ambient_rc.desc->current == NULL);
        if(weak(src)) return;
        revision* r = src.ambient_rc.desc->back();
        dst.ambient_rc.desc->current = r;
        // do not deallocate or reuse
        if(!r->valid() && r->state != ambient::locality::remote){
            assert(r->spec.region != region_t::delegated);
            r->spec.protect();
        }
        assert(!r->valid() || !r->spec.bulked() || ambient::models::ssm::model::remote(r)); // can't rely on bulk memory
        r->spec.crefs++;
    }

    template<typename V>
    inline void swap_with(V& left, V& right){
        std::swap(left.ambient_rc.desc, right.ambient_rc.desc);
        left.ambient_after = left.ambient_rc.desc->current;
        right.ambient_after = right.ambient_rc.desc->current;
    }

    template<typename V>
    inline dim2 get_dim(const V& obj){
        return obj.ambient_rc.desc->dim;
    }

    template<typename V> 
    inline size_t get_square_dim(V& obj){ 
        return get_dim(obj).square();
    }

    template<typename V>
    inline size_t get_length(V& obj){
        return get_dim(obj).y;
    }
    
    template<typename V> 
    inline size_t extent(V& obj){ 
        return obj.ambient_rc.desc->extent;
    }

    template<typename V>
    inline rank_t get_owner(const V& o){
        return o.ambient_rc.desc->current->owner;
    }

    template<typename V>
    inline bool locked_once(const V& o){
        return o.ambient_before->locked_once();
    }

    inline rank_t which(){
        return ambient::selector.get_actor().which();
    }

    inline actor& get_actor(){
        return ambient::selector.get_actor();
    }

}

#endif
