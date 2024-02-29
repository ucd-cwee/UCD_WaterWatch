#pragma once

#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/cweeInterlocked.h"
#include "../WaterWatchCpp/cwee_math.h"
#include "../WaterWatchCpp/cweeTime.h"
#include "../WaterWatchCpp/Clock.h"
#include "../WaterWatchCpp/Iterator.h"

#include <assert.h>
#include <stdint.h>
#include <atomic>
#include <array>
#include <thread>
#include <ppl.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_set.h>

class FTL {
public:
	static uint64_t fnFiberTasks2b(int numTasks);
	static uint64_t fnFiberTasks2c(int numTasks);
	static uint64_t fnFiberTasks2d(int numTasks, int numSubTasks);
	static uint64_t fnFiberTasks3(int numTasks, int numSubTasks);
};
