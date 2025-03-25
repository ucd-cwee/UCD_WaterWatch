#pragma once
#include "Any.h"
#include "Units_Base.h"
#include "Units_Instances.h"
#include <map>

namespace GoodLang {
	// Units
	namespace Units {
		// lookup_abbreviation & lookup_typename
		static constexpr long double conversionConst = 1e8l;
		static constexpr uint64_t Encode(long double x) {
			return static_cast<uint64_t>(x * conversionConst);
		};
		static constexpr long double Decode(uint64_t x) {
			return static_cast<long double>(x) / conversionConst;
		};

		// Static functions
		static /*__forceinline*/ bool IsInteger(double value) {
			double intpart;
			return modf(value, &intpart) == 0.0;
		};
		static /*__forceinline*/ void removeTrailingCharacters(std::string& str, const char charToRemove) {
			str.erase(str.find_last_not_of(charToRemove) + 1, std::string::npos);
		};
		static /*__forceinline*/ void AddToDelimiter(std::string& obj, std::string const& toAdd, std::string const& delim) {
			if (obj.length() == 0) {
				obj += toAdd;
			}
			else {
				obj += delim;
				obj += toAdd;
			}
		};
		template <typename T> static constexpr T abs(T x) { return x > (T)0 ? x : -x; };

		namespace impl {
			using TreeType = GoodLang::Map<uint64_t, std::pair<Units::value, std::weak_ptr<GoodLang::Type_Info>>>; // look-up with unit ratio
			using ModelType = GoodLang::Map<size_t, TreeType>; // look-up with unit category

			static ModelType& Shared_Data() noexcept {
				static ModelType model;

				if (model.size() == 0) { // add all rows
#define CalculateMetricPrefixV(metric) \
	((long double)std::metric::num / (long double)std::metric::den)

#define DerivedUnitType(type, category, abbreviation, Ratio) \
    model.UniqueAt(HashUnits(Categories::category::unitType_m))->insert_or_assign(Encode( type ::conversion_ratio), std::pair<Units::value, std::weak_ptr<GoodLang::Type_Info>>{ type(), GoodLang::user_type_shared< type >() });

#define DerivedUnitTypeWithMetricPrefix(type, category, prefix) \
    model.UniqueAt(HashUnits(Categories::category::unitType_m))->insert_or_assign(Encode( prefix ## type ::conversion_ratio), std::pair<Units::value, std::weak_ptr<GoodLang::Type_Info>>{ prefix ## type(), GoodLang::user_type_shared< prefix ## type >() });

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
	DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, category, femto); \
	DerivedUnitTypeWithMetricPrefix(type, category, pico); \
	DerivedUnitTypeWithMetricPrefix(type, category, nano); \
	DerivedUnitTypeWithMetricPrefix(type, category, micro); \
	DerivedUnitTypeWithMetricPrefix(type, category, milli); \
	DerivedUnitTypeWithMetricPrefix(type, category, centi); \
	DerivedUnitTypeWithMetricPrefix(type, category, deci); \
	DerivedUnitTypeWithMetricPrefix(type, category, deca); \
	DerivedUnitTypeWithMetricPrefix(type, category, hecto); \
	DerivedUnitTypeWithMetricPrefix(type, category, kilo); \
	DerivedUnitTypeWithMetricPrefix(type, category, mega); \
	DerivedUnitTypeWithMetricPrefix(type, category, giga); \
	DerivedUnitTypeWithMetricPrefix(type, category, tera); \
	DerivedUnitTypeWithMetricPrefix(type, category, peta)

					DerivedUnitList; // this loops through the definitions for DerivedUnitTypeWithMetricPrefixes() and DerivedUnitType() for all units. Change thosse macro definitions to change the implimentations. 

#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitType
				}

				return model;
			};

			/*
			UnitHash determines the class of unit (length, time, length/time, length/time^2, length^1.25, etc.
			UnitRatio determines the specific ratio within that class (meter, foot, inch, etc.)
			*/
			static const std::pair< Units::value, std::weak_ptr<GoodLang::Type_Info>>& lookup_impl(size_t UnitHash, double& UnitRatio) noexcept {
				auto targetRatio = Encode(UnitRatio);

				static std::pair<Units::value, std::weak_ptr<GoodLang::Type_Info>> out{ Units::value(), std::weak_ptr<GoodLang::Type_Info>() };

				auto& model = Shared_Data();

				// Do the look-up
				{
					auto f = model.find(UnitHash);
					if (f != model.end()) {
						auto& curve = f->second;
						auto end = curve.end();

						auto iter1 = curve.FindLargestSmallerEqual(targetRatio);
						if ((iter1 != end) && (iter1->first == targetRatio)) {
							// exact find -- best case scenario
							UnitRatio = Decode(iter1->first);
							return iter1->second;
						}
						else {
							// not an exact find. 
							auto iter2 = curve.FindSmallestLargerEqual(targetRatio);
							if ((iter1 != end) && (iter2 != end)) {
								if (std::abs(static_cast<long double>(iter1->first) - static_cast<long double>(targetRatio)) < std::abs(static_cast<long double>(iter2->first) - static_cast<long double>(targetRatio))) {
									UnitRatio = Decode(iter1->first);
									return iter1->second;
								}
								else {
									UnitRatio = Decode(iter2->first);
									return iter2->second;
								}
							}
							else if (iter1 != end) {
								UnitRatio = Decode(iter1->first);
								return iter1->second;
							}
							else if (iter2 != end) {
								UnitRatio = Decode(iter2->first);
								return iter2->second;
							}
						}
					}
				}
				return out;
			};
		};

		Map<std::weak_ptr<GoodLang::Type_Info>, Units::value> value::GetValueTypes() noexcept {
			Map<std::weak_ptr<GoodLang::Type_Info>, Units::value> out;
			for (auto& x : impl::Shared_Data()) {
				for (auto& y : x.second) {
					const auto& T = y.second.second;
					const auto& v = y.second.first;
					out.try_emplace(T, v);
				}
			}
			return out;
		};

		namespace Definitions {
			// UnitDefinitionBase
			Number& UnitDefinitionBase::value() { return value_m; };
			const Number& UnitDefinitionBase::value() const { return value_m; };
			const char* UnitDefinitionBase::BuiltInName() const { return ""; };
			const char* UnitDefinitionBase::BuiltInAbbreviation() const { return ""; };
			bool UnitDefinitionBase::IsSI() const {
				auto& ut = unitType();
				return ((abs(ut[0]) + abs(ut[1]) + abs(ut[2]) + abs(ut[3]) + abs(ut[4])) == 1.0) && (abs(ratio()) == 1.0);
			};
			bool UnitDefinitionBase::IsScalar() const {
				auto& ut = unitType();
				return ((abs(ut[0]) + abs(ut[1]) + abs(ut[2]) + abs(ut[3]) + abs(ut[4])) == 0.0) && (abs(ratio()) == 1.0);
			};
			bool UnitDefinitionBase::IsSameCategory(UnitDefinitionBase const& other) const noexcept {
				if (IsScalar() && other.IsScalar()) return true;
				auto& ut1 = unitType();
				auto& ut2 = other.unitType();
				return std::memcmp(&ut1, &ut2, sizeof(ut1)) == 0;
			};
			bool UnitDefinitionBase::IsSameUnit(UnitDefinitionBase const& other) const noexcept {
				return IsSameCategory(other) && (ratio() == other.ratio());
			};
			size_t UnitDefinitionBase::HashCategory() const noexcept {
				auto& ut = unitType();
				return Units::HashUnits(ut[0], ut[1], ut[2], ut[3], ut[4]);
			};
			void UnitDefinitionBase::Clear() {
				this->value_m = 0;
			};

			// dynamicUnitDefinition
			std::unique_ptr<UnitDefinitionBase> dynamicUnitDefinition::Copy() const {
				return std::make_unique<dynamicUnitDefinition>(*this);
			}
			const std::array<double, UnitDefinitionBase::NumUnits>& dynamicUnitDefinition::unitType() const {
				return unitType_m;
			};
			const double& dynamicUnitDefinition::ratio() const {
				return ratio_m;
			};
			std::array<double, UnitDefinitionBase::NumUnits>& dynamicUnitDefinition::unitType() {
				return unitType_m;
			};
			double& dynamicUnitDefinition::ratio() {
				return ratio_m;
			};
			void dynamicUnitDefinition::Clear() {
				this->value_m = 0;
				this->ratio_m = 0;
				for (auto& x : this->unitType_m) x = 0;
			};

			// scalarDefinition
			const std::array<double, UnitDefinitionBase::NumUnits>& scalarDefinition::unitType() const { return unitType_m; };
			const double& scalarDefinition::ratio() const { return ratio_m; };
			const char* scalarDefinition::BuiltInName() const { return ""; };
			const char* scalarDefinition::BuiltInAbbreviation() const { return ""; };
			std::unique_ptr<UnitDefinitionBase> scalarDefinition::Copy() const {
				typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;
				return std::make_unique<thisType>(*this);
			};
		};

		// Performs the look-up for all built-in types, including (if needed)
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
					bool lookupFailed = true;
					// I am not a scalar, and I am a non-static (e.g. dynamic) variable.
					Number ratio_to_use = Data.ratio();

					auto lookup = impl::lookup_impl(HashUnits(Data.unitType()), ratio_to_use);
					auto result = impl::value_accessor::GetUnits(std::get<0>(lookup)).Shared();
					toReturn = result->BuiltInAbbreviation();
					if (toReturn != "") {
						lookupFailed = false;
						ratio_to_use = result->ratio();
					}

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
		static /*__forceinline*/ Number GetVisibleValue(value const& V) noexcept {
			Number out{ 0 };
			auto Data = impl::value_accessor::GetUnits(V).Shared();
			(void)Abbreviation(*Data, &out);
			return out;
		};
		static /*__forceinline*/ bool IdenticalUnits(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) noexcept { return LHS.IsSameCategory(RHS); };
		static /*__forceinline*/ bool NormalArithmeticOkay(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) noexcept {
			if (LHS.IsScalar() || RHS.IsScalar()) return true;
			if (LHS.IsSameCategory(RHS)) return true;
			return false;
		};
		static /*__forceinline*/ bool UnaryArithmeticOkay(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) noexcept {
			if (RHS.IsScalar()) return true;
			if (LHS.IsSameCategory(RHS)) return true;
			return false;
		};
		static /*__forceinline*/ void HandleNormalArithmetic(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) {
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
		static /*__forceinline*/ void HandleUnaryArithmetic(Definitions::UnitDefinitionBase const& LHS, Definitions::UnitDefinitionBase const& RHS) {
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
		static /*__forceinline*/ void HandleNotScalar(Definitions::UnitDefinitionBase const& V) {
			if (V.IsScalar()) return;
			else {
				auto A1 = Abbreviation(V, nullptr);
				throw(std::runtime_error(GoodLang::printf("Type must be scalar (was '%s').",
					A1.c_str()
				)));
			}
		};
		static /*__forceinline*/ std::string GetValueStr(Number const& v) noexcept {
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
		template<typename Func> static /*__forceinline*/ bool DoComparison(Func const& toDo, value const& A, value const& V) noexcept {
			auto Data1 = impl::value_accessor::GetUnits(A).Shared();
			auto Data2 = impl::value_accessor::GetUnits(V).Shared();

			if (!NormalArithmeticOkay(*Data1, *Data2)) return false; // we aren't the same category -- just early-exit

			// we are the same category
			if (Data1->IsScalar() == Data2->IsScalar()) {
				return toDo(Data1->value(), Data2->value());
			}
			else if (Data2->IsScalar()) {
				value x{ A };
				x = V;
				return toDo(Data1->value(), impl::value_accessor::GetUnits(x).Unique()->value());
			}
			else {
				// LHS is a scalar. 
				value x{ V };
				x = A;
				return toDo(Data2->value(), impl::value_accessor::GetUnits(x).Unique()->value());
			}
		};
		template<typename Func> static /*__forceinline*/ void DoUnaryOperation(Func const& toDo, value& A, value const& V) {
			auto Data = impl::value_accessor::GetUnits(A).Unique();
			auto other = impl::value_accessor::GetUnits(V).Shared();

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
				value temp(Data->Copy());
				temp = value(other->Copy());
				(void)toDo(Data->value(), impl::value_accessor::GetUnits(temp).Unique()->value());
			}
		};
		template <typename Func> static /*__forceinline*/ value AddOrSubtract(Func const& toDo, value const& a, value const& b) {
			auto a_struct = impl::value_accessor::GetUnits(a).Shared();
			auto b_struct = impl::value_accessor::GetUnits(b).Shared();

			if (a_struct->IsSameCategory(*b_struct)) { // same category, but perhaps different conversion factor / ratio. That's OK. 
				return value(toDo(a_struct->value(), b_struct->value()) * a_struct->ratio(), *a_struct);
			}
			else if (b_struct->IsScalar()) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				return value(toDo(a_struct->value(), ((b_struct->value() / b_struct->ratio()) * a_struct->ratio())) * a_struct->ratio(), *a_struct);
			}
			else if (a_struct->IsScalar()) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
				return value(toDo(b_struct->value(), ((a_struct->value() / a_struct->ratio()) * b_struct->ratio())) * b_struct->ratio(), *b_struct);
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
		static /*__forceinline*/ value Add(value const& a, value const& b) {
			return AddOrSubtract([](Number const& lhs, Number const& rhs) -> Number { return lhs + rhs; }, a, b);
		};
		static /*__forceinline*/ value Sub(value const& a, value const& b) {
			return AddOrSubtract([](Number const& lhs, Number const& rhs) -> Number { return lhs - rhs; }, a, b);
		};
		/* Used for multiplication or division operations */
		static /*__forceinline*/ value CompoundUnits(value const& LHS, value const& RHS, bool multiplication = true) {
			// if we are multiplying or dividing by a scalar, then we can simply do a Unary operation
			auto lhs = impl::value_accessor::GetUnits(LHS).Shared();
			auto rhs = impl::value_accessor::GetUnits(RHS).Shared();

			bool lhs_is_scalar = lhs->IsScalar();
			bool rhs_is_scalar = rhs->IsScalar();

			// early-exit if the RHS is a scalar, which will not change the units of the LHS
			if (rhs_is_scalar) {
				if (multiplication) {
					return value((lhs->value() * rhs->value()) * lhs->ratio(), *lhs);
				}
				else {
					return value((lhs->value() / rhs->value()) * lhs->ratio(), *lhs);
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

			return value(std::move(ptr));
		};
		static /*__forceinline*/ value Multiply(value const& LHS, value const& RHS) {
			return CompoundUnits(LHS, RHS, true);
		};
		static /*__forceinline*/ value Divide(value const& LHS, value const& RHS) {
			return CompoundUnits(LHS, RHS, false);
		};
		static /*__forceinline*/ value MultiplyUnits(value const& LHS, double RHS) {
			if (RHS == 1.0) {
				return LHS;
			}
			else {
				auto Data = impl::value_accessor::GetUnits(LHS).Shared();
				if (Data->IsScalar()) {
					return value(std::pow(Data->value() / Data->ratio(), RHS) * Data->ratio() * Data->ratio(), *Data);
				}
				else {
					return value(std::make_unique<Definitions::dynamicUnitDefinition>(
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

		value::operator Number() const noexcept { return GetVisibleValue(*this); };
		Number value::operator()() const noexcept { return GetVisibleValue(*this); };
		std::string value::ToString() const {
			Number out{ 0.0 };
			auto Data = this->unit_m.Shared();
			std::string abbreviation{ Abbreviation(*Data, &out) };
			if (abbreviation.length() > 0) return GetValueStr(out) + " " + abbreviation;
			else return GetValueStr(out);
		};
		std::ostream& operator<<(std::ostream& os, value const& obj) { os << obj.ToString(); return os; };;
		std::string_view value::UnitName() const noexcept {
			auto Data = this->unit_m.Shared();
			if (Data->IsScalar()) {
				return "";
			}
			else {
				std::string_view toReturn{ Data->BuiltInName() };
				if (toReturn.empty()) {
					double ratio = Data->ratio();
					auto lookup = impl::lookup_impl(HashUnits(Data->unitType()), ratio);
					auto result = std::get<0>(lookup).unit_m.Shared();
					toReturn = result->BuiltInName();
				}
				if (toReturn.empty()) 
					return "Value";
				else 
					return toReturn;
			}
		};
		std::string value::UnitAbbreviation() const noexcept {
			auto Data = unit_m.Shared();
			return Abbreviation(*Data, nullptr);
		};
		void value::Clear() {
			auto Data = unit_m.Unique();
			Data->Clear();
		};
		value& value::operator=(value const& other) {
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
				auto A1 = Abbreviation(*Data, nullptr);
				auto A2 = Abbreviation(*V, nullptr);
				throw(std::runtime_error(GoodLang::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.",
					A1.c_str(),
					A2.c_str()
				)));
			}
			return *this;
		};		
		bool operator==(value const& A, value const& V) noexcept {
			return DoComparison([](Number LHS, Number RHS) -> bool {
				return LHS == RHS;
				}, A, V);
		};
		bool operator<(value const& A, value const& V) {
			return DoComparison([](Number LHS, Number RHS) -> bool {
				return LHS < RHS;
				}, A, V);
		};
		bool operator<=(value const& A, value const& V) {
			return DoComparison([](Number LHS, Number RHS) -> bool {
				return LHS <= RHS;
				}, A, V);
		};
		bool operator>(value const& A, value const& V) { return !(A <= V); };
		bool operator>=(value const& A, value const& V) { return !(A < V); };
		bool operator!=(value const& A, value const& V) noexcept { return !(operator==(A, V)); };
		value& value::operator++() {
			auto Data = this->unit_m.Unique();
			Data->value() += Data->ratio();			
			return *this;
		};
		value& value::operator--() {
			auto Data = this->unit_m.Unique();
			Data->value() -= Data->ratio();
			return *this;
		};
		value& value::operator+=(value const& V) {
			DoUnaryOperation([](Number& lhs, Number const& rhs) { lhs += rhs; }, *this, V);
			return *this;
		};
		value& value::operator-=(value const& V) {
			DoUnaryOperation([](Number& lhs, Number const& rhs) { lhs -= rhs; }, *this, V);
			return *this;
		};
		value& value::operator*=(value const& V) {
			auto other = V.unit_m.Shared();
			HandleNotScalar(*other);
			auto Data = this->unit_m.Unique();
			Data->value() *= other->value();
			return *this;
		};
		value& value::operator/=(value const& V) {
			auto other = V.unit_m.Shared();
			HandleNotScalar(*other);
			auto Data = this->unit_m.Unique();
			Data->value() /= other->value();
			return *this;
		};
		[[nodiscard]] value value::operator++(int) {
			auto Data = this->unit_m.Unique();
			value out{ Data->Copy() };
			Data->value() += Data->ratio();
			return out;
		};
		[[nodiscard]] value value::operator--(int) {
			auto Data = this->unit_m.Unique();
			value out{ Data->Copy() };
			Data->value() -= Data->ratio();
			return out;
		};
		value operator+(value const& A, value const& B) {
			return Add(A, B);
		};
		value operator-(value const& A, value const& B) {
			return Sub(A, B);
		};
		value operator*(value const& A, value const& V) {
			return Multiply(A, V);
		};
		value operator/(value const& A, value const& V) {
			return Divide(A, V);
		};
		value value::operator-() const {
			return Multiply(*this, -1);
		};
		// atomicly updates the value with a custom user-provided function.
		value& value::update(std::function<double(double)> const& updateFunction) {
			auto Data = this->unit_m.Unique();
			Data->value() = updateFunction(Data->value() / Data->ratio()) * Data->ratio();
			return *this;
		};
		// Creats a copy of the value and updates it with a custom user-provided function.
		value value::update(std::function<double(double)> const& updateFunction) const {
			auto Data = this->unit_m.Shared();
			return value(updateFunction(Data->value() / Data->ratio()) * Data->ratio() * Data->ratio(), *Data);
		};
		// Returns a new value multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
		value value::pow(value const& V) const {
			auto other = V.unit_m.Shared();
			HandleNotScalar(*other);
			return MultiplyUnits(*this, other->value());
		};
		// atomicly updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
		value& value::pow_value(value const& V) {
			auto Data = this->unit_m.Unique();
			auto Other = V.unit_m.Shared();
			HandleNotScalar(*Other);
			Data->value() = std::pow(Data->value() / Data->ratio(), Other->value()) * Data->ratio();
			return *this;
		};
		// atomicly updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
		value value::pow_value(value const& V) const {
			auto Other = V.unit_m.Shared();
			HandleNotScalar(*Other);
			return this->update([&Other](double x)->double { return std::pow(x, Other->value()); });
		};
		// pow(0.5)
		value value::sqrt() const {
			return pow(0.5);
		};
		// atomicly floors (rounds to lower whole integer) the underlying value
		value& value::floor() {
			return update([](double v)->double { return std::floor(v); });
		};
		// Creats a copy of the value and floors (rounds to lower whole integer) the underlying value
		value value::floor() const {
			return update([](double v)->double { return std::floor(v); });
		};
		// atomicly ceilings (rounds to upper whole integer) the underlying value
		value& value::ceiling() {
			return update([](double v)->double { return std::ceil(v); });
		};
		// Creats a copy of the value and ceilings (rounds to upper whole integer) the underlying value
		value value::ceiling() const {
			return update([](double v)->double { return std::ceil(v); });
		};
		// return a unique hash for the unit/value combo
		size_t value::hash() const {
			auto Data = this->unit_m.Shared();
			size_t out{ 37 };
			GoodLang::details::hash_combine(
				out, 
				Data->value(), 
				Data->unitType()[0], 
				Data->unitType()[1],
				Data->unitType()[2],
				Data->unitType()[3],
				Data->unitType()[4]
			);
			return out;
		};

	};

};
