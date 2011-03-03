#ifndef MPO_OPS_H
#define MPO_OPS_H

#include "mpo.h"
#include "mpotensor.h"

template<class Matrix, class SymmGroup>
std::string identify_op(block_matrix<Matrix, SymmGroup> const & m)
{
    if (m.n_blocks() == 2)
        return "I";
    else {
        typename SymmGroup::charge c1 = m.left_basis()[0].first, c2 = m.right_basis()[0].first;
        if (c1 == 1 && c2 == 0)
            return "c";
        else if (c1 == 0 && c2 == 1)
            return "d";
        else if (c1 == 1 && c2 == 1)
            return "cd";
        else if (c1 == 0 && c2 == 0)
            return "dc";
    }
    
    return "wtf?";
}

template<class Matrix, class SymmGroup>
void follow_mpo(MPO<Matrix, SymmGroup> const & mpo,
                std::string s = std::string(),
                int p = 0, int start = 0)
{
    using std::size_t;
    
    for (size_t k = 0; k < mpo[p].col_dim(); ++k)
    {
        if (mpo[p](start,k).n_blocks() == 0)
            continue;
        
        std::ostringstream oss;
//        oss << mpo[p](start, k) << endl;
//        oss << "(" << start << "," << k << ") ";
        oss << " " << identify_op(mpo[p](start, k)) << " ";
        if (p+1 < mpo.length())
            follow_mpo(mpo, s+oss.str(), p+1, k);
        else
            cout << s+oss.str() << endl;
    }
}

template<class Matrix, class SymmGroup>
void cleanup_mpo_(MPO<Matrix, SymmGroup> const & in_mpo,
                  MPO<Matrix, SymmGroup> & out_mpo,
                  std::vector<boost::tuple<int, int, block_matrix<Matrix, SymmGroup> > > & ops,
                  int p, int start)
{
    for (std::size_t k = 0; k < in_mpo[p].col_dim(); ++k)
    {
        if (!in_mpo[p].has(start,k))
            continue;
        if (in_mpo[p](start,k).n_blocks() == 0)
            continue;
        
        ops[p] = boost::make_tuple(start, k, in_mpo[p](start, k));
        
        if (p+1 < in_mpo.length())
            cleanup_mpo_(in_mpo, out_mpo, ops, p+1, k);
        else
        {
            assert( ops.size() == out_mpo.length() );
            for (std::size_t t = 0; t < in_mpo.length(); ++t)
                out_mpo[t](boost::tuples::get<0>(ops[t]),
                           boost::tuples::get<1>(ops[t])) =
                           boost::tuples::get<2>(ops[t]);
        }
    }
}

template<class Matrix, class SymmGroup>
MPO<Matrix, SymmGroup> cleanup_mpo(MPO<Matrix, SymmGroup> const & mpo)
{
    MPO<Matrix, SymmGroup> ret(mpo.length());
    for (std::size_t p = 0; p < ret.length(); ++p)
        ret[p] = MPOTensor<Matrix, SymmGroup>(mpo[p].row_dim(), mpo[p].col_dim());
    
    std::vector<boost::tuple<int, int, block_matrix<Matrix, SymmGroup> > > prempo(mpo.length());
    cleanup_mpo_(mpo, ret, prempo, 0, 0);
    return ret;
}

template<class Matrix, class SymmGroup>
MPO<Matrix, SymmGroup>
square_mpo(MPO<Matrix, SymmGroup> const & mpo)
{
    using std::size_t;
    typedef typename SymmGroup::charge charge;
    
    size_t L = mpo.length();
    
    MPO<Matrix, SymmGroup> sq(L);
    
    for (size_t p = 0; p < L; ++p)
    {
        MPOTensor<Matrix, SymmGroup> const & inp = mpo[p];
        cout << "MPOTensor " << inp.row_dim()*inp.row_dim() << " " << inp.col_dim()*inp.col_dim() << endl;
        MPOTensor<Matrix, SymmGroup> ret(inp.row_dim()*inp.row_dim(),
                                         inp.col_dim()*inp.col_dim());
        
        for (size_t r1 = 0; r1 < inp.row_dim(); ++r1)
            for (size_t r2 = 0; r2 < inp.row_dim(); ++r2)
                for (size_t c1 = 0; c1 < inp.col_dim(); ++c1)
                    for (size_t c2 = 0; c2 < inp.col_dim(); ++c2) {
                        if (!inp.has(r1, c1))
                            continue;
                        if (!inp.has(r2, c2))
                            continue;
                        
                        block_matrix<Matrix, SymmGroup> t;
                        gemm(inp(r1, c1), inp(r2, c2), t);
                        if (t.n_blocks() > 0)
                            ret(r1*inp.row_dim()+r2,
                                c1*inp.col_dim()+c2) = t;
                    }
        
        sq[p] = ret;
    }
    
    cout << "Done squaring." << endl;
    
    sq = cleanup_mpo(sq);
    cout << "Done cleaning up." << endl;

    return sq;
}

#endif
