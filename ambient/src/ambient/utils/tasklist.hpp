#ifndef AMBIENT_UTILS_TASKLIST
#define AMBIENT_UTILS_TASKLIST
#define TASKLIST_LENGTH 131072

namespace ambient{

// 8192 -- ff_short
// 65536 -- ff_large
// 131072 -- fermi ladder

    class tasklist_async {
    public:
        struct task {
            inline task(){}
            inline task(void* o, dim2 pin):o(o),pin(pin){}
            void* o;
            dim2 pin;
        };

        inline tasklist_async(): ri(content), wi(content) { }

        inline task* get_task(){
            return ri++;
        }
        inline void add_task(task&& e){
            *wi++ = e;
        }
        inline bool end_reached(){
            return (ri == wi);
        }
        inline void reset(){
            ri = wi = content;
        }
    private:
        task content[TASKLIST_LENGTH];
        task* wi; 
        task* ri;
    };

    class tasklist {
    public:
        class task {
        public:
            inline task(void* o, dim2 pin):o(o),pin(pin),next(NULL){}
            task* next;
            void* o;
            dim2 pin;
        };

        inline tasklist()
        : seed(NULL), tail(NULL), active(true)
        {
            pthread_mutex_init(&mutex, NULL);
        }
        inline ~tasklist(){
            pthread_mutex_destroy(&mutex);
        }
        inline void add_task(task* t){ // delegate thread
            pthread_mutex_lock(&mutex);
            if(this->seed == NULL) this->seed = t;
            else this->tail->next = t;
            this->tail = t;
            pthread_mutex_unlock(&mutex);
        }
        inline task* get_task(){ // worker thread
            pthread_mutex_lock(&mutex);
            task* data = this->seed;
            if(this->seed != NULL){
                if(this->seed == this->tail) this->tail = this->seed = NULL;
                else this->seed = this->seed->next;
            }
            pthread_mutex_unlock(&mutex);
            return data;
        }

        task* seed;
        task* tail;
        pthread_mutex_t mutex;
        bool active;
        size_t id;
    };

}

#endif
