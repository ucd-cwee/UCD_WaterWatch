#pragma once

#include "UnitsLibrary.h"

namespace scripting {
	namespace UnitsLibrary {
		void UnitsLibrary::Part1(std::shared_ptr<scripting::Namespace2> const& std_namespace, std::shared_ptr<scripting::Class2> const& value_namespace) {
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

			CreateRowWithMetricPrefixes(meter);
			CreateRow(foot);
			CreateRow(inch);
			CreateRow(mile);
			CreateRow(nauticalMile);
			CreateRow(astronicalUnit);
			CreateRow(yard);
			CreateRowWithMetricPrefixes(gram);
			CreateRow(metric_ton);
			CreateRow(pound);
			CreateRow(long_ton);
			CreateRow(short_ton);
			CreateRow(stone);
			CreateRow(ounce);
			CreateRow(carat);
			CreateRow(slug);
			CreateRowWithMetricPrefixes(second);
			CreateRow(minute);
			CreateRow(hour);
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
			CreateRowWithMetricPrefixes(pound_f);
			CreateRow(dyne);
			CreateRow(kilopond);
			CreateRow(poundal);
			CreateRowWithMetricPrefixes(pascals);
			CreateRowWithMetricPrefixes(bar);
			CreateRow(atmosphere);
			CreateRow(pounds_per_square_inch);
			CreateRow(head);
			CreateRow(torr);

#undef CreateRowWithMetricPrefixes
#undef CreateRow


		};
	}
};