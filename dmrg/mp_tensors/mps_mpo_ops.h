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
    Boundary<Matrix, SymmGroup> left = mps.left_boundary();
    left_[0] = left;
    
    for (int i = 0; i < L; ++i) {
        MPSTensor<Matrix, SymmGroup> bkp = mps[i];
        left = contraction::overlap_mpo_left_step(mps[i], bkp, left, mpo[i]);
        left_[i+1] = left;
//        zout << "Left at " << i+1 << " " << left.data_[0] << endl;
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
    Boundary<Matrix, SymmGroup> right = mps.right_boundary();
    right_[L] = right;
    
    for (int i = L-1; i >= 0; --i) {
        MPSTensor<Matrix, SymmGroup> bkp = mps[i];
        right = contraction::overlap_mpo_right_step(mps[i], bkp, right, mpo[i]);
        right_[i] = right;
//        zout << "right at " << i << " " << right.data_[0] << endl;
    }
    return right_;
}

template<class Matrix, class SymmGroup>
double expval(MPS<Matrix, SymmGroup> const & mps, MPO<Matrix, SymmGroup> const & mpo, int d)
{
    if (d == 0) {
        std::vector<Boundary<Matrix, SymmGroup> > left_ = left_mpo_overlaps(mps, mpo);
        return left_[mps.length()].traces()[0];
    } else {
        std::vector<Boundary<Matrix, SymmGroup> > right_ = right_mpo_overlaps(mps, mpo);
        return right_[0].traces()[0];
    }
}

template<class Matrix, class SymmGroup>
double expval(MPS<Matrix, SymmGroup> const & mps, MPO<Matrix, SymmGroup> const & mpo,
              bool verbose = false)
{
    assert(mpo.length() == mps.length());
    std::size_t L = mps.length();
    
    Boundary<Matrix, SymmGroup> left = mps.left_boundary();
    
    for (int i = 0; i < L; ++i) {
        if (verbose)
            cout << "expval site " << i << endl;
        MPSTensor<Matrix, SymmGroup> bkp = mps[i];
        left = contraction::overlap_mpo_left_step(mps[i], bkp, left, mpo[i]);
    }
    
    return left.traces()[0];
}

template<class Matrix, class SymmGroup>
std::vector<double> multi_expval(MPS<Matrix, SymmGroup> const & mps,
                                 MPO<Matrix, SymmGroup> const & mpo)
{
    assert(mpo.length() == mps.length());
    std::size_t L = mps.length();
    
    Boundary<Matrix, SymmGroup> left = mps.left_boundary();
    
    for (int i = 0; i < L; ++i) {
        MPSTensor<Matrix, SymmGroup> bkp = mps[i];
        left = contraction::overlap_mpo_left_step(mps[i], bkp, left, mpo[i]);
    }
    
    return left.traces();
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

template<class Matrix, class SymmGroup>
std::vector<double>
calculate_bond_renyi_entropies(MPS<Matrix, SymmGroup> & mps, double n)
{
    std::size_t L = mps.length();
    std::vector<double> ret;
    
    mps.normalize_right();
//    mps.canonize(1);
    
    for (std::size_t p = 1; p < L; ++p)
    {
        block_matrix<Matrix, SymmGroup> t, u, v;
        block_matrix<blas::diagonal_matrix<double>, SymmGroup> s;
        
        mps[p-1].make_left_paired();
        mps[p].make_right_paired();
        
        gemm(mps[p-1].data(), mps[p].data(), t);
        
        svd(t, u, v, s);
        
        std::vector<double> sv;
        
        double r = 0;
        for (std::size_t k = 0; k < s.n_blocks(); ++k)
            for (typename blas::diagonal_matrix<double>::element_iterator it = elements(s[k]).first;
                 it != elements(s[k]).second; ++it)
            {
                double a = fabs(*it);
                if (a > 1e-10)
                    sv.push_back(a*a);
            }
        
        r = std::accumulate(sv.begin(), sv.end(), double(0));
        std::transform(sv.begin(), sv.end(), sv.begin(),
                       boost::lambda::_1 / r);
        
//        std::sort(sv.begin(), sv.end());
//        std::reverse(sv.begin(), sv.end());
//        std::copy(sv.begin(), sv.begin()+10, std::ostream_iterator<double>(cout, " ")); cout << endl;
        
        r = 0;
        if (n == 1) {
            for (std::vector<double>::const_iterator it = sv.begin();
                 it != sv.end(); ++it)
                r += *it * log(*it);
            ret.push_back(-r);
        } else {
            for (std::vector<double>::const_iterator it = sv.begin();
                 it != sv.end(); ++it)
                r += pow(*it, n);
            ret.push_back(1/(1-n)*log(r));
        }
        
//        cout << ret.back() << endl;
        
        t = mps[p-1].normalize_left(SVD);
        mps[p].multiply_from_left(t);
    }
    
    return ret;
}

template<class Matrix, class SymmGroup>
std::vector<double>
calculate_bond_entropies(MPS<Matrix, SymmGroup> & mps)
{
    return calculate_bond_renyi_entropies(mps, 1);
}

#endif
