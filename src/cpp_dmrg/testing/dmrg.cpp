#include <cmath>
#include <iterator>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

#include "general_matrix.hpp"
#include "matrix_interface.hpp"
#include "resizable_matrix_interface.hpp"
#include "general_matrix_algorithms.h"
#include "matrix_algorithms.hpp"
typedef blas::general_matrix<double> Matrix;

#include "indexing.h"
#include "mpstensor.h"
#include "mpotensor.h"
#include "contractions.h"

#include "special_mpos.h"

typedef NullGroup grp;

typedef std::vector<MPOTensor<Matrix, grp> > mpo_t;
typedef Boundary<Matrix, grp> boundary_t;

#include <boost/random.hpp>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/io.hpp>

typedef boost::numeric::ublas::vector<double> Vector;

struct SiteProblem
{
    MPSTensor<Matrix, grp> ket_tensor;
    boundary_t left, right;
    MPOTensor<Matrix, grp> mpo;
};

namespace ietl
{
    void mult(SiteProblem const & H, Vector const & x, Vector & y)
    {   
        MPSTensor<Matrix, grp> t1(H.ket_tensor), t2;
        t1.make_left_paired();
        
        Vector::const_iterator cit = x.begin();
        for (std::size_t k = 0; k < t1.data().n_blocks(); ++k) {
            std::copy(cit, cit+num_rows(t1.data()[k])*num_columns(t1.data()[k]), elements(t1.data()[k]).first);
            cit += num_rows(t1.data()[k])*num_columns(t1.data()[k]);
        }
        
        t2 = contraction::site_hamil(t1, H.left, H.right, H.mpo);
        
        Vector::iterator it = y.begin();
        for (std::size_t k = 0; k < t2.data().n_blocks(); ++k)
            it = std::copy(elements(t2.data()[k]).first, elements(t2.data()[k]).second, it);
        
//        cout << x << " " << y << endl;
//        cout << "IP: " << inner_prod(x-y, x-y) << endl;
    }
}

void v2m(Vector const & x, MPSTensor<Matrix, grp> & t1)
{
//    cout << "t1 before: " << t1 << endl;
    
    Vector::const_iterator cit = x.begin();
    for (std::size_t k = 0; k < t1.data().n_blocks(); ++k) {
        std::copy(cit, cit+num_rows(t1.data()[k])*num_columns(t1.data()[k]), elements(t1.data()[k]).first);
        cit += num_rows(t1.data()[k])*num_columns(t1.data()[k]);
    }
    
//    cout << "t1 after: " << t1 << endl;
//    exit(0);
}

#include <ietl/interface/ublas.h>
#include <ietl/lanczos.h>
#include <ietl/vectorspace.h>

struct MPS
{
    MPS(std::size_t L_, std::size_t Mmax)
    : mps_(L_), L(L_)
    {
        std::vector<std::size_t> bond_sizes(L+1, 1);
        for (std::size_t k = 1; k < L+1; ++k)
            bond_sizes[k] = std::min(Mmax, 2*bond_sizes[k-1]);
        bond_sizes[L] = 1;
        for (std::size_t k = L; k > 0; --k) {
            bond_sizes[k-1] = std::min(bond_sizes[k-1], 2*bond_sizes[k]);
        }
        std::copy(bond_sizes.begin(), bond_sizes.end(), std::ostream_iterator<int>(cout, " "));
        cout << endl;
        
        Index<grp> phys; phys.insert(std::make_pair(NullGroup::Plus, 2));
        
        for (int i = 0; i < L; ++i)
        {
            Index<grp> li; li.insert(std::make_pair(NullGroup::Plus, bond_sizes[i]));
            Index<grp> ri; ri.insert(std::make_pair(NullGroup::Plus, bond_sizes[i+1]));
            
            mps_[i] = MPSTensor<Matrix, grp>(phys, li, ri);
        }
        
        for (int i = 0; i < L; ++i)
            mps_[i].normalize_left(SVD);
//        for (int i = 0; i < L; ++i)
//            mps_[i].normalize_right(SVD);
    }
    
    boundary_t start_mtx()
    {
        Index<grp> i; i.insert(std::make_pair(NullGroup::Plus, 1));
        boundary_t ret(i, i, 1);
        ret(0, std::make_pair(NullGroup::Plus, 0), std::make_pair(NullGroup::Plus, 0)) = 1;
        return ret;
    }
    
    std::vector<boundary_t> left_mpo_overlaps(mpo_t mpo)
    {
        std::vector<boundary_t> left_(L+1);
        boundary_t left = start_mtx();
        left_[0] = left;
        
        for (int i = 0; i < L; ++i) {
            MPSTensor<Matrix, grp> bkp = mps_[i];
            left = contraction::overlap_mpo_left_step(mps_[i], bkp, left, mpo[i]);
            left_[i+1] = left;
        }
        return left_;
    }
    
    std::vector<boundary_t> right_mpo_overlaps(mpo_t mpo)
    {
        std::vector<boundary_t> right_(L+1);
        boundary_t right = start_mtx();
        right_[L] = right;
        
        for (int i = L-1; i >= 0; --i) {
            MPSTensor<Matrix, grp> bkp = mps_[i];
            right = contraction::overlap_mpo_right_step(mps_[i], bkp, right, mpo[i]);
            right_[i] = right;
        }
        return right_;
    }  
    
    double norm(MPOTensor<Matrix, grp> * mpo = NULL)
    {
        MPOTensor<Matrix, grp> id_mpo;
        if (mpo != NULL)
            id_mpo = *mpo;
        else
            id_mpo = identity_mpo<Matrix>(mps_[0].site_dim());
        
        std::vector<boundary_t> left_ = left_mpo_overlaps(mpo_t(L, id_mpo));
        return left_[L](0, std::make_pair(NullGroup::Plus, 0), std::make_pair(NullGroup::Plus, 0));
    }
    
    void renormalize()
    {
        block_matrix<Matrix, grp> t;
        for (int i = 0; i < L; ++i) {
            t = mps_[i].normalize_left(SVD);
            if (i < L-1)
                mps_[i+1].multiply_from_left(t);
        }
        mps_[L-1].multiply_by_scalar(trace(t));
        cout << "renorm norm: " << norm() << endl;
    }

    
    double expval(mpo_t mpo)
    {
        std::vector<boundary_t> left_ = left_mpo_overlaps(mpo);
        return left_[L](0, std::make_pair(NullGroup::Plus, 0), std::make_pair(NullGroup::Plus, 0));
        
//        std::vector<boundary_t> right_ = right_mpo_overlaps(mpo);
//        return trace(right_[0]);
    }
    
    void normalize_upto(int w)
    {
        for (int i = 0; i < w; ++i)
        {
            block_matrix<Matrix, grp> t = mps_[i].normalize_left(SVD);
            mps_[i+1].multiply_from_left(t);
        }
        
        for (int i = L-1; i > w; --i)
        {
            block_matrix<Matrix, grp> t = mps_[i].normalize_right(SVD);
            mps_[i-1].multiply_from_right(t);
        }
    }
    
    void optimize(std::vector<MPOTensor<Matrix, grp> > mpo)
    {
        std::vector<boundary_t>
        left_ = left_mpo_overlaps(mpo),
        right_ = right_mpo_overlaps(mpo);
        normalize_upto(0);
        
        for (int sweep = 0; sweep < 50; ++sweep) {
            // turn off if you feel numerical errors accumulate
            // which they shouldn't
//            renormalize();
//            normalize_upto(0);
            
            for (int _site = 0; _site < 2*L; ++_site) {
                int site, lr;
                if (_site < L) {
                    site = _site;
                    lr = 1;
                } else {
                    site = 2*L-_site-1;
                    lr = -1;
                }
                
                cout << "Sweep " << sweep << ", optimizing site " << site << endl;
//                normalize_upto(site);
//                mps_[site].multiply_by_scalar(1/norm());
//                left_ = left_mpo_overlaps(mpo);
//                right_ = right_mpo_overlaps(mpo);
                
//                cout << "Normerr: " << norm() - 1 << endl;
//                assert( fabs(norm() - 1) < 1e-6 );
                
                unsigned int N = 0;
                for (std::size_t k = 0; k < mps_[site].data().n_blocks(); ++k)
                    N += num_rows(mps_[site].data()[k]) * num_columns(mps_[site].data()[k]);
                ietl::vectorspace<Vector> vs(N);
                boost::lagged_fibonacci607 gen;
                
                // This should not be necessary here.
                // If you find out why it is, let me know.
                mps_[site].make_left_paired();
                SiteProblem sp;
                sp.ket_tensor = mps_[site];
                sp.mpo = mpo[site];
                sp.left = left_[site];
                sp.right = right_[site+1];
                
                typedef ietl::vectorspace<Vector> Vecspace;
                typedef boost::lagged_fibonacci607 Gen;  
                
                Vecspace vec(N);
                Gen mygen;
                ietl::lanczos<SiteProblem,Vecspace> lanczos(sp,vec);
                
                double rel_tol = 500*std::numeric_limits<double>::epsilon();
                double abs_tol = std::pow(std::numeric_limits<double>::epsilon(),2./3);
                ietl::lanczos_iteration_nlowest<double> 
//                iter(max_iter, n_lowest_eigenval, rel_tol, abs_tol);
                iter(100, 1, rel_tol, abs_tol);
                
                std::vector<double> eigen, err;
                std::vector<int> multiplicity;  
                
                try{
                    lanczos.calculate_eigenvalues(iter,mygen);
                    eigen = lanczos.eigenvalues();
                    err = lanczos.errors();
                    multiplicity = lanczos.multiplicities();
                    std::cout << "number of iterations: " << iter.iterations() << "\n";
                }
                catch (std::runtime_error& e) {
                    cout << "Error in eigenvalue calculation: " << endl;
                    cout << e.what() << endl;
                }
                
                cout << "Energy: " << eigen[0] << endl;
                
                std::vector<double>::iterator start = eigen.begin();  
                std::vector<double>::iterator end = eigen.begin()+1;
                std::vector<Vector> eigenvectors; // for storing the eigen vectors. 
                ietl::Info<double> info; // (m1, m2, ma, eigenvalue, residualm, status).
                
                try {
                    lanczos.eigenvectors(start,end,std::back_inserter(eigenvectors),info,mygen); 
                }
                catch (std::runtime_error& e) {
                    cout << "Error in eigenvector calculation: " << endl;
                    cout << e.what() << endl;
                }  
                
                v2m(eigenvectors[0], mps_[site]);
                
                if (lr == +1) {
                    block_matrix<Matrix, grp> t = mps_[site].normalize_left(SVD);
                    if (site < L-1)
                        mps_[site+1].multiply_from_left(t);

                    
                    MPSTensor<Matrix, grp> bkp = mps_[site];
                    left_[site+1] = contraction::overlap_mpo_left_step(mps_[site], bkp,
                                                                       left_[site], mpo[site]);
                } else if (lr == -1) {
                    block_matrix<Matrix, grp> t = mps_[site].normalize_right(SVD);
                    if (site > 0)
                        mps_[site-1].multiply_from_right(t);
                    
                    MPSTensor<Matrix, grp> bkp = mps_[site];
                    right_[site] = contraction::overlap_mpo_right_step(mps_[site], bkp,
                                                                       right_[site+1], mpo[site]);
                }
            }
        }
    }
    
    std::size_t L;
    std::vector<MPSTensor<Matrix, grp> > mps_;
};

int main()
{
    int L = 32, M = 20;
    MPS mps(L, M);
    
    MPOTensor<Matrix, grp> id_mpo = identity_mpo<Matrix>(mps.mps_[0].site_dim());
//    cout << mps.norm() << endl;
//    mps.normalize_upto(3);
//    cout << mps.norm() << endl;
    
    MPOTensor<Matrix, grp> sz_mpo = s12_sz_mpo<Matrix>(mps.mps_[0].site_dim());
//    for (int k = 0; k < L; ++k) {
//        cout << mps.stupid_site_expval(id_mpo, k) << " ";
//        cout << mps.stupid_site_expval(sz_mpo, k) << endl;
//    }
    
//    cout << mps.norm(&sz_mpo) << endl;
//    mps.optimize(mpo_t(L, id_mpo));
//    cout << "Sz: " << mps.norm(&sz_mpo) << endl;
//    cout << "Norm: " << mps.norm() << endl;

    mpo_t szsz = s12_ising<Matrix>(L, -1, 1);
//    mpo_t szsz(L, id_mpo);
//    cout << mps.expval(szsz) << endl;
    mps.optimize(szsz);
//    
//    mpo_t magn = s12_ising<Matrix>(L, 0, 1);
//    cout << mps.expval(magn) << endl;
}
