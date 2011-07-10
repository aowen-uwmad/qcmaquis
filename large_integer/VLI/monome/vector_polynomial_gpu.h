/*
 *  vector_polynomial_gpu.h
 *
 *  Created by Tim Ewart (timothee.ewart@unige.ch) and  Andreas Hehn (hehn@phys.ethz.ch) on 18.03.11.
 *  Copyright 2011 University of Geneva and Eidgenössische Technische Hochschule Züric. All rights reserved.
 *
 */

#ifndef VLI_VECTOR_POLYNOME_GPU_H
#define VLI_VECTOR_POLYNOME_GPU_H
#include "boost/swap.hpp"
#include "monome/polynome_cpu.h"
#include "monome/polynome_gpu.h"
#include "vli_cpu/vli_number_cpu.hpp"
#include "vli_gpu/vli_number_gpu.hpp"
#include "function_hooks/vli_vector_polynomial_gpu_function_hooks.hpp"

#include <ostream>





namespace vli
{

    template<class BaseInt, int Size>
    class vli_cpu;
    
    template<class Vli, int Order>
	class polynomial_cpu;

    template<class polynomial>
    class vector_polynomial_cpu;

    template<class polynomial_gpu> 
    class vector_polynomial_gpu : public gpu_vector<typename polynomial_gpu::vli_value_type>{ 
    private:
        typedef typename polynomial_gpu::vli_value_type vli_value_type; 
        typedef typename std::size_t size_t;
        enum {max_order_poly = polynomial_gpu::max_order };
        enum {vli_size   = polynomial_gpu::size }; 
        enum {OffSet = max_order_poly*max_order_poly*vli_size}; 
    public:
        // the usual proxy for have acces to element, we initialize with a polynomial, take care of the size !
        class proxy
        {
        public:
            proxy(vli_value_type* p, size_t i)
            :pdata_(p), pos(i)
            {
            }
            
            proxy& operator=(polynomial_gpu const& p)
            {
                gpu::cu_check_error(cudaMemcpy((void*)(pdata_+pos*OffSet),(void*)(p.p()), max_order_poly*max_order_poly*vli_size*sizeof(typename polynomial_gpu::vli_value_type), cudaMemcpyDeviceToDevice ), __LINE__); 	           
                return *this;
            }
            
            friend std::ostream& operator << (std::ostream& os, proxy const & pr){
                pr.print(os);
                return os; 
            }
            
            void print(std::ostream & os) const{ // CONST ! 
                polynomial_cpu<vli_cpu<vli_value_type, vli_size>, max_order_poly > P;
                gpu::cu_check_error(cudaMemcpy((void*)(&P(0,0)),(void*)(pdata_+pos*OffSet),max_order_poly*max_order_poly*vli_size*sizeof(typename polynomial_gpu::vli_value_type), cudaMemcpyDeviceToHost), __LINE__);
                os << P;
            }
                                 
        private:
            vli_value_type* pdata_;
            size_t pos;
        };    
        
        vector_polynomial_gpu(size_t size = 1) // by default one polynomial
        :gpu_vector<typename polynomial_gpu::vli_value_type>(size*max_order_poly*max_order_poly*vli_size), size_(size)
        {
        }
        
        vector_polynomial_gpu(vector_polynomial_gpu const& v)
        :gpu_vector<typename polynomial_gpu::vli_value_type>(v),size_(v.size_){	
           
        }
        
        vector_polynomial_gpu& operator=(vector_polynomial_gpu v)
        {  
            swap(*this, v);
            return *this;
        }
        
        proxy operator[](size_t i)
        {
            return proxy(this->p(),i);
        }
    
        const size_t size() const{
            return size_;
        }
    
        void resize(std::size_t num){
            size_ = num;
            gpu_vector<typename polynomial_gpu::vli_value_type>::vec_resize(num*max_order_poly*max_order_poly*vli_size);
        }
        
        bool operator==(vector_polynomial_cpu<polynomial_cpu<vli_cpu<vli_value_type, vli_size>, max_order_poly> >  & v) const 
        {//slow, need for boost test
            bool B(true);
            size_t i(0);
            while(B || i == v.size()){
               //(v[i] == (*this)[i]) ? (B =true) : (B = false) ;
                ++i;
            }
            return B;
        }
            
    private:
        std::size_t size_; // number of polynomial
        
    };
   
    template <class BaseInt, int Size, int Order>
    vector_polynomial_gpu<polynomial_gpu<vli_gpu<BaseInt, Size>, Order> > 
    inner_product( vector_polynomial_gpu<polynomial_gpu<vli_gpu<BaseInt, Size>, Order> >  const& a, 
                   vector_polynomial_gpu<polynomial_gpu<vli_gpu<BaseInt, Size>, Order> >  const& b){
        assert(a.size() == b.size());
        vector_polynomial_gpu<polynomial_gpu<vli_gpu<BaseInt, Size>, Order> > res;
        inner_product_multiplication_gpu(a,b,res);
        return res;
    }
    
    template <class BaseInt, int Order, int SizeVli >
	std::ostream & operator<<(std::ostream & os, vector_polynomial_gpu< polynomial_gpu< vli_gpu<BaseInt, SizeVli>, Order > >   & v)
    {
        os << "---------- GPU ----------" << std::endl;
    
        for(std::size_t i = 0; i < v.size(); i++)
            os << v[i] << std::endl;

        return os;
    }
}

#endif //VLI_VECTOR_POLYNOME_GPU_H
