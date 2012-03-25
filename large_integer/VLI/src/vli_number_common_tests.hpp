



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
    BOOST_CHECK_EQUAL(a == b, false);
    
    a.negate();
    BOOST_CHECK_EQUAL(a.is_negative(), false);
    BOOST_CHECK_EQUAL(a,b);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( negate_and_construct_from_negative_int, Vli, vli_types )
{
    Vli a(-2437);
    Vli am(2437);
    a.negate();
    BOOST_CHECK_EQUAL(a,am);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_plus_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a);
    fill_random(b);
    
    Vli b_orig(b);
   
    Vli ab = a + b;
    Vli ba = b + a;
    a += b;
    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);

    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_plus_equivalence_int, Vli, vli_types )
{
    Vli a;
    fill_random(a);
    int b = rnd_valid_int<Vli>();
    int b_orig(b);

    Vli ab = a + b;
    Vli ba = b + a;
    a += b;
    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);
    
    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_by_negative_number_minus_assign_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(b);
    Vli b_orig(b);
    Vli c(a);
    
    a -= b;
    b.negate(); b_orig.negate();
    c += b;

    BOOST_CHECK_EQUAL(a,c);
    
    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( plus_assign_by_negative_number_minus_assign_equivalence_int, Vli, vli_types )
{
    Vli a;
    Vli b(rnd_valid_int<Vli>());
    Vli b_orig(b);
    Vli c(a);
std::cout << " sdjfghqobfqohebgvouebrvqve " ;
    a -= b;
    c += (-b);

    BOOST_CHECK_EQUAL(a,c);
    
    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( minus_assign_minus_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a);
    fill_random(b);
    Vli b_orig(b);
   
    Vli ab = a - b;
    Vli ba = b - a;
    a -= b;
    BOOST_CHECK_EQUAL(a,ab);
    a.negate();
    BOOST_CHECK_EQUAL(a,ba);
    
    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( minus_assign_minus_equivalence_int, Vli, vli_types )
{
    Vli a;
    fill_random(a);
    int b = rnd_valid_int<Vli>();
    int b_orig(b);
   
    Vli ab = a - b;
    a -= b;
    BOOST_CHECK_EQUAL(a,ab);
    
    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}

/*
BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_assign_multiplies_equivalence, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a);
    fill_random(b);
    Vli b_orig(b);

    Vli ab = a*b;
    Vli ba = b*a;
    a*=b;

    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);
    
    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}
*/

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_assign_multiplies_equivalence_int, Vli, vli_types )
{
    Vli a;
    fill_random(a);
    int b = rnd_valid_int<Vli>();
    int b_orig(b);

    Vli ab = a*b;
    Vli ba = b*a;
    a*=b;

    BOOST_CHECK_EQUAL(a,ab);
    BOOST_CHECK_EQUAL(a,ba);
    
    //Check that b hasn't changed
    BOOST_CHECK_EQUAL(b,b_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size-1);

    Vli a_orig(a);
    
    Vli b = a+a+a;
    Vli c = a * 3; 

    BOOST_CHECK_EQUAL(c,b);
    
    //Check that a hasn't changed
    BOOST_CHECK_EQUAL(a,a_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_int, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size-1); 
    Vli a_orig(a);
    
    Vli b = a+a+a;
    Vli c = a * 3;

    BOOST_CHECK_EQUAL(c,b);
    
    //Check that a hasn't changed
    BOOST_CHECK_EQUAL(a,a_orig);
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
    fill_random(a,Vli::size);

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
    fill_random(a,Vli::size);
    fill_random(b,Vli::size-1); 
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a-b;
    mpz_class cgmp = agmp - bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( minus_int_gmp, Vli, vli_types )
{
    Vli a;
    fill_random(a,Vli::size);
    int b = rnd_valid_int<Vli>();
    
    mpz_class agmp(a.get_str());
    
    Vli c = a-b;
    mpz_class cgmp = agmp - b;
    
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

BOOST_AUTO_TEST_CASE_TEMPLATE( comparison_vli, Vli, vli_types )
{
    BOOST_STATIC_ASSERT(Vli::size > 1);

    Vli a(0);
    Vli b(0);
    Vli a_orig(a);
    Vli b_orig(b);
    
    BOOST_CHECK_EQUAL(a<b, false);
    BOOST_CHECK_EQUAL(a>b, false);

    BOOST_CHECK_EQUAL(a,a_orig);
    BOOST_CHECK_EQUAL(b,b_orig);

    a[1] = 1; a_orig[1] = 1;
    b[1] = 1; b_orig[1] = 1;

    BOOST_CHECK_EQUAL(a<b, false);
    BOOST_CHECK_EQUAL(a>b, false);
    
    BOOST_CHECK_EQUAL(a,a_orig);
    BOOST_CHECK_EQUAL(b,b_orig);

    b[1] = 2; b_orig[1] = 2;
    a[0] = 1; a_orig[0] = 1;

    BOOST_CHECK_EQUAL(a<b, true);
    BOOST_CHECK_EQUAL(b<a, false);
    BOOST_CHECK_EQUAL(a>b, false);
    BOOST_CHECK_EQUAL(b>a, true);
    
    BOOST_CHECK_EQUAL(a,a_orig);
    BOOST_CHECK_EQUAL(b,b_orig);

    // How about different signs?
    b.negate(); b_orig.negate();

    BOOST_CHECK_EQUAL(a<b, false);
    BOOST_CHECK_EQUAL(b<a, true);
    BOOST_CHECK_EQUAL(a>b, true);
    BOOST_CHECK_EQUAL(b>a, false);
    
    BOOST_CHECK_EQUAL(a,a_orig);
    BOOST_CHECK_EQUAL(b,b_orig);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( comparison_int, Vli, vli_types )
{
    BOOST_STATIC_ASSERT(Vli::size > 1);
    
    Vli a(0);
    Vli a_orig(a);

    int zero = 0;
    int one = 1;
    int minus_one = -1;
    BOOST_CHECK_EQUAL(a<zero, false);
    BOOST_CHECK_EQUAL(a>zero, false);
    BOOST_CHECK_EQUAL(a<minus_one, false);
    BOOST_CHECK_EQUAL(a>minus_one, true);
    BOOST_CHECK_EQUAL(a<one, true);
    BOOST_CHECK_EQUAL(a>one, false);

    BOOST_CHECK_EQUAL(a,a_orig);
    BOOST_CHECK_EQUAL(zero, 0);
    BOOST_CHECK_EQUAL(one, 1);
    BOOST_CHECK_EQUAL(minus_one,-1);


    a += 1;  a_orig += 1;
    BOOST_CHECK_EQUAL(a<zero, false);
    BOOST_CHECK_EQUAL(a>zero, true);
    BOOST_CHECK_EQUAL(a<minus_one, false);
    BOOST_CHECK_EQUAL(a>minus_one, true);
    BOOST_CHECK_EQUAL(a<one, false);
    BOOST_CHECK_EQUAL(a>one, false);
    
    BOOST_CHECK_EQUAL(a,a_orig);

    a.negate(); a_orig.negate();
    BOOST_CHECK_EQUAL(a<zero, true);
    BOOST_CHECK_EQUAL(a>zero, false);
    BOOST_CHECK_EQUAL(a<minus_one, false);
    BOOST_CHECK_EQUAL(a>minus_one, false);
    BOOST_CHECK_EQUAL(a<one, true);
    BOOST_CHECK_EQUAL(a>one, false);
    
    BOOST_CHECK_EQUAL(a,a_orig);
    

    Vli b(0);
    b[1] = 1;
    Vli b_orig(b);
    BOOST_CHECK_EQUAL(b<zero, false);
    BOOST_CHECK_EQUAL(b>zero, true);
    BOOST_CHECK_EQUAL(b<minus_one, false);
    BOOST_CHECK_EQUAL(b>minus_one, true);
    BOOST_CHECK_EQUAL(b<one, false);
    BOOST_CHECK_EQUAL(b>one, true);
    
    BOOST_CHECK_EQUAL(b,b_orig);

    b.negate(); b_orig.negate();
    BOOST_CHECK_EQUAL(b<zero, true);
    BOOST_CHECK_EQUAL(b>zero, false);
    BOOST_CHECK_EQUAL(b<minus_one, true);
    BOOST_CHECK_EQUAL(b>minus_one, false);
    BOOST_CHECK_EQUAL(b<one, true);
    BOOST_CHECK_EQUAL(b>one, false);
    
    BOOST_CHECK_EQUAL(b,b_orig);
}


BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a,Vli::size/2);
    fill_random(b,Vli::size/2); 
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a*b;
    mpz_class cgmp = agmp * bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_negative_numbers_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a,Vli::size/2);
    fill_random(b,Vli::size/2); 
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

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_reverse_negative_numbers_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;    
    
    fill_random(a,Vli::size/2);
    fill_random(b,Vli::size/2); 
    b.negate();
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a*b;
    mpz_class cgmp = agmp * bgmp;
    
    b.negate();
    Vli d = a*b;
    mpz_class dgmp = agmp * (-bgmp);
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
    BOOST_CHECK_EQUAL(d.get_str(),dgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( multiplies_double_negative_numbers_gmp, Vli, vli_types )
{
    Vli a;
    Vli b;
    fill_random(a,Vli::size/2);
    fill_random(b,Vli::size/2); 
    a.negate();
    b.negate();
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());
    
    Vli c = a*b;
    mpz_class cgmp = agmp * bgmp;
    
    b.negate();
    Vli d = a*b;
    mpz_class dgmp = agmp * (-bgmp);
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
    BOOST_CHECK_EQUAL(d.get_str(),dgmp.get_str());
}


BOOST_AUTO_TEST_CASE_TEMPLATE( pointer_range_overflows, Vli, vli_types )
{
    Vli a;
    for(std::size_t i=0; i<Vli::size;++i)
        a[i] = max_int_value<Vli>::value;
    
    Vli b(a);
    b-= Vli(1);

    BOOST_CHECK_EQUAL(b,a-Vli(1));
    
    Vli a_orig(a);

    Vli *c = new Vli[3];
    c[0] = Vli(0);
    c[1] = a;
    c[2] = Vli(0);

    c[1] *= b;

    a *= b;

    BOOST_CHECK_EQUAL(c[0],Vli(0));
    BOOST_CHECK_EQUAL(c[1],a);
    BOOST_CHECK_EQUAL(c[2],Vli(0));

    delete[] c;
}

BOOST_AUTO_TEST_CASE_TEMPLATE( two_times_not_equal_minus_one, Vli, vli_types )
{
    Vli a;
    for(std::size_t i=0; i<Vli::size;++i)
        a[i] = max_int_value<Vli>::value;

    Vli c(a); 
    Vli b(2);
    
    a *= b;
    c -= 1;
    
    BOOST_CHECK_EQUAL((a == c), true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE( no_truncated_multiplication_2positive, Vli, vli_types )
{
    typedef vli::vli_cpu<typename Vli::value_type,  2*Vli::size > vli_result_type_cpu;    

    Vli a,b;
    vli_result_type_cpu c;
    
    vli::test::fill_random(a);
    vli::test::fill_random(b);

    mul(c,a,b);
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());    
    mpz_class cgmp = agmp * bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( no_truncated_multiplication_1positive_1negative, Vli, vli_types )
{
    typedef vli::vli_cpu<typename Vli::value_type,  2*Vli::size > vli_result_type_cpu;    
    
    Vli a,b;
    vli_result_type_cpu c;
    
    vli::test::fill_random(a);
    vli::test::fill_random(b);
    
    a.negate();
    mul(c,a,b);
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());    
    mpz_class cgmp = agmp * bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}

BOOST_AUTO_TEST_CASE_TEMPLATE( no_truncated_multiplication_2negative, Vli, vli_types )
{
    typedef vli::vli_cpu<typename Vli::value_type,  2*Vli::size > vli_result_type_cpu;    
    
    Vli a,b;
    vli_result_type_cpu c;
    
    vli::test::fill_random(a);
    vli::test::fill_random(b);
    
    a.negate();
    b.negate();

    mul(c,a,b);
    
    mpz_class agmp(a.get_str()), bgmp(b.get_str());    
    mpz_class cgmp = agmp * bgmp;
    
    BOOST_CHECK_EQUAL(c.get_str(),cgmp.get_str());
}



