// AMBIENT USAGE TIPS //
//
// Arguments locality:
//
// when executing the kernel be sure to remember that arguments
// that you are feeding to it are bounded by the life time of 
// C++ scope { }. That is either you need to be sure that the
// kernel will be executed inside current scope or allocate
// them inside heap so that they won't be deleted.
// For example for kernel that takes argument "int*& array"
// one can allocate the argument array the following way:
//
// int*const *array = new int*((int*)malloc(sizeof(int)*LEN));
// for(int i = 0; i < LEN; i++) (*array)[i] = ...;
// ambient::push(... *array);
// 
// Here I'm purposedly underlying that array is a pointer to
// constant pointer to ints - that is I don't want it to be
// deleted. Upon the exit from C++ scope the array ptr will
// be deleted but we would be safe as its value is still valid
// because the int*const was alocated inside heap.

template<typename T> 
void info(T& obj){
    if(rank.is_master(scope.get_group())){
        parallel_t& p = breakdown(obj);
        printf("M%d:%d / %d x %d groups (each %d x %d items of %d x %d els)\n", 
               *p.group_id, p.id, p.get_grid_dim().y, p.get_grid_dim().x, p.get_mem_dim().y, p.get_mem_dim().x, p.get_item_dim().x, p.get_item_dim().y);
    }
}

template<typename T>
void block_2d_cycle_assign(T& target)
{
///////////////////////////////////////////// 2D-block-cyclic decomposition
    int np = scope.np = 1; // can be a function arg   // process grid's num of rows 
    int nq = scope.nq = (int)(scope.get_size() / np); // process grid's num of cols 
    int rank_i = (int)(scope.get_rank() / nq); // process row
    int rank_j = (int)(scope.get_rank() % nq); // process col
///////////////////////////////////////////////////////////////////////////
    for(int i = rank_i; i < get_grid_dim(target).y; i += np){
        for(int j = rank_j; j < get_grid_dim(target).x; j += nq){
            assign(target, i, j);
        }
    }
}

template<typename T>
void block_2d_cycle_transposed_assign(T& target)
{
///////////////////////////////////////////// 2D-block-cyclic decomposition
    int np = scope.np = 1; // can be a function arg   // process grid's num of rows 
    int nq = scope.nq = (int)(scope.get_size() / np); // process grid's num of cols 
    int rank_i = (int)(scope.get_rank() / nq); // process row
    int rank_j = (int)(scope.get_rank() % nq); // process col
///////////////////////////////////////////////////////////////////////////
    for(int i = rank_i; i < get_grid_dim(target).x; i += np){
        for(int j = rank_j; j < get_grid_dim(target).y; j += nq){
            assign(target, j, i);
        }
    }
}

template<typename T>
void block_outright_assign(T& target)
{
// this no "distribution" is only needed where we need a contiguous array (no splitting between proc) for the output (schmidt values ...)
///////////////////////////////////////////////////////////////////////////
    for(int i = 0; i < get_grid_dim(target).y; i++){
        for(int j = 0; j < get_grid_dim(target).x; j++){
            assign(target, i, j);
        }
    }
}

template<typename T>
void block_diagonal_assign(T& target)
{
    size_t m = get_mem_t_dim(target).y;
    size_t n = get_mem_t_dim(target).x;
    for(int i = 0; i < get_grid_dim(target).y; i++){
        for(int j = 0; j < get_grid_dim(target).x; j++){
            if((i+1)*m <= j*n) continue;
            if(i*m >= (j+1)*n) continue;
            assign(target, i, j);
        }
    }
}

template<typename T>
void copy_l(p_dense_matrix<T>& ac, pinned const p_dense_matrix<T>& a)
{
    scope_select("1 from ambient as copy where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in copy ("<< ambient::rank() <<"):\n"; info(ac); info(a);

    block_2d_cycle_assign(ac);
    block_2d_cycle_assign(a);
}

void copy_after_l(pinned p_dense_matrix<double>& ac, const size_t& pos, const p_dense_matrix<double>& a)
{
    scope_select("1 from ambient as copy where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in copy ("<< ambient::rank() <<"):\n"; info(ac); info(a);

    block_2d_cycle_assign(ac);
    block_2d_cycle_assign(a);
}

void copy_after_std_l(std::vector<double>*& ac, const size_t& pos, pinned const p_dense_matrix<double>& a)
{
    scope_select("* from ambient as copy_std where master is 0");
    if(!scope.involved()) return;
    //gzout << "2dbcd in copy_std ("<< ambient::rank() <<"):\n"; info(a);

    block_outright_assign(a);
}

void push_back_sqr_gt_l(std::vector<double>*& ac, pinned const p_dense_matrix<double>& a, const double& prec)
{
    scope_select("* from ambient as copy_std where master is 0");
    if(!scope.involved()) return;
    //gzout << "2dbcd in copy_std ("<< ambient::rank() <<"):\n"; info(a);

    block_outright_assign(a);
}

template<typename T>
void cast_to_dense_l(std::vector<T>*& ac, pinned const p_dense_matrix<T>& a, const size_t& m, const size_t& n)
{
    scope_select("* from ambient as cast_to_dense where master is 0");
    if(!scope.involved()) return;

    block_outright_assign(a); //because the dense matrix must be known by every procs
} 

template<typename T>
void cast_to_p_dense_l(const std::vector<T>*& ac, pinned p_dense_matrix<T>& a, const size_t& m, const size_t& n, const size_t& lda)
{
    scope_select("1 from ambient as cast_to_p_dense where master is 0");
    if(!scope.involved()) return;

    block_2d_cycle_assign(a);
} 

void touch_l(p_dense_matrix<double>& a)
{
    scope_select("1 from ambient as touch where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in touch ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a);
}

template<typename T>
void resize_l(p_dense_matrix<T>& a, const size_t& rows, const size_t& cols)
{
    breakdown(a).set_dim(ambient::dim2(cols,rows));
    scope_select("1 from ambient as resize where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in resize ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a);
}

template<typename T>
void remove_rows_l(pinned p_dense_matrix<T>& a, const size_t& i_mark, const size_t& k)
{
    scope_select("1 from ambient as remove_rows where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in remove_rows ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a);
}

template<typename T>
void remove_cols_l(pinned p_dense_matrix<T>& a, const size_t& j_mark, const size_t& k)
{
    scope_select("1 from ambient as remove_cols where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in remove_cols ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a);
}

template<typename T>
void sqrt_diagonal_l(pinned p_dense_matrix<T>& a)
{
    scope_select("1 from ambient as sqrt_diagonal where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in sqrt_diagonal ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a);
}

template<typename T>
void exp_diagonal_l(pinned p_dense_matrix<T>& a)
{
    scope_select("1 from ambient as exp_diagonal where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in sqrt_diagonal ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a);
}

template<typename T>
void gemm_inplace_l(pinned p_dense_matrix<T>& a, const p_dense_matrix<T>& b)
{
    int num = 1;//get_grid_dim(a).y;
    scope_select(num+" from ambient as gemm where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd for "<< num <<" procs in gemm ("<< ambient::rank() <<"):\n"; info(a); info(b);

    block_2d_cycle_assign(a);
    block_2d_cycle_assign(b);
}

template<typename T>
void gemm_l(pinned const p_dense_matrix<T>& a, const p_dense_matrix<T>& b, p_dense_matrix<T>& c)
{
    int num = 1;//get_grid_dim(a).y;
    scope_select(num+" from ambient as gemm where master is 0 and breakdown contains "+get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd for "<< num <<" procs in gemm ("<< ambient::rank() <<"):\n"; info(a); info(b); info(c);

    block_2d_cycle_assign(a);
    block_2d_cycle_assign(b);
    block_2d_cycle_assign(c);
}

template<typename T>
void scalar_norm_l(pinned const p_dense_matrix<T>& a, T*& norm)
{
    scope_select("* from ambient as scalar_norm where master is 0");
    if(!scope.involved()) return;
    //gzout << "2dbcd in scalar_norm ("<< ambient::rank() <<"):\n"; info(a);

    block_outright_assign(a);
}

template<typename T>
void scalar_overlap_l(pinned const p_dense_matrix<T>& a, const p_dense_matrix<T>& b, T*& overlap)
{
    scope_select("* from ambient as scalar_overlap where master is 0");
    if(!scope.involved()) return;
    //gzout << "2dbcd in scalar_overlap ("<< ambient::rank() <<"):\n"; info(a); info(b);

    block_outright_assign(a);
    block_outright_assign(b);
}

template<typename T>
void mem_bound_l(pinned p_dense_matrix<T>& a, const p_dense_matrix<T>& b)
{
    scope_select(1 +" from ambient as mem_bound where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in membound ("<< ambient::rank() <<"):\n"; info(a); info(b);

    block_2d_cycle_assign(a);
    block_2d_cycle_assign(b);
}

template<typename T, typename T2>
void scale_l(pinned p_dense_matrix<T>& m, const T2& t)
{
    scope_select(1 +" from ambient as scale where master is 0 and breakdown contains "+ get_id(m));
    if(!scope.involved()) return;
    //gzout << "2dbcd in scale ("<< ambient::rank() <<"):\n"; info(m);

    block_2d_cycle_assign(m);
}
/////////////////////
// testing kernels // 

template<typename T>
void svd_l(const p_dense_matrix<T>& a, int& m, int& n, p_dense_matrix<T>& u, p_dense_matrix<T>& vt, p_dense_matrix<T>& s)
{
    int num = 1;
    scope_select(num+" from ambient as svd where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in svd ("<< ambient::rank() <<"):\n"; info(a); info(u); info(vt); info(s);

    block_outright_assign(s);
    block_2d_cycle_assign(a);
    block_2d_cycle_assign(u);
    block_2d_cycle_assign(vt);
}

void syev_l(const p_dense_matrix<double>& a, int& m, p_dense_matrix<double>& w)
{
    int num = 1;
    scope_select(num+" from ambient as syev where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in syev ("<< ambient::rank() <<"):\n"; info(a); info(w);

    block_2d_cycle_assign(a);
    block_2d_cycle_assign(w);
}

void heev_l(const p_dense_matrix<double>& a, int& m, p_dense_matrix<double>& w)
{
    int num = 1;
    scope_select(num+" from ambient as heev where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in syev ("<< ambient::rank() <<"):\n"; info(a); info(w);

    block_2d_cycle_assign(a);
    block_2d_cycle_assign(w); // C - block_outright_assign(w) is possible, if yes remove solidify and disperse for w 
}

template<typename T>
void gemm_diagonal_lhs_l(const p_dense_matrix<T>& a_diag, pinned const p_dense_matrix<T>& b, p_dense_matrix<T>& c)
{
    scope_select("1 from ambient as gemm_diagonal_lhs where master is 0 and breakdown contains "+ get_id(b));
    if(!scope.involved()) return;
    //gzout << "2dbcd in gemm_diagonal_lhs ("<< ambient::rank() <<"):\n"; info(a_diag); info(b); info(c);

    block_2d_cycle_assign(a_diag);
    block_2d_cycle_assign(b);
    block_2d_cycle_assign(c);
}

template<typename T>
void gemm_diagonal_rhs_l(pinned const p_dense_matrix<T>& a, const p_dense_matrix<T>& b_diag, p_dense_matrix<T>& c)
{
    scope_select("1 from ambient as gemm_diagonal_rhs where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in gemm_diagonal_rhs ("<< ambient::rank() <<"):\n"; info(a); info(b_diag); info(c);

    block_2d_cycle_assign(a);
    block_2d_cycle_assign(b_diag);
    block_2d_cycle_assign(c);
}

template<typename T>
void trace_l(pinned const p_dense_matrix<T>& a, T*& trace)
{
    int num = 1;
    scope_select("* from ambient as trace where master is 0");
    if(!scope.involved()) return;
    //gzout << "2dbcd in trace ("<< ambient::rank() <<"):\n"; info(a);

    block_diagonal_assign(a);
}

template<typename T>
void transpose_l(pinned p_dense_matrix<T>& transposed, const p_dense_matrix<T>& original)
{
    int num = 1; //get_grid_dim(a_ambient).y; 
    scope_select(num+" from ambient as transpose_l where master is 0 and breakdown contains "+ get_id(original));
    if(!scope.involved()) return;
    //gzout << "2dbcd in validation ("<< ambient::rank() <<"):\n"; info(transposed); info(original);

    block_2d_cycle_assign(transposed); 
    block_2d_cycle_transposed_assign(original);
}

template<typename T>
void validation_l(pinned const p_dense_matrix<T>& a, const p_dense_matrix<T>& b, int*& bl) // C - bl <=> boolean 
{
    scope_select("2 from ambient as validation where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in validation ("<< ambient::rank() <<"):\n"; info(a_ambient); info(b_scalapack);

    block_2d_cycle_assign(a); 
    block_2d_cycle_assign(b); 
}

template<typename T>
void reshape_l2r_l(const p_dense_matrix<T>& left, pinned p_dense_matrix<T>& right,
                   const size_t& left_offset, const size_t& right_offset, 
                   const size_t& sdim, const size_t& ldim, const size_t& rdim)
{
    int num = 1; //get_grid_dim(a_ambient).y; 
    scope_select(num+" from ambient as reshape_l2r where master is 0 and breakdown contains "+ get_id(right));
    if(!scope.involved()) return;
    //gzout << "2dbcd in reshape_l2r ("<< ambient::rank() <<"):\n"; info(left); info(right);

    block_2d_cycle_assign(left); 
    block_2d_cycle_assign(right); 
}

template<typename T>
void reshape_r2l_l(pinned p_dense_matrix<T>& left, const p_dense_matrix<T>& right,
                   const size_t& left_offset, const size_t& right_offset, 
                   const size_t& sdim, const size_t& ldim, const size_t& rdim)
{
    int num = 1; //get_grid_dim(a_ambient).y; 
    scope_select(num+" from ambient as reshape_l2r where master is 0 and breakdown contains "+ get_id(left));
    if(!scope.involved()) return;
    //gzout << "2dbcd in reshape_r2l ("<< ambient::rank() <<"):\n"; info(left); info(right);

    block_2d_cycle_assign(left); 
    block_2d_cycle_assign(right); 
}

template <typename T>
void rb_tensor_mpo_l(pinned p_dense_matrix<T>& out, const p_dense_matrix<T>& in, const p_dense_matrix<T>& alfa,
                     const size_t& out_offset, const size_t& in_offset, 
                     const size_t& sdim1, const size_t& sdim2, const size_t& ldim, const size_t& rdim)
{
    int num = 1; //get_grid_dim(a_ambient).y; 
    scope_select(num+" from ambient as rb_tensor_mpo where master is 0 and breakdown contains "+ get_id(out));
    if(!scope.involved()) return;
    //gzout << "2dbcd in rb_tensor_mpo ("<< ambient::rank() <<"):\n"; info(out); info(in); info(alfa);

    block_2d_cycle_assign(out); 
    block_2d_cycle_assign(in); 
    block_2d_cycle_assign(alfa); 
}

template <typename T>
void lb_tensor_mpo_l(pinned p_dense_matrix<T>& out, const p_dense_matrix<T>& in, const p_dense_matrix<T>& alfa,
                     const size_t& out_offset, const size_t& in_offset, 
                     const size_t& sdim1, const size_t& sdim2, const size_t& ldim, const size_t& rdim)
{
    int num = 1; //get_grid_dim(a_ambient).y; 
    scope_select(num+" from ambient as rb_tensor_mpo where master is 0 and breakdown contains "+ get_id(out));
    if(!scope.involved()) return;
    //gzout << "2dbcd in rb_tensor_mpo ("<< ambient::rank() <<"):\n"; info(out); info(in); info(alfa);

    block_2d_cycle_assign(out); 
    block_2d_cycle_assign(in); 
    block_2d_cycle_assign(alfa); 
}

void associated_copy_l(p_dense_matrix<double>& ac, pinned const p_dense_matrix<double>& a)
{
    int num = 1;
    scope_select(num+" from ambient as associated_copy where master is 0 and breakdown contains "+ get_id(a)); 
    if(!scope.involved()) return;
    //gzout << "2dbcd in associated_copy ("<< ambient::rank() <<"):\n"; info(ac); info(a);

    block_outright_assign(ac);
    block_outright_assign(a);
}

void variable_free_l(void*& a) // to modify
{
    int num = 1;
    scope_select(num+" from ambient as variable_free where master is 0");
    //gzout << "null assign in variable_free ("<< ambient::rank() <<"):\n";
    if(!scope.involved()) return;
}

template <typename T>
void apply_writes_l(p_dense_matrix<T>& a)
{
    int num = 1; //get_grid_dim(a_ambient).y; 
    scope_select(num+" from ambient as apply_wriTes where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in apply_writes ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a); 
}

template<typename T>
void nullcut_l(pinned p_dense_matrix<T>& a, const size_t& num_rows, const size_t& num_cols)
{
    int num = 1; //get_grid_dim(a_ambient).y; 
    scope_select(num+" from ambient as nullcut where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    //gzout << "2dbcd in touch ("<< ambient::rank() <<"):\n"; info(a);

    block_2d_cycle_assign(a); 
}

template<typename T>
void print_l(const p_dense_matrix<T>& a, int& m, int& n)
{
    int num = 1;
    scope_select(num+" from ambient as print where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    
    block_2d_cycle_assign(a); 
}

template<typename T>
void initv_l(pinned p_dense_matrix<T>& a, T const& v)
{
    int num = 1;
    scope_select(num+" from ambient as initv where master is 0 and breakdown contains "+ get_id(a));
    if(!scope.involved()) return;
    
    block_2d_cycle_assign(a); 
}


