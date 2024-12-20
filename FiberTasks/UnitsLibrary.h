#pragma once

#include "ScriptingLanguage2.h"

namespace GoodLang {
#if 0
	namespace UnitsLibrary {
		template<typename T>
		static void AddUnit(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace) {
			std::string UnitName = T().UnitName();
			auto foot_namespace{ std::make_shared<Class>(std_namespace, UnitName, user_type_shared<T>().lock(), value_namespace) };
			foot_namespace->SetSelf(foot_namespace);
			std_namespace->AddChild(foot_namespace);
			{
				// Constructors
				// foot()
				foot_namespace->AddFunction(UnitName, make_callable([]() -> T { return T{}; }));
				// foot(double)
				//foot_namespace->AddFunction(UnitName, make_callable([](double const& o)->T { return o; }));
				// foot(value)
				foot_namespace->AddFunction(UnitName, make_callable([](Units::value const& makeCopy) -> T { return makeCopy; }));
				// foot() = value();
				foot_namespace->AddFunction("=", make_callable(
					[](Any const& a, Units::value const& b) -> Any { T& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<T&>(), user_type_shared<Units::value const&>()})));

				// value(foot)
				value_namespace->AddFunction(value_namespace->GetName(), make_callable([](Any const& from) -> std::shared_ptr<Units::value> {
					return std::dynamic_pointer_cast<Units::value>(from.cast<std::shared_ptr<T>>());
				}, ParamTypes({ foot_namespace->GetClassType() })));
			}
		};

		class UnitsLibrary {
		public:
			static void Part1(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
			static void Part2(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
			static void Part3(std::shared_ptr<Namespace> const& std_namespace, std::shared_ptr<Class> const& value_namespace);
		};

	}
#endif
};