#define BOOST_TEST_MODULE vli_cpu
#include <iostream>
#include <boost/test/unit_test.hpp>


#define SIZE_BITS 256

/*
BOOST_AUTO_TEST_CASE( constructors_test )
{
	gpu::gpu_manager* GPU;
	GPU->instance();
	
    vli_vector_gpu< vli_cpu<int> > a(10);
    vli_vector_gpu< vli_cpu<int> > b(a);
    vli_vector_gpu< vli_cpu<int> > c(10);
	
	c=a+b;
    std::cout << c << std::endl;	
//	BOOST_CHECK_EQUAL(a[0],b[0]);

	GPU->instance().destructor();
}


BOOST_AUTO_TEST_CASE( addition_vector )
{
	gpu::gpu_manager* GPU;
	GPU->instance();
	
    vli_vector_cpu<vli_cpu<int> > a(10);
    vli_vector_cpu<vli_cpu<int> > b(a);
    vli_vector_cpu<vli_cpu<int> > c(10);
	
	
	for (int i =0; i < 10; i++)
	{
	    a[i][0] = 255;
	    a[i][1] = 255;
	    a[i][2] = 255;
		
		b[i][0] = 255;
	    b[i][1] = 255;
	    b[i][2] = 255;
	}
	
	vli_vector_gpu<vli_cpu<int> > a_gpu(a);
	vli_vector_gpu<vli_cpu<int> > b_gpu(b);
		
//	c = a+b;

//	c_gpu(c);//=a_gpu+b_gpu;
	
	std::cout << a << std::endl;	
	std::cout << a_gpu << std::endl;	

//	BOOST_CHECK_EQUAL(a[0],b[0]);
	GPU->instance().destructor();
}*/
