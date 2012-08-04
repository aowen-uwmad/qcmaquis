namespace ambient { namespace models { namespace velvet {

    inline void* revision::operator new (size_t size){
        return boost::singleton_pool<ambient::utils::empty, sizeof(revision)>::malloc(); 
    }

    inline void revision::operator delete (void* ptr){
        boost::singleton_pool<ambient::utils::empty, sizeof(revision)>::free(ptr); 
    }

    inline revision::revision(memspec* spec, bool clean)
    : spec(spec), clean(clean), generator(NULL) { 
    }

    inline revision::~revision(){
        free(content.header);
    }

    inline void revision::embed(void* memory, size_t bound){
        this->content.set_memory(memory, bound);
    }

    inline void revision::set_generator(void* m){
        this->generator = m;
    }

    inline void revision::reset_generator(){
        this->generator = NULL;
    }

    inline void* revision::get_generator(){
        return this->generator;
    }

    // {{{ revision::entry //

    inline revision::entry::entry()
    : header(NULL), data(NULL){ }

    inline void revision::entry::swap(entry& e){
        this->data = e.data;
        this->header = e.header;
        e.header = NULL;
    }

    inline void revision::entry::set_memory(void* memory, size_t bound){
        this->header = memory;
        this->data = (void*)((size_t)memory + bound);
    }

    inline void* revision::entry::get_memory(){
        return this->header;
    }

    inline bool revision::entry::valid(){
        return (this->data != NULL);
    }

    inline bool revision::entry::occupied(){
        return (this->data == NULL);
    }

    // }}}

} } }
