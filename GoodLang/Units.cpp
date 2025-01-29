#pragma once
#include "Any.h"
#include "Units.h"

namespace GoodLang {
	// Units
	namespace Units {
		size_t HashUnits(double a, double b, double c, double d, double e) noexcept {
			size_t out{ 37 };
			GoodLang::details::hash_combine(out, a, b, c, d, e);
			return out;
		};
		const char* lookup_abbreviation(size_t UnitHash, double& UnitRatio) noexcept {
			return "";
			//return std::get<0>(lookup_impl(UnitHash, UnitRatio));
		};
		const char* lookup_typename(size_t UnitHash, double& UnitRatio) noexcept {
			return "";
			//return std::get<1>(lookup_impl(UnitHash, UnitRatio));
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
		std::pair<const char*, double> UnitDefinition::LookupAbbreviation(bool isStatic) const noexcept {
			double ratio_bestFit = (double)ratio_m;
			auto* abbrev_bestFit = Units::lookup_abbreviation(HashCategory(), ratio_bestFit);
			return { abbrev_bestFit, ratio_bestFit };
		};
		const char* UnitDefinition::LookupTypeName() const noexcept {
			double ratio_bestFit = (double)ratio_m;
			auto* TypeName_bestFit = Units::lookup_typename(HashCategory(), ratio_bestFit);
			return TypeName_bestFit;
		};
		std::string UnitDefinition::CreateAbbreviation(bool isStatic) const noexcept {
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
					constexpr static std::array< const char*, UnitDefinition::NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

					bool anyNegatives = false;
					std::string Num;
					for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
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
						for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
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
		value::operator double() const noexcept { return GetVisibleValue(); };
		double value::operator()() const noexcept { return GetVisibleValue(); };
		const char* value::UnitName() const noexcept {
			const char* toReturn{ "" };
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

							constexpr static std::array< const char*, UnitDefinition::NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

							bool anyNegatives = false;
							std::string Num;
							for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
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
								for (int i = UnitDefinition::NumUnits - 1; i >= 0; i--) {
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
				Data.value_m = std::pow(Data.value_m / Data.ratio_m, V) * Data.ratio_m; // save in SI value

				return Data;
				});
			return *this;
		};








	};
};