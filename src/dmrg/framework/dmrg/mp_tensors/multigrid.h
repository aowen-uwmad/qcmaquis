/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *                            Michele Dolfi <dolfim@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef MULTIGRID_H
#define MULTIGRID_H

#include "dmrg/mp_tensors/mpstensor.h"
#include "dmrg/block_matrix/indexing.h"

#include "dmrg/mp_tensors/optimize.h"

struct multigrid {
    
    template<class Matrix, class SymmGroup>
    void
    static restriction (MPS<Matrix, SymmGroup> const & mps_large,
                        MPS<Matrix, SymmGroup> & mps_small)
    {
        std::size_t L = mps_small.length();
        std::size_t LL = mps_large.length();
        assert(LL == 2*L);
        
        typedef typename SymmGroup::charge charge;
        typedef typename Index<SymmGroup>::basis_iterator bi_t;
        
        
        for (std::size_t p = 0; p < L; ++p)
        {
            //            maquis::cout << std::endl << std::endl << "********************" << std::endl << "starting loop for p = " << p << std::endl;
            
            block_matrix<Matrix, SymmGroup> M;
            
            Index<SymmGroup> alpha_basis = mps_large.row_dim(2*p);
            Index<SymmGroup> b_basis = mps_large.col_dim(2*p);
            Index<SymmGroup> beta_basis = mps_large.col_dim(2*p+1);
            Index<SymmGroup> s_basis = mps_small.site_dim(p);
            Index<SymmGroup> s1_basis = mps_large.site_dim(2*p);
            Index<SymmGroup> s2_basis = mps_large.site_dim(2*p+1);
            
            //            maquis::cout << "alpha_basis:" <<std::endl << alpha_basis << std::endl;
            //            maquis::cout << "beta_basis:" <<std::endl << beta_basis << std::endl;
            //            maquis::cout << "s_basis:" <<std::endl << s_basis << std::endl;
            //            maquis::cout << "s1_basis:" <<std::endl << s1_basis << std::endl;
            //            maquis::cout << "s2_basis:" <<std::endl << s2_basis << std::endl;
            
            
            mps_large[2*p].make_left_paired();
            mps_large[2*p+1].make_right_paired();
            
            ProductBasis<SymmGroup> in1_left(s1_basis, alpha_basis);
            ProductBasis<SymmGroup> in2_right(s2_basis, beta_basis,
                                              boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                                                  -boost::lambda::_1, boost::lambda::_2));
            
            ProductBasis<SymmGroup> out_left(s_basis, alpha_basis);
            
            for (bi_t alpha = alpha_basis.basis_begin(); !alpha.end(); ++alpha)
                for (bi_t beta = beta_basis.basis_begin(); !beta.end(); ++beta)
                    for (bi_t b = b_basis.basis_begin(); !b.end(); ++b)
                        for (bi_t s1 = s1_basis.basis_begin(); !s1.end(); ++s1)
                            for (bi_t s2 = s2_basis.basis_begin(); !s2.end(); ++s2)
                            {
                                std::pair<charge, std::size_t> s(SymmGroup::fuse(s1->first, s2->first),
                                                                 s1->second*s2_basis.size_of_block(s2->first)+s2->second);
                                
                                if (!s_basis.has(s.first) || s_basis.size_of_block(s.first) <= s.second)
                                    continue;
                                
                                charge in1_left_c = SymmGroup::fuse(s1->first, alpha->first);
                                charge in1_right_c = b->first;
                                
                                charge in2_left_c = b->first;
                                charge in2_right_c = SymmGroup::fuse(-s2->first, beta->first);
                                
                                if (!mps_large[2*p].data().has_block(in1_left_c, in1_right_c))
                                    continue;
                                if (!mps_large[2*p+1].data().has_block(in2_left_c, in2_right_c))
                                    continue;
                                
                                charge out_left_c = SymmGroup::fuse(s.first, alpha->first);
                                charge out_right_c = beta->first;
                                
                                //                            maquis::cout << "--" << std::endl;
                                //                            maquis::cout << "alpha: " << alpha->first << ", " << alpha->second << std::endl;
                                //                            maquis::cout << "beta: " << beta->first << ", " << beta->second << std::endl;
                                //                            maquis::cout << "s1: " << s1->first << ", " << s1->second << std::endl;
                                //                            maquis::cout << "s2: " << s2->first << ", " << s2->second << std::endl;
                                //                            maquis::cout << "s: " << s.first << ", " << s.second << std::endl;
                                
                                if (!M.has_block(out_left_c, out_right_c))
                                    M.insert_block(new Matrix(out_left.size(s.first, alpha->first),
                                                              beta_basis.size_of_block(beta->first),
                                                              0),
                                                   out_left_c, out_right_c);
                                
                                
                                //                            maquis::cout << "block has size " << out_left.size(s1->first, alpha->first)
                                //                            << "x"
                                //                            << out_right.size(s2->first, beta->first,
                                //                                              boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                //                                                                  -boost::lambda::_1, boost::lambda::_2)) << std::endl;
                                
                                std::size_t in1_left_offset = in1_left(s1->first, alpha->first);
                                std::size_t in1_right_offset = 0;
                                std::size_t in2_left_offset = 0;
                                std::size_t in2_right_offset = in2_right(s2->first, beta->first);
                                
                                std::size_t out_left_offset = out_left(s.first, alpha->first);
                                std::size_t out_right_offset = 0;
                                
                                //                            maquis::cout << "M[" << out_left_c << ", " << out_right_c << "]"
                                //                            << "(" << out_left_offset + s1->second*alpha_basis.size_of_block(alpha->first)+alpha->second << ", " << out_right_offset + s2->second*beta_basis.size_of_block(beta->first)+beta->second << ")" << std::endl;
                                //                            maquis::cout << " = " << M(std::make_pair(out_left_c, out_left_offset + s1->second*alpha_basis.size_of_block(alpha->first)+alpha->second),
                                //                                                    std::make_pair(out_right_c, out_right_offset + s2->second*beta_basis.size_of_block(beta->first)+beta->second)) << std::endl;
                                //                            
                                //                            maquis::cout << "mps[" << in_left_c << ", " << in_right_c << "]"
                                //                            << "(" << in_left_offset + s.second*alpha_basis.size_of_block(alpha->first) + alpha->second << ", " << in_right_offset + beta->second << ")" << std::endl;
                                //                            maquis::cout << " = " << mps_small[p].data()(std::make_pair(in_left_c, in_left_offset + s.second*alpha_basis.size_of_block(alpha->first) + alpha->second),
                                //                                                                      std::make_pair(in_right_c, in_right_offset + beta->second)) << std::endl;
                                
                                M(std::make_pair(out_left_c, out_left_offset + s.second*alpha_basis.size_of_block(alpha->first) + alpha->second),
                                  std::make_pair(out_right_c, out_right_offset + beta->second))
                                += mps_large[2*p].data()(std::make_pair(in1_left_c, in1_left_offset + s1->second*alpha_basis.size_of_block(alpha->first) + alpha->second),
                                                         std::make_pair(in1_right_c, in1_right_offset + b->second))
                                * mps_large[2*p+1].data()(std::make_pair(in2_left_c, in2_left_offset + b->second),
                                                          std::make_pair(in2_right_c, in2_right_offset + s2->second*beta_basis.size_of_block(beta->first) + beta->second))
                                ;
                                
                            }
            
            
            mps_small[p].data() = M;
            mps_small[p].left_i = alpha_basis;
            mps_small[p].right_i = beta_basis;
            mps_small[p].cur_storage = LeftPaired;
            
        }
        
        mps_small.normalize_left();
        
    }
    
    
    template<class Matrix, class SymmGroup>
    void
    static site_extension (MPSTensor<Matrix, SymmGroup> const & mps_small,
                           MPSTensor<Matrix, SymmGroup> & M1,
                           MPSTensor<Matrix, SymmGroup> & M2)
    {        
        typedef typename SymmGroup::charge charge;
        typedef typename Index<SymmGroup>::basis_iterator bi_t;
        
        block_matrix<Matrix, SymmGroup> M;
        
        Index<SymmGroup> alpha_basis = mps_small.row_dim();
        Index<SymmGroup> beta_basis = mps_small.col_dim();
        Index<SymmGroup> s_basis = mps_small.site_dim();
        Index<SymmGroup> s1_basis = M1.site_dim();
        Index<SymmGroup> s2_basis = M2.site_dim();
        
        //            maquis::cout << "alpha_basis:" <<std::endl << alpha_basis << std::endl;
        //            maquis::cout << "beta_basis:" <<std::endl << beta_basis << std::endl;
        //            maquis::cout << "s_basis:" <<std::endl << s_basis << std::endl;
        //            maquis::cout << "s1_basis:" <<std::endl << s1_basis << std::endl;
        //            maquis::cout << "s2_basis:" <<std::endl << s2_basis << std::endl;
        
        
        mps_small.make_left_paired();
        
        ProductBasis<SymmGroup> in_left(s_basis, alpha_basis);
        
        ProductBasis<SymmGroup> out_left(s1_basis, alpha_basis);
        ProductBasis<SymmGroup> out_right(s2_basis, beta_basis,
                                          boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                                              -boost::lambda::_1, boost::lambda::_2));
        
        //            maquis::cout << mps_small[p].data() << std::endl;
        for (bi_t alpha = alpha_basis.basis_begin(); !alpha.end(); ++alpha)
            for (bi_t beta = beta_basis.basis_begin(); !beta.end(); ++beta)
                for (bi_t s1 = s1_basis.basis_begin(); !s1.end(); ++s1)
                    for (bi_t s2 = s2_basis.basis_begin(); !s2.end(); ++s2)
                    {
                        std::pair<charge, std::size_t> s(SymmGroup::fuse(s1->first, s2->first),
                                                         s1->second*s2_basis.size_of_block(s2->first)+s2->second);
                        
                        if (!s_basis.has(s.first) || s_basis.size_of_block(s.first) <= s.second)
                            continue;
                        
                        charge in_left_c = SymmGroup::fuse(s.first, alpha->first);
                        charge in_right_c = beta->first;
                        
                        if (!mps_small.data().has_block(in_left_c, in_right_c))
                            continue;
                        
                        charge out_left_c = SymmGroup::fuse(s1->first, alpha->first);
                        charge out_right_c = SymmGroup::fuse(-s2->first, beta->first);
                        
                        //                            maquis::cout << "--" << std::endl;
                        //                            maquis::cout << "alpha: " << alpha->first << ", " << alpha->second << std::endl;
                        //                            maquis::cout << "beta: " << beta->first << ", " << beta->second << std::endl;
                        //                            maquis::cout << "s1: " << s1->first << ", " << s1->second << std::endl;
                        //                            maquis::cout << "s2: " << s2->first << ", " << s2->second << std::endl;
                        //                            maquis::cout << "s: " << s.first << ", " << s.second << std::endl;
                        
                        if (!M.has_block(out_left_c, out_right_c))
                            M.insert_block(new Matrix(out_left.size(s1->first, alpha->first),
                                                      out_right.size(s2->first, beta->first,
                                                                    boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                                                                         -boost::lambda::_1, boost::lambda::_2)),
                                                      0),
                                           out_left_c, out_right_c);
                        
                        
                        //                            maquis::cout << "block has size " << out_left.size(s1->first, alpha->first)
                        //                            << "x"
                        //                            << out_right.size(s2->first, beta->first,
                        //                                              boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                        //                                                                  -boost::lambda::_1, boost::lambda::_2)) << std::endl;
                        
                        std::size_t in_left_offset = in_left(s.first, alpha->first);
                        std::size_t in_right_offset = 0;
                        
                        std::size_t out_left_offset = out_left(s1->first, alpha->first);
                        std::size_t out_right_offset = out_right(s2->first, beta->first);
                        
                        //                            maquis::cout << "M[" << out_left_c << ", " << out_right_c << "]"
                        //                            << "(" << out_left_offset + s1->second*alpha_basis.size_of_block(alpha->first)+alpha->second << ", " << out_right_offset + s2->second*beta_basis.size_of_block(beta->first)+beta->second << ")" << std::endl;
                        //                            maquis::cout << " = " << M(std::make_pair(out_left_c, out_left_offset + s1->second*alpha_basis.size_of_block(alpha->first)+alpha->second),
                        //                                                    std::make_pair(out_right_c, out_right_offset + s2->second*beta_basis.size_of_block(beta->first)+beta->second)) << std::endl;
                        //                            
                        //                            maquis::cout << "mps[" << in_left_c << ", " << in_right_c << "]"
                        //                            << "(" << in_left_offset + s.second*alpha_basis.size_of_block(alpha->first) + alpha->second << ", " << in_right_offset + beta->second << ")" << std::endl;
                        //                            maquis::cout << " = " << mps_small[p].data()(std::make_pair(in_left_c, in_left_offset + s.second*alpha_basis.size_of_block(alpha->first) + alpha->second),
                        //                                                                      std::make_pair(in_right_c, in_right_offset + beta->second)) << std::endl;
                        
                        M(std::make_pair(out_left_c, out_left_offset + s1->second*alpha_basis.size_of_block(alpha->first)+alpha->second),
                          std::make_pair(out_right_c, out_right_offset + s2->second*beta_basis.size_of_block(beta->first)+beta->second))
                        = mps_small.data()(std::make_pair(in_left_c, in_left_offset + s.second*alpha_basis.size_of_block(alpha->first) + alpha->second),
                                           std::make_pair(in_right_c, in_right_offset + beta->second));
                        
                    }
        
        
        block_matrix<Matrix, SymmGroup> V, left, right;
        block_matrix<typename alps::numeric::associated_diagonal_matrix<Matrix>::type, SymmGroup> S;
        
        svd(M, left, V, S);
        gemm(S, V, right);
        
        M1.data() = left; // replace_left_paired cannot be used here
        M1.left_i = alpha_basis;
        M1.right_i = left.right_basis();
        M1.cur_storage = LeftPaired;
        
        M2.data() = right; // replace_right_paired cannot be used here
        M2.right_i = beta_basis;
        M2.left_i = right.left_basis();
        M2.cur_storage = RightPaired;
        
    }

    
    template<class Matrix, class SymmGroup>
    void
    static extension (MPS<Matrix, SymmGroup> const & mps_small,
                      MPS<Matrix, SymmGroup> & mps_large)
    {
        std::size_t L1 = mps_small.length();
        std::size_t L2 = mps_large.length();
        assert(L2 == 2*L1);
                
        block_matrix<Matrix, SymmGroup> t_norm;
        
        for (std::size_t p = 0; p < L1; ++p)
        {            
            site_extension(mps_small[p], mps_large[2*p], mps_large[2*p+1]);
                        
            // Normalization
            if (p != 0)
                mps_large[2*p].multiply_from_left(t_norm);
            t_norm = mps_large[2*p].normalize_left(DefaultSolver());
            mps_large[2*p+1].multiply_from_left(t_norm);
            t_norm = mps_large[2*p+1].normalize_left(DefaultSolver());
        }
        
    }
    
    
    template<class Matrix, class SymmGroup>
    void
    static extension_optim (BaseParameters & parms,
                            Logger & iteration_log,
                            MPS<Matrix, SymmGroup> mps_small,
                            MPS<Matrix, SymmGroup> & mps_large,
                            std::vector<MPO<Matrix, SymmGroup> > const & mpos_mix)
    {
                
        std::size_t L = mps_small.length();
        std::size_t LL = mps_large.length();
        assert(LL == 2*L);
        
        int sweep = 0;
        
        /*
         * INIT SWEEP PARAMETERS
         */
        double alpha;
        int ngs = parms.get<int>("ngrowsweeps"), nms = parms.get<int>("nmainsweeps");
        if (sweep < ngs)
            alpha = parms.get<double>("alpha_initial");
        else if (sweep < ngs + nms)
            alpha = parms.get<double>("alpha_main");
        else
            alpha = parms.get<double>("alpha_final");
        
        
        double cutoff;
        if (sweep >= parms.get<int>("ngrowsweeps"))
            cutoff = parms.get<double>("truncation_final");
        else
            cutoff = log_interpolate(parms.get<double>("truncation_initial"), parms.get<double>("truncation_final"), parms.get<int>("ngrowsweeps"), sweep);
        
        std::size_t Mmax;
        if (parms.is_set("sweep_bond_dimensions")) {
            std::vector<std::size_t> ssizes = parms.get<std::vector<std::size_t> >("sweep_bond_dimensions");
            if (sweep >= ssizes.size())
                Mmax = *ssizes.rbegin();
            else
                Mmax = ssizes[sweep];
        } else
            Mmax = parms.get<std::size_t>("max_bond_dimension");
        
        
        mps_small.normalize_right();
        /*
         * INIT BOUNDARIES
         */
        
        std::vector<Boundary<Matrix, SymmGroup> > left_(LL+1), right_(L+1);        
        {
            left_[0] = mps_small.left_boundary();
            
            Boundary<Matrix, SymmGroup> right = mps_small.right_boundary();
            right_[L] = right;
            
            for(int i = L-1; i >= 0; --i) {
                MPSTensor<Matrix, SymmGroup> bkp = mps_small[i];
                right = contraction::overlap_mpo_right_step(mps_small[i], bkp, right, mpos_mix[0][i]);
                right_[i] = right;
            }
        }
        
        block_matrix<Matrix, SymmGroup> t_norm;
        
        MPSTensor<Matrix, SymmGroup> Msmall = mps_small[0];
        
        typedef typename SymmGroup::charge charge;
        typedef typename Index<SymmGroup>::basis_iterator bi_t;
        
        for (std::size_t p = 0; p < L; ++p)
        {
            
            site_extension(Msmall, mps_large[2*p], mps_large[2*p+1]);
            
            assert( mps_large[2*p].num_check() );
            assert( mps_large[2*p+1].num_check() );
            
            // Normalizing the MPS
            mps_large[2*p+1].divide_by_scalar(mps_large[2*p+1].scalar_norm()); // that may playout
            assert( mps_large[2*p].num_check() );
            assert( mps_large[2*p+1].num_check() );
            t_norm = mps_large[2*p+1].normalize_right(DefaultSolver());
            mps_large[2*p].multiply_from_right(t_norm);
            mps_large[2*p].divide_by_scalar(mps_large[2*p].scalar_norm());
            assert( mps_large[2*p].num_check() );
            assert( mps_large[2*p+1].num_check() );

            
            MPSTensor<Matrix, SymmGroup> bkp;
            Boundary<Matrix, SymmGroup> right;
            
            
            Boundary<Matrix, SymmGroup> right_mixed;
            if (p<L-1) {
                right_mixed = right_[p+2];
                bkp = mps_small[p+1];
                right_mixed = contraction::overlap_mpo_right_step(mps_small[p+1], bkp, right_mixed, mpos_mix[p+1][2*(p+1)]);
            }

            // Testing energy calculations
            if (false) {
                
                MPS<Matrix, SymmGroup> mps_mixed = mps_small;
                mps_mixed.resize(2*(p+1) + (L-1-p));
                
                std::copy(mps_large.const_begin(), mps_large.const_begin()+2*(p+1), mps_mixed.begin());
                std::copy(mps_small.const_begin()+(p+1), mps_small.const_end(), mps_mixed.begin()+2*(p+1));
                
                
                if (false && p == 0) {
                    for (int shift=0; shift<4; shift++) {
                        maquis::cout << "MPO(" << 2*p+shift << ")::" << std::endl;
                        for (int i=0; i<mpos_mix[p+1][2*p+shift].row_dim(); ++i)
                            for(int j=0; j<mpos_mix[p+1][2*p+shift].col_dim(); ++j)
                                maquis::cout << "(" << i << " --> " << j << "):" << std::endl << mpos_mix[p+1][2*p+shift](i, j);
                    }
                    
                    maquis::cout << "MPO_large(last)::" << std::endl;
                    for (int i=0; i<mpos_mix[L][LL-1].row_dim(); ++i)
                        for(int j=0; j<mpos_mix[L][LL-1].col_dim(); ++j)
                            maquis::cout << "(" << i << " --> " << j << "):" << std::endl << mpos_mix[L][LL-1](i, j);
                    exit(-1);
                    
                }
                
                {
                    maquis::cout << "Norm " << norm(mps_mixed) << std::endl;
                    maquis::cout << "Energy " << "finegraining_fullmpomix " << expval(mps_mixed, mpos_mix[p+1]) << std::endl;
                }
                
            }
            
            /*
             * OPTIMIZATION 2*p
             */
            {
                right = (p<L-1) ? right_mixed : right_[p+1];
                bkp = mps_large[2*p+1];
                right = contraction::overlap_mpo_right_step(mps_large[2*p+1], bkp, right, mpos_mix[p+1][2*p+1]);
                if (p == 0)
                    left_[0] = mps_large.left_boundary();
                
                SiteProblem<Matrix, SymmGroup> sp(mps_large[2*p], left_[2*p], right, mpos_mix[p+1][2*p]);

                
                // solver
                if (parms.get<bool>("finegrain_optim"))
                {
                    
                    std::pair<double, MPSTensor<Matrix, SymmGroup> > res;
                    timeval now, then;
                    if (parms.get<std::string>("eigensolver") == std::string("IETL")) {
                        BEGIN_TIMING("IETL")
                        res = solve_ietl_lanczos(sp, mps_large[2*p], parms);
                        END_TIMING("IETL")
                    } else if (parms.get<std::string>("eigensolver") == std::string("IETL_JCD")) {
                        BEGIN_TIMING("JCD")
                        res = solve_ietl_jcd(sp, mps_large[2*p], parms);
                        END_TIMING("JCD")
                    } else {
                        throw std::runtime_error("I don't know this eigensolver.");
                    }
                    
                    mps_large[2*p] = res.second;
                    
                    maquis::cout << "Energy " << "finegraining_1 " << res.first << std::endl;
                    iteration_log << make_log("Energy", res.first);

                } else if (true) {
                    // Compute Energy
                    MPSTensor<Matrix, SymmGroup> vec2 =
                    contraction::site_hamil2(sp.ket_tensor, sp.left, sp.right, sp.mpo);
                    double energy = sp.ket_tensor.scalar_overlap(vec2);
                    maquis::cout << "Energy " << "finegraining_00 " << energy << std::endl;
                    iteration_log << make_log("Energy", energy);

                }
                
                // growing
                /*
                
                maquis::cout << "Growing, alpha = " << alpha << std::endl;
                mps_large.grow_l2r_sweep(mpos_mix[L][2*p], left_[2*p], right,
                                         2*p, alpha, cutoff, Mmax, iteration_log);                
                 */
                 
                t_norm = mps_large[2*p].normalize_left(DefaultSolver());
                mps_large[2*p+1].multiply_from_left(t_norm);

            }
            
            /*
             * OPTIMIZATION 2*p+1
             */
            {
                right = (p<L-1) ? right_mixed : right_[p+1];
                bkp = mps_large[2*p];
                left_[2*p+1] = contraction::overlap_mpo_left_step(mps_large[2*p], bkp,
                                                                  left_[2*p], mpos_mix[L][2*p]);
                
                SiteProblem<Matrix, SymmGroup> sp(mps_large[2*p+1], left_[2*p+1], right, mpos_mix[p+1][2*p+1]);

                // solver
                if (parms.get<bool>("finegrain_optim"))
                {
                    
                    std::pair<double, MPSTensor<Matrix, SymmGroup> > res;
                    timeval now, then;
                    if (parms.get<std::string>("eigensolver") == std::string("IETL")) {
                        BEGIN_TIMING("IETL")
                        res = solve_ietl_lanczos(sp, mps_large[2*p+1], parms);
                        END_TIMING("IETL")
                    } else if (parms.get<std::string>("eigensolver") == std::string("IETL_JCD")) {
                        BEGIN_TIMING("JCD")
                        res = solve_ietl_jcd(sp, mps_large[2*p+1], parms);
                        END_TIMING("JCD")
                    } else {
                        throw std::runtime_error("I don't know this eigensolver.");
                    }
                    
                    mps_large[2*p+1] = res.second;
                    
                    maquis::cout << "Energy " << "finegraining_2 " << res.first << std::endl;
                    iteration_log << make_log("Energy", res.first);

                } else if (true) {
                    // Compute Energy
                    MPSTensor<Matrix, SymmGroup> vec2 =
                    contraction::site_hamil2(sp.ket_tensor, sp.left, sp.right, sp.mpo);
                    double energy = sp.ket_tensor.scalar_overlap(vec2);
                    maquis::cout << "Energy " << "finegraining_01 " << energy << std::endl;
                    iteration_log << make_log("Energy", energy);
                }
                
                // growing
                /*
                
                if (p < L-1) {
                    maquis::cout << "Growing, alpha = " << alpha << std::endl;
                    MPSTensor<Matrix, SymmGroup> new_mps =
                    contraction::predict_new_state_l2r_sweep(mps_large[2*p+1], mpos_mix[L][2*p+1], left_[2*p+1], right, alpha, cutoff, Mmax, iteration_log);
                    // New tensor for next iteration
                    Msmall = contraction::predict_lanczos_l2r_sweep(mps_small[p+1],
                                                                    mps_large[2*p+1], new_mps);
                    mps_large[2*p+1] = new_mps;
                } else {
                    block_matrix<Matrix, SymmGroup> t = mps_large[2*p+1].normalize_left(DefaultSolver());
                }

                */

                
                t_norm = mps_large[2*p+1].normalize_left(DefaultSolver());

                if (p < L-1) {
                    Msmall = mps_small[p+1];
                    Msmall.multiply_from_left(t_norm);
                }
            }
            
            // Preparing left boundary
            if (p < L-1) {
                bkp = mps_large[2*p+1];
                left_[2*p+2] = contraction::overlap_mpo_left_step(mps_large[2*p+1], bkp,
                                                                  left_[2*p+1], mpos_mix[L][2*p+1]);
            }
            
        }
        
#ifndef NDEBUG
        double energy = expval(mps_large, mpos_mix[L]);
        maquis::cout << "Energy " << "finegraining_final " << energy << std::endl;
#endif
        
    }
    
};

#endif
