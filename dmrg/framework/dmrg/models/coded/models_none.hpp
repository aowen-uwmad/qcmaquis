/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *                            Michele Dolfi <dolfim@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef MODELS_CODED_NONE_H
#define MODELS_CODED_NONE_H

#include <sstream>

#include "dmrg/models/model.h"
#include "dmrg/utils/BaseParameters.h"

/* ****************** BOSE-HUBBARD */
template<class Matrix>
class BoseHubbardNone : public Model<Matrix, TrivialGroup>
{
    typedef Hamiltonian<Matrix, TrivialGroup> ham;
    typedef typename ham::hamterm_t hamterm_t;
    typedef typename ham::op_t op_t;
    typedef Measurement_Term<Matrix, TrivialGroup> mterm_t;
    
public:
    BoseHubbardNone (const Lattice& lat, BaseParameters & model_)
    : model(model_)
    {
        int Nmax = model["Nmax"];
        double U = model["U"];
        double t = model["t"];
        double V = model["V"];
        
        TrivialGroup::charge C = TrivialGroup::IdentityCharge;
        size_t N = Nmax+1;
        
        phys.insert(std::make_pair(C, N));
        ident.insert_block(Matrix::identity_matrix(N), C, C);
        
        Matrix mcount(N,N), minteraction(N,N), mcreate(N,N), mdestroy(N,N);
        for (int n=1; n<=Nmax; ++n)
        {
            mcount(n,n) = n;
            if ((n*n-n) != 0)
                minteraction(n,n) = n*n-n;
            
            mcreate(n-1,n) = std::sqrt(n);   // input n-1, output n
            mdestroy(n,n-1) = std::sqrt(n);  // input n,   output n-1
        }
        count.insert_block(mcount, C,C);
        interaction.insert_block(minteraction, C,C);
        create.insert_block(mcreate, C,C);
        destroy.insert_block(mdestroy, C,C);
        
        for (int p=0; p<lat.size(); ++p) {
            /* interaction */
            {
                hamterm_t term;
                term.with_sign = false;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, interaction) );
                terms.push_back(term);
            }
            
            std::vector<int> neighs = lat.forward(p);
            for (int n=0; n<neighs.size(); ++n) {
                /* hopping */
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, -t*create) );
                    term.operators.push_back( std::make_pair(neighs[n], destroy) );
                    terms.push_back(term);
                }
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, -t*destroy) );
                    term.operators.push_back( std::make_pair(neighs[n], create) );
                    terms.push_back(term);
                }
                /* nearest-neighborn interaction */
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, V*count) );
                    term.operators.push_back( std::make_pair(neighs[n], count) );
                    terms.push_back(term);
                }
            }
        }
        
    }
    
    Index<TrivialGroup> get_phys() const
    {
        return phys;
    }
    
    Hamiltonian<Matrix, TrivialGroup> H () const
    {
        return ham(phys, ident, terms);
    }
    
    Measurements<Matrix, TrivialGroup> measurements () const
    {
        Measurements<Matrix, TrivialGroup> meas;
        meas.set_identity(ident);
        
        if (model["ENABLE_MEASURE[Density]"]) {
            mterm_t term;
            term.fill_operator = ident;
            term.name = "Density";
            term.type = mterm_t::Average;
            term.operators.push_back( std::make_pair(count, false) );
            
            meas.add_term(term);
        }
        if (model["ENABLE_MEASURE[Local density]"]) {
            mterm_t term;
            term.fill_operator = ident;
            term.name = "Local density";
            term.type = mterm_t::Local;
            term.operators.push_back( std::make_pair(count, false) );
            
            meas.add_term(term);
        }
        
        if (model["ENABLE_MEASURE[Onebody density matrix]"]) {
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
    
    op_t get_op(std::string const & op) const
    {
        if (op == "n")
            return count;
        else if (op == "bdag")
            return create;
        else if (op == "b")
            return destroy;
        else
            throw std::runtime_error("Operator not valid for this model.");
        return op_t();
    }
    
    
private:
    BaseParameters & model;

    op_t ident;
    op_t create, destroy, count, interaction;
    Index<TrivialGroup> phys;
    
    std::vector<hamterm_t> terms;
};



#endif
