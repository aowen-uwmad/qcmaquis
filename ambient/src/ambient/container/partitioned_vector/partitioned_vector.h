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

#ifndef AMBIENT_CONTAINER_PARTITIONED_VECTOR_PARTITIONED_VECTOR_H
#define AMBIENT_CONTAINER_PARTITIONED_VECTOR_PARTITIONED_VECTOR_H

#include "ambient/container/iterator/associated_iterator.hpp">

namespace ambient {

    template <class Vector, int IB = AMBIENT_DEFAULT_IB>
    class partitioned_vector : public ambient::memory::use_fixed_new<partitioned_vector<Vector,IB> > {
    public:
        typedef typename Vector::value_type value_type;
        typedef typename Vector::size_type size_type;
        typedef typename Vector::difference_type difference_type;
        typedef typename Vector::allocator_type allocator_type;
        typedef Vector partition_type;
        typedef associated_iterator<partitioned_vector<Vector,IB> > iterator;
        typedef associated_iterator<const partitioned_vector<Vector,IB> > const_iterator;
        static constexpr int ib = IB;

       ~partitioned_vector();
        explicit partitioned_vector();
        explicit partitioned_vector(Vector* a);
        explicit partitioned_vector(size_type length, value_type init_value = value_type());
        partitioned_vector(const partitioned_vector& a);
        partitioned_vector& operator = (const partitioned_vector& rhs);
        template <class OtherVector, int OtherIB> partitioned_vector& operator = (const partitioned_vector<OtherVector,OtherIB>& rhs);
        size_type size() const;
        bool empty() const;
        void swap(partitioned_vector& r);
        void resize(size_type n); 
        Vector& partition(size_type k);
        const Vector& partition(size_type k) const;
        Vector& locate(size_type i);
        const Vector& locate(size_type i) const;
        value_type& operator[] (size_type i);
        const value_type& operator[] (size_type i) const;
        size_t addr(size_type i) const;
        iterator begin();
        iterator end();
        const_iterator cbegin() const;
        const_iterator cend() const;
        void normalize();
    public:
        std::vector<Vector*> data;
        size_type length;
        size_type nt;
    };

}

#endif
