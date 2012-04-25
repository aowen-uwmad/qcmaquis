#define BOOST_TEST_MODULE ambient_c_kernels
#include <mpi.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "types/p_dense_matrix/p_dense_matrix.h"

#include "types/dense_matrix/dense_matrix.h"
#include "types/dense_matrix/dense_matrix_blas.hpp"
#include "types/dense_matrix/matrix_interface.hpp"
#include "types/dense_matrix/resizable_matrix_interface.hpp"

#include "types/utils/matrix_cast.h"

#include "utilities.h"

BOOST_AUTO_TEST_CASE_TEMPLATE( remove_first_rows, T, test_types)
{
    ambient::model >> dim(32,32), dim(32,32), dim(32,32);

    pMatrix pA(T::valuex,T::valuey);
    sMatrix sA(T::valuex,T::valuey);

    pA.set_init(ambient::random_i<typename T::dbl>);

    sA = maquis::traits::matrix_cast<sMatrix>(pA); // playout is inside the cast

    sA.remove_rows(T::null,1);
    pA.remove_rows(T::null,1);

    BOOST_CHECK(pA==sA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}

BOOST_AUTO_TEST_CASE_TEMPLATE( remove_last_rows, T, test_types)
{
    ambient::model >> dim(32,32), dim(32,32), dim(32,32);

    pMatrix pA(T::valuex,T::valuey);
    sMatrix sA(T::valuex,T::valuey);

    pA.set_init(ambient::random_i<typename T::dbl>);

    sA = maquis::traits::matrix_cast<sMatrix>(pA); // playout is inside the cast

    sA.remove_rows(T::valuex-1,1);
    pA.remove_rows(T::valuex-1,1);

    BOOST_CHECK(pA==sA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}

BOOST_AUTO_TEST_CASE_TEMPLATE( remove_rows, T, test_types)
{
    ambient::model >> dim(32,32), dim(32,32), dim(32,32);

    pMatrix pA(T::valuex,T::valuey);
    sMatrix sA(T::valuex,T::valuey);

    pA.set_init(ambient::random_i<typename T::dbl>);

    sA = maquis::traits::matrix_cast<sMatrix>(pA); // playout is inside the cast

    int row =  Rd.IntRd()%(sA.num_rows());  
    sA.remove_rows(row,1);
    pA.remove_rows(row,1);

    BOOST_CHECK(pA==sA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}
/*
BOOST_AUTO_TEST_CASE_TEMPLATE( remove_several_rows, T, test_types)
{
    ambient::model >> dim(32,32), dim(32,32), dim(32,32);

    pMatrix pA(T::valuex,T::valuey);
    sMatrix sA(T::valuex,T::valuey);

    pA.set_init(ambient::random_i<typename T::dbl>);

    sA = maquis::traits::matrix_cast<sMatrix>(pA); // playout is inside the cast

    int row =  Rd.IntRd()%(T::valuex-1);  
    int numrows = (int)(T::valuex-1 - row)/2;   
    sA.remove_rows(row,numrows);
    pA.remove_rows(row,numrows);

    BOOST_CHECK(pA==sA); // BOOST_CHECK_EQUAL necessitates == inside the class, here == is a free function 
}*/
