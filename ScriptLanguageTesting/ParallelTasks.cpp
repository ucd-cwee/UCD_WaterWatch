#include "ParallelTasks.h"

namespace GL {
	namespace parallel {
		// Definition
		namespace id {
			GL::ticket_dispensor<false>
				tickets;
			GL::atomic_vector<id_ticket> 
				slots;

			unsigned long new_id() noexcept {
				unsigned long uuid = (unsigned long)tickets.get_ticket();
				auto& ref = slots.get_or_make(uuid);
				ref.count = 1;
				ref.data._ctx.counter = 0;
				ref.data._ctx.e = nullptr;
				ref.data._ctx.callback = nullptr;
				ref.data._ctx.callback_data = nullptr;
				ref.data._result = nullptr;
				ref.data._parent_uuid.clear();
				ref.data._function_pointer = nullptr;
				for (auto& x : ref.data._arguments) x = nullptr;
				ref.data._count = 0;
				ref.data._offset = 0;
				ref.data._job_completed = false;
				ref.data._deferred_dispatch = nullptr;
				ref.data._parent_count = 0;
				return uuid;
			};
			void free_id(unsigned long uuid) noexcept {
				uuid &= INV_FLAGS;
				if (uuid > 0) {
					auto& ref = slots[uuid];
					//std::vector<unsigned long> parent_uuid_copy = std::move(ref.data._parent_uuid);
					ref.data._ctx.counter = 0;
					ref.data._ctx.e = nullptr;
					ref.data._ctx.callback = nullptr;
					ref.data._ctx.callback_data = nullptr;
					ref.data._result = nullptr;
					ref.data._function_pointer = nullptr;
					ref.data._arguments = {};
					ref.data._count = 0;
					ref.data._offset = 0;
					ref.data._job_completed = false;
					ref.data._deferred_dispatch = nullptr;
					ref.data._parent_count = 0;
					tickets.return_ticket(uuid);
				}
			};
			id_ticket& get_id(unsigned long uuid) noexcept {
				return slots.at(uuid & INV_FLAGS);
			};
		};
		
	}
}
