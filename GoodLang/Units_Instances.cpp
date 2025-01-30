#include "Units_Instances.h"

namespace GoodLang {
	// Units
	namespace Units {
		// math
		Units::value math::fabs(const Units::value& V) {
			if (V < 0) return V * -1.0; else return V;
		};
		Units::value math::abs(const Units::value& V) {
			return fabs(V);
		};
		Units::value math::clamp(const Units::value& V, const Units::value& min, const Units::value& max) {
			if (V < min) return min;
			if (V > max) return max;
			return V;
		};
		Units::value math::floor(const Units::value& f) {
			return f.floor();
		};
		Units::value math::ceiling(const Units::value& f) {
			return f.ceiling();
		};
		Units::value math::round(const Units::value& a, float magnitude) {
			return floor((a / magnitude) + 0.5) * magnitude;
		};
		Units::value math::max(const Units::value& a, const Units::value& b) {
			return a > b ? a : b;
		};
		Units::value math::min(const Units::value& a, const Units::value& b) {
			return a < b ? a : b;
		};
		void math::max_ref(Units::value& a, const Units::value& b) {
			if (b > a) a = b;
		};
		void math::min_ref(Units::value& a, const Units::value& b) {
			if (b < a) a = b;
		};

		// constants
		Units::scalar					    constants::pi() {
			return 3.141592653589793238462643383279502884197169399375105820974944;
		};
		Units::meters_per_second		    constants::c() {
			return 299792458.0;
		};
		Units::value				        constants::G() {
			return Units::meter(6.67408e-11) * Units::meter(1) * Units::meter(1) / (Units::kilogram(1) * Units::second(1) * Units::second(1));
		};
		Units::meters_per_second_squared	constants::g() {
			return Units::meters_per_second_squared(9.8067);
		};
		Units::kilograms_per_cubic_meter    constants::d() {
			return Units::kilograms_per_cubic_meter(998.57);
		};

	};
};
