#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

#include "p_dense_matrix/p_dense_matrix.h"
#include "p_dense_matrix/matrix_interface.hpp"
#include "p_dense_matrix/resizable_matrix_interface.hpp"
#include "p_dense_matrix/dense_matrix_algorithms.h"
#include "p_dense_matrix/matrix_algorithms.hpp"
typedef blas::p_dense_matrix<double> Matrix;

#include "p_block_matrix/indexing.h"
#include "mp_tensors/mpstensor.h"
#include "mp_tensors/mpotensor.h"
#include "mp_tensors/contractions.h"
#include "mp_tensors/special_mpos.h"

void ng()
{
    typedef NullGroup grp;
    
    Index<grp> physical, aux1, aux2;
    physical.insert(std::make_pair(NullGroup::Plus, 2));
    aux1.insert(std::make_pair(NullGroup::Plus, 10));
    aux2.insert(std::make_pair(NullGroup::Plus, 11));
    
    MPSTensor<Matrix, grp> mps(physical, aux1, aux1);
    cout << mps.scalar_norm() << endl;
    mps.multiply_by_scalar(1/mps.scalar_norm());
    cout << mps.scalar_norm() << endl;
    
    mps.normalize_left(SVD);
    cout << "Norm after normalization: " << mps.scalar_norm() << endl;
    
    MPSTensor<Matrix, grp> mps2 = mps;
    p_block_matrix<Matrix, grp> left = identity_matrix<Matrix, grp>(mps.row_dim());
    left = contraction::overlap_left_step(mps, mps2, left);
    cout << left << endl;
    
    mps.normalize_right(SVD);
    mps2 = mps;
    p_block_matrix<Matrix, grp> right = identity_matrix<Matrix, grp>(mps.row_dim());
    right = contraction::overlap_right_step(mps, mps2, right);
    cout << right << endl;
    
    mps = MPSTensor<Matrix, grp>(physical, aux1, aux1);
    mps2 = MPSTensor<Matrix, grp>(physical, aux2, aux2);
    
    p_block_matrix<Matrix, grp> ovlp(aux2, aux1);
    contraction::overlap_left_step(mps2, mps, ovlp);
    contraction::overlap_left_step(mps2, mps, ovlp);
    contraction::overlap_right_step(mps2, mps, ovlp);
    contraction::overlap_right_step(mps2, mps, ovlp);
}

void u1()
{
    typedef U1 grp;
    
    Index<grp> physical, aux1, aux2;
    physical.insert(std::make_pair(-1, 1));
    physical.insert(std::make_pair(1, 1));
    
    aux1.insert(std::make_pair(1, 3));
    aux1.insert(std::make_pair(-1, 3));
    
    aux2.insert(std::make_pair(2, 3));
    aux2.insert(std::make_pair(0, 3));
    aux2.insert(std::make_pair(-2, 3));
    
    MPSTensor<Matrix, grp> mps(physical, aux1, aux2);
    
    cout << mps.scalar_norm() << endl;
    mps.multiply_by_scalar(1/mps.scalar_norm());
    cout << mps.scalar_norm() << endl;
    mps.normalize_left(SVD);
    cout << mps.scalar_norm() << endl;
    
//    mps.make_left_paired();
//    cout << mps << endl;
//    
//    mps.make_right_paired();
//    mps.make_left_paired();
//    
//    cout << mps << endl;
    
    MPSTensor<Matrix, grp> mps2 = mps;
//    p_block_matrix<Matrix, grp> left = identity_matrix<Matrix, grp>(mps.row_dim());
//    left = contraction::overlap_left_step(mps, mps2, left);
//    cout << left << endl;
//    mps.normalize_right(SVD);
//    mps2 = mps;
//    left = contraction::overlap_right_step(mps, mps2, left);
//    cout << left << endl;
    
    MPOTensor<Matrix, U1> ident(physical, 1, 1);
    ident(0,0) = p_block_matrix<Matrix, U1>();
    ident(0,0).insert_block(boost::tuples::make_tuple(Matrix(1, 1, 1), -1, -1));
    ident(0,0).insert_block(boost::tuples::make_tuple(Matrix(1, 1, 1), 1, 1));
    
    MPOTensor<Matrix, U1> splus(physical, 1, 1);
    splus(0,0) = p_block_matrix<Matrix, U1>();
    splus(0,0).insert_block(boost::tuples::make_tuple(Matrix(1, 1, 1), 1, -1));
    splus(0,0).insert_block(boost::tuples::make_tuple(Matrix(1, 1, 1), -1, 1));
    
    mps.normalize_left(SVD);
    mps2 = mps;
    Boundary<Matrix, grp> mleft(mps.row_dim(), mps.row_dim(), 1);
    mleft.data_[0] = identity_matrix<Matrix, grp>(mps.row_dim());
//    cout << mleft.data_[0] << endl;
//    mleft = contraction::overlap_mpo_left_step(mps, mps2, mleft, splus);
//    cout << mleft.data_[0] << endl;
    
    mps.normalize_right(SVD);
    mps2 = mps;
    
    mleft.data_[0] = identity_matrix<Matrix, grp>(mps.col_dim());
    mleft.data_[0] = contraction::overlap_right_step(mps, mps2, mleft.data_[0]);
    cout << mleft.data_[0] << endl;
    
    mleft.data_[0] = identity_matrix<Matrix, grp>(mps.col_dim());
    mleft = contraction::overlap_mpo_right_step(mps, mps2, mleft, ident);
    cout << mleft.data_[0] << endl;
}

int main()
{
//    ng();
    u1();
}

