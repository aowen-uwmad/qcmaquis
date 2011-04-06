#include "p_dense_matrix/p_dense_matrix.h"
#define STRONG_BARRIER MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout); \
                       MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout); \
                       MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout); \
                       MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout); \
                       MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout);MPI_Barrier(MPI_COMM_WORLD);fflush(stdout); 


namespace blas {

    template <typename T>
    p_dense_matrix<T>::p_dense_matrix(size_type rows = 0, size_type cols = 0, T init_value = T() )
    : rows(rows), cols(cols), lda(rows), sda(cols)
    {
        profile = new ambient::void_pt(this);
    }

    template <typename T>
    p_dense_matrix<T>::p_dense_matrix(p_dense_matrix<T> const& m)
    : rows(m.rows), cols(m.cols), lda(m.lda), sda(m.sda)
    {
        profile = new ambient::void_pt(this);
        ambient::push(ambient::copy_l_kernel, ambient::copy_c_kernel, *this, m);
    }

    template <typename T>
    void p_dense_matrix<T>::swap(p_dense_matrix & r)
    {
        std::swap(this->profile, r.profile);
        std::swap(this->rows,    r.rows);
        std::swap(this->cols,    r.cols);
        std::swap(this->lda,     r.lda);
        std::swap(this->sda,     r.sda);
    }

#ifdef HAVE_ALPS_HDF5
    template <typename T>
    inline void p_dense_matrix<T>::serialize(alps::hdf5::iarchive & ar)
    {
        ar >> alps::make_pvp("rows",   rows);
        ar >> alps::make_pvp("cols",   cols);
        ar >> alps::make_pvp("lda",    lda);
        //ar >> alps::make_pvp("values", data); ???
    }
    template <typename T>
    inline void p_dense_matrix<T>::serialize(alps::hdf5::oarchive & ar) const
    {
        ar << alps::make_pvp("rows", rows);
        ar << alps::make_pvp("cols", cols);
        ar << alps::make_pvp("lda",  lda);
        //ar << alps::make_pvp("values", data); ???
    }
#endif	

    template <typename T>
    inline bool p_dense_matrix<T>::empty() const { return (this->rows == 0 || this->cols == 0); }

    template <typename T>
    inline size_t p_dense_matrix<T>::num_rows() const { return this->rows; }

    template <typename T>
    inline size_t p_dense_matrix<T>::num_cols() const { return this->cols; }

    template <typename T>
    inline ptrdiff_t p_dense_matrix<T>::stride1() const { return 1; }

    template <typename T>
    inline ptrdiff_t p_dense_matrix<T>::stride2() const { return this->lda; }

    template <typename T>
    inline ptrdiff_t p_dense_matrix<T>::get_lda() const { return this->lda; }

    template <typename T>
    inline ptrdiff_t p_dense_matrix<T>::get_sda() const { return this->sda; }

    template <typename T>
    void p_dense_matrix<T>::clear(){ this->rows = this->cols = 0; }

    template <typename T>
    void p_dense_matrix<T>::resize(size_type rows, size_type cols)
    {
        this->rows=rows; 
        this->cols=cols;
        const size_t* rows_h = new size_t(rows); // preventing arguments from destruction
        const size_t* cols_h = new size_t(cols); // preventing arguments from destruction
        ambient::push(ambient::resize_l_kernel, ambient::resize_c_kernel, *this, *rows_h, *cols_h);
    }

    template <typename T>
    void p_dense_matrix<T>::remove_rows(size_type i, difference_type k = 1)
    {
        assert( i+k <= this->rows );
        const size_t* ih = new size_t(i); // preventing arguments from destruction
        const size_t* kh = new size_t(k); // preventing arguments from destruction
        ambient::push(ambient::remove_rows_l_kernel, ambient::remove_rows_c_kernel, *this, *ih, *kh);
        this->resize(this->rows - k, this->cols);
    }

    template <typename T>
    void p_dense_matrix<T>::remove_cols(size_type j, difference_type k = 1)
    {
        assert( j+k <= this->cols );
        const size_t *jh = new size_t(j); // preventing arguments from destruction
        const size_t *kh = new size_t(k); // preventing arguments from destruction
        ambient::push(ambient::remove_cols_l_kernel, ambient::remove_cols_c_kernel, *this, *jh, *kh);
        this->resize(this->rows, this->cols - k);
    }

    template <typename T>
    void p_dense_matrix<T>::inplace_conjugate()
    {
        assert(false);
        // todo
    }

    template <typename T>
    inline T& p_dense_matrix<T>::get(size_type i, size_type j) const
    {
        assert(i < this->rows);
        assert(j < this->cols);
        ambient::playout();
        int group_i = i / (this->profile->get_group_t_dim().y);
        int group_j = j / (this->profile->get_group_t_dim().x);
        int element_i = i % (this->profile->get_group_t_dim().y);
        int element_j = j % (this->profile->get_group_t_dim().x);
        if(this->profile->group(group_i,group_j)->available())
            return *(T*)(*this->profile)(group_i, group_j).element(element_i, element_j);
        else //return *(new T()); //using default value of T
            throw ambient::core::remote_memory_e();
    }

    template <typename T>
    inline T& p_dense_matrix<T>::operator()(size_type i, size_type j){ return this->get(i,j); }
    template <typename T>
    inline const T& p_dense_matrix<T>::operator()(size_type i, size_type j) const { return this->get(i,j); }
    template <typename T>
    p_dense_matrix<T>& p_dense_matrix<T>::operator += (const p_dense_matrix& rhs){ return(*this = *this + rhs); }
    template <typename T>
    p_dense_matrix<T>& p_dense_matrix<T>::operator -= (const p_dense_matrix& rhs){ return (*this = *this - rhs); }
    template <typename T>
    template <typename T2>
    p_dense_matrix<T>& p_dense_matrix<T>::operator *= (const T2& t){ return (*this = *this * t); }
    template <typename T>
    template <typename T2>
    p_dense_matrix<T>& p_dense_matrix<T>::operator /= (const T2& t){ return (*this = *this * (1/t)); }

    template <typename T>
    p_dense_matrix<T>::~p_dense_matrix(){ // #destructor
        this->profile->invalidate();      // tricky, possibly the auto_ptr is the best solution but too slow
    }
    template <typename T> // proxy object construction
    p_dense_matrix<T>::p_dense_matrix(ambient::void_pt* p): profile(p){ }
    template <typename T>
    p_dense_matrix<T>& p_dense_matrix<T>::operator = (p_dense_matrix<T> const& rhs){
        if(rhs.profile->is_proxy()) ambient::pin(rhs, *this); // no copying - pinning profile
        else ambient::push(ambient::copy_l_kernel, ambient::copy_c_kernel, *this, rhs);
        return *this;
    }

    template <typename T>
    const p_dense_matrix<T> operator + (const p_dense_matrix<T>& a, const p_dense_matrix<T>& b){ return ambient::push< p_dense_matrix<T> >(ambient::mem_bound_l_kernel, ambient::add_c_kernel, a, b); }
    template <typename T>
    const p_dense_matrix<T> operator - (const p_dense_matrix<T>& a, const p_dense_matrix<T>& b){ return ambient::push< p_dense_matrix<T> >(ambient::mem_bound_l_kernel, ambient::sub_c_kernel, a, b); }
    template<typename T>
    const p_dense_matrix<T> operator * (const p_dense_matrix<T>& lhs, const p_dense_matrix<T>& rhs){ return ambient::push< p_dense_matrix<T> >(ambient::gemm_l_kernel, ambient::gemm_c_kernel, lhs, rhs); }
    template<typename T, typename T2>
    const p_dense_matrix<T> operator * (const p_dense_matrix<T>& m, const T2& t){ return ambient::push< p_dense_matrix<T> >(ambient::scale_l_kernel, ambient::scale_c_kernel, m, t); }
    template<typename T, typename T2>
    const p_dense_matrix<T> operator * (const T2& t, const p_dense_matrix<T>& m){ return ambient::push< p_dense_matrix<T> >(ambient::scale_l_kernel, ambient::scale_c_kernel, m, t); }
//////////////////////////////////// AMBIENT PART ////////////////////////////////////////////////////

    template <typename T>
    std::ostream& operator << (std::ostream& o, p_dense_matrix<T> const& m)
    {
        ambient::playout();
        STRONG_BARRIER
        for(typename p_dense_matrix<T>::size_type i=0; i< m.num_rows(); ++i)
        {
            STRONG_BARRIER
            for(typename p_dense_matrix<T>::size_type j=0; j < m.num_cols(); ++j){
                STRONG_BARRIER
                try{
                    m(i,j); printf("%.2f ", m(i,j)); // faster than cout
                }catch(...){ usleep(10*1000); }      // 10 ms sleep
                STRONG_BARRIER
            }
            if(ambient::is_master()) printf("\n");
            STRONG_BARRIER
        }
        return o;
    }
}
