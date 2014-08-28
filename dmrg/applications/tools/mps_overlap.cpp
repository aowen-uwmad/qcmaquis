/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2011-2013 by Michele Dolfi <dolfim@phys.ethz.ch>
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

#ifdef USE_AMBIENT
#include <mpi.h>
#endif
#include <cmath>
#include <iterator>
#include <iostream>
#include <sys/time.h>
#include <sys/stat.h>

using std::cerr;
using std::cout;
using std::endl;

#ifdef USE_AMBIENT
    #include "dmrg/block_matrix/detail/ambient.hpp"
    typedef ambient::numeric::tiles<ambient::numeric::matrix<double> > Matrix;
#else
    #include "dmrg/block_matrix/detail/alps.hpp"
    typedef alps::numeric::matrix<double> Matrix;
#endif

#include "dmrg/block_matrix/indexing.h"
#include "dmrg/mp_tensors/mps.h"
#include "dmrg/mp_tensors/mpo.h"
#include "dmrg/mp_tensors/contractions.h"
#include "dmrg/mp_tensors/mps_mpo_ops.h"
#include "dmrg/mp_tensors/mpo_ops.h"

#ifdef USE_TWOU1
typedef TwoU1 grp;
#else
#ifdef USE_NONE
typedef TrivialGroup grp;
#else
typedef U1 grp;
#endif
#endif

template<class Matrix, class SymmGroup>
Boundary<Matrix, SymmGroup>
mixed_left_boundary(MPS<Matrix, SymmGroup> const & bra, MPS<Matrix, SymmGroup> const & ket)
{
    assert(ket.length() == bra.length());
    Index<SymmGroup> i = ket[0].row_dim();
    Index<SymmGroup> j = bra[0].row_dim();
    Boundary<Matrix, SymmGroup> ret(i, j, 1);
    
    //   for(std::size_t k(0); k < ret[0].n_blocks(); ++k)
    //       detail::iterable_matrix_impl<Matrix, SymmGroup>::left_right_boundary_init_impl(ret[0][k]);       
    for(typename Index<SymmGroup>::basis_iterator it1 = i.basis_begin(); !it1.end(); ++it1)
    	for(typename Index<SymmGroup>::basis_iterator it2 = j.basis_begin(); !it2.end(); ++it2)
            ret[0](*it1, *it2) = 1;
    
    return ret;
}


template<class Matrix, class SymmGroup>
typename Matrix::value_type expval(MPS<Matrix, SymmGroup> const & bra,
              MPS<Matrix, SymmGroup> const & ket,
    	      MPO<Matrix, SymmGroup> const & mpo,
              bool verbose = false)
{
    assert(mpo.length() == bra.length() && bra.length() == ket.length());
    std::size_t L = bra.length();
    
    Boundary<Matrix, SymmGroup> left = mixed_left_boundary(bra, ket);
    
    for (int i = 0; i < L; ++i) {
        if (verbose)
            std::cout << "expval site " << i << std::endl;
        left = contraction::overlap_mpo_left_step(bra[i], ket[i], left, mpo[i]);
    }
    
    // MD: if bra and ket are different, result might be complex!
    return left.traces()[0];
}


int main(int argc, char ** argv)
{
    try {
        if (argc != 3) {
            std::cout << "Usage: " << argv[0] << " <mps1.h5> <mps2.h5>" << std::endl;
            return 1;
        }
        MPS<Matrix, grp> mps1, mps2;
        load(argv[1], mps1);
        load(argv[2], mps2);
        
        if (true) {
            std::cout << "<mps1 | mps2> = " << overlap(mps1, mps2) << std::endl;
            std::cout << "<mps2 | mps1> = " << overlap(mps2, mps1) << std::endl;
        }
        
        if (false) {
            block_matrix<Matrix, grp> ident;
            for (int i=0; i<mps1.site_dim(0).size(); ++i)
                ident.insert_block(Matrix::identity_matrix(mps1.site_dim(0)[i].second),
                                   mps1.site_dim(0)[i].first, mps1.site_dim(0)[i].first);
            
            MPO<Matrix, grp> mpo;
            
            MPOTensor<Matrix, grp> mpot;
            mpot.set(0,0, ident);
            mpo = MPO<Matrix, grp>(mps1.length());
            for (int p=0; p<mps1.length(); ++p)
                mpo[p] = mpot;
            
            std::cout << "<mps1 | 1 | mps2> = " << expval(mps1, mps2, mpo) << std::endl;
            std::cout << "<mps2 | 1 | mps1> = " << expval(mps2, mps1, mpo) << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Error:" << std::endl << e.what() << std::endl;
        return 1;
    }
}
