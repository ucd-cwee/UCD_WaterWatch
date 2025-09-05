#include "units.h"
#include <concurrent_unordered_map.h>

namespace GL {
	// boost::type_info hashes are unpredictable and therefore we must use a map.
	static concurrency::concurrent_unordered_map<uint16_t, value::si_unit> si_unit_types;
	// get the cached si unit for this type
	value::si_unit& value::get_si_unit(uint16_t hash) {
		return si_unit_types[hash];
	};
};