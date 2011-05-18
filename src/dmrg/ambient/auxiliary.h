#ifndef AMBIENT_AUX_H
#define AMBIENT_AUX_H

#include "ambient/core/layout.h"
#include "boost/shared_ptr.hpp"
#include <vector>

#define HASH_MAP_PARTITION_BIT_SIZE 8
#define HASH_MAP_PARTITION_SIZE 256 // 2^HAH_MAP_PARTITION_BIT_SIZE
#define HASH_MAP_VECTOR_RESERVATION 10
#define STACK_CONTENT_RESERVATION 10

#define __a_ceil(x) (((double)x-(int)x) == 0 ? (int)x : (int)x+1)


namespace ambient{
namespace core{ class operation; }

    class access_marker
    {
    private:
        access_marker();                                  // constructor is private
        access_marker(access_marker const&);              // copy constructor is private
        access_marker& operator=(access_marker const&);   // assignment operator is private
    public:
        static access_marker& instance();
        void write_only_mark();
        bool write_only_marked();
    private:
        bool write_only;
    };

    enum policy 
    {
      ANY     ,  // used for stack allocs, makes replicas in heap
      REPLICA ,  // replica created from any (usual for stack vars)
      MANUAL  ,  // manual deallocation, handling (heap variables)
      WEAK       // weak userspace deallocs (Ambient will free asap)
    };

    class dim2
    {
    public:
        unsigned int x, y;
        dim2(unsigned int x = 1, unsigned int y = 1) : x(x), y(y) {}
        dim2& operator=(int value){
            this->x = this->y = value;
            return *this;
        }
        dim2& operator*=(const dim2 & b){
            this->x *= b.x;
            this->y *= b.y;
            return *this;
        }
        unsigned int operator*(const dim2 & b) const { // multiplication of all components
            return this->x * b.x *
                   this->y * b.y ;
        }
        bool operator==(int value) const {
            return (x == value && y == value);
        }
        bool operator==(dim2 m) const {
            return (x == m.x && y == m.y);
        }
        bool operator!=(dim2 m) const {
            return !(x == m.x && y == m.y);
        }
    };

    class hash_map {
    private: 
        hash_map();                             // constructor is private
        hash_map(hash_map const&);              // copy constructor is private
        hash_map& operator=(hash_map const&);   // assignment operator is private
    public:
        static hash_map& instance();
    public:
        unsigned int insert(unsigned int* hash, unsigned int hash_len, core::layout_table* value, int shift = 0);
        core::layout_table* find(unsigned int* hash, unsigned int hash_len, unsigned int id, int shift = 0) const;
    private:
        std::vector< std::pair<hash_map*,std::vector<core::layout_table*>* > > content;
    };

    template<typename T>
    class one_touch_stack {
    public:
        one_touch_stack();
        void push_back(T element);
        bool end_reached();
        bool alt_end_reached();
        T* pick();
        T* alt_pick();
        void reset();
        void alt_reset();
        void clean();
        bool empty();
    private:
        T* content;
        size_t write_iterator; 
        size_t read_iterator;
        size_t alt_read_iterator;
        size_t length;
        size_t reserved;
    };

    class delegate {
    public:
        delegate();
        void operator+=(core::operation* handler);
        void operator()();
    private:
        core::operation** handlers;
        size_t length;
        size_t reserved;
    };
}

#endif
