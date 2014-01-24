/*
 * Ambient, License - Version 1.0 - May 3rd, 2012
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AMBIENT_INTERFACE_TYPED
#define AMBIENT_INTERFACE_TYPED

#define EXTRACT(var) T* var = (T*)m->arguments[arg];

namespace ambient { namespace numeric {
    template <class T, class Allocator> class matrix;
    template <class T> class transpose_view;
    template <class T> class diagonal_matrix;
} }

namespace ambient {
    template<typename T> class default_allocator;
    using ambient::controllers::ssm::functor;
    using ambient::models::ssm::history;
    using ambient::models::ssm::revision;
    using ambient::memory::instr_bulk;

    // {{{ compile-time type info: singular types + inplace and future specializations
    template <typename T> struct singular_info {
        template<size_t arg> static void deallocate     (functor* m){                        }
        template<size_t arg> static bool pin            (functor* m){ return false;          }
        template<size_t arg> static void score          (T& obj)     {                        }
        template<size_t arg> static bool ready          (functor* m){ return true;           }
        template<size_t arg> static T&   revised        (functor* m){ EXTRACT(o); return *o; }
        template<size_t arg> static void modify (T& obj, functor* m){
            m->arguments[arg] = (void*)new(ambient::pool::malloc<instr_bulk,T>()) T(obj); 
        }
        template<size_t arg> static void modify_remote(T& obj)       {                        }
        template<size_t arg> static void modify_local(T& obj, functor* m){
            m->arguments[arg] = (void*)new(ambient::pool::malloc<instr_bulk,T>()) T(obj);
        }
    };
    template <typename T> struct singular_inplace_info : public singular_info<T> {
        template<size_t arg> static T& revised(functor* m){ return *(T*)&m->arguments[arg]; }
        template<size_t arg> static void modify_remote(T& obj){ }
        template<size_t arg> static void modify_local(T& obj, functor* m){ *(T*)&m->arguments[arg] = obj; }
        template<size_t arg> static void modify(T& obj, functor* m){ *(T*)&m->arguments[arg] = obj; }
    };
    template <typename T> struct future_info : public singular_info<T> {
        template<size_t arg> static void deallocate(functor* m){       
            EXTRACT(o); o->core->generator = NULL;
        }
        template<size_t arg> static void modify_remote(T& obj){ 
            cell().rsync(obj.core);
        }
        template<size_t arg> static void modify_local(const T& obj, functor* m){ 
            obj.core->generator = m;
            cell().lsync(obj.core);
            m->arguments[arg] = (void*)new(ambient::pool::malloc<instr_bulk,T>()) T(obj.core);
        }
        template<size_t arg> static void modify(const T& obj, functor* m){ 
            m->arguments[arg] = (void*)new(ambient::pool::malloc<instr_bulk,T>()) T(obj.core);
        }
    };
    template <typename T> struct read_future_info : public future_info<T> {
        template<size_t arg> static void deallocate(functor* m){ }
        template<size_t arg> static void modify_remote(T& obj){ }
        template<size_t arg> static void modify_local(const T& obj, functor* m){
            m->arguments[arg] = (void*)new(ambient::pool::malloc<instr_bulk,T>()) T(obj.core);
        }
    };
    // }}}
    // {{{ compile-time type info: iteratable derived types
    template <typename T> struct iteratable_info : public singular_info<T> {
        template<size_t arg> 
        static void deallocate(functor* m){
            EXTRACT(o);
            revision& parent  = *o->before;
            revision& current = *o->after;
            current.complete();
            current.release();
            cell().squeeze(&parent);
            parent.release();
        }
        template<size_t arg>
        static void modify_remote(T& obj){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            cell().rsync(o->back());
            cell().collect(o->back());
            cell().add_revision<ambient::remote>(o, cell().which()); 
        }
        template<size_t arg>
        static void modify_local(T& obj, functor* m){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            T* var = (T*)ambient::pool::malloc<instr_bulk,T>(); memcpy((void*)var, &obj, sizeof(T)); 
            m->arguments[arg] = (void*)var;
            cell().lsync(o->back());
            cell().use_revision(o);
            cell().collect(o->back());

            var->before = o->current;
            cell().add_revision<ambient::local>(o, m); 
            cell().use_revision(o);
            var->after = o->current;
        }
        template<size_t arg>
        static void modify(T& obj, functor* m){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            T* var = (T*)ambient::pool::malloc<instr_bulk,T>(); memcpy((void*)var, &obj, sizeof(T)); m->arguments[arg] = (void*)var;
            cell().sync(o->back());
            cell().use_revision(o);
            cell().collect(o->back());

            var->before = o->current;
            cell().add_revision<ambient::common>(o, m); 
            cell().use_revision(o);
            var->after = o->current;
        }
        template<size_t arg> 
        static bool pin(functor* m){ 
            EXTRACT(o);
            revision& r = *o->before;
            void* generator = r.generator;
            if(generator != NULL){
                ((functor*)generator)->queue(m);
                return true;
            }
            return false;
        }
        template<size_t arg> 
        static void score(T& obj){
            cell().intend_read(obj.versioned.core);
            cell().intend_write(obj.versioned.core);
        }
        template<size_t arg> 
        static bool ready(functor* m){
            EXTRACT(o);
            revision& r = *o->before;
            void* generator = r.generator;
            if(generator == NULL || generator == m) return true;
            return false;
        }
    };
    // }}}
    // {{{ compile-time type info: only read/write iteratable derived types
    template <typename T> struct read_iteratable_info : public iteratable_info<T> {
        template<size_t arg> static void deallocate(functor* m){
            EXTRACT(o);
            revision& r = *o->before;
            cell().squeeze(&r);
            r.release();
        }
        template<size_t arg> static void modify_remote(T& obj){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            cell().rsync(o->back());
        }
        template<size_t arg> static void modify_local(T& obj, functor* m){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            T* var = (T*)ambient::pool::malloc<instr_bulk,T>(); memcpy((void*)var, &obj, sizeof(T)); m->arguments[arg] = (void*)var;
            var->before = o->current;
            cell().lsync(o->back());
            cell().use_revision(o);
        }
        template<size_t arg> static void modify(T& obj, functor* m){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            T* var = (T*)ambient::pool::malloc<instr_bulk,T>(); memcpy((void*)var, &obj, sizeof(T)); m->arguments[arg] = (void*)var;
            var->before = o->current;
            cell().sync(o->back());
            cell().use_revision(o);
        }
        template<size_t arg> 
        static void score(T& obj){
            cell().intend_read(obj.versioned.core);
        }
    };
    template <typename T> struct write_iteratable_info : public iteratable_info<T> {
        template<size_t arg> static void modify_remote(T& obj){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            cell().collect(o->back());
            cell().add_revision<ambient::remote>(o, cell().which()); 
        }
        template<size_t arg> static void modify_local(T& obj, functor* m){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            T* var = (T*)ambient::pool::malloc<instr_bulk,T>(); memcpy((void*)var, &obj, sizeof(T)); m->arguments[arg] = (void*)var;

            cell().use_revision(o);
            cell().collect(o->back());

            var->before = o->current;
            cell().add_revision<ambient::local>(o, m); 
            cell().use_revision(o);
            var->after = o->current;
        }
        template<size_t arg> static void modify(T& obj, functor* m){
            decltype(obj.versioned.core) o = obj.versioned.core;
            cell().touch(o);
            T* var = (T*)ambient::pool::malloc<instr_bulk,T>(); memcpy((void*)var, &obj, sizeof(T)); m->arguments[arg] = (void*)var;
            cell().use_revision(o);
            cell().collect(o->back());

            var->before = o->current;
            cell().add_revision<ambient::common>(o, m); 
            cell().use_revision(o);
            var->after = o->current;
        }
        template<size_t arg> static bool pin(functor* m){ return false; }
        template<size_t arg> static void score(T& obj) {               
            cell().intend_write(obj.versioned.core);
        }
        template<size_t arg> static bool ready (functor* m){ return true;  }
    };
    // }}}

    // {{{ compile-time type info: specialization for forwarded types
    using ambient::numeric::future;
    using ambient::numeric::matrix;
    using ambient::numeric::diagonal_matrix;
    using ambient::numeric::transpose_view;

    template <typename T>
    struct has_allocator {
        template <typename T1> static typename T1::allocator_type test(int);
        template <typename>    static void test(...);
        enum { value = !std::is_void<decltype(test<T>(0))>::value };
    };
    template <bool HAS, typename T> struct checked_get_allocator {};
    template <typename T> struct checked_get_allocator<true, T> { typedef typename T::allocator_type type; };
    template <typename T> struct checked_get_allocator<false, T> { typedef typename ambient::default_allocator<T> type; }; // or T::value_type
    template <typename T> struct get_allocator { typedef typename checked_get_allocator<has_allocator<T>::value, T>::type type; };

    template <class T> struct unbound : public T {
        typedef typename get_allocator<T>::type allocator_type;
    };

    template<typename T> struct has_versioning {
        template<std::size_t V> struct valuekeeper {};
        template<typename R, typename C> static char helper(R(C::*)());
        template<typename C> static char check(valuekeeper<sizeof(helper(&C::enable_versioning))>*);
        template<typename C> static double check(...);
        enum { value = (sizeof(char) == sizeof(check<T>(0))) };
    };

    template <bool V, typename T> struct versioned_info { };
    template<typename T> struct versioned_info<true, T> { typedef iteratable_info< T > type; };
    template<typename T> struct versioned_info<false, T> { typedef singular_info< T > type; };

    template <bool V, typename T> struct const_versioned_info { };
    template<typename T> struct const_versioned_info<true, T> { typedef read_iteratable_info< const T > type; };
    template<typename T> struct const_versioned_info<false, T> { typedef singular_info< const T > type; };

    template <typename T>
    struct info {
        typedef typename       versioned_info<has_versioning<T>::value,T>::type typed;
        template <typename U> static U& unfold(T& naked){ return *static_cast<U*>(&naked); }
    };
    template <typename T>
    struct info <const T> {
        typedef typename const_versioned_info<has_versioning<T>::value,T>::type typed;
        template <typename U> static const T& unfold(const T& naked){ return naked; }
    };

    template <>
    struct info < size_t > {
        typedef size_t type;
        typedef singular_inplace_info<type> typed; 
        template <typename U> static type& unfold(type& naked){ return naked; }
    };

    template <typename S>
    struct info < future<S> > {
        typedef future<S> type;
        typedef future_info<type> typed; 
        template <typename U> static type& unfold(type& folded){ return folded.unfold(); }
    };

    template <typename S>
    struct info < const future<S> > { 
        typedef const future<S> type;
        typedef read_future_info<type> typed; 
        template <typename U> static type& unfold(type& folded){ return folded.unfold(); }
    };

    template <typename S>
    struct info < diagonal_matrix<S> > {
        typedef diagonal_matrix<S> type;
        template <typename U> static U& unfold(type& folded){ return *static_cast<U*>(&folded.get_data()); }
    };

    template <typename S>
    struct info < const diagonal_matrix<S> > {
        typedef const diagonal_matrix<S> type;
        template <typename U> static const matrix<S, ambient::default_allocator<S> >& unfold(type& folded){ return folded.get_data(); }
    };

    template <class Matrix>
    struct info < const transpose_view<Matrix> > {
        typedef const transpose_view<Matrix> type;
        template <typename U> static const Matrix& unfold(type& folded){ return *(const Matrix*)&folded; }
    };

    template <class Matrix>
    struct info < transpose_view<Matrix> > {
        typedef transpose_view<Matrix> type;
        template <typename U> static Matrix& unfold(type& folded){ return *(Matrix*)&folded; }
    };

    template <typename T>
    struct info < unbound<T> > {
        typedef unbound<T> type;
        typedef write_iteratable_info< type > typed; 
        template <typename U> static type& unfold(type& naked){ return naked; }
    };

    // }}}

    #define ambient_non_destroyable  static int disable_destructor(int);
    #define ambient_version(...)     mutable ambient::revision* before; \
                                     mutable ambient::revision* after; \
                                     void enable_versioning(); \
                                     static void disable_destructor(...); \
                                     enum { destructor_disabled = !std::is_void<decltype(disable_destructor(0))>::value }; \
                                     struct unnamed { \
                                           struct mapping {\
                                               __VA_ARGS__; \
                                           };\
                                           unnamed(){ core = new ambient::history(ambient::dim2(1,1),sizeof(unnamed)); } \
                                           unnamed(size_t length){ core = new ambient::history(ambient::dim2(1,1),length); } \
                                           unnamed(size_t length, size_t sz){ core = new ambient::history(ambient::dim2(1,length),sz); } \
                                           unnamed(size_t rows, size_t cols, size_t sz){ core = new ambient::history(ambient::dim2(cols, rows), sz); } \
                                           unnamed(ambient::dim2 dim, size_t sz){ core = new ambient::history(dim, sz); } \
                                           template<typename U> unnamed(const U& other):core(other.core){ } \
                                           unnamed(ambient::history* core):core(core){ } \
                                          ~unnamed(){ if(!destructor_disabled){ if(core->weak()) delete core; else ambient::destroy(core); } } \
                                           ambient::history* core; \
                                     } versioned;

    #define AMBIENT_VAR_LENGTH 1
}

#undef EXTRACT
#endif
