/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2014-2014 by Sebastian Keller <sebkelle@phys.ethz.ch>
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

#ifndef ABELIAN_ENGINE
#define ABELIAN_ENGINE

#include "dmrg/mp_tensors/mpstensor.h"
#include "dmrg/mp_tensors/mpotensor.h"
#include "dmrg/block_matrix/indexing.h"

// implementation includes see below
#include "dmrg/mp_tensors/contractions/engine.h"

namespace contraction {

template <class Matrix, class OtherMatrix, class SymmGroup>
class AbelianEngine : public Engine<Matrix, OtherMatrix, SymmGroup>
{

    std::vector<block_matrix<OtherMatrix, SymmGroup> >
    boundary_times_mps(MPSTensor<Matrix, SymmGroup> const & mps,
                       Boundary<OtherMatrix, SymmGroup> const & left,
                       MPOTensor<Matrix, SymmGroup> const & mpo);

    std::vector<block_matrix<OtherMatrix, SymmGroup> >
    mps_times_boundary(MPSTensor<Matrix, SymmGroup> const & mps,
                       Boundary<OtherMatrix, SymmGroup> const & right,
                       MPOTensor<Matrix, SymmGroup> const & mpo);
public:
    AbelianEngine() {}

    virtual block_matrix<OtherMatrix, SymmGroup>
    overlap_left_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                      MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                      block_matrix<OtherMatrix, SymmGroup> const & left,
                      block_matrix<OtherMatrix, SymmGroup> * localop = NULL);


    virtual block_matrix<OtherMatrix, SymmGroup>
    overlap_right_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                       MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                       block_matrix<OtherMatrix, SymmGroup> const & right,
                       block_matrix<OtherMatrix, SymmGroup> * localop = NULL);

    virtual Boundary<Matrix, SymmGroup>
    left_boundary_tensor_mpo(MPSTensor<Matrix, SymmGroup> mps,
                             Boundary<OtherMatrix, SymmGroup> const & left,
                             MPOTensor<Matrix, SymmGroup> const & mpo,
                             Index<SymmGroup> const * in_low = NULL);

    virtual Boundary<Matrix, SymmGroup>
    right_boundary_tensor_mpo(MPSTensor<Matrix, SymmGroup> mps,
                              Boundary<OtherMatrix, SymmGroup> const & right,
                              MPOTensor<Matrix, SymmGroup> const & mpo,
                              Index<SymmGroup> const * in_low = NULL);

    virtual Boundary<OtherMatrix, SymmGroup>
    overlap_mpo_left_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                          MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                          Boundary<OtherMatrix, SymmGroup> const & left,
                          MPOTensor<Matrix, SymmGroup> const & mpo);

    virtual Boundary<OtherMatrix, SymmGroup>
    overlap_mpo_right_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                           MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                           Boundary<OtherMatrix, SymmGroup> const & right,
                           MPOTensor<Matrix, SymmGroup> const & mpo);
};


template<class Matrix, class OtherMatrix, class SymmGroup>
std::vector<block_matrix<OtherMatrix, SymmGroup> >
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
boundary_times_mps(MPSTensor<Matrix, SymmGroup> const & mps,
                   Boundary<OtherMatrix, SymmGroup> const & left,
                   MPOTensor<Matrix, SymmGroup> const & mpo)
{
    std::vector<block_matrix<OtherMatrix, SymmGroup> > ret(left.aux_dim());
    int loop_max = left.aux_dim();
    {
        select_proc(storage::actor_t::common);
        mps.make_right_paired();
        storage::hint(mps);
    }
    parallel_for(int b1, range(0,loop_max), {
        select_proc(ambient::scope::permute(b1, mpo.placement_l));
        gemm_trim_left(transpose(left[b1]), mps.data(), ret[b1]);
    });
    return ret;
}

template<class Matrix, class OtherMatrix, class SymmGroup>
std::vector<block_matrix<OtherMatrix, SymmGroup> >
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
mps_times_boundary(MPSTensor<Matrix, SymmGroup> const & mps,
                   Boundary<OtherMatrix, SymmGroup> const & right,
                   MPOTensor<Matrix, SymmGroup> const & mpo)
{
    std::vector<block_matrix<OtherMatrix, SymmGroup> > ret(right.aux_dim());
    int loop_max = right.aux_dim();
    {
        select_proc(storage::actor_t::common);
        mps.make_left_paired();
        storage::hint(mps);
    }
    parallel_for(int b2, range(0,loop_max), {
        select_proc(ambient::scope::permute(b2, mpo.placement_r));
        gemm_trim_right(mps.data(), right[b2], ret[b2]);
    });
    return ret;
}

template<class Matrix, class OtherMatrix, class SymmGroup>
block_matrix<OtherMatrix, SymmGroup>
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
overlap_left_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                  MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                  block_matrix<OtherMatrix, SymmGroup> const & left,
                  block_matrix<OtherMatrix, SymmGroup> * localop)
{
    if (localop != NULL)
        throw std::runtime_error("Not implemented!");

    assert(ket_tensor.phys_i == bra_tensor.phys_i);

    bra_tensor.make_left_paired();

    block_matrix<OtherMatrix, SymmGroup> t1;
    block_matrix<Matrix, SymmGroup> t3;
    ket_tensor.make_right_paired();
    gemm(left, ket_tensor.data(), t1);

    reshape_right_to_left_new(ket_tensor.site_dim(), bra_tensor.row_dim(), ket_tensor.col_dim(),
                              t1, t3);
    gemm(transpose(conjugate(bra_tensor.data())), t3, t1);
    return t1;
}


template<class Matrix, class OtherMatrix, class SymmGroup>
block_matrix<OtherMatrix, SymmGroup>
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
overlap_right_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                   MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                   block_matrix<OtherMatrix, SymmGroup> const & right,
                   block_matrix<OtherMatrix, SymmGroup> * localop)
{
    if (localop != NULL)
        throw std::runtime_error("Not implemented!");

    assert(ket_tensor.phys_i == bra_tensor.phys_i);

    bra_tensor.make_right_paired();
    ket_tensor.make_left_paired();

    block_matrix<OtherMatrix, SymmGroup> t1;
    block_matrix<Matrix, SymmGroup> t3;
    gemm(ket_tensor.data(), transpose(right), t1);
    reshape_left_to_right_new(ket_tensor.site_dim(), ket_tensor.row_dim(), bra_tensor.col_dim(), t1, t3);
    gemm(conjugate(bra_tensor.data()), transpose(t3), t1);

    return t1;
}


template<class Matrix, class OtherMatrix, class SymmGroup>
Boundary<Matrix, SymmGroup>
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
left_boundary_tensor_mpo(MPSTensor<Matrix, SymmGroup> mps,
                         Boundary<OtherMatrix, SymmGroup> const & left,
                         MPOTensor<Matrix, SymmGroup> const & mpo,
                         Index<SymmGroup> const * in_low)
{
    typedef typename SymmGroup::charge charge;
    typedef typename MPOTensor<Matrix, SymmGroup>::index_type index_type;

    if (in_low == NULL)
        in_low = &mps.row_dim();

    std::vector<block_matrix<Matrix, SymmGroup> > t = boundary_times_mps(mps, left, mpo);

    Index<SymmGroup> physical_i = mps.site_dim(), left_i = *in_low, right_i = mps.col_dim(),
                                  out_left_i = physical_i * left_i;
    ProductBasis<SymmGroup> out_left_pb(physical_i, left_i);
    ProductBasis<SymmGroup> in_right_pb(physical_i, right_i,
                            boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                    -boost::lambda::_1, boost::lambda::_2));

    index_type loop_max = mpo.col_dim();

#ifdef USE_AMBIENT
    ContractionGrid<Matrix, SymmGroup> contr_grid(mpo, left.aux_dim(), mpo.col_dim());
    contr_grid.hint_left(t);

    parallel_for(index_type b2, range<index_type>(0,loop_max), {
        lbtm_kernel(b2, contr_grid, left, t, mpo, physical_i, right_i, out_left_i, in_right_pb, out_left_pb);
    });

    return contr_grid.make_boundary();
#else
    Boundary<Matrix, SymmGroup> ret;
    ret.resize(mpo.col_dim());

    omp_for(index_type b2, range<index_type>(0,loop_max), {
        ContractionGrid<Matrix, SymmGroup> contr_grid(mpo, 0, 0);
        lbtm_kernel(b2, contr_grid, left, t, mpo, physical_i, right_i, out_left_i, in_right_pb, out_left_pb);
        swap(ret[b2], contr_grid(0,0));
    });

    return ret;
#endif
}

template<class Matrix, class OtherMatrix, class SymmGroup>
Boundary<Matrix, SymmGroup>
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
right_boundary_tensor_mpo(MPSTensor<Matrix, SymmGroup> mps,
                          Boundary<OtherMatrix, SymmGroup> const & right,
                          MPOTensor<Matrix, SymmGroup> const & mpo,
                          Index<SymmGroup> const * in_low)
{
    typedef typename SymmGroup::charge charge;
    typedef typename MPOTensor<Matrix, SymmGroup>::index_type index_type;

    if (in_low == NULL)
        in_low = &mps.col_dim();

    std::vector<block_matrix<Matrix, SymmGroup> > t = mps_times_boundary(mps, right, mpo);

    Index<SymmGroup> physical_i = mps.site_dim(), left_i = mps.row_dim(), right_i = *in_low,
                     out_right_i = adjoin(physical_i) * right_i;

    ProductBasis<SymmGroup> in_left_pb(physical_i, left_i);
    ProductBasis<SymmGroup> out_right_pb(physical_i, right_i,
                                         boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                                             -boost::lambda::_1, boost::lambda::_2));
    Boundary<Matrix, SymmGroup> ret;
    ret.resize(mpo.row_dim());

    index_type loop_max = mpo.row_dim();

    omp_for(index_type b1, range<index_type>(0,loop_max), {
        select_proc(ambient::scope::permute(b1,mpo.placement_l));
        ret[b1] = rbtm_kernel(b1, right, t, mpo, physical_i, left_i, right_i, out_right_i, in_left_pb, out_right_pb);
    });

    return ret;
}

template<class Matrix, class OtherMatrix, class SymmGroup>
Boundary<OtherMatrix, SymmGroup>
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
overlap_mpo_right_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                       MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                       Boundary<OtherMatrix, SymmGroup> const & right,
                       MPOTensor<Matrix, SymmGroup> const & mpo)
{
    #ifdef AMBIENT_TRACKING
    ambient::overseer::log::region("parallel::overlap_mpo_right_step");
    #endif

    typedef typename SymmGroup::charge charge;
    typedef typename MPOTensor<Matrix, SymmGroup>::index_type index_type;

    MPSTensor<Matrix, SymmGroup> ket_cpy = ket_tensor;
    std::vector<block_matrix<Matrix, SymmGroup> > t = this->mps_times_boundary(ket_cpy, right, mpo);

    Index<SymmGroup> const & left_i = ket_tensor.row_dim();
    Index<SymmGroup> const & right_i = bra_tensor.col_dim();
    Index<SymmGroup> out_right_i = adjoin(ket_tensor.site_dim()) * right_i;
    ProductBasis<SymmGroup> in_left_pb(ket_tensor.site_dim(), left_i);
    ProductBasis<SymmGroup> out_right_pb(ket_tensor.site_dim(), right_i,
                                         boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                                             -boost::lambda::_1, boost::lambda::_2));
    Boundary<Matrix, SymmGroup> ret;
    ret.resize(mpo.row_dim());

    //ket_tensor.make_right_paired();
    index_type loop_max = mpo.row_dim();

    bra_tensor.make_right_paired();
    block_matrix<Matrix, SymmGroup> bra_conj = conjugate(bra_tensor.data());
    omp_for(index_type b1, range<index_type>(0,loop_max), {
        select_proc(ambient::scope::permute(b1,mpo.placement_l));
        block_matrix<Matrix, SymmGroup> tmp;
        tmp = rbtm_kernel(b1, right, t, mpo, ket_tensor.site_dim(), left_i, right_i, out_right_i, in_left_pb, out_right_pb);
        gemm(tmp, transpose(bra_conj), ret[b1]);
    });
    #ifdef AMBIENT_TRACKING
    ambient::overseer::log::region("serial::continue");
    #endif

    return ret;
}

template<class Matrix, class OtherMatrix, class SymmGroup>
Boundary<OtherMatrix, SymmGroup>
AbelianEngine<Matrix, OtherMatrix, SymmGroup>::
overlap_mpo_left_step(MPSTensor<Matrix, SymmGroup> const & bra_tensor,
                      MPSTensor<Matrix, SymmGroup> const & ket_tensor,
                      Boundary<OtherMatrix, SymmGroup> const & left,
                      MPOTensor<Matrix, SymmGroup> const & mpo)
{
    #ifdef AMBIENT_TRACKING
    ambient::overseer::log::region("parallel::overlap_mpo_left_step");
    #endif

    typedef typename SymmGroup::charge charge;
    typedef typename MPOTensor<Matrix, SymmGroup>::index_type index_type;

    MPSTensor<Matrix, SymmGroup> ket_cpy = ket_tensor;
    std::vector<block_matrix<Matrix, SymmGroup> > t = this->boundary_times_mps(ket_cpy, left, mpo);

    Index<SymmGroup> const & left_i = bra_tensor.row_dim();
    Index<SymmGroup> const & right_i = ket_tensor.col_dim();
    Index<SymmGroup> out_left_i = ket_tensor.site_dim() * left_i;
    ProductBasis<SymmGroup> out_left_pb(ket_tensor.site_dim(), left_i);
    ProductBasis<SymmGroup> in_right_pb(ket_tensor.site_dim(), right_i,
                            boost::lambda::bind(static_cast<charge(*)(charge, charge)>(SymmGroup::fuse),
                                    -boost::lambda::_1, boost::lambda::_2));

    index_type loop_max = mpo.col_dim();

    bra_tensor.make_left_paired();
    block_matrix<Matrix, SymmGroup> bra_conj = conjugate(bra_tensor.data());

#ifdef USE_AMBIENT
    ContractionGrid<Matrix, SymmGroup> contr_grid(mpo, left.aux_dim(), mpo.col_dim());
    contr_grid.hint_left(t);

    parallel_for(index_type b2, range<index_type>(0,loop_max), {
        lbtm_kernel(b2, contr_grid, left, t, mpo, ket_tensor.site_dim(), right_i, out_left_i, in_right_pb, out_left_pb);
    });
    for(index_type b2 = 0; b2 < loop_max; b2++){
        contr_grid.multiply_column_trans(b2, bra_conj);
    };
    #ifdef AMBIENT_TRACKING
    ambient::overseer::log::region("serial::continue");
    #endif

    return contr_grid.make_boundary();
#else
    Boundary<Matrix, SymmGroup> ret;
    ret.resize(loop_max);

    omp_for(index_type b2, range<index_type>(0,loop_max), {
        ContractionGrid<Matrix, SymmGroup> contr_grid(mpo, 0, 0);
        block_matrix<Matrix, SymmGroup> tmp;
        lbtm_kernel(b2, contr_grid, left, t, mpo, ket_tensor.site_dim(), right_i, out_left_i, in_right_pb, out_left_pb);
        gemm(transpose(contr_grid(0,0)), bra_conj, ret[b2]);
    });

        return ret;
#endif

    }

} // namespace contractions

#endif