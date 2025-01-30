#pragma once
#include "ThreadSafeContainers.h"
#include "../WaterWatchCpp/enum.h"
#include <string_view>

#define DefineCategoryType(type, a, b, c, d, e) namespace categories { class type : public value { public: \
	type() noexcept = delete; \
	type(double V) noexcept = delete; \
	type(double V, const char* abbreviation) noexcept = delete; \
	type(double V, const char* abbreviation, double ratio) noexcept : value(UnitDefinition(a, b, c, d, e, false, abbreviation, ratio, V)) {}; \
    type(value const&) noexcept = delete; \
    type& operator=(value const&) noexcept = delete; \
    virtual ~type() {}; \
	static size_t UnitHash() { return GoodLang::Units::HashUnits(a,b,c,d,e); }; \
}; };
#define DerivedUnitType(type, category, abbreviation, ratio) class type final : public categories::category  { public: \
	static constexpr long double conversion { ratio }; \
	static constexpr std::string_view specialized_abbreviation { #abbreviation }; \
	static constexpr std::string_view specialized_name { #type }; \
	type() noexcept : categories::category(0.0, #abbreviation, ratio) {}; \
	type(double V) noexcept : categories::category(V, #abbreviation, ratio) {}; \
	type(value const& other) : categories::category(0.0, #abbreviation, ratio) { \
		this->unit_m.Update([V = other.unit_m.load(), this, &other](GoodLang::Units::UnitDefinition Data)->GoodLang::Units::UnitDefinition { \
			if (Data.IsSameCategory(V)) Data.value_m = V.value_m; \
			else if (V.IsScalar()) Data.value_m = (V.value_m / V.ratio_m) * ratio; \
			else if (Data.IsScalar()) Data = V; \
			else throw(std::runtime_error(GoodLang::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", this->Abbreviation().c_str(), other.Abbreviation().c_str()))); \
            return Data; \
		}); \
    }; \
    virtual bool IsStaticType() const override { return true; }; \
    virtual ~type() {}; \
};
#define CalculateMetricPrefixV(metric) ((long double)std::metric::num / (long double)std::metric::den)
#define DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) DerivedUnitType(prefix ## type, category, prefix_abbrev ## abbreviation, static_cast<long double>(ratio) * CalculateMetricPrefixV(prefix))
#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio)\
	DerivedUnitType(type, category, abbreviation, ratio);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, pico, p);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, nano, n);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, micro, u);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, milli, m);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, centi, c);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deci, d);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deca, da);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, hecto, h);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, kilo, k);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, mega, M);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, giga, G);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, tera, T);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, peta, P)
#define DerivedUnitStd(type, category, abbreviation, ratio) namespace std { template<> class numeric_limits<GoodLang::Units::type> { public: \
	static constexpr double min() { return std::numeric_limits<double>::min(); } \
	static constexpr double max() { return std::numeric_limits<double>::max(); } \
	static constexpr double lowest() { return std::numeric_limits<double>::lowest(); } \
	static constexpr bool is_integer = std::numeric_limits<double>::is_integer; \
	static constexpr bool is_signed = std::numeric_limits<double>::is_signed; }; \
}; /* Unit Litersl (e.g. 1_ft, 1_gpm, etc.) */ namespace literals { \
	__forceinline auto operator""_ ## abbreviation (long double d) { return GoodLang::Units::type(static_cast<double>(d)); } \
	__forceinline auto operator""_ ## abbreviation (unsigned long long d) { return GoodLang::Units::type(static_cast<double>(d)); }\
};

#define DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) DerivedUnitStd(prefix ## type, category, prefix_abbrev ## abbreviation, ratio * CalculateMetricPrefixV(prefix))
#define DerivedUnitStdWithMetricPrefixes(type, category, abbreviation, ratio)\
	DerivedUnitStd(type, category, abbreviation, ratio);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, pico, p);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, nano, n);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, micro, u);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, milli, m);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, centi, c);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, deci, d);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, deca, da);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, hecto, h);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, kilo, k);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, mega, M);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, giga, G);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, tera, T);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, peta, P)

namespace GoodLang {
	namespace Units {
		BETTER_ENUM(units_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);

		// container for all of the information that defines a unit value, including the unit data as well as the underlying SI value.
		class UnitDefinition {
		public:
			static constexpr size_t NumUnits{ Units::units_type::_size_constant };
			template <typename T> static constexpr T abs(T x) { return x > (T)0 ? x : -x; };

		public:
			std::array< DoubleWrapper, NumUnits> unitType_m; // power exponents for the SI units (e.g. m^1 * kg^0 * s^-1 * A^0 * $^0 = m/s)
			DoubleWrapper ratio_m; // ratio multiplier for converting from the SI units to this actual unit (e.g. 1 = meters, 0.304 = feet, etc.) 
			DoubleWrapper value_m; // underlying value of the unit if represented as SI units. (e.g. will always be in meters, regardless of the actual unit being in feet)

			bool IsSI() const;
			bool IsScalar() const;
			static const size_t sizeOfUnits{ sizeof(unitType_m) };
		public:
			UnitDefinition() :
				unitType_m{ 0.f, 0.f, 0.f, 0.f, 0.f },
				ratio_m{ 1. },
				value_m{ 0. }
			{};
			UnitDefinition(double V) :
				unitType_m{ 0.f, 0.f, 0.f, 0.f, 0.f },
				ratio_m{ 1. },
				value_m{ V }
			{};
			UnitDefinition(double a, double b, double c, double d, double e, bool isScalar_p, const char* abbreviation_p, double ratio_p, double value_p = 0.0) noexcept :
				unitType_m{ static_cast<float>(a), static_cast<float>(b), static_cast<float>(c), static_cast<float>(d), static_cast<float>(e) },
				ratio_m{ ratio_p },
				value_m{ value_p * ratio_p }
			{};
			UnitDefinition(std::array< DoubleWrapper, NumUnits> const& unitType_p, double ratio_p, double value_p) noexcept :
				unitType_m{ unitType_p },
				ratio_m{ ratio_p },
				value_m{ value_p }
			{};

			UnitDefinition(UnitDefinition const&) = default;
			UnitDefinition(UnitDefinition&&) = default;
			UnitDefinition& operator=(UnitDefinition const&) = default;
			UnitDefinition& operator=(UnitDefinition&&) = default;

		public:
			bool IsSameCategory(UnitDefinition const& other) const noexcept;
			bool IsSameUnit(UnitDefinition const& other) const noexcept;
			size_t HashCategory() const noexcept;
			std::pair<std::string_view, double> LookupAbbreviation(bool isStatic) const noexcept;
			std::string_view LookupTypeName() const noexcept;
			std::string CreateAbbreviation(bool isStatic) const noexcept;

		};

		static __forceinline size_t HashUnits(double a, double b, double c, double d, double e) noexcept {
			size_t out{ 37 };
			GoodLang::details::hash_combine(out, a, b, c, d, e);
			return out;
		};
		template <typename Derived> static constexpr __forceinline long double Conversion(long double X) { return Derived::conversion * X; };
		static constexpr __forceinline long double SQUARED(long double X) { return X * X; };
		static constexpr __forceinline long double CUBED(long double X) { return X * X * X; };
		template<typename T, typename U> static auto constexpr pow(T base, U exponent) {
			return exponent == 0 ? 1 : base * pow(base, exponent - 1);
		};

		class value {
		public:
			mutable CAS_Container<UnitDefinition> unit_m;

		public: // constructors
			value() : unit_m{ UnitDefinition{} } {};
			explicit value(UnitDefinition const& unit_p) : unit_m{ unit_p } {};
			explicit value(double V, UnitDefinition const& unit_p) :
				unit_m{ UnitDefinition((float)unit_p.unitType_m[0], (float)unit_p.unitType_m[1], (float)unit_p.unitType_m[2], (float)unit_p.unitType_m[3], (float)unit_p.unitType_m[4], false, ""/*unit_p.abbreviation_m*/, (double)unit_p.ratio_m, V) }
			{};
			explicit value(UnitDefinition&& unit_p) : unit_m{ std::forward<UnitDefinition>(unit_p) } {};
			value(value&& V) : unit_m{ std::move(V.unit_m) } {};
			value(value const& V) : unit_m{ V.unit_m.load() } {};
			value(double V) : unit_m{ UnitDefinition(0,0,0,0,0,true,"",1,V) } {};
			virtual ~value() = default;

		protected:
			virtual bool IsStaticType() const;

		public: // value operator
			explicit operator double() const noexcept;
			double operator()() const noexcept;

		public: // Functions
			std::string_view UnitName() const noexcept;
			void Clear();
			void Swap(value const& other) const;

		public:
			std::string Abbreviation(double* visibleValue = nullptr) const noexcept;
			std::string ToString() const;

		public: // Streaming functions (should be specialized per type)
			friend std::ostream& operator<<(std::ostream& os, value const& obj);
			friend std::stringstream& operator>>(std::stringstream& os, value& obj);

		public: // = Operators
			value& operator=(value const& other);

		public: // Comparison operators
			friend bool operator==(value const& A, value const& V) noexcept;
			friend bool operator<(value const& A, value const& V);
			friend bool operator<=(value const& A, value const& V);
			friend bool operator>(value const& A, value const& V);
			friend bool operator>=(value const& A, value const& V);
			friend bool operator!=(value const& A, value const& V) noexcept;

		public: // Unary operators
			value& operator++();
			value& operator--();
			value operator++(int);
			value operator--(int);

		public: // + and - Operators
			value operator-() const;

			friend value operator+(value const& A, value const& V);
			friend value operator-(value const& A, value const& V);
			value& operator+=(value const& V);
			value& operator-=(value const& V);

		public: // * and / Operators
			friend value operator*(value const& A, value const& V);
			friend value operator/(value const& A, value const& V);
			value& operator*=(value const& V);
			value& operator/=(value const& V);

		public: // pow and sqrt Operators
			// atomicly updates the value with a custom user-provided function.
			value& update(std::function<double(double)> const& updateFunction);
			// Creats a copy of the value and updates it with a custom user-provided function.
			value update(std::function<double(double)> const& updateFunction) const;
			// Returns a new value multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
			value pow(value const& V) const;
			// atomicly updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
			value& pow_value(value const& V);
			// pow(0.5)
			value sqrt() const;
			// atomicly floors (rounds to lower whole integer) the underlying value
			value& floor();
			// Creats a copy of the value and floors (rounds to lower whole integer) the underlying value
			value floor() const;
			// atomicly ceilings (rounds to upper whole integer) the underlying value
			value& ceiling();
			// Creats a copy of the value and ceilings (rounds to upper whole integer) the underlying value
			value ceiling() const;

		};
		using scalar = value;

		// Base classes
		DefineCategoryType(length, 1, 0, 0, 0, 0);
		DefineCategoryType(mass, 0, 1, 0, 0, 0);
		DefineCategoryType(time, 0, 0, 1, 0, 0);
		DefineCategoryType(current, 0, 0, 0, 1, 0);
		DefineCategoryType(dollar, 0, 0, 0, 0, 1);
		// Derived classes
		DefineCategoryType(frequency, 0, 0, -1, 0, 0);
		DefineCategoryType(velocity, 1, 0, -1, 0, 0);
		DefineCategoryType(acceleration, 1, 0, -2, 0, 0);
		DefineCategoryType(force, 1, 1, -2, 0, 0);
		DefineCategoryType(pressure, -1, 1, -2, 0, 0);
		DefineCategoryType(charge, 0, 0, 1, 1, 0);
		DefineCategoryType(power, 2, 1, -3, 0, 0);
		DefineCategoryType(energy, 2, 1, -2, 0, 0);
		DefineCategoryType(voltage, 2, 1, -3, -1, 0);
		DefineCategoryType(impedance, 2, 1, -3, -2, 0);
		DefineCategoryType(conductance, -2, -1, 3, 2, 0);
		DefineCategoryType(area, 2, 0, 0, 0, 0);
		DefineCategoryType(volume, 3, 0, 0, 0, 0);
		DefineCategoryType(fillrate, 0, 1, -1, 0, 0);
		DefineCategoryType(flowrate, 3, 0, -1, 0, 0);
		DefineCategoryType(density, -3, 1, 0, 0, 0);
		DefineCategoryType(energy_cost_rate, -2, -1, 2, 0, 1);
		DefineCategoryType(power_cost_rate, -2, -1, 3, 0, 1);
		DefineCategoryType(volume_cost_rate, -3, 0, 0, 0, 1);
		DefineCategoryType(energy_intensity, -1, 1, -2, 0, 1);
		DefineCategoryType(length_cost_rate, -1, 0, 0, 0, 1);
		DefineCategoryType(mass_cost_rate, 0, -1, 0, 0, 1);
		DefineCategoryType(emission_rate, -2, 0, 2, 0, 1);
		DefineCategoryType(time_rate, 0, 0, -1, 0, 1);

		/* LENGTH DERIVATIONS */
		DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0);
		DerivedUnitType(foot, length, ft, 0.3048); // 381.0 / 1250.0
		DerivedUnitType(inch, length, in, Conversion<foot>(1.0 / 12.0));
		DerivedUnitType(furlong, length, fur, Conversion<foot>(660));
		DerivedUnitType(mile, length, mi, Conversion<foot>(5280));
		DerivedUnitType(nauticalMile, length, nmi, Conversion<meter>(1852.0));
		DerivedUnitType(astronicalUnit, length, au, Conversion<meter>(149597870700.0));
		DerivedUnitType(yard, length, yd, Conversion<foot>(3.0));

		/* MASS DERIVATIONS */
		DerivedUnitTypeWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0);
		DerivedUnitType(metric_ton, mass, t, Conversion<kilogram>(1000.0));
		DerivedUnitType(pound, mass, lb, Conversion<kilogram>(45359237.0 / 100000000.0));
		DerivedUnitType(long_ton, mass, ln_t, Conversion < pound>(2240.0));
		DerivedUnitType(short_ton, mass, sh_t, Conversion < pound>(2000.0));
		DerivedUnitType(stone, mass, st, Conversion < pound>(14.0));
		DerivedUnitType(ounce, mass, oz, Conversion < pound>(1.0 / 16.0));
		DerivedUnitType(carat, mass, ct, Conversion < milligram>(200.0));
		DerivedUnitType(slug, mass, slug, Conversion<kilogram>(145939029.0 / 10000000.0));

		/* TIME DERIVATIONS */
		DerivedUnitTypeWithMetricPrefixes(second, time, s, 1.0);
		DerivedUnitType(minute, time, min, Conversion<second>(60.0));
		DerivedUnitType(hour, time, hr, Conversion<minute>(60.0));
		DerivedUnitType(day, time, d, Conversion<hour>(24.0));
		DerivedUnitType(week, time, wk, Conversion<day>(7.0));
		DerivedUnitType(year, time, yr, Conversion<day>(365));
		DerivedUnitType(month, time, mnth, Conversion<year>(1.0 / 12.0));
		DerivedUnitType(julian_year, time, a_j, Conversion<second>(31557600.0));
		DerivedUnitType(gregorian_year, time, a_g, Conversion<second>(31556952.0));

		/* CURRENT DERIVATIONS */
		DerivedUnitTypeWithMetricPrefixes(ampere, current, A, 1.0);

		/* DOLLAR DERIVATIONS */
		DerivedUnitType(Dollar, dollar, USD, 1.0);
		DerivedUnitType(MillionDollar, dollar, MUSD, Conversion<Dollar>(1000000.0));

		/* FREQUENCY DERIVATIONS */
		DerivedUnitTypeWithMetricPrefixes(hertz, frequency, Hz, 1.0);

		/* VELOCITY DERIVATIONS */
		DerivedUnitType(meters_per_second, velocity, mps, Conversion<meter>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(feet_per_second, velocity, fps, Conversion<foot>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(feet_per_minute, velocity, fpm, Conversion<foot>(1.0) / Conversion<minute>(1.0));
		DerivedUnitType(feet_per_hour, velocity, fph, Conversion<foot>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(miles_per_hour, velocity, mph, Conversion<mile>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(kilometers_per_hour, velocity, kph, Conversion<kilometer>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(knot, velocity, kts, Conversion<nauticalMile>(1.0) / Conversion<hour>(1.0));

		/* ACCELERATION DERIVATIONS */
		DerivedUnitType(meters_per_second_squared, acceleration, mps_sq, Conversion<meter>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
		DerivedUnitType(feet_per_second_squared, acceleration, fps_sq, Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
		DerivedUnitType(standard_gravity, acceleration, SG, Conversion<meters_per_second_squared>(980665.0 / 100000.0));

		// FORCE DERIVATIONS
		DerivedUnitTypeWithMetricPrefixes(newton, force, N, Conversion<kilogram>(1.0)* Conversion<meters_per_second_squared>(1.0));
		DerivedUnitTypeWithMetricPrefixes(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0));
		DerivedUnitType(dyne, force, dyn, Conversion <newton>(1.0 / 100000.0));
		DerivedUnitType(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0));
		DerivedUnitType(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));

		// PRESSURE DERIVATIONS
		DerivedUnitTypeWithMetricPrefixes(pascals, pressure, Pa, 1.0);
		DerivedUnitTypeWithMetricPrefixes(bar, pressure, bar, Conversion<kilopascals>(100.0));
		DerivedUnitType(atmosphere, pressure, atm, Conversion<pascals>(101325.0));
		DerivedUnitType(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0)));
		DerivedUnitType(head, pressure, ft_water, Conversion<pound_f>(62.43) / (Conversion<foot>(1.0) * Conversion<foot>(1.0)));
		DerivedUnitType(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0));

		// CHARGE DERIVATIONS
		DerivedUnitType(coulomb, charge, C, 1.0); /* WithMetricPrefixes */
		DerivedUnitTypeWithMetricPrefixes(ampere_hour, charge, Ah, Conversion< ampere>(1.0)* Conversion<hour>(1.0));

		// POWER DERIVATIONS
		DerivedUnitTypeWithMetricPrefixes(watt, power, W, 1.0);
		DerivedUnitType(horsepower, power, hp, Conversion<watt>(7457.0 / 10.0));

		// ENERGY DERIVATIONS
		DerivedUnitTypeWithMetricPrefixes(joule, energy, J, 1.0);
		DerivedUnitTypeWithMetricPrefixes(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0));
		DerivedUnitTypeWithMetricPrefixes(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0));
		DerivedUnitTypeWithMetricPrefixes(watt_hour, energy, Wh, Conversion<watt>(1.0)* Conversion<hour>(1.0));
		DerivedUnitType(watt_day, energy, Wd, Conversion<watt>(1.0)* Conversion<day>(1.0));
		DerivedUnitType(british_thermal_unit, energy, BTU, Conversion<joule>(105505585262.0 / 100000000.0));
		DerivedUnitType(british_thermal_unit_iso, energy, BTU_iso, Conversion<joule>(1055056.0 / 1000.0));
		DerivedUnitType(british_thermal_unit_59, energy, BTU59, Conversion<joule>(1054804.0 / 1000.0));
		DerivedUnitType(therm, energy, thm, Conversion<british_thermal_unit_59>(100000.0));
		DerivedUnitType(foot_pound, energy, ftlbf, Conversion<joule>(13558179483314004.0 / 10000000000000000.0));

		// VOLTAGE DERIVATIONS
		DerivedUnitTypeWithMetricPrefixes(volt, voltage, V, 1.0);

		// IMPEDANCE DERIVATIONS
		DerivedUnitTypeWithMetricPrefixes(ohm, impedance, Ohm, 1.0);

		// CONDUCTANCE DERIVATIONS
		DerivedUnitType(siemens, conductance, S, 1.0); // WithMetricPrefixes

		// AREA DERIVATIONS
		DerivedUnitType(square_meter, area, sq_m, 1.0);
		DerivedUnitType(square_foot, area, sq_ft, Conversion<foot>(1.0)* Conversion<foot>(1.0));
		DerivedUnitType(square_inch, area, sq_in, Conversion<inch>(1.0)* Conversion<inch>(1.0));
		DerivedUnitType(square_mile, area, sq_mi, Conversion<mile>(1.0)* Conversion<mile>(1.0));
		DerivedUnitType(square_kilometer, area, sq_km, Conversion<kilometer>(1.0)* Conversion<kilometer>(1.0));
		DerivedUnitType(hectare, area, ha, Conversion<square_meter>(1000.0));
		DerivedUnitType(acre, area, acre, Conversion<square_foot>(43560.0));

		// VOLUME DERIVATIONS
		DerivedUnitType(cubic_meter, volume, cu_m, 1.0);
		DerivedUnitType(cubic_millimeter, volume, cu_mm, CUBED(Conversion<millimeter>(1.0)));
		DerivedUnitType(cubic_kilometer, volume, cu_km, CUBED(Conversion<kilometer>(1.0)));
		DerivedUnitTypeWithMetricPrefixes(liter, volume, L, CUBED(Conversion<decimeter>(1.0)));
		DerivedUnitType(cubic_inch, volume, cu_in, CUBED(Conversion<inch>(1.0)));
		DerivedUnitType(cubic_foot, volume, cu_ft, CUBED(Conversion<foot>(1.0)));
		DerivedUnitType(cubic_yard, volume, cu_yd, CUBED(Conversion<yard>(1.0)));
		DerivedUnitType(cubic_mile, volume, cu_mi, CUBED(Conversion<mile>(1.0)));
		DerivedUnitTypeWithMetricPrefixes(gallon, volume, gal, Conversion<cubic_inch>(231.0));
		DerivedUnitType(imperial_gallon, volume, igal, Conversion<gallon>(10.0 / 12.0));
		DerivedUnitType(million_gallon, volume, MG, Conversion<gallon>(1.0)* CalculateMetricPrefixV(mega));
		DerivedUnitType(imperial_million_gallon, volume, IMG, Conversion<imperial_gallon>(1.0)* CalculateMetricPrefixV(mega));
		DerivedUnitType(acre_foot, volume, ac_ft, Conversion<acre>(1.0)* Conversion<foot>(1.0));
		DerivedUnitType(quart, volume, qt, Conversion<gallon>(0.25));
		DerivedUnitType(pint, volume, pt, Conversion<quart>(0.5));
		DerivedUnitType(cup, volume, c, Conversion<pint>(0.5));
		DerivedUnitType(fluid_ounce, volume, fl_oz, Conversion<cup>(0.125));
		DerivedUnitType(barrel, volume, bl, Conversion<gallon>(42.0));
		DerivedUnitType(bushel, volume, bu, Conversion<cubic_inch>(215042.0 / 100.0));
		DerivedUnitType(cord, volume, cord, Conversion<cubic_foot>(128.0));
		DerivedUnitType(tablespoon, volume, tbsp, Conversion<fluid_ounce>(0.5));
		DerivedUnitType(teaspoon, volume, tsp, Conversion<fluid_ounce>(1.0 / 6.0));
		DerivedUnitType(pinch, volume, pinch, Conversion<teaspoon>(1.0 / 8.0));
		DerivedUnitType(dash, volume, dash, Conversion<pinch>(1.0 / 2.0));
		DerivedUnitType(drop, volume, drop, Conversion<fluid_ounce>(1.0 / 360.0));
		DerivedUnitType(fifth, volume, fifth, Conversion<gallon>(0.2));
		DerivedUnitType(dram, volume, dr, Conversion<fluid_ounce>(0.125));
		DerivedUnitType(gill, volume, gi, Conversion<fluid_ounce>(4.0));
		DerivedUnitType(peck, volume, pk, Conversion<bushel>(0.25));
		DerivedUnitType(sack, volume, sacks, Conversion<bushel>(3.0));
		DerivedUnitType(shot, volume, shots, Conversion<fluid_ounce>(3.0 / 2.0));
		DerivedUnitType(strike, volume, strikes, Conversion<bushel>(2.0));

		// FILLRATE DERIVATIONS
		DerivedUnitTypeWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0);
		DerivedUnitType(metric_ton_per_second, fillrate, mTs, Conversion<metric_ton>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(metric_ton_per_minute, fillrate, mTm, Conversion<metric_ton>(1.0) / Conversion<minute>(1.0));
		DerivedUnitType(metric_ton_per_hour, fillrate, mTh, Conversion<metric_ton>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(metric_ton_per_day, fillrate, mTd, Conversion<metric_ton>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(metric_ton_per_year, fillrate, mTy, Conversion<metric_ton>(1.0) / Conversion<year>(1.0));

		// FLOWRATE DERIVATIONS
		DerivedUnitType(cubic_meter_per_second, flowrate, cms, 1.0);
		DerivedUnitType(cubic_meter_per_hour, flowrate, cmh, Conversion<cubic_meter>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(cubic_meter_per_day, flowrate, cmd, Conversion<cubic_meter>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(cubic_millimeter_per_second, flowrate, cmms, Conversion<cubic_millimeter>(1.0) / Conversion<second>(1.0));
		DerivedUnitTypeWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion<liter>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(liter_per_minute, flowrate, lpm, Conversion<liter>(1.0) / Conversion<minute>(1.0));
		DerivedUnitType(liter_per_day, flowrate, lpd, Conversion<liter>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(megaliter_per_day, flowrate, Mlpd, Conversion<megaliter>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(cubic_inch_per_second, flowrate, cis, Conversion<cubic_inch>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(cubic_inch_per_hour, flowrate, cih, Conversion<cubic_inch>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(cubic_foot_per_second, flowrate, cfs, Conversion<cubic_foot>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(cubic_foot_per_hour, flowrate, cfh, Conversion<cubic_foot>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(gallon_per_second, flowrate, gps, Conversion<gallon>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(gallon_per_minute, flowrate, gpm, Conversion<gallon>(1.0) / Conversion<minute>(1.0));
		DerivedUnitType(gallon_per_hour, flowrate, gph, Conversion<gallon>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(gallon_per_day, flowrate, gpd, Conversion<gallon>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(gallon_per_year, flowrate, gpy, Conversion<gallon>(1.0) / Conversion<year>(1.0));
		DerivedUnitType(million_gallon_per_second, flowrate, MGS, Conversion<megagallon>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(million_gallon_per_minute, flowrate, MGM, Conversion<megagallon>(1.0) / Conversion<minute>(1.0));
		DerivedUnitType(million_gallon_per_hour, flowrate, MGH, Conversion<megagallon>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(million_gallon_per_day, flowrate, MGD, Conversion<megagallon>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(million_gallon_per_year, flowrate, MGY, Conversion<megagallon>(1.0) / Conversion<year>(1.0));
		DerivedUnitType(imperial_million_gallon_per_second, flowrate, IMGS, Conversion<imperial_million_gallon>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion<imperial_million_gallon>(1.0) / Conversion<minute>(1.0));
		DerivedUnitType(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion<imperial_million_gallon>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(imperial_million_gallon_per_day, flowrate, IMGD, Conversion<imperial_million_gallon>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(imperial_million_gallon_per_year, flowrate, IMGY, Conversion<imperial_million_gallon>(1.0) / Conversion<year>(1.0));
		DerivedUnitType(acre_foot_per_second, flowrate, ac_ft_s, Conversion<acre_foot>(1.0) / Conversion<second>(1.0));
		DerivedUnitType(acre_foot_per_minute, flowrate, ac_ft_m, Conversion<acre_foot>(1.0) / Conversion<minute>(1.0));
		DerivedUnitType(acre_foot_per_hour, flowrate, ac_ft_h, Conversion<acre_foot>(1.0) / Conversion<hour>(1.0));
		DerivedUnitType(acre_foot_per_day, flowrate, ac_ft_d, Conversion<acre_foot>(1.0) / Conversion<day>(1.0));
		DerivedUnitType(acre_foot_per_year, flowrate, ac_ft_y, Conversion<acre_foot>(1.0) / Conversion<year>(1.0));

		// DENSITY DERIVATIONS
		DerivedUnitType(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0);
		DerivedUnitType(grams_per_milliliter, density, g_per_mL, Conversion<gram>(1.0) / Conversion<milliliter>(1.0));
		DerivedUnitType(kilograms_per_liter, density, kg_per_L, Conversion<kilogram>(1.0) / Conversion<liter>(1.0));
		DerivedUnitType(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion<ounce>(1.0) / Conversion<cubic_foot>(1.0));
		DerivedUnitType(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion<ounce>(1.0) / Conversion<cubic_inch>(1.0));
		DerivedUnitType(ounces_per_gallon, density, oz_per_gal, Conversion<ounce>(1.0) / Conversion<gallon>(1.0));
		DerivedUnitType(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion<pound>(1.0) / Conversion<cubic_foot>(1.0));
		DerivedUnitType(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion<pound>(1.0) / Conversion<cubic_inch>(1.0));
		DerivedUnitType(pounds_per_gallon, density, lb_per_gal, Conversion<pound>(1.0) / Conversion<gallon>(1.0));
		DerivedUnitType(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion<slug>(1.0) / Conversion<cubic_foot>(1.0));

		// DOLLAR RATES DERIVATIONS
		DerivedUnitType(Dollar_per_joule, energy_cost_rate, USD_per_j, Conversion<Dollar>(1.0) / Conversion<joule>(1.0));
		DerivedUnitType(Dollar_per_kilowatt_hour, energy_cost_rate, USD_per_kWh, Conversion<Dollar>(1.0) / Conversion<kilowatt_hour>(1.0));
		DerivedUnitType(Dollar_per_watt, power_cost_rate, USD_per_w, Conversion<Dollar>(1.0) / Conversion<watt>(1.0));
		DerivedUnitType(Dollar_per_kilowatt, power_cost_rate, USD_per_kW, Conversion<Dollar>(1.0) / Conversion<kilowatt>(1.0));
		DerivedUnitType(Dollar_per_cubic_meter, volume_cost_rate, USD_per_cm, Conversion<Dollar>(1.0) / Conversion<cubic_meter>(1.0));
		DerivedUnitType(Dollar_per_gallon, volume_cost_rate, USD_per_gal, Conversion<Dollar>(1.0) / Conversion<gallon>(1.0));

		// Rates
		DerivedUnitType(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion<kilowatt_hour>(1.0) / Conversion<acre_foot>(1.0));
		DerivedUnitType(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion<Dollar>(1.0) / Conversion<mile>(1.0));
		DerivedUnitType(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion<Dollar>(1.0) / Conversion<metric_ton>(1.0));
		DerivedUnitType(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion<metric_ton>(1.0) / Conversion<kilowatt_hour>(1.0));

		static std::vector<std::vector<std::tuple<std::string, std::string, Units::value, std::weak_ptr<GoodLang::Type_Info>>>> GetValueTypes() noexcept;
	
		namespace math {
			static Units::value fabs(const Units::value& V);
			static Units::value abs(const Units::value& V);
			static Units::value clamp(const Units::value& V, const Units::value& min, const Units::value& max);
			static Units::value floor(const Units::value& f);
			static Units::value ceiling(const Units::value& f);
			static Units::value round(const Units::value& a, float magnitude);
			static Units::value max(const Units::value& a, const Units::value& b);
			static Units::value min(const Units::value& a, const Units::value& b);
			static void max_ref(Units::value* a, const Units::value& b);
			static void min_ref(Units::value* a, const Units::value& b);
		};

		namespace constants {
			/* PI (unitless) */
			static Units::scalar					pi();

			/* speed of light in a vacuum (m/s) */
			static Units::meters_per_second		    c();

			/* ( m^3 / (kg * s^2) ) */
			static Units::value				        G();

			/* acceleration due to gravity ( m/s^2 ) */
			static Units::value				        g();

			/* density of water ( kg/m^3 ) */
			static Units::value                     d();
		};
	};
};

/* Unit Literals (e.g. 1_ft, 10.0_gpm, 0.01_cfs, etc.) */
DerivedUnitStdWithMetricPrefixes(meter, length, m, 1.0);
DerivedUnitStd(foot, length, ft, 381.0 / 1250.0);
DerivedUnitStd(inch, length, in, Conversion<foot>(1.0 / 12.0));
DerivedUnitStd(mile, length, mi, Conversion<foot>(5280.0 / 1.0));
DerivedUnitStd(nauticalMile, length, nmi, Conversion<meter>(1852.0));
DerivedUnitStd(astronicalUnit, length, au, Conversion<meter>(149597870700.0));
DerivedUnitStd(yard, length, yd, Conversion<foot>(3.0));

/* MASS DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0);
DerivedUnitStd(metric_ton, mass, t, Conversion<kilogram>(1000.0));
DerivedUnitStd(pound, mass, lb, Conversion<kilogram>(45359237.0 / 100000000.0));
DerivedUnitStd(long_ton, mass, ln_t, Conversion < pound>(2240.0));
DerivedUnitStd(short_ton, mass, sh_t, Conversion < pound>(2000.0));
DerivedUnitStd(stone, mass, st, Conversion < pound>(14.0));
DerivedUnitStd(ounce, mass, oz, Conversion < pound>(1.0 / 16.0));
DerivedUnitStd(carat, mass, ct, Conversion < milligram>(200.0));
DerivedUnitStd(slug, mass, slug, Conversion<kilogram>(145939029.0 / 10000000.0));

/* TIME DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(second, time, s, 1.0);
DerivedUnitStd(minute, time, min, Conversion<second>(60.0));
DerivedUnitStd(hour, time, hr, Conversion<minute>(60.0));
DerivedUnitStd(day, time, d, Conversion<hour>(24.0));
DerivedUnitStd(week, time, wk, Conversion<day>(7.0));
DerivedUnitStd(year, time, yr, Conversion<day>(365));
DerivedUnitStd(month, time, mnth, Conversion<year>(1.0 / 12.0));
DerivedUnitStd(julian_year, time, a_j, Conversion<second>(31557600.0));
DerivedUnitStd(gregorian_year, time, a_g, Conversion<second>(31556952.0));

/* CURRENT DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(ampere, current, A, 1.0);

/* DOLLAR DERIVATIONS */
DerivedUnitStd(Dollar, dollar, USD, 1.0);
DerivedUnitStd(MillionDollar, dollar, MUSD, Conversion<Dollar>(1000000.0));

/* FREQUENCY DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(hertz, frequency, Hz, 1.0);

/* VELOCITY DERIVATIONS */
DerivedUnitStd(meters_per_second, velocity, mps, Conversion<meter>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(feet_per_second, velocity, fps, Conversion<foot>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(feet_per_minute, velocity, fpm, Conversion<foot>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(feet_per_hour, velocity, fph, Conversion<foot>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(miles_per_hour, velocity, mph, Conversion<mile>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(kilometers_per_hour, velocity, kph, Conversion<kilometer>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(knot, velocity, kts, Conversion<nauticalMile>(1.0) / Conversion<hour>(1.0));

/* ACCELERATION DERIVATIONS */
DerivedUnitStd(meters_per_second_squared, acceleration, mps_sq, Conversion<meter>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
DerivedUnitStd(feet_per_second_squared, acceleration, fps_sq, Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
DerivedUnitStd(standard_gravity, acceleration, SG, Conversion<meters_per_second_squared>(980665.0 / 100000.0));

// FORCE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(newton, force, N, Conversion<kilogram>(1.0)* Conversion<meters_per_second_squared>(1.0));
DerivedUnitStdWithMetricPrefixes(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0));
DerivedUnitStd(dyne, force, dyn, Conversion <newton>(1.0 / 100000.0));
DerivedUnitStd(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0));
DerivedUnitStd(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));

// PRESSURE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(pascals, pressure, Pa, 1.0);
DerivedUnitStdWithMetricPrefixes(bar, pressure, bar, Conversion<kilopascals>(100.0));
DerivedUnitStd(atmosphere, pressure, atm, Conversion<pascals>(101325.0));
DerivedUnitStd(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0)));
DerivedUnitStd(head, pressure, ft_water, Conversion<pound_f>(62.43) / (Conversion<foot>(1.0) * Conversion<foot>(1.0)));
DerivedUnitStd(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0));

// CHARGE DERIVATIONS
DerivedUnitStd(coulomb, charge, C, 1.0); /* WithMetricPrefixes */
DerivedUnitStdWithMetricPrefixes(ampere_hour, charge, Ah, Conversion< ampere>(1.0)* Conversion<hour>(1.0));

// POWER DERIVATIONS
DerivedUnitStdWithMetricPrefixes(watt, power, W, 1.0);
DerivedUnitStd(horsepower, power, hp, Conversion<watt>(7457.0 / 10.0));

// ENERGY DERIVATIONS
DerivedUnitStdWithMetricPrefixes(joule, energy, J, 1.0);
DerivedUnitStdWithMetricPrefixes(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0));
DerivedUnitStdWithMetricPrefixes(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0));
DerivedUnitStdWithMetricPrefixes(watt_hour, energy, Wh, Conversion<watt>(1.0)* Conversion<hour>(1.0));
DerivedUnitStd(watt_day, energy, Wd, Conversion<watt>(1.0)* Conversion<day>(1.0));
DerivedUnitStd(british_thermal_unit, energy, BTU, Conversion<joule>(105505585262.0 / 100000000.0));
DerivedUnitStd(british_thermal_unit_iso, energy, BTU_iso, Conversion<joule>(1055056.0 / 1000.0));
DerivedUnitStd(british_thermal_unit_59, energy, BTU59, Conversion<joule>(1054804.0 / 1000.0));
DerivedUnitStd(therm, energy, thm, Conversion<british_thermal_unit_59>(100000.0));
DerivedUnitStd(foot_pound, energy, ftlbf, Conversion<joule>(13558179483314004.0 / 10000000000000000.0));

// VOLTAGE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(volt, voltage, V, 1.0);

// IMPEDANCE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(ohm, impedance, Ohm, 1.0);

// CONDUCTANCE DERIVATIONS
DerivedUnitStd(siemens, conductance, S, 1.0); // WithMetricPrefixes

// AREA DERIVATIONS
DerivedUnitStd(square_meter, area, sq_m, 1.0);
DerivedUnitStd(square_foot, area, sq_ft, Conversion<foot>(1.0)* Conversion<foot>(1.0));
DerivedUnitStd(square_inch, area, sq_in, Conversion<inch>(1.0)* Conversion<inch>(1.0));
DerivedUnitStd(square_mile, area, sq_mi, Conversion<mile>(1.0)* Conversion<mile>(1.0));
DerivedUnitStd(square_kilometer, area, sq_km, Conversion<kilometer>(1.0)* Conversion<kilometer>(1.0));
DerivedUnitStd(hectare, area, ha, Conversion<square_meter>(1000.0));
DerivedUnitStd(acre, area, acre, Conversion<square_foot>(43560.0));

// VOLUME DERIVATIONS
DerivedUnitStd(cubic_meter, volume, cu_m, 1.0);
DerivedUnitStd(cubic_millimeter, volume, cu_mm, CUBED(Conversion<millimeter>(1.0)));
DerivedUnitStd(cubic_kilometer, volume, cu_km, CUBED(Conversion<kilometer>(1.0)));
DerivedUnitStdWithMetricPrefixes(liter, volume, L, CUBED(Conversion<decimeter>(1.0)));
DerivedUnitStd(cubic_inch, volume, cu_in, CUBED(Conversion<inch>(1.0)));
DerivedUnitStd(cubic_foot, volume, cu_ft, CUBED(Conversion<foot>(1.0)));
DerivedUnitStd(cubic_yard, volume, cu_yd, CUBED(Conversion<yard>(1.0)));
DerivedUnitStd(cubic_mile, volume, cu_mi, CUBED(Conversion<mile>(1.0)));
DerivedUnitStdWithMetricPrefixes(gallon, volume, gal, Conversion<cubic_inch>(231.0));
DerivedUnitStd(imperial_gallon, volume, igal, Conversion<gallon>(10.0 / 12.0));
DerivedUnitStd(million_gallon, volume, MG, Conversion<gallon>(1.0)* CalculateMetricPrefixV(mega));
DerivedUnitStd(imperial_million_gallon, volume, IMG, Conversion<imperial_gallon>(1.0)* CalculateMetricPrefixV(mega));
DerivedUnitStd(acre_foot, volume, ac_ft, Conversion<acre>(1.0)* Conversion<foot>(1.0));
DerivedUnitStd(quart, volume, qt, Conversion<gallon>(0.25));
DerivedUnitStd(pint, volume, pt, Conversion<quart>(0.5));
DerivedUnitStd(cup, volume, c, Conversion<pint>(0.5));
DerivedUnitStd(fluid_ounce, volume, fl_oz, Conversion<cup>(0.125));
DerivedUnitStd(barrel, volume, bl, Conversion<gallon>(42.0));
DerivedUnitStd(bushel, volume, bu, Conversion<cubic_inch>(215042.0 / 100.0));
DerivedUnitStd(cord, volume, cord, Conversion<cubic_foot>(128.0));
DerivedUnitStd(tablespoon, volume, tbsp, Conversion<fluid_ounce>(0.5));
DerivedUnitStd(teaspoon, volume, tsp, Conversion<fluid_ounce>(1.0 / 6.0));
DerivedUnitStd(pinch, volume, pinch, Conversion<teaspoon>(1.0 / 8.0));
DerivedUnitStd(dash, volume, dash, Conversion<pinch>(1.0 / 2.0));
DerivedUnitStd(drop, volume, drop, Conversion<fluid_ounce>(1.0 / 360.0));
DerivedUnitStd(fifth, volume, fifth, Conversion<gallon>(0.2));
DerivedUnitStd(dram, volume, dr, Conversion<fluid_ounce>(0.125));
DerivedUnitStd(gill, volume, gi, Conversion<fluid_ounce>(4.0));
DerivedUnitStd(peck, volume, pk, Conversion<bushel>(0.25));
DerivedUnitStd(sack, volume, sacks, Conversion<bushel>(3.0));
DerivedUnitStd(shot, volume, shots, Conversion<fluid_ounce>(3.0 / 2.0));
DerivedUnitStd(strike, volume, strikes, Conversion<bushel>(2.0));

// FILLRATE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0);
DerivedUnitStd(metric_ton_per_second, fillrate, mTs, Conversion<metric_ton>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(metric_ton_per_minute, fillrate, mTm, Conversion<metric_ton>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(metric_ton_per_hour, fillrate, mTh, Conversion<metric_ton>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(metric_ton_per_day, fillrate, mTd, Conversion<metric_ton>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(metric_ton_per_year, fillrate, mTy, Conversion<metric_ton>(1.0) / Conversion<year>(1.0));

// FLOWRATE DERIVATIONS
DerivedUnitStd(cubic_meter_per_second, flowrate, cms, 1.0);
DerivedUnitStd(cubic_meter_per_hour, flowrate, cmh, Conversion<cubic_meter>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(cubic_meter_per_day, flowrate, cmd, Conversion<cubic_meter>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(cubic_millimeter_per_second, flowrate, cmms, Conversion<cubic_millimeter>(1.0) / Conversion<second>(1.0));
DerivedUnitStdWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion<liter>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(liter_per_minute, flowrate, lpm, Conversion<liter>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(liter_per_day, flowrate, lpd, Conversion<liter>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(megaliter_per_day, flowrate, Mlpd, Conversion<megaliter>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(cubic_inch_per_second, flowrate, cis, Conversion<cubic_inch>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(cubic_inch_per_hour, flowrate, cih, Conversion<cubic_inch>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(cubic_foot_per_second, flowrate, cfs, Conversion<cubic_foot>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(cubic_foot_per_hour, flowrate, cfh, Conversion<cubic_foot>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(gallon_per_second, flowrate, gps, Conversion<gallon>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(gallon_per_minute, flowrate, gpm, Conversion<gallon>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(gallon_per_hour, flowrate, gph, Conversion<gallon>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(gallon_per_day, flowrate, gpd, Conversion<gallon>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(gallon_per_year, flowrate, gpy, Conversion<gallon>(1.0) / Conversion<year>(1.0));
DerivedUnitStd(million_gallon_per_second, flowrate, MGS, Conversion<megagallon>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(million_gallon_per_minute, flowrate, MGM, Conversion<megagallon>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(million_gallon_per_hour, flowrate, MGH, Conversion<megagallon>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(million_gallon_per_day, flowrate, MGD, Conversion<megagallon>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(million_gallon_per_year, flowrate, MGY, Conversion<megagallon>(1.0) / Conversion<year>(1.0));
DerivedUnitStd(imperial_million_gallon_per_second, flowrate, IMGS, Conversion<imperial_million_gallon>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion<imperial_million_gallon>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion<imperial_million_gallon>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(imperial_million_gallon_per_day, flowrate, IMGD, Conversion<imperial_million_gallon>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(imperial_million_gallon_per_year, flowrate, IMGY, Conversion<imperial_million_gallon>(1.0) / Conversion<year>(1.0));
DerivedUnitStd(acre_foot_per_second, flowrate, ac_ft_s, Conversion<acre_foot>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(acre_foot_per_minute, flowrate, ac_ft_m, Conversion<acre_foot>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(acre_foot_per_hour, flowrate, ac_ft_h, Conversion<acre_foot>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(acre_foot_per_day, flowrate, ac_ft_d, Conversion<acre_foot>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(acre_foot_per_year, flowrate, ac_ft_y, Conversion<acre_foot>(1.0) / Conversion<year>(1.0));

// DENSITY DERIVATIONS
DerivedUnitStd(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0);
DerivedUnitStd(grams_per_milliliter, density, g_per_mL, Conversion<gram>(1.0) / Conversion<milliliter>(1.0));
DerivedUnitStd(kilograms_per_liter, density, kg_per_L, Conversion<kilogram>(1.0) / Conversion<liter>(1.0));
DerivedUnitStd(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion<ounce>(1.0) / Conversion<cubic_foot>(1.0));
DerivedUnitStd(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion<ounce>(1.0) / Conversion<cubic_inch>(1.0));
DerivedUnitStd(ounces_per_gallon, density, oz_per_gal, Conversion<ounce>(1.0) / Conversion<gallon>(1.0));
DerivedUnitStd(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion<pound>(1.0) / Conversion<cubic_foot>(1.0));
DerivedUnitStd(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion<pound>(1.0) / Conversion<cubic_inch>(1.0));
DerivedUnitStd(pounds_per_gallon, density, lb_per_gal, Conversion<pound>(1.0) / Conversion<gallon>(1.0));
DerivedUnitStd(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion<slug>(1.0) / Conversion<cubic_foot>(1.0));

// DOLLAR RATES DERIVATIONS
DerivedUnitStd(Dollar_per_joule, energy_cost_rate, USD_per_j, Conversion<Dollar>(1.0) / Conversion<joule>(1.0));
DerivedUnitStd(Dollar_per_kilowatt_hour, energy_cost_rate, USD_per_kWh, Conversion<Dollar>(1.0) / Conversion<kilowatt_hour>(1.0));
DerivedUnitStd(Dollar_per_watt, power_cost_rate, USD_per_w, Conversion<Dollar>(1.0) / Conversion<watt>(1.0));
DerivedUnitStd(Dollar_per_kilowatt, power_cost_rate, USD_per_kW, Conversion<Dollar>(1.0) / Conversion<kilowatt>(1.0));
DerivedUnitStd(Dollar_per_cubic_meter, volume_cost_rate, USD_per_cm, Conversion<Dollar>(1.0) / Conversion<cubic_meter>(1.0));
DerivedUnitStd(Dollar_per_gallon, volume_cost_rate, USD_per_gal, Conversion<Dollar>(1.0) / Conversion<gallon>(1.0));

// Rates
DerivedUnitStd(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion<kilowatt_hour>(1.0) / Conversion<acre_foot>(1.0));
DerivedUnitStd(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion<Dollar>(1.0) / Conversion<mile>(1.0));
DerivedUnitStd(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion<Dollar>(1.0) / Conversion<metric_ton>(1.0));
DerivedUnitStd(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion<metric_ton>(1.0) / Conversion<kilowatt_hour>(1.0));

namespace std {
	template<> class numeric_limits<GoodLang::Units::value> {
	public:
		static constexpr double min() { return std::numeric_limits<double>::min(); }
		static constexpr double max() { return std::numeric_limits<double>::max(); }
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); }
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer;
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed;
	};
};

#undef DefineCategoryType
#undef DefineCategoryStd
#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitStdWithMetricPrefixes
#undef DerivedUnitStdWithMetricPrefix
#undef CalculateMetricPrefixV
#undef DerivedUnitType