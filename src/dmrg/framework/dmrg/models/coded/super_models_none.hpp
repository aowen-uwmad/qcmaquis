/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *                            Michele Dolfi <dolfim@phys.ethz.ch>
 *               2012      by Jan Gukelberger <gukelberger@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef SUPER_MODELS_NONE_HPP
#define SUPER_MODELS_NONE_HPP

#include <sstream>

#include "dmrg/models/model.h"
#include "dmrg/utils/BaseParameters.h"


/*
 * Time evolution for Bose-Hubbard density operators.
 *
 * The density operator rho is represented as a superstate whose time evolution
 * is generated by a superoperator according to the Lindblad master equation
 *      d rho/dt = (-i ad H + lind L) rho .
 * In this framework the time evolution of the density operator is computed
 * just like that of a pure state
 *      rho(t) = exp{ -i (ad H + i lind L) t } rho(0) .
 *
 * Operators are represented in the occupation number basis
 *      O_{ij} = <i|O|j>
 * with basis states {|i>}, i=0,...,N-1 for each site.
 * The corresponding superstate is obtained by the mapping
 *      O_{ij} -> O_n, n=i*N+j ,
 * i.e. the basis for operator states is given by the matrix elements
 *      |i><j|
 *
 * Superoperators represent the map
 *      rho -> d rho/dt .
 * We mostly need the adjoint Hamiltonian superoperator
 *      ad H: rho -> [H,rho]
 * and the dissipative superoperator
 *      lind L: rho -> 2 L rho L^\dag - L^\dag L rho - rho L^\dag L .
 * The superoperator representation of any linear map is conveniently computed
 * by applying the map to all N^2 operator basis states, filling the
 * corresponding columns of the superoperator.
 */


/// Take a Hamilton operator H and construct its adjoint Hamiltonian
/// superoperator, corresponding to the map
///   ad H: rho -> [H,rho]
template<class Matrix>
Matrix adjoint_hamiltonian(const Matrix& h)
{
    const size_t N = num_rows(h);
    const size_t N2 = N*N;
    assert( num_cols(h) == N );

    Matrix adH(N2,N2);
    for( size_t i = 0; i < N; ++i )
    {
        for( size_t j = 0; j < N; ++j )
        {
            const size_t n = i + j*N;
            Matrix phi (N,N);   phi(i,j) = 1.;      // n'th operator basis state phi
            Matrix hphi(N,N);   gemm(h,phi,hphi);   // H.phi
            Matrix phih(N,N);   gemm(phi,h,phih);   // phi.H
            Matrix comm = hphi - phih;              // [H,phi]
            
            // operator state adH.phi -> n'th column of adH
            for( size_t ii = 0; ii < N; ++ii )
                for( size_t jj = 0; jj < N; ++jj )
                    adH(ii+jj*N, n) = comm(ii,jj);
        }
    }
    
    return adH;
}

/// Take a Lindblad operator L and construct its contribution to the Liouville 
/// superoperator, i.e. the map
///   lind L: rho -> 2 L rho L^\dag - L^\dag L rho - rho L^\dag L
template<class Matrix>
Matrix super_lindblad(const Matrix& l)
{
    const size_t N = num_rows(l);
    const size_t N2 = N*N;
    assert( num_cols(l) == N );

    Matrix ldag = transpose(conj(l));       // L^\dag
    Matrix ldagl(N,N); gemm(ldag,l,ldagl);  // L^\dag L
    Matrix lindL(N2,N2);

    for( size_t i = 0; i < N; ++i )
    {
        for( size_t j = 0; j < N; ++j )
        {
            const size_t n = i + j*N;
            Matrix phi (N,N);   phi(i,j) = 1.;  // n'th operator basis state phi

            // phi -> lindL.phi = 2*l.phi.ldag - ldag.l.phi - phi.ldag.l
            Matrix lphi    (N,N); gemm(l,phi,lphi);
            Matrix lphildag(N,N); gemm(lphi,ldag,lphildag);
            Matrix ldaglphi(N,N); gemm(ldag,lphi,ldaglphi);
            Matrix phildagl(N,N); gemm(phi,ldagl,phildagl);
            Matrix ll = 2*lphildag - ldaglphi - phildagl;
            
            // operator state lindL.phi -> n'th column of lindL
            for( size_t ii = 0; ii < N; ++ii )
                for( size_t jj = 0; jj < N; ++jj )
                    lindL(ii+jj*N, n) = ll(ii,jj);
        }
    }

    return lindL;
}

/// Take a Lindblad operator L and construct the map corresponding to left
/// multiplication with L
///   rho -> L rho
template<class Matrix>
Matrix super_left(const Matrix& l)
{
    const size_t N = num_rows(l);
    const size_t N2 = N*N;
    assert( num_cols(l) == N );

    Matrix superL(N2,N2);

    for( size_t i = 0; i < N; ++i )
    {
        for( size_t j = 0; j < N; ++j )
        {
            const size_t n = i + j*N;
            Matrix phi(N,N);   phi(i,j) = 1.;  // n'th operator basis state phi

            // phi -> L.phi
            Matrix lphi(N,N); gemm(l,phi,lphi);
            for( size_t ii = 0; ii < N; ++ii )
                for( size_t jj = 0; jj < N; ++jj )
                    superL(ii+jj*N, n) = lphi(ii,jj);
        }
    }

    return superL;
}

/// Take a Lindblad operator L and construct the map corresponding to right
/// multiplication with L
///   rho -> rho L
template<class Matrix>
Matrix super_right(const Matrix& l)
{
    const size_t N = num_rows(l);
    const size_t N2 = N*N;
    assert( num_cols(l) == N );

    Matrix superL(N2,N2);

    for( size_t i = 0; i < N; ++i )
    {
        for( size_t j = 0; j < N; ++j )
        {
            const size_t n = i + j*N;
            Matrix phi (N,N);   phi(i,j) = 1.;  // n'th operator basis state phi

            // phi -> phi.L
            Matrix phil(N,N); gemm(phi,l,phil);
            for( size_t ii = 0; ii < N; ++ii )
                for( size_t jj = 0; jj < N; ++jj )
                    superL(ii+jj*N, n) = phil(ii,jj);
        }
    }

    return superL;
}


/* ****************** BOSE-HUBBARD */
template<class Matrix>
class SuperBoseHubbardNone : public Model<Matrix, TrivialGroup>
{
    typedef Hamiltonian<Matrix, TrivialGroup> ham;
    typedef typename ham::hamterm_t hamterm_t;
    typedef typename ham::op_t op_t;
    typedef Measurement_Term<Matrix, TrivialGroup> mterm_t;
    
public:
    // Dissipation needs complex types, that's why we forward to do_init with a tag
    SuperBoseHubbardNone(const Lattice& lat, BaseParameters & model_)
    : model(model_)
    {
        do_init(lat,model.get<int>("Nmax"),model.get<double>("t"),model.get<double>("U"),model.get<double>("V"),
                model.get<double>("Delta"),model.get<double>("Gamma1a"),model.get<double>("Gamma1b"),model.get<double>("Gamma2"),
                typename Matrix::value_type());
    }
    
    void do_init(const Lattice& lat, int Nmax, double t, double U, double V, 
                 double Delta, double Gamma1a, double Gamma1b, double Gamma2, double)
    {
        throw std::runtime_error("need complex value type");
    }
    
    void do_init(const Lattice& lat, int Nmax, double t, double U, double V, 
                 double Delta, double Gamma1a, double Gamma1b, double Gamma2, std::complex<double>)
    {
        TrivialGroup::charge C = TrivialGroup::IdentityCharge;
        const size_t N = Nmax+1;
        const size_t N2 = N*N;
        const std::complex<double> I(0,1);
        
        phys_psi.insert(std::make_pair(C, N));
        
        phys.insert(std::make_pair(C, N2));
        ident.insert_block(Matrix::identity_matrix(N2), C, C);
        
        // construct basic on-site operators
        mcount.resize(N,N); minteraction.resize(N,N); mcreate.resize(N,N); mdestroy.resize(N,N);
        for( int n = 1; n <= Nmax; ++n )
        {
            mcount(n,n) = n;
            if ((n*n-n) != 0)
                minteraction(n,n) = n*n-n;
            
            mcreate(n,n-1) = std::sqrt(n);   // input n-1, output n
            mdestroy(n-1,n) = std::sqrt(n);  // input n,   output n-1
        }
        Matrix mcreate2 (N,N);   gemm(mcreate ,mcreate ,mcreate2 );
        Matrix mdestroy2(N,N);   gemm(mdestroy,mdestroy,mdestroy2);
        
        // construct on-site superoperators
        Matrix screate      = adjoint_hamiltonian(mcreate);
        Matrix sdestroy     = adjoint_hamiltonian(mdestroy);
        Matrix spump        = adjoint_hamiltonian(mcreate2+mdestroy2);
        Matrix scount       = adjoint_hamiltonian(mcount);
        Matrix sinteraction = adjoint_hamiltonian(minteraction);
        
        Matrix ldestroy  = super_lindblad(mdestroy );
        Matrix ldestroy2 = super_lindblad(mdestroy2);
        
        // cast superoperators to op_t
        create     .insert_block(transpose(screate     ), C,C);
        destroy    .insert_block(transpose(sdestroy    ), C,C);
        pump       .insert_block(transpose(spump       ), C,C);
        count      .insert_block(transpose(scount      ), C,C);
        interaction.insert_block(transpose(sinteraction), C,C);

        lindDestroy .insert_block(transpose(ldestroy     ), C,C);
        lindDestroy2.insert_block(transpose(ldestroy2    ), C,C);
        
        leftDestroy.insert_block(transpose(super_left(mdestroy)), C,C);
        rightCreate.insert_block(transpose(super_right(mcreate)), C,C);
        
        
        // insert superoperators for each site
        for( int p=0; p < lat.size(); ++p ) 
        {
            // interaction H_U = U/2 n_i (n_i - 1)
            {
                hamterm_t term;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, 0.5*U*interaction) );
                terms.push_back(term);
            }
            
            // pump H_Delta = Delta (b_i^\dag^2 + b_i^2)
            {
                hamterm_t term;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, Delta*pump) );
                terms.push_back(term);
            }
            
            // one-boson dissipation L_{1a} = Gamma_{1a} lind b_i
            //   = Gamma_{1a} (2 b_i rho b_i^\dag - b_i^\dag b_i rho - rho b_i^\dag b_i)
            {
                hamterm_t term;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, I*Gamma1a*lindDestroy) );
                terms.push_back(term);
            }
            
            // two-boson dissipation L_2 = Gamma_2/2 lind b_i^2
            //   = Gamma_2/2 (2 b_i^2 rho b_i^\dag^2 - b_i^\dag^2 b_i^2 rho - rho b_i^\dag^2 b_i^2)
            {
                hamterm_t term;
                term.fill_operator = ident;
                term.operators.push_back( std::make_pair(p, I*0.5*Gamma2*lindDestroy2) );
                terms.push_back(term);
            }
            
            // bond terms
            std::vector<int> neighs = lat.forward(p);
            for( int n = 0; n < neighs.size(); ++n ) 
            {
                // hopping H_J = -J (b_i^\dag b_{i+1} + b_i b_{i+1}^\dag)
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
                
                // nearest-neighbor interaction H_V = V n_i n_{i+1}
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, V*count) );
                    term.operators.push_back( std::make_pair(neighs[n], count) );
                    terms.push_back(term);
                }
            
                // one-boson dissipation L_{1b} = -Gamma_{1b}/2 (
                //          2 b_{i+1} rho b_i^\dag - b_i^\dag b_{i+1} rho - rho b_i^\dag b_{i+1}
                //        + 2 b_i rho b_{i+1}^\dag - b_{i+1}^\dag b_i rho - rho b_{i+1}^\dag b_i )
                //  = Gamma_{1b}/2 (
                //          (ad b^\dag)_i (b rho)_{i+1} + (b rho)_i (ad b^\dag)_{i+1}
                //        - (ad b)_i (rho b^\dag)_{i+1} - (rho b^\dag)_i (ad b)_{i+1} )
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, I*Gamma1b/2.*create) );
                    term.operators.push_back( std::make_pair(neighs[n], leftDestroy) );
                    terms.push_back(term);
                }
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, I*Gamma1b/2.*leftDestroy) );
                    term.operators.push_back( std::make_pair(neighs[n], create) );
                    terms.push_back(term);
                }
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, I*Gamma1b/2.*destroy) );
                    term.operators.push_back( std::make_pair(neighs[n], rightCreate) );
                    terms.push_back(term);
                }
                {
                    hamterm_t term;
                    term.fill_operator = ident;
                    term.operators.push_back( std::make_pair(p, I*Gamma1b/2.*rightCreate) );
                    term.operators.push_back( std::make_pair(neighs[n], destroy) );
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
        TrivialGroup::charge C = TrivialGroup::IdentityCharge;
        
        op_t ident_psi = identity_matrix<Matrix>(phys_psi);
        op_t count_psi, create_psi, destroy_psi;
        count_psi.insert_block(mcount, C, C);
        create_psi.insert_block(transpose(mcreate), C, C);
        destroy_psi.insert_block(transpose(mdestroy), C, C);
        
        Measurements<Matrix, TrivialGroup> meas(Measurements<Matrix, TrivialGroup>::Densitymatrix);
        meas.set_identity(ident_psi);
        
        if (model.get<bool>("ENABLE_MEASURE[Density]")) {
            mterm_t term;
            term.fill_operator = ident_psi;
            term.name = "Density";
            term.type = mterm_t::Average;
            term.operators.push_back( std::make_pair(count_psi, false) );
            
            meas.add_term(term);
        }
        
        if (model.get<bool>("ENABLE_MEASURE[Local density]")) {
            mterm_t term;
            term.fill_operator = ident_psi;
            term.name = "Local density";
            term.type = mterm_t::Local;
            term.operators.push_back( std::make_pair(count_psi, false) );
            
            meas.add_term(term);
        }
        
        if (model.get<bool>("ENABLE_MEASURE[Onebody density matrix]")) {
            mterm_t term;
            term.fill_operator = ident_psi;
            term.name = "Onebody density matrix";
            term.type = mterm_t::HalfCorrelation;
            term.operators.push_back( std::make_pair(create_psi, false) );
            term.operators.push_back( std::make_pair(destroy_psi, false) );
            
            meas.add_term(term);
        }

        if (model.get<bool>("ENABLE_MEASURE[Density correlation]")) {
            mterm_t term;
            term.fill_operator = ident_psi;
            term.name = "Density correlation";
            term.type = mterm_t::HalfCorrelation;
            term.operators.push_back( std::make_pair(count_psi, false) );
            term.operators.push_back( std::make_pair(count_psi, false) );
            
            meas.add_term(term);
        }

        return meas;
    }
    
    op_t get_op(std::string const & op) const
    {
        return op_t();
    }
    
    
private:
    BaseParameters & model;
    
    op_t ident;
    Matrix mcount, minteraction, mcreate, mdestroy;
    op_t create, destroy, pump, count, interaction;
    op_t lindDestroy, lindDestroy2, leftDestroy, rightCreate;
    Index<TrivialGroup> phys, phys_psi;
    
    std::vector<hamterm_t> terms;
};



#endif
