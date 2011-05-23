#ifndef __ALPS_DENSE_MATRIX_ALGORITHMS_HPP__
#define __ALPS_DENSE_MATRIX_ALGORITHMS_HPP__

#include "p_dense_matrix/concept/matrix_concept_check.hpp"

#include <boost/numeric/bindings/lapack/driver/gesdd.hpp>
#include <boost/numeric/bindings/std/vector.hpp>

#include "p_dense_matrix/p_dense_matrix.h"
#include "utils/timings.h"

namespace blas
{
    template<typename T>
    p_dense_matrix<T> transpose(const p_dense_matrix<T>& m)
    {   // TODO: perhaps this could return a proxy object
        //printf("transpose: %d %d\n", m.num_rows(), m.num_cols());
        p_dense_matrix<T> mt(num_cols(m), num_rows(m));
        ambient::push(ambient::transpose_l, ambient::transpose_c, mt, m);
        return mt;
    }

    template<typename T>
    const T trace(const p_dense_matrix<T>& m)
    {
        assert(num_rows(m) == num_cols(m));
        T* tr = (T*)calloc(1,sizeof(T));
        if(m.is_abstract()) m.touch();
        ambient::push(ambient::trace_l, ambient::trace_c, m, tr);
        ambient::playout(); // to remove in future
        return *tr;
    }
        
    template<typename T>
    p_dense_matrix<T> conjugate(p_dense_matrix<T> M)
    {
        assert(false);
        M.inplace_conjugate();
        return M;
    }
        
    template<typename T>
    void pblas_gemm(const p_dense_matrix<T>& A, const p_dense_matrix<T>& B, p_dense_matrix<T>& C)
    {
        //C.resize(A.num_rows(), B.num_cols());
        C.set_init(ambient::null_i<T>);
        ambient::push(ambient::gemm_l_scalapack, ambient::gemm_c_scalapack, A, B, C);
    }

    template<typename T>
    void gemm(const p_dense_matrix<T>& A, const p_dense_matrix<T>& B, p_dense_matrix<T>& C)
    {
        //C.resize(A.num_rows(), B.num_cols());
        C.set_init(ambient::null_i<T>);
        //printf("gemm: %d %d\n", C.num_rows(), C.num_cols());
        ambient::push(ambient::gemm_l, ambient::gemm_c, A, B, C);
    }

    template<typename T>
    void gemm(const p_dense_matrix<T>& A, const p_diagonal_matrix<T>& B, p_dense_matrix<T>& C)
    {
        assert(num_cols(A) == num_rows(B));
        C.resize(A.num_rows(), B.num_cols());
        //printf("gemm: %d %d\n", C.num_rows(), C.num_cols());
        ambient::push(ambient::gemm_diagonal_rhs_l, ambient::gemm_diagonal_rhs_c, A, B.get_data(), C);
    }
    
    template<typename T>
    void gemm(const p_diagonal_matrix<T>& A, const p_dense_matrix<T>& B, p_dense_matrix<T>& C)
    {
        assert(num_cols(A) == num_rows(B));
        C.resize(A.num_rows(), B.num_cols());
        //printf("gemm: %d %d\n", C.num_rows(), C.num_cols());
        ambient::push(ambient::gemm_diagonal_lhs_l,ambient::gemm_diagonal_lhs_c, A.get_data(), B, C);
    }
    
    template<typename T>
    void svd(const p_dense_matrix<T>& A,
                   p_dense_matrix<T>& U,
                   p_dense_matrix<T>& V,
             typename associated_diagonal_matrix<p_dense_matrix<T> >::type& S)
    {
        BOOST_CONCEPT_ASSERT((blas::Matrix<p_dense_matrix<T> >));
        typename p_dense_matrix<T>::size_type k = std::min(num_rows(A), num_cols(A));
        U.resize(num_rows(A), k);
        V.resize(k, num_cols(A));
        S.resize(k, k);
        //printf("svd: %d %d; %d %d; %d %d\n", A.num_rows(), A.num_cols(), U.num_rows(), U.num_cols(), V.num_rows(), V.num_cols());
        ambient::push(ambient::svd_l_scalapack, ambient::svd_c_scalapack, A, U, V, S.get_data());
    }
    
    template<typename T>
    void qr(p_dense_matrix<T> M,
            p_dense_matrix<T> & Q,
            p_dense_matrix<T> & R)
    {
        assert(false);
        /* implement thin QR decomposition, i.e. for a (m,n) matrix, where m >= n, the result should be
         Q: (m,n)
         R: (n,n) */
    }

    template<class T>
    void reverse(typename associated_diagonal_matrix<T>::type & S)
    { // reverse only the first col
        size_t num_rows = S.num_rows();
        ambient::push(ambient::associated_reverse_l, ambient::associated_reverse_c, S.get_data(), num_rows);
    }

    template<typename T>
    void syev(p_dense_matrix<T> M,
              p_dense_matrix<T>& evecs,
              typename associated_diagonal_matrix< p_dense_matrix<T> >::type& evals)
    {
        assert(num_rows(M) == num_cols(M));
        assert(num_rows(evals) == num_rows(M));

        evecs.resize(num_rows(M), num_cols(M));
        ambient::push(ambient::syev_l_scalapack, ambient::syev_c_scalapack, M, evals.get_data(), evecs); // destoys U triangle of M
        reverse< p_dense_matrix<T> >(evals);          
    }
 
    template<typename T>
    void validation(const p_dense_matrix<T>& a, const p_dense_matrix<T>& b)
    {
        ambient::push(ambient::validation_l, ambient::validation_c, a, b);
    }
  
    template<typename T>
    void associated_validation(const p_dense_matrix<T>& a, const p_dense_matrix<T>& b )
    {
        ambient::push(ambient::associated_validation_l, ambient::associated_validation_c, a, b);
    }
 
    template<typename T, class Generator>
    void generate(p_dense_matrix<T>& A, Generator g)
    {
        A.set_init(ambient::random_i<T>);
    }
 
    template<typename T>
    void copy(typename associated_vector<T>::type& SC, typename associated_diagonal_matrix<T>::type& S)
    { // this kernel copies only the first cols of the work group, only used with associated_diagonal_matrix and associated_vector 
        ambient::push(ambient::associated_copy_l, ambient::copy_c, SC.get_data(), S.get_data());
    }
 
    template<typename T>
    void copy_after(typename associated_vector<T>::type& SC, const size_t pos, typename associated_diagonal_matrix<T>::type& S)
    { // this kernel copies only the first cols of the work group, only used with associated_diagonal_matrix and associated_vector 
        ambient::push(ambient::copy_after_l, ambient::copy_after_c, SC.get_data(), pos, S.get_data());
    }
    
    template<class T>
    void sort(typename associated_vector<T>::type& S)
    {
        ambient::playout(); // because of bugbug place...
        ambient::push(ambient::associated_sort_l, ambient::associated_sort_c, S.get_data());
        size_t grid_dim_y = get_grid_dim(S.get_data()).y; // bugbug
        size_t num = __a_ceil(grid_dim_y/2);
        for(size_t i=0; i < num ; i++){
            ambient::push(ambient::associated_oe_sort_l, ambient::associated_sort_o_c,S.get_data());
            ambient::push(ambient::associated_oe_sort_l, ambient::associated_sort_e_c,S.get_data());
        }
        ambient::push(ambient::associated_oe_sort_l, ambient::move_offset_c,S.get_data());
    }

    template<class T>
    void find_if(typename associated_vector<T>::type& S, const double& value, size_t* out_value)
    {
        ambient::push(ambient::associated_find_if_l, ambient::associated_find_if_c, S.get_data(), value, out_value);
    }

    template<class T>
    void accumulate(typename associated_vector<T>::type& S, const size_t* begin, double* out_value)
    {
        ambient::push(ambient::associated_accumulate_l, ambient::associated_accumulate_c, S.get_data(), begin, out_value);
    }

    template<class T>
    void max(typename associated_vector<T>::type& S, const double& evalscut, const size_t& Mmax, double* out_value) 
    {
        ambient::push(ambient::associated_max_l, ambient::associated_max_c, S.get_data(), evalscut, Mmax, out_value); 
    }
 
    void variable_free(void* a)
    { 
        ambient::push(ambient::variable_free_l, ambient::variable_free_c, a);
    }
 
} /* namespace blas */

#endif
