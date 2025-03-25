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
		template <typename Derived> static constexpr __forceinline long double Conversion(double X) { return Derived::conversion_ratio * X; };
		template <typename Derived> static constexpr __forceinline long double Conversion_t(double X) { return Derived::conversion_ratio * X; };
		static constexpr __forceinline double SQUARED(double X) { return X * X; };
		static constexpr __forceinline double CUBED(double X) { return X * X * X; };
	
		namespace impl {
			class value_accessor;

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

			public:
				UnitDefinitionBase() = delete; //  : value_m{ 0 } {};
				UnitDefinitionBase(Number V) : value_m{ std::move(V) } {};
				virtual ~UnitDefinitionBase() = default;

				virtual std::unique_ptr<UnitDefinitionBase> Copy() const = 0;
				virtual const std::array<double, NumUnits>& unitType() const = 0;
				virtual const double& ratio() const = 0;
				Number& value();
				const Number& value() const;
				virtual const char* BuiltInName() const;
				virtual const char* BuiltInAbbreviation() const;
				bool IsSI() const;
				bool IsScalar() const;
				bool IsSameCategory(UnitDefinitionBase const& other) const noexcept;
				bool IsSameUnit(UnitDefinitionBase const& other) const noexcept;
				size_t HashCategory() const noexcept;
				virtual void Clear();
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

				std::unique_ptr<UnitDefinitionBase> Copy() const override;
				const std::array<double, UnitDefinitionBase::NumUnits>& unitType() const override;
				const double& ratio() const override;
				std::array<double, UnitDefinitionBase::NumUnits>& unitType();
				double& ratio();
				void Clear() override;
			};
			class scalarDefinition final : public UnitDefinitionBase {
			private:
				static constexpr std::array<double, NumUnits> unitType_m{ 0, 0, 0, 0, 0 };
				static constexpr double ratio_m{ 1.0 };
			public:
				scalarDefinition() : UnitDefinitionBase(0.0) {};
				scalarDefinition(Number V) : UnitDefinitionBase(V) {};
				scalarDefinition(scalarDefinition const&) = default;
				scalarDefinition(scalarDefinition&&) = default;
				scalarDefinition& operator=(scalarDefinition const&) = default;
				scalarDefinition& operator=(scalarDefinition&&) = default;
				~scalarDefinition() = default;
				const std::array<double, NumUnits>& unitType() const override;
				const double& ratio() const override;
				const char* BuiltInName() const override;
				const char* BuiltInAbbreviation() const override;
				std::unique_ptr<UnitDefinitionBase> Copy() const override;
			};
		};

		class value {
			friend class impl::value_accessor;

		protected:
			SharedLockable<Definitions::UnitDefinitionBase> 
				unit_m; // allows shared/unique locking and abstracts the underlying actual unit definition. 
			
		public: // constructors
			value() : unit_m{ std::make_unique<Definitions::scalarDefinition>() } {};
			explicit value(std::unique_ptr<Definitions::UnitDefinitionBase>&& unit_p) : unit_m{ std::move(unit_p) } {};
			explicit value(Number const& V, Definitions::UnitDefinitionBase const& unit_p) : unit_m{ unit_p.Copy() } {
				auto get = unit_m.Unique();
				get->value() = (V / get->ratio());
			};
			value(value&& V) : unit_m{ V.unit_m.Shared()->Copy() } {};
			value(value const& V) : unit_m{ V.unit_m.Shared()->Copy() } {};
			value(Number V) : unit_m{ std::make_unique<Definitions::scalarDefinition>(std::move(V)) } {};
			virtual ~value() = default;

		public: 
			explicit operator Number() const noexcept;
			Number operator()() const noexcept;
			std::string ToString() const;
			friend std::ostream& operator<<(std::ostream& os, value const& obj);
			std::string_view UnitName() const noexcept;
			std::string UnitAbbreviation() const noexcept;
			void Clear();

		public: // Assignment operator
			value& operator=(value const& other);
		
		public: // Comparison operators
			friend bool operator==(value const& A, value const& V) noexcept;
			friend bool operator<(value const& A, value const& V);
			friend bool operator<=(value const& A, value const& V);
			friend bool operator>(value const& A, value const& V);
			friend bool operator>=(value const& A, value const& V);
			friend bool operator!=(value const& A, value const& V) noexcept;

		public: // Unary operators (e.g. mody in-place)
			value& operator++();
			value& operator--();
			value& operator+=(value const& V);
			value& operator-=(value const& V);
			value& operator*=(value const& V);
			value& operator/=(value const& V);
			// atomicly updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
			value& pow_value(value const& V);
			// atomicly floors (rounds to lower whole integer) the underlying value
			value& floor();
			// atomicly ceilings (rounds to upper whole integer) the underlying value
			value& ceiling();

		public: // Instance operators (e.g. makes a copy and modifies the copy)
			[[nodiscard]] value operator++(int);
			[[nodiscard]] value operator--(int);
			friend value operator+(value const& A, value const& B);
			friend value operator-(value const& A, value const& B);
			friend value operator*(value const& A, value const& V);
			friend value operator/(value const& A, value const& V);
			// inverts the sign of the value
			value operator-() const;
			// atomicly updates the value with a custom user-provided function.
			value& update(std::function<double(double)> const& updateFunction);
			// Creates a copy of the value and updates it with a custom user-provided function.
			value update(std::function<double(double)> const& updateFunction) const;
			// Returns a new value multiplied by itself "V" times. (e.g. (3_m).pow(3) => 3_cu_m)
			value pow(value const& V) const;
			// atomicly updates the value by exponentiating the underlying value (e.g. (3_m).pow_value(3) => 9_m)
			value pow_value(value const& V) const;
			// pow(0.5)
			value sqrt() const;
			// Creates a copy of the value and floors (rounds to lower whole integer) the underlying value
			value floor() const;
			// Creates a copy of the value and ceilings (rounds to upper whole integer) the underlying value
			value ceiling() const;
			// return a unique hash for the unit/value combo
			size_t hash() const;

		public:
			static Map<std::weak_ptr<GoodLang::Type_Info>, Units::value> GetValueTypes() noexcept;

		};
		using scalar = value;
		
		namespace impl {
			class value_accessor {
			public:
				static const SharedLockable<Definitions::UnitDefinitionBase>& GetUnits(value const& rhs) {
					return rhs.unit_m;
				};
				static SharedLockable<Definitions::UnitDefinitionBase>& GetUnits(value& rhs) {
					return rhs.unit_m;
				};
			};
		};

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

		// METRIC PREFIX CLASS DEFINITION
		namespace Definitions {
			template<typename ratioType, typename baseType>
			class MetricPrefixedDefinition final : public UnitDefinitionBase {
			public:
				using unitDefBase = UnitDefinitionBase;
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
				~MetricPrefixedDefinition() = default;
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
	};
	namespace Impl {
		__forceinline void ToString(Tag<Units::value>, Units::value const& r, std::string& out) {
			out = r.ToString();
		};
		// __forceinline void GetChildren(Tag<Units::value>, Units::value const& r, std::vector< NodeCache >& out) { };
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