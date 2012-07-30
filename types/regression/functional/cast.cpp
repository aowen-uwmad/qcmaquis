
#define BOOST_TEST_MODULE ambient_c_kernels
#include <mpi.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/random.hpp>

#include "alps/numeric/matrix.hpp"
#include "alps/numeric/matrix/algorithms.hpp"
#include "ambient/numeric/matrix.hpp"
#include "utilities.h"

BOOST_AUTO_TEST_CASE_TEMPLATE( cast_p2s, T, test_types)
{
    pMatrix pA(T::valuex,T::valuey);
    sMatrix sA(T::valuex,T::valuey);
    pA.fill_random();
    sA = maquis::bindings::matrix_cast<sMatrix>(pA); // playout is inside the cast
    BOOST_CHECK(pA==sA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}

BOOST_AUTO_TEST_CASE_TEMPLATE( cast_s2p, T, test_types)
{
    pMatrix pA(T::valuex,T::valuey);
    sMatrix sA(T::valuex,T::valuey);
    generate(sA,Rd); // Rd is rand generator static variable inside utilities
    pA = maquis::bindings::matrix_cast<pMatrix>(sA); // playout is inside the cast
    BOOST_CHECK(sA==pA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}
/*
BOOST_AUTO_TEST_CASE_TEMPLATE( cast_p2s_diag, T, test_types)
{
    pDiagMatrix pA(T::valuex,0);
    sDiagMatrix sA(T::valuex,0);

    pA.get_data().fill_random();
    sA = maquis::bindings::matrix_cast<sDiagMatrix>(pA); // playout is inside the cast
    BOOST_CHECK(pA==sA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}

BOOST_AUTO_TEST_CASE_TEMPLATE( cast_s2p_diag, T, test_types)
{
    pDiagMatrix pA(T::valuex,0);
    sDiagMatrix sA(T::valuex,0);
   
    sA.generate(Rd); // Rd is rand generator static variable inside utilities
    pA = maquis::bindings::matrix_cast<pDiagMatrix>(sA); // playout is inside the cast
    BOOST_CHECK(sA==pA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}
*/
