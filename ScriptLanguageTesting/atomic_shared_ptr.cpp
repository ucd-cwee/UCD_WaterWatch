
#include "atomic_shared_ptr.h"
#include "atomic_stack.h"

namespace GL {
    void control_block_base::DeferredDeletion(control_block_base* to_delete) {     
        if (to_delete && (to_delete->refCount.fetch_sub(1, std::memory_order_relaxed) == 1)) {
            to_delete->Delete();
            to_delete->DeleteSelf(to_delete);
        }        
    };
};