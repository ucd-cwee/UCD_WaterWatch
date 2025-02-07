#pragma once 

#include "Units.h"
#include "Scopes.h"
#include "DateTime.h"
#include <memory>
#include <unordered_map>

namespace GoodLang {
	template<typename A, typename B> static B convert(A const& from) { return from; };

	template<typename A, typename B>
	static void add_converter(std::shared_ptr<Class>& classPtr, std::string const& Name) {
		classPtr->AddFunction(Name, Function(make_callable(&convert<A, B>)), true);
	};

	template<typename T>
	static void add_converters(std::shared_ptr<Class>& classPtr, std::string const& Name) {
		add_converter<bool, T>(classPtr, Name);
		add_converter<char, T>(classPtr, Name);
		add_converter<int, T>(classPtr, Name);
		add_converter<long, T>(classPtr, Name);
		add_converter<float, T>(classPtr, Name);
		add_converter<double, T>(classPtr, Name);
		add_converter<size_t, T>(classPtr, Name);
		// add_converter<fibers::containers::number < double >, T>(classPtr, Name);
		add_converter<signed char, T>(classPtr, Name);
		add_converter<unsigned char, T>(classPtr, Name);
		add_converter<char16_t, T>(classPtr, Name);
		add_converter<char32_t, T>(classPtr, Name);
		add_converter<wchar_t, T>(classPtr, Name);
		add_converter<short, T>(classPtr, Name);
		add_converter<unsigned short, T>(classPtr, Name);
		add_converter<unsigned int, T>(classPtr, Name);
		add_converter<unsigned long, T>(classPtr, Name);
		add_converter<long long, T>(classPtr, Name);
		add_converter<long double, T>(classPtr, Name);
	};


	template<typename T>
	static void DefineBuiltInType(Global* This, T typeImpl, std::string const& Name) {
		// make it a class
		std::shared_ptr<Class> classPtr; {
			classPtr.reset(new Class(This->GetSelf(), Name, user_type_shared<T>().lock()));
		}
		classPtr->SetSelf(classPtr);
		This->AddChild(classPtr);

		// add converters
		add_converters<T>(classPtr, Name);

		// Constructors
		classPtr->AddFunction(Name, make_callable([]() ->  T { return  T{}; }));
		// classPtr->AddFunction(Name, make_callable([](T const& makeCopy) ->  T { return makeCopy; }));
		classPtr->AddFunction("=", make_callable(
			[](Any const& a, T const& b) -> Any { T& x = a.cast(); x = b; return a; }
			, ParamTypes({ user_type_shared<T>().lock()->MakeRef(), user_type_shared<T>().lock()->MakeConstRef() })
		));

		// Comparisons
		classPtr->AddFunction("==", make_callable([](T const& x, T const& y) -> bool { return x == y; }));
		classPtr->AddFunction("!=", make_callable([](T const& x, T const& y) -> bool { return x != y; }));
		classPtr->AddFunction("<", make_callable([](T const& x, T const& y) -> bool { return x < y; }));
		classPtr->AddFunction("<=", make_callable([](T const& x, T const& y) -> bool { return x <= y; }));
		classPtr->AddFunction(">", make_callable([](T const& x, T const& y) -> bool { return x > y; }));
		classPtr->AddFunction(">=", make_callable([](T const& x, T const& y) -> bool { return x >= y; }));
		classPtr->AddFunction("+", make_callable([](T const& x, T const& y) -> T { return x + y; }));
		classPtr->AddFunction("-", make_callable([](T const& x, T const& y) -> T { return x - y; }));
		classPtr->AddFunction("*", make_callable([](T const& x, T const& y) -> T { return x * y; }));
		if constexpr (!std::is_same_v<bool, T>) {
			classPtr->AddFunction("/", make_callable([](T const& x, T const& y) -> T { if (y == 0) return std::numeric_limits<T>::max(); else return x / y; }));
			classPtr->AddFunction("+=", make_callable([](T& x, T const& y) -> void { x += y; }));
			classPtr->AddFunction("-=", make_callable([](T& x, T const& y) -> void { x -= y; }));
			classPtr->AddFunction("*=", make_callable([](T& x, T const& y) -> void { x *= y; }));
			classPtr->AddFunction("/=", make_callable([](T& x, T const& y) -> void { if (y == 0) x = std::numeric_limits<T>::max(); else x /= y; }));
		}
		// Functions
		classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<T>::max(); }));
		classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<T>::lowest(); }));
		classPtr->AddFunction("to_string", make_callable([](T const& o) -> std::string { return std::to_string(o); }));
		if constexpr (utilities::is_std_hashable_v<T>) {
			classPtr->AddFunction("to_hash", make_callable([](T const& o) -> size_t { return std::hash<T>()(o); }));
		}
	};


	void AddBasicTypes(Global* This) {
		DefineBuiltInType(This, bool{}, "bool");
		DefineBuiltInType(This, char{}, "char");
		DefineBuiltInType(This, int{}, "int");
		DefineBuiltInType(This, long{}, "long");
		DefineBuiltInType(This, float{}, "float");
		DefineBuiltInType(This, double{}, "double");
		DefineBuiltInType(This, size_t{}, "size_t");
		// DefineBuiltInType(This, fibers::containers::number<double>(), "Number");
		DefineBuiltInType(This, char16_t{}, "char16_t");
		DefineBuiltInType(This, char32_t{}, "char32_t");
		DefineBuiltInType(This, wchar_t{}, "wchar_t");
		DefineBuiltInType(This, short{}, "short");
		DefineBuiltInType(This, unsigned char(0), "uchar");
		DefineBuiltInType(This, unsigned short(0), "ushort");
		DefineBuiltInType(This, unsigned int(0), "uint");
		DefineBuiltInType(This, unsigned long(0), "ulong");
		DefineBuiltInType(This, long long(0), "llong");
		DefineBuiltInType(This, long double(), "ldouble");		
	};

	void Global::AddBuiltIns() {
		// Built-in types
		if (1) {
			AddBasicTypes(this);

			// String
			if (1) {
				// make it a class
				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), "string", user_type_shared<std::string>().lock()));
				}
				classPtr->SetSelf(classPtr);
				this->AddChild(classPtr);

				// add converters
				classPtr->AddFunction("string", make_callable([](bool const& from) -> std::string { if (from) return "true"; else return "false"; }));
				classPtr->AddFunction("string", make_callable([](char const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](int const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](long const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](float const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](double const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](size_t const& from) -> std::string { return std::to_string(from); }));
				// classPtr->AddFunction("string", make_callable([](fibers::containers::number < double > const& from) -> std::string { return std::to_string(from.load()); }));
				classPtr->AddFunction("string", make_callable([](signed char const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](unsigned char const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](char16_t const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](char32_t const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](wchar_t const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](short const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](unsigned short const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](unsigned int const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](unsigned long const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](long long const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([](long double const& from) -> std::string { return std::to_string(from); }));
				classPtr->AddFunction("string", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
					if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
					if (auto p = from.lock()) return p->name();
					else return user_type<void>().name();
				}));

				// Constructors
				classPtr->AddFunction("string", make_callable([]() -> std::string { return std::string{}; }));
				classPtr->AddFunction("string", make_callable([](std::string const& makeCopy) -> std::string { return makeCopy; }));
				classPtr->AddFunction("=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<std::string>().lock()->MakeRef(), user_type_shared<std::string>().lock()->MakeConstRef() })));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](std::string const& x, std::string const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](std::string const& x, std::string const& y) -> bool { return x != y; }));
				classPtr->AddFunction("<", make_callable([](std::string const& x, std::string const& y) -> bool { return x < y; }));
				classPtr->AddFunction("<=", make_callable([](std::string const& x, std::string const& y) -> bool { return x <= y; }));
				classPtr->AddFunction(">", make_callable([](std::string const& x, std::string const& y) -> bool { return x > y; }));
				classPtr->AddFunction(">=", make_callable([](std::string const& x, std::string const& y) -> bool { return x >= y; }));
				classPtr->AddFunction("+", make_callable([](std::string const& x, std::string const& y) -> std::string { return x + y; }));
				classPtr->AddFunction("+=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out += b; return a; }, ParamTypes({ user_type_shared<std::string>().lock()->MakeRef(), user_type_shared<std::string>().lock()->MakeConstRef() })));

				// Functions
				classPtr->AddFunction("length", make_callable([](std::string const& a) -> size_t { return a.length(); }));
				classPtr->AddFunction("size", make_callable([](std::string const& a) -> size_t { return a.size(); }));
				classPtr->AddFunction("[]", make_callable([](std::string const& a, size_t index) -> char { return a[index]; }));
				classPtr->AddFunction("front", make_callable([](std::string const& a) -> char { return a.front(); }));
				classPtr->AddFunction("back", make_callable([](std::string const& a) -> char { return a.back(); }));
				classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind) -> size_t { return a.find(toFind); }));
				classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind, size_t startPos) -> size_t { return a.find(toFind, startPos); }));
				classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off) -> std::string { return x.substr(Off); })/*, { "input", "Off" }*/);
				classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off, size_t Count) -> std::string { return x.substr(Off, Count); })/*, { "input", "Off", "Count" }*/);
				classPtr->AddFunction("to_string", make_callable([](std::string const& o) -> std::string { return o; }));
				classPtr->AddFunction("to_hash", make_callable([](std::string const& o) -> size_t { return std::hash<std::string>()(o); }));

				// Objects or Constants
				classPtr->AddObj("npos", std::make_shared<Any>(std::string::npos));
			}

			// Types
			if (1) {
				// make it a class
				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), "Type_Info", user_type_shared<std::weak_ptr<Type_Info>>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);

				// Constructors
				//classPtr->AddFunction("Type_Info", make_callable([]() -> std::weak_ptr<Type_Info> { return std::weak_ptr<Type_Info>{}; }));
				//classPtr->AddFunction("Type_Info", make_callable([](std::weak_ptr<Type_Info> const& makeCopy) -> std::weak_ptr<Type_Info> { return makeCopy; }));
				classPtr->AddFunction("=", make_callable([](Any const& a, std::weak_ptr<Type_Info> const& b) -> Any { std::weak_ptr<Type_Info>& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<std::weak_ptr<Type_Info>>().lock()->MakeRef(), user_type_shared<std::weak_ptr<Type_Info>>().lock()->MakeConstRef() })));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](std::weak_ptr<Type_Info> const& x, std::weak_ptr<Type_Info> const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](std::weak_ptr<Type_Info> const& x, std::weak_ptr<Type_Info> const& y) -> bool { return x != y; }));

				// Functions
				classPtr->AddFunction("to_string", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
					if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
					else if (auto p = from.lock()) return p->name();
					else return user_type<void>().name();
				}));
				classPtr->AddFunction("to_hash", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->size_t {
					if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return std::hash<std::string>()(p2->GetName());
					else if (auto p = from.lock()) return std::hash<std::string>()(p->name());
					else return std::hash<std::string>()(user_type<void>().name());
				}));
				classPtr->AddFunction("name", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
					if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
					else if (auto p = from.lock()) return p->name();
					else return user_type<void>().name();
				}));
				classPtr->AddFunction("cpp_name", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
					if (auto p = from.lock()) return p->name();
					else return user_type<void>().name();
				}));
				classPtr->AddFunction("is_const", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)-> bool {
					if (auto p = from.lock()) return p->is_const();
					else return false;
				}));
				classPtr->AddFunction("is_ref", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)-> bool {
					if (auto p = from.lock()) return p->is_ref();
					else return false;
				}));
				classPtr->AddFunction("is_void", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)-> bool {
					if (auto p = from.lock()) return p->is_void();
					else return true;
				}));
				//classPtr->AddFunction("member_objects", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from) -> 
				//	fibers::containers::Map<std::string, Any> {
				//	if (auto p = self.lock()) {
				//		if (auto p2 = p->FindClass(from)) {
				//			fibers::containers::Map<std::string, Any> out;
				//			for (auto& x : p2->GetAllMemberObjects()) {
				//				out.emplace(x.first, x.second);
				//			}
				//			return out;
				//		}
				//	}
				//	return {};
				//}));


			}

			// Units
			if (1) {
				auto selfPtr = this->p_self.lock();
				auto std_namespace{ std::make_shared<Namespace>(selfPtr, "Units") };
				std_namespace->SetSelf(std_namespace);
				this->AddChild(std_namespace);

				// the "std" namespace imports the "string" namespace...
				{
					auto value_namespace{ std::make_shared<Class>(std_namespace, "value", user_type_shared<Units::value>().lock()) };
					value_namespace->SetSelf(value_namespace);
					std_namespace->AddChild(value_namespace);

					// which has the following types groups... 
					{
						// value -> double
						if (auto p = std_namespace->FindClass(user_type_shared<double>())) {
							p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> double { return o(); }));
						}
						// value -> float
						if (auto p = std_namespace->FindClass(user_type_shared<float>())) {
							p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> float { return o(); }));
						}
						// value -> int
						if (auto p = std_namespace->FindClass(user_type_shared<int>())) {
							p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> int { return o(); }));
						}
						// value -> string
						if (auto p = std_namespace->FindClass(user_type_shared<std::string>())) {
							p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> std::string { return o.ToString(); }));
						}

						value_namespace->AddFunction("abbreviation", make_callable([](Units::value const& x)->std::string {
							return x.Abbreviation();
						}));
						value_namespace->AddFunction("name", make_callable([](Units::value const& x)->std::string {
							return x.UnitName().data();
						}));
						value_namespace->AddFunction("to_string", make_callable([](Units::value const& x)->std::string {
							return x.ToString();
						}));
						// hashes do not exist for Units because their values are only approximations

						// Constructors
						value_namespace->AddFunction("value", make_callable([]() -> Units::value { return Units::value{}; }));
						value_namespace->AddFunction("value", make_callable([](Units::value const& makeCopy) -> Units::value { return makeCopy; }));
						value_namespace->AddFunction("value", make_callable([](int const& o)->Units::value { return o; }));
						value_namespace->AddFunction("value", make_callable([](float const& o)->Units::value { return o; }));
						value_namespace->AddFunction("value", make_callable([](double const& o)->Units::value { return o; }));
						value_namespace->AddFunction("=", make_callable(
							[](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));

						// Comparisons & operators
						value_namespace->AddFunction("==", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x == y; }));
						value_namespace->AddFunction("!=", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x != y; }));
						value_namespace->AddFunction("<", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x < y; }));
						value_namespace->AddFunction(">", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x > y; }));
						value_namespace->AddFunction("<=", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x <= y; }));
						value_namespace->AddFunction(">=", make_callable([](Units::value const& x, Units::value const& y) -> bool { return x >= y; }));
						value_namespace->AddFunction("+", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x + y; }));
						value_namespace->AddFunction("-", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x - y; }));
						value_namespace->AddFunction("*", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x * y; }));
						value_namespace->AddFunction("/", make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x / y; }));
						value_namespace->AddFunction("+=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out += b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
						value_namespace->AddFunction("-=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out -= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
						value_namespace->AddFunction("*=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out *= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
						value_namespace->AddFunction("/=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out /= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>() })));
					}

					if (1) {
						UnitsLibrary::UnitsLibrary::Part1(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part2(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part3(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part4(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part5(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part6(std_namespace, value_namespace);
					}
				}
			}

			// DateTime
			if (1) {
				using thisType = DateTime;
				std::string thisTypeName = "DateTime";

				// make it a class
				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
				}
				classPtr->SetSelf(classPtr);
				this->AddChild(classPtr);
				auto thisTypeInfo = classPtr->ClassType;

				// Constructors
				classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
				classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
				classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any { thisType& out = a.cast(); out = b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })));
				classPtr->AddFunction(thisTypeName, make_callable([](Units::second const& from) -> thisType { return from; }));
				classPtr->AddFunction(thisTypeName, Function(make_callable([](std::string const& from) -> thisType { return thisType(from); }), true)); // explicit = cannot be used for conversion trees, and must be called directly with exact type match, e.g. DateTime("string")

				// Converters
				if (auto p = this->FindClass(user_type_shared<std::string>())) {
					p->AddFunction(p->GetName(), Function(make_callable([](thisType const& from) -> std::string { return from.c_str(); }), true)); // explicit
				}
				if (auto p = this->FindClass(user_type_shared<Units::second>())) {
					p->AddFunction(p->GetName(), make_callable([](thisType const& from) -> Units::second { return (Units::second)from; }));
				}

				// Comparisons
				classPtr->AddFunction("==", make_callable([](thisType const& x, thisType const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](thisType const& x, thisType const& y) -> bool { return x != y; }));
				classPtr->AddFunction("<", make_callable([](thisType const& x, thisType const& y) -> bool { return x < y; }));
				classPtr->AddFunction("<=", make_callable([](thisType const& x, thisType const& y) -> bool { return x <= y; }));
				classPtr->AddFunction(">", make_callable([](thisType const& x, thisType const& y) -> bool { return x > y; }));
				classPtr->AddFunction(">=", make_callable([](thisType const& x, thisType const& y) -> bool { return x >= y; }));

				// Operators
				classPtr->AddFunction("+", make_callable([](thisType const& x, thisType const& y) -> thisType { return x + y; }));
				classPtr->AddFunction("-", make_callable([](thisType const& x, thisType const& y) -> thisType { return x - y; }));
				classPtr->AddFunction("*", make_callable([](thisType const& x, thisType const& y) -> thisType { return x * y; }));
				classPtr->AddFunction("/", make_callable([](thisType const& x, thisType const& y) -> thisType { if (y == 0) return (Units::second)(std::numeric_limits<Units::second>::max()); else return x / y; }));
				classPtr->AddFunction("+=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x += b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));
				classPtr->AddFunction("-=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x -= b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));
				classPtr->AddFunction("*=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x *= b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));
				classPtr->AddFunction("/=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); if (b != 0) x /= b; else x = (Units::second)(std::numeric_limits<Units::second>::max()); return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() })));

				// Functions
				classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<thisType>::max(); }));
				classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<thisType>::lowest(); }));
				classPtr->AddFunction("to_string", make_callable([](thisType const& o) -> std::string { return o.c_str(); }));
				classPtr->AddFunction("to_hash", make_callable([](thisType const& o) -> size_t { return (size_t)(double)(Units::millisecond)(Units::second)o; }));

				classPtr->AddFunction("Epoch", make_callable(&DateTime::Epoch));
				classPtr->AddFunction("Now", make_callable(&DateTime::Now));
				// classPtr->AddFunction("Now", make_callable([]() -> DateTime { return DateTime::Now(); }));					
				classPtr->AddFunction("tm_fractionalsec", make_callable(&DateTime::tm_fractionalsec));
				classPtr->AddFunction("tm_sec", make_callable(&DateTime::tm_sec));
				classPtr->AddFunction("tm_min", make_callable(&DateTime::tm_min));
				classPtr->AddFunction("tm_hour", make_callable(&DateTime::tm_hour));
				classPtr->AddFunction("tm_mday", make_callable(&DateTime::tm_mday));
				classPtr->AddFunction("tm_mon", make_callable(&DateTime::tm_mon));
				classPtr->AddFunction("tm_yday", make_callable(&DateTime::tm_yday));
				classPtr->AddFunction("tm_wday", make_callable(&DateTime::tm_wday));
				classPtr->AddFunction("tm_year", make_callable(&DateTime::tm_year));
				classPtr->AddFunction("load", make_callable(&DateTime::load));
				classPtr->AddFunction("getNumDaysInSameMonth", make_callable(&DateTime::getNumDaysInSameMonth));
				//classPtr->AddFunction("getNumDaysInSameMonth", make_callable([](DateTime const& dt) -> int {
				//	return DateTime::getNumDaysInSameMonth(dt);
				//}));
				classPtr->AddFunction("make_time", make_callable([]() { return DateTime::make_time(); }));
				classPtr->AddFunction("make_time", make_callable([](int year) { return DateTime::make_time(year); }));
				classPtr->AddFunction("make_time", make_callable([](int year, int month) { return DateTime::make_time(year, month); }));
				classPtr->AddFunction("make_time", make_callable([](int year, int month, int day) { return DateTime::make_time(year, month, day); }));
				classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour) { return DateTime::make_time(year, month, day, hour); }));
				classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute) { return DateTime::make_time(year, month, day, hour, minute); }));
				classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute, float second) { return DateTime::make_time(year, month, day, hour, minute, second); }));
				classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute, float second, bool useLocalTime) { return DateTime::make_time(year, month, day, hour, minute, second, useLocalTime); }));
				classPtr->AddFunction("ToStartOfMonth", make_callable([](DateTime from) -> DateTime { return from.ToStartOfMonth(); }));
				classPtr->AddFunction("ToEndOfMonth", make_callable([](DateTime from) -> DateTime { return from.ToEndOfMonth(); }));
				classPtr->AddFunction("ToStartOfDay", make_callable([](DateTime from) -> DateTime { return from.ToStartOfDay(); }));
				classPtr->AddFunction("ToEndOfDay", make_callable([](DateTime from) -> DateTime { return from.ToEndOfDay(); }));
				classPtr->AddFunction("ToStartOfHour", make_callable([](DateTime from) -> DateTime { return from.ToStartOfHour(); }));
				classPtr->AddFunction("ToEndOfHour", make_callable([](DateTime from) -> DateTime { return from.ToEndOfHour(); }));
				classPtr->AddFunction("ToStartOfMinute", make_callable([](DateTime from) -> DateTime { return from.ToStartOfMinute(); }));
				classPtr->AddFunction("ToEndOfMinute", make_callable([](DateTime from) -> DateTime { return from.ToEndOfMinute(); }));

				// Parameters
				classPtr->AddFunction("time", make_callable(&DateTime::time));
			}

			// Var
			if (1) {
				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), "Var", user_type_shared<Var>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);

				// Constructors
				// Var()
				classPtr->AddFunction("Var", make_callable([]() -> Var {
					return Var();
					})); // explicit = cannot be used for conversion trees
					// Var(Var const&)
				classPtr->AddFunction("Var", make_callable([](Var const& obj) -> Var {
					return obj;
					})); // explicit = cannot be used for conversion trees
					// Var(Any)
				classPtr->AddFunction("Var", Function(make_callable([](Any const& obj) -> Var {
					if (obj.IsTypeOf<Var>()) {
						return obj.cast<Var&>();
					}
					else {
						return Var(obj);
					}
					}), true)); // explicit = cannot be used for conversion trees
					// Var& = Var const&
				classPtr->AddFunction("=", make_callable([](Any const& a, Var const& b) -> Any {
					Var& out = a.cast(); out = b; return a;
					}, ParamTypes({ user_type_shared<Var>().lock()->MakeRef(), user_type_shared<Var>().lock()->MakeConstRef() }), user_type_shared<Var>().lock()->MakeRef()));
				// Var& = Any
				classPtr->AddFunction("=", make_callable([](Any const& a, Any const& b) -> Any {
					Var& out = a.cast(); out = Var(b); return a;
					}, ParamTypes({ user_type_shared<Var>().lock()->MakeRef(), user_type_shared<Any>() }), user_type_shared<Var>().lock()->MakeRef()));

				// Reset a Var
				classPtr->AddFunction("try_reset", make_callable([](Any const& a) -> bool {
					if (Var* p = a.cast<Var*>()) { // will succeed for "Var" types. Note that Var's with values will LIE about their types -- can only find out by trying to do this cast.
						p->p_data = std::make_shared<Any>();
						return true;
					}
					return false;
					}));
				// Reset a Var
				classPtr->AddFunction("try_reset", make_callable([](Any const& a, Any const& b) -> bool {
					if (Var* p = a.cast<Var*>()) { // will succeed for "Var" types. Note that Var's with values will LIE about their types -- can only find out by trying to do this cast.
						p->p_data = std::make_shared<Any>(b);
						return true;
					}
					return false;
					}));

				// template func, Any = Var const&
				classPtr->AddFunction("=", make_callable([self = std::weak_ptr<Class>(classPtr)](Any& a, Var const& b)->Any {
					if (auto Self = self.lock()) {
						return Self->CallFunction("=", { a, b.p_data });
					}
					else {
						a = b.p_data;
						return a;
					}
				}));

				// Returns the type of the contained object. By not specifying the type, the Any is treated like a Template
				this->AddFunction("Type_Info", make_callable([](Var const& obj) -> std::weak_ptr<Type_Info> {
					return obj.p_data->Type();
					}));
				// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
				this->AddFunction("Type", make_callable([](Var const& obj) -> std::weak_ptr<Type_Info> {
					return obj.p_data->Type();
					}));
				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](Var const& x)->std::string {
					if (auto Self = self.lock()) {
						return Self->Cast<std::string>(Self->CallFunction("to_string", { x.p_data }));
					}
					else {
						auto name = x.p_data->TypeName();
						return GoodLang::printf("`%s`", name.c_str());
					}
				}));
				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_hash", make_callable([self = std::weak_ptr<Class>(classPtr)](Var const& x)->size_t {
					if (auto Self = self.lock()) {
						return Self->Cast<size_t>(Self->CallFunction("to_hash", { x.p_data }));
					}
					else {
						throw exception::not_found_error("to_hash");
					}
				}));
			}

			// Pair
			if (1) {
				using thisType = std::pair<Var, Var>;
				std::string thisTypeName = "pair";

				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);
				auto thisTypeInfo = classPtr->ClassType;

				// Constructors
				classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
				classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
				classPtr->AddFunction(thisTypeName, make_callable([](Any const& a, Any const& b) -> thisType { return thisType{ Var(a), Var(b) }; }));
				classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any {
					thisType& out = a.cast(); out = b; return a;
					},
					ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })));

				// Functions
				classPtr->AddFunction("first", make_callable([](thisType& r) -> Var& { return r.first; }));
				classPtr->AddFunction("first", make_callable([](thisType const& r) -> Var const& { return r.first; }));
				classPtr->AddFunction("second", make_callable([](thisType& r) -> Var& { return r.second; }));
				classPtr->AddFunction("second", make_callable([](thisType const& r) -> Var const& { return r.second; }));

				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x)->std::string {
					if (auto Self = self.lock()) {
						std::string a = Self->Cast<std::string>(Self->CallFunction("to_string", { x.first.p_data }));
						std::string b = Self->Cast<std::string>(Self->CallFunction("to_string", { x.second.p_data }));
						return GoodLang::printf("[%s, %s]", a.c_str(), b.c_str());
					}
					else {
						throw exception::not_found_error("to_string");
					}
				}));
				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_hash", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x)->size_t {
					if (auto Self = self.lock()) {
						size_t a = Self->Cast<size_t>(Self->CallFunction("to_hash", { x.first.p_data }));
						size_t b = Self->Cast<size_t>(Self->CallFunction("to_hash", { x.second.p_data }));
						Scope::hash_combine(a, b);
						return a;
					}
					else {
						throw exception::not_found_error("to_hash");
					}
				}));
			}

#if 0
			// Map
			if (1) {
				using thisType = UnorderedMap<
					std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>
					, std::shared_ptr<Var>
					, std::hash<std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>>
				>;
				std::string thisTypeName = "Map";

				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);
				auto thisTypeInfo = classPtr->ClassType;

				// Constructor
				classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
				// Copy constructor
				classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
				// assignment operator
				classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any {
					thisType& out = a.cast(); out = b; return a;
					},
					ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })));

				// Functions
				classPtr->AddFunction("at", make_callable([Self = classPtr->p_self](thisType const& r, Any const& key)->Any {
					if (auto ptr = *r.at({ key, Self })) {
						return Any(ptr);
					}
					throw std::out_of_range("key was not found");
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), user_type_shared<Any>() }), user_type_shared<Var>().lock()->MakeRef()));
				classPtr->AddFunction("[]", make_callable([Self = classPtr->p_self](thisType& r, Any const& key)->Any {
					if (auto ptr = *r[{ key, Self }]) {
						return Any(ptr);
					}
					else {
						return Any(*r.get_or_insert({ key, Self }, std::make_shared<Var>()));
					}
				}, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Any>() }), user_type_shared<Var>().lock()->MakeRef()));
				classPtr->AddFunction("emplace", make_callable([Self = classPtr->p_self](thisType& r, Any const& key, Any const& value) {
					r.emplace({ key, Self }, std::make_shared<Var>(value));
				}));
				classPtr->AddFunction("contains", make_callable([Self = classPtr->p_self](thisType& r, Any const& key) -> bool {
					return r.count({ key, Self }) > 0;
				}));
				classPtr->AddFunction("size", make_callable([Self = classPtr->p_self](thisType& r, Any const& key)->size_t {
					return r.size();
				}));

				// to_string
				classPtr->AddFunction("to_string", make_callable([Self = classPtr->p_self](thisType const& r)->std::string {
					if (auto self = Self.lock()) {
						std::string out;
						for (auto x : r) {
							const Any& key = x.first.first;
							Any value = x.second;

							auto key_str = self->Cast<std::string>(self->CallFunction("to_string", { key }));
							auto value_str = self->Cast<std::string>(self->CallFunction("to_string", { value }));

							if (out.size() == 0) {
								out = key_str + ":" + value_str;
							}
							else {
								out += ", ";
								out += (key_str + ":" + value_str);
							}
							
						}
						return "[" + out + "]";
					}
					return "[]";
				}));

				// to_hash
				classPtr->AddFunction("to_hash", make_callable([Self = classPtr->p_self](thisType const& r)->size_t {
					std::size_t out = 0;
					if (auto self = Self.lock()) {
						for (auto x : r) {
							const Any& key = x.first.first;
							Any value = x.second;

							auto key_hash = self->Cast<size_t>(self->CallFunction("to_hash", { key }));
							auto value_hash = self->Cast<size_t>(self->CallFunction("to_hash", { value }));

							Scope::hash_combine(out, key_hash);
							Scope::hash_combine(out, value_hash);							
						}
					}
					return out;
				}));

				// Iterator
				if (1) {
					using thisIteratorType = thisType::iterator;
					std::string thisIteratorTypeName = "iterator";

					std::shared_ptr<Class> iterator_classPtr; {
						iterator_classPtr.reset(new Class(classPtr, thisIteratorTypeName, user_type_shared<thisIteratorType>().lock()));
					}
					iterator_classPtr->p_self = iterator_classPtr;
					classPtr->AddChild(iterator_classPtr);
					auto thisIteratorTypeInfo = iterator_classPtr->ClassType;

					// Constructor
					iterator_classPtr->AddFunction(thisIteratorTypeName, make_callable([]() -> thisIteratorType { return thisIteratorType{}; }));
					// Copy constructor
					iterator_classPtr->AddFunction(thisIteratorTypeName, make_callable([](thisIteratorType const& makeCopy) -> thisIteratorType { return makeCopy; }));
					// assignment operator
					iterator_classPtr->AddFunction("=", make_callable([](Any const& a, thisIteratorType const& b) -> Any {
						thisIteratorType& out = a.cast(); out = b; return a;
					},
						ParamTypes({ thisIteratorTypeInfo->MakeRef(), thisIteratorTypeInfo->MakeConstRef() }), thisIteratorTypeInfo->MakeRef()));

					// equality
					iterator_classPtr->AddFunction("==", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs == rhs; }));
					iterator_classPtr->AddFunction("!=", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs != rhs; }));
					// iter
					iterator_classPtr->AddFunction("++", make_callable([](Any const& a) -> Any {
						thisIteratorType& out = a.cast();
						out.operator++();
						return a;
						},
						ParamTypes({ thisIteratorTypeInfo->MakeRef() }), thisIteratorTypeInfo->MakeRef()));
					// first (convenience function for this specialization)
					iterator_classPtr->AddFunction("first", make_callable([](thisIteratorType const& makeCopy) -> Any {
						return (*makeCopy).first.first;
					}));
					// second (convenience function for this specialization)
					iterator_classPtr->AddFunction("second", make_callable([](thisIteratorType const& makeCopy) -> Any {
						return (*makeCopy).second;
					}));
					// get // must be implimented by the iterator
					iterator_classPtr->AddFunction("get", make_callable([](thisIteratorType const& makeCopy) -> std::pair<Var, Var> {
						return std::pair<Var, Var>{ Any((*makeCopy).first.first), Any((*makeCopy).second) };
					}));
				}
				classPtr->AddFunction("begin", make_callable([Self = classPtr->p_self](thisType const& r)->thisType::iterator {
					return r.begin();
				}));
				classPtr->AddFunction("end", make_callable([Self = classPtr->p_self](thisType const& r)->thisType::iterator {
					return r.end();
				}));

			}
#endif

#if 0
			// Set 
			if (1) {
				using thisType = fibers::containers::Set<
					std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>
					, std::hash<std::pair<GoodLang::Any, std::weak_ptr<GoodLang::Scope>>>
				>;
				std::string thisTypeName = "Set";

				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);
				auto thisTypeInfo = classPtr->ClassType;

				// Constructor
				classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
				// Copy constructor
				classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
				// assignment operator
				classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any {
					thisType& out = a.cast(); out = b; return a;
					},
					ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })));

				// Functions
				classPtr->AddFunction("emplace", make_callable([Self = classPtr->p_self](thisType& r, Any const& key) {
					r.emplace({ key, Self });
				}));
				classPtr->AddFunction("contains", make_callable([Self = classPtr->p_self](thisType& r, Any const& key) -> bool {
					return r.contains({ key, Self });
				}));
				classPtr->AddFunction("size", make_callable([Self = classPtr->p_self](thisType& r, Any const& key)->size_t {
					return r.size();
				}));

				// to_string
				classPtr->AddFunction("to_string", make_callable([Self = classPtr->p_self](thisType const& r)->std::string {
					if (auto self = Self.lock()) {
						std::string out;
						for (auto x : r) {
							const Any& key = x.first;
							auto key_str = self->Cast<std::string>(self->CallFunction("to_string", { key }));

							if (out.size() == 0) {
								out = key_str;
							}
							else {
								out += ", ";
								out += key_str;
							}
						}
						return "[" + out + "]";
					}
					return "[]";
				}));

				// to_hash
				classPtr->AddFunction("to_hash", make_callable([Self = classPtr->p_self](thisType const& r)->size_t {
					std::size_t out = 0;
					if (auto self = Self.lock()) {
						for (auto x : r) {
							const Any& key = x.first;
							auto key_hash = self->Cast<size_t>(self->CallFunction("to_hash", { key }));
							Scope::hash_combine(out, key_hash);
						}
					}
					return out;
				}));

				// Iterator
				if (1) {
					using thisIteratorType = thisType::iterator;
					std::string thisIteratorTypeName = "iterator";

					std::shared_ptr<Class> iterator_classPtr; {
						iterator_classPtr.reset(new Class(classPtr, thisIteratorTypeName, user_type_shared<thisIteratorType>().lock()));
					}
					iterator_classPtr->p_self = iterator_classPtr;
					classPtr->AddChild(iterator_classPtr);
					auto thisIteratorTypeInfo = iterator_classPtr->ClassType;

					// Constructor
					iterator_classPtr->AddFunction(thisIteratorTypeName, make_callable([]() -> thisIteratorType { return thisIteratorType{}; }));
					// Copy constructor
					iterator_classPtr->AddFunction(thisIteratorTypeName, make_callable([](thisIteratorType const& makeCopy) -> thisIteratorType { return makeCopy; }));
					// assignment operator
					iterator_classPtr->AddFunction("=", make_callable([](Any const& a, thisIteratorType const& b) -> Any {
						thisIteratorType& out = a.cast(); out = b; return a;
						},
						ParamTypes({ thisIteratorTypeInfo->MakeRef(), thisIteratorTypeInfo->MakeConstRef() }), thisIteratorTypeInfo->MakeRef()));

					// equality
					iterator_classPtr->AddFunction("==", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs == rhs; }));
					iterator_classPtr->AddFunction("!=", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs != rhs; }));
					// iter
					iterator_classPtr->AddFunction("++", make_callable([](Any const& a) -> Any {
						thisIteratorType& out = a.cast();
						out.operator++();
						return a;
						},
						ParamTypes({ thisIteratorTypeInfo->MakeRef() }), thisIteratorTypeInfo->MakeRef()));
					// get // must be implimented by the iterator
					iterator_classPtr->AddFunction("get", make_callable([](thisIteratorType const& makeCopy) -> Any {
						return (*makeCopy).first;
						}));
				}
				classPtr->AddFunction("begin", make_callable([Self = classPtr->p_self](thisType const& r)->thisType::iterator {
					return r.begin();
				}));
				classPtr->AddFunction("end", make_callable([Self = classPtr->p_self](thisType const& r)->thisType::iterator {
					return r.end();
				}));

			}
#endif
		}

		// Built-In static, templated functions
		if (1) {
			// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
			this->AddFunction("Type_Info", make_callable([](Any const& obj) -> std::weak_ptr<Type_Info> {
				return obj.Type();
				}));
			// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
			this->AddFunction("Type", make_callable([](Any const& obj) -> std::weak_ptr<Type_Info> {
				return obj.Type();
				}));
			// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
			this->AddFunction("to_string", make_callable([](Any const& x) -> std::string {
				auto name = x.TypeName();
				return GoodLang::printf("`%s`", name.c_str());
			}));
		}
	};

};