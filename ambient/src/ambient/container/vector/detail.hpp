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

#ifndef AMBIENT_CONTAINER_VECTOR_DETAIL_HPP
#define AMBIENT_CONTAINER_VECTOR_DETAIL_HPP

namespace ambient {
     
    template<class T, class Allocator> class vector;
    namespace detail {
        template<typename T, typename Allocator>
        void set_size(ambient::vector<T,Allocator>& a, const size_t& size){
            a.resize(size);
        }
        template<typename T, typename Allocator>
        void measure_size(const ambient::vector<T,Allocator>& a, ambient::future<size_t>& size){
            size.set(a.size());
        }
        template<class T, class Allocator>
        void init_value_vector(unbound< ambient::vector<T,Allocator> >& a, T& value){
            a.resize(a.cached_size());
            for(size_t i = 0; i < a.size(); ++i) a[i] = value;
        }
        template<class T, class Allocator, class OtherAllocator = Allocator>
        void copy_vector(unbound< ambient::vector<T,Allocator> >& dst, const ambient::vector<T,OtherAllocator>& src, const size_t& n){
            for(size_t i = 0; i < n; ++i) dst[i] = src[i];
        }
        template<typename T, typename Allocator>
        void add(ambient::vector<T,Allocator>& a, const ambient::vector<T,Allocator>& b){
            for(int i = 0; i < a.size(); ++i) a[i] += b[i];
        }
    }

    AMBIENT_EXPORT_TEMPLATE(detail::set_size,          set_size)
    AMBIENT_EXPORT_TEMPLATE(detail::measure_size,      measure_size)
    AMBIENT_EXPORT_TEMPLATE(detail::init_value_vector, init_value_vector)
    AMBIENT_EXPORT_TEMPLATE(detail::copy_vector,       copy_vector)
    AMBIENT_EXPORT_TEMPLATE(detail::add,               add)
}

#endif
