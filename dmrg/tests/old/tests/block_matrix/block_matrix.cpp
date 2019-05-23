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

#define BOOST_TEST_MODULE block_matrix

#include "selector.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/geometry/geometries/adapted/boost_array.hpp>

#include "dmrg/block_matrix/symmetry.h"
#include "dmrg/block_matrix/block_matrix.h"

typedef U1 SymmGroup;

BOOST_AUTO_TEST_CASE(block_matrix_constructor_one){
    Index<U1> rows,cols;
    rows.insert(std::make_pair(1, 1));
    rows.insert(std::make_pair(1, 1));
    rows.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));
    
    block_matrix<Matrix, U1> ba(rows,cols);

    BOOST_CHECK_EQUAL(ba[0](0,0),0);
    BOOST_CHECK_EQUAL(ba[1](0,0),0);
    BOOST_CHECK_EQUAL(true,ba[1](0,0)!=1);
}
/* the alps::numeric::associate constructor is not used into the DMRG
BOOST_AUTO_TEST_CASE(block_matrix_constructor_two){
    block_matrix<Matrix, U1> ba(1,1,*(new Matrix(1,1)));
    BOOST_CHECK_EQUAL(ba[0](0,0),0);
}
*/
BOOST_AUTO_TEST_CASE(block_matrix_left_basis_right_basis){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 1));
    rows.insert(std::make_pair(1, 2));
    rows.insert(std::make_pair(1, 3));
    cols.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 2));
    cols.insert(std::make_pair(1, 3));
    
    block_matrix<Matrix, U1> ba(rows,cols);
    
    Index<U1> rows_res(ba.left_basis()),cols_res(ba.right_basis());

    BOOST_CHECK_EQUAL(rows, rows_res);
    BOOST_CHECK_EQUAL(cols, cols_res);
}

BOOST_AUTO_TEST_CASE(block_matrix_n_blocks){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 1));
    rows.insert(std::make_pair(1, 2));
    rows.insert(std::make_pair(1, 3));
    cols.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 2));
    cols.insert(std::make_pair(1, 3));
    
    block_matrix<Matrix, U1> ba(rows,cols);
    BOOST_CHECK_EQUAL(ba.n_blocks(),3);
}

BOOST_AUTO_TEST_CASE(block_matrix_operators_operator_bracket){
    Index<U1> rows,cols;

    Matrix m(1,1,1);
    rows.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));
    block_matrix<Matrix, U1> ba(rows,cols);

    BOOST_CHECK_EQUAL(ba[0](0,0),0);//const []
    ba[0] = m;// non const []
    BOOST_CHECK_EQUAL(ba[0](0,0),1);
}

BOOST_AUTO_TEST_CASE(block_matrix_has_block){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(3, 1));
    rows.insert(std::make_pair(2, 1));
    rows.insert(std::make_pair(1, 1));

    cols.insert(std::make_pair(3, 1));
    cols.insert(std::make_pair(2, 1));
    cols.insert(std::make_pair(1, 1));

    block_matrix<Matrix, U1> ba(rows,cols);

    BOOST_CHECK_EQUAL(true, ba.has_block(3,3));
    BOOST_CHECK_EQUAL(true, ba.has_block(2,2));
    BOOST_CHECK_EQUAL(true, ba.has_block(1,1));
    BOOST_CHECK_EQUAL(false, ba.has_block(1,2));//check a ghost pair
   
    BOOST_CHECK_EQUAL(true, ba.has_block(std::make_pair(1, 1),std::make_pair(1, 1)));  
    BOOST_CHECK_EQUAL(true, ba.has_block(std::make_pair(2, 1),std::make_pair(2, 1)));  
    BOOST_CHECK_EQUAL(true, ba.has_block(std::make_pair(3, 1),std::make_pair(3, 1)));  
    BOOST_CHECK_EQUAL(false, ba.has_block(std::make_pair(2, 1),std::make_pair(3, 1)));  
}

BOOST_AUTO_TEST_CASE(block_matrix_operator_parenthesis){
  // todo 
}

/* the alps::numeric::associate functions are disable into block_matrix 
BOOST_AUTO_TEST_CASE(block_matrix_remove_rows_cols_from_block){
    Index<U1> rows,cols;
    rows.insert(std::make_pair(1, 2));
    cols.insert(std::make_pair(1, 2));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba[0](0,0) = 1;
    ba[0](0,1) = 2;
    ba[0](1,0) = 3;
    ba[0](1,1) = 4;

    ba.remove_rows_from_block(0,0,1);
    BOOST_CHECK_EQUAL(ba[0].num_rows(),1);
    ba.remove_cols_from_block(0,1,1);
    BOOST_CHECK_EQUAL(ba[0](0,0),3);

}
*/
BOOST_AUTO_TEST_CASE(block_matrix_remove_trace){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 2));
    rows.insert(std::make_pair(1, 2));
    rows.insert(std::make_pair(1, 2));

    cols.insert(std::make_pair(1, 2));
    cols.insert(std::make_pair(1, 2));
    cols.insert(std::make_pair(1, 2));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba[0](0,0) = 1;
    ba[0](1,1) = 2;

    ba[1](0,0) = 3;
    ba[1](1,1) = 4;

    ba[2](0,0) = 5;
    ba[2](1,1) = 6;

    BOOST_CHECK_EQUAL(ba.trace(),21);
}

BOOST_AUTO_TEST_CASE(block_matrix_transpose_inplace){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 2));
    rows.insert(std::make_pair(1, 2));

    cols.insert(std::make_pair(1, 2));
    cols.insert(std::make_pair(1, 2));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba[0](0,1) = 1;
    ba[0](1,0) = 2;

    ba[1](0,1) = 3;
    ba[1](1,0) = 4;
   
    ba.transpose_inplace();

    BOOST_CHECK_EQUAL(ba[0](0,1),2);
    BOOST_CHECK_EQUAL(ba[0](1,0),1);

    BOOST_CHECK_EQUAL(ba[1](0,1),4);
    BOOST_CHECK_EQUAL(ba[1](1,0),3);
}

BOOST_AUTO_TEST_CASE(block_matrix_clear){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 1));
    rows.insert(std::make_pair(1, 1));

    cols.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));

    block_matrix<Matrix, U1> ba(rows,cols);
    ba.clear();

    BOOST_CHECK_EQUAL(ba.n_blocks(),0);
   //question to michel why not ras.clear() ??
}


BOOST_AUTO_TEST_CASE(block_matrix_operator_multiply_equal){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 1));
    rows.insert(std::make_pair(1, 1));

    cols.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba[0](0,0) = 1 ;
    ba[1](0,0) = 2 ;

    ba *= 2;

    BOOST_CHECK_EQUAL(ba[0](0,0),2);
    BOOST_CHECK_EQUAL(ba[1](0,0),4);
}

BOOST_AUTO_TEST_CASE(block_matrix_operator_divide_equal){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 1));
    rows.insert(std::make_pair(1, 1));

    cols.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba[0](0,0) = 4 ;
    ba[1](0,0) = 8 ;

    ba /= 2;

    BOOST_CHECK_EQUAL(ba[0](0,0),2);
    BOOST_CHECK_EQUAL(ba[1](0,0),4);
}

BOOST_AUTO_TEST_CASE(block_matrix_resize_block){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 3));
    cols.insert(std::make_pair(1, 3));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba[0](0,0) = 1 ;
    ba[0](0,1) = 2 ;
    ba[0](0,2) = 3 ;

    ba[0](1,0) = 4 ;
    ba[0](1,1) = 5 ;
    ba[0](1,2) = 6 ;

    ba[0](2,0) = 7 ;
    ba[0](2,1) = 8 ;
    ba[0](2,2) = 9 ;

    ba.resize_block(1,1,2,2,false);


    BOOST_CHECK_EQUAL(ba[0](0,0),1);
    BOOST_CHECK_EQUAL(ba[0](0,1),2);

    BOOST_CHECK_EQUAL(ba[0](1,0),4);
    BOOST_CHECK_EQUAL(ba[0](1,1),5);
}

BOOST_AUTO_TEST_CASE(block_matrix_remove_block){
    Index<U1> rows,cols;

    rows.insert(std::make_pair(1, 1));
    cols.insert(std::make_pair(1, 1));

    rows.insert(std::make_pair(2, 1));
    cols.insert(std::make_pair(2, 1));

    rows.insert(std::make_pair(3, 1));
    cols.insert(std::make_pair(3, 1));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba[0](0,0)=1; // block (3,3)
    ba[1](0,0)=2; // block (2,2)
    ba[2](0,0)=3; // block (1,1)

    ba.remove_block(1);
    BOOST_CHECK_EQUAL(ba[0](0,0),1);
    BOOST_CHECK_EQUAL(ba[1](0,0),3);
    ba.remove_block(3,3);
    BOOST_CHECK_EQUAL(ba[0](0,0),3);
}


BOOST_AUTO_TEST_CASE(match_and_add_block){

    Index<U1> rows,cols;
    rows.insert(std::make_pair(2, 2));
    cols.insert(std::make_pair(2, 2));

    block_matrix<Matrix, U1> ba(rows,cols);
  
    ba[0](0,0) = 1;
    ba[0](0,1) = 2;
    ba[0](1,0) = 3;
    ba[0](1,1) = 4;

    Matrix M1(1,1,2);
    
    ba.match_and_add_block(M1,1,1);
    
    //BOOST_CHECK_EQUAL(ba[1],M1);

    Matrix M2(2,2,1);

    ba.match_and_add_block(M2,2,2);

    BOOST_CHECK_EQUAL(ba[0](0,0),2);
    BOOST_CHECK_EQUAL(ba[0](0,1),3);
    BOOST_CHECK_EQUAL(ba[0](1,0),4);
    BOOST_CHECK_EQUAL(ba[0](1,1),5);

    Matrix M3(3,3,1);
    ba.match_and_add_block(M3,2,2);

    BOOST_CHECK_EQUAL(ba[0](0,0),3);
    BOOST_CHECK_EQUAL(ba[0](0,1),4);
    BOOST_CHECK_EQUAL(ba[0](0,2),1);

    BOOST_CHECK_EQUAL(ba[0](1,0),5);
    BOOST_CHECK_EQUAL(ba[0](1,1),6);
    BOOST_CHECK_EQUAL(ba[0](1,2),1);

    BOOST_CHECK_EQUAL(ba[0](2,0),1);
    BOOST_CHECK_EQUAL(ba[0](2,1),1);
    BOOST_CHECK_EQUAL(ba[0](2,2),1);

    ba.match_and_add_block(M1,2,2);

    BOOST_CHECK_EQUAL(ba[0](0,0),5);
    BOOST_CHECK_EQUAL(ba[0](0,1),4);
    BOOST_CHECK_EQUAL(ba[0](0,2),1);

    BOOST_CHECK_EQUAL(ba[0](1,0),5);
    BOOST_CHECK_EQUAL(ba[0](1,1),6);
    BOOST_CHECK_EQUAL(ba[0](1,2),1);

    BOOST_CHECK_EQUAL(ba[0](2,0),1);
    BOOST_CHECK_EQUAL(ba[0](2,1),1);
    BOOST_CHECK_EQUAL(ba[0](2,2),1);

}

BOOST_AUTO_TEST_CASE(reserve){

    Index<U1> rows,cols;
    rows.insert(std::make_pair(2, 2));
    cols.insert(std::make_pair(2, 2));

    block_matrix<Matrix, U1> ba(rows,cols);
    BOOST_CHECK_EQUAL(true, ba.has_block(2,2));

    ba.reserve(2,2,3,4);

    BOOST_CHECK_EQUAL(ba.left_basis()[0].second,3);
    BOOST_CHECK_EQUAL(ba.right_basis()[0].second,4);

    ba.reserve(3,3,2,3);
    BOOST_CHECK_EQUAL(true, ba.has_block(3,3));

    BOOST_CHECK_EQUAL(ba.left_basis()[0].second,2);
    BOOST_CHECK_EQUAL(ba.left_basis()[0].first,3);

}


BOOST_AUTO_TEST_CASE(reserve_pos){
    Index<U1> rows,cols;
    rows.insert(std::make_pair(2, 2));
    cols.insert(std::make_pair(2, 2));

    block_matrix<Matrix, U1> ba(rows,cols);
   
    ba.reserve_pos(2,2,3,4);

    BOOST_CHECK_EQUAL(ba.left_basis()[0].second,4);
    BOOST_CHECK_EQUAL(ba.right_basis()[0].second,5);

    ba.reserve_pos(3,3,2,3);

    BOOST_CHECK_EQUAL(ba.left_basis()[0].second,3);
    BOOST_CHECK_EQUAL(ba.right_basis()[0].second,4);
}

BOOST_AUTO_TEST_CASE(allocate_blocks){
    Index<U1> rows,cols;
    rows.insert(std::make_pair(2, 1));
    cols.insert(std::make_pair(2, 1));

    block_matrix<Matrix, U1> ba(rows,cols);

    ba.reserve_pos(2,2,2,2);
    ba.allocate_blocks();

    BOOST_CHECK_EQUAL(ba[0](1,1),0); // if no allocate_blocks, segfault !

}

BOOST_AUTO_TEST_CASE(cleanup_zeros_1){ // remove block in the middle
    block_matrix<Matrix, U1> ba;
    {
        Matrix m(4, 6, 1.);
        m(3,2) = m(2,2) = m(3,3) = 1e-15;
        ba.insert_block(m, 0, 0);
    }
    {
        Matrix m(3, 2, 1e-15);
        ba.insert_block(m, 1, 1);
    }
    {
        Matrix m(2, 1, 1.);
        m(1,0) = 1e-15;
        ba.insert_block(m, 2, 2);
    }
    
    ba.cleanup_zeros(1e-10);

    BOOST_CHECK_EQUAL(ba.n_blocks(),2);
    BOOST_CHECK_EQUAL(ba(0,0)(3,2),0.);
    BOOST_CHECK_EQUAL(ba(0,0)(2,2),0.);
    BOOST_CHECK_EQUAL(ba(0,0)(3,3),0.);
    BOOST_CHECK_EQUAL(ba(2,2)(1,0),0.);
    
    ba.cleanup_zeros(10.);
    BOOST_CHECK_EQUAL(ba.n_blocks(),0);
}

BOOST_AUTO_TEST_CASE(cleanup_zeros_2){ // remove first block
    block_matrix<Matrix, U1> ba;
    {
        Matrix m(3, 2, 1e-15);
        ba.insert_block(m, 0, 0);
    }
    {
        Matrix m(4, 6, 1.);
        m(3,2) = m(2,2) = m(3,3) = 1e-15;
        ba.insert_block(m, 1, 1);
    }
    {
        Matrix m(2, 1, 1.);
        m(1,0) = 1e-15;
        ba.insert_block(m, 2, 2);
    }
    
    ba.cleanup_zeros(1e-10);

    BOOST_CHECK_EQUAL(ba.n_blocks(),2);
    BOOST_CHECK_EQUAL(ba(1,1)(3,2),0.);
    BOOST_CHECK_EQUAL(ba(1,1)(2,2),0.);
    BOOST_CHECK_EQUAL(ba(1,1)(3,3),0.);
    BOOST_CHECK_EQUAL(ba(2,2)(1,0),0.);
    
    ba.cleanup_zeros(10.);
    BOOST_CHECK_EQUAL(ba.n_blocks(),0);
}

BOOST_AUTO_TEST_CASE(cleanup_zeros_3){ // remove last block
    block_matrix<Matrix, U1> ba;
    {
        Matrix m(4, 6, 1.);
        m(3,2) = m(2,2) = m(3,3) = 1e-15;
        ba.insert_block(m, 0, 0);
    }
    {
        Matrix m(2, 1, 1.);
        m(1,0) = 1e-15;
        ba.insert_block(m, 1, 1);
    }
    {
        Matrix m(3, 2, 1e-15);
        ba.insert_block(m, 2, 2);
    }
    
    ba.cleanup_zeros(1e-10);

    BOOST_CHECK_EQUAL(ba.n_blocks(),2);
    BOOST_CHECK_EQUAL(ba(0,0)(3,2),0.);
    BOOST_CHECK_EQUAL(ba(0,0)(2,2),0.);
    BOOST_CHECK_EQUAL(ba(0,0)(3,3),0.);
    BOOST_CHECK_EQUAL(ba(1,1)(1,0),0.);
    
    ba.cleanup_zeros(10.);
    BOOST_CHECK_EQUAL(ba.n_blocks(),0);
}




