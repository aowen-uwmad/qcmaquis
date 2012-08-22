#ifndef AMBIENT_NUMERIC_MATRIX_KERNELS
#define AMBIENT_NUMERIC_MATRIX_KERNELS
#define pin this->pin

namespace ambient { namespace numeric { namespace kernels {

    using ambient::numeric::matrix;
    using ambient::numeric::weak_view;

    template<class ViewA, class ViewB, typename T>
    struct gemm : public kernel< gemm<ViewA, ViewB, T> > 
    { // gs
        typedef void(gemm::*F)(const matrix<T>&, const matrix<T>&, weak_view<T>&);

        inline void l(const matrix<T>& a, const matrix<T>& b, weak_view<T>& c){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(b));
            assign(current(c));
        }
        inline void c(const matrix<double>& a, const matrix<double>& b, weak_view<double>& c){
            __A_TIME_C("ambient_gemm_general_c_kernel"); 
            double* ad = c_current(a);
            double* bd = c_current(b);
            double* cd = w_updated(c);
            int m = ViewA::rows(a);
            int k = ViewA::cols(a);
            int n = ViewB::cols(b);
            int lda = __a_get_dim(a).y;
            int ldb = __a_get_dim(b).y;
            int ldc = __a_get_dim(c).y;
            static const double alpha(1.0); 
            static const double beta(0.0);
            dgemm_(ViewA::code(), ViewB::code(), &m, &n, &k, &alpha, ad, &lda, bd, &ldb, &beta, cd, &ldc);
            __A_TIME_C_STOP
        }
        inline void c(const matrix<std::complex<double> >& a, const matrix<std::complex<double> >& b, weak_view<std::complex<double> >& c){
            __A_TIME_C("ambient_gemm_general_c_kernel"); 
            T* ad   = c_current(a);
            T* bd   = c_current(b);
            T* cd   = w_updated(c);
            int m   = __a_get_dim(a).y;
            int n   = __a_get_dim(b).x;
            int k   = __a_get_dim(b).y;
            T alpha(1.0); 
            T beta(0.0);
            __a_gemm("N","N", &m, &n, &k, &alpha, ad, &m, bd, &k, &beta, cd, &m);
            __A_TIME_C_STOP
        }
    };
        
    template<class ViewB, typename T, typename D>
    struct gemm_diagonal_lhs : public kernel< gemm_diagonal_lhs<ViewB,T,D> > 
    {
        typedef void (gemm_diagonal_lhs::*F)(const matrix<D>&, const matrix<T>&, weak_view<T>&);

        inline void l(const matrix<D>& a_diag, const matrix<T>& b, weak_view<T>& c){
            pin(current(b)); //if(!ctxt.involved()) return;
            assign(current(a_diag));
            assign(current(c));
        }

        inline void c(const matrix<D>& a_diag, const matrix<T>& b, weak_view<T>& c){
            // gs
            __A_TIME_C("ambient_gemm_diagonal_lhs_c_kernel"); 
            int sizey = __a_get_dim(a_diag).y;
            int size = __a_get_dim(b).x;
            int ONE  = 1;
            D* bd = c_current(b);
            D* cd = p_updated(c);
            D* alpha = c_current(a_diag);
        
            for(int k = 0 ; k < sizey; k++){
        	     axpy(&size, &alpha[k], &bd[k], &sizey, &cd[k], &sizey);
            }
            __A_TIME_C_STOP
        }
    };
        
    template<typename T, typename D>
    struct gemm_diagonal_lhs<transpose_view<matrix<T> >,T,D> : public kernel< gemm_diagonal_lhs<transpose_view<matrix<T> >,T,D> > 
    {
        typedef void (gemm_diagonal_lhs::*F)(const matrix<D>&, const matrix<T>&, weak_view<T>&);

        inline void l(const matrix<D>& a_diag, const matrix<T>& b, weak_view<T>& c){
            pin(current(b)); //if(!ctxt.involved()) return;
            assign(current(a_diag));
            assign(current(c));
        }

        inline void c(const matrix<D>& a_diag, const matrix<T>& b, weak_view<T>& c){
            // gs
            __A_TIME_C("ambient_gemm_diagonal_lhs_c_kernel"); 
            printf("Special DIAGONAL!\n");
            size_t sizex = __a_get_dim(b).x;
            int size  = __a_get_dim(a_diag).y;
            int ONE  = 1;
            D* bd = c_current(b);
            D* cd = p_updated(c);
            D* alpha = c_current(a_diag);
        
            for(int k = 0 ; k < sizex; k++){
        	     axpy(&size, &alpha[k], &bd[k*size], &ONE, &cd[k], &size);
            }
            __A_TIME_C_STOP
        }
    };
        
    template<class ViewA, typename T, typename D>
    struct gemm_diagonal_rhs : public kernel< gemm_diagonal_rhs<ViewA,T,D> > 
    {
        typedef void (gemm_diagonal_rhs::*F)(const matrix<T>&, const matrix<D>&, weak_view<T>&);

        inline void l(const matrix<T>& a, const matrix<D>& b_diag, weak_view<T>& c){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(b_diag));
            assign(current(c));
        }

        inline void c(const matrix<T>& a, const matrix<D>& b_diag, weak_view<T>& c){
            // gs
            __A_TIME_C("ambient_gemm_diagonal_rhs_c_kernel"); 
            size_t sizex = __a_get_dim(b_diag).y;
            int size = __a_get_dim(a).y; // for the case of complex
            int ONE = 1;
            D* ad = c_current(a);
            D* cd = p_updated(c);
        	D* alpha = c_current(b_diag);
        
            for(int k = 0 ; k < sizex; k++){
        	    axpy(&size, &alpha[k], &ad[k*size], &ONE, &cd[k*size], &ONE);
            }
            __A_TIME_C_STOP
        }
    };

    template<typename T, typename D>
    struct gemm_diagonal_rhs<transpose_view<matrix<T> >,T,D> : public kernel< gemm_diagonal_rhs<transpose_view<matrix<T> >,T,D> > 
    {
        typedef void (gemm_diagonal_rhs::*F)(const matrix<T>&, const matrix<D>&, weak_view<T>&);

        inline void l(const matrix<T>& a, const matrix<D>& b_diag, weak_view<T>& c){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(b_diag));
            assign(current(c));
        }

        inline void c(const matrix<T>& a, const matrix<D>& b_diag, weak_view<T>& c){
            // gs
            __A_TIME_C("ambient_gemm_diagonal_rhs_c_kernel"); 
            printf("Special DIAGONAL!\n");
            int sizey = __a_get_dim(b_diag).y;
            int size = __a_get_dim(a).x;
            int ONE = 1;
            D* ad = c_current(a);
            D* cd = p_updated(c);
        	D* alpha = c_current(b_diag);
        
            for(int k = 0 ; k < sizey; k++){
        	    axpy(&size, &alpha[k], &ad[k], &sizey, &cd[k*size], &ONE);
            }
            __A_TIME_C_STOP
        }
    };


    template<typename T>
    struct copy : public kernel< copy<T> > 
    { // gs
        typedef void(copy::*F)(weak_view<T>&, const matrix<T>&);

        inline void l(weak_view<T>& ac, const matrix<T>& a){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(ac));
        }

        inline void c(weak_view<T>& ac, const matrix<T>& a){
            __A_TIME_C("ambient_copy_c_kernel"); 
            T* ad  = c_current(a);
            T* acd  = w_updated(ac);
            memcpy(acd, ad, __a_sizeof(a));
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct op_kron : public kernel< op_kron<T> > 
    { // gs - 2su
        typedef void (op_kron::*F)(matrix<T>&, const matrix<T>&, const matrix<T>&,
                                          const size_t&, const size_t&, 
                                          const size_t&, const size_t&,
                                          const size_t&, const size_t&);

        inline void l(matrix<T>& out, const matrix<T>& in, const matrix<T>& alfa,
                      const size_t& out_y_offset, const size_t& out_x_offset, 
                      const size_t& ldim1, const size_t& ldim2, 
                      const size_t& rdim1, const size_t& rdim2)
        {
            pin(current(out)); //if(!ctxt.involved()) return;
            assign(current(in));
            assign(current(alfa));
        }

        inline void c(matrix<T>& out, const matrix<T>& in, const matrix<T>& alfa,
                      const size_t& out_y_offset, const size_t& out_x_offset, 
                      const size_t& ldim1, const size_t& ldim2, 
                      const size_t& rdim1, const size_t& rdim2)
        {
            __A_TIME_C("ambient_op_kron_c_kernel"); 
            T* alfad = c_current(alfa);
            for(size_t l1 = 0; l1 < ldim1; ++l1)
            for(size_t r1 = 0; r1 < rdim1; ++r1)
            __a_memptf_r<T, __a_memscal>(r_updated(out), __a_get_dim(out).y, dim2(out_x_offset + r1*rdim2, out_y_offset + l1*ldim2),
                                                c_current(in), __a_get_dim(in).y,  dim2(0, 0), 
                                                dim2(rdim2, ldim2), alfad[l1 + r1*__a_get_dim(alfa).y]);
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct reshape_l2b : public kernel< reshape_l2b<T> > 
    { // gs - 2su
        typedef void (reshape_l2b::*F)(matrix<T>&, const matrix<T>&,
                                              const size_t&, const size_t&, 
                                              const size_t&, const size_t&, 
                                              const size_t&, const size_t&,
                                              const size_t&, const size_t&);

        inline void l(matrix<T>& out, const matrix<T>& in,
                      const size_t& in_left_offset, const size_t& in_phys_offset, 
                      const size_t& out_left_offset, const size_t& out_x_offset,
                      const size_t& sdim1, const size_t& sdim2, 
                      const size_t& ldim, const size_t& rdim)
        {
            pin(current(out)); //if(!ctxt.involved()) return;
            assign(current(in));
        }

        inline void c(matrix<T>& out, const matrix<T>& in,
                      const size_t& in_left_offset, const size_t& in_phys_offset, 
                      const size_t& out_left_offset, const size_t& out_x_offset,
                      const size_t& sdim1, const size_t& sdim2, 
                      const size_t& ldim, const size_t& rdim)
        {
            __A_TIME_C("ambient_reshape_l2b_c_kernel"); 

            size_t in_y_offset  = in_left_offset + ldim*in_phys_offset;
            size_t out_y_offset = out_left_offset;

            __a_refresh<T>(w_updated(out), c_current(out), __a_sizeof(out));
            for(size_t ss1 = 0; ss1 < sdim1; ++ss1){
                for(size_t ss2 = 0; ss2 < sdim2; ++ss2){
                    __a_memptf_r<T, __a_memcpy>(r_updated(out), __a_get_dim(out).y, dim2(out_x_offset + rdim*ss2, out_y_offset), 
                                                       c_current(in), __a_get_dim(in).y,  dim2(0, in_y_offset), 
                                                       dim2( rdim, ldim ));
                    in_y_offset += ldim;
                }
                out_y_offset += ldim;
            }
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct reshape_b2l : public kernel< reshape_b2l<T> > 
    { // gs - 2su
        typedef void (reshape_b2l::*F)(matrix<T>&, const matrix<T>&,
                                              const size_t&, const size_t&, 
                                              const size_t&, const size_t&, 
                                              const size_t&, const size_t&,
                                              const size_t&, const size_t&);

        inline void l(matrix<T>& out, const matrix<T>& in,
                      const size_t& in_left_offset, const size_t& in_x_offset, 
                      const size_t& out_left_offset, const size_t& out_phys_offset,
                      const size_t& sdim1, const size_t& sdim2, 
                      const size_t& ldim, const size_t& rdim)
        {
            pin(current(out)); //if(!ctxt.involved()) return;
            assign(current(in));
        }

        inline void c(matrix<T>& out, const matrix<T>& in,
                      const size_t& in_left_offset, const size_t& in_x_offset, 
                      const size_t& out_left_offset, const size_t& out_phys_offset,
                      const size_t& sdim1, const size_t& sdim2, 
                      const size_t& ldim, const size_t& rdim)
        {
            __A_TIME_C("ambient_reshape_b2l_c_kernel"); 

            size_t in_y_offset  = in_left_offset;
            size_t out_y_offset = out_left_offset + out_phys_offset*ldim;

            __a_refresh<T>(w_updated(out), c_current(out), __a_sizeof(out));
            for(size_t ss1 = 0; ss1 < sdim1; ++ss1){
                for(size_t ss2 = 0; ss2 < sdim2; ++ss2)
                {
                    __a_memptf_r<T, __a_memcpy>(r_updated(out), __a_get_dim(out).y, dim2(0, out_y_offset), 
                                                       c_current(in), __a_get_dim(in).y,  dim2(in_x_offset + rdim*ss2, in_y_offset), 
                                                       dim2( rdim, ldim ));
                    out_y_offset += ldim;
                }
                in_y_offset += ldim;
            }
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct reshape_l2r : public kernel< reshape_l2r<T> > 
    { // gs
        typedef void (reshape_l2r::*F)(const matrix<T>&, matrix<T>&,
                                              const size_t&, const size_t&, 
                                              const size_t&, const size_t&, const size_t&);

        inline void l(const matrix<T>& left, matrix<T>& right,
                      const size_t& left_offset, const size_t& right_offset, 
                      const size_t& sdim, const size_t& ldim, const size_t& rdim)
        {
            pin(current(right)); //if(!ctxt.involved()) return;
            assign(current(left));
        }

        inline void c(const matrix<T>& left, matrix<T>& right,
                      const size_t& left_offset, const size_t& right_offset, 
                      const size_t& sdim, const size_t& ldim, const size_t& rdim)
        {
            __A_TIME_C("ambient_reshape_l2r_c_kernel"); 
            __a_refresh<T>(w_updated(right), c_current(right), __a_sizeof(right));
            for(size_t ss = 0; ss < sdim; ++ss){
                __a_memptf_r<T, __a_memcpy>(r_updated(right), __a_get_dim(right).y, dim2(ss*rdim + right_offset, 0), 
                                                   c_current(left), __a_get_dim(left).y,  dim2(0, ss*ldim + left_offset), 
                                                   dim2( rdim, ldim ));
            }
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct reshape_r2l : public kernel< reshape_r2l<T> > 
    { // gs
        typedef void (reshape_r2l::*F)(matrix<T>&, const matrix<T>&,
                                              const size_t&, const size_t&, 
                                              const size_t&, const size_t&, const size_t&);

        inline void l(matrix<T>& left, const matrix<T>& right,
                      const size_t& left_offset, const size_t& right_offset, 
                      const size_t& sdim, const size_t& ldim, const size_t& rdim)
        {
            pin(current(left)); //if(!ctxt.involved()) return;
            assign(current(right));
        }

        inline void c(matrix<T>& left, const matrix<T>& right,
                      const size_t& left_offset, const size_t& right_offset, 
                      const size_t& sdim, const size_t& ldim, const size_t& rdim)
        {
            __A_TIME_C("ambient_reshape_r2l_c_kernel"); 
            __a_refresh<T>(w_updated(left), c_current(left), __a_sizeof(left));
            for(size_t ss = 0; ss < sdim; ++ss)
                __a_memptf_r<T, __a_memcpy>(r_updated(left), __a_get_dim(left).y,  dim2(0, ss*ldim + left_offset), 
                                                   c_current(right), __a_get_dim(right).y, dim2(ss*rdim + right_offset,0), 
                                                   dim2( rdim, ldim ));
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct lb_tensor_mpo : public kernel< lb_tensor_mpo<T> > 
    { // gs
        typedef void (lb_tensor_mpo::*F)(matrix<T>&, const matrix<T>&, const matrix<T>&,
                                                const size_t&, const size_t&, 
                                                const size_t&, const size_t&, const size_t&, const size_t&);

        inline void l(matrix<T>& out, const matrix<T>& in, const matrix<T>& alfa,
                      const size_t& out_offset, const size_t& in_offset, 
                      const size_t& sdim1, const size_t& sdim2, const size_t& ldim, const size_t& rdim)
        {
            pin(current(out)); //if(!ctxt.involved()) return;
            assign(current(in));
            assign(current(alfa));
        }

        inline void c(matrix<T>& out, const matrix<T>& in, const matrix<T>& alfa,
                      const size_t& out_offset, const size_t& in_offset, 
                      const size_t& sdim1, const size_t& sdim2, const size_t& ldim, const size_t& rdim)
        {
            __A_TIME_C("ambient_lb_tensor_mpo_c_kernel"); 
            __a_refresh<T>(w_updated(out), c_current(out), __a_sizeof(out));
            T* alfad = c_current(alfa);
            for(size_t ss2 = 0; ss2 < sdim2; ++ss2)
            for(size_t ss1 = 0; ss1 < sdim1; ++ss1)
            __a_memptf_r<T, __a_memscal>(r_updated(out), __a_get_dim(out).y, dim2(0, out_offset + ss2*ldim),
                                                c_current(in), __a_get_dim(in).y,  dim2(0, in_offset + ss1*ldim),
                                                dim2(rdim, ldim), alfad[ss1 + ss2*__a_get_dim(alfa).y]);
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct rb_tensor_mpo : public kernel< rb_tensor_mpo<T> > 
    { // gs
        typedef void (rb_tensor_mpo::*F)(matrix<T>&, const matrix<T>&, const matrix<T>&,
                                                const size_t&, const size_t&, 
                                                const size_t&, const size_t&, const size_t&, const size_t&);

        inline void l(matrix<T>& out, const matrix<T>& in, const matrix<T>& alfa,
                      const size_t& out_offset, const size_t& in_offset, 
                      const size_t& sdim1, const size_t& sdim2, const size_t& ldim, const size_t& rdim)
        {
            pin(current(out)); //if(!ctxt.involved()) return;
            assign(current(in));
            assign(current(alfa));
        }

        inline void c(matrix<T>& out, const matrix<T>& in, const matrix<T>& alfa,
                      const size_t& out_offset, const size_t& in_offset, 
                      const size_t& sdim1, const size_t& sdim2, const size_t& ldim, const size_t& rdim)
        {
            __A_TIME_C("ambient_rb_tensor_mpo_c_kernel"); 
            __a_refresh<T>(w_updated(out), c_current(out), __a_sizeof(out));
            T* alfad = c_current(alfa);
            for(size_t ss2 = 0; ss2 < sdim2; ++ss2)
            for(size_t ss1 = 0; ss1 < sdim1; ++ss1)
            __a_memptf_r<T, __a_memscal>(r_updated(out), __a_get_dim(out).y, dim2(out_offset + ss2*rdim, 0),
                                                c_current(in), __a_get_dim(in).y,  dim2(in_offset + ss1*rdim, 0),
                                                dim2(rdim, ldim), alfad[ss1 + ss2*__a_get_dim(alfa).y]);
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct trace : public kernel< trace<T> > 
    {
        typedef void (trace::*F)(const matrix<T>&, future<T>&);

        inline void l(const matrix<T>& a, future<T>& trace){
            pin(current(a));
        }

        inline void c(const matrix<T>& a, future<T>& trace){
            // gs
            __A_TIME_C("ambient_trace_c_kernel"); 
            size_t m = __a_get_dim(a).y;
            size_t n = __a_get_dim(a).x;
            T* ad = c_current(a);
        
            size_t sizex = std::min(n,m);
            for(size_t jj = 0; jj < sizex; jj++){
                trace.get_value() += ad[jj + jj*m];
            }
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct scalar_norm : public kernel< scalar_norm<T> > 
    {// gs
        typedef void (scalar_norm::*F)(const matrix<T>&, future<double>&);

        inline void l(const matrix<T>& a, future<double>& norm){
            pin(current(a));
        }

        inline void c(const matrix<T>& a, future<double>& norm){
            __A_TIME_C("ambient_scalar_norm_c_kernel"); 
            T* ad = c_current(a);
            norm.get_value() = alps::numeric::real(__a_dot(ad, ad, __a_get_dim(a).square()));
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct overlap : public kernel< overlap<T> > 
    { // gs
        typedef void (overlap::*F)(const matrix<T>&, const matrix<T>&, future<T>&);

        inline void l(const matrix<T>& a, const matrix<T>& b, future<T>& overlap){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(b));
        }

        inline void c(const matrix<T>& a, const matrix<T>& b, future<T>& overlap){
            __A_TIME_C("ambient_scalar_overlap_c_kernel"); 
            T* ad = c_current(a);
            T* bd = c_current(b);
            overlap.get_value() = __a_dot(ad, bd, __a_get_dim(a).square());
            __A_TIME_C_STOP
        }
    };

        
    template<typename T>
    struct add : public kernel< add<T> > 
    { // gs
        typedef void (add::*F)(matrix<T>&, const matrix<T>&);

        inline void l(matrix<T>& a, const matrix<T>& b){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(b));
        }

        inline void c(matrix<T>& a, const matrix<T>& b){
            __A_TIME_C("ambient_add_c_kernel"); 
            T* ad = c_current(a);
            T* bd = c_current(b);
            T* ar = r_updated(a);
            int size = __a_get_dim(a).square();
            static const T sign = 1.;
            static const int ONE = 1;
            axpy(&size, &sign, bd, &ONE, ar, &ONE);
            __A_TIME_C_STOP
        }
    };

        
    template<typename T>
    struct sub : public kernel< sub<T> > 
    { // gs
        typedef void (sub::*F)(matrix<T>&, const matrix<T>&);

        inline void l(matrix<T>& a, const matrix<T>& b){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(b));
        }

        inline void c(matrix<T>& a, const matrix<T>& b){
            __A_TIME_C("ambient_sub_c_kernel"); 
            T* bd = c_current(b);
            T* ar = r_updated(a);
            int size = __a_get_dim(a).square();
            static const T sign = -1.;
            static const int ONE = 1;
            axpy(&size, &sign, bd, &ONE, ar, &ONE);
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct scale : public kernel< scale<T> > 
    { // gs
        typedef void (scale::*F)(matrix<T>&, const future<T>&);

        inline void l(matrix<T>& a, const future<T>& t){
            pin(current(a));
        }

        inline void c(matrix<double>& a, const future<double>& t){
            __A_TIME_C("ambient_scale_c_kernel"); 
            T* ar = r_updated(a);
            int size = __a_get_dim(a).square();
            static const int ONE = 1;
            dscal_( &size, &t.get_value(), ar, &ONE );
            __A_TIME_C_STOP
        }

        inline void c(matrix<std::complex<double> >& a, const future< std::complex<double> >& t){
            __A_TIME_C("ambient_scale_c_kernel"); 
            T* ad = c_current(a);
            T* ar = w_updated(a);
            int size = __a_get_dim(a).square();
            for(int k=0; k < size; k++) 
                ar[k] = ad[k] * t.get_value();
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct scale_inverse : public kernel< scale_inverse<T> > 
    { // gs
        typedef void (scale_inverse::*F)(matrix<T>&, const future<T>&);

        inline void l(matrix<T>& a, const future<T>& t){
            pin(current(a));
        }

        inline void c(matrix<double>& a, const future<double>& t){
            __A_TIME_C("ambient_scale_inverse_c_kernel"); 
            T* ar = r_updated(a);
            int size = __a_get_dim(a).square();
            static const int ONE = 1;
            double factor = 1. / t.get_value();
            dscal_( &size, &factor, ar, &ONE );
            __A_TIME_C_STOP
        }

        inline void c(matrix<std::complex<double> >& a, const future< std::complex<double> >& t){
            __A_TIME_C("ambient_scale_inverse_c_kernel"); 
            T* ad = c_current(a);
            T* ar = w_updated(a);
            int size = __a_get_dim(a).square();
            for(int k=0; k < size; k++) 
                ar[k] = ad[k] / t.get_value();
            __A_TIME_C_STOP
        }
    };

    template<typename T>
    struct transpose_out : public kernel< transpose_out<T> > 
    { // gs
        typedef void (transpose_out::*F)(const matrix<T>&, weak_view<T>&);

        inline void l(const matrix<T>& a, weak_view<T>& t){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(t));
        }

        inline void c(const matrix<T>& a, weak_view<T>& t){
            __A_TIME_C("ambient_transpose_out_c_kernel"); 
            T* od = c_current(a);
            T* td = w_updated(t);
            int m = __a_get_dim(a).y;
            int n = __a_get_dim(a).x;

            for(int i = 0; i < m; i++){
                for(int j = 0; j < n; j++) *td++ = od[j*m];
                od++;
            }
            __A_TIME_C_STOP
        }
    };

    template<typename T>
    struct resize : public kernel< resize<T> > 
    {
        typedef void (resize::*F)(weak_view<T>&, const matrix<T>&, const size_t&, const size_t&);

        inline void l(weak_view<T>& r, const matrix<T>& a, const size_t& m, const size_t& n){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(r));
        }

        inline void c(weak_view<T>& r, const matrix<T>& a, const size_t& m, const size_t& n){
            __A_TIME_C("ambient_resize_c_kernel"); 
            __a_memptf_r<T, __a_memcpy>(r_updated(r), __a_get_dim(r).y, dim2(0,0), 
                                               c_current(a), __a_get_dim(a).y, dim2(0,0), dim2(n, m)); 
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct init_identity : public kernel< init_identity<T> > 
    {
        typedef void (init_identity::*F)(weak_view<T>&);

        inline void l(weak_view<T>& a){
            pin(current(a));
        }

        inline void c(weak_view<T>& a){
            __A_TIME_C("ambient_init_identity_c_kernel"); 
            size_t n = __a_get_dim(a).x;
            size_t m = __a_get_dim(a).y;
            T* ad = r_updated(a);

            size_t sizex = std::min(m,n); // respecting borders
            for(size_t jj = 0; jj < sizex; ++jj) ad[jj + m*jj] = 1.;
            __A_TIME_C_STOP
        }
    };
       
    template<typename T>
    struct init_random : public kernel< init_random<T> > 
    {
        typedef void (init_random::*F)(weak_view<T>&);
     
        template<typename T> inline void randomize(T* ad){ *ad = drand48(); }
        template<typename T> inline void randomize(std::complex<T>* ad){
            ad->real(drand48());
            ad->imag(drand48());
        }

        inline void l(weak_view<T>& a){
            pin(current(a));
        }
        
        inline void c(weak_view<T>& a){
            __A_TIME_C("ambient_init_random_c_kernel"); 
            size_t m = __a_get_dim(a).y;
            size_t n = __a_get_dim(a).x;
            T* ad = w_updated(a);
          
            for(size_t jj = 0; jj < n; jj++){
                for(size_t ii = 0; ii < m; ii++)
                    randomize((ad+(jj*m+ii)));
            }
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct init_value : public kernel< init_value<T> > 
    {
        typedef void (init_value::*F)(weak_view<T>&, const T&);

        inline void l(weak_view<T>& a, const T& value){
            pin(current(a));
        }

        inline void c(weak_view<T>& a, const T& value){
            __A_TIME_C("ambient_init_value_c_kernel"); 
            T* ad = w_updated(a);
            size_t m = __a_get_dim(a).y;
            size_t n = __a_get_dim(a).x;
        
            for(size_t j=0; j < n; ++j){
                for(size_t i = 0; i < m; ++i){
                    ad[i+j*m] = value; // not a memset due to complex
                }
            }
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct round_square : public kernel< round_square<T> > 
    {
        typedef void (round_square::*F)(const matrix<T>&, std::vector<T>*&);

        inline void l(const matrix<T>& a, std::vector<T>*& ac){
            pin(current(a));
        }

        inline void c(const matrix<T>& a, std::vector<T>*& ac){
            // gs
            __A_TIME_C("ambient_round_square_c_kernel"); 
            T* ad = c_current(a);
            size_t sizey = __a_get_dim(a).y;
            for(int i=0; i < sizey; i++){
                double v = std::abs(ad[i]);
                if(v > 1e-10) ac->push_back(v*v);
            }
            __A_TIME_C_STOP
        }
    };

    template<typename T>
    struct cast_to_vector : public kernel< cast_to_vector<T> > 
    {
        typedef void (cast_to_vector::*F)(std::vector<T>*&, const matrix<T>&, const size_t&, const size_t&, const size_t&);

        inline void l(std::vector<T>*& ac, const matrix<T>& a, const size_t& m, const size_t& n, const size_t& offset){
            pin(current(a));
        }

        inline void c(std::vector<T>*& ac, const matrix<T>& a, const size_t& m, const size_t& n, const size_t& offset){
            // gs
            __A_TIME_C("ambient_cast_to_vector_c_kernel"); 
            T* ad = c_current(a);
            for(int j=0; j < n; ++j) memcpy((void*)&(*ac)[j*m + offset],(void*)&ad[j*m], m*sizeof(T));  
            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct cast_from_vector : public kernel< cast_from_vector<T> > 
    {
        typedef void (cast_from_vector::*F)(const std::vector<T>*&, matrix<T>&, const size_t&, const size_t&, const size_t&);

        inline void l(const std::vector<T>*& ac, matrix<T>& a, const size_t& m, const size_t& n, const size_t& lda){
            pin(current(a));
        }

        inline void c(const std::vector<T>*& ac, matrix<T>& a, const size_t& m, const size_t& n, const size_t& lda){
            __A_TIME_C("ambient_cast_from_vector_c_kernel"); 
            T* ad = w_updated(a);
            for(int j=0; j < n; ++j) memcpy((void*)&ad[j*m],(void*)&(*ac)[j*lda], m*sizeof(T));
            __A_TIME_C_STOP 
        }
    };

    template<typename T>
    struct validation : public kernel< validation<T> > 
    {
        typedef void (validation::*F)(const matrix<T>&, const matrix<T>&, future<bool>&);

        inline void l(const matrix<T>& a, const matrix<T>& b, future<bool>& ret){
            pin(current(a));  //if(!ctxt.involved()) return;
            assign(current(b)); 
        }
        
        inline void c(const matrix<T>& a, const matrix<T>& b, future<bool>& ret){ // see paper for Reference Dongara 
            T* ad = c_current(a); 
            T* bd = c_current(b); 
            double res; 
            double epsilon = std::numeric_limits<double>::epsilon();
            size_t position_xy; 
       
            for(size_t ii=0; ii < __a_get_dim(a).y; ++ii){
                for(size_t jj=0; jj < __a_get_dim(a).x; ++jj){
                    position_xy = jj*__a_get_dim(a).y+ii;
                    res = (norm(ad[position_xy])-norm(bd[position_xy]))/fabs(epsilon*norm(bd[position_xy])); // to do : rotation pb  with complex to change
                    if(res > 256){ // 16 is recommended by Dongara, 256 because lapack gives != runs after runs
                        std::cout << ii << " " << jj << " : " << ad[position_xy] << " " << bd[position_xy] << std::endl;
                        ret.get_value() = false; // test failed return 0 (bool false)
                    }
                }
            }
        }
    };

    template<typename T>
    struct validation_t : public kernel< validation_t<T> > 
    {
        typedef void (validation_t::*F)(const matrix<T>&, const matrix<T>&, future<bool>&);

        inline void l(const matrix<T>& a, const matrix<T>& b, future<bool>& ret){
            pin(current(a));  //if(!ctxt.involved()) return;
            assign(current(b)); 
        }
        
        inline void c(const matrix<T>& a, const matrix<T>& b, future<bool>& ret){ // see paper for Reference Dongara 
            T* ad = c_current(a); 
            T* bd = c_current(b); 
            double res; 
            double epsilon = std::numeric_limits<double>::epsilon();
            size_t position_xy, position_xy_t; 
       
            for(size_t ii=0; ii < __a_get_dim(a).y; ++ii){
                for(size_t jj=0; jj < __a_get_dim(a).x; ++jj){
                    position_xy = jj*__a_get_dim(a).y+ii;
                    position_xy_t = ii*__a_get_dim(a).y+jj;
                    res = (norm(ad[position_xy])-norm(bd[position_xy_t]))/fabs(epsilon*norm(bd[position_xy_t])); // to do : rotation pb  with complex to change
                    if(res > 256){ // 16 is recommended by Dongara, 256 because lapack gives != runs after runs
                        std::cout << ii << " " << jj << " : " << ad[position_xy] << " " << bd[position_xy_t] << std::endl;
                        ret.get_value() = false; // test failed return 0 (bool false)
                    }
                }
            }
        }
    };

    // {{{ MKL LAPACK kernels

    template<typename T>
    struct svd : public kernel< svd<T> > 
    {
        typedef void (svd::*F)(const matrix<T>&, weak_view<T>&, weak_view<T>&, weak_view<double>&);

        inline void l(const matrix<T>& a, weak_view<T>& u, weak_view<T>& vt, weak_view<double>& s){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(s));
            assign(current(u));
            assign(current(vt));
        }

        inline void c(const matrix<T>& a, weak_view<T>& u, weak_view<T>& vt, weak_view<double>& s){
            // gs
            __A_TIME_C("ambient_svd_c_kernel"); 
            int m = __a_get_dim(a).y;
            int n = __a_get_dim(a).x;
            int k = std::min(m,n);
            int info;
            int lwork = -1; // C - Alex, netlib said -1 for the best workspace
            T wkopt;
            T* ad  = c_current(a);
            T* ud  = r_updated(u);
            T* vtd = r_updated(vt);
            double* sd  = r_updated(s);
            double* rwork; // = new double[5*m]; // C - useless for double but need for complex 
            T* work;
            gesvd( "S", "S", &m, &n, ad, &m, sd, ud, &m, vtd, &k, &wkopt, &lwork, rwork, &info ); // query and allocate the optimal workspace
            lwork = OptimalSize(wkopt);
            work = (T*)malloc( lwork*sizeof(T) );
            gesvd( "S", "S", &m, &n, ad, &m, sd, ud, &m, vtd, &k, work, &lwork, rwork, &info );   // compute SVD
            assert( info == 0 ); // otherwise the algorithm computing atomic SVD failed to converge
            free(work);
            __A_TIME_C_STOP
        }
    };

    template<typename T>
    struct qr : public kernel< qr<T> > 
    {
        typedef void (qr::*F)(const matrix<T>&, weak_view<T>&, weak_view<T>&);

        inline void l(const matrix<T>& a, weak_view<T>& q, weak_view<T>& r){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(q));
            assign(current(r));
        }

        inline void c(const matrix<T>& a, weak_view<T>& q, weak_view<T>& r){
            // gs
            __A_TIME_C("ambient_qr_c_kernel"); 
            int m = __a_get_dim(a).y; //numrow a
            int n = __a_get_dim(a).x; //numcol a, numcol r
            int k = std::min(m,n); //numrow r
            int info;
            int lwork = -1; 
            T wkopt;
            T* tau = (T*)malloc(k*sizeof(T));
            T* ad  = c_current(a);
            T* qd  = w_updated(q);
            T* rd = p_updated(r);
            T* work;
            T* more_work;
            T  kwork;

            geqrf(&m, &n, ad, &m, tau, &kwork, &lwork, &info);
            lwork = OptimalSize(kwork);
            work = (T*)malloc( lwork*sizeof(T) );
            geqrf(&m, &n, ad, &m, tau, work, &lwork, &info);
            assert( info == 0 );

            for (std::size_t c = 0; c < n; ++c)
                for (std::size_t r = 0; r <= c && r < k; ++r)
                    rd[r+k*c] = ad[r+m*c]; 

            lwork = -1;

            getq_qr(&m, &k, &k, ad, &m, tau, &kwork, &lwork, &info);

            lwork = OptimalSize(kwork);
            more_work = (T*)malloc( lwork*sizeof(T) );
            getq_qr(&m, &k, &k, ad, &m, tau, more_work, &lwork, &info);
            assert( info == 0 ); 
             
            memcpy((void*)qd, (void*)ad, k*__a_get_dim(a).y*sizeof(T)); // l 235 

            free(work);
            free(more_work);
            free(tau);

            __A_TIME_C_STOP
        }
    };
        
    template<typename T>
    struct lq : public kernel< lq<T> > 
    {
        typedef void (lq::*F)(const matrix<T>&, weak_view<T>&, weak_view<T>&);

        inline void l(const matrix<T>& a, weak_view<T>& l, weak_view<T>& q){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(l));
            assign(current(q));
        }

        inline void c(const matrix<T>& a, weak_view<T>& l, weak_view<T>& q){
            // gs
            __A_TIME_C("ambient_lq_c_kernel"); 
            int m = __a_get_dim(a).y; //numrow a, numrow l
            int n = __a_get_dim(a).x; //numcol a
            int k = std::min(m,n); //numcol l
            int info;
            int lwork = -1; 
            T wkopt;
            T* tau = (T*)malloc(k*sizeof(T));
            T* ad  = c_current(a);
            T* ld  = p_updated(l);
            T* qd  = w_updated(q);
            T* work;
            T* more_work;
            T  kwork;

            gelqf(&m, &n, ad, &m, tau, &kwork, &lwork, &info);
            lwork = OptimalSize(kwork);
            work = (T*)malloc( lwork*sizeof(T) );
            gelqf(&m, &n, ad, &m, tau, work, &lwork, &info);
            assert( info == 0 );

            for (std::size_t c = 0; c < k; ++c)
                for (std::size_t r = c; r < m ;++r)
                    ld[r+m*c] = ad[r+m*c]; 

            lwork = -1;
            getq_lq(&k, &n, &k, ad, &m, tau, &kwork, &lwork, &info);

            lwork = OptimalSize(kwork);
            more_work = (T*)malloc( lwork*sizeof(T) );

            getq_lq(&k, &n, &k, ad, &m, tau, more_work, &lwork, &info);
            assert( info == 0 ); 

            for (std::size_t c = 0; c < n; ++c)
                for (std::size_t r = 0; r < k ;++r)
                    qd[r+k*c] = ad[r+m*c]; 

            free(work);
            free(more_work);
            free(tau);

            __A_TIME_C_STOP
        }
    };

    template<typename T>
    struct heev : public kernel< heev<T> > 
    {
        typedef void (heev::*F)(matrix<T>&, weak_view<double>&);

        inline void l(matrix<T>& a, weak_view<double>& w){
            pin(current(a)); //if(!ctxt.involved()) return;
            assign(current(w));
        }

        inline void c(matrix<T>& a, weak_view<double>& w){
            // gs
            __A_TIME_C("ambient_heev_c_kernel"); 
            int m = __a_get_dim(a).y;
            int info, lwork = -1;
            double wkopt;
            double* work;
            double* ad = (double*)__a_solidify(c_current(a), __a_sizeof(a));
            double* wd = (double*)__a_solidify(c_current(w), __a_sizeof(w));

            dsyev_("V","U",&m,ad,&m,wd,&wkopt,&lwork,&info);
            lwork = (int)wkopt;
            work = (double*)malloc( lwork*sizeof(double) );
            dsyev_("V","U",&m,ad,&m,wd,work,&lwork,&info);
            assert( info == 0 ); // otherwise the algorithm computing SYEV failed to converge
            // First we reverse the eigenvalues, to be coherent with the serial version ! 
            for(int i=0; i < (int)(m/2); i++){ 
                wkopt = wd[i];
                wd[i] = wd[m-i-1];
                wd[m-i-1] = wkopt;
            } 
            // Second we reverse the eigenvectors
            size_t len = m*sizeof(double);
            work = (double*)realloc(work, len);
            for (int i=0; i < (int)(m/2); i++){ 
                memcpy(work, &ad[i*m], len);
                memcpy(&ad[i*m], &ad[(m-1-i)*m], len);
                memcpy(&ad[(m-1-i)*m], work, len);
            }
            __a_disperse(ad, w_updated(a), __a_sizeof(a));
            __a_disperse(wd, w_updated(w), __a_sizeof(w));
            free(work);
            __A_TIME_C_STOP
        }
    };

    // }}}
} } }

#undef pin
#endif
