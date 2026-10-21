#pragma once
#include "Parallel.h"
#include "functions.h"

namespace GL {
	namespace parallel {
		static constexpr bool use_cached_function_args{ true };

		namespace tasks {
			class TaskBase {
			public:
				impl::dispatch_context
					_ctx; // waitable context for the job
				GL::/*atomic_*/any
					_result;
				unsigned long
					_parent_uuid;
				void*
					_function_pointer;
				std::array<GL::any, 16>
					_arguments;
				size_t 
					_count;
				size_t
					_offset;
			};

			// Declaration
			namespace id {
				static constexpr unsigned long TEMPORARY = 0x1000'0000;
				static constexpr unsigned long FLAGS = 0xF000'0000;
				static constexpr unsigned long INV_FLAGS = ~FLAGS;

				struct id_ticket {
					size_t count{ 0 };
					TaskBase data{ { 0, nullptr, nullptr, nullptr }, nullptr, 0, nullptr, {}, 0, 0 };
				};
				unsigned long new_id() noexcept;
				void free_id(unsigned long uuid) noexcept;
				id_ticket& get_id(unsigned long uuid) noexcept;
			};

			class iTask {
				template<typename ReturnType> friend class Task;
			protected:
				unsigned long
					m_uuid;

			public:
				TaskBase& get_task() const noexcept {
					if ((m_uuid & id::INV_FLAGS) > 0)
						return id::get_id(m_uuid).data;
					else {
						static TaskBase out;
						return out;
					}
				};

				iTask(unsigned long uuid) noexcept : m_uuid{ uuid } {};
				iTask() noexcept : m_uuid{ 0 } {};
				iTask(std::nullptr_t) noexcept : m_uuid{ 0 } {};
				iTask(iTask const& rhs) : m_uuid{ rhs.m_uuid & id::INV_FLAGS } { if (m_uuid > 0) GL::interlocked::increment(id::get_id(m_uuid).count); };
				// declares that the parent will NOT go out-of-scope before this child does. That guarrantee allows us to skip a increment and decrement call to the counter. 
				iTask(iTask const& rhs, bool) : m_uuid{ rhs.m_uuid | id::TEMPORARY } {};
				iTask(iTask&& rhs) noexcept : m_uuid{ std::move(rhs.m_uuid) } { rhs.m_uuid = 0; };
				iTask const& operator=(iTask const& rhs) {
					if ((m_uuid & id::TEMPORARY) == 0)
						if ((m_uuid & id::INV_FLAGS) > 0)
							if (GL::interlocked::decrement(id::get_id(m_uuid).count) == 0)
								id::free_id(m_uuid);

					m_uuid = rhs.m_uuid & id::INV_FLAGS;
					if (m_uuid > 0) GL::interlocked::increment(id::get_id(m_uuid).count);
					return *this;
				};
				iTask const& operator=(iTask&& rhs) noexcept {
					if ((m_uuid & id::TEMPORARY) == 0)
						if ((m_uuid & id::INV_FLAGS) > 0)
							if (GL::interlocked::decrement(id::get_id(m_uuid).count) == 0)
								id::free_id(m_uuid);

					m_uuid = std::move(rhs.m_uuid);
					rhs.m_uuid = 0;
					return *this;
				};
				virtual ~iTask() {
					if ((m_uuid & id::TEMPORARY) == 0)
						if ((m_uuid & id::INV_FLAGS) > 0)
							if (GL::interlocked::decrement(id::get_id(m_uuid).count) == 0)
								id::free_id(m_uuid);
				};

				void wait() const {
					if ((m_uuid & id::INV_FLAGS) > 0) {
						auto& Base = id::get_id(m_uuid).data;
						if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
							iTask parentTask(Base._parent_uuid | id::TEMPORARY); // temporary does not invoke the shared counter
							parentTask.wait();
						}
						impl::Wait(Base._ctx, true);
					}
				};
				bool busy() const {
					if ((m_uuid & id::INV_FLAGS) > 0) {
						auto& Base = id::get_id(m_uuid).data;
						if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
							iTask parentTask(Base._parent_uuid | id::TEMPORARY); // temporary does not invoke the shared counter
							if (parentTask.busy()) return true;
						}
						if (Base._ctx.is_busy()) return true;
					}
					return false;
				};
				auto& get_any() const {
					return get_task()._result;
				};
			};

			template<typename ReturnType>
			class Task : public iTask {
				template<bool DispatchMultiple, typename ParentReturnType, typename StaticFunctionPointer_t, typename FunctionTraits> friend class TaskImpl;

			public:
				Task(unsigned long uuid) : iTask(uuid) {};
				Task() : iTask() {};
				Task(std::nullptr_t) : iTask(nullptr) {};
				Task(Task const& r) : iTask(r) {};
				Task(Task&& r) noexcept : iTask(std::forward<iTask>(r)) {
					r.m_uuid = 0;
				};
				Task& operator=(Task const& r) {
					dynamic_cast<iTask*>(this)->operator=(r);
					return this;
				};
				Task& operator=(Task&& r) noexcept {
					dynamic_cast<iTask*>(this)->operator=(std::forward<iTask>(r));
					return this;
				};
				virtual ~Task() = default;

				decltype(auto) get() const {
					return get_task()._result.cast<ReturnType>();
				};
				decltype(auto) get_ref() const {
					return get_task()._result.cast<ReturnType>();
				};
				decltype(auto) get_shared() const {
					return get_task()._result.cast<GL::shared_ptr<ReturnType>>();
				};

				template<bool DispatchMultiple, typename ParentReturnType, typename StaticFunctionPointer_t, typename FunctionTraits>
				class TaskImpl final : public Task {
				public:
					using traits = FunctionTraits;
					static void Callback(void* _args) {
						iTask this_task(reinterpret_cast<unsigned long>(_args)); // will decrement the count, and may even delete the shared job memory
					};
					static void DoTask(impl::job_argument const& _args) {						
						if constexpr (DispatchMultiple && use_cached_function_args) {
							if (_args.group_index == 0) {
								// Starting a group, we have a responsibility to cache the arguments. 
								auto& Base = id::get_id(reinterpret_cast<unsigned long>(_args.task_memory)).data;
								FunctionTraits::perform_caching(&Base._arguments[0], static_cast<char*>(_args.group_memory));		

								if (_args.job_index == 0) {
									if constexpr (std::is_void_v<typename FunctionTraits::returnType>)
										(void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory));
									else 
										Base._result = GL::any::instance(FunctionTraits::cached_call(static_cast<char*>(_args.group_memory)));									
								}
								else
									(void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory));
							}
							else 
								(void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory));												
						}
						else {
							auto& Base = id::get_id(reinterpret_cast<unsigned long>(_args.task_memory)).data;
							if (_args.job_index == 0) {
								if constexpr (std::is_void_v<typename FunctionTraits::returnType>) 
									(void)FunctionTraits::call(&Base._arguments[0]);
								else 
									Base._result = GL::any::instance(FunctionTraits::call(&Base._arguments[0]));
							}
							else 
								(void)FunctionTraits::call(&Base._arguments[0]);
						}
					};
					void Dispatch(TaskBase& Base) {
						if (auto Count = GL::interlocked::exchange(Base._count, (decltype(Base._count))0); Count > 0) {
							GL::interlocked::increment(id::get_id(dynamic_cast<iTask*>(this)->m_uuid).count);
							if constexpr (DispatchMultiple && use_cached_function_args)
								impl::Dispatch(Base._ctx, Count, &DoTask, reinterpret_cast<void*>(dynamic_cast<iTask*>(this)->m_uuid), FunctionTraits::cache_size_total,
									[](void* const& group_memory) -> void {
										// group memory start
									}, 
									[](void* const& group_memory) -> void {
										// group memory end
										FunctionTraits::undo_caching(static_cast<char*>(group_memory));
									}								
								);
							else
								impl::Dispatch(Base._ctx, Count, &DoTask, reinterpret_cast<void*>(dynamic_cast<iTask*>(this)->m_uuid));
						}
					}

					TaskImpl(size_t start, size_t end, StaticFunctionPointer_t todo, Task<ParentReturnType> parent)
						: Task(id::new_id())
					{
						auto& Base = dynamic_cast<Task*>(this)->get_task();
						Base._parent_uuid = parent.m_uuid;
						if (auto parent_uuid = Base._parent_uuid; ((m_uuid & id::TEMPORARY) == 0) && ((m_uuid & id::INV_FLAGS) > 0)) GL::interlocked::increment(id::get_id(parent_uuid).count);
						Base._function_pointer = static_cast<void*>(todo);
						FunctionTraits traits(reinterpret_cast<StaticFunctionPointer_t>(Base._function_pointer));
						Base._count = end - start;
						Base._offset = start;
						Base._ctx.callback = &Callback;
						Base._ctx.callback_data = reinterpret_cast<void*>(dynamic_cast<iTask*>(this)->m_uuid);
						// if (!busy()) Dispatch(Base);
					};

					Task AsTask() {
						Task out(dynamic_cast<Task*>(this)->m_uuid); // does not increment
						dynamic_cast<Task*>(this)->m_uuid = 0; // does not decrement
						return out;
					};

					template <std::size_t... Is, typename... Args>
					static void processPackWithIndices(TaskBase& Base, std::index_sequence<Is...>, Args&&... args) {
						auto processItem = [](TaskBase& Base, auto I, auto&& item) {
							// 1. Extract the compile-time index
							using from_t = std::decay_t<decltype(item)>;
							using to_t = std::tuple_element_t<decltype(I)::value, typename FunctionTraits::argType>;
							if constexpr (std::is_same_v<std::decay_t<from_t>, GL::any> || std::is_same_v<std::decay_t<from_t>, GL::atomic_any>) {
								// provided an "any", and therefore the type information is already lost.
								Base._arguments[decltype(I)::value] = std::forward<std::decay_t<decltype(item)>>(item);
							}
							else if constexpr (std::is_same_v<typename GL::type_erasure::get_type< from_t >::type, typename GL::type_erasure::get_type< to_t >::type>) {
								// the "any" casting should be able to handle this conversion automatically, since the underlying types match. 
								Base._arguments[decltype(I)::value] = GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_same_v<std::decay_t<to_t>, GL::any> || std::is_same_v<std::decay_t<to_t>, GL::atomic_any>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value] = GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_convertible_v<from_t, to_t>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value] = GL::any::instance((to_t)std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_convertible_v<from_t, typename GL::type_erasure::get_type< to_t >::type>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value] = GL::any::instance((typename GL::type_erasure::get_type< to_t >::type)std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_convertible_v<typename GL::type_erasure::get_type< from_t >::type, to_t>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value] = GL::any::instance((to_t)GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item)).cast<typename GL::type_erasure::get_type< from_t >::type>());
							}
							else {
								static_assert(false, "Was not able to handle the conversion from the provided types");
							}
						};
						((processItem(Base, std::integral_constant<std::size_t, Is>{}, std::forward<Args>(args))), ...);
					};
				};
			};

			template<typename Func, typename... Args> decltype(auto) do_task(Func const& func, Args&&...  args) {
				typedef decltype(GL::detail::function_signature(func)) function_header;
				if constexpr ((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function) { // function objects, e.g. auto x = [](){};
					using traits = std::conditional_t<false && use_cached_function_args, typename decltype(Caching_Static_Function_Caller(+func))::traits, typename decltype(Static_Function_Caller(+func))::traits>;
					auto out = Task<typename traits::returnType>::template TaskImpl<false, void, decltype(traits::m_attr), traits>(0ull, 1ull, +func, nullptr);
					auto& Base = out.get_task();
					decltype(out)::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					out.Dispatch(Base);
					return out.AsTask();
				}
				else {
					static_assert(false, "task must be a static function or non-capturing lambda");
				}
			};
			template<typename Func, typename... Args> decltype(auto) do_tasks(size_t start, size_t end, Func const& func, Args&&... args) {
				typedef decltype(GL::detail::function_signature(func)) function_header;
				if constexpr ((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function) { // function objects, e.g. auto x = [](){};
					using traits = std::conditional_t<true && use_cached_function_args, typename decltype(Caching_Static_Function_Caller(+func))::traits, typename decltype(Static_Function_Caller(+func))::traits>;
					auto out = Task<typename traits::returnType>::template TaskImpl<true, void, decltype(traits::m_attr), traits>(start, end, +func, nullptr);
					auto& Base = out.get_task();
					decltype(out)::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					out.Dispatch(Base);
					return out.AsTask();
				}
				else {
					static_assert(false, "task must be a static function or non-capturing lambda");
				}
			};
		};


	};


};