#include "shared_ptr.h"

namespace GL {

	std::mutex* __atomic_mutexes() {
		static std::mutex atomic_mutexes[64];
		return &atomic_mutexes[0];
	};
	std::condition_variable* __atomic_conds() {
		static std::condition_variable atomic_conds[64];
		return &atomic_conds[0];
	};

};