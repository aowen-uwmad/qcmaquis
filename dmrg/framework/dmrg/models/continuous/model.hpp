/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Michele Dolfi <dolfim@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef CONTINUOUS_MODELS_H
#define CONTINUOUS_MODELS_H

#include "dmrg/models/hamiltonian.h"
#include "dmrg/models/measurements.h"

#include <cmath>

#include "dmrg/utils/BaseParameters.h"

namespace app {
    
    /* ****************** OPTICAL LATTICE (with symmetry) */
    template<class Matrix>
    class OpticalLattice {
        typedef Hamiltonian<Matrix, U1> ham;        
        typedef typename ham::hamterm_t hamterm_t;        
        typedef Measurement_Term<Matrix, U1> mterm_t;
        typedef typename ham::op_t op_t;
    public:
        OpticalLattice (const Lattice& lat_, BaseParameters & model_)
        : lat(lat_)
        , model(model_)
        {
            phys.insert(std::make_pair(0, 1));
            ident.insert_block(Matrix(1, 1, 1), 0, 0);
            
            for (int n=1; n<=model.get<int>("Nmax"); ++n)
            {
                phys.insert(std::make_pair(n, 1));
                
                ident.insert_block(Matrix(1, 1, 1), n, n);
                
                count.insert_block(Matrix(1, 1, n), n, n);
                if ((n*n-n) != 0)
                    interaction.insert_block(Matrix(1, 1, n*n-n), n, n);
                
                
                create.insert_block(Matrix(1, 1, std::sqrt(n)), n-1, n);
                destroy.insert_block(Matrix(1, 1, std::sqrt(n)), n, n-1);
            }
        }
        
        Hamiltonian<Matrix, U1> H () const
        {
            std::vector<hamterm_t> terms;
            for (int p=0; p<lat.size(); ++p)
            {
                std::vector<int> neighs = lat.all(p);
                // normal, b0, b1:
                // (nothing)
                
                // b2:
                //                if (lat.get_prop<bool>("at_open_left_boundary", p))
                //                    neighs.push_back(p+2);
                //                if (lat.get_prop<bool>("at_open_right_boundary", p))
                //                    neighs.push_back(p-2);
                
                
                double exp_potential = model.get<double>("V0")*std::pow( std::cos(model.get<double>("k")*lat.get_prop<double>("x", p)), 2 );
                
                //              double dx = std::min(lat.get_prop<double>("dx", p, p+1), lat.get_prop<double>("dx", p-1, p));
                double dx1 = lat.get_prop<double>("dx", p, neighs[0]);
                double dx2;
                if (neighs.size() == 1 && lat.get_prop<bool>("at_open_left_boundary", p))
                    dx2 = lat.get_prop<double>("dx", p, p-1);
                else if (neighs.size() == 1 && lat.get_prop<bool>("at_open_right_boundary", p))
                    dx2 = lat.get_prop<double>("dx", p, p+1);
                else
                    dx2 = lat.get_prop<double>("dx", p, neighs[1]);
                
                //                double dx0 = (std::abs(dx1) + std::abs(dx2)) / 2.;
                //                dx0 = std::min(std::abs(dx1), std::abs(dx2));
                
                double dx0 = lat.get_prop<double>("dx", p);
                
                // Psi''(x) = coeff1 * Psi(x+dx1) + coeff0 * Psi(x) + coeff2 * Psi(x+dx2)
                double coeff1 = 2. / (dx1*dx1 - dx1*dx2);
                double coeff2 = 2. / (dx2*dx2 - dx1*dx2);
                double coeff0 = -(coeff1 + coeff2);
                
                if (lat.get_prop<bool>("at_open_boundary", p)) {
                    //                    dx0 = std::min(std::abs(dx1),std::abs(dx2));
                    // b0, b2:
                    // (nothing)
                    
                    // normal:
                    //                    coeff0 = -1./(dx0*dx0);
                    
                    // b1:
                    //                    coeff0 = -coeff1;
                }
                
                double U = model.get<double>("c") / dx0;
                double mu = -model.get<double>("mu") + exp_potential;
                mu += -coeff0 * model.get<double>("h");
                
                /*
                 if (!lat.get_prop<bool>("at_open_boundary", p) && equal_grid)
                 mu += 2 * model.get<double>("h") / (dx*dx);
                 else if (lat.get_prop<bool>("at_open_boundary", p) && equal_grid)
                 mu += model.get<double>("h") / (dx*dx);
                 else if (!lat.get_prop<bool>("at_open_boundary", p) && !equal_grid)
                 mu += model.get<double>("h") / (dx*dx);
                 else if (lat.get_prop<bool>("at_open_right_boundary", p))
                 mu += 2./3. * model.get<double>("h") / (dx*dx);
                 else if (lat.get_prop<bool>("at_open_left_boundary", p))
                 mu += 1./3. * model.get<double>("h") / (dx*dx);
                 */
                
                op_t site_op;
                for (int n=1; n<=model.get<int>("Nmax"); ++n)
                    if (U*(n*n-n)+mu*n != 0)
                        site_op.insert_block(Matrix(1, 1, U*(n*n-n)+mu*n), n, n);
                
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
                //                    term.with_sign = false;
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
                
                for (int n=0; n<neighs.size(); ++n) { // hopping
                    
                    double t;
                    /*
                     // if (equal_grid || lat.get_prop<bool>("at_open_boundary", p))
                     if (equal_grid)
                     t = model.get<double>("h") / (dx*dx);
                     else if (lat.get_prop<double>("dx", p, neighs[n]) == dx)
                     t = 2./3. * model.get<double>("h") / (dx*dx);
                     else if (lat.get_prop<double>("dx", p, neighs[n]) == 2*dx)
                     t = 1./3. * model.get<double>("h") / (dx*dx);
                     else
                     throw std::runtime_error("I don't know the Laplacian operator in this kind of lattice!");
                     */
                    if (lat.get_prop<double>("dx", p, neighs[n]) == dx1)
                        t = coeff1 * model.get<double>("h");
                    else
                        t = coeff2 * model.get<double>("h");
                    
                    {
                        hamterm_t term;
                        term.with_sign = false;
                        term.fill_operator = ident;
                        term.operators.push_back( std::make_pair(p, -t*create) );
                        term.operators.push_back( std::make_pair(neighs[n], destroy) );
                        terms.push_back(term);
                    }
                }
            }
            
            return ham(phys, ident, terms);
        }
        
        Measurements<Matrix, U1> measurements () const
        {
            Measurements<Matrix, U1> meas;
            meas.set_identity(ident);
            
            {
                mterm_t term;
                term.fill_operator = ident;
                term.name = "Density";
                term.type = mterm_t::Average;
                term.operators.push_back( std::make_pair(count, false) );
                
                meas.add_term(term);
            }
            {
                mterm_t term;
                term.fill_operator = ident;
                term.name = "Local density";
                term.type = mterm_t::Local;
                term.operators.push_back( std::make_pair(count, false) );
                
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
        Index<U1> phys;
    };
    
    
    /* ****************** OPTICAL LATTICE (without symmetry) */
    template<class Matrix>
    class OpticalLatticeNull {
        typedef Hamiltonian<Matrix, TrivialGroup> ham;        
        typedef typename ham::hamterm_t hamterm_t;        
        typedef Measurement_Term<Matrix, TrivialGroup> mterm_t;
        typedef typename ham::op_t op_t;
    public:
        OpticalLatticeNull (const Lattice& lat_, BaseParameters & model_)
        : lat(lat_)
        , model(model_)
        {
            
            throw std::runtime_error("This model is broken, it has to be adapted to the one above...");
            
            int phys_size = model.get<int>("Nmax")+1;
            TrivialGroup::charge c = TrivialGroup::IdentityCharge;
            
            phys.insert(std::make_pair(c, phys_size));
            ident.insert_block(Matrix::identity_matrix(phys_size), c, c);
            count.insert_block(Matrix(phys_size, phys_size, 0), c, c);
            interaction.insert_block(Matrix(phys_size, phys_size, c), c, c);
            create.insert_block(Matrix(phys_size, phys_size, 0), c, c);
            destroy.insert_block(Matrix(phys_size, phys_size, 0), c, c);
            
            for (int n=1; n<=model.get<int>("Nmax"); ++n)
            {                                
                count[0](n, n) = n;
                
                interaction[0](n, n) = n*n-n;
                
                create[0](n-1, n) = std::sqrt(n);
                destroy[0](n, n-1) = std::sqrt(n);
            }
        }
        
        Hamiltonian<Matrix, TrivialGroup> H () const
        {
            TrivialGroup::charge c = TrivialGroup::IdentityCharge;
            
            std::vector<hamterm_t> terms;
            for (int p=0; p<lat.size(); ++p)
            {
                double exp_potential = model.get<double>("V0")*std::pow( std::cos(model.get<double>("k")*lat.get_prop<double>("x", p)), 2 );
                
                double U = model.get<double>("c")/lat.get_prop<double>("dx", p);
                double mu = ( -model.get<double>("mu")
                             + exp_potential
                             + model.get<double>("h")/(lat.get_prop<double>("dx", p)*lat.get_prop<double>("dx", p)) );
                if (!lat.get_prop<bool>("at_open_boundary", p))
                    mu += model.get<double>("h")/(lat.get_prop<double>("dx", p)*lat.get_prop<double>("dx", p));
                
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
                    
                    double t = model.get<double>("h") / (lat.get_prop<double>("dx", p, neighs[n])*lat.get_prop<double>("dx", p, neighs[n]));
                    
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
            
            {
                mterm_t term;
                term.fill_operator = ident;
                term.name = "Density";
                term.type = mterm_t::Average;
                term.operators.push_back( std::make_pair(count, false) );
                
                meas.add_term(term);
            }
            {
                mterm_t term;
                term.fill_operator = ident;
                term.name = "Local density";
                term.type = mterm_t::Local;
                term.operators.push_back( std::make_pair(count, false) );
                
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
    
    
    
    /* ****************** FERMIONIC OPTICAL LATTICE (with symmetry) */
    template<class Matrix>
    class FermiOpticalLattice {
        typedef Hamiltonian<Matrix, TwoU1> ham;        
        typedef typename ham::hamterm_t hamterm_t;        
        typedef Measurement_Term<Matrix, TwoU1> mterm_t;
        typedef typename ham::op_t op_t;
    public:
        FermiOpticalLattice (const Lattice& lat_, BaseParameters & model_)
        : lat(lat_)
        , model(model_)
        , none(_(0,0))
        , up(_(1,0))
        , down(_(0,1))
        , updown(_(1,1))
        {
            phys.insert(std::make_pair(none, 1));
            phys.insert(std::make_pair(up, 1));
            phys.insert(std::make_pair(down, 1));
            phys.insert(std::make_pair(updown, 1));
            
            ident.insert_block(Matrix(1, 1, 1), none, none);
            ident.insert_block(Matrix(1, 1, 1), up, up);
            ident.insert_block(Matrix(1, 1, 1), down, down);
            ident.insert_block(Matrix(1, 1, 1), updown, updown);
            
            count.insert_block(Matrix(1, 1, 1), up, up);
            count.insert_block(Matrix(1, 1, 1), down, down);
            count.insert_block(Matrix(1, 1, 2), updown, updown);
            
            count_up.insert_block(Matrix(1, 1, 1), up, up);
            count_down.insert_block(Matrix(1, 1, 1), down, down);
            count_up.insert_block(Matrix(1, 1, 1), updown, updown);
            count_down.insert_block(Matrix(1, 1, 1), updown, updown);
            
            sign_up.insert_block(Matrix(1, 1, 1), none, none);
            sign_up.insert_block(Matrix(1, 1, -1), up, up);
            sign_up.insert_block(Matrix(1, 1, 1), down, down);
            sign_up.insert_block(Matrix(1, 1, 1), updown, updown);
            sign_down.insert_block(Matrix(1, 1, 1), none, none);
            sign_down.insert_block(Matrix(1, 1, 1), up, up);
            sign_down.insert_block(Matrix(1, 1, -1), down, down);
            sign_down.insert_block(Matrix(1, 1, 1), updown, updown);
            
            create_up.insert_block(Matrix(1, 1, 1), none, up);
            create_up.insert_block(Matrix(1, 1, 1), down, updown);
            create_down.insert_block(Matrix(1, 1, 1), none, down);
            create_down.insert_block(Matrix(1, 1, 1), up, updown);
            
            destroy_up.insert_block(Matrix(1, 1, 1), up, none);
            destroy_up.insert_block(Matrix(1, 1, 1), updown, down);
            destroy_down.insert_block(Matrix(1, 1, 1), down, none);
            destroy_down.insert_block(Matrix(1, 1, 1), updown, up);
            
            doubly_occ.insert_block(Matrix(1, 1, 1), updown, updown);
        }
        
        Hamiltonian<Matrix, TwoU1> H () const
        {
            std::vector<hamterm_t> terms;
            for (int p=0; p<lat.size(); ++p)
            {
                std::vector<int> neighs = lat.all(p);
                // normal, b0, b1:
                // (nothing)
                
                // b2:
                //                if (lat.get_prop<bool>("at_open_left_boundary", p))
                //                    neighs.push_back(p+2);
                //                if (lat.get_prop<bool>("at_open_right_boundary", p))
                //                    neighs.push_back(p-2);
                
                
                double exp_potential = model.get<double>("V0")*std::pow( std::cos(model.get<double>("k")*lat.get_prop<double>("x", p)), 2 );
                
                //              double dx = std::min(lat.get_prop<double>("dx", p, p+1), lat.get_prop<double>("dx", p-1, p));
                double dx1 = lat.get_prop<double>("dx", p, neighs[0]);
                double dx2;
                if (neighs.size() == 1 && lat.get_prop<bool>("at_open_left_boundary", p))
                    dx2 = lat.get_prop<double>("dx", p, p-1);
                else if (neighs.size() == 1 && lat.get_prop<bool>("at_open_right_boundary", p))
                    dx2 = lat.get_prop<double>("dx", p, p+1);
                else
                    dx2 = lat.get_prop<double>("dx", p, neighs[1]);
                
                //                double dx0 = (std::abs(dx1) + std::abs(dx2)) / 2.;
                //                dx0 = std::min(std::abs(dx1), std::abs(dx2));
                
                double dx0 = lat.get_prop<double>("dx", p);
                
                // Psi''(x) = coeff1 * Psi(x+dx1) + coeff0 * Psi(x) + coeff2 * Psi(x+dx2)
                double coeff1 = 2. / (dx1*dx1 - dx1*dx2);
                double coeff2 = 2. / (dx2*dx2 - dx1*dx2);
                double coeff0 = -(coeff1 + coeff2);
                
                if (lat.get_prop<bool>("at_open_boundary", p)) {
                    //                    dx0 = std::min(std::abs(dx1),std::abs(dx2));
                    // b0, b2:
                    // (nothing)
                    
                    // normal:
                    //                    coeff0 = -1./(dx0*dx0);
                    
                    // b1:
                    //                    coeff0 = -coeff1;
                }
                
                double U = model.get<double>("c") / dx0;
                double mu = -model.get<double>("mu") + exp_potential;
                mu += -coeff0 * model.get<double>("h");
                
                /*
                 if (!lat.get_prop<bool>("at_open_boundary", p) && equal_grid)
                 mu += 2 * model.get<double>("h") / (dx*dx);
                 else if (lat.get_prop<bool>("at_open_boundary", p) && equal_grid)
                 mu += model.get<double>("h") / (dx*dx);
                 else if (!lat.get_prop<bool>("at_open_boundary", p) && !equal_grid)
                 mu += model.get<double>("h") / (dx*dx);
                 else if (lat.get_prop<bool>("at_open_right_boundary", p))
                 mu += 2./3. * model.get<double>("h") / (dx*dx);
                 else if (lat.get_prop<bool>("at_open_left_boundary", p))
                 mu += 1./3. * model.get<double>("h") / (dx*dx);
                 */
                
                op_t site_op = mu*count_up;
                site_op += mu*mu*count_down;
                
                { // site term
                    hamterm_t term;
                    term.with_sign = false;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, site_op) );
                    terms.push_back(term);
                }
                
                for (int n=0; n<neighs.size(); ++n) { // hopping
                    
                    double t;
                    /*
                     // if (equal_grid || lat.get_prop<bool>("at_open_boundary", p))
                     if (equal_grid)
                     t = model.get<double>("h") / (dx*dx);
                     else if (lat.get_prop<double>("dx", p, neighs[n]) == dx)
                     t = 2./3. * model.get<double>("h") / (dx*dx);
                     else if (lat.get_prop<double>("dx", p, neighs[n]) == 2*dx)
                     t = 1./3. * model.get<double>("h") / (dx*dx);
                     else
                     throw std::runtime_error("I don't know the Laplacian operator in this kind of lattice!");
                     */
                    if (lat.get_prop<double>("dx", p, neighs[n]) == dx1)
                        t = coeff1 * model.get<double>("h");
                    else
                        t = coeff2 * model.get<double>("h");
                    
                    {
                        hamterm_t term;
                        term.with_sign = true;
                        term.fill_operator = sign_up;
                        term.operators.push_back( std::make_pair(p, -t*create_up) );
                        term.operators.push_back( std::make_pair(neighs[n], destroy_up) );
                        terms.push_back(term);
                    }
                    {
                        hamterm_t term;
                        term.with_sign = true;
                        term.fill_operator = sign_down;
                        term.operators.push_back( std::make_pair(p, -t*create_down) );
                        term.operators.push_back( std::make_pair(neighs[n], destroy_down) );
                        terms.push_back(term);
                    }
                }
            }
            
            return ham(phys, ident, terms);
        }
        
        Measurements<Matrix, TwoU1> measurements () const
        {
            Measurements<Matrix, TwoU1> meas;
            meas.set_identity(ident);
            
            {
                mterm_t term;
                term.fill_operator = ident;
                term.name = "Density";
                term.type = mterm_t::Average;
                term.operators.push_back( std::make_pair(count, false) );
                
                meas.add_term(term);
            }
            {
                mterm_t term;
                term.fill_operator = ident;
                term.name = "Local density up";
                term.type = mterm_t::Local;
                term.operators.push_back( std::make_pair(count_up, false) );
                
                meas.add_term(term);
            }
            {
                mterm_t term;
                term.fill_operator = ident;
                term.name = "Local density down";
                term.type = mterm_t::Local;
                term.operators.push_back( std::make_pair(count_down, false) );
                
                meas.add_term(term);
            }
            return meas;
        }
        
    private:
        const Lattice & lat;
        BaseParameters & model;
        
        op_t ident;
        op_t create_up, destroy_up;
        op_t create_down, destroy_down;
        op_t count, doubly_occ;
        op_t count_up, count_down;
        op_t sign_up, sign_down;
        Index<TwoU1> phys;
        
        TwoU1::charge none, up, down, updown;
    };
    
    
    
} // namespace

#endif
