/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *                            Michele Dolfi <dolfim@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef HAMILTONIAN_H
#define HAMILTONIAN_H

#include "dmrg/block_matrix/block_matrix.h"
#include "dmrg/block_matrix/block_matrix_algorithms.h"
#include "dmrg/block_matrix/symmetry.h"

#include <vector>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include "dmrg/models/lattice.h"

#include "dmrg/models/tag_table.h"
#include "dmrg/models/generate_mpo.hpp"

enum TermType {all_term, site_term, bond_term};

template<class Matrix, class SymmGroup>
struct Hamiltonian_Term : public generate_mpo::Operator_Term<Matrix, SymmGroup>
{
	typedef typename generate_mpo::Operator_Term<Matrix, SymmGroup>::op_t op_t;
};

template<class Matrix, class SymmGroup>
class Hamiltonian
{
    typedef typename OPTagTable<Matrix, SymmGroup>::op_tag_t tag_t;
public:
    typedef Hamiltonian_Term<Matrix, SymmGroup> hamterm_t;
    typedef typename generate_mpo::Operator_Tag_Term<Matrix, SymmGroup> hamtagterm_t;
    typedef typename hamterm_t::op_t op_t;
    typedef typename std::vector<hamterm_t>::const_iterator const_iterator;
    typedef typename std::vector<hamterm_t>::iterator iterator;
    
    Hamiltonian () {}
    Hamiltonian (Index<SymmGroup> const & phys_
                 , op_t ident_
                 , std::vector<hamterm_t> const & terms_
                 , tag_t ident_tag_ = 0
                 , std::vector<hamtagterm_t> const & tagterms_ = std::vector<hamtagterm_t>()
                 , boost::shared_ptr<OPTagTable<Matrix, SymmGroup> > op_tags_ = boost::shared_ptr<OPTagTable<Matrix, SymmGroup> >() )
    : phys(phys_)
    , ident(ident_)
    , ident_tag(ident_tag_)
    , terms(terms_)
    , tagterms(tagterms_)
    , op_tags(op_tags_)
    {}
    
    virtual std::size_t n_terms (TermType what=all_term) const { return terms.size(); }
    virtual hamterm_t const & operator[] (int i) const { return terms[i]; }

    virtual std::size_t n_tagterms (TermType what=all_term) const { return tagterms.size(); }
    virtual hamtagterm_t const & tag(int i) const { return tagterms[i]; }

    virtual void add_term (hamterm_t const & term) { terms.push_back(term); }
    
    virtual Index<SymmGroup> get_phys () const { return phys; }
    virtual void set_phys (Index<SymmGroup> const & phys_) { phys = phys_; }
    virtual op_t const & get_identity () const { return ident; }
    virtual tag_t const & get_identity_tag () const { return ident_tag; }
    virtual void set_identity (op_t const & ident_) { ident = ident_; }
    
    const_iterator begin () const { return terms.begin(); }
    const_iterator end () const { return terms.end(); }
    
    iterator begin () { return terms.begin(); }
    iterator end () { return terms.end(); }

    boost::shared_ptr<OPTagTable<Matrix, SymmGroup> > get_tag_table() const { return op_tags; }

protected:
    std::vector<hamterm_t> terms;
    Index<SymmGroup> phys;
    op_t ident;
    tag_t ident_tag;

    std::vector<hamtagterm_t> tagterms;
    boost::shared_ptr<OPTagTable<Matrix, SymmGroup> > op_tags;
};

template<class Matrix, class SymmGroup>
MPO<Matrix, SymmGroup> make_mpo(typename Lattice::pos_t L, Hamiltonian<Matrix, SymmGroup> const & H, TermType what=all_term)
{
    // Use tags if available
    if (H.n_tagterms(what) > 0) {
        generate_mpo::TaggedMPOMaker<Matrix, SymmGroup> mpom(L, H.get_identity_tag(), H.get_tag_table());
        for (std::size_t i = 0; i < H.n_tagterms(what); ++i)
            mpom.add_term(H.tag(i));

        return mpom.create_mpo();

    } else  {
        generate_mpo::MPOMaker<Matrix, SymmGroup> mpom(L, H.get_identity());
        for (std::size_t i = 0; i < H.n_terms(what); ++i)
            mpom.add_term(H[i]);

        return mpom.create_mpo();
    }

}

template<class Matrix, class SymmGroup>
void make_compressed_mpo(
     MPO<Matrix, SymmGroup> & mpoc,
     double cutoff,
     typename Lattice::pos_t L, Hamiltonian<Matrix, SymmGroup> const & H,
     TermType what=all_term)
{
    generate_mpo::TaggedMPOMaker<Matrix, SymmGroup> mpom(L, H.get_identity_tag(), H.get_tag_table());
    for (std::size_t i = 0; i < H.n_tagterms(what); ++i)
        mpom.add_term(H.tag(i));

    mpoc = mpom.create_compressed_mpo(H.get_phys(), cutoff);
}

template<class Matrix, class SymmGroup>
std::ostream& operator<<(std::ostream& os, Hamiltonian_Term<Matrix, SymmGroup> const & h)
{
    os << " - Fill operator:" << std::endl << h.fill_operator;
    for (int i=0; i<h.operators.size(); ++i)
        os << " - Operator at " << h.operators[i].first << ":" << std::endl << h.operators[i].second;
    return os;
}

template<class Matrix, class SymmGroup>
std::ostream& operator<<(std::ostream& os, Hamiltonian<Matrix, SymmGroup> const & H)
{
    for (int i=0; i<H.n_terms(); ++i)
        os << "* TERM(" << i << ") *" << std::endl << H[i];
    return os;
}

#endif
