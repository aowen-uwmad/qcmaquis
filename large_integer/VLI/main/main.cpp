
/* \cond I do not need this part in the doc*/
//system and boost
#include <boost/mpl/for_each.hpp>
#include "boost/lexical_cast.hpp"
#include <gmpxx.h>
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

//#include "main/inline_add.h"

#ifdef VLI_USE_GPU
#include "vli/detail/gpu/inner_product_gpu_accelerator.hpp"
#endif //VLI_USE_GPU
//vli
#include "vli/polynomial/vector_polynomial_cpu.hpp"
#include "vli/polynomial/polynomial.hpp"
#include "vli/vli.hpp"
#include "vli/detail/kernels_cpu.h"
//utils
#include "utils/timings.h"
#include "utils/tools.h"
#include "misc.hpp"

#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/stringize.hpp>

#define Size_vec 16// play with this 1024 - 16384
//The order __ORDER__ is passed now by cmake, see cmakelist of the main
using vli::polynomial;
using vli::vector;
typedef vli::integer<128> integer_type_cpu_128;
typedef vli::integer<192> integer_type_cpu_192;
typedef vli::integer<256> integer_type_cpu_256;
/*  -------------------------------------------------------------------- 128 bits ---------------------------------------------------------------------------------- */
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_each<__ORDER__>, vli::var<'x'>  >polynomial_type_each_x_128;
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>  >polynomial_type_each_xy_128;
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>  >polynomial_type_each_xyz_128;
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>, vli::var<'w'>  >polynomial_type_each_xyzw_128;
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_combined<__ORDER__>, vli::var<'x'> > polynomial_type_combined_x_128;
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'> > polynomial_type_combined_xy_128;
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'> > polynomial_type_combined_xyz_128;
typedef vli::polynomial< integer_type_cpu_128, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>, vli::var<'w'> > polynomial_type_combined_xyzw_128;
/*  -------------------------------------------------------------------- 192 bits ---------------------------------------------------------------------------------- */
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_each<__ORDER__>, vli::var<'x'>  >polynomial_type_each_x_192;
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>  >polynomial_type_each_xy_192;
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>  >polynomial_type_each_xyz_192;
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>, vli::var<'w'>  >polynomial_type_each_xyzw_192;
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_combined<__ORDER__>, vli::var<'x'> > polynomial_type_combined_x_192;
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'> > polynomial_type_combined_xy_192;
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'> > polynomial_type_combined_xyz_192;
typedef vli::polynomial< integer_type_cpu_192, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>, vli::var<'w'> > polynomial_type_combined_xyzw_192;
/*  -------------------------------------------------------------------- 256 bits ---------------------------------------------------------------------------------- */
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_each<__ORDER__>, vli::var<'x'>  >polynomial_type_each_x_256;
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>  >polynomial_type_each_xy_256;
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>  >polynomial_type_each_xyz_256;
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_each<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>, vli::var<'w'>  >polynomial_type_each_xyzw_256;
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_combined<__ORDER__>, vli::var<'x'> > polynomial_type_combined_x_256;
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'> > polynomial_type_combined_xy_256;
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'> > polynomial_type_combined_xyz_256;
typedef vli::polynomial< integer_type_cpu_256, vli::max_order_combined<__ORDER__>, vli::var<'x'>, vli::var<'y'>, vli::var<'z'>, vli::var<'w'> > polynomial_type_combined_xyzw_256;

typedef boost::mpl::vector<
                            polynomial_type_each_xyzw_128,// buffer can be too large cpu/gpu, be cautious
                            polynomial_type_each_xyz_128,
                            polynomial_type_each_xy_128,
                            polynomial_type_each_x_128
                          > polynomial_list_128_each;

typedef boost::mpl::vector<
                            polynomial_type_combined_xyz_128,
                            polynomial_type_combined_xy_128,
                            polynomial_type_combined_x_128 
                          > polynomial_list_128_combined;

typedef boost::mpl::vector<
                           polynomial_type_each_xyzw_192,
                           polynomial_type_each_xyz_192,
                           polynomial_type_each_xy_192,
                           polynomial_type_each_x_192 
                          > polynomial_list_192_each;

typedef boost::mpl::vector<
                           polynomial_type_combined_xyzw_192,
                           polynomial_type_combined_xyz_192,
                           polynomial_type_combined_xy_192,
                           polynomial_type_combined_x_192
                          > polynomial_list_192_combined;

typedef boost::mpl::vector<
                            polynomial_type_each_xyzw_256,// buffer can be too large cpu/gpu, be cautious
                            polynomial_type_each_xyz_256,
                            polynomial_type_each_xy_256,
                            polynomial_type_each_x_256
                          > polynomial_list_256_each;

typedef boost::mpl::vector<
                            polynomial_type_combined_xyzw_256,
                            polynomial_type_combined_xyz_256,
                            polynomial_type_combined_xy_256,
                            polynomial_type_combined_x_256
                          > polynomial_list_256_combined;

   template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
   class polynomial;

   template <typename polynomial>
   struct timescheduler;

   template <typename Coeff, int Order, class Var0, class Var1, class Var2, class Var3>
   struct timescheduler<vli::polynomial<Coeff,vli::max_order_each<Order>,Var0,Var1,Var2,Var3> >{
       static void save(double tgmp, double tcpu, double tgpu = 0){
           std::string name("MaxOrderEachTime");
           name += boost::lexical_cast<std::string>(Order);
           std::ofstream os(name.c_str(),std::ios::app);
           os << Order << " " << Coeff::numbits << " " << vli::detail::num_of_variables_helper<Var0,Var1,Var2,Var3>::value << " "<< tgmp << " " << tcpu << " " << tgpu << std::endl;
           os.close();
       };
   };   

   template <typename Coeff, int Order, class Var0, class Var1, class Var2, class Var3>
   struct timescheduler<vli::polynomial<Coeff,vli::max_order_combined<Order>,Var0,Var1,Var2,Var3> >{
       static void save(double tgmp, double tcpu, double tgpu = 0){
           std::string name("MaxOrderCombinedTime");
           name += boost::lexical_cast<std::string>(Order);
           std::ofstream os(name.c_str(),std::ios::app);
           os << Order << " " << Coeff::numbits << " " << vli::detail::num_of_variables_helper<Var0,Var1,Var2,Var3>::value << " "<< tgmp << " " << tcpu << " " << tgpu << std::endl;
           os.close();
       };
   };   
 
   struct test_case {

   template <typename Polynomial>
   void operator()(Polynomial const&) {
       std::cout.precision(5);
       //GMP polys give by class traits
       typedef typename vli::polynomial_multiply_type_gmp<Polynomial>::type Polynomial_gmp;
       typedef typename vli::polynomial_multiply_type_gmp<Polynomial>::type_res Polynomial_gmp_res;
       typedef vli::vector<Polynomial_gmp> vector_gmp;
       typedef vli::vector<Polynomial_gmp_res> vector_gmp_res;
       //VLI poly
       typedef typename vli::polynomial_multiply_result_type<Polynomial>::type Polynomial_res;
       typedef vli::vector<Polynomial> vector;
       typedef vli::vector<Polynomial_res> vector_res;
       //VLI polys
       vector v1(Size_vec),v2(Size_vec);
       Polynomial_res p1_res, p2_res;
       //GMP polys
       vector_gmp v1_gmp(Size_vec),v2_gmp(Size_vec);
       Polynomial_gmp_res p_gmp_res;
        
       tools::fill_vector_random(v1);
       tools::fill_vector_random(v2);

       tools::converter(v1,v1_gmp);
       tools::converter(v2,v2_gmp); 

       Timer tgmp("CPU GMP ");
       tgmp.begin();
           p_gmp_res = vli::detail::inner_product_cpu(v1_gmp,v2_gmp);
       tgmp.end();
        
       Timer t0("CPU ");
       t0.begin();
           p1_res = vli::detail::inner_product_cpu(v1,v2);
       t0.end();

       #ifdef VLI_USE_GPU
       Timer t1("GPU ");
       t1.begin();
           p2_res =  vli::detail::inner_product_gpu_helper<Polynomial>::inner_product_gpu(v1,v2);
       t1.end();
       #endif
 

       if(tools::equal<Polynomial>(p1_res,p_gmp_res))
               std::cout << "  OK, cpu/gmp " << t0.get_time() ;
       else 
               std::cout << "  PB, cpu/gmp " << t0.get_time() ;
          
       #ifdef VLI_USE_GPU
               if(p1_res == p2_res)
                   std::cout << " gpu " << t1.get_time() ;
               else
                   std::cout << " gpu no ok";
       #endif
               std::cout.precision(2);
       #ifdef VLI_USE_GPU
               std::cout << " G vli: "  << tgmp.get_time()/t0.get_time() << " G gpu: " << tgmp.get_time()/t1.get_time()   ; 
               timescheduler<Polynomial>::save(tgmp.get_time(),t0.get_time(),t1.get_time());
               timescheduler<Polynomial>::save(0,t0.get_time(),t1.get_time());
       #else
               std::cout << " G vli: "  << tgmp.get_time()/t0.get_time() ; 
               timescheduler<Polynomial>::save(tgmp.get_time(),t0.get_time());
       #endif
       }
   };

    template<int num>
    inline void tim_add(boost::uint64_t* x,  const boost::uint64_t* y);

    template<>
    inline void tim_add<5>(boost::uint64_t* x,  const boost::uint64_t *y){
        boost::uint64_t tmp_register;
        __asm__ __volatile__(
                 "movq  (%[y]) ,   %[tmp_register] \n\t"
                 "addq  %[tmp_register], (%[x]) \n\t"
                 "movq  8(%[y]) ,   %[tmp_register] \n\t"
                 "adcq  %[tmp_register], 8(%[x]) \n\t"
                 "adcq  0,            16(%[x]) \n\t"
                 "adcq  0,            24(%[x]) \n\t"
                 "adcq  0,            32(%[x]) \n\t"
        : [tmp_register] "=&r" (tmp_register)
        : [x] "r" (x), [y] "r" (y)
        : "memory", "cc");
    }

    template<>
    inline void tim_add<4>(boost::uint64_t* x,  const boost::uint64_t *y){
        boost::uint64_t tmp_register;
        asm __volatile__(
                 "movq  (%[y]) ,   %[tmp_register] \n\t"
                 "addq  %[tmp_register], (%[x]) \n\t"
                 "movq  8(%[y]) ,   %[tmp_register] \n\t"
                 "adcq  %[tmp_register], 8(%[x]) \n\t"
                 "adcq  $0,            16(%[x]) \n\t"
                 "adcq  $0,            24(%[x]) \n\t"
        : [tmp_register] "=&r" (tmp_register)
        : [x] "r" (x), [y] "r" (y)
        : "memory", "cc");
    }

    template<>
    inline void tim_add<3>(boost::uint64_t* x,  const boost::uint64_t *y){
        boost::uint64_t tmp_register;
        asm __volatile__(
                 "movq  (%[y]) ,   %[tmp_register] \n\t"
                 "addq  %[tmp_register], (%[x]) \n\t"
                 "movq  8(%[y]) ,   %[tmp_register] \n\t"
                 "adcq  %[tmp_register], 8(%[x]) \n\t"
                 "adcq  $0,            16(%[x]) \n\t"
        : [tmp_register] "=&r" (tmp_register)
        : [x] "r" (x), [y] "r" (y)
        : "memory", "cc");
    }

    template<>
    inline void tim_add<2>(boost::uint64_t* x,  const boost::uint64_t *y){
        boost::uint64_t tmp_register;
        asm __volatile__(
                 "movq  (%[y]) ,   %[tmp_register] \n\t"
                 "addq  %[tmp_register], (%[x]) \n\t"
                 "movq  8(%[y]) ,   %[tmp_register] \n\t"
                 "adcq  %[tmp_register], 8(%[x]) \n\t"
        : [tmp_register] "=&r" (tmp_register)
        : [x] "r" (x), [y] "r" (y)
        : "memory", "cc");
    }

    inline void mul_base(boost::uint64_t* c, boost::uint64_t   a, boost::uint64_t  b){//
        asm ("mulq %3;" :"=a"(c[0]), "=d"(c[1]) :"%0" (a), "r"(b): "cc");
    }

    template<int NumBits>
    inline void tim_mul(boost::uint64_t *x, const boost::uint64_t *y);

    template<> // 128 -> 128
    inline void tim_mul<2>(boost::uint64_t *x, const boost::uint64_t *y){
        boost::uint64_t tmp_mul[2];
        mul_base(&tmp_mul[0], x[0], y[0]);
        tmp_mul[1] += x[0] * y[1];
        tmp_mul[1] += x[1] * y[0];
        x[0] = tmp_mul[0];
        x[1] = tmp_mul[1];
    }

    template<int NumBits>
    inline void tim_mul_extend(boost::uint64_t* z,  const boost::uint64_t *x, const boost::uint64_t *y);

    template<> // 128 -> 256
    inline void tim_mul_extend<4>(boost::uint64_t* z,  const boost::uint64_t *x, const boost::uint64_t *y){
        boost::uint64_t tmp_mul[2];
        //clean z to avoid pb
        z[0] ^= z[0];
        z[1] ^= z[1];
        z[2] ^= z[2];
        z[3] ^= z[3];

        mul_base(&tmp_mul[0], x[0], y[0]);
        tim_add<2>(&z[0],&tmp_mul[0]);

        mul_base(&tmp_mul[0], x[0], y[1]);
        tim_add<2>(&z[1],&tmp_mul[0]);

        mul_base(&tmp_mul[0], x[1], y[0]);
        tim_add<3>(&z[1],&tmp_mul[0]);

        mul_base(&tmp_mul[0], x[1], y[1]);
        tim_add<2>(&z[2],&tmp_mul[0]);
    }

    template<> // 192 -> 384
    inline void tim_mul_extend<5>(boost::uint64_t* z,  const boost::uint64_t *x, const boost::uint64_t *y){
        boost::uint64_t tmp_mul[2];
        //clean z to avoid pb
        z[0] ^= z[0];
        z[1] ^= z[1];
        z[2] ^= z[2];
        z[3] ^= z[3];
        z[4] ^= z[4];
        z[5] ^= z[5];
        // first pass
        mul_base(&tmp_mul[0], x[0], y[0]);
        tim_add<2>(&z[0],&tmp_mul[0]);

        mul_base(&tmp_mul[0], x[0], y[1]);
        tim_add<2>(&z[1],&tmp_mul[0]);

        mul_base(&tmp_mul[0], x[0], y[2]);
        tim_add<2>(&z[2],&tmp_mul[0]);

        //second pass
        mul_base(&tmp_mul[0], x[1], y[2]);
        tim_add<2>(&z[3],&tmp_mul[0]);

        mul_base(&tmp_mul[0], x[1], y[0]);
        tim_add<3>(&z[1],&tmp_mul[0]); // should be 4 to check with test
        
        mul_base(&tmp_mul[0], x[1], y[1]);
        tim_add<3>(&z[2],&tmp_mul[0]);
        
        //third pass
        mul_base(&tmp_mul[0], x[2], y[2]);
        tim_add<2>(&z[4],&tmp_mul[0]);

        mul_base(&tmp_mul[0], x[2], y[0]);
        tim_add<3>(&z[2],&tmp_mul[0]); // should be 4 to check with test
        
        mul_base(&tmp_mul[0], x[2], y[1]);
        tim_add<3>(&z[3],&tmp_mul[0]);
    }


   



int main(int argc, char* argv[]) {

    vli::integer<192> a,b;
    vli::integer<384> c,d;
    a[0] = b[0] =  0xfffffffffff;
    a[1] = b[1] =  0xffffffaaaaa;
  //  vli::integer<192> a_copy(a), b_copy(b);

    a[2] = b[2] = 443545;
    a[3] = b[3] = 4343545;
   
    mpz_class agmp(a), bgmp(b), cgmp;
/*
    a*=b ;
    tim_mul<2>(&a_copy[0],&b_copy[0]);

    std::cout << a << std::endl;
    
    std::cout << a_copy << std::endl;
*/
    
    Timer t1("vli plus");
    t1.begin();
        for(int i(0); i < 1000000; ++i)
            multiply_extend(c,a,b);
    t1.end();


    Timer t2("gmp plus ");
    t2.begin();
        for(int i(0); i < 1000000; ++i)
            cgmp = agmp * bgmp;
    t2.end();

    Timer to("vli inline");
    to.begin();
        for(int i(0); i < 1000000; ++i)
            tim_mul_extend<5>(&d[0],&a[0],&b[0]);
    to.end();


    std::cout << " vli inline "  << to.get_time()<< std::endl;

    std::cout << " vli old "  << t1.get_time()<< std::endl;

    std::cout << " gmp "  << t2.get_time()<< std::endl;

    std::cout << std::hex << d << std::endl;
    std::cout << std::hex << c << std::endl;
    std::cout << std::hex << cgmp << std::endl;


    
 //   std::cout<< std::hex << e[0] << " " << e[1] << " " << e[2] << " " << e[3] << " "
 //                        << e[4] << " " << e[5] << " " << e[6] << " " << e[7] << " " << std::endl;

 //   vli::integer<512> a(e);// = {{-1,-1,-1,0}} ;
 //   vli::integer<512> b(f);

//       a+=b;
//     helper_inline_add<8>::inline_add((&e[0]),(&f[0]));
    /*
    Timer to("old ");
    to.begin();

        for(int i(0); i < 0xff; ++i){
            a+=b; 
        for(int j(0); j < 0xff; ++j){
            a+=b; 
        for(int k(0); k < 0xff; ++k){
            a+=b;}}} 

    to.end();
 
    Timer t("new ");
    t.begin();
  

        for(int i(0); i < 0xff; ++i){
            helper_inline_add<16>::inline_add((&a1[0]),(&a2
                                                        [0]));    // <-- horible but no choice conversion (BigEndian/LittleEndian)
        for(int j(0); j < 0xff; ++j){
            helper_inline_add<16>::inline_add((&a1[0]),(&a2[0]));    // <-- horible but no choice conversion (BigEndian/LittleEndian)
        for(int k(0); k < 0xff; ++k){
            helper_inline_add<16>::inline_add((&a1[0]),(&a2[0]));    // <-- horible but no choice conversion (BigEndian/LittleEndian)
        }}}

    t.end();

    std::cout << " inline "  << t.get_time()<< std::endl;
    std::cout << " vli.a "  << to.get_time()<< std::endl;
  
    std::cout << std::hex << a << std::endl;
    std::cout << std::hex << e << std::endl;
    
    if (e == a) {
      std::cout << "OK" << std::endl;
    }else{
      std::cout << "NOT OK" << std::endl;
    }

  */  

  /*  
       std::cout << " -------ASCII ART ^_^' --------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -------Size vector : " << Size_vec  << ", Order " << __ORDER__ << std::endl;

       std::cout << " -----  Max_Order_Each --------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  3 variable --------------------------------- 2 variables --------------------------------- 1 variables ----------------------------------------------------------------- " << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  256bits * 256bits = 512 bits ------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       boost::mpl::for_each<polynomial_list_256_each>(test_case());
       std::cout << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  192bits * 192bits = 384 bits ------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       boost::mpl::for_each<polynomial_list_192_each>(test_case());
       std::cout << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;

       std::cout << " -----  128bits * 128bits = 256 bits ------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       boost::mpl::for_each<polynomial_list_128_each>(test_case());
       std::cout << std::endl;

       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  Max__Order_Combined ---------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  4 variable --------------------------------- 3 variables --------------------------------- 2 variables --------------------------------- 1 variables ------------------- " << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  256bits * 256bits = 512 bits ------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       boost::mpl::for_each<polynomial_list_256_combined>(test_case());
       std::cout << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  192bits * 192bits = 384 bits ------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       boost::mpl::for_each<polynomial_list_192_combined>(test_case());
       std::cout << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       std::cout << " -----  128bits * 128bits = 256 bits ------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
       boost::mpl::for_each<polynomial_list_128_combined>(test_case());
       std::cout << std::endl;
       std::cout << " ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- " << std::endl;
*/
       return 0;
}

/* \endcond I do not need this part in the doc*/

