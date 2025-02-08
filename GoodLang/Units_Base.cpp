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

#define CreateRow(model, Type) { model->second[Type::UnitHash()][Encode(Type::conversion)] = model->first.Alloc(std::tuple< std::string_view, std::string_view, Units::value, std::weak_ptr<GoodLang::Type_Info>>{ Type::specialized_abbreviation, std::string_view(#Type), Type(), GoodLang::user_type_shared<Type>() }); }
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

		static std::pair<std::shared_mutex, std::shared_ptr<void>>& Shared_Data() noexcept {
			static std::pair<std::shared_mutex, std::shared_ptr<void>> out;
			return out;
		};
		/*
		UnitHash determines the class of unit (length, time, length/time, length/time^2, length^1.25, etc.
		UnitRatio determines the specific ratio within that class (meter, foot, inch, etc.)
		*/
		static std::tuple< std::string_view, std::string_view, Units::value, std::weak_ptr<GoodLang::Type_Info>>& lookup_impl(size_t UnitHash, double& UnitRatio) noexcept {
			auto targetRatio = Encode(UnitRatio);

			static std::tuple<std::string_view, std::string_view, Units::value, std::weak_ptr<GoodLang::Type_Info>> out{ "", "", Units::value(), std::weak_ptr<GoodLang::Type_Info>() };
			auto& [mut, Tag] = Shared_Data();

			using AllocType = GoodLang::utilities::FastAllocator<std::tuple< std::string_view, std::string_view, Units::value, std::weak_ptr<GoodLang::Type_Info>>>;
			using TreeType = std::map<uint64_t, std::tuple< std::string_view, std::string_view, Units::value, std::weak_ptr<GoodLang::Type_Info>>*>;
			using ModelType = std::pair< AllocType, std::map<size_t, TreeType>>;

			auto FindLargestSmallerEqual = [](TreeType const& f, uint64_t pos) {
				auto ptr = f.lower_bound(pos); // equal to or larger than pos
				if (ptr != f.end()) {
					// map is not empty, AND we either found the actual key, or the one JUST larger than it 
					if (ptr->first == pos) {
						// DONE
					}
					else {
						// we are beyond our goal -- move back one. 
						ptr = std::prev(ptr);

						// what if this was actually the first position? 
						if (ptr == f.end()) {
							ptr = f.begin();
						}
					}
				}
				else {
					if (!f.empty()) {
						ptr = std::prev(f.end()); // get what you get
					}
				}
				return ptr;
			};
			auto FindSmallestLargerEqual = [](TreeType const& f, uint64_t pos) {
				return f.lower_bound(pos); // equal to or larger than pos				
			};

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

			{
				auto locked{ std::shared_lock(mut) };
				if (model && model->second.count(UnitHash) > 0) {
					auto& curve = model->second.at(UnitHash);
					// defer(curve.TryCleanupUnusedMemory());
					// auto iter1 = curve.FindLargestSmallerEqual(targetRatio);
					auto iter1 = FindLargestSmallerEqual(curve, targetRatio);
					if ((iter1 != curve.end()) && iter1->first == targetRatio) {
						// exact find -- best case scenario
						UnitRatio = Decode(iter1->first);
						return *iter1->second;
					}
					else {
						// not an exact find. 
						// auto iter2 = curve.FindSmallestLargerEqual(targetRatio);
						auto iter2 = FindSmallestLargerEqual(curve, targetRatio);
						if (iter1 != curve.end() && iter2 != curve.end()) {
							if (std::abs(static_cast<long double>(iter1->first) - static_cast<long double>(targetRatio)) < std::abs(static_cast<long double>(iter2->first) - static_cast<long double>(targetRatio))) {
								UnitRatio = Decode(iter1->first);
								return *iter1->second;
							}
							else {
								UnitRatio = Decode(iter2->first);
								return *iter2->second;
							}
						}
						else if (iter1 != curve.end()) {
							UnitRatio = Decode(iter1->first);
							return *iter1->second;
						}
						else if (iter2 != curve.end()) {
							UnitRatio = Decode(iter2->first);
							return *iter2->second;
						}
					}
				}
			}
			return out;
		};
		std::vector<std::vector<std::tuple<std::string, std::string, Units::value, std::weak_ptr<GoodLang::Type_Info>>>> value::GetValueTypes() noexcept {
			static double temp{ 0 };
			static auto temp2{ lookup_impl(0, temp) };

			std::vector<
				std::vector<
				std::tuple<
				std::string,
				std::string,
				Units::value,
				std::weak_ptr<GoodLang::Type_Info>
				>
				>
			> out;

			auto& [mut, Tag] = Shared_Data();

			using AllocType = GoodLang::utilities::FastAllocator<std::tuple< std::string_view, std::string_view, Units::value, std::weak_ptr<GoodLang::Type_Info>>>;
			using TreeType = std::map<uint64_t, std::tuple< std::string_view, std::string_view, Units::value, std::weak_ptr<GoodLang::Type_Info>>*>;
			using ModelType = std::pair< AllocType, std::map<size_t, TreeType>>;

			auto locked{ std::shared_lock(mut) };
			if (std::shared_ptr < ModelType > model = std::static_pointer_cast<ModelType>(Tag)) {
				for (auto& typeValue : model->second) {
					std::vector<
						std::tuple<
						std::string,
						std::string,
						Units::value,
						std::weak_ptr<GoodLang::Type_Info>
						>
					> temp;

					for (auto& each : typeValue.second) {
						std::string_view abbrev = std::get<0>(*each.second);
						std::string_view fullName = std::get<1>(*each.second);
						Units::value& impl = std::get<2>(*each.second);
						temp.push_back(std::tuple<std::string, std::string, Units::value, std::weak_ptr<GoodLang::Type_Info>>(
							abbrev,
							fullName,
							impl,
							std::get<3>(*each.second)
						));
					}

					out.push_back(temp);
				}
			}
			return out;
		};
#undef CreateRowWithMetricPrefixes
#undef CreateRow
		std::string_view lookup_abbreviation(size_t UnitHash, double& UnitRatio) noexcept {
			return std::get<0>(lookup_impl(UnitHash, UnitRatio));
		};
		std::string_view lookup_typename(size_t UnitHash, double& UnitRatio) noexcept {
			return std::get<1>(lookup_impl(UnitHash, UnitRatio));
		};

		// Static functions
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

		// UnitDefinition
		bool UnitDefinition::IsSI() const {
			return (abs(unitType_m[0]) + abs(unitType_m[1]) + abs(unitType_m[2]) + abs(unitType_m[3]) + abs(unitType_m[4])) == 1.0f
				&& abs((double)ratio_m) == 1.0;
		};
		bool UnitDefinition::IsScalar() const {
			return (abs(unitType_m[0]) + abs(unitType_m[1]) + abs(unitType_m[2]) + abs(unitType_m[3]) + abs(unitType_m[4])) == 0.0f
				&& abs((double)ratio_m) == 1.0;
		};
		bool UnitDefinition::IsSameCategory(UnitDefinition const& other) const noexcept {
			if (IsScalar() && other.IsScalar()) return true;
			return std::memcmp(&unitType_m, &other.unitType_m, sizeof(unitType_m)) == 0;
		};
		bool UnitDefinition::IsSameUnit(UnitDefinition const& other) const noexcept {
			return IsSameCategory(other) && (ratio_m == other.ratio_m);
		};
		size_t UnitDefinition::HashCategory() const noexcept {
			return Units::HashUnits(unitType_m[0], unitType_m[1], unitType_m[2], unitType_m[3], unitType_m[4]);
		};
		std::pair<std::string_view, valueType_t> UnitDefinition::LookupAbbreviation(bool isStatic) const noexcept {
			double ratio_bestFit = (double)ratio_m;
			std::string_view abbrev_bestFit = Units::lookup_abbreviation(HashCategory(), ratio_bestFit);
			return { abbrev_bestFit, ratio_bestFit };
		};
		std::string_view UnitDefinition::LookupTypeName() const noexcept {
			double ratio_bestFit = (double)ratio_m;
			std::string_view TypeName_bestFit = Units::lookup_typename(HashCategory(), ratio_bestFit);
			return TypeName_bestFit;
		};
		std::string UnitDefinition::CreateAbbreviation(bool isStatic) const noexcept {
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
									Num = std::to_string(v);
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
										Num = std::to_string((-1.0 * v));
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
			return "";
		};

		// Value
		bool value::IsStaticType() const { return false; };
		valueType_t GetVisibleValue(value const& V) noexcept {
			valueType_t out{ 0. };
			V.Abbreviation(&out);
			return out;
		};
		std::string GetValueStr(valueType_t const& v) noexcept {
			if (std::fmod((double)v, 1.0) == 0.0) { // integer
				return std::to_string((long long)v);
			}
			else { // floating-point
				std::string out{ std::to_string((double)v) /*Units::printf("%.4f", v)*/ };
				Units::removeTrailingCharacters(out, '0'); // e.g. 25.5000 -> 25.5
				Units::removeTrailingCharacters(out, '.'); // e.g. 25.0000 -> 25. -> 25
				return out;
			}
		};
		bool IdenticalUnits(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept { return LHS.IsSameCategory(RHS); };
		bool is_scalar(UnitDefinition const& V) noexcept { return V.IsScalar(); };
		bool NormalArithmeticOkay(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept {
			if (is_scalar(LHS) || is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		bool UnaryArithmeticOkay(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept {
			if (is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		std::string AbbreviationFast(UnitDefinition const& V) noexcept {
			std::string toReturn{ /*V.abbreviation_m*/ };

			if (V.IsScalar() && toReturn.empty()) {
				auto [abbreviation, ratio] = V.LookupAbbreviation(false);
				toReturn = abbreviation;
				if (!V.IsScalar() && toReturn.empty()) {
					constexpr static std::array< std::string_view, UnitDefinition::NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

					bool anyNegatives = false;
					std::string Num;
					for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
						decltype(auto) unitBase = unitBases[i];
						decltype(auto) v = V.unitType_m[i];

						if (v > 0) {
							if (v == 1)
								AddToDelimiter(toReturn, unitBase.data(), " ");
							else {
								if (IsInteger(v)) {
									Num = std::to_string((int)v);
								}
								else {
									Num = std::to_string(v);
									removeTrailingCharacters(Num, '0');
									removeTrailingCharacters(Num, '.');
								}
								AddToDelimiter(toReturn, printf("%s^%s", unitBase.data(), Num.c_str()), " ");
							}
						}
						else if (v < 0) {
							anyNegatives = true;
						}
					}
					if (anyNegatives) {
						toReturn += " /";
						for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
							decltype(auto) unitBase = unitBases[i];
							decltype(auto) v = V.unitType_m[i];

							if (v < 0) {
								if (v == -1)
									AddToDelimiter(toReturn, unitBase.data(), " ");
								else {
									if (IsInteger(v)) {
										Num = std::to_string((int)(-1.0 * v));
									}
									else {
										Num = std::to_string((-1.0 * v));
										removeTrailingCharacters(Num, '0');
										removeTrailingCharacters(Num, '.');
									}
									AddToDelimiter(toReturn, printf("%s^%s", unitBase.data(), Num.c_str()), " ");
								}
							}
						}
					}
				}
			}

			return toReturn;
		};
		void HandleNormalArithmetic(UnitDefinition const& LHS, UnitDefinition const& RHS) {
			if (NormalArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(GoodLang::printf("Normal, dynamic arithmetic failed due to incompatible non-scalar value: '%s' and '%s'", AbbreviationFast(LHS).c_str(), AbbreviationFast(RHS).c_str())));
		};
		void HandleUnaryArithmetic(UnitDefinition const& LHS, UnitDefinition const& RHS) {
			if (UnaryArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(GoodLang::printf("Unary (in-place or self-modifying) arithmetic failed due to incompatible value: '%s' and '%s'", AbbreviationFast(LHS).c_str(), AbbreviationFast(RHS).c_str())));
		};
		void HandleNotScalar(UnitDefinition const& V) {
			if (is_scalar(V)) return;
			throw(std::runtime_error(GoodLang::printf("Type must be scalar (was '%s').", AbbreviationFast(V).c_str())));
		};
		
		value::operator double() const noexcept { return (double)GetVisibleValue(*this); };
		double value::operator()() const noexcept { return (double)GetVisibleValue(*this); };
		std::string_view value::UnitName() const noexcept {
			std::string_view toReturn{ "" };
			unit_m.Update([&toReturn, this](UnitDefinition Data)->UnitDefinition {
				auto [abbreviation, ratio] = Data.LookupAbbreviation(this->IsStaticType());

				Data.ratio_m = ratio;

				toReturn = Data.LookupTypeName();

				return Data;
			});
			return toReturn;
		};
		void value::Clear() {
			unit_m.store(UnitDefinition{});
		};
		void value::Swap(value const& other) const {
			unit_m.Swap(other.unit_m.load());
		};
		std::string value::Abbreviation(valueType_t* visibleValue) const noexcept {
			bool isStatic{ IsStaticType() };
			std::string toReturn{ "" };

			{
				unit_m.Update([isStatic, &toReturn, this, &visibleValue](UnitDefinition Data)->UnitDefinition {
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

							constexpr static std::array< std::string_view, UnitDefinition::NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

							bool anyNegatives = false;
							std::string Num;
							for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
								std::string_view unitBase = unitBases[i];
								double v = Data.unitType_m[i];

								if (v > 0) {
									if (v == 1)
										AddToDelimiter(toReturn, unitBase.data(), " ");
									else {
										if (IsInteger(v)) {
											Num = std::to_string(static_cast<int>(v));
										}
										else {
											Num = std::to_string(v);
											removeTrailingCharacters(Num, '0');
											removeTrailingCharacters(Num, '.');
										}
										AddToDelimiter(toReturn, printf("%s^%s", unitBase.data(), Num.c_str()), " ");
									}
								}
								else if (v < 0) {
									anyNegatives = true;
								}
							}
							if (anyNegatives) {
								toReturn += " /";
								for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
									std::string_view unitBase = unitBases[i];
									double v = Data.unitType_m[i];

									if (v < 0) {
										if (v == -1)
											AddToDelimiter(toReturn, unitBase.data(), " ");
										else {
											if (IsInteger(v)) {
												Num = std::to_string(static_cast<int>(-1.0 * v));
											}
											else {
												Num = std::to_string((-1.0 * v));
												removeTrailingCharacters(Num, '0');
												removeTrailingCharacters(Num, '.');
											}
											AddToDelimiter(toReturn, printf("%s^%s", unitBase.data(), Num.c_str()), " ");
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
		std::string value::ToString() const {
			valueType_t out{ 0.0 };
			std::string abbreviation{ Abbreviation(&out) };
			if (abbreviation.length() > 0) return GetValueStr(out) + " " + abbreviation;
			else return GetValueStr(out);
		};
		value& MultiplyUnits(value& This, valueType_t const& V) noexcept {
			This.unit_m.Update([&V](UnitDefinition Data)->UnitDefinition {
				if (V == 1.0 || is_scalar(Data)) {

					Data.value_m = std::pow((double)(Data.value_m / Data.ratio_m), (double)V) * Data.ratio_m; // save in SI value

					return Data;
				}
				for (int i = Data.unitType_m.size() - 1; i >= 0; i--) Data.unitType_m[i] *= (double)V;
				// if (V == 0) Data.IsScalar() = true;

				// remove the abbreviation since we either don't know what we are or we will become empty anyhow.
				// Data.abbreviation_m = const_cast<char*>("");

				// now that we have modified the value, the conversion ratio makes no sense anymore and must be reset. 
				Data.ratio_m = std::pow((double)Data.ratio_m, (double)V);

				// do the exonentiation of the value
				// i.e. (10 (ft)) ^ (3) -> (1000 (cu_ft)) * (1 / 35.3147 (cu_m/cu_ft)) -> 28.3168 cu_m in SI value
				// Data.value_m = std::pow(Data.value_m / Data.ratio_m, V) * Data.ratio_m; // save in SI value
				Data.value_m = std::pow((double)Data.value_m, (double)V);
				return Data;
			});
			return This;
		};
		value& value::operator=(value const& other) {
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
		value& value::operator++() {
			auto r{ unit_m.Read() };
			r->value_m += r->ratio_m;
			return *this;
		};
		value& value::operator--() {
			auto r{ unit_m.Read() };
			r->value_m -= r->ratio_m;
			return *this;
		};
		value value::operator++(int) {
			auto r{ unit_m.Read() };
			value out{ *r };
			r->value_m += r->ratio_m;
			return out;
		};
		value value::operator--(int) {
			auto r{ unit_m.Read() };
			value out{ *r };
			r->value_m -= r->ratio_m;
			return out;
		};
		value Add(value const& a, value const& b) {
			auto a_struct = a.unit_m.load();
			auto b_struct = b.unit_m.load();

			if (a_struct.IsSameCategory(b_struct)) { // same category, but perhaps different conversion factor. That's OK. 
				return value(UnitDefinition(
					a_struct.unitType_m,
					a_struct.ratio_m,
					a_struct.value_m + b_struct.value_m
				));
			}
			else if (is_scalar(b_struct)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				return value(UnitDefinition(
					a_struct.unitType_m,
					a_struct.ratio_m,
					a_struct.value_m + ((b_struct.value_m / b_struct.ratio_m) * a_struct.ratio_m)
				));
			}
			else if (is_scalar(a_struct)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
				return value(UnitDefinition(
					b_struct.unitType_m,
					b_struct.ratio_m,
					b_struct.value_m + ((a_struct.value_m / a_struct.ratio_m) * b_struct.ratio_m)
				));
			}
			else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
				throw(std::runtime_error(GoodLang::printf("Add operation failed due to incompatible non-scalar value: '%s' and '%s'.", a.Abbreviation().c_str(), b.Abbreviation().c_str())));
			}
		};
		value Sub(value const& a, value const& b) {
			auto a_struct = a.unit_m.load();
			auto b_struct = b.unit_m.load();

			if (a_struct.IsSameCategory(b_struct)) { // same category, but perhaps different conversion factor. That's OK. 
				return value(UnitDefinition(
					a_struct.unitType_m,
					a_struct.ratio_m,
					a_struct.value_m - b_struct.value_m
				));
			}
			else if (is_scalar(b_struct)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				return value(UnitDefinition(
					a_struct.unitType_m,
					a_struct.ratio_m,
					a_struct.value_m - ((b_struct.value_m / b_struct.ratio_m) * a_struct.ratio_m)
				));
			}
			else if (is_scalar(a_struct)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
				return value(UnitDefinition(
					b_struct.unitType_m,
					b_struct.ratio_m,
					b_struct.value_m - ((a_struct.value_m / a_struct.ratio_m) * b_struct.ratio_m)
				));
			}
			else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
				throw(std::runtime_error(GoodLang::printf("Subtract operation failed due to incompatible non-scalar value: '%s' and '%s'.", a.Abbreviation().c_str(), b.Abbreviation().c_str())));
			}
		};

		/* Used for multiplication or division operations */
		void CompoundUnits(value& This, value const& other, bool multiplication = true) noexcept {
			UnitDefinition V{ other.unit_m.load() };
			This.unit_m.Update([&V, multiplication](UnitDefinition Data)->UnitDefinition {
				bool 
					V_Is_Scalar{ V.IsScalar() }, 
					I_am_Scalar{ Data.IsScalar() };

				if (V_Is_Scalar) {
					if (multiplication) {
						Data.value_m *= V.value_m;
					}
					else {
						Data.value_m /= V.value_m;
					}
					return Data; // do nothing
				}

				// V is not a scaler, but I could become one.
				if (multiplication) {
					for (int i = Data.unitType_m.size() - 1; i >= 0; i--)
						Data.unitType_m[i] += V.unitType_m[i];					
				}
				else {
					for (int i = Data.unitType_m.size() - 1; i >= 0; i--) 
						Data.unitType_m[i] -= V.unitType_m[i];					
				}

				// now that we have modified the unit type (length, time, etc.), the conversion ratio makes no sense anymore (e.g. within length, is it a foot, meter, yard, etc.)
				if (multiplication) {
					Data.ratio_m *= V.ratio_m;
				}
				else {
					Data.ratio_m /= V.ratio_m;
				}

				// unitless values cannot have "ratios" -- there are not alternatives of "unitless". 
				if (Data.IsScalar()) {
					Data.ratio_m = 1;
				}

				if (multiplication) {
					Data.value_m *= V.value_m;
				}
				else {
					Data.value_m /= V.value_m;
				}
				
				return Data;
			});
		};

		value Multiply(value const& A, value const& V) {
			value out = A;
			CompoundUnits(out, V, true);
			return out;
		};
		value Divide(value const& A, value const& V) {
			value out = A;
			CompoundUnits(out, V, false);
			return out;
		};
		value value::operator-() const { return -1.0 * (*this); };
		value& value::operator+=(value const& V) {
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
		value& value::operator-=(value const& V) {
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
		value& value::operator*=(value const& V) {
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([&other](UnitDefinition Data)->UnitDefinition {
				Data.value_m *= other.value_m;
				return Data;
				});
			return *this;
		};
		value& value::operator/=(value const& V) {
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([&other](UnitDefinition Data)->UnitDefinition {
				Data.value_m /= other.value_m;
				return Data;
				});
			return *this;
		};
		value& value::update(std::function<valueType_t(valueType_t)> const& updateFunction) {
			unit_m.Update([&updateFunction](UnitDefinition Data)->UnitDefinition {
				Data.value_m = updateFunction(Data.value_m / Data.ratio_m) * Data.ratio_m;
				return Data;
			});
			return *this;
		};
		value value::update(std::function<valueType_t(valueType_t)> const& updateFunction) const {
			auto out{ value(*this) };
			out.update(updateFunction);
			return out;
		};
		value value::pow(value const& V) const {
			auto other{ V.unit_m.load() };

			HandleNotScalar(other);

			value out = *this;
			MultiplyUnits(out, other.value_m);

			return out;
		};
		value& value::pow_value(value const& V) {
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([&other](UnitDefinition Data)->UnitDefinition {
				Data.value_m = std::pow((double)(Data.value_m / Data.ratio_m), (double)(other.value_m / other.ratio_m)) * Data.ratio_m;
				return Data;
			});
			return *this;
		};
		value value::sqrt() const {
			return pow(0.5);
		};
		value& value::floor() {
			return update([](valueType_t v)->valueType_t { return std::floor((double)v); });
		};
		value value::floor() const {
			auto out{ value(*this) };
			out.floor();
			return out;
		};
		value& value::ceiling() {
			return update([](valueType_t v)->valueType_t { return std::ceil((double)v); });
		};
		value value::ceiling() const {
			auto out{ value(*this) };
			out.ceiling();
			return out;
		};

		bool operator==(value const& A, value const& V)  noexcept {
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
		bool operator<(value const& A, value const& V) {
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
		bool operator<=(value const& A, value const& V) {
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
		bool operator>(value const& A, value const& V) { return !(A <= V); };
		bool operator>=(value const& A, value const& V) { return !(A < V); };
		bool operator!=(value const& A, value const& V) noexcept { return !(operator==(A, V)); };
		std::ostream& operator<<(std::ostream& os, value const& obj) { os << obj.ToString(); return os; };
		std::stringstream& operator>>(std::stringstream& os, value& obj) { double v = 0; os >> v; obj = v; return os; };
		value operator+(value const& A, value const& V) { return Add(A, V); };
		value operator-(value const& A, value const& V) { return Sub(A, V); };
		value operator*(value const& A, value const& V) { return Multiply(A, V); };
		value operator/(value const& A, value const& V) { return Divide(A, V); };


	};

#if 1
	namespace Units {
		namespace impl {
			using TreeType = GoodLang::Map<uint64_t, std::tuple<Units::value_t, std::weak_ptr<GoodLang::Type_Info>>>; // look-up with unit ratio
			using ModelType = GoodLang::Map<size_t, TreeType>; // look-up with unit category

			static ModelType& Shared_Data() noexcept {
				static ModelType out;
				return out;
			};

			/*
			UnitHash determines the class of unit (length, time, length/time, length/time^2, length^1.25, etc.
			UnitRatio determines the specific ratio within that class (meter, foot, inch, etc.)
			*/
			static const std::tuple< Units::value_t, std::weak_ptr<GoodLang::Type_Info>>& lookup_impl(size_t UnitHash, double& UnitRatio) noexcept {
				auto targetRatio = Encode(UnitRatio);

				static std::tuple<Units::value_t, std::weak_ptr<GoodLang::Type_Info>> out{ Units::value_t(), std::weak_ptr<GoodLang::Type_Info>() };

				auto& model = Shared_Data();

				if (model.size() == 0) { // add all rows					
					// model.UniqueAt(HashUnits(Categories::length::unitType_m))->try_emplace(Encode(meter_t::conversion_ratio), std::tuple<Units::value_t, std::weak_ptr<GoodLang::Type_Info>>{ meter_t(), GoodLang::user_type_shared<meter_t>() });

#define CalculateMetricPrefixV(metric) \
	((long double)std::metric::num / (long double)std::metric::den)

#define DerivedUnitType(type, category, abbreviation, Ratio) \
    *model.UniqueAt(HashUnits(Categories::category::unitType_m))->operator[](Encode( type ## _t::conversion_ratio)) = std::tuple<Units::value_t, std::weak_ptr<GoodLang::Type_Info>>{ type ## _t(), GoodLang::user_type_shared< type ## _t >() };
		
#define DerivedUnitTypeWithMetricPrefix(type, category, prefix) \
    *model.UniqueAt(HashUnits(Categories::category::unitType_m))->operator[](Encode( prefix ## type ## _t::conversion_ratio)) = std::tuple<Units::value_t, std::weak_ptr<GoodLang::Type_Info>>{ prefix ## type ## _t(), GoodLang::user_type_shared< prefix ## type ## _t >() };

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
					bool lookupFailed = true;
					// I am not a scalar, and I am a non-static (e.g. dynamic) variable.
					Number ratio_to_use = Data.ratio();

					auto lookup = impl::lookup_impl(HashUnits(Data.unitType()), ratio_to_use);

					auto result = std::get<0>(lookup).unit_m.Shared();
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

		static __forceinline Number GetVisibleValue(value_t const& V) noexcept {
			Number out{ 0 };
			auto Data = V.unit_m.Shared();
			(void)Abbreviation(*Data, &out);
			return out;
		};
#if 0
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
#endif
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
#if 0
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
#endif

		value_t::operator Number() const noexcept { return GetVisibleValue(*this); };
		Number value_t::operator()() const noexcept { return GetVisibleValue(*this); };
		std::string value_t::ToString() const {
			Number out{ 0.0 };
			auto Data = this->unit_m.Shared();
			std::string abbreviation{ Abbreviation(*Data, &out) };
			if (abbreviation.length() > 0) return GetValueStr(out) + " " + abbreviation;
			else return GetValueStr(out);
		};
		std::ostream& operator<<(std::ostream& os, value_t const& obj) { os << obj.ToString(); return os; };;
		std::string_view value_t::UnitName() const noexcept {
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
		void value_t::Clear() {
			auto Data = unit_m.Unique();
			Data->Clear();
		};
		value_t& value_t::operator=(value_t const& other) {
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
		bool operator==(value_t const& A, value_t const& V) noexcept {
			return DoComparison([](Number LHS, Number RHS) -> bool {
				return LHS == RHS;
				}, A, V);
		};
		bool operator<(value_t const& A, value_t const& V) {
			return DoComparison([](Number LHS, Number RHS) -> bool {
				return LHS < RHS;
				}, A, V);
		};
		bool operator<=(value_t const& A, value_t const& V) {
			return DoComparison([](Number LHS, Number RHS) -> bool {
				return LHS <= RHS;
				}, A, V);
		};
		bool operator>(value_t const& A, value_t const& V) { return !(A <= V); };
		bool operator>=(value_t const& A, value_t const& V) { return !(A < V); };
		bool operator!=(value_t const& A, value_t const& V) noexcept { return !(operator==(A, V)); };

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
		value_t& value_t::operator++() {
			auto Data = this->unit_m.Unique();
			Data->value() += Data->ratio();
			return *this;
		};
		value_t& value_t::operator--() {
			auto Data = this->unit_m.Unique();
			Data->value() -= Data->ratio();
			return *this;
		};
		value_t& value_t::operator+=(value_t const& V) {
			DoUnaryOperation([](Number& lhs, Number const& rhs) { lhs += rhs; }, *this, V);
			return *this;
		};
		value_t& value_t::operator-=(value_t const& V) {
			DoUnaryOperation([](Number& lhs, Number const& rhs) { lhs -= rhs; }, *this, V);
			return *this;
		};
		value_t& value_t::operator*=(value_t const& V) {
			auto other = V.unit_m.Shared();
			HandleNotScalar(*other);
			auto Data = this->unit_m.Unique();
			Data->value() *= other->value();
			return *this;
		};
		value_t& value_t::operator/=(value_t const& V) {
			auto other = V.unit_m.Shared();
			HandleNotScalar(*other);
			auto Data = this->unit_m.Unique();
			Data->value() /= other->value();
			return *this;
		};

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

		value_t value_t::operator++(int) {
			value_t out{ *this }; // copy
			defer(auto Data{ this->unit_m.Unique() }; Data->value() += Data->ratio();); // increment
			return out; // returns old value, increments after
		};
		value_t value_t::operator--(int) {
			value_t out{ *this }; // copy
			defer(auto Data{ this->unit_m.Unique() }; Data->value() -= Data->ratio();); // increment
			return out; // returns old value, increments after
		};
		value_t operator+(value_t const& A, value_t const& B) {
			return Add(A, B);
		};
		value_t operator-(value_t const& A, value_t const& B) {
			return Sub(A, B);
		};
		value_t operator*(value_t const& A, value_t const& V) {
			return Multiply(A, V);
		};
		value_t operator/(value_t const& A, value_t const& V) {
			return Divide(A, V);
		};
		value_t value_t::operator-() const {
			return Multiply(*this, -1);
		};
		// atomicly updates the value_t with a custom user-provided function.
		value_t& value_t::update(std::function<double(double)> const& updateFunction) {
			auto Data = this->unit_m.Unique();
			Data->value() = updateFunction(Data->value() / Data->ratio()) * Data->ratio();
			return *this;
		};
		// Creats a copy of the value_t and updates it with a custom user-provided function.
		value_t value_t::update(std::function<double(double)> const& updateFunction) const {
			auto Data = this->unit_m.Shared();
			return value_t(updateFunction(Data->value() / Data->ratio()) * Data->ratio() * Data->ratio(), *Data);
		};
		// Returns a new value_t multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
		value_t value_t::pow(value_t const& V) const {
			auto other = V.unit_m.Shared();
			HandleNotScalar(*other);
			return MultiplyUnits(*this, other->value());
		};
		// atomicly updates the value_t by exponentiating the underlying value_t (e.g. (3_m).pow_value_t(3) => 9_m)
		value_t& value_t::pow_value(value_t const& V) {
			auto Data = this->unit_m.Unique();
			auto Other = V.unit_m.Shared();
			HandleNotScalar(*Other);
			Data->value() = std::pow(Data->value() / Data->ratio(), Other->value()) * Data->ratio();
			return *this;
		};
		// atomicly updates the value_t by exponentiating the underlying value_t (e.g. (3_m).pow_value_t(3) => 9_m)
		value_t value_t::pow_value(value_t const& V) const {
			auto Other = V.unit_m.Shared();
			HandleNotScalar(*Other);
			return this->update([&Other](double x)->double { return std::pow(x, Other->value()); });
		};
		// pow(0.5)
		value_t value_t::sqrt() const {
			return pow(0.5);
		};
		// atomicly floors (rounds to lower whole integer) the underlying value_t
		value_t& value_t::floor() {
			return update([](double v)->double { return std::floor(v); });
		};
		// Creats a copy of the value_t and floors (rounds to lower whole integer) the underlying value_t
		value_t value_t::floor() const {
			return update([](double v)->double { return std::floor(v); });
		};
		// atomicly ceilings (rounds to upper whole integer) the underlying value_t
		value_t& value_t::ceiling() {
			return update([](double v)->double { return std::ceil(v); });
		};
		// Creats a copy of the value_t and ceilings (rounds to upper whole integer) the underlying value_t
		value_t value_t::ceiling() const {
			return update([](double v)->double { return std::ceil(v); });
		};




























	};
#endif

};
