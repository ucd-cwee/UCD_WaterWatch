#include "units.h"
#include <concurrent_unordered_map.h>
#include "types.h"

#define CalculateMetricPrefixV(metric) ((long double)std::metric::num / (long double)std::metric::den)
namespace GL {
	// boost::type_info hashes are unpredictable and therefore we must use a map.

	// static GL::epoch_map< value::si_unit, uint16_t> si_unit_types;
	static concurrency::concurrent_unordered_map<uint16_t, value::si_unit> si_unit_types;
	
	// get the cached si unit for this type
	value::si_unit& value::get_si_unit(uint16_t hash) {
		return si_unit_types[hash];
	};
	const concurrency::concurrent_unordered_map<uint16_t, value::si_unit>& 
    //const GL::epoch_map< value::si_unit, uint16_t>&
		value::all_known_unit_types() {
		return si_unit_types;
	};

#ifdef DECL_UNIT_LITERALS
#define DerivedUnitType(type, category, abbreviation, Ratio) \
    static auto* type ## _pkg{ &GL::value::get_si_unit(value::Categories::##category##::unitType_m[0], value::Categories::##category##::unitType_m[1], value::Categories::##category##::unitType_m[2], value::Categories::##category##::unitType_m[3], value::Categories::##category##::unitType_m[4], value::Categories::##category##::unitType_m[5]).get_impl_unit(Ratio, #type, #abbreviation) }; \
    static bool type ## _added_to_base { GL::type_of< type >().add_base(GL::type_of<GL::value>()) && GL::type_of< type >().try_update_name( #type ) }; \
    value::package type ## ::unique_pkg() { return type ## _pkg->default_bits; };

#define DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) \
    DerivedUnitType(prefix ## type, category, prefix_abbrev ## abbreviation, ratio * CalculateMetricPrefixV(prefix))

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
	DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, pico, p); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, nano, n); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, micro, u); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, milli, m); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, centi, c); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deci, d); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deca, da); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, hecto, h); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, kilo, k); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, mega, M); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, giga, G); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, tera, T); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, peta, P)

	DerivedUnitList; // this loops through the definitions for DerivedUnitTypeWithMetricPrefixes() and DerivedUnitType() for all units. Change thosse macro definitions to change the implimentations. 

    static auto* kelvin_pkg{ &GL::value::get_si_unit(value::Categories::temperature::unitType_m[0], value::Categories::temperature::unitType_m[1], value::Categories::temperature::unitType_m[2], value::Categories::temperature::unitType_m[3], value::Categories::temperature::unitType_m[4], value::Categories::temperature::unitType_m[5]).get_impl_unit(1.0, -273.15, "kelvin", "degK") }; 
    static bool kelvin_added_to_base { GL::type_of< kelvin >().add_base(GL::type_of<GL::value>()) && GL::type_of< kelvin >().try_update_name("kelvin") };
    value::package kelvin::unique_pkg() { return kelvin_pkg->default_bits; };

	static auto* fahrenheit_pkg{ &GL::value::get_si_unit(value::Categories::temperature::unitType_m[0], value::Categories::temperature::unitType_m[1], value::Categories::temperature::unitType_m[2], value::Categories::temperature::unitType_m[3], value::Categories::temperature::unitType_m[4], value::Categories::temperature::unitType_m[5]).get_impl_unit(5.0 / 9.0, -32.0, "fahrenheit", "degF") };
	static bool fahrenheit_added_to_base{ GL::type_of< fahrenheit >().add_base(GL::type_of<GL::value>()) && GL::type_of< fahrenheit >().try_update_name("fahrenheit") };
	value::package fahrenheit::unique_pkg() { return fahrenheit_pkg->default_bits; };

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitType

	static float Sin(float a) {
		float s;
		if ((a < 0.0f) || (a >= (2.0f * 3.14159265358979323846f))) {
			a -= std::floor(a / (2.0f * 3.14159265358979323846f)) * (2.0f * 3.14159265358979323846f);
		}
		if (a < 3.14159265358979323846f) {
			if (a > (0.5f * 3.14159265358979323846f)) {
				a = 3.14159265358979323846f - a;
			}
		}
		else {
			if (a > 3.14159265358979323846f + (0.5f * 3.14159265358979323846f)) {
				a = a - (2.0f * 3.14159265358979323846f);
			}
			else {
				a = 3.14159265358979323846f - a;
			}
		}
		s = a * a;
		return a * (((((-2.39e-08f * s + 2.7526e-06f) * s - 1.98409e-04f) * s + 8.3333315e-03f) * s - 1.666666664e-01f) * s + 1.0f);
	};
	value value::sin_fast() const {
		static const auto radian_hash = radian_pkg->default_bits.m_bits2.unit_hash;
		static const auto degree_hash = degree_pkg->default_bits.m_bits2.unit_hash;
		static const float degree_ratio = static_cast<float>(degree_pkg->ratio);

		value::package copied = this->packed;
		if (copied.m_bits2.unit_hash == radian_hash) {
			return Sin(copied.m_bits.val);
		}
		else if (copied.m_bits2.unit_hash == degree_hash) {
			return Sin(copied.m_bits.val * degree_ratio);
		} 
		else {
			if (copied.m_bits.si_unit == 0) {
				throw std::runtime_error(GL::string("Provided type ('" + abbreviation(copied) + "') must be convertable to radians for trig functions, and may not be scalar due to ambiguity of the type").to_string());
			}
			return std::sin(GL::radian(GL::value(std::move(copied))).operator float());
		}
	};

#endif

};