/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
 *            See LICENSE.txt for details.
 */

#include "dmrg/models/alps/symm_handler.hpp"
#include "dmrg/block_matrix/symmetry/nu1dg.h"

// Symmetry dependent implementation

// U1DG Symmetry
template <>
U1DG::charge state_to_charge<U1DG>(alps::site_state<short> const & state, alps::SiteBasisDescriptor<short> const& b,
                                     std::map<std::string, int> const& all_conserved_qn)
{
    typedef std::map<std::string, int> qn_map_type;
    U1DG::charge c = U1DG::IdentityCharge;
    for (alps::SiteBasisDescriptor<short>::const_iterator it = b.begin(); it != b.end(); ++it) {
        qn_map_type::const_iterator match = all_conserved_qn.find(it->name());
        if (match != all_conserved_qn.end())
            c[match->second] = detail::to_integer( get_quantumnumber(state, it->name(), b) );
    }
    return c;
}

template <>
U1DG::charge init_charge<U1DG> (const alps::Parameters& parms, std::map<std::string, int> const& all_conserved_qn)
{
    typedef std::map<std::string, int> qn_map_type;
    assert(all_conserved_qn.size() == 2);

    U1DG::charge c = U1DG::IdentityCharge;
    for (qn_map_type::const_iterator it=all_conserved_qn.begin(); it!=all_conserved_qn.end(); ++it) {
        alps::half_integer<short> tmp = alps::evaluate<double>(static_cast<std::string>(parms[it->first+"_total"]), parms);
        c[it->second] = detail::to_integer(tmp);
    }
    
    return c;
}
