#include <boost/concept_check.hpp>
#include <boost/type_traits/remove_const.hpp>

namespace blas
{

template <typename X>
struct Matrix
        : boost::Assignable<X>
{
    public:
        typedef typename X::value_type                          value_type;
        // TODO BOOST_CONCEPT_ASSERT((Field<value_type>));
        typedef typename X::size_type                           size_type;
        BOOST_CONCEPT_ASSERT((boost::UnsignedInteger<size_type>));
        typedef typename X::difference_type                     difference_type;
        BOOST_CONCEPT_ASSERT((boost::SignedInteger<difference_type>));
        
        // TODO write more restrictive BOOST_CONCEPT_ASSERTs for iterators
        typedef typename X::row_element_iterator                row_element_iterator;
        BOOST_CONCEPT_ASSERT((boost::InputIterator<row_element_iterator>));
        typedef typename X::const_row_element_iterator          const_row_element_iterator;
        BOOST_CONCEPT_ASSERT((boost::InputIterator<const_row_element_iterator>));
        typedef typename X::column_element_iterator             column_element_iterator;
        BOOST_CONCEPT_ASSERT((boost::InputIterator<column_element_iterator>));
        typedef typename X::const_column_element_iterator       const_column_element_iterator;
        BOOST_CONCEPT_ASSERT((boost::InputIterator<const_column_element_iterator>));
        

    BOOST_CONCEPT_USAGE(Matrix)
    {
        // Constructor
        typename boost::remove_const<X>::type x(1,1);
        // Copy constructor
        const X y(x);
        typename boost::remove_const<X>::type z = x;

        // Swap
        std::swap(x,z);

        // num_rows(), num_columns()
        std::size_t s = num_rows(y);
        s = num_columns(y);
        
        // Element access
        t = x(0,0);
        ++x(0,0);

        // Swap rows/columns
        swap_rows(x,0,1);
        swap_columns(x,0,1);
        
        // Iterator functions
        std::pair<row_element_iterator,row_element_iterator>                    row_range = row(x,0);
        std::pair<column_element_iterator,column_element_iterator>              column_range = column(x,0);
        std::pair<const_row_element_iterator,const_row_element_iterator>        const_row_range = row(y,0);
        std::pair<const_column_element_iterator,const_column_element_iterator>  const_column_range = column(y,0);

        // operators
        z = x;
        x += y;
        x -= y;
        x *= t;

        z = x + y;
        z = x - y;
        z = x * y;

        //TODO matrix vector multiplication

    }

    private:
        // Default constructable value_type
        value_type t;
};

}
