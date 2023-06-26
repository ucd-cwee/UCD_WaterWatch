
#pragma hdrstop
#include "precompiled.h"
#include "ConcurrentJobs.h"

//2 threads, 4 break the game.
//3 threads, 5 breaks the game.
//4 threads, 6 break the game.
//32 max, 2 num; >6 didn't break. Probably needs ~34. 
const static int		MAX_THREADS = 32;
#define MAX_JOB_THREADS		1
#define NUM_JOB_THREADS		"1"
const int JOB_THREAD_STACK_SIZE = 256 * 1024;
extern void Sys_CPUCount(int& logicalNum, int& coreNum, int& packageNum);



/*
================================================================================================

	Job and Job-List names

================================================================================================
*/
const char* jobNames[] = {
	ASSERT_ENUM_STRING(JOBLIST_FRONTEND,	0),
	ASSERT_ENUM_STRING(JOBLIST_BACKEND,		1),
	ASSERT_ENUM_STRING(JOBLIST_PARALLEL,    2),
	ASSERT_ENUM_STRING(JOBLIST_UTILITY,		9),
};

static const int MAX_REGISTERED_JOBS = 1024;

struct registeredJob {
	jobRun_t		function;
	const char* name;
} registeredJobs[MAX_REGISTERED_JOBS];
static int numRegisteredJobs;

const char* GetJobListName(jobListId_t id) {
	return jobNames[id];
}

static bool IsRegisteredJob(jobRun_t function) {
	for (int i = 0; i < numRegisteredJobs; i++) {
		if (registeredJobs[i].function == function) {
			return true;
		}
	}
	return false;
}

void RegisterJob(jobRun_t function, const char* name) {
	if (IsRegisteredJob(function)) {
		return;
	}
	registeredJobs[numRegisteredJobs].function = function;
	registeredJobs[numRegisteredJobs].name = name;
	numRegisteredJobs++;
}

const char* GetJobName(jobRun_t function) {
	for (int i = 0; i < numRegisteredJobs; i++) {
		if (registeredJobs[i].function == function) {
			return registeredJobs[i].name;
		}
	}
	return "unknown";
}

cweeParallelJobRegistration::cweeParallelJobRegistration(jobRun_t function, const char* name) {
	RegisterJob(function, name);
}




struct threadJobListState_t {
	threadJobListState_t() :
		jobList(NULL),
		version(0xFFFFFFFF),
		signalIndex(0),
		lastJobIndex(0),
		nextJobIndex(-1) {}
	threadJobListState_t(int _version) :
		jobList(NULL),
		version(_version),
		signalIndex(0),
		lastJobIndex(0),
		nextJobIndex(-1) {}
	cweeParallelJobList_Threads* jobList;
	int							version;
	int							signalIndex;
	int							lastJobIndex;
	int							nextJobIndex;
};

struct threadStats_t {
	unsigned int	numExecutedJobs;
	unsigned int	numExecutedSyncs;
	uint64			submitTime;
	uint64			startTime;
	uint64			endTime;
	uint64			waitTime;
	uint64			threadExecTime[MAX_THREADS];
	uint64			threadTotalTime[MAX_THREADS];
};


class cweeParallelJobList_Threads {
public:
	cweeParallelJobList_Threads(jobListId_t id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs);
	~cweeParallelJobList_Threads();

	//------------------------
	// These are called from the one thread that manages this list.
	//------------------------
	INLINE void				AddParallelJob(jobRun_t function, void* data);
	INLINE void				InsertSyncPoint(jobSyncType_t syncType);
	void					Submit(cweeParallelJobList_Threads* waitForJobList_, int parallelism);
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

	jobListId_t				GetId() const { return listId; }
	jobListPriority_t		GetPriority() const { return listPriority; }
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

	int						RunJobs(unsigned int threadNum, threadJobListState_t& state, bool singleJob);

private:
	static const int		NUM_DONE_GUARDS = 4;	// cycle through 4 guards so we can cyclicly chain job lists

	bool					threaded;
	bool					done;
	bool					hasSignal;
	jobListId_t				listId;
	jobListPriority_t		listPriority;
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
	cweeThreadedList< job_t, TAG_JOBLIST >		jobList;
	cweeThreadedList< cweeSysInterlockedInteger, TAG_JOBLIST >	signalJobCount;
	cweeSysInterlockedInteger				currentJob;
	cweeSysInterlockedInteger				fetchLock;
	cweeSysInterlockedInteger				numThreadsExecuting;

	threadStats_t						deferredThreadStats;
	threadStats_t						threadStats;

	int						RunJobsInternal(unsigned int threadNum, threadJobListState_t& state, bool singleJob);

	static void				Nop(void* data) {}

	static int				JOB_SIGNAL;
	static int				JOB_SYNCHRONIZE;
	static int				JOB_LIST_DONE;
};

int cweeParallelJobList_Threads::JOB_SIGNAL;
int cweeParallelJobList_Threads::JOB_SYNCHRONIZE;
int cweeParallelJobList_Threads::JOB_LIST_DONE;


cweeParallelJobList_Threads::cweeParallelJobList_Threads(jobListId_t id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs) :
	threaded(true),
	done(true),
	hasSignal(false),
	listId(id),
	listPriority(priority),
	numSyncs(0),
	lastSignalJob(0),
	waitForGuard(NULL),
	currentDoneGuard(0),
	jobList() {

	assert(listPriority != JOBLIST_PRIORITY_NONE);

	this->maxJobs = maxJobs;
	this->maxSyncs = maxSyncs;
	jobList.AssureSize(maxJobs + maxSyncs * 2 + 1);	// syncs go in as dummy jobs and one more to update the doneCount
	jobList.SetNum(0);
	signalJobCount.AssureSize(maxSyncs + 1);			// need one extra for submit
	signalJobCount.SetNum(0);

	memset(&deferredThreadStats, 0, sizeof(threadStats_t));
	memset(&threadStats, 0, sizeof(threadStats_t));
}

cweeParallelJobList_Threads::~cweeParallelJobList_Threads() {
	Wait();
}

INLINE void cweeParallelJobList_Threads::AddParallelJob(jobRun_t function, void* data) {
	assert(done);

	// make sure there isn't already a job with the same function and data in the list
	// comment this out before release.
	if (jobList.Num() < 1000) {	// don't do this N^2 slow check on big lists
		for (int i = 0; i < jobList.Num(); i++) {
			if (jobList[i].function == function && jobList[i].data == data) // duplicate, submitting to engine is gurranteed to crash the thread. It's undefined behavior. 
				return;
		}
	}

	job_t& job = jobList.Alloc();
	job.function = function;
	job.data = data;
	job.executed = 0;
}

INLINE void cweeParallelJobList_Threads::InsertSyncPoint(jobSyncType_t syncType) {
	assert(done);
	switch (syncType) {
	case SYNC_SIGNAL: {
		assert(!hasSignal);
		if (jobList.Num()) {
			assert(!hasSignal);
			signalJobCount.Alloc();
			signalJobCount[signalJobCount.Num() - 1].SetValue(jobList.Num() - lastSignalJob);
			lastSignalJob = jobList.Num();
			job_t& job = jobList.Alloc();
			job.function = Nop;
			job.data = &JOB_SIGNAL;
			hasSignal = true;
		}
		break;
	}
	case SYNC_SYNCHRONIZE: {
		if (hasSignal) {
			job_t& job = jobList.Alloc();
			job.function = Nop;
			job.data = &JOB_SYNCHRONIZE;
			hasSignal = false;
			numSyncs++;
		}
		break;
	}
	}
}

void cweeParallelJobList_Threads::Submit(cweeParallelJobList_Threads* waitForJobList, int parallelism) {
	assert(done);
	assert(numSyncs <= maxSyncs);
	assert((unsigned int)jobList.Num() <= maxJobs + numSyncs * 2);
	assert(fetchLock.GetValue() == 0);

	done = false;
	currentJob.SetValue(0);

	memset(&deferredThreadStats, 0, sizeof(deferredThreadStats));
	deferredThreadStats.numExecutedJobs = jobList.Num() - numSyncs * 2;
	deferredThreadStats.numExecutedSyncs = numSyncs;
	deferredThreadStats.submitTime = Sys_Microseconds();
	deferredThreadStats.startTime = 0;
	deferredThreadStats.endTime = 0;
	deferredThreadStats.waitTime = 0;

	if (jobList.Num() == 0) {
		return;
	}

	if (waitForJobList != NULL) {
		waitForGuard = &waitForJobList->doneGuards[waitForJobList->currentDoneGuard];
	}
	else {
		waitForGuard = NULL;
	}

	currentDoneGuard = (currentDoneGuard + 1) & (NUM_DONE_GUARDS - 1);
	doneGuards[currentDoneGuard].SetValue(1);

	signalJobCount.Alloc();
	signalJobCount[signalJobCount.Num() - 1].SetValue(jobList.Num() - lastSignalJob);

	job_t& job = jobList.Alloc();
	job.function = Nop;
	job.data = &JOB_LIST_DONE;

	if (threaded) {
		// hand over to the manager
		void SubmitJobList(cweeParallelJobList_Threads * jobList, int parallelism);
		SubmitJobList(this, parallelism);
	}
	else {
		// run all the jobs right here
		threadJobListState_t state(GetVersion());
		RunJobs(0, state, false);
	}
}

void cweeParallelJobList_Threads::Wait() {
	if (jobList.Num() > 0)
	{
		// don't lock up but return if the job list was never properly submitted
		if (!verify(!done && signalJobCount.Num() > 0)) {
			return;
		}

		bool waited = false;
		uint64 waitStart = Sys_Microseconds();

		while (signalJobCount[signalJobCount.Num() - 1].GetValue() > 0)
		{
			Sys_Yield();
			waited = true;
		}
		version.Increment();
		while (numThreadsExecuting.GetValue() > 0)
		{
			Sys_Yield();
			waited = true;
		}

		jobList.SetNum(0);
		signalJobCount.SetNum(0);
		numSyncs = 0;
		lastSignalJob = 0;

		uint64 waitEnd = Sys_Microseconds();
		deferredThreadStats.waitTime = waited ? (waitEnd - waitStart) : 0;
	}
	memcpy(&threadStats, &deferredThreadStats, sizeof(threadStats));
	done = true;
}

bool cweeParallelJobList_Threads::TryWait() {
	if (jobList.Num() == 0 || signalJobCount[signalJobCount.Num() - 1].GetValue() <= 0)
	{
		Wait();
		return true;
	}
	return false;
}

bool cweeParallelJobList_Threads::IsSubmitted() const {
	return !done;
}

uint64 cweeParallelJobList_Threads::GetTotalProcessingTimeMicroSec() const {
	uint64 total = 0;
	for (int unit = 0; unit < MAX_THREADS; unit++) {
		total += threadStats.threadExecTime[unit];
	}
	return total;
}

uint64 cweeParallelJobList_Threads::GetTotalWastedTimeMicroSec() const {
	uint64 total = 0;
	for (int unit = 0; unit < MAX_THREADS; unit++) {
		total += threadStats.threadTotalTime[unit] - threadStats.threadExecTime[unit];
	}
	return total;
}

uint64 cweeParallelJobList_Threads::GetUnitProcessingTimeMicroSec(int unit) const {
	if (unit < 0 || unit >= MAX_THREADS) {
		return 0;
	}
	return threadStats.threadExecTime[unit];
}

uint64 cweeParallelJobList_Threads::GetUnitWastedTimeMicroSec(int unit) const {
	if (unit < 0 || unit >= MAX_THREADS) {
		return 0;
	}
	return threadStats.threadTotalTime[unit] - threadStats.threadExecTime[unit];
}

int cweeParallelJobList_Threads::RunJobsInternal(unsigned int threadNum, threadJobListState_t& state, bool singleJob) {
	if (state.version != version.GetValue()) {
		// trying to run an old version of this list that is already done
		return RUN_DONE;
	}

	assert(threadNum < MAX_THREADS);

	if (deferredThreadStats.startTime == 0) {
		deferredThreadStats.startTime = Sys_Microseconds();	// first time any thread is running jobs from this list
	}

	int result = RUN_OK;

	do {
		// run through all signals and syncs before the last job that has been or is being executed
		// this loop is really an optimization to minimize the time spent in the fetchLock section below
		for (; state.lastJobIndex < (int)currentJob.GetValue() && state.lastJobIndex < jobList.Num(); state.lastJobIndex++) {
			if (jobList[state.lastJobIndex].data == &JOB_SIGNAL) {
				state.signalIndex++;
				assert(state.signalIndex < signalJobCount.Num());
			}
			else if (jobList[state.lastJobIndex].data == &JOB_SYNCHRONIZE) {
				assert(state.signalIndex > 0);
				if (signalJobCount[state.signalIndex - 1].GetValue() > 0) {
					// stalled on a synchronization point
					return (result | RUN_STALLED);
				}
			}
			else if (jobList[state.lastJobIndex].data == &JOB_LIST_DONE) {
				if (signalJobCount[signalJobCount.Num() - 1].GetValue() > 0) {
					// stalled on a synchronization point
					return (result | RUN_STALLED);
				}
			}
		}

		// try to lock to fetch a new job
		if (fetchLock.Increment() == 1) {

			// grab a new job
			state.nextJobIndex = currentJob.Increment() - 1;

			// run through any remaining signals and syncs (this should rarely iterate more than once)
			for (; state.lastJobIndex <= state.nextJobIndex && state.lastJobIndex < jobList.Num(); state.lastJobIndex++) {
				if (jobList[state.lastJobIndex].data == &JOB_SIGNAL) {
					state.signalIndex++;
					assert(state.signalIndex < signalJobCount.Num());
				}
				else if (jobList[state.lastJobIndex].data == &JOB_SYNCHRONIZE) {
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
				else if (jobList[state.lastJobIndex].data == &JOB_LIST_DONE) {
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
		if (state.nextJobIndex >= jobList.Num()) {
			return (result | RUN_DONE);
		}

		// execute the next job
		{
			uint64 jobStart = Sys_Microseconds();

			jobList[state.nextJobIndex].function(jobList[state.nextJobIndex].data);
			jobList[state.nextJobIndex].executed = 1;

			uint64 jobEnd = Sys_Microseconds();
			deferredThreadStats.threadExecTime[threadNum] += jobEnd - jobStart;
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

int cweeParallelJobList_Threads::RunJobs(unsigned int threadNum, threadJobListState_t& state, bool singleJob) {
	uint64 start = Sys_Microseconds();
	numThreadsExecuting.Increment();
	int result = RunJobsInternal(threadNum, state, singleJob);
	numThreadsExecuting.Decrement();
	deferredThreadStats.threadTotalTime[threadNum] += Sys_Microseconds() - start;
	return result;
}

bool cweeParallelJobList_Threads::WaitForOtherJobList() {
	if (waitForGuard != NULL) {
		if (waitForGuard->GetValue() > 0) {
			return true;
		}
	}
	return false;
}













struct threadJobList_t {
	cweeParallelJobList_Threads* jobList;
	int							version;
};

class cweeJobThread : public cweeSysThread {
public:
	cweeJobThread();
	~cweeJobThread();

	void						Start(core_t core, unsigned int threadNum);

	void						AddParallelJobList(cweeParallelJobList_Threads* jobList);

private:
	threadJobList_t				jobLists[MAX_JOBLISTS];	// cyclic buffer with job lists
	unsigned int				firstJobList;			// index of the last job list the thread grabbed
	unsigned int				lastJobList;			// index where the next job list to work on will be added
	cweeSysMutex				AddParallelJobMutex;

	unsigned int				threadNum;

	virtual int					Run();
};

cweeJobThread::cweeJobThread() :
	firstJobList(0),
	lastJobList(0),
	threadNum(0) {
}

cweeJobThread::~cweeJobThread() {
}

void cweeJobThread::Start(core_t core, unsigned int threadNum) {
	this->threadNum = threadNum;
	StartWorkerThread((cweeStr("JobListProcessor_") + cweeStr(threadNum)).c_str(), core, THREAD_NORMAL, JOB_THREAD_STACK_SIZE);
}

void cweeJobThread::AddParallelJobList(cweeParallelJobList_Threads* jobList) {
	AddParallelJobMutex.Lock(); // must lock because multiple threads may try to add new job lists at the same time
	while (lastJobList - firstJobList >= MAX_JOBLISTS) { // wait until there is space available because in rare cases multiple versions of the same job lists may still be queued 
			//Sys_Yield();
		bool test = Sys_TryYield();
	}
	assert(lastJobList - firstJobList < MAX_JOBLISTS);
	jobLists[lastJobList & (MAX_JOBLISTS - 1)].jobList = jobList;
	jobLists[lastJobList & (MAX_JOBLISTS - 1)].version = jobList->GetVersion();
	lastJobList++;
	AddParallelJobMutex.Unlock();
}

int cweeJobThread::Run() {
	threadJobListState_t threadJobListState[MAX_JOBLISTS];
	int numJobLists = 0;
	int lastStalledJobList = -1;

	while (!IsTerminating()) {
		// fetch any new job lists and add them to the local list
		if (numJobLists < MAX_JOBLISTS && firstJobList < lastJobList) {
			threadJobListState[numJobLists].jobList = jobLists[firstJobList & (MAX_JOBLISTS - 1)].jobList;
			threadJobListState[numJobLists].version = jobLists[firstJobList & (MAX_JOBLISTS - 1)].version;
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
		jobListPriority_t priority = JOBLIST_PRIORITY_NONE;
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
		bool singleJob = (priority == JOBLIST_PRIORITY_HIGH) ? false : true;

		int result = threadJobListState[currentJobList].jobList->RunJobs(threadNum, threadJobListState[currentJobList], singleJob); // try running one or more jobs from the current job list

		if ((result & cweeParallelJobList_Threads::RUN_DONE) != 0) {

			for (int i = currentJobList; i < numJobLists - 1; i++) { // done with this job list so remove it from the local list
				threadJobListState[i] = threadJobListState[i + 1];
			}
			numJobLists--;
			lastStalledJobList = -1;
		}
		else if ((result & cweeParallelJobList_Threads::RUN_STALLED) != 0) {

			if (currentJobList == lastStalledJobList) { // yield when stalled on the same job list again without making any progress
				if ((result & cweeParallelJobList_Threads::RUN_PROGRESS) == 0) {
					//Sys_Yield();
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
cweeParallelJobList
================================================================================================
*/

cweeParallelJobList::cweeParallelJobList(jobListId_t id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs) {
	assert(priority > JOBLIST_PRIORITY_NONE);
	this->jobListThreads = new (TAG_JOBLIST) cweeParallelJobList_Threads(id, priority, maxJobs, maxSyncs);
}

cweeParallelJobList::~cweeParallelJobList() {
	delete jobListThreads;
}

void cweeParallelJobList::AddConcurrentJob(jobRun_t function, void* data) {
	// note: if we TryWait() here and it fails, it's not ready to recieve the list. 
	bool test = IsRegisteredJob(function);
	assert(test);
	if ((IsStalled() == true)) return;
	if (TryWait()) {
		jobListThreads->AddParallelJob(function, data);
	}
	else {
		// what if we create a seperate list of intermediate requests that would get processed and removed at a later date? 
		jobBuffer.Append(std::make_tuple(function, data, (jobSyncType_t)SYNC_NONE));
	}
}

void cweeParallelJobList::InsertSyncPoint(jobSyncType_t syncType) {
	if ((IsStalled() == true)) return;
	if (TryWait()) {
		jobListThreads->InsertSyncPoint(syncType);
	}
	else {
		// what if we create a seperate list of intermediate requests that would get processed and removed at a later date? 
		void* function = nullptr;
		void* data = nullptr;
		jobBuffer.Append(std::make_tuple((jobRun_t)function, data, syncType));
	}
}

void cweeParallelJobList::Wait() {
	if (jobListThreads != NULL) {
		jobListThreads->Wait();
		if ((IsStalled() == true)) {
			do {
				Submit();
				jobListThreads->Wait();
			} while (jobBuffer.Num() != 0);
		}
	}
}

bool cweeParallelJobList::TryWait() {
	if ((IsStalled() == true)) return false;
	bool out = true;
	if (jobListThreads != NULL) {
		out &= jobListThreads->TryWait();
	}
	return out;
}

void cweeParallelJobList::Submit(cweeParallelJobList* waitForJobList, int parallelism) {
	assert(waitForJobList != this);

	// what if there's already a buffer of old jobs to be done? Add them now. 
	if (jobBuffer.Num() > 0) {
		for (auto it : jobBuffer) {
			if (std::get<2>(it) == SYNC_NONE)
				jobListThreads->AddParallelJob(std::get<0>(it), std::get<1>(it));
			else
				jobListThreads->InsertSyncPoint(std::get<2>(it));
		}
	}
	cweeThreadedList<std::tuple<jobRun_t, void*, jobSyncType_t>> newJobList; // start anew without modifying or deleting old data. Just drop it. 
	jobBuffer = newJobList;
	//jobBuffer.Clear(); // this may not work...

	jobListThreads->Submit((waitForJobList != NULL) ? waitForJobList->jobListThreads : NULL, parallelism);
}

void cweeParallelJobList::PerformConcurrentJobs(cweeParallelJobList* waitForJobList, int parallelism) {
	Submit(waitForJobList, parallelism);
	Wait();
}

bool cweeParallelJobList::IsSubmitted() const {
	return jobListThreads->IsSubmitted();
}

unsigned int cweeParallelJobList::GetNumExecutedJobs() const {
	return jobListThreads->GetNumExecutedJobs();
}

unsigned int cweeParallelJobList::GetNumSyncs() const {
	return jobListThreads->GetNumSyncs();
}

uint64 cweeParallelJobList::GetSubmitTimeMicroSec() const {
	return jobListThreads->GetSubmitTimeMicroSec();
}

uint64 cweeParallelJobList::GetStartTimeMicroSec() const {
	return jobListThreads->GetStartTimeMicroSec();
}

uint64 cweeParallelJobList::GetFinishTimeMicroSec() const {
	return jobListThreads->GetFinishTimeMicroSec();
}

uint64 cweeParallelJobList::GetWaitTimeMicroSec() const {
	return jobListThreads->GetWaitTimeMicroSec();
}

uint64 cweeParallelJobList::GetTotalProcessingTimeMicroSec() const {
	return jobListThreads->GetTotalProcessingTimeMicroSec();
}

uint64 cweeParallelJobList::GetTotalWastedTimeMicroSec() const {
	return jobListThreads->GetTotalWastedTimeMicroSec();
}

uint64 cweeParallelJobList::GetUnitProcessingTimeMicroSec(int unit) const {
	return jobListThreads->GetUnitProcessingTimeMicroSec(unit);
}

uint64 cweeParallelJobList::GetUnitWastedTimeMicroSec(int unit) const {
	return jobListThreads->GetUnitWastedTimeMicroSec(unit);
}

jobListId_t cweeParallelJobList::GetId() const {
	return jobListThreads->GetId();
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

class cweeParallelJobManagerLocal : public cweeParallelJobManager {
public:
	virtual							~cweeParallelJobManagerLocal() {}
	virtual void					Think();

	virtual void					Init();
	virtual void					Shutdown();

	virtual bool					IsStalled() const;
	virtual void					Stall();
	virtual void					UnStall();

	virtual cweeParallelJobList* AllocJobList(jobListId_t id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs);
	void							Submit(cweeParallelJobList_Threads* jobList, int parallelism);
	virtual void					WaitForAllJobLists();
	virtual void					FreeJobList(cweeParallelJobList* jobList);

	virtual int						GetNumStagedJobs() const;
	virtual int						GetTotalProcessingTimeMicroSec() const;
	virtual int						GetTotalJobsExecuted() const;
	virtual void					UpdateStates();
	virtual int						GetCumulativeJobsExecuted() const;
	virtual int						GetCumulativeProcessingTimeMicroSec() const;

	virtual int						GetNumJobLists() const;
	virtual int						GetNumFreeJobLists() const;
	virtual cweeParallelJobList* GetJobList(int index);
	virtual cweeParallelJobList* GetJobList(jobListId_t id);

	virtual int						GetNumProcessingUnits();
	virtual int						GetNumPhysicalCpuCores() { return numPhysicalCpuCores; };
	virtual int						GetNumLocalCpuCores() { return numLogicalCpuCores; };
	virtual int						GetNumCpuPackages() { return numCpuPackages; };
private:
	managerStats_t					stats;
	cweeJobThread					threads[MAX_JOB_THREADS];
	unsigned int					maxThreads;
	int								numPhysicalCpuCores;
	int								numLogicalCpuCores;
	int								numCpuPackages;
	cweeStaticList< cweeParallelJobList*, MAX_JOBLISTS >	jobLists;
};

cweeParallelJobManagerLocal parallelJobManagerLocal;
cweeParallelJobManager* parallelJobManager = &parallelJobManagerLocal;



bool					cweeParallelJobManagerLocal::IsStalled() const {
	for (int i = 0; i < jobLists.Num(); i++) {
		if (jobLists[i]->IsStalled() == true) return true;
	}
	return false;
}

void					cweeParallelJobManagerLocal::Stall() {
	for (int i = 0; i < jobLists.Num(); i++)	jobLists[i]->Stall();
}

void					cweeParallelJobManagerLocal::UnStall() {
	for (int i = 0; i < jobLists.Num(); i++) {
		jobLists[i]->Wait();
		jobLists[i]->UnStall();
	}
}

int	cweeParallelJobManagerLocal::GetCumulativeJobsExecuted() const {
	return stats.cumulativeExecutedJobs;
}

int	cweeParallelJobManagerLocal::GetCumulativeProcessingTimeMicroSec() const {
	return stats.cumulativeProcessingTime;
}

void cweeParallelJobManagerLocal::Think() {
	for (int i = 0; i < jobLists.Num(); i++)
		if (jobLists[i]->TryWait()) jobLists[i]->Submit(0);// , 1);
}

void cweeParallelJobManagerLocal::UpdateStates() {
	unsigned int prevJobs = stats.prevExecutedJobs;
	stats.prevExecutedJobs = GetTotalJobsExecuted();
	stats.cumulativeExecutedJobs += cweeMath::Fmax(0, stats.prevExecutedJobs - prevJobs);

	unsigned int prevTime = stats.prevProcessingTime;
	stats.prevProcessingTime = GetTotalProcessingTimeMicroSec();
	stats.cumulativeProcessingTime += cweeMath::Fmax(0, stats.prevProcessingTime - prevTime);
}

void SubmitJobList(cweeParallelJobList_Threads* jobList, int parallelism) {
	parallelJobManagerLocal.Submit(jobList, parallelism);
}

void cweeParallelJobManagerLocal::Init() {
	// on select hardware, we can choose to this will have specific cores for specific threads, but on most PC's they will all be CORE_ANY
	core_t cores[] = JOB_THREAD_CORES;
	assert(sizeof(cores) / sizeof(cores[0]) >= MAX_JOB_THREADS);

	for (int i = 0; i < MAX_JOB_THREADS; i++) {
		threads[i].Start(cores[i], i);
	}
	maxThreads = (int)NUM_JOB_THREADS;
	Sys_CPUCount(numLogicalCpuCores, numPhysicalCpuCores, numCpuPackages);
}

void cweeParallelJobManagerLocal::Shutdown() {
	for (int i = 0; i < MAX_JOB_THREADS; i++) {
		threads[i].StopThread();
	}
}

cweeParallelJobList* cweeParallelJobManagerLocal::AllocJobList(jobListId_t id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs) {
	for (int i = 0; i < jobLists.Num(); i++) {
		if (jobLists[i]->GetId() == id) {
			return jobLists[i];
		}
	}
	cweeParallelJobList* jobList = new (TAG_JOBLIST) cweeParallelJobList(id, priority, maxJobs, maxSyncs);
	jobLists.Append(jobList);
	return jobList;
}

void cweeParallelJobManagerLocal::FreeJobList(cweeParallelJobList* jobList) {
	if (jobList == NULL) {
		return;
	}
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

int cweeParallelJobManagerLocal::GetNumStagedJobs() const {
	int num = 0;
	for (int i = 0; i < GetNumJobLists(); i++)
		num += jobLists[i]->GetNumStagedJobs();
	return num;
}

int cweeParallelJobManagerLocal::GetTotalProcessingTimeMicroSec() const {
	int num = 0;
	for (int i = 0; i < GetNumJobLists(); i++)
		num += jobLists[i]->GetTotalProcessingTimeMicroSec();
	return num;
}

int cweeParallelJobManagerLocal::GetTotalJobsExecuted() const {
	int num = 0;
	for (int i = 0; i < GetNumJobLists(); i++)
		num += jobLists[i]->GetNumExecutedJobs();
	return num;
}

int cweeParallelJobManagerLocal::GetNumJobLists() const {
	return jobLists.Num();
}

int cweeParallelJobManagerLocal::GetNumFreeJobLists() const {
	return MAX_JOBLISTS - jobLists.Num();
}

cweeParallelJobList* cweeParallelJobManagerLocal::GetJobList(int index) {
	return jobLists[index];
}

cweeParallelJobList* cweeParallelJobManagerLocal::GetJobList(jobListId_t id) {
	for (int i = 0; i < jobLists.Num(); i++) {
		if (jobLists[i]->GetId() == id) {
			return jobLists[i];
		}
	}
	return AllocJobList(id, JOBLIST_PRIORITY_MEDIUM, 2048, 0);
}

int cweeParallelJobManagerLocal::GetNumProcessingUnits() {
	return maxThreads;
}

void cweeParallelJobManagerLocal::WaitForAllJobLists() {
	// wait for all job lists to complete
	for (int i = 0; i < jobLists.Num(); i++) {
		jobLists[i]->Wait();
	}
}

void cweeParallelJobManagerLocal::Submit(cweeParallelJobList_Threads* jobList, int parallelism) {


	maxThreads = MAX_JOB_THREADS;

	// determine the number of threads to use
	int numThreads = maxThreads;
	if (parallelism == JOBLIST_PARALLELISM_DEFAULT) {
		numThreads = maxThreads;
	}
	else if (parallelism == JOBLIST_PARALLELISM_MAX_CORES) {
		numThreads = numLogicalCpuCores;
	}
	else if (parallelism == JOBLIST_PARALLELISM_MAX_THREADS) {
		numThreads = MAX_JOB_THREADS;
	}
	else if (parallelism > MAX_JOB_THREADS) {
		numThreads = MAX_JOB_THREADS;
	}
	else {
		numThreads = parallelism;
	}

	if (numThreads <= 0) {
		threadJobListState_t state(jobList->GetVersion());
		jobList->RunJobs(0, state, false);
		return;
	}

	for (int i = 0; i < numThreads; i++) {
		threads[i].AddParallelJobList(jobList);
		threads[i].SignalWork();
	}
}