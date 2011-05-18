// nested inside ambient.hpp in ambient namespace


template <typename T, policy P>  
void breakdown_model(void_pt* profile, const p_dense_matrix<T,P>* ptr)
{
    if(ptr == NULL){
        profile->state  = PROXY;
        profile->dim.x  = 0;
        profile->dim.y  = 0;
    }else{
        profile->t_size = sizeof(T);
        profile->dim.x  = ptr->num_cols();
        profile->dim.y  = ptr->num_rows();
    }
}

template <typename T, policy P>
void bind_model(const p_dense_matrix<T,P>* ptr)
{
    ptr->set_init(value_i<T>); // ptr is self here (shouldn't be a problem)
    ptr->breakdown()->set_dim(dim2((unsigned int)ptr->num_cols(), (unsigned int)ptr->num_rows()));
} 

template <typename T, policy P>
void copy_bind_model(const p_dense_matrix<T,P>* ptr)
{
    ptr->set_init(value_i<T>); // ptr is self here (shouldn't be a problem)
    ptr->breakdown()->set_dim(dim2((unsigned int)ptr->num_cols(), (unsigned int)ptr->num_rows()));
} 

template <typename T, policy P>
void breakdown_proxy_model(void_pt* proxy, void_pt* profile, const p_dense_matrix<T,P>* ptr)
{
    proxy->profile      = profile;
    proxy->group_id     = profile->group_id;
    proxy->id           = profile->id;
    proxy->scope        = profile->scope;
    proxy->layout       = profile->layout;         // pointer
    proxy->dim          = profile->dim;
    proxy->t_size       = profile->t_size; 
    proxy->packet_type  = profile->packet_type;    // pointer
    proxy->state        = GENERIC;                 // spike for now
    proxy->imitate(profile); 
    proxy->state        = PROXY;
}


// size_t //
template<> // constant size_t //
void_pt& breakdown(const size_t& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, const size_t *ptr){  }
template<> // non-constant size_t //
void_pt& breakdown(size_t& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, size_t *ptr){  }
template<> // constant pointer to size_t //
void_pt& breakdown(size_t*& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, size_t*const *ptr){  }
template<> // constant pointer to constant size_t //
void_pt& breakdown(const size_t*& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, const size_t*const *ptr){  }

// int //
template<> // constant int //
void_pt& breakdown(const int& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, const int *ptr){  }
template<> // non-constant int //
void_pt& breakdown(int& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, int *ptr){  }
template<> // constant pointer to int //
void_pt& breakdown(int*& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, int*const *ptr){  }
template<> // constant pointer to constant int //
void_pt& breakdown(const int*& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, const int*const *ptr){  }

// double //
template<> // constant double //
void_pt& breakdown(const double& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, const double *ptr){  }
template<> // non-constant double //
void_pt& breakdown(double& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, double *ptr){  }
template<> // constant pointer to double //
void_pt& breakdown(double*& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, double*const *ptr){  }
template<> // constant pointer to constant double //
void_pt& breakdown(const double*& obj){ return *(new void_pt(&obj)); }
void breakdown_model(void_pt *profile, const double*const *ptr){  }



template<>
void plus_reduce< p_dense_matrix<double> >(memblock* grp, void* update){
    double* a = (double*)grp->data;
    double* u = (double*)update;

    int n = grp->get_mem_t_dim().x;
    int m = grp->get_mem_t_dim().y;
    int ld = m;

    for(int i=0; i<m*n; i++) a[i] += u[i];
}

