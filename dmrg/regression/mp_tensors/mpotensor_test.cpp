#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
#include "utils/zout.hpp"

#ifdef MPI_PARALLEL
#include "p_dense_matrix/p_dense_matrix.h"
#include "p_dense_matrix/matrix_interface.hpp"
#include "p_dense_matrix/resizable_matrix_interface.hpp"
#include "p_dense_matrix/dense_matrix_algorithms.h"
#include "p_dense_matrix/matrix_algorithms.hpp"
typedef blas::p_dense_matrix<double> Matrix;
#else
#include "dense_matrix/p_dense_matrix.h"
#include "dense_matrix/matrix_interface.hpp"
#include "dense_matrix/resizable_matrix_interface.hpp"
#include "dense_matrix/dense_matrix_algorithms.h"
#include "dense_matrix/matrix_algorithms.hpp"
typedef blas::dense_matrix<double> Matrix;
#endif

#include "block_matrix/indexing.h"
#include "mp_tensors/mpstensor.h"
#include "mp_tensors/mpotensor.h"
#include "mp_tensors/contractions.h"

#include "mp_tensors/special_mpos.h"

int main()
{
    typedef NullGroup grp;
    
    Index<grp> physical, aux, mpo_aux;
    physical.insert(std::make_pair(NullGroup::Plus, 2));
    aux.insert(std::make_pair(NullGroup::Plus, 10));
    mpo_aux.insert(std::make_pair(NullGroup::Plus, 1));
    
    MPOTensor<Matrix, grp> mpo(physical, mpo_aux, mpo_aux);
    MPSTensor<Matrix, grp> mps(physical, aux, aux);
    
    block_matrix<Matrix, grp> left(aux*aux, aux), right(aux*aux, aux), o;
    
    left = contraction::overlap_mpo_left_step(mps, mps, left, mpo);
    left = contraction::overlap_mpo_left_step(mps, mps, left, mpo);
    
    right = contraction::overlap_mpo_right_step(mps, mps, right, mpo);
    right = contraction::overlap_mpo_right_step(mps, mps, right, mpo);
    
    right = transpose(right);
    gemm(left, right, o);
    zout << trace(o) << endl;
}
