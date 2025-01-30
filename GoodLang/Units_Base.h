#pragma once
#include "ThreadSafeContainers.h"
#include "../WaterWatchCpp/enum.h"
#include <string_view>

namespace GoodLang {
	namespace Units {
		BETTER_ENUM(units_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);

		// container for all of the information that defines a unit value, including the unit data as well as the underlying SI value.
		class UnitDefinition {
		public:
			static constexpr size_t NumUnits{ Units::units_type::_size_constant };
			template <typename T> static constexpr T abs(T x) { return x > (T)0 ? x : -x; };

		public:
			std::array< double, NumUnits> unitType_m; // power exponents for the SI units (e.g. m^1 * kg^0 * s^-1 * A^0 * $^0 = m/s)
			double ratio_m; // ratio multiplier for converting from the SI units to this actual unit (e.g. 1 = meters, 0.304 = feet, etc.) 
			double value_m; // underlying value of the unit if represented as SI units. (e.g. will always be in meters, regardless of the actual unit being in feet)

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
			UnitDefinition(std::array< double, NumUnits> const& unitType_p, double ratio_p, double value_p) noexcept :
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
			std::pair<std::string_view, double> LookupAbbreviation(bool isStatic) const noexcept;
			std::string_view LookupTypeName() const noexcept;
			std::string CreateAbbreviation(bool isStatic) const noexcept;

		};
		
		static __forceinline size_t HashUnits(double a, double b, double c, double d, double e) noexcept {
			size_t out{ 37 };
			GoodLang::details::hash_combine(out, a, b, c, d, e);
			return out;
		};
		template <typename Derived> static constexpr __forceinline long double Conversion(long double X) { return Derived::conversion * X; };
		static constexpr __forceinline long double SQUARED(long double X) { return X * X; };
		static constexpr __forceinline long double CUBED(long double X) { return X * X * X; };

		class value {
		public:
			mutable Lockable<UnitDefinition> unit_m;

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

		public: // value operator
			explicit operator double() const noexcept;
			double operator()() const noexcept;

		public: // Functions
			std::string_view UnitName() const noexcept;
			void Clear();
			void Swap(value const& other) const;

		public:
			std::string Abbreviation(double* visibleValue = nullptr) const noexcept;
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
			value& update(std::function<double(double)> const& updateFunction);
			// Creats a copy of the value and updates it with a custom user-provided function.
			value update(std::function<double(double)> const& updateFunction) const;
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
	template<> class numeric_limits<GoodLang::Units::value> {
	public:
		static constexpr double min() { return std::numeric_limits<double>::min(); }
		static constexpr double max() { return std::numeric_limits<double>::max(); }
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); }
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer;
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed;
	};
};