
#ifndef __CONCURRENT_JOBS_H__
#define __CONCURRENT_JOBS_H__

typedef void (*jobRun_t)(void*);

enum jobSyncType_t {
	SYNC_NONE,
	SYNC_SIGNAL,
	SYNC_SYNCHRONIZE
};

enum jobListId_t {
	JOBLIST_FRONTEND = 0,
	JOBLIST_BACKEND = 1,
	JOBLIST_PARALLEL = 2,
	JOBLIST_UTILITY = 9,
	MAX_JOBLISTS = 32							// max determined by target platform, but should try to not exceed number of "logical processors" on computer. 
};

enum jobListPriority_t {
	JOBLIST_PRIORITY_NONE,						// any order
	JOBLIST_PRIORITY_LOW,						// always last
	JOBLIST_PRIORITY_MEDIUM,					// FIFO
	JOBLIST_PRIORITY_HIGH						// do immediately, single-run type thing. Need data back immediately. 
};
enum jobListParallelism_t {
	JOBLIST_PARALLELISM_DEFAULT = -1,			// use "jobs_numThreads" number of threads
	JOBLIST_PARALLELISM_MAX_CORES = -2,			// use a thread for each logical core (includes hyperthreads)
	JOBLIST_PARALLELISM_MAX_THREADS = -3		// use the maximum number of job threads, which can help if there is IO to overlap
};

class cweeParallelJobList {
	friend class cweeParallelJobManagerLocal;
public:
	void					AddConcurrentJob(jobRun_t function, void* data);
	void					InsertSyncPoint(jobSyncType_t syncType);
	void					Submit(cweeParallelJobList* waitForJobList = NULL, int parallelism = JOBLIST_PARALLELISM_DEFAULT); 	// Submit the jobs in this list.
	void					PerformConcurrentJobs(cweeParallelJobList* waitForJobList = NULL, int parallelism = JOBLIST_PARALLELISM_DEFAULT);
	bool					IsStalled() const { return (mustWait.GetValue() > 0); };
	void					Stall() { if (!(IsStalled() == true)) mustWait.Increment(); };
	void					UnStall() { if ((IsStalled() == true)) mustWait.Decrement(); };
	void					Wait();												// Wait for the jobs in this list to finish. Will spin in place if any jobs are not done.
	bool					TryWait();											// Try to wait for the jobs in this list to finish but either way return immediately. Returns true if all jobs are done.
	bool					IsSubmitted() const;								// returns true if the job list has been submitted.
	unsigned int			GetNumExecutedJobs() const;							// Get the number of jobs executed in this job list.
	unsigned int			GetNumSyncs() const;								// Get the number of sync points.
	uint64					GetSubmitTimeMicroSec() const;						// Time at which the job list was submitted.
	uint64					GetStartTimeMicroSec() const;						// Time at which execution of this job list started.
	uint64					GetFinishTimeMicroSec() const;						// Time at which all jobs in the list were executed.
	uint64					GetWaitTimeMicroSec() const;						// Time the host thread waited for this job list to finish.
	uint64					GetTotalProcessingTimeMicroSec() const;				// Get the total time all units spent processing this job list.
	uint64					GetTotalWastedTimeMicroSec() const;					// Get the total time all units wasted while processing this job list.
	uint64					GetUnitProcessingTimeMicroSec(int unit) const;		// Time the given unit spent processing this job list.
	uint64					GetUnitWastedTimeMicroSec(int unit) const;			// Time the given unit wasted while processing this job list.
	jobListId_t				GetId() const;										// Get the job list ID
	int						GetNumStagedJobs() const { return jobBuffer.Num(); }
private:
	cweeThreadedList<std::tuple<jobRun_t, void*, jobSyncType_t>> jobBuffer;
	cweeSysInterlockedInteger mustWait;
	class cweeParallelJobList_Threads* jobListThreads;
	cweeParallelJobList(jobListId_t id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs);
	~cweeParallelJobList();
};

class cweeParallelJobManager {
public:

	virtual							~cweeParallelJobManager() {}

	// Utility functions for the ParallelJobManager
	virtual void					Think() = 0; // Run once a cycle to allow deffered work to be performed.

	virtual void					Init() = 0;
	virtual void					Shutdown() = 0;

	virtual bool					IsStalled() const = 0;
	virtual void					Stall() = 0;
	virtual void					UnStall() = 0;

	virtual cweeParallelJobList* AllocJobList(jobListId_t id, jobListPriority_t priority, unsigned int maxJobs, unsigned int maxSyncs) = 0;
	void							Submit(cweeParallelJobList_Threads* jobList, int parallelism);
	virtual void					WaitForAllJobLists() = 0;
	virtual void					FreeJobList(cweeParallelJobList* jobList) = 0;

	virtual int						GetCumulativeJobsExecuted() const = 0;
	virtual int						GetCumulativeProcessingTimeMicroSec() const = 0;
	virtual void					UpdateStates() = 0;
	virtual int						GetNumStagedJobs() const = 0;
	virtual int						GetTotalProcessingTimeMicroSec() const = 0;
	virtual int						GetTotalJobsExecuted() const = 0;

	virtual int						GetNumJobLists() const = 0;
	virtual int						GetNumFreeJobLists() const = 0;
	virtual cweeParallelJobList* GetJobList(int index) = 0;
	virtual cweeParallelJobList* GetJobList(jobListId_t id) = 0;

	virtual int						GetNumProcessingUnits() = 0;
	virtual int						GetNumPhysicalCpuCores() = 0;
	virtual int						GetNumLocalCpuCores() = 0;
	virtual int						GetNumCpuPackages() = 0;
};

extern cweeParallelJobManager* parallelJobManager;






/*
================================================
cweeParallelJobRegistration
================================================
*/

void RegisterJob(jobRun_t function, const char* name);

class cweeParallelJobRegistration {
public:
	cweeParallelJobRegistration(jobRun_t function, const char* name);
};

#define REGISTER_PARALLEL_JOB( function, name )		static cweeParallelJobRegistration register_##function( (jobRun_t) function, name )




#endif
