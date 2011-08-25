/*
 *  monome.h
 *
 *  Created by Tim Ewart (timothee.ewart@unige.ch) and  Andreas Hehn (hehn@phys.ethz.ch) on 18.03.11.
 *  Copyright 2011 University of Geneva and Eidgenössische Technische Hochschule Züric. All rights reserved.
 *
 */

#ifndef VLI_MONOME_H
#define VLI_MONOME_H

#include <ostream>
#include <cmath>

namespace vli
{
	
    template<class BaseInt, int Size>
    class vli_gpu;
        
	template <class Vli>
    struct monomial
    {
        typedef std::size_t size_type;
         
        /**
         * Constructor: Creates a monomial 1*J^j_exp*h^h_exp
         */
        explicit monomial(size_type j_exp = 0, size_type h_exp = 0)
        :j_exp_(j_exp), h_exp_(h_exp), coeff_(1){
        }

        explicit monomial(Vli a,size_type j_exp = 0, size_type h_exp = 0)
        :j_exp_(j_exp), h_exp_(h_exp), coeff_(a){
        }


        
        monomial& operator *= (Vli const& c){
            coeff_ *= c;
            return (*this);
        }
        
        monomial& operator *= (int c){
            coeff_ *= c;
            return (*this);
        }

        /**
        * For the GPU proxy 
        * MEGA DANGEROUS BUT PUTAIN DE BORDEL DE TEMPLATE A LA CON
        * Andreas one free drink, if you get a safer solution solution
        * I tried to specify but my compilo is blind
        */        
//      template<class Order>
//      monomial& operator *= (typename vector_polynomial_gpu<polynomial_gpu<Vli,Order> >::proxy const& p){
        template <class T>                                    
        monomial& operator *= (T const& p){
            Vli Tmp = p.BuildProxyToVli();
            coeff_ *= Tmp;
            return (*this);
        }
        
        void print(std::ostream& os) const
        {
            if(coeff_ > 0)
                os<<"+";
            os<<coeff_<<"*J^"<<j_exp_<<"*h^"<<h_exp_;
        }

        bool operator == (monomial const& m) const{
            return (j_exp_ == m.j_exp_) && (h_exp_ == m.h_exp_) && (coeff_ == m.coeff_);
        }
        
        size_type j_exp_;
        size_type h_exp_;
        Vli coeff_;
    };
    
    template<class Vli>
    std::ostream& operator<<(std::ostream& os, monomial<Vli> const& m){
        m.print(os);
        return os;
    }
    
    template <typename Vli, typename T>
    monomial<Vli> operator * (monomial<Vli> m, T const& c)
    {
        m*=c;
        return m;
    }

    template <typename Vli, typename T>
    monomial<Vli> operator * (T const& c, monomial<Vli> const& m)
    {
        return m*c;
    }

}


#endif //VLI_MONOME_H
