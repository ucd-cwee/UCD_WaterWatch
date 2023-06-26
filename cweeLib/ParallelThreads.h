#ifndef __PARALLEL_THREADS_H__
#define __PARALLEL_THREADS_H__

#pragma hdrstop


typedef void (*jobRun_t)(void*);

enum jobType {
	IO_thread = 0,
	SIM_thread = 1,
	OPT_thread = 2,
	PRIORITY_thread = 3,
	SCRIPT_thread = 4,
	thread5,
	thread6,
	thread7,
	thread8,
	thread9,
	thread10,
	thread11,
	thread12,
	thread13,
	thread14,
	thread15,
	thread16,
	thread17,
	thread18,
	thread19,
	thread20,
	thread21,
	thread22,
	thread23,
	thread24,
	thread25,
	thread26,
	thread27,
	thread28,
	thread29,
	thread30,
	thread31,
	thread32,
	MAX_NUM_PARALLEL_THREADS = 5000				// Must be a large value
};

class cweeJob;

struct jobHandle {
	jobType thread = jobType::MAX_NUM_PARALLEL_THREADS;
	jobRun_t function = nullptr;

	// cweeJob Function;

	uint64 startTime = 0;
	uint64 endTime = 0;
	int handleNum = -1;
	int cpuCore = -1;
};

class CweeJobContainer;
static void RunCweeJobInParallel(CweeJobContainer* job);

class cweeparallel_ThreadList;
class cweeparallelProcessor;
extern cweeparallelProcessor* parallelProcessor; // global object that handles multithreading, task management, and more. 

class cweeJob {
public:
	class cweeJob_Impl {
	public:
		cweeJob_Impl() : todo(new cweeAction()), _ContinueWith(make_cwee_shared<cweeThreadedList < cweeJob >>()) {};
		cweeJob_Impl(const cweeJob_Impl& other) : todo(other.todo), _ContinueWith(other._ContinueWith) {};
		cweeJob_Impl(cweeJob_Impl&& other) : todo(other.todo), _ContinueWith(other._ContinueWith) {};

		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<cweeJob_Impl, std::decay_t<T>> && !std::is_same_v<cweeAny, std::decay_t<T>> >>
		explicit cweeJob_Impl(T function, Args... Fargs) : todo(new cweeAction(cweeFunction(std::function(function), Fargs...))), _ContinueWith(make_cwee_shared<cweeThreadedList < cweeJob >>()){};

		cweeJob_Impl& operator=(const cweeJob_Impl& other) {
			todo = other.todo;
			_ContinueWith = other._ContinueWith;

			return *this;
		};
		cweeJob_Impl& operator=(cweeJob_Impl&& other) {
			todo = cweeSharedPtr < cweeAction >().Swap(other.todo);
			_ContinueWith = decltype(_ContinueWith)().Swap(other._ContinueWith);

			return *this;
		};

		cweeAny Invoke(int iterationNumber = 0) {
			cweeAny out; bool previouslyFinished(true);
			AUTO job = todo.Get();
			if (job) {
				previouslyFinished = job->IsFinished();
				AUTO reply = job->Invoke(iterationNumber);
				if (reply) {			
					out = *reply;
				}
			}
			
			if (!previouslyFinished) {
				cweeThreadedList < cweeJob > L;
				_ContinueWith.Lock();
				AUTO toLoop = _ContinueWith.UnsafeGet();
				if (toLoop) L = *toLoop;				
				_ContinueWith.Unlock();

				if (L.Num() > 0) {
					for (auto& j : L) {
						j.Invoke();
					}
				}
			}

			return out;
		};
		cweeAny ForceInvoke(int iterationNumber = 0) {
			cweeAny out; 
			AUTO job = todo.Get();
			if (job) {
				AUTO reply = job->ForceInvoke(iterationNumber);
				if (reply) {
					out = *reply;
				}
			}
			if (true) {
				cweeThreadedList < cweeJob > L;
				_ContinueWith.Lock();
				AUTO toLoop = _ContinueWith.UnsafeGet();
				if (toLoop) L = *toLoop;
				_ContinueWith.Unlock();

				if (L.Num() > 0) {
					for (auto& j : L) {
						j.ForceInvoke();
					}
				}
			}
			return out;
		};
		cweeAny GetResult() {
			return Invoke();
		};
		cweeAny operator()() {
			return Invoke();
		};

		bool IsFinished() const {
			AUTO job = todo.Get();
			if (job) {
				return job->IsFinished();
			}
			return true;
		};

		cweeJob ContinueWith(const cweeJob& next) {
			if (IsFinished()) {
				const_cast<cweeJob&>(next).Invoke();
			}
			_ContinueWith.Lock();
			AUTO cont = _ContinueWith.UnsafeGet();
			if (cont) {
				cont->Append(next);
			}
			_ContinueWith.Unlock();
			return next;
		};
		cweeJob ContinueWith(cweeJob&& next) {
			if (IsFinished()) {
				next.Invoke();
			}
			_ContinueWith.Lock();
			AUTO cont = _ContinueWith.UnsafeGet();
			if (cont) {
				cont->Append(std::forward<cweeJob>(next));
			}
			_ContinueWith.Unlock();
			return next;
		};

		template < typename T, typename... Args, typename = std::enable_if_t<!std::is_same_v<cweeJob_Impl, std::decay_t<T>>>>
		cweeJob ContinueWith(T function, Args... Fargs) {
			return ContinueWith(cweeJob_Impl(function, Fargs...));
		};

		const char* FunctionName() const {
			AUTO job = todo.Get();
			if (job) {
				return job->FunctionName();
			}
			return cweeStr("No Function").c_str();
		};

	public:
		cweeSharedPtr < cweeAction > todo;
		cweeSharedPtr < cweeThreadedList < cweeJob > > _ContinueWith;
	};
	
	static cweeJob Finished() {
		AUTO toReturn = cweeJob();

		AUTO ptrp = toReturn.GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			ptr->todo = cweeSharedPtr<cweeAction>(new cweeAction(cweeAction::Finished()));
		}

		return toReturn;
	};
	template <typename T> static cweeAction Finished(const T& returnMe) {
		AUTO toReturn = cweeJob();

		AUTO ptrp = toReturn.GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			ptr->todo = std::make_shared<cweeAction>(cweeAction::Finished(returnMe));
		}

		return toReturn;
	};

public:
	cweeJob() : impl(new cweeJob_Impl()), syncContext(nullptr) {};
	cweeJob(const cweeJob& other) : impl(other.GetImpl()), syncContext(other.syncContext) {};
	cweeJob(cweeJob&& other) : impl(other.GetImpl()), syncContext(other.syncContext) {};
	cweeJob& operator=(const cweeJob& other) {
		syncContext = other.syncContext;
		impl = other.GetImpl();
		return *this;
	};
	cweeJob& operator=(cweeJob&& other) {
		syncContext = other.syncContext;
		impl = other.GetImpl();
		return *this;
	};

	template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<cweeJob, std::decay_t<T>> && !std::is_same_v<cweeAny, std::decay_t<T>> >>
	explicit cweeJob(T function, Args... Fargs) : impl(new cweeJob_Impl(function, Fargs...)), syncContext(nullptr) {};

	cweeJob& AsyncInvoke();
	cweeJob& AsyncInvoke(jobType type);
	cweeJob& AsyncInvoke(jobType type, jobListPriority_t priority);	

	cweeJob& AsyncForceInvoke();
	cweeJob& AsyncForceInvoke(jobType type);
	cweeJob& AsyncForceInvoke(jobType type, jobListPriority_t priority);

	uintptr_t DelayedAsyncInvoke(u64 milliseconds_delay);
	uintptr_t DelayedAsyncInvoke(u64 milliseconds_delay, jobType type);
	uintptr_t DelayedAsyncForceInvoke(u64 milliseconds_delay);
	uintptr_t DelayedAsyncForceInvoke(u64 milliseconds_delay, jobType type);

	cweeAny Invoke(int iterationNumber = 0) {
		AUTO ptr = GetImpl();
		AUTO ptrI = ptr.Get();
		if (ptrI) {
			return ptrI->Invoke(iterationNumber);
		}
		return cweeAny();
	};
	cweeAny ForceInvoke(int iterationNumber = 0) {
		AUTO ptr = GetImpl();
		AUTO ptrI = ptr.Get();
		if (ptrI) {
			return ptrI->ForceInvoke(iterationNumber);
		}
		return cweeAny();
	};
	cweeAny GetResult() const {
		AUTO ptr = GetImpl();
		AUTO ptrR = ptr.Get();
		if (ptrR) {
			return ptrR->GetResult();
		}
		return cweeAny();
	};
	cweeAny operator()() {
		return Invoke();
	};

	bool IsFinished() const {
		AUTO ptrp = GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			return ptr->IsFinished();
		}
		return false;	
	};

	cweeAny Await();
	cweeAny AwaitAll() {
		Await();

		AUTO ptrAp = GetImpl();
		AUTO ptrA = ptrAp.Get();
		if (ptrA) {
			cweeThreadedList < cweeJob > L;
			ptrA->_ContinueWith.Lock();
			AUTO toLoop = ptrA->_ContinueWith.UnsafeGet();
			if (toLoop) L = *toLoop;
			ptrA->_ContinueWith.Unlock();

			if (L.Num() > 0) {
				for (auto& j : L) {
					j.AwaitAll();
				}
			}
		}

		return std::move(GetResult());
	};

	cweeJob ContinueWith(const cweeJob& next) {
		AUTO ptrp = GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			return ptr->ContinueWith(next);
		}
		return next;
	};
	cweeJob ContinueWith(cweeJob&& next) {
		AUTO ptrp = GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			return ptr->ContinueWith(std::forward<cweeJob>(next));
		}
		return next;
	};
	template < typename T, typename... Args, typename = std::enable_if_t<!std::is_same_v<cweeJob, std::decay_t<T>>>>
	cweeJob ContinueWith(T function, Args... Fargs) {
		return ContinueWith(cweeJob(function, Fargs...));
	};
	const char* FunctionName() const {
		AUTO ptr = GetImpl();
		AUTO ptrF = ptr.Get();
		if (ptrF) {
			return ptrF->FunctionName();
		}
		return "Unknown Function";
	};

public: // part of the cweeParallelJobSystem
	int*  syncContext;

protected:
	mutable cweeSharedPtr< cweeJob_Impl > impl;

private:
	cweeSharedPtr< cweeJob_Impl > GetImpl() const {
		return impl;
	};
};




class cweeparallel_ThreadList {
	friend class cweeparallelProcessorLocal;
public:
	int										GetNumExecutingThreads();
	void									AllowMultithreading(bool T_or_F);
	int										GetMultithreadingSetting(void);
	void									AddParallelJob(void* function, void* data);
	void									AddParallelJob(jobRun_t function, void* data);
	void									InsertSyncPoint(jobSyncType_t syncType);
	void									Submit(cweeparallel_ThreadList* waitForJobList = nullptr, int parallelism = JOBLIST_PARALLELISM_DEFAULT); 	// Submit the jobs in this list.
	void									Wait();												// Wait for the jobs in this list to finish. Will spin in place if any jobs are not done.
	bool									TryWait();											// Try to wait for the jobs in this list to finish but either way return immediately. Returns true if all jobs are done.
	bool									IsSubmitted() const;								// returns true if the job list has been submitted.
	unsigned int							GetNumExecutedJobs() const;							// Get the number of jobs executed in this job list.
	unsigned int							GetNumSyncs() const;								// Get the number of sync points.
	uint64									GetSubmitTimeMicroSec() const;						// Time at which the job list was submitted.
	uint64									GetStartTimeMicroSec() const;						// Time at which execution of this job list started.
	uint64									GetFinishTimeMicroSec() const;						// Time at which all jobs in the list were executed.
	uint64									GetWaitTimeMicroSec() const;						// Time the host thread waited for this job list to finish.
	uint64									GetTotalProcessingTimeMicroSec() const;				// Get the total time all units spent processing this job list.
	uint64									GetTotalWastedTimeMicroSec() const;					// Get the total time all units wasted while processing this job list.
	uint64									GetUnitProcessingTimeMicroSec(int unit) const;		// Time the given unit spent processing this job list.
	uint64									GetUnitWastedTimeMicroSec(int unit) const;			// Time the given unit wasted while processing this job list.
	const jobType&							GetId() const;										// Get the job list ID
	const jobListPriority_t&				GetPriority() const;
	int										GetNumStagedJobs() const {
		return jobBuffer.Num();
	}

private:
	cweeThreadedList<std::tuple<jobRun_t, void*, jobSyncType_t>> jobBuffer;
	class cweeparallel_JobList_Threads* jobListThreads;
	cweeparallel_ThreadList(jobType id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs, cweeUnorderedList< jobHandle >* JobHandlesList);
	~cweeparallel_ThreadList();
};

class cweeparallelProcessor {
public: // construction and destruction
	/*!
	Destroy the parallel thread manager.
	*/
	virtual							~cweeparallelProcessor() {}

public: // required methods
	/*!
	Initialize the parallel thread manager.
	*/
	virtual void					Init() = 0;

	/*!
	Allow dynamically allocated tasks to be processed and performed per logical cycle of the application. Call this every 0.2 seconds or so.
	*/
	virtual void					Think() = 0; // Run once a cycle to allow deffered work to be performed.

	/*!
	Shutdown the parallel thread manager.
	*/
	virtual void					Shutdown() = 0;

public: // basic settings
	/*!
	Allow dynamic multithreading using logical cores or force single-threading
	*/
	virtual void					AllowMultithreading(bool T_or_F) = 0;

	/*!
	Set or change the number of threads to be utilized by the parallel processor. In-progress tasks may need to finish before change takes effect.
	Must be between 0 and the number of logical cores on machine.
	*/
	virtual void					SetNumThreads(int num) = 0;

	/*!
	Get current number of threads being used. Returns 0 if multithreading is off and single-threading is active.
	*/
	virtual int						GetMultithreadingSetting() = 0;

	/*!
	Get current number of processing units on machine
	*/
	virtual int						GetNumProcessingUnits() = 0;

	/*!
	Get current number of physical cores on machine
	*/
	virtual int						GetNumPhysicalCpuCores() = 0;

	/*!
	Get current number of logical cores on machine
	*/
	virtual int						GetNumLocalCpuCores() = 0;

	/*!
	Get current number of cpu packages on machine
	*/
	virtual int						GetNumCpuPackages() = 0;

public: // advanced interface
	/*
	Create a new "job list" which will compete for work with other threads.
	*/
	virtual cweeparallel_ThreadList* AllocJobList(jobType id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs) = 0;

	/*
	Spin in place until all jobs (including not-yet-submitted ones) are complete
	*/
	virtual void					WaitForAllJobLists() = 0;

	/*
	Returns true if all jobs are complete. Returns false otherwise.
	*/
	virtual bool					TryWaitAllJobLists() = 0;
public: // internal designer interface

	virtual cweeparallel_ThreadList* GetJobList(int index) = 0;
	virtual cweeparallel_ThreadList* GetJobList(jobType id, jobListPriority_t priority = jobListPriority_t::JOBLIST_PRIORITY_MEDIUM) = 0;
	virtual int						GetNumJobLists() const = 0;
	virtual void					FreeJobList(cweeparallel_ThreadList* jobList) = 0;
	virtual int						GetNumFreeJobLists() const = 0;

	virtual void					UpdateStates() = 0;
	virtual int						GetCumulativeJobsExecuted() const = 0;
	virtual int						GetCumulativeProcessingTimeMicroSec() const = 0;
	virtual int						GetNumStagedJobs() const = 0;
	virtual int						GetTotalProcessingTimeMicroSec() const = 0;
	virtual int						GetTotalJobsExecuted() const = 0;

	void							Submit(cweeparallel_JobList_Threads* jobList, int parallelism);

	cweeUnorderedList< jobHandle >	JobHandles;
	virtual cweeStr					GetJobName(jobRun_t func) = 0;
	virtual cweeStr					GetThreadName(jobType id) = 0;

	virtual	void					SetJobAssignmentMode(int in) = 0;
	virtual int						GetJobAssignmentMode() = 0;
};






/*
================================================
cweeparallel_OPT_ThreadRegistration
================================================
*/
void Register_Thread_Job(jobRun_t function, const char* name);
class cweeparallel_ThreadRegistration {
public:
	cweeparallel_ThreadRegistration(jobRun_t function, const char* name);
};

#define STR(x) _STR(x)
#define _STR(x) #x
#define ID(x) x
#define STR_FUNC(x) STR(x)
#define REGISTER_WITH_PARALLEL_THREAD( function )		static cweeparallel_ThreadRegistration register_##function( (jobRun_t) (void*) function, STR_FUNC(function) )


class CweeJobContainer {
public:
	cweeJob task;
};
INLINE static void RunCweeJobInParallel(CweeJobContainer* job) {
	if (job) {
		job->task.Invoke();
		delete job;
	}
};
REGISTER_WITH_PARALLEL_THREAD(RunCweeJobInParallel);
INLINE static void ForceRunCweeJobInParallel(CweeJobContainer* job) {
	if (job) {
		job->task.ForceInvoke();
		delete job;
	}
};
REGISTER_WITH_PARALLEL_THREAD(ForceRunCweeJobInParallel);

class cweeJobThreads {
public:
	typedef cweeThreadedMap<u64, cweeJob> jobListType;
	class cweeJobThreadsData {
	public:
		class cweeCpuThread {
		public:
			class cweeCpuThreadData {
			private:
				cweeSysInterlockedInteger														m_Terminate;
				cweeSysSignal																	m_Signal;
				cweeSysInterlockedInteger														m_Waiting;
				cweeSysInterlockedInteger														m_Running;
				cweeUnpooledInterlocked<cweeUnion<cweeStr, cweeSysInterlockedPointer<void>>>	m_Content;
				cweeSharedPtr<jobListType>														m_SharedJobs;
				cweeSharedPtr<cweeThreadedMap<int, cweeUnion<cweeSharedPtr<jobListType>, cweeCpuThread>>> m_taggedThreads;
				cweeSharedPtr<cweeSysInterlockedInteger>										m_NumActiveThreads;

			public:
				auto	UniqueName() const {
					m_Content.Lock();
					auto out = m_Content.UnsafeRead()->get<0>();
					m_Content.Unlock();
					return out;
				};
				static void ThreadDoWork(cweeCpuThreadData* threadData, cweeJob& todo) {
					// primary responsibility... 
					while (threadData->TryExtractNextJob(threadData->m_SharedJobs, todo)) { // for any and all jobs in the queue that we are sharing...						
						todo.Invoke(); // do the work. 						
					}
					
					// secondary responsibility...
					if (threadData->m_taggedThreads) {
						for (auto& tag : *threadData->m_taggedThreads) {
							while (threadData->TryExtractNextJob(tag.second->get<0>(), todo)) {
								todo.Invoke(); // do the work.
								todo = cweeJob(); // forget about the work.
							}
						}
					}
				};
				static void ThreadSleep(cweeCpuThreadData* threadData) {
					threadData->m_Waiting.Increment();
					threadData->m_Signal.Wait(cweeSysSignal::WAIT_INFINITE);
					threadData->m_Waiting.Decrement();
				};
				static int ThreadProc(cweeCpuThreadData* threadData) {
					cweeJob todo;
					while (1) {
						ThreadSleep(threadData);
						if (threadData->m_Terminate.Decrement() == 0) break;
						else threadData->m_Terminate.Increment();		

						ThreadDoWork(threadData, todo);
						
						threadData->m_Signal.Clear();
					}
					threadData->m_Running.Decrement();
					threadData->m_NumActiveThreads->Decrement();
					return 0;
				};
				static bool	TryExtractNextJob(cweeSharedPtr<jobListType> jobs, cweeJob& nextJob) {
					cweeSharedPtr<cweeJob> j;
					bool out = jobs->ExtractIndex(0, j);
					if (j) {
						nextJob = *j;
					}
					return out;
				};

			private:
				void	TryStartThread() {
					m_Signal.Raise();
				};
				void	ClearThreadHandle() {
					m_Content.Lock();
					uintptr_t out = (uintptr_t)m_Content.UnsafeRead()->get<1>().Set(nullptr);
					m_Content.Unlock();
					if (out != 0) {
						m_Terminate.Increment(); // thread will end soon, and will set this to 0 when done.
						Sys_DestroyThread(out); // an old thread is being 'replaced' by this new one... spin-wait for it to die. 
					}
				};

			public:
				explicit cweeCpuThreadData(cweeSharedPtr<jobListType> const& p_sharedJobs, cweeSharedPtr<cweeThreadedMap<int, cweeUnion<cweeSharedPtr<jobListType>, cweeCpuThread>>> p_taggedThreads, cweeSharedPtr<cweeSysInterlockedInteger> numActive) :
					m_Waiting(0), 
					m_Terminate(0),
					m_Signal(true),
					m_Running(1),
					m_Content(cweeUnion<cweeStr, cweeSysInterlockedPointer<void>>(cweeStr::printf("cweeCpuThreadData_%i_%i", cweeRandomInt(0, 10000), cweeRandomInt(0, 10000)), nullptr)),
					m_SharedJobs(p_sharedJobs),
					m_NumActiveThreads(numActive),
					m_taggedThreads(p_taggedThreads)
				{
					m_Content = cweeUnion<cweeStr, cweeSysInterlockedPointer<void>>(
						UniqueName(),
						(void*)Sys_CreateThread((xthread_t)ThreadProc, this, xthreadPriority::THREAD_NORMAL, UniqueName() + cweeStr::printf("_%i", cweeRandomInt(0, 10000)), core_t::CORE_ANY, 512 * 1024, false)
					);
				};
				void WakeUp() {
					TryStartThread();
				};
				bool IsRunning() {
					return (m_Waiting.GetValue() != 1);
				};
				~cweeCpuThreadData() {
					ClearThreadHandle();
				};
			};

		private:
			cweeSharedPtr<cweeCpuThreadData>	m_Data;
			cweeCpuThread& swap(cweeCpuThread& a) {
				if (&a == this) { return *this; }
				m_Data.swap(a.m_Data);
				return *this;
			};

		public:
			cweeCpuThread() : m_Data(nullptr) {};
			explicit cweeCpuThread(cweeSharedPtr<jobListType> const& p_sharedJobs, cweeSharedPtr<cweeThreadedMap<int, cweeUnion<cweeSharedPtr<jobListType>, cweeCpuThread>>> p_taggedThreads, cweeSharedPtr<cweeSysInterlockedInteger> numActive) :
				m_Data(make_cwee_shared<cweeCpuThreadData>(p_sharedJobs, p_taggedThreads, numActive))
			{};
			cweeCpuThread(cweeCpuThread const& a) : m_Data(a.m_Data) {};
			cweeCpuThread(cweeCpuThread&& a) : m_Data(a.m_Data) { a.m_Data = nullptr; };
			cweeCpuThread& operator=(cweeCpuThread const& a) {
				m_Data = a.m_Data;
				return *this;
			};
			cweeCpuThread& operator=(cweeCpuThread&& a) {
				m_Data = a.m_Data;
				return *this;
			};
			~cweeCpuThread() {};

		public:
			void WakeUp() {
				cweeSharedPtr<cweeCpuThreadData>	data = m_Data;
				cweeCpuThreadData* p = data.Get();
				if (p) {
					p->WakeUp();
				}
			};
			bool IsRunning() {
				cweeSharedPtr<cweeCpuThreadData>	data = m_Data;
				cweeCpuThreadData* p = data.Get();
				if (p) {
					return p->IsRunning();
				}
				return false;
			};
		};
		class CPU_DATA {
		public:
			CPU_DATA() : m_numPhysicalCpuCores(0), m_numLogicalCpuCores(0), m_numCpuPackages(0) { InitCpuData(); };
			struct cweeCpuInfo_t {
				int processorPackageCount;
				int processorCoreCount;
				int logicalProcessorCount;
				int numaNodeCount;
				struct cacheInfo_t {
					int count;
					int associativity;
					int lineSize;
					int size;
				} cacheLevel[3];
			};
			static DWORD CountSetBits(ULONG_PTR bitMask) {
				DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
				DWORD bitSetCount = 0;
				ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

				for (DWORD i = 0; i <= LSHIFT; i++) {
					bitSetCount += ((bitMask & bitTest) ? 1 : 0);
					bitTest /= 2;
				}

				return bitSetCount;
			};
			enum e_LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL {
				e_localRelationProcessorCore,
				e_localRelationNumaNode,
				e_localRelationCache,
				e_localRelationProcessorPackage
			};
			static bool GetCPUInfo(cweeCpuInfo_t& cpuInfo) {
				typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

				PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr; // NULL
				PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = nullptr; // NULL
				PCACHE_DESCRIPTOR Cache;
				LPFN_GLPI	glpi;
				BOOL		done = FALSE;
				DWORD		returnLength = 0;
				DWORD		byteOffset = 0;

				::memset(&cpuInfo, 0, sizeof(cpuInfo));

				glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
				if (NULL == glpi) {
					return 0;
				}

				while (!done) {
					DWORD rc = glpi(buffer, &returnLength);

					if (FALSE == rc) {
						if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
							if (buffer) {
								free(buffer);
							}

							buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
						}
						else {
							return false;
						}
					}
					else {
						done = TRUE;
					}
				}

				ptr = buffer;

				while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
					switch ((e_LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL)ptr->Relationship) {
					case e_localRelationProcessorCore: // A hyperthreaded core supplies more than one logical processor.
						cpuInfo.processorCoreCount++;
						cpuInfo.logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
						break;

					case e_localRelationNumaNode: // Non-NUMA systems report a single record of this type.
						cpuInfo.numaNodeCount++;
						break;

					case e_localRelationCache: // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
						Cache = &ptr->Cache;
						if (Cache->Level >= 1 && Cache->Level <= 3) {
							int level = Cache->Level - 1;
							if (cpuInfo.cacheLevel[level].count > 0) {
								cpuInfo.cacheLevel[level].count++;
							}
							else {
								cpuInfo.cacheLevel[level].associativity = Cache->Associativity;
								cpuInfo.cacheLevel[level].lineSize = Cache->LineSize;
								cpuInfo.cacheLevel[level].size = Cache->Size;
							}
						}
						break;

					case e_localRelationProcessorPackage: // Logical processors share a physical package.
						cpuInfo.processorPackageCount++;
						break;

					default:
						break;
					}
					byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
					ptr++;
				}

				free(buffer);

				return true;
			};
			void InitCpuData() {
				cweeCpuInfo_t cpuInfo;
				GetCPUInfo(cpuInfo);

				this->m_numPhysicalCpuCores = cpuInfo.processorCoreCount;
				this->m_numLogicalCpuCores = cpuInfo.logicalProcessorCount;
				this->m_numCpuPackages = cpuInfo.processorPackageCount;
			};
		
		public:
			int										m_numPhysicalCpuCores;
			int										m_numLogicalCpuCores;
			int										m_numCpuPackages;
		};

	public:
		cweeJobThreadsData(CPU_DATA const& d = CPU_DATA()) :
			m_numPhysicalCpuCores(d.m_numPhysicalCpuCores),
			m_numLogicalCpuCores(d.m_numLogicalCpuCores),
			m_numCpuPackages(d.m_numCpuPackages),
			m_Jobs(make_cwee_shared<jobListType>()),
			m_NumActiveThreads(make_cwee_shared<cweeSysInterlockedInteger>(0)),
			m_Threads(),
			m_numJobs(0), 
			m_taggedThreads(make_cwee_shared<cweeThreadedMap<int, cweeUnion<cweeSharedPtr<jobListType>, cweeCpuThread>>>())
		{
			m_Threads.Append(cweeCpuThread(m_Jobs, m_taggedThreads, m_NumActiveThreads)); // minimum of one thread
			for (int i = 1; i < m_numLogicalCpuCores - 1; i++) { // one thread less to allow for the "owner" to run
				m_Threads.Append(cweeCpuThread(m_Jobs, m_taggedThreads, m_NumActiveThreads));
			}
		};
		cweeJobThreadsData(cweeJobThreadsData const&) = delete;
		cweeJobThreadsData(cweeJobThreadsData&&) = delete;
		cweeJobThreadsData& operator=(cweeJobThreadsData const&) = delete;
		cweeJobThreadsData& operator=(cweeJobThreadsData&&) = delete;
		~cweeJobThreadsData() {
			Wait();
		};
	
	public:
		void WakeUpAll() {
			int n = m_Jobs->Num();
			if (m_taggedThreads) {
				for (auto& tag : *m_taggedThreads) {
					if (tag.second->get<0>()->Num() > 0) {
						tag.second->get<1>().WakeUp();
						n += tag.second->get<0>()->Num();
					}
				}
			}
			for (auto& thread : m_Threads) {
				if (--n >= 0) {
					thread.WakeUp();
				}
			}
		};
		void AddTask(cweeJob const& j) {
			m_Jobs->Emplace(m_numJobs.Increment(), j);
			WakeUpAll();
		};
		void AddTask(cweeJob const& j, int tag) {
			{
				{
					AUTO g = m_taggedThreads->Guard();
					if (!m_taggedThreads->UnsafeCheck(tag)) {
						AUTO p = m_taggedThreads->UnsafeAppendAt(tag);
						p->get<0>() = jobListType();
						p->get<1>() = cweeCpuThread(p->get<0>(), nullptr, m_NumActiveThreads); // nullptr because these threads are designed to only support themselves - not each other. The generic threads do that.
					}
				}
				AUTO p = m_taggedThreads->GetPtr(tag);
				p->get<0>()->Emplace(m_numJobs.Increment(), j);
			}
			WakeUpAll();
		};
		void Wait() {
			cweeJob out;
			WakeUpAll();
			while (1) {
				bool stop = true;
				while (cweeCpuThread::cweeCpuThreadData::TryExtractNextJob(m_Jobs, out)) { out.Invoke(); WakeUpAll(); }
				for (auto& thread : m_Threads) {
					if (thread.IsRunning()) {
						stop = false;
						break;
					};
				}
				if (stop) {
					break;
				}
			}
		};
		bool Wait_Once() {
			cweeJob out;
			WakeUpAll();
			if (1) {
				bool busy = false;
				if (cweeCpuThread::cweeCpuThreadData::TryExtractNextJob(m_Jobs, out)) { out.Invoke(); WakeUpAll(); busy = true; }
				for (auto& thread : m_Threads) {
					if (thread.IsRunning()) {
						busy = true;
						break;
					};
				}
				return !busy;
			}
		};
		bool Wait_Once(int tag) {
			cweeJob out;
			WakeUpAll();
			if (1) {
				// see if the tagged thread is done
				{
					AUTO g = m_taggedThreads->Guard();
					if (!m_taggedThreads->UnsafeCheck(tag)) {
						AUTO p = m_taggedThreads->UnsafeAppendAt(tag);
						p->get<0>() = jobListType();
						p->get<1>() = cweeCpuThread(p->get<0>(), nullptr, m_NumActiveThreads); // nullptr because these threads are designed to only support themselves - not each other. The generic threads do that.
					}
				}
				AUTO p = m_taggedThreads->GetPtr(tag);
				bool NoQueuedJobs = (p->get<0>()->Num() == 0);
				if (NoQueuedJobs && !p->get<1>().IsRunning()) {
					return true;
				}
				else {
					return false;
				}
			}
		};

	public:
		cweeSysInterlockedInteger				m_numJobs;
		cweeSharedPtr<jobListType>				m_Jobs;
		cweeSharedPtr<cweeSysInterlockedInteger> m_NumActiveThreads;
		const int								m_numPhysicalCpuCores;
		const int								m_numLogicalCpuCores;
		const int								m_numCpuPackages;
		cweeThreadedList<cweeCpuThread>			m_Threads;
		cweeSharedPtr<cweeThreadedMap<int, cweeUnion<cweeSharedPtr<jobListType>, cweeCpuThread>>> m_taggedThreads;
	};

private:
	cweeSharedPtr<cweeJobThreadsData> m_data;

public:
	cweeJobThreads() : m_data(new cweeJobThreadsData()) {};
	cweeJobThreads(cweeJobThreads const& a) : m_data(a.m_data) {};
	cweeJobThreads(cweeJobThreads&& a) : m_data(a.m_data) {};
	cweeJobThreads& operator=(cweeJobThreads const& a) {
		m_data = a.m_data;
		return *this;
	};
	cweeJobThreads& operator=(cweeJobThreads&& a) {
		m_data = a.m_data;
		return *this;
	};
	~cweeJobThreads() {
		// Await();
	};

public:
	void Queue(cweeJob j) const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			p->AddTask(j);
		}
		else {
			j.Invoke();
		}
	};
	void Queue(cweeJob j, int tag) const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			p->AddTask(j, tag);
		}
		else {
			j.Invoke();
		}
	};
	void Await() const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			p->Wait();
		}
	};
	bool Await_Once() const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			return p->Wait_Once();
		}
		return false;
	};
	bool Await_Once(int thread) const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			return p->Wait_Once(thread);
		}
		return false;
	};
};
static cweeSharedPtr<cweeJobThreads> cweeSharedJobThreads = make_cwee_shared<cweeJobThreads>();

/*!
Support for parallel processing of methods.
*/
class cweeMultithreading {
public:
	/*!
	Submits job for later completion by one of the three main threads (randomly selected for fair distribution)
	void task(T data);
	*/
	static void ADD_JOB(const cweeJob& task, jobListPriority_t priority = jobListPriority_t::JOBLIST_PRIORITY_MEDIUM) {
		cweeSharedJobThreads->Queue(task);
	};

	/*!
	Submits job for later completion by the requested thread. Required format of job:
	void task(T data);
	*/
	static void ADD_JOB(const cweeJob& task, jobType thread, jobListPriority_t priority = jobListPriority_t::JOBLIST_PRIORITY_MEDIUM) {
		cweeSharedJobThreads->Queue(task, static_cast<int>(thread));
	};

	/*!
	Submits job for later completion by one of the three main threads (randomly selected for fair distribution)
	void task(T data);
	*/
	template< typename T, typename U, typename = std::enable_if_t<!std::is_same_v<cweeJob, std::decay_t<T>>>>
	static void ADD_JOB(U task, T data, jobListPriority_t priority = jobListPriority_t::JOBLIST_PRIORITY_MEDIUM) {
		cweeSharedJobThreads->Queue(cweeJob([=]() {
			task(data);
		}));
	};

	/*!
	Submits job for later completion by the requested thread. Required format of job:
	void task(T data);
	*/
	template< typename T, typename U, typename = std::enable_if_t<!std::is_same_v<cweeJob, std::decay_t<T>>>>
	static void ADD_JOB(U task, T data, jobType thread, jobListPriority_t priority = jobListPriority_t::JOBLIST_PRIORITY_MEDIUM) {
		cweeSharedJobThreads->Queue(cweeJob([=]() {
			task(data);
		}), static_cast<int>(thread));		
	};

	/*!
	Returns true if the thread is not performing a job. Returns false otherwise.
	*/
	static bool TRYWAIT(jobType thread, int priority = -1) {
		return cweeSharedJobThreads->Await_Once(static_cast<int>(thread));
	};

	/*!
	Spins in place until all jobs submitted to the thread are complete - including jobs submitted within jobs.
	*/
	static void WAIT(jobType thread, int priority = -1) {
		cweeSharedJobThreads->Await();
	};

	/*!
	Must be called routinely to allow processor to submit and manage workload.
	*/
	static void THINK() {
		// parallelProcessor->Think();
		// parallelProcessor->UpdateStates();
	};

	/*!
	Returns true if all threads are not performing jobs. Returns false otherwise.
	*/
	static bool TRYWAIT() {
		return cweeSharedJobThreads->Await_Once();
	};

	/*!
	Spins in place until all jobs submitted to all threads are complete - including jobs submitted within jobs.
	*/
	static void WAIT() {
		cweeSharedJobThreads->Await();
	};

	static cweeThreadedList<jobHandle> GET_COMPLETED_JOBS() {
		cweeThreadedList<jobHandle> out(parallelProcessor->JobHandles.Num() + 16);
		/*std::vector<int> list;
		cweeUnorderedList<jobHandle>& handles = parallelProcessor->JobHandles;

		list.reserve(handles.Num() + 16);
		for (int x : handles.GetList()) list.push_back(x);
		std::sort(list.begin(), list.end());

		for (auto& index : list) {
			jobHandle obj = parallelProcessor->JobHandles[index];
			if (obj.handleNum == index) {
				out.Append(obj);
			}
		}*/
		return out;
	};

	static cweeStr GET_JOB_NAME(jobRun_t job) {
		return parallelProcessor->GetJobName(job);
	};
	static cweeStr GET_THREAD_NAME(jobType id) {
		return parallelProcessor->GetThreadName(id);
	};

	static int JOB_ASSIGNMENT_MODE(int in) {
		parallelProcessor->SetJobAssignmentMode(in);
		return in;
	};
	static int JOB_ASSIGNMENT_MODE() {
		return parallelProcessor->GetJobAssignmentMode();
	};
};

INLINE cweeJob& cweeJob::AsyncInvoke(jobType type, jobListPriority_t priority) {
	if (!IsFinished()) { cweeSharedJobThreads->Queue(*this, static_cast<int>(type)); }

	//if (!IsFinished()) { cweeMultithreading::ADD_JOB(*this, type, priority); }
	return *this;
};
INLINE cweeJob& cweeJob::AsyncInvoke(jobType type) {
	if (!IsFinished()) { cweeSharedJobThreads->Queue(*this, static_cast<int>(type)); }
	// if (!IsFinished()) { cweeMultithreading::ADD_JOB(*this, type); }
	return *this;
};
INLINE cweeJob& cweeJob::AsyncInvoke() {
	if (!IsFinished()) { cweeSharedJobThreads->Queue(*this); }
	// if (!IsFinished()) { cweeMultithreading::ADD_JOB(*this); }
	return *this;
};
INLINE cweeJob& cweeJob::AsyncForceInvoke(jobType type, jobListPriority_t priority) {
	cweeSharedJobThreads->Queue(cweeJob([](cweeJob& todo) { todo.ForceInvoke(); }, *this), static_cast<int>(type));

	//CweeJobContainer* ToDo = new CweeJobContainer();
	//ToDo->task = *this;
	//parallelProcessor->GetJobList(type, priority)->AddParallelJob((jobRun_t)ForceRunCweeJobInParallel, (void*)ToDo);
	return *this;
};
INLINE cweeJob& cweeJob::AsyncForceInvoke(jobType type) {
	cweeSharedJobThreads->Queue(cweeJob([](cweeJob& todo) { todo.ForceInvoke(); }, *this), static_cast<int>(type));

	//CweeJobContainer* ToDo = new CweeJobContainer();
	//ToDo->task = *this;
	//parallelProcessor->GetJobList(type, jobListPriority_t::JOBLIST_PRIORITY_MEDIUM)->AddParallelJob((jobRun_t)ForceRunCweeJobInParallel, (void*)ToDo);
	return *this;
};
INLINE cweeJob& cweeJob::AsyncForceInvoke() {
	cweeSharedJobThreads->Queue(cweeJob([](cweeJob& todo) { todo.ForceInvoke(); }, *this));

	//CweeJobContainer* ToDo = new CweeJobContainer();
	//ToDo->task = *this;
	//parallelProcessor->GetJobList((jobType)cweeRandomInt(0, 2), jobListPriority_t::JOBLIST_PRIORITY_MEDIUM)->AddParallelJob((jobRun_t)ForceRunCweeJobInParallel, (void*)ToDo);
	return *this;
};
INLINE cweeAny cweeJob::Await() {
#if 0
	AUTO signal = make_cwee_shared<cweeSysSignal>(0);
	this->ContinueWith([](cweeSysSignal& r) { r.Raise(); }, signal);
	cweeSysSignal& r = *signal.Get();
	r.Wait(cweeSysSignal::WAIT_INFINITE); // wait forever, without consuming CPU cycles. 
#else
	while (!this->IsFinished()) {
		cweeSharedJobThreads->Await_Once();
	}
#endif
	return GetResult();
};
INLINE uintptr_t cweeJob::DelayedAsyncInvoke(u64 milliseconds_delay) {
	cweeUnion< u64, cweeJob >* data = new cweeUnion<u64, cweeJob>(milliseconds_delay, *this);
	return Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
		if (_anon_ptr != nullptr) {
			cweeUnion< u64, cweeJob >* T = static_cast<cweeUnion< u64, cweeJob>*>(_anon_ptr);
			::Sleep(T->get<0>());
			T->get<1>().AsyncInvoke();
			delete T;
		}
		return 0;
	}), (void*)data, xthreadPriority::THREAD_NORMAL, "Fire and Forget C++ Thread", core_t::CORE_ANY, 1024, false);
};
INLINE uintptr_t cweeJob::DelayedAsyncInvoke(u64 milliseconds_delay, jobType type) {
	cweeUnion< u64, cweeJob, jobType >* data = new cweeUnion<u64, cweeJob, jobType>(milliseconds_delay, *this, type);
	return Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
		if (_anon_ptr != nullptr) {
			cweeUnion< u64, cweeJob, jobType >* T = static_cast<cweeUnion< u64, cweeJob, jobType>*>(_anon_ptr);
			::Sleep(T->get<0>());
			T->get<1>().AsyncInvoke(T->get<2>());
			delete T;
		}
		return 0;
	}), (void*)data, xthreadPriority::THREAD_NORMAL, "Fire and Forget C++ Thread", core_t::CORE_ANY, 1024, false);
};
INLINE uintptr_t cweeJob::DelayedAsyncForceInvoke(u64 milliseconds_delay) {
	cweeUnion< u64, cweeJob >* data = new cweeUnion<u64, cweeJob>(milliseconds_delay, *this);
	return Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
		if (_anon_ptr != nullptr) {
			cweeUnion< u64, cweeJob >* T = static_cast<cweeUnion< u64, cweeJob>*>(_anon_ptr);
			::Sleep(T->get<0>());
			T->get<1>().AsyncForceInvoke();
			delete T;
		}
		return 0;
	}), (void*)data, xthreadPriority::THREAD_NORMAL, "Fire and Forget C++ Thread", core_t::CORE_ANY, 1024, false);
};
INLINE uintptr_t cweeJob::DelayedAsyncForceInvoke(u64 milliseconds_delay, jobType type) {
	cweeUnion< u64, cweeJob, jobType >* data = new cweeUnion<u64, cweeJob, jobType>(milliseconds_delay, *this, type);
	return Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
		if (_anon_ptr != nullptr) {
			cweeUnion< u64, cweeJob, jobType >* T = static_cast<cweeUnion< u64, cweeJob, jobType>*>(_anon_ptr);
			::Sleep(T->get<0>());
			T->get<1>().AsyncForceInvoke(T->get<2>());
			delete T;
		}
		return 0;
	}), (void*)data, xthreadPriority::THREAD_NORMAL, "Fire and Forget C++ Thread", core_t::CORE_ANY, 1024, false);
};



#endif