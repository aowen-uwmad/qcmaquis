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

#ifndef AMBIENT_CONTROLLERS_SSM_SCOPE_HPP
#define AMBIENT_CONTROLLERS_SSM_SCOPE_HPP

namespace ambient { 

        inline int scope::balance(int k, int max_k){
            if(max_k > ambient::num_workers()){
                int k_ = k/((int)(max_k / ambient::num_workers()));
                if(k_ < ambient::num_workers()) return k_;
            }
            return k;
        }
        inline int scope::permute(int k, const std::vector<int>& s){
            if(k >= s.size()){ printf("Error: permutation overflow!\n"); return k; }
            return s[k];
        }
        inline scope::~scope(){
            if(dry) return;
            selector.revoke_controller(controller);
            selector.pop_scope();
        }
        inline scope::scope(scope_t t) : type(t){
            if(t == scope_t::common){
                if(selector.has_nested_scope()){
                    if(selector.get_scope().type == scope_t::common){ this->dry = true; return; }
                    printf("Error: common scope inside other scope type\n");
                }else{
                    this->dry = false;
                    this->rank = selector.get_controller().get_shared_rank();
                    this->state = ambient::locality::common;
                    controller = selector.provide_controller();
                    selector.push_scope(this);
                }
            }else{
                printf("Error: unknown scope type!\n");
            }
        }
        inline scope::scope(int r) : type(scope_t::single) {
            controller = selector.provide_controller(); // need to change dry stuff
            if(ambient::selector.has_nested_scope()) dry = true;
            else{ 
                dry = false; 
                selector.push_scope(this); 
            }
            this->round = controller->get_num_workers();
            this->set(r);
        }
        inline void scope::set(int r){
            this->rank = r % this->round;
            this->state = (this->rank == controller->get_rank()) ? ambient::locality::local : ambient::locality::remote;
        }
        inline bool scope::remote() const {
            return (state == ambient::locality::remote);
        }
        inline bool scope::local() const {
            return (state == ambient::locality::local);
        }
        inline bool scope::common() const {
            return (state == ambient::locality::common);
        }
        inline int scope::which() const {
            return this->rank;
        }
        

        inline base_scope::base_scope(){
            this->controller = selector.provide_controller();
            this->controller->reserve(ambient::isset("AMBIENT_DB_NUM_PROCS") ? ambient::getint("AMBIENT_DB_NUM_PROCS") : 0);
            this->round = controller->get_num_workers();
            this->scores.resize(round, 0);
            this->set(0);
        }
        inline void base_scope::intend_read(models::ssm::revision* r){
            if(r == NULL || model_type::common(r)) return;
            this->scores[model_type::owner(r)] += r->spec.extent;
        }
        inline void base_scope::intend_write(models::ssm::revision* r){
            if(r == NULL || model_type::common(r)) return;
            this->stakeholders.push_back(model_type::owner(r));
        }
        inline void base_scope::schedule(){
            int max = 0;
            int rank = this->rank;
            if(stakeholders.empty()){
                for(int i = 0; i < this->round; i++)
                if(scores[i] >= max){
                    max = scores[i];
                    rank = i;
                }
            }else{
                for(int i = 0; i < stakeholders.size(); i++){
                    int k = stakeholders[i];
                    if(scores[k] >= max){
                        max = scores[k];
                        rank = k;
                    }
                }
                stakeholders.clear();
            }
            std::fill(scores.begin(), scores.end(), 0);
            this->set(rank);
        }

}

#endif
