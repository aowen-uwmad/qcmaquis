

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

BOOST_AUTO_TEST_CASE_TEMPLATE( IDENTITY, T, test_types)
{
    std::size_t accessx(T::valuex-1), accessy(T::valuey-1);
    typename T::dbl x;
    typename T::dbl y;
    // check if we are writing inside the matrix
    BOOST_STATIC_ASSERT(T::valuex-1>0);
    BOOST_STATIC_ASSERT(T::valuex-1>0);

    pMatrix pA(T::valuex,T::valuey);
    sMatrix sA(T::valuex,T::valuey);

    generate(sA,Rd); // Rd is rand generator static variable inside utilities
    pA = maquis::bindings::matrix_cast<pMatrix>(sA);

    pA(accessx,accessy) = 3;
    sA(accessx,accessy) = 3;

    x =  pA(accessx,accessy) ;
    y =  sA(accessx,accessy) ;

    Boost_check_close_adapter(x,y);
    BOOST_CHECK(pA==sA); // memory corruption check
}

BOOST_AUTO_TEST_CASE_TEMPLATE( IDENTITY_TRACE, T, test_types)
{
    typename T::dbl x,y;
    pMatrix pA = pMatrix::identity_matrix(T::valuex);
    sMatrix sA = sMatrix::identity_matrix(T::valuex);
    ambient::sync();
   
    x = trace(pA);
    y = trace(sA);
   
    Boost_check_close_adapter(x,y);
}

