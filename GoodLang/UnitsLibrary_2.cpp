#pragma once

#include "Units.h"
#include "Scopes.h"

namespace GoodLang {
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

		CreateRow(day);
		CreateRow(week);
		CreateRow(year);
		CreateRow(month);
		CreateRow(julian_year);
		CreateRow(gregorian_year);
		CreateRowWithMetricPrefixes(ampere);
		CreateRow(Dollar);
		CreateRow(MillionDollar);
		CreateRowWithMetricPrefixes(hertz);
		CreateRow(meters_per_second);
		CreateRow(feet_per_second);
		CreateRow(feet_per_minute);
		CreateRow(feet_per_hour);
		CreateRow(miles_per_hour);
		CreateRow(kilometers_per_hour);
		CreateRow(knot);
		CreateRow(meters_per_second_squared);
		CreateRow(feet_per_second_squared);
		CreateRow(standard_gravity);
		CreateRowWithMetricPrefixes(newton);
		CreateRow(pound_f);
		CreateRow(dyne);
		CreateRow(kilopond);
		CreateRow(poundal);

#undef CreateRowWithMetricPrefixes
#undef CreateRow


	};
	
};