#ifndef __ALPS_DENSE_MATRIX_BLAS_HPP__
#define __ALPS_DENSE_MATRIX_BLAS_HPP__

#include "types/dense_matrix/detail/blasmacros.h"

#include <utils/timings.h>
    
namespace maquis {
    namespace types {
        template <typename T, typename MemoryBlock>
            class dense_matrix;
    }
}    
    //
    // dense matrix blas function hooks
    //
namespace maquis {
    namespace types {
    #define MATRIX_MATRIX_MULTIPLY(T) \
        template <typename MemoryBlock> \
        const dense_matrix<T,MemoryBlock> matrix_matrix_multiply(dense_matrix<T,MemoryBlock> const& lhs, dense_matrix<T,MemoryBlock> const& rhs) \
        { \
            assert( !(lhs.num_cols() > rhs.num_rows()) ); \
            assert( !(lhs.num_cols() < rhs.num_rows()) ); \
            assert( lhs.num_cols() == rhs.num_rows() ); \
            DCOLLECTOR_ADD(gemm_collector, lhs.num_cols()) \
            dense_matrix<T,MemoryBlock> result(lhs.num_rows(),rhs.num_cols()); \
            boost::numeric::bindings::blas::gemm \
                ( \
                   typename dense_matrix<T,MemoryBlock>::value_type(1), \
                   lhs, \
                   rhs, \
                   typename dense_matrix<T,MemoryBlock>::value_type(0), \
                   result \
                ); \
            return result; \
        }
    IMPLEMENT_FOR_ALL_BLAS_TYPES(MATRIX_MATRIX_MULTIPLY)
    #undef MATRIX_MATRIX_MULTIPLY
    
    #define MATRIX_VECTOR_MULTIPLY(T) \
        template <typename MemoryBlock, typename MemoryBlock2> \
        const vector<T,MemoryBlock2> matrix_vector_multiply(dense_matrix<T,MemoryBlock> const& m, vector<T,MemoryBlock2> const& v) \
        { \
            assert( m.num_cols() == v.size()); \
            vector<T,MemoryBlock2> result(m.num_rows()); \
            boost::numeric::bindings::blas::gemv \
                ( \
                  typename dense_matrix<T,MemoryBlock>::value_type(1), \
                  m, \
                  v, \
                  typename dense_matrix<T,MemoryBlock>::value_type(0), \
                  result \
                ); \
            return result; \
        }
    IMPLEMENT_FOR_ALL_BLAS_TYPES(MATRIX_VECTOR_MULTIPLY)
    #undef MATRIX_VECTOR_MULTIPLY
    
    // This seems to be the best solution for the *_ASSIGN dispatchers at the moment even though they call functions within the detail namespace
    #define PLUS_MINUS_ASSIGN(T) \
        template <typename MemoryBlock> \
        void plus_and_minus_assign_impl(dense_matrix<T,MemoryBlock>& m, dense_matrix<T,MemoryBlock> const& rhs, typename dense_matrix<T,MemoryBlock>::value_type const& sign) \
        { \
            assert( m.num_cols() == rhs.num_cols() && m.num_rows() == rhs.num_rows() ); \
            if(!(m.is_shrinkable() || rhs.is_shrinkable()) ) \
            { \
                boost::numeric::bindings::blas::detail::axpy( m.num_rows() * m.num_cols(), sign, &(*rhs.column(0).first), 1, &(*m.column(0).first), 1); \
            } \
            else \
            { \
                for(std::size_t j=0; j < m.num_cols(); ++j) \
                    boost::numeric::bindings::blas::detail::axpy( m.num_rows(), sign, &(*rhs.column(j).first), 1, &(*m.column(j).first), 1); \
            } \
        } \
        template <typename MemoryBlock> \
        void plus_assign(dense_matrix<T,MemoryBlock>& m, dense_matrix<T,MemoryBlock> const& rhs) \
            { plus_and_minus_assign_impl(m, rhs, typename dense_matrix<T,MemoryBlock>::value_type(1)); } \
        template <typename MemoryBlock> \
        void minus_assign(dense_matrix<T,MemoryBlock>& m, dense_matrix<T,MemoryBlock> const& rhs) \
            { plus_and_minus_assign_impl(m, rhs, typename dense_matrix<T,MemoryBlock>::value_type(-1)); }
    IMPLEMENT_FOR_ALL_BLAS_TYPES(PLUS_MINUS_ASSIGN)
    #undef PLUS_MINUS_ASSIGN
    
    #define MULTIPLIES_ASSIGN(T) \
        template <typename MemoryBlock> \
        void multiplies_assign(dense_matrix<T,MemoryBlock>& m, T const& t) \
        { \
            if( !(m.is_shrinkable()) ) \
            { \
                boost::numeric::bindings::blas::detail::scal( m.num_rows()*m.num_cols(), t, &(*m.column(0).first), 1 ); \
            } \
            else \
            { \
                for(std::size_t j=0; j <m.num_cols(); ++j) \
                    boost::numeric::bindings::blas::detail::scal( m.num_rows(), t, &(*m.column(j).first), 1 ); \
            } \
        }
        IMPLEMENT_FOR_ALL_BLAS_TYPES(MULTIPLIES_ASSIGN)
    #undef MULTIPLIES_ASSIGN
    
    } // namespace types
}//namespace maquis  
    #endif // __ALPS_DENSE_MATRIX_BLAS_HPP__
