#ifndef VLI_POLYNOMIAL_GPU_FUNCTION_HOOKS_HPP
#define VLI_POLYNOMIAL_GPU_FUNCTION_HOOKS_HPP

#include "detail/kernels_gpu.h"

namespace vli{
    
    template<class Vli, int Order>
    class polynomial_gpu;

    /** I think all these specializations are useless */
    template <class BaseInt, int Size, int Order>
    void poly_multiply(polynomial_gpu<vli_gpu<BaseInt, Size>, Order> & result, 
                       polynomial_gpu<vli_gpu<BaseInt, Size>, Order> const & p1, 
                       polynomial_gpu<vli_gpu<BaseInt, Size>, Order> const & p2)
    {
        detail::poly_multiply_gpu(p1.p(),p2.p(),result.p(),Size,Order);
    }
    
    template <class BaseInt, int Size, int Order>
    void poly_addition(polynomial_gpu<vli_gpu<BaseInt, Size>, Order> & result, 
                       polynomial_gpu<vli_gpu<BaseInt, Size>, Order> const & p)
    {
        detail::poly_addition_gpu(result.p(),p.p(),Size,Order);
    }

    template <class BaseInt, int Size, int Order, class monome> // monome can be a monome_gpu or a vli_gpu
    void poly_mono_multiply(polynomial_gpu<vli_gpu<BaseInt, Size>, Order> & result, 
                              polynomial_gpu<vli_gpu<BaseInt, Size>, Order> const & p1, 
                              monome const & m2)
    {
        detail::poly_mono_multiply_gpu(p1.p(),m2.p(),result.p(),Size,Order);
    }
    
    
    
}// end namespace 

#endif
