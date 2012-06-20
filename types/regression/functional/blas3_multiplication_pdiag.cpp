#define BOOST_TEST_MODULE ambient_c_kernels
#include <mpi.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "types/p_dense_matrix/p_dense_matrix.h"

#include "alps/numeric/matrix/matrix.hpp"
#include "alps/numeric/matrix/matrix_blas.hpp"
#include "alps/numeric/matrix/algorithms.hpp"
#include "alps/numeric/matrix/matrix_interface.hpp"
#include "alps/numeric/matrix/resizable_matrix_interface.hpp"

#include "types/utils/bindings.hpp"

#include "utilities.h"

BOOST_AUTO_TEST_CASE_TEMPLATE( addition, T, test_types)
{
    pMatrix pA(T::valuex,T::valuex);
    pMatrix pB(T::valuex,T::valuex);
    pDiagMatrix pC(T::valuex,T::valuex);

    sMatrix sA(T::valuex,T::valuex);
    sMatrix sB(T::valuex,T::valuex);
    sDiagMatrix sC(T::valuex,T::valuex);

    generate(sB,Rd); // Rd is rand generator static variable inside utilities
    sC.generate(Rd); // Rd is rand generator static variable inside utilities

    pB = maquis::traits::matrix_cast<pMatrix>(sB); // playout is inside the cast
    pC = maquis::traits::matrix_cast<pDiagMatrix>(sC); // playout is inside the cast
 
    gemm(pC,pB,pA);
    gemm(sC,sB,sA);

    BOOST_CHECK(pA==sA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}

