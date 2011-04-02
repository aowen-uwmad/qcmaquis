#define NODE_COUNT 1
#include "mkl.h"

extern "C" {
    double  sqrt( double );
    void Cblacs_get( int, int, int* );
    void Cblacs_gridinit( int*, const char*, int, int );
    void Cblacs_gridinfo( int, int*, int*, int*, int* );
	void Cblacs_gridexit(int *);
	void Cblacs_exit(int *);
    int Csys2blacs_handle( MPI_Comm );
    int numroc_( int*, int*, int*, int*, int* );
    void descinit_( int*, int*, int*, int*, int*, int*, int*, int*, int*, int* );
    void pdgemm_(const char*,const char*,int*,int*,int*,double*,double*,int*,int*,int*,double*,int*,int*,int*,double*,double*,int*,int*,int*);
    void pdgesvd_(char *jobu, char *jobvt, int *m, int *n,double *a, int *ia, int *ja, int *desca,double *s,double *u, int *iu, int *ju, int *descu,double *vt, int *ivt, int *jvt, int *descvt,double *work, int *lwork,int *info);
}

/*
 --- --- ---       --- --- ---       --- --- ---
| 0 | 1 | 2 |     | 0 | 1 | 2 |     | 0 | 1 | 2 |
 --- --- ---       --- --- ---       --- --- ---
| 0 | 1 | 2 |  x  | 0 | 1 | 2 |  =  | 0 | 1 | 2 |
 --- --- ---       --- --- ---       --- --- ---
| 0 | 1 | 2 |     | 0 | 1 | 2 |     | 0 | 1 | 2 |
 --- --- ---       --- --- ---       --- --- ---
partial reduce?
*/

void gemm_c_kernel(pinned const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, p_dense_matrix<double>& c)
{
    int m   = get_group_t_dim(a).y;
    int n   = get_group_t_dim(b).x;
    int k   = get_group_t_dim(b).y;
    int lda = m;
    int ldb = k;
    int ldc = m;
    double alpha = 1.0; 
    double beta  = 0.0;
// a(i,j) => b(j,i) x a(z,j) where z : [0,m)
// current group of matrix a:
    int i = get_group_id(a).y;
    int j = get_group_id(a).x;
// taking (j,i) of b:
    double* bd = current(b)(j,i); // remote
// multiplying with column of a:
    for(int z = 0; z < get_grid_dim(a).y; z++){
        double* ad = current(a)(z,j);
        double* cd = reduced<'+'>(c)(z,i); // a(z,j) x b(j,i) => c(z,i)
        //printf("Performing DGEMM for %d,%d and %d,%d\n", z,j,j,i);
        dgemm("N","N", &m, &n, &k, &alpha, ad, &lda, bd, &ldb, &beta, cd, &ldc);
        //for(int ii=0; ii < m; ii++){
        //  for(int jj=0; jj < n; jj++)
        //  printf("%.2f	", cd[jj*ldc + ii]);
        //  printf("\n");
        //}
    }
}


void pdgemm_c_kernel(p_dense_matrix<double>& a, p_dense_matrix<double>& b, p_dense_matrix<double>& c){
//    printf("R%d: Executing ScaLAPACK PDGEMM kernel\n", scope.get_rank());

/*    int i, j, k;
    int bhandle, ictxt, nprow, npcol, myrow, mycol,nb;
    nprow = NODE_COUNT; npcol = (int)(scope.get_size()/NODE_COUNT); 
    nb = get_group_dim(c).x*get_item_dim(c).x;
    int M = get_grid_dim(c)*get_group_dim(c).x*get_item_dim(c).x;
    int info,itemp;
    int ZERO=0,ONE=1;
 
    bhandle = Csys2blacs_handle(scope.get_group()->mpi_comm);
    ictxt = bhandle;
    //Cblacs_get( -1, 0, &ictxt );
    Cblacs_gridinit( &ictxt, "Row", nprow, npcol );
    Cblacs_gridinfo( ictxt, &nprow, &npcol, &myrow, &mycol );
    //printf("Number of rows: %d\nNumber of cols: %d\nMy row: %d\nMy col: %d\n", nprow, npcol, myrow, mycol);

    int descA[9],descB[9],descC[9];
    int mA = numroc_( &M, &nb, &myrow, &ZERO, &nprow );
    int nA = numroc_( &M, &nb, &mycol, &ZERO, &npcol );
    int mB = numroc_( &M, &nb, &myrow, &ZERO, &nprow );
    int nB = numroc_( &M, &nb, &mycol, &ZERO, &npcol );
    int mC = numroc_( &M, &nb, &myrow, &ZERO, &nprow );
    int nC = numroc_( &M, &nb, &mycol, &ZERO, &npcol );
    descinit_(descA, &M,   &M,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mA,  &info);
    descinit_(descB, &M,   &M,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mB,  &info);
    descinit_(descC, &M,   &M,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mC,  &info);
    
   // breakdown(a).solidify();
   // breakdown(b).solidify();
   // breakdown(c).solidify();
    double *A = (double*) malloc(mA*nA*sizeof(double));
    double *B = (double*) malloc(mB*nB*sizeof(double));
    double *C = (double*) malloc(mC*nC*sizeof(double));
    for(i=0;i<mA;i++) for(j=0;j<nA;j++){
                 A[j*mA+i]=0.01;
         }
    for(i=0;i<mB;i++) for(j=0;j<nB;j++){
                 B[j*mB+i]=0.01;
         }
    for(i=0;i<mC;i++) for(j=0;j<nC;j++){
                 C[j*mC+i]=0.01;
         }
    printf("M: %d; mA: %d; nA: %d\n", M, mA, nA);
    double alpha = 1.0; double beta = 0.0;
    
//    for(i=0;i<VECTOR_SIZE;i++)
    //pdgemm_("N","N",&M,&M,&M,&alpha,(double*)breakdown(a).data,&ONE,&ONE,descA,(double*)breakdown(b).data,&ONE,&ONE,descB,&beta,(double*)breakdown(c).data,&ONE,&ONE,descC);
    pdgemm_("N","N",&M,&M,&M,&alpha,A,&ONE,&ONE,descA,B,&ONE,&ONE,descB,&beta,C,&ONE,&ONE,descC);
*/
}


void copy_c_kernel(p_dense_matrix<double>& ac, pinned const p_dense_matrix<double>& a)
{    
    int i = get_group_id(a).y;
    int j = get_group_id(a).x;
    double* a_elements  = current(a)(i,j);
    double* ac_elements = current(ac)(i,j);
    memcpy(ac_elements, a_elements, sizeof(double)*(get_group_dim(a)*get_item_dim(a)));
}

void remove_rows_c_kernel(pinned p_dense_matrix<double>& a, const size_t& i_mark, const size_t& k)
{
    typedef double T;
    
    double* ad    = NULL;
    double* ad_r  = NULL;
    double* ad_r0 = NULL;

    size_t i   = get_group_id(a).y;
    size_t j   = get_group_id(a).x;
    size_t lda = get_group_t_dim(a).y;

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
            for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                                           // memcpy from replacement block #1
                memcpy(&ad[lda*j + remains_u], &ad_r0[lda*j+lda-remains_l], sizeof(T)*remains_l);
        }else if(i >= group_i_mark){
            for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                                           // memcpy from replacement block #1
                memcpy(&ad[lda*j], &ad_r0[lda*j + (lda-remains)], sizeof(T)*remains);
        }
        for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                                               // memcpy from replacement block #2
            memcpy(&ad[lda*j + remains], &ad_r[lda*j], sizeof(T)*(lda-remains));
    }else{                                                                                             // get only one following block
        ambient::memoryfence();
        if(i == group_i_mark){
            if(remains_u + k < lda){
                for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                                       // first memmove inside block
                    memmove(&ad[lda*j + remains_u], &ad[lda*j + remains_u+k], sizeof(T)*(lda-remains_u-k));
                if(ad_r != NULL) for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                      // memcpy from replacement block
                    memcpy(&ad[lda*j + lda - k], &ad_r[lda*j], sizeof(T)*k);
            }else{
                if(ad_r != NULL) for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                      // memcpy from replacement block
                    memcpy(&ad[lda*j + remains_u], &ad_r[lda*j + lda-remains_l], sizeof(T)*(lda-remains_u));
            }
        }else if(i >= group_i_mark){
            for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                                           // first memmove inside block
                memmove(&ad[lda*j], &ad[lda*j + k_wo_blocks], sizeof(T)*(lda-k_wo_blocks) );
            if(ad_r != NULL) for(size_t j = 0; j < get_group_t_dim(a).x; ++j)                          // memcpy from replacement block
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

    size_t i   = get_group_id(a).y;
    size_t j   = get_group_id(a).x;
    size_t lda = get_group_t_dim(a).y;
    size_t sda = get_group_t_dim(a).x;

    size_t remains_l = j_mark % sda;
    size_t remains_r = sda - (remains_l+k) % sda;
    size_t remains   = remains_l + remains_r;
    size_t shift     = __a_ceil(k / sda);
    size_t group_j_mark = j_mark / sda;
    size_t k_wo_blocks = std::min((2*sda-remains), k);

    if(j < group_j_mark) return;                                                                       // easy-out
    ad   = current(a)(i,j);
    if(j+shift < get_grid_dim(a).x) ad_r = current(a)(i,j+shift);

    if(remains < sda && (remains_l + k) > sda){                                                        // get two following blocks (j+shift-1;j+shift)
        if((j+shift-1) < get_grid_dim(a).x) ad_r0 = current(a)(i,j+shift-1);
        ambient::memoryfence();
        if(ad_r0 == NULL) return;                                                                      // out of matrix request
        if(j == group_j_mark){
            memcpy(&ad[lda*remains_l], &ad_r0[lda*(sda-remains_r)], sizeof(T)*lda*remains_r);          // memcpy from replacement block #1
        }else if(j >= group_j_mark){
            memcpy(ad, &ad_r0[lda*(sda-remains)], sizeof(T)*lda*remains);                              // memcpy from replacement block #1
        }
        memcpy(&ad[lda*remains], ad_r, sizeof(T)*lda*(sda-remains));                                   // memcpy from replacement block #2
    }else{                                                                                             // get only one following block
        ambient::memoryfence();
        if(j == group_j_mark){
            if(remains_l + k < sda){
                memmove(&ad[lda*remains_l], &ad[lda*(remains_l+k)], sizeof(T)*lda*(sda-remains_l-k));   // first memmove inside block
                if(ad_r != NULL) memcpy(&ad[lda*(sda-k)], ad_r, sizeof(T)*lda*k);                                       // memcpy from replacement block
            }else{
                if(ad_r != NULL) memcpy(&ad[lda*remains_l], &ad_r[lda*(sda-remains_r)], sizeof(T)*lda*(sda-remains_l)); // memcpy from replacement block
            }
        }else if(i >= group_j_mark){
            memmove(ad, &ad[lda*k_wo_blocks], sizeof(T)*lda*(sda-k_wo_blocks) );                        // first memmove inside block
            if(ad_r != NULL) memcpy(&ad[lda*(sda-k_wo_blocks)], ad_r, sizeof(T)*lda*k_wo_blocks);                       //  memcpy from replacement block
        }
    }
}

void resize_c_kernel(p_dense_matrix<double>& a, const size_t& rows, const size_t& cols)
{ 
    for(int i = 0; i < get_grid_dim(a).y; i++)
    if(current(a).group(i, get_grid_dim(a).x-1)->available()){
        size_t cutoff = get_grid_dim(a).x*get_group_t_dim(a).x - get_dim(a).x;
        if(cutoff > 0) memset((double*)current(a)(i, get_grid_dim(a).x-1), 0, cutoff*sizeof(double));
    }
    for(int j = 0; j < get_grid_dim(a).x; j++)
    if(current(a).group(get_grid_dim(a).y-1, j)->available()){
        double* ad = current(a)(get_grid_dim(a).y-1,j);
        size_t cutoff = get_grid_dim(a).y*get_group_t_dim(a).y - get_dim(a).y;
        size_t offset = get_group_t_dim(a).y - cutoff;
        if(cutoff > 0) for(int jj=0; jj < get_group_t_dim(a).x; jj++)
            memset(&ad[get_group_t_dim(a).y*jj+offset], 0, cutoff*sizeof(double));
    }
}

void sqrt_diagonal_c_kernel(pinned p_dense_matrix<double>& a){
    double* ad = current(a)(get_group_id(a).y, get_group_id(a).x);
    for(int i=0; i < get_group_t_dim(a).y; i++)
        ad[i] = sqrt(ad[i]);
}

void null_c_kernel(const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, pinned p_dense_matrix<double>& c){
}
void add_c_kernel(const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, pinned p_dense_matrix<double>& c){ printf("Executed add kernel\n"); }
void sub_c_kernel(const p_dense_matrix<double>& a, const p_dense_matrix<double>& b, pinned p_dense_matrix<double>& c){}
void scale_c_kernel(const p_dense_matrix<double>& a, const double& alfa, pinned p_dense_matrix<double>& out){}



void gemm_c_scalapack_kernel(const p_dense_matrix<double>  &  A, const p_dense_matrix<double>  & B, p_dense_matrix<double> & C){
#ifdef SCALAPACK
	int nmyidBLACS,nnumprocsBLACS,nContinue;
	int nContxt,nVal;  
	int i, j, k;
	int bhandle, ictxt, nprow, npcol, myrow, mycol,nb;
	int nN = get_grid_dim(C).x*get_group_dim(C).x*get_item_dim(C).x; 
	int nM = get_grid_dim(C).y*get_group_dim(C).y*get_item_dim(C).y;
  							 
	int info,itemp;
	int ZERO=0,ONE=1;

	nprow = scope.np;
	npcol = scope.nq; 
	nb = get_group_dim(C).x*get_item_dim(C).x;

	ictxt =Csys2blacs_handle(scope.get_group()->mpi_comm);
	Cblacs_get( -1, 0, &ictxt );
	Cblacs_gridinit( &ictxt, "Row", nprow, npcol );
	Cblacs_gridinfo( ictxt, &nprow, &npcol, &myrow, &mycol );
	
	int descA[9],descB[9],descC[9];
	
	int mA = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );	
	int nA = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );
	int mB = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );
	int nB = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );
	int mC = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );
	int nC = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );
		
	descinit_(descA, &nM,   &nN,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mA,  &info);
	descinit_(descB, &nM,   &nN,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mB,  &info);
	descinit_(descC, &nM,   &nN,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mC,  &info);

	breakdown(A).solidify();
	breakdown(B).solidify();
	breakdown(C).solidify();
	
	double alpha = 1.0; double beta = 0.0;

	pdgemm_("N","N",&nM,&nN,&nM,&alpha,(double*)breakdown(A).data,&ONE,&ONE,descA,(double*)breakdown(B).data,&ONE,&ONE,descB,&beta,(double*)breakdown(C).data,&ONE,&ONE,descC);

	breakdown(A).disperse();
	breakdown(B).disperse();
	breakdown(C).disperse();		
#endif							 
}

void svd_c_scalapack_kernel(const p_dense_matrix<double>  &  M, p_dense_matrix<double>  & U, p_dense_matrix<double> & V, double* & S)
{
#ifdef SCALAPACK

	int descA[9],descV[9],descU[9];
	int nmyidBLACS,nnumprocsBLACS,nContinue;
	int nContxt,nVal;  
	int i, j, k;
	int bhandle, ictxt, nprow, npcol, myrow, mycol,nb;
	int nN = get_grid_dim(M).x*get_group_t_dim(M).x; 
	int nM = get_grid_dim(M).y*get_group_t_dim(M).y;

	int info,itemp;
	int ZERO=0,ONE=1;

	nprow = scope.np;
	npcol = scope.nq; 
	nb = get_group_dim(M).x*get_item_dim(M).x;

	ictxt =Csys2blacs_handle(scope.get_group()->mpi_comm);
	Cblacs_get( -1, 0, &ictxt );
	Cblacs_gridinit( &ictxt, "Row", nprow, npcol );
	Cblacs_gridinfo( ictxt, &nprow, &npcol, &myrow, &mycol );
	
	int mA = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );	
	int nA = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );
	int mU = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );
	int nU = numroc_( &nM, &nb, &mycol, &ZERO, &npcol );
	int mV = numroc_( &nN, &nb, &myrow, &ZERO, &nprow );
	int nV = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );
	
	descinit_(descA, &nM,   &nN,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mA,  &info);
	descinit_(descU, &nM,   &nM,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mU,  &info);
	descinit_(descV, &nN,   &nN,   &nb,  &nb,  &ZERO, &ZERO, &ictxt, &mV,  &info);
	
	breakdown(M).solidify();
	breakdown(U).solidify();
	breakdown(V).solidify();
	
	char JOBU[1] ={'V'};
	char JOBV[1] ={'V'};
	int lwork = -1;
	double wkopt;

	//SCALAPACK, firsti, dry run to allocate buffer	
	pdgesvd_(JOBU, JOBV, &nM, &nN, (double*)breakdown(M).data,&ONE,&ONE,descA, S, (double*)breakdown(U).data,&ONE,&ONE,descU, (double*)breakdown(V).data,&ONE,&ONE,descV, &wkopt, &lwork, &info);

	lwork = static_cast<int> (wkopt);
	double *work = new double[lwork];

	//COMPUTATIONAL run	
	pdgesvd_(JOBU, JOBV, &nM, &nN, (double*)breakdown(M).data,&ONE,&ONE,descA, S, (double*)breakdown(U).data,&ONE,&ONE,descU, (double*)breakdown(V).data,&ONE,&ONE,descV, work, &lwork, &info);
	
	breakdown(M).disperse();
	breakdown(U).disperse();
	breakdown(V).disperse();		
	
	delete[] work; //clean the working buffer

#endif
}
void single_integer_c_kernel(int*& input){
    zout << "single integer kernel: output is " << input[2] << "\n";
}
