#pragma once
#include "ThreadSafeContainers.h"
#include "../WaterWatchCpp/enum.h"
#include <string_view>
#include <initializer_list>
#include <WinDNS.h>

namespace GoodLang {
	namespace Units {
		BETTER_ENUM(units_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);
		
		using unitType_t = float;
		using valueType_t = double; // there was no improvement to accuracy with the high-precision units approach

		static __forceinline size_t HashUnits(double a, double b, double c, double d, double e) noexcept {
			size_t out{ 37 };
			GoodLang::details::hash_combine(out, a, b, c, d, e);
			return out;
		};
		template <typename Derived> static constexpr __forceinline long double Conversion(double X) { return Derived::conversion * X; };
		template <typename Derived> static constexpr __forceinline long double Conversion_t(double X) { return Derived::conversion_ratio * X; };
		static constexpr __forceinline double SQUARED(double X) { return X * X; };
		static constexpr __forceinline double CUBED(double X) { return X * X * X; };

#if 1   // experiment to convert unit system to utilize std::unique_ptr<> within		
		namespace impl {
			__forceinline constexpr unsigned const_hash(char const* input) {
				return *input ? static_cast<unsigned int>(*input) + 33 * const_hash(input + 1) : 5381;
			};

			// we cannot return a char array from a function, therefore we need a wrapper
			template <unsigned N> struct String { char c[N]; };

			template<unsigned ...Len>
			constexpr auto concat(const char(&...strings)[Len]) {
				constexpr unsigned N = (... + Len) - sizeof...(Len);
				String<N + 1> result = {};
				result.c[N] = '\0';

				char* dst = result.c;
				for (const char* src : { strings... }) {
					for (; *src != '\0'; src++, dst++) {
						*dst = *src;
					}
				}
				return result;
			}

			template<typename ratioType> class ratioType_to_name {
			public:
				static constexpr auto Abbreviation() {
					if constexpr (std::is_same_v<ratioType, std::femto>) { return impl::concat("f"); }
					else if constexpr (std::is_same_v<ratioType, std::pico>) { return impl::concat("p"); }
					else if constexpr (std::is_same_v<ratioType, std::nano>) { return impl::concat("n"); }
					else if constexpr (std::is_same_v<ratioType, std::micro>) { return impl::concat("u"); }
					else if constexpr (std::is_same_v<ratioType, std::milli>) { return impl::concat("m"); }
					else if constexpr (std::is_same_v<ratioType, std::centi>) { return impl::concat("c"); }
					else if constexpr (std::is_same_v<ratioType, std::deci>) { return impl::concat("d"); }
					else if constexpr (std::is_same_v<ratioType, std::deca>) { return impl::concat("da"); }
					else if constexpr (std::is_same_v<ratioType, std::hecto>) { return impl::concat("k"); }
					else if constexpr (std::is_same_v<ratioType, std::kilo>) { return impl::concat("k"); }
					else if constexpr (std::is_same_v<ratioType, std::mega>) { return impl::concat("M"); }
					else if constexpr (std::is_same_v<ratioType, std::giga>) { return impl::concat("G"); }
					else if constexpr (std::is_same_v<ratioType, std::tera>) { return impl::concat("T"); }
					else if constexpr (std::is_same_v<ratioType, std::peta>) { return impl::concat("P"); }
					else {
						throw std::runtime_error("Type not supported");
					}
				};
				static constexpr auto Name() {
					if constexpr (std::is_same_v<ratioType, std::femto>) { return impl::concat("femto"); }
					else if constexpr (std::is_same_v<ratioType, std::pico>) { return impl::concat("pico"); }
					else if constexpr (std::is_same_v<ratioType, std::nano>) { return impl::concat("nano"); }
					else if constexpr (std::is_same_v<ratioType, std::micro>) { return impl::concat("micro"); }
					else if constexpr (std::is_same_v<ratioType, std::milli>) { return impl::concat("milli"); }
					else if constexpr (std::is_same_v<ratioType, std::centi>) { return impl::concat("centi"); }
					else if constexpr (std::is_same_v<ratioType, std::deci>) { return impl::concat("deci"); }
					else if constexpr (std::is_same_v<ratioType, std::deca>) { return impl::concat("deca"); }
					else if constexpr (std::is_same_v<ratioType, std::hecto>) { return impl::concat("hecto"); }
					else if constexpr (std::is_same_v<ratioType, std::kilo>) { return impl::concat("kilo"); }
					else if constexpr (std::is_same_v<ratioType, std::mega>) { return impl::concat("mega"); }
					else if constexpr (std::is_same_v<ratioType, std::giga>) { return impl::concat("giga"); }
					else if constexpr (std::is_same_v<ratioType, std::tera>) { return impl::concat("tera"); }
					else if constexpr (std::is_same_v<ratioType, std::peta>) { return impl::concat("peta"); }
					else {
						throw std::runtime_error("Type not supported");
					}
				};
			};
		};
	
		using Number = double;

		namespace Definitions {
			class UnitDefinitionBase {
			public:
				static constexpr size_t NumUnits{ Units::units_type::_size_constant };
			protected:
				Number value_m; // underlying value of the unit if represented as SI units. (e.g. will always be in meters, regardless of the actual unit being in feet)

			private:
				template <typename T> static constexpr T abs(T x) { return x > (T)0 ? x : -x; };

			public:
				UnitDefinitionBase() = delete; //  : value_m{ 0 } {};
				UnitDefinitionBase(Number V) : value_m{ std::move(V) } {};
				~UnitDefinitionBase() = default;

				virtual std::unique_ptr<UnitDefinitionBase> Copy() const = 0;
				virtual const std::array<double, NumUnits>& unitType() const = 0;
				virtual const double& ratio() const = 0;
				Number& value() { return value_m; };
				const Number& value() const { return value_m; };
				virtual const char* BuiltInName() const { return ""; };
				virtual const char* BuiltInAbbreviation() const { return ""; };

				bool IsSI() const {
					auto& ut = unitType();
					return ((abs(ut[0]) + abs(ut[1]) + abs(ut[2]) + abs(ut[3]) + abs(ut[4])) == 1.0) && (abs(ratio()) == 1.0);
				};
				bool IsScalar() const {
					auto& ut = unitType();
					return ((abs(ut[0]) + abs(ut[1]) + abs(ut[2]) + abs(ut[3]) + abs(ut[4])) == 0.0) && (abs(ratio()) == 1.0);
				};
				bool IsSameCategory(UnitDefinitionBase const& other) const noexcept {
					if (IsScalar() && other.IsScalar()) return true;
					auto& ut1 = unitType();
					auto& ut2 = other.unitType();
					return std::memcmp(&ut1, &ut2, sizeof(ut1)) == 0;
				};
				bool IsSameUnit(UnitDefinitionBase const& other) const noexcept {
					return IsSameCategory(other) && (ratio() == other.ratio());
				};
				size_t HashCategory() const noexcept {
					auto& ut = unitType();
					return Units::HashUnits(ut[0], ut[1], ut[2], ut[3], ut[4]);
				};
				/* TO-DO */ std::pair<std::string_view, double> LookupAbbreviation(bool isStatic) const noexcept { return { "", 0.0 }; };
				/* TO-DO */ std::string_view LookupTypeName() const noexcept { return BuiltInName(); };
				/* TO-DO */ std::string CreateAbbreviation(bool isStatic) const noexcept { return BuiltInAbbreviation(); };
				virtual void Clear() {
					this->value_m = 0;
				};

			};

			class dynamicUnitDefinition final : public UnitDefinitionBase {
			protected:
				std::array< double, NumUnits> unitType_m; // power exponents for the SI units (e.g. m^1 * kg^0 * s^-1 * A^0 * $^0 = m/s)
				double ratio_m; // ratio multiplier for converting from the SI units to this actual unit (e.g. 1 = meters, 0.304 = feet, etc.) 

			public:
				dynamicUnitDefinition() : UnitDefinitionBase(0.0) {};
				dynamicUnitDefinition(Number value_p = 0.0)
					: UnitDefinitionBase(std::move(value_p)),
					unitType_m{ 0,0,0,0,0 },
					ratio_m{ 1.0 }
				{}
				dynamicUnitDefinition(dynamicUnitDefinition const&) = default;
				dynamicUnitDefinition(dynamicUnitDefinition&&) = default;
				dynamicUnitDefinition& operator=(dynamicUnitDefinition const&) = default;
				dynamicUnitDefinition& operator=(dynamicUnitDefinition&&) = default;
				dynamicUnitDefinition(double a, double b, double c, double d, double e, double ratio_p, Number value_p)
					: UnitDefinitionBase(std::move(value_p)),
					unitType_m{ std::move(a), std::move(b), std::move(c), std::move(d), std::move(e) },
					ratio_m{ std::move(ratio_p) }
				{}
				~dynamicUnitDefinition() = default;

				std::unique_ptr<UnitDefinitionBase> Copy() const override {
					return std::make_unique<dynamicUnitDefinition>(*this);
				}
				const std::array<double, NumUnits>& unitType() const override {
					return unitType_m;
				};
				const double& ratio() const override {
					return ratio_m;
				};
				std::array<double, NumUnits>& unitType() {
					return unitType_m;
				};
				double& ratio() {
					return ratio_m;
				};
				void Clear() override {
					this->value_m = 0;
					this->ratio_m = 0;
					for (auto& x : this->unitType_m) x = 0;
				};
			};
			template<unsigned nameHash> class unitDefinition : public UnitDefinitionBase {
			protected:
				static constexpr size_t thisHash{ nameHash };

			protected:
				unitDefinition() : UnitDefinitionBase(0.0) {};
				unitDefinition(Number V) : UnitDefinitionBase(std::move(V)) {};
			public:
				virtual ~unitDefinition() = default;
			};
		};
		namespace Definitions {
			class scalarDefinition final : public Definitions::unitDefinition<0> {
			private:
				static constexpr std::array<double, NumUnits> unitType_m{ 0, 0, 0, 0, 0 };
				static constexpr double ratio_m{ 1.0 };
			public:
				scalarDefinition() : unitDefinition(0.0) {};
				scalarDefinition(Number V) : unitDefinition(V) {};
				scalarDefinition(scalarDefinition const&) = default;
				scalarDefinition(scalarDefinition&&) = default;
				scalarDefinition& operator=(scalarDefinition const&) = default;
				scalarDefinition& operator=(scalarDefinition&&) = default;
				virtual ~scalarDefinition() = default;
				const std::array<double, NumUnits>& unitType() const override { return unitType_m; };
				const double& ratio() const override { return ratio_m; };
				const char* BuiltInName() const override { return ""; };
				const char* BuiltInAbbreviation() const override { return ""; };
				std::unique_ptr<UnitDefinitionBase> Copy() const override {
					typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;
					return std::make_unique<thisType>(*this);
				};
			};
		};

		class value_t {
		public:
			mutable SharedLockable<Definitions::UnitDefinitionBase> unit_m;

		public: // constructors
			value_t() : unit_m{ std::make_unique<Definitions::scalarDefinition>() } {};
			explicit value_t(std::unique_ptr<Definitions::UnitDefinitionBase>&& unit_p) : unit_m{ std::move(unit_p) } {};
			explicit value_t(Number const& V, Definitions::UnitDefinitionBase const& unit_p) : unit_m{ unit_p.Copy() } {
				auto get = unit_m.Unique();
				get->value() = (V / get->ratio());
			};
			value_t(value_t&& V) : unit_m{ std::move(V.unit_m) } {};
			value_t(value_t const& V) : unit_m{ V.unit_m.Shared()->Copy() } {};
			value_t(Number V) : unit_m{ std::make_unique<Definitions::scalarDefinition>(std::move(V)) } {};
			virtual ~value_t() = default;

		protected:
			// TO-DO, support full look-up table for data types
			static std::string Abbreviation(Definitions::UnitDefinitionBase const& Data, Number* visibleValue) noexcept {
				std::string toReturn{ "" };
				{
					toReturn = Data.BuiltInAbbreviation();
					bool isStatic{ toReturn != "" };
					if (isStatic) {
						// early exit
						if (visibleValue) *visibleValue = Data.value() / Data.ratio();
					}
					else if (Data.IsScalar()) {
						if (visibleValue) *visibleValue = Data.value();
					}
					else {
						// I am not a scalar, and I am a non-static (e.g. dynamic) variable.
						Number ratio_to_use = Data.ratio();

						// need to do a look-up
						bool lookupFailed = true; // To-Do...

						// if the look-up failed, create a custom unit name
						if (lookupFailed) {
							if (!Data.IsScalar() && toReturn.empty()) {
								// const_cast<double&>(Data.ratio()) = 1; // return to SI units. It couldn't be looked-up, so we don't have an option. 
								ratio_to_use = 1; // return to SI units. It couldn't be looked-up, so we don't have an option. 
								std::array< const char*, Definitions::UnitDefinitionBase::NumUnits> unitBases{ "m", "kg", "s", "A", "$" };
								bool anyNegatives = false;
								for (int i = Definitions::UnitDefinitionBase::NumUnits - 1; i >= 0; i--) {
									decltype(auto) unitBase = unitBases[i];
									decltype(auto) v = Data.unitType()[i];

									if (v > 0) {
										if (v == 1)
											AddToDelimiter(toReturn, unitBase, " ");
										else {
											std::string Num;
											if (IsInteger(v)) {
												Num = std::to_string((int)v);
											}
											else {
												Num = std::to_string(v);
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
									for (int i = Definitions::UnitDefinitionBase::NumUnits - 1; i >= 0; i--) {
										decltype(auto) unitBase = unitBases[i];
										decltype(auto) v = Data.unitType()[i];

										if (v < 0) {
											if (v == -1)
												AddToDelimiter(toReturn, unitBase, " ");
											else {
												std::string Num;
												if (IsInteger(v)) {
													Num = std::to_string((int)(-1.0 * v));
												}
												else {
													Num = std::to_string((-1.0 * v));
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

						if (visibleValue) *visibleValue = Data.value() / ratio_to_use; // the ratio may have been updated by the previous functions, and that's OK
					}
					return toReturn;					
				}
			};
		
		private:
			Number GetVisibleValue(value_t const& V) const noexcept {
				Number out{ 0 };
				auto Data = this->unit_m.Shared();
				(void)V.Abbreviation(*Data, &out);
				return out;
			};
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
			static __forceinline bool IdenticalUnits(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) noexcept { return LHS.IsSameCategory(RHS); };
			static __forceinline bool NormalArithmeticOkay(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) noexcept {
				if (LHS.IsScalar() || RHS.IsScalar()) return true;
				if (LHS.IsSameCategory(RHS)) return true;
				return false;
			};
			static __forceinline bool UnaryArithmeticOkay(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) noexcept {
				if (RHS.IsScalar()) return true;
				if (LHS.IsSameCategory(RHS)) return true;
				return false;
			};
			static __forceinline void HandleNormalArithmetic(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) {
				if (NormalArithmeticOkay(LHS, RHS)) return;
				else {
					auto A1 = Abbreviation(LHS, nullptr);
					auto A2 = Abbreviation(RHS, nullptr);
					throw(std::runtime_error(GoodLang::printf("Normal, dynamic arithmetic failed due to incompatible non-scalar value: '%s' and '%s'",
						A1.c_str(),
						A2.c_str()
					)));
				}
			};
			static __forceinline void HandleUnaryArithmetic(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) {
				if (UnaryArithmeticOkay(LHS, RHS)) return;
				else {
					auto A1 = Abbreviation(LHS, nullptr);
					auto A2 = Abbreviation(RHS, nullptr);

					throw(std::runtime_error(GoodLang::printf("Unary (in-place or self-modifying) arithmetic failed due to incompatible value: '%s' and '%s'",
						A1.c_str(),
						A2.c_str()
					)));
				}
			};
			static __forceinline void HandleNotScalar(Definitions::UnitDefinitionBase const& V) {
				if (V.IsScalar()) return;
				else {
					auto A1 = Abbreviation(V, nullptr);
					throw(std::runtime_error(GoodLang::printf("Type must be scalar (was '%s').", 
						A1.c_str()
					)));
				}
			};
			static __forceinline std::string GetValueStr(Number const& v) noexcept {
				if (std::fmod((double)v, 1.0) == 0.0) { // integer
					return std::to_string((long long)v);
				}
				else { // floating-point
					std::string out{ std::to_string((double)v) };
					removeTrailingCharacters(out, '0'); // e.g. 25.5000 -> 25.5
					removeTrailingCharacters(out, '.'); // e.g. 25.0000 -> 25. -> 25
					return out;
				}
			};

		public:
			explicit operator Number() const noexcept { return GetVisibleValue(*this); };
			Number operator()() const noexcept { return GetVisibleValue(*this); };
			std::string ToString() const {
				Number out{ 0.0 };
				auto Data = this->unit_m.Shared();
				std::string abbreviation{ Abbreviation(*Data, &out) };
				if (abbreviation.length() > 0) return GetValueStr(out) + " " + abbreviation;
				else return GetValueStr(out);
			};
			friend std::ostream& operator<<(std::ostream& os, value_t const& obj) { os << obj.ToString(); return os; };;
			std::string_view UnitName() const noexcept {
				auto Data = this->unit_m.Shared();
				return Data->BuiltInName();
			};
			void Clear() {
				auto Data = unit_m.Unique();
				Data->Clear();
			};
		public: // = Operators
			value_t& operator=(value_t const& other) {
				if (this == &other) return *this;

				auto V = other.unit_m.Shared();
				auto Data = this->unit_m.Unique();

				if (Data->IsSameCategory(*V)) { // same category, but perhaps different conversion factor. That's OK. 
					Data->value() = V->value();
				}
				else if (V->IsScalar()) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					Data->value() = (V->value() / V->ratio()) * Data->ratio();
				}
				else if (Data->IsScalar()) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
					this->unit_m.unsafe_set_ptr(V->Copy());
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					throw(std::runtime_error(GoodLang::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", 
						this->Abbreviation(*Data, nullptr).c_str(),
						other.Abbreviation(*V, nullptr).c_str()
					)));
				}
				return *this;
			};

		// Comparison operators
		private:
			template<typename Func> static __forceinline bool DoComparison(Func const& toDo, value_t const& A, value_t const& V) noexcept {
				auto Data1 = A.unit_m.Shared();
				auto Data2 = V.unit_m.Shared();

				if (!NormalArithmeticOkay(*Data1, *Data2)) return false; // we aren't the same category -- just early-exit

				// we are the same category
				if (Data1->IsScalar() == Data2->IsScalar()) {
					return toDo(Data1->value(), Data2->value());
				}
				else if (Data2->IsScalar()) {
					return toDo(Data1->value(), ((value_t(A) = V).unit_m.Unique()->value()));
				}
				else {
					// LHS is a scalar. 
					return toDo(Data2->value(), ((value_t(V) = A).unit_m.Unique()->value()));
				}
			};
		public:
			friend bool operator==(value_t const& A, value_t const& V) noexcept {
				return DoComparison([](Number LHS, Number RHS) -> bool {
					return LHS == RHS;
				}, A, V);
			};
			friend bool operator<(value_t const& A, value_t const& V) {
				return DoComparison([](Number LHS, Number RHS) -> bool {
					return LHS < RHS;
				}, A, V);
			};
			friend bool operator<=(value_t const& A, value_t const& V) {
				return DoComparison([](Number LHS, Number RHS) -> bool {
					return LHS <= RHS;
				}, A, V);
			};
			friend bool operator>(value_t const& A, value_t const& V) { return !(A <= V); };
			friend bool operator>=(value_t const& A, value_t const& V) { return !(A < V); };
			friend bool operator!=(value_t const& A, value_t const& V) noexcept { return !(operator==(A, V)); };

		private: // Unary operators
			template<typename Func> static __forceinline void DoUnaryOperation(Func const& toDo, value_t& A, value_t const& V) noexcept {
				auto Data = A.unit_m.Unique();
				auto other = V.unit_m.Shared();

				if (Data->IsSameCategory(*other)) { // same category, but perhaps different conversion factor. That's OK. 
					(void)toDo(Data->value(), other->value());
				}
				else if (other->IsScalar()) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					(void)toDo(Data->value(), (other->value() / other->ratio()) * Data->ratio());
				}
				else if (Data->IsScalar()) { // I am a scaler but the incoming unit is not. Copy the incoming value's visible value.
					(void)toDo(Data->value(), other->value() / other->ratio());
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					HandleUnaryArithmetic(*Data, *other);
					value_t temp(Data->Copy());
					temp = value_t(other->Copy());
					(void)toDo(Data->value(), temp.unit_m.Unique()->value());
				}
			};
		public:
			value_t& operator++() {
				auto Data = this->unit_m.Unique();
				Data->value() += Data->ratio();
				return *this;
			};
			value_t& operator--() {
				auto Data = this->unit_m.Unique();
				Data->value() -= Data->ratio();
				return *this;
			};
			value_t& operator+=(value_t const& V) {
				DoUnaryOperation([](Number& lhs, Number const& rhs) { lhs += rhs; }, *this, V);
				return *this;
			};
			value_t& operator-=(value_t const& V) {
				DoUnaryOperation([](Number& lhs, Number const& rhs) { lhs -= rhs; }, *this, V);
				return *this;
			};
			value_t& operator*=(value_t const& V) {
				auto other = V.unit_m.Shared();
				HandleNotScalar(*other);
				auto Data = this->unit_m.Unique();
				Data->value() *= other->value();
				return *this;
			};
			value_t& operator/=(value_t const& V) {
				auto other = V.unit_m.Shared();
				HandleNotScalar(*other);
				auto Data = this->unit_m.Unique();
				Data->value() /= other->value();
				return *this;
			};

		private: // Instancing Operators
			template <typename Func> static __forceinline value_t AddOrSubtract(Func const& toDo, value_t const& a, value_t const& b) {
				auto a_struct = a.unit_m.Shared();
				auto b_struct = b.unit_m.Shared();

				if (a_struct->IsSameCategory(*b_struct)) { // same category, but perhaps different conversion factor / ratio. That's OK. 
					return value_t(toDo(a_struct->value(), b_struct->value()) * a_struct->ratio(), *a_struct);
				}
				else if (b_struct->IsScalar()) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
					return value_t(toDo(a_struct->value(), ((b_struct->value() / b_struct->ratio()) * a_struct->ratio())) * a_struct->ratio(), *a_struct);
				}
				else if (a_struct->IsScalar()) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
					return value_t(toDo(b_struct->value(), ((a_struct->value() / a_struct->ratio()) * b_struct->ratio())) * b_struct->ratio(), *b_struct);
				}
				else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
					auto A1 = Abbreviation(*a_struct, nullptr);
					auto A2 = Abbreviation(*b_struct, nullptr);
					throw(std::runtime_error(GoodLang::printf("Add or subtract operation failed due to incompatible non-scalar value: '%s' and '%s'.", 
						A1.c_str(),
						A2.c_str()
					)));
				}
			};
			static __forceinline value_t Add(value_t const& a, value_t const& b) {
				return AddOrSubtract([](Number const& lhs, Number const& rhs) -> Number { return lhs + rhs; }, a, b);
			};
			static __forceinline value_t Sub(value_t const& a, value_t const& b) {
				return AddOrSubtract([](Number const& lhs, Number const& rhs) -> Number { return lhs - rhs; }, a, b);
			};

			/* Used for multiplication or division operations */
			static __forceinline value_t CompoundUnits(value_t const& LHS, value_t const& RHS, bool multiplication = true) noexcept {
				// if we are multiplying or dividing by a scalar, then we can simply do a Unary operation
				auto lhs = LHS.unit_m.Shared();
				auto rhs = RHS.unit_m.Shared();

				bool lhs_is_scalar = lhs->IsScalar();
				bool rhs_is_scalar = rhs->IsScalar();

				// early-exit if the RHS is a scalar, which will not change the units of the LHS
				if (rhs_is_scalar) {
					if (multiplication) {
						return value_t((lhs->value() * rhs->value()) * lhs->ratio(), *lhs);
					}
					else {
						return value_t((lhs->value() / rhs->value()) * lhs->ratio(), *lhs);
					}
				}

				// RHS is not a scaler, so the result could become one.
				std::unique_ptr<Definitions::dynamicUnitDefinition> ptr;
				if (multiplication) {
					ptr = std::make_unique<Definitions::dynamicUnitDefinition>(
						lhs->unitType()[0] + rhs->unitType()[0], 
						lhs->unitType()[1] + rhs->unitType()[1], 
						lhs->unitType()[2] + rhs->unitType()[2], 
						lhs->unitType()[3] + rhs->unitType()[3], 
						lhs->unitType()[4] + rhs->unitType()[4], 
						lhs->ratio() * rhs->ratio(), 
						lhs->value() * rhs->value()
					);
				}
				else {
					ptr = std::make_unique<Definitions::dynamicUnitDefinition>(
						lhs->unitType()[0] - rhs->unitType()[0],
						lhs->unitType()[1] - rhs->unitType()[1],
						lhs->unitType()[2] - rhs->unitType()[2],
						lhs->unitType()[3] - rhs->unitType()[3],
						lhs->unitType()[4] - rhs->unitType()[4],
						lhs->ratio() / rhs->ratio(),
						lhs->value() / rhs->value()
					);
				}

				// unitless values cannot have "ratios" -- there are not alternatives of "unitless". 
				if (ptr->IsScalar()) {
					ptr->ratio() = 1;
				}

				return value_t(std::move(ptr));
			};
			static __forceinline value_t Multiply(value_t const& LHS, value_t const& RHS) {
				return CompoundUnits(LHS, RHS, true);
			};
			static __forceinline value_t Divide(value_t const& LHS, value_t const& RHS) {
				return CompoundUnits(LHS, RHS, false);
			};

			static value_t MultiplyUnits(value_t const& LHS, double RHS) noexcept {				
				if (RHS == 1.0) {
					return LHS;
				}
				else {
					auto Data = LHS.unit_m.Shared();
					if (Data->IsScalar()) {
						return value_t(std::pow(Data->value() / Data->ratio(), RHS) * Data->ratio() * Data->ratio(), *Data);
					}
					else {
						return value_t(std::make_unique<Definitions::dynamicUnitDefinition>(
							Data->unitType()[0] * RHS,
							Data->unitType()[1] * RHS,
							Data->unitType()[2] * RHS,
							Data->unitType()[3] * RHS,
							Data->unitType()[4] * RHS,
							std::pow(Data->ratio(), RHS),
							std::pow(Data->value(), RHS)
						));
					}
				}
			};

		public: 
			value_t operator++(int) {
				value_t out{ *this }; // copy
				defer(auto Data{ this->unit_m.Unique() }; Data->value() += Data->ratio();); // increment
				return out; // returns old value, increments after
			};
			value_t operator--(int) {
				value_t out{ *this }; // copy
				defer(auto Data{ this->unit_m.Unique() }; Data->value() -= Data->ratio();); // increment
				return out; // returns old value, increments after
			};
			friend value_t operator+(value_t const& A, value_t const& B) {
				return Add(A, B);
			};
			friend value_t operator-(value_t const& A, value_t const& B) {
				return Sub(A, B);
			};
			friend value_t operator*(value_t const& A, value_t const& V) {
				return Multiply(A, V);
			};
			friend value_t operator/(value_t const& A, value_t const& V) {
				return Divide(A, V);
			};
			value_t operator-() const {
				return Multiply(*this, -1);
			};
			// atomicly updates the value_t with a custom user-provided function.
			value_t& update(std::function<double(double)> const& updateFunction) {
				auto Data = this->unit_m.Unique();
				Data->value() = updateFunction(Data->value() / Data->ratio()) * Data->ratio();
				return *this;
			};
			// Creats a copy of the value_t and updates it with a custom user-provided function.
			value_t update(std::function<double(double)> const& updateFunction) const {
				auto Data = this->unit_m.Shared();
				return value_t(updateFunction(Data->value() / Data->ratio()) * Data->ratio() * Data->ratio(), *Data);
			};
			// Returns a new value_t multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
			value_t pow(value_t const& V) const {
				auto other = V.unit_m.Shared();
				HandleNotScalar(*other);
				return MultiplyUnits(*this, other->value());
			};
			// atomicly updates the value_t by exponentiating the underlying value_t (e.g. (3_m).pow_value_t(3) => 9_m)
			value_t& pow_value(value_t const& V) {
				auto Data = this->unit_m.Unique();
				auto Other = V.unit_m.Shared();
				HandleNotScalar(*Other);
				Data->value() = std::pow(Data->value() / Data->ratio(), Other->value()) * Data->ratio();
				return *this;
			};
			// atomicly updates the value_t by exponentiating the underlying value_t (e.g. (3_m).pow_value_t(3) => 9_m)
			value_t pow_value(value_t const& V) const {
				auto Other = V.unit_m.Shared(); 
				HandleNotScalar(*Other);
				return this->update([&Other](double x)->double { return std::pow(x, Other->value()); });
			};
			// pow(0.5)
			value_t sqrt() const {
				return pow(0.5);
			};
			// atomicly floors (rounds to lower whole integer) the underlying value_t
			value_t& floor() {
				return update([](double v)->double { return std::floor(v); });
			};
			// Creats a copy of the value_t and floors (rounds to lower whole integer) the underlying value_t
			value_t floor() const {
				return update([](double v)->double { return std::floor(v); });
			};
			// atomicly ceilings (rounds to upper whole integer) the underlying value_t
			value_t& ceiling() {
				return update([](double v)->double { return std::ceil(v); });
			};
			// Creats a copy of the value_t and ceilings (rounds to upper whole integer) the underlying value_t
			value_t ceiling() const {
				return update([](double v)->double { return std::ceil(v); });
			};
#if 0
		public:
			static std::vector<std::vector<std::tuple<std::string, std::string, Units::value_t, std::weak_ptr<GoodLang::Type_Info>>>> GetValueTypes() noexcept;
#endif

		};
		using scalar_t = value_t;

		namespace Categories {
			class length { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 1, 0, 0, 0, 0 }; };
		    class mass { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 1, 0, 0, 0 }; };
			class time { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 0, 1, 0, 0 }; };
			class current { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 0, 0, 1, 0 }; };
			class dollar { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 0, 0, 0, 1 }; };
			class frequency { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 0, -1, 0, 0 }; };
			class velocity { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 1, 0, -1, 0, 0 }; };
			class acceleration { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 1, 0, -2, 0, 0 }; };
			class force { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 1, 1, -2, 0, 0 }; };
			class pressure { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -1, 1, -2, 0, 0 }; };
			class charge { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 0, 1, 1, 0 }; };
			class power { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 2, 1, -3, 0, 0 }; };
			class energy { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 2, 1, -2, 0, 0 }; };
			class voltage {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 2, 1, -3, -1, 0 }; };
			class impedance {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 2, 1, -3, -2, 0 }; };
			class conductance {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -2, -1, 3, 2, 0 }; };
			class area {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 2, 0, 0, 0, 0 }; };
			class volume { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 3, 0, 0, 0, 0 }; };
			class fillrate { 
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 1, -1, 0, 0 }; };
			class flowrate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 3, 0, -1, 0, 0 }; };
			class density {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -3, 1, 0, 0, 0 }; };
			class energy_cost_rate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -2, -1, 2, 0, 1 }; };
			class power_cost_rate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -2, -1, 3, 0, 1 }; };
			class volume_cost_rate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -3, 0, 0, 0, 1 }; };
			class energy_intensity {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -1, 1, -2, 0, 1 }; };
			class length_cost_rate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -1, 0, 0, 0, 1 }; };
			class mass_cost_rate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, -1, 0, 0, 1 }; };
			class emission_rate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ -2, 0, 2, 0, 1 }; };
			class time_rate {
			public: static constexpr std::array<double, Definitions::UnitDefinitionBase::NumUnits> unitType_m{ 0, 0, -1, 0, 1 }; };


		};

		// METRIX PREFIX CLASS DEFINITION
		namespace Definitions {
			template<typename ratioType, typename baseType>
			class MetricPrefixedDefinition final : public unitDefinition<impl::const_hash(impl::concat(impl::ratioType_to_name<ratioType>::Name().c, baseType::Name_m.c).c)> {
			public:
				using unitDefBase = unitDefinition<impl::const_hash(impl::concat(impl::ratioType_to_name<ratioType>::Name().c, baseType::Name_m.c).c)>;
				static constexpr double ratio_m{ baseType::ratio_m * ((double)ratioType::num / (double)ratioType::den) };
				static constexpr auto Name_m{ impl::concat(impl::ratioType_to_name<ratioType>::Name().c, baseType::Name_m.c) };
				static constexpr auto Abbreviation_m{ impl::concat(impl::ratioType_to_name<ratioType>::Abbreviation().c, baseType::Abbreviation_m.c) };
			public:
				MetricPrefixedDefinition() : unitDefBase(0) {};
				MetricPrefixedDefinition(Number V) : unitDefBase(V* (Number)ratio_m) {};
				MetricPrefixedDefinition(MetricPrefixedDefinition const&) = default;
				MetricPrefixedDefinition(MetricPrefixedDefinition&&) = default;
				MetricPrefixedDefinition& operator=(MetricPrefixedDefinition const&) = default;
				MetricPrefixedDefinition& operator=(MetricPrefixedDefinition&&) = default;
				virtual ~MetricPrefixedDefinition() = default;
				const std::array<double, UnitDefinitionBase::NumUnits>& unitType() const override { return baseType::unitType_m; };
				const double& ratio() const override { return ratio_m; };
				const char* BuiltInName() const override { return &Name_m.c[0]; };
				const char* BuiltInAbbreviation() const override { return &Abbreviation_m.c[0]; };
				std::unique_ptr<UnitDefinitionBase> Copy() const override {
					typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;
					return std::make_unique<thisType>(*this);
				};
			};
		};

#define CalculateMetricPrefixV(metric) ((long double)std::metric::num / (long double)std::metric::den)
#define DerivedUnitList \
	DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0); \
	DerivedUnitType(foot, length, ft, Conversion_t<meter_t>(381.0 / 1250.0)); \
	DerivedUnitType(inch, length, in, Conversion_t<foot_t>(1.0 / 12.0)); \
	DerivedUnitType(furlong, length, fur, Conversion_t<foot_t>(660)); \
	DerivedUnitType(mile, length, mi, Conversion_t<foot_t>(5280)); \
	DerivedUnitType(nauticalMile, length, nmi, Conversion_t<meter_t>(1852.0)); \
	DerivedUnitType(astronicalUnit, length, au, Conversion_t<meter_t>(149597870700.0)); \
	DerivedUnitType(yard, length, yd, Conversion_t<foot_t>(3.0)); \
	DerivedUnitTypeWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0); \
	DerivedUnitType(metric_ton, mass, t, Conversion_t<kilogram_t>(1000.0)); \
	DerivedUnitType(pound, mass, lb, Conversion_t<kilogram_t>(45359237.0 / 100000000.0)); \
	DerivedUnitType(long_ton, mass, ln_t, Conversion_t<pound_t>(2240.0)); \
	DerivedUnitType(short_ton, mass, sh_t, Conversion_t<pound_t>(2000.0)); \
	DerivedUnitType(stone, mass, st, Conversion_t<pound_t>(14.0)); \
	DerivedUnitType(ounce, mass, oz, Conversion_t<pound_t>(1.0 / 16.0)); \
	DerivedUnitType(carat, mass, ct, Conversion_t<milligram_t>(200.0)); \
	DerivedUnitType(slug, mass, slug, Conversion_t<kilogram_t>(145939029.0 / 10000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(second, time, s, 1.0); \
	DerivedUnitType(minute, time, min, Conversion_t<second_t>(60.0)); \
	DerivedUnitType(hour, time, hr, Conversion_t<minute_t>(60.0)); \
	DerivedUnitType(day, time, d, Conversion_t<hour_t>(24.0)); \
	DerivedUnitType(week, time, wk, Conversion_t<day_t>(7.0)); \
	DerivedUnitType(year, time, yr, Conversion_t<day_t>(365)); \
	DerivedUnitType(month, time, mnth, Conversion_t<year_t>(1.0 / 12.0)); \
	DerivedUnitType(julian_year, time, a_j, Conversion_t<second_t>(31557600.0)); \
	DerivedUnitType(gregorian_year, time, a_g, Conversion_t<second_t>(31556952.0)); \
	DerivedUnitTypeWithMetricPrefixes(ampere, current, A, 1.0); \
	DerivedUnitType(Dollar, dollar, USD, 1.0); \
	DerivedUnitType(MillionDollar, dollar, MUSD, Conversion_t<Dollar_t>(1000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(hertz, frequency, Hz, 1.0); \
	DerivedUnitType(meters_per_second, velocity, mps, Conversion_t<meter_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(feet_per_second, velocity, fps, Conversion_t<foot_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(feet_per_minute, velocity, fpm, Conversion_t<foot_t>(1.0) / Conversion_t<minute_t>(1.0)); \
	DerivedUnitType(feet_per_hour, velocity, fph, Conversion_t<foot_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(miles_per_hour, velocity, mph, Conversion_t<mile_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(kilometers_per_hour, velocity, kph, Conversion_t<kilometer_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(knot, velocity, kts, Conversion_t<nauticalMile_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(meters_per_second_squared, acceleration, mps_sq, Conversion_t<meter_t>(1.0) / (Conversion_t<second_t>(1.0) * Conversion_t<second_t>(1.0))); \
	DerivedUnitType(feet_per_second_squared, acceleration, fps_sq, Conversion_t<foot_t>(1.0) / (Conversion_t<second_t>(1.0) * Conversion_t<second_t>(1.0))); \
	DerivedUnitType(standard_gravity, acceleration, SG, Conversion_t<meters_per_second_squared_t>(980665.0 / 100000.0)); \
	DerivedUnitTypeWithMetricPrefixes(newton, force, N, Conversion_t<kilogram_t>(1.0)* Conversion_t<meters_per_second_squared_t>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(pound_f, force, lbf, Conversion_t<slug_t>(1.0)* Conversion_t<feet_per_second_squared_t>(1.0)); \
	DerivedUnitType(dyne, force, dyn, Conversion_t<newton_t>(1.0 / 100000.0)); \
	DerivedUnitType(kilopond, force, kp, Conversion_t<standard_gravity_t>(1.0)* Conversion_t<kilogram_t>(1.0)); \
	DerivedUnitType(poundal, force, pdl, Conversion_t<pound_t>(1.0)* Conversion_t<foot_t>(1.0) / (Conversion_t<second_t>(1.0) * Conversion_t<second_t>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(pascals, pressure, Pa, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(bar, pressure, bar, Conversion_t<kilopascals_t>(100.0)); \
	DerivedUnitType(atmosphere, pressure, atm, Conversion_t<pascals_t>(101325.0)); \
	DerivedUnitType(pounds_per_square_inch, pressure, psi, Conversion_t<pound_f_t>(1.0) / (Conversion_t<inch_t>(1.0) * Conversion_t<inch_t>(1.0))); \
	DerivedUnitType(head, pressure, ft_water, Conversion_t<pound_f_t>(62.43) / (Conversion_t<foot_t>(1.0) * Conversion_t<foot_t>(1.0))); \
	DerivedUnitType(torr, pressure, torr, Conversion_t<atmosphere_t>(1.0 / 760.0)); \
	DerivedUnitType(coulomb, charge, C, 1.0);  \
	DerivedUnitTypeWithMetricPrefixes(ampere_hour, charge, Ah, Conversion_t< ampere_t>(1.0)* Conversion_t<hour_t>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt, power, W, 1.0); \
	DerivedUnitType(horsepower, power, hp, Conversion_t<watt_t>(7457.0 / 10.0)); \
	DerivedUnitTypeWithMetricPrefixes(joule, energy, J, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(calorie, energy, cal, Conversion_t<joule_t>(4184.0 / 1000.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt_minute, energy, Wm, Conversion_t<watt_t>(1.0)* Conversion_t<minute_t>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(watt_hour, energy, Wh, Conversion_t<watt_t>(1.0)* Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(watt_day, energy, Wd, Conversion_t<watt_t>(1.0)* Conversion_t<day_t>(1.0)); \
	DerivedUnitType(british_thermal_unit, energy, BTU, Conversion_t<joule_t>(105505585262.0 / 100000000.0)); \
	DerivedUnitType(british_thermal_unit_iso, energy, BTU_iso, Conversion_t<joule_t>(1055056.0 / 1000.0)); \
	DerivedUnitType(british_thermal_unit_59, energy, BTU59, Conversion_t<joule_t>(1054804.0 / 1000.0)); \
	DerivedUnitType(therm, energy, thm, Conversion_t<british_thermal_unit_59_t>(100000.0)); \
	DerivedUnitType(foot_pound, energy, ftlbf, Conversion_t<joule_t>(13558179483314004.0 / 10000000000000000.0)); \
	DerivedUnitTypeWithMetricPrefixes(volt, voltage, V, 1.0); \
	DerivedUnitTypeWithMetricPrefixes(ohm, impedance, Ohm, 1.0); \
	DerivedUnitType(siemens, conductance, S, 1.0);  \
	DerivedUnitType(square_meter, area, sq_m, 1.0); \
	DerivedUnitType(square_foot, area, sq_ft, Conversion_t<foot_t>(1.0)* Conversion_t<foot_t>(1.0)); \
	DerivedUnitType(square_inch, area, sq_in, Conversion_t<inch_t>(1.0)* Conversion_t<inch_t>(1.0)); \
	DerivedUnitType(square_mile, area, sq_mi, Conversion_t<mile_t>(1.0)* Conversion_t<mile_t>(1.0)); \
	DerivedUnitType(square_kilometer, area, sq_km, Conversion_t<kilometer_t>(1.0)* Conversion_t<kilometer_t>(1.0)); \
	DerivedUnitType(hectare, area, ha, Conversion_t<square_meter_t>(1000.0)); \
	DerivedUnitType(acre, area, acre, Conversion_t<square_foot_t>(43560.0)); \
	DerivedUnitType(cubic_meter, volume, cu_m, 1.0); \
	DerivedUnitType(cubic_millimeter, volume, cu_mm, CUBED(Conversion_t<millimeter_t>(1.0))); \
	DerivedUnitType(cubic_kilometer, volume, cu_km, CUBED(Conversion_t<kilometer_t>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(liter, volume, L, CUBED(Conversion_t<decimeter_t>(1.0))); \
	DerivedUnitType(cubic_inch, volume, cu_in, CUBED(Conversion_t<inch_t>(1.0))); \
	DerivedUnitType(cubic_foot, volume, cu_ft, CUBED(Conversion_t<foot_t>(1.0))); \
	DerivedUnitType(cubic_yard, volume, cu_yd, CUBED(Conversion_t<yard_t>(1.0))); \
	DerivedUnitType(cubic_mile, volume, cu_mi, CUBED(Conversion_t<mile_t>(1.0))); \
	DerivedUnitTypeWithMetricPrefixes(gallon, volume, gal, Conversion_t<cubic_inch_t>(231.0)); \
	DerivedUnitType(imperial_gallon, volume, igal, Conversion_t<gallon_t>(10.0 / 12.0)); \
	DerivedUnitType(million_gallon, volume, MG, Conversion_t<gallon_t>(1.0)* CalculateMetricPrefixV(mega)); \
	DerivedUnitType(imperial_million_gallon, volume, IMG, Conversion_t<imperial_gallon_t>(1.0)* CalculateMetricPrefixV(mega)); \
	DerivedUnitType(acre_foot, volume, ac_ft, Conversion_t<acre_t>(1.0)* Conversion_t<foot_t>(1.0)); \
	DerivedUnitType(quart, volume, qt, Conversion_t<gallon_t>(0.25)); \
	DerivedUnitType(pint, volume, pt, Conversion_t<quart_t>(0.5)); \
	DerivedUnitType(cup, volume, c, Conversion_t<pint_t>(0.5)); \
	DerivedUnitType(fluid_ounce, volume, fl_oz, Conversion_t<cup_t>(0.125)); \
	DerivedUnitType(barrel, volume, bl, Conversion_t<gallon_t>(42.0)); \
	DerivedUnitType(bushel, volume, bu, Conversion_t<cubic_inch_t>(215042.0 / 100.0)); \
	DerivedUnitType(cord, volume, cord, Conversion_t<cubic_foot_t>(128.0)); \
	DerivedUnitType(tablespoon, volume, tbsp, Conversion_t<fluid_ounce_t>(0.5)); \
	DerivedUnitType(teaspoon, volume, tsp, Conversion_t<fluid_ounce_t>(1.0 / 6.0)); \
	DerivedUnitType(pinch, volume, pinch, Conversion_t<teaspoon_t>(1.0 / 8.0)); \
	DerivedUnitType(dash, volume, dash, Conversion_t<pinch_t>(1.0 / 2.0)); \
	DerivedUnitType(drop, volume, drop, Conversion_t<fluid_ounce_t>(1.0 / 360.0)); \
	DerivedUnitType(fifth, volume, fifth, Conversion_t<gallon_t>(0.2)); \
	DerivedUnitType(dram, volume, dr, Conversion_t<fluid_ounce_t>(0.125)); \
	DerivedUnitType(gill, volume, gi, Conversion_t<fluid_ounce_t>(4.0)); \
	DerivedUnitType(peck, volume, pk, Conversion_t<bushel_t>(0.25)); \
	DerivedUnitType(sack, volume, sacks, Conversion_t<bushel_t>(3.0)); \
	DerivedUnitType(shot, volume, shots, Conversion_t<fluid_ounce_t>(3.0 / 2.0)); \
	DerivedUnitType(strike, volume, strikes, Conversion_t<bushel_t>(2.0)); \
	DerivedUnitTypeWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0); \
	DerivedUnitType(metric_ton_per_second, fillrate, mTs, Conversion_t<metric_ton_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(metric_ton_per_minute, fillrate, mTm, Conversion_t<metric_ton_t>(1.0) / Conversion_t<minute_t>(1.0)); \
	DerivedUnitType(metric_ton_per_hour, fillrate, mTh, Conversion_t<metric_ton_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(metric_ton_per_day, fillrate, mTd, Conversion_t<metric_ton_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(metric_ton_per_year, fillrate, mTy, Conversion_t<metric_ton_t>(1.0) / Conversion_t<year_t>(1.0)); \
	DerivedUnitType(cubic_meter_per_second, flowrate, cms, 1.0); \
	DerivedUnitType(cubic_meter_per_hour, flowrate, cmh, Conversion_t<cubic_meter_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(cubic_meter_per_day, flowrate, cmd, Conversion_t<cubic_meter_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(cubic_millimeter_per_second, flowrate, cmms, Conversion_t<cubic_millimeter_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitTypeWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion_t<liter_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(liter_per_minute, flowrate, lpm, Conversion_t<liter_t>(1.0) / Conversion_t<minute_t>(1.0)); \
	DerivedUnitType(liter_per_day, flowrate, lpd, Conversion_t<liter_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(megaliter_per_day, flowrate, Mlpd, Conversion_t<megaliter_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(cubic_inch_per_second, flowrate, cis, Conversion_t<cubic_inch_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(cubic_inch_per_hour, flowrate, cih, Conversion_t<cubic_inch_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(cubic_foot_per_second, flowrate, cfs, Conversion_t<cubic_foot_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(cubic_foot_per_hour, flowrate, cfh, Conversion_t<cubic_foot_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(gallon_per_second, flowrate, gps, Conversion_t<gallon_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(gallon_per_minute, flowrate, gpm, Conversion_t<gallon_t>(1.0) / Conversion_t<minute_t>(1.0)); \
	DerivedUnitType(gallon_per_hour, flowrate, gph, Conversion_t<gallon_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(gallon_per_day, flowrate, gpd, Conversion_t<gallon_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(gallon_per_year, flowrate, gpy, Conversion_t<gallon_t>(1.0) / Conversion_t<year_t>(1.0)); \
	DerivedUnitType(million_gallon_per_second, flowrate, MGS, Conversion_t<megagallon_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(million_gallon_per_minute, flowrate, MGM, Conversion_t<megagallon_t>(1.0) / Conversion_t<minute_t>(1.0)); \
	DerivedUnitType(million_gallon_per_hour, flowrate, MGH, Conversion_t<megagallon_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(million_gallon_per_day, flowrate, MGD, Conversion_t<megagallon_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(million_gallon_per_year, flowrate, MGY, Conversion_t<megagallon_t>(1.0) / Conversion_t<year_t>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_second, flowrate, IMGS, Conversion_t<imperial_million_gallon_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion_t<imperial_million_gallon_t>(1.0) / Conversion_t<minute_t>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion_t<imperial_million_gallon_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_day, flowrate, IMGD, Conversion_t<imperial_million_gallon_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(imperial_million_gallon_per_year, flowrate, IMGY, Conversion_t<imperial_million_gallon_t>(1.0) / Conversion_t<year_t>(1.0)); \
	DerivedUnitType(acre_foot_per_second, flowrate, ac_ft_s, Conversion_t<acre_foot_t>(1.0) / Conversion_t<second_t>(1.0)); \
	DerivedUnitType(acre_foot_per_minute, flowrate, ac_ft_m, Conversion_t<acre_foot_t>(1.0) / Conversion_t<minute_t>(1.0)); \
	DerivedUnitType(acre_foot_per_hour, flowrate, ac_ft_h, Conversion_t<acre_foot_t>(1.0) / Conversion_t<hour_t>(1.0)); \
	DerivedUnitType(acre_foot_per_day, flowrate, ac_ft_d, Conversion_t<acre_foot_t>(1.0) / Conversion_t<day_t>(1.0)); \
	DerivedUnitType(acre_foot_per_year, flowrate, ac_ft_y, Conversion_t<acre_foot_t>(1.0) / Conversion_t<year_t>(1.0)); \
	DerivedUnitType(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0); \
	DerivedUnitType(grams_per_milliliter, density, g_per_mL, Conversion_t<gram_t>(1.0) / Conversion_t<milliliter_t>(1.0)); \
	DerivedUnitType(kilograms_per_liter, density, kg_per_L, Conversion_t<kilogram_t>(1.0) / Conversion_t<liter_t>(1.0)); \
	DerivedUnitType(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion_t<ounce_t>(1.0) / Conversion_t<cubic_foot_t>(1.0)); \
	DerivedUnitType(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion_t<ounce_t>(1.0) / Conversion_t<cubic_inch_t>(1.0)); \
	DerivedUnitType(ounces_per_gallon, density, oz_per_gal, Conversion_t<ounce_t>(1.0) / Conversion_t<gallon_t>(1.0)); \
	DerivedUnitType(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion_t<pound_t>(1.0) / Conversion_t<cubic_foot_t>(1.0)); \
	DerivedUnitType(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion_t<pound_t>(1.0) / Conversion_t<cubic_inch_t>(1.0)); \
	DerivedUnitType(pounds_per_gallon, density, lb_per_gal, Conversion_t<pound_t>(1.0) / Conversion_t<gallon_t>(1.0)); \
	DerivedUnitType(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion_t<slug_t>(1.0) / Conversion_t<cubic_foot_t>(1.0)); \
	DerivedUnitType(Dollar_per_joule, energy_cost_rate, USD_per_j, Conversion_t<Dollar_t>(1.0) / Conversion_t<joule_t>(1.0)); \
	DerivedUnitType(Dollar_per_kilowatt_hour, energy_cost_rate, USD_per_kWh, Conversion_t<Dollar_t>(1.0) / Conversion_t<kilowatt_hour_t>(1.0)); \
	DerivedUnitType(Dollar_per_watt, power_cost_rate, USD_per_w, Conversion_t<Dollar_t>(1.0) / Conversion_t<watt_t>(1.0)); \
	DerivedUnitType(Dollar_per_kilowatt, power_cost_rate, USD_per_kW, Conversion_t<Dollar_t>(1.0) / Conversion_t<kilowatt_t>(1.0)); \
	DerivedUnitType(Dollar_per_cubic_meter, volume_cost_rate, USD_per_cm, Conversion_t<Dollar_t>(1.0) / Conversion_t<cubic_meter_t>(1.0)); \
	DerivedUnitType(Dollar_per_gallon, volume_cost_rate, USD_per_gal, Conversion_t<Dollar_t>(1.0) / Conversion_t<gallon_t>(1.0)); \
	DerivedUnitType(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion_t<kilowatt_hour_t>(1.0) / Conversion_t<acre_foot_t>(1.0)); \
	DerivedUnitType(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion_t<Dollar_t>(1.0) / Conversion_t<mile_t>(1.0)); \
	DerivedUnitType(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion_t<Dollar_t>(1.0) / Conversion_t<metric_ton_t>(1.0)); \
	DerivedUnitType(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion_t<metric_ton_t>(1.0) / Conversion_t<kilowatt_hour_t>(1.0));

#define DerivedUnitType(type, category, abbreviation, Ratio) \
	class type ## _t final : public value_t { \
	public: \
		constexpr static double conversion_ratio{ Ratio }; \
		type ## _t(); \
		type ## _t(value_t const& other); \
		type ## _t(Number const& V); \
		virtual ~type ## _t() = default; \
	};

#define DerivedUnitTypeWithMetricPrefix(type, prefix) class prefix ## type ## _t final : public value_t { \
	public: \
		constexpr static double conversion_ratio{ type ## _t::conversion_ratio * ((double)std::prefix::num / (double)std::prefix::den) }; \
		prefix ## type ## _t(); \
		prefix ## type ## _t(value_t const& other); \
		prefix ## type ## _t(Number const& V); \
		virtual ~prefix ## type ## _t() = default; \
	};

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
	DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, femto); \
	DerivedUnitTypeWithMetricPrefix(type, pico); \
	DerivedUnitTypeWithMetricPrefix(type, nano); \
	DerivedUnitTypeWithMetricPrefix(type, micro); \
	DerivedUnitTypeWithMetricPrefix(type, milli); \
	DerivedUnitTypeWithMetricPrefix(type, centi); \
	DerivedUnitTypeWithMetricPrefix(type, deci); \
	DerivedUnitTypeWithMetricPrefix(type, deca); \
	DerivedUnitTypeWithMetricPrefix(type, hecto); \
	DerivedUnitTypeWithMetricPrefix(type, kilo); \
	DerivedUnitTypeWithMetricPrefix(type, mega); \
	DerivedUnitTypeWithMetricPrefix(type, giga); \
	DerivedUnitTypeWithMetricPrefix(type, tera); \
	DerivedUnitTypeWithMetricPrefix(type, peta)

	DerivedUnitList; // this loops through the definitions for DerivedUnitTypeWithMetricPrefixes() and DerivedUnitType() for all units. Change thosse macro definitions to change the implimentations. 

#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitType

	class math_t {
	public:
		// return absolute value
		static Units::value_t fabs(const Units::value_t& V);
		// return absolute value
		static Units::value_t abs(const Units::value_t& V);
		// clamp number to lower/upper bound
		static Units::value_t clamp(const Units::value_t& V, const Units::value_t& min, const Units::value_t& max);
		// round to lower whole number
		static Units::value_t floor(const Units::value_t& f);
		// round to higher whole number
		static Units::value_t ceiling(const Units::value_t& f);
		// round to nearest whole number
		static Units::value_t round(const Units::value_t& a, float magnitude);
		// return max(a, b);
		static Units::value_t max(const Units::value_t& a, const Units::value_t& b);
		// return min(a, b);
		static Units::value_t min(const Units::value_t& a, const Units::value_t& b);
		// if (b > a) a = b; // prevents copying when not necessary
		static void max_ref(Units::value_t& a, const Units::value_t& b);
		// if (b < a) a = b; // prevents copying when not necessary
		static void min_ref(Units::value_t& a, const Units::value_t& b);
	};

	class constants_t {
	public:
		/* PI (unitless) */
		static Units::scalar_t					pi();

		/* speed of light in a vacuum (m/s) */
		static Units::meters_per_second_t		    c();

		/* ( m^3 / (kg * s^2) ) */
		static Units::value_t				        G();

		/* acceleration due to gravity ( m/s^2 ) */
		static Units::meters_per_second_squared_t	g();

		/* density of water ( kg/m^3 ) */
		static Units::kilograms_per_cubic_meter_t d();
	};



#endif	



















		// container for all of the information that defines a unit value, including the unit data as well as the underlying SI value.
		class UnitDefinition {
		public:
			static constexpr size_t NumUnits{ Units::units_type::_size_constant };
			template <typename T> static constexpr T abs(T x) { return x > (T)0 ? x : -x; };

		public:
			std::array< unitType_t, NumUnits> unitType_m; // power exponents for the SI units (e.g. m^1 * kg^0 * s^-1 * A^0 * $^0 = m/s)
			valueType_t ratio_m; // ratio multiplier for converting from the SI units to this actual unit (e.g. 1 = meters, 0.304 = feet, etc.) 
			valueType_t value_m; // underlying value of the unit if represented as SI units. (e.g. will always be in meters, regardless of the actual unit being in feet)

			bool IsSI() const;
			bool IsScalar() const;
			static const size_t sizeOfUnits{ sizeof(unitType_m) };
		public:
			UnitDefinition() :
				unitType_m{ 0, 0, 0, 0, 0 },
				ratio_m{ 1. },
				value_m{ 0. }
			{};
			UnitDefinition(valueType_t V) :
				unitType_m{ 0, 0, 0, 0, 0 },
				ratio_m{ 1. },
				value_m{ V }
			{};
			UnitDefinition(valueType_t a, valueType_t b, valueType_t c, valueType_t d, valueType_t e, bool isScalar_p, const char* abbreviation_p, valueType_t ratio_p, valueType_t value_p = 0.0) noexcept :
				unitType_m{ (unitType_t)a, (unitType_t)b, (unitType_t)c, (unitType_t)d, (unitType_t)e },
				ratio_m{ ratio_p },
				value_m{ value_p * ratio_p }
			{};
			UnitDefinition(std::array< unitType_t, NumUnits> const& unitType_p, valueType_t ratio_p, valueType_t value_p) noexcept :
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
			std::pair<std::string_view, valueType_t> LookupAbbreviation(bool isStatic) const noexcept;
			std::string_view LookupTypeName() const noexcept;
			std::string CreateAbbreviation(bool isStatic) const noexcept;

		};
		class value {
		public:
			mutable Lockable<UnitDefinition> unit_m;

		public: // constructors
			value() : unit_m{ UnitDefinition{} } {};
			explicit value(UnitDefinition const& unit_p) : unit_m{ unit_p } {};
			explicit value(valueType_t V, UnitDefinition const& unit_p) :
				unit_m{ UnitDefinition(unit_p.unitType_m[0], unit_p.unitType_m[1], unit_p.unitType_m[2], unit_p.unitType_m[3], unit_p.unitType_m[4], false, ""/*unit_p.abbreviation_m*/, (valueType_t)unit_p.ratio_m, V) }
			{};
			explicit value(UnitDefinition&& unit_p) : unit_m{ std::forward<UnitDefinition>(unit_p) } {};
			value(value&& V) : unit_m{ std::move(V.unit_m) } {};
			value(value const& V) : unit_m{ V.unit_m.load() } {};
			value(double V) : unit_m{ UnitDefinition(0,0,0,0,0,true,"",1,V) } {};
			virtual ~value() = default;

		protected:
			virtual bool IsStaticType() const;

		public: // value operator
			explicit operator double() const noexcept;
			double operator()() const noexcept;

		public: // Functions
			std::string_view UnitName() const noexcept;
			void Clear();
			void Swap(value const& other) const;

		public:
			std::string Abbreviation(valueType_t* visibleValue = nullptr) const noexcept;
			std::string ToString() const;

		public: // Streaming functions (should be specialized per type)
			friend std::ostream& operator<<(std::ostream& os, value const& obj);
			friend std::stringstream& operator>>(std::stringstream& os, value& obj);

		public: // = Operators
			value& operator=(value const& other);

		public: // Comparison operators
			friend bool operator==(value const& A, value const& V) noexcept;
			friend bool operator<(value const& A, value const& V);
			friend bool operator<=(value const& A, value const& V);
			friend bool operator>(value const& A, value const& V);
			friend bool operator>=(value const& A, value const& V);
			friend bool operator!=(value const& A, value const& V) noexcept;

		public: // Unary operators
			value& operator++();
			value& operator--();
			value operator++(int);
			value operator--(int);

		public: // + and - Operators
			value operator-() const;

			friend value operator+(value const& A, value const& V);
			friend value operator-(value const& A, value const& V);
			value& operator+=(value const& V);
			value& operator-=(value const& V);

		public: // * and / Operators
			friend value operator*(value const& A, value const& V);
			friend value operator/(value const& A, value const& V);
			value& operator*=(value const& V);
			value& operator/=(value const& V);

		public: // pow and sqrt Operators
			// atomicly updates the value with a custom user-provided function.
			value& update(std::function<valueType_t(valueType_t)> const& updateFunction);
			// Creats a copy of the value and updates it with a custom user-provided function.
			value update(std::function<valueType_t(valueType_t)> const& updateFunction) const;
			// Returns a new value multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
			value pow(value const& V) const;
			// atomicly updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
			value& pow_value(value const& V);
			// pow(0.5)
			value sqrt() const;
			// atomicly floors (rounds to lower whole integer) the underlying value
			value& floor();
			// Creats a copy of the value and floors (rounds to lower whole integer) the underlying value
			value floor() const;
			// atomicly ceilings (rounds to upper whole integer) the underlying value
			value& ceiling();
			// Creats a copy of the value and ceilings (rounds to upper whole integer) the underlying value
			value ceiling() const;

		public:
			static std::vector<std::vector<std::tuple<std::string, std::string, Units::value, std::weak_ptr<GoodLang::Type_Info>>>> GetValueTypes() noexcept;

		};
		using scalar = value;




	};
};

namespace std {
	template<> class numeric_limits<GoodLang::Units::value_t> {
	public:
		static constexpr double min() { return std::numeric_limits<double>::min(); }
		static constexpr double max() { return std::numeric_limits<double>::max(); }
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); }
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer;
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed;
	};
};

namespace std {
	template<> class numeric_limits<GoodLang::Units::value> {
	public:
		static constexpr double min() { return std::numeric_limits<double>::min(); }
		static constexpr double max() { return std::numeric_limits<double>::max(); }
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); }
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer;
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed;
	};
};