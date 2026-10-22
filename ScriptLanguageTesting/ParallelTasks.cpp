#include "ParallelTasks.h"

namespace GL {
	namespace parallel {
		// Definition
		namespace id {
			static GL::ticket_dispensor<false> tickets;
			static GL::atomic_vector< id_ticket > slots;
			unsigned long new_id() noexcept {
				unsigned long uuid = (unsigned long)tickets.get_ticket();
				auto& ref = slots.get_or_make(uuid);
				// std::memset(&ref, 0, sizeof(id_ticket));
				ref.count = 1;
				ref.data._ctx.counter = 0;
				ref.data._ctx.e = nullptr;
				ref.data._ctx.callback = nullptr;
				ref.data._ctx.callback_data = nullptr;
				//ref.data._result = nullptr;
				ref.data._parent_uuid = 0;
				ref.data._function_pointer = nullptr;
				//ref.data._arguments = {};
				ref.data._count = 0;
				ref.data._offset = 0;
				//ref.data._job_completed = false;
				return uuid;
			};
			void free_id(unsigned long uuid) noexcept {
				uuid &= INV_FLAGS;
				if (uuid > 0) {
					auto& ref = slots[uuid];
					if ((ref.data._parent_uuid & id::INV_FLAGS) > 0) {
						if ((ref.data._parent_uuid & id::TEMPORARY) == 0)
							if ((ref.data._parent_uuid & id::INV_FLAGS) > 0) {
								if (GL::interlocked::decrement(id::get_id(ref.data._parent_uuid).count) == 0)
									free_id(ref.data._parent_uuid);
								ref.data._parent_uuid = 0;
							}
					}
					ref.data._ctx.counter = 0;
					ref.data._ctx.e = nullptr;
					ref.data._ctx.callback = nullptr;
					ref.data._ctx.callback_data = nullptr;
					ref.data._result = nullptr;
					ref.data._parent_uuid = 0;
					ref.data._function_pointer = nullptr;
					ref.data._arguments = {};
					ref.data._count = 0;
					ref.data._offset = 0;
					ref.data._job_completed = false;

					tickets.return_ticket(uuid);
				}
			};
			id_ticket& get_id(unsigned long uuid) noexcept {
				return slots.at(uuid & INV_FLAGS);
			};
		};
		
	}
}
