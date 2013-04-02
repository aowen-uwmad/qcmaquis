#define BOOST_TEST_MODULE ambient_c_kernels
#include <mpi.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/random.hpp>

#include "alps/numeric/matrix.hpp"
#include "alps/numeric/diagonal_matrix.hpp"
#include "ambient/numeric/matrix.hpp"
#include "utilities.h"

BOOST_AUTO_TEST_CASE_TEMPLATE( SQRT, T, test_types)
{
    typedef alps::numeric::diagonal_matrix<typename T::value_type> sDiagMatrix;
    typedef ambient::numeric::tiles<ambient::numeric::diagonal_matrix<typename T::value_type> > pDiagMatrix;

    pDiagMatrix pA(T::valuex);
    sDiagMatrix sA((std::size_t)T::valuex);

    generate(pA);
    sA = maquis::bindings::matrix_cast<sDiagMatrix>(pA);

    sA = sqrt(sA);
    ambient::numeric::sqrt_inplace(pA);

    BOOST_CHECK(pA==sA);
}
