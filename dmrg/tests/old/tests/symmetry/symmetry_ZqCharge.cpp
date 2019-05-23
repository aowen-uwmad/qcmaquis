/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2011-2013 by Michele Dolfi <dolfim@phys.ethz.ch>
 * 
 * This software is part of the ALPS Applications, published under the ALPS
 * Application License; you can use, redistribute it and/or modify it under
 * the terms of the license, either version 1 or (at your option) any later
 * version.
 * 
 * You should have received a copy of the ALPS Application License along with
 * the ALPS Applications; see the file LICENSE.txt. If not, the license is also
 * available from http://alps.comp-phys.org/.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#define BOOST_TEST_MODULE symmetry

#include <boost/test/included/unit_test.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/geometry/geometries/adapted/boost_array.hpp>
#include "dmrg/block_matrix/symmetry.h"

BOOST_AUTO_TEST_CASE(symmetry_ZqCharge_constructor_test){
  ZqCharge<0> q;
  BOOST_CHECK_EQUAL(q,0);
  ZqCharge<0> q0(0);
  BOOST_CHECK_EQUAL(q,q0);
  ZqCharge<0> q1(1);
  ZqCharge<0> q1bis(1);
  BOOST_CHECK_EQUAL(q1,q1bis);
}

BOOST_AUTO_TEST_CASE(symmetry_ZqCharge_operator_test){
  ZqCharge<1> q;
  ZqCharge<1> q0(0);
  ZqCharge<1> q1(1);
  ZqCharge<1> qres;
  BOOST_CHECK_EQUAL(true,q==q0);
  BOOST_CHECK_EQUAL(false,q==q1);
  BOOST_CHECK_EQUAL(false,q!=q0);
  BOOST_CHECK_EQUAL(true,q!=q1);
  BOOST_CHECK_EQUAL(true,q<q1);
  BOOST_CHECK_EQUAL(false,q1<q);
  qres = q0 + q1;
  BOOST_CHECK_EQUAL(q0,qres);
}
