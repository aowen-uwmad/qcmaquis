#ifndef MPS_MPO_OPS_H
#define MPS_MPO_OPS_H

#include "mp_tensors/mps.h"
#include "mp_tensors/mpo.h"

#include "mp_tensors/special_mpos.h"

template<class Matrix, class SymmGroup>
std::vector<Boundary<Matrix, SymmGroup> >
left_mpo_overlaps(MPS<Matrix, SymmGroup> const & mps, MPO<Matrix, SymmGroup> const & mpo)
{
    assert(mpo.length() == mps.length());
    std::size_t L = mps.length();
    
    std::vector<Boundary<Matrix, SymmGroup> > left_(L+1);
    Boundary<Matrix, SymmGroup> left = mps.start_mtx();
    left_[0] = left;
    
    for (int i = 0; i < L; ++i) {
        MPSTensor<Matrix, SymmGroup> bkp = mps[i];
        left = contraction::overlap_mpo_left_step(mps[i], bkp, left, mpo[i]);
        left_[i+1] = left;
//        cout << "Left at " << i+1 << " " << left.data_[0] << endl;
    }
    return left_;
}

template<class Matrix, class SymmGroup>
std::vector<Boundary<Matrix, SymmGroup> >
right_mpo_overlaps(MPS<Matrix, SymmGroup> const & mps, MPO<Matrix, SymmGroup> const & mpo)
{
    assert(mpo.length() == mps.length());
    std::size_t L = mps.length();
    
    std::vector<Boundary<Matrix, SymmGroup> > right_(L+1);
    Boundary<Matrix, SymmGroup> right = mps.start_mtx();
    right_[L] = right;
    
    for (int i = L-1; i >= 0; --i) {
        MPSTensor<Matrix, SymmGroup> bkp = mps[i];
        right = contraction::overlap_mpo_right_step(mps[i], bkp, right, mpo[i]);
        right_[i] = right;
//        cout << "right at " << i << " " << right.data_[0] << endl;
    }
    return right_;
}

template<class Matrix, class SymmGroup>
double expval(MPS<Matrix, SymmGroup> const & mps, MPO<Matrix, SymmGroup> const & mpo, int d = 0)
{
    if (d == 0) {
        std::vector<Boundary<Matrix, SymmGroup> > left_ = left_mpo_overlaps(mps, mpo);
        return left_[mps.length()](0, std::make_pair(SymmGroup::SingletCharge, 0), std::make_pair(SymmGroup::SingletCharge, 0));
    } else {
        std::vector<Boundary<Matrix, SymmGroup> > right_ = right_mpo_overlaps(mps, mpo);
        return right_[0](0, std::make_pair(SymmGroup::SingletCharge, 0), std::make_pair(SymmGroup::SingletCharge, 0));
    }
}

template<class Matrix, class SymmGroup>
typename Matrix::value_type norm(MPS<Matrix, SymmGroup> const & mps,
                                 MPOTensor<Matrix, SymmGroup> * mpo = NULL)
{
    std::size_t L = mps.length();
    
    MPOTensor<Matrix, SymmGroup> id_mpo;
    if (mpo != NULL)
        id_mpo = *mpo;
    else
        id_mpo = identity_mpo<Matrix>(mps[0].site_dim());
    
    return expval(id_mpo);
}

#endif
