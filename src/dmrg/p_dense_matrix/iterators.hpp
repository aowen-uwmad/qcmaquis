#ifndef __ALPS_P_DENSE_MATRIX_ITERATORS_HPP__
#define __ALPS_P_DENSE_MATRIX_ITERATORS_HPP__

#include <boost/iterator/iterator_facade.hpp>
#include <boost/static_assert.hpp>
#include <cassert>

template <typename Matrix, typename T>
class p_matrix_element_iterator : public boost::iterator_facade<
                                p_matrix_element_iterator<Matrix,T>,
                                T,
                                boost::random_access_traversal_tag,
                                T&,
                                typename Matrix::difference_type
                                >
{
    public:
        typedef T value_type;

        p_matrix_element_iterator(Matrix* m, typename Matrix::size_type row_p, typename Matrix::size_type col_p)
            : m(m), i(row_p), j(col_p)
        {
            // The value_type of the iterator must be the value_type of the matrix or const Matrix::value_type
            BOOST_STATIC_ASSERT( (boost::is_same<typename Matrix::value_type, T>::value
                                 || boost::is_same<const typename Matrix::value_type,T>::value) );
#ifndef DISABLE_MATRIX_ELEMENT_ITERATOR_WARNING
            std::cerr<<"WARNING: p_matrix_element_iterators are very slow!"<<std::endl;
            std::cerr<<"You should use strided_iterators (eg. row_iterator) instead, unless you really don't care."<<std::endl;
            std::cerr<<"To disable this warning compile with -DDISABLE_MATRIX_ELEMENT_ITERATOR_WARNING ."<<std::endl;
#endif //DISABLE_MATRIX_ELEMENT_ITERATOR_WARNING
        }

        template<typename Matrix2, typename U>
        p_matrix_element_iterator(p_matrix_element_iterator<Matrix2,U> const& r)
            : m(r.m), i(r.i), j(r.j)
        {
            BOOST_STATIC_ASSERT( (boost::is_same<
                        typename boost::add_const<Matrix>::type,
                        typename boost::add_const<Matrix2>::type
                        >::value ));
        }

    private:
        friend class boost::iterator_core_access;
        template <typename,typename> friend class p_matrix_element_iterator;

        value_type& dereference() const
        { return m->operator()(i,j); }

        template <typename Matrix2,typename U>
        bool equal(p_matrix_element_iterator<Matrix2,U> const& y) const
        {
            BOOST_STATIC_ASSERT( (boost::is_same<
                        typename boost::add_const<Matrix>::type,
                        typename boost::add_const<Matrix2>::type
                        >::value ));
            if( i == y.i && j == y.j )
                return true;
            else
                return false;
        }
        void increment()
        {
            ++i;
            if( i >= num_rows(*m) )
            {
                i=0;
                ++j;
            }
        }
        void decrement()
        {
            --i;
            if( i < 0)
            {
                i = num_rows(*m)-1;
                --j;
            }
        }
        void advance(typename Matrix::difference_type n)
        {
            j += n / num_rows(*m);
            i += n % num_rows(*m);
        }

        template <typename U>
        typename Matrix::difference_type distance_to(p_matrix_element_iterator<Matrix,U> const& z) const
        {
            return (z.j - j)*num_rows(*m) + z.i - i;
        }

        // Matrix
        Matrix* m;
        // position row i, column j
        typename Matrix::difference_type i;
        typename Matrix::difference_type j;
};

#endif //__ALPS_P_DENSE_MATRIX_ITERATORS_HPP__
