#pragma once

#include "Units.h"
#include "Scopes.h"

namespace GoodLang {
	void UnitsLibrary::Part6(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
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

		CreateRow(gram_per_second);
		CreateRow(metric_ton_per_second);
		CreateRow(metric_ton_per_minute);
		CreateRow(metric_ton_per_hour);
		CreateRow(metric_ton_per_day);
		CreateRow(metric_ton_per_year);
		CreateRow(cubic_meter_per_second);
		CreateRow(cubic_meter_per_hour);
		CreateRow(cubic_meter_per_day);
		CreateRow(cubic_millimeter_per_second);
		CreateRowWithMetricPrefixes(liter_per_second);
		CreateRow(liter_per_minute);
		CreateRow(liter_per_day);
		CreateRow(megaliter_per_day);
		CreateRow(cubic_inch_per_second);
		CreateRow(cubic_inch_per_hour);
		CreateRow(cubic_foot_per_second);
		CreateRow(cubic_foot_per_hour);
		CreateRow(gallon_per_second);
		CreateRow(gallon_per_minute);
		CreateRow(gallon_per_hour);
		CreateRow(gallon_per_day);
		CreateRow(gallon_per_year);
		CreateRow(million_gallon_per_second);
		CreateRow(million_gallon_per_minute);
		CreateRow(million_gallon_per_hour);
		CreateRow(million_gallon_per_day);
		CreateRow(million_gallon_per_year);
		CreateRow(imperial_million_gallon_per_second);
		CreateRow(imperial_million_gallon_per_minute);
		CreateRow(imperial_million_gallon_per_hour);
		CreateRow(imperial_million_gallon_per_day);
		CreateRow(imperial_million_gallon_per_year);
		CreateRow(acre_foot_per_second);
		CreateRow(acre_foot_per_minute);
		CreateRow(acre_foot_per_hour);
		CreateRow(acre_foot_per_day);
		CreateRow(acre_foot_per_year);
		CreateRow(kilograms_per_cubic_meter);
		CreateRow(grams_per_milliliter);
		CreateRow(kilograms_per_liter);
		CreateRow(ounces_per_cubic_foot);
		CreateRow(ounces_per_cubic_inch);
		CreateRow(ounces_per_gallon);
		CreateRow(pounds_per_cubic_foot);
		CreateRow(pounds_per_cubic_inch);
		CreateRow(pounds_per_gallon);
		CreateRow(slugs_per_cubic_foot);
		CreateRow(Dollar_per_joule);
		CreateRow(Dollar_per_kilowatt_hour);
		CreateRow(Dollar_per_watt);
		CreateRow(Dollar_per_kilowatt);
		CreateRow(Dollar_per_cubic_meter);
		CreateRow(Dollar_per_gallon);
		CreateRow(kilowatt_hour_per_acre_foot);
		CreateRow(Dollar_per_mile);
		CreateRow(Dollar_per_ton);
		CreateRow(ton_per_kilowatt_hour);

#undef CreateRowWithMetricPrefixes
#undef CreateRow


	};
};