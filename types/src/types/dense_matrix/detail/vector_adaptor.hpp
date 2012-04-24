#ifndef __ALPS_VECTOR_ADAPTOR_HPP__
#define __ALPS_VECTOR_ADAPTOR_HPP__

#include <boost/numeric/bindings/detail/adaptor.hpp>

namespace maquis { namespace types {
    template <typename T, typename MemoryBlock>
    class vector;
} }

//
// An adaptor for the vector to the boost::numeric::bindings
//

namespace boost { namespace numeric { namespace bindings { namespace detail {

    template< typename T, typename MemoryBlock, typename Id, typename Enable >
    struct adaptor<::maquis::types::vector<T,MemoryBlock>, Id, Enable>
    {
        typedef typename copy_const< Id, T >::type value_type;
        typedef std::ptrdiff_t  size_type;
        
        typedef mpl::map<
            mpl::pair< tag::value_type,     value_type >,
            mpl::pair< tag::entity,         tag::vector >,
            mpl::pair< tag::size_type<1>,   size_type >,
            mpl::pair< tag::data_structure, tag::linear_array >,
            mpl::pair< tag::stride_type<1>, tag::contiguous >
        > property_map;

        static std::ptrdiff_t size1( const Id& id ) {
            return id.size();
        }

        static value_type* begin_value( Id& id ) {
            return &(*id.begin());
        }

        static value_type* end_value( Id& id ) {
            return &(*id.end());
        }
    };
}}}}

#endif // __ALPS_VECTOR_ADAPTOR_HPP__
