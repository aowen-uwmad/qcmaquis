/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
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

#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include "dmrg/mp_tensors/mps_mpo_ops.h"

#include "dmrg/deprecated/mpos/adjacency.h"
#include "dmrg/deprecated/mpos/generate_mpo.h"
#include "dmrg/deprecated/mpos/hamiltonians.h"

#include "dmrg/utils/BaseParameters.h"

template<class Matrix, class SymmGroup>
struct measure_
{
    template<class Archive>
    void operator()(MPS<Matrix, SymmGroup> & mps,
                    adj::Adjacency & adj,
                    mpos::Hamiltonian<Matrix, SymmGroup> & H,
                    BaseParameters & model,
                    Archive & ar)
    { }
};

template<class Matrix, class SymmGroup, class Archive>
void measure_2pt_correlation(MPS<Matrix, SymmGroup> & mps,
                             adj::Adjacency & adj,
                             block_matrix<Matrix, SymmGroup> const & identity,
                             block_matrix<Matrix, SymmGroup> const & fill,
                             Archive & ar,
                             std::vector<block_matrix<Matrix, SymmGroup> > & ops,
                             std::string base_path)
{
    std::vector<double> dc;
    std::vector<std::string> labels;
    for (size_t p = 0; p < adj.size()-1; ++p) {
        mpos::CorrMaker<Matrix, U1> dcorr(mps.length(), identity, fill,
                                          ops, p);
        MPO<Matrix, U1> mpo = dcorr.create_mpo();
        
        std::vector<double> dct = maquis::real(multi_expval(mps, mpo));
        std::copy(dct.begin(), dct.end(), std::back_inserter(dc));
        
        std::vector<std::string> lbt = dcorr.label_strings();
        std::copy(lbt.begin(), lbt.end(), std::back_inserter(labels));
    }
    
    ar[base_path + std::string("/mean/value")] << dc;
    ar[base_path + std::string("/labels")] << labels;
}

template<class Matrix>
struct measure_<Matrix, U1>
{
    template<class Archive>
    void measure_blbq(MPS<Matrix, U1> & mps,
                      adj::Adjacency & adj,
                      mpos::Hamiltonian<Matrix, U1> & H,
                      BaseParameters & model,
                      Archive & ar)
    {
        std::vector<double> magns;

        std::vector<block_matrix<Matrix, U1> > ops(4);
        std::vector<std::string> names;
        
        block_matrix<Matrix, U1> ident = H.get_free();
        
        ops[0].insert_block(Matrix(1, 1, 1), 1, 1);
        ops[0].insert_block(Matrix(1, 1, 0), 0, 0);
        ops[0].insert_block(Matrix(1, 1, -1), -1, -1);
        names.push_back("Magnetization");
        
        ops[1].insert_block(Matrix(1, 1, 1), -1, -1);
        names.push_back("ColorDensity1");
        ops[2].insert_block(Matrix(1, 1, 1), 0, 0);
        names.push_back("ColorDensity2");
        ops[3].insert_block(Matrix(1, 1, 1), 1, 1);
        names.push_back("ColorDensity3");
        
        for (int i = 0; i < 4; ++i) {
            magns.clear();
            
            for (std::size_t p = 0; p < adj.size(); ++p)
            {
                mpos::MPOMaker<Matrix, U1> mpom(adj, H);
                std::vector<std::pair<int, block_matrix<Matrix, U1> > > v;
                v.push_back( std::make_pair( p, ops[i] ) );
                mpom.add_term(v);
                MPO<Matrix, U1> mpo = mpom.create_mpo();
                
                double val = maquis::real((expval(mps, mpo));
                magns.push_back(val);
            }
            
            std::string n = std::string("/spectrum/results/") + names[i] + std::string("/mean/value");
            ar[n] << magns;

        }
     
        for (int i = 0; i < 4; ++i)
        {
            std::vector<block_matrix<Matrix, U1> > corr;
            corr.push_back( ops[i] );
            corr.push_back( ops[i] );
//            mpos::CorrMaker<Matrix, U1> mcorr(mps.length(), ident, ident, corr);
//            MPO<Matrix, U1> mpo = mcorr.create_mpo();
//            
//            std::vector<double> mc_v = maquis::real(multi_expval(mps, mpo));
            std::string name = std::string("/spectrum/results/") + names[i] + std::string("Correlation");
            
            measure_2pt_correlation(mps, adj, ident, ident, ar,
                                    corr, name);
        }
        
    }
   
    template<class Archive>
    void measure_superf(MPS<Matrix, U1> & mps,
                        adj::Adjacency & adj,
                        mpos::Hamiltonian<Matrix, U1> & H,
                        BaseParameters & model,
                        Archive & ar)
    {
        std::vector<double> magns;
        
        block_matrix<Matrix, U1> dens;
        
        dens.insert_block(Matrix(1, 1, 1), 1, 1);
            
        for (std::size_t p = 0; p < adj.size(); ++p)
        {
            mpos::MPOMaker<Matrix, U1> mpom(adj, H);
            std::vector<std::pair<std::size_t, block_matrix<Matrix, U1> > > v;
            v.push_back( std::make_pair( p, dens ) );
            mpom.add_term(v);
            MPO<Matrix, U1> mpo = mpom.create_mpo();
            
            double val = maquis::real(expval(mps, mpo));
            magns.push_back(val);
        }
            
        ar["/spectrum/results/Density/mean/value"] << magns;
        
        std::vector<double> corrs;
        
        for (std::size_t p = 0; p < adj.size(); ++p)
        {
            std::vector<int> neighs = adj.forward(p);
            for (std::vector<int>::iterator it = neighs.begin();
                 it != neighs.end(); ++it)
            {
                mpos::MPOMaker<Matrix, U1> mpom(adj, H);
                std::vector<std::pair<std::size_t, block_matrix<Matrix, U1> > > v;
                v.push_back( std::make_pair( p, dens ) );
                v.push_back( std::make_pair(*it, dens) );
                mpom.add_term(v);
                MPO<Matrix, U1> mpo = mpom.create_mpo();
                
                double val = maquis::real(expval(mps, mpo));
                corrs.push_back(val);
            }
        }
        
        ar["/spectrum/results/NNDensityCorrelation/mean/value"] << corrs;
    }
    
    template<class Archive>
    void measure_ff(MPS<Matrix, U1> & mps,
                    adj::Adjacency & adj,
                    mpos::Hamiltonian<Matrix, U1> & H,
                    BaseParameters & model,
                    Archive & ar)
    {
        block_matrix<Matrix, U1> dens, create, destroy, sign, ident;
        
        dens.insert_block(Matrix(1, 1, 1), 1, 1);
        create.insert_block(Matrix(1, 1, 1), 0, 1);
        destroy.insert_block(Matrix(1, 1, 1), 1, 0);
        
        sign.insert_block(Matrix(1, 1, 1), 0, 0);
        sign.insert_block(Matrix(1, 1, -1), 1, 1);
        
        ident.insert_block(Matrix(1, 1, 1), 0, 0);
        ident.insert_block(Matrix(1, 1, 1), 1, 1);
        
        std::vector<double> density;
        for (std::size_t p = 0; p < adj.size(); ++p)
        {
            mpos::MPOMaker<Matrix, U1> mpom(adj, H);
            std::vector<std::pair<int, block_matrix<Matrix, U1> > > v;
            v.push_back( std::make_pair( p, dens ) );
            mpom.add_term(v);
            MPO<Matrix, U1> mpo = mpom.create_mpo();
            
            double val = maquis::real(expval(mps, mpo));
            density.push_back(val);
        }
        ar["/spectrum/results/Density/mean/value"] << density;
        
        std::vector<block_matrix<Matrix, U1> > density_corr;
        density_corr.push_back( dens );
        density_corr.push_back( dens );
        measure_2pt_correlation(mps, adj, ident, ident,
                                ar, density_corr,
                                "/spectrum/results/DensityCorrelation");
        
        std::vector<block_matrix<Matrix, U1> > onebody;
        onebody.push_back( create );
        onebody.push_back( destroy );
        measure_2pt_correlation(mps, adj, ident, sign,
                                ar, onebody,
                                "/spectrum/results/OneBodyDM");
    }
    
    template<class Archive>
    void operator()(MPS<Matrix, U1> & mps,
                    adj::Adjacency & adj,
                    mpos::Hamiltonian<Matrix, U1> & H,
                    BaseParameters & model,
                    Archive & ar)
    {
        if (model["model"] == std::string("biquadratic"))
            measure_blbq(mps, adj, H, model, ar);
        else if (model["model"] == std::string("FreeFermions"))
            measure_ff(mps, adj, H, model, ar);
    }
};

template<class Matrix, class SymmGroup, class Archive>
void measure(MPS<Matrix, SymmGroup> & mps,
             adj::Adjacency & adj,
             mpos::Hamiltonian<Matrix, SymmGroup> & H,
             BaseParameters & model,
             Archive & ar)
{
    measure_<Matrix, SymmGroup>()(mps, adj, H, model, ar);
}

#endif
