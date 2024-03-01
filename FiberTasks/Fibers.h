#pragma once

#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/Iterator.h"
#include <ppl.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_set.h>

namespace fibers {
	namespace utilities {

		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };
	};

	namespace containers {
		/* Fiber- and thread-safe vector. Objects are stored and returned as std::shared_ptr. Growth, iterations, and push_back operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire vector. */
		template<typename _Ty>
		class vector {
		protected:
			using underlying = typename concurrency::concurrent_vector<std::shared_ptr<_Ty>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const vector* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const vector* ref) { ++pos; }
				inline void end(const vector* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(vector* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const vector* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const vector* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(vector, typename underlying::value_type, it_state);

			vector() : data(new underlying()) {};
			explicit vector(size_type _N) : data(new underlying(_N)) {};
			vector(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			vector(vector const& r) : data(new underlying()) {
				operator=(r);
			}
			vector(vector&& r) = default;
			vector& operator=(vector const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					vector out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			vector& operator=(vector&& r) = default;
			~vector() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(std::make_shared<_Ty>(_Item)); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::make_shared<_Ty>(std::forward<_Ty>(_Item))); };
			decltype(auto) push_back(std::shared_ptr<_Ty> const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(std::shared_ptr<_Ty>&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			std::shared_ptr<_Ty> operator[](size_type _Index) { return data->operator[](_Index); };
			std::shared_ptr<_Ty> operator[](size_type _Index) const { return data->operator[](_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) { return data->at(_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
			decltype(auto) erase(const_iterator _Where) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) clear() {
				vector out;
				data.swap(out.data);
			};
			decltype(auto) swap(vector& r) {
				data.swap(r.data);
			};
		};

		/* Fiber- and thread-safe vector that cannot erase elements. Objects are stored as-is, and therefore the array must outlive the underlying objects. All operations are concurrent except for 'operator=(array)'.
		Higher performance is expected with the array than the vector, but the user needs to be slighly more careful with their timing to ensure the array remaings alive until access is complete. */
		template<typename _Ty>
		class array {
		protected:
			using underlying = typename concurrency::concurrent_vector<_Ty>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const array* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const array* ref) { ++pos; }
				inline void end(const array* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(array* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const array* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const array* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(array, typename underlying::value_type, it_state);

			array() : data(new underlying()) {};
			explicit array(size_type _N) : data(new underlying(_N)) {};
			array(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			array(array const& r) : data(new underlying()) {
				operator=(r);
			}
			array(array&& r) = default;
			array& operator=(array const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					array out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			array& operator=(array&& r) = default;
			~array() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			decltype(auto) operator[](size_type _Index) { return data->operator[](_Index); };
			decltype(auto) operator[](size_type _Index) const { return data->operator[](_Index); };
			decltype(auto) at(size_type _Index) { return data->at(_Index); };
			decltype(auto) at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
		};

		/* Fiber- and thread-safe map / dictionary. Objects are stored and returned as std::shared_ptr. Growth, iterations, and insert/emplace operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire map. */
		template<typename _Key_type, typename _Element_type>
		class unordered_map {
		protected:
			using underlying = typename concurrency::concurrent_unordered_map<_Key_type, std::shared_ptr<_Element_type>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;

				inline void begin(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const unordered_map* ref) { ++pos; }
				inline void end(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(unordered_map* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				inline void prev(const unordered_map* ref) { --pos; }
				inline const typename underlying::value_type& get(const unordered_map* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(unordered_map, typename underlying::value_type, it_state);

			typedef typename underlying::key_type key_type;
			typedef typename underlying::mapped_type mapped_type;
			typedef typename underlying::key_equal key_equal;
			typedef typename underlying::hasher hasher;
			typedef iterator local_iterator;
			typedef const_iterator const_local_iterator;

			unordered_map() : data(new underlying()) {};
			explicit unordered_map(size_type _N) : data(new underlying(_N)) {};
			unordered_map(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			template<class _InputIterator> unordered_map(_InputIterator _Begin, _InputIterator _End) : data(new underlying(_Begin, _End)) {};
			unordered_map(unordered_map const& r) : data(new underlying()) {
				this->operator=(r);
			};
			unordered_map(unordered_map&& r) = default;
			unordered_map& operator=(unordered_map const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					unordered_map out;
					for (auto& x : *r.data) out[x.first] = x.second;
					out.data.swap(data);
				}
				return *this;
			};
			unordered_map& operator=(unordered_map&& r) = default;

			decltype(auto) insert(const value_type& _Value) { return data->insert(_Value); };
			decltype(auto) insert(const_iterator _Where, const value_type& _Value) { return data->insert(_Where, _Value); };
			template<class _Iterator> decltype(auto) insert(_Iterator _First, _Iterator _Last) { return data->insert(_First, _Last); };
			template<class _Valty> decltype(auto) insert(_Valty&& _Value) { return data->insert(_Value); };
			template<class _Valty> decltype(auto) insert(const_iterator _Where, _Valty&& _Value) { return data->insert(_Where, _Value); };
			decltype(auto) hash_function() const { return data->hash_function(); };
			decltype(auto) key_eq() const { return data->key_eq(); };
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) const {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) { return data->at(_Keyval); };
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) const { return data->at(_Keyval); };
			decltype(auto) front() { return data->front(); };
			decltype(auto) front() const { return data->front(); };
			decltype(auto) back() { return data->back(); };
			decltype(auto) back() const { return data->back(); };
			decltype(auto) erase(const_iterator _Where) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const key_type& _Keyval) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter->first == _Keyval) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) unsafe_erase(const key_type& _Keyval) {
				data->unsafe_erase(_Keyval);
			};
			decltype(auto) count(const key_type& _Keyval) const { return data->count(_Keyval); };
			decltype(auto) emplace(const key_type& _Keyval, const std::shared_ptr<_Element_type>& _Value) { return insert(value_type(_Keyval, _Value)); };
			decltype(auto) emplace(const key_type& _Keyval, std::shared_ptr<_Element_type>&& _Value) { return insert(value_type(_Keyval, std::forward<typename underlying::value_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, const _Element_type& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, _Element_type&& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(std::forward<typename underlying::value_type>(_Value)))); };
			decltype(auto) clear() {
				unordered_map out;
				data.swap(out.data);
			};
		};

		template<typename _Key_type> using unordered_set = concurrency::concurrent_unordered_set<_Key_type>; /* Wrapper To-Do */
		template<typename _Value_type> using queue = concurrency::concurrent_queue<_Value_type>; /* Wrapper To-Do */
    };

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup;

	/*!
	Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wraper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	Job([](){ return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, allowing you to continue work on this thread.
	*/
	class Job {
		friend JobGroup;
	protected:
		mutable std::shared_ptr<Action> impl;

	public:
		static Job Finished() {
			decltype(auto) toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished());
			return toReturn;
		};
		template <typename T> static Job Finished(const T& returnMe) {
			decltype(auto) toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished(returnMe));
			return toReturn;
		};

		Job() : impl(std::make_shared<Action>()) {};
		Job(const Job& other) = default;
		Job(Job&& other) = default;
		Job& operator=(const Job& other) = default;
		Job& operator=(Job&& other) = default;
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<T>> && !std::is_same_v<Any, std::decay_t<T>> >>
		explicit Job(T function, Args... Fargs) : impl(new Action(function, Fargs...)) {};

	public:
		/* Do the task immediately, without using any thread/fiber tools. Does not do the task if it has been previously performed. */
		Any Invoke() noexcept {
			Any out;
			auto* p = impl->Invoke();
			if (p) out = *p;
			return out;
		};

		/* Do the task immediately, without using any thread/fiber tools, whether or not it has been performed before. */
		Any ForceInvoke() noexcept {
			Any out;
			auto* p = impl->ForceInvoke();
			if (p) out = *p;
			return out;
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. Does not do the task if it has been previously performed. */
		JobGroup AsyncInvoke();

		/* Add the task to a thread / fiber, and retrieve an awaiter group, whether or not it has been performed before. */
		JobGroup AsyncForceInvoke();

		/* Queue job, and return tool to await the result */
		void DelayedInvoke(u64 milliseconds_delay) {
			cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
			cweeSysThreadTools::Sys_ForgetThread(cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
				if (_anon_ptr != nullptr) {
					cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
					::Sleep(T->get<0>());
					T->get<1>().Invoke();
					delete T;
				}
				return 0;
			}), (void*)data, 1024));
		};

		/* Queue job, and return tool to await the result */
		void DelayedForceInvoke(u64 milliseconds_delay) {
			cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
			cweeSysThreadTools::Sys_ForgetThread(cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
				if (_anon_ptr != nullptr) {
					cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
					::Sleep(T->get<0>());
					T->get<1>().ForceInvoke();
					delete T;
				}
				return 0;
			}), (void*)data, 1024));
		};

		/* Queues the job onto a new thread to take place in the future */
		void AsyncDelayedInvoke(u64 milliseconds_delay);

		/* Queues the job onto a new thread to take place in the future */
		void AsyncDelayedForceInvoke(u64 milliseconds_delay);

		/* Returns the potential name of the static function, if one was provided. */
		const char* FunctionName() const {
			return impl->FunctionName();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any GetResult() {
			return Invoke();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any operator()() {
			return Invoke();
		};

		/* Checks if the job has been completed before */
		bool IsFinished() const {
			return impl->IsFinished();
		};

		bool ReturnsNothing() const {
			return impl->ReturnsNothing();
		};
	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup {
		friend Job;
	private:
		class JobGroupImpl {
		public:
			std::shared_ptr<void> waitGroup;
			fibers::containers::vector<Job> jobs;

			JobGroupImpl() : waitGroup(nullptr), jobs() {};
			JobGroupImpl(std::shared_ptr<void> wg) : waitGroup(wg), jobs() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;

			void Queue(Job const& job);
			void ForceQueue(Job const& job); 
			void Queue(std::vector<Job> const& listOfJobs);
			void ForceQueue(std::vector<Job> const& listOfJobs);
			void Wait();
			~JobGroupImpl() {
				// Wait(); // any jobs queued with this waiter will want to talk w/it when finished -- we must wait to ensure they go out of scope before we do.
			};
		};

	public:
		JobGroup();
		JobGroup(Job const& job);

		// The waiter should not be passed around. Ideally we want to follow Fiber job logic, e.g. splitting jobs quickly and 
		// then finishing them in the same job that started them, continuing like the split never happened.
		JobGroup(JobGroup const&) = delete;
		JobGroup(JobGroup&&) = delete;
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() {};

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			impl->Queue(job);
			return *this;
		};
		/* Queue job, and return tool to await the result */
		JobGroup& ForceQueue(Job const& job) {
			impl->ForceQueue(job);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			impl->Queue(listOfJobs);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& ForceQueue(std::vector<Job> const& listOfJobs) {
			impl->ForceQueue(listOfJobs);
			return *this;
		};

		/* Await all jobs in this group, and get the return values (which may be empty) for each job */
		std::vector<Any> Wait_Get() {
			impl->Wait();

			typename decltype(JobGroupImpl::jobs) out;
			out.swap(impl->jobs);

			if (out.size() > 0) {
				std::vector<Any> any;
				for (auto& job : out) {
					if (job) {
						any.push_back(job->GetResult());
					}
				}
				return any;
			}
			else {
				return std::vector<Any>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
			impl->jobs.clear();
		};

	private:
		std::unique_ptr<JobGroupImpl> impl;

	};

	namespace synchronization {
		/* Fiber mutex */
		class mutex {
		public:
			mutex();
			mutex(const mutex& other);
			mutex(mutex&& other);
			mutex& operator=(const mutex& s) { return *this; };
			mutex& operator=(mutex&& s) { return *this; };
			~mutex() {};

			[[nodiscard]] std::lock_guard<mutex>	guard() noexcept;
			void			lock() noexcept;
			void			unlock() noexcept;
			bool            try_lock() noexcept;

		protected:
			std::shared_ptr<void> Handle;

		};

		/* Wrapper for non-atomic objects to allow for threads to lock them for exclusive access */
		template< typename type, typename MutexType = fibers::synchronization::mutex>
		class interlocked {
		public:
			class ExclusiveObject {
			public:
				constexpr ExclusiveObject(const interlocked<type>& mut) : owner(const_cast<interlocked<type>&>(mut)) { this->owner.Lock(); };
				~ExclusiveObject() { this->owner.Unlock(); };

				ExclusiveObject() = delete;
				ExclusiveObject(const ExclusiveObject& other) = delete;
				ExclusiveObject(ExclusiveObject&& other) = delete;
				ExclusiveObject& operator=(const ExclusiveObject& other) = delete;
				ExclusiveObject& operator=(ExclusiveObject&& other) = delete;

				type& operator=(const type& a) { data() = a; return data(); };
				type& operator=(type&& a) { data() = a; return data(); };
				type& operator*() const { return data(); };
				type* operator->() const { return &data(); };

			protected:
				type& data() const { return owner.UnsafeRead(); };
				interlocked<type>& owner;
			};

		public: // construction and destruction
			typedef type Type;

			interlocked() : data() {};
			interlocked(const type& other) : data(other) {};
			interlocked(type&& other) : data(std::forward<type>(other)) {};
			interlocked(const interlocked& other) : data() { this->Copy(other); };
			interlocked(interlocked&& other) : data() { this->Copy(std::forward<interlocked>(other)); };
			~interlocked() {};

		public: // copy and clear
			interlocked<type>& operator=(const interlocked<type>& other) {
				this->Copy(other);
				return *this;
			};
			interlocked<type>& operator=(interlocked<type>&& other) {
				this->Copy(std::forward<interlocked<type>>(other));
				return *this;
			};
			void Copy(const interlocked<type>& copy) {
				if (&copy == this) return;
				lock.Lock();
				copy.Lock();
				data = copy.data;
				copy.Unlock();
				lock.Unlock();
			};
			void Copy(interlocked<type>&& copy) {
				if (&copy == this) return;
				lock.Lock();
				copy.Lock();
				data = copy.data;
				copy.Unlock();
				lock.Unlock();
			};
			void Clear() {
				lock.Lock();
				data = type();
				lock.Unlock();
			};

		public: // read and swap
			type Read() const {
				decltype(auto) g = Guard();
				return data;
			};
			void Swap(const type& replacement) {
				decltype(auto) g = Guard();
				data = replacement;
			};
			interlocked<type>& operator=(const type& other) {
				Swap(other);
				return *this;
			};

			operator type() const { return Read(); };
			type* operator->() const { return &data; };

		public: // lock, unlock, and direct edit
			ExclusiveObject GetExclusive() const { return ExclusiveObject(*this); };

			[[nodiscard]] decltype(auto) Guard() const { return lock.Guard(); };
			void Lock() const { lock.Lock(); };
			void Unlock() const { lock.Unlock(); };
			type& UnsafeRead() const { return data; };

		private:
			mutable type data;
			MutexType lock;

		};

		/* Tool that allows fibers to busy-work until a signla is raised by another fiber or thread. */
		class signal {
		public:
			signal(bool manualReset = true);
			signal(const signal&) = default;
			signal(signal&&) = default;
			signal& operator=(const signal&) = default;
			signal& operator=(signal&&) = default;
			~signal() {};

		public:
			/* Raise (set to one) the signal. Free & fast. */
			void	Raise() noexcept;
			/* Clear (zero-out) the signal. Free & fast. */
			void	Clear() noexcept;
			/* Busy-waits for the signal to be raised. */
			void	Wait() noexcept;
			/* Tests if the signal has been raised. Free & fast. */
			bool	TryWait() noexcept;

		private:
			std::shared_ptr<void> impl;

		};

		/* Read-Write mutex that allows multiple readers and one writer to cooperatively access an underlying object. Very fast for 100% reading operations, as (effectively) no locking actually happens. */
		class shared_mutex {
		private:
			mutex    mut_;
			std::condition_variable_any gate1_;
			std::condition_variable_any gate2_;
			unsigned state_;

			static const unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
			static const unsigned n_readers_ = ~write_entered_;

		public:

			shared_mutex() : state_(0) {}

			// Exclusive/Writer ownership
			void lock() {
				std::unique_lock<mutex> lk(mut_);
				while (state_ & write_entered_) gate1_.wait(lk);
				state_ |= write_entered_;
				while (state_ & n_readers_) gate2_.wait(lk);
			};
			// Exclusive/Writer ownership
			bool try_lock() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock);
				if (lk.owns_lock() && state_ == 0)
				{
					state_ = write_entered_;
					return true;
				}
				return false;
			};
			// Exclusive/Writer ownership
			void unlock() {
				{
					std::scoped_lock<mutex> _(mut_);
					state_ = 0;
				}
				gate1_.notify_all();
			};

			// Shared/Reader ownership
			void lock_shared() {
				std::unique_lock<mutex> lk(mut_);
				while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_)
					gate1_.wait(lk);
				unsigned num_readers = (state_ & n_readers_) + 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
			};
			// Shared/Reader ownership
			bool try_lock_shared() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock);
				unsigned num_readers = state_ & n_readers_;
				if (lk.owns_lock() && !(state_ & write_entered_) && num_readers != n_readers_)
				{
					++num_readers;
					state_ &= ~n_readers_;
					state_ |= num_readers;
					return true;
				}
				return false;
			};
			// Shared/Reader ownership
			void unlock_shared() {
				std::scoped_lock<mutex> _(mut_);
				unsigned num_readers = (state_ & n_readers_) - 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
				if (state_ & write_entered_)
				{
					if (num_readers == 0)
						gate2_.notify_one();
				}
				else
				{
					if (num_readers == n_readers_ - 1)
						gate1_.notify_one();
				}
			};

			[[nodiscard]] decltype(auto) Write_Guard() noexcept { return std::lock_guard(*this); };
			[[nodiscard]] decltype(auto) Read_Guard() noexcept { return std::shared_lock(*this); };
		};
	};

	namespace parallel {
		/* parallel_for (auto i = start; i < end; i++){ todo(i); } */
		template<typename iteratorType, typename F>
		decltype(auto) For(iteratorType start, iteratorType end, F&& ToDo) {
			auto todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;
			for (iteratorType iter = start; iter < end; iter++) {
				jobs.push_back(fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter));
			}
			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};

		/* parallel_for (auto i = start; i < end; i += step){ todo(i); } */
		template<typename iteratorType, typename F>
		decltype(auto) For(iteratorType start, iteratorType end, iteratorType step, F&& ToDo) {
			auto todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;
			for (iteratorType iter = start; iter < end; iter += step) {
				jobs.push_back(fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); } */
		template<typename containerType, typename F>
		decltype(auto) ForEach(containerType const& container, F&& ToDo) {
			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;

			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs.push_back(fibers::Job([todo](typename containerType::const_iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(const_cast<typename containerType::value_type*>(&*T), [](typename containerType::value_type*) {})).cast()); }, (typename containerType::const_iterator)(iter)));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};

		/* parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); } */
		template<typename containerType, typename F>
		decltype(auto) ForEach(containerType& container, F&& ToDo) {
			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;

			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs.push_back(fibers::Job([todo](typename containerType::iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(&*T, [](typename containerType::value_type*) {})).cast()); }, (typename containerType::iterator)(iter)));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};
	};
};


class FTL {
public:
	static fibers::Job fnFiberTasks2b(int numTasks);
	static fibers::Job fnFiberTasks2c(int numTasks);
	static fibers::Job fnFiberTasks2d(int numTasks, int numSubTasks);
	static fibers::Job fnFiberTasks3(int numTasks, int numSubTasks);
};