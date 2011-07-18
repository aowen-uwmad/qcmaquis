
/**
Do not forget NVCC will compile this file, it dislikes STL and boost ....
Just very basic C++ or C (template ok). 
*/

/**
	The link between the DMRG code (CPU) and the DMRG code (GPU) is done inside the file  kernels_gpu.h
	where the definition of function/wrapper are presented
*/

#ifdef VLI_GPU_DEBUG
#include <cstdio>
#endif //VLI_GPU_DEBUG

#include "kernels_gpu.h"
#include "detail/common_macros.h"
#include <cassert>


namespace vli {
namespace detail {
template < typename T>
__device__ void copy_kernel_gpu(T* x, T const* y, int size)
{
   #pragma unroll
   for(int i=0;i<size;++i)
       *(x+i) = *(y+i);
}    
    
template <typename T>
__device__ void negate(T* x, int vli_size)
{
    T one[8] = {1,0,0,0,0,0,0,0}; //to do find something else
    
    #pragma unroll
    for(size_type i=0; i < vli_size-1; ++i)
        *(x+i) = (~*(x+i))&BASE_MINUS;

    *(x+vli_size-1) = (~*(x+vli_size-1))&(BASE+BASE_MINUS);
    
    addition_classic_kernel_gpu(x, &one[0], vli_size);
}
    
/**
classical addition, the operation " addition " is serial but we sum all number together 
*/
template <typename T>
__device__ void addition_kernel_gpu(T* x, T const* y, int k)
{
	T carry_bit;
	*(x+k)    += *(y+k);
	carry_bit  = *(x+k) >> LOG_BASE;
	*(x+k)    %= BASE;
	*(x+k+1)  += carry_bit;
}

template <typename T>
__device__ void addition_classic_kernel_gpu(T* x, T const* y, int vli_size)
{
    #pragma unroll
    for (int i = 0; i < vli_size-1; ++i) 
        addition_kernel_gpu(x,y,i);
    
    *(x+vli_size-1) += *(y+vli_size-1);
    *(x+vli_size-1) = *(x+vli_size-1)&(BASE+BASE_MINUS);
}
    
template <typename T>
__global__ void single_addition(T* x,   T const* y, int vli_size)     
{
    addition_classic_kernel_gpu(x, y, vli_size);    
}    
    

template <typename T>
__global__ void single_substraction(T* x,   T* y, int vli_size)     
{
    negate(y,vli_size);
    addition_classic_kernel_gpu(x, y, vli_size);        
}    

    

template <typename T>    
__global__ void polynome_polynome_addition(T* x, T const* y,  int vli_size, int max_order ) 
{
    const int xIndex = blockIdx.x*blockDim.x + threadIdx.x; // all index on x
	int offset(0);
    
	if(xIndex < 2) //useless, because we set the number of thread to one
	{
        for(int i=0; i< max_order*max_order;++i){
            addition_classic_kernel_gpu(&x[offset],&y[offset],vli_size);    //1 see line 148
            offset += vli_size;
        }
    }
}
    
/**
classical multiplication, the operation " addition " is serial but we sum all number together 
*/
template <typename T>
__device__ void addition_kernel_gpu_noiter(T* x, const T* y )
{
	int carry_bit; 
	*(x)    +=  *(y);
	carry_bit  = *(x) >> LOG_BASE;
	*(x)    %= BASE;
	*(x+1)  += carry_bit; //MAYBE PB TO CHECK
}

template <typename T>
__device__ void multiplication_kernel_up_gpu(const T*   x, const T*    y, T * r)	
{
	*r	    = ((*x & MASK_UP) >> LOG_BASE_HALF ) * (*y & MASK_DOWN);	
	*(r+1)	= ((*x & MASK_UP) >> LOG_BASE_HALF ) * ((*y & MASK_UP) >> LOG_BASE_HALF);
}		
	
template <typename T>
__device__ void multiplication_kernel_down_gpu(const T*  x,  const T*   y, T * r)	
{	
	*r     = (*x & MASK_DOWN) * (*y & MASK_DOWN);
	*(r+1) = (*x & MASK_DOWN) * ((*y & MASK_UP) >> LOG_BASE_HALF);
}

template <typename T>
__device__ void multiplication_kernel_base_reshaping_gpu(T*  a, const T*   b, T * r)	
{	
	T q1(0),q2(0);
	T r1(0),r2(0);
	q1 = (*(a+1) + *b)/BASE_HALF;
	r1 = (*(a+1) + *b)%BASE_HALF;
	r1 = r1 * BASE_HALF;
	q2 = (r1 + *a)/BASE; 
	r2 = (r1 + *a)%BASE;
	*r  = r2;
	*(r+1) = q1 + q2 + *(b+1);
}

template <typename T>
__device__  void multiplication_block_gpu( const T*   x,   const  T*   y, T *r)	
{
    T a[2] = {0,0};
	T b[2] = {0,0};
	/**
	 Divide and conquer algo (see my notes - for the euclidian division tips)
		X <=> Xl Xr (half of the binary number)
	 x  Y <=> Yl Yr (half of the binary number)
	-------
	= 2^n XlYl + 2^(n/2) (XlYr + XrYl) + XrYr (multiplication_kernel_gpu_down and multiplication_kernel_gpu_up)
	------- 
	= (q1+q2 + Xl*Yl)*BASE + r2  (multiplication_kernel_base_reshaping)
	*/
	multiplication_kernel_down_gpu(x,y,a);
	multiplication_kernel_up_gpu(x,y,b);
	multiplication_kernel_base_reshaping_gpu(a,b, r);
}	

template <typename T>
__device__ void multiplication_classic_kernel_gpu(const T* x,  const T* y , T* z, int vli_size)// remove num_int maybe ?
{
	T r[2] = {0,0};	//for local block calculation
	
    for (int i = 0 ; i < vli_size; ++i) 
    {
        for(int j = 0 ; j < vli_size ; ++j)  			
        {	
            int m = j + i;
            multiplication_block_gpu(&x[i], &y[j], &r[0]);
            addition_kernel_gpu_noiter(&z[m] , &r[0]);
            addition_kernel_gpu_noiter(&z[m+1], &r[1]);
        }
    }
}
    
    
template <typename T>
__global__ void single_multiplication(T const* x, T const* y , T* z , int num_integers, int vli_size)     
{
    multiplication_classic_kernel_gpu(x, y, z, vli_size);    
}
    
template <typename T>
__global__ void monome_polynome_multiplication(T const* p, T const* m, T* res, int vli_size, int max_order)
{
    std::size_t offset(0);
    
    const int xIndex = blockIdx.x*blockDim.x + threadIdx.x; // all index on x
	
	if(xIndex < 2) //useless, because we set the number of thread to one
	{
        for(std::size_t i = 0 ; i < max_order*max_order ; i++)
        {
            offset = i*vli_size;
            multiplication_classic_kernel_gpu(&p[offset],&m[0],&res[offset],vli_size);
        }
    }
}
   
template <typename T>
__device__ void polynome_polynome_multiplication(T const* p1, T const* p2, T* res, int vli_size, int max_order)
{
    std::size_t offset0(0),offset1(0), offset2(0);
    
	{
        for(std::size_t je1 = 0; je1 < max_order; ++je1)
        {
            for(std::size_t he1 = 0; he1 < max_order; ++he1)
            {
                for(std::size_t je2 = 0; je2 < max_order - je1; ++je2)
                {
                    for(std::size_t he2 = 0; he2 < max_order - he1; ++he2)
                    {
                        T inter[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // to do find better
                        offset0 = ((je1+je2)*max_order + he1+he2)*vli_size;
                        offset1 = (je1*max_order+he1)*vli_size;                    
                        offset2 = (je2*max_order+he2)*vli_size;
                        multiplication_classic_kernel_gpu(&p1[offset1],&p2[offset2],&inter[0],vli_size);
                        addition_classic_kernel_gpu(&res[offset0],&inter[0],vli_size);
                    } 
                }
            }      
        }
    } 
    
} 


template <typename T>
__global__ void polynome_multication(T const* p1, T const* p2, T* res, int vli_size, int max_order)
{
    polynome_polynome_multiplication(p1,p2,res,vli_size,max_order);
}
    
    
template <typename T>
__global__ void inner_prod_vector(T const* p1, T const* p2, T* res, T* inter, int vli_size, int max_order, int size_vector)
{
    unsigned int xIndex = blockIdx.x*blockDim.x + threadIdx.x; // all index on x
    unsigned int tid = threadIdx.x;
    int size_poly = vli_size*max_order*max_order;
    int offset;

	if(xIndex < size_vector) 
	{   
        offset = xIndex*size_poly;
        //mutiplication
        polynome_polynome_multiplication(&p1[offset],&p2[offset],&inter[offset],vli_size,max_order); 
        
        for(unsigned int i= ( blockDim.x/2);  i>0;i>>=1){
            if(tid < i){
                addition_classic_kernel_gpu(&inter[offset],&inter[offset+i*size_poly],size_poly);
                __syncthreads();
            }
        }
        
        if(xIndex == 0){       
            copy_kernel_gpu(&res[0],&inter[0],size_poly);     //serial maximum 8 elements to copy
        }
    }
}
     
void plus_assign_gpu(TYPE*  A, TYPE const*  B, int num_integers, int vli_size)
{
    dim3 dimgrid(1,1,1);
	dim3 dimblock(1,1,1);
	single_addition <<< dimgrid, dimblock >>>(A, B, vli_size);
}

void minus_assign_gpu(TYPE*  A, TYPE*  B, int vli_size)
{
    dim3 dimgrid(1,1,1);
    dim3 dimblock(1,1,1);
    single_substraction <<< dimgrid, dimblock >>>(A, B, vli_size);
}    
    
    
void entrywise_multiplies_gpu(TYPE const* a, TYPE const* b, TYPE* c, int num_integers, int vli_size)
{
    dim3 dimgrid(1,1,1);
	dim3 dimblock(1,1,1);
	single_multiplication <<< dimgrid, dimblock >>>(a, b , c, num_integers, vli_size);
}

void inner_prod_gpu(TYPE const* A, TYPE const* B, TYPE* C, int num_integers, int vli_size)
{
    assert(false);
}
    
void poly_multiply_gpu(TYPE const* a, TYPE const* b, TYPE* c, int vli_size, int max_order)
{
   	dim3 dimgrid(1,1,1);
	dim3 dimblock(1,1,1);
    polynome_multication  <<< dimgrid, dimblock >>>(a, b , c, vli_size, max_order);
}
    
void poly_addition_gpu(TYPE* a, TYPE const* b, int vli_size, int max_order)
{
    dim3 dimgrid(1,1,1);
    dim3 dimblock(1,1,1);
    polynome_polynome_addition  <<< dimgrid, dimblock >>>(a, b, vli_size, max_order);
}
    
void poly_mono_multiply_gpu(TYPE const* a, TYPE const*b, TYPE* c, int vli_size, int max_order)
{
    dim3 dimgrid(1,1,1);
    dim3 dimblock(1,1,1);
    monome_polynome_multiplication  <<< dimgrid, dimblock >>>(a, b, c ,vli_size, max_order);
}
 
void inner_product_vector_gpu(TYPE const* A, TYPE const* B, TYPE* C, TYPE * D, int vli_size, int max_order, int vector_size)
{
    dim3 dimgrid(1,1,1);
    dim3 dimblock(vector_size,1,1);
    inner_prod_vector  <<< dimgrid, dimblock >>>(A, B, C , D ,vli_size, max_order,vector_size); 
}
    

} //namespace detail
} //namespace vli

/** TO DO make something clean later
void DeterminationGrid(dim3& dimgrid, dim3& dimblock, dim3& dimthread, int num_integers, int vli_size)
{
    //
    // TODO since an operation on a single vli_number is not parallelized yet,
    // dimblock.y = 1 instead of vli_size
    // setting it to vli_size will break the current addition_classic_kernel_gpu !!!
    //
	dimblock.x = NUM;
	dimblock.y = 1;
    //	dimblock.y = vli_size;
	dimblock.z = 1;
    
	dimthread.x = NUM;
    dimthread.x = 1;
    //	dimthread.y = vli_size;
	dimthread.z = 1;		
	
	dimgrid.x = (int(num_integers) + dimblock.x - 1)/ dimblock.x;
    //	dimgrid.y = (int(vli_size) + dimblock.y - 1)/ dimblock.y;
    dimgrid.y = 1;
	dimgrid.z = 1;
}*/

/**
 the size of each block is 16X16
 */

/**
 Do not forget, change to the desired type inside the kernels because I use special memory
 */

/**
 C = A + B
 if this kernel we introduce new arithmetic
 A. Avizienis. Signed-digit number representations for fast parallel arithmetic. IRE Transactions on electronic computers, vol 10, pages 389-400, 1961
 No carry bit ^^'
 num_integer = # of vli
 ld = size of one vli
 */
/*
 extern const int NUM_SHARED = 128; // must be equal to the size of a block
 extern const int SIZE = 8; // must be equal to the size of a block
 
 template <typename T>
 __global__ void addition_Avizienis_kernel_gpu(T x, T y , T z,int num_integer, int ld)
 {
 */
/**
 sharred array for the local calculation t must be larger (+1) due to the algo
 */
/*
 __shared__ TYPE t[NUM_SHARED+1]; 
 __shared__ TYPE w[NUM_SHARED];
 __shared__ TYPE s[NUM_SHARED];
 
 int nsize = ld*num_integer; //remove if the size of the grid is exact 
 */	
/**
 just to remember how to calculate the index of the grid
 int xIndex = blockIdx.x*blockDim.x + threadIdx.x; // all index on x
 int yIndex = blockIdx.y*blockDim.y + threadIdx.y; // all index on y
 int i = xIndex + yIndex* blockDim.x * gridDim.x ; // all index on xy 
 */
/*
 int i = threadIdx.y + blockDim.y*threadIdx.x; // index on the block (0,0)
 int j = i; // the copy for the local memory
 */

/**
 addition block by block to have a better access W/R of the shared memory
 We should have only one block on y; because y represents the size ot the vli
 
 k ( on x direction)
 --->
 ______________________ ......
 |blockId|blockId|
 |(0,0)  |(1,0)	|
 |		|		|
 |		|		|
 |		|		|
 ______________________ ......
 
 we make the addition on a block and we go to another block
 i is the index of the global memory
 j is the index of the shared memory
 
 */

/**
 inspire froan scan pattern design 
 http://developer.nvidia.com/object/cuda_training.html
 class : Stanford university CS193G Parallel Patterns I, slide 40
 */
/*
 for(int k =0 ; k < gridDim.x ; k++) //loop on the block
 {
 if(i < nsize) // To avoid the corruption of the memory card (overflow), remove is the size of the grid is perfect
 {
 s[j] = 0;
 w[j] = 0;
 t[j] = 0;
 
 s[j] = x[i] + y[i];
 
 __syncthreads(); // wait on read
 
 // To do : optimize 
 if(s[j] > BASE_MINUS2)
 t[j+1] = 1;
 if(s[j] < MINUS_BASE_PLUS2)
 t[j+1] = -1;
 if(s[j]<=BASE_MINUS2 && s[j]>= MINUS_BASE_PLUS2)
 t[j+1] = 0;
 
 w[j] = s[j] - BASE*t[j+1];
 
 z[i] = w[j] + t[j];
 
 __syncthreads(); // wait on write
 
 i+=NUM_SHARED; // iterate to go to the next block
 }
 }
 }
 */
/**
 num_integer is the number of integer to construct the very large integer
 ld is the number of very large integer (size of vector)
 */

