#pragma once
#include "Units_Base.h"

// value declarations
namespace GoodLang {
	namespace Units {
#define CalculateMetricPrefixV(metric) ((long double)std::metric::num / (long double)std::metric::den)
#define DerivedUnitList \
	DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0); \
	DerivedUnitType(foot, length, ft, Conversion<meter>(381.0 / 1250.0)); \
	DerivedUnitType(inch, length, in, Conversion<foot>(1.0 / 12.0)); \
	DerivedUnitType(furlong, length, fur, Conversion<foot>(660)); \
	DerivedUnitType(mile, length, mi, Conversion<foot>(5280)); \
	DerivedUnitType(nauticalMile, length, nmi, Conversion<meter>(1852.0)); \
	DerivedUnitType(astronicalUnit, length, au, Conversion<meter>(149597870700.0)); \
	DerivedUnitType(yard, length, yd, Conversion<foot>(3.0)); \
	DerivedUnitTypeWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0); \
	DerivedUnitType(metric_ton, mass, t, Conversion<kilogram>(1000.0)); \
	DerivedUnitType(pound, mass, lb, Conversion<kilogram>(45359237.0 / 100000000.0)); \
	DerivedUnitType(long_ton, mass, ln_t, Conversion<pound>(2240.0)); \
	DerivedUnitType(short_ton, mass, sh_t, Conversion<pound>(2000.0)); \
	DerivedUnitType(stone, mass, st, Conversion<pound>(14.0)); \
	DerivedUnitType(ounce, mass, oz, Conversion<pound>(1.0 / 16.0)); \
	DerivedUnitType(carat, mass, ct, Conversion<milligram>(200.0)); \
	DerivedUnitType(slug, mass, slug, Conversion<kilogram>(145939029.0 / 10000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(second, time, s, 1.0); \
	DerivedUnitType(minute, time, min, Conversion<second>(60.0)); \
	DerivedUnitType(hour, time, hr, Conversion<minute>(60.0)); \
	DerivedUnitType(day, time, d, Conversion<hour>(24.0)); \
	DerivedUnitType(week, time, wk, Conversion<day>(7.0)); \
	DerivedUnitType(year, time, yr, Conversion<day>(365)); \
	DerivedUnitType(month, time, mnth, Conversion<year>(1.0 / 12.0)); \
	DerivedUnitType(julian_year, time, a_j, Conversion<second>(31557600.0)); \
	DerivedUnitType(gregorian_year, time, a_g, Conversion<second>(31556952.0)); \
	DerivedUnitTypeWithMetricPrefixes(ampere, current, A, 1.0); \
	DerivedUnitType(Dollar, dollar, USD, 1.0); \
	DerivedUnitType(MillionDollar, dollar, MUSD, Conversion<Dollar>(1000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(hertz, frequency, Hz, 1.0); \
	DerivedUnitType(meters_per_second, velocity, mps, Conversion<meter>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(feet_per_second, velocity, fps, Conversion<foot>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(feet_per_minute, velocity, fpm, Conversion<foot>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(feet_per_hour, velocity, fph, Conversion<foot>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(miles_per_hour, velocity, mph, Conversion<mile>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(kilometers_per_hour, velocity, kph, Conversion<kilometer>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(knot, velocity, kts, Conversion<nauticalMile>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(meters_per_second_squared, acceleration, mps_sq, Conversion<meter>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0))); \
	DerivedUnitType(feet_per_second_squared, acceleration, fps_sq, Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0))); \
	DerivedUnitType(standard_gravity, acceleration, SG, Conversion<meters_per_second_squared>(980665.0 / 100000.0)); \
	DerivedUnitTypeWithMetricPrefixes(newton, force, N, Conversion<kilogram>(1.0)* Conversion<meters_per_second_squared>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0)); \
	DerivedUnitType(dyne, force, dyn, Conversion<newton>(1.0 / 100000.0)); \
	DerivedUnitType(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0)); \
	DerivedUnitType(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(pascals, pressure, Pa, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(bar, pressure, bar, Conversion<kilopascals>(100.0)); \
	DerivedUnitType(atmosphere, pressure, atm, Conversion<pascals>(101325.0)); \
	DerivedUnitType(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0))); \
	DerivedUnitType(head, pressure, ft_water, Conversion<pound_f>(62.43) / (Conversion<foot>(1.0) * Conversion<foot>(1.0))); \
	DerivedUnitType(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0)); \
	DerivedUnitType(coulomb, charge, C, 1.0);  \
	DerivedUnitTypeWithMetricPrefixes(ampere_hour, charge, Ah, Conversion< ampere>(1.0)* Conversion<hour>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt, power, W, 1.0); \
	DerivedUnitType(horsepower, power, hp, Conversion<watt>(7457.0 / 10.0)); \
	DerivedUnitTypeWithMetricPrefixes(joule, energy, J, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt_hour, energy, Wh, Conversion<watt>(1.0)* Conversion<hour>(1.0)); \
	DerivedUnitType(watt_day, energy, Wd, Conversion<watt>(1.0)* Conversion<day>(1.0)); \
	DerivedUnitType(british_thermal_unit, energy, BTU, Conversion<joule>(105505585262.0 / 100000000.0)); \
	DerivedUnitType(british_thermal_unit_iso, energy, BTU_iso, Conversion<joule>(1055056.0 / 1000.0)); \
	DerivedUnitType(british_thermal_unit_59, energy, BTU59, Conversion<joule>(1054804.0 / 1000.0)); \
	DerivedUnitType(therm, energy, thm, Conversion<british_thermal_unit_59>(100000.0)); \
	DerivedUnitType(foot_pound, energy, ftlbf, Conversion<joule>(13558179483314004.0 / 10000000000000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(volt, voltage, V, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(ohm, impedance, Ohm, 1.0); \
	DerivedUnitType(siemens, conductance, S, 1.0);  \
	DerivedUnitType(square_meter, area, sq_m, 1.0); \
	DerivedUnitType(square_foot, area, sq_ft, Conversion<foot>(1.0)* Conversion<foot>(1.0)); \
	DerivedUnitType(square_inch, area, sq_in, Conversion<inch>(1.0)* Conversion<inch>(1.0)); \
	DerivedUnitType(square_mile, area, sq_mi, Conversion<mile>(1.0)* Conversion<mile>(1.0)); \
	DerivedUnitType(square_kilometer, area, sq_km, Conversion<kilometer>(1.0)* Conversion<kilometer>(1.0)); \
	DerivedUnitType(hectare, area, ha, Conversion<square_meter>(1000.0)); \
	DerivedUnitType(acre, area, acre, Conversion<square_foot>(43560.0)); \
	DerivedUnitType(cubic_meter, volume, cu_m, 1.0); \
	DerivedUnitType(cubic_millimeter, volume, cu_mm, CUBED(Conversion<millimeter>(1.0))); \
	DerivedUnitType(cubic_kilometer, volume, cu_km, CUBED(Conversion<kilometer>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(liter, volume, L, CUBED(Conversion<decimeter>(1.0))); \
	DerivedUnitType(cubic_inch, volume, cu_in, CUBED(Conversion<inch>(1.0))); \
	DerivedUnitType(cubic_foot, volume, cu_ft, CUBED(Conversion<foot>(1.0))); \
	DerivedUnitType(cubic_yard, volume, cu_yd, CUBED(Conversion<yard>(1.0))); \
	DerivedUnitType(cubic_mile, volume, cu_mi, CUBED(Conversion<mile>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(gallon, volume, gal, Conversion<cubic_inch>(231.0)); \
	DerivedUnitType(imperial_gallon, volume, igal, Conversion<gallon>(10.0 / 12.0)); \
	DerivedUnitType(million_gallon, volume, MG, Conversion<gallon>(1.0)* CalculateMetricPrefixV(mega)); \
	DerivedUnitType(imperial_million_gallon, volume, IMG, Conversion<imperial_gallon>(1.0)* CalculateMetricPrefixV(mega)); \
	DerivedUnitType(acre_foot, volume, ac_ft, Conversion<acre>(1.0)* Conversion<foot>(1.0)); \
	DerivedUnitType(quart, volume, qt, Conversion<gallon>(0.25)); \
	DerivedUnitType(pint, volume, pt, Conversion<quart>(0.5)); \
	DerivedUnitType(cup, volume, c, Conversion<pint>(0.5)); \
	DerivedUnitType(fluid_ounce, volume, fl_oz, Conversion<cup>(0.125)); \
	DerivedUnitType(barrel, volume, bl, Conversion<gallon>(42.0)); \
	DerivedUnitType(bushel, volume, bu, Conversion<cubic_inch>(215042.0 / 100.0)); \
	DerivedUnitType(cord, volume, cord, Conversion<cubic_foot>(128.0)); \
	DerivedUnitType(tablespoon, volume, tbsp, Conversion<fluid_ounce>(0.5)); \
	DerivedUnitType(teaspoon, volume, tsp, Conversion<fluid_ounce>(1.0 / 6.0)); \
	DerivedUnitType(pinch, volume, pinch, Conversion<teaspoon>(1.0 / 8.0)); \
	DerivedUnitType(dash, volume, dash, Conversion<pinch>(1.0 / 2.0)); \
	DerivedUnitType(drop, volume, drop, Conversion<fluid_ounce>(1.0 / 360.0)); \
	DerivedUnitType(fifth, volume, fifth, Conversion<gallon>(0.2)); \
	DerivedUnitType(dram, volume, dr, Conversion<fluid_ounce>(0.125)); \
	DerivedUnitType(gill, volume, gi, Conversion<fluid_ounce>(4.0)); \
	DerivedUnitType(peck, volume, pk, Conversion<bushel>(0.25)); \
	DerivedUnitType(sack, volume, sacks, Conversion<bushel>(3.0)); \
	DerivedUnitType(shot, volume, shots, Conversion<fluid_ounce>(3.0 / 2.0)); \
	DerivedUnitType(strike, volume, strikes, Conversion<bushel>(2.0)); \
	DerivedUnitTypeWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0); \
	DerivedUnitType(metric_ton_per_second, fillrate, mTs, Conversion<metric_ton>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(metric_ton_per_minute, fillrate, mTm, Conversion<metric_ton>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(metric_ton_per_hour, fillrate, mTh, Conversion<metric_ton>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(metric_ton_per_day, fillrate, mTd, Conversion<metric_ton>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(metric_ton_per_year, fillrate, mTy, Conversion<metric_ton>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(cubic_meter_per_second, flowrate, cms, 1.0); \
	DerivedUnitType(cubic_meter_per_hour, flowrate, cmh, Conversion<cubic_meter>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(cubic_meter_per_day, flowrate, cmd, Conversion<cubic_meter>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(cubic_millimeter_per_second, flowrate, cmms, Conversion<cubic_millimeter>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion<liter>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(liter_per_minute, flowrate, lpm, Conversion<liter>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(liter_per_day, flowrate, lpd, Conversion<liter>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(megaliter_per_day, flowrate, Mlpd, Conversion<megaliter>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(cubic_inch_per_second, flowrate, cis, Conversion<cubic_inch>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(cubic_inch_per_hour, flowrate, cih, Conversion<cubic_inch>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(cubic_foot_per_second, flowrate, cfs, Conversion<cubic_foot>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(cubic_foot_per_hour, flowrate, cfh, Conversion<cubic_foot>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(gallon_per_second, flowrate, gps, Conversion<gallon>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(gallon_per_minute, flowrate, gpm, Conversion<gallon>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(gallon_per_hour, flowrate, gph, Conversion<gallon>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(gallon_per_day, flowrate, gpd, Conversion<gallon>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(gallon_per_year, flowrate, gpy, Conversion<gallon>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(million_gallon_per_second, flowrate, MGS, Conversion<megagallon>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(million_gallon_per_minute, flowrate, MGM, Conversion<megagallon>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(million_gallon_per_hour, flowrate, MGH, Conversion<megagallon>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(million_gallon_per_day, flowrate, MGD, Conversion<megagallon>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(million_gallon_per_year, flowrate, MGY, Conversion<megagallon>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_second, flowrate, IMGS, Conversion<imperial_million_gallon>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion<imperial_million_gallon>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion<imperial_million_gallon>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_day, flowrate, IMGD, Conversion<imperial_million_gallon>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_year, flowrate, IMGY, Conversion<imperial_million_gallon>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(acre_foot_per_second, flowrate, ac_ft_s, Conversion<acre_foot>(1.0) / Conversion<second>(1.0)); \
	DerivedUnitType(acre_foot_per_minute, flowrate, ac_ft_m, Conversion<acre_foot>(1.0) / Conversion<minute>(1.0)); \
	DerivedUnitType(acre_foot_per_hour, flowrate, ac_ft_h, Conversion<acre_foot>(1.0) / Conversion<hour>(1.0)); \
	DerivedUnitType(acre_foot_per_day, flowrate, ac_ft_d, Conversion<acre_foot>(1.0) / Conversion<day>(1.0)); \
	DerivedUnitType(acre_foot_per_year, flowrate, ac_ft_y, Conversion<acre_foot>(1.0) / Conversion<year>(1.0)); \
	DerivedUnitType(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0); \
	DerivedUnitType(grams_per_milliliter, density, g_per_mL, Conversion<gram>(1.0) / Conversion<milliliter>(1.0)); \
	DerivedUnitType(kilograms_per_liter, density, kg_per_L, Conversion<kilogram>(1.0) / Conversion<liter>(1.0)); \
	DerivedUnitType(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion<ounce>(1.0) / Conversion<cubic_foot>(1.0)); \
	DerivedUnitType(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion<ounce>(1.0) / Conversion<cubic_inch>(1.0)); \
	DerivedUnitType(ounces_per_gallon, density, oz_per_gal, Conversion<ounce>(1.0) / Conversion<gallon>(1.0)); \
	DerivedUnitType(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion<pound>(1.0) / Conversion<cubic_foot>(1.0)); \
	DerivedUnitType(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion<pound>(1.0) / Conversion<cubic_inch>(1.0)); \
	DerivedUnitType(pounds_per_gallon, density, lb_per_gal, Conversion<pound>(1.0) / Conversion<gallon>(1.0)); \
	DerivedUnitType(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion<slug>(1.0) / Conversion<cubic_foot>(1.0)); \
	DerivedUnitType(Dollar_per_joule, energy_cost_rate, USD_per_j, Conversion<Dollar>(1.0) / Conversion<joule>(1.0)); \
	DerivedUnitType(Dollar_per_kilowatt_hour, energy_cost_rate, USD_per_kWh, Conversion<Dollar>(1.0) / Conversion<kilowatt_hour>(1.0)); \
	DerivedUnitType(Dollar_per_watt, power_cost_rate, USD_per_w, Conversion<Dollar>(1.0) / Conversion<watt>(1.0)); \
	DerivedUnitType(Dollar_per_kilowatt, power_cost_rate, USD_per_kW, Conversion<Dollar>(1.0) / Conversion<kilowatt>(1.0)); \
	DerivedUnitType(Dollar_per_cubic_meter, volume_cost_rate, USD_per_cm, Conversion<Dollar>(1.0) / Conversion<cubic_meter>(1.0)); \
	DerivedUnitType(Dollar_per_gallon, volume_cost_rate, USD_per_gal, Conversion<Dollar>(1.0) / Conversion<gallon>(1.0)); \
	DerivedUnitType(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion<kilowatt_hour>(1.0) / Conversion<acre_foot>(1.0)); \
	DerivedUnitType(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion<Dollar>(1.0) / Conversion<mile>(1.0)); \
	DerivedUnitType(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion<Dollar>(1.0) / Conversion<metric_ton>(1.0)); \
	DerivedUnitType(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion<metric_ton>(1.0) / Conversion<kilowatt_hour>(1.0));

#define DerivedUnitType(type, category, abbreviation, Ratio) \
	class type final : public value { \
	public: \
		constexpr static double conversion_ratio{ Ratio }; \
		type(); \
		type(value const& other); \
		type(Number const& V); \
		~type() = default; \
	};

#define DerivedUnitTypeWithMetricPrefix(type, prefix) class prefix ## type final : public value { \
	public: \
		constexpr static double conversion_ratio{ type::conversion_ratio * ((double)std::prefix::num / (double)std::prefix::den) }; \
		prefix ## type(); \
		prefix ## type(value const& other); \
		prefix ## type(Number const& V); \
		~prefix ## type() = default; \
	};

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

		class math {
		public:
			// return absolute value
			static Units::value fabs(const Units::value& V);
			// return absolute value
			static Units::value abs(const Units::value& V);
			// clamp number to lower/upper bound
			static Units::value clamp(const Units::value& V, const Units::value& min, const Units::value& max);
			// round to lower whole number
			static Units::value floor(const Units::value& f);
			// round to higher whole number
			static Units::value ceiling(const Units::value& f);
			// round to nearest whole number
			static Units::value round(const Units::value& a, float magnitude);
			// return max(a, b);
			static Units::value max(const Units::value& a, const Units::value& b);
			// return min(a, b);
			static Units::value min(const Units::value& a, const Units::value& b);
			// if (b > a) a = b; // prevents copying when not necessary
			static void max_ref(Units::value& a, const Units::value& b);
			// if (b < a) a = b; // prevents copying when not necessary
			static void min_ref(Units::value& a, const Units::value& b);
		};

		class constants {
		public:
			/* PI (unitless) */
			static Units::scalar					pi();

			/* speed of light in a vacuum (m/s) */
			static Units::meters_per_second		    c();

			/* ( m^3 / (kg * s^2) ) */
			static Units::value				        G();

			/* acceleration due to gravity ( m/s^2 ) */
			static Units::meters_per_second_squared	g();

			/* density of water ( kg/m^3 ) */
			static Units::kilograms_per_cubic_meter d();
		};



	};

#define DerivedUnitType(type, category, abbreviation, Ratio) \
    template <> __forceinline std::string ToString(Units:: ## type const& r) { return r.ToString(); }

#define DerivedUnitTypeWithMetricPrefix(type, prefix) \
	template <> __forceinline std::string ToString(Units:: ## prefix ## type const& r) { return r.ToString(); }

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





};