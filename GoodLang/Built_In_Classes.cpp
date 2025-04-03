#pragma once 

#include "Units.h"
#include "Scopes.h"
#include "DateTime.h"
#include <memory>
#include <unordered_map>
#include "Parallel.h"
#include <iostream>

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
		classPtr->AddFunction(Name, make_callable([](T const& makeCopy) ->  T { return makeCopy; }));
		classPtr->AddFunction("=", make_callable(
			[](Any const& a, T const& b) -> Any { T& x = a.cast(); x = b; return a; }
			, ParamTypes({ user_type_shared<T>().lock()->MakeRef(), user_type_shared<T>().lock()->MakeConstRef() })
			, user_type_shared<T>().lock()->MakeRef()
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

			classPtr->AddFunction("++", make_callable([](Any const& a) -> Any { 
				T& x = a.cast();	
				x++;
				return a;
			}, ParamTypes({ user_type_shared<T>().lock()->MakeRef() }), user_type_shared<T>().lock()->MakeRef()));
			classPtr->AddFunction("--", make_callable([](Any const& a) -> Any {
				T& x = a.cast();
				x--;
				return a;
			}, ParamTypes({ user_type_shared<T>().lock()->MakeRef() }), user_type_shared<T>().lock()->MakeRef()));
			if constexpr (std::is_signed_v<T>) {
				classPtr->AddFunction("-", make_callable([](Any const& a) -> T {
					T& x = a.cast();
					return -x;
					}, ParamTypes({ user_type_shared<T>().lock()->MakeRef() })));
			}
			classPtr->AddFunction("+", make_callable([](Any const& a) -> T {
				T& x = a.cast();
				return +x;
			}, ParamTypes({ user_type_shared<T>().lock()->MakeRef() })));
			if constexpr (std::is_integral_v<T>) {
				classPtr->AddFunction("~", make_callable([](Any const& a) -> T {
					T& x = a.cast();
					return ~x;
				}, ParamTypes({ user_type_shared<T>().lock()->MakeRef() })));
			}
		}
		else {
			classPtr->AddFunction("!", make_callable([](bool const& a) -> bool {
				return !a;
			}));
		}


		// Functions
		classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<T>::max(); }));
		classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<T>::lowest(); }));
		classPtr->AddFunction("to_string", make_callable([](T const& o) -> std::string { return std::to_string(o); }));
		if constexpr (utilities::is_std_hashable_v<T>) {
			classPtr->AddFunction("to_hash", make_callable([](T const& o) -> size_t { return std::hash<T>()(o); }));
		}
		else {
			if constexpr (std::is_floating_point_v<T>) {
				classPtr->AddFunction("to_hash", make_callable([](T const& o) -> size_t { size_t out{ 37 }; details::hash_combine(out, (double)o); return out; }));
			}

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

			// Any
			if (1) {
				// make it a class
				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), "Any", user_type_shared<std::weak_ptr<Any>>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);
			};

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
					if (auto p = self.lock()) return p->GetTypeName(from);
					if (auto p = from.lock()) return p->name();
					else return user_type<void>().name();
				}));

				// Constructors
				classPtr->AddFunction("string", make_callable([]() -> std::string { return std::string{}; }));
				classPtr->AddFunction("string", make_callable([](std::string const& makeCopy) -> std::string { return makeCopy; }));
				classPtr->AddFunction("=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out = b; return a; }, ParamTypes({ user_type_shared<std::string>().lock()->MakeRef(), user_type_shared<std::string>().lock()->MakeConstRef() }), user_type_shared<std::string>().lock()->MakeRef()));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](std::string const& x, std::string const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](std::string const& x, std::string const& y) -> bool { return x != y; }));
				classPtr->AddFunction("<", make_callable([](std::string const& x, std::string const& y) -> bool { return x < y; }));
				classPtr->AddFunction("<=", make_callable([](std::string const& x, std::string const& y) -> bool { return x <= y; }));
				classPtr->AddFunction(">", make_callable([](std::string const& x, std::string const& y) -> bool { return x > y; }));
				classPtr->AddFunction(">=", make_callable([](std::string const& x, std::string const& y) -> bool { return x >= y; }));
				classPtr->AddFunction("+", make_callable([](std::string const& x, std::string const& y) -> std::string { return x + y; }));
				classPtr->AddFunction("+=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out += b; return a; }, ParamTypes({ user_type_shared<std::string>().lock()->MakeRef(), user_type_shared<std::string>().lock()->MakeConstRef() }), user_type_shared<std::string>().lock()->MakeRef()));

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
				classPtr->AddFunction("=", make_callable([](Any const& a, std::weak_ptr<Type_Info> const& b) -> Any { std::weak_ptr<Type_Info>& out = a.cast(); out = b; return a; 
					}, ParamTypes({ user_type_shared<std::weak_ptr<Type_Info>>().lock()->MakeRef(), user_type_shared<std::weak_ptr<Type_Info>>().lock()->MakeConstRef() }), user_type_shared<std::weak_ptr<Type_Info>>().lock()->MakeRef()));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](std::weak_ptr<Type_Info> const& x, std::weak_ptr<Type_Info> const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](std::weak_ptr<Type_Info> const& x, std::weak_ptr<Type_Info> const& y) -> bool { return x != y; }));

				// Functions
				classPtr->AddFunction("to_string", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
					if (auto p = self.lock()) return p->GetTypeName(from);
					else if (auto p = from.lock()) return p->name();
					else return user_type<void>().name();
				}));
				classPtr->AddFunction("to_hash", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->size_t {
					if (auto p = self.lock()) return std::hash<std::string>()(p->GetTypeName(from));
					else if (auto p = from.lock()) return std::hash<std::string>()(p->name());
					else return std::hash<std::string>()(user_type<void>().name());
				}));
				classPtr->AddFunction("name", make_callable([self = classPtr->p_self](std::weak_ptr<Type_Info> const& from)->std::string {
					if (auto p = self.lock()) return p->GetTypeName(from);
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
							return x.UnitAbbreviation().data();
						}));
						value_namespace->AddFunction("name", make_callable([](Units::value const& x)->std::string {
							return x.UnitName().data();
						}));
						value_namespace->AddFunction("to_string", make_callable([](Units::value const& x)->std::string {
							return x.ToString();
						}));
						value_namespace->AddFunction("to_hash", make_callable([](Units::value const& x)->size_t {
							return x.hash();
						}));

						// Constructors
						value_namespace->AddFunction("value", make_callable([]() -> Units::value { return Units::value{}; }));
						value_namespace->AddFunction("value", make_callable([](Units::value const& makeCopy) -> Units::value { return makeCopy; }));
						value_namespace->AddFunction("value", make_callable([](int const& o)->Units::value { return o; }));
						value_namespace->AddFunction("value", make_callable([](float const& o)->Units::value { return o; }));
						value_namespace->AddFunction("value", make_callable([](double const& o)->Units::value { return o; }));
						value_namespace->AddFunction("=", make_callable(
							[](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out = b; return a; 
						}, ParamTypes({ 
								user_type_shared<Units::value>().lock()->MakeRef(), 
								user_type_shared<Units::value>().lock()->MakeConstRef() 
							}), user_type_shared<Units::value>().lock()->MakeRef()));

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
						value_namespace->AddFunction("+=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out += b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>().lock()->MakeConstRef() }), user_type_shared<Units::value>().lock()->MakeRef()));
						value_namespace->AddFunction("-=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out -= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>().lock()->MakeConstRef() }), user_type_shared<Units::value>().lock()->MakeRef()));
						value_namespace->AddFunction("*=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out *= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>().lock()->MakeConstRef() }), user_type_shared<Units::value>().lock()->MakeRef()));
						value_namespace->AddFunction("/=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out /= b; return a; }, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef(), user_type_shared<Units::value>().lock()->MakeConstRef() }), user_type_shared<Units::value>().lock()->MakeRef()));

						value_namespace->AddFunction("++", make_callable([](Any const& a) -> Any { 
							Units::value& out = a.cast(); ++out; return a;
						}, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef() }), user_type_shared<Units::value>().lock()->MakeRef()));
						value_namespace->AddFunction("--", make_callable([](Any const& a) -> Any { 
							Units::value& out = a.cast(); --out; return a; 
						}, ParamTypes({ user_type_shared<Units::value>().lock()->MakeRef() }), user_type_shared<Units::value>().lock()->MakeRef()));
						value_namespace->AddFunction("-", make_callable([](Any const& a) -> Any { 
							Units::value& out = a.cast(); 
							return -out;
						}, ParamTypes({ user_type_shared<Units::value>().lock()->MakeConstRef() }), user_type_shared<Units::value>().lock()));
						value_namespace->AddFunction("+", make_callable([](Any const& a) -> Any { 
							Units::value& out = a.cast(); 
							return Units::math::abs(out);
						}, ParamTypes({ user_type_shared<Units::value>().lock()->MakeConstRef() }), user_type_shared<Units::value>().lock()));
						value_namespace->AddFunction("~", make_callable([](Any const& a) -> Any { 
							Units::value out = a.cast(); 
							out = ~(int)(double)out;
							return out;
						}, ParamTypes({ user_type_shared<Units::value>().lock()->MakeConstRef() }), user_type_shared<Units::value>().lock()));
					}

					if (1) {
						UnitsLibrary::UnitsLibrary::Part1(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part2(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part3(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part4(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part5(std_namespace, value_namespace);
						UnitsLibrary::UnitsLibrary::Part6(std_namespace, value_namespace);
					}

					for (auto& unit_type : Units::value::GetValueTypes()) {
						auto abbreviation = std::string("_") + std::string(unit_type.second.UnitAbbreviation());
						auto type_info = unit_type.first.lock();
						if (auto Class = std_namespace->FindClass(type_info)) {

							auto LambdaWrapped = [](Any const& x, Any const& y, auto ToDo) { // std::function<void(std::shared_ptr<Units::value> const&, std::shared_ptr<Units::value> const&)>
								return ToDo(x.cast<std::shared_ptr<Units::value>>(), y.cast<std::shared_ptr<Units::value>>());
							};

							// Comparisons & operators
							Class->AddFunction("==", make_callable([&](Any const& x, Any const& y) -> bool {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x == *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction("!=", make_callable([&](Any const& x, Any const& y) -> bool {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x != *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction(">", make_callable([&](Any const& x, Any const& y) -> bool {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x > *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction("<", make_callable([&](Any const& x, Any const& y) -> bool {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x < *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction(">=", make_callable([&](Any const& x, Any const& y) -> bool {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x >= *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction("<=", make_callable([&](Any const& x, Any const& y) -> bool {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x <= *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction("+", make_callable([&](Any const& x, Any const& y) -> Units::value {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x + *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction("-", make_callable([&](Any const& x, Any const& y) -> Units::value {
								return LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									return *x - *y; }); }, ParamTypes({ type_info->MakeConstRef(), type_info->MakeConstRef() })));
							Class->AddFunction("+=", make_callable([&](Any const& x, Any const& y) -> Any {
								(void)LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									*x += *y; 
								}); 
								return x;
							}, ParamTypes({ type_info->MakeRef(), type_info->MakeConstRef() }), type_info->MakeRef()));
							Class->AddFunction("-=", make_callable([&](Any const& x, Any const& y) -> Any {
								(void)LambdaWrapped(x, y, [](auto const& x, auto const& y) {
									*x -= *y; 
								}); 
								return x;
							}, ParamTypes({ type_info->MakeRef(), type_info->MakeConstRef() }), type_info->MakeRef()));
							Class->AddFunction("++", make_callable([&](Any const& x) -> Any {
								x.cast<std::shared_ptr<Units::value>>()->operator++();
								return x;
							}, ParamTypes({ type_info->MakeRef() }), type_info->MakeRef()));
							Class->AddFunction("--", make_callable([&](Any const& x) -> Any {
								x.cast<std::shared_ptr<Units::value>>()->operator--();
								return x;
							}, ParamTypes({ type_info->MakeRef() }), type_info->MakeRef()));
							Class->AddFunction("-", make_callable([&](Any const& x) -> Units::value {
								return x.cast<std::shared_ptr<Units::value>>()->operator-();
							}, ParamTypes({ type_info->MakeConstRef() })));

						}
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
				classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any { thisType& out = a.cast(); out = b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() }), thisTypeInfo->MakeRef()));
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
				classPtr->AddFunction("+=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x += b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() }), thisTypeInfo->MakeRef()));
				classPtr->AddFunction("-=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x -= b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() }), thisTypeInfo->MakeRef()));
				classPtr->AddFunction("*=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x *= b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() }), thisTypeInfo->MakeRef()));
				classPtr->AddFunction("/=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); if (b != 0) x /= b; else x = (Units::second)(std::numeric_limits<Units::second>::max()); return a; }, ParamTypes({ thisTypeInfo->MakeRef(), user_type_shared<Units::second>().lock()->MakeConstRef() }), thisTypeInfo->MakeRef()));

				// Functions
				classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<thisType>::max(); }));
				classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<thisType>::lowest(); }));
				classPtr->AddFunction("to_string", make_callable([](thisType const& o) -> std::string { return o.c_str(); }));
				classPtr->AddFunction("to_hash", make_callable([](thisType const& o) -> size_t { return ((Units::second)o).hash(); }));

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
				// classPtr->AddFunction("load", make_callable(&DateTime::load));
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
				classPtr->AddFunction("=", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& a, Var const& b) -> Any {
					Var& out = a.cast(); 
					if (*out.p_data) {
						if (auto Self = self.lock()) Self->CallFunction("=", { out.p_data, b.p_data });	else *out.p_data = *b.p_data;
					}
					else {
						*out.p_data = *b.p_data;
					}
					// out = b; // copy its references
					return a;
				}, ParamTypes({ user_type_shared<Var>().lock()->MakeRef(), user_type_shared<Var>().lock()->MakeConstRef() }), user_type_shared<Var>().lock()->MakeRef()));
				// Var& = Any
				classPtr->AddFunction("=", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& a, Any const& b) -> Any {
					Var& out = a.cast(); 
					if (*out.p_data) {
						if (auto Self = self.lock()) Self->CallFunction("=", { out.p_data, b }); else *out.p_data = b;
					}
					else {
						*out.p_data = b;
					}
					// out = Var(b); // redirect our reference
					return a;
				}, ParamTypes({ user_type_shared<Var>().lock()->MakeRef(), user_type_shared<Any>() }), user_type_shared<Var>().lock()->MakeRef()));
				// Reset a Var
				this->AddFunction("try_reset", make_callable([](Any const& a) -> bool {
					if (Var* p = a.cast<Var*>()) { // will succeed for "Var" types. Note that Var's with any value will LIE about their types -- can only find out by trying to do this cast on an Any.
						p->p_data = std::make_shared<Any>();
						return true;
					}
					return false;
				}/*, ParamTypes({ user_type_shared<Any>() })*/)); // not specifying "Var" was on-purpose.
				// Reset a Var
				this->AddFunction("try_reset", make_callable([](Any const& a, Any const& b) -> bool {
					if (Var* p = a.cast<Var*>()) { // will succeed for "Var" types. Note that Var's with any value will LIE about their types -- can only find out by trying to do this cast on an Any.
						p->p_data = std::make_shared<Any>(b);
						return true;
					}
					return false;
				}/*, ParamTypes({ user_type_shared<Any>(), user_type_shared<Any>() })*/)); // not specifying "Var" was on-purpose.
				// Reset a Var
				this->AddFunction("reset", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& a, Any const& b) -> Any {
					if (Var* p = a.cast<Var*>()) { // will succeed for "Var" types. Note that Var's with any value will LIE about their types -- can only find out by trying to do this cast on an Any.
						p->p_data = std::make_shared<Any>(b);
						return a;
					}
					else {
						if (auto Self = self.lock()) {
							return Self->CallFunction("=", { a, b });
						}
						else {
							throw std::runtime_error("Out of scope");
						}
					}
				}/*, ParamTypes({ user_type_shared<Any>(), user_type_shared<Any>() })*/)); // not specifying "Var" was on-purpose.
				// Reset a Var
				this->AddFunction(":=", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& a, Any const& b)->Any {
					if (Var* p = a.cast<Var*>()) { // will succeed for "Var" types. Note that Var's with any value will LIE about their types -- can only find out by trying to do this cast on an Any.
						p->p_data = std::make_shared<Any>(b);
						return a;
					}
					else {
						if (auto Self = self.lock()) {
							return Self->CallFunction("=", { a, b });
						}
						else {
							throw std::runtime_error("Out of scope");
						}
					}
				}/*, ParamTypes({ user_type_shared<Any>(), user_type_shared<Any>() })*/)); // not specifying "Var" was on-purpose.
				// Reset a Var
				this->AddFunction(":=", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& a, Var const& b)->Any {
					if (Var* p = a.cast<Var*>()) { // will succeed for "Var" types. Note that Var's with any value will LIE about their types -- can only find out by trying to do this cast on an Any.
						p->p_data = b.p_data;
						return a;
					}
					else {
						if (auto Self = self.lock()) {
							return Self->CallFunction("=", { a, b.p_data });
						}
						else {
							throw std::runtime_error("Out of scope");
						}
					}
				}, ParamTypes({ user_type_shared<Any>(), user_type_shared<Var>().lock()->MakeConstRef() }))); // not specifying "Var" was on-purpose.
				// template func, Any = Var const&
				this->AddFunction("=", make_callable([self = std::weak_ptr<Class>(classPtr)](Any& a, Var const& b)->Any {
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

			// Promise
			if (1) {
				using thisType = parallel::promise;
				std::string thisTypeName = "Promise";

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
				classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any { thisType& out = a.cast(); out = b; return a; }, ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() }), thisTypeInfo->MakeRef()));

				// Functions
				classPtr->AddFunction("Await", make_callable([](thisType& o) -> Var { return Var(o.wait_get_any()); }));
				classPtr->AddFunction("Returns", make_callable([](thisType const& o) -> std::weak_ptr<Type_Info> { return o.Type(); }));
			}

			// Pair
			if (1) {
				using thisType = std::pair<Var, Var>;
				std::string thisTypeName = "Pair";

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
				ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() }), thisTypeInfo->MakeRef()));

				// Functions
				classPtr->AddFunction("first", make_callable([](thisType& r) -> Any { return r.first; }, ParamTypes({ user_type_shared<thisType>().lock()->MakeRef() }), user_type_shared<Var>().lock()->MakeRef()));
				classPtr->AddFunction("first", make_callable([](thisType const& r) -> Any { return r.first; }, ParamTypes({ user_type_shared<thisType>().lock()->MakeConstRef() }), user_type_shared<Var>().lock()->MakeConstRef()));
				classPtr->AddFunction("second", make_callable([](thisType& r) -> Any { return r.second; }, ParamTypes({ user_type_shared<thisType>().lock()->MakeRef() }), user_type_shared<Var>().lock()->MakeRef()));
				classPtr->AddFunction("second", make_callable([](thisType const& r) -> Any { return r.second; }, ParamTypes({ user_type_shared<thisType>().lock()->MakeConstRef() }), user_type_shared<Var>().lock()->MakeConstRef()));

				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x)->std::string {
					if (auto Self = self.lock()) {
						std::string a = Self->Cast<std::string>(Self->CallFunction("to_string", { x.first.p_data }));
						std::string b = Self->Cast<std::string>(Self->CallFunction("to_string", { x.second.p_data }));
						return GoodLang::printf("<%s, %s>", a.c_str(), b.c_str());
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

			// Vector
			if (1) {
				using thisType = Vector<Var>;
				std::string thisTypeName = "Vector";

				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);
				auto thisTypeInfo = classPtr->ClassType;

				// Constructors
				classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
				classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
				classPtr->AddFunction("=", make_callable(
					[](Any const& a, thisType const& b) -> Any { thisType& out = a.cast(); out = b; return a; }
				    , ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })
					, thisTypeInfo->MakeRef()
				));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](thisType const& x, thisType const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](thisType const& x, thisType const& y) -> bool { return x != y; }));

				// Functions
				classPtr->AddFunction("size", make_callable(&thisType::size));
				classPtr->AddFunction("empty", make_callable(&thisType::empty));
				classPtr->AddFunction("capacity", make_callable(&thisType::capacity));
				classPtr->AddFunction("reserve", make_callable(&thisType::reserve));
				classPtr->AddFunction("max_size", make_callable(&thisType::max_size));
				classPtr->AddFunction("clear", make_callable(&thisType::clear));
				classPtr->AddFunction("erase", make_callable(&thisType::erase));
				classPtr->AddFunction("erase_fast", make_callable(&thisType::erase_fast));
				classPtr->AddFunction("pop_back", make_callable(&thisType::pop_back));
				classPtr->AddFunction("push_back", make_callable([](thisType& o, Any const& r) { o.push_back(Var(r)); }));
				classPtr->AddFunction("resize", make_callable([](thisType& o, size_t N) { o.resize(N); }));
				classPtr->AddFunction("resize", make_callable([](thisType& o, size_t N, Any const& r) { o.resize(N, Var(r)); }));

				// operator[], at, front, back
				classPtr->AddFunction("at", make_callable([](thisType const& o, size_t _Keyval) -> Var { 
					auto Shared = o.at(_Keyval);
					auto& var = *Shared;
					std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(var), [_lock = Shared.ForwardLock(), _shared = Shared->p_data](Any* p) {
						delete p;
					});
					return Var(toReturn);
				}));
				classPtr->AddFunction("[]", make_callable([](thisType& o, size_t _Keyval) -> Var {
					auto Shared = o[_Keyval];
					auto& var = *Shared;
					std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(var), [_lock = Shared.ForwardLock(), _shared = Shared->p_data](Any* p) {
						delete p;
					});
					return Var(toReturn);
				}));
				classPtr->AddFunction("[]", make_callable([](thisType const& o, size_t _Keyval) -> Var {
					auto Shared = o[_Keyval];
					auto& var = *Shared;
					std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(var), [_lock = Shared.ForwardLock(), _shared = Shared->p_data](Any* p) {
						delete p;
					});
					return Var(toReturn);
				}));
				classPtr->AddFunction("front", make_callable([](thisType const& o) -> Var {
					auto Shared = o.front();
					auto& var = *Shared;
					std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(var), [_lock = Shared.ForwardLock(), _shared = Shared->p_data](Any* p) {
						delete p;
					});
					return Var(toReturn);
				}));
				classPtr->AddFunction("back", make_callable([](thisType const& o) -> Var {
					auto Shared = o.back();
					auto& var = *Shared;
					std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(var), [_lock = Shared.ForwardLock(), _shared = Shared->p_data](Any* p) {
						delete p;
					});
					return Var(toReturn);
				}));
				// Returns a string
				this->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x) -> std::string {
					if (auto Self = self.lock()) {
						std::string r;
						for (auto& i : x) {
							if (r.empty())
								r = Self->Cast<std::string>(Self->CallFunction("to_string", { i.p_data }));
							else {
								r += ", ";
								r += Self->Cast<std::string>(Self->CallFunction("to_string", { i.p_data }));
							}
						}
						return GoodLang::printf("[%s]", r.c_str());
					}
					else {
						throw exception::not_found_error("to_string");
					}
				}));
				// Returns a hash
				this->AddFunction("to_hash", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x) -> size_t {
					if (auto Self = self.lock()) {
						size_t o{ 0 };
						for (auto& i : x) {
							if (o == 0) {
								o = Self->Cast<size_t>(Self->CallFunction("to_hash", { i.p_data }));
							}
							else {
								Scope::hash_combine( o, Self->Cast<size_t>(Self->CallFunction("to_hash", { i.p_data })) );
							}
						}
						return o;
					}
					else {
						throw exception::not_found_error("to_hash");
					}
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
					}, ParamTypes({ thisIteratorTypeInfo->MakeRef(), thisIteratorTypeInfo->MakeConstRef() }), thisIteratorTypeInfo->MakeRef()));

					// equality
					iterator_classPtr->AddFunction("==", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs == rhs; }));
					iterator_classPtr->AddFunction("!=", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs != rhs; }));
					
					// (optional) distance
					iterator_classPtr->AddFunction("-", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> size_t { return lhs - rhs; }));

					// iter
					iterator_classPtr->AddFunction("++", make_callable([](Any const& a) -> Any {
						thisIteratorType& out = a.cast();
						out.operator++();
						return a;
					}, ParamTypes({ thisIteratorTypeInfo->MakeRef() }), thisIteratorTypeInfo->MakeRef()));
					// (optional) jump
					iterator_classPtr->AddFunction("+=", make_callable([](Any const& a, size_t diff) -> Any {
						thisIteratorType& out = a.cast();
						out.operator+=(diff);
						return a;
					}, ParamTypes({ thisIteratorTypeInfo->MakeRef(), user_type_shared<size_t>() }), thisIteratorTypeInfo->MakeRef()));
					// get // must be implimented by the iterator
					iterator_classPtr->AddFunction("get", make_callable([](thisIteratorType const& makeCopy) -> Var {
						std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(*makeCopy), [_lock = makeCopy](Any* p) {
							delete p;
						});
						return Var(toReturn);
					}));
				}
				classPtr->AddFunction("begin", make_callable([](thisType const& r)->thisType::iterator {
					return r.begin();
				}));
				classPtr->AddFunction("end", make_callable([](thisType const& r)->thisType::iterator {
					return r.end();
				}));

			}

			// Map.
			if (1) {
				using thisType = Map<size_t, std::pair<Var, Var>>;
				std::string thisTypeName = "Map";

				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);
				auto thisTypeInfo = classPtr->ClassType;

				// Constructors
				classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
				classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
				classPtr->AddFunction("=", make_callable(
					[](Any const& a, thisType const& b) -> Any { thisType& out = a.cast(); out = b; return a; }
					, ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() })
					, thisTypeInfo->MakeRef()
				));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](thisType const& x, thisType const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", make_callable([](thisType const& x, thisType const& y) -> bool { return x != y; }));

				// Functions
				classPtr->AddFunction("size", make_callable(&thisType::size));
				classPtr->AddFunction("empty", make_callable(&thisType::empty));				
				classPtr->AddFunction("clear", make_callable(&thisType::clear));
				classPtr->AddFunction("erase", make_callable([Self = classPtr->p_self](thisType& r, Any const& key)->size_t {
					if (auto scope = Self.lock()) {
						auto _key = scope->Cast<size_t>(scope->CallFunction("to_hash", { key }));
						return r.erase(_key);
					}
					else {
						throw std::runtime_error("Class not available");
					}
				}));
				classPtr->AddFunction("count", make_callable([Self = classPtr->p_self](thisType& r, Any const& key)->size_t {
					if (auto scope = Self.lock()) {
						auto _key = scope->Cast<size_t>(scope->CallFunction("to_hash", { key }));
						return r.count(_key);
					}
					else {
						throw std::runtime_error("Class not available");
					}
				}));
				classPtr->AddFunction("at", make_callable([Self = classPtr->p_self](thisType const& o, Any const& key) -> Var {
					if (auto scope = Self.lock()) {
						auto _key = scope->Cast<size_t>(scope->CallFunction("to_hash", { key }));
						auto Shared = o.at(_key);
						std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(Shared->second), [_lock = Shared.ForwardLock(), _shared = Shared->second.p_data](Any* p) {
							delete p;
						});
						return Var(toReturn);
					}
					else {
						throw std::runtime_error("Class not available");
					}
				}));
				classPtr->AddFunction("[]", make_callable([Self = classPtr->p_self](thisType const& o, Any const& key) -> Var {
					if (auto scope = Self.lock()) {
						auto _key = scope->Cast<size_t>(scope->CallFunction("to_hash", { key }));
						auto Shared = o.at(_key);
						auto& var = *Shared;
						std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(Shared->second), [_lock = Shared.ForwardLock(), _shared = Shared->second.p_data](Any* p) {
							delete p;
						});
						return Var(toReturn);
					}
					else {
						throw std::runtime_error("Class not available");
					}
				}));
				classPtr->AddFunction("[]", make_callable([Self = classPtr->p_self](thisType& o, Any const& key) -> Var {
					if (auto scope = Self.lock()) {
						auto _key = scope->Cast<size_t>(scope->CallFunction("to_hash", { key }));
						auto Shared = o.get_or_insert(_key, std::pair<Var, Var>{ Var(key), Var() });
						auto& var = *Shared;
						std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(Shared->second), [_lock = Shared.ForwardLock(), _shared = Shared->second.p_data](Any* p) {
							delete p;
						});
						return Var(toReturn);
					}
					else {
						throw std::runtime_error("Class not available");
					}
				}));
				classPtr->AddFunction("emplace", make_callable([Self = classPtr->p_self](thisType& o, Any const& key, Any const& value) -> void {
					if (auto scope = Self.lock()) {
						auto _key = scope->Cast<size_t>(scope->CallFunction("to_hash", { key }));
						o.get_or_insert(_key, std::pair<Var, Var>{ Var(key), Var(value) });						
					}
					else {
						throw std::runtime_error("Class not available");
					}
				}));

				// Returns a string
				this->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x)->std::string {
					if (auto Self = self.lock()) {
						std::string r;
						for (auto& i : x) {
							if (r.empty())
								r = Self->Cast<std::string>(Self->CallFunction("to_string", { i.second }));
							else {
								r += ", ";
								r += Self->Cast<std::string>(Self->CallFunction("to_string", { i.second }));
							}
						}
						return GoodLang::printf("[%s]", r.c_str());
					}
					else {
						throw exception::not_found_error("to_string");
					}
				}));
				// Returns a hash
				this->AddFunction("to_hash", make_callable([self = std::weak_ptr<Class>(classPtr)](thisType const& x)->size_t {
					if (auto Self = self.lock()) {
						size_t o{ 0 };
						for (auto& i : x) {
							if (o == 0) {
								o = Self->Cast<size_t>(Self->CallFunction("to_hash", { i.second }));
							}
							else {
								Scope::hash_combine(o, Self->Cast<size_t>(Self->CallFunction("to_hash", { i.second })));
							}
						}
						return o;
					}
					else {
						throw exception::not_found_error("to_hash");
					}
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
					}, ParamTypes({ thisIteratorTypeInfo->MakeRef(), thisIteratorTypeInfo->MakeConstRef() }), thisIteratorTypeInfo->MakeRef()));

					// equality
					iterator_classPtr->AddFunction("==", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs == rhs; }));
					iterator_classPtr->AddFunction("!=", make_callable([](thisIteratorType const& lhs, thisIteratorType const& rhs) -> bool { return lhs != rhs; }));		

					// iter
					iterator_classPtr->AddFunction("++", make_callable([](Any const& a) -> Any {
						thisIteratorType& out = a.cast();
						out.operator++();
						return a;
					}, ParamTypes({ thisIteratorTypeInfo->MakeRef() }), thisIteratorTypeInfo->MakeRef()));
					// first (convenience function for this specialization)
					iterator_classPtr->AddFunction("first", make_callable([](thisIteratorType const& makeCopy) -> Any {
						std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(makeCopy->second.first), [_lock = makeCopy](Any* p) {
							delete p;
						});
						return Var(toReturn);
					}));
					// second (convenience function for this specialization)
					iterator_classPtr->AddFunction("second", make_callable([](thisIteratorType const& makeCopy) -> Any {
						std::shared_ptr<Any> toReturn = std::shared_ptr<Any>(new Any(makeCopy->second.second), [_lock = makeCopy](Any* p) {
							delete p;
						});
						return Var(toReturn);
					}));
					// get // must be implimented by the iterator
					iterator_classPtr->AddFunction("get", make_callable([](thisIteratorType const& makeCopy) -> std::pair<Var, Var> {
						std::shared_ptr<Any> toReturn1 = std::shared_ptr<Any>(new Any(makeCopy->second.first), [_lock = makeCopy](Any* p) {
							delete p;
						});
						std::shared_ptr<Any> toReturn2 = std::shared_ptr<Any>(new Any(makeCopy->second.second), [_lock = makeCopy](Any* p) {
							delete p;
						});
						return std::pair<Var, Var>{ Var(toReturn1), Var(toReturn2) };
					}));
				}
				classPtr->AddFunction("begin", make_callable([Self = classPtr->p_self](thisType const& r)->thisType::iterator {
					return r.begin();
				}));
				classPtr->AddFunction("end", make_callable([Self = classPtr->p_self](thisType const& r)->thisType::iterator {
					return r.end();
				}));

			}

			// Proxy_Function
			if (1) {
				using thisType = details::Proxy_Function_Base;
				using thisTypeShared = Proxy_Function;
				std::string thisTypeName = "Function";
				auto anyType = user_type_shared<Any>();

				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type_shared<thisType>().lock()));
				}
				classPtr->p_self = classPtr;
				this->AddChild(classPtr);
				auto thisTypeInfo = classPtr->ClassType;

				// Constructors
				classPtr->AddFunction(thisTypeName, make_callable([]() -> Any { return thisTypeShared{}; }, ParamTypes(), thisTypeInfo));
				classPtr->AddFunction(thisTypeName, make_callable([](Any const& makeCopy) -> Any { 
					return makeCopy;
				}, ParamTypes({ thisTypeInfo->MakeConstRef() }), thisTypeInfo));
				classPtr->AddFunction("=", make_callable([](Any const& a, Any const& makeCopy) -> Any {
					return makeCopy;
				}, ParamTypes({ thisTypeInfo->MakeRef(), thisTypeInfo->MakeConstRef() }), thisTypeInfo->MakeRef()));

				// Comparisons
				classPtr->AddFunction("==", make_callable([](Any const& x, Any const& y) -> bool { return x == y; }, ParamTypes({ thisTypeInfo->MakeConstRef(), thisTypeInfo->MakeConstRef() })));
				classPtr->AddFunction("!=", make_callable([](Any const& x, Any const& y) -> bool { return x != y; }, ParamTypes({ thisTypeInfo->MakeConstRef(), thisTypeInfo->MakeConstRef() })));

				classPtr->AddFunction("Returns", make_callable([](Any const& o) -> std::weak_ptr<Type_Info> {
					if (auto ptr = o.cast<thisTypeShared>()) {
						return ptr->Returns();
					}
					else {
						return user_type_shared<void>();
					}
				}, ParamTypes({ thisTypeInfo->MakeConstRef() })));
				classPtr->AddFunction("Argument", make_callable([](Any const& o, size_t i) -> std::weak_ptr<Type_Info> {
					if (auto ptr = o.cast<thisTypeShared>()) {
						return ptr->Argument(i);
					}
					else {
						return user_type_shared<void>();
					}
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), user_type_shared<size_t>() })));
				classPtr->AddFunction("Arguments", make_callable([](Any const& o) -> Vector<Var> {
					Vector<Var> out;
					if (auto ptr = o.cast<thisTypeShared>()) {
						for (auto& x : ptr->Arguments()) {
							out.push_back(Var(Any((std::weak_ptr<Type_Info>)x)));
						}
					}
					return out;				
				}, ParamTypes({ thisTypeInfo->MakeConstRef() })));
				classPtr->AddFunction("NumArguments", make_callable([](Any const& o) -> size_t {
					if (auto ptr = o.cast<thisTypeShared>()) {
						return ptr->NumArguments();
					}
					else return 0;
				}, ParamTypes({ thisTypeInfo->MakeConstRef() })));
				classPtr->AddFunction("to_hash", make_callable([](Any const& o) -> size_t {
					if (auto ptr = o.cast<thisTypeShared>()) {
						return ptr->hash();
					}
					else return 0;
				}, ParamTypes({ thisTypeInfo->MakeConstRef() })));
				classPtr->AddFunction("to_string", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o) -> std::string {
					if (auto Self = self.lock()) {
						if (auto ptr = o.cast<thisTypeShared>()) {
							auto sig = ptr->GetSignature();
							std::string out;
							auto returns = Self->Cast<std::string>(Self->CallFunction("to_string", { sig.Returns() }));
							if (!sig.Name().empty()) {
								// named function, probably from the FunctionsMap containers
								// int GetInt(double, double)

								out = returns;
								out += " ";
								out += sig.QualifiedName();
								out += "(";
								size_t i = 0;
								for (; (i < ptr->NumArguments()) && (i < 1); i++) {
									out += Self->Cast<std::string>(Self->CallFunction("to_string", { ptr->Argument(i) }));
								}
								for (; i < ptr->NumArguments(); i++) {
									out += ", ";
									out += Self->Cast<std::string>(Self->CallFunction("to_string", { ptr->Argument(i) }));
								}
								out += ")";
							} else {
								// may be a lambda or free function?
								// (double, double) -> int
								out = "(";
								size_t i = 0;
								for (; (i < ptr->NumArguments()) && (i < 1); i++) {
									out += Self->Cast<std::string>(Self->CallFunction("to_string", { ptr->Argument(i) }));
								}
								for (; i < ptr->NumArguments(); i++) {
									out += ", ";
									out += Self->Cast<std::string>(Self->CallFunction("to_string", { ptr->Argument(i) }));
								}
								out += ") -> ";
								out += returns;
							}
							return out;
						}
					}
					else throw exception::not_found_error("to_string");
				}, ParamTypes({ thisTypeInfo->MakeConstRef() })));

				classPtr->AddFunction("Invoke", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Vector<Var> const& Params) -> Var {
					std::vector<Any> T; for (auto& x : Params) { T.push_back(x.p_data); }
					if (auto Self = self.lock()) {
						auto tree = Self->GetTypeConverterTree();
						if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, T, *tree));						
					}
					return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), user_type_shared<Vector<Var>>().lock()->MakeConstRef() })));

				classPtr->AddFunction("()", make_callable([](Any const& o) -> Var {
					if (auto ptr = o.cast<thisTypeShared>()) {
						return Var(ptr->operator()(std::vector<Any>{}));
					}
					return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef() })));

#define Do(x, N) x ## N
#define Do1(x) Do(x, 1)
#define Do2(x) Do(x, 1), Do(x, 2)
#define Do3(x) Do(x, 1), Do(x, 2), Do(x, 3)
#define Do4(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4)
#define Do5(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), Do(x, 5)
#define Do6(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), Do(x, 5), Do(x, 6)
#define Do7(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), Do(x, 5), Do(x, 6), Do(x, 7)
#define Do8(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8)
#define Do9(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8), Do(x, 9)
#define Do10(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), \
                Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8), Do(x, 9), Do(x, 10)
#define Do11(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), \
                Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8), Do(x, 9), Do(x, 10), Do(x, 11)
#define Do12(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), \
                Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8), Do(x, 9), Do(x, 10), Do(x, 11), Do(x, 12)
#define Do13(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), \
                Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8), Do(x, 9), Do(x, 10), Do(x, 11), Do(x, 12), Do(x, 13)
#define Do14(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8), \
                Do(x, 9), Do(x, 10), Do(x, 11), Do(x, 12), Do(x, 13), Do(x, 14)
#define Do15(x) Do(x, 1), Do(x, 2), Do(x, 3), Do(x, 4), Do(x, 5), Do(x, 6), Do(x, 7), Do(x, 8), \
                Do(x, 9), Do(x, 10), Do(x, 11), Do(x, 12), Do(x, 13), Do(x, 14), Do(x, 15)

#define Repeat(x, N) x
#define Repeat1(x) Repeat(x, 1)
#define Repeat2(x) Repeat(x, 1), Repeat(x, 2)
#define Repeat3(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3)
#define Repeat4(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4)
#define Repeat5(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), Repeat(x, 5)
#define Repeat6(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), Repeat(x, 5), Repeat(x, 6)
#define Repeat7(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), Repeat(x, 5), Repeat(x, 6), Repeat(x, 7)
#define Repeat8(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8)
#define Repeat9(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8), Repeat(x, 9)
#define Repeat10(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), \
                Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8), Repeat(x, 9), Repeat(x, 10)
#define Repeat11(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), \
                Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8), Repeat(x, 9), Repeat(x, 10), Repeat(x, 11)
#define Repeat12(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), \
                Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8), Repeat(x, 9), Repeat(x, 10), Repeat(x, 11), Repeat(x, 12)
#define Repeat13(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), \
                Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8), Repeat(x, 9), Repeat(x, 10), Repeat(x, 11), Repeat(x, 12), Repeat(x, 13)
#define Repeat14(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8), \
                Repeat(x, 9), Repeat(x, 10), Repeat(x, 11), Repeat(x, 12), Repeat(x, 13), Repeat(x, 14)
#define Repeat15(x) Repeat(x, 1), Repeat(x, 2), Repeat(x, 3), Repeat(x, 4), Repeat(x, 5), Repeat(x, 6), Repeat(x, 7), Repeat(x, 8), \
                Repeat(x, 9), Repeat(x, 10), Repeat(x, 11), Repeat(x, 12), Repeat(x, 13), Repeat(x, 14), Repeat(x, 15)

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do1(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do1(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat1(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do2(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do2(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat2(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do3(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do3(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat3(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do4(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do4(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat4(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do5(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do5(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat5(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do6(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do6(p) }, * tree)); } return {};	
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat6(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do7(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do7(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat7(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do8(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do8(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat8(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do9(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do9(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat9(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do10(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do10(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat10(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do11(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do11(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat11(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do12(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do12(p) }, * tree)); } return {};
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat12(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do13(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do13(p) }, * tree)); } return {};	
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat13(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do14(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do14(p) }, * tree)); } return {}; 
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat14(anyType) })));

				classPtr->AddFunction("()", make_callable([self = std::weak_ptr<Class>(classPtr)](Any const& o, Do15(Any const& p)) -> Var {
					if (auto Self = self.lock()) { auto tree = Self->GetTypeConverterTree(); if (auto ptr = o.cast<thisTypeShared>()) return Var(call(ptr, std::vector<Any>{ Do15(p) }, * tree)); } return {}; 
				}, ParamTypes({ thisTypeInfo->MakeConstRef(), Repeat15(anyType) })));

#undef Repeat15
#undef Repeat14
#undef Repeat13
#undef Repeat12
#undef Repeat11
#undef Repeat10
#undef Repeat9
#undef Repeat8
#undef Repeat7
#undef Repeat6
#undef Repeat5
#undef Repeat4
#undef Repeat3
#undef Repeat2
#undef Repeat1
#undef Repeat
#undef Do15
#undef Do14
#undef Do13
#undef Do12
#undef Do11
#undef Do10
#undef Do9
#undef Do8
#undef Do7
#undef Do6
#undef Do5
#undef Do4
#undef Do3
#undef Do2
#undef Do1
#undef Do
			}
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
			// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
			this->AddFunction("print", make_callable([](Any const& x) -> void {
				std::cout << GoodLang::ToString(x) + "\n";
			}));
		}
	};

};