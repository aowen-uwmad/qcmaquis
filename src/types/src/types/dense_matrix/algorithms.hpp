#ifndef __ALPS_DENSE_MATRIX_ALGO_ALGORITHMS_HPP__
#define __ALPS_DENSE_MATRIX_ALGO_ALGORITHMS_HPP__
#include <vector>
#include <stdexcept>
#include "utils/data_collector.hpp"
#include "types/dense_matrix/matrix_concept_check.hpp"
#include "types/dense_matrix/diagonal_matrix.h"
#include "utils/function_objects.h"
#include "types/dense_matrix/matrix_algorithms.hpp"


#include <boost/numeric/bindings/lapack/driver/gesvd.hpp>
#include <boost/numeric/bindings/lapack/driver/gesdd.hpp>
#include <boost/numeric/bindings/lapack/driver/syevd.hpp>
#include <boost/numeric/bindings/lapack/driver/heevd.hpp>
#include <boost/numeric/bindings/lapack/computational/geqrf.hpp>
#include <boost/numeric/bindings/std/vector.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <utils/timings.h>

#include <alps/numeric/real.hpp>
#include <alps/numeric/imag.hpp>

// forward declaration for nested specialization, be cautious of the namespace

namespace maquis {
    namespace types {
        template<class T>
        class diagonal_matrix; 
    }
}

namespace maquis {
    namespace types {

        template <typename T>
        void reshape_r2l(dense_matrix<T>& left, const dense_matrix<T>& right,
                         size_t left_offset, size_t right_offset, 
                         size_t sdim, size_t ldim, size_t rdim)
        {
             for (size_t ss = 0; ss < sdim; ++ss)
                 for (size_t rr = 0; rr < rdim; ++rr)
                    for(size_t ll = 0; ll < ldim; ++ll)
                          left(left_offset + ss*ldim+ll, rr) = 
                          right(ll, right_offset + ss*rdim+rr);
                     // memcpy(&left(left_offset + ss*ldim, rr),
                     //        &right(0, right_offset + ss*rdim+rr),
                     //        sizeof(T) * ldim);
        }
        
        template <typename T>
        void reshape_l2r(const dense_matrix<T>& left, dense_matrix<T>& right,
                         size_t left_offset, size_t right_offset, 
                         size_t sdim, size_t ldim, size_t rdim)
        {
             for (size_t ss = 0; ss < sdim; ++ss)
                 for (size_t rr = 0; rr < rdim; ++rr)
                     for (size_t ll = 0; ll < ldim; ++ll)
                         right(ll, right_offset + ss*rdim+rr) = left(left_offset + ss*ldim+ll, rr);
        }
        
        template <typename T>
        void lb_tensor_mpo(dense_matrix<T>& out, const dense_matrix<T>& in, const dense_matrix<T>& alfa,
                           size_t out_offset, size_t in_offset, 
                           size_t sdim1, size_t sdim2, size_t ldim, size_t rdim)
        {
            for(size_t ss1 = 0; ss1 < sdim1; ++ss1)
                for(size_t ss2 = 0; ss2 < sdim2; ++ss2) {
                    T alfa_t = alfa(ss1, ss2);
                    for(size_t rr = 0; rr < rdim; ++rr) {
                        iterator_axpy(&in(in_offset + ss1*ldim, rr),
                                      &in(in_offset + ss1*ldim, rr) + ldim, // bugbug
                                      &out(out_offset + ss2*ldim, rr),
                                      alfa_t);
                    }
                }
        }
        
        template <typename T>
        void rb_tensor_mpo(dense_matrix<T>& out, const dense_matrix<T>& in, const dense_matrix<T>& alfa,
                           size_t out_offset, size_t in_offset, 
                           size_t sdim1, size_t sdim2, size_t ldim, size_t rdim)
        {
            for(size_t ss1 = 0; ss1 < sdim1; ++ss1)
                for(size_t ss2 = 0; ss2 < sdim2; ++ss2) {
                    T alfa_t = alfa(ss1, ss2);
                    for(size_t rr = 0; rr < rdim; ++rr)
                        for(size_t ll = 0; ll < ldim; ++ll) {
                            out(ll, out_offset + ss2*rdim+rr) += in(ll, in_offset + ss1*rdim+rr) * alfa_t;
                        }
                }
        }
         
        template <typename T>
        void scalar_norm(const dense_matrix<T>& M, typename dense_matrix<T>::value_type& ret){
            using utils::conj; 
            for (std::size_t c = 0; c < num_cols(M); ++c)
                for (std::size_t r = 0; r < num_rows(M); ++r)
                    ret += conj(M(r,c)) * M(r,c);
        }
        
        template <typename T>
        void scalar_norm(dense_matrix<T> & M1, dense_matrix<T> & M2, typename dense_matrix<T>::value_type & ret){ // not const due to nullcut
            using utils::conj; 
            for (std::size_t c = 0; c < num_cols(M1); ++c)
                for (std::size_t r = 0; r < num_rows(M1); ++r)
                    ret += conj(M1(r,c)) * M2(r,c);
        }
        
        template <typename T>
        void bond_renyi_entropies(const diagonal_matrix<T> & M, typename associated_real_vector<dense_matrix<T> >::type& sv){
            for (typename diagonal_matrix<T>::const_element_iterator it = elements(M).first;
                 it != elements(M).second; ++it)
            {
                double a = std::abs(*it);
                if (a > 1e-10)
                    sv.push_back(a*a);
            }
        }
        
        template <typename T>
        void left_right_boundary_init(dense_matrix<T> & M){
//            memset((void*)&M(0,0),1,num_rows(M)*num_cols(M)*sizeof(T));
            for_each(elements(M).first,elements(M).second, boost::lambda::_1 = 1); // boost::lambda ^^' because iterable matrix concept 
        }

        namespace detail {
            template<typename T> struct sv_type { typedef T type; };
            template<typename T>
            struct sv_type<std::complex<T> > { typedef T type; };
        }

        template<typename T, class MemoryBlock, class DiagMatrix>
        void svd(dense_matrix<T, MemoryBlock> M,
                 dense_matrix<T, MemoryBlock> & U,
                 dense_matrix<T, MemoryBlock>& V,
                 DiagMatrix & S)
        {
            BOOST_CONCEPT_ASSERT((maquis::types::Matrix<dense_matrix<T, MemoryBlock> >));
            DCOLLECTOR_ADD(svd_collector, M.num_cols())
            static Timer timer("SVD");
            timer.begin();
            typename dense_matrix<T, MemoryBlock>::size_type k = std::min(num_rows(M), num_cols(M));
            resize(U, num_rows(M), k);
            resize(V, k, num_cols(M));
            typename associated_vector<dense_matrix<typename detail::sv_type<T>::type, MemoryBlock> >::type S_(k);
            int info = boost::numeric::bindings::lapack::gesvd('S', 'S', M, S_, U, V);
            if (info != 0)
                throw std::runtime_error("Error in SVD!");

            S = DiagMatrix(S_);
            timer.end();
        }
        
        template<typename T, class MemoryBlock>
        void qr(dense_matrix<T, MemoryBlock> M,
                dense_matrix<T, MemoryBlock> & Q,
                dense_matrix<T, MemoryBlock> & R)
        {
            typename dense_matrix<T, MemoryBlock>::size_type k = std::min(num_rows(M), num_cols(M));
    
            typename associated_vector<dense_matrix<typename detail::sv_type<T>::type, MemoryBlock> >::type tau(k);
            
            int info = 0; //boost::numeric::bindings::lapack::geqrf(M, tau);
            if (info != 0)
                throw std::runtime_error("Error in geqrf");
            
            resize(Q, num_rows(M), k);
            resize(R, k, num_cols(M));
            
            // get R
            std::fill(elements(R).first, elements(R).second, 0);
            for (std::size_t c = 0; c < num_cols(M); ++c)
                for (std::size_t r = 0; r <= c; ++r)
                    R(r, c) = M(r, c);
            
            // get Q from householder reflections in M
            std::fill(elements(Q).first, elements(Q).second, 0);
            
        }
        
        template<typename T, class MemoryBlock>
        dense_matrix<T, MemoryBlock> exp (dense_matrix<T, MemoryBlock> M, T const & alpha=1)
        {
            dense_matrix<T, MemoryBlock> N, tmp;
            typename associated_real_vector<dense_matrix<T, MemoryBlock> >::type Sv(num_rows(M));
            
            heev(M, N, Sv);
            
            typename associated_diagonal_matrix<dense_matrix<T, MemoryBlock> >::type S(Sv);
            S = exp(alpha*S);
            gemm(N, S, tmp);
            gemm(tmp, maquis::types::conjugate(transpose(N)), M);
            
            return M;
        }
  
        template<typename T, class MemoryBlock, class Generator>
        void generate(dense_matrix<T, MemoryBlock>& m, Generator g)
        {
           std::generate(elements(m).first, elements(m).second, g);
        }
    
        template<typename T, class MemoryBlock>
        void heev(dense_matrix<T, MemoryBlock> M,
                  dense_matrix<T, MemoryBlock> & evecs,
                  typename associated_real_vector<dense_matrix<T, MemoryBlock> >::type & evals) 
        {
            assert(num_rows(M) == num_cols(M));
            assert(evals.size() == num_rows(M));
#ifndef NDEBUG
            for (int i = 0; i < num_rows(M); ++i)
                for (int j = 0; j < num_cols(M); ++j)
                    assert( abs( M(i,j) - utils::conj(M(j,i)) ) < 1e-10 );
#endif
            
            boost::numeric::bindings::lapack::heevd('V', M, evals);
            // to be consistent with the SVD, I reorder in decreasing order
            std::reverse(evals.begin(), evals.end());
            // and the same with the matrix
            evecs.resize(num_rows(M), num_cols(M));
            for (std::size_t c = 0; c < num_cols(M); ++c)
            		std::copy(column(M, c).first, column(M, c).second,
                          column(evecs, num_cols(M)-1-c).first);
        }
        
        template<typename T, class MemoryBlock>
        void heev(dense_matrix<T, MemoryBlock> M,
                  dense_matrix<T, MemoryBlock> & evecs,
                  typename associated_diagonal_matrix<dense_matrix<T, MemoryBlock> >::type & evals)
        {
            assert(num_rows(M) == num_cols(M));
            typename associated_real_vector<dense_matrix<T, MemoryBlock> >::type evals_(num_rows(M));
            heev(M, evecs, evals_);
            evals = typename associated_diagonal_matrix<dense_matrix<T, MemoryBlock> >::type(evals_);
        }

        template<typename T, class MemoryBlock, class ThirdArgument>
        void syev(dense_matrix<T, MemoryBlock> M,
                  dense_matrix<T, MemoryBlock> & evecs,
                  ThirdArgument & evals)
        {
            heev(M, evecs, evals);
        }
        /*
        * Some block_matrix algorithms necessitate nested specialization due to ambient scheduler
        * the algos are full rewritten or partly with subset specialization 
        * an alternative implementation is presented inside p_dense_matrix/algorithms.hpp
        */
    } // end namspace types
} //end namespace maquis

#endif
