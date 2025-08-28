#include "atomic_shared_ptr.h"
#include "atomic_stack.h"

namespace GL {
    // wrapper for std::thread. Can be constructed with a function that it will perform in a seperate thread. When the function returns, it will sleep until destroyed or woken to do the function once more, and so on. 
    template <void (*Func)(void)> class Taskable {
        std::atomic<bool> 
            alive;
        std::condition_variable 
            wakeCondition;
        std::mutex 
            wakeMutex;
        std::thread 
            thread;

    public:        
        Taskable() 
            : alive{ 1 }, wakeMutex{}, wakeCondition{}
        {
            thread = std::thread{ [this] {
                // pre-warm this thread's heap
                for (int i = 0; i < 100000; i++) delete (new int(i));

                while (this->alive.load()) {
                    // Work until no more jobs are found
                    Func();

                    // go to sleep, to be awoken when new jobs are added
                    auto lock{ std::unique_lock(this->wakeMutex) };
                    // this->wakeCondition.wait(lock);
                    this->wakeCondition.wait_for(lock, std::chrono::milliseconds(1000 / 60));
                }
            } };
        }
        Taskable(Taskable const&) = delete;
        Taskable(Taskable &&) = delete;
        Taskable& operator=(Taskable const&) = delete;
        Taskable& operator=(Taskable&&) = delete;
        ~Taskable() {
            if (alive) {
                alive = false; // indicate that new jobs cannot be started from this point                
                wakeCondition.notify_all();
                thread.join();
            }
        }

        void wake() { // try to wake-up the thread if it is sleeping. 
            wakeCondition.notify_one();
        };
    };

    void control_block_base::DeferredDeletion(control_block_base* to_delete) {
        static GL::atomic_parallel_stack< control_block_base* > _destruction_queue{};
        struct t {
            static void DeleteFunc(void) {
                control_block_base* out{ nullptr };
                while (_destruction_queue.try_pop(out)) {
                    if (out) {
                        out->Delete();
                        delete out;
                    }
                    out = nullptr;
                }
            };
        };        
        static Taskable<t::DeleteFunc> _destruction_thread{};
        if (to_delete) {
            size_t before = to_delete->refCount.fetch_sub(1);
            if (before == 1) {
                if ((_destruction_queue.push(to_delete) & 255) == 0) {
                    _destruction_thread.wake();
                }
            }
        }
    };
};