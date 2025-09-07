#include "units.h"
#include <concurrent_unordered_map.h>

namespace GL {
	// boost::type_info hashes are unpredictable and therefore we must use a map.
	static concurrency::concurrent_unordered_map<uint16_t, value::si_unit> si_unit_types;
	// get the cached si unit for this type
	value::si_unit& value::get_si_unit(uint16_t hash) {
		return si_unit_types[hash];
	};

	//static GL::value meter{ GL::value::get_si_unit(1, 0, 0, 0, 0).get_impl_unit(1.0, "meter", "m") };
	//static GL::value meter{ GL::value::get_si_unit(1, 0, 0, 0, 0).get_impl_unit(1.0, "meter", "m") };



};