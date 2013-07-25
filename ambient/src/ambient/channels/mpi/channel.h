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

#ifndef AMBIENT_CHANNELS_MPI_CHANNEL
#define AMBIENT_CHANNELS_MPI_CHANNEL
#include "ambient/channels/mpi/groups/group.h"
#include "ambient/channels/mpi/groups/multirank.h"

namespace ambient { namespace channels { namespace mpi {

    using ambient::models::velvet::revision;
    using ambient::models::velvet::transformable;

    class request {
    public:
        void* operator new (size_t size){ return ambient::pool::malloc<bulk,request>(); }
        void operator delete (void* ptr){ }
        request(void* memory);
        MPI_Request mpi_request;
        void* memory;
    };

    class channel : public singleton< channel > {
    public:
       ~channel();
        void  init();
        size_t dim();
        size_t wk_dim();
        size_t db_dim();
        request* get(revision* r);
        request* set(revision* r, int rank);
        request* get(transformable* v);
        request* set(transformable* v, int rank);
        bool test(request* r);
        bool wait(request* r);
        group* world;
    private:
        size_t volume;
        size_t db_volume;
    };

} } }

namespace ambient {
    extern channels::mpi::channel& channel;
}

#include "ambient/channels/mpi/channel.hpp"
#include "ambient/channels/mpi/groups/multirank.hpp"
#include "ambient/channels/mpi/groups/group.hpp"
#endif
