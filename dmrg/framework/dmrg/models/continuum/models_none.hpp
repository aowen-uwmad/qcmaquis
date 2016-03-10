/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *                            Michele Dolfi <dolfim@phys.ethz.ch>
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

#ifndef DMRG_CONTINUOUS_MODELS_NONE_H
#define DMRG_CONTINUOUS_MODELS_NONE_H

#include <sstream>

#include "dmrg/models/model.h"
#include "dmrg/models/measurements.h"
#include "dmrg/utils/BaseParameters.h"

/* ****************** OPTICAL LATTICE (without symmetry) */
template<class Matrix>
class OpticalLatticeNull : public Model<Matrix, TrivialGroup> {
    typedef Hamiltonian<Matrix, TrivialGroup> ham;        
    typedef typename ham::hamterm_t hamterm_t;        
    typedef Measurement_Term<Matrix, TrivialGroup> mterm_t;
    typedef typename ham::op_t op_t;
    typedef typename Matrix::value_type value_type;
public:
    OpticalLatticeNull (const Lattice& lat_, BaseParameters & model_)
    : lat(lat_)
    , model(model_)
    {
        
        throw std::runtime_error("This model is broken, it has to be adapted to the one above...");
        
        int phys_size = model["Nmax"]+1;
        TrivialGroup::charge c = TrivialGroup::IdentityCharge;
        
        phys.insert(std::make_pair(c, phys_size));
        ident.insert_block(Matrix::identity_matrix(phys_size), c, c);
        count.insert_block(Matrix(phys_size, phys_size, 0), c, c);
        interaction.insert_block(Matrix(phys_size, phys_size, 0), c, c);
        create.insert_block(Matrix(phys_size, phys_size, 0), c, c);
        destroy.insert_block(Matrix(phys_size, phys_size, 0), c, c);
        
        for (int n=1; n<=model["Nmax"]; ++n)
        {                                
            count[0](n, n) = n;
            
            interaction[0](n, n) = n*n-n;
            
            create[0](n-1, n) = std::sqrt(value_type(n));
            destroy[0](n, n-1) = std::sqrt(value_type(n));
        }
    }
    
    Index<TrivialGroup> get_phys() const
    {
        return phys;
    }

    Hamiltonian<Matrix, TrivialGroup> H () const
    {
        TrivialGroup::charge c = TrivialGroup::IdentityCharge;
        
        std::vector<hamterm_t> terms;
        for (int p=0; p<lat.size(); ++p)
        {
            double exp_potential = model["V0"]*std::pow( std::cos(model["k"]*lat.get_prop<double>("x", p)), 2 );
            
            double U = model["c"]/lat.get_prop<double>("dx", p);
            double mu = (  exp_potential
                         - model["mu"]
                         + model["h"]/(lat.get_prop<double>("dx", p)*lat.get_prop<double>("dx", p)) );
            if (!lat.get_prop<bool>("at_open_boundary", p))
                mu += model["h"]/(lat.get_prop<double>("dx", p)*lat.get_prop<double>("dx", p));
            
            op_t site_op;
            site_op.insert_block(U*interaction[0]+mu*count[0], c, c);
            
            { // site term
                hamterm_t term;
                term.with_sign = false;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, site_op) );
                terms.push_back(term);
            }
            
            //                if (mu != 0)
            //                { // density term
            //                    hamterm_t term;
            //                    term.fill_operator = ident;
            //                    term.operators.push_back( std::make_pair(p, mu*count) );
            //                    terms.push_back(term);
            //                }
            //                
            //                if (U != 0)
            //                { // interaction term
            //                    hamterm_t term;
            //                    term.fill_operator = ident;
            //                    term.operators.push_back( std::make_pair(p, U*interaction) );
            //                    terms.push_back(term);
            //                }
            
            std::vector<int> neighs = lat.forward(p);
            for (int n=0; n<neighs.size(); ++n) { // hopping
                
                double t = model["h"] / (lat.get_prop<double>("dx", p, neighs[n])*lat.get_prop<double>("dx", p, neighs[n]));
                
                {
                    hamterm_t term;
                    term.with_sign = false;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, -t*create) );
                    term.operators.push_back( std::make_pair(neighs[n], destroy) );
                    terms.push_back(term);
                }
                {
                    hamterm_t term;
                    term.with_sign = false;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, -t*destroy) );
                    term.operators.push_back( std::make_pair(neighs[n], create) );
                    terms.push_back(term);
                }
            }
        }
        
        return ham(phys, ident, terms);
    }
    
    Measurements<Matrix, TrivialGroup> measurements () const
    {
        Measurements<Matrix, TrivialGroup> meas;
        meas.set_identity(ident);
        
        if (model["MEASURE_CONTINUUM[Density]"]) {
            mterm_t term;
            term.fill_operator = ident;
            term.name = "Density";
            term.type = mterm_t::Average;
            term.operators.push_back( std::make_pair(count, false) );
            
            meas.add_term(term);
        }
        if (model["MEASURE_CONTINUUM[Local density]"]) {
            mterm_t term;
            term.fill_operator = ident;
            term.name = "Local density";
            term.type = mterm_t::Local;
            term.operators.push_back( std::make_pair(count, false) );
            
            meas.add_term(term);
        }

        if (model["MEASURE_CONTINUUM[Onebody density matrix]"]) {
            mterm_t term;
            term.fill_operator = ident;
            term.name = "Onebody density matrix";
            term.type = mterm_t::HalfCorrelation;
            term.operators.push_back( std::make_pair(create, false) );
            term.operators.push_back( std::make_pair(destroy, false) );
            
            meas.add_term(term);
        }

        return meas;
    }
    
private:
    const Lattice & lat;
    BaseParameters & model;
    
    op_t ident;
    op_t create, destroy;
    op_t count, interaction;
    Index<TrivialGroup> phys;
};

#endif
