#define NODE_COUNT 1

extern "C" {
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
    void pdsyev_(char *jobz, char *uplo, int *n, double *a, int *ia, int *ja, int *desca, double *w, double *z, int *iz, int *jz, int *descz, double *work, int *lwork, int *info);
}

void gemm_c_scalapack_kernel(const p_dense_matrix<double>&  A, const p_dense_matrix<double>& B, p_dense_matrix<double>& C){
#ifdef SCALAPACK
    int nmyidBLACS,nnumprocsBLACS,nContinue;
    int nContxt,nVal;  
    int bhandle, ictxt, nprow, npcol, myrow, mycol,nb;
    int nN = get_grid_dim(C).x*get_mem_dim(C).x*get_item_dim(C).x; 
    int nM = get_grid_dim(C).y*get_mem_dim(C).y*get_item_dim(C).y;

    int info,itemp;
    int ZERO=0,ONE=1;

    nprow = scope.np;
    npcol = scope.nq; 
    nb = get_mem_dim(C).x*get_item_dim(C).x;

    ictxt = Csys2blacs_handle(scope.get_group()->mpi_comm);
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

    assert(current(A).layout->get_list().size() != 0);
    current(A).solidify(current(A).layout->get_list());
    current(B).solidify(current(B).layout->get_list());
    current(C).solidify(current(C).layout->get_list());

    double alpha = 1.0; double beta = 0.0;

    pdgemm_("N","N",&nM,&nN,&nM,&alpha,(double*)breakdown(A).data,&ONE,&ONE,descA,(double*)breakdown(B).data,&ONE,&ONE,descB,&beta,(double*)breakdown(C).data,&ONE,&ONE,descC);
    current(C).disperse(current(C).layout->get_list());
#endif
}

void svd_c_scalapack_kernel(const p_dense_matrix<double>  &  M, p_dense_matrix<double>  & U, p_dense_matrix<double> & V, p_dense_matrix<double> & S)
{
#ifdef SCALAPACK
    int descA[9],descV[9],descU[9];
    int nmyidBLACS,nnumprocsBLACS,nContinue;
    int nContxt,nVal;  
    int bhandle, ictxt, nprow, npcol, myrow, mycol,nb;
    int nN = get_grid_dim(M).x*get_mem_t_dim(M).x; 
    int nM = get_grid_dim(M).y*get_mem_t_dim(M).y;

    int info,itemp;
    int ZERO=0,ONE=1;

    nprow = scope.np;
    npcol = scope.nq; 
    nb = get_mem_dim(M).x*get_item_dim(M).x;
    ictxt =Csys2blacs_handle(scope.get_group()->mpi_comm);
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
   
    assert(current(M).layout->get_list().size() != 0);
    current(M).solidify(current(M).layout->get_list());
    current(U).solidify(current(U).layout->get_list());
    current(V).solidify(current(V).layout->get_list());
    current(S).solidify(current(S).layout->get_list());

    char JOBU[1] ={'V'};
    char JOBV[1] ={'V'};
    int lwork = -1;
    double wkopt;

    //SCALAPACK, first, dry run to allocate buffer
    pdgesvd_(JOBU, JOBV, &nM, &nN, (double*)breakdown(M).data,&ONE,&ONE,descA, (double*)breakdown(S).data, (double*)breakdown(U).data,&ONE,&ONE,descU, (double*)breakdown(V).data,&ONE,&ONE,descV, &wkopt, &lwork, &info);

    lwork = static_cast<int> (wkopt);
    double *work = new double[lwork];

    pdgesvd_(JOBU, JOBV, &nM, &nN, (double*)breakdown(M).data,&ONE,&ONE,descA, (double*)breakdown(S).data, (double*)breakdown(U).data,&ONE,&ONE,descU, (double*)breakdown(V).data,&ONE,&ONE,descV, work, &lwork, &info);

    current(U).disperse(current(U).layout->get_list());
    current(V).disperse(current(V).layout->get_list());
    current(S).disperse(current(S).layout->get_list());

    delete[] work; //clean the working buffer
#endif
}


void syev_c_scalapack_kernel(const p_dense_matrix<double> & A, p_dense_matrix<double> & W, p_dense_matrix<double> & Z)
{
#ifdef SCALAPACK
    int descA[9],descW[9],descZ[9];
    int nmyidBLACS,nnumprocsBLACS,nContinue;
    int nContxt,nVal;  
    int i, j, k;
    int bhandle, ictxt, nprow, npcol, myrow, mycol,nb;
    int nM = get_grid_dim(A).y*get_mem_t_dim(A).y;
    int nN = get_grid_dim(A).x*get_mem_t_dim(A).x;

    int info,itemp;
    int ZERO=0,ONE=1;

    nprow = scope.np;
    npcol = scope.nq; 
    nb = get_mem_dim(A).x*get_item_dim(A).x;
    ictxt =Csys2blacs_handle(scope.get_group()->mpi_comm);
    Cblacs_gridinit( &ictxt, "Row", nprow, npcol );
    Cblacs_gridinfo( ictxt, &nprow, &npcol, &myrow, &mycol );

    int mA = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );
    int nA = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );
    int mZ = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );
    int nZ = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );
    int mW = numroc_( &nM, &nb, &myrow, &ZERO, &nprow );
    int nW = numroc_( &nN, &nb, &mycol, &ZERO, &npcol );

    descinit_(descA, &nM, &nN, &nb, &nb, &ZERO, &ZERO, &ictxt, &mA,  &info);
    descinit_(descZ, &nM, &nN, &nb, &nb, &ZERO, &ZERO, &ictxt, &mZ,  &info);
    descinit_(descW, &nM, &nN, &nb, &nb, &ZERO, &ZERO, &ictxt, &mW,  &info);
   
    assert(current(A).layout->get_list().size() != 0);
    current(A).solidify(current(A).layout->get_list());
    current(W).solidify(current(W).layout->get_list());
    current(Z).solidify(current(Z).layout->get_list());

    char JOBU[1] ={'V'};
    char JOBV[1] ={'U'};
    int lwork = -1;
    double wkopt;

    //SCALAPACK, first, dry run to allocate buffer
    pdsyev_(JOBU, JOBV, &nM, (double*)breakdown(A).data,&ONE,&ONE,descA, (double*)breakdown(W).data, (double*)breakdown(Z).data,&ONE,&ONE,descZ, &wkopt, &lwork, &info);
    std::cout << " infp " << info << std::endl;

    lwork = static_cast<int> (wkopt);
    double *work = new double[lwork];

    pdsyev_(JOBU, JOBV, &nM, (double*)breakdown(A).data,&ONE,&ONE,descA, (double*)breakdown(W).data, (double*)breakdown(Z).data,&ONE,&ONE,descZ, &wkopt, &lwork, &info);
    std::cout << " done " <<std::endl;
    current(W).disperse(current(W).layout->get_list());
    current(Z).disperse(current(Z).layout->get_list());

    delete[] work; //clean the working buffer
#endif
}

