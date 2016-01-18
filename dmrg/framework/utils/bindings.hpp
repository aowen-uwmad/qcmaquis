#ifndef MATRIX_BINDINGS_H
#define MATRIX_BINDINGS_H

#include <alps/numeric/real.hpp> 

namespace maquis { namespace bindings {

    template <typename O, typename I> struct binding { 
        static O convert(const I& m){ return static_cast<O>(m); }
    };

    template<typename O, typename I> O matrix_cast(I const& input){
       return binding<O,I>::convert(input);
    }

    template <typename T>
    struct binding< std::vector<T>, alps::numeric::diagonal_matrix<T> > {
        static std::vector<T> convert(const alps::numeric::diagonal_matrix<T>& m){
            return m.get_values();
        }
    };

    template <typename T, typename S, template<class M, class SS> class C>
    struct binding< std::vector< std::vector<T> >, C<alps::numeric::diagonal_matrix<T>, S> > {
        static std::vector< std::vector<T> > convert(const C<alps::numeric::diagonal_matrix<T>, S>& m){
            std::vector< std::vector<T> > set;
            for(size_t k = 0; k < m.n_blocks(); ++k){
                set.push_back(m[k].get_values());
            }
            return set;
        }
    };

} }

#ifndef USE_AMBIENT
namespace maquis {
    template <class T> 
    inline typename alps::numeric::real_type<T>::type real(T f){
        return alps::numeric::real(f);
    }

    template<typename _InputIterator, typename _Tp>
    inline _Tp
    accumulate(_InputIterator __first, _InputIterator __last, _Tp __init){
        return std::accumulate(__first, __last, __init);
    }

    template<typename T>
    T sqrt(T arg){
        return std::sqrt(arg);
    }
}
#else
#include <ambient/container/numeric/bindings/alps.hpp>
namespace maquis { 
    namespace bindings {
        #define am_tiles    ambient::tiles            
        #define am_diagonal ambient::diagonal_matrix  
        #define am_matrix   ambient::matrix           
        #define al_diagonal alps::numeric::diagonal_matrix 
        #define al_matrix   alps::numeric::matrix              

        using ambient::numeric::bindings::adaptor;
        
        template <typename T, class A> struct binding< am_matrix<T,A>, al_matrix<T> > : public adaptor< am_matrix<T,A>, al_matrix<T> > {};
        template <typename T, class A> struct binding< al_matrix<T>, am_matrix<T,A> > : public adaptor< al_matrix<T>, am_matrix<T,A> > {};
        template <typename T> struct binding< am_diagonal<T>, al_diagonal<T> > : public adaptor< am_diagonal<T>, al_diagonal<T> > {};
        template <typename T> struct binding< al_diagonal<T>, am_diagonal<T> > : public adaptor< al_diagonal<T>, am_diagonal<T> > {};
        template <typename T, class A> struct binding< am_tiles<am_matrix<T,A> >, al_matrix<T> > : public adaptor< am_tiles<am_matrix<T,A> >, al_matrix<T> > {};
        template <typename T2, typename T1, class A> struct binding< am_tiles<am_matrix<T2,A> >, al_matrix<T1> > : public adaptor< am_tiles<am_matrix<T2,A> >, al_matrix<T1> > {};
        template <typename T, class A> struct binding< al_matrix<T>, am_tiles<am_matrix<T,A> > > : public adaptor< al_matrix<T>, am_tiles<am_matrix<T,A> > > {};
        template <typename T> struct binding< am_tiles<am_diagonal<T> >, al_diagonal<T> > : public adaptor< am_tiles<am_diagonal<T> >, al_diagonal<T> > {};
        template <typename T> struct binding< al_diagonal<T>, am_tiles<am_diagonal<T> > > : public adaptor< al_diagonal<T>, am_tiles<am_diagonal<T> > > {};
        template <typename T> struct binding< std::vector<T>, am_diagonal<T> > : public adaptor< std::vector<T>, am_diagonal<T> > {};
        template <typename T, typename D> struct binding< am_tiles<am_diagonal<T> >, am_tiles<am_diagonal<D> > > : public adaptor< am_tiles<am_diagonal<T> >, am_tiles<am_diagonal<D> > > {};
        template <typename T, typename S, template<class M, class SS> class C> struct binding< std::vector< std::vector<T> >, C<am_diagonal<T>, S> > 
                                                                             : public adaptor< std::vector< std::vector<T> >, C<am_diagonal<T>, S> > {};
        template <typename T, typename S, template<class M, class SS> class C> struct binding< std::vector< std::vector<T> >, C<am_tiles<am_diagonal<T> >, S> > 
                                                                             : public adaptor< std::vector< std::vector<T> >, C<am_tiles<am_diagonal<T> >, S> > {};
        #undef am_tiles
        #undef am_diagonal
        #undef am_matrix
        #undef al_matrix
    }

    template<class T>
    struct real_type {
        typedef T type;
    };

    template<>
    struct real_type<std::complex<double> > {
        typedef double type;
    };

    template<>
    struct real_type<ambient::future<std::complex<double> > > {
        typedef typename ambient::future<double> type;
    };

    inline double real(double f){
        return f;
    }

    inline double real(const std::complex<double>& f){
        return f.real();
    }

    inline std::vector<double> real(const std::vector<std::complex<double> >& f){
        return alps::numeric::real(f);
    }

    inline std::vector<double> real(const std::vector<double>& f){
        return f;
    }

    template<typename T>
    inline const typename real_type<ambient::future<T> >::type& 
    real(const ambient::future<T>& f){
        return ambient::numeric::real(f);
    }

    template <typename T,                          // value_type
             template<class AT> class A,           // allocator 
             template<class TT, class AA> class C> // vector<value_type, allocator>
    inline const C<typename real_type<T>::type, A<typename real_type<T>::type> >
    real(const C<ambient::future<T>, A<ambient::future<T> > >& f){
        return ambient::numeric::real(f);
    }

    template<typename _InputIterator, typename _Tp>
    inline _Tp
    accumulate(_InputIterator __first, _InputIterator __last, _Tp __init)
    {
      __glibcxx_function_requires(_InputIteratorConcept<_InputIterator>)
      __glibcxx_requires_valid_range(__first, __last);

      for (; __first != __last; ++__first)
	__init = __init + *__first;

      #ifdef AMBIENT_LOOSE_FUTURE
      return std::move(__init);
      #else
      return __init;
      #endif
    }

    template<typename T>
    inline T sqrt(T arg){
        return std::sqrt(arg);
    }

    inline ambient::future<double> sqrt(const ambient::future<double>& f){
        return ambient::numeric::sqrt(f);
    }

}
namespace ietl {
    inline double real(const ambient::future<std::complex<double> >& f){
        return ambient::numeric::real(f);
    }
}

#endif
#endif
