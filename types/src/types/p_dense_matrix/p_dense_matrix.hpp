#include "types/p_dense_matrix/algorithms.hpp"

namespace maquis { namespace types {

    #define size_type   typename p_dense_matrix_impl<T>::size_type
    #define value_type  typename p_dense_matrix_impl<T>::value_type
    #define scalar_type typename p_dense_matrix_impl<T>::scalar_type

    template <typename T>
    p_dense_matrix_impl<T>::~p_dense_matrix_impl(){ } // #destructor // 

    template <typename T>
    p_dense_matrix_impl<T>::p_dense_matrix_impl(){ } // be cautious (implicit)

    template <typename T>
    p_dense_matrix_impl<T>::p_dense_matrix_impl(size_type rows, size_type cols = 0, T init_value = T() ){
        this->cols = cols;
        this->rows = rows;
        this->pt_set_dim(cols, rows);
        this->fill_value(init_value);
    }

    template <typename T>
    p_dense_matrix_impl<T>::p_dense_matrix_impl(const p_dense_matrix_impl& m)
    : ambient::parallel_t< p_dense_matrix_impl<T> >(m)
    {
        this->cols = m.num_cols();
        this->rows = m.num_rows();
    }

    template <typename T>
    inline bool p_dense_matrix_impl<T>::empty() const {
        return (this->rows == 0 || this->cols == 0); 
    }

    template <typename T>
    inline size_type p_dense_matrix_impl<T>::num_rows() const {
        return this->rows;   
    }

    template <typename T>
    inline size_type p_dense_matrix_impl<T>::num_cols() const {
        return this->cols;   
    }

    template <typename T>
    void p_dense_matrix_impl<T>::resize(size_type rows, size_type cols){
        assert(rows > 0); assert(cols > 0);
        if(this->rows != rows || this->cols != cols){
            ambient::playout(); 
            algorithms::resize(*this, rows, cols);
            this->pt_set_dim(cols, rows);
            this->rows = rows; 
            this->cols = cols;
            ambient::playout(); 
        }
    }

    template <typename T>
    void p_dense_matrix_impl<T>::remove_rows(size_type i, size_type k = 1){
        assert( i+k <= this->rows );
        algorithms::remove_rows(*this, i, k);
        this->pt_set_dim(this->cols, this->rows - k);
        this->rows -= k;
    }

    template <typename T>
    void p_dense_matrix_impl<T>::remove_cols(size_type j, size_type k = 1){
        assert( j+k <= this->cols );
        algorithms::remove_cols(*this, j, k);
        this->pt_set_dim(this->cols - k, this->rows);
        this->cols -= k;
    }

    template <typename T>
    void p_dense_matrix_impl<T>::fill_identity(){ 
        algorithms::fill_identity(*this);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::fill_random(){ 
        algorithms::fill_random(*this);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::fill_value(value_type v){ 
        algorithms::fill_value(*this, v);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::conjugate(){ 
        algorithms::inplace_conjugate(*this);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::transpose(){ 
        algorithms::inplace_transpose(*this);
    }

    template <typename T>
    value_type& p_dense_matrix_impl<T>::get(size_type i, size_type j){
        static TimerPTH time("ambient_serial_element_access"); time.begin();
        assert(i < this->rows); assert(j < this->cols);
        value_type& ref = this->pt_fetch(i / this->pt_mem_dim().y,  // blocked_i
                              j / this->pt_mem_dim().x,  // blocked_j 
                              i % this->pt_mem_dim().y,  // element_i 
                              j % this->pt_mem_dim().x); // element_j
        time.end();
        return ref;
    }

    template <typename T>
    scalar_type p_dense_matrix_impl<T>::trace() const {
        return algorithms::trace(*this);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::add(const p_dense_matrix_impl& rhs){
        algorithms::add_inplace(*this, rhs);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::sub(const p_dense_matrix_impl& rhs){
        algorithms::sub_inplace(*this, rhs);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::mul(const p_diagonal_matrix<T>& rhs){
        algorithms::gemm_diag_inplace(*this, *rhs.get_data().impl);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::mul(const p_dense_matrix_impl<T>& rhs){
        algorithms::gemm_inplace(*this, rhs);
    }

    template <typename T>
    template <typename T2>
    void p_dense_matrix_impl<T>::mul(const T2& t){
        algorithms::scale_inplace(*this, t);
    }

    template <typename T>
    void p_dense_matrix_impl<T>::cpy(const p_dense_matrix_impl<T>& rhs){
        this->resize(rhs.num_rows(), rhs.num_cols());
        algorithms::cpy(*this, rhs);
    }

    #undef size_type
    #undef value_type
    #undef scalar_type

} } // namespace maquis::types

#include "types/p_dense_matrix/p_diagonal_matrix.h" // redunant
