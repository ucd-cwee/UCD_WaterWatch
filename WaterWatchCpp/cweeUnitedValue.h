/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Units.h" // stands on it's own anyhow
#include <cstdarg>
#include <iostream>
#include <array>
#include <cstring>
#include <string>
#include <sstream>
#include <mutex>
#include <map>
#include <type_traits>
typedef long double				u64;
#include "enum.h"

	namespace cweeUnitValues {
		BETTER_ENUM(unit_value_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);

		__forceinline constexpr size_t HashUnits(double a, double b, double c, double d, double e) noexcept {
			constexpr double OFFSET = 1000;
			constexpr size_t A = 54059; /* a prime */
			constexpr double B = 76963; /* another prime */
			constexpr size_t C = 86969; /* yet another prime */
			constexpr size_t FIRSTH = 37; /* also prime */

			size_t h = FIRSTH;
			h = (h * A) ^ (size_t)((a+ OFFSET) * B * 100.0); 
			h = (h * A) ^ (size_t)((b + OFFSET) * B * 100.0); 
			h = (h * A) ^ (size_t)((c + OFFSET) * B * 100.0); 
			h = (h * A) ^ (size_t)((d + OFFSET) * B * 100.0); 
			h = (h * A) ^ (size_t)((e + OFFSET) * B * 100.0); 

			decltype(auto) result = h % C;
			return result;
		};
		__forceinline constexpr size_t HashUnitAndRatio(double unitHash, double ratio) noexcept {
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

		namespace cweeUnitValuesDetail { 
			static std::pair< const char*, const char*> lookup_impl(size_t ull);
			static const char* lookup_abbreviation(size_t ull);
			static const char* lookup_typename(size_t ull);
		};

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
				return std::memcmp(&unitType_m, &other.unitType_m, sizeof(unitType_m)) == 0;
			};
			bool IsSameUnit(Unit_ID const& other) const noexcept {
				return IsSameCategory(other) && (ratio_m == other.ratio_m);
			};
			decltype(auto) HashCategory() const noexcept {
				return cweeUnitValues::HashUnits(unitType_m[0], unitType_m[1], unitType_m[2], unitType_m[3], unitType_m[4]);
			};
			const char* LookupAbbreviation() const {
				using namespace cweeUnitValuesDetail;
				return lookup_abbreviation(HashUnitAndRatio(HashCategory(), ratio_m));
			};
			const char* LookupTypeName() const {
				using namespace cweeUnitValuesDetail;
				return lookup_typename(HashUnitAndRatio(HashCategory(), ratio_m));
			};
		public:
			std::array< double, NumUnits> unitType_m;
			bool isScalar_m;
			mutable const char* abbreviation_m;
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
			unit_value(double V, Unit_ID const& unit_p) noexcept : unit_m(unit_p), value_m(V* conversion()) {};
			unit_value(unit_value const& V) noexcept : unit_m(V.unit_m), value_m(V.value_m) {};
			unit_value(unit_value&& V) noexcept : unit_m(std::move(V.unit_m)), value_m(std::move(V.value_m)) {};
			template<typename T, class = std::enable_if_t<units::traits::is_unit_t<T>::value>>
			unit_value(T const& unitTypeObj) noexcept : unit_m(), value_m(0.0) {
				constexpr double length_Ratio = (double)T::unit_type::base_unit_type::meter_ratio::num / (double)T::unit_type::base_unit_type::meter_ratio::den;
				constexpr double mass_ratio = (double)T::unit_type::base_unit_type::kilogram_ratio::num / (double)T::unit_type::base_unit_type::kilogram_ratio::den;
				constexpr double second_ratio = (double)T::unit_type::base_unit_type::second_ratio::num / (double)T::unit_type::base_unit_type::second_ratio::den;
				constexpr double ampere_ratio = (double)T::unit_type::base_unit_type::ampere_ratio::num / (double)T::unit_type::base_unit_type::ampere_ratio::den;
				constexpr double dollar_ratio = (double)T::unit_type::base_unit_type::dollar_ratio::num / (double)T::unit_type::base_unit_type::dollar_ratio::den;
				constexpr bool isNotScalar = (length_Ratio != 0) || (mass_ratio != 0) || (second_ratio != 0) || (ampere_ratio != 0) || (dollar_ratio != 0);
				constexpr double factor = (double)T::unit_type::conversion_ratio::num / (double)T::unit_type::conversion_ratio::den;
				if constexpr (!isNotScalar)
					unit_m = Unit_ID(length_Ratio, mass_ratio, second_ratio, ampere_ratio, dollar_ratio, true, "", factor);
				else
					unit_m = Unit_ID(length_Ratio, mass_ratio, second_ratio, ampere_ratio, dollar_ratio, false, unitTypeObj.abbreviation(), factor);
				value_m = (unitTypeObj() * conversion()); // save it a an SI value
			};
			virtual ~unit_value() {};
		private:
			double GetVisibleValue() const noexcept { return value_m / conversion(); };
		public: // value operator
			explicit operator float() const noexcept { return GetVisibleValue(); };
			explicit operator u64() const noexcept { return GetVisibleValue(); };
			explicit operator double() const noexcept { return GetVisibleValue(); };
			double operator()() const noexcept { return GetVisibleValue(); };

		public: // Functions

			const char* UnitName() const noexcept {
				return cweeUnitValuesDetail::lookup_typename(HashUnitAndRatio(unit_m.HashCategory(), unit_m.ratio_m));
			};
			bool AreConvertableTypes(unit_value const& V) const {
				return unit_value::NormalArithmeticOkay(*this, V);
			};
			void Clear() { unit_m = Unit_ID(); value_m = 0.0; };

		private:
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

		public:
			std::string CreateAbbreviation() const {
				std::string out; double v;
				{
					v = unit_m.unitType_m[unit_value_type::METERS];
					if (v != 0) {
						if (v == 1)
							AddToDelimiter(out, "m", " ");
						else
							AddToDelimiter(out, printf("m^%s", std::to_string((float)v).c_str()), " ");
					}
				}
				{
					v = unit_m.unitType_m[unit_value_type::KILOGRAMS];
					if (v != 0) {
						if (v == 1)
							AddToDelimiter(out, "kg", " ");
						else
							AddToDelimiter(out, printf("kg^%s", std::to_string((float)v).c_str()), " ");
					}
				}
				{
					v = unit_m.unitType_m[unit_value_type::SECONDS];
					if (v != 0) {
						if (v == 1)
							AddToDelimiter(out, "s", " ");
						else
							AddToDelimiter(out, printf("s^%s", std::to_string((float)v).c_str()), " ");
					}
				}
				{
					v = unit_m.unitType_m[unit_value_type::AMPERES];
					if (v != 0) {
						if (v == 1)
							AddToDelimiter(out, "a", " ");
						else
							AddToDelimiter(out, printf("a^%s", std::to_string((float)v).c_str()), " ");
					}
				}
				{
					v = unit_m.unitType_m[unit_value_type::DOLLAR];
					if (v != 0) {
						if (v == 1)
							AddToDelimiter(out, "$", " ");
						else
							AddToDelimiter(out, printf("$^%s", std::to_string((float)v).c_str()), " ");
					}
				}
				return out;
			};

			std::string Abbreviation() const noexcept {
				if (std::strcmp(unit_m.abbreviation_m, "") == 0) {
					unit_m.abbreviation_m = unit_m.LookupAbbreviation();
					if (std::strcmp(unit_m.abbreviation_m, "") == 0) {
						std::string a(CreateAbbreviation());
						return a;
					}
					else {
						return unit_m.abbreviation_m;
					}
				}
				else {
					return unit_m.abbreviation_m;
				}
			};

			std::string ToString() const {
				std::string abbreviation(Abbreviation());
				if (abbreviation.length() > 0) return GetValueStr(*this) + " " + abbreviation;
				else return GetValueStr(*this);
			};

		public: // Streaming functions (should be specialized per type)
			friend inline std::ostream& operator<<(std::ostream& os, unit_value const& obj) { os << obj.ToString(); return os; };
			friend inline std::stringstream& operator>>(std::stringstream& os, unit_value& obj) { double v = 0; os >> v; obj = v; return os; };
			static bool IdenticalUnits(unit_value const& LHS, unit_value const& RHS) noexcept { return LHS.unit_m.IsSameCategory(RHS.unit_m); };
			static bool is_scalar(unit_value const& V) noexcept { return V.unit_m.isScalar_m; };

		private:
			static std::string GetValueStr(unit_value const& V) noexcept { return std::to_string((float)V()); };
			static bool NormalArithmeticOkay(unit_value const& LHS, unit_value const& RHS) noexcept {
				if (is_scalar(LHS) || is_scalar(RHS)) return true;
				if (IdenticalUnits(LHS, RHS)) return true;
				return false;
			};
			static bool UnaryArithmeticOkay(unit_value const& LHS, unit_value const& RHS) noexcept {
				if (is_scalar(RHS)) return true;
				if (IdenticalUnits(LHS, RHS)) return true;
				return false;
			};
			static void HandleNormalArithmetic(unit_value const& LHS, unit_value const& RHS) {
				if (NormalArithmeticOkay(LHS, RHS)) return;
				throw(std::runtime_error(printf("Normal, dynamic arithmetic failed due to incompatible non-scalar units: '%s' and '%s'", LHS.Abbreviation().c_str(), RHS.Abbreviation().c_str())));
			};
			static void HandleUnaryArithmetic(unit_value const& LHS, unit_value const& RHS) {
				if (UnaryArithmeticOkay(LHS, RHS)) return;
				throw(std::runtime_error(printf("Unary (in-place or self-modifying) arithmetic failed due to incompatible units: '%s' and '%s'", LHS.Abbreviation().c_str(), RHS.Abbreviation().c_str())));
			};
			static void HandleNotScalar(unit_value const& V) {
				if (is_scalar(V)) return;
				throw(std::runtime_error(printf("Type must be scalar (was '%s').", V.Abbreviation().c_str())));
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
				if constexpr (multiplication) {
					unit_m.ratio_m *= V.unit_m.ratio_m;
				}
				else {
					unit_m.ratio_m /= V.unit_m.ratio_m;
				}
				// unit_m.abbreviation_m = unit_m.LookupAbbreviation(); // To-Do: look-up the abbreviation based on the ratio and the unit vector. (i.e. [1.0, 0.0, 0.0, 0.0, 0.0, 1.0] = "m")
				// if (unit_m.abbreviation_m == "") unit_m.ratio_m = 1; // return to SI units if unknown combination
				unit_m.abbreviation_m = "";

				return *this;
			};
			unit_value& MultiplyUnits(double const& V) noexcept {
				if (is_scalar(*this)) return *this;
				for (int i = unit_m.unitType_m.size() - 1; i >= 0; i--) unit_m.unitType_m[i] *= V;
				if (V == 0) unit_m.isScalar_m = true;

				// now that we have modified the units, the conversion ratio makes no sense anymore and must be reset. 
				unit_m.ratio_m = std::pow(unit_m.ratio_m, V);
				// unit_m.abbreviation_m = unit_m.LookupAbbreviation();
				// if (unit_m.abbreviation_m == "") unit_m.ratio_m = 1; // return to SI units if unknown combination
				unit_m.abbreviation_m = "";

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
					throw(std::runtime_error(printf("Assignment failed due to incompatible non-scalar units: '%s' and '%s'.", this->Abbreviation().c_str(), V.Abbreviation().c_str())));
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
					throw(std::runtime_error(printf("Assignment failed due to incompatible non-scalar units: '%s' and '%s'.", this->Abbreviation().c_str(), V.Abbreviation().c_str())));
				}
				return *this;
			};

		public: // Comparison operators
			// friend bool operator==(unit_value const& A, unit_value const& V) noexcept { if (NormalArithmeticOkay(A, V) && A.GetVisibleValue() == V.GetVisibleValue()) { return true; } else { return false; } };
			friend bool operator==(unit_value const& A, unit_value const& V) noexcept { if (!NormalArithmeticOkay(A, V)) return false; if (is_scalar(V) == is_scalar(A)) { return A.value_m == V.value_m; } else if (is_scalar(V)) { unit_value W = A; W = V; return A.value_m == W.value_m; } else { unit_value W = V; W = A; return W.value_m == V.value_m; } };
			friend bool operator!=(unit_value const& A, unit_value const& V) noexcept { return !(operator==(A, V)); };
			friend bool operator<(unit_value const& A, unit_value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m < V.value_m; } else if (is_scalar(V)) { unit_value W = A; W = V; return A.value_m < W.value_m; } else { unit_value W = V; W = A; return W.value_m < V.value_m; } };
			friend bool operator<=(unit_value const& A, unit_value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m <= V.value_m; } else if (is_scalar(V)) { unit_value W = A; W = V; return A.value_m <= W.value_m; } else { unit_value W = V; W = A; return W.value_m <= V.value_m; } };
			friend bool operator>(unit_value const& A, unit_value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m > V.value_m; } else if (is_scalar(V)) { unit_value W = A; W = V; return A.value_m > W.value_m; } else { unit_value W = V; W = A; return W.value_m > V.value_m; } };
			friend bool operator>=(unit_value const& A, unit_value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m >= V.value_m; } else if (is_scalar(V)) { unit_value W = A; W = V; return A.value_m >= W.value_m; } else { unit_value W = V; W = A; return W.value_m >= V.value_m; } };

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
			friend unit_value operator+(unit_value const& A, unit_value const& V) { return Add(A, V); };
			friend unit_value operator-(unit_value const& A, unit_value const& V) { return Sub(A, V); };
			unit_value& operator+=(unit_value const& V) { 
				if (this->unit_m.IsSameCategory(V.unit_m)) { // same category, but perhaps different conversion factor. That's OK. 
					this->value_m += V.value_m;
				}
				else if (is_scalar(V)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					this->value_m += V.GetVisibleValue() * conversion();
				}
				else if (is_scalar(*this)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
					this->value_m += V.value_m;
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					HandleUnaryArithmetic(*this, V); 
					unit_value temp = *this; 
					temp = V; 
					this->value_m += temp.value_m; 					
				}

				//HandleUnaryArithmetic(*this, V); 
				//unit_value temp = *this; 
				//temp = V; 
				//this->value_m += temp.value_m; 

				return *this; 
			};
			unit_value& operator-=(unit_value const& V) { 
				HandleUnaryArithmetic(*this, V); 
				unit_value temp = *this; 
				temp = V; 
				this->value_m -= temp.value_m;
				return *this; 
			};

		public: // * and / Operators
			friend unit_value operator*(unit_value const& A, unit_value const& V) {
				unit_value out = A;
				out.CompoundUnits<true>(V);
				out.value_m *= V.value_m;
				return out;
			};
			friend unit_value operator/(unit_value const& A, unit_value const& V) {
				unit_value out = A;
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
			unit_value pow(unit_value const& V) const {
				HandleNotScalar(V);

				unit_value out = *this;
				out.MultiplyUnits(V.value_m);

				// i.e. (10 (ft)) ^ (3) -> (1000 (cu_ft)) * (1 / 35.3147 (cu_m/cu_ft)) -> 28.3168 cu_m in SI units
				out.value_m = std::pow(this->GetVisibleValue(), V.value_m) * out.conversion(); // save in SI units

				return out;
			};
			unit_value& pow_value(unit_value const& V) { HandleNotScalar(V); value_m = std::pow(GetVisibleValue(), V.GetVisibleValue()) * conversion(); return *this; };
			unit_value sqrt() const { unit_value out = *this; out.MultiplyUnits(0.5); out.value_m = std::sqrt(out.value_m); return out; };
			unit_value floor() const { unit_value out = *this; out.value_m = std::floor(GetVisibleValue()) * conversion(); return out; };
		};
		using scalar = unit_value;
	};

	namespace std {
		template<> class numeric_limits<cweeUnitValues::unit_value> {
		public:
			static constexpr double min() { return std::numeric_limits<double>::min(); }
			static constexpr double max() { return std::numeric_limits<double>::max(); }
			static constexpr double lowest() { return std::numeric_limits<double>::lowest(); }
			static constexpr bool is_integer = std::numeric_limits<double>::is_integer;
			static constexpr bool is_signed = std::numeric_limits<double>::is_signed;
		};
	};

#define DefineCategoryType(type, a, b, c, d, e) namespace cweeUnitValues { class type : public unit_value { public: \
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
	}; }; 	namespace std { template<> class numeric_limits<cweeUnitValues::type> { public: \
		static constexpr double min() { return std::numeric_limits<double>::min(); } \
		static constexpr double max() { return std::numeric_limits<double>::max(); } \
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); } \
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer; \
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed; }; \
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

	DefineCategoryType(energy_intensity, -1, 1, -2, 0, 1);
	DefineCategoryType(length_cost_rate, -1, 0, 0, 0, 1);
	DefineCategoryType(mass_cost_rate, 0, -1, 0, 0, 1);
	DefineCategoryType(emission_rate, -2, 0, 2, 0, 1);
	DefineCategoryType(time_rate, 0, 0, -1, 0, 1);


#undef DefineCategoryType

#define DerivedUnitType(type, category, abbreviation, ratio) namespace cweeUnitValues{ class type : public category  { public: \
		constexpr static double conversion() noexcept { return ratio; }; \
		constexpr static const char* specialized_abbreviation() noexcept { return #abbreviation; }; \
		constexpr static const char* specialized_name() noexcept { return #type; }; \
		type() noexcept : category(0.0, specialized_abbreviation(), ratio) {}; \
		type(double V) noexcept : category(V, specialized_abbreviation(), ratio) {}; \
		type(unit_value const& V) noexcept : category(V) {}; \
		type(unit_value&& V) noexcept : category(std::forward<unit_value>(V)) {}; \
		friend inline std::ostream& operator<<(std::ostream& os, type const& obj) { os << obj.ToString(); return os; }; \
		friend inline std::stringstream& operator>>(std::stringstream& os, type& obj) { double v = 0; os >> v; obj = v; return os; }; \
	}; }; namespace std { template<> class numeric_limits<cweeUnitValues::type> { public: \
		static constexpr double min() { return std::numeric_limits<double>::min(); } \
		static constexpr double max() { return std::numeric_limits<double>::max(); } \
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); } \
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer; \
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed; }; \
	}; namespace cweeUnitValues { namespace literals { \
		__forceinline type operator""_ ## abbreviation (long double d) { return type(static_cast<double>(d)); }\
		__forceinline type operator""_ ## abbreviation (unsigned long long d) { return type(static_cast<double>(d)); }\
	} }

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

	namespace cweeUnitValues {
		template <typename Derived> constexpr __forceinline double Conversion(double X) { return Derived::conversion() * X; };
		constexpr __forceinline double SQUARED(double X) { return X * X; };
		constexpr __forceinline double CUBED(double X) { return X * X * X; };
	};

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

	DerivedUnitType(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion<kilowatt_hour>(1.0) / Conversion<acre_foot>(1.0));
	DerivedUnitType(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion<Dollar>(1.0) / Conversion<mile>(1.0));
	DerivedUnitType(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion<Dollar>(1.0) / Conversion<metric_ton>(1.0));
	DerivedUnitType(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion<metric_ton>(1.0) / Conversion<kilowatt_hour>(1.0));
	DerivedUnitType(per_year, time_rate, p_yr, 1.0 / Conversion<year>(1.0));

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef CalculateMetricPrefixV
#undef DerivedUnitType




namespace cweeUnitValues {
	namespace  cweeUnitValuesDetail {

#define CreateRow(model, Type) model->insert({ HashUnitAndRatio(HashUnits(Type::A(), Type::B(), Type::C(), Type::D(), Type::E()), Type::conversion()), { Type::specialized_abbreviation(), #Type } })
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

		/*! Lookup the abbreviation for the type based on its unique characteristic combination (time/length/mass/etc.) */
		__forceinline static std::pair< const char*, const char*> lookup_impl(size_t ull) {
			// using namespace std::literals::string_view_literals;		

			static std::mutex mut;
			static std::shared_ptr<void> Tag;

			std::shared_ptr<std::map<size_t, std::pair< const char*, const char*>>> model;

			if (!Tag) {
				mut.lock();
			}
			if (!Tag) {
				model = std::make_shared<std::map<size_t, std::pair< const char*, const char*>>>();
				{
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
					CreateRow(model, per_year);
				}

				Tag = std::static_pointer_cast<void>(model);
				mut.unlock();
			}
			else {
				model = std::static_pointer_cast<std::map<size_t, std::pair< const char*, const char*>>>(Tag);
			}
			try {
				return model->at(ull);
			}
			catch (...) {
				return std::pair< const char*, const char*>("", "");
			}
		}
#undef CreateRowWithMetricPrefixes
#undef CreateRow

		__forceinline static const char* lookup_abbreviation(size_t ull) {
			decltype(auto) p = lookup_impl(std::move(ull));
			return p.first;
		}
		__forceinline static const char* lookup_typename(size_t ull) {
			decltype(auto) p = lookup_impl(std::move(ull));
			return p.second;
		}
	};

	namespace math {
		static unit_value fabs(const unit_value& V) {
			if (V < 0) return V * -1.0; else return V;
		};
		static unit_value abs(const unit_value& V) {
			return fabs(V);
		};
		static unit_value clamp(const unit_value& V, const unit_value& min, const unit_value& max) {
			if (V < min) return min;
			if (V > max) return max;
			return V;
		};
		static unit_value floor(const unit_value& f) {
			return f.floor();
		};
		static unit_value round(const unit_value& a, float magnitude) {
			//auto factor = cweeMath::RSqrt(magnitude);
			//factor *= factor;
			return floor((a / magnitude) + 0.5) * magnitude;
		};
	};

	namespace traits {
		template<class U1, class U2>
		struct is_convertible_unit_t {
			static constexpr const std::intmax_t value = HashUnits(U1::A(), U1::B(), U1::C(), U1::D(), U1::E()) == HashUnits(U2::A(), U2::B(), U2::C(), U2::D(), U2::E());
		};

		template<class U1>
		struct is_unit_t {
			static constexpr const std::intmax_t value = std::is_base_of<unit_value, U1>::value;
		};
	};

	

	namespace constants {
		static const scalar					pi(3.141592653589793238462643383279502884197169399375105820974944); ///< Ratio of a circle's circumference to its diameter.
		static const meters_per_second		c(299792458.0);		
		static const unit_value				G(meter(6.67408e-11)* meter(1)* meter(1) / (kilogram(1) * second(1) * second(1)));
		static const unit_value				g(meter(9.8067) / (second(1) * second(1)));
		static const unit_value				d(kilogram(998.57) / (meter(1)* meter(1)* meter(1)));
	};
};