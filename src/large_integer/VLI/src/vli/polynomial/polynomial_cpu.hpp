/*
 *  monome_cpu.h
 *
 *  Created by Tim Ewart (timothee.ewart@unige.ch) and  Andreas Hehn (hehn@phys.ethz.ch) on 18.03.11.
 *  Copyright 2011 University of Geneva and Eidgenössische Technische Hochschule Züric. All rights reserved.
 *
 */

/**
 * Multiplication of two polynomial_cpus
 */
 
 
template<class Vli, unsigned int Order>
polynomial_cpu<Vli,Order>::polynomial_cpu(){
    for(typename polynomial_cpu<Vli,Order>::exponent_type i=0; i<Order*Order;++i)
        coeffs_[i]=Vli();
}
    
template<class Vli, unsigned int Order>
polynomial_cpu<Vli,Order>::polynomial_cpu(const polynomial_cpu& p){
    for(exponent_type i=0; i<Order*Order;++i)
        coeffs_[i]=p.coeffs_[i];
}
    
template<class Vli, unsigned int Order>
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator = (polynomial_cpu p){
    swap(*this,p);
    return *this;
} 

template <class BaseInt, std::size_t Size, unsigned int Order> // C - the VliOut is twice larger
polynomial_cpu<vli_cpu<BaseInt, 2*Size>, 2*Order> operator * (polynomial_cpu<vli_cpu<BaseInt, Size>, Order> const& p1, polynomial_cpu<vli_cpu<BaseInt, Size>, Order> const& p2){
    polynomial_cpu<vli_cpu<BaseInt, 2*Size>, 2*Order> result;
    poly_multiply(result, p1, p2);
    // poly_multiply_block_algo(result,p1,p2);
    // poly_multiply_diag_algo(result,p1,p2);
    return result;
}

template <class BaseInt, std::size_t Size, unsigned int Order, class T>
polynomial_cpu<vli_cpu<BaseInt, Size>, Order> operator * (polynomial_cpu<vli_cpu<BaseInt, Size>, Order>  const& p, monomial<T> const& m)
{
    typedef typename polynomial_cpu<vli_cpu<BaseInt,Size>,Order>::exponent_type exponent_type;
    
    polynomial_cpu<vli_cpu<BaseInt, Size>, Order> r;
    // TODO perhaps there is a better way to write these loops,
    //      so that they can be unrolled.
    for(exponent_type je = 0; je < Order-m.j_exp_; ++je)
        for(exponent_type he = 0; he < Order-m.h_exp_; ++he)
            r(je+m.j_exp_,he+m.h_exp_) = p(je,he) * m.coeff_;
    return r;
}

template<class Vli, unsigned int Order>    
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator += (polynomial_cpu<Vli,Order> const& p){
        for(exponent_type i=0; i<Order*Order;++i)
            coeffs_[i]+=p.coeffs_[i];
        return *this;
}
    
template<class Vli, unsigned int Order>    
template<typename T>
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator += (monomial<T> const& m){
    operator()(m.j_exp_,m.h_exp_) += m.coeff_; 
    return *this;
}
    
template<class Vli, unsigned int Order>    
template <typename T>
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator += (T const& t){ 
    coeffs_[0]+=t;
    return *this;
}
   
template<class Vli, unsigned int Order>    
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator -= (polynomial_cpu const& p){
    for(exponent_type i=0; i<Order*Order;++i)
        coeffs_[i]-=p.coeffs_[i];        
    return *this;
}
    
template<class Vli, unsigned int Order>        
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator -= (monomial<Vli> const& m){
    operator()(m.j_exp_,m.h_exp_) -= m.coeff_; 
    return *this;
}


template<class Vli, unsigned int Order>    
template <typename T>
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator -= (T const& t){
    coeffs_[0]-=t;
    return *this;
}

template<class Vli, unsigned int Order>          
bool polynomial_cpu<Vli,Order>::operator==(polynomial_cpu const& p) const{
    int n = memcmp((void*)&coeffs_[0],(void*)&p.coeffs_[0],Order*Order*Vli::size*sizeof(typename Vli::value_type));
    return (0 == n);
}

template<class Vli, unsigned int Order>          
void polynomial_cpu<Vli,Order>::swap(polynomial_cpu& p1, polynomial_cpu& p2){
    using boost::swap;
    swap(p1.coeffs_,p2.coeffs_);
}
    
template<class Vli, unsigned int Order>          
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator *= (Vli const& c){
    for(exponent_type i=0; i<Order*Order;++i)
        coeffs_[i] *= c;
    return *this;
}

template<class Vli, unsigned int Order>              
polynomial_cpu<Vli,Order>& polynomial_cpu<Vli,Order>::operator *= (int c){
    for(exponent_type i=0; i<Order*Order;++i)
        coeffs_[i] *= c;
    return *this;
}
    
template<class Vli, unsigned int Order>              
inline Vli const& polynomial_cpu<Vli,Order>::operator ()(exponent_type j_exp, exponent_type h_exp) const{
    assert(j_exp < max_order);
    assert(h_exp < max_order);
    return coeffs_[j_exp*max_order+h_exp];
}

template<class Vli, unsigned int Order>              
inline Vli & polynomial_cpu<Vli,Order>::operator ()(exponent_type j_exp, exponent_type h_exp){
    assert(j_exp < max_order);
    assert(h_exp < max_order);
    return coeffs_[j_exp*max_order+h_exp];
}

template<class Vli, unsigned int Order>                  
void polynomial_cpu<Vli,Order>::print(std::ostream& os) const{
        for(exponent_type i = 0; i < Order ; i++){
            for(exponent_type j = 0; j < Order ; j++){
             //   os << "Coeff (j,h) = " << i <<" "<<j<< std::endl;
                os <<coeffs_[i*Order+j] << std::endl;
            }
        }
}
     
template<class T, class Vli, unsigned int Order>
polynomial_cpu<Vli, Order> operator * (monomial<T> const& m,polynomial_cpu<Vli, Order> const& p)
{
    return p * m;
}

template<class Vli, unsigned int Order>
polynomial_cpu<Vli, Order> operator * (polynomial_cpu<Vli, Order> p, Vli const& c){
    p *= c;
    return p;
}

template<class Vli, unsigned int Order>
polynomial_cpu<Vli, Order> operator * (polynomial_cpu<Vli, Order> p, int c){
    p *= c;
    return p;
}

template<class Vli, unsigned int Order>
polynomial_cpu<Vli, Order> operator * (Vli const& c, polynomial_cpu<Vli, Order> const& p){
    return p * c;
}

template<class Vli, unsigned int Order>
polynomial_cpu<Vli, Order> operator * (int c, polynomial_cpu<Vli, Order> const& p){
    return p * c; 
}

template<class Vli, unsigned int Order> 
std::ostream& operator<<(std::ostream& os, polynomial_cpu<Vli, Order> const& p){
    p.print(os);
    return os;
}
