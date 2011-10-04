#include <iostream>
#include <cstdio>

#include <boost/lexical_cast.hpp>
#ifdef VLI_USE_GPU
#include "vli/utils/gpu_manager.h"
#include "vli/utils/gpu_manager.hpp"
#endif //VLI_USE_GPU
#include "vli/polynomial/vector_polynomial_cpu.hpp"
#include "vli/polynomial/polynomial_cpu.hpp"
#include "vli/polynomial/monomial.hpp"
#include "vli/vli_cpu.hpp"
#include "vli/vli_traits.hpp"
#include "utils/timings.h"
#include "regression/vli_test.hpp"
#include <cuda_runtime.h>


using vli::vli_cpu;
using vli::max_int_value;
using vli::monomial;
using vli::polynomial_cpu;
using vli::vector_polynomial_cpu;

using vli::test::fill_random;
using vli::test::fill_poly_random;
using vli::test::fill_vector_random;


typedef vli_cpu<long unsigned int, 4> vli_type_cpu;

typedef vli::monomial<vli_type_cpu> monomial_type_cpu;

typedef vli::polynomial_cpu< vli_type_cpu, 21 > polynomial_type_cpu;

typedef vli::vector_polynomial_cpu<polynomial_type_cpu> vector_type_cpu;


int main (int argc, char * const argv[]) 
{
    vector_type_cpu v1(500); 
    vector_type_cpu v2(500);
    polynomial_type_cpu result;
    polynomial_type_cpu result_pure_gpu;
    polynomial_type_cpu result_pure_cpu; 

    fill_vector_random(v1,1);
    fill_vector_random(v2,1);


    Timer t3("CPU");
    t3.begin();
    result_pure_cpu = vli::detail::inner_product_plain(v1,v2);
    t3.end();

    Timer t1("Default");
    t1.begin();
    result = inner_product(v1,v2);
    t1.end();

    Timer t2("GPU");
    t2.begin();
    vli::detail::inner_product_gpu_booster<vector_type_cpu> gpu_boost(v1,v2,v1.size());
    result_pure_gpu = polynomial_type_cpu(gpu_boost);
    t2.end();


    std::cout << result << std::endl << std::endl <<std::endl;
    std::cout << result_pure_gpu << std::endl << std::endl << std::endl;
    std::cout << result_pure_cpu << std::endl << std::endl << std::endl;
    
    if(result == result_pure_gpu && result == result_pure_cpu)
        printf( "OK \n"); 
    else{
        printf( "NOT OK \n"); 
    }
    return 0;
}


