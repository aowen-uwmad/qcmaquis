/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2015 Institute for Theoretical Physics, ETH Zurich
 *               2012-2013 by Sebastian Keller <sebkelle@phys.ethz.ch>
 *
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

#ifndef QC_HAMILTONIANS_H
#define QC_HAMILTONIANS_H

#include <cmath>
#include <sstream>
#include <fstream>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "dmrg/models/model.h"
#include "dmrg/models/measurements.h"
#include "dmrg/utils/BaseParameters.h"

#include "dmrg/models/chem/util.h"
#include "dmrg/models/chem/parse_integrals.h"
#include "dmrg/models/chem/pg_util.h"
#include "dmrg/models/chem/2u1/term_maker.h"
#include "dmrg/models/chem/2u1/chem_helper.h"

template<class Matrix, class SymmGroup>
class qc_model : public model_impl<Matrix, SymmGroup>
{
    typedef model_impl<Matrix, SymmGroup> base;
    
    typedef typename base::table_type table_type;
    typedef typename base::table_ptr table_ptr;
    typedef typename base::tag_type tag_type;
    
    typedef typename base::term_descriptor term_descriptor;
    typedef typename base::terms_type terms_type;
    typedef typename base::op_t op_t;
    typedef typename base::measurements_type measurements_type;

    typedef typename Lattice::pos_t pos_t;
    typedef typename Matrix::value_type value_type;
    typedef typename alps::numeric::associated_one_matrix<Matrix>::type one_matrix;

public:
    
    qc_model(Lattice const & lat_, BaseParameters & parms_);
    
    void update(BaseParameters const& p)
    {
        // TODO: update this->terms_ with the new parameters
        throw std::runtime_error("update() not yet implemented for this model.");
        return;
    }
    
    // For this model: site_type == point group irrep
    Index<SymmGroup> const & phys_dim(size_t type) const
    {
        return phys_indices[type];
    }
    tag_type identity_matrix_tag(size_t type) const
    {
        return ident[type];
    }
    tag_type filling_matrix_tag(size_t type) const
    {
        return fill[type];
    }

    typename SymmGroup::charge total_quantum_numbers(BaseParameters & parms_) const
    {
        return chem_detail::qn_helper<SymmGroup>().total_qn(parms_);
    }

    tag_type get_operator_tag(std::string const & name, size_t type) const
    {
        if (name == "create_up")
            return create_up[type];
        else if (name == "create_down")
            return create_down[type];
        else if (name == "destroy_up")
            return destroy_up[type];
        else if (name == "destroy_down")
            return destroy_down[type];
        else if (name == "count_up")
            return count_up[type];
        else if (name == "count_down")
            return count_down[type];
        else if (name == "e2d")
            return e2d[type];
        else if (name == "d2e")
            return d2e[type];
        else if (name == "docc")
            return docc[type];
        else
            throw std::runtime_error("Operator not valid for this model.");
        return 0;
    }

    table_ptr operators_table() const
    {
        return tag_handler;
    }
    
    measurements_type measurements () const
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

        op_t create_up_op, create_down_op, destroy_up_op, destroy_down_op,
             count_up_op, count_down_op, docc_op, e2d_op, d2e_op,
             swap_d2u_op, swap_u2d_op,
             create_up_count_down_op, create_down_count_up_op, destroy_up_count_down_op, destroy_down_count_up_op,
             ident_op, fill_op;

        std::vector<op_t> ident_ops = tag_handler->get_ops(ident);
        std::vector<op_t> fill_ops = tag_handler->get_ops(fill);
        std::vector<op_t> create_up_ops = tag_handler->get_ops(create_up);
        std::vector<op_t> create_down_ops = tag_handler->get_ops(create_down);
        std::vector<op_t> destroy_up_ops = tag_handler->get_ops(destroy_up);
        std::vector<op_t> destroy_down_ops = tag_handler->get_ops(destroy_down);
        std::vector<op_t> count_up_ops = tag_handler->get_ops(count_up);
        std::vector<op_t> count_down_ops = tag_handler->get_ops(count_down);
        std::vector<op_t> e2d_ops = tag_handler->get_ops(e2d);
        std::vector<op_t> d2e_ops = tag_handler->get_ops(d2e);
        std::vector<op_t> docc_ops = tag_handler->get_ops(docc);

        gemm(destroy_down_op, create_up_op, swap_d2u_op); // S_plus
        gemm(destroy_up_op, create_down_op, swap_u2d_op); // S_minus
        gemm(count_down_op, create_up_op, create_up_count_down_op);
        gemm(count_up_op, create_down_op, create_down_count_up_op);
        gemm(count_down_op, destroy_up_op, destroy_up_count_down_op);
        gemm(count_up_op, destroy_down_op, destroy_down_count_up_op);

        #define GENERATE_SITE_SPECIFIC(opname) std::vector<op_t> opname ## s = this->generate_site_specific_ops(opname);

        GENERATE_SITE_SPECIFIC(swap_d2u_op)
        GENERATE_SITE_SPECIFIC(swap_u2d_op)
        GENERATE_SITE_SPECIFIC(create_up_count_down_op)
        GENERATE_SITE_SPECIFIC(create_down_count_up_op)
        GENERATE_SITE_SPECIFIC(destroy_up_count_down_op)
        GENERATE_SITE_SPECIFIC(destroy_down_count_up_op)

        #undef GENERATE_SITE_SPECIFIC

        measurements_type meas;

        typedef std::vector<op_t> op_vec;
        typedef std::vector<std::pair<op_vec, bool> > bond_element;
        {
            boost::regex expression("^MEASURE_LOCAL\\[(.*)]$");
            boost::smatch what;
            for (alps::Parameters::const_iterator it=parms.begin();it != parms.end();++it) {
                std::string lhs = it->key();
                if (boost::regex_match(lhs, what, expression)) {

                    op_vec meas_op;
                    if (it->value() == "Nup")
                        meas_op = count_up_ops;
                    else if (it->value() == "Ndown")
                        meas_op = count_down_ops;
                    else if (it->value() == "Nup*Ndown" || it->value() == "docc")
                        meas_op = docc_ops;
                    else
                        throw std::runtime_error("Invalid observable\nLocal measurements supported so far are \"Nup\" and \"Ndown\"\n");

                    meas.push_back( new measurements::local<Matrix, SymmGroup>(what.str(1), lat, ident_ops, fill_ops, meas_op) );
                }
            }
        }

        {
        boost::regex expression("^MEASURE_CORRELATIONS\\[(.*)]$");
        boost::regex expression_half("^MEASURE_HALF_CORRELATIONS\\[(.*)]$");
        boost::regex expression_nn("^MEASURE_NN_CORRELATIONS\\[(.*)]$");
        boost::regex expression_halfnn("^MEASURE_HALF_NN_CORRELATIONS\\[(.*)]$");
        boost::regex expression_twoptdm("^MEASURE_TWOPTDM(.*)$");
        boost::regex expression_transition_twoptdm("^MEASURE_TRANSITION_TWOPTDM(.*)$");
        boost::smatch what;
        for (alps::Parameters::const_iterator it=parms.begin();it != parms.end();++it) {
            std::string lhs = it->key();

            std::string name, value;
            bool half_only, nearest_neighbors_only;
            if (boost::regex_match(lhs, what, expression)) {
                value = it->value();
                name = what.str(1);
                half_only = false;
                nearest_neighbors_only = false;
            }
            if (boost::regex_match(lhs, what, expression_half)) {
                value = it->value();
                name = what.str(1);
                half_only = true;
                nearest_neighbors_only = false;
            }
            if (boost::regex_match(lhs, what, expression_nn)) {
                value = it->value();
                name = what.str(1);
                half_only = false;
                nearest_neighbors_only = true;
            }
            if (boost::regex_match(lhs, what, expression_halfnn)) {
                value = it->value();
                name = what.str(1);
                half_only = true;
                nearest_neighbors_only = true;
            }
            if (boost::regex_match(lhs, what, expression_twoptdm) ||
                    boost::regex_match(lhs, what, expression_transition_twoptdm)) {

                std::string bra_ckp("");
                if(lhs == "MEASURE_TRANSITION_TWOPTDM"){
                    name = "transition_twoptdm";
                    bra_ckp = it->value();
                }
                else
                    name = "twoptdm";

                std::vector<bond_element> synchronous_meas_operators;
                {
                bond_element meas_operators;
                meas_operators.push_back( std::make_pair(create_up_ops, true) );
                meas_operators.push_back( std::make_pair(create_up_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_up_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_up_ops, true) );
                synchronous_meas_operators.push_back(meas_operators);
                }
                {
                bond_element meas_operators;
                meas_operators.push_back( std::make_pair(create_up_ops, true) );
                meas_operators.push_back( std::make_pair(create_down_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_down_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_up_ops, true) );
                synchronous_meas_operators.push_back(meas_operators);
                }
                {
                bond_element meas_operators;
                meas_operators.push_back( std::make_pair(create_down_ops, true) );
                meas_operators.push_back( std::make_pair(create_up_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_up_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_down_ops, true) );
                synchronous_meas_operators.push_back(meas_operators);
                }
                {
                bond_element meas_operators;
                meas_operators.push_back( std::make_pair(create_down_ops, true) );
                meas_operators.push_back( std::make_pair(create_down_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_down_ops, true) );
                meas_operators.push_back( std::make_pair(destroy_down_ops, true) );
                synchronous_meas_operators.push_back(meas_operators);
                }
                half_only = true;
                nearest_neighbors_only = false;
                std::vector<pos_t> positions;
                meas.push_back( new measurements::NRankRDM<Matrix, SymmGroup>(name, lat, ident_ops, fill_ops, synchronous_meas_operators,
                                                                              half_only, nearest_neighbors_only, positions, bra_ckp));
            }
            else if (!name.empty()) {

                int f_ops = 0;
                bond_element meas_operators;
                
                /// split op1:op2:...@p1,p2,p3,... into {op1:op2:...}, {p1,p2,p3,...}
                std::vector<std::string> value_split;
                boost::split( value_split, value, boost::is_any_of("@"));

                /// parse operators op1:op2:...
                boost::char_separator<char> sep(":");
                tokenizer corr_tokens(value_split[0], sep);
                for (tokenizer::iterator it2=corr_tokens.begin();
                     it2 != corr_tokens.end();
                     it2++)
                {
                    if (*it2 == "c_up") {
                        meas_operators.push_back( std::make_pair(destroy_up_ops, true) );
                        ++f_ops;
                    }
                    else if (*it2 == "c_down") {
                        meas_operators.push_back( std::make_pair(destroy_down_ops, true) );
                        ++f_ops;
                    }
                    else if (*it2 == "cdag_up") {
                        meas_operators.push_back( std::make_pair(create_up_ops, true) );
                        ++f_ops;
                    }
                    else if (*it2 == "cdag_down") {
                        meas_operators.push_back( std::make_pair(create_down_ops, true) );
                        ++f_ops;
                    }

                    else if (*it2 == "id" || *it2 == "Id") {
                        meas_operators.push_back( std::make_pair(ident_ops, false) );
                    }
                    else if (*it2 == "Nup") {
                        meas_operators.push_back( std::make_pair(count_up_ops, false) );
                    }
                    else if (*it2 == "Ndown") {
                        meas_operators.push_back( std::make_pair(count_down_ops, false) );
                    }
                    else if (*it2 == "docc" || *it2 == "Nup*Ndown") {
                        meas_operators.push_back( std::make_pair(docc_ops, false) );
                    }
                    else if (*it2 == "cdag_up*c_down" || *it2 == "splus") {
                        meas_operators.push_back( std::make_pair(swap_d2u_ops, false) );
                    }
                    else if (*it2 == "cdag_down*c_up" || *it2 == "sminus") {
                        meas_operators.push_back( std::make_pair(swap_u2d_ops, false) );
                    }

                    else if (*it2 == "cdag_up*cdag_down") {
                        meas_operators.push_back( std::make_pair(e2d_ops, false) );
                    }
                    else if (*it2 == "c_up*c_down") {
                        meas_operators.push_back( std::make_pair(d2e_ops, false) );
                    }

                    else if (*it2 == "cdag_up*Ndown") {
                        meas_operators.push_back( std::make_pair(create_up_count_down_ops, true) );
                        ++f_ops;
                    }
                    else if (*it2 == "cdag_down*Nup") {
                        meas_operators.push_back( std::make_pair(create_down_count_up_ops, true) );
                        ++f_ops;
                    }
                    else if (*it2 == "c_up*Ndown") {
                        meas_operators.push_back( std::make_pair(destroy_up_count_down_ops, true) );
                        ++f_ops;
                    }
                    else if (*it2 == "c_down*Nup") {
                        meas_operators.push_back( std::make_pair(destroy_down_count_up_ops, true) );
                        ++f_ops;
                    }
                    else
                        throw std::runtime_error("Unrecognized operator in correlation measurement: " 
                                                    + boost::lexical_cast<std::string>(*it2) + "\n");
                }

                if (f_ops % 2 != 0)
                    throw std::runtime_error("In " + name + ": Number of fermionic operators has to be even in correlation measurements.");

                /// parse positions p1,p2,p3,... (or `space`)
                std::vector<pos_t> positions;
                if (value_split.size() > 1) {
                    boost::char_separator<char> pos_sep(", ");
                    tokenizer pos_tokens(value_split[1], pos_sep);
                    std::transform(pos_tokens.begin(), pos_tokens.end(), std::back_inserter(positions),
                                   static_cast<pos_t (*)(std::string const&)>(boost::lexical_cast<pos_t, std::string>));
                }
                
                std::vector<bond_element> synchronous_meas_operators;
                synchronous_meas_operators.push_back(meas_operators);
                meas.push_back( new measurements::NRankRDM<Matrix, SymmGroup>(name, lat, ident_ops, fill_ops, synchronous_meas_operators,
                                                                              half_only, nearest_neighbors_only, positions));
            }
        }
        }
        return meas;
    }

private:
    Lattice const & lat;
    BaseParameters & parms;
    std::vector<Index<SymmGroup> > phys_indices;

    boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler;
    // Need a vector to store operators corresponding to different irreps
    std::vector<tag_type> ident, fill,
                          create_up, create_down, destroy_up, destroy_down,
                          count_up, count_down, count_up_down, docc, e2d, d2e;

    typename SymmGroup::subcharge max_irrep;

    std::vector<op_t> generate_site_specific_ops(op_t const & op) const
    {
        PGDecorator<SymmGroup> set_symm;
        std::vector<op_t> ret;
        for (typename SymmGroup::subcharge sc=0; sc < max_irrep+1; ++sc) {
            op_t mod(set_symm(op.basis(), sc));
            for (std::size_t b = 0; b < op.n_blocks(); ++b)
                mod[b] = op[b];

            ret.push_back(mod);
        }
        return ret;
    }

    std::vector<tag_type> register_site_specific(std::vector<op_t> const & ops, tag_detail::operator_kind kind)
    {
        std::vector<tag_type> ret;
        for (typename SymmGroup::subcharge sc=0; sc < max_irrep+1; ++sc) {
            std::pair<tag_type, value_type> newtag = tag_handler->checked_register(ops[sc], kind);
            assert( newtag.first < tag_handler->size() );
            assert( std::abs(newtag.second - value_type(1.)) == value_type() );
            ret.push_back(newtag.first);
        }

        return ret;
    }
};


#include "dmrg/models/chem/2u1/model_qc.hpp"

#endif