/*
 * Ambient, License - Version 1.0 - May 3rd, 2012
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define AMBIENT_MASTER_RANK 0
//#define AMBIENT_MPI_THREADING MPI_THREAD_MULTIPLE
#define AMBIENT_MPI_THREADING MPI_THREAD_FUNNELED

namespace ambient { namespace channels { namespace mpi {

    inline channel::~channel(){
        MPI_Finalize();
    }

    inline void channel::init(){
        int level, zero = 0;
        //MPI_Init(&zero, NULL);
        MPI_Init_thread(&zero, NULL, AMBIENT_MPI_THREADING, &level);
        if(level != AMBIENT_MPI_THREADING) printf("Error: Wrong threading level\n");
        this->world = new group(AMBIENT_MASTER_RANK, MPI_COMM_WORLD);
        this->volume = this->world->size;
        this->db_volume = this->volume > AMBIENT_DB_PROCS ? AMBIENT_DB_PROCS : 0;
    }

    inline void channel::barrier(){
        MPI_Barrier(MPI_COMM_WORLD);
    }

    inline size_t channel::dim(){
        return this->volume;
    }

    inline size_t channel::wk_dim(){
        return (this->volume-this->db_volume);
    }

    inline size_t channel::db_dim(){
        return this->db_volume;
    }

    inline int channel::index(){
        int s = this->sid++;
        this->sid %= AMBIENT_MAX_SID;
        return s;
    }

    inline request* channel::get(transformable* v, int tag){
        request* q = new request();
        MPI_Irecv(&v->v, 
                  (int)sizeof(transformable::numeric_union)/sizeof(double), // should be multiple of 8
                  MPI_DOUBLE, 
                  MPI_ANY_SOURCE, 
                  tag, 
                  MPI_COMM_WORLD, 
                  &(q->mpi_request));
        return q;
    }

    inline request* channel::set(transformable* v, int rank, int tag){
        if(rank == ambient::rank()) return NULL;
        request* q = new request();
        MPI_Isend(&v->v, 
                  (int)sizeof(transformable::numeric_union)/sizeof(double), // should be multiple of 8
                  MPI_DOUBLE, 
                  rank, 
                  tag, 
                  MPI_COMM_WORLD, 
                  &(q->mpi_request));
        return q;
    }

    inline request* channel::get(revision* r, int tag){
        request* q = new request();
        MPI_Irecv(r->data, 
                  (int)r->spec.extent/sizeof(double), 
                  MPI_DOUBLE, 
                  MPI_ANY_SOURCE, 
                  tag, 
                  MPI_COMM_WORLD, 
                  &(q->mpi_request));
        return q;
    }

    inline request* channel::set(revision* r, int rank, int tag){
        if(rank == ambient::rank()) return NULL;
        request* q = new request();
        // can be ready-send
        MPI_Isend(r->data, 
                  (int)r->spec.extent/sizeof(double), 
                  MPI_DOUBLE, 
                  rank, 
                  tag, 
                  MPI_COMM_WORLD, 
                  &(q->mpi_request));
        return q;
    }

    inline bool channel::test(request* q){
        if(q == NULL) return true; // for transformable
        int flag = 0;
        MPI_Test(&(q->mpi_request), &flag, MPI_STATUS_IGNORE);
        return flag;
    }

    inline void channel::wait(request* q){
        if(q == NULL) return; // for transformable
        MPI_Wait(&(q->mpi_request), MPI_STATUS_IGNORE);
    }

} } }
