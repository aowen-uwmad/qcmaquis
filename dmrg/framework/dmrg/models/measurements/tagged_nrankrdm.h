/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *               2011-2013    Michele Dolfi <dolfim@phys.ethz.ch>
 *               2014-2014    Sebastian Keller <sebkelle@phys.ethz.ch>
 * Copyright (C) 2018 Institute for Theoretical Chemistry, ETH Zurich
 *               2018-2019    Stefan Knecht <stknecht@ethz.ch>
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

#ifndef MEASUREMENTS_TAGGED_NRANKRDM_H
#define MEASUREMENTS_TAGGED_NRANKRDM_H

#include <algorithm>
#include <functional>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/filesystem.hpp>

#include "dmrg/block_matrix/symmetry/nu1pg.h"
#include "dmrg/models/measurement.h"

#include "dmrg/models/chem/su2u1/term_maker.h"

namespace measurements_details {

    template <class symm, class = void>
    class checkpg
    {
    public:

        template <class matrix>
        bool operator()(term_descriptor<typename matrix::value_type> const & term,
                        boost::shared_ptr<TagHandler<matrix, symm> > tag_handler, Lattice const & lat)
        {
            return true;
        }
    };

    template <class symm>
    class checkpg<symm, typename boost::enable_if<symm_traits::HasPG<symm> >::type>
    {
    public:
        typedef typename symm::charge charge;
        typedef typename symm::subcharge subcharge;

        template <class matrix>
        bool operator()(term_descriptor<typename matrix::value_type> const & term,
				boost::shared_ptr<TagHandler<matrix, symm> > tag_handler,
				Lattice const & lat)
        {
            typedef typename TagHandler<matrix, symm>::op_t op_t;

		charge acc = symm::IdentityCharge;
            for (std::size_t p = 0; p < term.size(); ++p) {
		    charge local = symm::IdentityCharge;
		    if (tag_handler->is_fermionic(term.operator_tag(p)))
                    // stknecht: this check does not work properly for U1DG. FIXME!
                    if(symm_traits::HasU1<symm>::value)
                        if( p % 2 == 0)
		                symm::irrep(local) = lat.get_prop<subcharge>("type", term.position(p));
                        else
		                symm::irrep(local) = symm::adjoin(lat.get_prop<subcharge>("type", term.position(p)));
                    else
    		            symm::irrep(local) = lat.get_prop<subcharge>("type", term.position(p));

                    //maquis::cout << " index " << p << " --> accumulated charge (before) " << acc << " local charge " << local << std::endl;
		    acc = symm::fuse(acc, local);
            //maquis::cout << " index " << p << " --> accumulated charge (after ) " << acc << " local charge " << local << std::endl;
            }

		if (acc == symm::IdentityCharge)
            	return true;

		return false;
        }
    };

    template <class T>
    T get_indx_contr(std::vector<T> const & positions)
    {
        T contr_indx;
        return (((positions[0]+1-1)*(positions[0]+1)*(positions[0]+1+1)*(positions[0]+1+2))/24
               +((positions[1]+1-1)*(positions[1]+1)*(positions[1]+1+1))/6
               +((positions[2]+1-1)*(positions[2]+1))/2
               +  positions[3]+1
               );
    };

    template <class T>
    class compare_norm
    {
    public:
        bool operator()(std::vector<T> const & positions)
        {
            std::vector<T> positions_lhs;
            std::vector<T> positions_rhs;

            // set positions:
            for (int i=0; i<4; ++i) positions_lhs.push_back(positions[i]);
            for (int i=4; i<8; ++i) positions_rhs.push_back(positions[i]);

            // reverse sorting to ensure maximum norm
            std::sort(positions_lhs.begin(), positions_lhs.end(), std::greater<T>());
            std::sort(positions_rhs.begin(), positions_rhs.end(), std::greater<T>());

            T norm_lhs = (((positions_lhs[0]+1-1)*(positions_lhs[0]+1)*(positions_lhs[0]+1+1)*(positions_lhs[0]+1+2))/24
                   +((positions_lhs[1]+1-1)*(positions_lhs[1]+1)*(positions_lhs[1]+1+1))/6
                   +((positions_lhs[2]+1-1)*(positions_lhs[2]+1))/2
                   +  positions_lhs[3]+1
                   );
            T norm_rhs = (((positions_rhs[0]+1-1)*(positions_rhs[0]+1)*(positions_rhs[0]+1+1)*(positions_rhs[0]+1+2))/24
                   +((positions_rhs[1]+1-1)*(positions_rhs[1]+1)*(positions_rhs[1]+1+1))/6
                   +((positions_rhs[2]+1-1)*(positions_rhs[2]+1))/2
                   +  positions_rhs[3]+1
                   );

            //maquis::cout << "lhs norm "  << norm_lhs << " <--> rhs norm " << norm_rhs << std::endl;

            if(norm_rhs > norm_lhs)
                return true;

            return false;
        }
    };
}

namespace measurements {

    template <class Matrix, class SymmGroup, class = void>
    class TaggedNRankRDM : public measurement<Matrix, SymmGroup> {

        typedef measurement<Matrix, SymmGroup> base;

        typedef typename Model<Matrix, SymmGroup>::term_descriptor term_descriptor;

        typedef Lattice::pos_t pos_t;
        typedef std::vector<pos_t> positions_type;

        typedef typename base::op_t op_t;
        typedef typename OPTable<Matrix, SymmGroup>::tag_type tag_type;
        typedef typename Matrix::value_type value_type;

        typedef std::vector<tag_type> tag_vec;
        typedef std::vector<tag_vec> bond_term;
        typedef std::pair<std::vector<tag_vec>, value_type> scaled_bond_term;

    public:
        TaggedNRankRDM(std::string const& name_, const Lattice & lat,
                       boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_,
                       tag_vec const & identities_, tag_vec const & fillings_, std::vector<scaled_bond_term> const& ops_,
                       bool half_only_, positions_type const& positions_ = positions_type(),
                       std::string const& ckp_ = std::string(""))
        : base(name_)
        , lattice(lat)
        , tag_handler(tag_handler_)
        , positions_first(positions_)
        , identities(identities_)
        , fillings(fillings_)
        , operator_terms(ops_)
        , half_only(half_only_)
        , bra_ckp(ckp_)
        {
            pos_t extent = operator_terms.size() > 2 ? lattice.size() : lattice.size()-1;
            // the default setting is only required for "measure_correlation"
            if (positions_first.size() == 0 && operator_terms[0].first.size() == 2)
            {
                positions_first.resize(extent);
                std::iota(positions_first.begin(), positions_first.end(), 0);
            }

            //this->cast_to_real = is_hermitian_meas(ops[0]);
            this->cast_to_real = false;
        }

        void evaluate(MPS<Matrix, SymmGroup> const& ket_mps, boost::optional<reduced_mps<Matrix, SymmGroup> const&> rmps = boost::none)
        {
            this->vector_results.clear();
            this->labels.clear();

            MPS<Matrix, SymmGroup> bra_mps;
            if (bra_ckp != "") {
                if(boost::filesystem::exists(bra_ckp))
                    load(bra_ckp, bra_mps);
                else
                    throw std::runtime_error("The bra checkpoint file " + bra_ckp + " was not found\n");
            }

            maquis::cout << " measuring in 2u1 version of tagged_nrank " << std::endl;

            if (operator_terms[0].first.size() == 2)
                measure_correlation(bra_mps, ket_mps);
            else if (operator_terms[0].first.size() == 4)
                measure_2rdm(bra_mps, ket_mps);
            else if (operator_terms[0].first.size() == 6)
                measure_3rdm(bra_mps, ket_mps);
            else if (operator_terms[0].first.size() == 8)
                measure_4rdm(bra_mps, ket_mps);
            else
                throw std::runtime_error("correlation measurements at the moment supported with 2, 4, 6 and 8 operators, size is "
                                          + boost::lexical_cast<std::string>(operator_terms[0].first.size()));
        }

    protected:

        measurement<Matrix, SymmGroup>* do_clone() const
        {
            return new TaggedNRankRDM(*this);
        }

        void measure_correlation(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                                 MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            #ifdef MAQUIS_OPENMP
            #pragma omp parallel for
            #endif
            for (std::size_t i = 0; i < positions_first.size(); ++i) {
                pos_t p1 = positions_first[i];
                boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));

                std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                std::vector<std::vector<pos_t> > num_labels;
                for (pos_t p2 = p1+1; p2 < lattice.size(); ++p2)
                {
                    pos_t pos_[2] = {p1, p2};
                    std::vector<pos_t> positions(pos_, pos_ + 2);

                    tag_vec operators(2);
                    operators[0] = operator_terms[0].first[0][lattice.get_prop<typename SymmGroup::subcharge>("type", p1)];
                    operators[1] = operator_terms[0].first[1][lattice.get_prop<typename SymmGroup::subcharge>("type", p2)];

                    // check if term is allowed by symmetry
                    term_descriptor term = generate_mpo::arrange_operators(positions, operators, tag_handler_local);

                    if(measurements_details::checkpg<SymmGroup>()(term, tag_handler_local, lattice))
                    {
                        MPO<Matrix, SymmGroup> mpo = generate_mpo::sign_and_fill(term, identities, fillings, tag_handler_local, lattice);
                        typename MPS<Matrix, SymmGroup>::scalar_type value = operator_terms[0].second * expval(bra_mps, ket_mps, mpo);

                        dct.push_back(value);
                        num_labels.push_back(positions);
                    }
                    else {
                        dct.push_back(0.0);
                        num_labels.push_back(positions);
                    }
		            if(bra_neq_ket){
                        pos_t pos_[2] = {p2, p1};
                        std::vector<pos_t> positions(pos_, pos_ + 2);

                        tag_vec operators(2);
                        operators[0] = operator_terms[0].first[0][lattice.get_prop<typename SymmGroup::subcharge>("type", p2)];
                        operators[1] = operator_terms[0].first[1][lattice.get_prop<typename SymmGroup::subcharge>("type", p1)];

                        // check if term is allowed by symmetry
                        term_descriptor term = generate_mpo::arrange_operators(positions, operators, tag_handler_local);
                        if(measurements_details::checkpg<SymmGroup>()(term, tag_handler_local, lattice))
                        {
                            MPO<Matrix, SymmGroup> mpo = generate_mpo::sign_and_fill(term, identities, fillings, tag_handler_local, lattice);
                            typename MPS<Matrix, SymmGroup>::scalar_type value = operator_terms[0].second * expval(bra_mps, ket_mps, mpo);

                            dct.push_back(value);
                            num_labels.push_back(positions);
                        }
                        else {
                            dct.push_back(0.0);
                            num_labels.push_back(positions);
                        }
                    }
                }

                std::vector<std::string> lbt = label_strings(lattice,  num_labels);

                // save results and labels
                #ifdef MAQUIS_OPENMP
                #pragma omp critical
                #endif
                {
                    this->vector_results.reserve(this->vector_results.size() + dct.size());
                    std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                    this->labels.reserve(this->labels.size() + dct.size());
                    std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                    this->labels_num.reserve(this->labels_num.size() + dct.size());
                    std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                }
            }
        }

        void measure_2rdm(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                          MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            #ifdef MAQUIS_OPENMP
            #pragma omp parallel for collapse(1)
            #endif
            for (pos_t p1 = 0; p1 < lattice.size(); ++p1)
            for (pos_t p2 = 0; p2 < lattice.size(); ++p2)
            {
                boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));

                // Permutation symmetry for bra == ket: pqrs == rspq == qpsr == srqp
                // if bra != ket, pertmutation symmetry is only pqrs == qpsr
                pos_t subref = 0;
                if (bra_neq_ket)
                    subref = 0;
                else
                    subref = std::min(p1, p2);

                for (pos_t p3 = subref; p3 < lattice.size(); ++p3)
                {
                    std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                    std::vector<std::vector<pos_t> > num_labels;

                    for (pos_t p4 = p3; p4 < lattice.size(); ++p4)
                    {
                        pos_t pos_[4] = {p1, p2, p3, p4};
                        std::vector<pos_t> positions(pos_, pos_ + 4);

                        // Loop over operator terms that are measured synchronously and added together
                        // Used e.g. for the four spin combos of the 2-RDM
                        typename MPS<Matrix, SymmGroup>::scalar_type value = 0;
                        bool checkpass = false;
                        for (std::size_t synop = 0; synop < operator_terms.size(); ++synop) {

                            tag_vec operators(4);
                            operators[0] = operator_terms[synop].first[0][lattice.get_prop<typename SymmGroup::subcharge>("type", p1)];
                            operators[1] = operator_terms[synop].first[1][lattice.get_prop<typename SymmGroup::subcharge>("type", p2)];
                            operators[2] = operator_terms[synop].first[2][lattice.get_prop<typename SymmGroup::subcharge>("type", p3)];
                            operators[3] = operator_terms[synop].first[3][lattice.get_prop<typename SymmGroup::subcharge>("type", p4)];

                            // check if term is allowed by symmetry
                            term_descriptor term = generate_mpo::arrange_operators(positions, operators, tag_handler_local);
                            if(checkpass || measurements_details::checkpg<SymmGroup>()(term, tag_handler_local, lattice))
                            {
                                checkpass = true;
                                MPO<Matrix, SymmGroup> mpo = generate_mpo::sign_and_fill(term, identities, fillings, tag_handler_local, lattice);
                                value += operator_terms[synop].second * expval(bra_mps, ket_mps, mpo);
                            }
                            else break;
                        }
                        if(checkpass)
                        {
                             dct.push_back(value);
                             num_labels.push_back(positions);
                        }
                        else
                        {
                             dct.push_back(0.0);
                             num_labels.push_back(positions);
                        }
                    }

                    std::vector<std::string> lbt = label_strings(lattice,  num_labels);

                    // save results and labels
                    #ifdef MAQUIS_OPENMP
                    #pragma omp critical
                    #endif
                    {
                        this->vector_results.reserve(this->vector_results.size() + dct.size());
                        std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                        this->labels.reserve(this->labels.size() + dct.size());
                        std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                        this->labels_num.reserve(this->labels_num.size() + dct.size());
                        std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                    }
                }
            }
        }

        void measure_3rdm(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                          MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified -- if bra != ket, no transpose symmetry
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            pos_t p1_start = 0;
            pos_t p2_start = 0;
            pos_t p3_start = 0;
            pos_t p1_end   = lattice.size();
            pos_t p2_end   = lattice.size();
            pos_t p3_end   = lattice.size();
            pos_t p4_end   = lattice.size();
            pos_t p5_end   = lattice.size();

            if(positions_first.size() == 2){
                p1_start = positions_first[0];
                p2_start = positions_first[1];
                p1_end   = positions_first[0]+1;
                p2_end   = positions_first[1]+1;
            }

            // Leon: allow both 2-index and 3-index 3-RDM measurement splitting
            // warning: if two indices are used, the order in the input file is p1,p2; but for three it's p2,p1,p3 !

            if(positions_first.size() == 3){
                p1_start = positions_first[0];
                p2_start = positions_first[1];
                p3_start = positions_first[2];
                p1_end   = positions_first[0]+1;
                p2_end   = positions_first[1]+1;
                p3_end   = positions_first[2]+1;
            }

            #ifdef MAQUIS_OPENMP
            #pragma omp parallel for collapse(5) schedule (dynamic,1)
            #endif
            for (pos_t p1 = p1_start; p1 < p1_end; ++p1)
            for (pos_t p2 = p2_start; p2 < p2_end; ++p2)
            for (pos_t p3 = p3_start; p3 < p3_end; ++p3)
            for (pos_t p4 = 0;                p4 < p4_end; ++p4)
            for (pos_t p5 = 0;                p5 < p5_end; ++p5)
            {
                    // index restrictions
                    if(p1 < p2 ) continue;
                    if((p1 == p2 && p1 == p3) || (p3 < std::min(p1, p2))) continue;
                    if(!bra_neq_ket && p4 < std::min(p1, p2)) continue;
                    if(!bra_neq_ket && p5 < std::min(p1, p2)) continue;

                    boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));
                    MPS<Matrix, SymmGroup> ket_mps_local = ket_mps;
                    MPS<Matrix, SymmGroup> bra_mps_local = bra_mps;

                    std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                    std::vector<std::vector<pos_t> > num_labels;
                    for (pos_t p6 = std::min(p4, p5); p6 < lattice.size(); ++p6)
                    {
                        // sixth index must be different if p4 == p5
                        if(p4 == p5 && p4 == p6) continue;

                        // defines position vector for spin-free 3-RDM element
                        pos_t pos_[6] = {p1, p2, p3, p4, p5, p6};
                        std::vector<pos_t> positions(pos_, pos_ + 6);

                        // Loop over operator terms that are measured synchronously and added together
                        // Used e.g. for the spin combos of the 3-RDM
                        typename MPS<Matrix, SymmGroup>::scalar_type value = 0;
                        bool measured = false;
                        for (std::size_t synop = 0; synop < operator_terms.size(); ++synop) {

                            tag_vec operators(6);
                            operators[0] = operator_terms[synop].first[0][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[0])];
                            operators[1] = operator_terms[synop].first[1][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[1])];
                            operators[2] = operator_terms[synop].first[2][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[2])];
                            operators[3] = operator_terms[synop].first[3][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[3])];
                            operators[4] = operator_terms[synop].first[4][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[4])];
                            operators[5] = operator_terms[synop].first[5][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[5])];

                            // check if term is allowed by symmetry
                            term_descriptor term = generate_mpo::arrange_operators(positions, operators, tag_handler_local);
                            if(not measurements_details::checkpg<SymmGroup>()(term, tag_handler_local, lattice))
                                continue;
                            measured = true;

                            MPO<Matrix, SymmGroup> mpo = generate_mpo::sign_and_fill(term, identities, fillings, tag_handler_local, lattice);
                            value += operator_terms[synop].second * expval(bra_mps_local, ket_mps_local, mpo);

                        }
                        if(measured)
                        {
                            dct.push_back(value);
                            num_labels.push_back(positions);
                        }

                    }// p6

                    std::vector<std::string> lbt = label_strings(lattice,  num_labels);

                    // save results and labels
                    #ifdef MAQUIS_OPENMP
                    #pragma omp critical
                    #endif
                    {
                        this->vector_results.reserve(this->vector_results.size() + dct.size());
                        std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                        this->labels.reserve(this->labels.size() + dct.size());
                        std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                        this->labels_num.reserve(this->labels_num.size() + dct.size());
                        std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                    }
            }// p1, p2, p3, p4, p5
        }


        void measure_4rdm(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                          MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            pos_t p4_start = 0;
            pos_t p3_start = 0;
            pos_t p1_start = lattice.size()-1;
            pos_t p2_start = p1_start;
            pos_t p4_end   = lattice.size()-1;
            pos_t p3_end   = lattice.size()-1;
            pos_t p1_end   = 0;
            pos_t p2_end   = 0;
            pos_t p_max    = lattice.size();
            if(positions_first.size() == 4){
                p4_start = positions_first[0];
                p3_start = positions_first[1];
                p1_start = positions_first[2];
                p2_start = positions_first[3];
                p4_end   = positions_first[0]+1;
                p3_end   = positions_first[1]+1;
                p1_end   = positions_first[2];
                p2_end   = positions_first[3];
            }
            //#ifdef MAQUIS_OPENMP
            //#pragma omp parallel for collapse(4) schedule(dynamic,1)
            //#endif
            for (pos_t p4 = p4_start ; p4 < p4_end; ++p4)
            for (pos_t p3 = p3_start ; p3 < p3_end; ++p3)
            {
                 for (pos_t p1 = p1_start; p1 >= p1_end; --p1)
                 {
                      if(p4 > p3 || p4 > p1 || p3 > p1) continue;

                      if(positions_first.size() == 0){
                          p2_start = p1;
                          p2_end   = 0;
                      }

                      for (pos_t p2 = p2_start; p2 >= p2_end; --p2)
                      {
                          if(p3 > p2) continue;

                          // third index must be different if p1 == p2
                          if(p1 == p2 && p3 == p1) continue;

                          // fourth index must be different if p1 == p2 or p1 == p3 or p2 == p3
                          if((p1 == p2 && p4 == p1) || (p1 == p3 && p4 == p1) || (p2 == p3 && p4 == p2)) continue;

                          bool double_equal = (p1 == p2 && p3 == p4);             // case 1
                          bool     ij_equal = (p1 == p2 && p2 != p3 && p3 != p4); // case 2
                          bool     jk_equal = (p1 != p2 && p2 == p3 && p3 != p4); // case 3
                          bool     kl_equal = (p1 != p2 && p2 != p3 && p3 == p4); // case 4
                          bool   none_equal = (p1 != p2 && p2 != p3 && p3 != p4); // case 5

                          #ifdef MAQUIS_OPENMP
                          #pragma omp parallel for collapse(3) schedule(dynamic,1)
                          #endif

                          for (pos_t p5 = p1; p5 >= 0; --p5)
                          for (pos_t p6 = p1; p6 >= 0; --p6)
                          for (pos_t p7 = 0; p7 < p_max; ++p7)
                          {
                              // set restrictions on index p6
                              if ((double_equal || ij_equal) && p6 > p5 ) continue;

                              // set restrictions on index p7
                              if(double_equal)
                                  if(p7 > p5) continue;
                              else
                                  if(p7 > p1) continue;

                              if(p5 == p6 && p5 == p7) continue;

                              // set restrictions on index p8
                              pos_t p8_end = 0;
                              if (double_equal)
                                  p8_end = p5+1;
                              else if (kl_equal)
                                  p8_end = p7+1;
                              else
                                  p8_end = p1+1;

                              MPS<Matrix, SymmGroup> ket_mps_local = ket_mps;
                              boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));
                              std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                              std::vector<std::vector<pos_t> > num_labels;

                              for (pos_t p8 = 0; p8 < p8_end; ++p8)
                              {
                                  // eighth index must be different if p5 == p6 or p5 == p7 or p6 == p7
                                  if((p5 == p6 && p8 == p5) || (p5 == p7 && p8 == p5) || (p6 == p7 && p8 == p6)) continue;

                                  // case 1
                                  if(double_equal && p8 > p7 &&            (p8 < p6 || p8 == p5 || p8 == p6 || p5 == p6 || p5 == p7 || p6 == p7)) continue;
                                  if(double_equal && p8 > p7 && p7 > p6 && (p8 < p6 || p8 == p5 || p8 == p6 || p5 == p6 || p5 == p7 || p6 == p7)) continue;
                                  if(double_equal && p8 > p7 && p7 > p6 && (p8 > p6 || p8 == p5 || p8 == p6 || p5 == p6 || p5 == p7 || p6 == p7)) continue;
                                  if(double_equal && p8 < p7 && p7 > p6 && (p8 == p5 || p8 == p6 || p7 == p5 || p7 == p6)) continue;
                                  if(double_equal && p8 < p7 && p7 > p6 && p8 <  p6 ) continue;
                                  // case 2/3/4: 2x2 equal indices
                                  if((ij_equal || jk_equal || kl_equal) && p5 == p6 && p7 == p8 && p7 > p6) continue;

                                  // case 2/4: 2 equal indices
                                  if((ij_equal || kl_equal) && (p5 == p7 || p5 == p8 || p6 == p7|| p6 == p8)) continue;

                                  // case 3
                                  if(jk_equal){
                                      // 2 equal indices
                                      if(p5 == p7 || p6 == p7 || p6 == p8) continue;
                                      if(p5 == p6 && p7 != p8 && p6 > p7) continue;
                                      if(p5 == p8 && p7 > p6) continue;
                                      // none equal
                                      if(std::min(p5,p6) != std::min(p7,p8) && p7 != p8 && p7 > p6) continue;
                                  }

                                  // case 5
                                  if(none_equal){
                                      if((p5 == p6 && p7 == p8 && p5 < p7) || (p5 == p7 && p6 == p8 && p5 < p6) || (p5 == p8 && p6 == p7 && p5 < p6)) continue;
                                  }

                                  // defines position vector for spin-free 4-RDM element
                                  pos_t pos_[8] = {p1, p2, p3, p4, p5, p6, p7, p8};
                                  std::vector<pos_t> positions(pos_, pos_ + 8);

                                  // check norm of lhs and rhs - skip if norm of rhs > lhs
                                  if(measurements_details::compare_norm<pos_t>()(positions)) continue;

                                  // Loop over operator terms that are measured synchronously and added together
                                  // Used e.g. for the 16 spin combos of the 4-RDM
                                  typename MPS<Matrix, SymmGroup>::scalar_type value = 0;
                                  bool measured = false;
                                  for (std::size_t synop = 0; synop < operator_terms.size(); ++synop) {

                                      tag_vec operators(8);
                                      operators[0] = operator_terms[synop].first[0][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[0])];
                                      operators[1] = operator_terms[synop].first[1][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[1])];
                                      operators[2] = operator_terms[synop].first[2][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[2])];
                                      operators[3] = operator_terms[synop].first[3][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[3])];
                                      operators[4] = operator_terms[synop].first[4][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[4])];
                                      operators[5] = operator_terms[synop].first[5][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[5])];
                                      operators[6] = operator_terms[synop].first[6][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[6])];
                                      operators[7] = operator_terms[synop].first[7][lattice.get_prop<typename SymmGroup::subcharge>("type", positions[7])];

                                      // check if term is allowed by symmetry
                                      term_descriptor term = generate_mpo::arrange_operators(positions, operators, tag_handler_local);
                                      if(not measurements_details::checkpg<SymmGroup>()(term, tag_handler_local, lattice))
                                           continue;
                                      measured = true;

                                      MPO<Matrix, SymmGroup> mpo = generate_mpo::sign_and_fill(term, identities, fillings, tag_handler_local, lattice);
                                      typename MPS<Matrix, SymmGroup>::scalar_type local_value = expval(ket_mps_local, ket_mps_local, mpo);
                                      //maquis::cout << "synop term " << synop+1 << "--> local value: " << local_value << std::endl;
                                      //value += operator_terms[synop].second * expval(ket_mps_local, ket_mps_local, mpo);
                                      value += operator_terms[synop].second * local_value;

                                  }// spin combo loop
                                  if(measured)
                                  {
                                      // debug print
                                      /* if (std::abs(value) > 0)
                                      {
                                          std::transform(positions.begin(), positions.end(), std::ostream_iterator<pos_t>(std::cout, " "), boost::lambda::_1 + 1);
                                          maquis::cout << " " << value << std::endl;
                                      } */
                                      dct.push_back(value);
                                      num_labels.push_back(positions);
                                  }
                              } // p8 loop

                              std::vector<std::string> lbt = label_strings(lattice,  num_labels);
                              // save results and labels
                              #ifdef MAQUIS_OPENMP
                              #pragma omp critical
                              #endif
                              {
                                  this->vector_results.reserve(this->vector_results.size() + dct.size());
                                  std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                                  this->labels.reserve(this->labels.size() + dct.size());
                                  std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                                  this->labels_num.reserve(this->labels_num.size() + dct.size());
                                  std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                              }
                          } // p5,p6,p7
                      } // p1
                 } // p2
             } // p4+p3
         }

    private:
        Lattice lattice;
        boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler;
        positions_type positions_first;
        tag_vec identities, fillings;
        std::vector<scaled_bond_term> operator_terms;
        bool half_only;

        std::string bra_ckp;
    };


    template <class Matrix, class SymmGroup>
    class TaggedNRankRDM<Matrix, SymmGroup, typename boost::enable_if<symm_traits::HasSU2<SymmGroup> >::type >
        : public measurement<Matrix, SymmGroup>
    {
        typedef measurement<Matrix, SymmGroup> base;

        typedef typename Model<Matrix, SymmGroup>::term_descriptor term_descriptor;

        typedef Lattice::pos_t pos_t;
        typedef std::vector<pos_t> positions_type;

        typedef typename base::op_t op_t;
        typedef typename OPTable<Matrix, SymmGroup>::tag_type tag_type;
        typedef typename Matrix::value_type value_type;

        typedef std::vector<tag_type> tag_vec;
        typedef std::vector<tag_vec> bond_term;
        typedef std::pair<std::vector<tag_vec>, value_type> scaled_bond_term;

        typedef TermMakerSU2<Matrix, SymmGroup> TM;

    public:
        TaggedNRankRDM(std::string const& name_, const Lattice & lat,
                       boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_,
                       typename TM::OperatorCollection const & op_collection_,
                       positions_type const& positions_ = positions_type(),
                       std::string const& ckp_ = std::string(""))
        : base(name_)
        , lattice(lat)
        , tag_handler(tag_handler_)
        , op_collection(op_collection_)
        , positions_first(positions_)
        , identities(op_collection.ident.no_couple)
        , fillings(op_collection.fill.no_couple)
        , bra_ckp(ckp_)
        {
            pos_t extent = lattice.size();
            if (positions_first.size() == 0)
            {
                positions_first.resize(extent);
                std::iota(positions_first.begin(), positions_first.end(), 0);
            }

            //this->cast_to_real = is_hermitian_meas(ops[0]);
            this->cast_to_real = false;
        }

        void evaluate(MPS<Matrix, SymmGroup> const& ket_mps, boost::optional<reduced_mps<Matrix, SymmGroup> const&> rmps = boost::none)
        {
            this->vector_results.clear();
            this->labels.clear();
            this->labels_num.clear();

            MPS<Matrix, SymmGroup> bra_mps;
            if (bra_ckp != "") {
                if(boost::filesystem::exists(bra_ckp))
                    load(bra_ckp, bra_mps);
                else
                    throw std::runtime_error("The bra checkpoint file " + bra_ckp + " was not found\n");
            }

            maquis::cout << " measuring in su2 version of tagged_nrank " << std::endl;

            if (this->name() == "oneptdm")
                measure_correlation(bra_mps, ket_mps);
            else if (this->name() == "twoptdm")
                measure_2rdm(bra_mps, ket_mps);
        }

    protected:

        measurement<Matrix, SymmGroup>* do_clone() const
        {
            return new TaggedNRankRDM(*this);
        }

        void measure_correlation(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                                 MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            #ifdef MAQUIS_OPENMP
            #pragma omp parallel for schedule(dynamic)
            #endif
            for (std::size_t i = 0; i < positions_first.size(); ++i) {
                pos_t p1 = positions_first[i];
                boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));

                std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                std::vector<std::vector<pos_t> > num_labels;
                for (pos_t p2 = p1; p2 < lattice.size(); ++p2)
                {
                    pos_t pos_[2] = {p1, p2};
                    std::vector<pos_t> positions(pos_, pos_ + 2);

                    std::vector<term_descriptor> terms;
                    if (p1 != p2)
                        // The sqrt(2.) balances the magnitudes of Clebsch coeffs C^{1/2 1/2 0}_{mrm'} which apply at the second spin-1/2 operator
                        terms.push_back(TermMakerSU2<Matrix, SymmGroup>::positional_two_term(
                            true, op_collection.ident.no_couple, std::sqrt(2.), p1, p2, op_collection.create.couple_down, op_collection.create.fill_couple_up,
                                                              op_collection.destroy.couple_down, op_collection.destroy.fill_couple_up, lattice
                        ));
                    else {
                        term_descriptor term;
                        term.coeff = 1.;
                        term.push_back( boost::make_tuple(p1, op_collection.count.no_couple[lattice.get_prop<typename SymmGroup::subcharge>("type", p1)]) );
                        terms.push_back(term);
                    }

                    // check if term is allowed by symmetry
                    if(not measurements_details::checkpg<SymmGroup>()(terms[0], tag_handler_local, lattice))
                           continue;

                    generate_mpo::TaggedMPOMaker<Matrix, SymmGroup> mpo_m(lattice, op_collection.ident.no_couple, op_collection.ident_full.no_couple,
                                                                          op_collection.fill.no_couple, tag_handler_local, terms);
                    MPO<Matrix, SymmGroup> mpo = mpo_m.create_mpo();
                    typename MPS<Matrix, SymmGroup>::scalar_type value = expval(bra_mps, ket_mps, mpo);

                    dct.push_back(value);
                    num_labels.push_back(positions);
                }

                // the lattice knows the ordering and provides the correct orbital label for each position
                std::vector<std::string> lbt = label_strings(lattice,  num_labels);

                // save results and labels
                #ifdef MAQUIS_OPENMP
                #pragma omp critical
                #endif
                {
                    this->vector_results.reserve(this->vector_results.size() + dct.size());
                    std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                    this->labels.reserve(this->labels.size() + dct.size());
                    std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                    this->labels_num.reserve(this->labels_num.size() + dct.size());
                    std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                }
            }
        }

        void measure_2rdm(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                          MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            #ifdef MAQUIS_OPENMP
            #pragma omp parallel for collapse(1) schedule(dynamic)
            #endif
            for (pos_t p1 = 0; p1 < lattice.size(); ++p1)
            for (pos_t p2 = 0; p2 < lattice.size(); ++p2)
            {
                boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));

                // Permutation symmetry for bra == ket: pqrs == rspq == qpsr == srqp
                pos_t subref = std::min(p1, p2);

                // if bra != ket, pertmutation symmetry is only pqrs == qpsr
                if (bra_neq_ket) subref = 0;

                std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                std::vector<std::vector<pos_t> > num_labels;

                for (pos_t p3 = subref; p3 < lattice.size(); ++p3)
                {
                    for (pos_t p4 = p3; p4 < lattice.size(); ++p4)
                    {
                        pos_t pos_[4] = {p1, p2, p3, p4};
                        std::vector<pos_t> positions(pos_, pos_ + 4);

                        std::vector<term_descriptor> terms = SpinSumSU2<Matrix, SymmGroup>::V_term(1., p1, p2, p3, p4, op_collection, lattice);

                        // check if term is allowed by symmetry
                        if(not measurements_details::checkpg<SymmGroup>()(terms[0], tag_handler_local, lattice))
                               continue;

                        generate_mpo::TaggedMPOMaker<Matrix, SymmGroup> mpo_m(lattice, op_collection.ident.no_couple, op_collection.ident_full.no_couple,
                                                                              op_collection.fill.no_couple, tag_handler_local, terms);
                        MPO<Matrix, SymmGroup> mpo = mpo_m.create_mpo();
                        typename MPS<Matrix, SymmGroup>::scalar_type value = expval(bra_mps, ket_mps, mpo);

                        dct.push_back(value);
                        num_labels.push_back(positions);
                    }

                }

                std::vector<std::string> lbt = label_strings(lattice,  num_labels);

                // save results and labels
                #ifdef MAQUIS_OPENMP
                #pragma omp critical
                #endif
                {
                    this->vector_results.reserve(this->vector_results.size() + dct.size());
                    std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                    this->labels.reserve(this->labels.size() + dct.size());
                    std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                    this->labels_num.reserve(this->labels_num.size() + dct.size());
                    std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                }
            }
        }

    private:
        Lattice lattice;
        boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler;

        typename TM::OperatorCollection op_collection;

        positions_type positions_first;
        tag_vec identities, fillings;

        std::string bra_ckp;
    };

    template <class Matrix, class SymmGroup>
    class TaggedNRankRDM<Matrix, SymmGroup, typename boost::enable_if<symm_traits::HasU1<SymmGroup> >::type >
        : public measurement<Matrix, SymmGroup>
    {
        typedef measurement<Matrix, SymmGroup> base;

        typedef typename Model<Matrix, SymmGroup>::term_descriptor term_descriptor;

        typedef Lattice::pos_t pos_t;
        typedef std::vector<pos_t> positions_type;

        typedef typename base::op_t op_t;
        typedef typename OPTable<Matrix, SymmGroup>::tag_type tag_type;
        typedef typename Matrix::value_type value_type;

        typedef std::vector<tag_type> tag_vec;
        typedef std::vector<tag_vec> bond_term;
        typedef std::pair<std::vector<tag_vec>, value_type> scaled_bond_term;

    public:
        TaggedNRankRDM(std::string const& name_, const Lattice & lat,
                       boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_,
                       tag_vec const & identities_, tag_vec const & fillings_, std::vector<scaled_bond_term> const& ops_,
                       bool half_only_, positions_type const& positions_ = positions_type(),
                       std::string const& ckp_ = std::string(""))
        : base(name_)
        , lattice(lat)
        , tag_handler(tag_handler_)
        , positions_first(positions_)
        , identities(identities_)
        , fillings(fillings_)
        , operator_terms(ops_)
        , bra_ckp(ckp_)
        {
            pos_t extent = lattice.size();
            if (positions_first.size() == 0)
            {
                positions_first.resize(extent);
                std::iota(positions_first.begin(), positions_first.end(), 0);
            }

            this->cast_to_real = false;
        }

        void evaluate(MPS<Matrix, SymmGroup> const& ket_mps, boost::optional<reduced_mps<Matrix, SymmGroup> const&> rmps = boost::none)
        {
            this->vector_results.clear();
            this->labels.clear();
            this->labels_num.clear();

            MPS<Matrix, SymmGroup> bra_mps;
            if (bra_ckp != "") {
                if(boost::filesystem::exists(bra_ckp))
                    load(bra_ckp, bra_mps);
                else
                    throw std::runtime_error("The bra checkpoint file " + bra_ckp + " was not found\n");
            }

            maquis::cout << " measuring in rel version of tagged_nrank " << std::endl;

            if (operator_terms[0].first.size() == 2)
                measure_correlation(bra_mps, ket_mps);
            else if (operator_terms[0].first.size() == 4)
                measure_2rdm(bra_mps, ket_mps);
          //else if (operator_terms[0].first.size() == 6)
          //    measure_3rdm(bra_mps, ket_mps);
          //else if (operator_terms[0].first.size() == 8)
          //    measure_4rdm(bra_mps, ket_mps);
            else
                throw std::runtime_error("relativistic correlation measurements at the moment supported with 2 and 4 operators, size is "
                                          + boost::lexical_cast<std::string>(operator_terms[0].first.size()));
        }

    protected:

        measurement<Matrix, SymmGroup>* do_clone() const
        {
            return new TaggedNRankRDM(*this);
        }

        void measure_correlation(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                                 MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            #ifdef MAQUIS_OPENMP
            #pragma omp parallel for
            #endif
            for (std::size_t i = 0; i < positions_first.size(); ++i) {
                pos_t p1 = positions_first[i];
                boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));

                std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                std::vector<std::vector<pos_t> > num_labels;
                for (pos_t p2 = p1; p2 < lattice.size(); ++p2)
                {
                    pos_t pos_[2] = {p1, p2};
                    std::vector<pos_t> positions(pos_, pos_ + 2);

                    tag_vec operators(2);
                    operators[0] = operator_terms[0].first[0][lattice.get_prop<typename SymmGroup::subcharge>("type", p1)];
                    operators[1] = operator_terms[0].first[1][lattice.get_prop<typename SymmGroup::subcharge>("type", p2)];

                    // check if term is allowed by symmetry
                    term_descriptor term = generate_mpo::arrange_operators(positions, operators, tag_handler_local);

                    //if(measurements_details::checkpg<SymmGroup>()(term, tag_handler_local, lattice))
                    {
                        MPO<Matrix, SymmGroup> mpo = generate_mpo::sign_and_fill(term, identities, fillings, tag_handler_local, lattice);
                        typename MPS<Matrix, SymmGroup>::scalar_type value = operator_terms[0].second * expval(bra_mps, ket_mps, mpo);

                        dct.push_back(value);
                        num_labels.push_back(positions);
                    }
                    //else {
                    //    dct.push_back(0.0);
                    //    num_labels.push_back(positions);
                    //}
                }

                std::vector<std::string> lbt = label_strings(lattice,  num_labels);

                // save results and labels
                #ifdef MAQUIS_OPENMP
                #pragma omp critical
                #endif
                {
                    this->vector_results.reserve(this->vector_results.size() + dct.size());
                    std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                    this->labels.reserve(this->labels.size() + dct.size());
                    std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                    this->labels_num.reserve(this->labels_num.size() + dct.size());
                    std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                }
            }
        }

        void measure_2rdm(MPS<Matrix, SymmGroup> const & dummy_bra_mps,
                          MPS<Matrix, SymmGroup> const & ket_mps)
        {
            // Test if a separate bra state has been specified
            bool bra_neq_ket = (dummy_bra_mps.length() > 0);
            MPS<Matrix, SymmGroup> const & bra_mps = (bra_neq_ket) ? dummy_bra_mps : ket_mps;

            #ifdef MAQUIS_OPENMP
            #pragma omp parallel for collapse(1) schedule(dynamic)
            #endif
            for (pos_t p1 = 0; p1 < lattice.size(); ++p1)
            for (pos_t p2 = 0; p2 < lattice.size(); ++p2)
            {
                boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler_local(new TagHandler<Matrix, SymmGroup>(*tag_handler));

		    for (pos_t p3 = ((bra_neq_ket) ? 0 : std::min(p1, p2)); p3 < lattice.size(); ++p3)
                {
		            if(p1 == p2 && p1 == p3)
                        continue;

                    std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;
                    std::vector<std::vector<pos_t> > num_labels;

                    for (pos_t p4 = 0; p4 < lattice.size(); ++p4)
                    {
                       if(std::max(p1,p2)  > std::max(p3,p4)  && std::max(p3,p4) > p2)
                             continue;
                       if(std::max(p1,p2) == std::max(p3,p4) && p1 < p4)
                             continue;
		               if(p3 > p1 && p2 > p4)
                             continue;
		               if(p1 == p4 && p3 == p2)
                             continue;

                        pos_t pos_[4] = {p1, p3, p4, p2};
                        std::vector<pos_t> positions(pos_, pos_ + 4);

                        pos_t posORD_[4] = {p1, p2, p3, p4};
                        std::vector<pos_t> positionsORD(posORD_, posORD_ + 4);

                        // Loop over operator terms that are measured synchronously and added together
                        typename MPS<Matrix, SymmGroup>::scalar_type value = 0;
                        bool measured = false;
                        for (std::size_t synop = 0; synop < operator_terms.size(); ++synop) {

                            tag_vec operators(4);
                            operators[0] = operator_terms[synop].first[0][lattice.get_prop<typename SymmGroup::subcharge>("type", p1)];
                            operators[1] = operator_terms[synop].first[1][lattice.get_prop<typename SymmGroup::subcharge>("type", p2)];
                            operators[2] = operator_terms[synop].first[2][lattice.get_prop<typename SymmGroup::subcharge>("type", p3)];
                            operators[3] = operator_terms[synop].first[3][lattice.get_prop<typename SymmGroup::subcharge>("type", p4)];

                            term_descriptor term = generate_mpo::arrange_operators(positions, operators, tag_handler_local);
                            // check if term is allowed by symmetry
                            //if(not measurements_details::checkpg<SymmGroup>()(term, tag_handler_local, lattice))
                            //    continue;

                            measured = true;
                            MPO<Matrix, SymmGroup> mpo = generate_mpo::sign_and_fill(term, identities, fillings, tag_handler_local, lattice);
                            //value += operator_terms[synop].second * expval(bra_mps, ket_mps, mpo);
                            value += (this->cast_to_real) ?  maquis::real(operator_terms[synop].second * expval(bra_mps, ket_mps, mpo)) :  operator_terms[synop].second * expval(bra_mps, ket_mps, mpo);
                        }

                        if(measured)
                        {
                             /* debug print
                             if (std::abs(value) >= 0)
                             {
                                 maquis::cout << " " << value << std::endl;
                             }*/

                             dct.push_back(value);
                             num_labels.push_back(positionsORD);
                        }
                    }

                    std::vector<std::string> lbt = label_strings(lattice,  num_labels);

                    // save results and labels
                    #ifdef MAQUIS_OPENMP
                    #pragma omp critical
                    #endif
                    {
                        this->vector_results.reserve(this->vector_results.size() + dct.size());
                        std::copy(dct.rbegin(), dct.rend(), std::back_inserter(this->vector_results));

                        this->labels.reserve(this->labels.size() + dct.size());
                        std::copy(lbt.rbegin(), lbt.rend(), std::back_inserter(this->labels));

                        this->labels_num.reserve(this->labels_num.size() + dct.size());
                        std::copy(num_labels.rbegin(), num_labels.rend(), std::back_inserter(this->labels_num));
                    }
                }
            }
        }

    private:
        Lattice lattice;
        boost::shared_ptr<TagHandler<Matrix, SymmGroup> > tag_handler;
        positions_type positions_first;
        tag_vec identities, fillings;
        std::vector<scaled_bond_term> operator_terms;

        std::string bra_ckp;
    };
}

#endif
