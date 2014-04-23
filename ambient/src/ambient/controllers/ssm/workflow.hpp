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

#ifndef AMBIENT_CONTROLLERS_SSM_WORKFLOW_HPP
#define AMBIENT_CONTROLLERS_SSM_WORKFLOW_HPP

namespace ambient { 

        inline workflow::workflow() : context_lane(ambient::num_threads()) {
            for(thread_context& k : context_lane) k.scope_ = &base;
            if(ambient::isset("AMBIENT_VERBOSE")){
                ambient::cout << "ambient: initialized ("                   << AMBIENT_THREADING_TAGLINE     << ")\n";
                if(ambient::isset("AMBIENT_MKL_NUM_THREADS")) ambient::cout << "ambient: selective threading (mkl)\n";
                ambient::cout << "ambient: size of instr bulk chunks: "     << AMBIENT_INSTR_BULK_CHUNK       << "\n";
                ambient::cout << "ambient: size of data bulk chunks: "      << AMBIENT_DATA_BULK_CHUNK        << "\n";
                if(ambient::isset("AMBIENT_BULK_LIMIT")) ambient::cout << "ambient: max chunks of data bulk: " << ambient::getint("AMBIENT_BULK_LIMIT") << "\n";
                if(ambient::isset("AMBIENT_BULK_REUSE")) ambient::cout << "ambient: enabled bulk garbage collection\n";
                if(ambient::isset("AMBIENT_BULK_DEALLOCATE")) ambient::cout << "ambient: enabled bulk deallocation\n";
                ambient::cout << "ambient: maximum sid value: "             << AMBIENT_MAX_SID                << "\n";
                ambient::cout << "ambient: number of db procs: "            << ambient::num_db_procs()        << "\n";
                ambient::cout << "ambient: number of work procs: "          << ambient::num_workers()         << "\n";
                ambient::cout << "ambient: number of threads per proc: "    << ambient::num_threads()         << "\n";
                ambient::cout << "\n";
            }
            if(ambient::isset("AMBIENT_MKL_NUM_THREADS")) mkl_parallel();
        }

        inline workflow::thread_context::sid_t::divergence_guard::divergence_guard(){
            for(auto& k : selector.context_lane){
                k.sid.max = k.sid.min = selector.context_lane[0].sid.value;
                k.scope_ = selector.context_lane[0].scope_;
            }
        }
        inline workflow::thread_context::sid_t::divergence_guard::~divergence_guard(){
            int& max = selector.context_lane[0].sid.value;
            for(auto& k : selector.context_lane){
                max = std::max(max, k.sid.max);
                k.sid.inc = 1;
            }
        }
        inline void workflow::thread_context::sid_t::offset(int offset, int increment){
            this->value = this->min + offset;
            this->inc = increment;
        }
        inline void workflow::thread_context::sid_t::maximize(){
            if(value < min) value += AMBIENT_MAX_SID;
            if(value > max) max = value;
        }
        inline int workflow::thread_context::sid_t::generate(){
            value = (value + inc) % AMBIENT_MAX_SID;
            return value;
        }
        inline int workflow::generate_sid(){
            return get_thread_context().sid.generate();
        }
        inline int workflow::get_sid() const {
            return get_thread_context().sid.value;
        }
        inline workflow::thread_context& workflow::get_thread_context() const {
            return context_lane[AMBIENT_THREAD_ID];
        }
        inline typename workflow::controller_type& workflow::get_controller() const {
            return *get_scope().controller; // caution: != get_thread_context().controller;
        }
        inline typename workflow::controller_type* workflow::provide_controller(){
            return &get_thread_context().controller;
        }
        inline void workflow::revoke_controller(controller_type* c){
            // some cleanups ?
        }
        inline void workflow::sync_all(){
            for(int k = 1; k < context_lane.size(); k++){
                for(auto i : *context_lane[k].controller.chains) context_lane[0].controller.queue(i);
                context_lane[k].controller.chains->clear();
            }
            for(auto& k : context_lane){
                k.controller.flush();
                k.controller.clear();
            }
            memory::data_bulk::drop();
        }
        inline bool workflow::has_nested_scope() const {
            return (&get_scope() != &this->base);
        }
        inline scope& workflow::get_scope() const {
            return *get_thread_context().scope_;
        }
        inline void workflow::pop_scope(){
            get_thread_context().scope_ = &this->base;
        }
        inline void workflow::push_scope(scope* s){
            get_thread_context().scope_ = s; // no nesting
        }
        inline bool workflow::tunable() const { 
            return (!get_controller().is_serial() && !has_nested_scope());
        }
        inline void workflow::intend_read(models::ssm::revision* r) const {
            base.intend_read(r); 
        }
        inline void workflow::intend_write(models::ssm::revision* r) const {
            base.intend_write(r); 
        }
        inline void workflow::schedule() const {
            base.schedule();
        }
        inline ambient::mutex& workflow::get_mutex() const {
            return mtx;
        }
}

#endif

