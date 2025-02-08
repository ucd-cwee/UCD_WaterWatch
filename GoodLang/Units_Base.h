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
		static __forceinline size_t HashUnits(std::array< double, Units::units_type::_size_constant> const& r) noexcept {
			return HashUnits(r[0], r[1], r[2], r[3], r[4]);
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

		public:
			explicit operator Number() const noexcept;
			Number operator()() const noexcept;
			std::string ToString() const;
			friend std::ostream& operator<<(std::ostream& os, value_t const& obj);
			std::string_view UnitName() const noexcept;
			void Clear();
		public: // = Operators
			value_t& operator=(value_t const& other);

		// Comparison operators
		public:
			friend bool operator==(value_t const& A, value_t const& V) noexcept;
			friend bool operator<(value_t const& A, value_t const& V);
			friend bool operator<=(value_t const& A, value_t const& V);
			friend bool operator>(value_t const& A, value_t const& V);
			friend bool operator>=(value_t const& A, value_t const& V);
			friend bool operator!=(value_t const& A, value_t const& V) noexcept;

		// Unary operators
		public:
			value_t& operator++();
			value_t& operator--();
			value_t& operator+=(value_t const& V);
			value_t& operator-=(value_t const& V);
			value_t& operator*=(value_t const& V);
			value_t& operator/=(value_t const& V);

		public: 
			value_t operator++(int);
			value_t operator--(int);
			friend value_t operator+(value_t const& A, value_t const& B);
			friend value_t operator-(value_t const& A, value_t const& B);
			friend value_t operator*(value_t const& A, value_t const& V);
			friend value_t operator/(value_t const& A, value_t const& V);
			value_t operator-() const;
			// atomicly updates the value_t with a custom user-provided function.
			value_t& update(std::function<double(double)> const& updateFunction);
			// Creats a copy of the value_t and updates it with a custom user-provided function.
			value_t update(std::function<double(double)> const& updateFunction) const;
			// Returns a new value_t multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
			value_t pow(value_t const& V) const;
			// atomicly updates the value_t by exponentiating the underlying value_t (e.g. (3_m).pow_value_t(3) => 9_m)
			value_t& pow_value(value_t const& V);
			// atomicly updates the value_t by exponentiating the underlying value_t (e.g. (3_m).pow_value_t(3) => 9_m)
			value_t pow_value(value_t const& V) const;
			// pow(0.5)
			value_t sqrt() const;
			// atomicly floors (rounds to lower whole integer) the underlying value_t
			value_t& floor();
			// Creats a copy of the value_t and floors (rounds to lower whole integer) the underlying value_t
			value_t floor() const;
			// atomicly ceilings (rounds to upper whole integer) the underlying value_t
			value_t& ceiling();
			// Creats a copy of the value_t and ceilings (rounds to upper whole integer) the underlying value_t
			value_t ceiling() const;

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