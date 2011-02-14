
    template <typename T> p_profile::p_profile(const T* ptr, void*(*init_fp)(workgroup* grp)) 
    : p_profile_s()
    {
        this->specific = false;
        this->profile = this;
        this->init_fp = init_fp;
        p_profile_model(this, ptr);
        this->regroup(); 
    };

    template <typename ST, typename FL, typename FC, class T1, class T2> 
    const ST push(FL l_kernel, FC c_kernel, const T1& arg1, const T2& arg2){
	p_profile* handle = new p_profile((const ST*)NULL);
	ST out(handle);
	push(l_kernel, c_kernel, arg1, arg2, out);
	return out;
    }

    template <typename FL, typename FC, class T1, class T2, class ST>
    void push(FL l_kernel, FC c_kernel, const T1& arg1, const T2& arg2, const ST& structuring_arg){
        if(get_profile(arg1)->proxy) pin(const_cast<T1&>(arg1), structuring_arg);
        if(get_profile(arg2)->proxy) pin(const_cast<T2&>(arg2), structuring_arg);

        ambient::engine.push(new core::operation(l_kernel, get_profile(arg1), get_profile(arg2), get_profile(structuring_arg)) , 
                             new core::operation(c_kernel, get_profile(arg1), get_profile(arg2), get_profile(structuring_arg)) ); 
    }

    template <typename L, typename R>
    void pin(L& proxy_object, const R& real_object){
        p_profile* proxy = get_profile(proxy_object);
        p_profile* real  = get_profile(real_object);
        proxy->profile   = real->profile;
        proxy->dim       = real->dim;
// copy proxy settings to the actual profile
        real->specific   = proxy->specific;
        real->dim_distr  = proxy->dim_distr;
        real->dim_group  = proxy->dim_group;
        real->dim_item   = proxy->dim_item;
        real->dim_gpu    = proxy->dim_gpu;
        real->regroup();
    }

    template <typename L, typename R>
    void copy(const L& lhs, const R& rhs){
//        new p_action('|', &lhs, &rhs);
    }
