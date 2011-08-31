/*
 *  monome_cpu.h
 *
 *  Created by Tim Ewart (timothee.ewart@unige.ch) and  Andreas Hehn (hehn@phys.ethz.ch) on 18.03.11.
 *  Copyright 2011 University of Geneva and Eidgenössische Technische Hochschule Züric. All rights reserved.
 *
 */

#ifndef VLI_POLYNOME_CPU_H
#define VLI_POLYNOME_CPU_H
#include "boost/swap.hpp"
#include "function_hooks/vli_polynomial_cpu_function_hooks.hpp"

#include "polynomial/monomial.hpp"

#include <ostream>
#include <cmath>

namespace vli
{	    
    
template<class Vli, int Order>
class polynomial_cpu;

/**
 * Multiplication of two polynomial_cpus
 */
template<class Vli, int Order>
polynomial_cpu<Vli, Order> operator * (polynomial_cpu<Vli, Order> const& p1, polynomial_cpu<Vli, Order> const& p2)
{
    polynomial_cpu<Vli, Order> result;
    poly_multiply(result, p1, p2);
    return result;
}


template <class BaseInt, int Size, int Order, class T>
polynomial_cpu<vli_cpu<BaseInt, Size>, Order> operator * (polynomial_cpu<vli_cpu<BaseInt, Size>, Order>  const& p, monomial<T> const& m)
{
    typedef typename polynomial_cpu<vli_cpu<BaseInt,Size>,Order>::size_type size_type;
    
    polynomial_cpu<vli_cpu<BaseInt, Size>, Order> r;
    // TODO perhaps there is a better way to write these loops,
    //      so that they can be unrolled.
    for(size_type je = 0; je < Order-m.j_exp_; ++je)
        for(size_type he = 0; he < Order-m.h_exp_; ++he)
            r(je+m.j_exp_,he+m.h_exp_) = p(je,he) * m.coeff_;
    return r;
}

template<class Vli, int Order>
class polynomial_cpu{
public:
    typedef typename Vli::size_type size_type;      // Type of the exponents (has to be the same type as Vli::size_type)
    typedef Vli value_type;
    enum { max_order = Order};
    
    //   friend polynomial_cpu operator * <>(const polynomial_cpu& p, const monomial<Vli> & m);
    friend void poly_multiply <>(polynomial_cpu& result ,const polynomial_cpu& p1, const polynomial_cpu& p2);
    
    polynomial_cpu(){
        for(int i=0; i<Order*Order;++i)
            coeffs_[i]=Vli();
    }
    
    polynomial_cpu(const polynomial_cpu& p){
        for(int i=0; i<Order*Order;++i)
            coeffs_[i]=p.coeffs_[i];
    }
    
    polynomial_cpu& operator = (polynomial_cpu p)
    {
        swap(*this,p);
        return *this;
    }
    
    /**
     * Plus assign with a polynomial_cpu
     */
    polynomial_cpu& operator += (polynomial_cpu const& p)
    {
        for(int i=0; i<Order*Order;++i)
            coeffs_[i]+=p.coeffs_[i];
        return *this;
    }
    
    template <typename T>
    polynomial_cpu& operator += (monomial<T> const& m)
    {
        operator()(m.j_exp_,m.h_exp_) += m.coeff_; 
        return *this;
    }
    
    template <typename T>
    polynomial_cpu& operator += (T const& t)
    {
        coeffs_[0]+=t;
        return *this;
    }
    
    /**
     * Minus assign with a polynomial_cpu
     */
    polynomial_cpu& operator -= (polynomial_cpu const& p)
    {
        for(int i=0; i<Order*Order;++i)
            coeffs_[i]-=p.coeffs_[i];
        
        return *this;
    }
    
    polynomial_cpu& operator -= (monomial<Vli> const& m)
    {
        operator()(m.j_exp_,m.h_exp_) -= m.coeff_; 
        return *this;
    }

    template <typename T>
    polynomial_cpu& operator -= (T const& t)
    {
        coeffs_[0]-=t;
        return *this;
    }

    bool operator==(polynomial_cpu const& p) const
    {
        int n = memcmp((void*)&coeffs_[0],(void*)&p.coeffs_[0],Order*Order*Vli::size*sizeof(typename Vli::value_type));
        return (0 == n);
    }
    
    friend void swap(polynomial_cpu& p1, polynomial_cpu& p2)
    {
        using boost::swap;
        swap(p1.coeffs_,p2.coeffs_);
    }
    
    /**
     * Multiplies assign with coefficient
     * Mutliplies all elements the argument
     */
    polynomial_cpu& operator *= (Vli const& c)
    {
        for(int i=0; i<Order*Order;++i)
            coeffs_[i] *= c;
        return *this;
    }
    
     /**
     * Multiplies assign with coefficient
     * Mutliplies all elements the argument
     */
     polynomial_cpu& operator *= (int c)
     {
        for(int i=0; i<Order*Order;++i)
            coeffs_[i] *= c;
        return *this;
     }
    
    
    template <typename T> 
    polynomial_cpu& operator *= (monomial<T> const& c)
    {
        // TODO this implementation is wrong!!!
        (*this) *= c.coeff_;
        return *this;
    }
    
    /**
     * Access coefficient of monomial J^j_exp*h^h_exp
     */
    inline Vli const& operator ()(size_type j_exp, size_type h_exp) const
    {
        assert(j_exp < max_order);
        assert(h_exp < max_order);
        return coeffs_[j_exp*max_order+h_exp];
    }
    
    /**
     * Access coefficient of monomial J^j_exp*h^h_exp
     */
    inline Vli& operator ()(size_type j_exp, size_type h_exp)
    {
        assert(j_exp < max_order);
        assert(h_exp < max_order);
        return coeffs_[j_exp*max_order+h_exp];
    }
    
    void print(std::ostream& os) const
    {
        for(std::size_t i = 0; i < Order ; i++){
            for(std::size_t j = 0; j < Order ; j++){
                os << "Coeff (j,h) = " << i <<" "<<j<< std::endl;
                os <<coeffs_[i*Order+j] << std::endl;
            }
        }
    }
     
private:
    Vli coeffs_[Order*Order];
};

/*
 * Multiplication of a monomial with a polynomial_cpu
 */
template<class T, class Vli, int Order>
polynomial_cpu<Vli, Order> operator * (monomial<T> const& m,polynomial_cpu<Vli, Order> const& p)
{
    return p * m;
}

/**
 * Multiplication of a polynomial_cpu with a factor
 */
template<class Vli, int Order>
polynomial_cpu<Vli, Order> operator * (polynomial_cpu<Vli, Order> p, Vli const& c)
{
    p *= c;
    return p;
}

template<class Vli, int Order>
polynomial_cpu<Vli, Order> operator * (polynomial_cpu<Vli, Order> p, int c)
{
    p *= c;
    return p;
}

template<class Vli, int Order>
polynomial_cpu<Vli, Order> operator * (Vli const& c, polynomial_cpu<Vli, Order> const& p)
{
    return p * c;
}

template<class Vli, int Order>
polynomial_cpu<Vli, Order> operator * (int c, polynomial_cpu<Vli, Order> const& p)
{
    return p * c; 
}



template<class Vli, int Order> 
std::ostream& operator<<(std::ostream& os, polynomial_cpu<Vli, Order> const& p){
    p.print(os);
    return os;
}

} //end namespace
#endif
