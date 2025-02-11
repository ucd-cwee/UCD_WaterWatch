#include "Units_Instances.h"

// value definitions
namespace GoodLang {
	// Units
	namespace Units {

#define CalculateMetricPrefixV(metric) ((long double)std::metric::num / (long double)std::metric::den)
#define DerivedUnitType(type, category, abbreviation, Ratio) namespace Definitions { class type ## Definition final : public UnitDefinitionBase, public Categories::category { \
	public: \
		using unitDefBase = UnitDefinitionBase; \
		static constexpr auto Name_m{ impl::concat(#type) }; \
		static constexpr auto Abbreviation_m{ impl::concat(#abbreviation) }; \
		static constexpr double ratio_m{ Ratio }; \
		type ## Definition() : unitDefBase(0) {}; \
		type ## Definition(Number V) : unitDefBase(V* (Number)ratio_m) {}; \
        type ## Definition(type ## Definition const&) = default; \
		type ## Definition(type ## Definition&&) = default; \
		type ## Definition& operator=(type ## Definition const&) = default; \
		type ## Definition& operator=(type ## Definition&&) = default; \
		~type ## Definition() = default; \
		const std::array<double, UnitDefinitionBase::NumUnits>& unitType() const override { return this->unitType_m; }; \
		const double& ratio() const override { return ratio_m; }; \
		const char* BuiltInName() const override { return &Name_m.c[0]; }; \
		const char* BuiltInAbbreviation() const override { return &Abbreviation_m.c[0]; }; \
		std::unique_ptr<UnitDefinitionBase> Copy() const override { \
			return std::make_unique<typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > >>(*this); \
		}; \
	}; }; \
	type ::type () : value(std::make_unique<Definitions::type ## Definition>()) {}; \
	type ::type (value const& other) : type () { \
		auto V = impl::value_accessor::GetUnits(other).Shared(); \
		auto Data = this->unit_m.Unique(); \
		if (Data->IsSameCategory(*V)) Data->value() = V->value(); \
		else if (V->IsScalar()) Data->value() = (V->value() / V->ratio()) * Data->ratio(); \
		else if (Data->IsScalar()) this->unit_m.unsafe_set_ptr(V->Copy()); \
		else {  \
            std::string A1 = Data->BuiltInAbbreviation(); \
            std::string A2 = V->BuiltInAbbreviation(); \
			throw(std::runtime_error(GoodLang::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", \
				A1.c_str(), \
				A2.c_str() \
			))); \
		} \
	}; \
	type ::type (Number const& V) : type () { \
		auto get = this->unit_m.Unique(); \
		get->value() = V * get->ratio(); \
	};

#define DerivedUnitTypeWithMetricPrefix(type, prefix) \
		prefix ## type ::prefix ## type () : value(std::make_unique<Definitions::MetricPrefixedDefinition<std::prefix, Units::Definitions::type ## Definition>>()) {}; \
		prefix ## type ::prefix ## type (value const& other) : prefix ## type () { \
			auto V = impl::value_accessor::GetUnits(other).Shared(); \
			auto Data = this->unit_m.Unique(); \
			if (Data->IsSameCategory(*V)) Data->value() = V->value(); \
			else if (V->IsScalar()) Data->value() = (V->value() / V->ratio()) * Data->ratio(); \
			else if (Data->IsScalar()) this->unit_m.unsafe_set_ptr(V->Copy()); \
			else {  \
				std::string A1 = Data->BuiltInAbbreviation(); \
				std::string A2 = V->BuiltInAbbreviation(); \
				throw(std::runtime_error(GoodLang::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", \
					A1.c_str(), \
					A2.c_str() \
				))); \
			} \
		}; \
		prefix ## type ::prefix ## type (Number const& V) : prefix ## type () { \
			auto get = this->unit_m.Unique(); \
			get->value() = V * get->ratio(); \
		}; \

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
	DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, femto); \
	DerivedUnitTypeWithMetricPrefix(type, pico); \
	DerivedUnitTypeWithMetricPrefix(type, nano); \
	DerivedUnitTypeWithMetricPrefix(type, micro); \
	DerivedUnitTypeWithMetricPrefix(type, milli); \
	DerivedUnitTypeWithMetricPrefix(type, centi); \
	DerivedUnitTypeWithMetricPrefix(type, deci); \
	DerivedUnitTypeWithMetricPrefix(type, deca); \
	DerivedUnitTypeWithMetricPrefix(type, hecto); \
	DerivedUnitTypeWithMetricPrefix(type, kilo); \
	DerivedUnitTypeWithMetricPrefix(type, mega); \
	DerivedUnitTypeWithMetricPrefix(type, giga); \
	DerivedUnitTypeWithMetricPrefix(type, tera); \
	DerivedUnitTypeWithMetricPrefix(type, peta)

		DerivedUnitList; // this loops through the definitions for DerivedUnitTypeWithMetricPrefixes() and DerivedUnitType() for all units. Change thosse macro definitions to change the implimentations. 

#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitType

		// return absolute value
		Units::value math::fabs(const Units::value& V) {
			if (V < 0) return V * -1.0; else return V;
		};
		// return absolute value
		Units::value math::abs(const Units::value& V) {
			return fabs(V);
		};
		// clamp number to lower/upper bound
		Units::value math::clamp(const Units::value& V, const Units::value& min, const Units::value& max) {
			if (V < min) return min;
			if (V > max) return max;
			return V;
		};
		// round to lower whole number
		Units::value math::floor(const Units::value& f) {
			return f.floor();
		};
		// round to higher whole number
		Units::value math::ceiling(const Units::value& f) {
			return f.ceiling();
		};
		// round to nearest whole number
		Units::value math::round(const Units::value& a, float magnitude) {
			return floor((a / magnitude) + 0.5) * magnitude;
		};
		// return max(a, b);
		Units::value math::max(const Units::value& a, const Units::value& b) {
			return a > b ? a : b;
		};
		// return min(a, b);
		Units::value math::min(const Units::value& a, const Units::value& b) {
			return a < b ? a : b;
		};
		// if (b > a) a = b; // prevents copying when not necessary
		void math::max_ref(Units::value& a, const Units::value& b) {
			if (b > a) a = b;
		};
		// if (b < a) a = b; // prevents copying when not necessary
		void math::min_ref(Units::value& a, const Units::value& b) {
			if (b < a) a = b;
		};
		/* PI (unitless) */
		Units::scalar					constants::pi() {
			return 3.141592653589793238462643383279502884197169399375105820974944;
		};
		/* speed of light in a vacuum (m/s) */
		Units::meters_per_second		    constants::c() {
			return 299792458.0;
		};
		/* ( m^3 / (kg * s^2) ) */
		Units::value				        constants::G() {
			return Units::meter(6.67408e-11) * Units::meter(1) * Units::meter(1) / (Units::kilogram(1) * Units::second(1) * Units::second(1));
		};
		/* acceleration due to gravity ( m/s^2 ) */
		Units::meters_per_second_squared	constants::g() {
			return Units::meters_per_second_squared(9.8067);
		};
		/* density of water ( kg/m^3 ) */
		Units::kilograms_per_cubic_meter constants::d() {
			return Units::kilograms_per_cubic_meter(998.57);
		};

	};
};
