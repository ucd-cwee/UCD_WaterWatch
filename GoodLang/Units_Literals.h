#pragma once
#include "Units_Instances.h"

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
DerivedUnitStd(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0));
DerivedUnitStd(dyne, force, dyn, Conversion <newton>(1.0 / 100000.0));
DerivedUnitStd(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0));
DerivedUnitStd(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));

// PRESSURE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(pascals, pressure, Pa, 1.0);
DerivedUnitStd(bar, pressure, bar, Conversion<kilopascals>(100.0));
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
DerivedUnitStd(joule, energy, J, 1.0);
DerivedUnitStd(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0));
DerivedUnitStd(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0));
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
DerivedUnitStd(gram_per_second, fillrate, gs, 1.0 / 1000.0);
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

#undef DerivedUnitStdWithMetricPrefixes
#undef DerivedUnitStdWithMetricPrefix
#undef DerivedUnitStd



