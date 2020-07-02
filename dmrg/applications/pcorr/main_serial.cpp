/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2011-2013 by Michele Dolfi <dolfim@phys.ethz.ch>
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

#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <alps/hdf5.hpp>

#include "dmrg/block_matrix/detail/alps.hpp"
typedef alps::numeric::matrix<double> amatrix;

#ifdef USE_AMBIENT
#include "dmrg/block_matrix/detail/ambient.hpp"
typedef ambient::tiles<ambient::matrix<double> > pmatrix;
typedef pmatrix matrix;
#else
typedef amatrix matrix;
#endif

#include "dmrg/block_matrix/symmetry.h"
#if defined(USE_TWOU1)
typedef TwoU1 grp;
#elif defined(USE_U1)
typedef U1 grp;
#elif defined(USE_NONE)
typedef TrivialGroup grp;
#elif defined(USE_NU1)
typedef NU1 grp;
#else
#error "No valid symmetry defined."
#endif

#include "dmrg/models/lattice.h"
#include "dmrg/models/model.h"
#include "dmrg/models/generate_mpo.hpp"

#include "dmrg/mp_tensors/mps.h"
#include "dmrg/mp_tensors/twositetensor.h"

#include "dmrg/optimize/ietl_lanczos_solver.h"
#include "dmrg/optimize/ietl_jacobi_davidson.h"

#include "dmrg/utils/DmrgOptions.h"
#include "dmrg/utils/DmrgParameters.h"

#include "utils/timings.h"

struct corr_measurement {
    typedef operator_selector<matrix, grp>::type op_t;

    std::string name;
    std::vector<op_t> op1, op2;
    bool fermionic;
};

template<class Matrix, class SymmGroup>
void measure_correlation(MPS<Matrix, SymmGroup> const & bra,
                         MPS<Matrix, SymmGroup> const & ket,
                         Lattice const & lattice,
                         std::vector<typename operator_selector<Matrix, SymmGroup>::type> const & fillings,
                         corr_measurement const & ops,
                         std::vector<Boundary<Matrix,SymmGroup> > const & left,
                         std::vector<Boundary<Matrix,SymmGroup> > const & right,
                         std::vector<typename Matrix::value_type>& dc,
                         std::vector<std::string>& labels)
{
    typedef typename operator_selector<Matrix, SymmGroup>::type op_t;
    std::size_t L = lattice.size();

    MPOTensor<Matrix, SymmGroup> mpo_fill(1,1), mpo_op1(1,1), mpo_op2(1,1);

    for (size_t p = 0; p < L-1; ++p) {
        Timer tim("measure p="+boost::lexical_cast<std::string>(p)); tim.begin();

        std::vector<std::vector<typename Lattice::pos_t> > numeric_labels;
        std::vector<typename MPS<Matrix, SymmGroup>::scalar_type> dct;

        {
            int type = lattice.get_prop<int>("type", p);
            op_t tmp;
            gemm(fillings[type], ops.op1[type], tmp);
            mpo_op1.set(0,0, tmp);
        }

        Boundary<Matrix, SymmGroup> current = contraction::Engine<matrix, matrix, grp>::overlap_mpo_left_step(bra[p], ket[p], left[p], mpo_op1);

        for (int q = p+1; q < L; ++q) {
            int type = lattice.get_prop<int>("type", q);
            mpo_op2.set(0,0, ops.op2[type]);

            block_matrix<Matrix, SymmGroup> vec = contraction::Engine<matrix, matrix, grp>::overlap_mpo_left_step(bra[q], ket[q], current, mpo_op2)[0];

            double obs = 0.;
            for (int k=0; k<vec.n_blocks(); ++k) {
                std::size_t matched_block = right[q+1][0].find_block(vec.right_basis()[k].first, vec.left_basis()[k].first);
                if ( matched_block <  right[q+1][0].n_blocks() ) {
                    Matrix m = transpose( right[q+1][0][matched_block] );
                    obs += overlap(m, vec[k]);
                }
            }
            dct.push_back(obs);

            std::vector<typename Lattice::pos_t> lab(2);
            lab[0] = p; lab[1] = q;
            numeric_labels.push_back(lab);

            if (q < L-1) {
                mpo_fill.set(0,0, fillings[type]);
                current = contraction::Engine<matrix, matrix, grp>::overlap_mpo_left_step(bra[q], ket[q], current, mpo_fill);
            }
        }

        std::copy(dct.begin(), dct.end(), std::back_inserter(dc));

        std::vector<std::string> lbt = label_strings(lattice, numeric_labels);
        std::copy(lbt.begin(), lbt.end(), std::back_inserter(labels));
        tim.end();
    }
}

int main(int argc, char ** argv)
{
    try {
        DmrgOptions opt(argc, argv);
        if (!opt.valid) return 0;
        DmrgParameters parms = opt.parms;

        maquis::cout.precision(10);

        typedef matrix::value_type value_type;
        typedef operator_selector<matrix, grp>::type op_t;
        typedef Model<matrix, grp>::table_ptr table_ptr;
        typedef Model<matrix, grp>::tag_type tag_type;

        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

        /// Parsing model
        Lattice lattice(parms);
        int L = lattice.size();
        int ntypes = lattice.maximum_vertex_type()+1;
        Model<matrix, grp> model(lattice, parms);
        table_ptr tag_handler = model.operators_table();

        /// identities and fillings
        std::vector<op_t> identities(ntypes), fillings(ntypes);
        for (size_t type = 0; type < ntypes; ++type) {
            identities[type] = model.identity_matrix(type);
            fillings[type]   = model.filling_matrix(type);
        }

        /// parsing measurements
        std::vector<corr_measurement> measurements;
        {
            std::regex expression("^MEASURE_HALF_CORRELATIONS\\[(.*)]$");
            boost::smatch what;
            for (alps::Parameters::const_iterator it=parms.begin();it != parms.end();++it) {
                std::string lhs = it->key();

                if (std::regex_match(lhs, what, expression)) {
                    std::string value = it->value();

                    corr_measurement meas;
                    meas.name = what.str(1);
                    meas.op1.resize(ntypes);
                    meas.op2.resize(ntypes);

                    int f_ops = 0;
                    std::vector<std::string> corr_tokens;
                    boost::split(corr_tokens, value, boost::is_any_of(":"));

                    if (corr_tokens.size() != 2)
                        throw std::runtime_error("`pcorr` can compute only two-body correlation functions.");

                    for (std::vector<std::string>::const_iterator it2=corr_tokens.begin(); it2 != corr_tokens.end(); it2++)
                    {
                        enum {uknown, bosonic, fermionic} kind = uknown;

                        for (int type=0; type<ntypes; ++type) {
                            tag_type tag = model.get_operator_tag(*it2, type);
                            if (it2 == corr_tokens.begin()) meas.op1[type] = tag_handler->get_op(tag);
                            else                            meas.op2[type] = tag_handler->get_op(tag);

                            bool is_ferm = tag_handler->is_fermionic(tag);
                            if (kind == uknown)
                                kind = is_ferm ? fermionic : bosonic;
                            else if ((is_ferm && kind==bosonic) || (!is_ferm && kind==fermionic))
                                throw std::runtime_error("Model is inconsitent. On some site the operator " + *it2 + "fermionic, on others is bosonic.");
                        }

                        if (kind == fermionic) ++f_ops;
                    }

                    if (f_ops % 2 != 0)
                        throw std::runtime_error("Number of fermionic operators has to be even.");

                    meas.fermionic = (f_ops > 0);
                    measurements.push_back(meas);
                }
            }
        }

        /// Load MPS
        MPS<matrix, grp> mps;
        load(parms["chkpfile"].str(), mps);

        /// compute left / right boundaries
        /// note: this is the current bottleneck. this part has to run sequentially!
        std::vector<Boundary<matrix,grp> > right(L+1, Boundary<matrix,grp>(Index<grp>(), Index<grp>(), 1));
        std::vector<Boundary<matrix,grp> > left (L+1, Boundary<matrix,grp>(Index<grp>(), Index<grp>(), 1));

        {
            right[L] = make_right_boundary(mps, mps);
            for (int p=L-1; p>=static_cast<int>(0); --p)
                right[p][0] = contraction::Engine<matrix, matrix, grp>::overlap_right_step(mps[p], MPSTensor<matrix, grp>(mps[p]), right[p+1][0]);
        }

        {
            MPOTensor<matrix, grp> mpo_ident(1,1);
            left[0] = make_left_boundary(mps, mps);
            for (int p=0; p<L; ++p) {
                mpo_ident.set(0,0, identity_matrix<op_t>(mps[p].site_dim()) );
                left[p+1] = contraction::Engine<matrix, matrix, grp>::overlap_mpo_left_step(mps[p], mps[p], left[p], mpo_ident);
            }
        }
        /// Compute all measurements
        for (std::vector<corr_measurement>::const_iterator it = measurements.begin();
             it != measurements.end(); ++it) {

            maquis::cout << "Measure " << it->name << "." << std::endl;

            std::vector<value_type> values;
            std::vector<std::string> labels;
            measure_correlation(mps, mps, lattice, (it->fermionic) ? fillings : identities, *it, left, right, values, labels);


            storage::archive ar(parms["resultfile"].str(), "w");
            ar["/spectrum/results/" + it->name + "/labels"    ] << labels;
            ar["/spectrum/results/" + it->name + "/mean/value"] << std::vector<std::vector<value_type> >(1, values);
        }


    } catch (std::exception & e) {
        maquis::cerr << "Exception caught:" << std::endl << e.what() << std::endl;
        throw;
    }
}

