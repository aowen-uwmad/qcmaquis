/*
 6*  vli.h
 *  vli_cpu_gpu
 *
 *  Created by Tim Ewart on 18.03.11.
 *  Copyright 2011 University of Geneva. All rights reserved.
 *
 */


#ifndef VLI_NUMBER_CPU_HPP
#define VLI_NUMBER_CPU_HPP
#include "vli/detail/bit_masks.hpp"
#include "vli/function_hooks/vli_number_cpu_function_hooks.hpp"
#include <boost/lexical_cast.hpp>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <ostream>
#include <sstream>
#include <boost/swap.hpp>


namespace vli{
    
    template<typename BaseInt, std::size_t Size>
    void swap(vli_cpu<BaseInt,Size>& a, vli_cpu<BaseInt,Size>& b){
        boost::swap(a.data_,b.data_);
    }
    
    template<class BaseInt, std::size_t Size>
    class vli_cpu 
    {
    public:
        typedef BaseInt         value_type;     // Data type to store parts of the very long integer (usually int) -
        typedef std::size_t     size_type;      // Size type of the very long integers (number of parts)
        enum {size = Size};                     // Number of parts of the very long integer (eg. how many ints)
        // c - constructors, copy-swap, access   
        vli_cpu();
        explicit vli_cpu(int num);
        vli_cpu(vli_cpu const& r);
#if defined __GNU_MP_VERSION
        // TODO find a better solution for this.
        operator mpz_class() const;
        operator mpq_class() const;
#endif //__GNU_MP_VERSION
        friend void swap<> (vli_cpu& a, vli_cpu& b);
        vli_cpu& operator= (vli_cpu r);
        BaseInt& operator[](size_type i);
        const BaseInt& operator[](size_type i) const;
        // c - negative number
        void negate();
        bool is_negative() const;
        // c - basic operator
        vli_cpu& operator += (vli_cpu const& vli);        
        vli_cpu& operator += (BaseInt const a);
        vli_cpu& operator -= (vli_cpu const& vli);
        vli_cpu& operator -= (BaseInt a);
        vli_cpu& operator *= (BaseInt a); // 192 bits -> 192 bits
        vli_cpu& operator *= (vli_cpu const& a); // 192 bits -> 192 bits
        
        vli_cpu operator -() const;
        bool operator == (vli_cpu const& vli) const;
        bool operator != (vli_cpu const& vli) const;
        bool operator < (vli_cpu const& vli) const;
        bool operator < (int i) const;
        bool operator > (vli_cpu vli) const;
        bool operator > (int i) const;
        bool is_zero() const;
        void print_raw(std::ostream& os) const;
        void print(std::ostream& os) const;
        
        std::string get_str() const;
        size_type order_of_magnitude_base10(vli_cpu<BaseInt,size> const& value) const;
        std::string get_str_helper_inplace(vli_cpu<BaseInt,size>& value, size_type ten_exp) const;
    private:
        BaseInt data_[Size];
    };

    template <class BaseInt, std::size_t Size>
    class extend_helper {

    };
/*
c = extend(a) + b;
vli c = 5;
c = plus_extend(a,b);

vli c = 5;
plus_extend_inplace(c,a,b);
*/
    
    /**
     multiply and addition operators, suite ...
     */
    template <class BaseInt, std::size_t Size>
    bool is_zero(vli_cpu<BaseInt, Size> const& v);
    
    template <class BaseInt, std::size_t Size>
    void negate_inplace(vli_cpu<BaseInt, Size>& v);

    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator + (vli_cpu<BaseInt, Size> vli_a, vli_cpu<BaseInt, Size> const& vli_b);

    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator + (vli_cpu<BaseInt, Size> vli_a, int b);
    
    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator + (int b, vli_cpu<BaseInt, Size> const& vli_a);
    
    template <class BaseInt, std::size_t Size> //extented arithmetic
    const vli_cpu<BaseInt, Size+1> plus_extend(vli_cpu<BaseInt, Size> const& vli_a, vli_cpu<BaseInt, Size> const& vli_b);

    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator - (vli_cpu<BaseInt, Size> vli_a, vli_cpu<BaseInt, Size> const& vli_b);
    
    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator - (vli_cpu<BaseInt, Size> vli_a, int b);
    
    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator * (vli_cpu<BaseInt, Size>  vli_a, vli_cpu<BaseInt, Size> const& vli_b);

    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator * (vli_cpu<BaseInt, Size> vli_a, int b);

    template <class BaseInt, std::size_t Size>
    const vli_cpu<BaseInt, Size> operator * (int b, vli_cpu<BaseInt, Size> const& a);

    template <class BaseInt, std::size_t Size>
    void mul(vli_cpu<BaseInt, 2*Size>& vli_res, vli_cpu<BaseInt, Size> const&  vli_a, vli_cpu<BaseInt, Size> const& vli_b); // C nt = non truncated

    template <class BaseInt, std::size_t Size>
    void muladd(vli_cpu<BaseInt, 2*Size>& vli_res, vli_cpu<BaseInt, Size> const&  vli_a, vli_cpu<BaseInt, Size> const& vli_b); // C
    /**
    stream 
    */
    template<typename BaseInt, std::size_t Size>
    std::ostream& operator<< (std::ostream& os,  vli_cpu<BaseInt, Size> const& vli);
}

#include "vli/vli_cpu.hpp"

#endif //VLI_NUMBER_CPU_HPP
