#ifndef __ALPS_P_DENSE_MATRIX_HPP__
#define __ALPS_P_DENSE_MATRIX_HPP__

#include "p_dense_matrix/strided_iterator.hpp"
#include "p_dense_matrix/matrix_element_iterator.hpp"
#include "dense_matrix/vector.hpp"
#include "p_dense_matrix/detail/dense_matrix_adaptor.hpp"
#include "ambient/interfaces/i_dense_matrix.h"

#include "utils/function_objects.h"

#include "p_dense_matrix/diagonal_matrix.h"

#include <boost/lambda/lambda.hpp>
#include <boost/typeof/typeof.hpp>
#include <ostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>

#ifdef HAVE_ALPS_HDF5
#include <alps/hdf5.hpp>
#endif

namespace blas {
    /** A matrix template class
      *
      * The p_dense_matrix class is a matrix which can take any type T
      * @param T the type of the elements to be stored in the matrix
      * @param MemoryBlock the underlying (continous) Memory structure
      */
    template <typename T, typename MemoryBlock = std::vector<T> >
    class p_dense_matrix : public i_dense_matrix {
    public:
        // typedefs required for a std::container concept
        typedef T                       value_type;       // The type T of the elements of the matrix
        typedef T&                      reference;        // Reference to value_type
        typedef T const&                const_reference;  // Const reference to value_type
        typedef std::size_t             size_type;        // Unsigned integer type that represents the dimensions of the matrix
        typedef std::ptrdiff_t          difference_type;  // Signed integer type to represent the distance of two elements in the memory

        // for compliance with an std::container one would also need
        // -operators == != < > <= >=
        // -size()
        // -typedefs iterator, const_iterator

        // typedefs for matrix specific iterators
        typedef strided_iterator<p_dense_matrix,value_type>
            row_element_iterator;                         // Iterator to iterate through the elements of a row of the matrix
        typedef strided_iterator<const p_dense_matrix,const value_type>
            const_row_element_iterator;                   // Const version of row_element_iterator
        typedef value_type*
            column_element_iterator;                      // Iterator to iterate through the elements of a columns of the matrix
        typedef value_type const*
            const_column_element_iterator;                // Const version of column_element_iterator       
        typedef matrix_element_iterator<p_dense_matrix,value_type>
            element_iterator;                             // Iterator to iterate through all elements of the matrix (REALLY SLOW! USE row_-/column_iterators INSTEAD!)
        typedef matrix_element_iterator<const p_dense_matrix,const value_type>
            const_element_iterator;                       // Const version of element_iterator (REALLY SLOW! USE row_-/column_iterators INSTEAD!)

        /**
          * The constructor
          * @param rows the number of rows
          * @param columns the number of columns
          * @param init_value all matrix elements will be initialized to this value.
          */
        p_dense_matrix(size_type rows, size_type columns, T init_value);

        /**
          * The copy constructor
          *
          */
        template <typename OtherMemoryBlock>
        p_dense_matrix(p_dense_matrix<T,OtherMemoryBlock> const& m);

        /**
          * The i_dense_matrix implementation:
          * @returns memory pointer to the actual matrix memory
          *
          */
        void* memory_pointer();
        
        /**
          * Non-throwing swap function
          * @param r p_dense_matrix object which should be swapped with the p_dense_matrix (this)
          */
        void swap(p_dense_matrix & r);

        /**
          * Swaps two dense_matrices
          */
        friend void swap(p_dense_matrix & x, p_dense_matrix & y)
        {
            x.swap(y);
        }

        /**
          * Assigns the matrix to matrix rhs
          */
        p_dense_matrix& operator = (p_dense_matrix rhs);

        /**
          * Access the element in row i, column j
          * @param i 0<= i <= num_rows()
          * @param j 0<= j <= num_columns()
          * @return A mutable reference to the matrix element at position (i,j).
          */
        inline value_type& operator()(const size_type i, const size_type j);
        
        /**
          * Access the element in row i, column j
          * @return A constant reference to the matrix element at position (i,j).
          */
        inline value_type const& operator()(const size_type i, const size_type j) const;

        bool operator == (p_dense_matrix const& rhs) const;

        p_dense_matrix<T,MemoryBlock>& operator += (p_dense_matrix const& rhs); 

        p_dense_matrix<T,MemoryBlock>& operator -= (p_dense_matrix const& rhs);
        
        template <typename T2>
        p_dense_matrix<T,MemoryBlock>& operator *= (T2 const& t);
        
        template <typename T2>
        p_dense_matrix<T,MemoryBlock>& operator /= (T2 const& t);

        /**
          * Checks if a matrix is empty
          * @return true if the matrix is a 0x0 matrix, false otherwise.
          */
        inline const bool empty() const;

        /**
          * @return number of rows of the matrix
          */
        inline const size_type num_rows() const;

        /**
          * @return number of columns of the matrix
          */
        inline const size_type num_columns() const;

        /**
          * @return The stride for moving to the next element along a row
          */
        inline const difference_type stride1() const;
        
        /**
          * @return The stride for moving to the next element along a column
          */
        inline const difference_type stride2() const;

        /** Resize the matrix.
          * Resizes the matrix to size1 rows and size2 columns and initializes
          * the new elements to init_value. It also enlarges the MemoryBlock
          * if needed. If the new size for any dimension is
          * smaller only elements outside the new size will be deleted.
          * If the new size is larger for any dimension the new elements
          * will be initialized by the init_value.
          * All other elements will keep their value.  
          *
          * Exception behaviour:
          * As long as the assignment and copy operation of the T values don't throw an exception,
          * any exception will leave the matrix unchanged.
          * (Assuming the same behaviour of the underlying MemoryBlock. This is true for std::vector.)
          * @param size1 new number of rows
          * @param size2 new number of columns
          * @param init_value value to which the new elements will be initalized
          */
        void resize(size_type size1, size_type size2, T const& init_value);

        /**
          * Reserves memory for anticipated enlargements of the matrix
          * @param size1 For how many rows should memory be reserved, value is ignored if it's smaller than the current number of rows
          * @param size2 For how many columns should memory be reserved, value is ignored if it's smaller than the current number of columns
          * @param init_value i
          */
        void reserve(size_type size1, size_type size2, T const& init_value);

        std::pair<size_type,size_type> capacity() const;
        
        bool is_shrinkable() const;

        void clear();

        std::pair<row_element_iterator,row_element_iterator> row(size_type row = 0)
        {
            return std::make_pair( row_element_iterator(&values_[row],reserved_size1_), row_element_iterator(&values_[row+reserved_size1_*size2_], reserved_size1_) );
        }
        std::pair<const_row_element_iterator,const_row_element_iterator> row(size_type row = 0) const
        {
            return std::make_pair( const_row_element_iterator(&values_[row],reserved_size1_), const_row_element_iterator(&values_[row+reserved_size1_*size2_], reserved_size1_) );
        }
        std::pair<column_element_iterator,column_element_iterator> column(size_type col = 0 )
        {
            return std::make_pair( column_element_iterator(&values_[col*reserved_size1_]), column_element_iterator(&values_[col*reserved_size1_+size1_]) );
        }
        std::pair<const_column_element_iterator,const_column_element_iterator> column(size_type col = 0 ) const
        {
            return std::make_pair( const_column_element_iterator(&values_[col*reserved_size1_]), const_column_element_iterator(&values_[col*reserved_size1_+size1_]) );
        }
        std::pair<element_iterator,element_iterator> elements()
        {
            return std::make_pair( element_iterator(this,0,0), element_iterator(this,0, num_columns()) );
        }
        std::pair<element_iterator,element_iterator> elements() const
        {
            return std::make_pair( const_element_iterator(this,0,0), const_element_iterator(this,0,num_columns() ) );
        }

        template <typename InputIterator>
        void append_columns(std::pair<InputIterator,InputIterator> const& range, difference_type k);

        template <typename InputIterator>
        void append_rows(std::pair<InputIterator,InputIterator> const& range, difference_type k);

        template <typename InputIterator>
        void insert_rows(size_type i, std::pair<InputIterator,InputIterator> const& range, difference_type k);

        template <typename InputIterator>
        void insert_columns(size_type j, std::pair<InputIterator,InputIterator> const& range, difference_type k);

        void remove_rows(size_type i, difference_type k);

        void remove_columns(size_type j, difference_type k);

        void swap_rows(size_type i1, size_type i2);

        void swap_columns(size_type j1, size_type j2);


        // Default implementations
        void plus_assign(p_dense_matrix const& rhs);

        void minus_assign(p_dense_matrix const& rhs);

        template <typename T2>
        void multiplies_assign (T2 const& t);
        
        void inplace_conjugate();
		
		// Serialize functions to save dense_matrix using alps::hdf5
#ifdef HAVE_ALPS_HDF5
		void serialize(alps::hdf5::iarchive & ar);
		void serialize(alps::hdf5::oarchive & ar) const;
#endif

    private:
        template <typename OtherT,typename OtherMemoryBlock>
        friend class p_dense_matrix;

        inline bool automatic_reserve(size_type size1, size_type size2, T const& init_value);

        size_type size1_;
        size_type size2_;
        size_type reserved_size1_;
        // "reserved_size2_" is done automatically by underlying std::vector (see vector.reserve(), vector.capacity() )
        MemoryBlock values_;
    };
    
    template<typename T, typename MemoryBlock>
    struct associated_diagonal_matrix<p_dense_matrix<T, MemoryBlock> >
    {
        typedef diagonal_matrix<T> type;
    };
    
} // namespace blas

//
// Type promotion helper for mixed type matrix matrix and matrix vector operations
//
namespace blas {

    template <typename T1, typename MemoryBlock1, typename T2, typename MemoryBlock2>
    struct MultiplyReturnType
    {
        typedef char one;
        typedef long unsigned int two;
        static one test(T1 t) {return one();}
        static two test(T2 t) {return two();}
        typedef typename boost::mpl::if_<typename boost::mpl::bool_<(sizeof(test(T1()*T2())) == sizeof(one))>,T1,T2>::type value_type;
        typedef typename boost::mpl::if_<typename boost::mpl::bool_<(sizeof(test(T1()*T2())) == sizeof(one))>,MemoryBlock1,MemoryBlock2>::type memoryblock_type;
    };

// Specialize for U and T being the same type
    template <typename T,typename MemoryBlock1, typename MemoryBlock2>
    struct MultiplyReturnType<T,MemoryBlock1,T,MemoryBlock2>
    {
        typedef T value_type;
        typedef MemoryBlock2 memoryblock_type;
    };
}

//
// Function hooks
//
namespace blas {

    template <typename T, typename MemoryBlock>
    const p_dense_matrix<T,MemoryBlock> matrix_matrix_multiply(p_dense_matrix<T,MemoryBlock> const& lhs, p_dense_matrix<T,MemoryBlock> const& rhs);
    
    template<typename T, typename MemoryBlock, typename T2, typename MemoryBlock2>
    const vector<typename MultiplyReturnType<T,MemoryBlock,T2,MemoryBlock2>::value_type,typename MultiplyReturnType<T,MemoryBlock,T2,MemoryBlock2>::memoryblock_type>
    matrix_vector_multiply(p_dense_matrix<T,MemoryBlock> const& m, vector<T2,MemoryBlock2> const& v);
    
    template <typename T,typename MemoryBlock>
    void plus_assign(p_dense_matrix<T,MemoryBlock>& m, p_dense_matrix<T,MemoryBlock> const& rhs);

    template <typename T, typename MemoryBlock>
    void minus_assign(p_dense_matrix<T,MemoryBlock>& m, p_dense_matrix<T,MemoryBlock> const& rhs);

    template <typename T, typename MemoryBlock, typename T2>
    void multiplies_assign(p_dense_matrix<T,MemoryBlock>& m, T2 const& t);
}

//
// Free dense matrix functions
//
namespace blas {

    template <typename T, typename MemoryBlock>
    const p_dense_matrix<T,MemoryBlock> operator + (p_dense_matrix<T,MemoryBlock> a, p_dense_matrix<T,MemoryBlock> const& b);
    
    template <typename T, typename MemoryBlock>
    const p_dense_matrix<T,MemoryBlock> operator - (p_dense_matrix<T,MemoryBlock> a, p_dense_matrix<T,MemoryBlock> const& b);

    template<typename T, typename MemoryBlock, typename T2, typename MemoryBlock2>
    const vector<typename MultiplyReturnType<T,MemoryBlock,T2,MemoryBlock2>::value_type, typename MultiplyReturnType<T,MemoryBlock,T2,MemoryBlock2>::memoryblock_type>
    operator * (p_dense_matrix<T,MemoryBlock> const& m, vector<T2,MemoryBlock2> const& v);
   
    // TODO: adj(Vector) * Matrix, where adj is a proxy object

    template<typename T,typename MemoryBlock, typename T2>
    const p_dense_matrix<T,MemoryBlock> operator * (p_dense_matrix<T,MemoryBlock> m, T2 const& t);
    
    template<typename T,typename MemoryBlock, typename T2>
    const p_dense_matrix<T,MemoryBlock> operator * (T2 const& t, p_dense_matrix<T,MemoryBlock> m);

    template<typename T, typename MemoryBlock>
    const p_dense_matrix<T,MemoryBlock> operator * (p_dense_matrix<T,MemoryBlock> const& m1, p_dense_matrix<T,MemoryBlock> const& m2);

    template<typename T,typename MemoryBlock>
    void gemm(p_dense_matrix<T,MemoryBlock> const & A, p_dense_matrix<T,MemoryBlock> const & B, p_dense_matrix<T,MemoryBlock> & C);

    template <typename T, typename MemoryBlock>
    std::ostream& operator << (std::ostream& o, p_dense_matrix<T,MemoryBlock> const& m);

} // namespace blas

#include "p_dense_matrix/p_dense_matrix.hpp"
#endif //__ALPS_P_DENSE_MATRIX_HPP__
