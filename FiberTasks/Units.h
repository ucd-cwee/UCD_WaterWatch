#pragma once

#include "Fibers.h"

#define DefineCategoryType(type, a, b, c, d, e) class type : public value { public: \
	type() noexcept : value(AtomicUnitStruct(a, b, c, d, e, false, "", 1.0, 0.0)) {}; \
	type(double V) noexcept : value(AtomicUnitStruct(a, b, c, d, e, false, "", 1.0, V)) {}; \
	type(double V, const char* abbreviation) noexcept : value(AtomicUnitStruct(a, b, c, d, e, false, abbreviation, 1.0, V)) {}; \
	type(double V, const char* abbreviation, double ratio) noexcept : value(AtomicUnitStruct(a, b, c, d, e, false, abbreviation, ratio, V)) {}; \
    type(value const& V) noexcept = delete; \
    virtual ~type() {}; \
	friend inline std::ostream& operator<<(std::ostream& os, type const& obj) { os << obj.ToString(); return os; }; \
	friend inline std::stringstream& operator>>(std::stringstream& os, type& obj) { double v = 0; os >> v; obj = v; return os; }; \
	static constexpr size_t UnitHash() { return Units::HashUnits(a,b,c,d,e); } \
};
#define DefineCategoryStd(type, a, b, c, d, e) namespace std { template<> class numeric_limits<Units::type> { public: \
	static constexpr double min() { return std::numeric_limits<double>::min(); } \
	static constexpr double max() { return std::numeric_limits<double>::max(); } \
	static constexpr double lowest() { return std::numeric_limits<double>::lowest(); } \
	static constexpr bool is_integer = std::numeric_limits<double>::is_integer; \
	static constexpr bool is_signed = std::numeric_limits<double>::is_signed; }; \
};
#define DerivedUnitType(type, category, abbreviation, ratio) class type final : public category  { public: \
	constexpr static long double conversion() noexcept { return ratio; }; \
	constexpr static const char* specialized_abbreviation() noexcept { return #abbreviation; }; \
	constexpr static const char* specialized_name() noexcept { return #type; }; \
	type() noexcept : category(0.0, specialized_abbreviation(), ratio) {}; \
	type(double V) noexcept : category(V, specialized_abbreviation(), ratio) {}; \
	type(value const& other) : category(0.0, specialized_abbreviation(), ratio) { \
		this->unit_m.Update([V = other.unit_m.load(), this, &other](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct { \
			if (Data.IsSameCategory(V)) Data.value_m = V.value_m; \
			else if (V.IsScalar()) Data.value_m = (V.value_m / V.ratio_m) * ratio; \
			else if (Data.IsScalar()) Data = V; \
			else throw(std::runtime_error(Units::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", this->Abbreviation().c_str(), other.Abbreviation().c_str()))); \
            return Data; \
		}); \
    }; \
    virtual bool IsStaticType() const { return true; }; \
    virtual ~type() {}; \
	friend inline std::ostream& operator<<(std::ostream& os, type const& obj) { os << obj.ToString(); return os; }; \
	friend inline std::stringstream& operator>>(std::stringstream& os, type& obj) { double v = 0; os >> v; obj = v; return os; }; \
};
#define CalculateMetricPrefixV(metric) ((double)std::metric::num / (double)std::metric::den)
#define DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) DerivedUnitType(prefix ## type, category, prefix_abbrev ## abbreviation, ratio * CalculateMetricPrefixV(prefix))
#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio)\
	DerivedUnitType(type, category, abbreviation, ratio);\
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
#define DerivedUnitStd(type, category, abbreviation, ratio) namespace std { template<> class numeric_limits<Units::type> { public: \
	static constexpr double min() { return std::numeric_limits<double>::min(); } \
	static constexpr double max() { return std::numeric_limits<double>::max(); } \
	static constexpr double lowest() { return std::numeric_limits<double>::lowest(); } \
	static constexpr bool is_integer = std::numeric_limits<double>::is_integer; \
	static constexpr bool is_signed = std::numeric_limits<double>::is_signed; }; \
}; /* Unit Litersl (e.g. 1_ft, 1_gpm, etc.) */ namespace literals { \
	__forceinline auto operator""_ ## abbreviation (long double d) { return Units::type(static_cast<double>(d)); } \
	__forceinline auto operator""_ ## abbreviation (unsigned long long d) { return Units::type(static_cast<double>(d)); }\
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

#define FunctionNameTest(Name) \
        template<typename T, typename = void> struct has_ ## Name : std::false_type {}; \
		template<typename T> struct has_ ## Name<T, void_t<decltype(std::declval<T>(). ## Name() == true)>> : std::true_type {}

namespace unitTypes {
	BETTER_ENUM(units_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);
};

class Units {
public:
	template <typename Derived> static constexpr __forceinline long double Conversion(long double X) { return Derived::conversion() * X; };
	static constexpr __forceinline long double SQUARED(long double X) { return X * X; };
	static constexpr __forceinline long double CUBED(long double X) { return X * X * X; };

	static constexpr size_t HashUnits(double a, double b, double c, double d, double e) noexcept {
		constexpr double OFFSET = 1000;
		constexpr size_t A = 54059; /* a prime */
		constexpr double B = 76963; /* another prime */
		constexpr size_t C = 86969; /* yet another prime */
		constexpr size_t FIRSTH = 37; /* also prime */

		size_t h = FIRSTH;
		h = (h * A) ^ (size_t)((a + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((b + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((c + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((d + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((e + OFFSET) * B * 100.0);

		decltype(auto) result = h % C;
		return result;
	};
	static constexpr size_t HashUnitAndRatio(double unitHash, double ratio) noexcept {
		constexpr double OFFSETA = 10000;
		constexpr double OFFSETB = 1000;
		constexpr size_t A = 54059; /* a prime */
		constexpr double B = 76963; /* another prime */
		constexpr size_t C = 86969; /* yet another prime */
		constexpr size_t FIRSTH = 37; /* also prime */

		size_t h = FIRSTH;
		h = (h * A) ^ (size_t)((unitHash + OFFSETA) * B);
		h = (h * A) ^ (size_t)((ratio + OFFSETB) * B * 100000.0); // 10000000.0

		decltype(auto) result = h % C;
		return result;
	};

	static int INT32_SIGNBITNOTSET(int i) {
		int	r = ((~static_cast<const unsigned int>(i)) >> 31);
		assert(r == 0 || r == 1);
		return r;
	};
	static long long	StrCmp(const char* s1, const char* s2) {
		long long c1, c2, d;
		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			d = c1 - c2;
			if (d) {
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;		// strings are equal
	};
	static void AddToDelimiter(std::string& obj, std::string const& toAdd, std::string const& delim) {
		if (obj.length() == 0) {
			obj += toAdd;
		}
		else {
			obj += delim;
			obj += toAdd;
		}
	};
	static size_t		vsnPrintf(char* dest, size_t size, const char* fmt, va_list argptr) {
		size_t ret;
#undef _vsnprintf
		ret = ::_vsnprintf(dest, size - 1, fmt, argptr);
		dest[size - 1] = '\0';
		if (ret < 0 || ret >= size)  ret = -1;
		return ret;
	};
	static std::string	printf(const char* fmt, ...) {
		va_list argptr;

		decltype(auto) buffer = new char[128000];
		buffer[128000 - 1] = '\0';

		va_start(argptr, fmt);
		vsnPrintf(buffer, 128000 - 1 /*sizeof(buffer) - 1*/, fmt, argptr);
		va_end(argptr);
		buffer[128000 /*sizeof(buffer)*/ - 1] = '\0';

		std::string out(buffer);

		delete[] buffer;
		return out;
	};
	static bool IsInteger(double value) {
		double intpart;
		return modf(value, &intpart) == 0.0;
	};
	static void removeTrailingCharacters(std::string& str, const char charToRemove) {
		str.erase(str.find_last_not_of(charToRemove) + 1, std::string::npos);
	};
	
	// container for all of the information that defines a unit value, including the unit data as well as the underlying SI value.
	class AtomicUnitStruct {
	public:
		static constexpr size_t NumUnits{ unitTypes::units_type::_size_constant };
		template <typename T> static constexpr T abs(T x) { return x > (T)0 ? x : -x; };

	public:
		std::array< fibers::utilities::DoubleWrapper, NumUnits> unitType_m; // power exponents for the SI units (e.g. m^1 * kg^0 * s^-1 * A^0 * $^0 = m/s)
		fibers::utilities::DoubleWrapper ratio_m; // ratio multiplier for converting from the SI units to this actual unit (e.g. 1 = meters, 0.304 = feet, etc.) 
		fibers::utilities::DoubleWrapper value_m; // underlying value of the unit if represented as SI units. (e.g. will always be in meters, regardless of the actual unit being in feet)

		constexpr bool IsSI() const {
			return (abs((float)unitType_m[0]) + abs((float)unitType_m[1]) + abs((float)unitType_m[2]) + abs((float)unitType_m[3]) + abs((float)unitType_m[4])) == 1.0f
				&& abs((double)ratio_m) == 1.0;
		};
		constexpr bool IsScalar() const {
			return (abs((float)unitType_m[0]) + abs((float)unitType_m[1]) + abs((float)unitType_m[2]) + abs((float)unitType_m[3]) + abs((float)unitType_m[4])) == 0.0f
				&& abs((double)ratio_m) == 1.0;
		};
		constexpr static auto sizeOfUnits = sizeof(unitType_m);
	public:
		constexpr AtomicUnitStruct() : 
			unitType_m{ 0.f, 0.f, 0.f, 0.f, 0.f },
			ratio_m{ 1. },
			value_m{ 0. }
		{};
		constexpr AtomicUnitStruct(double V) :
			unitType_m{ 0.f, 0.f, 0.f, 0.f, 0.f },
			ratio_m{ 1. },
			value_m{ V }			
		{};
		constexpr AtomicUnitStruct(double a, double b, double c, double d, double e, bool isScalar_p, const char* abbreviation_p, double ratio_p, double value_p = 0.0) noexcept :
			unitType_m{ static_cast<float>(a), static_cast<float>(b), static_cast<float>(c), static_cast<float>(d), static_cast<float>(e) },
			ratio_m{ ratio_p },
			value_m{ value_p * ratio_p }			
		{};
		constexpr AtomicUnitStruct(AtomicUnitStruct const&) = default;
		constexpr AtomicUnitStruct(AtomicUnitStruct&&) = default;
		constexpr AtomicUnitStruct& operator=(AtomicUnitStruct const&) = default;
		constexpr AtomicUnitStruct& operator=(AtomicUnitStruct&&) = default;

	public:
		bool IsSameCategory(AtomicUnitStruct const& other) const noexcept {
			if (IsScalar() && other.IsScalar()) return true;
			return std::memcmp(&unitType_m, &other.unitType_m, sizeof(unitType_m)) == 0;
		};
		bool IsSameUnit(AtomicUnitStruct const& other) const noexcept {
			return IsSameCategory(other) && (ratio_m == other.ratio_m);
		};
		decltype(auto) HashCategory() const noexcept {
			return Units::HashUnits(unitType_m[0], unitType_m[1], unitType_m[2], unitType_m[3], unitType_m[4]);
		};
		std::pair<const char*, double> LookupAbbreviation(bool isStatic) const noexcept {
			double ratio_bestFit = (double)ratio_m;
			auto* abbrev_bestFit = Units::UnitsDetail::lookup_abbreviation(HashCategory(), ratio_bestFit);
			return { abbrev_bestFit, ratio_bestFit };
		};
		const char* LookupTypeName() const noexcept {
			double ratio_bestFit = (double)ratio_m;
			auto* TypeName_bestFit = Units::UnitsDetail::lookup_typename(HashCategory(), ratio_bestFit);
			return TypeName_bestFit;
		};
		std::string CreateAbbreviation(bool isStatic) const noexcept {
			if (IsScalar()) {
				return "";
			}
			else {
				std::string out;
				auto [abbreviation, ratio] = LookupAbbreviation(isStatic);
				out = abbreviation;
				if (!IsScalar() && out.empty()) {
					std::array< const char*, NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

					bool anyNegatives = false;
					for (int i = NumUnits - 1; i >= 0; i--) {
						decltype(auto) unitBase = unitBases[i];
						decltype(auto) v = unitType_m[i];

						if (v > 0) {
							if (v == 1)
								AddToDelimiter(out, unitBase, " ");
							else {
								std::string Num;
								if (IsInteger(v)) {
									Num = std::to_string((int)v);
								}
								else {
									Num = std::to_string((float)v);
									removeTrailingCharacters(Num, '0');
									removeTrailingCharacters(Num, '.');
								}
								AddToDelimiter(out, printf("%s^%s", unitBase, Num.c_str()), " ");
							}
						}
						else if (v < 0) {
							anyNegatives = true;
						}
					}
					if (anyNegatives) {
						out += " /";
						for (int i = NumUnits - 1; i >= 0; i--) {
							decltype(auto) unitBase = unitBases[i];
							decltype(auto) v = unitType_m[i];

							if (v < 0) {
								if (v == -1)
									AddToDelimiter(out, unitBase, " ");
								else {
									std::string Num;
									if (IsInteger(v)) {
										Num = std::to_string((int)(-1.0 * v));
									}
									else {
										Num = std::to_string((float)(-1.0 * v));
										removeTrailingCharacters(Num, '0');
										removeTrailingCharacters(Num, '.');
									}
									AddToDelimiter(out, printf("%s^%s", unitBase, Num.c_str()), " ");
								}
							}
						}
					}
				}
				return out;
			}
		};

	};

	class value {
	public:
		mutable fibers::utilities::CAS_Container<Units::AtomicUnitStruct> unit_m;

	public: // constructors
		value() : unit_m{ Units::AtomicUnitStruct{} } {};
		explicit value(Units::AtomicUnitStruct const& unit_p) : unit_m{ unit_p } {};
		explicit value(double V, Units::AtomicUnitStruct const& unit_p) :
			unit_m{ Units::AtomicUnitStruct((float)unit_p.unitType_m[0], (float)unit_p.unitType_m[1], (float)unit_p.unitType_m[2], (float)unit_p.unitType_m[3], (float)unit_p.unitType_m[4], false, ""/*unit_p.abbreviation_m*/, (double)unit_p.ratio_m, V)}
		{};
		explicit value(Units::AtomicUnitStruct && unit_p) : unit_m{ std::forward<Units::AtomicUnitStruct>(unit_p) } {};
		value(value&& V) : unit_m{ std::move(V.unit_m) } {};
		value(value const& V) : unit_m{ V.unit_m.load() } {};
		value(double V) : unit_m{ Units::AtomicUnitStruct(0,0,0,0,0,true,"",1,V) } {};
		virtual ~value() = default;

	protected:
		virtual bool IsStaticType() const { return false; };

	private:
		double GetVisibleValue() const noexcept {
			double out{ 0. };
			Abbreviation(&out);
			return out;
		};

		static std::string GetValueStr(value const& V) noexcept {
			auto v = V();
			if (std::fmod(v, 1.0) == 0.0) { // integer?
				return Units::printf("%lld", (long long)v);
			}
			else { // floating-point
				std::string out{ Units::printf("%.4f", v) };
				Units::removeTrailingCharacters(out, '0'); // e.g. 25.5000 -> 25.5
				Units::removeTrailingCharacters(out, '.'); // e.g. 25.0000 -> 25. -> 25
				return out;
			}
		};

		static bool IdenticalUnits(Units::AtomicUnitStruct const& LHS, Units::AtomicUnitStruct const& RHS) noexcept { return LHS.IsSameCategory(RHS); };
		static bool is_scalar(Units::AtomicUnitStruct const& V) noexcept { return V.IsScalar(); };

		static bool NormalArithmeticOkay(Units::AtomicUnitStruct const& LHS, Units::AtomicUnitStruct const& RHS) noexcept {
			if (is_scalar(LHS) || is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		static bool UnaryArithmeticOkay(Units::AtomicUnitStruct const& LHS, Units::AtomicUnitStruct const& RHS) noexcept {
			if (is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		static void HandleNormalArithmetic(Units::AtomicUnitStruct const& LHS, Units::AtomicUnitStruct const& RHS) {
			if (NormalArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(Units::printf("Normal, dynamic arithmetic failed due to incompatible non-scalar value: '%s' and '%s'", AbbreviationFast(LHS).c_str(), AbbreviationFast(RHS).c_str())));
		};
		static void HandleUnaryArithmetic(Units::AtomicUnitStruct const& LHS, Units::AtomicUnitStruct const& RHS) {
			if (UnaryArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(Units::printf("Unary (in-place or self-modifying) arithmetic failed due to incompatible value: '%s' and '%s'", AbbreviationFast(LHS).c_str(), AbbreviationFast(RHS).c_str())));
		};
		static void HandleNotScalar(Units::AtomicUnitStruct const& V) {
			if (is_scalar(V)) return;
			throw(std::runtime_error(Units::printf("Type must be scalar (was '%s').", AbbreviationFast(V).c_str())));
		};
		static std::string AbbreviationFast(Units::AtomicUnitStruct const& V) noexcept {
			std::string toReturn{ /*V.abbreviation_m*/ };
			
			if (V.IsScalar() && toReturn.empty()) {
				auto [abbreviation, ratio] = V.LookupAbbreviation(false);
				toReturn = abbreviation;
				if (!V.IsScalar() && toReturn.empty()) {
					constexpr static std::array< const char*, AtomicUnitStruct::NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

					bool anyNegatives = false;
					std::string Num;
					for (int i = AtomicUnitStruct::NumUnits - 1; i >= 0; i--) {
						decltype(auto) unitBase = unitBases[i];
						decltype(auto) v = V.unitType_m[i];

						if (v > 0) {
							if (v == 1)
								AddToDelimiter(toReturn, unitBase, " ");
							else {
								if (IsInteger(v)) {
									Num = std::to_string((int)v);
								}
								else {
									Num = std::to_string((float)v);
									removeTrailingCharacters(Num, '0');
									removeTrailingCharacters(Num, '.');
								}
								AddToDelimiter(toReturn, printf("%s^%s", unitBase, Num.c_str()), " ");
							}
						}
						else if (v < 0) {
							anyNegatives = true;
						}
					}
					if (anyNegatives) {
						toReturn += " /";
						for (int i = AtomicUnitStruct::NumUnits - 1; i >= 0; i--) {
							decltype(auto) unitBase = unitBases[i];
							decltype(auto) v = V.unitType_m[i];

							if (v < 0) {
								if (v == -1)
									AddToDelimiter(toReturn, unitBase, " ");
								else {
									if (IsInteger(v)) {
										Num = std::to_string((int)(-1.0 * v));
									}
									else {
										Num = std::to_string((float)(-1.0 * v));
										removeTrailingCharacters(Num, '0');
										removeTrailingCharacters(Num, '.');
									}
									AddToDelimiter(toReturn, printf("%s^%s", unitBase, Num.c_str()), " ");
								}
							}
						}
					}
				}
			}

			return toReturn;
		};

	public: // value operator
		explicit operator double() const noexcept { return GetVisibleValue(); };
		double operator()() const noexcept { return GetVisibleValue(); };

	public: // Functions
		const char* UnitName() const noexcept {
			const char* toReturn{ "" };
			unit_m.Update([&toReturn, this](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				auto [abbreviation, ratio] = Data.LookupAbbreviation(this->IsStaticType());
				//Data.abbreviation_m = const_cast<char*>(abbreviation);
				Data.ratio_m = ratio;

				toReturn = Data.LookupTypeName();

				return Data;
			});
			return toReturn;
		};
		void Clear() { 
			unit_m.store(Units::AtomicUnitStruct{}); 
		};
		void Swap(value const& other) const {
			unit_m.Swap(other.unit_m.load(), false); 
		};

	public:
		std::string Abbreviation(double* visibleValue = nullptr) const noexcept {
			bool isStatic{ IsStaticType() };
			std::string toReturn{ "" };

			{
				unit_m.Update([isStatic, &toReturn, this, &visibleValue](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
					if (Data.IsScalar()) {
						toReturn = "";
						if (visibleValue) {
							*visibleValue = (double)Data.value_m;
						}
					}
					else {
						auto [abbreviation, ratio] = Data.LookupAbbreviation(isStatic);
						//Data.abbreviation_m = const_cast<char*>(abbreviation);
						Data.ratio_m = ratio;
						toReturn = abbreviation;

						if (!Data.IsScalar() && toReturn.empty()) {
							// we failed to find this unit in the system -- it must be set to SI unit now. 
							Data.ratio_m = 1.0;

							constexpr static std::array< const char*, AtomicUnitStruct::NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

							bool anyNegatives = false;
							std::string Num;
							for (int i = AtomicUnitStruct::NumUnits - 1; i >= 0; i--) {
								const char* unitBase = unitBases[i];
								double v = (float)Data.unitType_m[i];

								if (v > 0) {
									if (v == 1)
										AddToDelimiter(toReturn, unitBase, " ");
									else {
										if (IsInteger(v)) {
											Num = std::to_string(static_cast<int>(v));
										}
										else {
											Num = std::to_string((float)v);
											removeTrailingCharacters(Num, '0');
											removeTrailingCharacters(Num, '.');
										}
										AddToDelimiter(toReturn, printf("%s^%s", unitBase, Num.c_str()), " ");
									}
								}
								else if (v < 0) {
									anyNegatives = true;
								}
							}
							if (anyNegatives) {
								toReturn += " /";
								for (int i = AtomicUnitStruct::NumUnits - 1; i >= 0; i--) {
									const char* unitBase = unitBases[i];
									double v = (float)Data.unitType_m[i];

									if (v < 0) {
										if (v == -1)
											AddToDelimiter(toReturn, unitBase, " ");
										else {
											if (IsInteger(v)) {
												Num = std::to_string(static_cast<int>(-1.0 * v));
											}
											else {
												Num = std::to_string((float)(-1.0 * v));
												removeTrailingCharacters(Num, '0');
												removeTrailingCharacters(Num, '.');
											}
											AddToDelimiter(toReturn, printf("%s^%s", unitBase, Num.c_str()), " ");
										}
									}
								}
							}
						}

						if (visibleValue) {
							*visibleValue = (double)Data.value_m / (double)Data.ratio_m;
						}
					}

					return Data;
					});
			}
			return toReturn;
		};
		std::string ToString() const {
			double out{0.0};
			std::string abbreviation{ Abbreviation(&out) };
			if (abbreviation.length() > 0) return GetValueStr(out) + " " + abbreviation;
			else return GetValueStr(out);
		};

	public: // Streaming functions (should be specialized per type)
		friend inline std::ostream& operator<<(std::ostream& os, value const& obj) { os << obj.ToString(); return os; };
		friend inline std::stringstream& operator>>(std::stringstream& os, value& obj) { double v = 0; os >> v; obj = v; return os; };

	private:
		/* Used for multiplication or division operations */
		template <bool multiplication = true> value& CompoundUnits(value const& other) noexcept {
			Units::AtomicUnitStruct V { other.unit_m.load() };

			unit_m.Update([V](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				bool V_Is_Scalar{ is_scalar(V) }, I_am_Scalar{ is_scalar(Data) };

				if (V_Is_Scalar) {
					if constexpr (multiplication) {
						Data.value_m *= V.value_m;
					}
					else {
						Data.value_m /= V.value_m;
					}
					return Data; // do nothing
				}

				// remove the abbreviation since we either don't know what we are or we will become empty anyhow.
				// Data.abbreviation_m = const_cast<char*>("");

				// V is not a scaler, but I could become one.
				//bool allZero = true;
				for (int i = Data.unitType_m.size() - 1; i >= 0; i--) {
					if constexpr (multiplication) {
						Data.unitType_m[i] += V.unitType_m[i];
					}
					else {
						Data.unitType_m[i] -= V.unitType_m[i];
					}
					//allZero = allZero && Data.unitType_m[i] == 0;
				}
				//if (allZero) { Data.IsScalar() = true; }
				//else { Data.IsScalar() = false; }

				// now that we have modified the unit type (length, time, etc.), the conversion ratio makes no sense anymore (e.g. within length, is it a foot, meter, yard, etc.)
				if constexpr (multiplication) {
					Data.ratio_m *= V.ratio_m;
				}
				else {
					Data.ratio_m /= V.ratio_m;
				}

				// unitless values cannot have "ratios" -- there are not alternatives of "unitless". 
				if (Data.IsScalar()) {
					Data.ratio_m = 1;
				}

				{
					if constexpr (multiplication) {
						Data.value_m *= V.value_m;
					}
					else {
						Data.value_m /= V.value_m;
					}
					return Data;
				}
			});

			return *this;
		};
		
		/* Used for exponential operations */
		value& MultiplyUnits(double const& V) noexcept {
			unit_m.Update([V](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				if (V == 1.0 || is_scalar(Data)) {

					Data.value_m = std::pow(Data.value_m / Data.ratio_m, V) * Data.ratio_m; // save in SI value

					return Data;
				}
				for (int i = Data.unitType_m.size() - 1; i >= 0; i--) Data.unitType_m[i] *= V;
				// if (V == 0) Data.IsScalar() = true;

				// remove the abbreviation since we either don't know what we are or we will become empty anyhow.
				// Data.abbreviation_m = const_cast<char*>("");

				// now that we have modified the value, the conversion ratio makes no sense anymore and must be reset. 
				Data.ratio_m = std::pow(Data.ratio_m, V);

				// do the exonentiation of the value
				// i.e. (10 (ft)) ^ (3) -> (1000 (cu_ft)) * (1 / 35.3147 (cu_m/cu_ft)) -> 28.3168 cu_m in SI value
				Data.value_m = std::pow(Data.value_m / Data.ratio_m, V) * Data.ratio_m; // save in SI value

				return Data;
			});
			return *this;
		};

	public: // = Operators
		value& operator=(value const& other) {
			Units::AtomicUnitStruct V{ other.unit_m.load() };

			unit_m.Update([V, this, &other](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				if (Data.IsSameCategory(V)) { // same category, but perhaps different conversion factor. That's OK. 
					Data.value_m = V.value_m;
				}
				else if (is_scalar(V)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					Data.value_m = (V.value_m / V.ratio_m) * Data.ratio_m;
				}
				else if (is_scalar(Data)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
					Data = V;
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					throw(std::runtime_error(Units::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", this->Abbreviation().c_str(), other.Abbreviation().c_str())));
				}
				return Data;
			});

			return *this;
		};

	public: // Comparison operators
		friend bool operator==(value const& A, value const& V) noexcept { 
			auto Data1{ A.unit_m.load() };
			auto Data2{ V.unit_m.load() };

			if (!NormalArithmeticOkay(Data1, Data2)) return false;

			if (is_scalar(Data1) == is_scalar(Data2)) {
				return Data1.value_m == Data2.value_m;
			} 
			else if (is_scalar(Data2)) {
				value W = A; W = V; return Data1.value_m == W.unit_m.load().value_m;
			} 
			else { 
				value W = V; W = A; return W.unit_m.load().value_m == Data2.value_m;
			} 
		};
		friend bool operator<(value const& A, value const& V) { 
			auto Data1{ A.unit_m.load() };
			auto Data2{ V.unit_m.load() };

			HandleNormalArithmetic(Data1, Data2);
			if (is_scalar(Data2) == is_scalar(Data1)) {
				return Data1.value_m < Data2.value_m;
			} 
			else if (is_scalar(Data2)) {
				value W = A; W = V; 
				return Data1.value_m < W.unit_m.load().value_m;
			} 
			else { 
				value W = V; W = A; 
				return W.unit_m.load().value_m < Data2.value_m;
			} 
		};
		friend bool operator<=(value const& A, value const& V) { 
			auto Data1{ A.unit_m.load() };
			auto Data2{ V.unit_m.load() };

			HandleNormalArithmetic(Data1, Data2);
			if (is_scalar(Data2) == is_scalar(Data1)) {
				return Data1.value_m <= Data2.value_m;
			} 
			else if (is_scalar(Data2)) {
				value W = A; W = V; 
				return Data1.value_m <= W.unit_m.load().value_m;
			} 
			else { 
				value W = V; W = A; 
				return W.unit_m.load().value_m <= Data2.value_m;
			} 
		};
		friend bool operator>(value const& A, value const& V) { return !(A <= V); };
		friend bool operator>=(value const& A, value const& V) { return !(A < V); };
		friend bool operator!=(value const& A, value const& V) noexcept { return !(operator==(A, V)); };

	public: // Unary operators
		value& operator++() { 
			unit_m.Update([](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m += (double)Data.ratio_m;
				return Data;
			});
			return *this;
		};
		value& operator--() {
			unit_m.Update([](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m -= (double)Data.ratio_m;
				return Data;
			});
			return *this;
		};
		value operator++(int) { 
			value out = *this; 
			unit_m.Update([](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m += (double)Data.ratio_m;
				return Data;
			});
			return out;
		};
		value operator--(int) { 
			value out = *this; 
			unit_m.Update([](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m -= (double)Data.ratio_m;
				return Data;
			});
			return out; 
		};

	private: 
		static value Add(value const& a, value const& b) {
			value out1 = a;
			value out2 = a; out2 = b;

			auto V{ out2.unit_m.load() };
			out1.unit_m.Update([V](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				HandleNormalArithmetic(Data, V);
				Data.value_m += V.value_m;
				return Data;
			});

			return out1;
		};
		static value Sub(value const& a, value const& b) {
			value out1 = a;
			value out2 = a; out2 = b;

			auto V{ out2.unit_m.load() };
			out1.unit_m.Update([V](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				HandleNormalArithmetic(Data, V);
				Data.value_m -= V.value_m;
				return Data;
			});

			return out1;
		};
		static value Multiply(value const& A, value const& V) {
			value out = A;
			out.CompoundUnits<true>(V);
			return out;
		};
		static value Divide(value const& A, value const& V) {
			value out = A;
			out.CompoundUnits<false>(V);
			return out;
		};

	public: // + and - Operators
		value operator-() const { return -1.0 * (*this); };

		friend value operator+(value const& A, value const& V) { return Add(A, V); };
		friend value operator-(value const& A, value const& V) { return Sub(A, V); };
		value& operator+=(value const& V) {
			unit_m.Update([other = V.unit_m.load()](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				if (Data.IsSameCategory(other)) { // same category, but perhaps different conversion factor. That's OK. 
					Data.value_m += other.value_m;
				}
				else if (is_scalar(other)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					Data.value_m += (other.value_m / other.ratio_m) * Data.ratio_m;
				}
				else if (is_scalar(Data)) { // I am a scaler but the incoming unit is not. Copy the incoming value's visible value.
					Data.value_m += (other.value_m / other.ratio_m);
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					HandleUnaryArithmetic(Data, other);
					value temp{ Data };
					temp = value(other);
					Data.value_m += temp.unit_m.load().value_m;
				}

				return Data;
			});
			return *this;
		};
		value& operator-=(value const& V) {
			unit_m.Update([other = V.unit_m.load()](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				if (Data.IsSameCategory(other)) { // same category, but perhaps different conversion factor. That's OK. 
					Data.value_m -= other.value_m;
				}
				else if (is_scalar(other)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					Data.value_m -= (other.value_m / other.ratio_m) * Data.ratio_m;
				}
				else if (is_scalar(Data)) { // I am a scaler but the incoming unit is not. Copy the incoming value's visible value.
					Data.value_m -= (other.value_m / other.ratio_m);
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					HandleUnaryArithmetic(Data, other);
					value temp{ Data };
					temp = value(other);
					Data.value_m -= temp.unit_m.load().value_m;
				}

				return Data;
			});
			return *this;
		};

	public: // * and / Operators
		friend value operator*(value const& A, value const& V) { return Multiply(A, V); };
		friend value operator/(value const& A, value const& V) { return Divide(A, V); };
		value& operator*=(value const& V) {
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([other](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m *= other.value_m;
				return Data;
			});
			return *this;
		};
		value& operator/=(value const& V) {
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([other](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m /= other.value_m;
				return Data;
			});
			return *this;
		};

	public: // pow and sqrt Operators
		// atomicly updates the value with a custom user-provided function.
		value& update(std::function<double(double)> updateFunction) {
			unit_m.Update([&updateFunction](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m = updateFunction(Data.value_m / Data.ratio_m) * Data.ratio_m;
				return Data;
			});
			return *this;
		};
		// Creats a copy of the value and updates it with a custom user-provided function.
		value update(std::function<double(double)> updateFunction) const {
			auto out{ value(*this) };
			out.update(updateFunction);
			return out;
		};
		// Returns a new value multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
		value pow(value const& V) const {
			auto other{ V.unit_m.load() };

			HandleNotScalar(other);

			value out = *this;
			out.MultiplyUnits(other.value_m);

			return out;
		};
		// atomicly updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
		value& pow_value(value const& V) { 
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([other](Units::AtomicUnitStruct Data)->Units::AtomicUnitStruct {
				Data.value_m = std::pow(Data.value_m / Data.ratio_m, other.value_m / other.ratio_m) * Data.ratio_m;
				return Data;
			});
			return *this; 
		};
		// pow(0.5)
		value sqrt() const { 
			return pow(0.5);
		};
		// atomicly floors (rounds to lower whole integer) the underlying value
		value& floor() { 
			return update([](double v)->double { return std::floor(v); });
		};
		// Creats a copy of the value and floors (rounds to lower whole integer) the underlying value
		value floor() const {
			auto out{ value(*this) };
			out.floor();
			return out;
		};
		// atomicly ceilings (rounds to upper whole integer) the underlying value
		value& ceiling() {
			return update([](double v)->double { return std::ceil(v); });
		}; 
		// Creats a copy of the value and ceilings (rounds to upper whole integer) the underlying value
		value ceiling() const {
			auto out{ value(*this) };
			out.ceiling();
			return out;
		};

	};
	using scalar = value;

	class traits {
		/* test if two unit types are convertable */
		template<class U1, class U2> struct is_convertible_unit_t {
			static constexpr const std::intmax_t value = (U1::UnitHash() == U2::UnitHash());
		};

		template<class U1> struct is_unit_t {
			static constexpr const std::intmax_t value = std::is_base_of<Units::value, U1>::value;
		};
	};

private:
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
	DefineCategoryType(energy_intensity, -1, 1, -2, 0, 1);
	DefineCategoryType(length_cost_rate, -1, 0, 0, 0, 1);
	DefineCategoryType(mass_cost_rate, 0, -1, 0, 0, 1);
	DefineCategoryType(emission_rate, -2, 0, 2, 0, 1);
	DefineCategoryType(time_rate, 0, 0, -1, 0, 1);

public:
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
	DerivedUnitType(month, time, mnth, Conversion<year>(1.0 / 12.0));
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
	DerivedUnitType(atmosphere, pressure, atm, Conversion<pascals>(101325.0));
	DerivedUnitType(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0)));
	DerivedUnitType(head, pressure, ft_water, Conversion<pound_f>(62.43) / (Conversion<foot>(1.0) * Conversion<foot>(1.0)));
	DerivedUnitType(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0));

	// CHARGE DERIVATIONS
	DerivedUnitType(coulomb, charge, C, 1.0); /* WithMetricPrefixes */
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

	// IMPEDANCE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(ohm, impedance, Ohm, 1.0);

	// CONDUCTANCE DERIVATIONS
	DerivedUnitType(siemens, conductance, S, 1.0); // WithMetricPrefixes

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

	// Rates
	DerivedUnitType(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion<kilowatt_hour>(1.0) / Conversion<acre_foot>(1.0));
	DerivedUnitType(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion<Dollar>(1.0) / Conversion<mile>(1.0));
	DerivedUnitType(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion<Dollar>(1.0) / Conversion<metric_ton>(1.0));
	DerivedUnitType(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion<metric_ton>(1.0) / Conversion<kilowatt_hour>(1.0));
	// DerivedUnitType(per_year, time_rate, p_yr, 1.0 / Conversion<year>(1.0));

	class UnitsDetail {
	public:
#define CreateRow(model, Type) { model->second[Type::UnitHash()].Insert(static_cast<uint64_t>(static_cast<long double>(Type::conversion()) * 1e8l), model->first.Alloc(std::pair< const char*, const char*>{ Type::specialized_abbreviation(), #Type }), false); }
#define CreateRowWithMetricPrefixes(model, Type)\
			CreateRow(model, Type); \
			CreateRow(model, femto ## Type); \
			CreateRow(model, pico ## Type); \
			CreateRow(model, nano ## Type); \
			CreateRow(model, micro ## Type); \
			CreateRow(model, milli ## Type); \
			CreateRow(model, centi ## Type); \
			CreateRow(model, deci ## Type); \
			CreateRow(model, deca ## Type); \
			CreateRow(model, hecto ## Type); \
			CreateRow(model, kilo ## Type); \
			CreateRow(model, mega ## Type); \
			CreateRow(model, giga ## Type); \
			CreateRow(model, tera ## Type); \
			CreateRow(model, peta ## Type);

		/*
		UnitHash determines the class of unit (length, time, length/time, length/time^2, length^1.25, etc.
		UnitRatio determines the specific ratio within that class (meter, foot, inch, etc.)
		*/
		static std::pair< const char*, const char*>& lookup_impl(size_t UnitHash, double& UnitRatio) noexcept {
			auto targetRatio = static_cast<uint64_t>(static_cast<long double>(UnitRatio) * 1e8l);

			static std::pair<const char*, const char*> out{ "","" };
			static std::mutex mut;
			static std::shared_ptr<void> Tag{ nullptr };

			using AllocType = fibers::utilities::Allocator<std::pair< const char*, const char*>>;
			using TreeType = fibers::containers::Pattern<uint64_t, std::pair< const char*, const char*>*>;
			using ModelType = std::pair< AllocType, std::map<size_t, TreeType>>;

			std::shared_ptr < ModelType > model;

			if (!Tag) {
				mut.lock();
			}
			if (!Tag) {
				model = std::make_shared<ModelType>();
#if 1
				CreateRowWithMetricPrefixes(model, meter);
				CreateRow(model, foot);
				CreateRow(model, inch);
				CreateRow(model, mile);
				CreateRow(model, nauticalMile);
				CreateRow(model, astronicalUnit);
				CreateRow(model, yard);
				CreateRowWithMetricPrefixes(model, gram);
				CreateRow(model, metric_ton);
				CreateRow(model, pound);
				CreateRow(model, long_ton);
				CreateRow(model, short_ton);
				CreateRow(model, stone);
				CreateRow(model, ounce);
				CreateRow(model, carat);
				CreateRow(model, slug);
				CreateRowWithMetricPrefixes(model, second);
				CreateRow(model, minute);
				CreateRow(model, hour);
				CreateRow(model, day);
				CreateRow(model, week);
				CreateRow(model, year);
				CreateRow(model, month);
				CreateRow(model, julian_year);
				CreateRow(model, gregorian_year);
				CreateRowWithMetricPrefixes(model, ampere);
				CreateRow(model, Dollar);
				CreateRow(model, MillionDollar);
				CreateRowWithMetricPrefixes(model, hertz);
				CreateRow(model, meters_per_second);
				CreateRow(model, feet_per_second);
				CreateRow(model, feet_per_minute);
				CreateRow(model, feet_per_hour);
				CreateRow(model, miles_per_hour);
				CreateRow(model, kilometers_per_hour);
				CreateRow(model, knot);
				CreateRow(model, meters_per_second_squared);
				CreateRow(model, feet_per_second_squared);
				CreateRow(model, standard_gravity);
				CreateRowWithMetricPrefixes(model, newton);
				CreateRowWithMetricPrefixes(model, pound_f);
				CreateRow(model, dyne);
				CreateRow(model, kilopond);
				CreateRow(model, poundal);
				CreateRowWithMetricPrefixes(model, pascals);
				CreateRowWithMetricPrefixes(model, bar);
				CreateRow(model, atmosphere);
				CreateRow(model, pounds_per_square_inch);
				CreateRow(model, head);
				CreateRow(model, torr);
				CreateRow(model, coulomb); // WithMetricPrefixes
				CreateRowWithMetricPrefixes(model, ampere_hour);
				CreateRowWithMetricPrefixes(model, watt);
				CreateRow(model, horsepower);
				CreateRowWithMetricPrefixes(model, joule);
				CreateRowWithMetricPrefixes(model, calorie);
				CreateRowWithMetricPrefixes(model, watt_minute);
				CreateRowWithMetricPrefixes(model, watt_hour);
				CreateRow(model, watt_day);
				CreateRow(model, british_thermal_unit);
				CreateRow(model, british_thermal_unit_iso);
				CreateRow(model, british_thermal_unit_59);
				CreateRow(model, therm);
				CreateRow(model, foot_pound);
				CreateRowWithMetricPrefixes(model, volt);
				CreateRowWithMetricPrefixes(model, ohm);
				CreateRow(model, siemens); // WithMetricPrefixes
				CreateRow(model, square_meter);
				CreateRow(model, square_foot);
				CreateRow(model, square_inch);
				CreateRow(model, square_mile);
				CreateRow(model, square_kilometer);
				CreateRow(model, hectare);
				CreateRow(model, acre);
				CreateRow(model, cubic_meter);
				CreateRow(model, cubic_millimeter);
				CreateRow(model, cubic_kilometer);
				CreateRowWithMetricPrefixes(model, liter);
				CreateRow(model, cubic_inch);
				CreateRow(model, cubic_foot);
				CreateRow(model, cubic_yard);
				CreateRow(model, cubic_mile);
				CreateRowWithMetricPrefixes(model, gallon);
				CreateRow(model, imperial_gallon);
				CreateRow(model, million_gallon);
				CreateRow(model, imperial_million_gallon);
				CreateRow(model, acre_foot);
				CreateRow(model, quart);
				CreateRow(model, pint);
				CreateRow(model, cup);
				CreateRow(model, fluid_ounce);
				CreateRow(model, barrel);
				CreateRow(model, bushel);
				CreateRow(model, cord);
				CreateRow(model, tablespoon);
				CreateRow(model, teaspoon);
				CreateRow(model, pinch);
				CreateRow(model, dash);
				CreateRow(model, drop);
				CreateRow(model, fifth);
				CreateRow(model, dram);
				CreateRow(model, gill);
				CreateRow(model, peck);
				CreateRow(model, sack);
				CreateRow(model, shot);
				CreateRow(model, strike);
				CreateRowWithMetricPrefixes(model, gram_per_second);
				CreateRow(model, metric_ton_per_second);
				CreateRow(model, metric_ton_per_minute);
				CreateRow(model, metric_ton_per_hour);
				CreateRow(model, metric_ton_per_day);
				CreateRow(model, metric_ton_per_year);
				CreateRow(model, cubic_meter_per_second);
				CreateRow(model, cubic_meter_per_hour);
				CreateRow(model, cubic_meter_per_day);
				CreateRow(model, cubic_millimeter_per_second);
				CreateRowWithMetricPrefixes(model, liter_per_second);
				CreateRow(model, liter_per_minute);
				CreateRow(model, liter_per_day);
				CreateRow(model, megaliter_per_day);
				CreateRow(model, cubic_inch_per_second);
				CreateRow(model, cubic_inch_per_hour);
				CreateRow(model, cubic_foot_per_second);
				CreateRow(model, cubic_foot_per_hour);
				CreateRow(model, gallon_per_second);
				CreateRow(model, gallon_per_minute);
				CreateRow(model, gallon_per_hour);
				CreateRow(model, gallon_per_day);
				CreateRow(model, gallon_per_year);
				CreateRow(model, million_gallon_per_second);
				CreateRow(model, million_gallon_per_minute);
				CreateRow(model, million_gallon_per_hour);
				CreateRow(model, million_gallon_per_day);
				CreateRow(model, million_gallon_per_year);
				CreateRow(model, imperial_million_gallon_per_second);
				CreateRow(model, imperial_million_gallon_per_minute);
				CreateRow(model, imperial_million_gallon_per_hour);
				CreateRow(model, imperial_million_gallon_per_day);
				CreateRow(model, imperial_million_gallon_per_year);
				CreateRow(model, acre_foot_per_second);
				CreateRow(model, acre_foot_per_minute);
				CreateRow(model, acre_foot_per_hour);
				CreateRow(model, acre_foot_per_day);
				CreateRow(model, acre_foot_per_year);
				CreateRow(model, kilograms_per_cubic_meter);
				CreateRow(model, grams_per_milliliter);
				CreateRow(model, kilograms_per_liter);
				CreateRow(model, ounces_per_cubic_foot);
				CreateRow(model, ounces_per_cubic_inch);
				CreateRow(model, ounces_per_gallon);
				CreateRow(model, pounds_per_cubic_foot);
				CreateRow(model, pounds_per_cubic_inch);
				CreateRow(model, pounds_per_gallon);
				CreateRow(model, slugs_per_cubic_foot);
				CreateRow(model, Dollar_per_joule);
				CreateRow(model, Dollar_per_kilowatt_hour);
				CreateRow(model, Dollar_per_watt);
				CreateRow(model, Dollar_per_kilowatt);
				CreateRow(model, Dollar_per_cubic_meter);
				CreateRow(model, Dollar_per_gallon);
				CreateRow(model, kilowatt_hour_per_acre_foot);
				CreateRow(model, Dollar_per_mile);
				CreateRow(model, Dollar_per_ton);
				CreateRow(model, ton_per_kilowatt_hour);
#endif
				Tag = std::static_pointer_cast<void>(model);
				mut.unlock();
			}
			else {
				model = std::static_pointer_cast<ModelType>(Tag);
			}

			if (model && model->second.count(UnitHash) > 0) {
				auto& curve = model->second.at(UnitHash);
				
				auto iter1 = curve.FindLargestSmallerEqual(targetRatio);
				if ((iter1/* != curve.end()*/) && iter1.first() == targetRatio) {
					// exact find -- best case scenario
					UnitRatio = static_cast<long double>(iter1.first()) / 1e8l;
					return *iter1.second();					
				}
				else {
					// not an exact find. 
					auto iter2 = curve.FindSmallestLargerEqual(targetRatio);
					if (iter1 /*!= curve.end()*/ && iter2 /*!= curve.end()*/) {
						if (std::abs(static_cast<long double>(iter1.first()) - static_cast<long double>(targetRatio)) < std::abs(static_cast<long double>(iter2.first()) - static_cast<long double>(targetRatio))) {
							UnitRatio = static_cast<long double>(iter1.first()) / 1e8l;
							return *iter1.second();
						}
						else {
							UnitRatio = static_cast<long double>(iter2.first()) / 1e8l;
							return *iter2.second();
						}
					}
					else if (iter1/* != curve.end()*/) {
						UnitRatio = static_cast<long double>(iter1.first()) / 1e8l;
						return *iter1.second();
					}
					else if (iter2/* != curve.end()*/) {
						UnitRatio = static_cast<long double>(iter2.first()) / 1e8l;
						return *iter2.second();
					}
				}
			}
			return out;
		};

#undef CreateRowWithMetricPrefixes
#undef CreateRow

		static const char* lookup_abbreviation(size_t UnitHash, double& UnitRatio) noexcept {
			return lookup_impl(UnitHash, UnitRatio).first;
		};
		static const char* lookup_typename(size_t UnitHash, double& UnitRatio) noexcept {
			return lookup_impl(UnitHash, UnitRatio).second;
		};
	};

	class math {
	public:
		static Units::value fabs(const Units::value& V) {
			if (V < 0) return V * -1.0; else return V;
		};
		static Units::value abs(const Units::value& V) {
			return fabs(V);
		};
		static Units::value clamp(const Units::value& V, const Units::value& min, const Units::value& max) {
			if (V < min) return min;
			if (V > max) return max;
			return V;
		};
		static Units::value floor(const Units::value& f) {
			return f.floor();
		};
		static Units::value ceiling(const Units::value& f) {
			return f.ceiling();
		};
		static Units::value round(const Units::value& a, float magnitude) {
			return floor((a / magnitude) + 0.5) * magnitude;
		};
		static Units::value max(const Units::value& a, const Units::value& b) {
			return a > b ? a : b;
		};
		static Units::value min(const Units::value& a, const Units::value& b) {
			return a < b ? a : b;
		};
		static void max_ref(Units::value* a, const Units::value& b) {
			if (b > *a) *a = b;

		};
		static void min_ref(Units::value* a, const Units::value& b) {
			if (b < *a) *a = b;
		};
	};

	class constants {
	public:
		/* PI (unitless) */
		static Units::scalar					pi() {
			return 3.141592653589793238462643383279502884197169399375105820974944;
		};

		/* speed of light in a vacuum (m/s) */
		static Units::meters_per_second		    c() {
			return 299792458.0;
		};

		/* ( m^3 / (kg * s^2) ) */
		static Units::value				        G() {
			return Units::meter(6.67408e-11) * Units::meter(1) * Units::meter(1) / (Units::kilogram(1) * Units::second(1) * Units::second(1));
		};

		/* acceleration due to gravity ( m/s^2 ) */
		static Units::value				        g() {
			return Units::meter(9.8067) / (Units::second(1) * Units::second(1));
		};

		/* density of water ( kg/m^3 ) */
		static Units::value d() {
			return Units::kilogram(998.57) / (Units::meter(1) * Units::meter(1) * Units::meter(1));
		};
	};
};

// Base classes
DefineCategoryStd(length, 1, 0, 0, 0, 0);
DefineCategoryStd(mass, 0, 1, 0, 0, 0);
DefineCategoryStd(time, 0, 0, 1, 0, 0);
DefineCategoryStd(current, 0, 0, 0, 1, 0);
DefineCategoryStd(dollar, 0, 0, 0, 0, 1);
// Derived classes
DefineCategoryStd(frequency, 0, 0, -1, 0, 0);
DefineCategoryStd(velocity, 1, 0, -1, 0, 0);
DefineCategoryStd(acceleration, 1, 0, -2, 0, 0);
DefineCategoryStd(force, 1, 1, -2, 0, 0);
DefineCategoryStd(pressure, -1, 1, -2, 0, 0);
DefineCategoryStd(charge, 0, 0, 1, 1, 0);
DefineCategoryStd(power, 2, 1, -3, 0, 0);
DefineCategoryStd(energy, 2, 1, -2, 0, 0);
DefineCategoryStd(voltage, 2, 1, -3, -1, 0);
DefineCategoryStd(impedance, 2, 1, -3, -2, 0);
DefineCategoryStd(conductance, -2, -1, 3, 2, 0);
DefineCategoryStd(area, 2, 0, 0, 0, 0);
DefineCategoryStd(volume, 3, 0, 0, 0, 0);
DefineCategoryStd(fillrate, 0, 1, -1, 0, 0);
DefineCategoryStd(flowrate, 3, 0, -1, 0, 0);
DefineCategoryStd(density, -3, 1, 0, 0, 0);
DefineCategoryStd(energy_cost_rate, -2, -1, 2, 0, 1);
DefineCategoryStd(power_cost_rate, -2, -1, 3, 0, 1);
DefineCategoryStd(volume_cost_rate, -3, 0, 0, 0, 1);
DefineCategoryStd(energy_intensity, -1, 1, -2, 0, 1);
DefineCategoryStd(length_cost_rate, -1, 0, 0, 0, 1);
DefineCategoryStd(mass_cost_rate, 0, -1, 0, 0, 1);
DefineCategoryStd(emission_rate, -2, 0, 2, 0, 1);
DefineCategoryStd(time_rate, 0, 0, -1, 0, 1);

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
DerivedUnitStdWithMetricPrefixes(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0));
DerivedUnitStd(dyne, force, dyn, Conversion <newton>(1.0 / 100000.0));
DerivedUnitStd(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0));
DerivedUnitStd(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));

// PRESSURE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(pascals, pressure, Pa, 1.0);
DerivedUnitStdWithMetricPrefixes(bar, pressure, bar, Conversion<kilopascals>(100.0));
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
DerivedUnitStdWithMetricPrefixes(joule, energy, J, 1.0);
DerivedUnitStdWithMetricPrefixes(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0));
DerivedUnitStdWithMetricPrefixes(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0));
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
DerivedUnitStdWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0);
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
// DerivedUnitStd(per_year, time_rate, p_yr, 1.0 / Conversion<year>(1.0));

namespace std {
	template<> class numeric_limits<Units::value> {
	public:
		static constexpr double min() { return std::numeric_limits<double>::min(); }
		static constexpr double max() { return std::numeric_limits<double>::max(); }
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); }
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer;
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed;
	};
};

#undef DefineCategoryType
#undef DefineCategoryStd

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitStdWithMetricPrefixes
#undef DerivedUnitStdWithMetricPrefix
#undef CalculateMetricPrefixV
#undef DerivedUnitType