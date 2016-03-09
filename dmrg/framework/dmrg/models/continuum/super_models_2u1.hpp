/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2013-2013 by Michele Dolfi <dolfim@phys.ethz.ch>
 *
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

#ifndef MAQUIS_DMRG_MODELS_CONTINUUM_SUPER_MODELS_2U1_HPP
#define MAQUIS_DMRG_MODELS_CONTINUUM_SUPER_MODELS_2U1_HPP

#include <sstream>
#include <cmath>

#include "dmrg/block_matrix/grouped_symmetry.h"
#include "dmrg/mp_tensors/mps_sectors.h"
#include "dmrg/mp_tensors/dm_op_kron.h"
#include "dmrg/mp_tensors/identity_mps.h"

#include "dmrg/models/model.h"
#include "dmrg/models/measurements.h"
#include "dmrg/models/meas_prepare.hpp"
#include "dmrg/utils/BaseParameters.h"


template<class Matrix>
class mps_ident_init : public mps_initializer<Matrix, TwoU1>
{
public:
    mps_ident_init(MPS<Matrix, TwoU1> const& mps_)
    : mps_ident(mps_)
    { }

    void operator()(MPS<Matrix, TwoU1> & mps,
                    std::size_t Mmax,
                    Index<TwoU1> const & phys_rho,
                    TwoU1::charge right_end)
    {
        assert( mps.length() == mps_ident.length() );
        assert( right_end == mps_ident[mps.length()-1].col_dim()[0].first );

        mps = mps_ident;
    }
    
private:
    MPS<Matrix, TwoU1> mps_ident;
};


/* ****************** OPTICAL LATTICE (with symmetry) */
template<class Matrix>
class DMOpticalLatticeTwoU1 : public Model<Matrix, TwoU1> {
    typedef Model<Matrix, TwoU1> base;
    
    typedef Hamiltonian<Matrix, TwoU1> ham;
    typedef typename ham::hamterm_t hamterm_t;
    typedef typename ham::hamtagterm_t hamtagterm_t;
    typedef Measurement_Term<Matrix, TwoU1> mterm_t;
    typedef typename ham::op_t op_t;
    typedef typename ham::table_type table_type;
    typedef typename ham::table_ptr table_ptr;

    typedef Hamiltonian<Matrix, U1> psi_ham;
    typedef typename psi_ham::hamterm_t psi_hamterm_t;
    typedef typename psi_ham::hamtagterm_t psi_hamtagterm_t;
    typedef typename psi_ham::op_t psi_op;
    typedef typename psi_ham::table_type psi_table_type;
    typedef typename psi_ham::table_ptr psi_table_ptr;
    
    typedef typename table_type::tag_type tag_type;
    typedef typename Matrix::value_type value_type;

public:
    DMOpticalLatticeTwoU1 (const Lattice& lat_, BaseParameters & model_)
    : lat(lat_)
    , model(model_)
    , tag_handler( new table_type() )
    , psi_tag_handler( new psi_table_type() )
    {
        psi_op psi_ident_op, psi_count_op, psi_interaction_op, psi_create_op, psi_destroy_op;
        op_t ident_op, count_op, interaction_op;

        psi_phys.insert(std::make_pair(0, 1));
        psi_ident_op.insert_block(Matrix(1, 1, 1), 0, 0);
        
        int Nmax = model["Nmax"];
        for (int n=1; n<=Nmax; ++n)
        {
            psi_phys.insert(std::make_pair(n, 1));
            
            psi_ident_op.insert_block(Matrix(1, 1, 1), n, n);
            
            psi_count_op.insert_block(Matrix(1, 1, n), n, n);
            if ((n*n-n) != 0)
                psi_interaction_op.insert_block(Matrix(1, 1, n*n-n), n, n);
            
            
            psi_create_op.insert_block(Matrix(1, 1, std::sqrt(value_type(n))), n-1, n);
            psi_destroy_op.insert_block(Matrix(1, 1, std::sqrt(value_type(n))), n, n-1);
        }
        
        /**********************************************************************/
        /*** Create operator tag table ****************************************/
        /**********************************************************************/
#define REGISTER(op, kind) op = psi_tag_handler->register_op(op ## _op, kind);
        REGISTER(psi_ident,       tag_detail::bosonic)
        REGISTER(psi_create,      tag_detail::bosonic)
        REGISTER(psi_destroy,     tag_detail::bosonic)
        REGISTER(psi_count,       tag_detail::bosonic)
        REGISTER(psi_interaction, tag_detail::bosonic)
#undef REGISTER
        /**********************************************************************/

        
        phys          = group(psi_phys, adjoin(psi_phys));
        ident_op       = identity_matrix<Matrix>(phys);
        count_op       = adjoint_site_term(psi_count_op);
        interaction_op = adjoint_site_term(psi_interaction_op);
        
        std::cout << "phys: " << phys << std::endl;
        std::vector< std::pair<op_t,op_t> > hopops_op = adjoint_bond_term(psi_create_op, psi_destroy_op);
        std::vector< std::pair<tag_type,tag_type> > hopops(hopops_op.size());

        /**********************************************************************/
        /*** Create operator tag table ****************************************/
        /**********************************************************************/
#define REGISTER(op, kind) op = tag_handler->register_op(op ## _op, kind);
        REGISTER(ident,       tag_detail::bosonic)
        REGISTER(count,       tag_detail::bosonic)
        REGISTER(interaction, tag_detail::bosonic)
        for (size_t i=0; i<hopops.size(); ++i)
            hopops[i] = std::make_pair( tag_handler->register_op(hopops_op[i].first,  tag_detail::bosonic),
                                        tag_handler->register_op(hopops_op[i].second, tag_detail::bosonic) );
#undef REGISTER
        /**********************************************************************/


        for (int p=0; p<lat.size(); ++p)
        {
            std::vector<int> neighs = lat.all(p);
            
            double exp_potential = model["V0"]*std::pow( std::cos(model["k"]*lat.get_prop<double>("x", p)), 2 );
            
            double dx1 = lat.get_prop<double>("dx", p, neighs[0]);
            double dx2;
            if (neighs.size() == 1 && lat.get_prop<bool>("at_open_left_boundary", p))
                dx2 = lat.get_prop<double>("dx", p, p-1);
            else if (neighs.size() == 1 && lat.get_prop<bool>("at_open_right_boundary", p))
                dx2 = lat.get_prop<double>("dx", p, p+1);
            else
                dx2 = lat.get_prop<double>("dx", p, neighs[1]);
            
            double dx0 = lat.get_prop<double>("dx", p);
            
            // Psi''(x) = coeff1 * Psi(x+dx1) + coeff0 * Psi(x) + coeff2 * Psi(x+dx2)
            double coeff1 = 2. / (dx1*dx1 - dx1*dx2);
            double coeff2 = 2. / (dx2*dx2 - dx1*dx2);
            double coeff0 = -(coeff1 + coeff2);
            
            double U = model["c"] / dx0;
            double mu = exp_potential - model["mu"];
            mu += -coeff0 * model["h"];
            
#ifndef NDEBUG
            maquis::cout << "U = " << U << ", mu = " << mu << ", t = " << coeff1 * model["h"] << std::endl;
#endif
            
            if (U != 0.)
            { // U * n * (n-1)
                hamtagterm_t term;
                term.with_sign = false;
                term.scale = U;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, interaction) );
                terms.push_back(term);
            }
            if (U != 0.)
            { // U * n * (n-1)
                psi_hamtagterm_t term;
                term.with_sign = false;
                term.scale = U;
                term.fill_operator = psi_ident;
                term.operators.push_back( std::make_pair(p, psi_interaction) );
                psi_terms.push_back(term);
            }
            
            if (mu != 0.)
            { // mu * n
                hamtagterm_t term;
                term.with_sign = false;
                term.scale = mu;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, count) );
                terms.push_back(term);
            }
            if (mu != 0.)
            { // mu * n
                psi_hamtagterm_t term;
                term.with_sign = false;
                term.scale = mu;
                term.fill_operator = psi_ident;
                term.operators.push_back( std::make_pair(p, psi_count) );
                psi_terms.push_back(term);
            }
            
            for (int n=0; n<neighs.size(); ++n) { // hopping
                
                double t;
                if (lat.get_prop<double>("dx", p, neighs[n]) == dx1)
                    t = coeff1 * model["h"];
                else
                    t = coeff2 * model["h"];
                
                if (t != 0.) {
                    for( unsigned i = 0; i < hopops.size(); ++i )
                    {
                        hamtagterm_t term;
                        term.with_sign = false;
                        term.scale = -t;
                        term.fill_operator = ident;
                        term.operators.push_back( std::make_pair(p, hopops[i].first) );
                        term.operators.push_back( std::make_pair(neighs[n], hopops[i].second) );
                        terms.push_back(term);
                    }
                }
                if (t != 0.) {
                    psi_hamtagterm_t term;
                    term.with_sign = false;
                    term.scale = -t;
                    term.fill_operator = psi_ident;
                    term.operators.push_back( std::make_pair(p, psi_create) );
                    term.operators.push_back( std::make_pair(neighs[n], psi_destroy) );
                    psi_terms.push_back(term);
                }
            }
        }
    }
    
    Index<TwoU1> get_phys() const
    {
        return phys;
    }
    
    Hamiltonian<Matrix, TwoU1> H () const
    {
        std::vector<hamterm_t> terms_ops;
        return ham(phys, tag_handler->get_op(ident), terms_ops, ident, terms, tag_handler);
    }
    
    Measurements<Matrix, TwoU1> measurements () const
    {
        Measurements<Matrix, TwoU1> meas(Measurements<Matrix, TwoU1>::Densitymatrix);
        typedef DMOverlapMeasurement<Matrix, TwoU1> rho_mterm_t;
        meas.set_identity(tag_handler->get_op(ident));
        psi_op const& psi_ident_op = psi_tag_handler->get_op(psi_ident);
        
        std::vector<Index<TwoU1> > allowed_blocks = allowed_sectors(lat.size(), phys, this->initc(model), 1);
        
        MPS<Matrix, TwoU1> mps_ident = identity_dm_mps<Matrix>(lat.size(), psi_phys, allowed_blocks);
        
        if (model["MEASURE_CONTINUUM[Density]"]) {
            rho_mterm_t term;
            term.name = "Density";
            term.type = mterm_t::DMOverlap;
            term.mps_ident = mps_ident;
            std::vector<std::pair<op_t, bool> > ops(1, std::make_pair(psi_tag_handler->get_op(psi_count), false));
            
            MPO<Matrix, U1> mpo = meas_prepare::average<Matrix, SymmGroup>(lat, psi_ident_op, psi_ident_op, ops);
            term.overlaps_mps.push_back( mpo_to_smps_group(mpo, psi_phys, allowed_blocks) );
            
            meas.add_term(term);
        }
        
        if (model["MEASURE_CONTINUUM[Local density]"]) {
            rho_mterm_t term;
            term.name = "Local density";
            term.type = mterm_t::DMOverlap;
            term.mps_ident = mps_ident;
            std::vector<std::pair<op_t, bool> > ops(1, std::make_pair(psi_tag_handler->get_op(psi_count), false));
            
            std::pair<std::vector<MPO<Matrix, U1> >, std::vector<std::string> > tmeas;
            tmeas = meas_prepare::local<Matrix, SymmGroup>(lat, psi_ident_op, psi_ident_op, ops);
            std::swap(tmeas.second, term.labels);
            
            term.overlaps_mps.reserve(tmeas.first.size());
            for (size_t i=0; i<tmeas.first.size(); ++i)
                term.overlaps_mps.push_back( mpo_to_smps_group(tmeas.first[i], psi_phys, allowed_blocks) );
            
            meas.add_term(term);
        }
        
        if (model["MEASURE_CONTINUUM[Psi energy]"]) {
            rho_mterm_t term;
            term.name = "Psi energy";
            term.type = mterm_t::DMOverlap;
            term.mps_ident = mps_ident;

            std::vector<psi_hamterm_t> terms_ops;
            psi_ham PsiH(psi_phys, psi_ident_op, terms_ops, psi_ident, psi_terms, psi_tag_handler);
            MPO<Matrix, U1> mpo = make_mpo(lat.size(), PsiH);
            term.overlaps_mps.push_back( mpo_to_smps_group(mpo, psi_phys, allowed_blocks) );
            
            meas.add_term(term);
        }
        
//        if (model["MEASURE_CONTINUUM[Onebody density matrix]"]) {
//            mterm_t term;
//            term.fill_operator = psi_ident;
//            term.name = "Onebody density matrix";
//            term.type = mterm_t::HalfCorrelation;
//            term.operators.push_back( std::make_pair(psi_create, false) );
//            term.operators.push_back( std::make_pair(psi_destroy, false) );
//            
//            meas.add_term(term);
//        }
        
        return meas;
    }
    
    typename base::initializer_ptr initializer(BaseParameters & p_) const
    {
        if ( p_["init_state"] == "identity_mps" ) {
            std::cout << "Identity MPS chosen." << std::endl;
            std::vector<Index<TwoU1> > allowed_blocks = allowed_sectors(lat.size(), phys, this->initc(model), 1);
            return typename base::initializer_ptr( new mps_ident_init<Matrix>( identity_dm_mps<Matrix>(lat.size(), psi_phys, allowed_blocks) ) );
        } else {
            return base::initializer(p_);
        }
    }

    
private:
    
    op_t adjoint_site_term(psi_op h) const
    {
        psi_op const& psi_ident_op = psi_tag_handler->get_op(psi_ident);
        /// h*rho*1 - 1*rho*h = (1 \otimes h) - (h^T \otimes 1) * rho
        ///                   = rho * (1 \otimes h^T) - (h \otimes 1)
        op_t idh, hid;
        dm_group_kron(psi_phys, h,            psi_ident_op, hid);
        h.transpose_inplace();
        dm_group_kron(psi_phys, psi_ident_op, h,            idh);
        if ( model["RUN_FINITE_T"] )
            return idh;
        else
            return idh - hid;
    }
        
    std::vector<std::pair<op_t,op_t> > adjoint_bond_term(psi_op h1, psi_op h2) const
    {
        psi_op const& psi_ident_op = psi_tag_handler->get_op(psi_ident);
        /// 1*rho*h = (h^T \otimes 1) * rho
        ///         = rho * (h \otimes 1)
        op_t h1id, h2id;
        dm_group_kron(psi_phys, h1, psi_ident_op, h1id);
        dm_group_kron(psi_phys, h2, psi_ident_op, h2id);
        
        /// h*rho*1 = (1 \otimes h) * rho
        ///         = rho * (1 \otimes h^T)
        op_t idh1, idh2;
        h1.transpose_inplace();
        h2.transpose_inplace();
        dm_group_kron(psi_phys, psi_ident_op, h1, idh1);
        dm_group_kron(psi_phys, psi_ident_op, h2, idh2);
        
        std::vector<std::pair<op_t,op_t> > ret; ret.reserve(2);
        ret.push_back( std::make_pair(idh1, idh2) );
        if ( ! static_cast<bool>(model["RUN_FINITE_T"]) )
            ret.push_back( std::make_pair(-1.*h1id, h2id) );
        
        return ret;
    }
    
    
    const Lattice & lat;
    BaseParameters & model;
    
    Index<U1> psi_phys;
    tag_type psi_ident, psi_count, psi_interaction, psi_create, psi_destroy;
    
    Index<TwoU1> phys;
    tag_type ident, count, interaction;
    
    table_ptr tag_handler;
    psi_table_ptr psi_tag_handler;
    
    std::vector<hamtagterm_t> terms;
    std::vector<psi_hamtagterm_t> psi_terms;
};


#endif
