#pragma once
#include "Parallel.h"
#include "functions.h"
namespace GL {
	namespace parallel {
		namespace id {
			struct TaskBase {
				impl::dispatch_context
					_ctx; // waitable context for the job
				GL::any
					_result;
				void*
					_function_pointer;
				std::array<GL::any, 16>
					_arguments;
				size_t
					_count;
				size_t
					_offset;
				std::vector<unsigned long>
					_parent_uuid;
				bool
					_job_completed;
				GL::atomic_uint_stack
					_children;
				void(*_deferred_dispatch)(unsigned int, TaskBase&);
				unsigned int
					_parent_count;
			};
			static constexpr unsigned long
				TEMPORARY = 0x1000'0000;
			static constexpr unsigned long
				FLAGS = 0xF000'0000;
			static constexpr unsigned long
				INV_FLAGS = ~FLAGS;

			struct id_ticket {
				size_t
					count;
				TaskBase
					data;
			};
			unsigned long new_id() noexcept;
			void free_id(unsigned long uuid) noexcept;
			id_ticket& get_id(unsigned long uuid) noexcept;
		};
		namespace single_threaded {
#include "ParallelTasksImpl.h" // code is repeated for the single-threaded version
		};
#include "ParallelTasksImpl.h" // code is repeated for the multi-threaded version
	};
};