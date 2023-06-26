#ifndef	__OBJMANAGER_H__
#define __OBJMANAGER_H__

class managedMutex {
public:
	cweeSysMutex* data = nullptr;
	int which = -1;
};

static cweeThreadedList<cweeSysMutex*> cweeMutexManager_data;
static cweeThreadedList<int> cweeMutexManager_inUse;
static cweeSysMutex cweeMutexManager_lock;
static int cweeMutexManager_numMutexes = 0;

class cweeMutexManager {
public:
	static const int minMutexes = 10;
	static const int maxMutexes = 5000;
	
	static managedMutex get() {
		managedMutex out;

		cweeMutexManager_lock.Lock();
		for (int i = 0; i < cweeMutexManager_inUse.Num(); i++) {								
			if (cweeMutexManager_inUse[i] <= 0) {
				// this object exists and is not in use -- pass it along. 
				out.data = cweeMutexManager_data[i];
				out.which = i;
				cweeMutexManager_inUse[i] = 1;
				break;
			}
		}

		if (out.data == nullptr) {
			// no existing object was available .. resubmit an old one? make a new one?

			if (cweeMutexManager_inUse.Num() < minMutexes) {
				// create a new one
				{ 
					auto ptr = new cweeSysMutex;

					cweeMutexManager_data.Append(ptr);
					cweeMutexManager_inUse.Append(1);

					out.data = ptr;
					out.which = cweeMutexManager_data.Num() - 1;
				}
			}
			else if (cweeMutexManager_inUse.Num() > maxMutexes) {
				// reuse mutex with smallest load
				{
					int load = std::numeric_limits<int>::max();
					int minIndex = -1;
					for (int i = 0; i < cweeMutexManager_inUse.Num(); i++) {
						if (cweeMutexManager_inUse[i] < load) {
							load = cweeMutexManager_inUse[i];
							minIndex = i;
							if (load <= 1) break;
						}						
					}

					if (minIndex >= 0) {
						cweeMutexManager_inUse[minIndex]++;
						out.data = cweeMutexManager_data[minIndex];
						out.which = minIndex;
					}
					else {
						// something failed
						// create a new one
						{
							auto ptr = new cweeSysMutex;

							cweeMutexManager_data.Append(ptr);
							cweeMutexManager_inUse.Append(1);

							out.data = ptr;
							out.which = cweeMutexManager_data.Num() - 1;
						}
					}
				}
			}
			else {
				// optional to make a new one or submit an old one. Depends on load. 
				if (cweeMutexManager_inUse.Num() > cweeMutexManager_numMutexes) {
					// reuse mutex with smallest load
					{
						int load = std::numeric_limits<int>::max();
						int minIndex = -1;
						for (int i = 0; i < cweeMutexManager_inUse.Num(); i++) {
							if (cweeMutexManager_inUse[i] < load) {
								load = cweeMutexManager_inUse[i];
								minIndex = i;
								if (load <= 1) break;
							}
						}

						if (minIndex >= 0) {
							cweeMutexManager_inUse[minIndex]++;
							out.data = cweeMutexManager_data[minIndex];
							out.which = minIndex;
						}
						else {
							// something failed
							// create a new one
							{
								auto ptr = new cweeSysMutex;

								cweeMutexManager_data.Append(ptr);
								cweeMutexManager_inUse.Append(1);

								out.data = ptr;
								out.which = cweeMutexManager_data.Num() - 1;
							}
						}
					}

				}
				else if ((cweeMutexManager_numMutexes % cweeMutexManager_inUse.Num()) > 7){
					// create a new one
					{
						auto ptr = new cweeSysMutex;

						cweeMutexManager_data.Append(ptr);
						cweeMutexManager_inUse.Append(1);

						out.data = ptr;
						out.which = cweeMutexManager_data.Num() - 1;
					}
				}
				else {
					// reuse mutex with smallest load
					{
						int load = std::numeric_limits<int>::max();
						int minIndex = -1;
						for (int i = 0; i < cweeMutexManager_inUse.Num(); i++) {
							if (cweeMutexManager_inUse[i] < load) {
								load = cweeMutexManager_inUse[i];
								minIndex = i;
								if (load <= 1) break;
							}
						}

						if (minIndex >= 0) {
							cweeMutexManager_inUse[minIndex]++;
							out.data = cweeMutexManager_data[minIndex];
							out.which = minIndex;
						}
						else {
							// something failed
							// create a new one
							{
								auto ptr = new cweeSysMutex;

								cweeMutexManager_data.Append(ptr);
								cweeMutexManager_inUse.Append(1);

								out.data = ptr;
								out.which = cweeMutexManager_data.Num() - 1;
							}
						}
					}
				}
			}	
		}

		cweeMutexManager_numMutexes++;

		cweeMutexManager_lock.Unlock();
		
		if (!out.data) {
			throw("Failure to create mutex");
		}

		
		return out;
	};
	static void free(managedMutex& obj) {
		if (obj.data) {		
			cweeMutexManager_lock.Lock();
			cweeMutexManager_inUse[obj.which]--;
			cweeMutexManager_numMutexes--;
			cweeMutexManager_lock.Unlock();

			obj.data = nullptr; obj.which = -1;
		}
	};

	static int numActiveMutexes() {
		int out = 0;
		cweeMutexManager_lock.Lock();
		for (int i = 0; i < cweeMutexManager_inUse.Num(); i++) {
			if (cweeMutexManager_inUse[i]) {				
				out++;
			}
		}
		cweeMutexManager_lock.Unlock();
		return out;
	};
	static int numMutexes() {
		int out = 0;
		cweeMutexManager_lock.Lock();
		out = cweeMutexManager_inUse.Num();
		cweeMutexManager_lock.Unlock();
		return out;
	};

};

class cweeManagedMutex {
public:
	cweeManagedMutex() {
		lock = cweeMutexManager::get();
	};

	~cweeManagedMutex() {
		cweeMutexManager::free(lock);
	};

	bool TryLock() {
		return lock.data->Lock(false);
	}
	bool Lock(bool blocking = true) {
		return lock.data->Lock(blocking);
	}
	void Unlock() {
		return lock.data->Unlock();
	}

	mutable managedMutex lock;
};

#endif