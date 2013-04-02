#ifndef AMBIENT_NUMERIC_TILES_ALGORITHMS
#define AMBIENT_NUMERIC_TILES_ALGORITHMS

#include "ambient/numeric/matrix/tiles.h"
#include <utility>

#define value_type      typename tiles<Matrix>::value_type
#define size_type       typename tiles<Matrix>::size_type
#define real_type       typename tiles<Matrix>::real_type
#define scalar_type     typename tiles<Matrix>::scalar_type
#define difference_type typename tiles<Matrix>::difference_type


namespace ambient { namespace numeric {

    class cross_iterator {
    public:
        cross_iterator(size_t first, size_t second, size_t size) 
        : first(first), second(second), lim(first+size){
            measure_step();
        }
        void operator++ (){
            first  += step;
            second += step;
            measure_step();
        }
        bool end(){
            return (first >= lim);
        }
        void measure_step(){
            step = std::min(std::min((AMBIENT_IB*__a_ceil((first+1)/AMBIENT_IB) - first), 
                                     (AMBIENT_IB*__a_ceil((second+1)/AMBIENT_IB) - second)),
                                     (lim-first));
        }
        size_t first;
        size_t second;
        size_t step;
        size_t lim;
    };

    template<class Matrix>
    bool is_hermitian(const tiles<Matrix>& a){
        return false; // todo
    }

    template<class Matrix>
    inline tiles<Matrix> exp(const tiles<Matrix>& a, const value_type& alfa = 1.){
        assert(false); printf("ERROR: NOT TESTED (EXP)\n");
    }

    template<class Matrix>
    inline const tiles<Matrix>& conj(const tiles<Matrix>& a){
        //a.conj();
        return a; // todo for complex
    }

    template<class Matrix>
    inline void conj_inplace(tiles<Matrix>& a){
        // gs (doubles) // todo for complex
    }

    template <class MatrixA, class MatrixB>
    inline void mul_inplace(tiles<MatrixA>& a, const tiles<MatrixB>& rhs){
        assert(false); printf("ERROR: NOT TESTED (GEMM BLOCKED INPLACE)\n");
    }

    template <class Matrix>
    inline tiles<Matrix> kron(const tiles<Matrix>& M1, const tiles<Matrix>& M2){
        assert(false); printf("ERROR: NOT TESTED (KRON)\n");
    }
 
    template<class Matrix>
    inline Matrix* merge(const tiles<subset_view<Matrix> >& a){
        Matrix* m = new Matrix(a.rows, a.cols);

        for(int j = 0; j < a.nt; j++){
            for(int i = 0; i < a.mt; i++){
                const Matrix* src = &a.tile(i,j);
                copy_block(*src, 0, 0, *m, i*AMBIENT_IB, j*AMBIENT_IB, src->num_rows(), src->num_cols());
            }
        }
        return m;
    }

    template<class Matrix>
    inline void split_d(const tiles<transpose_view<Matrix> >& a){
        printf("SPLIT OF TRANSPOSE VIEW!\n\n\n");
        if(a.data.size() == 1 && (__a_ceil(a.rows/AMBIENT_IB) != 1 || __a_ceil(a.cols/AMBIENT_IB) != 1)) printf("We have to split but we don't :(\n");
    }

    // {{{ normal merge / split
    
    template<class Matrix>
    inline void merge(const tiles<Matrix>& a){
        if(a.data.size() == 1) return;

        std::vector<Matrix*> m; 
        m.push_back(new Matrix(a.rows, a.cols));

        for(int j = 0; j < a.nt; j++){
            for(int i = 0; i < a.mt; i++){
                const Matrix* src = &a.tile(i,j);
                if(!src->core->weak())
                copy_block(*src, 0, 0, *m[0], i*AMBIENT_IB, j*AMBIENT_IB, src->num_rows(), src->num_cols());
                delete src;
            }
        }
        std::swap(const_cast<std::vector<Matrix*>&>(a.data), m);
    }

    template<class Matrix>
    inline void split_d(const tiles<Matrix>& a){
        if(a.data.size() != 1) return;
        if(a.mt == 1 && a.nt == 1) return;

        std::vector<Matrix*> split;
        int tailm = __a_mod(a.rows, AMBIENT_IB);
        int tailn = __a_mod(a.cols, AMBIENT_IB);
        split.reserve(a.mt*a.nt);
        for(int j = 1; j < a.nt; j++){
            for(int i = 1; i < a.mt; i++) 
                split.push_back(new Matrix(AMBIENT_IB, AMBIENT_IB));
            split.push_back(new Matrix(tailm, AMBIENT_IB));
        }
        for(int i = 1; i < a.mt; i++) 
            split.push_back(new Matrix(AMBIENT_IB, tailn));
        split.push_back(new Matrix(tailm, tailn));

        const Matrix* src = a.data[0];

        if(!a[0].core->weak())
        for(int j = 0; j < a.nt; j++){
            for(int i = 0; i < a.mt; i++){
                Matrix& dst = *split[i+j*a.mt];
                copy_block(*src, i*AMBIENT_IB, j*AMBIENT_IB, dst, 0, 0, dst.num_rows(), dst.num_cols());
            }
        }

        delete src;
        std::swap(const_cast<std::vector<Matrix*>&>(a.data), split);
    }

    // }}}
    // {{{ diagonal merge/split

    template<typename T>
    inline void merge(const tiles<diagonal_matrix<T> >& a){
        if(a.data.size() == 1) return;

        std::vector<diagonal_matrix<T> *> m; 
        m.push_back(new diagonal_matrix<T>(a.size));

        for(int i = 0; i < a.nt; i++){
            const diagonal_matrix<T>* src = a.data[i];
            if(!src->get_data().core->weak())
            copy_block(src->get_data(), 0, 0, m[0]->get_data(), i*AMBIENT_IB, 0, src->num_rows(), 1);
            delete src;
        }
        std::swap(const_cast<std::vector<diagonal_matrix<T>*>&>(a.data), m);
    }

    template<typename T>
    inline void split_d(const tiles<diagonal_matrix<T> >& a){
        if(a.data.size() != 1) return;
        if(a.nt == 1) return;

        std::vector<diagonal_matrix<T> *> split;
        int tailm = __a_mod(a.size, AMBIENT_IB);
        split.reserve(a.nt);
        for(int i = 1; i < a.nt; i++) 
            split.push_back(new diagonal_matrix<T>(AMBIENT_IB));
        split.push_back(new diagonal_matrix<T>(tailm));

        const diagonal_matrix<T>* src = a.data[0];

        if(!a[0].get_data().core->weak())
        for(int i = 0; i < a.nt; i++){
            diagonal_matrix<T>& dst = *split[i];
            copy_block(src->get_data(), i*AMBIENT_IB, 0, dst.get_data(), 0, 0, dst.num_rows(), 1);
        }

        delete src;
        std::swap(const_cast<std::vector<diagonal_matrix<T>*>&>(a.data), split);
    }

    // }}}

    template<class MatrixA, class MatrixB>
    inline void copy_block(const tiles<MatrixB>& in, size_t ii, size_t ij,
                           tiles<MatrixA>& out, size_t oi, size_t oj, 
                           size_t m, size_t n)
    {
        for(cross_iterator row(oi,ii,m); !row.end(); ++row)
        for(cross_iterator col(oj,ij,n); !col.end(); ++col)
        copy_block(in.locate(row.second, col.second), row.second%AMBIENT_IB, col.second%AMBIENT_IB,
                   out.locate(row.first, col.first), row.first%AMBIENT_IB, col.first%AMBIENT_IB, 
                   row.step, col.step);
    }

    template<class Matrix>
    inline void copy_block_s(const tiles<Matrix>& in, size_t ii, size_t ij,
                             tiles<Matrix>& out, size_t oi, size_t oj, 
                             const tiles<Matrix>& alfa, size_t ai, size_t aj,
                             size_t m, size_t n)
    {
        const Matrix& factor = alfa.locate(ai, aj); 
        ai %= AMBIENT_IB; aj %= AMBIENT_IB;
        for(cross_iterator row(oi,ii,m); !row.end(); ++row)
        for(cross_iterator col(oj,ij,n); !col.end(); ++col)
        copy_block_s(in.locate(row.second, col.second), row.second%AMBIENT_IB, col.second%AMBIENT_IB,
                     out.locate(row.first, col.first), row.first%AMBIENT_IB, col.first%AMBIENT_IB, 
                     factor, ai, aj, row.step, col.step);
    }

    template<class Matrix>
    inline void copy_block_sa(const tiles<Matrix>& in, size_t ii, size_t ij,
                              tiles<Matrix>& out, size_t oi, size_t oj, 
                              const tiles<Matrix>& alfa, size_t ai, size_t aj,
                              size_t m, size_t n)
    {
        const Matrix& factor = alfa.locate(ai, aj); 
        ai %= AMBIENT_IB; aj %= AMBIENT_IB;
        for(cross_iterator row(oi,ii,m); !row.end(); ++row)
        for(cross_iterator col(oj,ij,n); !col.end(); ++col)
        copy_block_sa(in.locate(row.second, col.second), row.second%AMBIENT_IB, col.second%AMBIENT_IB,
                      out.locate(row.first, col.first), row.first%AMBIENT_IB, col.first%AMBIENT_IB, 
                      factor, ai, aj, row.step, col.step);
    }

    template<class MatrixA, class MatrixB, class MatrixC>
    inline void gemm(const tiles<MatrixA>& a, const tiles<MatrixB>& b, tiles<MatrixC>& c){
        for(int i = 0; i < c.mt; i++){
            for(int j = 0; j < c.nt; j++){
                std::vector<MatrixC*> ctree; ctree.reserve(a.nt);
                ctree.push_back(&c.tile(i,j));
                size_t rows = c.tile(i,j).num_rows();
                size_t cols = c.tile(i,j).num_cols();
                for(int k = 1; k < a.nt; k++) 
                    ctree.push_back(new MatrixC(rows, cols));
                for(int k = 0; k < a.nt; k++){
                    const MatrixA& ab = a.tile(i, k);
                    const MatrixB& bb = b.tile(k, j);
                    gemm(ab, bb, *ctree[k]);
                }
                __a_reduce(ctree);
                for(int k = 1; k < a.nt; k++) 
                    delete ctree[k];
            }
        }
    }

    template<class MatrixA, class MatrixC, typename T>
    inline void gemm(const tiles<MatrixA>& a, const tiles<diagonal_matrix<T> >& b, tiles<MatrixC>& c){
        for(int i = 0; i < c.mt; i++){
            for(int j = 0; j < c.nt; j++){
                gemm(a.tile(i,j), b[j], c.tile(i,j));
            }
        }
    }

    template<class MatrixB, class MatrixC, typename T>
    inline void gemm(const tiles<diagonal_matrix<T> >& a, const tiles<MatrixB>& b, tiles<MatrixC>& c){
        for(int i = 0; i < c.mt; i++){
            for(int j = 0; j < c.nt; j++){
                gemm(a[i], b.tile(i,j), c.tile(i,j));
            }
        }
    }

    template<class Matrix, class DiagonalMatrix>
    inline void svd(tiles<Matrix> a, tiles<Matrix>& u, tiles<Matrix>& v, tiles<DiagonalMatrix>& s){
        size_t m = num_rows(a);
        size_t n = num_cols(a);
        size_t k = std::min(m,n);
        resize(u, m, k);
        resize(s, k, k);
        resize(v, k, n);

        merge(a); merge(u); merge(v); merge(s);
        svd(a[0], u[0], v[0], s[0]);
        split_d(u); split_d(v); split_d(s);
    }

    template<class Matrix, class DiagonalMatrix>
    inline void heev(const tiles<Matrix>& a, tiles<Matrix>& evecs, tiles<DiagonalMatrix>& evals){
        merge(a); merge(evecs); merge(evals);
        heev(a[0], evecs[0], evals[0]);
        split_d(a); split_d(evecs); split_d(evals);
    }

    template<class Matrix, class DiagonalMatrix>
    inline void syev(const tiles<Matrix>& a, tiles<Matrix>& evecs, tiles<DiagonalMatrix>& evals){
        heev(a, evecs, evals); // should be call syev instead?
    }
                
    template<class Matrix>
    void orgqr(const tiles<Matrix>&& a, tiles<Matrix>&& q, const tiles<Matrix>&& t){
        int k, m, n;

        for(k = std::min(a.mt, a.nt)-1; k >= 0; k--){
            for(m = q.mt - 1; m > k; m--)
                for(n = 0; n < q.nt; n++)
                    tsmqr<PlasmaNoTrans>(a.tile(m, k).num_cols(), q.tile(k, n), q.tile(m, n), a.tile(m, k), t.tile(m, k));
            
            for(n = 0; n < q.nt; n++)
                ormqr<PlasmaNoTrans>(std::min(a.tile(k,k).num_rows(), a.tile(k,k).num_cols()), a.tile(k, k), t.tile(k, k), q.tile(k, n));
        }
    }

    template<class Matrix>
    void orglq(const tiles<Matrix>&& a, tiles<Matrix>&& q, const tiles<Matrix>&& t){
        int k, m, n;

        for(k = std::min(a.mt, a.nt)-1; k >= 0; k--){
            for(n = q.nt-1; n > k; n--)
                for(m = 0; m < q.mt; m++)
                    tsmlq<PlasmaNoTrans>(a.tile(k, n).num_rows(), q.tile(m, k), q.tile(m, n), a.tile(k, n), t.tile(k, n));
        
            for(m = 0; m < q.mt; m++)
                ormlq<PlasmaNoTrans>(std::min(a.tile(k,k).num_rows(), a.tile(k,k).num_cols()), a.tile(k, k), t.tile(k, k), q.tile(m, k));
        }
    }

    template<class Matrix>
    void ormqr(tiles<Matrix>&& a, tiles<Matrix>&& b, tiles<Matrix>&& t){
        int k, m, n;
    
       // PlasmaLeft / PlasmaTrans
        for(k = 0; k < std::min(a.mt, a.nt); k++){
            size_t kmin = std::min(a.tile(k,k).num_rows(), a.tile(k,k).num_cols());
            for(n = 0; n < b.nt; n++){
                ormqr<PlasmaTrans>(kmin, a.tile(k, k), t.tile(k, k), b.tile(k, n));
            }
            for(m = k+1; m < b.mt; m++){
                for(n = 0; n < b.nt; n++)
                    tsmqr<PlasmaTrans>(kmin, b.tile(k, n), b.tile(m, n), a.tile(m, k), t.tile(m, k));
            }
        }
    }

    template<class Matrix>
    void ormlq(tiles<Matrix>&& a, tiles<Matrix>&& b, tiles<Matrix>&& t){
        int k, m, n;
        
        // PlasmaRight / PlasmaTrans
        for(k = 0; k < std::min(a.mt, a.nt); k++){
            size_t kmin = std::min(a.tile(k,k).num_rows(), a.tile(k,k).num_cols());
            for(m = 0; m < b.mt; m++){
                ormlq<PlasmaTrans>(kmin, a.tile(k, k), t.tile(k, k), b.tile(m, k));
            }
            for(n = k+1; n < b.nt; n++){
                for(m = 0; m < b.mt; m++){
                    tsmlq<PlasmaTrans>(kmin, b.tile(m, k), b.tile(m, n), a.tile(k, n), t.tile(k, n));
                }
            }
        }
    }

    template<class Matrix>
    inline void qr(tiles<Matrix>&& a, tiles<Matrix>&& t){
        int k, m, n;
        
        for(k = 0; k < std::min(a.mt, a.nt); k++){
            geqrt(a.tile(k, k), t.tile(k, k));
        
            for(n = k+1; n < a.nt; n++)
                ormqr<PlasmaTrans>(a.tile(k,n).num_rows(), a.tile(k, k), t.tile(k, k), a.tile(k, n));
            
            for(m = k+1; m < a.mt; m++){
                tsqrt(a.tile(k, k), a.tile(m, k), t.tile(m, k));
        
                for(n = k+1; n < a.nt; n++)
                    tsmqr<PlasmaTrans>(AMBIENT_IB, a.tile(k, n), a.tile(m, n), a.tile(m, k), t.tile(m, k));
            }
        }
    }


    template<class Matrix>
    inline void qr(tiles<Matrix> a, tiles<Matrix>& q, tiles<Matrix>& r){
        int am = num_rows(a);
        int an = num_cols(a);
        int ak = std::min(am,an);
        resize(q, am, an);
        resize(r, ak, an);
        
        tiles<Matrix> t(a.mt*AMBIENT_IB, a.nt*AMBIENT_IB);
        qr(AMBIENT_MOVE(a), AMBIENT_MOVE(t));
        
        // restoring R from A //
        for(int j = 0; j < a.nt; j++)
        for(int i = 0; i < j && i < std::min(a.mt, a.nt); i++)
            copy(a.tile(i,j), r.tile(i,j));
        
        for(int k = 0; k < std::min(a.mt, a.nt); k++)
            copy_rt(a.tile(k,k), r.tile(k,k));
        
        // restoring Q from T //
        // do we need to memset Q first?
        for(int i = 0; i < std::min(a.mt, a.nt); i++)
            fill_identity(q.tile(i,i));
       
        orgqr(AMBIENT_MOVE(a), AMBIENT_MOVE(q), AMBIENT_MOVE(t)); 
        resize(q, am, ak);
    }

    template<class Matrix>
    inline void lq(tiles<Matrix>&& a, tiles<Matrix>&& t){
        int k, m, n;
        
        for(k = 0; k < std::min(a.mt, a.nt); k++) {
            gelqt(a.tile(k, k), t.tile(k, k));
        
            for(m = k+1; m < a.mt; m++)
                ormlq<PlasmaTrans>(a.tile(m, k).num_cols(), a.tile(k, k), t.tile(k, k), a.tile(m, k));
        
            for(n = k+1; n < a.nt; n++){
                tslqt(a.tile(k, k), a.tile(k, n), t.tile(k, n));
        
                for(m = k+1; m < a.mt; m++)
                    tsmlq<PlasmaTrans>(AMBIENT_IB, a.tile(m, k), a.tile(m, n), a.tile(k, n), t.tile(k, n));
            }
        }
    }

    template<class Matrix>
    inline void lq(tiles<Matrix> a, tiles<Matrix>& l, tiles<Matrix>& q){
        int am = num_rows(a);
        int an = num_cols(a);
        int ak = std::min(am,an);
        resize(l, am, ak);
        resize(q, am, an); // instead of ak
        
        tiles<Matrix> t(a.mt*AMBIENT_IB, a.nt*AMBIENT_IB);
        lq(AMBIENT_MOVE(a), AMBIENT_MOVE(t));
        
        // restoring L from A //
        for(int j = 0; j < std::min(a.mt, a.nt); ++j)
        for(int i = j+1; i < a.mt; ++i)
            copy(a.tile(i,j), l.tile(i,j));
        
        for(int k = 0; k < std::min(a.mt, a.nt); k++)
            copy_lt(a.tile(k,k), l.tile(k,k));
        
        // restoring Q from T //
        // do we need to memset Q first?
        for(int i = 0; i < std::min(a.mt, a.nt); i++)
            fill_identity(q.tile(i,i));
       
        orglq(AMBIENT_MOVE(a), AMBIENT_MOVE(q), AMBIENT_MOVE(t)); 
        resize(q, ak, an);
    }

    template<class Matrix> 
    inline void resize(tiles<Matrix>& a, size_t m, size_t n){ 
        if(a.num_rows() == m && a.num_cols() == n) return;
        tiles<Matrix> r(m, n);

        int mb_min = std::min(r.mt, a.mt);
        int nb_min = std::min(r.nt, a.nt);
        
        for(int j = 0; j < nb_min; j++){
            for(int i = 0; i < mb_min; i++){
                r.tile(i,j) = a.tile(i,j);
            }
        }
        size_t margin = AMBIENT_IB;
        if(r.mt <= a.mt) margin = __a_mod(m, AMBIENT_IB);
        for(int j = 0; j < nb_min; j++){
            Matrix& block = r.tile(mb_min-1,j);
            resize(block, margin, block.num_cols());
        }
        margin = AMBIENT_IB;
        if(r.nt <= a.nt) margin = __a_mod(n, AMBIENT_IB);
        for(int i = 0; i < mb_min; i++){
            Matrix& block = r.tile(i,nb_min-1);
            resize(block, block.num_rows(), margin);
        }
        swap(a, r);
    }

    template<typename T> 
    inline void resize(tiles<diagonal_matrix<T> >& a, size_t m, size_t n){
        if(a.num_rows() == m) return;
        tiles<diagonal_matrix<T> > r(m);

        int nb_min = std::min(r.nt, a.nt);
        
        for(int i = 0; i < nb_min; i++){
            r[i] = a[i];
        }
        if(r.nt > a.nt){
            resize(r[nb_min-1], AMBIENT_IB, AMBIENT_IB);
        }else{
            size_t margin = __a_mod(m, AMBIENT_IB);
            resize(r[r.nt-1], margin, margin);
        }
        swap(a, r);
    }

    template<typename T> 
    inline void sqrt_inplace(tiles<diagonal_matrix<T> >& a){
        int size = a.data.size();
        for(int i = 0; i < size; i++){
            sqrt_inplace(a[i]);
        }
    }

    template<typename T>
    inline void exp_inplace(tiles<diagonal_matrix<T> >& a, const T& alfa = 1.){
        int size = a.data.size();
        for(int i = 0; i < size; i++){
            exp_inplace(a[i], alfa);
        }
    }

    template<class Matrix> 
    inline scalar_type trace(const tiles<Matrix>& a){
        int size = std::min(a.nt, a.mt);
        scalar_type result(0.);
        std::vector<scalar_type> parts;
        parts.reserve(size);
        
        for(int k = 0; k < size; k++) parts.push_back(trace(a.tile(k, k)));
        for(int k = 0; k < size; k++) result += parts[k];
        return result;
    }

    template <class Matrix>
    inline real_type norm_square(const tiles<Matrix>& a){
        int size = a.nt*a.mt;
        real_type result(0.);
        std::vector<real_type> parts;
        parts.reserve(size);
        
        for(int k = 0; k < size; k++) parts.push_back(norm_square(a[k]));
        for(int k = 0; k < size; k++) result += parts[k];
        return result;
    }

    template <class Matrix>
    inline scalar_type overlap(const tiles<Matrix>& a, const tiles<Matrix>& b){
        int size = a.nt*a.mt;
        scalar_type result(0.);
        std::vector<scalar_type> parts;
        parts.reserve(size);
        
        for(int k = 0; k < size; k++) parts.push_back(overlap(a[k], b[k]));
        for(int k = 0; k < size; k++) result += parts[k];
        return result;
    }
        
    template<class Matrix>
    inline void swap(tiles<Matrix>& x, tiles<Matrix>& y){ 
        x.swap(y);                     
    }

    template<class Matrix>
    inline void transpose_inplace(tiles<Matrix>& a){
        std::vector<Matrix*> t;
        t.reserve(a.mt*a.nt);
        for(int i = 0; i < a.mt; i++){
            for(int j = 0; j < a.nt; j++){
                Matrix& block = a.tile(i,j);
                transpose_inplace(block);
                t.push_back(&block);
            }
        }
        std::swap(a.data, t);
        std::swap(a.rows, a.cols);
        std::swap(a.mt,   a.nt);
    }

    template<class Matrix>
    inline tiles<transpose_view<Matrix> > transpose(const tiles<Matrix>& a){
        tiles<transpose_view<Matrix> > t;
        std::vector<transpose_view<Matrix>*> data;
        data.reserve(a.mt*a.nt);
        for(int i = 0; i < a.mt; i++)
            for(int j = 0; j < a.nt; j++)
                data.push_back(new transpose_view<Matrix>(a.tile(i,j)));
        std::swap(t.data, data);
        t.cols = a.num_rows();
        t.rows = a.num_cols();
        t.nt   = a.mt;
        t.mt   = a.nt;
        return t;
    }

    template<class Matrix>
    inline void adjoint_inplace(tiles<Matrix>& a){
        transpose_inplace(a);
    }

    template<class Matrix>
    inline tiles<transpose_view<Matrix> > adjoint(const tiles<Matrix>& a){
        return transpose(a);
    }

    template<class Matrix, class G>
    inline void generate(tiles<Matrix>& a, G g){
        generate(a);
    }

    template<class Matrix>
    inline void generate(tiles<Matrix>& a){
        int size = a.data.size();
        for(int i = 0; i < size; i++){
            fill_random(a[i]);
        }
    }

    template<class Matrix>
    inline void persist(const tiles<Matrix>& a){
        int size = a.data.size();
        for(int i = 0; i < size; i++){
            persist(a[i]);
        }
    }

    template <class MatrixA, class MatrixB>
    inline void add_inplace(tiles<MatrixA>& lhs, const tiles<MatrixB>& rhs){
        int size = lhs.data.size();
        for(int i = 0; i < size; i++){
            lhs[i] += rhs[i];
        }
    }

    template <class MatrixA, class MatrixB>
    inline void sub_inplace(tiles<MatrixA>& lhs, const tiles<MatrixB>& rhs){ 
        int size = lhs.data.size();
        for(int i = 0; i < size; i++){
            lhs[i] -= rhs[i];
        }
    }

    template <class Matrix>
    inline void mul_inplace(tiles<Matrix>& a, const scalar_type& rhs) {
        #ifdef AMBIENT_LOOSE_FUTURE
        if(!rhs.valid) rhs.get();
        #endif
        int size = a.data.size();
        for(int i = 0; i < size; i++){
            a[i] *= rhs;
        }
    }

    template <class Matrix>
    inline void div_inplace(tiles<Matrix>& a, const scalar_type& rhs){
        #ifdef AMBIENT_LOOSE_FUTURE
        if(!rhs.valid) rhs.get();
        #endif
        int size = a.data.size();
        for(int i = 0; i < size; i++)
            a[i] /= rhs;
    }
 
    template <class Matrix>
    inline void save(const tiles<Matrix>& a, size_t tag){
        split_d(a);
        int size = a.data.size();
        for(int i = 0; i < size; ++i)
           save(a[i], (tag+i));//tag is done over blocks      
    }

    template <class Matrix>
    inline void load(tiles<Matrix>& a, size_t tag){
        split_d(a);
        int size = a.data.size();
        for(int i = 0; i < size; ++i)
           load(a[i], (tag+i));        
    }

    template <class Matrix>
    std::ostream& operator << (std::ostream& o, const tiles<Matrix>& a){
        int size = a.data.size();
        for(int i = 0; i < size; i++)
            o << a[i];
        return o;
    }

    template <class Matrix> 
    inline tiles<Matrix> operator + (tiles<Matrix> lhs, const tiles<Matrix>& rhs){ 
        return (lhs += rhs);
    }

    template <class Matrix> 
    inline tiles<Matrix> operator - (tiles<Matrix> lhs, const tiles<Matrix>& rhs){ 
        return (lhs -= rhs);
    }

    template <class Matrix, class MatrixB>
    inline tiles<matrix<value_type> > operator * (const tiles<Matrix>& lhs, const tiles<MatrixB>& rhs){
        tiles<matrix<value_type> > ret(lhs.num_rows(), rhs.num_cols());
        gemm(lhs, rhs, ret);
        return ret; 
    }

    template<class Matrix, typename T> 
    inline const tiles<Matrix> operator * (tiles<Matrix> lhs, const T& rhs){ 
        return (lhs *= rhs); 
    }

    template<class Matrix, typename T> 
    inline const tiles<Matrix> operator * (const T& lhs, tiles<Matrix> rhs){ 
        return (rhs *= lhs); 
    }

    template<class Matrix> inline size_type num_rows(const tiles<Matrix>& a){ 
        return a.num_rows();
    }

    template<class Matrix> inline size_type num_cols(const tiles<Matrix>& a){
        return a.num_cols();
    }

} }

#undef size_type
#undef real_type
#undef scalar_type
#undef difference_type 
#endif
