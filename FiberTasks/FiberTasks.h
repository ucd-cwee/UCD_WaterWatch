#pragma once


class FTL {
public:
	static uint64_t fnFiberTasks2b(int numTasks);
	static uint64_t fnFiberTasks2c(int numTasks);
	static uint64_t fnFiberTasks2d(int numTasks, int numSubTasks);
	static uint64_t fnFiberTasks3(int numTasks, int numSubTasks);
};
