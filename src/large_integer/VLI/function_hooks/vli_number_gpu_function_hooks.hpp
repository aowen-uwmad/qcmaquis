#include "detail/kernels_gpu.h"

#ifndef VLI_NUMBER_GPU_FUNCTION_HOOKS_HPP
#define VLI_NUMBER_GPU_FUNCTION_HOOKS_HPP

namespace vli {

/**
template forward declaration 
*/
template <class BaseInt, int Size>
class vli_gpu;


template <class BaseInt, int Size>
void plus_assign(vli_gpu<BaseInt,Size> & vli_a, vli_gpu<BaseInt,Size> const& vli_b )
{
    using detail::plus_assign_gpu;
    plus_assign_gpu( vli_a.p(), vli_b.p(), 1, Size);
}

template <class BaseInt, int Size>
void minus_assign(vli_gpu<BaseInt,Size> & vli_a, vli_gpu<BaseInt,Size> const& vli_b )
{
    using detail::minus_assign_gpu;
    minus_assign_gpu(vli_a.p(),vli_b.p(),Size);
}

template <class BaseInt, int Size>
void multiplies_assign(vli_gpu<BaseInt,Size> & vli_a, vli_gpu<BaseInt,Size> const& vli_b )
{
    using detail::entrywise_multiplies_gpu;
    // TODO this seems to be quite inefficient
    // (We implement the a *= b as a regular c = a * b and do a swap(a,c)...)
    // (I guess it's better to try implement a *= b directly.)
    // I need an intermediate buffer unfortunatly
    vli_gpu<BaseInt,Size> vli_c;
    entrywise_multiplies_gpu( vli_a.p(), vli_b.p(), vli_c.p(), Size);
    swap(vli_a,vli_c);
}

} //namespace vli

#endif //VLI_NUMBER_GPU_FUNCTION_HOOKS_HPP
