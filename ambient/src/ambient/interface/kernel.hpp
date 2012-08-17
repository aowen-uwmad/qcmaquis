#ifndef AMBIENT_INTERFACE_KERNELS
#define AMBIENT_INTERFACE_KERNELS
#include "ambient/utils/timings.hpp"
 
//#define AMBIENT_SPAWN_TIMINGS
#ifdef AMBIENT_SPAWN_TIMINGS
    #define __A_TIME(name) static __a_timer time(name); time.begin();
    #define __A_TIME_STOP time.end();
#else
    #define __A_TIME(name) 
    #define __A_TIME_STOP 
#endif

namespace ambient {

    using ambient::controllers::velvet::cfunctor;
    using ambient::models::velvet::revision;

    using ambient::controllers::velvet::c_revision;
    using ambient::controllers::velvet::w_revision;
    using ambient::controllers::velvet::p_revision;
    using ambient::controllers::velvet::r_revision;

    template<typename FP, FP fp>
    struct kernel_inliner { 
        private:
        static void invoke    (){}
        static void latch     (){}
        static void cleanup   (){}
        static void place     (){}
        static bool ready(void*){}
        static bool match(void*){}
        static void tag  (void*){}
    };

    #include "ambient/interface/pp/kernel_inliner.pp.hpp"

    template<class K>
    class kernel : public cfunctor
    {
    public:
        inline void* operator new (size_t size){ 
            return ambient::bulk_pool.get<sizeof(K)>(); 
        }
        
        inline void operator delete (void* ptr){ }

        virtual void place()       { return kernel_inliner<typename K::F,&K::c>::place(this);      }
        virtual bool ready(void* e){ return kernel_inliner<typename K::F,&K::c>::ready(this, e);   }
        virtual bool match(void* t){ return kernel_inliner<typename K::F,&K::c>::match(this, t);   }
        virtual void tag(void* t)  { return kernel_inliner<typename K::F,&K::c>::tag(this, t);     }
        virtual void computation() {        kernel_inliner<typename K::F,&K::c>::invoke((K*)this); 
                                     return kernel_inliner<typename K::F,&K::c>::cleanup(this);    }
        virtual void logistics()   { return kernel_inliner<typename K::F,&K::l>::invoke((K*)this); }

        inline void pin(revision& r){
            this->set_group(channel.world()); 
            ambient::controller.ifetch(r);
            ambient::controller.schedule(this);
        }
        inline void assign(revision& r){ 
            ambient::controller.ifetch(r);
        }
        template <typename T> inline revision&   ui_l_current(T& obj){ return *obj.impl->content[obj.ref];                  }
        template <typename T> inline c_revision& ui_c_current(T& obj){ return *(c_revision*)obj.impl->content[obj.ref];     }
        template <typename T> inline w_revision& ui_w_updated(T& obj){ return *(w_revision*)(obj.impl->content[obj.ref+1]); }
        template <typename T> inline p_revision& ui_p_updated(T& obj){ return *(p_revision*)(obj.impl->content[obj.ref+1]); }
        template <typename T> inline r_revision& ui_r_updated(T& obj){ return *(r_revision*)(obj.impl->content[obj.ref+1]); }

        template <class T0>
        static inline void spawn(T0& arg0){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0));
        }
        template <class T0, class T1>
        static inline void spawn(T0& arg0, T1& arg1){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0), info<T1>::unfold(arg1));
        }
        template <class T0, class T1, class T2>
        static inline void spawn(T0& arg0, T1& arg1, T2& arg2){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0), info<T1>::unfold(arg1), info<T2>::unfold(arg2));
        }
        template <class T0 , class T1 , class T2 , class T3 >
        static inline void spawn(T0 &arg0 , T1 &arg1 , T2 &arg2 , T3 &arg3 ){
            kernel_inliner<typename K::F, &K::c>::latch(new K(), info<T0>::unfold(arg0) , info<T1>::unfold(arg1) , info<T2>::unfold(arg2) , info<T3>::unfold(arg3) );
        }
        template <class T0 , class T1 , class T2 , class T3 , class T4 >
        static inline void spawn(T0 &arg0 , T1 &arg1 , T2 &arg2 , T3 &arg3 , T4 &arg4 ){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0) , info<T1>::unfold(arg1) , info<T2>::unfold(arg2) , info<T3>::unfold(arg3) , info<T4>::unfold(arg4) );
        }
        template <class T0 , class T1 , class T2 , class T3 , class T4 , class T5 >
        static inline void spawn(T0 &arg0 , T1 &arg1 , T2 &arg2 , T3 &arg3 , T4 &arg4 , T5 &arg5 ){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0) , info<T1>::unfold(arg1) , info<T2>::unfold(arg2) , info<T3>::unfold(arg3) , info<T4>::unfold(arg4) , info<T5>::unfold(arg5) );
        }
        template <class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 >
        static inline void spawn(T0 &arg0 , T1 &arg1 , T2 &arg2 , T3 &arg3 , T4 &arg4 , T5 &arg5 , T6 &arg6 ){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0) , info<T1>::unfold(arg1) , info<T2>::unfold(arg2) , info<T3>::unfold(arg3) , info<T4>::unfold(arg4) , info<T5>::unfold(arg5) , info<T6>::unfold(arg6) );
        }
        template <class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 >
        static inline void spawn(T0 &arg0 , T1 &arg1 , T2 &arg2 , T3 &arg3 , T4 &arg4 , T5 &arg5 , T6 &arg6 , T7 &arg7 ){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0) , info<T1>::unfold(arg1) , info<T2>::unfold(arg2) , info<T3>::unfold(arg3) , info<T4>::unfold(arg4) , info<T5>::unfold(arg5) , info<T6>::unfold(arg6) , info<T7>::unfold(arg7) );
        }
        template <class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 >
        static inline void spawn(T0 &arg0 , T1 &arg1 , T2 &arg2 , T3 &arg3 , T4 &arg4 , T5 &arg5 , T6 &arg6 , T7 &arg7 , T8 &arg8 ){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0) , info<T1>::unfold(arg1) , info<T2>::unfold(arg2) , info<T3>::unfold(arg3) , info<T4>::unfold(arg4) , info<T5>::unfold(arg5) , info<T6>::unfold(arg6) , info<T7>::unfold(arg7) , info<T8>::unfold(arg8) );
        }
        template <class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 >
        static inline void spawn(T0 &arg0 , T1 &arg1 , T2 &arg2 , T3 &arg3 , T4 &arg4 , T5 &arg5 , T6 &arg6 , T7 &arg7 , T8 &arg8 , T9 &arg9 ){
            kernel_inliner<typename K::F,&K::c>::latch(new K(), info<T0>::unfold(arg0) , info<T1>::unfold(arg1) , info<T2>::unfold(arg2) , info<T3>::unfold(arg3) , info<T4>::unfold(arg4) , info<T5>::unfold(arg5) , info<T6>::unfold(arg6) , info<T7>::unfold(arg7) , info<T8>::unfold(arg8) , info<T9>::unfold(arg9) );
        }

    };
}

#endif
