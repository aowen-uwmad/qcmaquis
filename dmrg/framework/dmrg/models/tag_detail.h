/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2013-2013 by Sebastian Keller <sebkelle@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef MAQUIS_DMRG_MODELS_TAG_DETAIL_H
#define MAQUIS_DMRG_MODELS_TAG_DETAIL_H

#include <alps/numeric/isnan.hpp>
#include <alps/numeric/isinf.hpp>

namespace tag_detail {

    typedef unsigned tag_type;

    enum operator_kind { bosonic, fermionic };

    struct pair_cmp
    {
        bool operator()(std::pair<tag_type, tag_type> const & i,
                        std::pair<tag_type, tag_type> const & j) const
        {
            if (i.first < j.first)
                return true;
            else if (i.first > j.first)
                return false;
            else
                return i.second < j.second;
        }
    };

    template <class Matrix, class SymmGroup>
    bool is_uniform(block_matrix<Matrix, SymmGroup> & op)
    {
        typename Matrix::value_type invscale;

        // determine scale of matrices
        typename Matrix::const_element_iterator it = op[0].elements().first;
        for ( ; it != op[0].elements().second; ++it)
            if (std::abs(*it) > 1.e-20) {
                invscale = 1./(*it);
                break;
            }

        assert( it != op[0].elements().second );

        for (typename Matrix::size_type b=0; b < op.n_blocks(); ++b)
        {
            typename Matrix::const_element_iterator it = op[b].elements().first;
            for ( ; it != op[b].elements().second; ++it) {
                typename Matrix::value_type normalized = std::abs(*it * invscale);
                // if not 1 and not 0
                if (std::abs(normalized-1.0) > 1e-15 && normalized > 1e-15)
                    return false;
            }
        }

        return true;
    }

    template <class T>
    bool num_check(T x) {
        if (alps::numeric::isnan(x) || alps::numeric::isinf(x))
            throw std::runtime_error("NaN / INF numeric Error occured while comparing operator scales\n");
        return true;
    }

    inline bool num_check(std::complex<double> x) { return true; }

    template <class Matrix, class SymmGroup>
    std::pair<bool, typename Matrix::value_type>
    equal(block_matrix<Matrix, SymmGroup> & reference,
          block_matrix<Matrix, SymmGroup> & sample)
    {
        if (reference.left_basis() != sample.left_basis() || reference.right_basis() != sample.right_basis())
            return std::make_pair(false, 0.);

        typename Matrix::value_type invscale1, invscale2;
     
        // determine scale of matrices
        typename Matrix::const_element_iterator it1 = reference[0].elements().first;
        for ( ; it1 != reference[0].elements().second; ++it1)
            if (std::abs(*it1) > 1.e-50) {
                invscale1 = 1./(*it1);
                break;
            }
        if (it1 == reference[0].elements().second) {
            maquis::cout << "Null-block encountered\n";
            return std::make_pair(false, 0.);
        }

        typename Matrix::const_element_iterator it2 = sample[0].elements().first;
        for ( ; it2 != sample[0].elements().second; ++it2)
            if (std::abs(*it2) > 1.e-50) {
                invscale2 = 1./(*it2);
                break;
            }
        if (it2 == sample[0].elements().second) {
            maquis::cout << "Null-block encountered\n";
            return std::make_pair(false, 0.);
        }

        // Check all blocks for equality modulo scale factor
        for (typename Matrix::size_type b=0; b < reference.n_blocks(); ++b)
        {
            typename Matrix::const_element_iterator it1 = reference[b].elements().first,
                                                    it2 = sample[b].elements().first;
            for ( ; it1 != reference[b].elements().second; ++it1, ++it2)
            {
                typename Matrix::value_type t1 = *it1 * invscale1, t2 = *it2 * invscale2;
                if (std::abs(t1 - t2) > 1e-12)
                    return std::make_pair(false, 0.);
            }
        }

        typename Matrix::value_type scale = invscale1 / invscale2;

#ifndef NDEBUG
        try { num_check(invscale1); }
        catch (std::exception e) { maquis::cout << "invscale1 numcheck failed\n"; exit(1);}
        try { num_check(invscale2); }
        catch (std::exception e) { maquis::cout << "invscale2 numcheck failed\n"; exit(1);}
        try { num_check(scale); }
        catch (std::exception e) { maquis::cout << "scale numcheck failed\n"; exit(1);}
#endif

        return std::make_pair(true, scale);
    }
}

#endif
