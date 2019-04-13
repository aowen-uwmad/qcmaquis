/*****************************************************************************
*
* ALPS MPS DMRG Project
*
* Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
*               2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
*               2011-2013    Michele Dolfi <dolfim@phys.ethz.ch>
*               2014-2014    Sebastian Keller <sebkelle@phys.ethz.ch>
*               2018         Leon Freitag <lefreita@ethz.ch>
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
#ifndef DUMP_MPSTENSOR_H
#define DUMP_MPSTENSOR_H
#include "dmrg/mp_tensors/twositetensor.h"

namespace measurements {
    // A simple measurement that dumps MPS tensors
    // Used in the linear response (state-average gradients) calculation
    // if twosite == true, creates a TwoSiteTensor and dumps it
    // if twosite == false, dumps the (one-site) MPSTensor in question instead
    template <class Matrix, class SymmGroup>
    class DumpMPSTensor : public measurement<Matrix, SymmGroup> {
        typedef measurement<Matrix, SymmGroup> base;
    public:

        DumpMPSTensor(std::string name_, int site_ = 0, bool twosite_ = true) : base(name_), site(site_), twosite(twosite_) {}

        void evaluate(MPS<Matrix, SymmGroup> const& mps, boost::optional<reduced_mps<Matrix, SymmGroup> const&> rmps = boost::none)
        {
            this->vector_results.clear();
            this->labels.clear();

            MPS<Matrix, SymmGroup> mps_aux = mps;

            // The MPS needs to be canonized up to the site before the measurement to have the same local basis in all the states
            // However, we canonize all MPS simultaneously, so we don't need to canonize here again.

            // mps_aux.canonize(site);

            if (twosite)
            {
                // Prepare the two-site tensor from two sites of the MPS
                TwoSiteTensor<Matrix, SymmGroup> tst(mps_aux[site], mps_aux[site+1]);

                // To keep the consistency with other measurements (local Hamiltonian), or the yingjin-devel branch
                // which apparently works with left-paired two site tensors, we introduce left pairing
                // Note that for some reason, pairing may introduce an additional block with a zero element!
                tst.make_left_paired();

                maquis::cout << "Dumping the two-site tensor at site " << site << std::endl;
                // Dump the TST elements along with the labels

                parallel::scheduler_balanced_iterative scheduler(tst.data());

                for (int i = 0; i < tst.data().n_blocks(); i++)
                for (int j = 0; j < tst.data()[i].num_rows(); j++)
                for (int k = 0; k < tst.data()[i].num_cols(); k++)
                {
                    parallel::guard proc(scheduler(i));
                    this->vector_results.push_back(tst.data()[i](j,k));
                    // Labels are dumped as 'site, i, j, k'
                    this->labels.push_back(label_string_simple({site, i, j, k}));
                }
            }
            else
            {
                MPSTensor<Matrix, SymmGroup> & mpst = mps_aux[site];
                // One-site parameters: just dump the MPSTensor at current site
                mpst.make_left_paired();

                maquis::cout << "Dumping the one-site tensor at site " << site << std::endl;
                // Dump the MPSTensor elements along with the labels

                parallel::scheduler_balanced_iterative scheduler(mpst.data());

                for (int i = 0; i < mpst.data().n_blocks(); i++)
                for (int j = 0; j < mpst.data()[i].num_rows(); j++)
                for (int k = 0; k < mpst.data()[i].num_cols(); k++)
                {
                    parallel::guard proc(scheduler(i));
                    this->vector_results.push_back(mpst.data()[i](j,k));
                    // Labels are dumped as 'site, i, j, k'
                    this->labels.push_back(label_string_simple({site, i, j, k}));
                }
            }

        }

    protected:
        measurement<Matrix, SymmGroup>* do_clone() const
        {
            return new DumpMPSTensor(*this);
        }

    private:
        int site; // Site at which the TwoSiteTensor should be constructed/MPSTensor dumped
        bool twosite; // whether we should create a twosite tensor
    };

}
#endif