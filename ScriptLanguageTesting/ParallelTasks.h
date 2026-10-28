#pragma once
#include "Parallel.h"
#include "functions.h"

namespace GL {
	namespace parallel {
		namespace id {
			struct TaskBase {
				impl::dispatch_context
					_ctx; // waitable context for the job
				GL::atomic_any
					_result;
				void*
					_function_pointer;
				std::array<GL::any, 16>
					_arguments;
				size_t
					_count;
				size_t
					_offset;
				unsigned long
					_parent_uuid;
				bool
					_job_completed;
				GL::atomic_uint_stack
					_children;
				void(*_deferred_dispatch)(unsigned int, TaskBase&);
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
			class iTask {
				template<typename ReturnType> friend class Task;
			protected:
				unsigned long
					m_uuid;

			public:
				const unsigned long& id() const {
					return m_uuid;
				};

				id::TaskBase& get_task() const noexcept {
					if ((m_uuid & id::INV_FLAGS) > 0)
						return id::get_id(m_uuid).data;
					else {
						static id::TaskBase out;
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
					thread_local GL::atomic_uint_stack stack;
					unsigned int curr_uuid = m_uuid;
					bool downward = true;
					while ((curr_uuid & id::INV_FLAGS) > 0) {
						if (downward) {
							auto& Base = id::get_id(curr_uuid).data;
							if (!Base._job_completed && Base._function_pointer) {
								stack.push(curr_uuid);
								if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
									curr_uuid = Base._parent_uuid | id::TEMPORARY;
									continue;
								}
								else {
									do {
										impl::Wait(Base._ctx, true);
									} while (Base._function_pointer && !Base._job_completed);
								}
							}
							downward = false;
						}
						else {
							auto& Base = id::get_id(curr_uuid).data;
							if (!Base._job_completed && Base._function_pointer) {
								do {
									impl::Wait(Base._ctx, true);
								} while (Base._function_pointer && !Base._job_completed);
							}
						}
						if (!stack.try_pop(curr_uuid)) curr_uuid = 0;
					}

					//if ((m_uuid & id::INV_FLAGS) > 0) {
					//	auto& Base = id::get_id(m_uuid).data;
					//	if (!Base._job_completed) {
					//		if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
					//			iTask parentTask(Base._parent_uuid | id::TEMPORARY); // temporary does not invoke the shared counter
					//			parentTask.wait();
					//		}
					//		do {
					//			impl::Wait(Base._ctx, true);
					//		} while (Base._function_pointer && !Base._job_completed);
					//	}
					//}
				};
				bool busy() const {
					if ((m_uuid & id::INV_FLAGS) > 0) {
						auto& Base = id::get_id(m_uuid).data;
						if (!Base._job_completed) {
							//if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
							//	iTask parentTask(Base._parent_uuid | id::TEMPORARY); // temporary does not invoke the shared counter
							//	if (parentTask.busy()) return true;
							//}
							//if (Base._ctx.is_busy()) 
							return true;
						}
					}
					return false;
				};
				auto& get_any() const {
					auto& i = get_task();
					if (!i._job_completed) wait();
					return i._result;
				};
			};
			template<typename ReturnType> class Task : public iTask {
				template<bool DispatchMultiple, typename ParentReturnType, typename FunctionTraits, size_t numProvidedArgs> friend class TaskImpl;
				template<typename ReturnType2> friend class Task;

			public:
				Task(unsigned long uuid) : iTask(uuid) {};
				Task() : iTask() {};
				Task(std::nullptr_t) : iTask(nullptr) {};
				Task(Task const& rhs) : iTask(rhs.id()) { if (m_uuid > 0) GL::interlocked::increment(id::get_id(m_uuid).count); };
				Task(Task const& rhs, bool) : iTask(rhs.id() | id::TEMPORARY) {};
				Task(Task&& rhs) noexcept : iTask(GL::interlocked::exchange(const_cast<unsigned long&>(rhs.id()), 0ul)) {};
				//template <typename T> Task(Task<T>&& rhs) noexcept : iTask(GL::interlocked::exchange(const_cast<unsigned long&>(rhs.id()), 0ul)) {};

				Task const& operator=(Task const& rhs) {
					if ((this->m_uuid & id::TEMPORARY) == 0)
						if ((this->m_uuid & id::INV_FLAGS) > 0)
							if (GL::interlocked::decrement(id::get_id(this->m_uuid).count) == 0)
								id::free_id(this->m_uuid);

					this->m_uuid = rhs.m_uuid & id::INV_FLAGS;
					if (this->m_uuid > 0) GL::interlocked::increment(id::get_id(this->m_uuid).count);
					return *this;
				};
				Task const& operator=(Task&& rhs) noexcept {
					if ((this->m_uuid & id::TEMPORARY) == 0)
						if ((this->m_uuid & id::INV_FLAGS) > 0)
							if (GL::interlocked::decrement(id::get_id(this->m_uuid).count) == 0)
								id::free_id(this->m_uuid);

					m_uuid = GL::interlocked::exchange(const_cast<unsigned long&>(rhs.id()), 0ul);
					return *this;
				};
				virtual ~Task() = default;

				decltype(auto) get() const {
					if constexpr (!std::is_void_v<ReturnType>) {
						auto& i = get_task();
						if (!i._job_completed) wait();
						if constexpr (std::is_copy_constructible_v< ReturnType>) 
							return (ReturnType)i._result.cast<ReturnType>();							
						else 
							return i._result.cast<ReturnType>();							
					}
				};
				decltype(auto) get_ref() const {
					if constexpr (!std::is_void_v<ReturnType>) {
						auto& i = get_task();
						if (!i._job_completed) wait();
						return i._result.cast<ReturnType>();
					}
				};
				decltype(auto) get_ptr() const {
					if constexpr (!std::is_void_v<ReturnType>) {
						auto& i = get_task();
						if (!i._job_completed) wait();
						return i._result.cast<ReturnType*>();
					}
				};
				decltype(auto) get_shared() const {
					if constexpr (!std::is_void_v<ReturnType>) {
						auto& i = get_task();
						if (!i._job_completed) wait();
						return i._result.cast<GL::shared_ptr<ReturnType>>();
					}
				};

			protected:
				template<bool DispatchMultiple, typename ParentReturnType, typename FunctionTraits, size_t numProvidedArgs>
				class TaskImpl final : public Task {
				public:
					template<bool DispatchMultiple2, typename ParentReturnType2, typename FunctionTraits2, size_t numProvidedArgs2> friend class TaskImpl;

					constexpr static bool dispatch_multiple = DispatchMultiple;
					using traits = FunctionTraits;
					static void Callback(void* _args) {
						iTask this_task(static_cast<unsigned long>(reinterpret_cast<uintptr_t>(_args))); // will decrement the count, and may even delete the shared job memory
						auto& Base = this_task.get_task();
						Base._job_completed = true;

						// free reference to parent
						if ((Base._parent_uuid & id::TEMPORARY) == 0)
							if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
								if (GL::interlocked::decrement(id::get_id(Base._parent_uuid).count) == 0)
									id::free_id(Base._parent_uuid);
								Base._parent_uuid = 0;
							}

						unsigned int child_uuid;
						while (Base._children.try_pop(child_uuid)) {
							iTask temp((unsigned long)child_uuid);
							if (auto& child_base = id::get_id(child_uuid).data; child_base._deferred_dispatch)
								child_base._deferred_dispatch(child_uuid, child_base);
						}
					};
					static void DoTask(impl::job_argument& _args) {
						if (_args.group_index == 0) {
							if constexpr (!std::is_void_v<ParentReturnType>)
								if ((static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid & id::INV_FLAGS) > 0)
									impl::Wait(id::get_id(static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid).data._ctx, true);

							// Starting a group, we have a responsibility to cache the arguments. 							
							if constexpr (numProvidedArgs >= FunctionTraits::numArgs)
								FunctionTraits::perform_caching<0>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
							else if constexpr (std::is_void_v<ParentReturnType>) { // no parent...								
								if constexpr (DispatchMultiple) // ...but potentially uses an index at the 0th index									
									FunctionTraits::perform_caching<0>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
							}
							else {
								// has a parent...
								if constexpr (DispatchMultiple) {
									// ...and potentially uses an index at the 0th index, and the parent's results at the 1st index.
									FunctionTraits::perform_caching<2>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
									FunctionTraits::supplement_caching<0>(static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
									FunctionTraits::supplement_caching<1>(id::get_id(static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid).data._result, static_cast<char*>(_args.group_memory));
								}
								else {
									// ...but no index. Try to use the parent's value at the 0th index.
									FunctionTraits::perform_caching<1>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
									FunctionTraits::supplement_caching<0>(id::get_id(static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid).data._result, static_cast<char*>(_args.group_memory));
								}
							}

							// update the index, if this is a DispatchMultiple job
							if constexpr (DispatchMultiple) {
								if constexpr (numProvidedArgs > 16) {}
								else if constexpr (numProvidedArgs >= FunctionTraits::numArgs) {}
								else static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0].cast<std::decay_t<std::tuple_element_t<0, typename FunctionTraits::argType>>>() = _args.job_index;
							}

							// perform the call using the cached arguments
							if (_args.job_index == 0) {
								if constexpr (std::is_void_v<typename FunctionTraits::returnType>)
									(void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
								else {
									if constexpr (std::is_same_v<std::decay_t<typename FunctionTraits::returnType>, GL::any>
										|| std::is_same_v<std::decay_t<typename FunctionTraits::returnType>, GL::atomic_any>
										) static_cast<id::TaskBase*>(_args.task_memory)->_result = FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
									else static_cast<id::TaskBase*>(_args.task_memory)->_result = GL::any::instance(FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer));
								}
							}
							else (void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
						}
						else {
							if constexpr (numProvidedArgs > 16) {}
							else if constexpr (numProvidedArgs >= FunctionTraits::numArgs) {}
							else if constexpr (std::is_void_v<ParentReturnType>) // no parent, but potentially uses an index at the 0th index
								static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0].cast<std::decay_t<std::tuple_element_t<0, typename FunctionTraits::argType>>>() = _args.job_index;
							else if constexpr (DispatchMultiple) // has a parent and potentially uses an index at the 0th index, and the parent's results at the 1st index.									
								static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0].cast<std::decay_t<std::tuple_element_t<0, typename FunctionTraits::argType>>>() = _args.job_index;
							(void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
						}
					};
					void Dispatch(id::TaskBase& Base) {
						if (auto Count = GL::interlocked::exchange(Base._count, (decltype(Base._count))0); Count > 0) {
							auto& ticket = id::get_id(dynamic_cast<iTask*>(this)->m_uuid);			
							GL::interlocked::increment(ticket.count);
							impl::Dispatch(Base._ctx, Count, &DoTask, reinterpret_cast<void*>(&ticket.data), FunctionTraits::cache_size_total,
								nullptr,
								!FunctionTraits::requires_undo_caching ? nullptr : +[](void* const& group_memory) -> void {
									// group memory end
									FunctionTraits::undo_caching(static_cast<char*>(group_memory));
								}
							);
						}
					}
					static void DeferredDispatch(unsigned int uuid, id::TaskBase& Base) {
						iTask temp(uuid | id::TEMPORARY);
						reinterpret_cast<TaskImpl*>(&temp)->Dispatch(Base);
					};

					TaskImpl(size_t start, size_t end, decltype(FunctionTraits::m_attr) todo, Task<ParentReturnType> parent)
						: Task(id::new_id())
					{
						auto& Base = dynamic_cast<Task*>(this)->get_task();
						Base._parent_uuid = parent.m_uuid;
						if (auto parent_uuid = Base._parent_uuid; ((parent_uuid & id::TEMPORARY) == 0) && ((parent_uuid & id::INV_FLAGS) > 0)) GL::interlocked::increment(id::get_id(parent_uuid).count);
						Base._function_pointer = static_cast<void*>(todo);
						Base._count = end - start;
						Base._offset = start;
						Base._ctx.callback = &Callback;
						Base._ctx.callback_data = reinterpret_cast<void*>(static_cast<uintptr_t>(dynamic_cast<iTask*>(this)->m_uuid));
					};
					TaskImpl(TaskImpl const&) = delete;
					TaskImpl(TaskImpl&&) noexcept = delete;
					TaskImpl& operator=(TaskImpl const&) = delete;
					TaskImpl& operator=(TaskImpl&&) noexcept = delete;
					virtual ~TaskImpl() = default;

					Task AsTask() {
						return Task(GL::interlocked::exchange(const_cast<unsigned long&>(this->id()), 0ul)); // does not increment or decrement
					};

					template <size_t offset>
					struct wrapper {
						template <std::size_t... Is, typename... Args>
						static void processPackWithIndices(id::TaskBase& Base, std::index_sequence<Is...>, Args&&... args) {
							auto processItem = [](id::TaskBase& Base, auto I, auto&& item) {
								using from_t = std::decay_t<decltype(item)>;
								using to_t = std::tuple_element_t<decltype(I)::value + offset, typename FunctionTraits::argType>;

								if constexpr (std::is_same_v<std::decay_t<from_t>, GL::any> || std::is_same_v<std::decay_t<from_t>, GL::atomic_any>) {
									// provided an "any", and therefore the type information is already lost.
									Base._arguments[decltype(I)::value + offset] = std::forward<std::decay_t<from_t>>(item);
								}
								else if constexpr (std::is_same_v<typename GL::type_erasure::get_type< from_t >::type, typename GL::type_erasure::get_type< to_t >::type>) {
									// the "any" casting should be able to handle this conversion automatically, since the underlying types match. 
									Base._arguments[decltype(I)::value + offset] = GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item));
								}
								else if constexpr (std::is_same_v<std::decay_t<to_t>, GL::any> || std::is_same_v<std::decay_t<to_t>, GL::atomic_any>) {
									// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
									Base._arguments[decltype(I)::value + offset] = GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item));
								}
								else if constexpr (std::is_convertible_v<from_t, to_t>) {
									// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
									Base._arguments[decltype(I)::value + offset] = GL::any::instance((to_t)std::forward<std::decay_t<decltype(item)>>(item));
								}
								else if constexpr (std::is_convertible_v<from_t, typename GL::type_erasure::get_type< to_t >::type>) {
									// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
									Base._arguments[decltype(I)::value + offset] = GL::any::instance((typename GL::type_erasure::get_type< to_t >::type)std::forward<std::decay_t<decltype(item)>>(item));
								}
								else if constexpr (std::is_convertible_v<typename GL::type_erasure::get_type< from_t >::type, to_t>) {
									// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
									Base._arguments[decltype(I)::value + offset] = GL::any::instance((to_t)GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item)).cast<typename GL::type_erasure::get_type< from_t >::type>());
								}
								else {
									static_assert(false, "Was not able to handle the conversion from the provided types");
								}
							};
							((processItem(Base, std::integral_constant<std::size_t, Is>{}, std::forward<Args>(args))), ...);
						};
					};
				};
				template<typename Func, typename, typename... Args> friend decltype(auto) async(Func const& func, Args&&...  args);
				template<typename Func, typename... Args> friend decltype(auto) async(size_t start, size_t end, Func const& func, Args&&...  args);

			public:
				template<typename Func, typename... Args> decltype(auto) then(size_t start, size_t end, Func const& func, Args&&... args) const {
					typedef decltype(GL::detail::function_signature(func)) function_header;
					static_assert((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function, "task must be a static function or non-capturing lambda");

					using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
					auto out{ Task<typename traits::returnType>::template TaskImpl<true, ReturnType, traits, sizeof...(Args)>(start, end, +func, *this) };

					auto out_id = out.id();
					auto& out_ticket = id::get_id(out_id);
					auto& Base = out_ticket.data;

					if constexpr (sizeof...(Args) > 16) {
						static_assert(false, "The async system cannot process a function with too many arguments");
					}
					else if constexpr (sizeof...(Args) >= traits::numArgs) {
						// user provided all of the arguments.
						decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else if constexpr (std::is_void_v<ReturnType>) {
						// no parent...
						if constexpr (decltype(out)::dispatch_multiple) {
							// ...but potentially uses an index.
							static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index");
							static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
							static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
							Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
							decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
						}
						else {
							// ...and no index.
							static_assert(false, "The incorrect number of function arguments were passed to the async task");
						}
					}
					else {
						// has a parent...
						if constexpr (decltype(out)::dispatch_multiple) {
							// ...and potentially uses an index.
							static_assert((sizeof...(Args) + 2) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index and a return type from the parent task");
							static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
							static_assert(
								std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
								|| std::is_convertible_v<ReturnType&, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<ReturnType*, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
								|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
								, "The second argument was assumed to be a return type from the parent task, but was unable to be converted");
							static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
							Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
							Base._arguments[1] = nullptr;
							decltype(out)::template wrapper<2>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
						}
						else {
							// ...but no index.
							static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a return type from the parent task");
							static_assert(
								std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
								|| std::is_convertible_v<ReturnType&, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<ReturnType*, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
								|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
								, "The first argument was assumed to be a return type from the parent task, but was unable to be converted");
							Base._arguments[0] = nullptr;
							decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
						}
					}

					Base._deferred_dispatch = &decltype(out)::DeferredDispatch;
					//GL::interlocked::increment(out_ticket.count); // its own lifespan, which may be decremented when the task goes out-of-scope
					GL::interlocked::increment(out_ticket.count); // child lifespan
					auto& thisTask = this->get_task();
					thisTask._children.push(out_id);
					if (thisTask._job_completed) {
						unsigned int child_uuid;
						while (thisTask._children.try_pop(child_uuid)) {
							iTask temp((unsigned long)child_uuid);
							if (auto& child_base = id::get_id(child_uuid).data; child_base._deferred_dispatch)
								child_base._deferred_dispatch(child_uuid, child_base);
						}
					}
					// out.Dispatch(Base);
					return out.AsTask();
				};
				template<typename Func, typename = std::enable_if_t<std::is_empty_v<Func>>, typename... Args> decltype(auto) then(Func const& func, Args&&...  args) const {
					typedef decltype(GL::detail::function_signature(func)) function_header;
					static_assert((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function, "task must be a static function or non-capturing lambda");

					using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
					auto out{ Task<typename traits::returnType>::template TaskImpl<false, ReturnType, traits, sizeof...(Args)>(0, 1, +func, *this) };

					auto out_id = out.id();
					auto& out_ticket = id::get_id(out_id);
					auto& Base = out_ticket.data;

					if constexpr (sizeof...(Args) > 16) {
						static_assert(false, "The async system cannot process a function with too many arguments");
					}
					else if constexpr (sizeof...(Args) >= traits::numArgs) {
						// user provided all of the arguments.
						decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else if constexpr (std::is_void_v<ReturnType>) {
						// no parent...
						if constexpr (decltype(out)::dispatch_multiple) {
							// ...but potentially uses an index.
							static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index");
							static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
							static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
							Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
							decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
						}
						else {
							// ...and no index.
							static_assert(false, "The incorrect number of function arguments were passed to the async task");
						}
					}
					else {
						// has a parent...
						if constexpr (decltype(out)::dispatch_multiple) {
							// ...and potentially uses an index.
							static_assert((sizeof...(Args) + 2) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index and a return type from the parent task");
							static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
							static_assert(
								std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
								|| std::is_convertible_v<ReturnType&, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<ReturnType*, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
								|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
								|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
								, "The second argument was assumed to be a return type from the parent task, but was unable to be converted");
							static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
							Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
							Base._arguments[1] = nullptr;
							decltype(out)::template wrapper<2>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
						}
						else {
							// ...but no index.
							static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a return type from the parent task");
							static_assert(
								std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
								|| std::is_convertible_v<ReturnType&, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<ReturnType*, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
								|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
								|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
								, "The first argument was assumed to be a return type from the parent task, but was unable to be converted");
							Base._arguments[0] = nullptr;
							decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
						}
					}

					Base._deferred_dispatch = &decltype(out)::DeferredDispatch;
					//GL::interlocked::increment(out_ticket.count); // its own lifespan, which may be decremented when the task goes out-of-scope
					GL::interlocked::increment(out_ticket.count); // child lifespan
					auto& thisTask = this->get_task();
					thisTask._children.push(out_id);
					if (thisTask._job_completed) {
						unsigned int child_uuid;
						while (thisTask._children.try_pop(child_uuid)) {
							iTask temp((unsigned long)child_uuid);
							if (auto& child_base = id::get_id(child_uuid).data; child_base._deferred_dispatch)
								child_base._deferred_dispatch(child_uuid, child_base);
						}
					}
					// out.Dispatch(Base);
					return out.AsTask();
				};

			};
			// instance of an async task that can be waited on.
			template<typename Func, typename = std::enable_if_t<std::is_empty_v<Func>>, typename... Args> decltype(auto) async(Func const& func, Args&&...  args) {
				typedef decltype(GL::detail::function_signature(func)) function_header;
				if constexpr ((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function) { // function objects, e.g. auto x = [](){};
					using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
					auto out = Task<typename traits::returnType>::template TaskImpl<false, void, traits, sizeof...(Args)>(0ull, 1ull, +func, nullptr);
					auto out_id = out.id();
					auto& out_ticket = id::get_id(out_id);
					auto& Base = out_ticket.data;
					if constexpr (sizeof...(Args) > 16) {
						static_assert(false, "The async system cannot process a function with too many arguments");
					}
					else if constexpr (sizeof...(Args) >= traits::numArgs) {
						// user provided all of the arguments.
						decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else {
						// no parent and no index.
						static_assert(false, "The incorrect number of function arguments were passed to the async task");
					}
					//GL::interlocked::increment(out_ticket.count);
					out.Dispatch(Base);
					return out.AsTask();
				}
				else {
					static_assert(false, "task must be a static function or non-capturing lambda");
				}
			};
			// instance of an async task that can be waited on.
			template<typename Func, typename... Args> decltype(auto) async(size_t start, size_t end, Func const& func, Args&&... args) {
				typedef decltype(GL::detail::function_signature(func)) function_header;
				if constexpr ((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function) { // function objects, e.g. auto x = [](){};
					using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
					auto out = Task<typename traits::returnType>::template TaskImpl<true, void, traits, sizeof...(Args)>(start, end, +func, nullptr);
					auto out_id = out.id();
					auto& out_ticket = id::get_id(out_id);
					auto& Base = out_ticket.data;
					if constexpr (sizeof...(Args) > 16) {
						static_assert(false, "The async system cannot process a function with too many arguments");
					}
					else if constexpr (sizeof...(Args) >= traits::numArgs) {
						// user provided all of the arguments.
						decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else {
						// no parent but potentially uses an index.
						static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index");
						static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
						static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
						Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
						decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					//GL::interlocked::increment(out_ticket.count);
					out.Dispatch(Base);
					return out.AsTask();
				}
				else {
					static_assert(false, "task must be a static function or non-capturing lambda");
				}
			};

			//template<typename L, typename R> decltype(auto) both(Task<L> const& lhs, Task<R> const& rhs) {
			//	auto& rhs_ticket = id::get_id(rhs.id());
			//	GL::interlocked::increment(rhs_ticket.count);
			//	auto intermediate_task = lhs.then([](GL::any lhs_result, unsigned long rhs_uuid) {
			//		// the LHS task has completed at this point.
			//		Task<R> rhs(rhs_uuid); // will decrement it
			//		return rhs.then([](GL::any rhs_result, GL::any lhs_result) {
			//			if constexpr (std::is_void_v<L>) {
			//				if constexpr (std::is_void_v<R>) return;							
			//				else return rhs_result;							
			//			}
			//			else if constexpr (std::is_void_v<R>) return lhs_result;
			//			else return std::pair<GL::any, GL::any>(lhs_result, rhs_result);
			//		}, (GL::any)lhs_result);
			//	}, (unsigned long)rhs.id());
			//	if constexpr (std::is_void_v<L>) {
			//		if constexpr (std::is_void_v<R>) {
			//			return intermediate_task.then([](Task<void> const& intermediate) {});
			//		}
			//		else {
			//			return Task<R>(intermediate_task.then([](Task<R> const& intermediate) -> GL::any {
			//				return intermediate.get_any();
			//			}));
			//		}
			//	}
			//	else {
			//		if constexpr (std::is_void_v<R>) {
			//			return Task<L>(intermediate_task.then([](Task<L> const& intermediate) -> GL::any {
			//				return intermediate.get_any();
			//			}));
			//		}
			//		else {
			//			return Task<std::pair<GL::any, GL::any>>(intermediate_task.then([](Task<std::pair<GL::any, GL::any>> const& intermediate) -> GL::any {
			//				return intermediate.get_any();
			//			}));
			//		}
			//	}
			//};
        }

		class iTask {
			template<typename ReturnType> friend class Task;
		protected:
			unsigned long
				m_uuid;

		public:
			const unsigned long& id() const {
				return m_uuid;
			};

			id::TaskBase& get_task() const noexcept {
				if ((m_uuid & id::INV_FLAGS) > 0)
					return id::get_id(m_uuid).data;
				else {
					static id::TaskBase out;
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
				thread_local GL::atomic_uint_stack stack;
				unsigned int curr_uuid = m_uuid;
				bool downward = true;
				while ((curr_uuid & id::INV_FLAGS) > 0) {
					if (downward) {
						auto& Base = id::get_id(curr_uuid).data;
						if (!Base._job_completed && Base._function_pointer) {
							stack.push(curr_uuid);
							if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
								curr_uuid = Base._parent_uuid | id::TEMPORARY;
								continue;
							}
							else {
								do {
									impl::Wait(Base._ctx, true);
								} while (Base._function_pointer && !Base._job_completed);
							}
						}
						downward = false;
					}
					else {
						auto& Base = id::get_id(curr_uuid).data;
						if (!Base._job_completed && Base._function_pointer) {
							do {
								impl::Wait(Base._ctx, true);
							} while (Base._function_pointer && !Base._job_completed);
						}
					}
					if (!stack.try_pop(curr_uuid)) curr_uuid = 0;
				}

				//if ((m_uuid & id::INV_FLAGS) > 0) {
				//	auto& Base = id::get_id(m_uuid).data;
				//	if (!Base._job_completed) {
				//		if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
				//			iTask parentTask(Base._parent_uuid | id::TEMPORARY); // temporary does not invoke the shared counter
				//			parentTask.wait();
				//		}	
				//		do {
				//			impl::Wait(Base._ctx, true);
				//		} while (Base._function_pointer && !Base._job_completed);
				//	}
				//}
			};
			bool busy() const {
				if ((m_uuid & id::INV_FLAGS) > 0) {
					auto& Base = id::get_id(m_uuid).data;
					if (!Base._job_completed) {
						//if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
						//	iTask parentTask(Base._parent_uuid | id::TEMPORARY); // temporary does not invoke the shared counter
						//	if (parentTask.busy()) return true;
						//}
						//if (Base._ctx.is_busy()) 
						return true;
					}
				}
				return false;
			};
			auto& get_any() const {
				auto& i = get_task();
				if (!i._job_completed) wait();
				return i._result;
			};
		};
		template<typename ReturnType> class Task : public iTask {
			template<bool DispatchMultiple, typename ParentReturnType, typename FunctionTraits, size_t numProvidedArgs> friend class TaskImpl;
			template<typename ReturnType2> friend class Task;

		public:
			Task(unsigned long uuid) : iTask(uuid) {};
			Task() : iTask() {};
			Task(std::nullptr_t) : iTask(nullptr) {};
			Task(Task const& rhs) : iTask(rhs.id()) {
				if (m_uuid > 0) GL::interlocked::increment(id::get_id(m_uuid).count);
			};
			Task(Task const& rhs, bool) : iTask(rhs.id() | id::TEMPORARY) {};
			Task(Task&& rhs) noexcept : iTask(GL::interlocked::exchange(const_cast<unsigned long&>(rhs.id()), 0ul)) {};
			// template <typename T> Task(Task<T>&& rhs) noexcept : iTask(GL::interlocked::exchange(const_cast<unsigned long&>(rhs.id()), 0ul)) {};
			Task const& operator=(Task const& rhs) {
				if ((this->m_uuid & id::TEMPORARY) == 0)
					if ((this->m_uuid & id::INV_FLAGS) > 0)
						if (GL::interlocked::decrement(id::get_id(this->m_uuid).count) == 0)
							id::free_id(this->m_uuid);

				this->m_uuid = rhs.m_uuid & id::INV_FLAGS;
				if (this->m_uuid > 0) GL::interlocked::increment(id::get_id(this->m_uuid).count);
				return *this;
			};
			Task const& operator=(Task&& rhs) noexcept {
				if ((this->m_uuid & id::TEMPORARY) == 0)
					if ((this->m_uuid & id::INV_FLAGS) > 0)
						if (GL::interlocked::decrement(id::get_id(this->m_uuid).count) == 0)
							id::free_id(this->m_uuid);

				m_uuid = GL::interlocked::exchange(const_cast<unsigned long&>(rhs.id()), 0ul);
				return *this;
			};
			virtual ~Task() = default;

			decltype(auto) get() const {
				if constexpr (!std::is_void_v<ReturnType>) {
					auto& i = get_task();
					if (!i._job_completed) wait();
					if constexpr (std::is_copy_constructible_v< ReturnType>)
						return (ReturnType)i._result.cast<ReturnType>();
					else
						return i._result.cast<ReturnType>();
				}
			};
			decltype(auto) get_ref() const {
				if constexpr (!std::is_void_v<ReturnType>) {
					auto& i = get_task();
					if (!i._job_completed) wait();
					return i._result.cast<ReturnType>();
				}
			};
			decltype(auto) get_ptr() const {
				if constexpr (!std::is_void_v<ReturnType>) {
					auto& i = get_task();
					if (!i._job_completed) wait();
					return i._result.cast<ReturnType*>();
				}
			};
			decltype(auto) get_shared() const {
				if constexpr (!std::is_void_v<ReturnType>) {
					auto& i = get_task();
					if (!i._job_completed) wait();
					return i._result.cast<GL::shared_ptr<ReturnType>>();
				}
			};

		protected:
			template<bool DispatchMultiple, typename ParentReturnType, typename FunctionTraits, size_t numProvidedArgs>
			class TaskImpl final : public Task {
			public:
				template<bool DispatchMultiple2, typename ParentReturnType2, typename FunctionTraits2, size_t numProvidedArgs2> friend class TaskImpl;

				constexpr static bool dispatch_multiple = DispatchMultiple;
				using traits = FunctionTraits;
				static void Callback(void* _args) {
					iTask this_task(static_cast<unsigned long>(reinterpret_cast<uintptr_t>(_args))); // will decrement the count, and may even delete the shared job memory
					auto& Base = this_task.get_task();
					Base._job_completed = true;

					// free reference to parent
					if ((Base._parent_uuid & id::TEMPORARY) == 0)
						if ((Base._parent_uuid & id::INV_FLAGS) > 0) {
							if (GL::interlocked::decrement(id::get_id(Base._parent_uuid).count) == 0)
								id::free_id(Base._parent_uuid);
							Base._parent_uuid = 0;
						}

					unsigned int child_uuid;
					while (Base._children.try_pop(child_uuid)) {
						iTask temp((unsigned long)child_uuid);
						auto& child_base = id::get_id(child_uuid).data;
						if (child_base._deferred_dispatch)
							child_base._deferred_dispatch(child_uuid, child_base);
					}
				};
				static void DoTask(impl::job_argument& _args) {
					if (_args.group_index == 0) {
						if constexpr (!std::is_void_v<ParentReturnType>)
							if ((static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid & id::INV_FLAGS) > 0)
								impl::Wait(id::get_id(static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid).data._ctx, true);

						// Starting a group, we have a responsibility to cache the arguments. 							
						if constexpr (numProvidedArgs >= FunctionTraits::numArgs)
							FunctionTraits::perform_caching<0>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
						else if constexpr (std::is_void_v<ParentReturnType>) { // no parent...								
							if constexpr (dispatch_multiple) // ...but potentially uses an index at the 0th index									
								FunctionTraits::perform_caching<0>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
						}
						else {
							// has a parent...
							if constexpr (dispatch_multiple) {
								// ...and potentially uses an index at the 0th index, and the parent's results at the 1st index.
								FunctionTraits::perform_caching<2>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
								FunctionTraits::supplement_caching<0>(static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
								FunctionTraits::supplement_caching<1>(id::get_id(static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid).data._result, static_cast<char*>(_args.group_memory));
							}
							else {
								// ...but no index. Try to use the parent's value at the 0th index.
								FunctionTraits::perform_caching<1>(&static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0], static_cast<char*>(_args.group_memory));
								FunctionTraits::supplement_caching<0>(id::get_id(static_cast<id::TaskBase*>(_args.task_memory)->_parent_uuid).data._result, static_cast<char*>(_args.group_memory));
							}
						}

						// update the index, if this is a dispatch_multiple job
						if constexpr (dispatch_multiple) {
							if constexpr (numProvidedArgs > 16) {}
							else if constexpr (numProvidedArgs >= FunctionTraits::numArgs) {}
							else static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0].cast<std::decay_t<std::tuple_element_t<0, typename FunctionTraits::argType>>>() = _args.job_index;
						}

						// perform the call using the cached arguments
						if (_args.job_index == 0) {
							if constexpr (std::is_void_v<typename FunctionTraits::returnType>)
								(void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
							else {
								if constexpr (std::is_same_v<std::decay_t<typename FunctionTraits::returnType>, GL::any>
									|| std::is_same_v<std::decay_t<typename FunctionTraits::returnType>, GL::atomic_any>
									) static_cast<id::TaskBase*>(_args.task_memory)->_result = FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
								else static_cast<id::TaskBase*>(_args.task_memory)->_result = GL::any::instance(FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer));
							}
						}
						else (void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
					}
					else {
						if constexpr (numProvidedArgs > 16) {}
						else if constexpr (numProvidedArgs >= FunctionTraits::numArgs) {}
						else if constexpr (std::is_void_v<ParentReturnType>) // no parent, but potentially uses an index at the 0th index
							static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0].cast<std::decay_t<std::tuple_element_t<0, typename FunctionTraits::argType>>>() = _args.job_index;
						else if constexpr (dispatch_multiple) // has a parent and potentially uses an index at the 0th index, and the parent's results at the 1st index.									
							static_cast<id::TaskBase*>(_args.task_memory)->_arguments[0].cast<std::decay_t<std::tuple_element_t<0, typename FunctionTraits::argType>>>() = _args.job_index;
						(void)FunctionTraits::cached_call(static_cast<char*>(_args.group_memory), (decltype(FunctionTraits::m_attr))static_cast<id::TaskBase*>(_args.task_memory)->_function_pointer);
					}
				};
				void Dispatch(id::TaskBase& Base) {
					if (auto Count = GL::interlocked::exchange(Base._count, (decltype(Base._count))0); Count > 0) {
						auto& ticket = id::get_id(dynamic_cast<iTask*>(this)->m_uuid);
						GL::interlocked::increment(ticket.count);
						impl::Dispatch(Base._ctx, Count, &DoTask, reinterpret_cast<void*>(&ticket.data), FunctionTraits::cache_size_total,
							nullptr,
							!FunctionTraits::requires_undo_caching ? nullptr : +[](void* const& group_memory) -> void {
								// group memory end
								FunctionTraits::undo_caching(static_cast<char*>(group_memory));
							}
						);
					}
				}
				static void DeferredDispatch(unsigned int uuid, id::TaskBase& Base) {
					iTask temp(uuid | id::TEMPORARY);
					reinterpret_cast<TaskImpl*>(&temp)->Dispatch(Base);
				};

				TaskImpl(size_t start, size_t end, decltype(FunctionTraits::m_attr) todo, Task<ParentReturnType> parent)
					: Task(id::new_id())
				{
					auto& Base = dynamic_cast<Task*>(this)->get_task();
					Base._parent_uuid = parent.m_uuid;
					if (auto parent_uuid = Base._parent_uuid; ((parent_uuid & id::TEMPORARY) == 0) && ((parent_uuid & id::INV_FLAGS) > 0)) GL::interlocked::increment(id::get_id(parent_uuid).count);
					Base._function_pointer = static_cast<void*>(todo);
					//FunctionTraits traits(reinterpret_cast<decltype(FunctionTraits::m_attr)>(Base._function_pointer));
					Base._count = end - start;
					Base._offset = start;
					Base._ctx.callback = &Callback;
					Base._ctx.callback_data = reinterpret_cast<void*>(static_cast<uintptr_t>(dynamic_cast<iTask*>(this)->m_uuid));
					// if (!busy()) Dispatch(Base);
				};
				TaskImpl(TaskImpl const&) = delete;
				TaskImpl(TaskImpl&&) noexcept = delete;
				TaskImpl& operator=(TaskImpl const&) = delete;
				TaskImpl& operator=(TaskImpl&&) noexcept = delete;
				virtual ~TaskImpl() = default;

				Task AsTask() {
					return Task(GL::interlocked::exchange(const_cast<unsigned long&>(this->id()), 0ul)); // does not increment or decrement
				};

				template <size_t offset>
				struct wrapper {
					template <std::size_t... Is, typename... Args>
					static void processPackWithIndices(id::TaskBase& Base, std::index_sequence<Is...>, Args&&... args) {
						auto processItem = [](id::TaskBase& Base, auto I, auto&& item) {
							using from_t = std::decay_t<decltype(item)>;
							using to_t = std::tuple_element_t<decltype(I)::value + offset, typename FunctionTraits::argType>;

							if constexpr (std::is_same_v<std::decay_t<from_t>, GL::any> || std::is_same_v<std::decay_t<from_t>, GL::atomic_any>) {
								// provided an "any", and therefore the type information is already lost.
								Base._arguments[decltype(I)::value + offset] = std::forward<std::decay_t<decltype(item)>>(item);
							}
							else if constexpr (std::is_same_v<typename GL::type_erasure::get_type< from_t >::type, typename GL::type_erasure::get_type< to_t >::type>) {
								// the "any" casting should be able to handle this conversion automatically, since the underlying types match. 
								Base._arguments[decltype(I)::value + offset] = GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_same_v<std::decay_t<to_t>, GL::any> || std::is_same_v<std::decay_t<to_t>, GL::atomic_any>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value + offset] = GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_convertible_v<from_t, to_t>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value + offset] = GL::any::instance((to_t)std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_convertible_v<from_t, typename GL::type_erasure::get_type< to_t >::type>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value + offset] = GL::any::instance((typename GL::type_erasure::get_type< to_t >::type)std::forward<std::decay_t<decltype(item)>>(item));
							}
							else if constexpr (std::is_convertible_v<typename GL::type_erasure::get_type< from_t >::type, to_t>) {
								// requesting an "any", and therefore the type information will be thrown away. Accept whatever we got.
								Base._arguments[decltype(I)::value + offset] = GL::any::instance((to_t)GL::any::instance(std::forward<std::decay_t<decltype(item)>>(item)).cast<typename GL::type_erasure::get_type< from_t >::type>());
							}
							else {
								static_assert(false, "Was not able to handle the conversion from the provided types");
							}
						};
						((processItem(Base, std::integral_constant<std::size_t, Is>{}, std::forward<Args>(args))), ...);
					};
				};
			};
			template<typename Func, typename, typename... Args> friend decltype(auto) async(Func const& func, Args&&...  args);
			template<typename Func, typename... Args> friend decltype(auto) async(size_t start, size_t end, Func const& func, Args&&...  args);

		public:
			template<typename Func, typename... Args> decltype(auto) then(size_t start, size_t end, Func const& func, Args&&... args) {
				typedef decltype(GL::detail::function_signature(func)) function_header;
				static_assert((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function, "task must be a static function or non-capturing lambda");

				using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
				auto out = Task<typename traits::returnType>::template TaskImpl<true, ReturnType, traits, sizeof...(Args)>(start, end, +func, *this);

				auto out_id = out.id();
				auto& out_ticket = id::get_id(out_id);
				auto& Base = out_ticket.data;

				if constexpr (sizeof...(Args) > 16) {
					static_assert(false, "The async system cannot process a function with too many arguments");
				}
				else if constexpr (sizeof...(Args) >= traits::numArgs) {
					// user provided all of the arguments.
					decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
				}
				else if constexpr (std::is_void_v<ReturnType>) {
					// no parent...
					if constexpr (decltype(out)::dispatch_multiple) {
						// ...but potentially uses an index.
						static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index");
						static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
						static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
						Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
						decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else {
						// ...and no index.
						static_assert(false, "The incorrect number of function arguments were passed to the async task");
					}
				}
				else {
					// has a parent...
					if constexpr (decltype(out)::dispatch_multiple) {
						// ...and potentially uses an index.
						static_assert((sizeof...(Args) + 2) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index and a return type from the parent task");
						static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
						static_assert(
							std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
							|| std::is_convertible_v<ReturnType&, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<ReturnType*, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
							|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
							, "The second argument was assumed to be a return type from the parent task, but was unable to be converted");
						static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
						Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
						Base._arguments[1] = nullptr;
						decltype(out)::template wrapper<2>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else {
						// ...but no index.
						static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a return type from the parent task");
						static_assert(
							std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
							|| std::is_convertible_v<ReturnType&, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<ReturnType*, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
							|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
							, "The first argument was assumed to be a return type from the parent task, but was unable to be converted");
						Base._arguments[0] = nullptr;
						decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
				}

				Base._deferred_dispatch = &decltype(out)::DeferredDispatch;
				GL::interlocked::increment(out_ticket.count); // its own lifespan, which may be decremented when the task goes out-of-scope
				//GL::interlocked::increment(out_ticket.count); // child lifespan
				auto& thisTask = this->get_task();
				thisTask._children.push(out_id);
				if (thisTask._job_completed) {
					unsigned int child_uuid;
					while (thisTask._children.try_pop(child_uuid)) {
						iTask temp((unsigned long)child_uuid);
						if (auto& child_base = id::get_id(child_uuid).data; child_base._deferred_dispatch)
							child_base._deferred_dispatch(child_uuid, child_base);
					}
				}
				//out.Dispatch(Base);
				return out.AsTask();
			};
			template<typename Func, typename = std::enable_if_t<std::is_empty_v<Func>>, typename... Args> decltype(auto) then(Func const& func, Args&&...  args) {
				typedef decltype(GL::detail::function_signature(func)) function_header;
				static_assert((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function, "task must be a static function or non-capturing lambda");

				using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
				auto out = Task<typename traits::returnType>::template TaskImpl<false, ReturnType, traits, sizeof...(Args)>(0, 1, +func, *this);

				auto out_id = out.id();
				auto& out_ticket = id::get_id(out_id);
				auto& Base = out_ticket.data;

				if constexpr (sizeof...(Args) > 16) {
					static_assert(false, "The async system cannot process a function with too many arguments");
				}
				else if constexpr (sizeof...(Args) >= traits::numArgs) {
					// user provided all of the arguments.
					decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
				}
				else if constexpr (std::is_void_v<ReturnType>) {
					// no parent...
					if constexpr (decltype(out)::dispatch_multiple) {
						// ...but potentially uses an index.
						static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index");
						static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
						static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
						Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
						decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else {
						// ...and no index.
						static_assert(false, "The incorrect number of function arguments were passed to the async task");
					}
				}
				else {
					// has a parent...
					if constexpr (decltype(out)::dispatch_multiple) {
						// ...and potentially uses an index.
						static_assert((sizeof...(Args) + 2) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index and a return type from the parent task");
						static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
						static_assert(
							std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
							|| std::is_convertible_v<ReturnType&, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<ReturnType*, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<1, typename traits::argType>>
							|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
							|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<1, typename traits::argType>>>
							, "The second argument was assumed to be a return type from the parent task, but was unable to be converted");
						static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
						Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
						Base._arguments[1] = nullptr;
						decltype(out)::template wrapper<2>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
					else {
						// ...but no index.
						static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a return type from the parent task");
						static_assert(
							std::is_same_v< std::decay_t<ReturnType>, GL::any> || std::is_same_v<std::decay_t<ReturnType>, GL::atomic_any>
							|| std::is_convertible_v<ReturnType&, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<ReturnType*, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<std::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<GL::shared_ptr<ReturnType>, std::tuple_element_t<0, typename traits::argType>>
							|| std::is_same_v<GL::any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
							|| std::is_same_v<GL::atomic_any, std::decay_t<std::tuple_element_t<0, typename traits::argType>>>
							, "The first argument was assumed to be a return type from the parent task, but was unable to be converted");
						Base._arguments[0] = nullptr;
						decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
					}
				}

				Base._deferred_dispatch = &decltype(out)::DeferredDispatch;
				GL::interlocked::increment(out_ticket.count); // its own lifespan, which may be decremented when the task goes out-of-scope
				GL::interlocked::increment(out_ticket.count); // child lifespan
				this->get_task()._children.push(out_id);
				auto& thisTask = this->get_task();
				thisTask._children.push(out_id);
				if (thisTask._job_completed) {
					unsigned int child_uuid;
					while (thisTask._children.try_pop(child_uuid)) {
						iTask temp((unsigned long)child_uuid);
						if (auto& child_base = id::get_id(child_uuid).data; child_base._deferred_dispatch)
							child_base._deferred_dispatch(child_uuid, child_base);
					}
				}
				//out.Dispatch(Base);
				return out.AsTask();
			};

		};
		// instance of an async task that can be waited on.
		template<typename Func, typename = std::enable_if_t<std::is_empty_v<Func>>, typename... Args> decltype(auto) async(Func const& func, Args&&...  args) {
			typedef decltype(GL::detail::function_signature(func)) function_header;
			if constexpr ((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function) { // function objects, e.g. auto x = [](){};
				using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
				auto out = Task<typename traits::returnType>::template TaskImpl<false, void, traits, sizeof...(Args)>(0ull, 1ull, +func, nullptr);
				auto out_id = out.id();
				auto& out_ticket = id::get_id(out_id);
				auto& Base = out_ticket.data;
				if constexpr (sizeof...(Args) > 16) {
					static_assert(false, "The async system cannot process a function with too many arguments");
				}
				else if constexpr (sizeof...(Args) >= traits::numArgs) {
					// user provided all of the arguments.
					decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
				}
				else {
					// no parent and no index.
					static_assert(false, "The incorrect number of function arguments were passed to the async task");
				}
				//GL::interlocked::increment(out_ticket.count);
				out.Dispatch(Base);
				return out.AsTask();
			}
			else {
				static_assert(false, "task must be a static function or non-capturing lambda");
			}
		};
		// instance of an async task that can be waited on.
		template<typename Func, typename... Args> decltype(auto) async(size_t start, size_t end, Func const& func, Args&&... args) {
			typedef decltype(GL::detail::function_signature(func)) function_header;
			if constexpr ((function_header::is_object && std::is_empty_v< Func >) || function_header::is_static_member_function) { // function objects, e.g. auto x = [](){};
				using traits = typename decltype(Caching_Static_Function_Caller(+func))::traits;
				auto out = Task<typename traits::returnType>::template TaskImpl<true, void, traits, sizeof...(Args)>(start, end, +func, nullptr);
				auto out_id = out.id();
				auto& out_ticket = id::get_id(out_id);
				auto& Base = out_ticket.data;
				if constexpr (sizeof...(Args) > 16) {
					static_assert(false, "The async system cannot process a function with too many arguments");
				}
				else if constexpr (sizeof...(Args) >= traits::numArgs) {
					// user provided all of the arguments.
					decltype(out)::template wrapper<0>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
				}
				else {
					// no parent but potentially uses an index.
					static_assert((sizeof...(Args) + 1) == traits::numArgs, "The incorrect number of function arguments were passed to the async task, even assuming the use of a iteration index");
					static_assert(std::is_convertible_v<size_t, std::tuple_element_t<0, typename traits::argType>>, "The first argument was assumed to be an iteration index, but was unable to be converted");
					static_assert(std::is_pod_v<std::decay_t<std::tuple_element_t<0, typename traits::argType>>>, "The first argument was assumed to be an iteration index, which must be handled using a POD-type");
					Base._arguments[0] = GL::any::instance(std::decay_t<std::tuple_element_t<0, typename traits::argType>>{});
					decltype(out)::template wrapper<1>::processPackWithIndices(Base, std::make_index_sequence<sizeof...(Args)>{}, std::forward<Args>(args)...);
				}
				//GL::interlocked::increment(out_ticket.count);
				out.Dispatch(Base);
				return out.AsTask();
			}
			else {
				static_assert(false, "task must be a static function or non-capturing lambda");
			}
		};

	};


};