#include "Utilities.h"
#include "Stopwatch.h"
#include <chrono>

namespace GL {
    namespace util {
        std::atomic<long>& __thread_alive(size_t thread_id) {
            static concurrency::concurrent_vector<std::atomic<long>> thread_ids{};
            static std::atomic<size_t> thread_count{ 0 };
            if (thread_count.load(std::memory_order_relaxed) <= thread_id) {
                thread_ids.grow_to_at_least(thread_id + 1);
                thread_count.exchange(thread_id, std::memory_order_relaxed);
            }
            return thread_ids[thread_id];
        };
        bool get_thread_alive(size_t thread_id) {
            return static_cast<bool>(__thread_alive(thread_id).load());
        };
        size_t get_thread_id() {
            static TicketDispensor tickets;
            thread_local auto ticket{ tickets.get_ticket() };
            thread_local auto scoped_alive{
                // increments the "alive" during construction, and decrements during destruction.
                std::shared_ptr<size_t>(reinterpret_cast<size_t*>(ticket),[incrementOnce = ++__thread_alive(ticket)](size_t* p) -> void {
                    if (p && (incrementOnce > 0)) {
                        --__thread_alive(reinterpret_cast<size_t&>(p));
                        tickets.return_ticket(reinterpret_cast<size_t&>(p));
                    }
                })
            };
            return ticket;
        };
        long long get_current_epoch() {
            return clock::ms();
        };
    };
};

/*
 * GetOptimalCoreNumber() - Return the concurrency level on hardware
 *
 * This function returns the hardware concurrency level which usually
 * means independent sets of execution contexts. On C++11 this is the
 * return value of std::thread::hardware_concurrency, and its return value
 * counts hyper-threaded core as two different execution units.
 *
 * Nevertheless, the return value of this function is considered as the
 * optimal way of constructing an EM
 *
 * Please note that the EM is declared as a template which must take compile
 * time constant as its instanciation value. This function is only used for
 * debugging, and should not be used to derive the template argument
 */
unsigned int GetOptimalCoreNumber() {
    return std::thread::hardware_concurrency();
}