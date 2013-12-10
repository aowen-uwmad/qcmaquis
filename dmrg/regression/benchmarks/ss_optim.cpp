/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2013 by Michele Dolfi <dolfim@phys.ethz.ch>
 *                            Bela Bauer <bauerb@comp-phys.org>
 *
 *****************************************************************************/

#ifdef USE_AMBIENT
#include <mpi.h>
#endif
#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/shared_ptr.hpp>

#include <alps/hdf5.hpp>

#include "matrix_selector.hpp" /// define matrix
#include "symm_selector.hpp"   /// define grp

#include "dmrg/models/lattice.h"
#include "dmrg/models/model.h"
#include "dmrg/models/generate_mpo.hpp"

#include "dmrg/mp_tensors/mps.h"

#include "dmrg/mp_tensors/ietl_lanczos_solver.h"
#include "dmrg/mp_tensors/ietl_jacobi_davidson.h"

#include "dmrg/utils/DmrgParameters2.h"

#include "utils/timings.h"


template<class Matrix, class SymmGroup>
struct SiteProblem
{
    SiteProblem(Boundary<Matrix, SymmGroup> const & left_,
                Boundary<Matrix, SymmGroup> const & right_,
                MPOTensor<Matrix, SymmGroup> const & mpo_)
    : left(left_)
    , right(right_)
    , mpo(mpo_)
    {
    }
    
    Boundary<Matrix, SymmGroup> const & left;
    Boundary<Matrix, SymmGroup> const & right;
    MPOTensor<Matrix, SymmGroup> const & mpo;
    double ortho_shift;
};


int main(int argc, char ** argv)
{
    try {
        if (argc != 3)
            throw std::runtime_error("Usage: <parms> <model_parms>");
        
        /// Loading parameters
        std::ifstream param_file(argv[1]);
        if (!param_file)
            throw std::runtime_error("Could not open parameter file.");
        DmrgParameters parms(param_file);
        
        /// Loading model
        std::ifstream model_file(argv[2]);
        if (!model_file)
            throw std::runtime_error("Could not open model file.");
        ModelParameters model_parms(model_file);
        
        /// Timers
        Timer tim_model      ("Parsing model"),  tim_load        ("Load MPS");
        Timer tim_l_boundary ("Left boundary"),  tim_r_boundary  ("Right boundary");
        Timer tim_optim_jcd  ("Optim. JCD"   ),  tim_optim_alpha ("Optim. alpha");
        
        /// Parsing model
        tim_model.begin();
        Lattice lattice(parms, model_parms);
        Model<matrix, grp> model(lattice, parms, model_parms);
        
        MPO<matrix, grp> mpo = make_mpo(lattice, model, model_parms);
        tim_model.end();
        
        
        /// Initialize & load MPS
        tim_load.begin();
        int L = lattice.size();
        MPS<matrix, grp> mps;
        load(parms["chkpfile"].str(), mps);
        int _site;
        {
            alps::hdf5::archive ar(parms["chkpfile"].str()+"/props.h5");
            ar["/status/site"] >> _site;
        }
        int site, lr;
        if (_site < L) {
            site = _site;
            lr = 1;
        } else {
            site = 2*L-_site-1;
            lr = -1;
        }
        maquis::cout << "Optimization at site " << site << " in " << lr << " direction." << std::endl;
        tim_load.end();
        
        /// Canonize MPS
        mps.canonize(site);
        
        /// Compute left boundary
        tim_l_boundary.begin();
        Boundary<matrix, grp> left = mps.left_boundary();
        for (size_t i=0; i<site; ++i)
            left = contraction::overlap_mpo_left_step(mps[i], mps[i], left, mpo[i]);
        tim_l_boundary.end();
        
        /// Compute right boundary
        tim_r_boundary.begin();
        Boundary<matrix, grp> right = mps.right_boundary();
        for (int i=L-1; i>site; --i)
            right = contraction::overlap_mpo_right_step(mps[i], mps[i], right, mpo[i]);
        tim_r_boundary.end();


        /// Optimization
        std::vector<MPSTensor<matrix, grp> > ortho_vecs;
        std::pair<double, MPSTensor<matrix, grp> > res;
        SiteProblem<matrix, grp> sp(left, right, mpo[site]);
        
        /// Optimization: JCD
        tim_optim_jcd.begin();
        res = solve_ietl_jcd(sp, mps[site], parms, ortho_vecs);
        mps[site] = res.second;
        maquis::cout.precision(10);
        maquis::cout << "Energy " << lr << " " << res.first << std::endl;
        tim_optim_jcd.end();
        
        double alpha = parms["alpha_main"];
        double cutoff = parms["truncation_final"];
        std::size_t Mmax = parms["max_bond_dimension"];
        
        /// Optimization: grow alpha
        std::pair<std::size_t, double> trunc;
        tim_optim_alpha.begin();
        if (lr == +1) {
            if (site < L-1) {
                maquis::cout << "Growing, alpha = " << alpha << std::endl;
                mps.grow_l2r_sweep(mpo[site], left, right, site, alpha, cutoff, Mmax);
            } else {
                block_matrix<matrix, grp> t = mps[site].normalize_left(DefaultSolver());
                if (site < L-1)
                    mps[site+1].multiply_from_left(t);
            }
        } else if (lr == -1) {
            if (site > 0) {
                maquis::cout << "Growing, alpha = " << alpha << std::endl;
                mps.grow_r2l_sweep(mpo[site], left, right, site, alpha, cutoff, Mmax);
            } else {
                block_matrix<matrix, grp> t = mps[site].normalize_right(DefaultSolver());
                if (site > 0)
                    mps[site-1].multiply_from_right(t);
            }
        }
        tim_optim_alpha.end();
        
        /// Compute new boundary
        // TODO: optional here...
        
        
    } catch (std::exception & e) {
        maquis::cerr << "Exception caught:" << std::endl << e.what() << std::endl;
        exit(1);
    }
}

