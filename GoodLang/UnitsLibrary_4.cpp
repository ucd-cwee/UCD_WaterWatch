#pragma once

#include "Units.h"
#include "Scopes.h"

namespace GoodLang {
	void UnitsLibrary::Part4(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
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

		CreateRow(horsepower);
		CreateRow(joule);
		CreateRow(calorie);
		CreateRow(watt_minute);
		CreateRow(watt_hour);
		CreateRow(watt_day);
		CreateRow(british_thermal_unit);
		CreateRow(british_thermal_unit_iso);
		CreateRow(british_thermal_unit_59);
		CreateRow(therm);
		CreateRow(foot_pound);

#undef CreateRowWithMetricPrefixes
#undef CreateRow
	};	
};