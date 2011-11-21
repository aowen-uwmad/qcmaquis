#ifndef AMBIENT_CORE_LAYOUT_H
#define AMBIENT_CORE_LAYOUT_H
#include <list>
#include <map>
#include <vector>

namespace ambient { 
    class p_object;

namespace core {

    class layout_table {
    public:
        class composite_marker {
        public:
            composite_marker();
            bool active;
            size_t imarker;
            size_t jmarker;
            void mark(int i, int j);
            bool has_marked(int i, int j);
            void clear();
        };
        class entry {
        public:
            entry(); // default constructor
            entry(int owner, int i, int j = 0);
            int i;
            int j;
            int owner;
            int xowner;
            int get_xowner();
            int get_owner();
        };

        layout_table(p_object* object);
       ~layout_table();
        void remap();

        std::vector<core::layout_table::entry>& get_list();
        entry* get_entry(int i, int j = 0);
        entry* operator()(int i, int j = 0);

        void add_segment_entry(int owner, int i, int j = 0);
        void add_request_entry(int i, int j = 0);
        void update_map_entry(int owner, int i, int j = 0);

        void record(int i, int j = 0);  // general call invoking one above
        void request(int i, int j = 0); // request for the block (read-only purpose)

        void clean();
        void print();

        int  get_xmaster();
        int  get_master();
        void set_master(int master);

        p_object* object;
        std::vector< std::vector<entry*> > map;
        std::vector<entry> segment;
        std::vector<entry> requests;
        composite_marker init_marker;
        size_t reserved_x;
        size_t reserved_y;
        size_t count;
        size_t segment_count;
        size_t request_count;
        std::pair<int,int>  master_relay;
    };

    void apply_changes(p_object** objects, size_t count);

} }
#endif
