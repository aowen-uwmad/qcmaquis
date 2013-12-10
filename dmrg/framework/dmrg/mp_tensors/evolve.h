/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2013 Institute for Theoretical Physics, ETH Zurich
 *               2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 * 
 * This software is part of the ALPS Applications, published under the ALPS
 * Application License; you can use, redistribute it and/or modify it under
 * the terms of the license, either version 1 or (at your option) any later
 * version.
 * 
 * You should have received a copy of the ALPS Application License along with
 * the ALPS Applications; see the file LICENSE.txt. If not, the license is also
 * available from http://alps.comp-phys.org/.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef EVOLVE_H
#define EVOLVE_H

#include "dmrg/mp_tensors/mpstensor.h"
#include "dmrg/mp_tensors/mpotensor.h"

#include "dmrg/mp_tensors/reshapes.h"
#include "dmrg/block_matrix/indexing.h"

#include "dmrg/mp_tensors/compression.h"
#include "dmrg/mp_tensors/contractions.h"


template<class Matrix, class SymmGroup>
MPS<Matrix, SymmGroup>
evolve(MPS<Matrix, SymmGroup> mps,
       block_matrix<Matrix, SymmGroup> const & op,
       std::size_t Mmax, double cutoff)
{
    std::size_t L = mps.length();
    
    for (int i = 0; i < 2; ++i) { // odd/even
        mps.canonize(i+1);
        block_matrix<Matrix, SymmGroup> v0, v1, t;
        
        for (std::size_t p = i; p < L-1; p += 2)
        {
//            maquis::cout << "Doing " << p << " " << p+1 << std::endl;
            
            mps[p].make_left_paired();
            mps[p+1].make_right_paired();
            
            gemm(mps[p].data(), mps[p+1].data(), v0);
            
            v1 = contraction::multiply_with_twosite(v0, op,
                                                    mps[p].row_dim(), mps[p+1].col_dim(),
                                                    mps[p].site_dim());
            
            compression::replace_two_sites_l2r(mps, Mmax, cutoff, v1, p);
            
            // move two to the right, if possible
            t = mps[p+1].normalize_left(DefaultSolver());
            if (p+2 < L) {
                mps[p+2].multiply_from_left(t);
                t = mps[p+2].normalize_left(DefaultSolver());
                if (p+3 < L) {
                    mps[p+3].multiply_from_left(t);
                }
            }
        }
        
//        maquis::cout << "Norm loss " << i << ": " << trace(t)-1.0 << " " << -log(trace(t)) << std::endl;
    }
    
    return mps;
}

// Same function, but working with different matrix on each bond.
// map_op should already contain non overlapping terms.
template<class Matrix, class SymmGroup>
truncation_results evolve_l2r(MPS<Matrix, SymmGroup> & mps,
                              std::vector<block_matrix<Matrix, SymmGroup> > const & ops,
                              std::vector<long> const & idx,
                              int pfirst, std::size_t Mmax, double cutoff)
{
    assert(mps.length() == idx.size());
    std::size_t L = mps.length();
    
    MPS<Matrix, SymmGroup> const& constmps = mps;
    
    if (mps.canonization() != pfirst + 1)
        mps.canonize(pfirst + 1);

    for (std::size_t p = pfirst; p <= L-1; p += 2)
    {
        if (idx[p] != -1)
        {
            constmps[p].make_left_paired();
            constmps[p+1].make_right_paired();
            
            block_matrix<Matrix, SymmGroup> v0, v1;
            gemm(constmps[p].data(), constmps[p+1].data(), v0); // outer product of two sites
            
            v1 = contraction::multiply_with_twosite(v0, ops[idx[p]],
                                                    constmps[p].row_dim(), constmps[p+1].col_dim(),
                                                    constmps[p].site_dim());
            compression::replace_two_sites_l2r(mps, Mmax, cutoff, v1, p);
        }
        mps.move_normalization_l2r(p+1, p+3, DefaultSolver());
    }
    mps.canonization(true);
    assert(mps.canonization() == mps.length()-1);
    // maquis::cout << "Norm loss " << i << ": " << trace(t) << " " << -log(trace(t)) << std::endl;
}

template<class Matrix, class SymmGroup>
void
evolve_r2l(MPS<Matrix, SymmGroup> & mps,
           std::vector<block_matrix<Matrix, SymmGroup> > const & ops,
           std::vector<long> const & idx,
           int pfirst, std::size_t Mmax, double cutoff)
{
    assert(mps.length() == idx.size());
    std::size_t L = mps.length();

    MPS<Matrix, SymmGroup> const& constmps = mps;
    
    int startpos = std::min(L-2-(L-pfirst)%2, L-2);
    if (mps.canonization() != startpos)
        mps.canonize(startpos);

    for (int p = std::min(L-2-(L-pfirst)%2, L-2); p >= pfirst; p -= 2)
    {
        if (idx[p] != -1)
        {
            mps[p].make_left_paired();
            mps[p+1].make_right_paired();

            block_matrix<Matrix, SymmGroup> v0, v1;
            gemm(constmps[p].data(), constmps[p+1].data(), v0); // outer product of two sites
            
            v1 = contraction::multiply_with_twosite(v0, ops[idx[p]],
                                                    constmps[p].row_dim(), constmps[p+1].col_dim(),
                                                    constmps[p].site_dim());
            compression::replace_two_sites_r2l(mps, Mmax, cutoff, v1, p);
        }
        mps.move_normalization_r2l(p, std::max(static_cast<long>(p)-2,0L), DefaultSolver());
    }
    
    
    mps.canonization(true);
    assert(mps.canonization() == 0);
    // maquis::cout << "Norm loss " << i << ": " << trace(t) << " " << -log(trace(t)) << std::endl;
}

#endif
