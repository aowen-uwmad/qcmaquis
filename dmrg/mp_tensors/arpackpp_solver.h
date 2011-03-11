/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *
 *****************************************************************************/
 
#ifndef ARPACKPP_SOLVER_H
#define ARPACKPP_SOLVER_H

#include "utils/zout.hpp"
#include "utils/DmrgParameters.h"

namespace arpack {
    
#include "arpack++/make_me_header_only.hpp"
#include "arpack++/arssym.h"    
    
}

template<class Matrix, class SymmGroup> struct SiteProblem;

template<class Matrix, class SymmGroup>
void copy_mps_to_ptr(MPSTensor<Matrix, SymmGroup> & mps,
                     typename Matrix::value_type * ptr)
{
    static Timer timer("copy_mps_to_ptr");
    timer.begin();
    mps.make_left_paired();
    for (std::size_t k = 0; k < mps.data().n_blocks(); ++k)
        for (typename Matrix::element_iterator it = elements(mps.data()[k]).first;
             it != elements(mps.data()[k]).second; ++it)
            *(ptr++) = *it;
    timer.end();
}

template<class Matrix, class SymmGroup>
void copy_ptr_to_mps(typename Matrix::value_type * ptr,
                     MPSTensor<Matrix, SymmGroup> & mps)
{
    static Timer timer("copy_ptr_to_mps");
    timer.begin();
    mps.make_left_paired();
    for (std::size_t k = 0; k < mps.data().n_blocks(); ++k)
        for (typename Matrix::element_iterator it = elements(mps.data()[k]).first;
             it != elements(mps.data()[k]).second; ++it)
            *it = *(ptr++);
    timer.end();
}

template<class Matrix, class SymmGroup>
class ArpackppMatrix
{
public:
    ArpackppMatrix(SiteProblem<Matrix, SymmGroup> & prob_)
    : prob(prob_)
    {
        N = 0;
        for (std::size_t k = 0; k < prob.ket_tensor.data().n_blocks(); ++k)
            N += num_rows(prob.ket_tensor.data()[k]) * num_columns(prob.ket_tensor.data()[k]);
    }
    
    int getN() { return N; }
    
    void MultMv(double * v, double * w)
    {
        MPSTensor<Matrix, SymmGroup> vec = prob.ket_tensor, vec2, vec3;
        
        copy_ptr_to_mps(v, vec);
//        vec2 = contraction::site_hamil(vec, prob.left, prob.right, prob.mpo);
//        zout << "vec2 " << vec2 << endl;
        static Timer timer("site_hamil2 from ARPACK");
        timer.begin();
        vec3 = contraction::site_hamil2(vec, prob.left, prob.right, prob.mpo);
        timer.end();
//        zout << "vec3 " << vec3 << endl;
        copy_mps_to_ptr(vec3, w);
//        zout << endl << endl;
    }
    
private:
    SiteProblem<Matrix, SymmGroup> & prob;
    std::size_t N;
};

template<class Matrix, class SymmGroup>
std::pair<double, MPSTensor<Matrix, SymmGroup> >
solve_arpackpp(SiteProblem<Matrix, SymmGroup> & sp,
               MPSTensor<Matrix, SymmGroup> initial,
               BaseParameters & parms)
{
    ArpackppMatrix<Matrix, SymmGroup> mtx(sp);
    
    std::vector<double> initial_ptr(mtx.getN());
    copy_mps_to_ptr(initial, &initial_ptr[0]);
   
    int ncv = std::min(mtx.getN(), parms.get<int>("arpack_ncv"));
    static Timer timer("All of ARPACK");
    timer.begin();
    arpack::ARSymStdEig<double, ArpackppMatrix<Matrix, SymmGroup> >
    solver(mtx.getN(), 1,
           &mtx, &ArpackppMatrix<Matrix, SymmGroup>::MultMv, "SA",
           // Parameters are: ncvp, tolp, maxitp, residp
           ncv, parms.get<double>("arpack_tol"), 1000, &initial_ptr[0]);
    
    try {
        int nconv = solver.FindEigenvalues();
        nconv = solver.FindEigenvectors();
        
        zout << "ARPACK used " << solver.GetIter() << " iterations." << endl;
        if (solver.GetIter() <= 2)
            parms.set<int>("arpack_ncv",
                           std::max(5, ncv-1));
        else if (solver.GetIter() > 4)
            parms.set<int>("arpack_ncv",
                           std::min(50, ncv+4));
        zout << "Setting ncv = " << parms.get<int>("arpack_ncv") << endl;
        
        std::vector<double> evals;
        for (int i = 0; i < nconv; ++i)
            evals.push_back( solver.Eigenvalue(i) );
        
        std::vector<double> evec;
        for (int i = 0; i < mtx.getN(); ++i)
            evec.push_back( solver.Eigenvector(0, i) );
        
        copy_ptr_to_mps(&evec[0], initial);
        
        timer.end();
        return std::make_pair(evals[0], initial);
    } catch (arpack::ArpackError & e) {
        zout << "Error in ARPACK." << endl;
        return std::make_pair(0, initial);
    }
}

#endif
