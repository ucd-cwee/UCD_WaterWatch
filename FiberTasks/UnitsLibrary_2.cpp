#pragma once

#include "UnitsLibrary.h"

namespace GoodLang {
	namespace UnitsLibrary {
		void UnitsLibrary::Part2(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
#define CreateRow(Type) AddUnit<Units::Type>(std_namespace, value_namespace)
#define CreateRowWithMetricPrefixes(Type)\
			CreateRow(Type); \
			CreateRow(femto ## Type); \
			CreateRow(pico ## Type); \
			CreateRow(nano ## Type); \
			CreateRow(micro ## Type); \
			CreateRow(milli ## Type); \
			CreateRow(centi ## Type); \
			CreateRow(deci ## Type); \
			CreateRow(deca ## Type); \
			CreateRow(hecto ## Type); \
			CreateRow(kilo ## Type); \
			CreateRow(mega ## Type); \
			CreateRow(giga ## Type); \
			CreateRow(tera ## Type); \
			CreateRow(peta ## Type);

			CreateRow(coulomb); // WithMetricPrefixes
			CreateRowWithMetricPrefixes(ampere_hour);
			CreateRowWithMetricPrefixes(watt);
			CreateRow(horsepower);
			CreateRowWithMetricPrefixes(joule);
			CreateRowWithMetricPrefixes(calorie);
			CreateRowWithMetricPrefixes(watt_minute);
			CreateRowWithMetricPrefixes(watt_hour);
			CreateRow(watt_day);
			CreateRow(british_thermal_unit);
			CreateRow(british_thermal_unit_iso);
			CreateRow(british_thermal_unit_59);
			CreateRow(therm);
			CreateRow(foot_pound);
			CreateRowWithMetricPrefixes(volt);
			CreateRowWithMetricPrefixes(ohm);
			CreateRow(siemens); // WithMetricPrefixes
			CreateRow(square_meter);
			CreateRow(square_foot);
			CreateRow(square_inch);
			CreateRow(square_mile);
			CreateRow(square_kilometer);
			CreateRow(hectare);
			CreateRow(acre);
			CreateRow(cubic_meter);
			CreateRow(cubic_millimeter);
			CreateRow(cubic_kilometer);
			CreateRowWithMetricPrefixes(liter);
			CreateRow(cubic_inch);
			CreateRow(cubic_foot);
			CreateRow(cubic_yard);
			CreateRow(cubic_mile);
			CreateRowWithMetricPrefixes(gallon);
			CreateRow(imperial_gallon);
			CreateRow(million_gallon);
			CreateRow(imperial_million_gallon);
			CreateRow(acre_foot);
			CreateRow(quart);
			CreateRow(pint);
			CreateRow(cup);
			CreateRow(fluid_ounce);
			CreateRow(barrel);
			CreateRow(bushel);
			CreateRow(cord);
			CreateRow(tablespoon);
			CreateRow(teaspoon);
			CreateRow(pinch);
			CreateRow(dash);
			CreateRow(drop);
			CreateRow(fifth);
			CreateRow(dram);
			CreateRow(gill);
			CreateRow(peck);
			CreateRow(sack);
			CreateRow(shot);
			CreateRow(strike);

#undef CreateRowWithMetricPrefixes
#undef CreateRow


		};
	}
};