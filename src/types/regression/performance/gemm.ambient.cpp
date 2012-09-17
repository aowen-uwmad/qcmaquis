#define BOOST_TEST_MODULE ambient_c_kernels
#include <mpi.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "ambient/utils/timings.hpp"
#include "utilities.h"

BOOST_AUTO_TEST_CASE_TEMPLATE( test, T, test_types){
    typedef ambient::dim2 dim;
    typedef alps::numeric::matrix<typename T::value_type> sMatrix;
    typedef alps::numeric::diagonal_matrix<typename T::value_type> sDiagMatrix;
    typedef ambient::numeric::matrix<typename T::value_type> pMatrix;
    typedef ambient::numeric::diagonal_matrix<typename T::value_type> pDiagMatrix;

    size_t x = get_input_x<T>();
    size_t y = get_input_y<T>();
    size_t nthreads = get_input_threads<T>();

    pMatrix pA(x, y);
    pMatrix pB(x, y);
    pMatrix pC(x, y);

    sMatrix sA(x, y);
    sMatrix sB(x, y);

    fill_random(pA);
    fill_random(pB);

    sA = maquis::bindings::matrix_cast<sMatrix>(pA);
    sB = maquis::bindings::matrix_cast<sMatrix>(pB);
    ambient::sync();

    ambient::numeric::gemm(pA, pB, pC); 

    __a_timer time("ambient");
    time.begin();
    ambient::sync();
    time.end();

    report(time, GFlopsGemm, x, y, nthreads);
}

