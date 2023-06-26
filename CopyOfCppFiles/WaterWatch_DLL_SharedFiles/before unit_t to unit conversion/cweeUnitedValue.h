#pragma once
#include "Precompiled.h"
#include <cstdint>
#include <type_traits>

	namespace constexpr_to_string {

		constexpr char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

		/**
		 * @struct to_string_t
		 * @brief Provides the ability to convert any integral to a string at compile-time.
		 * @tparam N Number to convert
		 * @tparam base Desired base, can be from 2 to 36
		 */
		template<std::intmax_t N, int base, typename char_type,
			std::enable_if_t<(base > 1 && base < sizeof(digits)), int> = 0>
			class to_string_t {

			constexpr static AUTO buflen() noexcept {
				unsigned int len = N > 0 ? 1 : 2;
				for (auto n = N; n; len++, n /= base);
				return len;
			};

			constexpr static AUTO bufLen = buflen();
			char_type buf[bufLen] = {};

			public:
				/**
				 * Constructs the object, filling `buf` with the string representation of N.
				 */
				constexpr to_string_t() noexcept {
					auto ptr = end();
					*--ptr = '\0';

					if (N != 0) {
						for (auto n = N; n; n /= base)
							*--ptr = digits[(N < 0 ? -1 : 1) * (n % base)];
						if (N < 0)
							*--ptr = '-';
					}
					else {
						buf[0] = '0';
					}
				}

				// Support implicit casting to `char *` or `const char *`.
				constexpr operator char_type* () noexcept { return buf; }
				constexpr operator const char_type* () const noexcept { return buf; }

				constexpr auto size() const noexcept { return sizeof(buf) / sizeof(buf[0]); }

				// Element access
				constexpr auto data() noexcept { return buf; }
				constexpr const auto data() const noexcept { return buf; }
				constexpr auto& operator[](unsigned int i) noexcept { return buf[i]; }
				constexpr const auto& operator[](unsigned int i) const noexcept { return buf[i]; }
				constexpr auto& front() noexcept { return buf[0]; }
				constexpr const auto& front() const noexcept { return buf[0]; }
				constexpr auto& back() noexcept { return buf[size() - 1]; }
				constexpr const auto& back() const noexcept { return buf[size() - 1]; }

				// Iterators
				constexpr auto begin() noexcept { return buf; }
				constexpr const auto begin() const noexcept { return buf; }
				constexpr auto end() noexcept { return buf + size(); }
				constexpr const auto end() const noexcept { return buf + size(); }
		};

	} // namespace constexpr_to_string
	template<std::intmax_t N> constexpr const char* ConstexprIntToString() { static constexpr AUTO str = constexpr_to_string::to_string_t<N, 10, char>(); return str(); };

namespace cweeUnitValues {
	BETTER_ENUM(unit_value_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);

	INLINE constexpr unsigned long long HashUnits(double a, double b, double c, double d, double e) noexcept {
		constexpr AUTO A = 54059; /* a prime */
		constexpr AUTO B = 76963; /* another prime */
		constexpr AUTO C = 86969; /* yet another prime */
		constexpr AUTO FIRSTH = 37; /* also prime */

		size_t h = FIRSTH;
		h = (h * A) ^ (long long)(a * B * 100.0);
		h = (h * A) ^ (long long)(b * B * 100.0);
		h = (h * A) ^ (long long)(c * B * 100.0);
		h = (h * A) ^ (long long)(d * B * 100.0);
		h = (h * A) ^ (long long)(e * B * 100.0);

		AUTO result = h % C;
		return result;
	};
	INLINE constexpr unsigned long long HashUnitAndRatio(unsigned long long unitHash, double ratio) noexcept {
		constexpr AUTO A = 54059; /* a prime */
		constexpr AUTO B = 76963; /* another prime */
		constexpr AUTO C = 86969; /* yet another prime */
		constexpr AUTO FIRSTH = 37; /* also prime */

		size_t h = FIRSTH;
		h = (h * A) ^ (long long)(unitHash * B);
		h = (h * A) ^ (long long)(ratio * B * 10000000.0);

		AUTO result = h % C;
		return result;
	};

	namespace cweeUnitValuesDetail { const char* lookup_abbreviation(unsigned long long ull); };

	class Unit_ID {
		static constexpr size_t NumUnits = unit_value_type::_size_constant;

	public:
		constexpr Unit_ID() noexcept : 
			unitType_m{ 0.0, 0.0, 0.0, 0.0, 0.0 }, isScalar_m(true), abbreviation_m(""), ratio_m(1.) 
		{};
		constexpr Unit_ID(double a, double b, double c, double d, double e, double isScalar_p, const char* abbreviation_p, double ratio_p) noexcept :
			unitType_m{ a, b, c, d, e }, isScalar_m(isScalar_p), abbreviation_m(abbreviation_p), ratio_m(ratio_p)
		{};
		~Unit_ID() {};

	public:
		bool IsSameCategory(Unit_ID const& other) const noexcept {
			if (isScalar_m && other.isScalar_m) return true;
			for (int i = NumUnits - 1; i >= 0; i--)
				if (unitType_m[i] != other.unitType_m[i]) return false;
			return true;
		};
		bool IsSameUnit(Unit_ID const& other) const noexcept {
			return IsSameCategory(other) && (ratio_m == other.ratio_m);
		};
		unsigned long long HashCategory() const noexcept {
			return cweeUnitValues::HashUnits(unitType_m[0], unitType_m[1], unitType_m[2], unitType_m[3], unitType_m[4]);
		};
		const char* LookupAbbreviation() const {
			using namespace cweeUnitValuesDetail;
			return lookup_abbreviation(HashUnitAndRatio(HashCategory(), ratio_m));
		};

	public:
		std::array< double, NumUnits> unitType_m;
		bool isScalar_m;
		const char* abbreviation_m;
		double ratio_m;
	};

	class unit_value {
	public:
		Unit_ID unit_m;
		double value_m;

	protected:
		double conversion() const noexcept { return unit_m.ratio_m; };

	public: // constructors
		unit_value() noexcept : unit_m(), value_m(0.0) {};
		unit_value(double V) noexcept : unit_m(), value_m(V * conversion()) {};
		unit_value(Unit_ID const& unit_p) noexcept : unit_m(unit_p), value_m(0.0) {};
		unit_value(double V, Unit_ID const& unit_p) noexcept : unit_m(unit_p), value_m(V * conversion()) {};
		unit_value(unit_value const& V) noexcept : unit_m(V.unit_m), value_m(V.value_m) {};
		unit_value(unit_value&& V) noexcept : unit_m(std::move(V.unit_m)), value_m(std::move(V.value_m)) {};
		virtual ~unit_value() {};
	private:
		double GetVisibleValue() const noexcept { return value_m / conversion(); };
	public: // value operator
		explicit operator double() const noexcept { return GetVisibleValue(); };
		double operator()() const noexcept { return GetVisibleValue(); };

	public: // Functions
		const char* Abbreviation() const noexcept {
			if (unit_m.abbreviation_m == "") {
				return CreateAbbreviation();
			}
			else {
				return unit_m.abbreviation_m;
			}
		};
	
	public: // Non-overriden Functions
		cweeStr ToString() const {
			cweeStr abbreviation(Abbreviation());
			if (abbreviation.Length() > 0) return GetValueStr(*this) + " " + abbreviation;
			else return GetValueStr(*this);
		};
		cweeStr CreateAbbreviation() const {
			cweeStr out; double v;
			{
				v = unit_m.unitType_m[unit_value_type::METERS];
				if (v != 0)
					if (v == 1)
						out.AddToDelimiter("m", " ");
					else
						out.AddToDelimiter(cweeStr::printf("m^%s", cweeStr((float)v).c_str()), " ");
			}
			{
				v = unit_m.unitType_m[unit_value_type::KILOGRAMS];
				if (v != 0)
					if (v == 1)
						out.AddToDelimiter("kg", " ");
					else
						out.AddToDelimiter(cweeStr::printf("kg^%s", cweeStr((float)v).c_str()), " ");
			}
			{
				v = unit_m.unitType_m[unit_value_type::SECONDS];
				if (v != 0)
					if (v == 1)
						out.AddToDelimiter("s", " ");
					else
						out.AddToDelimiter(cweeStr::printf("s^%s", cweeStr((float)v).c_str()), " ");
			}
			{
				v = unit_m.unitType_m[unit_value_type::AMPERES];
				if (v != 0)
					if (v == 1)
						out.AddToDelimiter("a", " ");
					else
						out.AddToDelimiter(cweeStr::printf("a^%s", cweeStr((float)v).c_str()), " ");
			}
			{
				v = unit_m.unitType_m[unit_value_type::DOLLAR];
				if (v != 0)
					if (v == 1)
						out.AddToDelimiter("$", " ");
					else
						out.AddToDelimiter(cweeStr::printf("$^%s", cweeStr((float)v).c_str()), " ");
			}
			return out;
		};

	public: // Streaming functions (should be specialized per type)
		friend inline std::ostream& operator<<(std::ostream& os, unit_value const& obj) { os << obj.ToString(); return os; };
		friend inline std::stringstream& operator>>(std::stringstream& os, unit_value& obj) { double v = 0; os >> v; obj = v; return os; };

	private:
		static cweeStr GetValueStr(unit_value const& V) noexcept { return cweeStr((float)V()); };
		static bool is_scalar(unit_value const& V) noexcept { if (V.unit_m.isScalar_m) return true; return false; };
		static bool IdenticalUnits(unit_value const& LHS, unit_value const& RHS) noexcept { return LHS.unit_m.IsSameCategory(RHS.unit_m); };
		static bool NormalArithmeticOkay(unit_value const& LHS, unit_value const& RHS) noexcept {
			if (is_scalar(LHS) || is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		static bool UnaryArithmeticOkay(unit_value& LHS, unit_value const& RHS) noexcept {
			if (is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		static void HandleNormalArithmetic(unit_value const& LHS, unit_value const& RHS) {
			if (NormalArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error("Normal, dynamic arithmetic failed due to incompatible non-scalar units."));
		};
		static void HandleUnaryArithmetic(unit_value& LHS, unit_value const& RHS) {
			if (UnaryArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error("Unary (in-place or self-modifying) arithmetic failed due to incompatible units."));
		};
		static void HandleNotScalar(unit_value const& V) {
			if (is_scalar(V)) return;
			throw(std::runtime_error("Type must be scalar."));
		};
		template <bool multiplication = true> 
		unit_value& CompoundUnits(unit_value const& V) noexcept {
			if (is_scalar(*this) && is_scalar(V)) return *this;
			// if V is a scaler, then there is no point changing this unit type
			if (is_scalar(V)) return *this;
			// V is not a scaler, but I could be.
			bool allZero = true;
			for (int i = unit_m.unitType_m.size() - 1; i >= 0; i--) {
				if constexpr (multiplication) {
					unit_m.unitType_m[i] += V.unit_m.unitType_m[i];
				}
				else {
					unit_m.unitType_m[i] -= V.unit_m.unitType_m[i];
				}
				allZero = allZero && unit_m.unitType_m[i] == 0;
			}
			if (allZero) { unit_m.isScalar_m = true; }
			else { unit_m.isScalar_m = false; }

			// now that we have modified the units, the conversion ratio makes no sense anymore and must be reset.
#if 0
			// effectively turn it into SI units since we don't know what it is supposed to be
			unit_m.abbreviation_m = ""; 
			unit_m.ratio_m = 1.0; 
#else
			if constexpr (multiplication) {
				unit_m.ratio_m *= V.unit_m.ratio_m;
			}
			else {
				unit_m.ratio_m /= V.unit_m.ratio_m;
			}
			unit_m.abbreviation_m = unit_m.LookupAbbreviation(); // To-Do: look-up the abbreviation based on the ratio and the unit vector. (i.e. [1.0, 0.0, 0.0, 0.0, 0.0, 1.0] = "m")
			if (unit_m.abbreviation_m == "") unit_m.ratio_m = 1; // return to SI units if unknown combination
#endif
			return *this;
		};
		unit_value& MultiplyUnits(double const& V) noexcept {
			if (is_scalar(*this)) return *this;
			for (int i = unit_m.unitType_m.size() - 1; i >= 0; i--) unit_m.unitType_m[i] *= V;
			if (V == 0) unit_m.isScalar_m = true;

			// now that we have modified the units, the conversion ratio makes no sense anymore and must be reset. 
#if 0
			unit_m.abbreviation_m = "";
			unit_m.ratio_m = 1.0;
#else
			unit_m.ratio_m = std::pow(unit_m.ratio_m, V);
			unit_m.abbreviation_m = unit_m.LookupAbbreviation();
			if (unit_m.abbreviation_m == "") unit_m.ratio_m = 1; // return to SI units if unknown combination
#endif

			return *this;
		};

	public: // = Operators
		unit_value& operator=(unit_value const& V) {
			if (this->unit_m.IsSameCategory(V.unit_m)) { // same category, but perhaps different conversion factor. That's OK. 
				value_m = V.value_m;
			}
			else if (is_scalar(V)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				value_m = V.GetVisibleValue() * conversion();
			}
			else if (is_scalar(*this)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
				unit_m = V.unit_m;
				value_m = V.value_m;
			}
			else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
				throw(std::runtime_error("Assignment failed due to incompatible non-scalar units."));
			}
			return *this;
		};
		unit_value& operator=(unit_value&& V) {
			if (this->unit_m.IsSameCategory(V.unit_m)) { //same category, but perhaps different conversion factor. That's OK. 
				value_m = V.value_m;
			}
			else if (is_scalar(V)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				value_m = V.GetVisibleValue() * conversion();
			}
			else if (is_scalar(*this)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
				unit_m = V.unit_m;
				value_m = V.value_m;
			}
			else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
				throw(std::runtime_error("Assignment failed due to incompatible non-scalar units."));
			}
			return *this;
		};

	public: // Comparison operators
		bool operator==(unit_value const& V) const noexcept { if (NormalArithmeticOkay(*this, V) && GetVisibleValue() == V.GetVisibleValue()) { return true; } else { return false; } };
		bool operator!=(unit_value const& V) const noexcept { return !(operator==(V)); };
		bool operator<(unit_value const& V) const { HandleNormalArithmetic(*this, V); return value_m < V.value_m; };
		bool operator<=(unit_value const& V) const { HandleNormalArithmetic(*this, V); return value_m <= V.value_m; };
		bool operator>(unit_value const& V) const { HandleNormalArithmetic(*this, V); return value_m > V.value_m; };
		bool operator>=(unit_value const& V) const { HandleNormalArithmetic(*this, V); return value_m >= V.value_m; };

	public: // Unary operators
		unit_value& operator++() { value_m = (GetVisibleValue() + 1) * conversion(); return *this; };
		unit_value& operator--() { value_m = (GetVisibleValue() - 1) * conversion(); return *this; };
		unit_value operator++(int) { unit_value out = *this; value_m = (GetVisibleValue() + 1) * conversion(); return out; };
		unit_value operator--(int) { unit_value out = *this; value_m = (GetVisibleValue() - 1) * conversion(); return out; };

	public: // + and - Operators
		static unit_value Add(unit_value const& a, unit_value const& b) {
			HandleNormalArithmetic(a, b);
			unit_value out1 = a;
			unit_value out2 = a; out2 = b;
			out1.value_m += out2.value_m;
			return out1;
		};
		static unit_value Sub(unit_value const& a, unit_value const& b) {
			HandleNormalArithmetic(a, b);
			unit_value out1 = a;
			unit_value out2 = a; out2 = b;
			out1.value_m -= out2.value_m;
			return out1;
		};
		unit_value operator+(unit_value const& V) const { return Add(*this, V); };
		unit_value operator-(unit_value const& V) const { return Sub(*this, V); };
		unit_value& operator+=(unit_value const& V) { HandleUnaryArithmetic(*this, V); unit_value temp = *this; temp = V; value_m += temp.value_m; return *this; };
		unit_value& operator-=(unit_value const& V) { HandleUnaryArithmetic(*this, V); unit_value temp = *this; temp = V; value_m -= temp.value_m; return *this; };

	public: // * and / Operators
		unit_value operator*(unit_value const& V) const {
			unit_value out = *this;
			out.CompoundUnits<true>(V);
			out.value_m *= V.value_m;
			return out;
		};
		unit_value operator/(unit_value const& V) const {
			unit_value out = *this;
			out.CompoundUnits<false>(V);
			out.value_m /= V.value_m;
			return out;
		};
		unit_value& operator*=(unit_value const& V) { 
			HandleNotScalar(V); 
			value_m *= V.value_m; 
			return *this; 
		};
		unit_value& operator/=(unit_value const& V) { 
			HandleNotScalar(V); 
			value_m /= V.value_m; 
			return *this; 
		};

	public: // pow and sqrt Operators
		unit_value pow(unit_value const& V) const { HandleNotScalar(V); unit_value out = *this; out.MultiplyUnits(V.value_m); out.value_m = std::pow(out.GetVisibleValue(), V.GetVisibleValue()) * (conversion() * V.GetVisibleValue()); return out; };
		unit_value& pow_value(unit_value const& V) { HandleNotScalar(V); value_m = std::pow(GetVisibleValue(), V.GetVisibleValue()) * conversion(); return *this; };
		unit_value sqrt() const { unit_value out = *this; out.MultiplyUnits(0.5); out.value_m = std::sqrt(out.value_m); return out; };

	};
	using scalar = unit_value;

#define DefineCategoryType(type, a, b, c, d, e) class type : public unit_value { public: \
		type() noexcept : unit_value(0.0, Unit_ID(a, b, c, d, e, false, "", 1.0)) {}; \
		type(double V) noexcept : unit_value(V, Unit_ID(a, b, c, d, e, false, "", 1.0)) {}; \
		type(double V, const char* abbreviation) noexcept : unit_value(V, Unit_ID(a, b, c, d, e, false, abbreviation, 1.0)) {}; \
		type(double V, const char* abbreviation, double ratio) noexcept : unit_value(V, Unit_ID(a, b, c, d, e, false, abbreviation, ratio)) {}; \
		type(unit_value const& V) noexcept : unit_value(V) {}; \
		type(unit_value&& V) noexcept : unit_value(std::forward<unit_value>(V)) {}; \
		friend inline std::ostream& operator<<(std::ostream& os, type const& obj) { os << obj.ToString(); return os; }; \
		friend inline std::stringstream& operator>>(std::stringstream& os, type& obj) { double v = 0; os >> v; obj = v; return os; }; \
		constexpr static double A() noexcept { return a; } \
		constexpr static double B() noexcept { return b; } \
		constexpr static double C() noexcept { return c; } \
		constexpr static double D() noexcept { return d; } \
		constexpr static double E() noexcept { return e; } \
	};

	// Base classes
	DefineCategoryType(length, 1, 0, 0, 0, 0);
	DefineCategoryType(mass, 0, 1, 0, 0, 0);
	DefineCategoryType(time, 0, 0, 1, 0, 0);
	DefineCategoryType(current, 0, 0, 0, 1, 0);
	DefineCategoryType(dollar, 0, 0, 0, 0, 1);

	// Derived classes
	DefineCategoryType(frequency, 0, 0, -1, 0, 0);
	DefineCategoryType(velocity, 1, 0, -1, 0, 0);
	DefineCategoryType(acceleration, 1, 0, -2, 0, 0);
	DefineCategoryType(force, 1, 1, -2, 0, 0);
	DefineCategoryType(pressure, -1, 1, -2, 0, 0);
	DefineCategoryType(charge, 0, 0, 1, 1, 0);
	DefineCategoryType(power, 2, 1, -3, 0, 0);
	DefineCategoryType(energy, 2, 1, -2, 0, 0);
	DefineCategoryType(voltage, 2, 1, -3, -1, 0);
	DefineCategoryType(impedance, 2, 1, -3, -2, 0);
	DefineCategoryType(conductance, -2, -1, 3, 2, 0);
	DefineCategoryType(area, 2, 0, 0, 0, 0);
	DefineCategoryType(volume, 3, 0, 0, 0, 0);
	DefineCategoryType(fillrate, 0, 1, -1, 0, 0);
	DefineCategoryType(flowrate, 3, 0, -1, 0, 0);
	DefineCategoryType(density, -3, 1, 0, 0, 0);
	DefineCategoryType(energy_cost_rate, -2, -1, 2, 0, 1);
	DefineCategoryType(power_cost_rate, -2, -1, 3, 0, 1);
	DefineCategoryType(volume_cost_rate, -3, 0, 0, 0, 1);

#undef DefineCategoryType

#define DerivedUnitType(type, category, abbreviation, ratio) class type : public category  { public: \
		constexpr static double conversion() noexcept { return ratio; }; \
		constexpr static const char* specialized_abbreviation() noexcept { return #abbreviation; }; \
		type() noexcept : category(0.0, specialized_abbreviation(), ratio) {}; \
		type(double V) noexcept : category(V, specialized_abbreviation(), ratio) {}; \
		type(unit_value const& V) noexcept : category(V) {}; \
		type(unit_value&& V) noexcept : category(std::forward<unit_value>(V)) {}; \
		friend inline std::ostream& operator<<(std::ostream& os, type const& obj) { os << obj.ToString(); return os; }; \
		friend inline std::stringstream& operator>>(std::stringstream& os, type& obj) { double v = 0; os >> v; obj = v; return os; }; \
	};
#define CalculateMetricPrefixV(metric) ((double)std::metric::num / (double)std::metric::den)
#define DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) DerivedUnitType(prefix ## type, category, prefix_abbrev ## abbreviation, (ratio) * CalculateMetricPrefixV(prefix))
#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio)\
	DerivedUnitType(type, category, abbreviation, (ratio));\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, pico, p);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, nano, n);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, micro, u);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, milli, m);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, centi, c);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deci, d);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deca, da);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, hecto, h);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, kilo, k);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, mega, M);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, giga, G);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, tera, T);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, peta, P)

	template <typename Derived> constexpr INLINE double Conversion(double X) { return Derived::conversion() * X; };
	constexpr INLINE double SQUARED(double X) { return X * X; };
	constexpr INLINE double CUBED(double X) { return X * X * X; };

	/* LENGTH DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0);
	DerivedUnitType(foot, length, ft, 381.0 / 1250.0);
	DerivedUnitType(inch, length, in, Conversion<foot>(1.0 / 12.0));
	DerivedUnitType(mile, length, mi, Conversion<foot>(5280.0 / 1.0));
	DerivedUnitType(nauticalMile, length, nmi, Conversion<meter>(1852.0));
	DerivedUnitType(astronicalUnit, length, au, Conversion<meter>(149597870700.0));
	DerivedUnitType(yard, length, yd, Conversion<foot>(3.0));

	/* MASS DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0);
	DerivedUnitType(metric_ton, mass, t, Conversion<kilogram>(1000.0));
	DerivedUnitType(pound, mass, lb, Conversion<kilogram>(45359237.0 / 100000000.0));
	DerivedUnitType(long_ton, mass, ln_t, Conversion < pound>(2240.0));
	DerivedUnitType(short_ton, mass, sh_t, Conversion < pound>(2000.0));
	DerivedUnitType(stone, mass, st, Conversion < pound>(14.0));
	DerivedUnitType(ounce, mass, oz, Conversion < pound>(1.0 / 16.0));
	DerivedUnitType(carat, mass, ct, Conversion < milligram>(200.0));
	DerivedUnitType(slug, mass, slug, Conversion<kilogram>(145939029.0 / 10000000.0));

	/* TIME DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(second, time, s, 1.0);
	DerivedUnitType(minute, time, min, Conversion<second>(60.0));
	DerivedUnitType(hour, time, hr, Conversion<minute>(60.0));
	DerivedUnitType(day, time, d, Conversion<hour>(24.0));
	DerivedUnitType(week, time, wk, Conversion<day>(7.0));
	DerivedUnitType(year, time, yr, Conversion<day>(365));
	DerivedUnitType(julian_year, time, a_j, Conversion<second>(31557600.0));
	DerivedUnitType(gregorian_year, time, a_g, Conversion<second>(31556952.0));

	/* CURRENT DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(ampere, current, A, 1.0);

	/* DOLLAR DERIVATIONS */
	DerivedUnitType(Dollar, dollar, USD, 1.0);
	DerivedUnitType(MillionDollar, dollar, MUSD, Conversion<Dollar>(1000000.0));
	 
	/* FREQUENCY DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(hertz, frequency, Hz, 1.0);

	/* VELOCITY DERIVATIONS */
	DerivedUnitType(meters_per_second, velocity, mps, Conversion<meter>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(feet_per_second, velocity, fps, Conversion<foot>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(feet_per_minute, velocity, fpm, Conversion<foot>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(feet_per_hour, velocity, fph, Conversion<foot>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(miles_per_hour, velocity, mph, Conversion<mile>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(kilometers_per_hour, velocity, kph, Conversion<kilometer>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(knot, velocity, kts, Conversion<nauticalMile>(1.0) / Conversion<hour>(1.0));

	/* ACCELERATION DERIVATIONS */
	DerivedUnitType(meters_per_second_squared, acceleration, mps_sq, Conversion<meter>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
	DerivedUnitType(feet_per_second_squared, acceleration, fps_sq, Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
	DerivedUnitType(standard_gravity, acceleration, SG, Conversion<meters_per_second_squared>(980665.0 / 100000.0));

	// FORCE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(newton, force, N, Conversion<kilogram>(1.0)* Conversion<meters_per_second_squared>(1.0));
	DerivedUnitTypeWithMetricPrefixes(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0));
	DerivedUnitType(dyne, force, dyn, Conversion <newton>(1.0 / 100000.0));
	DerivedUnitType(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0));
	DerivedUnitType(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));

	// PRESSURE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(pascals, pressure, Pa, 1.0);
	DerivedUnitTypeWithMetricPrefixes(bar, pressure, bar, Conversion<kilopascals>(100.0));
	DerivedUnitType(mbar, pressure, mbar, Conversion<millibar>(1.0));
	DerivedUnitType(atmosphere, pressure, atm, Conversion<pascals>(101325.0));
	DerivedUnitType(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0)));
	DerivedUnitType(head, pressure, ft_water, Conversion<pounds_per_square_inch>(100.0 / 231.0));
	DerivedUnitType(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0));

	// CHARGE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(coulomb, charge, C, 1.0);
	DerivedUnitTypeWithMetricPrefixes(ampere_hour, charge, Ah, Conversion< ampere>(1.0)* Conversion<hour>(1.0));

	// POWER DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(watt, power, W, 1.0);
	DerivedUnitType(horsepower, power, hp, Conversion<watt>(7457.0 / 10.0));

	// ENERGY DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(joule, energy, J, 1.0);
	DerivedUnitTypeWithMetricPrefixes(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0));
	DerivedUnitTypeWithMetricPrefixes(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0));
	DerivedUnitTypeWithMetricPrefixes(watt_hour, energy, Wh, Conversion<watt>(1.0)* Conversion<hour>(1.0));
	DerivedUnitType(watt_day, energy, Wd, Conversion<watt>(1.0)* Conversion<day>(1.0));
	DerivedUnitType(british_thermal_unit, energy, BTU, Conversion<joule>(105505585262.0 / 100000000.0));
	DerivedUnitType(british_thermal_unit_iso, energy, BTU_iso, Conversion<joule>(1055056.0 / 1000.0));
	DerivedUnitType(british_thermal_unit_59, energy, BTU59, Conversion<joule>(1054804.0 / 1000.0));
	DerivedUnitType(therm, energy, thm, Conversion<british_thermal_unit_59>(100000.0));
	DerivedUnitType(foot_pound, energy, ftlbf, Conversion<joule>(13558179483314004.0 / 10000000000000000.0));

	// VOLTAGE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(volt, voltage, V, 1.0);
	//DerivedUnitType(statvolt, voltage, statV, Conversion<volt>(1000000.0 / 299792458.0));
	//DerivedUnitType(abvolt, voltage, abV, Conversion<volt>(1.0 / 100000000.0));

	// CAPACITANCE DERIVATIONS
	// DerivedUnitTypeWithMetricPrefixes(farad, capacitance, F, 1.0);

	// IMPEDANCE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(ohm, impedance, Ohm, 1.0);

	// CONDUCTANCE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(siemens, conductance, S, 1.0);

	// MAGNETIC FLUX
	//DerivedUnitTypeWithMetricPrefixes(weber, magnetic_flux, Wb, 1.0);
	//DerivedUnitType(maxwell, magnetic_flux, Mx, Conversion<weber>(1.0 / 100000000.0));

	// INDUCTANCE DERIVATIONS
	// DerivedUnitTypeWithMetricPrefixes(henry, inductance, H, 1.0);

	// TORQUE DERIVATIONS
	// DerivedUnitType(newton_meter, torque, Nm, 1.0);
	// DerivedUnitType(foot_pound, torque, ftlb, Conversion<foot>(1.0) * Conversion<pound_f>(1.0));
	// DerivedUnitType(foot_poundal, torque, ftpdl, Conversion<foot>(1.0)* Conversion<poundal>(1.0));
	// DerivedUnitType(inch_pound, torque, inlb, Conversion<inch>(1.0)* Conversion<pound_f>(1.0));
	// DerivedUnitType(meter_kilogram, torque, mkgf, Conversion<foot>(1.0)* Conversion<kilopond>(1.0));

	// AREA DERIVATIONS
	DerivedUnitType(square_meter, area, sq_m, 1.0);
	DerivedUnitType(square_foot, area, sq_ft, Conversion<foot>(1.0)* Conversion<foot>(1.0));
	DerivedUnitType(square_inch, area, sq_in, Conversion<inch>(1.0)* Conversion<inch>(1.0));
	DerivedUnitType(square_mile, area, sq_mi, Conversion<mile>(1.0)* Conversion<mile>(1.0));
	DerivedUnitType(square_kilometer, area, sq_km, Conversion<kilometer>(1.0)* Conversion<kilometer>(1.0));
	DerivedUnitType(hectare, area, ha, Conversion<square_meter>(1000.0));
	DerivedUnitType(acre, area, acre, Conversion<square_foot>(43560.0));

	// VOLUME DERIVATIONS
	DerivedUnitType(cubic_meter, volume, cu_m, 1.0);
	DerivedUnitType(cubic_millimeter, volume, cu_mm, CUBED(Conversion<millimeter>(1.0)));
	DerivedUnitType(cubic_kilometer, volume, cu_km, CUBED(Conversion<kilometer>(1.0)));
	DerivedUnitTypeWithMetricPrefixes(liter, volume, L, CUBED(Conversion<decimeter>(1.0)));
	DerivedUnitType(cubic_inch, volume, cu_in, CUBED(Conversion<inch>(1.0)));
	DerivedUnitType(cubic_foot, volume, cu_ft, CUBED(Conversion<foot>(1.0)));
	DerivedUnitType(cubic_yard, volume, cu_yd, CUBED(Conversion<yard>(1.0)));
	DerivedUnitType(cubic_mile, volume, cu_mi, CUBED(Conversion<mile>(1.0)));
	DerivedUnitTypeWithMetricPrefixes(gallon, volume, gal, Conversion<cubic_inch>(231.0));
	DerivedUnitType(imperial_gallon, volume, igal, Conversion<gallon>(10.0 / 12.0));
	DerivedUnitType(million_gallon, volume, MG, Conversion<gallon>(1.0)* CalculateMetricPrefixV(mega));
	DerivedUnitType(imperial_million_gallon, volume, IMG, Conversion<imperial_gallon>(1.0)* CalculateMetricPrefixV(mega));
	DerivedUnitType(acre_foot, volume, ac_ft, Conversion<acre>(1.0)* Conversion<foot>(1.0));
	DerivedUnitType(quart, volume, qt, Conversion<gallon>(0.25));
	DerivedUnitType(pint, volume, pt, Conversion<quart>(0.5));
	DerivedUnitType(cup, volume, c, Conversion<pint>(0.5));
	DerivedUnitType(fluid_ounce, volume, fl_oz, Conversion<cup>(0.125));
	DerivedUnitType(barrel, volume, bl, Conversion<gallon>(42.0));
	DerivedUnitType(bushel, volume, bu, Conversion<cubic_inch>(215042.0 / 100.0));
	DerivedUnitType(cord, volume, cord, Conversion<cubic_foot>(128.0));
	DerivedUnitType(tablespoon, volume, tbsp, Conversion<fluid_ounce>(0.5));
	DerivedUnitType(teaspoon, volume, tsp, Conversion<fluid_ounce>(1.0 / 6.0));
	DerivedUnitType(pinch, volume, pinch, Conversion<teaspoon>(1.0 / 8.0));
	DerivedUnitType(dash, volume, dash, Conversion<pinch>(1.0 / 2.0));
	DerivedUnitType(drop, volume, drop, Conversion<fluid_ounce>(1.0 / 360.0));
	DerivedUnitType(fifth, volume, fifth, Conversion<gallon>(0.2));
	DerivedUnitType(dram, volume, dr, Conversion<fluid_ounce>(0.125));
	DerivedUnitType(gill, volume, gi, Conversion<fluid_ounce>(4.0));
	DerivedUnitType(peck, volume, pk, Conversion<bushel>(0.25));
	DerivedUnitType(sack, volume, sacks, Conversion<bushel>(3.0));
	DerivedUnitType(shot, volume, shots, Conversion<fluid_ounce>(3.0 / 2.0));
	DerivedUnitType(strike, volume, strikes, Conversion<bushel>(2.0));

	// FILLRATE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0);
	DerivedUnitType(metric_ton_per_second, fillrate, mTs, Conversion<metric_ton>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(metric_ton_per_minute, fillrate, mTm, Conversion<metric_ton>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(metric_ton_per_hour, fillrate, mTh, Conversion<metric_ton>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(metric_ton_per_day, fillrate, mTd, Conversion<metric_ton>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(metric_ton_per_year, fillrate, mTy, Conversion<metric_ton>(1.0) / Conversion<year>(1.0));

	// FLOWRATE DERIVATIONS
	DerivedUnitType(cubic_meter_per_second, flowrate, cms, 1.0);
	DerivedUnitType(cubic_meter_per_hour, flowrate, cmh, Conversion<cubic_meter>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(cubic_meter_per_day, flowrate, cmd, Conversion<cubic_meter>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(cubic_millimeter_per_second, flowrate, cmms, Conversion<cubic_millimeter>(1.0) / Conversion<second>(1.0));
	DerivedUnitTypeWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion<liter>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(liter_per_minute, flowrate, lpm, Conversion<liter>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(liter_per_day, flowrate, lpd, Conversion<liter>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(megaliter_per_day, flowrate, Mlpd, Conversion<megaliter>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(cubic_inch_per_second, flowrate, cis, Conversion<cubic_inch>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(cubic_inch_per_hour, flowrate, cih, Conversion<cubic_inch>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(cubic_foot_per_second, flowrate, cfs, Conversion<cubic_foot>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(cubic_foot_per_hour, flowrate, cfh, Conversion<cubic_foot>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(gallon_per_second, flowrate, gps, Conversion<gallon>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(gallon_per_minute, flowrate, gpm, Conversion<gallon>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(gallon_per_hour, flowrate, gph, Conversion<gallon>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(gallon_per_day, flowrate, gpd, Conversion<gallon>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(gallon_per_year, flowrate, gpy, Conversion<gallon>(1.0) / Conversion<year>(1.0));
	DerivedUnitType(million_gallon_per_second, flowrate, MGS, Conversion<megagallon>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(million_gallon_per_minute, flowrate, MGM, Conversion<megagallon>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(million_gallon_per_hour, flowrate, MGH, Conversion<megagallon>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(million_gallon_per_day, flowrate, MGD, Conversion<megagallon>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(million_gallon_per_year, flowrate, MGY, Conversion<megagallon>(1.0) / Conversion<year>(1.0));
	DerivedUnitType(imperial_million_gallon_per_second, flowrate, IMGS, Conversion<imperial_million_gallon>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion<imperial_million_gallon>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion<imperial_million_gallon>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(imperial_million_gallon_per_day, flowrate, IMGD, Conversion<imperial_million_gallon>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(imperial_million_gallon_per_year, flowrate, IMGY, Conversion<imperial_million_gallon>(1.0) / Conversion<year>(1.0));
	DerivedUnitType(acre_foot_per_second, flowrate, ac_ft_s, Conversion<acre_foot>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(acre_foot_per_minute, flowrate, ac_ft_m, Conversion<acre_foot>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(acre_foot_per_hour, flowrate, ac_ft_h, Conversion<acre_foot>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(acre_foot_per_day, flowrate, ac_ft_d, Conversion<acre_foot>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(acre_foot_per_year, flowrate, ac_ft_y, Conversion<acre_foot>(1.0) / Conversion<year>(1.0));

	// DENSITY DERIVATIONS
	DerivedUnitType(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0);
	DerivedUnitType(grams_per_milliliter, density, g_per_mL, Conversion<gram>(1.0) / Conversion<milliliter>(1.0));
	DerivedUnitType(kilograms_per_liter, density, kg_per_L, Conversion<kilogram>(1.0) / Conversion<liter>(1.0));
	DerivedUnitType(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion<ounce>(1.0) / Conversion<cubic_foot>(1.0));
	DerivedUnitType(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion<ounce>(1.0) / Conversion<cubic_inch>(1.0));
	DerivedUnitType(ounces_per_gallon, density, oz_per_gal, Conversion<ounce>(1.0) / Conversion<gallon>(1.0));
	DerivedUnitType(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion<pound>(1.0) / Conversion<cubic_foot>(1.0));
	DerivedUnitType(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion<pound>(1.0) / Conversion<cubic_inch>(1.0));
	DerivedUnitType(pounds_per_gallon, density, lb_per_gal, Conversion<pound>(1.0) / Conversion<gallon>(1.0));
	DerivedUnitType(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion<slug>(1.0) / Conversion<cubic_foot>(1.0));

	// DOLLAR RATES DERIVATIONS
	DerivedUnitType(Dollar_per_joule, energy_cost_rate, USD_per_j, Conversion<Dollar>(1.0) / Conversion<joule>(1.0));
	DerivedUnitType(Dollar_per_kilowatt_hour, energy_cost_rate, USD_per_kWh, Conversion<Dollar>(1.0) / Conversion<kilowatt_hour>(1.0));
	DerivedUnitType(Dollar_per_watt, power_cost_rate, USD_per_w, Conversion<Dollar>(1.0) / Conversion<watt>(1.0));
	DerivedUnitType(Dollar_per_kilowatt, power_cost_rate, USD_per_kW, Conversion<Dollar>(1.0) / Conversion<kilowatt>(1.0));
	DerivedUnitType(Dollar_per_cubic_meter, volume_cost_rate, USD_per_cm, Conversion<Dollar>(1.0) / Conversion<cubic_meter>(1.0));
	DerivedUnitType(Dollar_per_gallon, volume_cost_rate, USD_per_gal, Conversion<Dollar>(1.0) / Conversion<gallon>(1.0));

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef CalculateMetricPrefixV
#undef DerivedUnitType

	namespace  cweeUnitValuesDetail {
		namespace cweeUnitValuesDetails {
			template <typename Key, typename Value, std::size_t Size> struct Map {
				std::array<std::pair<Key, Value>, Size> data;

				[[nodiscard]] constexpr Value at(const Key& key) const {
					const auto itr =
						std::find_if(begin(data), end(data),
							[&key](const auto& v) { return v.first == key; });
					if (itr != end(data)) {
						return itr->second;
					}
					else {
						throw std::range_error("Not Found");
					}
				}

			};
			using namespace std::literals::string_view_literals;
			
			//template <typename T> static constexpr std::pair<unsigned long long, const char*> CreateRow() {
			//	constexpr std::pair<unsigned long long, const char*> out { HashUnitAndRatio(HashUnits(T::A(), T::B(), T::C(), T::D(), T::E()), T::conversion()), T::specialized_abbreviation() };
			//	return out;
			//};

#define CreateRow(Type) \
			{ HashUnitAndRatio(HashUnits(Type::A(), Type::B(), Type::C(), Type::D(), Type::E()), Type::conversion()), Type::specialized_abbreviation() }

#define CreateRowWithMetricPrefixes(Type)\
			CreateRow(Type), \
			CreateRow(femto ## Type), \
			CreateRow(pico ## Type), \
			CreateRow(nano ## Type), \
			CreateRow(micro ## Type), \
			CreateRow(milli ## Type), \
			CreateRow(centi ## Type), \
			CreateRow(deci ## Type), \
			CreateRow(deca ## Type), \
			CreateRow(hecto ## Type), \
			CreateRow(kilo ## Type), \
			CreateRow(mega ## Type), \
			CreateRow(giga ## Type), \
			CreateRow(tera ## Type), \
			CreateRow(peta ## Type)
			
			static constexpr std::array<std::pair<unsigned long long, const char*>, 552> unit_types{
				{
					CreateRowWithMetricPrefixes(meter),
					CreateRow(foot),
					CreateRow(inch),
					CreateRow(mile),
					CreateRow(nauticalMile),
					CreateRow(astronicalUnit),
					CreateRow(yard),
					CreateRowWithMetricPrefixes(gram),
					CreateRow(metric_ton),
					CreateRow(pound),
					CreateRow(long_ton),
					CreateRow(short_ton),
					CreateRow(stone),
					CreateRow(ounce),
					CreateRow(carat),
					CreateRow(slug),
					CreateRowWithMetricPrefixes(second),
					CreateRow(minute),
					CreateRow(hour),
					CreateRow(day),
					CreateRow(week),
					CreateRow(year),
					CreateRow(julian_year),
					CreateRow(gregorian_year),
					CreateRowWithMetricPrefixes(ampere),
					CreateRow(Dollar),
					CreateRow(MillionDollar),
					CreateRowWithMetricPrefixes(hertz),
					CreateRow(meters_per_second),
					CreateRow(feet_per_second),
					CreateRow(feet_per_minute),
					CreateRow(feet_per_hour),
					CreateRow(miles_per_hour),
					CreateRow(kilometers_per_hour),
					CreateRow(knot),
					CreateRow(meters_per_second_squared),
					CreateRow(feet_per_second_squared),
					CreateRow(standard_gravity),
					CreateRowWithMetricPrefixes(newton),
					CreateRowWithMetricPrefixes(pound_f),
					CreateRow(dyne),
					CreateRow(kilopond),
					CreateRow(poundal),
					CreateRowWithMetricPrefixes(pascals),
					CreateRowWithMetricPrefixes(bar),
					CreateRow(mbar),
					CreateRow(atmosphere),
					CreateRow(pounds_per_square_inch),
					CreateRow(head),
					CreateRow(torr),
					CreateRowWithMetricPrefixes(coulomb),
					CreateRowWithMetricPrefixes(ampere_hour),
					CreateRowWithMetricPrefixes(watt),
					CreateRow(horsepower),
					CreateRowWithMetricPrefixes(joule),
					CreateRowWithMetricPrefixes(calorie),
					CreateRowWithMetricPrefixes(watt_minute),
					CreateRowWithMetricPrefixes(watt_hour),
					CreateRow(watt_day),
					CreateRow(british_thermal_unit),
					CreateRow(british_thermal_unit_iso),
					CreateRow(british_thermal_unit_59),
					CreateRow(therm),
					CreateRow(foot_pound),
					CreateRowWithMetricPrefixes(volt),
					CreateRowWithMetricPrefixes(ohm),
					CreateRowWithMetricPrefixes(siemens),
					CreateRow(square_meter),
					CreateRow(square_foot),
					CreateRow(square_inch),
					CreateRow(square_mile),
					CreateRow(square_kilometer),
					CreateRow(hectare),
					CreateRow(acre),
					CreateRow(cubic_meter),
					CreateRow(cubic_millimeter),
					CreateRow(cubic_kilometer),
					CreateRowWithMetricPrefixes(liter),
					CreateRow(cubic_inch),
					CreateRow(cubic_foot),
					CreateRow(cubic_yard),
					CreateRow(cubic_mile),
					CreateRowWithMetricPrefixes(gallon),
					CreateRow(imperial_gallon),
					CreateRow(million_gallon),
					CreateRow(imperial_million_gallon),
					CreateRow(acre_foot),
					CreateRow(quart),
					CreateRow(pint),
					CreateRow(cup),
					CreateRow(fluid_ounce),
					CreateRow(barrel),
					CreateRow(bushel),
					CreateRow(cord),
					CreateRow(tablespoon),
					CreateRow(teaspoon),
					CreateRow(pinch),
					CreateRow(dash),
					CreateRow(drop),
					CreateRow(fifth),
					CreateRow(dram),
					CreateRow(gill),
					CreateRow(peck),
					CreateRow(sack),
					CreateRow(shot),
					CreateRow(strike),
					CreateRowWithMetricPrefixes(gram_per_second),
					CreateRow(metric_ton_per_second),
					CreateRow(metric_ton_per_minute),
					CreateRow(metric_ton_per_hour),
					CreateRow(metric_ton_per_day),
					CreateRow(metric_ton_per_year),
					CreateRow(cubic_meter_per_second),
					CreateRow(cubic_meter_per_hour),
					CreateRow(cubic_meter_per_day),
					CreateRow(cubic_millimeter_per_second),
					CreateRowWithMetricPrefixes(liter_per_second),
					CreateRow(liter_per_minute),
					CreateRow(liter_per_day),
					CreateRow(megaliter_per_day),
					CreateRow(cubic_inch_per_second),
					CreateRow(cubic_inch_per_hour),
					CreateRow(cubic_foot_per_second),
					CreateRow(cubic_foot_per_hour),
					CreateRow(gallon_per_second),
					CreateRow(gallon_per_minute),
					CreateRow(gallon_per_hour),
					CreateRow(gallon_per_day),
					CreateRow(gallon_per_year),
					CreateRow(million_gallon_per_second),
					CreateRow(million_gallon_per_minute),
					CreateRow(million_gallon_per_hour),
					CreateRow(million_gallon_per_day),
					CreateRow(million_gallon_per_year),
					CreateRow(imperial_million_gallon_per_second),
					CreateRow(imperial_million_gallon_per_minute),
					CreateRow(imperial_million_gallon_per_hour),
					CreateRow(imperial_million_gallon_per_day),
					CreateRow(imperial_million_gallon_per_year),
					CreateRow(acre_foot_per_second),
					CreateRow(acre_foot_per_minute),
					CreateRow(acre_foot_per_hour),
					CreateRow(acre_foot_per_day),
					CreateRow(acre_foot_per_year),
					CreateRow(kilograms_per_cubic_meter),
					CreateRow(grams_per_milliliter),
					CreateRow(kilograms_per_liter),
					CreateRow(ounces_per_cubic_foot),
					CreateRow(ounces_per_cubic_inch),
					CreateRow(ounces_per_gallon),
					CreateRow(pounds_per_cubic_foot),
					CreateRow(pounds_per_cubic_inch),
					CreateRow(pounds_per_gallon),
					CreateRow(slugs_per_cubic_foot),
					CreateRow(Dollar_per_joule),
					CreateRow(Dollar_per_kilowatt_hour),
					CreateRow(Dollar_per_watt),
					CreateRow(Dollar_per_kilowatt),
					CreateRow(Dollar_per_cubic_meter),
					CreateRow(Dollar_per_gallon)
				}
			};

#undef CreateRowWithMetricPrefixes
#undef CreateRow
		}
		INLINE const char* lookup_abbreviation(unsigned long long ull) {
			using namespace cweeUnitValuesDetails;
			static constexpr auto map = Map<unsigned long long, const char*, unit_types.size()>{ {unit_types} };
			try {
				return map.at(ull);
			}
			catch (std::range_error) {
				return "";
			}
		}
	};
};

namespace cweeUnitValues {
	INLINE void UnitsExample() {
		meters_per_second mps = 100;
		miles_per_hour mph = mps;
		AUTO h = hour(50);
		AUTO distanceTravelled = mph * h;
		foot distanceTraveled = mph * hour(50);
		std::cout << (distanceTravelled == distanceTraveled) << std::endl;

		std::cout << distanceTraveled << std::endl;
		unit_value a = distanceTraveled;
		std::cout << a << std::endl;

		Dollar_per_gallon dpg = 10.0;
		Dollar cost = dpg * inch(1) * inch(1) * inch(1);

		// CAUSE AN ERROR ON PURPOSE:
		feet_per_minute fpm = foot(1) / second(60);
		std::cout << fpm << std::endl;
		fpm *= 10;
		std::cout << fpm << std::endl;
		try {
			//fpm += cost; // non-sense and should throw
			//std::cout << fpm << std::endl;
		}
		catch (...) {
			std::cout << "CAUGHT ERROR FROM ADDITION IN UNITED VALUES" << std::endl;
		}


		meters_per_second_squared mps2 = mps / hour(50);





		unit_value b;
		b = a;
		scalar c;
		c = a;
		a = c;

		foot d;
		d = c;
		d = a;


		meter Len1 = 100.0;
		foot Len2 = 100;
		inch Len3 = cweeMath::PI;

		AUTO total_length = Len1 + Len2 + Len3;

		foot totalLen;
		totalLen = foot(total_length);
		totalLen = (foot)total_length;
		totalLen = total_length;





	};
};