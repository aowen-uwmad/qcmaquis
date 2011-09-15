#include <iostream>
#include <cstdio>

#include <boost/lexical_cast.hpp>

#include "vli/utils/gpu_manager.h"
#include "vli/utils/gpu_manager.hpp"

#include "vli/polynomial/vector_polynomial_cpu.hpp"
#include "vli/polynomial/vector_polynomial_gpu.hpp"
#include "vli/polynomial/polynomial_gpu.hpp"
#include "vli/polynomial/polynomial_cpu.hpp"
#include "vli/polynomial/monomial.hpp"
#include "vli/vli_cpu.hpp"
#include "vli/vli_traits.hpp"
#include "vli/vli_gpu.hpp"
#include "utils/timings.h"
#include "regression/vli_test.hpp"
#include <cublas_v2.h>
#include <cuda_runtime.h>


using vli::vli_cpu;
using vli::max_int_value;
using vli::vli_gpu;
using vli::monomial;
using vli::polynomial_cpu;
using vli::polynomial_gpu;
using vli::vector_polynomial_gpu;
using vli::vector_polynomial_cpu;

using vli::test::fill_random;
using vli::test::fill_poly_random;
using vli::test::fill_vector_random;


typedef vli_cpu<long unsigned int, 3> vli_type_cpu;
typedef vli_gpu<long unsigned int, 3> vli_type_gpu;

typedef vli::monomial<vli_type_cpu> monomial_type_cpu;
typedef vli::monomial<vli_type_gpu> monomial_type_gpu;

typedef vli::polynomial_cpu< vli_type_cpu, vli::detail::size_poly_vli::value > polynomial_type_cpu;
typedef vli::polynomial_gpu< vli_type_gpu, vli::detail::size_poly_vli::value > polynomial_type_gpu;

typedef vli::vector_polynomial_cpu<polynomial_type_cpu> vector_type_cpu;
typedef vli::vector_polynomial_gpu<polynomial_type_gpu> vector_type_gpu;


int main (int argc, char * const argv[]) 
{
    
    gpu::gpu_manager* gpu;
    gpu->instance();

    cublasHandle_t cuhandle;
    cublasCreate(&cuhandle);
    vector_type_cpu VaCPU(10000); 
    vector_type_cpu VbCPU(10000); 
    polynomial_type_cpu result; 
    polynomial_type_gpu result_gpu; 

    fill_vector_random(VaCPU,1);
    fill_vector_random(VbCPU,1);

//    vector_type_gpu VaGPU( VaCPU); 
//    vector_type_gpu VbGPU( VbCPU); 

    Timer CPU("CPU");
    CPU.begin();
    result = inner_product(VaCPU,VbCPU);
    CPU.end();

//    TimerCuda GPU("GPU");
//    GPU.begin();    
//    result_gpu = inner_product(VaGPU,VbGPU);
//    GPU.end();
   
    cublasDestroy(cuhandle);
    std::cout<<result<<std::endl;
//    if(result == result_gpu)
//        printf( "OK \n"); 
//    else{
//        printf( "NO OK \n"); 
//        std::cout << result << std::endl;
//        std::cout << result_gpu << std::endl;
//    }
    return 0;
}


