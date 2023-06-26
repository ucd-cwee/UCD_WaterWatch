#pragma hdrstop
#include "precompiled.h"
#include "ParallelThreads.h"

constexpr const static int			MAX_CORES = 32;
constexpr const static int			MAX_JOB_THREADS = 32;
static int							NUM_JOB_THREADS = 3;
static int							JOB_ASSIGNMENT_MODE = 1; // -1 = original / conservative mode, 1 = new / progressive mode
constexpr const static int			JOB_THREAD_STACK_SIZE = 512 * 1024;
extern void							Sys_CPUCount(int& logicalNum, int& coreNum, int& packageNum);


/*
================================================================================================
	Job and Job-List names
================================================================================================
*/
const char* threadNames[] = {
	ASSERT_ENUM_STRING(IO_thread,					0),
	ASSERT_ENUM_STRING(SIM_thread,					1),
	ASSERT_ENUM_STRING(OPT_thread,					2),
	ASSERT_ENUM_STRING(PRIORITY_thread,				3),
//	ASSERT_ENUM_STRING(SCRIPT_thread,				4),
};

const char* Get_ThreadListName(jobType id) {
	if ((int)id <= 4) {
		return threadNames[id];
	}
	else {
		int i = (int)id;
		return cweeStr::printf("C++ Thread %i", i).c_str();
	}
}

static const int MAX_REGISTERED_THREAD_JOBS = 2048;

struct registered_Thread_Job {
	jobRun_t		function;
	const char*		name;
} registered_Thread_Jobs[MAX_REGISTERED_THREAD_JOBS];

static int numRegistered_Thread_Jobs;

const char* Get_ThreadListName(jobListId_t id) {
	return threadNames[id];
}

static bool IsRegistered_Thread_Job(jobRun_t function) {
	for (int i = 0; i < numRegistered_Thread_Jobs; i++) {
		if (registered_Thread_Jobs[i].function == function) {
			return true;
		}
	}
	return false;
}

void Register_Thread_Job(jobRun_t function, const char* name) {
	if (IsRegistered_Thread_Job(function)) {
		return;
	}
	registered_Thread_Jobs[numRegistered_Thread_Jobs].function = function;
	registered_Thread_Jobs[numRegistered_Thread_Jobs].name = name;
	numRegistered_Thread_Jobs++;
}

const char* Get_ThreadJobName(jobRun_t function) {
	for (int i = 0; i < numRegistered_Thread_Jobs; i++) {
		if (registered_Thread_Jobs[i].function == function) {
			return registered_Thread_Jobs[i].name;
		}
	}
	return "unknown";
}

cweeparallel_ThreadRegistration::cweeparallel_ThreadRegistration(jobRun_t function, const char* name) {
	Register_Thread_Job(function, name);
}


struct threadListState_t {
	threadListState_t() :
		jobList(NULL),
		version(0xFFFFFFFF),
		signalIndex(0),
		lastJobIndex(0),
		nextJobIndex(-1) {}
	threadListState_t(int _version) :
		jobList(NULL),
		version(_version),
		signalIndex(0),
		lastJobIndex(0),
		nextJobIndex(-1) {}
	cweeparallel_JobList_Threads* jobList;
	int							version;
	int							signalIndex;
	int							lastJobIndex;
	int							nextJobIndex;
};


struct ThreadStats_t {
	unsigned int	numExecutedJobs;
	unsigned int	numExecutedSyncs;
	uint64			submitTime;
	uint64			startTime;
	uint64			endTime;
	uint64			waitTime;
	uint64			threadExecTime[MAX_CORES];
	uint64			threadTotalTime[MAX_CORES];
};








class cweeparallel_JobList_Threads {
public:
	cweeparallel_JobList_Threads(jobType id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs, cweeUnorderedList< jobHandle >* JobHandlesList);
	~cweeparallel_JobList_Threads();
	void					AllowMultithreading(bool T_or_F);
	int						GetMultithreadingSetting(void);
	INLINE void				AddParallelJob(jobRun_t function, void* data);
	INLINE void				InsertSyncPoint(jobSyncType_t syncType);
	void					Submit(cweeparallel_JobList_Threads* waitForJobList_, int parallelism);
	int						GetNumExecutingThreads() const {
		return numThreadsExecuting.GetValue();
	}
	int						GetNumQueuedJobs() const;

	void					Wait();
	bool					TryWait();
	bool					IsSubmitted() const;

	unsigned int			GetNumExecutedJobs() const { return threadStats.numExecutedJobs; }
	unsigned int			GetNumSyncs() const { return threadStats.numExecutedSyncs; }
	uint64					GetSubmitTimeMicroSec() const { return threadStats.submitTime; }
	uint64					GetStartTimeMicroSec() const { return threadStats.startTime; }
	uint64					GetFinishTimeMicroSec() const { return threadStats.endTime; }
	uint64					GetWaitTimeMicroSec() const { return threadStats.waitTime; }
	uint64					GetTotalProcessingTimeMicroSec() const;
	uint64					GetTotalWastedTimeMicroSec() const;
	uint64					GetUnitProcessingTimeMicroSec(int unit) const;
	uint64					GetUnitWastedTimeMicroSec(int unit) const;

	const jobType&			GetId() const { return listId; }
	const jobListPriority_t& GetPriority() const { return listPriority; }
	int						GetVersion() { return version.GetValue(); }

	bool					WaitForOtherJobList();

	//------------------------
	// This is thread safe and called from the job threads.
	//------------------------
	enum runResult_t {
		RUN_OK = 0,
		RUN_PROGRESS = BIT(0),
		RUN_DONE = BIT(1),
		RUN_STALLED = BIT(2)
	};

	int						RunJobs(unsigned int threadNum, threadListState_t& state, bool singleJob);



private:
	cweeUnorderedList< jobHandle >* JobHandles = nullptr;

	static const int		NUM_DONE_GUARDS = 4; // was 128 // was 4	// cycle through 4 guards so we can cyclicly chain job lists	
	cweeSysInterlockedInteger earlyExitAllJobs;
	bool					threaded;
	bool					done;
	bool					hasSignal;
	const jobType			listId;
	const jobListPriority_t	listPriority;
	unsigned int			maxJobs;
	unsigned int			maxSyncs;
	unsigned int			numSyncs;
	int						lastSignalJob;
	cweeSysInterlockedInteger* waitForGuard;
	cweeSysInterlockedInteger doneGuards[NUM_DONE_GUARDS];
	int						currentDoneGuard;
	cweeSysInterlockedInteger	version;
	struct job_t {
		jobRun_t	function;
		void* data;
		int			executed;
	};





	cweeUnpooledInterlocked< cweeThreadedList< job_t, TAG_JOBLIST > >	executableJobList;

	cweeThreadedList< cweeSysInterlockedInteger, TAG_JOBLIST >	signalJobCount;




	cweeSysInterlockedInteger				currentJob;
	cweeSysInterlockedInteger				fetchLock;
	cweeSysInterlockedInteger				numThreadsExecuting;

	ThreadStats_t						deferredThreadStats;
	ThreadStats_t						threadStats;

	int						RunJobsInternal(unsigned int threadNum, threadListState_t& state, bool singleJob);

	static void				Nop(void* data) {}

	static int				THREAD_SIGNAL;
	static int				THREAD_SYNCHRONIZE;
	static int				THREAD_LIST_DONE;
};

int cweeparallel_JobList_Threads::THREAD_SIGNAL;
int cweeparallel_JobList_Threads::THREAD_SYNCHRONIZE;
int cweeparallel_JobList_Threads::THREAD_LIST_DONE;












cweeparallel_JobList_Threads::cweeparallel_JobList_Threads(jobType id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs, cweeUnorderedList< jobHandle >* JobHandlesList) :
	threaded(true),
	done(true),
	hasSignal(false),
	listId(id),
	listPriority(priority),
	numSyncs(0),
	lastSignalJob(0),
	waitForGuard(NULL),
	currentDoneGuard(0) {

	JobHandles = JobHandlesList;

	assert(listPriority != JOBLIST_PRIORITY_NONE);

	this->maxJobs = maxJobs;
	this->maxSyncs = maxSyncs;
	executableJobList.Lock();
	executableJobList.UnsafeRead()->AssureSize(maxJobs + maxSyncs * 2 + 1);	// syncs go in as dummy jobs and one more to update the doneCount
	executableJobList.UnsafeRead()->SetNum(0);
	executableJobList.Unlock();
	signalJobCount.AssureSize(maxSyncs + 1);			// need one extra for submit
	signalJobCount.SetNum(0);

	memset(&deferredThreadStats, 0, sizeof(ThreadStats_t));
	memset(&threadStats, 0, sizeof(ThreadStats_t));
}

cweeparallel_JobList_Threads::~cweeparallel_JobList_Threads() {
	Wait();
}

INLINE void cweeparallel_JobList_Threads::AddParallelJob(jobRun_t function, void* data) {
	assert(done);	
	executableJobList.Lock();
	for (int i = 0; i < executableJobList.UnsafeRead()->Num(); i++) {
		if (executableJobList.UnsafeRead()->operator[](i).function == function && executableJobList.UnsafeRead()->operator[](i).data == data) {

			u64 d = (uintptr_t)data;

			cweeStr err = cweeStr::printf(
				"EDMS Error: Submitting a multithread job with the same function and the same data simultaneously is undefined behavior. Job: %s; Thread: %s; Data: %s;", 
					Get_ThreadJobName(function),
					Get_ThreadListName(this->GetId()),
					cweeStr(d).c_str()
				);

			std::cout << err << std::endl;
			throw std::invalid_argument::logic_error(err);
		}
	}
	executableJobList.Unlock();

	executableJobList.Lock(); {
		{
			job_t& job = executableJobList.UnsafeRead()->Alloc(); {
				job.function = function;
				job.data = data;
				job.executed = 0;
			}
		}
	} executableJobList.Unlock();
}

INLINE void cweeparallel_JobList_Threads::InsertSyncPoint(jobSyncType_t syncType) {
	assert(done);
	switch (syncType) {
	case SYNC_SIGNAL: {
		assert(!hasSignal);
		executableJobList.Lock();
		if (executableJobList.UnsafeRead()->Num()) {
			assert(!hasSignal);
			signalJobCount.Alloc();
			signalJobCount[signalJobCount.Num() - 1].SetValue(executableJobList.UnsafeRead()->Num() - lastSignalJob);
			lastSignalJob = executableJobList.UnsafeRead()->Num();
			job_t& job = executableJobList.UnsafeRead()->Alloc();
			job.function = Nop;
			job.data = &THREAD_SIGNAL;
			hasSignal = true;
		}
		executableJobList.Unlock();
		break;
	}
	case SYNC_SYNCHRONIZE: {
		if (hasSignal) {
			executableJobList.Lock();
			job_t& job = executableJobList.UnsafeRead()->Alloc();
			job.function = Nop;
			job.data = &THREAD_SYNCHRONIZE;
			hasSignal = false;
			numSyncs++;
			executableJobList.Unlock();
		}
		break;
	}
	}
}

void cweeparallel_JobList_Threads::AllowMultithreading(bool T_or_F) {
	if (T_or_F != threaded) {
		if (T_or_F == false) {
			earlyExitAllJobs.Increment();
			Wait(); // ensure clean start
			earlyExitAllJobs.Decrement();
		}
		else {
			Wait(); // ensure clean start
		}
	}
	threaded = T_or_F;
}

int	cweeparallel_JobList_Threads::GetMultithreadingSetting(void) {
	int out = 0;
	if (threaded == true) out = cweeMath::max(1, NUM_JOB_THREADS);
	else out = 0;
	return out;
}


void cweeparallel_JobList_Threads::Submit(cweeparallel_JobList_Threads* waitForJobList, int parallelism) {
	assert(done);
	assert(numSyncs <= maxSyncs);
	assert((unsigned int)jobList.Num() <= maxJobs + numSyncs * 2);
	assert(fetchLock.GetValue() == 0);

	done = false;
	currentJob.SetValue(0);

	memset(&deferredThreadStats, 0, sizeof(deferredThreadStats));
	executableJobList.Lock();
	int size = executableJobList.UnsafeRead()->Num();
	executableJobList.Unlock();
	deferredThreadStats.numExecutedJobs = size - numSyncs * 2;		
	deferredThreadStats.numExecutedSyncs = numSyncs;
	deferredThreadStats.submitTime = Sys_Microseconds();
	deferredThreadStats.startTime = 0;
	deferredThreadStats.endTime = 0;
	deferredThreadStats.waitTime = 0;
	if (size != 0) {
		if (waitForJobList != NULL) {
			waitForGuard = &waitForJobList->doneGuards[waitForJobList->currentDoneGuard];
		}
		else {
			waitForGuard = NULL;
		}

		currentDoneGuard = (currentDoneGuard + 1) & (NUM_DONE_GUARDS - 1);
		doneGuards[currentDoneGuard].SetValue(1);

		signalJobCount.Alloc();
		executableJobList.Lock();
		signalJobCount[signalJobCount.Num() - 1].SetValue(executableJobList.UnsafeRead()->Num() - lastSignalJob);
		job_t& job = executableJobList.UnsafeRead()->Alloc();
		executableJobList.Unlock();
		job.function = Nop;
		job.data = &THREAD_LIST_DONE;

		if (threaded) {
			// hand over to the manager
			void SubmitJobList(cweeparallel_JobList_Threads * jobList, int parallelism);
			SubmitJobList((cweeparallel_JobList_Threads*)this, parallelism);
		}
		else {
			// run all the jobs right here
			threadListState_t state(GetVersion());
			RunJobs(0, state, false);
		}
	}
}

void cweeparallel_JobList_Threads::Wait() {
	
	executableJobList.Lock();
	int size = executableJobList.UnsafeRead()->Num();
	executableJobList.Unlock();
	
	if (size > 0) {
		if (!verify(!done && signalJobCount.Num() > 0)) { // don't lock up but return if the job list was never properly submitted
			return;
		}

		bool waited = false;
		uint64 waitStart = Sys_Microseconds();

		while (signalJobCount[signalJobCount.Num() - 1].GetValue() > 0) {
			bool test = Sys_TryYield();
			waited = true;
		}
		version.Increment();
		while (numThreadsExecuting.GetValue() > 0) {
			bool test = Sys_TryYield();
			waited = true;
		}

		executableJobList.Lock();
		executableJobList.UnsafeRead()->SetNum(0);
		executableJobList.Unlock();
		
		signalJobCount.SetNum(0);
		numSyncs = 0;
		lastSignalJob = 0;

		uint64 waitEnd = Sys_Microseconds();
		deferredThreadStats.waitTime = waited ? (waitEnd - waitStart) : 0;
	}

	memcpy(&threadStats, &deferredThreadStats, sizeof(threadStats));
	done = true;
}

int	cweeparallel_JobList_Threads::GetNumQueuedJobs() const {
	int out = 0;

	executableJobList.Lock();
	out = executableJobList.UnsafeRead()->Num();
	executableJobList.Unlock();

	return out;
}

bool cweeparallel_JobList_Threads::TryWait() {
	executableJobList.Lock();
	int size = executableJobList.UnsafeRead()->Num();
	executableJobList.Unlock();

	if (size > 0) {
		if (!verify(!done && signalJobCount.Num() > 0)) { // don't lock up but return if the job list was never properly submitted
			return true;
		}
		if (signalJobCount[signalJobCount.Num() - 1].GetValue() > 0) {
			bool test = Sys_TryYield();
			return false;
		}
		if (numThreadsExecuting.GetValue() > 0) {
			bool test = Sys_TryYield();
			return false;
		}
	}
	Wait();
	return true;
}

bool cweeparallel_JobList_Threads::IsSubmitted() const {
	return !done;
}

uint64 cweeparallel_JobList_Threads::GetTotalProcessingTimeMicroSec() const {
	uint64 total = 0;
	for (int unit = 0; unit < MAX_CORES; unit++) {
		total += threadStats.threadExecTime[unit];
	}
	return total;
}

uint64 cweeparallel_JobList_Threads::GetTotalWastedTimeMicroSec() const {
	uint64 total = 0;
	for (int unit = 0; unit < MAX_CORES; unit++) {
		total += threadStats.threadTotalTime[unit] - threadStats.threadExecTime[unit];
	}
	return total;
}

uint64 cweeparallel_JobList_Threads::GetUnitProcessingTimeMicroSec(int unit) const {
	if (unit < 0 || unit >= MAX_CORES) {
		return 0;
	}
	return threadStats.threadExecTime[unit];
}

uint64 cweeparallel_JobList_Threads::GetUnitWastedTimeMicroSec(int unit) const {
	if (unit < 0 || unit >= MAX_CORES) {
		return 0;
	}
	return threadStats.threadTotalTime[unit] - threadStats.threadExecTime[unit];
}

int cweeparallel_JobList_Threads::RunJobsInternal(unsigned int threadNum, threadListState_t& state, bool singleJob) {
	if (state.version != version.GetValue()) {
		// trying to run an old version of this list that is already done
		return RUN_DONE;
	}

	if (deferredThreadStats.startTime == 0) {
		deferredThreadStats.startTime = Sys_Microseconds();	// first time any thread is running jobs from this list
	}

	int result = RUN_OK;

	do {
		// run through all signals and syncs before the last job that has been or is being executed
		// this loop is really an optimization to minimize the time spent in the fetchLock section below

		executableJobList.Lock();
		for (; state.lastJobIndex < (int)currentJob.GetValue() && state.lastJobIndex < executableJobList.UnsafeRead()->Num(); state.lastJobIndex++) {
			if (executableJobList.UnsafeRead()->operator[](state.lastJobIndex).data == &THREAD_SIGNAL) {
				state.signalIndex++;
				assert(state.signalIndex < signalJobCount.Num());
			}
			else if (executableJobList.UnsafeRead()->operator[](state.lastJobIndex).data == &THREAD_SYNCHRONIZE) {
				assert(state.signalIndex > 0);
				if (signalJobCount[state.signalIndex - 1].GetValue() > 0) {
					// stalled on a synchronization point
					return (result | RUN_STALLED);
				}
			}
			else if (executableJobList.UnsafeRead()->operator[](state.lastJobIndex).data == &THREAD_LIST_DONE) {
				if (signalJobCount[signalJobCount.Num() - 1].GetValue() > 0) {
					// stalled on a synchronization point
					return (result | RUN_STALLED);
				}
			}
		}
		executableJobList.Unlock();




		// try to lock to fetch a new job
		if (fetchLock.Increment() == 1) {

			// grab a new job
			state.nextJobIndex = currentJob.Increment() - 1;

			// run through any remaining signals and syncs (this should rarely iterate more than once)
			executableJobList.Lock();
			int size = executableJobList.UnsafeRead()->Num();
			executableJobList.Unlock();
					
			for (; state.lastJobIndex <= state.nextJobIndex && state.lastJobIndex < size; state.lastJobIndex++) {
				executableJobList.Lock();
				if (executableJobList.UnsafeRead()->operator[](state.lastJobIndex).data == &THREAD_SIGNAL) {
					state.signalIndex++;
					assert(state.signalIndex < signalJobCount.Num());
				}
				else if (executableJobList.UnsafeRead()->operator[](state.lastJobIndex).data == &THREAD_SYNCHRONIZE) {
					assert(state.signalIndex > 0);
					if (signalJobCount[state.signalIndex - 1].GetValue() > 0) {
						// return this job to the list
						currentJob.Decrement();
						// release the fetch lock
						fetchLock.Decrement();
						// stalled on a synchronization point
						return (result | RUN_STALLED);
					}
				}
				else if (executableJobList.UnsafeRead()->operator[](state.lastJobIndex).data == &THREAD_LIST_DONE) {
					if (signalJobCount[signalJobCount.Num() - 1].GetValue() > 0) {
						// return this job to the list
						currentJob.Decrement();
						// release the fetch lock
						fetchLock.Decrement();
						// stalled on a synchronization point
						return (result | RUN_STALLED);
					}
					// decrement the done count
					doneGuards[currentDoneGuard].Decrement();
				}
				executableJobList.Unlock();
			}
			
			// release the fetch lock
			fetchLock.Decrement();
		}
		else {
			// release the fetch lock
			fetchLock.Decrement();
			// another thread is fetching right now so consider stalled
			return (result | RUN_STALLED);
		}

		// if at the end of the job list we're done
		executableJobList.Lock();
		if (state.nextJobIndex >= executableJobList.UnsafeRead()->Num()) {
			executableJobList.Unlock();
			return (result | RUN_DONE);
		}
		executableJobList.Unlock();

		// execute the next job
		{
#if 1
			this->JobHandles->Lock();
			int handleNum = this->JobHandles->UnsafeAppend();
			auto handlePtr = this->JobHandles->UnsafeRead(handleNum);
			jobRun_t theJob;
			if (handlePtr) {
				handlePtr->handleNum = handleNum;
				if (state.jobList) handlePtr->thread = state.jobList->GetId();
				executableJobList.Lock(); {
					handlePtr->function = executableJobList.UnsafeRead()->operator[](state.nextJobIndex).function;					
				} executableJobList.Unlock();
				theJob = handlePtr->function;
				handlePtr->startTime = clock_ns();
				handlePtr->endTime = 0;
				handlePtr->cpuCore = threadNum;
			}
			else {
				executableJobList.Lock(); {
					theJob = executableJobList.UnsafeRead()->operator[](state.nextJobIndex).function;
				} executableJobList.Unlock();
			}
			this->JobHandles->Unlock();

			uint64 jobStart = Sys_Microseconds();
			if (earlyExitAllJobs.GetValue() == 0) {
				executableJobList.Lock();
				void* data = executableJobList.UnsafeRead()->operator[](state.nextJobIndex).data;
				executableJobList.Unlock();
				
				try {
					if (theJob) {
						theJob(data);
					}
					else {
						int i = 0;
					}
				}
				catch (...) {
					auto trace = cweeStackTrace::GetTrace();
					submitToast("WaterWatch Error", trace);
					
					executableJobList.Lock();
					executableJobList.UnsafeRead()->operator[](state.nextJobIndex).executed = 1;
					executableJobList.Unlock();
					this->JobHandles->Lock();
					handlePtr = this->JobHandles->UnsafeRead(handleNum);
					if (handlePtr) {
						handlePtr->endTime = clock_ns();
					}
					this->JobHandles->Unlock();
					uint64 jobEnd = Sys_Microseconds();
					deferredThreadStats.threadExecTime[threadNum] += jobEnd - jobStart;

					result |= RUN_PROGRESS;

					// decrease the job count for the current signal
					if (signalJobCount[state.signalIndex].Decrement() == 0) {
						// if this was the very last job of the job list
						if (state.signalIndex == signalJobCount.Num() - 1) {
							deferredThreadStats.endTime = Sys_Microseconds();
							return (result | RUN_DONE);
						}
					}

					return result;
				}

				//obj.function(data);
				//executableJobList[state.nextJobIndex].function(executableJobList[state.nextJobIndex].data); // theory: use a global modifier (atomic int?) to determine whether to execute or throw-out the method/data.
			}
			else {
				// --> Introduces a memory leak, but allows for early-exit in case of emergency.	
				// TO-DO, find a way to remove the memory leak in the future. Perhaps utilize shared pointers instead of new/delete pairs within tasks. 
			}
			executableJobList.Lock();
			executableJobList.UnsafeRead()->operator[](state.nextJobIndex).executed = 1;
			executableJobList.Unlock();
			
			this->JobHandles->Lock();
			handlePtr = this->JobHandles->UnsafeRead(handleNum);
			if (handlePtr) {
				handlePtr->endTime = clock_ns();
			}
			this->JobHandles->Unlock();
			constexpr int maxNumJobs = 500;
			int numHandles = this->JobHandles->Num();
			if (numHandles > (maxNumJobs / 2)) {
				if (numHandles > maxNumJobs) { // TO-DO, set a constexpr or static public variable for num handles
					// remove all
					this->JobHandles->Clear();
				}

				auto list = this->JobHandles->GetList();
				list.Sort<int, TAG_LIST>();
				if (list.Num() > ((maxNumJobs / 4) + 1)) {
					numHandles = list[(maxNumJobs / 4)];
					for (auto& x : list) {
						if (x < numHandles) {
							this->JobHandles->Lock();
							handlePtr = this->JobHandles->UnsafeRead(x);
							if (handlePtr && handlePtr->endTime != 0) {
								this->JobHandles->UnsafeErase(x);
							}
							this->JobHandles->Unlock();
						}
					}
				}	
			}


			//obj.endTime = clock_ns();
			//this->JobHandles->Swap(obj.handleNum, obj);

			uint64 jobEnd = Sys_Microseconds();
			deferredThreadStats.threadExecTime[threadNum] += jobEnd - jobStart;
#else



			uint64 jobStart = Sys_Microseconds();
			if (earlyExitAllJobs.GetValue() == 0) {
				jobList[state.nextJobIndex].function(jobList[state.nextJobIndex].data); // theory: use a global modifier (atomic int?) to determine whether to execute or throw-out the method/data.
			}
			else {
				// --> Introduces a memory leak, but allows for early-exit in case of emergency.	
				// TO-DO, find a way to remove the memory leak in the future. Perhaps utilize shared pointers instead of new/delete pairs within tasks. 
			}
			jobList[state.nextJobIndex].executed = 1;

			uint64 jobEnd = Sys_Microseconds();
			deferredThreadStats.threadExecTime[threadNum] += jobEnd - jobStart;
#endif




		}

		result |= RUN_PROGRESS;

		// decrease the job count for the current signal
		if (signalJobCount[state.signalIndex].Decrement() == 0) {
			// if this was the very last job of the job list
			if (state.signalIndex == signalJobCount.Num() - 1) {
				deferredThreadStats.endTime = Sys_Microseconds();
				return (result | RUN_DONE);
			}
		}

	} while (!singleJob);

	return result;
}

int cweeparallel_JobList_Threads::RunJobs(unsigned int threadNum, threadListState_t& state, bool singleJob) {
	uint64 start = Sys_Microseconds();
	numThreadsExecuting.Increment();
	int result = RunJobsInternal(threadNum, state, singleJob);
	numThreadsExecuting.Decrement();
	deferredThreadStats.threadTotalTime[threadNum] += Sys_Microseconds() - start;
	return result;
}

bool cweeparallel_JobList_Threads::WaitForOtherJobList() {
	if (waitForGuard != NULL) {
		if (waitForGuard->GetValue() > 0) {
			return true;
		}
	}
	return false;
}



















struct threadList_t {
	cweeparallel_JobList_Threads* jobList;
	int							version;
};

class cweeThread : public cweeSysThread {
public:
	cweeThread();
	~cweeThread();

	bool						Start(core_t core, unsigned int threadNum);

	void						AddParallelJobList(cweeparallel_JobList_Threads* jobList);

private:
	threadList_t				jobLists[jobType::MAX_NUM_PARALLEL_THREADS];	// cyclic buffer with job lists
	unsigned int				firstJobList;			// index of the last job list the thread grabbed
	unsigned int				lastJobList;			// index where the next job list to work on will be added
	cweeSysMutex				AddParallelJobMutex;

	unsigned int				threadNum;

	virtual int					Run();
};

cweeThread::cweeThread() :
	firstJobList(0),
	lastJobList(0),
	threadNum(0) {
}

cweeThread::~cweeThread() {
}

bool cweeThread::Start(core_t core, unsigned int threadNum) {
	this->threadNum = threadNum;
	return StartWorkerThread((cweeStr("JobListProcessor_") + cweeStr(threadNum)).c_str(), core, THREAD_NORMAL, JOB_THREAD_STACK_SIZE);
}

void cweeThread::AddParallelJobList(cweeparallel_JobList_Threads* jobList) {
	AUTO G = AddParallelJobMutex.Guard(); // must lock because multiple threads may try to add new job lists at the same time
	while (lastJobList - firstJobList >= MAX_NUM_PARALLEL_THREADS) { // wait until there is space available because in rare cases multiple versions of the same job lists may still be queued 
		bool test = Sys_TryYield();
	}
	assert(lastJobList - firstJobList < MAX_NUM_PARALLEL_THREADS);
	jobLists[lastJobList & (MAX_NUM_PARALLEL_THREADS - 1)].jobList = jobList;
	jobLists[lastJobList & (MAX_NUM_PARALLEL_THREADS - 1)].version = jobList->GetVersion();

	lastJobList++;
}

int cweeThread::Run() {
	threadListState_t threadJobListState[MAX_NUM_PARALLEL_THREADS];
	int numJobLists = 0;
	int lastStalledJobList = -1;

	while (!IsTerminating()) {
		// fetch any new job lists and add them to the local list
		if (numJobLists < MAX_NUM_PARALLEL_THREADS && firstJobList < lastJobList) {
			AUTO G = AddParallelJobMutex.Guard();
			threadJobListState[numJobLists].jobList = jobLists[firstJobList & (MAX_NUM_PARALLEL_THREADS - 1)].jobList;
			threadJobListState[numJobLists].version = jobLists[firstJobList & (MAX_NUM_PARALLEL_THREADS - 1)].version;
			threadJobListState[numJobLists].signalIndex = 0;
			threadJobListState[numJobLists].lastJobIndex = 0;
			threadJobListState[numJobLists].nextJobIndex = -1;
			numJobLists++;
			firstJobList++;
		}
		if (numJobLists == 0) {
			break;
		}
		int currentJobList = 0;
		jobListPriority_t priority = (jobListPriority_t)JOBLIST_PRIORITY_NONE;
		if (lastStalledJobList < 0) {
			// find the job list with the highest priority
			for (int i = 0; i < numJobLists; i++) {
				if (threadJobListState[i].jobList->GetPriority() > priority && !threadJobListState[i].jobList->WaitForOtherJobList()) {
					priority = threadJobListState[i].jobList->GetPriority();
					currentJobList = i;
				}
			}
		}
		else {
			// try to hide the stall with a job from a list that has equal or higher priority
			currentJobList = lastStalledJobList;
			priority = threadJobListState[lastStalledJobList].jobList->GetPriority();
			for (int i = 0; i < numJobLists; i++) {
				if (i != lastStalledJobList && threadJobListState[i].jobList->GetPriority() >= priority && !threadJobListState[i].jobList->WaitForOtherJobList()) {
					priority = threadJobListState[i].jobList->GetPriority();
					currentJobList = i;
				}
			}
		}

		// if the priority is high then try to run through the whole list to reduce the overhead
		// otherwise run a single job and re-evaluate priorities for the next job
		bool singleJob = false; //  (priority == JOBLIST_PRIORITY_HIGH) ? false : true;

		int result = threadJobListState[currentJobList].jobList->RunJobs(threadNum, threadJobListState[currentJobList], singleJob); // try running one or more jobs from the current job list

		if ((result & cweeparallel_JobList_Threads::RUN_DONE) != 0) {

			for (int i = currentJobList; i < numJobLists - 1; i++) { // done with this job list so remove it from the local list
				threadJobListState[i] = threadJobListState[i + 1];
			}
			numJobLists--;
			lastStalledJobList = -1;
		}
		else if ((result & cweeparallel_JobList_Threads::RUN_STALLED) != 0) {

			if (currentJobList == lastStalledJobList) { // yield when stalled on the same job list again without making any progress
				if ((result & cweeparallel_JobList_Threads::RUN_PROGRESS) == 0) {
					bool test = Sys_TryYield();
				}
			}
			lastStalledJobList = currentJobList;
		}
		else {
			lastStalledJobList = -1;
		}
	}
	return 0;
}




















/*
================================================================================================
cweeparallel_ThreadList
================================================================================================
*/

cweeparallel_ThreadList::cweeparallel_ThreadList(jobType id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs, cweeUnorderedList< jobHandle >* JobHandlesList) {
	assert(priority > JOBLIST_PRIORITY_NONE);
	this->jobListThreads = new (TAG_JOBLIST) cweeparallel_JobList_Threads(id, priority, maxJobs, maxSyncs, JobHandlesList);
}

cweeparallel_ThreadList::~cweeparallel_ThreadList() {
	delete jobListThreads;
}

void cweeparallel_ThreadList::AddParallelJob(void* function, void* data) {
	AddParallelJob((jobRun_t)function, data);
}

void cweeparallel_ThreadList::AddParallelJob(jobRun_t function, void* data) {
	// note: if we TryWait() here and it fails, it's not ready to recieve the list. 
	bool test = IsRegistered_Thread_Job(function);
	if (test == false) {		
		REGISTER_WITH_PARALLEL_THREAD(function);
		test = IsRegistered_Thread_Job(function);
	}
	assert(test);

	if (TryWait()) {
		jobListThreads->AddParallelJob(function, data);
	}
	else {
		// what if we create a seperate list of intermediate requests that would get processed and removed at a later date? 
		//jobBuffer.Swap(jobBuffer.Append(), std::make_tuple(function, data, (jobSyncType_t)SYNC_NONE));		
		jobBuffer.Append(std::make_tuple(function, data, (jobSyncType_t)SYNC_NONE));
	}
}

void cweeparallel_ThreadList::InsertSyncPoint(jobSyncType_t syncType) {
	if (TryWait()) {
		jobListThreads->InsertSyncPoint(syncType);
	}
	else {
		// what if we create a seperate list of intermediate requests that would get processed and removed at a later date? 
		void* function = nullptr;
		void* data = nullptr;
		//jobBuffer.Swap(jobBuffer.Append(), std::make_tuple((jobRun_t)function, data, syncType));
		jobBuffer.Append(std::make_tuple((jobRun_t)function, data, syncType));
	}
}

void cweeparallel_ThreadList::Wait() {
	if (jobListThreads != NULL) {
		jobListThreads->Wait();
		do {
			Submit();
			jobListThreads->Wait();
		} while (jobBuffer.Num() != 0);
	}
}

bool cweeparallel_ThreadList::TryWait() {
	if (jobListThreads != NULL) {
		return jobListThreads->TryWait();
	}
	return true;
}

int	cweeparallel_ThreadList::GetNumExecutingThreads() {
	return jobListThreads->GetNumExecutingThreads();
}

void cweeparallel_ThreadList::AllowMultithreading(bool T_or_F) {
	jobListThreads->AllowMultithreading(T_or_F);
}

int	cweeparallel_ThreadList::GetMultithreadingSetting(void) {
	return jobListThreads->GetMultithreadingSetting();
}

void cweeparallel_ThreadList::Submit(cweeparallel_ThreadList* waitForJobList, int parallelism) {
	assert(waitForJobList != this);
	
	// cweeStackTrace::GetTrace();

	if (JOB_ASSIGNMENT_MODE > 0) {
		bool mustDoAll = false;
		for (; jobBuffer.Num() > 0;) {
			std::tuple<jobRun_t, void*, jobSyncType_t>& it = jobBuffer[0];
			if (std::get<2>(it) == SYNC_NONE) {
				jobListThreads->AddParallelJob(std::get<0>(it), std::get<1>(it));
			}
			else {
				jobListThreads->InsertSyncPoint(std::get<2>(it));
				mustDoAll = true;
			}
			jobBuffer.RemoveIndex(0);

			if (!mustDoAll) break;
		}
	}
	else {
		for (int i = 0; i < jobBuffer.Num(); i++) { // what if there's already a buffer of old jobs to be done? Add them now. 
			auto& it = jobBuffer[i];
			if (std::get<2>(it) == SYNC_NONE)
				jobListThreads->AddParallelJob(std::get<0>(it), std::get<1>(it));
			else
				jobListThreads->InsertSyncPoint(std::get<2>(it));
		}
		cweeThreadedList<std::tuple<jobRun_t, void*, jobSyncType_t>> newJobList; // start anew without modifying or deleting old data. Just drop it. 	
		jobBuffer = newJobList;
	}

	// cweeStackTrace::GetTrace();

	jobListThreads->Submit((waitForJobList != NULL) ? waitForJobList->jobListThreads : NULL, parallelism);	
}

bool cweeparallel_ThreadList::IsSubmitted() const {
	return jobListThreads->IsSubmitted();
}

unsigned int cweeparallel_ThreadList::GetNumExecutedJobs() const {
	return jobListThreads->GetNumExecutedJobs();
}

unsigned int cweeparallel_ThreadList::GetNumSyncs() const {
	return jobListThreads->GetNumSyncs();
}

uint64 cweeparallel_ThreadList::GetSubmitTimeMicroSec() const {
	return jobListThreads->GetSubmitTimeMicroSec();
}

uint64 cweeparallel_ThreadList::GetStartTimeMicroSec() const {
	return jobListThreads->GetStartTimeMicroSec();
}

uint64 cweeparallel_ThreadList::GetFinishTimeMicroSec() const {
	return jobListThreads->GetFinishTimeMicroSec();
}

uint64 cweeparallel_ThreadList::GetWaitTimeMicroSec() const {
	return jobListThreads->GetWaitTimeMicroSec();
}

uint64 cweeparallel_ThreadList::GetTotalProcessingTimeMicroSec() const {
	return jobListThreads->GetTotalProcessingTimeMicroSec();
}

uint64 cweeparallel_ThreadList::GetTotalWastedTimeMicroSec() const {
	return jobListThreads->GetTotalWastedTimeMicroSec();
}

uint64 cweeparallel_ThreadList::GetUnitProcessingTimeMicroSec(int unit) const {
	return jobListThreads->GetUnitProcessingTimeMicroSec(unit);
}

uint64 cweeparallel_ThreadList::GetUnitWastedTimeMicroSec(int unit) const {
	return jobListThreads->GetUnitWastedTimeMicroSec(unit);
}

const jobType& cweeparallel_ThreadList::GetId() const {
	return jobListThreads->GetId();
}

const jobListPriority_t& cweeparallel_ThreadList::GetPriority() const {
	return jobListThreads->GetPriority();
}



















#define JOB_THREAD_CORES	{	CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY,	\
								CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY,	\
								CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY,	\
								CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY,	\
								CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY,	\
								CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY,	\
								CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY,	\
								CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY } // 32 max cores hard-coded

struct managerStats_t {
	unsigned int    cumulativeExecutedJobs;
	unsigned int    prevExecutedJobs;

	unsigned int    cumulativeProcessingTime;
	unsigned int    prevProcessingTime;
};

class cweeparallelProcessorLocal : public cweeparallelProcessor {
public:
	virtual							~cweeparallelProcessorLocal() {}

	// Convenient multi-threading functions for general users. 
	virtual void					AllowMultithreading(bool T_or_F);
	virtual int						GetMultithreadingSetting();
	virtual void					Think();

	virtual void					Init();
	virtual void					Shutdown();
	virtual void					SetNumThreads(int num);

	virtual cweeparallel_ThreadList* AllocJobList(jobType id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs);
	void							Submit(cweeparallel_JobList_Threads* jobList, int parallelism);
	virtual void					WaitForAllJobLists();
	virtual bool					TryWaitAllJobLists();
	virtual void					FreeJobList(cweeparallel_ThreadList* jobList);

	virtual int						GetNumJobLists() const;
	virtual int						GetNumFreeJobLists() const;
	virtual cweeparallel_ThreadList* GetJobList(int index);
	virtual cweeparallel_ThreadList* GetJobList(jobType id, jobListPriority_t priority = jobListPriority_t::JOBLIST_PRIORITY_MEDIUM);
	virtual int						GetNumStagedJobs() const;
	virtual int						GetTotalProcessingTimeMicroSec() const;
	virtual int						GetTotalJobsExecuted() const;
	virtual void					UpdateStates();
	virtual int						GetCumulativeJobsExecuted() const;
	virtual int						GetCumulativeProcessingTimeMicroSec() const;

	virtual int						GetNumProcessingUnits();
	virtual int						GetNumPhysicalCpuCores() { return numPhysicalCpuCores; };
	virtual int						GetNumLocalCpuCores() { return numLogicalCpuCores; };
	virtual int						GetNumCpuPackages() { return numCpuPackages; };

	virtual cweeStr					GetJobName(jobRun_t func) { return cweeStr(Get_ThreadJobName(func)); };
	virtual cweeStr					GetThreadName(jobType id) { return cweeStr(Get_ThreadListName(id));	};

	virtual	void					SetJobAssignmentMode(int in) {
		JobAssignmentMode = in; JOB_ASSIGNMENT_MODE = in;
	};
	virtual int						GetJobAssignmentMode() { 
		int jAM;
		JobAssignmentMode.Lock();
		jAM = *JobAssignmentMode.UnsafeRead();
		JobAssignmentMode.Unlock();
		return jAM;	
	};

private:
	cweeUnpooledInterlocked<int>			JobAssignmentMode = 1; // -1 = old, 1 = new
	mutable cweeSysMutex			parallelProcessorMutex;
	managerStats_t					stats;
	cweeThread						threads[MAX_JOB_THREADS];
	unsigned int					maxThreads;
	int								numPhysicalCpuCores;
	int								numLogicalCpuCores;
	int								numCpuPackages;
	cweeThreadedList< cweeparallel_ThreadList* >	jobLists;
	//cweeStaticList< cweeparallel_ThreadList*, MAX_NUM_PARALLEL_THREADS >	jobLists;
};

cweeparallelProcessorLocal parallelProcessorLocal;
cweeparallelProcessor* parallelProcessor = &parallelProcessorLocal;








int	cweeparallelProcessorLocal::GetCumulativeJobsExecuted() const {
	int out = 0;
	parallelProcessorMutex.Lock();
	out = stats.cumulativeExecutedJobs;
	parallelProcessorMutex.Unlock();
	return out;
}

int	cweeparallelProcessorLocal::GetCumulativeProcessingTimeMicroSec() const {
	int out = 0;
	parallelProcessorMutex.Lock();
	out = stats.cumulativeProcessingTime;
	parallelProcessorMutex.Unlock();
	return out;
}

void cweeparallelProcessorLocal::UpdateStates() {
	parallelProcessorMutex.Lock();
	unsigned int prevJobs = stats.prevExecutedJobs;
	stats.prevExecutedJobs = GetTotalJobsExecuted();
	stats.cumulativeExecutedJobs += cweeMath::Fmax(0, stats.prevExecutedJobs - prevJobs);

	unsigned int prevTime = stats.prevProcessingTime;
	stats.prevProcessingTime = GetTotalProcessingTimeMicroSec();
	stats.cumulativeProcessingTime += cweeMath::Fmax(0, stats.prevProcessingTime - prevTime);
	parallelProcessorMutex.Unlock();
}

void SubmitJobList(cweeparallel_JobList_Threads* jobList, int parallelism) {
	parallelProcessorLocal.Submit(jobList, parallelism);
}

void cweeparallelProcessorLocal::SetNumThreads(int num) {
	NUM_JOB_THREADS = cweeMath::min(cweeMath::max(num, 1), MAX_JOB_THREADS);
}

void cweeparallelProcessorLocal::Init() {
	// on select hardware, we can choose specific cores for specific threads, but on most PC's they will all be CORE_ANY
	core_t cores[] = JOB_THREAD_CORES;
	AUTO G = parallelProcessorMutex.Guard();
	Sys_CPUCount(numLogicalCpuCores, numPhysicalCpuCores, numCpuPackages);

	for (int i = 0; i < MAX_JOB_THREADS; i++) threads[i].Start(cores[i], i);

	maxThreads = (int)NUM_JOB_THREADS;
	SetNumThreads(cweeMath::max(cweeMath::max(numLogicalCpuCores, numPhysicalCpuCores), 0)); // allow one thread for the application and operating system.
}

void cweeparallelProcessorLocal::Shutdown() {
	AUTO G = parallelProcessorMutex.Guard();
	for (int i = 0; i < MAX_JOB_THREADS; i++) threads[i].StopThread();
}

void cweeparallelProcessorLocal::Think() {
	AUTO G = parallelProcessorMutex.Guard();
	for (int i = 0; i < jobLists.Num(); i++) {
		if (jobLists[i]->TryWait()) {			
			jobLists[i]->Submit(NULL, -1);
		}
	}
}

void cweeparallelProcessorLocal::AllowMultithreading(bool T_or_F) {
	AUTO G = parallelProcessorMutex.Guard();
	for (int i = 0; i < jobLists.Num(); i++)
		jobLists[i]->AllowMultithreading(T_or_F);
}

int cweeparallelProcessorLocal::GetMultithreadingSetting() {
	int out = 0;
	parallelProcessorMutex.Lock();
	for (int i = 0; i < jobLists.Num(); i++) {
		out = jobLists[i]->GetMultithreadingSetting();
		break;
	}
	parallelProcessorMutex.Unlock();
	return out;
}

cweeparallel_ThreadList* cweeparallelProcessorLocal::AllocJobList(jobType id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs) {
	cweeparallel_ThreadList* out = nullptr;

	AUTO G = parallelProcessorMutex.Guard();
	for (auto& x : jobLists) {
		if (x->GetId() == id && x->GetPriority() == priority) {
			out = x;
			break;
		}
	}
	if (out != nullptr) {
		return out;
	}

	out = new (TAG_JOBLIST) cweeparallel_ThreadList(id, priority, maxJobs, maxSyncs, &JobHandles);
	jobLists.Append(out);
	return out;
}

void cweeparallelProcessorLocal::FreeJobList(cweeparallel_ThreadList* jobList) {
	if (jobList == NULL) {
		return;
	}
	AUTO G = parallelProcessorMutex.Guard();
	// wait for all job threads to finish because job list deletion is not thread safe
	for (unsigned int i = 0; i < maxThreads; i++) {
		threads[i].WaitForThread();
	}
	int index = jobLists.FindIndex(jobList);
	assert(index >= 0 && jobLists[index] == jobList);
	jobLists[index]->Wait();
	delete jobLists[index];
	jobLists.RemoveIndexFast(index);
}

int cweeparallelProcessorLocal::GetNumJobLists() const {
	int out = 0;
	AUTO G = parallelProcessorMutex.Guard();
	out = jobLists.Num();
	return out;
}

int cweeparallelProcessorLocal::GetNumFreeJobLists() const {
	int out = 0;
	AUTO G = parallelProcessorMutex.Guard();
	out = MAX_NUM_PARALLEL_THREADS - jobLists.Num();
	return out;
}

cweeparallel_ThreadList* cweeparallelProcessorLocal::GetJobList(int index) {
	cweeparallel_ThreadList* out = NULL;
	AUTO G = parallelProcessorMutex.Guard();
	out = jobLists[index];
	return out;
}

cweeparallel_ThreadList* cweeparallelProcessorLocal::GetJobList(jobType id, jobListPriority_t priority) {
	cweeparallel_ThreadList* out = nullptr;
	parallelProcessorMutex.Lock();
	for (auto& x : jobLists) {
		if (x->GetId() == id && x->GetPriority() == priority) {
			out = x;
			break;
		}
	}
	parallelProcessorMutex.Unlock();
	if (out != nullptr)
		return out;
	else
		return AllocJobList((jobType)id, (jobListPriority_t)priority, MAX_REGISTERED_THREAD_JOBS, 100);
}

int cweeparallelProcessorLocal::GetNumStagedJobs() const {
	int num = 0;
	for (int i = 0; i < jobLists.Num(); i++)
		num += jobLists[i]->GetNumStagedJobs();
	return num;
}

int cweeparallelProcessorLocal::GetTotalProcessingTimeMicroSec() const {
	int num = 0;
	for (int i = 0; i < jobLists.Num(); i++)
		num += jobLists[i]->GetTotalProcessingTimeMicroSec();
	return num;
}

int cweeparallelProcessorLocal::GetTotalJobsExecuted() const {
	int num = 0;
	for (int i = 0; i < jobLists.Num(); i++)
		num += jobLists[i]->GetNumExecutedJobs();
	return num;
}

int cweeparallelProcessorLocal::GetNumProcessingUnits() {
	int out = 0;
	out = maxThreads;
	return out;
}

void cweeparallelProcessorLocal::WaitForAllJobLists() {
	while (1) {
		Think();
		if (TryWaitAllJobLists() == true) break;
	}
}

bool cweeparallelProcessorLocal::TryWaitAllJobLists() {
	bool free = true;
	parallelProcessorMutex.Lock();
	for (int i = 0; i < jobLists.Num(); i++)
		free = ((free == true) && (jobLists[i]->TryWait() == true));
	parallelProcessorMutex.Unlock();
	return free;
}

void cweeparallelProcessorLocal::Submit(cweeparallel_JobList_Threads* jobList, int parallelism) {
	//parallelProcessorMutex.Lock(); // potentially unsafe // TODO

	int jAM;
	JobAssignmentMode.Lock();
	jAM = *JobAssignmentMode.UnsafeRead();
	JobAssignmentMode.Unlock();

	if (jAM <= 0) {
		// determine the number of threads to use
		int numThreads = cweeMath::min(MAX_JOB_THREADS, cweeMath::max(1, (jobList->GetNumQueuedJobs() - 1)));
		if (parallelism == JOBLIST_PARALLELISM_DEFAULT) {
			numThreads = cweeMath::min(numThreads, NUM_JOB_THREADS);
		}
		else if (parallelism == JOBLIST_PARALLELISM_MAX_CORES) {
			numThreads = cweeMath::min(numThreads, numPhysicalCpuCores);
		}
		else if (parallelism == JOBLIST_PARALLELISM_MAX_THREADS) {
			numThreads = cweeMath::min(numThreads, numLogicalCpuCores);
		}
		else if (parallelism > MAX_JOB_THREADS) {
			numThreads = cweeMath::min(numThreads, MAX_JOB_THREADS);
		}
		else {
			numThreads = parallelism;
		}

		// No threads = do the work immediately.
		if (numThreads <= 0) {
			threadListState_t state(jobList->GetVersion());
			jobList->RunJobs(0, state, false);
			//parallelProcessorMutex.Unlock();
			return;
		}

		// Balance untold number of competing parallel threads with limited number of CPU cores. 
		// I *know* there are better strategies for this, which UC Davis has published on: 
		// TO-DO: Task stealing is a known strategy that performs well here. 

		// A minimum of one CPU has to "own" this jobList. Each logical CPU is assigned ownership of one jobList. 
		threads[(int)jobList->GetId()].AddParallelJobList(jobList);
		threads[(int)jobList->GetId()].SignalWork();
		numThreads--;

		int numRemove1 = 0;
		parallelProcessorMutex.Lock();
		for (int j = 0; j < jobLists.Num(); j++) {
			if (jobLists[j]->GetId() != (int)jobList->GetId()) {
				if (!threads[(int)jobLists[j]->GetId()].IsWorkDone() || threads[(int)jobLists[j]->GetId()].IsRunning()) { // !threads[(int)jobLists[j]->GetId()].IsWorkDone() && 
					numRemove1++; // remove the list of possible threads by the number already working.
				}
			}
		}
		parallelProcessorMutex.Unlock();

		int numRemove2 = 0;
		for (int i = 0; i < MAX_JOB_THREADS; i++) {
			if (i != (int)jobList->GetId()) {
				if (!threads[i].IsWorkDone() || threads[i].IsRunning()) { // !threads[i].IsWorkDone() && 
					numRemove2++; // remove the list of possible threads by the number already working.
				}
			}
		}
		numThreads -= cweeMath::min(numRemove2, numRemove1);


		for (int i = 0; i < MAX_JOB_THREADS; i++) {
			// Do not assign a job to another jobList - this should prevent one job list from locking down another. 
			bool mustSkip = false;
			parallelProcessorMutex.Lock();
			for (int j = 0; j < jobLists.Num(); j++) {
				if (i == (int)jobLists[j]->GetId()) {
					mustSkip = true;
					break;
				}
			}
			parallelProcessorMutex.Unlock();
			if (mustSkip == true) continue;

			if (numThreads <= 0) return; // ensure we don't wake up excess cores otherwise CPU will chug		
				
			// wake the core up and notify it that it is a candidate to do work.
			threads[i].AddParallelJobList(jobList);
			threads[i].SignalWork();
			numThreads--;
		}

	} 
	else {

#if 1
		
		int numThreads = cweeMath::max(0, cweeMath::min(NUM_JOB_THREADS, MAX_JOB_THREADS));
		if (numThreads == 0) {
			// No threads = do the work immediately.
			if (numThreads <= 0) {
				threadListState_t state(jobList->GetVersion());
				jobList->RunJobs(0, state, false);
				return;
			}
		}
		else {
			// theory: user is asking for a certain number of threads. 
			// this function is only aware of a list (size 1 to size INF) of jobs for this jobType_t AND this jobPriority_t
			// it doesn't know about other jobs submitted to the same threads simultaneously, even with the same jobType_t. 
			// it also doesn't know about jobs waiting for assignment, which will require this submission being completed. 
			
			// additionally, we can not find out what jobs are submitted - we only know whether that CPU core is working or not. (i.e. threads[0].IsWorkDone() )
			int numWorkingThreads(0); for (int i = 0; i < MAX_JOB_THREADS; i++) if (!threads[i].IsWorkDone()) numWorkingThreads++;						
			int numJobs = cweeMath::max(1,jobList->GetNumQueuedJobs());

			if (numWorkingThreads < numThreads) {
				// we are under-budget, and should assign work to a CPU that is not currently working already (We should be able to work immediately)
				if (threads[(int)jobList->GetId()].IsWorkDone() && numJobs > 0) { // ideally, assign to this thread's owning CPU
					threads[(int)jobList->GetId()].AddParallelJobList(jobList);
					threads[(int)jobList->GetId()].SignalWork();
					return;
					// numJobs--;
				}
				
				for (int i = jobType::PRIORITY_thread + 1; i < MAX_JOB_THREADS; i++) // otherwise, assign to any of the non-assigned CPUs
					if (threads[i].IsWorkDone() && numJobs > 0) {
						threads[i].AddParallelJobList(jobList);
						threads[i].SignalWork();
						return;
						// numJobs--;
					}

				if (numJobs > 0) {
					threads[(int)jobList->GetId()].AddParallelJobList(jobList); // something is wrong and every single thread was working - submit to the owner and move on. 
					threads[(int)jobList->GetId()].SignalWork();
					return;
					// numJobs--;
				}

			} else {

				// we are on-budget or over-budget, and should assign work to a CPU that is currently working already (essentially we are getting queued)
				if (!threads[(int)jobList->GetId()].IsWorkDone() && numJobs > 0) { // ideally, assign to this thread's owning CPU
					threads[(int)jobList->GetId()].AddParallelJobList(jobList);
					threads[(int)jobList->GetId()].SignalWork();
					return;
					// numJobs--;
				}

				for (int i = jobType::PRIORITY_thread + 1; i < MAX_JOB_THREADS; i++) // otherwise, assign to any of the non-assigned CPUs
					if (!threads[i].IsWorkDone() && numJobs > 0) {
						threads[i].AddParallelJobList(jobList);
						threads[i].SignalWork();
						return;
						// numJobs--;
					}

				for (int i = 0; i < MAX_JOB_THREADS; i++) // otherwise, assign to any CPU
					if (!threads[i].IsWorkDone() && numJobs > 0) {
						threads[i].AddParallelJobList(jobList);
						threads[i].SignalWork();
						return;
						// numJobs--;
					}

				if (numJobs > 0) {
					threads[(int)jobList->GetId()].AddParallelJobList(jobList); // something is wrong and every single thread was not working - submit to the owner and move on. 
					threads[(int)jobList->GetId()].SignalWork();
					return;
					// numJobs--;
				}
			}

		}





#else
		// worked, but could result in stalling
		int numThreads = cweeMath::max(0, cweeMath::min(NUM_JOB_THREADS, MAX_JOB_THREADS));
		if (numThreads == 0) {
			// No threads = do the work immediately.
			if (numThreads <= 0) {
				threadListState_t state(jobList->GetVersion());
				jobList->RunJobs(0, state, false);
				return;
			}
		}
		else {
			if (numThreads < jobType::PRIORITY_thread) {
				// we cannot give the threads the breathing room they require
				int num = 0;
				for (int i = 0; i < numThreads; i++) {
					if (threads[i].IsWorkDone()) {
						threads[i].AddParallelJobList(jobList);
						threads[i].SignalWork();
						return;
					}
				}
				int assignment = cweeRandomInt(0, numThreads - 1);
				threads[assignment].AddParallelJobList(jobList);
				threads[assignment].SignalWork();
			}
			else {
				// we can give the threads the breathing room they require
				if (threads[(int)jobList->GetId()].IsWorkDone()) {
					threads[(int)jobList->GetId()].AddParallelJobList(jobList);
					threads[(int)jobList->GetId()].SignalWork();
					return;
				}

				int start = jobType::PRIORITY_thread;
				int end = numThreads;
				for (int i = start; i < end; i++) {
					if (threads[i].IsWorkDone()) {
						threads[i].AddParallelJobList(jobList);
						threads[i].SignalWork();
						return;
					}
				}

				int assignment = cweeRandomInt(0, numThreads - 1);
				threads[assignment].AddParallelJobList(jobList);
				threads[assignment].SignalWork();
			}
		}
#endif
	}

	//parallelProcessorMutex.Unlock(); // potentially unsafe // TODO
}















