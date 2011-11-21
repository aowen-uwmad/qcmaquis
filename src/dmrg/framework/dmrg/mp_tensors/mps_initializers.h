/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *                            Michele Dolfi <dolfim@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef MPS_INITIALIZER_H
#define MPS_INITIALIZER_H

#include "compression.h"

template<class T>
T tri_min(T a, T b, T c)
{
    return std::min(std::min(a, b),
                    std::min(a, c));
}

template<class Matrix, class SymmGroup>
struct default_mps_init : public mps_initializer<Matrix, SymmGroup>
{
    
    void operator()(MPS<Matrix, SymmGroup> & mps,
                    std::size_t Mmax,
                    Index<SymmGroup> const & phys,
                    typename SymmGroup::charge right_end)
    {
        return init_sectors(mps, Mmax, phys, right_end, true);
    }
    
    void init_sectors(MPS<Matrix, SymmGroup> & mps,
                      std::size_t Mmax,
                      Index<SymmGroup> const & phys,
                      typename SymmGroup::charge right_end,
                      bool fillrand = true,
                      typename Matrix::value_type val = 0)
    {
        std::size_t L = mps.length();
        
        Index<SymmGroup> l_triv, r_triv;
        l_triv.insert( std::make_pair(SymmGroup::IdentityCharge, 1) );
        r_triv.insert( std::make_pair(right_end, 1) );
        
        std::vector<Index<SymmGroup> > left_allowed(L+1), right_allowed(L+1), allowed(L+1);
        left_allowed[0] = l_triv;
        right_allowed[L] = r_triv;
        
        for (int i = 1; i < L+1; ++i) {
            left_allowed[i] = phys * left_allowed[i-1];
            for (typename Index<SymmGroup>::iterator it = left_allowed[i].begin();
                 it != left_allowed[i].end(); ++it)
                it->second = std::min(Mmax, it->second);
        }
        for (int i = L-1; i >= 0; --i) {
            right_allowed[i] = adjoin(phys) * right_allowed[i+1];
            for (typename Index<SymmGroup>::iterator it = right_allowed[i].begin();
                 it != right_allowed[i].end(); ++it)
                it->second = std::min(Mmax, it->second);
        }
        
        for (int i = 0; i < L+1; ++i) {
            allowed[i] = common_subset(left_allowed[i], right_allowed[i]);
            for (typename Index<SymmGroup>::iterator it = allowed[i].begin();
                 it != allowed[i].end(); ++it)
                it->second = tri_min(Mmax,
                                     left_allowed[i].size_of_block(it->first),
                                     right_allowed[i].size_of_block(it->first));
        }
        
        for (int i = 0; i < L; ++i)
            mps[i] = MPSTensor<Matrix, SymmGroup>(phys, allowed[i], allowed[i+1], fillrand, val);
        
        zout << mps.description() << endl;
    }
};

template<class Matrix, class SymmGroup>
struct const_mps_init : public mps_initializer<Matrix, SymmGroup>
{
    void operator()(MPS<Matrix, SymmGroup> & mps,
                    std::size_t Mmax,
                    Index<SymmGroup> const & phys,
                    typename SymmGroup::charge right_end)
    {
        default_mps_init<Matrix, SymmGroup> di;
        di.init_sectors(mps, Mmax, phys, right_end, false, 1);
    }
};

template<class Matrix, class SymmGroup>
struct thin_mps_init : public mps_initializer<Matrix, SymmGroup>
{
    void operator()(MPS<Matrix, SymmGroup> & mps,
                    std::size_t Mmax,
                    Index<SymmGroup> const & phys,
                    typename SymmGroup::charge right_end)
    {
        default_mps_init<Matrix, SymmGroup> di;
        di(mps, 5, phys, right_end);
        mps = compression::l2r_compress(mps, Mmax, 1e-6); 
    }
};

template<class Matrix, class SymmGroup>
struct thin_const_mps_init : public mps_initializer<Matrix, SymmGroup>
{
    void operator()(MPS<Matrix, SymmGroup> & mps,
                    std::size_t Mmax,
                    Index<SymmGroup> const & phys,
                    typename SymmGroup::charge right_end)
    {
        const_mps_init<Matrix, SymmGroup> di;
        di(mps, 5, phys, right_end);
        mps = compression::l2r_compress(mps, Mmax, 1e-6); 
    }
};


template<class Matrix, class SymmGroup>
struct empty_mps_init : public mps_initializer<Matrix, SymmGroup>
{
    void operator()(MPS<Matrix, SymmGroup> & mps,
                    std::size_t Mmax,
                    Index<SymmGroup> const & phys,
                    typename SymmGroup::charge right_end)
    {
        std::size_t L = mps.length();
        
        Index<SymmGroup> triv;
        triv.insert( std::make_pair(SymmGroup::IdentityCharge, 1) );
        
        for (int i = 0; i < L; ++i)
            mps[i] = MPSTensor<Matrix, SymmGroup>(phys, triv, triv);        
    }
};


#endif
