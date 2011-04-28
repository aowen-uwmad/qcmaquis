/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *
 *****************************************************************************/

#include "block_matrix/block_matrix.h"

#include "utils/function_objects.h"
#include <boost/lambda/bind.hpp>

template<class Matrix, class SymmGroup>
block_matrix<Matrix, SymmGroup>::block_matrix(Index<SymmGroup> rows,
                                              Index<SymmGroup> cols)
: rows_(rows)
, cols_(cols)
{
    assert(rows_.size() == cols_.size());
    for (size_type k = 0; k < rows_.size(); ++k)
        data_.push_back(Matrix(rows_[k].second, cols_[k].second));
}

template<class Matrix, class SymmGroup>
block_matrix<Matrix, SymmGroup> & block_matrix<Matrix, SymmGroup>::operator+=(block_matrix const & rhs)
{
    for (size_type k = 0; k < rhs.n_blocks(); ++k)
    {
        charge rhs_rc = rhs.rows_[k].first;
        charge rhs_cc = rhs.cols_[k].first;
        if (this->has_block(rhs_rc, rhs_cc))
            (*this)(rhs_rc, rhs_cc) += rhs.data_[k];
        else
            this->insert_block(rhs.data_[k], rhs_rc, rhs_cc);
    }
    return *this;
}

template<class Matrix, class SymmGroup>
block_matrix<Matrix, SymmGroup> & block_matrix<Matrix, SymmGroup>::operator-=(block_matrix const & rhs)
{
    for (size_type k = 0; k < rhs.n_blocks(); ++k)
    {
        charge rhs_rc = rhs.rows_[k].first;
        charge rhs_cc = rhs.cols_[k].first;
        if (this->has_block(rhs_rc, rhs_cc))
            (*this)(rhs_rc, rhs_cc) -= rhs.data_[k];
        else
            this->insert_block(-1*rhs.data_[k], rhs_rc, rhs_cc);
    }
    return *this;
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::insert_block(Matrix const & mtx, charge c1, charge c2)
{
    assert( !has_block(c1, c2) );
    
    std::pair<charge, size_type>
    p1 = std::make_pair(c1, mtx.num_rows()),
    p2 = std::make_pair(c2, mtx.num_cols());
    
    size_type i1 = rows_.insert(p1);
    cols_.insert(i1, p2);
    data_.insert(data_.begin() + i1, mtx);
    
    //rows_.push_back(p1);
    //cols_.push_back(p2);
    //data_.push_back(mtx);
}

template<class Matrix, class SymmGroup>
block_matrix<Matrix, SymmGroup> & block_matrix<Matrix, SymmGroup>::operator=(block_matrix rhs)
{
    swap(*this, rhs);
    return *this;
}

template<class Matrix, class SymmGroup>
Index<SymmGroup> const & block_matrix<Matrix, SymmGroup>::left_basis() const { return rows_; }

template<class Matrix, class SymmGroup>
Index<SymmGroup> const & block_matrix<Matrix, SymmGroup>::right_basis() const { return cols_; }

template<class Matrix, class SymmGroup>
typename Matrix::size_type block_matrix<Matrix, SymmGroup>::n_blocks() const { return data_.size(); }

template<class Matrix, class SymmGroup>
std::string block_matrix<Matrix, SymmGroup>::description() const
{
    std::ostringstream oss;
    oss << rows_ << cols_;
    return oss.str();
}

template<class Matrix, class SymmGroup>
Matrix & block_matrix<Matrix, SymmGroup>::operator[](size_type c) { return data_[c]; }

template<class Matrix, class SymmGroup>
Matrix const & block_matrix<Matrix, SymmGroup>::operator[](size_type c) const { return data_[c]; }

template<class Matrix, class SymmGroup>
bool block_matrix<Matrix, SymmGroup>::has_block(charge r, charge c) const
{
    bool ret;
    static Timer hb("has_block");
    hb.begin();
    std::size_t p1 = rows_.position(r);
    if (p1 == rows_.size())
        ret = false;
    else {
        std::size_t p2 = cols_.position(c);
        if (p2 == cols_.size())
            ret = false;
        else
            ret = (p1 == p2);
    }
    hb.end();
    return ret;
}

template<class Matrix, class SymmGroup>
bool block_matrix<Matrix, SymmGroup>::has_block(std::pair<charge, size_type> const & r,
                                                std::pair<charge, size_type> const & c) const
{
    return has_block(r.first, c.first);
}

template<class Matrix, class SymmGroup>
typename Matrix::value_type & block_matrix<Matrix, SymmGroup>::operator()(std::pair<charge, size_type> const & r,
                                                                          std::pair<charge, size_type> const & c)
{
    assert( rows_.position(r.first) == cols_.position(c.first) );
    return data_[rows_.position(r.first)](r.second, c.second);
}

template<class Matrix, class SymmGroup>
typename Matrix::value_type const & block_matrix<Matrix, SymmGroup>::operator()(std::pair<charge, size_type> const & r,
                                                                                std::pair<charge, size_type> const & c) const
{
    assert( rows_.position(r.first) == cols_.position(c.first) );
    return data_[rows_.position(r.first)](r.second, c.second);
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::remove_rows_from_block(size_type block, size_type r, size_type k)
{
    remove_rows(data_[block], r, k);
    rows_[block].second -= k;
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::remove_cols_from_block(size_type block, size_type r, size_type k)
{
    remove_columns(data_[block], r, k);
    cols_[block].second -= k;
}

template<class Matrix, class SymmGroup>
block_matrix<Matrix, SymmGroup> const & block_matrix<Matrix, SymmGroup>::operator*=(value_type v)
{
    std::for_each(data_.begin(), data_.end(), boost::lambda::_1 *= v);
    return *this;
}

template<class Matrix, class SymmGroup>
block_matrix<Matrix, SymmGroup> const & block_matrix<Matrix, SymmGroup>::operator/=(value_type v)
{
    std::for_each(data_.begin(), data_.end(), boost::lambda::_1 /= v);
    return *this;
}


template<class Matrix, class SymmGroup>
typename Matrix::value_type block_matrix<Matrix, SymmGroup>::trace() const
{
    std::vector<value_type> vt(n_blocks());
    std::transform(data_.begin(), data_.end(), vt.begin(), utils::functor_trace());
    return std::accumulate(vt.begin(), vt.end(), value_type());
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::inplace_transpose()
{
    std::transform(data_.begin(), data_.end(), data_.begin(), utils::functor_transpose());
    std::swap(rows_, cols_);
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::inplace_conjugate()
{
    std::transform(data_.begin(), data_.end(), data_.begin(), utils::functor_conjugate());
}

template<class Matrix, class SymmGroup>
template<class Generator>
void block_matrix<Matrix, SymmGroup>::generate(Generator g)
{
    for(std::size_t k = 0; k < n_blocks(); ++k) blas::generate(data_[k], g);
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::clear()
{
    data_.clear();
    rows_ = Index<SymmGroup>();
    cols_ = Index<SymmGroup>();
}

template<class Matrix, class SymmGroup>
std::ostream& operator<<(std::ostream& os, block_matrix<Matrix, SymmGroup> const & m)
{
    os << "Left HS: " << m.left_basis() << std::endl;
    os << "Right HS: " << m.right_basis() << std::endl;
    for (std::size_t k = 0; k < m.n_blocks(); ++k)
        os << "Block (" << m.left_basis()[k].first << "," << m.right_basis()[k].first << "):" << std::endl << m[k];
    os << std::endl;
    return os;
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::match_and_add_block(Matrix const & mtx, charge c1, charge c2)
{
    
    static Timer timer("match_and_add_block");
    timer.begin();
    if (this->has_block(c1, c2))
    {
        if (num_rows(mtx) == num_rows((*this)(c1, c2)) &&
            num_cols(mtx) == num_cols((*this)(c1, c2)))
            (*this)(c1, c2) += mtx;
        else if (num_rows(mtx) > num_rows((*this)(c1, c2)) &&
                 num_cols(mtx) > num_cols((*this)(c1, c2)))
        {
            resize_block(c1, c2, num_rows(mtx), num_cols(mtx));
            (*this)(c1, c2) += mtx;
        } else {
            std::size_t maxrows = std::max(num_rows(mtx),
                                           num_rows((*this)(c1, c2)));
            std::size_t maxcols = std::max(num_cols(mtx),
                                           num_cols((*this)(c1, c2)));
            
            Matrix cpy = mtx; // only in this case do we need to copy the argument matrix
            
            resize_block(c1, c2, maxrows, maxcols);
            resize(cpy, maxrows, maxcols);
            
            (*this)(c1, c2) += cpy;
        }
    } else
        insert_block(mtx, c1, c2);
    timer.end();
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::resize_block(charge r, charge c,
                                                   size_type new_r, size_type new_c,
                                                   bool pretend)
{
    if (!pretend)
        resize((*this)(r,c), new_r, new_c);
    rows_[rows_.position(r)].second = new_r;
    cols_[cols_.position(c)].second = new_c;
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::remove_block(charge r, charge c)
{
    assert( has_block(r, c) );
    assert( rows_.position(r) == cols_.position(c) );
    
    std::size_t which = rows_.position(r);
    
    rows_.erase(rows_.begin() + which);
    cols_.erase(cols_.begin() + which);
    data_.erase(data_.begin() + which);
}
#ifdef HAVE_ALPS_HDF5

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::serialize(alps::hdf5::iarchive & ar)
{
    ar >> alps::make_pvp("rows_", rows_);
    ar >> alps::make_pvp("cols_", cols_);
    ar >> alps::make_pvp("data_", data_);
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::serialize(alps::hdf5::oarchive & ar) const
{
    ar << alps::make_pvp("rows_", rows_);
    ar << alps::make_pvp("cols_", cols_);
    ar << alps::make_pvp("data_", data_);
}

#endif

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::reserve(charge c1, charge c2,
                                              std::size_t r, std::size_t c)
{
    if (this->has_block(c1, c2))
    {
        std::size_t maxrows = std::max(rows_[rows_.position(c1)].second, r);
        std::size_t maxcols = std::max(cols_[cols_.position(c2)].second, c);
    
        rows_[rows_.position(c1)].second = maxrows;
        cols_[cols_.position(c2)].second = maxcols;
    } else {
        std::pair<charge, size_type>
        p1 = std::make_pair(c1, r),
        p2 = std::make_pair(c2, c);
        
        assert(rows_.size() == cols_.size());
        assert(rows_.size() == data_.size());
        
        size_type i1 = rows_.insert(p1);
        cols_.insert(i1, p2);
        data_.insert(data_.begin() + i1, Matrix()); 
        /*rows_.push_back(p1);
        cols_.push_back(p2);
        data_.push_back(Matrix());*/
    }
}

template<class Matrix, class SymmGroup>
void block_matrix<Matrix, SymmGroup>::allocate_blocks()
{
    for (std::size_t k = 0; k < n_blocks(); ++k)
        resize(data_[k], rows_[k].second, cols_[k].second);
}
