// FiberTasks.cpp : Defines the functions for the static library.

#undef FTL_FIBER_CANARY_BYTES
#define FTL_NUM_WAITING_FIBER_SLOTS 4
#undef FTL_VALGRIND
#undef FTL_FIBER_STACK_GUARD_PAGES
#define FTL_CPP_17
#undef FTL_WERROR
#undef FTL_DISABLE_ITERATOR_DEBUG

#include "ftl/task_scheduler.h"
#include "ftl/wait_group.h"
#include "ftl/fibtex.h"

#include <assert.h>
#include <stdint.h>





struct NumberSubset {
	uint64_t start;
	uint64_t end;

	uint64_t total;
};
void AddNumberSubset(ftl::TaskScheduler* taskScheduler, void* arg) {
	(void)taskScheduler;
	NumberSubset* subset = reinterpret_cast<NumberSubset*>(arg);

	subset->total = 0;

	while (subset->start != subset->end) {
		subset->total += subset->start;
		++subset->start;
	}

	subset->total += subset->end;
}

#include "FiberTasks.h"
uint64_t FTL::fnFiberTasks() {
	//if constexpr (true) {
		// Create the task scheduler and bind the main thread to it
		ftl::TaskScheduler taskScheduler;
		taskScheduler.Init();

		// Define the constants to test
		constexpr uint64_t triangleNum = 47593243ULL;
		constexpr uint64_t numAdditionsPerTask = 10000ULL;
		constexpr uint64_t numTasks = (triangleNum + numAdditionsPerTask - 1ULL) / numAdditionsPerTask;

		// Create the tasks
		// FTL allows you to create Tasks on the stack.
		// However, in this case, that would cause a stack overflow
		decltype(auto) tasks = std::shared_ptr< ftl::Task[]>(new ftl::Task[numTasks]);
		decltype(auto) subsets = std::shared_ptr< NumberSubset[]>(new NumberSubset[numTasks]);

		uint64_t nextNumber = 1ULL;

		for (uint64_t i = 0ULL; i < numTasks; ++i) {
			decltype(auto) subset = &subsets[i];

			subset->start = nextNumber;
			subset->end = nextNumber + numAdditionsPerTask - 1ULL;
			if (subset->end > triangleNum) {
				subset->end = triangleNum;
			}

			tasks[i] = { AddNumberSubset, subset };

			nextNumber = subset->end + 1;
		}

		// Schedule the tasks
		ftl::WaitGroup wg(&taskScheduler);
		taskScheduler.AddTasks(numTasks, tasks.get(), ftl::TaskPriority::Normal, &wg);

		// FTL creates its own copies of the tasks, so we can safely delete the memory
		//delete[] tasks;

		// Wait for the tasks to complete
		wg.Wait();

		// Add the results
		uint64_t result = 0ULL;
		for (uint64_t i = 0; i < numTasks; ++i) {
			result += subsets[i].total;
		}

		// Test
		assert(triangleNum * (triangleNum + 1ULL) / 2ULL == result);

		// Cleanup
		//delete[] subsets;

		// The destructor of TaskScheduler will shut down all the worker threads
		// and unbind the main thread
		return result;
	//}
};

#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/cweeInterlocked.h"
#include "../WaterWatchCpp/cwee_math.h"
#include "../WaterWatchCpp/Toasts.h"
#include "../WaterWatchCpp/cweeJob.h"

uint64_t FTL::fnFiberTasks2a(int numTasks) {
	ftl::TaskScheduler taskScheduler;
	taskScheduler.Init();

	struct FuncStruct {
		cweeJob job;
		ftl::WaitGroup* wg;
	};
	auto lambda = [](ftl::TaskScheduler* taskScheduler, void* arg) {
		FuncStruct* data = (FuncStruct*)arg;

		data->job.Invoke();

		delete data;
	};

	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;

	ftl::WaitGroup wg(&taskScheduler);
	for (int i = 0; i < numTasks; ++i) {
		cweeJob todo;
		switch (cweeRandomInt(1, 4)) {
		case 1:
			todo = cweeJob([&numParallel, &maxParallel](double& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment(); 
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				::Sleep(5);
				numParallel.Decrement();

				return j;
			}, double(i));
			break;
		case 2:
			todo = cweeJob([&numParallel, &maxParallel](float j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				::Sleep(5);
				numParallel.Decrement();

				return j;
			}, float(i));
			break;
		case 3:
			todo = cweeJob([&numParallel, &maxParallel](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				::Sleep(5);
				numParallel.Decrement();

				return *j;
			}, cweeStr(i));
			break;
		case 4:
			todo = cweeJob([&numParallel, &maxParallel](std::string const& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				::Sleep(5);
				numParallel.Decrement();

				return j;
			}, std::to_string(i));
			break;
		}
		taskScheduler.AddTask({ lambda, new FuncStruct({ todo, &wg }) }, ftl::TaskPriority::Normal, &wg);
	}
	wg.Wait();

	return maxParallel.GetValue();
};

extern DelayedInstantiation< ftl::TaskScheduler > Fibers = DelayedInstantiation< ftl::TaskScheduler >([]()-> ftl::TaskScheduler* { 
	auto* p = new ftl::TaskScheduler(); 
	p->Init({ 400, 0, ftl::EmptyQueueBehavior::Sleep, {} });
	p->SetEmptyQueueBehavior(ftl::EmptyQueueBehavior::Sleep);
	return p; 
});
class cweeFiberMutex {
public:
	using Handle_t = ftl::Fibtex;
	using Phandle = std::shared_ptr<Handle_t>;

	class cweeSysMutexLifetimeGuard {
	public:
		explicit operator bool() const { return (bool)owner; };
		explicit cweeSysMutexLifetimeGuard(const Phandle& mut) noexcept : owner(mut) { owner->lock(); };
		~cweeSysMutexLifetimeGuard() noexcept { owner->unlock(); };
		explicit cweeSysMutexLifetimeGuard() = delete;
		explicit cweeSysMutexLifetimeGuard(const cweeSysMutexLifetimeGuard& other) = delete;
		explicit cweeSysMutexLifetimeGuard(cweeSysMutexLifetimeGuard&& other) = delete;
		cweeSysMutexLifetimeGuard& operator=(const cweeSysMutexLifetimeGuard&) = delete;
		cweeSysMutexLifetimeGuard& operator=(cweeSysMutexLifetimeGuard&&) = delete;
	protected:
		Phandle owner;
	};

public:
	cweeFiberMutex() : Handle(new Handle_t(&*Fibers)) {};
	cweeFiberMutex(const cweeFiberMutex& other) : Handle(new Handle_t(&*Fibers)) {};
	cweeFiberMutex(cweeFiberMutex&& other) : Handle(new Handle_t(&*Fibers)) {};
	cweeFiberMutex& operator=(const cweeFiberMutex& s) { return *this; };
	cweeFiberMutex& operator=(cweeFiberMutex&& s) { return *this; };
	~cweeFiberMutex() {};

	NODISCARD cweeSysMutexLifetimeGuard	Guard() const { return cweeSysMutexLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const { Handle->lock(); return true; };
	void			Unlock() const { Handle->unlock(); };

protected:
	Phandle Handle;

};

#include "../WaterWatchCpp/cweeTime.h"

struct FunctionStruct {
	cweeJob job;
};
static void DoFuncStruct(ftl::TaskScheduler* taskScheduler, void* arg) {
	AUTO data = std::shared_ptr<FunctionStruct>(static_cast<FunctionStruct*>(arg));
	data->job.Invoke();
};
uint64_t FTL::fnFiberTasks2b(int numTasks) {
	ftl::TaskScheduler& taskScheduler = *Fibers;

	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;
	
	ftl::WaitGroup wg(&taskScheduler);

	std::vector<ftl::Task> tasks;

	for (int i = 0; i < numTasks; ++i) {
		cweeJob todo;
		switch (cweeRandomInt(1, 4)) {
		default:
		case 1:
			todo = cweeJob([&numParallel, &maxParallel](double& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j;
			}, double(i));
			break;
		case 2:
			todo = cweeJob([&numParallel, &maxParallel](float j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j;
			}, float(i));
			break;
		case 3:
			todo = cweeJob([&numParallel, &maxParallel](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return *j;
			}, cweeStr(i));
			break;
		case 4:
			todo = cweeJob([&numParallel, &maxParallel](std::string const& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j;
			}, std::to_string(i));
			break;
		}
		tasks.push_back({ DoFuncStruct, new FunctionStruct({ std::move(todo) }) });
	}

	taskScheduler.AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);

	wg.Wait();

	return maxParallel.GetValue();
};

uint64_t FTL::fnFiberTasks2c(int numTasks) {
	ftl::TaskScheduler& taskScheduler = *Fibers;
	// taskScheduler.Init();
	cweeFiberMutex mutex; 


	struct FuncStruct {
		cweeJob job;
		ftl::WaitGroup* wg;
	};
	auto lambda = [](ftl::TaskScheduler* taskScheduler, void* arg) {
		FuncStruct* data = (FuncStruct*)arg;

		data->job.Invoke();

		delete data;
	};

	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;

	ftl::WaitGroup wg(&taskScheduler);

	std::vector<ftl::Task> tasks;

	for (int i = 0; i < numTasks; ++i) {
		cweeJob todo;
		switch (cweeRandomInt(1, 4)) {
		default:
		case 1:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](double& j) -> std::remove_reference_t<decltype(j)> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();
				
				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return j;
			}, double(i));
			break;
		case 2:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](float j) -> std::remove_reference_t<decltype(j)> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return j;
			}, float(i));
			break;
		case 3:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return *j;
			}, cweeStr(i));
			break;
		case 4:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](std::string const& j) -> std::remove_reference_t<decltype(j)> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return j;
			}, std::to_string(i));
			break;
		}

		tasks.push_back({ lambda, new FuncStruct({ todo, &wg }) });
	}

	mutex.Lock();
	taskScheduler.AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
	mutex.Unlock();

	wg.Wait();

	return maxParallel.GetValue();
};










#include <random>
class fiber_rand {
public:
	class cwee_pcg {
	public:
		using result_type = uint32_t;
		static constexpr result_type(min)() { return 0; }
		static constexpr result_type(max)() { return UINT32_MAX; }

		cwee_pcg() noexcept : m_state(0), m_inc(0), rd() { seed(); };
		void seed() noexcept {
			uint64_t s0 = uint64_t(rd()) << 31 | uint64_t(rd());
			uint64_t s1 = uint64_t(rd()) << 31 | uint64_t(rd());
			m_state = 0;
			m_inc = (s1 << 1) | 1;
			(void)operator()();
			m_state += s0;
			(void)operator()();
		};
		result_type operator()() const noexcept {
			uint64_t oldstate = cweeSysThreadTools::Sys_InterlockedExchangeV< uint64_t>(m_state, (cweeSysThreadTools::Sys_InterlockedIncrementV<uint64_t>(m_state) - 1) * 6364136223846793005ULL + m_inc);
			uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
			int rot = oldstate >> 59u;
			return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
		};
		void discard(unsigned long long n) const noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };
	private:
		mutable uint64_t m_state;
		uint64_t m_inc;
		std::random_device rd;
	};
private:
	// mutable cweeFiberMutex mut;
	mutable cwee_pcg rand;
	mutable std::uniform_real_distribution<u64> u;
public:
	fiber_rand() noexcept : /*mut(), */rand(), u(0.0, 1.0) { Random_Impl(); /*Instantiate the range*/ };
	u64 Random(u64 t1 = 0.0, u64 t2 = 1.0) const noexcept { return Random_HighRes(std::move(t1), std::move(t2)); };
	double Random(double t1 = 0.0, double t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	float Random(float t1 = 0.0, float t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	int Random(int t1 = 0, int t2 = std::numeric_limits<int>::max()) const noexcept { return std::floor(Random_HighRes(t1, t2) + 0.5); };
private:
	u64 Random_Impl() const noexcept {
		return u(rand);
		//u64 out(0);
		//mut.Lock();
		//out = u(rand);
		//mut.Unlock();
		//return out;
	};
	u64 Random_HighRes(u64 t1, u64 t2) const noexcept {
		t2 -= t1;
		t2 *= Random_Impl();
		t1 += t2;
		return t1;
	};
};
extern DelayedInstantiation< fiber_rand > FiberRandomGenerator = DelayedInstantiation< fiber_rand >([]()-> fiber_rand* { return new fiber_rand(); });

/*! random float between 0 and 1 */ INLINE float fiberRandomFloat() { return FiberRandomGenerator->Random(0.0f, 1.0f); };
/*! random float between 0 and max */ INLINE float fiberRandomFloat(float max) { return FiberRandomGenerator->Random(0.0f, max); };
/*! random float between min and max */ INLINE float fiberRandomFloat(float min, float max) { return FiberRandomGenerator->Random(min, max); };
/*! random int between 0 and cweeMath::INF */ INLINE int fiberRandomInt() { return FiberRandomGenerator->Random(0, std::numeric_limits<int>::max()); };
/*! random int between 0 and max */ INLINE int fiberRandomInt(int max) { return FiberRandomGenerator->Random(0, max); };
/*! random int between min and max */ INLINE int fiberRandomInt(int min, int max) { return FiberRandomGenerator->Random(min, max); };

/*! Thread-safe list that performs garbage collection and manages read/write/create/delete operations on data. Intended to act as a multi-threaded database. */
template< typename Key, typename Value>
class fiberMap {
public:
	typedef Value						Type;
	typedef std::shared_ptr < Type >		PtrType;
	typedef std::pair<Key, PtrType>		_iterType;
	typedef long long					IdxType;

	class fiberMap_Impl {
	public:
		/*! prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope. */
		NODISCARD auto	Guard(void) const { // assumes unlocked!
			return lock.Guard();
		}
		NODISCARD auto	SharedGuard(void) const { // assumes unlocked!
			return lock.Guard();
		}
		/*! Prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock -- A "Unlock" must be called to re-enable access to the list. */
		void			Lock(void) const { // assumes unlocked!
			lock.Lock();
		}
		/*! Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior. */
		void			Unlock(void) const { // assumes already locked! 
			lock.Unlock();
		};
		/*! After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping. */
		PtrType			UnsafeRead(Key index) const { // was non-const			
			auto it = list.find(index);
			if (it != list.end()) {
				return it->second;
			}
			return nullptr;
		}
		/*! After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list. */
		cweeThreadedList<Key> UnsafeList(void) const {
			cweeThreadedList<Key> out;

			if (lastCreatedListVersion == CreatedListVersion) {
				out = indexList;

				return out;
			}
			int size = list.size();

			indexList.Clear();
			indexList.SetGranularity(size + 16);
			for (auto& kv : list) indexList.Append(kv.first);
			lastCreatedListVersion = CreatedListVersion;

			out = indexList;

			return out;
		};
		AUTO			UnsafeIndexForKey(Key index) const {
			return std::distance(list.begin(), list.find(index));
		};

		/* Clear the list */
		void Clear() {
			AUTO g = Guard();

			list_version = 0;
			CreatedListVersion = 0;

			for (auto& kv : list) {
				PtrType& p = const_cast<PtrType&>(kv.second); // need to access the underlying thing
				p = nullptr;
			}
			list.clear();

			indexList.Clear();
			lastSearchID = Key();
			lastResult = list.end();
			lastVersion = -1;
			lastCreatedListVersion = -1;
		};

		fiberMap_Impl() : lock(), list(), indexList(), list_version(0), lastSearchID(), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1) {
			list.reserve(16);
			indexList.SetGranularity(16);
		};
		~fiberMap_Impl() {
			Clear();
		};

		/*! Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection */
		mutable cweeFiberMutex													    lock; // cweeSysMutex
		/* Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions. */
		mutable tsl::robin_map<Key, PtrType, robin_hood::hash<Key>, std::equal_to<Key>, std::allocator<_iterType>, true>	list;
		/* Optimized search parameters */
		mutable cweeThreadedList<Key>												indexList;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											list_version;
		/* Optimized search parameters */
		mutable Key																	lastSearchID;
		/* Optimized search parameters */
		mutable typename decltype(list)::iterator		                            lastResult;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											lastVersion;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											CreatedListVersion;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											lastCreatedListVersion;
	};

	typedef std::shared_ptr<fiberMap_Impl>		ImplType;

	struct it_state {
		inline void at(const fiberMap* ref, Key index) {
			ref->Lock();
			AUTO h = ref->UnsafeIndexForKey(index);
			ref->Unlock();
			at(ref, index, h);
		};
		inline void at(const fiberMap* ref, Key index, const IdxType t_hint) {
			listOfIndexes = ref->GetList();
			idx = 0;
			IdxType n = listOfIndexes.Num();


			//idx = t_hint <= n ? t_hint : n;
			//return;

			for (idx = t_hint; idx < n; ++idx) {
				if (listOfIndexes[idx] == index) {
					return;
				}
			}
			for (idx = 0; idx < n && idx < t_hint; ++idx) {
				if (listOfIndexes[idx] == index) {
					return;
				}
			}
			idx = n;
		};
		mutable cweeThreadedList<Key>	listOfIndexes;
		mutable IdxType idx;
		mutable _iterType iter;
		inline void begin(const fiberMap* ref) {
			listOfIndexes = ref->GetList();
			idx = 0;
		}
		inline void next(const fiberMap* ref) {
			++idx;
			while (idx < listOfIndexes.Num() && !ref->Check(listOfIndexes[idx])) {
				++idx;
			}
		}
		inline void end(const fiberMap* ref) {
			listOfIndexes = ref->GetList();
			idx = listOfIndexes.Num();
		}
		inline _iterType& get(fiberMap* ref) {
			iter = ref->GetIterator(listOfIndexes[idx]);
			return iter;
		}
		inline bool cmp(const it_state& s) const { return (idx == s.idx) ? false : true; }
		inline IdxType distance(const it_state& s) const { return idx - s.idx; }

		// Optional to allow operator--() and reverse iterators:
		inline void prev(const fiberMap* ref) {
			--idx;
			while (idx >= 0 && !ref->Check(listOfIndexes[idx])) {
				--idx;
			}
		}
		// Optional to allow `const_iterator`:
		inline const _iterType& get(const fiberMap* ref) const { iter = ref->GetIterator(listOfIndexes[idx]); return iter; }
	};

	SETUP_STL_ITERATOR(fiberMap, _iterType, it_state);

	/*! Default constructor */
	fiberMap() : impl(new fiberMap_Impl()) {};
	/*! Default copy */
	fiberMap(const fiberMap& other) : impl(new fiberMap_Impl()) { this->Copy(other); };
	/*! Take reference to underlying data */
	fiberMap(fiberMap&& other) : impl(other.impl) {};
	/*! Default copy */
	fiberMap& operator=(const fiberMap& other) {
		this->Copy(other);
		return *this;
	};
	/*! Default destructor */
	~fiberMap() {
		impl = nullptr;
	};
	/*! Convert UnorderedMap to a list of keys */
	operator cweeThreadedList<Key>() const {
		return GetList();
	};
	/*! Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack. */
	PtrType			operator[](Key index) const { return GetPtr(index); };
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			Emplace(Key index, const Value& obj) {
		bool out;
		Lock();
		out = UnsafeEmplace(index, obj);
		Unlock();
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			Emplace(Key index, Value&& obj) {
		bool out;
		Lock();
		out = UnsafeEmplace(index, std::forward<Value>(obj));
		Unlock();
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			UnsafeEmplace(Key index, const Value& obj) {
		bool out;
		auto ptr = UnsafeRead(index);
		if (ptr) {
			// already exists
			*ptr = obj;
			out = false;
		}
		else {
			// does not yet exist
			ptr = UnsafeAppendAt(index);
			if (ptr) {
				*ptr = obj;
			}
			out = true;
		}
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			UnsafeEmplace(Key index, Value&& obj) {
		bool out;
		auto ptr = UnsafeRead(index);
		if (ptr) {
			// already exists
			*ptr = std::forward<Value>(obj);
			out = false;
		}
		else {
			// does not yet exist
			ptr = UnsafeAppendAt(index);
			if (ptr) {
				*ptr = std::forward<Value>(obj);
			}
			out = true;
		}
		return out;
	};
	/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does.   */
	PtrType			UnsafeGetPtr(Key index) const {
		if (impl->list.count(index) <= 0)
			return UnsafeAppendAt(index);
		else
			return impl->list[index];
	};
	/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does.   */
	PtrType			GetPtr(Key index) const {
		PtrType out{ TryGetPtr(index) };
		if (!out) {
			Lock();
			out = UnsafeAppendAt(index);
			Unlock();
		}
		return out;
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
	PtrType			TryGetPtr(Key index) const {
		auto* p = impl.get();
		if (p) {
			AUTO g{ p->lock.Guard() };
			if (p->list.count(index) > 0) {
				return p->list.at(index);
			}
		}
		return nullptr;
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
	_iterType		GetIterator(Key index) const {
		return _iterType(index, GetPtr(index));
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
	_iterType		TryGetIterator(Key index) const {
		_iterType out;
		SharedLock();
		if (impl->list.count(index) > 0) {
			out = _iterType(index, impl->list[index]);
		}
		SharedUnlock();
		return out;
	};
	/*! True/False if the index exists on the heap */
	bool			Check(Key index) const {
		bool result = true;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			SharedLock();
			result = (impl->indexList.FindIndex(index) != -1);
			SharedUnlock();
			return result;
		}
		SharedLock();
		result = (impl->list.count(index) != 0);
		SharedUnlock();
		return result;
	};
	/*! True/False if the index exists on the heap */
	bool			UnsafeCheck(Key index) const {
		bool result = true;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			result = (impl->indexList.FindIndex(index) != -1);
			return result;
		}
		result = (impl->list.count(index) != 0);
		return result;
	};
	/*! Clear the current list and create a copy of the incoming list. */
	void			Copy(const fiberMap& copy, const bool threadSafePtr = false) {
		Clear();

		AUTO List = copy.GetList();

		if (threadSafePtr) {
			// Go through and perform swaps					
			for (int i = 0; i < List.Num(); i++) {
				copy.Lock();
				auto readCopy = copy.UnsafeRead(List[i]);
				copy.Unlock();

				if (readCopy) {
					Lock();
					UnsafeAppendAt(List[i]);
					auto access = UnsafeRead(List[i]);
					Unlock();

					if (access) *access = *readCopy;
				}
			}
		}
		else {
			// Go through and perform swaps		
			for (int i = 0; i < List.Num(); i++) {
				copy.Lock();
				auto readCopy = copy.UnsafeRead(List[i]);
				if (readCopy) {
					Lock();
					UnsafeAppendAt(List[i]);
					auto access = UnsafeRead(List[i]);
					if (access) *access = *readCopy;
					Unlock();
				}
				copy.Unlock();
			}
		}
	};
	/*! Copy the Value object from the stack into the heap at the index specified. If the index does not exist on the heap, nothing happens. */
	void			Swap(Key index, const Value& replacement) {
		if (Check(index)) {
			while (!CheckCanDeleteIndex(index)) {};
			Lock();
			AUTO it = impl->lastResult;
			if (impl->lastSearchID == index && impl->list.count(impl->lastSearchID) != 0 && impl->lastVersion == impl->list_version)	it = impl->lastResult;
			else { it = impl->list.find(index); impl->lastSearchID = index; impl->lastResult = it; impl->lastVersion = impl->list_version; }
			impl->list_version.Increment();
			if (it != impl->list.end()) *it->second = replacement;
			Unlock();
		}
	};
	/*! get the number of objects on the heap managed by this list */
	int				Num(void) const {
		int i = 0;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			SharedLock();
			i = impl->indexList.Num();
			SharedUnlock();
			return i;
		}

		SharedLock();
		i = impl->list.size();
		SharedUnlock();
		return i;
	};
	/*! get the number of objects on the heap managed by this list */
	int				UnsafeNum(void) const {
		int i = 0;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			i = impl->indexList.Num();
			return i;
		}
		i = impl->list.size();
		return i;
	};
	/*! get the index position of the key */
	AUTO			UnsafeIndexForKey(Key index) const {
		return impl->UnsafeIndexForKey(std::move(index));
	};
	/*! get a list of all indexes currently on the heap that are currently valid */
	cweeThreadedList<Key> GetList(void) const {
		cweeThreadedList<Key> out;

		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			SharedLock();
			out = impl->indexList;
			SharedUnlock();
			return out;
		}
		int size = Num();

		Lock();
		impl->indexList.Clear();
		impl->indexList.SetGranularity(size + 16);
		for (auto& kv : impl->list) {
			impl->indexList.Append(kv.first);
		}
		impl->lastCreatedListVersion = impl->CreatedListVersion;

		out = impl->indexList;

		Unlock();
		return out;
	};
	/*! Erase the indexed object from the heap and free the memory for the list to re-use. */
	void			Erase(Key index) {
		Lock();
		if (impl->list.count(index) > 0) {
			impl->list[index] = nullptr;
			impl->list.erase(index);
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		Unlock();
	};
	/*! Clear the current list and free all memory back to the operating system */
	void			Clear(void) {
		impl->Clear();
	};
	/*!
	prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock
	A "Unlock" must be called to re-enable access to the list.
	*/
	void			Lock(void) const {
		impl->Lock();
	};
	/*!
	Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior.
	*/
	void			Unlock(void) const {
		impl->Unlock();
	};
	/*!
	prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope.
	*/
	NODISCARD auto	Guard(void) const { // assumes unlocked!
		return impl->Guard();
	}
	/*!
	After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping.
	*/
	PtrType			UnsafeRead(Key index) const {
		return impl->UnsafeRead(index);
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
	_iterType		UnsafeIterator(Key index) const {
		PtrType out;
		if (impl->list.count(index) <= 0) {
			out = UnsafeAppendAt(index);
		}
		else
		{
			out = impl->list[index];
		}
		return _iterType(index, out);
	};
	/*!
	After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list.
	*/
	cweeThreadedList<Key> UnsafeList(void) const {
		return impl->UnsafeList();
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			//Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			//Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<Key> SelectIndexes(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<Key> out;
		_iterType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//SharedLock();
			if (x && predicate(x.Ptr())) {
				out.Append(i);
			}
			//SharedUnlock();
		}
		return out;
	};
	/*!
	After calling "Lock", Append new data at a specified key location, regardless of the counter.
	*/
	PtrType			UnsafeAppendAt(Key index) const {
		if (impl->list.count(index) != 0) { return impl->list[index]; }

		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		/* Allocate a _type_ and store at specified index */
		AUTO newPtr = std::make_shared<Value>();
		impl->list.emplace(index, newPtr);
		return newPtr;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing.
	*/
	void			UnsafeErase(Key index) {
		if (impl->list.count(index) > 0) {
			impl->list.erase(index);
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtract(Key index, PtrType& out) {
		bool result = false;
		if (impl->list.count(index) > 0) {
			out = impl->list[index];
			impl->list.erase(index);
			result = true;
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		return result;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtractAny(PtrType& out) {
		bool result = false;
		for (auto& x : impl->list) {
			out = x.second;
			impl->list.erase(x.first);
			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
			return true;
		}
		return false;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	ExtractAny(Type& out) {
		bool res(false);
		Lock();
		AUTO bg = impl->list.cbegin();
		if (bg != impl->list.cend()) {
			if (bg->second) out = *bg->second;
			impl->list.erase(bg->first);
			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
			res = true;
		}
		Unlock();
		return res;
	};
	bool	Extract(Key index, PtrType& out) {
		bool result = false;
		AUTO G = Guard();
		result = UnsafeExtract(std::move(index), out);
		return result;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtractIndex(IdxType idx, PtrType& out) {
		_iterType p = unsafe_pair_at_index(idx);
		if (p.second) {
			UnsafeErase(p.first);
			out = p.second;
			return true;
		}
		return false;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	ExtractIndex(IdxType idx, PtrType& out) {
		bool result = false;
		AUTO G = Guard();
		result = UnsafeExtractIndex(std::move(idx), out);
		return result;
	};
public: // Compatability functions
	PtrType			at(Key index) const { return GetPtr(index); };
	PtrType			unsafe_at_index(IdxType idx) const {
		return unsafe_pair_at_index(idx).second;
	};
	_iterType		unsafe_pair_at_index(IdxType idx) const {
		_iterType out(-1, nullptr);
		bool foundKey = false;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			if (impl->indexList.Num() > idx) {
				out.first = impl->indexList[idx];
				foundKey = true;
			}
		}
		if (!foundKey) {
			auto _keys = UnsafeList();
			if (_keys.Num() > idx) {
				out.first = _keys[idx];
				foundKey = true;
			}
		}
		if (foundKey) {
			out.second = UnsafeGetPtr(out.first);
		}
		return out;
	};
	PtrType			at_index(IdxType idx) const {
		PtrType out;
		Lock();
		out = unsafe_pair_at_index(std::move(idx)).second;
		Unlock();
		return out;
	};
	_iterType		pair_at_index(IdxType idx) const {
		_iterType out;
		Lock();
		out = unsafe_pair_at_index(std::move(idx));
		Unlock();
		return out;
	};
	void			erase_at(IdxType idx) {
		AUTO p = pair_at_index(idx);
		if (p.second) {
			Erase(p.first);
		}
	};
	void			erase(Key index) {
		Erase(index);
	};
	bool			empty() const {
		return (size() == 0);
	};
	void			clear() {
		Clear();
	};
	_iterType		back() const {
		return pair_at_index(Num() - 1);
	};
	iterator		find(Key s) noexcept {
		iterator itr = iterator(this);
		itr.state.at(this, s);
		return itr;
	}
	iterator		find(Key s, const IdxType t_hint) noexcept {
		iterator itr = iterator(this);
		itr.state.at(this, s, t_hint);
		return itr;
	}
	const_iterator	find(Key s) const noexcept {
		const_iterator itr = const_iterator(this);
		itr.state.at(this, s);
		return itr;
	}
	const_iterator	find(Key s, const IdxType t_hint) const noexcept {
		const_iterator itr = const_iterator(this);
		itr.state.at(this, s, t_hint);
		return itr;
	}
	AUTO			size() const noexcept { return Num(); }
	template<typename M> AUTO insert_or_assign(Key&& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	template<typename M> AUTO insert_or_assign(const Key& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	IdxType count(const Key& s) const noexcept {
		return Check(s) ? 1 : 0;
		// return (find(s) != end()) ? 1 : 0;
	}
	std::pair<iterator, bool> insert(std::pair<Key, Value>&& value) {
		iterator out = iterator(this);
		Lock();
		bool didNotExist = UnsafeEmplace(value.first, value.second);
		Unlock();
		out.state.at(this, value.first);
		return std::pair(out, didNotExist);
	};
	template<typename M> AUTO insert(Key&& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	template<typename M> AUTO insert(const Key& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};

private:
	/*! Duplicate method to allow for advanced locking mechanisms. */
	void			SharedLock(void) const {
		impl->lock.Lock();
	};
	/*! Duplicate method to allow for advanced locking mechanisms. */
	void			SharedUnlock(void) const {
		impl->lock.Unlock();
	};

	ImplType impl;
};

extern void DoJob(ftl::TaskScheduler* taskScheduler, void* arg) {
	AUTO job = std::shared_ptr<cweeJob>(static_cast<cweeJob*>(arg));
	job->Invoke();
};
uint64_t FTL::fnFiberTasks3(int numTasks, int numSubTasks) {
	auto* fibers = &*Fibers; 
	std::shared_ptr<cweeFiberMutex> fibersLock = std::make_shared<cweeFiberMutex>();

	fiberMap<int, fiberMap <int, double>> lists;
	ftl::WaitGroup wg(fibers);

	std::vector<ftl::Task> tasks;
	for (int i = 0; i < numTasks; ++i) {	
		tasks.push_back({ DoJob, new cweeJob([&lists, &numSubTasks, &fibers, fibersLock, &wg](int j) {
			AUTO list = lists[j];
			if (numSubTasks <= 1) {
				*list->operator[](0) = fiberRandomFloat(0, 100);
			}
			else {
				std::vector<ftl::Task> tasks;

				//ftl::WaitGroup wg(fibers);
				for (int k = 0; k < numSubTasks; ++k) {
					tasks.push_back({ DoJob, new cweeJob([list](int index) {
					  *list->operator[](index) = fiberRandomFloat(0,100);
				  }, int(k)) });
				}
				fibersLock->Lock();
				fibers->AddTasks(numSubTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
				fibersLock->Unlock();
				//wg.Wait();
			}
		}, int(i)) });
	}
	fibersLock->Lock();
	fibers->AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
	fibersLock->Unlock();

	wg.Wait();

	int n = 0; 
	for (auto& x : lists) {
		for (auto& y : *x.second) {
			n++;
		}
	}

	return n;
};








template < typename T, typename... Args>
__forceinline cweeAction* ToAction(T function, Args... Fargs) {
	return new cweeAction(cweeFunction(std::function(function), Fargs...));
};
extern void DoAction(ftl::TaskScheduler* taskScheduler, void* arg) {
	AUTO job = std::shared_ptr<cweeAction>(static_cast<cweeAction*>(arg));
	job->Invoke();
};


template<typename T > class constexprDelayedInstantiation {
public:
	constexprDelayedInstantiation() : obj(nullptr), createFunc() {};
	constexprDelayedInstantiation(std::function<T* ()> create) : obj(nullptr), createFunc(create) {};
	~constexprDelayedInstantiation() {};

	T* operator->() { Ensure(); return obj.get(); };
	T& operator*() { Ensure(); return *obj; };

private:
	void Ensure() {
		if (!obj) {
			lock.Lock();
			auto* p = obj.get();
			if (!p) {
				obj = std::shared_ptr<T>(createFunc());
			}
			lock.Unlock();
		}
	};
	std::shared_ptr<T> obj;
	cweeConstexprLock lock;
	std::function< T* () > createFunc;
};
#include <random>
class constexpr_rand {
public:
	class cwee_pcg {
	public:
		using result_type = uint32_t;
		static constexpr result_type(min)() { return 0; }
		static constexpr result_type(max)() { return UINT32_MAX; }

		cwee_pcg() noexcept : m_state(0), m_inc(0), rd() { seed(); };
		void seed() const noexcept {
			uint64_t s0 = uint64_t(rd()) << 31 | uint64_t(rd());
			uint64_t s1 = uint64_t(rd()) << 31 | uint64_t(rd());
			m_state = 0;
			m_inc = (s1 << 1) | 1;
			(void)operator()();
			m_state += s0;
			(void)operator()();
		};
		result_type operator()() const noexcept {
			uint64_t oldstate = m_state;
			m_state = oldstate * 6364136223846793005ULL + m_inc;
			uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
			int rot = oldstate >> 59u;
			return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
		};
		void discard(unsigned long long n) const noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };

	private:
		mutable uint64_t m_state;
		mutable uint64_t m_inc;
		mutable std::random_device rd;
	};

private:
	mutable cweeConstexprLock mut;
	mutable cwee_pcg rand;
	mutable std::uniform_real_distribution<u64> u;

public:
	constexpr_rand() noexcept : mut(), rand(), u(0.0, 1.0) { Random_Impl(); /*Instantiate the range*/ };
	u64 Random(u64 t1 = 0.0, u64 t2 = 1.0) const noexcept { return Random_HighRes(std::move(t1), std::move(t2)); };
	double Random(double t1 = 0.0, double t2 = 1.0) const noexcept { return Random_HighRes(std::move(t1), std::move(t2)); };
	float Random(float t1 = 0.0, float t2 = 1.0) const noexcept { return Random_HighRes(std::move(t1), std::move(t2)); };
	int Random(int t1 = 0, int t2 = std::numeric_limits<int>::max()) const noexcept { return std::floor(Random_HighRes(std::move(t1), std::move(t2)) + 0.5); };

private:
	u64 Random_Impl() const noexcept {
		u64 out{ 0 };
		mut.Lock();
		out = u(rand);
		mut.Unlock();
		return out;
	};
	u64 Random_HighRes(u64 t1, u64 t2) const noexcept {
		t2 -= t1;
		t2 *= Random_Impl();
		t1 += t2;
		return t1;
	};

};
extern constexprDelayedInstantiation< constexpr_rand > constexprRandomGenerator = constexprDelayedInstantiation< constexpr_rand >([]()-> constexpr_rand* { return new constexpr_rand(); });

/*! random float between 0 and 1 */ INLINE float constexprRandomFloat() { return constexprRandomGenerator->Random(0.0f, 1.0f); };
/*! random float between 0 and max */ INLINE float constexprRandomFloat(float max) { return constexprRandomGenerator->Random(0.0f, max); };
/*! random float between min and max */ INLINE float constexprRandomFloat(float min, float max) { return constexprRandomGenerator->Random(min, max); };
/*! random int between 0 and cweeMath::INF */ INLINE int constexprRandomInt() { return constexprRandomGenerator->Random(0, std::numeric_limits<int>::max()); };
/*! random int between 0 and max */ INLINE int constexprRandomInt(int max) { return constexprRandomGenerator->Random(0, max); };
/*! random int between min and max */ INLINE int constexprRandomInt(int min, int max) { return constexprRandomGenerator->Random(min, max); };

/*! Thread-safe list that performs garbage collection and manages read/write/create/delete operations on data. Intended to act as a multi-threaded database. */
template< typename Key, typename Value>
class constexprMap {
public:
	typedef Value						Type;
	typedef std::shared_ptr< Type >		PtrType;
	typedef std::pair<Key, PtrType>		_iterType;
	typedef long long					IdxType;

	class constexprMap_Impl {
	public:
		/*! prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope. */
		NODISCARD auto	Guard(void) const { // assumes unlocked!
			return lock.Guard();
		}
		NODISCARD auto	SharedGuard(void) const { // assumes unlocked!
			return lock.Guard();
		}
		/*! Prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock -- A "Unlock" must be called to re-enable access to the list. */
		void			Lock(void) const { // assumes unlocked!
			lock.Lock();
		}
		/*! Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior. */
		void			Unlock(void) const { // assumes already locked! 
			lock.Unlock();
		};
		/*! After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping. */
		PtrType			UnsafeRead(Key index) const { // was non-const			
			auto it = list.find(index);
			if (it != list.end()) {
				return it->second;
			}
			return nullptr;
		}
		/*! After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list. */
		cweeThreadedList<Key> UnsafeList(void) const {
			cweeThreadedList<Key> out;

			if (lastCreatedListVersion == CreatedListVersion) {
				out = indexList;

				return out;
			}
			int size = list.size();

			indexList.Clear();
			indexList.SetGranularity(size + 16);
			for (auto& kv : list) indexList.Append(kv.first);
			lastCreatedListVersion = CreatedListVersion;

			out = indexList;

			return out;
		};
		AUTO			UnsafeIndexForKey(Key index) const {
			return std::distance(list.begin(), list.find(index));
		};

		/* Clear the list */
		void Clear() {
			AUTO g = Guard();

			list_version = 0;
			CreatedListVersion = 0;

			for (auto& kv : list) {
				PtrType& p = const_cast<PtrType&>(kv.second); // need to access the underlying thing
				p = nullptr;
			}
			list.clear();

			indexList.Clear();
			lastSearchID = Key();
			lastResult = list.end();
			lastVersion = -1;
			lastCreatedListVersion = -1;
		};

		constexprMap_Impl() : lock(), list(), indexList(), list_version(0), lastSearchID(), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1) {
			list.reserve(16);
			indexList.SetGranularity(16);
		};
		~constexprMap_Impl() {
			Clear();
		};

		/*! Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection */
		mutable cweeConstexprLock													lock; // cweeSysMutex
		/* Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions. */
		mutable tsl::robin_map<Key, PtrType, robin_hood::hash<Key>, std::equal_to<Key>, std::allocator<_iterType>, true>	list;
		/* Optimized search parameters */
		mutable cweeThreadedList<Key>												indexList;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											list_version;
		/* Optimized search parameters */
		mutable Key																	lastSearchID;
		/* Optimized search parameters */
		mutable typename decltype(list)::iterator		                            lastResult;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											lastVersion;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											CreatedListVersion;
		/* Optimized search parameters */
		mutable cweeSysInterlockedInteger											lastCreatedListVersion;
	};

	typedef std::shared_ptr<constexprMap_Impl>		ImplType;

	struct it_state {
		inline void at(const constexprMap* ref, Key index) {
			ref->Lock();
			AUTO h = ref->UnsafeIndexForKey(index);
			ref->Unlock();
			at(ref, index, h);
		};
		inline void at(const constexprMap* ref, Key index, const IdxType t_hint) {
			listOfIndexes = ref->GetList();
			idx = 0;
			IdxType n = listOfIndexes.Num();


			//idx = t_hint <= n ? t_hint : n;
			//return;

			for (idx = t_hint; idx < n; ++idx) {
				if (listOfIndexes[idx] == index) {
					return;
				}
			}
			for (idx = 0; idx < n && idx < t_hint; ++idx) {
				if (listOfIndexes[idx] == index) {
					return;
				}
			}
			idx = n;
		};
		mutable cweeThreadedList<Key>	listOfIndexes;
		mutable IdxType idx;
		mutable _iterType iter;
		inline void begin(const constexprMap* ref) {
			listOfIndexes = ref->GetList();
			idx = 0;
		}
		inline void next(const constexprMap* ref) {
			++idx;
			while (idx < listOfIndexes.Num() && !ref->Check(listOfIndexes[idx])) {
				++idx;
			}
		}
		inline void end(const constexprMap* ref) {
			listOfIndexes = ref->GetList();
			idx = listOfIndexes.Num();
		}
		inline _iterType& get(constexprMap* ref) {
			iter = ref->GetIterator(listOfIndexes[idx]);
			return iter;
		}
		inline bool cmp(const it_state& s) const { return (idx == s.idx) ? false : true; }
		inline IdxType distance(const it_state& s) const { return idx - s.idx; }

		// Optional to allow operator--() and reverse iterators:
		inline void prev(const constexprMap* ref) {
			--idx;
			while (idx >= 0 && !ref->Check(listOfIndexes[idx])) {
				--idx;
			}
		}
		// Optional to allow `const_iterator`:
		inline const _iterType& get(const constexprMap* ref) const { iter = ref->GetIterator(listOfIndexes[idx]); return iter; }
	};

	SETUP_STL_ITERATOR(constexprMap, _iterType, it_state);

	/*! Default constructor */
	constexprMap() : impl(new constexprMap_Impl()) {};
	/*! Default copy */
	constexprMap(const constexprMap& other) : impl(new constexprMap_Impl()) { this->Copy(other); };
	/*! Take reference to underlying data */
	constexprMap(constexprMap&& other) : impl(other.impl) {};
	/*! Default copy */
	constexprMap& operator=(const constexprMap& other) {
		this->Copy(other);
		return *this;
	};
	/*! Default destructor */
	~constexprMap() {
		impl = nullptr;
	};
	/*! Convert UnorderedMap to a list of keys */
	operator cweeThreadedList<Key>() const {
		return GetList();
	};
	/*! Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack. */
	PtrType			operator[](Key index) const { return GetPtr(index); };
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			Emplace(Key index, const Value& obj) {
		bool out;
		Lock();
		out = UnsafeEmplace(index, obj);
		Unlock();
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			Emplace(Key index, Value&& obj) {
		bool out;
		Lock();
		out = UnsafeEmplace(index, std::forward<Value>(obj));
		Unlock();
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			UnsafeEmplace(Key index, const Value& obj) {
		bool out;
		auto ptr = UnsafeRead(index);
		if (ptr) {
			// already exists
			*ptr = obj;
			out = false;
		}
		else {
			// does not yet exist
			ptr = UnsafeAppendAt(index);
			if (ptr) {
				*ptr = obj;
			}
			out = true;
		}
		return out;
	};
	/*! Swap if the object exists, otherwise append at the index and then swap. */
	bool			UnsafeEmplace(Key index, Value&& obj) {
		bool out;
		auto ptr = UnsafeRead(index);
		if (ptr) {
			// already exists
			*ptr = std::forward<Value>(obj);
			out = false;
		}
		else {
			// does not yet exist
			ptr = UnsafeAppendAt(index);
			if (ptr) {
				*ptr = std::forward<Value>(obj);
			}
			out = true;
		}
		return out;
	};
	/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying constexprMap lifetime does.   */
	PtrType			UnsafeGetPtr(Key index) const {
		if (impl->list.count(index) <= 0)
			return UnsafeAppendAt(index);
		else
			return impl->list[index];
	};
	/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying constexprMap lifetime does.   */
	PtrType			GetPtr(Key index) const {
		PtrType out{ TryGetPtr(index) };
		if (!out) {
			Lock();
			out = UnsafeAppendAt(index);
			Unlock();
		}
		return out;
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying constexprMap lifetime does. */
	PtrType			TryGetPtr(Key index) const {
		auto* p = impl.get();
		if (p) {
			AUTO g{ p->lock.Guard() };
			if (p->list.count(index) > 0) {
				return p->list.at(index);
			}
		}
		return nullptr;
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying constexprMap lifetime does. */
	_iterType		GetIterator(Key index) const {
		return _iterType(index, GetPtr(index));
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying constexprMap lifetime does. */
	_iterType		TryGetIterator(Key index) const {
		_iterType out;
		SharedLock();
		if (impl->list.count(index) > 0) {
			out = _iterType(index, impl->list[index]);
		}
		SharedUnlock();
		return out;
	};
	/*! True/False if the index exists on the heap */
	bool			Check(Key index) const {
		bool result = true;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			SharedLock();
			result = (impl->indexList.FindIndex(index) != -1);
			SharedUnlock();
			return result;
		}
		SharedLock();
		result = (impl->list.count(index) != 0);
		SharedUnlock();
		return result;
	};
	/*! True/False if the index exists on the heap */
	bool			UnsafeCheck(Key index) const {
		bool result = true;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			result = (impl->indexList.FindIndex(index) != -1);
			return result;
		}
		result = (impl->list.count(index) != 0);
		return result;
	};
	/*! Clear the current list and create a copy of the incoming list. */
	void			Copy(const constexprMap& copy, const bool threadSafePtr = false) {
		Clear();

		AUTO List = copy.GetList();

		if (threadSafePtr) {
			// Go through and perform swaps					
			for (int i = 0; i < List.Num(); i++) {
				copy.Lock();
				auto readCopy = copy.UnsafeRead(List[i]);
				copy.Unlock();

				if (readCopy) {
					Lock();
					UnsafeAppendAt(List[i]);
					auto access = UnsafeRead(List[i]);
					Unlock();

					if (access) *access = *readCopy;
				}
			}
		}
		else {
			// Go through and perform swaps		
			for (int i = 0; i < List.Num(); i++) {
				copy.Lock();
				auto readCopy = copy.UnsafeRead(List[i]);
				if (readCopy) {
					Lock();
					UnsafeAppendAt(List[i]);
					auto access = UnsafeRead(List[i]);
					if (access) *access = *readCopy;
					Unlock();
				}
				copy.Unlock();
			}
		}
	};
	/*! Copy the Value object from the stack into the heap at the index specified. If the index does not exist on the heap, nothing happens. */
	void			Swap(Key index, const Value& replacement) {
		if (Check(index)) {
			while (!CheckCanDeleteIndex(index)) {};
			Lock();
			AUTO it = impl->lastResult;
			if (impl->lastSearchID == index && impl->list.count(impl->lastSearchID) != 0 && impl->lastVersion == impl->list_version)	it = impl->lastResult;
			else { it = impl->list.find(index); impl->lastSearchID = index; impl->lastResult = it; impl->lastVersion = impl->list_version; }
			impl->list_version.Increment();
			if (it != impl->list.end()) *it->second = replacement;
			Unlock();
		}
	};
	/*! get the number of objects on the heap managed by this list */
	int				Num(void) const {
		int i = 0;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			SharedLock();
			i = impl->indexList.Num();
			SharedUnlock();
			return i;
		}

		SharedLock();
		i = impl->list.size();
		SharedUnlock();
		return i;
	};
	/*! get the number of objects on the heap managed by this list */
	int				UnsafeNum(void) const {
		int i = 0;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			i = impl->indexList.Num();
			return i;
		}
		i = impl->list.size();
		return i;
	};
	/*! get the index position of the key */
	AUTO			UnsafeIndexForKey(Key index) const {
		return impl->UnsafeIndexForKey(std::move(index));
	};
	/*! get a list of all indexes currently on the heap that are currently valid */
	cweeThreadedList<Key> GetList(void) const {
		cweeThreadedList<Key> out;

		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			SharedLock();
			out = impl->indexList;
			SharedUnlock();
			return out;
		}
		int size = Num();

		Lock();
		impl->indexList.Clear();
		impl->indexList.SetGranularity(size + 16);
		for (auto& kv : impl->list) {
			impl->indexList.Append(kv.first);
		}
		impl->lastCreatedListVersion = impl->CreatedListVersion;

		out = impl->indexList;

		Unlock();
		return out;
	};
	/*! Erase the indexed object from the heap and free the memory for the list to re-use. */
	void			Erase(Key index) {
		Lock();
		if (impl->list.count(index) > 0) {
			impl->list[index] = nullptr;
			impl->list.erase(index);
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		Unlock();
	};
	/*! Clear the current list and free all memory back to the operating system */
	void			Clear(void) {
		impl->Clear();
	};
	/*!
	prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock
	A "Unlock" must be called to re-enable access to the list.
	*/
	void			Lock(void) const {
		impl->Lock();
	};
	/*!
	Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior.
	*/
	void			Unlock(void) const {
		impl->Unlock();
	};
	/*!
	prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope.
	*/
	NODISCARD auto	Guard(void) const { // assumes unlocked!
		return impl->Guard();
	}
	/*!
	After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping.
	*/
	PtrType			UnsafeRead(Key index) const {
		return impl->UnsafeRead(index);
	};
	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying constexprMap lifetime does. */
	_iterType		UnsafeIterator(Key index) const {
		PtrType out;
		if (impl->list.count(index) <= 0) {
			out = UnsafeAppendAt(index);
		}
		else
		{
			out = impl->list[index];
		}
		return _iterType(index, out);
	};
	/*!
	After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list.
	*/
	cweeThreadedList<Key> UnsafeList(void) const {
		return impl->UnsafeList();
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			//Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			//Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<Key> SelectIndexes(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<Key> out;
		_iterType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//SharedLock();
			if (x && predicate(x.Ptr())) {
				out.Append(i);
			}
			//SharedUnlock();
		}
		return out;
	};
	/*!
	After calling "Lock", Append new data at a specified key location, regardless of the counter.
	*/
	PtrType			UnsafeAppendAt(Key index) const {
		if (impl->list.count(index) != 0) { return impl->list[index]; }

		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		/* Allocate a _type_ and store at specified index */
		AUTO newPtr = std::make_shared<Value>();
		impl->list.emplace(index, newPtr);
		return newPtr;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing.
	*/
	void			UnsafeErase(Key index) {
		if (impl->list.count(index) > 0) {
			impl->list.erase(index);
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtract(Key index, PtrType& out) {
		bool result = false;
		if (impl->list.count(index) > 0) {
			out = impl->list[index];
			impl->list.erase(index);
			result = true;
		}
		impl->list_version.Increment();
		impl->CreatedListVersion.Increment();
		return result;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtractAny(PtrType& out) {
		bool result = false;
		for (auto& x : impl->list) {
			out = x.second;
			impl->list.erase(x.first);
			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
			return true;
		}
		return false;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	ExtractAny(Type& out) {
		bool res(false);
		Lock();
		AUTO bg = impl->list.cbegin();
		if (bg != impl->list.cend()) {
			if (bg->second) out = *bg->second;
			impl->list.erase(bg->first);
			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
			res = true;
		}
		Unlock();
		return res;
	};
	bool	Extract(Key index, PtrType& out) {
		bool result = false;
		AUTO G = Guard();
		result = UnsafeExtract(std::move(index), out);
		return result;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	UnsafeExtractIndex(IdxType idx, PtrType& out) {
		_iterType p = unsafe_pair_at_index(idx);
		if (p.second) {
			UnsafeErase(p.first);
			out = p.second;
			return true;
		}
		return false;
	};
	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
	*/
	bool	ExtractIndex(IdxType idx, PtrType& out) {
		bool result = false;
		AUTO G = Guard();
		result = UnsafeExtractIndex(std::move(idx), out);
		return result;
	};
public: // Compatability functions
	PtrType			at(Key index) const { return GetPtr(index); };
	PtrType			unsafe_at_index(IdxType idx) const {
		return unsafe_pair_at_index(idx).second;
	};
	_iterType		unsafe_pair_at_index(IdxType idx) const {
		_iterType out(-1, nullptr);
		bool foundKey = false;
		if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
			if (impl->indexList.Num() > idx) {
				out.first = impl->indexList[idx];
				foundKey = true;
			}
		}
		if (!foundKey) {
			auto _keys = UnsafeList();
			if (_keys.Num() > idx) {
				out.first = _keys[idx];
				foundKey = true;
			}
		}
		if (foundKey) {
			out.second = UnsafeGetPtr(out.first);
		}
		return out;
	};
	PtrType			at_index(IdxType idx) const {
		PtrType out;
		Lock();
		out = unsafe_pair_at_index(std::move(idx)).second;
		Unlock();
		return out;
	};
	_iterType		pair_at_index(IdxType idx) const {
		_iterType out;
		Lock();
		out = unsafe_pair_at_index(std::move(idx));
		Unlock();
		return out;
	};
	void			erase_at(IdxType idx) {
		AUTO p = pair_at_index(idx);
		if (p.second) {
			Erase(p.first);
		}
	};
	void			erase(Key index) {
		Erase(index);
	};
	bool			empty() const {
		return (size() == 0);
	};
	void			clear() {
		Clear();
	};
	_iterType		back() const {
		return pair_at_index(Num() - 1);
	};
	iterator		find(Key s) noexcept {
		iterator itr = iterator(this);
		itr.state.at(this, s);
		return itr;
	}
	iterator		find(Key s, const IdxType t_hint) noexcept {
		iterator itr = iterator(this);
		itr.state.at(this, s, t_hint);
		return itr;
	}
	const_iterator	find(Key s) const noexcept {
		const_iterator itr = const_iterator(this);
		itr.state.at(this, s);
		return itr;
	}
	const_iterator	find(Key s, const IdxType t_hint) const noexcept {
		const_iterator itr = const_iterator(this);
		itr.state.at(this, s, t_hint);
		return itr;
	}
	AUTO			size() const noexcept { return Num(); }
	template<typename M> AUTO insert_or_assign(Key&& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	template<typename M> AUTO insert_or_assign(const Key& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	IdxType count(const Key& s) const noexcept {
		return Check(s) ? 1 : 0;
		// return (find(s) != end()) ? 1 : 0;
	}
	std::pair<iterator, bool> insert(std::pair<Key, Value>&& value) {
		iterator out = iterator(this);
		Lock();
		bool didNotExist = UnsafeEmplace(value.first, value.second);
		Unlock();
		out.state.at(this, value.first);
		return std::pair(out, didNotExist);
	};
	template<typename M> AUTO insert(Key&& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};
	template<typename M> AUTO insert(const Key& key, M&& m) {
		Emplace(key, m);
		return GetPtr(key);
	};

private:
	/*! Duplicate method to allow for advanced locking mechanisms. */
	void			SharedLock(void) const {
		impl->lock.Lock();
	};
	/*! Duplicate method to allow for advanced locking mechanisms. */
	void			SharedUnlock(void) const {
		impl->lock.Unlock();
	};

	ImplType impl;
};


uint64_t FTL::fnFiberTasks4(int numTasks, int numSubTasks) {
	constexprMap<int, constexprMap <int, double>> lists;

	if (true) {
		ftl::TaskScheduler fibers;
		fibers.Init({ (unsigned)std::max(200, numTasks+20), 0, ftl::EmptyQueueBehavior::Yield, {}}); // num fibers == num "wait" caches?
		std::shared_ptr<cweeConstexprLock> fibersLock = std::make_shared<cweeConstexprLock>();

		ftl::WaitGroup wg(&fibers);

		std::vector<ftl::Task> tasks;
		for (int i = 0; i < numTasks; ++i) {
			tasks.push_back({ DoAction, ToAction([&lists, numSubTasks, &fibers, fibersLock, &wg](int j) {
				AUTO list = lists[j];

				if (numSubTasks <= 1) {
					*list->operator[](0) = constexprRandomFloat(0, 100);
				}
				else {
					std::vector<ftl::Task> tasks;

					//ftl::WaitGroup wg(&fibers);					
					for (int k = 0; k < numSubTasks; ++k) {
						tasks.push_back({ DoAction, ToAction([list](int index) {
							*list->operator[](index) = constexprRandomFloat(0,100);
						}, int(k)) });
					}
					fibersLock->Lock();
					fibers.AddTasks(numSubTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
					fibersLock->Unlock();
					//wg.Wait();
				}				
			}, int(i)) });
		}

		fibersLock->Lock();
		fibers.AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
		fibersLock->Unlock();

		wg.Wait();
	}

	int n = 0; 
	for (auto& x : lists) {
		for (auto& y : *x.second) {
			n++;
		}
	}

	return n;
};