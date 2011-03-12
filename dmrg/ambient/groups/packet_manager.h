#ifndef AMBIENT_GROUPS_PACKET_MANAGER_H
#define AMBIENT_GROUPS_PACKET_MANAGER_H

#include "ambient/packets/types.h"
#include "ambient/packets/packet.h"
#include "ambient/auxiliary.h"
#include <list>

using namespace ambient::packets; 

namespace ambient{ namespace groups{

    class group;
    class packet_manager
    {
    public: 
        enum locking_fsm { OPEN, LOOSE, CLOSURE, CLOSED };
        enum direction   { IN, OUT };
    private: 
        packet_manager(packet_manager const&);             // copy constructor is private
        packet_manager& operator=(packet_manager const&);  // assignment operator is private
        group* scope;
        MPI_Comm* comm;
    public:
        int closure_mutex;
        int approve_closure_mutex;

        packet_manager(group* scope);
        class ambient_request
        {
        public:
            ambient_request(void* memory);
            MPI_Request request;
            void* memory;
        };
        class typed_q
        {
        public:
            typed_q(packet_manager* manager, const packet_t& type, direction flow, int reservation, int priority);
           ~typed_q();
            delegate packet_delivered;
            void push(packet* pack);
            void process();
            packet* get_target_packet();
            int priority;
            size_t active_requests_number;
            direction flow;
            packet_manager* manager;
            const packet_t& type;
        private:
            void recv(ambient_request* request);
            void send(ambient_request* request);

            packet* target_packet;
            int reservation;
            std::vector<ambient_request*> requests;
        };

        locking_fsm state;
        std::list<typed_q*> qs;
        typed_q* add_typed_q(const packet_t& type, direction flow, int reservation = 1, int priority = 1);

        bool     subscribed(const packet_t& type);
        void     subscribe(const packet_t& type);
        void     add_handler(const packet_t& type, core::operation* callback);
        void     emit(packet* pack);
        typed_q* get_pipe(const packet_t& type, direction flow);

        void process();
        bool process_locking(size_t active_sends_number);
        group* get_scope();
    };

} }
#endif
