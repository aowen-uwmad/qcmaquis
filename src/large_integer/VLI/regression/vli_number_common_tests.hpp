
BOOST_AUTO_TEST_CASE_TEMPLATE( constructors_test, Vli, vli_types )
{
    Vli a;
    Vli b(0);

    BOOST_CHECK_EQUAL(a,b);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( equal_operator, Vli, vli_types )
{
    Vli a(0);
    Vli b;

    for(typename Vli::size_type i=0; i < Vli::size; ++i)
    {
        b[i] = 1;
        BOOST_CHECK_EQUAL((a == b),false);
        b[i] = 0;
    }

    BOOST_CHECK_EQUAL(a,b);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( copy_constructor_and_assignment, Vli, vli_types )
{ 
    Vli a;
    fill_random(a); 
    Vli b(a);
    BOOST_CHECK_EQUAL(a,b);

    Vli c;
    fill_random(c);

    c = b;
    BOOST_CHECK_EQUAL(c,b);
    
    // Check if c stays the same if we change b
    b[1] = 57642;
    BOOST_CHECK_EQUAL(c == b, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( negate, Vli, vli_types )
{
    Vli a;
    fill_random(a);    

    Vli b(a);
    BOOST_CHECK_EQUAL(a.is_negative(), false);
    
    a.negate();
    BOOST_CHECK_EQUAL(a.is_negative(), true);
    
    a.negate();
    BOOST_CHECK_EQUAL(a.is_negative(), false);
    BOOST_CHECK_EQUAL(a,b);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( negate_and_construct_from_negative_int, Vli, vli_types )
{
    Vli a(2437284);
    Vli am(-2437284);
    a.negate();
    BOOST_CHECK_EQUAL(a,am);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_plus_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a);
    fill_random(b);
   
    Vli ab = a + b;
    Vli ba = b + a;
    a += b;
    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_plus_equivalence_int, Vli, vli_types )
{
    Vli a;
    fill_random(a);
    int b = rnd_valid_int<Vli>();

    Vli ab = a + b;
    Vli ba = b + a;
    a += b;
    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_by_negative_number_minus_assign_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(b);

    Vli c(a);
    
    a -= b;
    b.negate();
    c += b;

    BOOST_CHECK_EQUAL(a,c);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_by_negative_number_minus_assign_equivalence_int, Vli, vli_types )
{
    Vli a;
    int b = rnd_valid_int<Vli>();
    Vli c(a);

    a -= b;
    c += (-b);

    BOOST_CHECK_EQUAL(a,c);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( minus_assign_minus_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a);
    fill_random(b);
   
    Vli ab = a - b;
    Vli ba = b - a;
    a -= b;
    BOOST_CHECK_EQUAL(a,ab);
    a.negate();
    BOOST_CHECK_EQUAL(a,ba);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( minus_assign_minus_equivalence_int, Vli, vli_types )
{
    Vli a;
    fill_random(a);
    int b = rnd_valid_int<Vli>();
   
    Vli ab = a - b;
    a -= b;
    BOOST_CHECK_EQUAL(a,ab);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_assign_multiplies_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a);
    fill_random(b);

    Vli ab = a*b;
    Vli ba = b*a;
    a*=b;

    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_assign_multiplies_equivalence_int, Vli, vli_types )
{
    Vli a;
    fill_random(a);
    int b = rnd_valid_int<Vli>();

    Vli ab = a*b;
    Vli ba = b*a;
    a*=b;

    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size-1);
    
    Vli b = a+a+a;
    Vli c = a * Vli(3);

    BOOST_CHECK_EQUAL(c,b);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_int, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size-1); 
    
    Vli b = a+a+a;
    Vli c = a * 3;

    BOOST_CHECK_EQUAL(c,b);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a,Vli::size-1);
    fill_random(b,Vli::size-1); 
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a+b;
    mpz_class cgmp = agmp + bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_int_gmp, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size-1);

    int b = rnd_valid_int<Vli>();
    
    mpz_class agmp(a.get_str());
    
    Vli c = a+b;
    mpz_class cgmp = agmp + b;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( minus_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a,Vli::size-1);
    fill_random(b,Vli::size-1); 
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a-b;
    mpz_class cgmp = agmp - bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( minus_int_gmp, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size-1);
    int b = rnd_valid_int<Vli>();
    
    mpz_class agmp(a.get_str());
    
    Vli c = a-b;
    mpz_class cgmp = agmp - b;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a,Vli::size/2);
    fill_random(b,Vli::size/4); 
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a*b;
    mpz_class cgmp = agmp * bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_int_gmp, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size/2);
    int b = rnd_valid_int<Vli>();
    
    mpz_class agmp(a.get_str());
    
    Vli c = a*b;
    mpz_class cgmp = agmp * b;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_negative_numbers_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a,Vli::size/2);
    fill_random(b,Vli::size/4); 
    a.negate();
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a*b;
    mpz_class cgmp = agmp * bgmp;
    
    b.negate();
    Vli d = a*b;
    mpz_class dgmp = agmp * (-bgmp);
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
    BOOST_CHECK_EQUAL(d.get_str(),dgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( comparison_vli, Vli, vli_types )
{
    BOOST_STATIC_ASSERT(Vli::size > 1);

    Vli a(0);
    Vli b(0);
    
    BOOST_CHECK_EQUAL(a<b, false);
    BOOST_CHECK_EQUAL(a>b, false);
    
    a[1] = 1;
    b[1] = 1;

    BOOST_CHECK_EQUAL(a<b, false);
    BOOST_CHECK_EQUAL(a>b, false);

    b[1] = 2;
    a[0] = 1;

    BOOST_CHECK_EQUAL(a<b, true);
    BOOST_CHECK_EQUAL(b<a, false);
    BOOST_CHECK_EQUAL(a>b, false);
    BOOST_CHECK_EQUAL(b>a, true);

    // How about different signs?
    b.negate();

    BOOST_CHECK_EQUAL(a<b, false);
    BOOST_CHECK_EQUAL(b<a, true);
    BOOST_CHECK_EQUAL(a>b, true);
    BOOST_CHECK_EQUAL(b>a, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( comparison_int, Vli, vli_types )
{
    BOOST_STATIC_ASSERT(Vli::size > 1);
    
    Vli a(0);

    int zero = 0;
    int one = 1;
    int minus_one = -1;
    BOOST_CHECK_EQUAL(a<zero, false);
    BOOST_CHECK_EQUAL(a>zero, false);
    BOOST_CHECK_EQUAL(a<minus_one, false);
    BOOST_CHECK_EQUAL(a>minus_one, true);
    BOOST_CHECK_EQUAL(a<one, true);
    BOOST_CHECK_EQUAL(a>one, false);

    a+=1;
    BOOST_CHECK_EQUAL(a<zero, false);
    BOOST_CHECK_EQUAL(a>zero, true);
    BOOST_CHECK_EQUAL(a<minus_one, false);
    BOOST_CHECK_EQUAL(a>minus_one, true);
    BOOST_CHECK_EQUAL(a<one, false);
    BOOST_CHECK_EQUAL(a>one, false);

    a.negate();
    BOOST_CHECK_EQUAL(a<zero, true);
    BOOST_CHECK_EQUAL(a>zero, false);
    BOOST_CHECK_EQUAL(a<minus_one, false);
    BOOST_CHECK_EQUAL(a>minus_one, false);
    BOOST_CHECK_EQUAL(a<one, true);
    BOOST_CHECK_EQUAL(a>one, false);
    


    Vli b(0);
    b[1] = 1;
    BOOST_CHECK_EQUAL(b<zero, false);
    BOOST_CHECK_EQUAL(b>zero, true);
    BOOST_CHECK_EQUAL(b<minus_one, false);
    BOOST_CHECK_EQUAL(b>minus_one, true);
    BOOST_CHECK_EQUAL(b<one, false);
    BOOST_CHECK_EQUAL(b>one, true);

    b.negate();
    BOOST_CHECK_EQUAL(b<zero, true);
    BOOST_CHECK_EQUAL(b>zero, false);
    BOOST_CHECK_EQUAL(b<minus_one, true);
    BOOST_CHECK_EQUAL(b>minus_one, false);
    BOOST_CHECK_EQUAL(b<one, true);
    BOOST_CHECK_EQUAL(b>one, false);
}
