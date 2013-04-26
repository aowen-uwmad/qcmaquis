/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef SS_OPTIMIZE_H
#define SS_OPTIMIZE_H

#include "dmrg/mp_tensors/mpo_ops.h"


template<class Matrix, class SymmGroup, class StorageMaster>
class ss_optimize : public optimizer_base<Matrix, SymmGroup, StorageMaster>
{
public:

    typedef optimizer_base<Matrix, SymmGroup, StorageMaster> base;
    using base::mpo;
    using base::mpo_orig;
    using base::mps;
    using base::left_;
    using base::left_stores_;
    using base::right_;
    using base::right_stores_;
    using base::parms;

    ss_optimize(MPS<Matrix, SymmGroup> const & mps_,
                MPO<Matrix, SymmGroup> const & mpo_,
                BaseParameters & parms_,
                StorageMaster & sm)
    : base(mps_, mpo_, parms_, sm) { }
    
    int sweep(int sweep, Logger & iteration_log,
               OptimizeDirection d = Both,
               int resume_at = -1,
               int max_secs = -1)
    {
        mpo = mpo_orig;
        
        timeval sweep_now, sweep_then;
        gettimeofday(&sweep_now, NULL);
        
        std::size_t L = mps.length();
        
        if (resume_at != -1)
        {
            int site;
            if (resume_at < L)
                site = resume_at;
            else
                site = 2*L-resume_at-1;
            mps.canonize(site);
            this->init_left_right(mpo, site);
        }

//        if (parms.template <bool>("beta_mode") && sweep == 0 && resume_at < L) {
//            int site = (resume_at == -1) ? 0 : resume_at;
//            mpo = zero_after(mpo_orig, site+2);
//            mps.canonize(site);
//            this->init_left_right(mpo, site);
//        }
        
        storage::prefetch(left_[0], left_stores_[0]);
        storage::prefetch(right_[1], right_stores_[1]);
        
#ifndef NDEBUG
        maquis::cout << mps.description() << std::endl;
#endif
        for (int _site = (resume_at == -1 ? 0 : resume_at);
             _site < 2*L; ++_site) {
            
            int site, lr;
            if (_site < L) {
                site = _site;
                lr = 1;
            } else {
                site = 2*L-_site-1;
                lr = -1;
            }

            if (lr == -1 && site == L-1) {
                maquis::cout << "Syncing storage" << std::endl;
                base::storage_master.sync();
            }
            
            maquis::cout << "Sweep " << sweep << ", optimizing site " << site << std::endl;
//            storage_master.print_size();
            
//            mps[site].make_left_paired();
            
            if (parms.template get<bool>("beta_mode")) {
                if (sweep == 0 && lr == 1) {
                    mpo = zero_after(mpo_orig, 0);
                    if (site == 0)
                        this->init_left_right(mpo, 0);
                } else if (sweep == 0 && lr == -1 && site == L-1) {
                    mpo = mpo_orig;
                    //this->init_left_right(mpo, site);
                }
            }
            
            
            storage::load(left_[site], left_stores_[site]);
            storage::load(right_[site+1], right_stores_[site+1]);
            
            if (lr == +1) {
                storage::prefetch(left_[site+1], left_stores_[site+1]);
            } else {
                storage::prefetch(right_[site], right_stores_[site]);
            }
            assert( left_[site].reasonable() );    // in case something is wrong with storage
            assert( right_[site+1].reasonable() ); // in case something is wrong with storage
            
            
//            maquis::cout << "My size: " << std::endl;
//            maquis::cout << "  left_: " << utils::size_of(left_.begin(), left_.end())/1024.0/1024 << std::endl;
//            maquis::cout << "  right_: " << utils::size_of(right_.begin(), right_.end())/1024.0/1024 << std::endl;
//            maquis::cout << "  MPS: " << utils::size_of(mps.begin(), mps.end())/1024.0/1024 << std::endl;
//            maquis::cout << "  MPS[i]: " << utils::size_of(mps[site])/1024.0/1024 << std::endl;
            
            //SiteProblem<Matrix, SymmGroup> sp(mps[site], left_[site], right_[site+1], mpo[site]);
            
            timeval now, then;

            std::pair<double, MPSTensor<Matrix, SymmGroup> > res;
          
            std::vector<MPSTensor<Matrix, SymmGroup> > ortho_vecs(base::northo);
            for (int n = 0; n < base::northo; ++n) {
                base::ortho_mps[n][site].make_right_paired();
                block_matrix<Matrix, SymmGroup> t, t2, t3;
                gemm(base::ortho_left_[n][site], base::ortho_mps[n][site].data(), t);
                reshape_right_to_left_new(mps[site].site_dim(),
                                          base::ortho_left_[n][site].left_basis(), base::ortho_mps[n][site].col_dim(),
                                          t, t2);
                gemm(t2, transpose(base::ortho_right_[n][site+1]), t3);
                
                mps[site].make_left_paired();
                t = mps[site].data();
                reshape_and_pad_left(mps[site].site_dim(),
                                     base::ortho_left_[n][site].left_basis(), base::ortho_right_[n][site+1].left_basis(),
                                     mps[site].row_dim(), mps[site].col_dim(),
                                     t3, t);
                
                MPSTensor<Matrix, SymmGroup> t4(mps[site].site_dim(),
                                                mps[site].row_dim(), mps[site].col_dim(),
                                                t, LeftPaired);
                ortho_vecs[n] = t4;
            }

            SiteProblem<Matrix, SymmGroup> sp(left_[site], right_[site+1], mpo[site]);
            
            if (d == Both ||
                (d == LeftOnly && lr == -1) ||
                (d == RightOnly && lr == +1))
            {
                if (parms.template get<std::string>("eigensolver") == std::string("IETL")) {
                    BEGIN_TIMING("IETL")
                    res = solve_ietl_lanczos(sp, mps[site], parms);
                    END_TIMING("IETL")
                } else if (parms.template get<std::string>("eigensolver") == std::string("IETL_JCD")) {
                    BEGIN_TIMING("JCD")
                    res = solve_ietl_jcd(sp, mps[site], parms, ortho_vecs);
                    END_TIMING("JCD")
                } else {
                    throw std::runtime_error("I don't know this eigensolver.");
                }
 
                mps[site] = res.second;
            }
            
#ifndef NDEBUG
            // Caution: this is an O(L) operation, so it really should be done only in debug mode
            for (int n = 0; n < base::northo; ++n)
                maquis::cout << "MPS overlap: " << overlap(mps, base::ortho_mps[n]) << std::endl;
#endif
            
            maquis::cout << "Energy " << lr << " " << res.first << std::endl;
//            maquis::cout << "Energy check " << maquis::real(expval(mps, mpo)) << std::endl;
            
            iteration_log << make_log("Energy", res.first);
            
            double alpha;
            int ngs = parms.template get<int>("ngrowsweeps"), nms = parms.template get<int>("nmainsweeps");
            if (sweep < ngs)
                alpha = parms.template get<double>("alpha_initial");
            else if (sweep < ngs + nms)
                alpha = parms.template get<double>("alpha_main");
            else
                alpha = parms.template get<double>("alpha_final");
            
            
            double cutoff;
            if (sweep >= parms.template get<int>("ngrowsweeps"))
                cutoff = parms.template get<double>("truncation_final");
            else
                cutoff = log_interpolate(parms.template get<double>("truncation_initial"), parms.template get<double>("truncation_final"), parms.template get<int>("ngrowsweeps"), sweep);
            
            std::size_t Mmax;
            if (parms.is_set("sweep_bond_dimensions")) {
                std::vector<std::size_t> ssizes = parms.template get<std::vector<std::size_t> >("sweep_bond_dimensions");
                if (sweep >= ssizes.size())
                    Mmax = *ssizes.rbegin();
                else
                    Mmax = ssizes[sweep];
            } else
                Mmax = parms.template get<std::size_t>("max_bond_dimension");
            
            std::pair<std::size_t, double> trunc;
            
                
            if (lr == +1) {
                if (site < L-1) {
                    maquis::cout << "Growing, alpha = " << alpha << std::endl;
                    mps.grow_l2r_sweep(mpo[site], left_[site], right_[site+1],
                                       site, alpha, cutoff, Mmax, iteration_log);
                } else {
                    block_matrix<Matrix, SymmGroup> t = mps[site].normalize_left(DefaultSolver());
                    if (site < L-1)
                        mps[site+1].multiply_from_left(t);
                }
                
                
                storage::reset(left_stores_[site+1]); // left_stores_[site+1] is outdated
                this->boundary_left_step(mpo, site); // creating left_[site+1]
            } else if (lr == -1) {
                if (site > 0) {
                    maquis::cout << "Growing, alpha = " << alpha << std::endl;
                    // Invalid read occurs after this!\n
                    mps.grow_r2l_sweep(mpo[site], left_[site], right_[site+1],
                                       site, alpha, cutoff, Mmax, iteration_log);
                } else {
                    block_matrix<Matrix, SymmGroup> t = mps[site].normalize_right(DefaultSolver());
                    if (site > 0)
                        mps[site-1].multiply_from_right(t);
                }
                
                
                storage::reset(right_stores_[site]); // right_stores_[site] is outdated
                this->boundary_right_step(mpo, site); // creating right_[site]
            }
            
        	storage::store(left_[site], left_stores_[site]); // store currently used boundary
        	storage::store(right_[site+1], right_stores_[site+1]); // store currently used boundary

            
            
            gettimeofday(&sweep_then, NULL);
            double elapsed = sweep_then.tv_sec-sweep_now.tv_sec + 1e-6 * (sweep_then.tv_usec-sweep_now.tv_usec);
            maquis::cout << "Sweep has been running for " << elapsed << " seconds." << std::endl;
            if (max_secs != -1 && elapsed > max_secs && _site+1<2*L) {
                return _site+1;
            }
            else
               maquis::cout << max_secs - elapsed << " seconds left." << std::endl;
        }
        
        return -1;
    }
    
};

#endif

