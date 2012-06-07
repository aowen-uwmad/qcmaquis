#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/config/limits.hpp>

//g++ -DNUM=1 -E -P -I /opt/boost/include/ main.cpp | sed  "s/n/; \\`echo -e '\n\r      '`/g"
#define MAX_ITERATION 7
#define MAX_ITERATION_MINUS_ONE 6
#define AOS 1 // if you change this value you move to the SOA structure be carefull 
//give the name of the function addition
#define NAME_ADD_NBITS_PLUS_NBITS(n)                 BOOST_PP_CAT(BOOST_PP_CAT(add,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)))  /* addnx64_nx64 */
#define NAME_ADD_NBITS_PLUS_NMINUS1BITS(n)           BOOST_PP_CAT(BOOST_PP_CAT(add,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,BOOST_PP_CAT(BOOST_PP_ADD(n,1),x64)))  /* addnx64_(n-1)x64 starts from 128_64 */
#define NAME_ADD_NBITS_PLUS_64BITS(n)                BOOST_PP_CAT(BOOST_PP_CAT(add,BOOST_PP_CAT(BOOST_PP_ADD(n,3),x64)),BOOST_PP_CAT(_,64))  /* addnx64_64 starts from 192_64 */
//give the name of the function substraction        
#define NAME_SUB_NBITS_MINUS_NBITS(n)                BOOST_PP_CAT(BOOST_PP_CAT(sub,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)))  /* addnx64_nx64 */
#define NAME_SUB_NBITS_MINUS_NMINUS1BITS(n)          BOOST_PP_CAT(BOOST_PP_CAT(sub,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,BOOST_PP_CAT(BOOST_PP_ADD(n,1),x64)))  /* addnx64_(n-1)x64 starts from 128_64 */
#define NAME_SUB_NBITS_MINUS_64BITS(n)               BOOST_PP_CAT(BOOST_PP_CAT(sub,BOOST_PP_CAT(BOOST_PP_ADD(n,3),x64)),BOOST_PP_CAT(_,64))  /* addnx64_64 starts from 192_64 */
//give the name of the multiplication VLI<64*n> *= long
#define NAME_MUL_NBITS_64BITS(n)                     BOOST_PP_CAT(BOOST_PP_CAT(mul,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,64)) /* mulnx64_64*/
//give the name of the if statement for the multiplication VLI<64*n> *= long 
#define NAME_CONDITIONAL_MUL_NBITS_64BITS(n)         BOOST_PP_STRINGIZE(BOOST_PP_CAT(BOOST_PP_CAT(_IsNegative   ,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,64))) /* _IsNegativenx64_64, for the input sign */
#define NAME_RES_CONDITIONAL_MUL_NBITS_64BITS(n)     BOOST_PP_STRINGIZE(BOOST_PP_CAT(BOOST_PP_CAT(_IsNegativeRes,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,64))) /* _IsNegativeResnx64_64, for the output sign */

//The pp is limited to 256 for arithmetic therefore I calculated intermediate value, close your eyes
//Addition
#define add2x64_2x64 add128_128
#define add3x64_3x64 add192_192
#define add4x64_4x64 add256_256
#define add5x64_5x64 add320_320
#define add6x64_6x64 add384_384
#define add7x64_7x64 add448_448
#define add8x64_8x64 add512_512
#define add9x64_9x64 add576_576

#define add2x64_64 add128_64
#define add3x64_64 add192_64
#define add4x64_64 add256_64
#define add5x64_64 add320_64
#define add6x64_64 add384_64
#define add7x64_64 add448_64
#define add8x64_64 add512_64
#define add9x64_64 add576_64

#define add2x64_1x64 add128_64
#define add3x64_2x64 add192_128    
#define add4x64_3x64 add256_192
#define add5x64_4x64 add320_256
#define add6x64_5x64 add384_320
#define add7x64_6x64 add448_384
#define add8x64_7x64 add512_448
#define add9x64_8x64 add576_512

//Substraction
#define sub2x64_2x64 sub128_128
#define sub3x64_3x64 sub192_192
#define sub4x64_4x64 sub256_256
#define sub5x64_5x64 sub320_320
#define sub6x64_6x64 sub384_384
#define sub7x64_7x64 sub448_448
#define sub8x64_8x64 sub512_512
#define sub9x64_9x64 sub576_576

#define sub2x64_64 sub128_64
#define sub3x64_64 sub192_64
#define sub4x64_64 sub256_64
#define sub5x64_64 sub320_64
#define sub6x64_64 sub384_64
#define sub7x64_64 sub448_64
#define sub8x64_64 sub512_64
#define sub9x64_64 sub576_64

#define sub2x64_1x64 sub128_64
#define sub3x64_2x64 sub192_128    
#define sub4x64_3x64 sub256_192
#define sub5x64_4x64 sub320_256
#define sub6x64_5x64 sub384_320
#define sub7x64_6x64 sub448_384
#define sub8x64_7x64 sub512_448

//Multiplication 
#define mul2x64_64 mul128_64
#define mul3x64_64 mul192_64
#define mul4x64_64 mul256_64
#define mul5x64_64 mul320_64
#define mul6x64_64 mul384_64
#define mul7x64_64 mul448_64
#define mul8x64_64 mul512_64

//macro to get the correct name of the register
#define R(n)        BOOST_PP_STRINGIZE(BOOST_PP_CAT(%%r, BOOST_PP_ADD(8,n))) // give register starts from r8 
#define RCLOTHER(n) BOOST_PP_STRINGIZE(BOOST_PP_CAT(r, BOOST_PP_ADD(8,n)))  // for clother
#define PPS(m,n)    BOOST_PP_STRINGIZE(BOOST_PP_MUL(BOOST_PP_MUL(m,n),8)) // m*n*8, 8 because long int, if one day SoA today AoS

// move ASM operator to get the data from the mem
#define LOAD_register(z, n, unused) "movq "PPS(AOS,n)"(%%rdi)                 ,"R(n)"                 \n" /* load 0x??(%%rdi) */     
// addition ASM operators
#define  ADC_register(z, n, unused) "adcq "PPS(AOS,BOOST_PP_ADD(n,1))"(%%rsi) ,"R(BOOST_PP_ADD(n,1))" \n" /* adcq rsi + rdi + CB  */     
#define ADC0_register(z, n, unused) "adcq $0x0                                ,"R(BOOST_PP_ADD(n,1))" \n" /* adcq 0 + rdi + CB    */     
// substraction ASM operators 
#define  SBB_register(z, n, unused) "sbbq "PPS(AOS,BOOST_PP_ADD(n,1))"(%%rsi) ,"R(BOOST_PP_ADD(n,1))" \n" /* adcq rsi - rdi - SB  */     
#define SBB0_register(z, n, unused) "sbbq $0x0                                ,"R(BOOST_PP_ADD(n,1))" \n" /* adcq 0 - rdi - SB    */     
// multiplication VLI<n*64> *= 64 bits, note : results are saved in to r8, r9, r10 .... thus for the first iteration I move direclty inside
#define  MUL_register(z, n, unused) "mulq "PPS(1,BOOST_PP_ADD(n,1))"(%%rdi)             \n" /* mulq r??*rax */                \
                                    "addq %%rax            ,"R(BOOST_PP_ADD(n,1))"      \n" /* add hia?b? + loa?b? */         \
                                    "movq %%rdx            ,"R(BOOST_PP_ADD(n,2))"      \n" /* save the hi into rcx */        \
                                    "adcq $0               ,"R(BOOST_PP_ADD(n,2))"      \n" /* perhaps carry */               \
                                    "movq %%rbx            ,%%rax                       \n" /* reload rax(a0) from the rbx */ \
// negate for 2CM method, combine with ADC0_register macro
#define NOT_register(z, n, unused)  "notq "R(n)"                                        \n" /* start C2M negate */ 
// movi ASM operators to set up the data into the mem
#define SAVE_register(z, n, unused) "movq "R(n)"           ,"PPS(AOS,n)"(%%rdi)         \n" /* save 0x??(%%rdi) */     
// generate the list of registers clother
#define CLOTHER_register(z, n, unused) RCLOTHER(n), /* "r8","r9", ... */
