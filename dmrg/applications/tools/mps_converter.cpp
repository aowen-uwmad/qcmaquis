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

#include <cmath>
#include <iterator>
#include <iostream>
#include <sys/time.h>
#include <sys/stat.h>

using std::cerr;
using std::cout;
using std::endl;

#include "dmrg/block_matrix/detail/ambient.hpp"
#include "dmrg/block_matrix/detail/alps.hpp"

typedef alps::numeric::matrix<double> alps_matrix;
typedef ambient::tiles<ambient::matrix<double> > ambient_matrix;

#include "dmrg/block_matrix/block_matrix.h"
#include "dmrg/block_matrix/block_matrix_algorithms.h"

#include "dmrg/block_matrix/indexing.h"
#include "dmrg/mp_tensors/mps.h"
#include "dmrg/mp_tensors/mpo.h"
#include "dmrg/mp_tensors/contractions.h"
#include "dmrg/mp_tensors/mps_mpo_ops.h"
#include "dmrg/mp_tensors/mpo_ops.h"

#include "utils/bindings.hpp"

#if defined(USE_NONE)
typedef TrivialGroup grp;
#elif defined(USE_U1)
typedef U1 grp;
#elif defined(USE_TWOU1)
typedef TwoU1 grp;
#else
#error "SymmGroup has to be defined explicitly. (-DUSE_NONE, -DUSE_U1, -DUSE_TWOU1)"
#endif


int main(int argc, char ** argv)
{
    try {
        if (argc != 3) {
            std::cout << "Usage: " << argv[0] << " <in.h5> <out.h5>" << std::endl;
            return 1;
        }
        MPS<ambient_matrix, grp> mps_in;
        
        {
            storage::archive ar(argv[1]);
            ar["/state"] >> mps_in;
        }
        
        MPS<alps_matrix, grp> mps_out(mps_in.length());
        for (int i=0; i<mps_in.length(); ++i) {
            mps_in[i].make_left_paired();
            block_matrix<alps_matrix, grp> m;
            for (int k=0; k<mps_in[i].data().n_blocks(); ++k)
                m.insert_block(maquis::bindings::matrix_cast<alps_matrix>(mps_in[i].data()[k]), mps_in[i].data().left_basis()[k].first, mps_in[i].data().right_basis()[k].first);
            mps_out[i] = MPSTensor<alps_matrix, grp>(mps_in[i].site_dim(),mps_in[i].row_dim(), mps_in[i].col_dim(), m, LeftPaired);
        }
        
        {
            storage::archive ar(argv[2], "w");
            ar["/state"] << mps_out;
        }
        
    } catch (std::exception& e) {
        std::cerr << "Error:" << std::endl << e.what() << std::endl;
        return 1;
    }
}
