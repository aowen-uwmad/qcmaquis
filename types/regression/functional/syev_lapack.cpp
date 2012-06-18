
#define BOOST_TEST_MODULE ambient_c_kernels
#include <mpi.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "types/p_dense_matrix/p_dense_matrix.h"

#include "types/dense_matrix/dense_matrix.h"
#include "types/dense_matrix/matrix_interface.hpp"
#include "types/dense_matrix/resizable_matrix_interface.hpp"
#include "types/dense_matrix/matrix_algorithms.hpp"
#include "types/dense_matrix/algorithms.hpp"
#include "types/dense_matrix/dense_matrix_blas.hpp"
#include "types/dense_matrix/aligned_allocator.h"

#include "types/utils/bindings.hpp"
#include "utilities.h"

BOOST_AUTO_TEST_CASE_TEMPLATE( syev, T, test_types)
{
    pMatrix pA(T::valuex,T::valuex);
    pMatrix pV(T::valuex,T::valuex);

    sMatrix sA(T::valuex,T::valuex);
    sMatrix sV(T::valuex,T::valuex);

    pA.fill_random();
    sA = maquis::traits::matrix_cast<sMatrix>(pA); // playout is inside the cast
 
    typename maquis::types::associated_diagonal_matrix<pMatrix>::type pE(T::valuex,T::valuex); 
    typename maquis::types::associated_diagonal_matrix<sMatrix>::type sE(T::valuex,T::valuex);

    maquis::types::syev(pA,pV,pE); // to modify the algo we need the reverse inside !
    maquis::types::syev(sA,sV,sE);
     
    BOOST_CHECK(sE == pE);
}
