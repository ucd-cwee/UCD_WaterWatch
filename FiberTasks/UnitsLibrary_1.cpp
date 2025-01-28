#pragma once

#include "ScriptingLanguage2.h"

namespace GoodLang {
	namespace UnitsLibrary {
		void UnitsLibrary::Part1(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
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

			//CreateRowWithMetricPrefixes(meter);
			//CreateRow(foot);
			//CreateRow(inch);
			//CreateRow(mile);
			//CreateRow(nauticalMile);
			//CreateRow(astronicalUnit);
			//CreateRow(yard);
			//CreateRowWithMetricPrefixes(gram);
			//CreateRow(metric_ton);
			//CreateRow(pound);
			//CreateRow(long_ton);
			//CreateRow(short_ton);
			//CreateRow(stone);
			//CreateRow(ounce);
			//CreateRow(carat);
			//CreateRow(slug);
			//CreateRowWithMetricPrefixes(second);			
			//CreateRow(minute);
			//CreateRow(hour);

#undef CreateRowWithMetricPrefixes
#undef CreateRow


		};
	}
};