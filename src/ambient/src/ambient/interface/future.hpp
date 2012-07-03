#ifndef AMBIENT_INTERFACE_FUTURE
#define AMBIENT_INTERFACE_FUTURE
// see history for an advanced version // supports multiple revisions

namespace ambient {

    template <typename T>
    class future {
    public:
        typedef typename boost::intrusive_ptr< container<sizeof(T)> > ptr;
        typedef T value_type;

        inline future(){
            this->naked = new container<sizeof(T)>();
            this->ghost = (container<sizeof(T)>*)this->naked;
            *(T*)this->naked = T();
            this->value = (T*)this->naked;
        }

        inline future(const future& f){
            this->naked = new container<sizeof(T)>();
            *(T*)this->naked = (T)f; // unfolding f (can be more optimal / wo playout)
            this->ghost = (container<sizeof(T)>*)this->naked;
            this->value = NULL; // let's playout again for copy
        }

        inline future(double value){
            this->naked = new container<sizeof(T)>();
            this->ghost = (container<sizeof(T)>*)this->naked;
            *(T*)this->naked = value;
            this->value = (T*)this->naked;
        }

        inline future(std::complex<double> value){
            this->naked = new container<sizeof(T)>();
            this->ghost = (container<sizeof(T)>*)this->naked;
            *(T*)this->naked = value;
            this->value = (T*)this->naked;
        }

        inline operator T () const {
            if(this->value == NULL){
                ambient::playout();
                this->value = (T*)this->naked;
            }
            return *this->value;
        }

        inline T*& unfold(){
            this->value = NULL;
            return (T*&)this->naked;
        }

        inline const T*& unfold() const {
            return (const T*&)this->naked;
        }
    //private:
        ptr    ghost;
        mutable T* value;
        void*  naked;
    };

    template<typename T1, typename T2>
    inline const T2 operator / (T1 lhs, future<T2> rhs){ 
        return (lhs / (T2)rhs); 
    }

    template<typename T>
    inline const future<T> operator + (future<T> lhs, future<T> rhs){ 
        return future<T>((T)lhs + (T)rhs); // explicit
    }

}

#endif
