#include <mpi.h>

#include <iostream>
#include <cmath>
//#include "utils/zout.hpp"
#include "ambient/ambient.h"
#include "ambient/traits.h"
#include "utils/zout.hpp"
#include <complex>

#include "types/p_dense_matrix/p_dense_matrix.h"
#include "types/p_dense_matrix/concept/matrix_interface.hpp"
#include "types/p_dense_matrix/algorithms.hpp"
#include "types/p_dense_matrix/concept/resizable_matrix_interface.hpp"
#include "types/p_dense_matrix/p_dense_matrix_algorithms.hpp"
#include "types/p_dense_matrix/p_diagonal_matrix.h"

#include "types/dense_matrix/dense_matrix.h"
#include "types/dense_matrix/matrix_interface.hpp"
#include "types/dense_matrix/resizable_matrix_interface.hpp"
#include "types/dense_matrix/dense_matrix_algorithms.h"
#include "types/dense_matrix/matrix_algorithms.hpp"
#include "types/dense_matrix/algorithms.hpp"
#include "types/dense_matrix/dense_matrix_blas.hpp"
#include "types/dense_matrix/aligned_allocator.h"

#include "types/utils/matrix_cast.h"

//#define  T double
#define  T typename traits::type

//using namespace maquis::types::algorithms;
using namespace maquis::types;
using namespace ambient;

typedef maquis::types::dense_matrix<T> sMatrix;
typedef maquis::types::diagonal_matrix<T> sdMatrix;
typedef maquis::types::p_dense_matrix<T> pMatrix;
typedef maquis::types::p_diagonal_matrix<T> pdMatrix;

typedef ambient::dim2 dim;

namespace Random{
   struct random {
       random(){};
       double operator()(){return drand48();}
       int IntRd(){return mrand48();}
   };
}

static Random::random Rd; //one generator is enough so static

int main(int argc, char* argv[])
{
    ambient::init();

    ambient::layout >> dim(1,1), dim(1,1), dim(1,1);

    int NUM=std::atoi(argv[1]);
    
    pMatrix pA(NUM,NUM);
    pMatrix pB(NUM,NUM);
    pA.set_init(ambient::random_i<T>);
    pB.set_init(ambient::random_i<T>);
    std::cout << pA << std::endl;  
    std::cout << pB << std::endl;  
    pA+=pB;
    pB+=pA;
    std::cout << pB << std::endl;  
    pB*=3;
    std::cout << pB << std::endl;  
    ambient::finalize();

}
