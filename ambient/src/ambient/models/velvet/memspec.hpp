#define BOUND 30

namespace ambient { namespace models { namespace velvet {

    inline memspec::memspec(size_t size):size(size),clean(false){ }

    inline void* memspec::alloc() const {    
        void* memory = malloc(size + BOUND);
        if(clean) memset((char*)memory + BOUND, 0, size);
        return memory;
    }

    inline void* memspec::calloc() const {    
        void* memory = malloc(size + BOUND);
        memset((char*)memory + BOUND, 0, size);
        return memory;
    }

    inline size_t memspec::get_bound() const {
        return BOUND; // ambient::channel.get_block_packet_type(size).get_bound(9);
    }
 
    void* memspec::get_packet_t() const {
        return &ambient::channel.get_block_packet_type(size);
    }

} } }

#undef BOUND
