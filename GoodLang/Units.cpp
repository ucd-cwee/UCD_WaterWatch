#pragma once
#include "Any.h"
#include "Units.h"
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
		std::vector<std::vector<std::tuple<std::string, std::string, Units::value, std::weak_ptr<GoodLang::Type_Info>>>> GetValueTypes() noexcept {
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

			using AllocType = GoodLang::utilities::FastAllocator<std::tuple< const char*, const char*, Units::value, std::weak_ptr<GoodLang::Type_Info>>>;
			using TreeType = std::map<uint64_t, std::tuple< const char*, const char*, Units::value, std::weak_ptr<GoodLang::Type_Info>>*>;
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
						std::string abbrev = std::get<0>(*each.second);
						std::string fullName = std::get<1>(*each.second);
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

		// UnitDefinition
		bool UnitDefinition::IsSI() const {
			return (abs((float)unitType_m[0]) + abs((float)unitType_m[1]) + abs((float)unitType_m[2]) + abs((float)unitType_m[3]) + abs((float)unitType_m[4])) == 1.0f
				&& abs((double)ratio_m) == 1.0;
		};
		bool UnitDefinition::IsScalar() const {
			return (abs((float)unitType_m[0]) + abs((float)unitType_m[1]) + abs((float)unitType_m[2]) + abs((float)unitType_m[3]) + abs((float)unitType_m[4])) == 0.0f
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
		std::pair<std::string_view, double> UnitDefinition::LookupAbbreviation(bool isStatic) const noexcept {
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
			// TO-DO
#if 0
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
#endif
			return "";
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

		// Value
		bool value::IsStaticType() const { return false; };
		double value::GetVisibleValue() const noexcept {
			double out{ 0. };
			Abbreviation(&out);
			return out;
		};
		std::string value::GetValueStr(value const& V) noexcept {
			auto v = V();
			if (std::fmod(v, 1.0) == 0.0) { // integer
				return std::to_string((long long)v);
				//return Units::printf("%lld", (long long)v);
			}
			else { // floating-point
				std::string out{ std::to_string(v) /*Units::printf("%.4f", v)*/ };
				Units::removeTrailingCharacters(out, '0'); // e.g. 25.5000 -> 25.5
				Units::removeTrailingCharacters(out, '.'); // e.g. 25.0000 -> 25. -> 25
				return out;
			}
		};
		bool value::IdenticalUnits(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept { return LHS.IsSameCategory(RHS); };
		bool value::is_scalar(UnitDefinition const& V) noexcept { return V.IsScalar(); };
		bool value::NormalArithmeticOkay(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept {
			if (is_scalar(LHS) || is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		bool value::UnaryArithmeticOkay(UnitDefinition const& LHS, UnitDefinition const& RHS) noexcept {
			if (is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		void value::HandleNormalArithmetic(UnitDefinition const& LHS, UnitDefinition const& RHS) {
			if (NormalArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(GoodLang::printf("Normal, dynamic arithmetic failed due to incompatible non-scalar value: '%s' and '%s'", AbbreviationFast(LHS).c_str(), AbbreviationFast(RHS).c_str())));
		};
		void value::HandleUnaryArithmetic(UnitDefinition const& LHS, UnitDefinition const& RHS) {
			if (UnaryArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(GoodLang::printf("Unary (in-place or self-modifying) arithmetic failed due to incompatible value: '%s' and '%s'", AbbreviationFast(LHS).c_str(), AbbreviationFast(RHS).c_str())));
		};
		void value::HandleNotScalar(UnitDefinition const& V) {
			if (is_scalar(V)) return;
			throw(std::runtime_error(GoodLang::printf("Type must be scalar (was '%s').", AbbreviationFast(V).c_str())));
		};
		std::string value::AbbreviationFast(UnitDefinition const& V) noexcept {
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
									Num = std::to_string((float)v);
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
										Num = std::to_string((float)(-1.0 * v));
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
		value::operator double() const noexcept { return GetVisibleValue(); };
		double value::operator()() const noexcept { return GetVisibleValue(); };
		std::string_view value::UnitName() const noexcept {
			std::string_view toReturn{ "" };
			unit_m.Update([&toReturn, this](UnitDefinition Data)->UnitDefinition {
				auto [abbreviation, ratio] = Data.LookupAbbreviation(this->IsStaticType());
				//Data.abbreviation_m = const_cast<char*>(abbreviation);
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
			unit_m.Swap(other.unit_m.load(), false);
		};
		std::string value::Abbreviation(double* visibleValue) const noexcept {
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
								double v = (float)Data.unitType_m[i];

								if (v > 0) {
									if (v == 1)
										AddToDelimiter(toReturn, unitBase.data(), " ");
									else {
										if (IsInteger(v)) {
											Num = std::to_string(static_cast<int>(v));
										}
										else {
											Num = std::to_string((float)v);
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
									double v = (float)Data.unitType_m[i];

									if (v < 0) {
										if (v == -1)
											AddToDelimiter(toReturn, unitBase.data(), " ");
										else {
											if (IsInteger(v)) {
												Num = std::to_string(static_cast<int>(-1.0 * v));
											}
											else {
												Num = std::to_string((float)(-1.0 * v));
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
			double out{ 0.0 };
			std::string abbreviation{ Abbreviation(&out) };
			if (abbreviation.length() > 0) return GetValueStr(out) + " " + abbreviation;
			else return GetValueStr(out);
		};
		value& value::MultiplyUnits(double const& V) noexcept {
			unit_m.Update([&V](UnitDefinition Data)->UnitDefinition {
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
				// Data.value_m = std::pow(Data.value_m / Data.ratio_m, V) * Data.ratio_m; // save in SI value
				Data.value_m = std::pow(Data.value_m, V);
				return Data;
			});
			return *this;
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
			unit_m.Update([](UnitDefinition Data)->UnitDefinition {
				Data.value_m += (double)Data.ratio_m;
				return Data;
				});
			return *this;
		};
		value& value::operator--() {
			unit_m.Update([](UnitDefinition Data)->UnitDefinition {
				Data.value_m -= (double)Data.ratio_m;
				return Data;
				});
			return *this;
		};
		value value::operator++(int) {
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
		value value::operator--(int) {
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
		value value::Add(value const& a, value const& b) {
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
		value value::Sub(value const& a, value const& b) {
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
		value value::Multiply(value const& A, value const& V) {
			value out = A;
			out.CompoundUnits<true>(V);
			return out;
		};
		value value::Divide(value const& A, value const& V) {
			value out = A;
			out.CompoundUnits<false>(V);
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
			unit_m.Update([other](UnitDefinition Data)->UnitDefinition {
				Data.value_m *= other.value_m;
				return Data;
				});
			return *this;
		};
		value& value::operator/=(value const& V) {
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([other](UnitDefinition Data)->UnitDefinition {
				Data.value_m /= other.value_m;
				return Data;
				});
			return *this;
		};
		value& value::update(std::function<double(double)> const& updateFunction) {
			unit_m.Update([&updateFunction](UnitDefinition Data)->UnitDefinition {
				Data.value_m = updateFunction(Data.value_m / Data.ratio_m) * Data.ratio_m;
				return Data;
				});
			return *this;
		};
		value value::update(std::function<double(double)> const& updateFunction) const {
			auto out{ value(*this) };
			out.update(updateFunction);
			return out;
		};
		value value::pow(value const& V) const {
			auto other{ V.unit_m.load() };

			HandleNotScalar(other);

			value out = *this;
			out.MultiplyUnits(other.value_m);

			return out;
		};
		value& value::pow_value(value const& V) {
			auto other{ V.unit_m.load() };
			HandleNotScalar(other);
			unit_m.Update([other](UnitDefinition Data)->UnitDefinition {
				Data.value_m = std::pow(Data.value_m / Data.ratio_m, other.value_m / other.ratio_m) * Data.ratio_m;
				return Data;
			});
			return *this;
		};
		value value::sqrt() const {
			return pow(0.5);
		};
		value& value::floor() {
			return update([](double v)->double { return std::floor(v); });
		};
		value value::floor() const {
			auto out{ value(*this) };
			out.floor();
			return out;
		};
		value& value::ceiling() {
			return update([](double v)->double { return std::ceil(v); });
		};
		value value::ceiling() const {
			auto out{ value(*this) };
			out.ceiling();
			return out;
		};

		// DerivedUnits

		// math
		namespace math {
			Units::value fabs(const Units::value& V) {
				if (V < 0) return V * -1.0; else return V;
			};
			Units::value abs(const Units::value& V) {
				return fabs(V);
			};
			Units::value clamp(const Units::value& V, const Units::value& min, const Units::value& max) {
				if (V < min) return min;
				if (V > max) return max;
				return V;
			};
			Units::value floor(const Units::value& f) {
				return f.floor();
			};
			Units::value ceiling(const Units::value& f) {
				return f.ceiling();
			};
			Units::value round(const Units::value& a, float magnitude) {
				return floor((a / magnitude) + 0.5) * magnitude;
			};
			Units::value max(const Units::value& a, const Units::value& b) {
				return a > b ? a : b;
			};
			Units::value min(const Units::value& a, const Units::value& b) {
				return a < b ? a : b;
			};
			void max_ref(Units::value* a, const Units::value& b) {
				if (b > *a) *a = b;

			};
			void min_ref(Units::value* a, const Units::value& b) {
				if (b < *a) *a = b;
			};
		};

		// constants
		namespace constants {
			Units::scalar					    pi() {
				return 3.141592653589793238462643383279502884197169399375105820974944;
			};
			Units::meters_per_second		    c() {
				return 299792458.0;
			};
			Units::value				        G() {
				return Units::meter(6.67408e-11) * Units::meter(1) * Units::meter(1) / (Units::kilogram(1) * Units::second(1) * Units::second(1));
			};
			Units::value				        g() {
				return Units::meter(9.8067) / (Units::second(1) * Units::second(1));
			};
			Units::value                        d() {
				return Units::kilogram(998.57) / (Units::meter(1) * Units::meter(1) * Units::meter(1));
			};
		};

	};
};
