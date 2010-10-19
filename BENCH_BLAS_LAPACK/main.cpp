#include <iostream>
#include <complex>
#include <vector>
#include <ctime>
#include <fstream>
#include <string>


typedef std::complex<double> dcomplex;
typedef std::complex<float> fcomplex;

#include "Const.h"
#include "CSMatrix.cpp"


using namespace std;


int main (int argc, char * const argv[]) 
{  
	//Datas
	int	NUM_ROWS(static_cast<size_t> (atoi(argv[1])));
	int	NUM_COLS(static_cast<size_t> (atoi(argv[2])));	
	string file;
	string extention(".txt");
	
	time_t time_start, time_end;
	ofstream out;
	
/*------------------------------------------------ DGEMM Benchmark--------------------------------------------------------------*/	

#ifdef DOUBLE	
	
#ifdef DGEMM
	{ //<- tip for the memory
	//outputfile
	file = "time_DGEMM_";	
	file = file + argv[4] + extention;
	
	//Matrix NUM_ROWS >= NUM_COLS
	CSMatrix<double> A(NUM_ROWS,NUM_COLS);
	CSMatrix<double> B(NUM_COLS,NUM_ROWS);
	CSMatrix<double> C(NUM_ROWS,NUM_ROWS);

	double one = 1;
	double zero = 0;
	
	time_start = time(NULL);

	//dgemm_("T",  "T", &A.NUM_ROWS(), &NUM_ROWS, &NUM_ROWS, &one, &A(0,0), &NUM_ROWS, &B(0,0), &NUM_COLS, &zero, &C(0,0), &NUM_ROWS);
		
	int ANumRow = A.GetnNumRow();
	int ANumCol = A.GetnNumCol();
	int BNumRow = B.GetnNumRow();		
	int CNumRow = C.GetnNumRow();
		
		
	dgemm_("T",  "T", &ANumRow, &BNumRow, &ANumCol, &one, &A(0,0), &ANumRow, &B(0,0), &BNumRow, &zero, &C(0,0), &CNumRow);

	time_end = time(NULL);
	
	
	out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
	out.close();
	}
#endif	

/*------------------------------------------------ DGESVD Benchmark--------------------------------------------------------------*/	
	
#ifdef DGESVD	
	{ //<- tip for the memory
	//outputfile
	file = "time_DGESVD_";	
	file = file + argv[4] + extention;	
		
	//Matrix
	CSMatrix<double> A(NUM_ROWS,NUM_COLS);
	CSMatrix<double> U(NUM_ROWS,NUM_ROWS);
	CSMatrix<double> VT(NUM_COLS,NUM_COLS);	
	CSVector<double> S(NUM_COLS);
	
	int lwork = -1;
	int info = 0;
	double wkopt;
	
	int ANumRow = A.GetnNumRow();
	int ANumCol = A.GetnNumCol();
	int UNumRow = U.GetnNumRow();		
	int VTNumRow = VT.GetnNumRow();					
		
	time_start = time(NULL);	
		
	dgesvd_("A", "A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&wkopt,&lwork,&info); 
		
	lwork = static_cast<long int> (wkopt);
	CSVector<double> work(lwork);
	
	dgesvd_("A", "A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&work(0),&lwork,&info); 
			
	time_end = time(NULL);			
		
	out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
	out.close();					
	}	
#endif	
	
/*------------------------------------------------ DGESDD Benchmark--------------------------------------------------------------*/	
	
#ifdef DGESDD	
	{ //<- tip for the memory
		//outputfile
		file = "time_DGESDD_";	
		file = file + argv[4] + extention;	
		
		//Matrix
		CSMatrix<double> A(NUM_ROWS,NUM_COLS);
		CSMatrix<double> U(NUM_ROWS,NUM_ROWS);
		CSMatrix<double> VT(NUM_COLS,NUM_COLS);	
		CSVector<double> S(NUM_COLS);
		CSVector<int> IWORK(8*min(NUM_ROWS,NUM_COLS));

		int lwork = -1;
		int info = 0;
		double wkopt;
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		int UNumRow = U.GetnNumRow();		
		int VTNumRow = VT.GetnNumRow();					
		
		time_start = time(NULL);	

		dgesdd_("A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&wkopt,&lwork,&IWORK(0),&info);
		
		lwork = static_cast<long int> (wkopt);
		CSVector<double> work(lwork);
		
		dgesdd_("A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&work(0),&lwork,&IWORK(0),&info);

		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
			out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
	
	
	/*------------------------------------------------ DGEQRF Benchmark--------------------------------------------------------------*/	
	
#ifdef DGEQRF	
	{ //<- tip for the memory
		//outputfile
		file = "time_DGEQRF_";	
		file = file + argv[4] + extention;	
		
		//Matrix
		CSMatrix<double> A(NUM_ROWS,NUM_COLS);
		CSVector<double> TAU(min(NUM_COLS,NUM_ROWS));
				
		int info = 0;
		int lwork = -1;
		double wkopt = 0;

		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		
		time_start = time(NULL);	
		
		dgeqrf_(&ANumRow,&ANumCol,&A(0,0),&ANumRow,&TAU(0), &wkopt,&lwork,&info);
		
		lwork = static_cast<int> (wkopt);
		CSVector<double> work(lwork);
		
		dgeqrf_(&ANumRow,&ANumCol,&A(0,0),&ANumRow,&TAU(0), &work(0),&lwork,&info);		
		
		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
	
	/*------------------------------------------------ DSYTRF Benchmark--------------------------------------------------------------*/	//  symetric matrix = square matrix
	
#ifdef DSYTRF	
	{ //<- tip for the memory
		//outputfile
		file = "time_DSYTRF_";	
		file = file + argv[4] + extention;	
		
		//Matrix
		CSMatrix<double> A(NUM_ROWS,NUM_COLS);
		CSVector<int> IPIV(NUM_ROWS);
		
		int info = 0;
		int lwork = -1;
		double wkopt = 0;
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		
		time_start = time(NULL);	
		
		dsytrf_("U",&ANumRow,&A(0,0),&ANumRow,&IPIV(0), &wkopt,&lwork,&info);
		
		lwork = static_cast<int> (wkopt);
		CSVector<double> work(lwork);
		
		dsytrf_("U",&ANumRow,&A(0,0),&ANumRow,&IPIV(0), &work(0), &lwork,&info);	
		
		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
	
	/*------------------------------------------------ DSYEVD Benchmark--------------------------------------------------------------*/	//  symetric matrix = square matrix
	
#ifdef DSYEVD	
	{ //<- tip for the memory
		//outputfile
		file = "time_DSYEVD_";	
		file = file + argv[4] + extention;	
		
		CSMatrix<double> A(NUM_ROWS,NUM_COLS);
		CSVector<double> N(NUM_COLS);
		
		int info = 0;
		int lwork = -1;
		double wkopt = 0;
		int wiwork;
		int liwork = -1;
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		

		time_start = time(NULL);	
		
		dsyevd_("V", "L", &ANumRow, &A(0,0),&ANumRow, &N(0), &wkopt,&lwork,&wiwork,&liwork,&info);
		
		lwork = static_cast< int> (wkopt);
		CSVector<double> work(lwork);
		
		liwork = static_cast< int> (wiwork);
		CSVector<int> iwork(liwork);
		
		dsyevd_("V", "L", &ANumRow, &A(0,0),  &ANumRow, &N(0),&work(0),&lwork,&iwork(0),&liwork,&info);
		
		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
#endif // FIN ifdef DOUBLE
	
	
#ifdef COMPLEX
	
/*------------------------------------------------ ZGEMM Benchmark--------------------------------------------------------------*/				
	
#ifdef ZGEMM
	{ //<- tip for the memory
		//outputfile
		file = "time_ZGEMM_";	
		file = file + argv[4] + extention;
		
		//Matrix NUM_ROWS >= NUM_COLS
		CSMatrix<dcomplex> A(NUM_ROWS,NUM_COLS);
		CSMatrix<dcomplex> B(NUM_COLS,NUM_ROWS);
		CSMatrix<dcomplex> C(NUM_ROWS,NUM_ROWS);
		
		dcomplex one = 1;
		dcomplex zero = 0;
		
		time_start = time(NULL);
		
		//dgemm_("T",  "T", &A.NUM_ROWS(), &NUM_ROWS, &NUM_ROWS, &one, &A(0,0), &NUM_ROWS, &B(0,0), &NUM_COLS, &zero, &C(0,0), &NUM_ROWS);
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		int BNumRow = B.GetnNumRow();		
		int CNumRow = C.GetnNumRow();
		
		
		zgemm_("T",  "T", &ANumRow, &BNumRow, &ANumCol, &one, &A(0,0), &ANumRow, &B(0,0), &BNumRow, &zero, &C(0,0), &CNumRow);
		
		time_end = time(NULL);
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();
	}
#endif	
	
	/*------------------------------------------------ ZGESVD Benchmark--------------------------------------------------------------*/	
	
#ifdef ZGESVD	
	{ //<- tip for the memory
		//outputfile
		file = "time_ZGESVD_";	
		file = file + argv[4] + extention;	
		
		//Matrix
		CSMatrix<dcomplex> A(NUM_ROWS,NUM_COLS);
		CSMatrix<dcomplex> U(NUM_ROWS,NUM_ROWS);
		CSMatrix<dcomplex> VT(NUM_COLS,NUM_COLS);	
		CSVector<double> S(NUM_COLS);
		CSVector<double> rwork(5*min(NUM_ROWS,NUM_COLS));
		
		int lwork = -1;
		int info = 0;
		dcomplex wkopt;
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		int UNumRow = U.GetnNumRow();		
		int VTNumRow = VT.GetnNumRow();					
		
		time_start = time(NULL);	
		
		zgesvd_("A", "A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&wkopt,&lwork,&rwork(0),&info); 
		
		lwork = static_cast< int> (wkopt.real());
		CSVector<dcomplex> work(lwork);
		
		zgesvd_("A", "A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&work(0),&lwork,&rwork(0),&info); 
		
		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
	/*------------------------------------------------ ZGESDD Benchmark--------------------------------------------------------------*/	
	
#ifdef ZGESDD	
	{ //<- tip for the memory
		//outputfile
		file = "time_ZGESDD_";	
		file = file + argv[4] + extention;	
		
		//Matrix
		CSMatrix<dcomplex> A(NUM_ROWS,NUM_COLS);
		CSMatrix<dcomplex> U(NUM_ROWS,NUM_ROWS);
		CSMatrix<dcomplex> VT(NUM_COLS,NUM_COLS);	
		CSVector<double> S(NUM_COLS);
		CSVector<int> IWORK(8*min(NUM_ROWS,NUM_COLS));
		CSVector<double> rwork(5*min(NUM_ROWS,NUM_COLS)*min(NUM_ROWS,NUM_COLS) + 5*min(NUM_ROWS,NUM_COLS));
		
		int lwork = -1;
		int info = 0;
		dcomplex wkopt;
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		int UNumRow = U.GetnNumRow();		
		int VTNumRow = VT.GetnNumRow();					
		
		time_start = time(NULL);	
		
		zgesdd_("A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&wkopt,&lwork,&rwork(0),&IWORK(0),&info);
		
		lwork = static_cast< int> (wkopt.real());
		CSVector<dcomplex> work(lwork);
		
		zgesdd_("A",&ANumRow,&ANumCol,&A(0,0),&ANumRow,&S(0),&U(0,0),&UNumRow,&VT(0,0),&VTNumRow,&work(0),&lwork,&rwork(0),&IWORK(0),&info);
		
		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
	
	/*------------------------------------------------ ZGEQRF Benchmark--------------------------------------------------------------*/	
	
#ifdef ZGEQRF	
	{ //<- tip for the memory
		//outputfile
		file = "time_ZGEQRF_";	
		file = file + argv[4] + extention;	
		
		//Matrix
		CSMatrix<dcomplex> A(NUM_ROWS,NUM_COLS);
		CSVector<dcomplex> TAU(min(NUM_COLS,NUM_ROWS));
		
		int info = 0;
		int lwork = -1;
		dcomplex wkopt;
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		
		time_start = time(NULL);	
		
		zgeqrf_(&ANumRow,&ANumCol,&A(0,0),&ANumRow,&TAU(0), &wkopt,&lwork,&info);
		
		lwork = static_cast<int> (wkopt.real());
		CSVector<dcomplex> work(lwork);
		
		zgeqrf_(&ANumRow,&ANumCol,&A(0,0),&ANumRow,&TAU(0), &work(0),&lwork,&info);		
		
		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
	/*------------------------------------------------ CHEEVD Benchmark--------------------------------------------------------------*/	
	
#ifdef CHEEVD	
	{ //<- tip for the memory
		//outputfile
		file = "time_CHEEVD_";	
		file = file + argv[4] + extention;	
		
		CSMatrix<fcomplex> A(NUM_ROWS,NUM_COLS);
				
		
		CSVector<float> N(NUM_COLS);
		
		int info = 0;
		int lwork = -1;
		fcomplex wkopt = (0,0);
		float rwork =0;
		int lrwork = -1;
		int wiwork;
		int liwork = -1;
		
		int ANumRow = A.GetnNumRow();
		int ANumCol = A.GetnNumCol();
		
		time_start = time(NULL);	
		
		cheevd_("V", "L", &ANumRow, &A(0,0),&ANumRow, &N(0), &wkopt,&lwork,&rwork,&lrwork, &wiwork,&liwork,&info);
		
		CSVector<fcomplex> work(wkopt.real());
		lwork = wkopt.real();
		
		int riwork = static_cast<int> (lwork);
		CSVector<float> workr(riwork);
		
	//	liwork = static_cast< int> (wiwork);
		CSVector<int> iwork(wiwork);
		
		cheevd_("V", "L", &ANumRow, &A(0,0),&ANumRow, &N(0), &work(0),&lwork,&workr(0),&riwork, &iwork(0),&wiwork,&info);
		
		time_end = time(NULL);			
		
		out.open(file.c_str(),ios::app);
		out <<  time_end - time_start << " " << argv[3]  << " " << argv[4] << endl;
		out.close();					
	}	
#endif	
	
	
	
#endif // FIN ifdef COMPLEX	
	
	
	
    return 0;
}
