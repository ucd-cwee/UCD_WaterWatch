#pragma once
#include "ThreadSafeContainers.h"
#include "../WaterWatchCpp/enum.h"

namespace GoodLang {
	namespace Units {
		BETTER_ENUM(units_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);

		// container for all of the information that defines a unit value, including the unit data as well as the underlying SI value.
		class UnitDefinition {
		public:
			static constexpr size_t NumUnits{ Units::units_type::_size_constant };
			template <typename T> static constexpr T abs(T x) { return x > (T)0 ? x : -x; };

		public:
			std::array< DoubleWrapper, NumUnits> unitType_m; // power exponents for the SI units (e.g. m^1 * kg^0 * s^-1 * A^0 * $^0 = m/s)
			DoubleWrapper ratio_m; // ratio multiplier for converting from the SI units to this actual unit (e.g. 1 = meters, 0.304 = feet, etc.) 
			DoubleWrapper value_m; // underlying value of the unit if represented as SI units. (e.g. will always be in meters, regardless of the actual unit being in feet)

			bool IsSI() const;
			bool IsScalar() const;
			static const size_t sizeOfUnits{ sizeof(unitType_m) };
		public:
			UnitDefinition() :
				unitType_m{ 0.f, 0.f, 0.f, 0.f, 0.f },
				ratio_m{ 1. },
				value_m{ 0. }
			{};
			UnitDefinition(double V) :
				unitType_m{ 0.f, 0.f, 0.f, 0.f, 0.f },
				ratio_m{ 1. },
				value_m{ V }
			{};
			UnitDefinition(double a, double b, double c, double d, double e, bool isScalar_p, const char* abbreviation_p, double ratio_p, double value_p = 0.0) noexcept :
				unitType_m{ static_cast<float>(a), static_cast<float>(b), static_cast<float>(c), static_cast<float>(d), static_cast<float>(e) },
				ratio_m{ ratio_p },
				value_m{ value_p * ratio_p }
			{};
			UnitDefinition(std::array< DoubleWrapper, NumUnits> const& unitType_p, double ratio_p, double value_p) noexcept :
				unitType_m{ unitType_p },
				ratio_m{ ratio_p },
				value_m{ value_p }
			{};

			UnitDefinition(UnitDefinition const&) = default;
			UnitDefinition(UnitDefinition&&) = default;
			UnitDefinition& operator=(UnitDefinition const&) = default;
			UnitDefinition& operator=(UnitDefinition&&) = default;

		public:
			bool IsSameCategory(UnitDefinition const& other) const noexcept;
			bool IsSameUnit(UnitDefinition const& other) const noexcept;
			size_t HashCategory() const noexcept;
			std::pair<const char*, double> LookupAbbreviation(bool isStatic) const noexcept;
			const char* LookupTypeName() const noexcept;
			std::string CreateAbbreviation(bool isStatic) const noexcept;

		};

		template <typename Derived> static constexpr __forceinline long double Conversion(long double X) { return Derived::conversion() * X; };
		static constexpr __forceinline long double SQUARED(long double X) { return X * X; };
		static constexpr __forceinline long double CUBED(long double X) { return X * X * X; };
		static __forceinline bool IsInteger(double value) {
			double intpart;
			return modf(value, &intpart) == 0.0;
		};
		static __forceinline void removeTrailingCharacters(std::string& str, const char charToRemove) {
			str.erase(str.find_last_not_of(charToRemove) + 1, std::string::npos);
		};
		static __forceinline void AddToDelimiter(std::string& obj, std::string const& toAdd, std::string const& delim) {
			if (obj.length() == 0) {
				obj += toAdd;
			}
			else {
				obj += delim;
				obj += toAdd;
			}
		};


		class value {
		public:
			mutable CAS_Container<UnitDefinition> unit_m;

		public: // constructors
			value() : unit_m{ UnitDefinition{} } {};
			explicit value(UnitDefinition const& unit_p) : unit_m{ unit_p } {};
			explicit value(double V, UnitDefinition const& unit_p) :
				unit_m{ UnitDefinition((float)unit_p.unitType_m[0], (float)unit_p.unitType_m[1], (float)unit_p.unitType_m[2], (float)unit_p.unitType_m[3], (float)unit_p.unitType_m[4], false, ""/*unit_p.abbreviation_m*/, (double)unit_p.ratio_m, V) }
			{};
			explicit value(UnitDefinition&& unit_p) : unit_m{ std::forward<UnitDefinition>(unit_p) } {};
			value(value&& V) : unit_m{ std::move(V.unit_m) } {};
			value(value const& V) : unit_m{ V.unit_m.load() } {};
			value(double V) : unit_m{ UnitDefinition(0,0,0,0,0,true,"",1,V) } {};
			virtual ~value() = default;

		protected:
			virtual bool IsStaticType() const;

		private:
			double GetVisibleValue() const noexcept;

			static std::string GetValueStr(value const& V) noexcept;

			static bool IdenticalUnits(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept;
			static bool is_scalar(UnitDefinition const& V) noexcept;

			static bool NormalArithmeticOkay(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept;
			static bool UnaryArithmeticOkay(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept;
			static void HandleNormalArithmetic(UnitDefinition const& LHS, UnitDefinition const& RHS);
			static void HandleUnaryArithmetic(UnitDefinition const& LHS, UnitDefinition const& RHS);
			static void HandleNotScalar(UnitDefinition const& V);
			static std::string AbbreviationFast(UnitDefinition const& V) noexcept;

		public: // value operator
			explicit operator double() const noexcept;
			double operator()() const noexcept;

		public: // Functions
			const char* UnitName() const noexcept;
			void Clear();
			void Swap(value const& other) const;

		public:
			std::string Abbreviation(double* visibleValue = nullptr) const noexcept;
			std::string ToString() const;

		public: // Streaming functions (should be specialized per type)
			friend std::ostream& operator<<(std::ostream& os, value const& obj) { os << obj.ToString(); return os; };
			friend std::stringstream& operator>>(std::stringstream& os, value& obj) { double v = 0; os >> v; obj = v; return os; };

		private:
			/* Used for multiplication or division operations */
			template <bool multiplication = true> value& CompoundUnits(value const& other) noexcept {
				UnitDefinition V{ other.unit_m.load() };

				unit_m.Update([&V](UnitDefinition Data)->UnitDefinition {
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
			value& MultiplyUnits(double const& V) noexcept;

		public: // = Operators
			value& operator=(value const& other) {
				UnitDefinition V{ other.unit_m.load() };

				unit_m.Update([&V, this, &other](UnitDefinition Data)->UnitDefinition {
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
						throw(std::runtime_error(GoodLang::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", this->Abbreviation().c_str(), other.Abbreviation().c_str())));
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
				unit_m.Update([](UnitDefinition Data)->UnitDefinition {
					Data.value_m += (double)Data.ratio_m;
					return Data;
					});
				return *this;
			};
			value& operator--() {
				unit_m.Update([](UnitDefinition Data)->UnitDefinition {
					Data.value_m -= (double)Data.ratio_m;
					return Data;
					});
				return *this;
			};
			value operator++(int) {
				auto a_struct = unit_m.load();
				return value(UnitDefinition(
					a_struct.unitType_m,
					a_struct.ratio_m.load(),
					a_struct.value_m.load() + a_struct.ratio_m.load()
				));

				//value out = *this; 
				//unit_m.Update([](UnitDefinition Data)->UnitDefinition {
				//	Data.value_m += (double)Data.ratio_m;
				//	return Data;
				//});
				//return out;
			};
			value operator--(int) {
				auto a_struct = unit_m.load();
				return value(UnitDefinition(
					a_struct.unitType_m,
					a_struct.ratio_m.load(),
					a_struct.value_m.load() - a_struct.ratio_m.load()
				));

				//value out = *this; 
				//unit_m.Update([](UnitDefinition Data)->UnitDefinition {
				//	Data.value_m -= (double)Data.ratio_m;
				//	return Data;
				//});
				//return out; 
			};

		private:
			static value Add(value const& a, value const& b) {
				auto a_struct = a.unit_m.load();
				auto b_struct = b.unit_m.load();

				if (a_struct.IsSameCategory(b_struct)) { // same category, but perhaps different conversion factor. That's OK. 
					return value(UnitDefinition(
						a_struct.unitType_m,
						a_struct.ratio_m.load(),
						a_struct.value_m.load() + b_struct.value_m.load()
					));
				}
				else if (is_scalar(b_struct)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					return value(UnitDefinition(
						a_struct.unitType_m,
						a_struct.ratio_m.load(),
						a_struct.value_m.load() + ((b_struct.value_m.load() / b_struct.ratio_m.load()) * a_struct.ratio_m.load())
					));
				}
				else if (is_scalar(a_struct)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
					return value(UnitDefinition(
						b_struct.unitType_m,
						b_struct.ratio_m.load(),
						b_struct.value_m.load() + ((a_struct.value_m.load() / a_struct.ratio_m.load()) * b_struct.ratio_m.load())
					));
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					throw(std::runtime_error(GoodLang::printf("Add operation failed due to incompatible non-scalar value: '%s' and '%s'.", a.Abbreviation().c_str(), b.Abbreviation().c_str())));
				}
			};
			static value Sub(value const& a, value const& b) {
				auto a_struct = a.unit_m.load();
				auto b_struct = b.unit_m.load();

				if (a_struct.IsSameCategory(b_struct)) { // same category, but perhaps different conversion factor. That's OK. 
					return value(UnitDefinition(
						a_struct.unitType_m,
						a_struct.ratio_m.load(),
						a_struct.value_m.load() - b_struct.value_m.load()
					));
				}
				else if (is_scalar(b_struct)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					return value(UnitDefinition(
						a_struct.unitType_m,
						a_struct.ratio_m.load(),
						a_struct.value_m.load() - ((b_struct.value_m.load() / b_struct.ratio_m.load()) * a_struct.ratio_m.load())
					));
				}
				else if (is_scalar(a_struct)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
					return value(UnitDefinition(
						b_struct.unitType_m,
						b_struct.ratio_m.load(),
						b_struct.value_m.load() - ((a_struct.value_m.load() / a_struct.ratio_m.load()) * b_struct.ratio_m.load())
					));
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					throw(std::runtime_error(GoodLang::printf("Subtract operation failed due to incompatible non-scalar value: '%s' and '%s'.", a.Abbreviation().c_str(), b.Abbreviation().c_str())));
				}
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
				unit_m.Update([other = V.unit_m.load()](UnitDefinition Data)->UnitDefinition {
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
				unit_m.Update([other = V.unit_m.load()](UnitDefinition Data)->UnitDefinition {
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
				unit_m.Update([other](UnitDefinition Data)->UnitDefinition {
					Data.value_m *= other.value_m;
					return Data;
					});
				return *this;
			};
			value& operator/=(value const& V) {
				auto other{ V.unit_m.load() };
				HandleNotScalar(other);
				unit_m.Update([other](UnitDefinition Data)->UnitDefinition {
					Data.value_m /= other.value_m;
					return Data;
					});
				return *this;
			};

		public: // pow and sqrt Operators
			// atomicly updates the value with a custom user-provided function.
			value& update(std::function<double(double)> updateFunction) {
				unit_m.Update([&updateFunction](UnitDefinition Data)->UnitDefinition {
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
				unit_m.Update([other](UnitDefinition Data)->UnitDefinition {
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

	};
};

