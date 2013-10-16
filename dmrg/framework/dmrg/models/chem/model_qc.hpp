/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2012-2013 by Sebastian Keller <sebkelle@phys.ethz.ch>
 *
 *
 *****************************************************************************/

#ifndef QC_HAMILTONIANS_HPP
#define QC_HAMILTONIANS_HPP

template <class Matrix>
template <class M>
Hamiltonian<M, TwoU1> qc_model<Matrix>::H_impl() const
{
    typedef typename tag_type<M>::type tag_type;

    typename op_t<M>::type create_up_op, create_down_op, destroy_up_op, destroy_down_op,
                           count_up_op, count_down_op, docc_op, e2d_op, d2e_op,
                           ident_op, fill_op;

    TwoU1::charge A(0), B(0), C(0), D(1);
    B[0]=1; C[1]=1;
    ident_op.insert_block(M(1, 1, 1), A, A);
    ident_op.insert_block(M(1, 1, 1), B, B);
    ident_op.insert_block(M(1, 1, 1), C, C);
    ident_op.insert_block(M(1, 1, 1), D, D);

    create_up_op.insert_block(M(1, 1, 1), A, B);
    create_up_op.insert_block(M(1, 1, 1), C, D);
    create_down_op.insert_block(M(1, 1, 1), A, C);
    create_down_op.insert_block(M(1, 1, 1), B, D);

    destroy_up_op.insert_block(M(1, 1, 1), B, A);
    destroy_up_op.insert_block(M(1, 1, 1), D, C);
    destroy_down_op.insert_block(M(1, 1, 1), C, A);
    destroy_down_op.insert_block(M(1, 1, 1), D, B);

    count_up_op.insert_block(M(1, 1, 1), B, B);
    count_up_op.insert_block(M(1, 1, 1), D, D);

    count_down_op.insert_block(M(1, 1, 1), C, C);
    count_down_op.insert_block(M(1, 1, 1), D, D);

    docc_op.insert_block(M(1, 1, 1), D, D);

    e2d_op.insert_block(M(1, 1, 1), A, D);
    d2e_op.insert_block(M(1, 1, 1), D, A);

    fill_op.insert_block(M(1, 1, 1), A, A);
    fill_op.insert_block(M(1, 1, -1), B, B);
    fill_op.insert_block(M(1, 1, -1), C, C);
    fill_op.insert_block(M(1, 1, 1), D, D);

    typename op_t<M>::type tmp;

    gemm(fill_op, create_down_op, tmp);
    create_down_op = tmp;
    gemm(destroy_down_op, fill_op, tmp);
    destroy_down_op = tmp;

    /**********************************************************************/
    /*** Create operator tag table ****************************************/
    /**********************************************************************/

    boost::shared_ptr<TagHandler<M, TwoU1> > tag_handler(new TagHandler<M, TwoU1>());
    tag_type create_up, create_down, destroy_up, destroy_down,
          count_up, count_down, docc, e2d, d2e,
          fill, ident;

    #define REGISTER(op, kind) op = tag_handler->register_op(op ## _op, kind);

    REGISTER(ident,        tag_detail::bosonic)
    REGISTER(fill,         tag_detail::bosonic)
    REGISTER(create_up,    tag_detail::fermionic)
    REGISTER(create_down,  tag_detail::fermionic)
    REGISTER(destroy_up,   tag_detail::fermionic)
    REGISTER(destroy_down, tag_detail::fermionic)
    REGISTER(count_up,     tag_detail::bosonic)
    REGISTER(count_down,   tag_detail::bosonic)
    REGISTER(e2d,          tag_detail::bosonic)
    REGISTER(d2e,          tag_detail::bosonic)
    REGISTER(docc,         tag_detail::bosonic)

    #undef REGISTER
    /**********************************************************************/

    chem_detail::ChemHelper<M> term_assistant(parms, lat, ident, fill, tag_handler);
    std::vector<value_type> & matrix_elements = term_assistant.getMatrixElements();

    std::vector<int> used_elements(matrix_elements.size(), 0);
 
    /**********************************************************************/
    /*** Tagged terms *****************************************************/
    /**********************************************************************/
    std::vector<typename hamtagterm_t<M>::type > tagterms;
    for (std::size_t m=0; m < matrix_elements.size(); ++m) {
        int i = term_assistant.idx(m, 0);
        int j = term_assistant.idx(m, 1);
        int k = term_assistant.idx(m, 2);
        int l = term_assistant.idx(m, 3);

        // Core electrons energy
        if ( i==-1 && j==-1 && k==-1 && l==-1) {

            typename hamtagterm_t<M>::type term;
            term.fill_operator = ident;
            term.scale = matrix_elements[m];
            term.operators.push_back(std::make_pair(0, ident));
            tagterms.push_back(term);
            
            used_elements[m] += 1;
        }

        // On site energy t_ii
        else if ( i==j && k == -1 && l == -1) {
            {
                typename hamtagterm_t<M>::type term;
                term.fill_operator = ident;
                term.scale = matrix_elements[m];
                term.operators.push_back(std::make_pair(i, count_up));
                tagterms.push_back(term);
            }
            {
                typename hamtagterm_t<M>::type term;
                term.fill_operator = ident;
                term.scale = matrix_elements[m];
                term.operators.push_back(std::make_pair(i, count_down));
                tagterms.push_back(term);
            }

            used_elements[m] += 1;
            continue;
        }

        // Hopping term t_ij 
        else if (k == -1 && l == -1) {

            tagterms.push_back(TermMaker<M>::positional_two_term(
                true, fill, matrix_elements[m], i, j, create_up, destroy_up, tag_handler)
            );
            tagterms.push_back(TermMaker<M>::positional_two_term(
                true, fill, matrix_elements[m], i, j, create_down, destroy_down, tag_handler)
            );
            tagterms.push_back(TermMaker<M>::positional_two_term(
                true, fill, matrix_elements[m], j, i, create_up, destroy_up, tag_handler)
            );
            tagterms.push_back(TermMaker<M>::positional_two_term(
                true, fill, matrix_elements[m], j, i, create_down, destroy_down, tag_handler)
            );

            used_elements[m] += 1;
        }

        // On site Coulomb repulsion V_iiii
        else if ( i==j && j==k && k==l) {

            typename hamtagterm_t<M>::type term;
            term.fill_operator = ident;
            term.scale = matrix_elements[m];
            term.operators.push_back(std::make_pair(i, docc));
            tagterms.push_back(term);

            used_elements[m] += 1;
        }

        // V_ijjj = V_jijj = V_jjij = V_jjji
        else if ( (i==j && j==k && k!=l) || (i!=j && j==k && k==l) ) {

            int same_idx, pos1;
            typename op_t<M>::type tmp;

            if      (i==j) { same_idx = i; pos1 = l; }
            else if (k==l) { same_idx = l; pos1 = i; }
            else           { throw std::runtime_error("Term generation logic has failed for V_ijjj term\n"); }

            std::pair<tag_type, value_type> ptag;

            // 1a
            // --> c_l_up * n_i_down * cdag_i_up
            ptag = tag_handler->get_product_tag(count_down, create_up);
            tagterms.push_back( TermMaker<M>::positional_two_term(true, fill, matrix_elements[m] * ptag.second, same_idx, pos1,
                                           ptag.first, destroy_up, tag_handler) );

            // 1a_dagger
            // --> c_i_up * n_i_down * cdag_l_up
            ptag = tag_handler->get_product_tag(destroy_up, count_down);
            tagterms.push_back( TermMaker<M>::positional_two_term(true, fill, -matrix_elements[m] * ptag.second, same_idx, pos1,
                                           ptag.first, create_up, tag_handler) );

            // 1b
            // --> c_l_down * n_i_up * cdag_i_down (1b)
            ptag = tag_handler->get_product_tag(count_up, create_down);
            tagterms.push_back( TermMaker<M>::positional_two_term(true, fill, matrix_elements[m] * ptag.second, same_idx, pos1,
                                           ptag.first, destroy_down, tag_handler) );

            // (1b)_dagger
            // --> c_i_down * n_i_up * cdag_l_down
            ptag = tag_handler->get_product_tag(destroy_down, count_up);
            tagterms.push_back( TermMaker<M>::positional_two_term(true, fill, -matrix_elements[m] * ptag.second, same_idx, pos1,
                                           ptag.first, create_down, tag_handler) );

            used_elements[m] += 1;
        }

        // V_iijj == V_jjii
        else if ( i==j && k==l && j!=k) {

            term_assistant.add_term(tagterms, matrix_elements[m], i, k, count_up, count_up);
            term_assistant.add_term(tagterms, matrix_elements[m], i, k, count_up, count_down);
            term_assistant.add_term(tagterms, matrix_elements[m], i, k, count_down, count_up);
            term_assistant.add_term(tagterms, matrix_elements[m], i, k, count_down, count_down);

            used_elements[m] += 1;
        }

        // V_ijij == V_jiji = V_ijji = V_jiij
        else if ( i==k && j==l && i!=j) {

            typename op_t<M>::type tmp1, tmp2;

            term_assistant.add_term(tagterms,  matrix_elements[m], i, j, e2d, d2e);
            term_assistant.add_term(tagterms,  matrix_elements[m], i, j, d2e, e2d);
            term_assistant.add_term(tagterms, -matrix_elements[m], i, j, count_up, count_up);
            term_assistant.add_term(tagterms, -matrix_elements[m], i, j, count_down, count_down);

            std::pair<tag_type, value_type> ptag1, ptag2;

            // Could insert fill operators without changing the result
            // --> -c_j_up * cdag_j_down * c_i_down * cdag_i_up
            ptag1 = tag_handler->get_product_tag(destroy_down, create_up);
            ptag2 = tag_handler->get_product_tag(destroy_up, create_down);
            term_assistant.add_term(
                tagterms, -matrix_elements[m] * ptag1.second * ptag2.second, i, j, ptag1.first, ptag2.first
            );

            // --> -c_i_up * cdag_i_down * c_j_down * cdag_j_up
            ptag1 = tag_handler->get_product_tag(destroy_up, create_down);
            ptag2 = tag_handler->get_product_tag(destroy_down, create_up);
            term_assistant.add_term(
                tagterms, -matrix_elements[m] * ptag1.second * ptag2.second, i, j, ptag1.first, ptag2.first
            );
            
            used_elements[m] += 1;
        }

        // 9987 9877

        // 8 (4x2)-fold degenerate V_iilk == V_iikl = V_lkii = V_klii  <--- coded
        //                         V_ijkk == V_jikk = V_kkij = V_kkji  <--- contained above
        else if ( (i==j && j!=k && k!=l) || (k==l && i!=j && j!=k)) {

            int same_idx;
            if (i==j) { same_idx = i; }
            if (k==l) { same_idx = k; k = i; l = j; }

            // n_up * cdag_up * c_up <--
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, k, l, create_up, destroy_up, create_up, destroy_up);
            // n_up * cdag_down * c_down <--
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, k, l, create_up, destroy_up, create_down, destroy_down);
            // n_down * cdag_up * c_up <--
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, k, l, create_down, destroy_down, create_up, destroy_up);
            // n_down * cdag_down * c_down <--
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, k, l, create_down, destroy_down, create_down, destroy_down);

            // --> n_up * c_up * cdag_up
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, l, k, create_up, destroy_up, create_up, destroy_up);
            // --> n_up * c_down * cdag_down
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, l, k, create_up, destroy_up, create_down, destroy_down);
            // --> n_down * c_up * cdag_up
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, l, k, create_down, destroy_down, create_up, destroy_up);
            // --> n_down * c_down * cdag_down
            term_assistant.add_term(tagterms, matrix_elements[m], same_idx, l, k, create_down, destroy_down, create_down, destroy_down);

            used_elements[m] += 1;
        }

        // 9887 7371 8727

        // 4-fold degenerate (+spin) V_ijil = V_ijli = V_jiil = V_jili  <--- coded
        //                           V_ilij = V_ilji = V_liij = V_liji
        else if ( ((i==k && j!=l) || j==k || (j==l && i!=k)) && (i!=j && k!=l)) {
            int same_idx, pos1, pos2;
            if (i==k) { same_idx = i; pos1 = l; pos2 = j; }
            if (j==k) { same_idx = j; pos1 = l; pos2 = i; }
            if (j==l) { same_idx = j; pos1 = k; pos2 = i; }

            std::pair<tag_type, value_type> ptag;

            ptag = tag_handler->get_product_tag(create_up, fill);
            term_assistant.add_term(
                tagterms, matrix_elements[m]*ptag.second, same_idx, pos1, pos2, ptag.first, create_down , destroy_down, destroy_up
            );
            ptag = tag_handler->get_product_tag(create_down, fill);
            term_assistant.add_term(
                tagterms, matrix_elements[m]*ptag.second, same_idx, pos1, pos2, ptag.first, create_up   , destroy_up  , destroy_down
            );
            ptag = tag_handler->get_product_tag(destroy_down, fill);
            term_assistant.add_term(
                tagterms, matrix_elements[m]*ptag.second, same_idx, pos1, pos2, ptag.first, destroy_up  , create_up   , create_down
            );
            ptag = tag_handler->get_product_tag(destroy_up, fill);
            term_assistant.add_term(
                tagterms, matrix_elements[m]*ptag.second, same_idx, pos1, pos2, ptag.first, destroy_down, create_down , create_up
            );

            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos1, pos2, create_up,   destroy_up,   create_up,   destroy_up
            );
            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos1, pos2, create_up,   destroy_down, create_down, destroy_up
            );
            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos1, pos2, create_down, destroy_up,   create_up,   destroy_down
            );
            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos1, pos2, create_down, destroy_down, create_down, destroy_down
            );

            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos2, pos1, create_up,   destroy_up,   create_up,   destroy_up
            );
            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos2, pos1, create_up,   destroy_down, create_down, destroy_up
            );
            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos2, pos1, create_down, destroy_up,   create_up,   destroy_down
            );
            term_assistant.add_term(
                tagterms, -matrix_elements[m], same_idx, pos2, pos1, create_down, destroy_down, create_down, destroy_down
            );

            used_elements[m] += 1;
        }

        // 32 (8x4)-fold degenerate V_ijkl = V_jikl = V_ijlk = V_jilk = V_klij = V_lkij = V_klji = V_lkji * spin
        // V_ijkl -> 24 permutations which fall into 3 equivalence classes of 8 elements (with identical V_ matrix element)
        // coded: 4 index permutations x 4 spin combinations 
        else if (i!=j && j!=k && k!=l && i!=k && j!=l) {
            
            // 1
            term_assistant.add_term(tagterms, i,k,l,j, create_up, create_up, destroy_up, destroy_up);
            term_assistant.add_term(tagterms, i,k,l,j, create_up, create_down, destroy_down, destroy_up);
            term_assistant.add_term(tagterms, i,k,l,j, create_down, create_up, destroy_up, destroy_down);
            term_assistant.add_term(tagterms, i,k,l,j, create_down, create_down, destroy_down, destroy_down);

            // 2
            term_assistant.add_term(tagterms, i,l,k,j, create_up, create_up, destroy_up, destroy_up);
            term_assistant.add_term(tagterms, i,l,k,j, create_up, create_down, destroy_down, destroy_up);
            term_assistant.add_term(tagterms, i,l,k,j, create_down, create_up, destroy_up, destroy_down);
            term_assistant.add_term(tagterms, i,l,k,j, create_down, create_down, destroy_down, destroy_down);

            // 3
            term_assistant.add_term(tagterms, j,k,l,i, create_up, create_up, destroy_up, destroy_up);
            term_assistant.add_term(tagterms, j,k,l,i, create_up, create_down, destroy_down, destroy_up);
            term_assistant.add_term(tagterms, j,k,l,i, create_down, create_up, destroy_up, destroy_down);
            term_assistant.add_term(tagterms, j,k,l,i, create_down, create_down, destroy_down, destroy_down);

            // 4
            term_assistant.add_term(tagterms, j,l,k,i, create_up, create_up, destroy_up, destroy_up);
            term_assistant.add_term(tagterms, j,l,k,i, create_up, create_down, destroy_down, destroy_up);
            term_assistant.add_term(tagterms, j,l,k,i, create_down, create_up, destroy_up, destroy_down);
            term_assistant.add_term(tagterms, j,l,k,i, create_down, create_down, destroy_down, destroy_down);
        
            used_elements[m] += 1;
        }
    } // matrix_elements for

    // make sure all elements have been used
    std::vector<int>::iterator it_0;
    it_0 = std::find(used_elements.begin(), used_elements.end(), 0);
    assert( it_0 == used_elements.end() );

    term_assistant.commit_terms(tagterms);
    maquis::cout << "The hamiltonian will contain " << tagterms.size() << " terms\n";

    std::vector<typename hamterm_t<M>::type > terms;
    return Hamiltonian<M, TwoU1>(phys, ident_op, terms, ident, tagterms, tag_handler);

}
    
#endif
