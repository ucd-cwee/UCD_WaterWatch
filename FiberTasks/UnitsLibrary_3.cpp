#pragma once

#include "UnitsLibrary.h"

namespace GoodLang {
	namespace UnitsLibrary {
		void UnitsLibrary::Part3(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
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

			CreateRowWithMetricPrefixes(pascals);
			CreateRowWithMetricPrefixes(bar);
			CreateRow(atmosphere);
			CreateRow(pounds_per_square_inch);
			CreateRow(head);
			CreateRow(torr);
			CreateRow(coulomb); // WithMetricPrefixes
			CreateRowWithMetricPrefixes(ampere_hour);
			CreateRowWithMetricPrefixes(watt);

#undef CreateRowWithMetricPrefixes
#undef CreateRow


		};
	}
};