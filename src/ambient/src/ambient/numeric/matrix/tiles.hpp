#ifndef __AMBIENT_NUMERIC_MATRIX_HPP__
#define __AMBIENT_NUMERIC_MATRIX_HPP__

#include "ambient/numeric/matrix/tiles.h"
#include "ambient/numeric/matrix/tiles_algorithms.hpp"

#define size_type   typename tiles<Matrix>::size_type
#define value_type  typename tiles<Matrix>::value_type
#define scalar_type typename tiles<Matrix>::scalar_type

#define TILE_SIZE 256

namespace ambient { namespace numeric {

    template<class Matrix>
    inline void* tiles<Matrix>::operator new (size_t size){
        return ambient::static_memory::malloc<tiles<Matrix> >();
    }

    template<class Matrix>
    inline void* tiles<Matrix>::operator new (size_t size, void* placement){
        return placement; 
    }

    template<class Matrix>
    inline void tiles<Matrix>::operator delete (void* ptr){
        ambient::static_memory::free<tiles<Matrix> >(ptr); 
    }

    template<class Matrix>
    inline void tiles<Matrix>::operator delete (void*, void*){
        // in case of new placement fails // to implement if needed
    }

    template <class Matrix>
    inline tiles<Matrix> tiles<Matrix>::identity_matrix(size_type size){
        tiles t(size, size);
        int nb = __a_ceil(size/TILE_SIZE);
        int tailn = size % TILE_SIZE;
        for(int i = 0; i < nb-1; i++)
            t.data[i*nb + i] = Matrix::identity_matrix(TILE_SIZE);
        t.data[nb*nb-1] = Matrx::identity_matrix(tailn);
        return t;
    }

    template <class Matrix>
    inline tiles<Matrix>::tiles()
    : rows(0), cols(0) { 
        this->data.push_back(new Matrix());
    }

    template <class Matrix>
    inline tiles<Matrix>::tiles(size_type rows, size_type cols, value_type init_value)
    : rows(rows), cols(cols)
    {
        int nb = __a_ceil(cols/TILE_SIZE);
        int mb = __a_ceil(rows/TILE_SIZE);
        int tailn = cols % TILE_SIZE;
        int tailm = rows % TILE_SIZE;
        if(tailn == 0) tailn = TILE_SIZE;
        if(tailm == 0) tailm = TILE_SIZE;

        this->data.reserve(mb*nb);

        for(int j = 1; j < nb; j++){
            for(int i = 1; i < mb; i++) 
                this->data.push_back(new Matrix(TILE_SIZE, TILE_SIZE));
            this->data.push_back(new Matrix(tailm, TILE_SIZE));
        }
        for(int i = 1; i < mb; i++) 
            this->data.push_back(new Matrix(TILE_SIZE, tailn));
        this->data.push_back(new Matrix(tailm, tailn));
    }

    template <class Matrix>
    inline tiles<Matrix>::tiles(const tiles& m)
    : tiles(m.num_rows(), m.num_cols()){
        int nb = __a_ceil(cols/TILE_SIZE) * __a_ceil(rows/TILE_SIZE);
        for(int k = 0; k < nb; k++) this->data[k] = m.data[k];
    }
    
    template <class Matrix>
    tiles<Matrix>& tiles<Matrix>::operator = (const tiles& rhs){
        matrix c(rhs);
        this->swap(c);
        return *this;
    }

    template<class Matrix>
    inline size_type tiles<Matrix>::num_rows() const { 
        return this->rows;
    }

    template<class Matrix>
    inline size_type tiles<Matrix>::num_cols() const {
        return this->cols;
    }

    template<class Matrix>
    inline scalar_type tiles<Matrix>::trace() const { 
        return trace(*this);           
    }

    template<class Matrix>
    inline void tiles<Matrix>::transpose(){ 
        transpose_inplace(*this);      
    }

    template<class Matrix>
    inline void tiles<Matrix>::conj(){ 
        conj_inplace(*this);           
    }

    template<class Matrix>
    inline bool tiles<Matrix>::empty() const { 
        return (this->rows == 0); 
    }

    template<class Matrix>
    inline void tiles<Matrix>::swap(tiles& r){ 
        std::swap(r.data, this->data);
        std::swap(r.rows, this->rows);
        std::swap(r.cols, this->cols);
    }

    template<class Matrix>
    inline void tiles<Matrix>::resize(size_type rows, size_type cols){
        ambient::numeric::resize(*this, rows, cols);
    }

    template<class Matrix>
    inline void tiles<Matrix>::remove_rows(size_type i, size_type k){
        remove_rows(*this, i, k);
    }

    template<class Matrix>
    inline void tiles<Matrix>::remove_cols(size_type j, size_type k){
        remove_cols(*this, j, k); 
    }

    template<class Matrix>
    inline tiles<Matrix>& tiles<Matrix>::operator += (const tiles& rhs){
        add_inplace(*this, rhs);
        return *this;
    }

    template<class Matrix>
    inline tiles<Matrix>& tiles<Matrix>::operator -= (const tiles& rhs){
        sub_inplace(*this, rhs);
        return *this;
    }

    template<class Matrix>
    template <typename T2> 
    inline tiles<Matrix>& tiles<Matrix>::operator *= (const T2& t){
        mul_inplace(*this, t);
        return *this;
    }

    template<class Matrix>
    template <typename T2> 
    inline tiles<Matrix>& tiles<Matrix>::operator /= (const T2& t){
        div_inplace(*this, t);
        return *this;
    }

    template<class Matrix>
    inline value_type& tiles<Matrix>::operator() (size_type i, size_type j){
        int mb = __a_ceil(rows/TILE_SIZE);
        return this->data[mb*(int)j/TILE_SIZE + (int)i/TILE_SIZE](i % TILE_SIZE, j % TILE_SIZE);
    }

    template<class Matrix>
    inline const value_type& tiles<Matrix>::operator() (size_type i, size_type j) const {
        int mb = __a_ceil(rows/TILE_SIZE);
        return this->data[mb*(int)j/TILE_SIZE + (int)i/TILE_SIZE](i % TILE_SIZE, j % TILE_SIZE);
    }

} }

#undef size_type
#undef value_type
#undef scalar_type
#endif
