#define BOUND 0 //30

namespace ambient { namespace models { namespace velvet {

    inline memspec::memspec(dim2 dim, size_t ts) : dim(dim), size(dim.square()*ts) { }

    inline void* memspec::alloc() const {    
        return ambient::range_pool.malloc(size + BOUND);
    }

    inline void* memspec::calloc() const {    
        void* memory = ambient::range_pool.malloc(size + BOUND);
        memset((char*)memory + BOUND, 0, size);
        return memory;
    }

    inline void memspec::free(void* ptr) const {    
        return ambient::range_pool.free(ptr, size+BOUND);
    }

    inline size_t memspec::get_bound() const {
        return BOUND; // ambient::channel.get_block_packet_type(size).get_bound(9);
    }
 
    void* memspec::get_packet_t() const {
        return &ambient::channel.get_block_packet_type(size);
    }

} } }

#undef BOUND
