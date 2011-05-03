#define NODE_COUNT 1
#include "mkl.h"

extern "C" {
    double sqrt(double);
    double fabs(double);
}

void gemm_c_kernel(pinned const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, p_dense_matrix<double>& c)
{
//  --- --- ---       --- --- ---       --- --- ---
// | 0 | 1 | 2 |     | 0 | 1 | 2 |     | 0 | 1 | 2 |
//  --- --- ---       --- --- ---       --- --- ---
// | 0 | 1 | 2 |  x  | 0 | 1 | 2 |  =  | 0 | 1 | 2 |
//  --- --- ---       --- --- ---       --- --- ---
// | 0 | 1 | 2 |     | 0 | 1 | 2 |     | 0 | 1 | 2 |
//  --- --- ---       --- --- ---       --- --- ---
//
// partial reduce?..
/////////////////////////////////////////////////////////////////////////
    int m   = get_mem_t_dim(a).y;
    int n   = get_mem_t_dim(b).x;
    int k   = get_mem_t_dim(b).y;
    int lda = m;
    int ldb = k;
    int ldc = m;
    double alpha = 1.0; 
    double beta  = 1.0;
// a(i,j) => b(j,i) x a(z,j) where z : [0,m)
// current block of matrix a:
    int i = get_block_id(a).y;
    int j = get_block_id(a).x;
// taking (j,i) of b:
    if(get_grid_dim(b).y > j) while(i < get_grid_dim(b).x){
        double* bd = current(b)(j,i); // remote
// multiplying with column of a:
        for(int z = 0; z < get_grid_dim(a).y; z++){
            double* ad = current(a)(z,j);
            double* cd = reduced<'+'>(c)(z,i); // a(z,j) x b(j,i) => c(z,i)
            dgemm("N","N", &m, &n, &k, &alpha, ad, &lda, bd, &ldb, &beta, cd, &ldc);
        }
        i += get_grid_dim(a).x;
    }
}

void copy_c_kernel(p_dense_matrix<double>& ac, pinned const p_dense_matrix<double>& a)
{    
    int i = get_block_id(a).y;
    int j = get_block_id(a).x;
    double* a_elements  = current(a)(i,j);
    double* ac_elements = current(ac)(i,j);
    memcpy(ac_elements, a_elements, sizeof(double)*(get_mem_dim(a)*get_item_dim(a)));
}

void copy_c_kernel3(p_dense_matrix<double>& ac, pinned const p_dense_matrix<double>& a, p_dense_matrix<double>& d)
{    
    int i = get_block_id(a).y;
    int j = get_block_id(a).x;
    double* a_elements  = current(a)(i,j);
    double* ac_elements = current(ac)(i,j);
    memcpy(ac_elements, a_elements, sizeof(double)*(get_mem_dim(a)*get_item_dim(a)));
}

void remove_rows_c_kernel(pinned p_dense_matrix<double>& a, const size_t& i_mark, const size_t& k)
{
    typedef double T;
    
    double* ad    = NULL;
    double* ad_r  = NULL;
    double* ad_r0 = NULL;

    size_t i   = get_block_id(a).y;
    size_t j   = get_block_id(a).x;
    size_t lda = get_mem_t_dim(a).y;

    size_t remains_u = i_mark % lda;
    size_t remains_l = lda - (remains_u+k) % lda;
    size_t remains   = remains_u + remains_l;
    size_t shift     = __a_ceil(k / lda);
    size_t group_i_mark = i_mark / lda;
    size_t k_wo_blocks = std::min((2*lda-remains), k);
    if(i < group_i_mark) return;                                                                       // easy-out
    ad   = current(a)(i,j);
    if(i+shift < get_grid_dim(a).y) ad_r = current(a)(i+shift,j);
 
    if(remains < lda && (remains_u + k) > lda){                                                        // get two following blocks (i+shift-1;i+shift)
        if((i+shift-1) < get_grid_dim(a).y) ad_r0 = current(a)(i+shift-1,j);
        ambient::memoryfence();
        if(ad_r0 == NULL) return;                                                                      // out of matrix request
        if(i == group_i_mark){
            for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                                             // memcpy from replacement block #1
                memcpy(&ad[lda*j + remains_u], &ad_r0[lda*j+lda-remains_l], sizeof(T)*remains_l);
        }else if(i >= group_i_mark){
            for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                                             // memcpy from replacement block #1
                memcpy(&ad[lda*j], &ad_r0[lda*j + (lda-remains)], sizeof(T)*remains);
        }
        for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                                                 // memcpy from replacement block #2
            memcpy(&ad[lda*j + remains], &ad_r[lda*j], sizeof(T)*(lda-remains));
    }else{                                                                                             // get only one following block
        ambient::memoryfence();
        if(i == group_i_mark){
            if(remains_u + k < lda){
                for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                                         // first memmove inside block
                    memmove(&ad[lda*j + remains_u], &ad[lda*j + remains_u+k], sizeof(T)*(lda-remains_u-k));
                if(ad_r != NULL) for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                        // memcpy from replacement block
                    memcpy(&ad[lda*j + lda - k], &ad_r[lda*j], sizeof(T)*k);
            }else{
                if(ad_r != NULL) for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                        // memcpy from replacement block
                    memcpy(&ad[lda*j + remains_u], &ad_r[lda*j + lda-remains_l], sizeof(T)*(lda-remains_u));
            }
        }else if(i >= group_i_mark){
            for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                                             // first memmove inside block
                memmove(&ad[lda*j], &ad[lda*j + k_wo_blocks], sizeof(T)*(lda-k_wo_blocks));
            if(ad_r != NULL) for(size_t j = 0; j < get_mem_t_dim(a).x; ++j)                            // memcpy from replacement block
                memcpy(&ad[lda*j + lda-k_wo_blocks], &ad_r[lda*j], sizeof(T)*k_wo_blocks);
        }
    }
}

void remove_cols_c_kernel(pinned p_dense_matrix<double>& a, const size_t& j_mark, const size_t& k)
{
    typedef double T;

    double* ad    = NULL;
    double* ad_r  = NULL;
    double* ad_r0 = NULL;

    size_t i   = get_block_id(a).y;
    size_t j   = get_block_id(a).x;
    size_t lda = get_mem_t_dim(a).y;
    size_t sda = get_mem_t_dim(a).x;

    size_t remains_l = j_mark % sda;
    size_t remains_r = sda - (remains_l+k) % sda;
    size_t remains   = remains_l + remains_r;
    size_t shift     = __a_ceil(k / sda);
    size_t group_j_mark = j_mark / sda;
    size_t k_wo_blocks = std::min((2*sda-remains), k);

    if(j < group_j_mark) return;                                                                                        // easy-out
    ad   = current(a)(i,j);                                                                                        
    if(j+shift < get_grid_dim(a).x) ad_r = current(a)(i,j+shift);                                                  
                                                                                                                   
    if(remains < sda && (remains_l + k) > sda){                                                                         // get two following blocks (j+shift-1;j+shift)
        if((j+shift-1) < get_grid_dim(a).x) ad_r0 = current(a)(i,j+shift-1);                                       
        ambient::memoryfence();                                                                                    
        if(ad_r0 == NULL) return;                                                                                       // out of matrix request
        if(j == group_j_mark){                                                                                     
            memcpy(&ad[lda*remains_l], &ad_r0[lda*(sda-remains_r)], sizeof(T)*lda*remains_r);                           // memcpy from replacement block #1
        }else if(j >= group_j_mark){                                                                               
            memcpy(ad, &ad_r0[lda*(sda-remains)], sizeof(T)*lda*remains);                                               // memcpy from replacement block #1
        }                                                                                                          
        memcpy(&ad[lda*remains], ad_r, sizeof(T)*lda*(sda-remains));                                                    // memcpy from replacement block #2
    }else{                                                                                                              // get only one following block
        ambient::memoryfence();
        if(j == group_j_mark){
            if(remains_l + k < sda){
                memmove(&ad[lda*remains_l], &ad[lda*(remains_l+k)], sizeof(T)*lda*(sda-remains_l-k));                   // first memmove inside block
                if(ad_r != NULL) memcpy(&ad[lda*(sda-k)], ad_r, sizeof(T)*lda*k);                                       // memcpy from replacement block
            }else{
                if(ad_r != NULL) memcpy(&ad[lda*remains_l], &ad_r[lda*(sda-remains_r)], sizeof(T)*lda*(sda-remains_l)); // memcpy from replacement block
            }
        }else if(i >= group_j_mark){
            memmove(ad, &ad[lda*k_wo_blocks], sizeof(T)*lda*(sda-k_wo_blocks) );                                        // first memmove inside block
            if(ad_r != NULL) memcpy(&ad[lda*(sda-k_wo_blocks)], ad_r, sizeof(T)*lda*k_wo_blocks);                       //  memcpy from replacement block
        }
    }
}

void resize_c_kernel(p_dense_matrix<double>& a, const size_t& rows, const size_t& cols)
{
    for(int i = 0; i < get_grid_dim(a).y; i++)
    if(current(a).block(i, get_grid_dim(a).x-1)->available()){
        size_t cutoff = get_grid_dim(a).x*get_mem_t_dim(a).x - get_dim(a).x;
        if(cutoff > 0){
            double* ad = current(a)(i, get_grid_dim(a).x-1);
            memset(&ad[(get_mem_t_dim(a).x - cutoff)*get_mem_t_dim(a).y], 0, cutoff*sizeof(double));
        }
    }
    for(int j = 0; j < get_grid_dim(a).x; j++)
    if(current(a).block(get_grid_dim(a).y-1, j)->available()){
        double* ad = current(a)(get_grid_dim(a).y-1,j);
        size_t cutoff = get_grid_dim(a).y*get_mem_t_dim(a).y - get_dim(a).y;
        size_t offset = get_mem_t_dim(a).y - cutoff;
        if(cutoff > 0) for(int jj=0; jj < get_mem_t_dim(a).x; jj++)
            memset(&ad[get_mem_t_dim(a).y*jj+offset], 0, cutoff*sizeof(double));
    }
}

void sqrt_diagonal_c_kernel(pinned p_dense_matrix<double>& a)
{
    double* ad = current(a)(get_block_id(a).y, get_block_id(a).x);
    for(int i=0; i < get_mem_t_dim(a).y; i++)
    ad[i] = sqrt(ad[i]);
}

template<typename T>
void __a_memcpy(T& dest, dim2 dest_p, const T& src, dim2 src_p, dim2 size)
{
    size_t starti, startj, limi, limj;
    size_t di = get_block_id(dest).y * get_mem_t_dim(dest).y;
    size_t dj = get_block_id(dest).x * get_mem_t_dim(dest).x;

    assert(get_grid_dim(dest).x*get_mem_t_dim(dest).x - dest_p.x >= size.x);
    assert(get_grid_dim(dest).y*get_mem_t_dim(dest).y - dest_p.y >= size.y);
    assert(get_grid_dim(src).x*get_mem_t_dim(src).x - src_p.x >= size.x);
    assert(get_grid_dim(src).y*get_mem_t_dim(src).y - src_p.y >= size.y);

    if(size.x == 0 || size.y == 0) return;
    if((di + get_mem_t_dim(dest).y <= dest_p.y) || (dj + get_mem_t_dim(src).x  <= dest_p.x)) return;
    if((di >= dest_p.y + size.y) || (dj >= dest_p.x + size.x)) return;
// lets find dest-block copy limits
    if(di + get_mem_t_dim(dest).y > dest_p.y + size.y) limi = (dest_p.y + size.y) % get_mem_t_dim(dest).y;
    else limi = get_mem_t_dim(dest).y;
    if(dj + get_mem_t_dim(dest).x > dest_p.x + size.x) limj = (dest_p.x + size.x) % get_mem_t_dim(dest).x;
    else limj = get_mem_t_dim(dest).x;
// lets find dest-block starting point
    if(di < dest_p.y) starti = dest_p.y % get_mem_t_dim(dest).y;
    else starti = 0;
    if(dj < dest_p.x) startj = dest_p.x % get_mem_t_dim(dest).x;
    else startj = 0;

    size_t si = di + starti - dest_p.y + src_p.y;
    size_t sii = si % get_mem_t_dim(src).y;
// let's find how many blocks do we need for this one
    size_t src_blocks_i = 0;
    int num_src_blocks = limi-starti-get_mem_t_dim(src).y+sii;
    if(num_src_blocks > 0) src_blocks_i = __a_ceil( num_src_blocks / get_mem_t_dim(src).y ) + 1;
// let's exhaust first src block
    typename T::value_type* dd = current(dest)(get_block_id(dest).y, get_block_id(dest).x);

    for(size_t j = startj; j < limj; j++){
        size_t sj = dj + j - dest_p.x + src_p.x;
        size_t sii = si % get_mem_t_dim(src).y;
        size_t sjj = sj % get_mem_t_dim(src).x;
        size_t w = limi - starti;
        size_t i = starti;
        for(int k = 0; k < src_blocks_i; k++){
            typename T::value_type* sd = current(src)(si / get_mem_t_dim(src).y + k, sj / get_mem_t_dim(src).x);
            memcpy(&dd[j*get_mem_t_dim(dest).y + i],
                   &sd[sjj*get_mem_t_dim(src).y+sii],
                   std::min(get_mem_t_dim(src).y-sii, w)*sizeof(T::value_type));
            w -= get_mem_t_dim(src).y-sii;
            i += get_mem_t_dim(src).y-sii;
            sii = 0;
        }
    }
}

void reshape_l2r_c_kernel(const p_dense_matrix<double>& left, pinned p_dense_matrix<double>& right,
                          const size_t& left_offset, const size_t& right_offset, 
                          const size_t& sdim, const size_t& ldim, const size_t& rdim)
{
    for(size_t ss = 0; ss < sdim; ++ss)
        __a_memcpy(right, dim2(ss*rdim + right_offset,0), 
                   left,  dim2(0, ss*ldim + left_offset), 
                   dim2( rdim, ldim ));
}

void reshape_r2l_c_kernel(pinned p_dense_matrix<double>& left, const p_dense_matrix<double>& right,
                          const size_t& left_offset, const size_t& right_offset, 
                          const size_t& sdim, const size_t& ldim, const size_t& rdim)
{
    for(size_t ss = 0; ss < sdim; ++ss)
        __a_memcpy(left,  dim2(0, ss*ldim + left_offset), 
                   right, dim2(ss*rdim + right_offset,0), 
                   dim2( rdim, ldim ));
}

void add_c_kernel(const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, pinned p_dense_matrix<double>& c)
{
    double* ad = current(a)(get_block_id(c).y, get_block_id(c).x);
    double* bd = current(b)(get_block_id(c).y, get_block_id(c).x);
    double* cd = current(c)(get_block_id(c).y, get_block_id(c).x);
    for(int i=0; i < get_mem_t_dim(c).x*get_mem_t_dim(c).y; i++)
    cd[i] = ad[i] + bd[i];
}

void sub_c_kernel(const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, pinned p_dense_matrix<double>& c)
{
    double* ad = current(a)(get_block_id(c).y, get_block_id(c).x);
    double* bd = current(b)(get_block_id(c).y, get_block_id(c).x);
    double* cd = current(c)(get_block_id(c).y, get_block_id(c).x);
    for(int i=0; i < get_mem_t_dim(c).x*get_mem_t_dim(c).y; i++)
    cd[i] = ad[i] - bd[i];
}

void scale_c_kernel(const p_dense_matrix<double>& m, const double& t, pinned p_dense_matrix<double>& out)
{
    double* md   = current(m)(get_block_id(out).y, get_block_id(out).x);
    double* outd = current(out)(get_block_id(out).y, get_block_id(out).x);
    for(int i=0; i < get_mem_t_dim(out).x*get_mem_t_dim(out).y; i++)
    outd[i] = md[i]*t;
}

void gemm_diagonal_lhs_c_kernel(const p_dense_matrix<double>& a_diag, pinned const p_dense_matrix<double>& b, p_dense_matrix<double>& c)
{
    int j = get_block_id(b).y*get_mem_t_dim(b).y;
    int size = get_mem_t_dim(b).x;
    int lda  = get_mem_t_dim(b).y;
    int ONE  = 1;
    double* bd = current(b)(get_block_id(b).y, get_block_id(b).x);
    double* cd = current(c)(get_block_id(b).y, get_block_id(b).x);

    memset(cd, 0, get_mem_t_dim(c).x*get_mem_t_dim(c).y*sizeof(double));
    for(int jj = 0 ; jj < get_mem_t_dim(b).y ; jj++){
         double* alpha = current(a_diag)((j+jj)/get_mem_t_dim(a_diag).y,0);
	 daxpy(&size, &alpha[(j+jj)%get_mem_t_dim(a_diag).y], &bd[jj], &lda, &cd[jj], &lda);
    }
}

void gemm_diagonal_rhs_c_kernel(pinned const p_dense_matrix<double>& a, const p_dense_matrix<double>& b_diag, p_dense_matrix<double>& c)
{
    int j = get_block_id(a).x*get_mem_t_dim(a).x;
    int size = get_mem_t_dim(a).y;
    int ONE = 1;
    double* ad = current(a)(get_block_id(a).y, get_block_id(a).x);
    double* cd = current(c)(get_block_id(a).y, get_block_id(a).x);

    memset(cd, 0, get_mem_t_dim(c).x*get_mem_t_dim(c).y*sizeof(double));
    for(int jj = 0 ; jj < get_mem_t_dim(a).x ; jj++){
	 double* alpha = current(b_diag)((j+jj)/get_mem_t_dim(b_diag).y,0);
	 daxpy(&size, &alpha[(j+jj)%get_mem_t_dim(b_diag).y], &ad[jj*get_mem_t_dim(a).y], &ONE, &cd[jj*get_mem_t_dim(c).y], &ONE);
    }
}

void init_double_c_kernel(pinned p_dense_matrix<double> & a)
{
    double* ad = current(a)(get_block_id(a).y, get_block_id(a).x);
    for(int jj = 0 ; jj < get_mem_t_dim(a).x*get_mem_t_dim(a).y ; jj++)
    {
	ad[jj] = drand48();
    }
}

void copy_svd_c_kernel(pinned p_dense_matrix<double>& a, double*& s) 
{ 
    int j = get_block_id(a).y*get_mem_t_dim(a).y; 
    double* ad = current(a)(get_block_id(a).y, get_block_id(a).x); 
    int size = (get_mem_t_dim(a)*sizeof(double)); 
    memcpy(ad,s+j,size); 
} 
	 
void one_null_c_kernel(const p_dense_matrix<double>& a){ }
void two_null_c_kernel(const p_dense_matrix<double>& a, const p_dense_matrix<double>& b){ }
void three_null_c_kernel(const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, const p_dense_matrix<double>& c){ }

/** 
Validation kernel source:  
 @article{Dongarra:1990:SLB:77626.79170, 
 author = {Dongarra, J. J. and Du Croz, Jeremy and Hammarling, Sven and Duff, I. S.}, 
 title = {A set of level 3 basic linear algebra subprograms}, 
 journal = {ACM Trans. Math. Softw.}, 
 volume = {16}, 
 issue = {1}, 
 month = {March}, 
 year = {1990}, 
 issn = {0098-3500}, 
 pages = {1--17}, 
 numpages = {17}, 
 url = {http://doi.acm.org/10.1145/77626.79170}, 
 doi = {http://doi.acm.org/10.1145/77626.79170}, 
 acmid = {79170}, 
 publisher = {ACM}, 
 address = {New York, NY, USA}, 
}  
*/ 
void validation_c_kernel(pinned const p_dense_matrix<double>& a_ambient, const p_dense_matrix<double>& b_scalapack) 
{ 
    double* ad = current(a_ambient)(get_block_id(a_ambient).y, get_block_id(a_ambient).x); 
    double* bd = current(b_scalapack)(get_block_id(a_ambient).y, get_block_id(a_ambient).x); 

    double res = 0; 
    double epsilon = 0.0000000000000001; 
    for(int i=0; i < get_mem_t_dim(a_ambient).x*get_mem_t_dim(a_ambient).y; i++) 
    { 
        res = (fabs(ad[i]-bd[i]))/fabs(epsilon*bd[i]); 
        if(res > 16){ // 16 is recommended by dongara,  
             printf("validation test failed in block %d %d, res %.10f Ambient: %.10f Scala: %.10f \n", get_block_id(a_ambient).y, get_block_id(a_ambient).x, res, ad[i], bd[i]);
             exit(-1);
        }                
    } 
} 
