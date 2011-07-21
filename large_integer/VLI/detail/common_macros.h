/**
  Some common macros
*/

//#define TYPE_VLI int
//#define NUM 32;
//#define VLI_SIZE 4;
//#define NUM 1;

// BASE 2**8
/*
#define LOG_BASE			 0x8   // 8 
#define LOG_BASE_HALF		 0x4   // 4
#define BASE				 0x100 // 256
#define BASE_HALF			 0x10  // 16
#define BASE_MINUS			 0xFF  //255  // 16
#define MASK_DOWN			 0xF
#define MASK_UP				 0xF0
#define PARITYBITINT         0x1F  // TO calculate the parity bit with int  
#define PARITYBITLONGINT     0x1F  // TO calculate the parity bit with int  
*/

//#define MINUS_BASE_PLUS2	 -254 for Avizienis
//#define BASE_MINUS2			 0xFE  // 254   



//BASE 2**30 unsigned + one carry = parity bit 
/*
#define MAX_VALUE            0x3FFFFFFF
#define LOG_BASE			 30   
#define LOG_BASE_HALF		 15   
#define BASE				 0x40000000 
#define BASE_HALF			 0x8000 
#define BASE_MINUS			 0x3FFFFFFF
#define MASK_DOWN			 0x7FFF
#define MASK_UP				 0x3FFF8000
*/

//BASE 2**62 unsigned + one carry = parity bit 
#define MAX_VALUE            0x3FFFFFFFFFFFFFFF
#define LOG_BASE			 62   
#define LOG_BASE_HALF		 31   
#define BASE				 0x4000000000000000
#define BASE_HALF			 0x8000000000000000
#define BASE_MINUS			 0x3FFFFFFFFFFFFFFF
#define MASK_DOWN			 0x7FFFFFFF
#define MASK_UP				 0x3FFFFFFF80000000


//#define BASE_MINUS2			 0x7FFFFFFD
//#define MINUS_BASE_PLUS2	 -0x7FFFFFFD Avizienis




namespace vli {

    typedef std::size_t size_int;
    typedef std::size_t size_type;
}
