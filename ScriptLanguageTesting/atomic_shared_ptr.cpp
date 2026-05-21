#include "atomic_shared_ptr.h"
#include "atomic_stack.h"
#include "multithreaded_allocator.h"

namespace GL {    
    static auto Settup = []() -> bool {
        struct calc {
            static bool in_parallel(void) {
                return true;
            };
            static size_t thread_num(void) {
                return GL::util::get_thread_id();
            };
        };
        CppAD::thread_alloc::hold_memory(true);
        CppAD::thread_alloc::parallel_setup(GL::util::get_hardware_thread_count(), &calc::in_parallel, &calc::thread_num);
        return true;
    }();

    void* control_block_base::Allocate(size_t bytes) {
        // return ::_aligned_malloc(bytes, 16);
        return CppAD::thread_alloc::get_memory(bytes, bytes); // return ::_aligned_malloc(bytes, 16);
    };
    void control_block_base::Deallocate(void* ptr) {
        // ::_aligned_free(ptr);
        CppAD::thread_alloc::return_memory(ptr); // ::_aligned_free(ptr);
    };

    void control_block_base::DeferredDeletion(control_block_base* to_delete) {     
        if (to_delete && (to_delete->refCount.fetch_sub(1, std::memory_order_relaxed) == 1)) {
            to_delete->Delete();
            to_delete->DeleteSelf(to_delete);
        } 
        to_delete = nullptr;
    };
};