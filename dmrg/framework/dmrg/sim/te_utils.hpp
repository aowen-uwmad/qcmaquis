
#ifndef APP_TE_UTILS_H
#define APP_TE_UTILS_H

#include "dmrg/models/hamiltonian.h"

#include "utils/traits.hpp"
#include "dmrg/block_matrix/block_matrix.h"
#include "dmrg/block_matrix/multi_index.h"
#include "dmrg/mp_tensors/reshapes.h"
#include "dmrg/mp_tensors/generic_reshape.h"
#include "dmrg/mp_tensors/mpo_manip.h"
#include "dmrg/mp_tensors/compression.h"

#include "dmrg/models/generate_mpo/utils.hpp"

#include <vector>
#include <set>
#include <algorithm>

// Return: Bond terms are allowed to be in the same Hamiltonian object if they do not overlap
//         Site terms are splitted among all Hamiltonian objects using that site
template <class Matrix, class SymmGroup>
std::vector<Hamiltonian<Matrix, SymmGroup> > separate_overlaps (Hamiltonian<Matrix, SymmGroup> const & H)
{
    typedef std::map<std::size_t, std::set<std::size_t> > pos_where_t;
    typedef typename Hamiltonian<Matrix, SymmGroup>::hamtagterm_t term_t;
    typedef std::map<std::size_t, std::vector<term_t> > pos_terms_t;
    
    std::vector<Hamiltonian<Matrix, SymmGroup> > ret;
    pos_where_t pos_where;
    pos_terms_t pos_terms;
    
    for (int i=0; i<H.n_tagterms(); ++i)
    {            
        if (H.tag(i).operators.size() == 1) {
            pos_terms[H.tag(i).operators[0].first].push_back(H.tag(i));
            continue;
        }
        
        term_t term = H.tag(i);
        term.canonical_order(); // TODO: check and fix for fermions!!
        bool used = false;
        for (int n=0; n<ret.size() && !used; ++n)
        {
            bool overlap = false;
            for (int j=0; j<ret[n].n_tagterms() && !overlap; ++j)
            {
                if ( ret[n].tag(j).site_match(term) ) break;
                overlap = ret[n].tag(j).overlap(term);
            }
            
            if (!overlap) {
            	ret[n].add_tagterm(term);
            	for (int p=0; p<term.operators.size(); ++p)
            		pos_where[term.operators[p].first].insert(n);
                used = true;
            }
        }
        
        if (!used) {
            Hamiltonian<Matrix, SymmGroup> tmp(H.get_phys(), H.get_identity(), std::vector<typename Hamiltonian<Matrix, SymmGroup>::hamterm_t>(),
                                               H.get_identity_tag(), std::vector<term_t>(1, term), H.get_operator_table());
            ret.push_back(tmp);
            
        	for (int p=0; p<term.operators.size(); ++p)
        		pos_where[term.operators[p].first].insert(ret.size()-1);
        }
    }
    
    // Adding site terms to all Hamiltonians acting on site i
    for (typename pos_terms_t::const_iterator it = pos_terms.begin();
         it != pos_terms.end();
         ++it)
    {
        double coeff = 1. / pos_where[it->first].size();
        
        for (typename std::set<std::size_t>::const_iterator it2 = pos_where[it->first].begin();
             it2 != pos_where[it->first].end();
             ++it2)
            for (int k=0; k<it->second.size(); ++k)
            {
            	term_t tmp_term = it->second[k];
            	tmp_term.scale *= coeff;
                ret[*it2].add_tagterm(tmp_term);
            }
    }
    
    // Sorting individual Hamiltonians
    for (int n=0; n<ret.size(); ++n)
        std::sort(ret[n].begin(), ret[n].end());
    
    // Output
    //    for (int n=0; n<ret.size(); ++n)
    //    {
    //        maquis::cout << "Hamiltonian #" << n << std::endl;
    //        maquis::cout << ret[n];
    //    }
    
    return ret;
}

// Precondition: Hamiltonian has to be sorted with bond terms coming before site terms (default behaviour of Operator_Term::operator<())
template <class Matrix, class SymmGroup>                                                                                                // (todo: 30.04.12 / Matthias scalar/value types discussion)
std::map<std::size_t, block_matrix<Matrix, SymmGroup> > make_exp_nn (Hamiltonian<Matrix, SymmGroup> const & H, typename Matrix::value_type const & alpha = 1.) // type of the time step // template
{
    typedef Hamiltonian<Matrix, SymmGroup> ham;
    
    std::map<std::size_t, block_matrix<Matrix, SymmGroup> > map_exp;
    
    for (int n=0; n<H.n_terms(); )
    {
        assert(H[n].operators.size() == 2);
        int pos1 = H[n].operators[0].first;
        int pos2 = H[n].operators[1].first;
        assert(std::abs(pos1-pos2) == 1);
        
        typename ham::op_t bond_op;
        op_kron(H.get_phys(), H[n].operators[0].second, H[n].operators[1].second, bond_op);
        
        int k = n+1;
        for (; k<H.n_terms(); ++k)
        {
            Hamiltonian_Term<Matrix, SymmGroup> term = H[k];
            if (! H[n].site_match(term))
                break;
            typename ham::op_t tmp;
            if (term.operators.size() == 2)
                op_kron(H.get_phys(), term.operators[0].second, term.operators[1].second, tmp);
            else if (term.operators[0].first == pos1)
                op_kron(H.get_phys(), term.operators[0].second, H.get_identity(), tmp);
            else if (term.operators[0].first == pos2)
                op_kron(H.get_phys(), H.get_identity(), term.operators[0].second, tmp);
            else
                throw std::runtime_error("Operator k not matching any valid position.");
            bond_op += tmp;
        }
        
        bond_op = op_exp(H.get_phys()*H.get_phys(), bond_op, alpha);
        
        map_exp[pos1] = bond_op;
        
        n = k;
    }
            
    return map_exp;
}


template <class Matrix, class SymmGroup>
block_matrix<Matrix, SymmGroup> term2block(typename Hamiltonian<Matrix, SymmGroup>::hamtagterm_t const & term, std::size_t pos1,
                                           Index<SymmGroup> const & phys_i, block_matrix<Matrix, SymmGroup> const & ident,
                                           typename Hamiltonian<Matrix, SymmGroup>::table_ptr tag_handler)
{
#ifndef NDEBUG
    if (term.operators.size() == 2)
        assert(std::abs( term.operators[0].first-term.operators[1].first ) == 1);
#endif
    
    block_matrix<Matrix, SymmGroup> bond_op;
    if (term.operators.size() == 2)
        op_kron(phys_i, tag_handler->get_op(term.operators[0].second), tag_handler->get_op(term.operators[1].second), bond_op);
    else if (term.operators[0].first == pos1)
        op_kron(phys_i, tag_handler->get_op(term.operators[0].second), ident, bond_op);
    else
        op_kron(phys_i, ident, tag_handler->get_op(term.operators[0].second), bond_op);
//    else
//        throw std::runtime_error("Operator k not matching any valid position.");
    
    bond_op *= term.scale;
    
    return bond_op;
}

template <class Matrix, class SymmGroup>
std::vector<block_matrix<Matrix, SymmGroup> > hamil_to_blocks(Hamiltonian<Matrix, SymmGroup> const & H, std::size_t L)
{
    std::vector<block_matrix<Matrix, SymmGroup> > ret_blocks(L-1);
    
    for (int i=0; i<H.n_tagterms(); ++i)
    {
        typename Hamiltonian<Matrix, SymmGroup>::hamtagterm_t term = H.tag(i);
        term.canonical_order(); // TODO: check and fix for fermions!!
        std::size_t pos1 = term.operators[0].first;
        if (term.operators.size() == 1) {
            if (pos1 != 0 && pos1 != L-1)
                term.scale /= 2.;
            if (pos1 < L-1)
                ret_blocks[pos1] += term2block(term, pos1, H.get_phys(), H.get_identity(), H.get_operator_table());
            if (pos1 > 0)
                ret_blocks[pos1-1] += term2block(term, pos1-1, H.get_phys(), H.get_identity(), H.get_operator_table());
        } else if (term.operators.size() == 2) {
            ret_blocks[pos1] += term2block(term, pos1, H.get_phys(), H.get_identity(), H.get_operator_table());
        }
    }
    
    return ret_blocks;
}

template <class Matrix, class SymmGroup>
class exp_mpo_maker {
    typedef block_matrix<Matrix, SymmGroup> op_t;
    
    typedef typename Hamiltonian<Matrix, SymmGroup>::hamtagterm_t hamtagterm_t;
    typedef OPTable<Matrix, SymmGroup> op_table_t;
    typedef boost::shared_ptr<op_table_t> op_table_ptr;
    typedef typename op_table_t::tag_type tag_type;
    
    typedef boost::tuple<std::size_t, std::size_t, tag_type, typename Matrix::value_type> pretensor_value;
    typedef std::vector<pretensor_value> pretensor_t;
    typedef std::vector<pretensor_t> prempo_t;
    
public:
    exp_mpo_maker(Index<SymmGroup> const& phys_, op_t const& ident_,
                 std::size_t pos1_, std::size_t pos2_)
    : ident_op(ident_)
    , fill_op(ident_)
    , phys(phys_)
    , pos1(pos1_), pos2(pos2_), L(pos2-pos1+1)
    , n_boso(0), n_ferm(0)
    { }
    
    void add_term(hamtagterm_t const & term, op_table_ptr op_table)
    {
        if (term.operators.size() > 2)
            throw std::runtime_error("time evolution requires at max bond term.");
        
        /// kron product of operators
        op_t bond_op;
        if (term.operators.size() == 2)
            op_kron(phys, (*op_table)[term.operators[0].second], (*op_table)[term.operators[1].second], bond_op);
        else if (term.operators[0].first == pos1)
            op_kron(phys, (*op_table)[term.operators[0].second], ident_op, bond_op);
        else if (term.operators[0].first == pos2)
            op_kron(phys, ident_op, (*op_table)[term.operators[0].second], bond_op);
        else
            throw std::runtime_error("Operator k not matching any valid position.");
        
        if (term.with_sign) {
            fermionic_bond += term.scale * bond_op;
            n_ferm += 1;
            fill_op = (*op_table)[term.fill_operator];
        } else {
            bosonic_bond += term.scale * bond_op;
            n_boso += 1;
        }
    }
    
    MPO<Matrix, SymmGroup> exp_mpo(typename Matrix::value_type const & alpha, op_table_ptr op_table) const
    {
        std::size_t maximum_b = 0;
        prempo_t prempo(L);
        
        tag_type ident = op_table->register_op(ident_op);
        tag_type fill;
        {
            typename Matrix::value_type coeff;
            boost::tie(fill, coeff) = op_table->checked_register(fill_op);
            if (coeff != 1.) throw std::runtime_error("multiple of fill operator already in op_table.");
        }
        
        if (n_boso > 0) {
            std::vector<op_t> left_ops, right_ops;
            exp_and_split(bosonic_bond, alpha, left_ops, right_ops);
            
            maximum_b = add_to_mpo(prempo, maximum_b, left_ops, right_ops, 1., ident, op_table);
        }
        
        if (n_ferm > 0) {
            /// exp(alpha * op1 \otimes op2)
            {
                std::vector<op_t> left_ops, right_ops;
                exp_and_split(fermionic_bond, alpha, left_ops, right_ops);
                
                maximum_b = add_to_mpo(prempo, maximum_b, left_ops, right_ops, .5, ident, op_table);
                maximum_b = add_to_mpo(prempo, maximum_b, left_ops, right_ops, .5, fill, op_table);
            }
            
            /// exp(-alpha * op1 \otimes op2)
            {
                std::vector<op_t> left_ops, right_ops;
                exp_and_split(fermionic_bond, -alpha, left_ops, right_ops);
                
                maximum_b = add_to_mpo(prempo, maximum_b, left_ops, right_ops, .5, ident, op_table);
                maximum_b = add_to_mpo(prempo, maximum_b, left_ops, right_ops, -.5, fill, op_table);
            }
        }
        
        
        MPO<Matrix, SymmGroup> mpo(L);
        for (size_t p=0; p<L; ++p) {
            size_t nrows, ncols;
            using generate_mpo::rcdim;
            boost::tie(nrows, ncols) = rcdim(prempo[p]);
            MPOTensor<Matrix, SymmGroup> tmp(nrows, ncols, prempo[p], op_table);
            std::swap(mpo[p], tmp);
        }
        
        return mpo;
    }
    
private:
    
    void exp_and_split(op_t const& bond_op, typename Matrix::value_type const & alpha,
                       std::vector<op_t> & left_ops, std::vector<op_t> &  right_ops) const
    {
        op_t bond_exp;
        bond_exp = op_exp_hermitian(phys*phys, bond_op, alpha);
        bond_exp = reshape_2site_op(phys, bond_exp);
        typedef typename alps::numeric::associated_real_diagonal_matrix<Matrix>::type diag_matrix;
        block_matrix<Matrix, SymmGroup> U, V, left, right;
        block_matrix<diag_matrix, SymmGroup> S, Ssqrt;
        svd(bond_exp, U, V, S);
        Ssqrt = sqrt(S);
        gemm(U, Ssqrt, left);
        gemm(Ssqrt, V, right);
        
#ifdef AMBIENT
        locale_shared i;
        for(std::size_t k = 0; k < S.n_blocks(); ++k){
            ambient::numeric::merge(S[k]);
            ambient::numeric::touch(S[k][0]);
        }
        ambient::sync();
#endif
        for(std::size_t k = 0; k < S.n_blocks(); ++k){
            int keep = std::find_if(S[k].diagonal().first, S[k].diagonal().second, boost::lambda::_1 < 1e-10)-S[k].diagonal().first;
            
            left.resize_block(left.left_basis()[k].first, left.right_basis()[k].first,
                              left.left_basis()[k].second, keep);
            right.resize_block(right.left_basis()[k].first, right.right_basis()[k].first,
                               keep, right.right_basis()[k].second);
        }
        
        left_ops  = reshape_right_to_list(phys, left);
        right_ops = reshape_left_to_list(phys, right);
        assert(left_ops.size() == right_ops.size());
    }
    
    std::size_t add_to_mpo(prempo_t & prempo, std::size_t maximum_b,
                           std::vector<op_t> const& left_ops, std::vector<op_t> const& right_ops,
                           typename Matrix::value_type s, tag_type fill,  op_table_ptr op_table) const
    {
        for (std::size_t i=0; i<left_ops.size(); ++i)
        {
            std::size_t b = (maximum_b++);
            
            std::pair<tag_type, typename Matrix::value_type> left_tag, right_tag;
            left_tag = op_table->checked_register(left_ops[i]);
            right_tag = op_table->checked_register(right_ops[i]);
            
            s *= left_tag.second * right_tag.second;
            
            prempo[0].push_back  ( pretensor_value(0, b, left_tag.first,  s ) );
            prempo[L-1].push_back( pretensor_value(b, 0, right_tag.first, 1.) );
            for (std::size_t p=1; p<L-1; ++p)
                prempo[p].push_back( pretensor_value(b, b, fill, 1.) );
        }
        return maximum_b;
    }
    
    op_t ident_op, fill_op;
    Index<SymmGroup> phys;
    std::size_t pos1, pos2, L;
    std::size_t n_boso, n_ferm;
    op_t bosonic_bond, fermionic_bond;
};

// Precondition: Hamiltonian has to be sorted with bond terms coming before site terms (default behaviour of Operator_Term::operator<())
template <class Matrix, class SymmGroup>
MPO<Matrix, SymmGroup> make_exp_mpo(std::size_t length,
                                    Hamiltonian<Matrix, SymmGroup> const & H,
                                    typename Matrix::value_type const & alpha = 1)
{
    typedef Hamiltonian<Matrix, SymmGroup> ham;
    typedef OPTable<Matrix, SymmGroup> op_table_t;
    typedef boost::shared_ptr<op_table_t> op_table_ptr;
    typedef typename op_table_t::tag_type tag_type;
    typedef boost::tuple<std::size_t, std::size_t, tag_type, typename Matrix::value_type> pretensor_value;
    typedef std::vector<pretensor_value> pretensor_t;

    op_table_ptr op_table( new op_table_t() );
    
    MPO<Matrix, SymmGroup> mpo(length);
    std::vector<bool> used_p(length, false);
    
    for (int n=0; n<H.n_tagterms(); )
    {
        assert(H.tag(n).operators.size() == 2);
        int pos1 = H.tag(n).operators[0].first;
        int pos2 = H.tag(n).operators[1].first;
        
        exp_mpo_maker<Matrix, SymmGroup> maker(H.get_phys(), H.get_identity(), pos1, pos2);
        
        maker.add_term(H.tag(n), H.get_operator_table()->get_operator_table());
        
        int k = n+1;
        for (; k<H.n_tagterms() && H.tag(n).site_match(H.tag(k)); ++k)
            maker.add_term(H.tag(k), H.get_operator_table()->get_operator_table());
        
        MPO<Matrix, SymmGroup> block_mpo = maker.exp_mpo(alpha, op_table);
        for (size_t p=0; p<pos2-pos1+1; ++p) {
            std::swap(mpo[pos1+p], block_mpo[p]);
            used_p[pos1+p] = true;
        }
        
        n = k;
    }
    
    tag_type ident; typename Matrix::value_type scale;
    boost::tie(ident, scale) = op_table->checked_register(H.get_identity());
    pretensor_t preident;
    preident.push_back( pretensor_value(0, 0, ident, scale) );
    
    // Filling missing identities
    for (std::size_t p=0; p<length; ++p) {
        if (!used_p[p]) {
            MPOTensor<Matrix, SymmGroup> r(1, 1, preident, op_table);
            std::swap(mpo[p], r);
            used_p[p] = true;
        }
    }
    
    return mpo;
}

#endif
