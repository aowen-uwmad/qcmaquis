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

#ifndef AMBIENT_CONTROLLERS_VELVET_CFUNCTOR
#define AMBIENT_CONTROLLERS_VELVET_CFUNCTOR

namespace ambient { namespace controllers { namespace velvet {
    
    using ambient::models::velvet::revision;
    using ambient::models::velvet::transformable;
    using ambient::channels::mpi::request;

    class cfunctor {
        typedef ambient::bulk_allocator<cfunctor*> allocator;
    public:
        virtual void invoke() = 0;
        virtual bool ready() = 0;
        #if defined(AMBIENT_COMPUTATIONAL_DATAFLOW) || defined(AMBIENT_TRACKING)
        virtual const char* name() = 0;
        size_t id;
        #endif
        void queue(cfunctor* d){ deps.push_back(d); }
        std::vector<cfunctor*, allocator> deps;
        void* arguments[1]; // note: trashing the vtptr of derived object
    };

    template<class T> class get {};
    template<class T> class set {};

    // {{{ revision get/set

    class set<revision> : public cfunctor {
    public:
        void* operator new (size_t size, void* placement){ return placement; }
        void* operator new (size_t size){ return ambient::pool::malloc<bulk,set>(); }
        void operator delete (void*, void*){ }
        void operator delete (void* ptr){ }
        static set<revision>& spawn(revision& r);
        set(revision& r);
        virtual void operator >> (int p);
        virtual bool ready();
        virtual void invoke();
        #if defined(AMBIENT_COMPUTATIONAL_DATAFLOW) || defined(AMBIENT_TRACKING)
        virtual const char* name(){ return "set"; }
        #endif
    private:
        revision* target;
        request* handle;
        bool evaluated;
        bool active;
        size_t clock;
        int sid;
    };

    struct assistance {
        assistance():handle(-1),valid(false){ 
            this->states = (bool*)ambient::pool::malloc<bulk>(ambient::channel.dim()*sizeof(bool));
            memset(this->states, 0, ambient::channel.dim()*sizeof(bool));
        }
        void operator += (int rank){
            if(!states[rank]){
                if(valid && handle == -1) handle = rank;
                if(ambient::rank() == rank) valid = true; 
                states[rank] = true;
            }
        }
        bool* states;
        bool valid;
        int handle;
        int sid;
    };

    class get<revision> : public cfunctor {
    public:
        void* operator new (size_t size, void* placement){ return placement; }
        void* operator new (size_t size){ return ambient::pool::malloc<bulk,get>(); }
        void operator delete (void*, void*){ }
        void operator delete (void* ptr){ }
        static void spawn(revision& r);
        static void assist(revision& r, int rank);
        get(revision& r);
        virtual bool ready();
        virtual void invoke();
        #if defined(AMBIENT_COMPUTATIONAL_DATAFLOW) || defined(AMBIENT_TRACKING)
        virtual const char* name(){ return "get"; }
        #endif
    private:
        revision* target;
        request* handle;
    };

    // }}}
    // {{{ transformable broadcast get/set

    class get<transformable> : public cfunctor {
    public:
        void* operator new (size_t size){ return ambient::pool::malloc<bulk,get>(); }
        void operator delete (void* ptr){ }
        static void spawn(transformable& v, int owner);
        get(transformable& v, int owner);
        virtual bool ready();
        virtual void invoke();
        #if defined(AMBIENT_COMPUTATIONAL_DATAFLOW) || defined(AMBIENT_TRACKING)
        virtual const char* name(){ return "get"; }
        #endif
    private:
        bool evaluated;
        request* handle;
        transformable* target;
        int sid;
    };

    class set<transformable> : public cfunctor {
    public:
        void* operator new (size_t size){ return ambient::pool::malloc<bulk,set>(); }
        void operator delete (void* ptr){ }
        static void spawn(transformable& v, int owner);
        set(transformable& v);
        virtual bool ready();
        virtual void invoke();
        #if defined(AMBIENT_COMPUTATIONAL_DATAFLOW) || defined(AMBIENT_TRACKING)
        virtual const char* name(){ return "set"; }
        #endif
    private:
        request* handle;
        transformable* target;
        int sid;
    };

    // }}}

} } }

#endif
