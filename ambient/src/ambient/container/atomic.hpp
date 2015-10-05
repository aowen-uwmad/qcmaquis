/*
 * Copyright Institute for Theoretical Physics, ETH Zurich 2015.
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

#ifndef AMBIENT_CONTAINER_ATOMIC_HPP
#define AMBIENT_CONTAINER_ATOMIC_HPP

#include "utils/static_bind.hpp"

namespace ambient {

    // atomic: single value of T that has versioning symantics
    // note: do not use it unless you know what you are doing

    template <class T, class Allocator = default_allocator>
    class atomic {
    public:
        typedef Allocator allocator_type;
        typedef T value_type;
        typedef size_t size_type;
        typedef size_t difference_type;

        explicit atomic(T value = T());
        atomic(const atomic& a);
        atomic& operator = (const atomic& rhs);
        template<class OtherAllocator>
        atomic& operator = (const atomic<T,OtherAllocator>& rhs);

        void init(T value);
        void swap(atomic<T,Allocator>& r);
        void load() const;

        value_type get() const;
        void set(value_type value);
    public:
    AMBIENT_DELEGATE(
        value_type value;
    )};

    namespace detail {
        template<class T, class Allocator, class OtherAllocator = Allocator>
        void copy_atomic(volatile ambient::atomic<T,Allocator>& dst, const ambient::atomic<T,OtherAllocator>& src){
            const_cast<ambient::atomic<T,Allocator>&>(dst).set(src.get());
        }
        template<class T, class Allocator>
        void init_value_atomic(volatile ambient::atomic<T,Allocator>& a, T value){
            const_cast<ambient::atomic<T,Allocator>&>(a).set(value);
        }
    }

    AMBIENT_STATIC_BIND_CPU_TEMPLATE(detail::init_value_atomic, init_value_atomic)
    AMBIENT_STATIC_BIND_CPU_TEMPLATE(detail::copy_atomic,       copy_atomic)


    template<class T, class Allocator>
    atomic<T,Allocator>::atomic(T value) : ambient_allocator(sizeof(T)) {
        this->init(value);
    }

    template <typename T, class Allocator>
    atomic<T,Allocator>::atomic(const atomic& a) : ambient_allocator(sizeof(T)) {
        ambient::merge(a, *this);
    }
    
    template <typename T, class Allocator>
    atomic<T,Allocator>& atomic<T,Allocator>::operator = (const atomic& rhs){
        atomic c(rhs);
        this->swap(c);
        return *this;
    }

    template <typename T, class Allocator>
    template <class OtherAllocator>
    atomic<T,Allocator>& atomic<T,Allocator>::operator = (const atomic<T,OtherAllocator>& rhs){
        if(ambient::weak(rhs)){
            atomic weak();
            this->swap(weak);
        } else {
            copy_atomic<T,Allocator,OtherAllocator>(*this, rhs);
        }
        return *this;
    }

    template<class T, class Allocator>
    void atomic<T,Allocator>::init(T value){
        if(value == value_type()) return; // 0 by default
        init_value_atomic<T,Allocator>(*this, value);
    }

    template<class T, class Allocator>
    void atomic<T,Allocator>::swap(atomic<T,Allocator>& r){
        ambient::ext::swap(*this, r);
    }

    template<typename T, class Allocator>
    void atomic<T,Allocator>::load() const {
        ambient::load(*this);
    }

    template<class T, class Allocator>
    typename atomic<T,Allocator>::value_type atomic<T,Allocator>::get() const {
        return ambient::delegated(*this).value;
    }

    template<class T, class Allocator>
    void atomic<T,Allocator>::set(typename atomic<T,Allocator>::value_type value){
        ambient::delegated(*this).value = value;
    }

}

#endif
