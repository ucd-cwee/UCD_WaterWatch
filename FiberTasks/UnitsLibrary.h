#pragma once


#include "ScriptingLanguage.h"
// #include "Units.h"

namespace scripting {
	namespace UnitsLibrary {
		template<typename T>
		static void AddUnit(std::shared_ptr<scripting::Namespace2> const& std_namespace, std::shared_ptr<scripting::Class2> const& value_namespace) {
			std::string UnitName = T().UnitName();
			auto foot_namespace{ std::make_shared<scripting::Class2>(std_namespace, UnitName, scripting::user_type<T>(), value_namespace) };
			foot_namespace->SetSelf(foot_namespace);
			std_namespace->AddChild(foot_namespace);
			{
				// Constructors
				// foot()
				foot_namespace->AddFunction(UnitName, scripting::make_callable([]() -> T { return T{}; }));
				// foot(double)
				//foot_namespace->AddFunction(UnitName, scripting::make_callable([](double const& o)->T { return o; }));
				// foot(value)
				foot_namespace->AddFunction(UnitName, scripting::make_callable([](Units::value const& makeCopy) -> T { return makeCopy; }));
				// foot() = value();
				foot_namespace->AddFunction("=", scripting::make_callable([](scripting::Any const& a, Units::value const& b) -> scripting::Any { T& out = a.cast(); out = b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<Units::value>() } }));

				// Comparisons & operators
				//foot_namespace->AddFunction("==", scripting::make_callable([](T const& x, T const& y) -> bool { return x == y; }));
				//foot_namespace->AddFunction("!=", scripting::make_callable([](T const& x, T const& y) -> bool { return x != y; }));
				//foot_namespace->AddFunction("<", scripting::make_callable([](T const& x, T const& y) -> bool { return x < y; }));
				//foot_namespace->AddFunction(">", scripting::make_callable([](T const& x, T const& y) -> bool { return x > y; }));
				//foot_namespace->AddFunction("<=", scripting::make_callable([](T const& x, T const& y) -> bool { return x <= y; }));
				//foot_namespace->AddFunction(">=", scripting::make_callable([](T const& x, T const& y) -> bool { return x >= y; }));
				////foot_namespace->AddFunction("+", scripting::make_callable([](T const& x, T const& y) -> T { return x + y; }));
				////foot_namespace->AddFunction("-", scripting::make_callable([](T const& x, T const& y) -> T { return x - y; }));
				////foot_namespace->AddFunction("*", scripting::make_callable([](T const& x, T const& y) -> T { return x * y; }));
				////foot_namespace->AddFunction("/", scripting::make_callable([](T const& x, T const& y) -> T { return x / y; }));
				//foot_namespace->AddFunction("+=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out += b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));
				//foot_namespace->AddFunction("-=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out -= b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));
				//foot_namespace->AddFunction("*=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out *= b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));
				//foot_namespace->AddFunction("/=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out /= b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));

				// value(foot)
				value_namespace->AddFunction(value_namespace->GetName(), scripting::make_callable([](scripting::Any const& from) -> std::shared_ptr<Units::value> {
					return std::dynamic_pointer_cast<Units::value>(from.cast<std::shared_ptr<T>>());
				}), scripting::Param_Types({ { "from", foot_namespace->GetClassType() } }));
			}
		};

		class UnitsLibrary {
		public:
			static void Part1(std::shared_ptr<scripting::Namespace2> const& std_namespace, std::shared_ptr<scripting::Class2> const& value_namespace);
			static void Part2(std::shared_ptr<scripting::Namespace2> const& std_namespace, std::shared_ptr<scripting::Class2> const& value_namespace);
			static void Part3(std::shared_ptr<scripting::Namespace2> const& std_namespace, std::shared_ptr<scripting::Class2> const& value_namespace);
		};

	}

};