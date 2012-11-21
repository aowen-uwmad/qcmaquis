/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2012 by Michele Dolfi <dolfim@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef MAQUIS_DMRG_DMRG_INIT_SIM_H
#define MAQUIS_DMRG_DMRG_INIT_SIM_H

#include "dmrg_version.h"

#include "dmrg/mp_tensors/mps.h"
#include "dmrg/mp_tensors/compression.h"
#include "dmrg/models/factory.h"


template <class Matrix, class SymmGroup>
class dmrg_init {
public:
    typedef typename SymmGroup::charge charge;
    typedef std::pair<charge, size_t> local_state;
    
    dmrg_init(DmrgParameters const & parms_, ModelParameters const & model_)
    : parms(parms_)
    , model(model_)
    , chkpfile(parms.get<std::string>("chkpfile"))
    , num_states1(0)
    , num_states2(0)
    {
        maquis::cout << DMRG_VERSION_STRING << std::endl;
        
        // TODO: insert boost::chrono timers
        
        model_parser<Matrix, SymmGroup>(parms.get<std::string>("lattice_library"), parms.get<std::string>("model_library"),
                                        model, lat, phys_model);
        initc = phys_model->initc(model);
        phys = phys_model->H().get_phys();
        L = lat->size();
    }
    
    void build()
    {
        build_slow();
        
        // final join of mps1 and mps2
        if (num_states2 > 0) {
            mps1 = join(mps1, mps2);
            mps1 = compression::l2r_compress(mps1, parms.get<size_t>("init_bond_dimension"), parms.get<size_t>("truncation_initial"));
            mps2 = MPS<Matrix, SymmGroup>();
            num_states2 = 0;
        }
        
        // write parameters and mps
        alps::hdf5::archive h5ar(rfile, alps::hdf5::archive::WRITE | alps::hdf5::archive::REPLACE);
        h5ar << alps::make_pvp("/parameters", parms);
        h5ar << alps::make_pvp("/parameters", model);
        h5ar << alps::make_pvp("/version", DMRG_VERSION_STRING);
        h5ar << alps::make_pvp("/state", mps1);
    }
    
private:
    void build_slow()
    {
        // some functions are missing for complex charge objects
#ifndef HAVE_TwoU1
        typedef typename std::vector<local_state>::const_iterator states_iterator;
        
        std::vector<local_state> alllocal;
        for (size_t i=0; i<phys.size(); ++i)
            for (size_t j=0; j<phys[i].second; ++j)
                alllocal.push_back( local_state(phys[i].first, j) );
        
        std::vector<states_iterator> it(L);
        for (size_t i=0; i<L; ++i)
            it[i] = alllocal.begin();
        
        std::vector<local_state> basis;
        while (it[0] != alllocal.end()) {
            std::vector<local_state> state(L);
            for (size_t i=0; i<L; ++i)
                state[i] = *(it[i]);
            charge N = std::accumulate(state.begin(), state.end(), SymmGroup::IdentityCharge,
                                       boost::lambda::_1 + boost::lambda::bind(index_detail::get_first<SymmGroup>, boost::lambda::_2));
            if (N == initc)
                add_state(state);
            
            ++it[L-1];
            for (int i = L-1; (i > 0) && (it[i] == alllocal.end()); --i) {
                it[i] = alllocal.begin();
                ++it[i-1];
            }
        }
#endif
    }
    
    MPS<Matrix, SymmGroup> state_mps(std::vector<local_state> const & state)
    {
        MPS<Matrix, SymmGroup> mps(state.size());
        
        Index<SymmGroup> curr_i;
        curr_i.insert(std::make_pair(0, 1));
        size_t curr_b = 0;
        for (int i=0; i<state.size(); ++i)
        {
            charge newc = SymmGroup::fuse(curr_i[0].first, state[i].first);
            size_t news = (i == state.size()-1) ? 1 : phys[phys.position(state[i].first)].second;
            Index<SymmGroup> new_i;
            new_i.insert(std::make_pair(newc, news));
            ProductBasis<SymmGroup> left(phys, curr_i);
            mps[i] = MPSTensor<Matrix, SymmGroup>(phys, curr_i, new_i, false, 0);
            size_t b_in = left(state[i].first, curr_i[0].first) + state[i].second * curr_i[0].second + curr_b;
            size_t b_out = (i == state.size()-1) ? 0 : state[i].second;
            mps[i].make_left_paired();
            mps[i].data()(SymmGroup::fuse(curr_i[0].first, state[i].first), new_i[0].first)(b_in, b_out) = 1.;
            curr_i = new_i;
            curr_b = state[i].second;
        }
        mps.normalize_left();
        return mps;
    }
    
    void add_state(std::vector<local_state> const & state)
    {
        MPS<Matrix, SymmGroup> & curr = (num_states1 < parms.get<size_t>("init_bond_dimension")) ? mps1 : mps2;
        size_t & num_states = (num_states1 < parms.get<size_t>("init_bond_dimension")) ? num_states1 : num_states2;
        
        MPS<Matrix, SymmGroup> temp = state_mps(state);
        if (curr.length() > 1)
            curr = join(curr, temp);
        else
            swap(curr, temp);
        num_states += 1;

        if (num_states2 > parms.get<size_t>("init_bond_dimension")) {
            mps1 = join(mps1, mps2);
            mps1 = compression::l2r_compress(mps1, parms.get<size_t>("init_bond_dimension"), parms.get<size_t>("truncation_initial"));
            mps2 = MPS<Matrix, SymmGroup>();
            num_states2 = 0;
        }
    }
    
    
private:
    DmrgParameters parms;
    ModelParameters model;
    
    std::string chkpfile;
    std::string rfile;
    
    Lattice_ptr lat;
    typename model_traits<Matrix, SymmGroup>::model_ptr phys_model;
    Index<SymmGroup> phys;
    charge initc;
    size_t L;
    MPS<Matrix, SymmGroup> mps1, mps2;
    size_t num_states1, num_states2;
};



#endif
