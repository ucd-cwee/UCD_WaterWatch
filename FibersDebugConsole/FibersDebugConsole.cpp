// FibersDebugConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include "../FiberTasks/Fibers.h"
#include "../FiberTasks/ScriptingLanguage.h"
#include "../FiberTasks/UnitsLibrary.h"
#include <execution>
#include "../WaterWatchCpp/Clock.h"

class stackThing {
public:
	std::string varName;
	fibers::Any var;

public:
	stackThing() : varName(), var() {};
	stackThing(std::string const& name) : varName(name), var() {};
	template<typename T> stackThing(std::string const& name, T const& obj) : varName(name), var(obj) {};
	template<typename T> stackThing(std::string const& name, T&& obj) : varName(name), var(std::forward<T>(obj)) {};
	stackThing(stackThing const& r) : varName(r.varName), var(r.var) {};
	stackThing(stackThing&& r) : varName(std::move(r.varName)), var(std::move(r.var)) {};
	stackThing& operator=(stackThing const& r) { varName = r.varName; var = r.var; return *this; };
	stackThing& operator=(stackThing&& r) { varName = std::move(r.varName); var = std::move(r.var); return *this; };
	~stackThing() { 
		if (!varName.empty()) { 
			std::cout << Units::printf("DELETING %s\n", varName.c_str());
		}
	};
	int length() const { return varName.length(); };
	std::string& get_var_name() { return varName; };
	bool operator==(stackThing const& a) const { return varName == a.varName; };
	bool operator!=(stackThing const& a) const { return varName != a.varName; };
};

static bool Thing() { return true; };

#define SINGLE_ARG(...) __VA_ARGS__
#define EXPECT_EQ(a,b) [&]()->bool{ if ((a) == (b)) { return true; } else { std::cout << Units::printf("FAILURE AT LINE %i\n", (int)__LINE__); return false; } }()
#define EXPECT_NE(a, b) [&]()->bool{ if ((a) != (b)) { return true; } else { std::cout << Units::printf("FAILURE AT LINE %i\n", (int)__LINE__); return false; } }()

int main() {
	// pre-warm the heap
	for (int i = 0; i < 100000; i++) delete (new int(5));


	fibers::utilities::Computer_Usage usage_start;
	auto TestMemory = [&](long currentLine) {
		fibers::utilities::Computer_Usage currentMemory;

		auto memoryIncrease = currentMemory.CurrentRSS() - usage_start.CurrentRSS();
		if (memoryIncrease != 0) {
			// std::printf("%i virtual memory use increase at %i\n", (int)(float)(double)(memoryIncrease), (int)currentLine);
		}
		usage_start = currentMemory;
	};
	TestMemory(__LINE__);



	while (1) {
		std::this_thread::yield();

		// Scripting language 
		if (0) {
			if (1) {
				fibers::containers::Map<std::string, double> map;
				EXPECT_EQ(true, map.emplace("x", 100));
				EXPECT_EQ(map.at("x"), 100);
				EXPECT_EQ(map.contains("x"), true);
				EXPECT_EQ(map.contains("y"), false);
				EXPECT_EQ(map.emplace("y", -2), true);
				EXPECT_EQ(map.contains("y"), true);
				EXPECT_EQ(map.key_of(100).value(), "x");
				EXPECT_EQ(map.key_of(-2).value(), "y");
				EXPECT_EQ(map.key_of(5).has_value(), false);
				EXPECT_EQ(map.erase("y"), true);
				EXPECT_EQ(map.contains("y"), false);				
				for (auto& key : map.keys()) {
					std::cout << key << std::endl;
				}
				for (auto& pair : map) {
					if (pair) {
						std::cout << pair->first << ", " << pair->second << std::endl;
					}
				}
			}
			if (1) {
				fibers::containers::Map<std::string, std::shared_ptr<stackThing>> map;
				EXPECT_EQ(true, map.emplace("x", std::make_shared<stackThing>("x", 100)));
				EXPECT_EQ(map.at("x").value()->get_var_name(), "x");
				EXPECT_EQ(map.contains("x"), true);
				EXPECT_EQ(map.contains("y"), false);
				EXPECT_EQ(map.emplace("y", std::make_shared<stackThing>("y", -2)), true);
				EXPECT_EQ(map.contains("y"), true);
				EXPECT_EQ(map.erase("y"), true);
				EXPECT_EQ(map.contains("y"), false);
				for (auto& key : map.keys()) {
					std::cout << key << std::endl;
				}
				for (auto& pair : map) {
					if (pair) {
						std::cout << pair->first << ", " << pair->second->get_var_name() << std::endl;
					}
				}
			}
			if (1) {
				fibers::containers::Map<std::string, std::shared_ptr<stackThing>> map;
				fibers::parallel::For(0, 100, [&](int i) {
					map.emplace(Units::printf("%i", i), std::make_shared<stackThing>(Units::printf("%i", i), i));
				});
				EXPECT_EQ(map.size(), 100);
			}
			if (1) {
				fibers::containers::Map<std::string, std::shared_ptr<stackThing>> map;
				fibers::parallel::For(0, 1000, [&](int i) {
					if (i % 2 == 0) {
						map.emplace(Units::printf("%i", i), std::make_shared<stackThing>(Units::printf("%i", i), i));
					}
					else {
						for (auto& x : map) {
							if (x) {
								if (x->first == "-1") {
									std::cout << x->first << std::endl;
								}
							}
						}
					}
				});
				EXPECT_EQ(map.size() >= 499, true);
			}
			if (1) {
				fibers::containers::Map<std::string, std::shared_ptr<stackThing>> map;
				fibers::parallel::For(0, 500, [&](int i) {
					if (i % 2 == 0) {
						map.emplace(Units::printf("%i", i), std::make_shared<stackThing>(Units::printf("%i", i), i));
					}
					else {
						std::string keyToErase;
						for (auto& x : map) {
							if (x) {
								keyToErase = x->first;
								break;
							}
						}
						map.erase(keyToErase);
					}
				});
			}
			if (1) {
				fibers::containers::Map<std::string, std::shared_ptr<stackThing>> map;
				fibers::parallel::For(0, 100, [&](int i) {
					map.emplace(Units::printf("%i", i), std::make_shared<stackThing>(Units::printf("%i", i), i));
				});
				fibers::parallel::ForEach(map, [&](auto& pair) {
					if (pair)
						std::cout << pair->first << std::endl;
				});
			}
		}

#if 0
		if (1) {
			scripting::ScriptingState objs;

			EXPECT_EQ(objs.AddType("int", scripting::user_type<int>()), true);
			EXPECT_EQ(objs.AddType("float", scripting::user_type<float>()), true);
			EXPECT_EQ(objs.AddType("double", scripting::user_type<double>()), true);
			EXPECT_EQ(objs.AddType("std::string", scripting::user_type<std::string>()), true);
			EXPECT_EQ(objs.GetType("int").value(), scripting::user_type<int>());
			EXPECT_EQ(objs.GetType("float").value(), scripting::user_type<float>());
			for (auto& obj : objs.GetTypes()) {}

			objs.AddObj("x", 100);
			objs.AddObj("PI", Units::constants::pi());
			EXPECT_EQ(objs.GetObj("x").value().cast<int>(), 100);
			EXPECT_EQ(objs.GetObj("PI").value().cast<Units::scalar>() > 3, true);
			for (auto& obj : objs.GetObjects()) {}

			objs.AddFunction("std::to_string", scripting::make_callable([](int i)->std::string { return std::to_string(i); }));
			objs.AddFunction("std::to_string", scripting::make_callable([](long i)->std::string { return std::to_string(i); }));
			objs.AddFunction("std::to_string", scripting::make_callable([](float i)->std::string { return std::to_string(i); }));
			objs.AddFunction("std::to_string", scripting::make_callable([](double i)->std::string { return std::to_string(i); }));
			for (auto& func_group : objs.GetFunctions()) {
				for (auto& func_pair : *func_group.second) {
					auto& func{ func_pair.second };
				}
			}

			{ // EXPECTED TO WORK
				std::vector<fibers::Any> params{ fibers::Any{ 1 } };
				auto func = objs.GetFunction("std::to_string", params);
				EXPECT_EQ(func.has_value(), true);
				auto result = scripting::call(func.value(), params, objs.GetConversionTree());
				EXPECT_EQ(result.Type(), fibers::user_type<std::string>());
			}
			{ // EXPECTED TO WORK
				std::vector<fibers::Any> params{ fibers::Any{ 1l } };
				auto func = objs.GetFunction("std::to_string", params);
				EXPECT_EQ(func.has_value(), true);
				auto result = scripting::call(func.value(), params, objs.GetConversionTree());
				EXPECT_EQ(result.Type(), fibers::user_type<std::string>());
			}
			{ // EXPECTED TO WORK
				std::vector<fibers::Any> params{ fibers::Any{ 1.0f } };
				auto func = objs.GetFunction("std::to_string", params);
				EXPECT_EQ(func.has_value(), true);
				auto result = scripting::call(func.value(), params, objs.GetConversionTree());
				EXPECT_EQ(result.Type(), fibers::user_type<std::string>());
			}
			{ // EXPECTED TO WORK
				std::vector<fibers::Any> params{ fibers::Any{ 1.1 } };
				auto func = objs.GetFunction("std::to_string", params);
				EXPECT_EQ(func.has_value(), true);
				auto result = scripting::call(func.value(), params, objs.GetConversionTree());
				EXPECT_EQ(result.Type(), fibers::user_type<std::string>());
			}
			{ // EXPECTED TO FAIL
				std::vector<fibers::Any> params{ fibers::Any{ 1.1l } };
				auto func = objs.GetFunction("std::to_string", params);
				EXPECT_EQ(func.has_value(), false);
			}







		}
#endif

		// Leak Test
		if (0) {
			delete (new int(5));
			if (true) {
				fibers::containers::Pattern<double, double> test;
			}
			delete (new int(5));
		}

		if (0) {
			delete (new int(5));
			if (true) {
				fibers::utilities::GarbageCollectedAllocator<std::pair<std::string, double>> nodeAllocator;
			}
			delete (new int(5));
		}

		if (0) {
			delete (new int(5));
			if (true) {
				fibers::containers::Pattern<double, double> test;
				test.Insert(1, 1, true);
			}
			delete (new int(5));
		}

		if (0) {
			delete (new int(5));
			if (true) {
				fibers::utilities::GarbageCollectedAllocator<std::pair<std::string, double>> nodeAllocator;
				nodeAllocator.Free(nodeAllocator.Alloc());
			}
			delete (new int(5));
		}

#if 0
		if (1) {
			delete (new int(5));
			if (true) {
				fibers::containers::Map2<std::string, std::shared_ptr<fibers::containers::number<double>>> obj;
				obj.emplace("TEST", std::make_shared<fibers::containers::number<double>>(100));
				obj.emplace("TEST2", std::make_shared<fibers::containers::number<double>>(100));
				

				int i = 0;
				for (auto& objs : obj) { i++; }
				EXPECT_EQ(2, i);

				obj.emplace("TEST3", std::make_shared<fibers::containers::number<double>>(100));

				i = 0;
				for (auto& objs : obj) { i++; }
				EXPECT_EQ(3, i);


				EXPECT_EQ(true, obj.erase("TEST"));
				EXPECT_EQ(2, obj.size());
				EXPECT_EQ(false, obj.contains("TEST"));
				EXPECT_EQ(true, obj.contains("TEST2"));

				fibers::containers::number<double> D{ 0 };
				fibers::parallel::ForEach(obj, [&D](auto& iter) {
					D++;
				});
				EXPECT_EQ(2, D);



				fibers::parallel::For(0, 1000, [&obj](int i) {
					
					for (auto& iter : obj) {
						if (iter)
							*iter->second += 1;
					}
					obj.emplace(std::to_string(i), std::make_shared<fibers::containers::number<double>>(i));

				});

				auto printf = [](auto x) { std::cout << x << std::endl; };
				fibers::parallel::ForEach(obj, [&D, &printf](auto& iter) {
					if (iter)
						printf(iter->first.c_str());
				});

				try {
					fibers::parallel::For(0, 1000, [&obj](int i) {
						for (auto& iter : obj) {
							if (iter)
								*iter->second += 1;
						}
						obj.erase(std::to_string(i));

					});
				}
				catch (std::exception& e) {
					printf(e.what());
				}




			}
			delete (new int(5));
		}

		if (1) {
			delete (new int(5));
			if (true) {
				fibers::containers::Map2<std::string, std::shared_ptr<int>> obj;
				obj.emplace("TEST", std::shared_ptr<int>());
				obj.emplace("TEST2", std::shared_ptr<int>());


				int i = 0;
				for (auto& objs : obj) { i++; }
				EXPECT_EQ(2, i);

				obj.emplace("TEST3", std::shared_ptr<int>());

				i = 0;
				for (auto& objs : obj) { i++; }
				EXPECT_EQ(3, i);


				EXPECT_EQ(true, obj.erase("TEST"));
				EXPECT_EQ(2, obj.size());
				EXPECT_EQ(false, obj.contains("TEST"));
				EXPECT_EQ(true, obj.contains("TEST2"));

				fibers::containers::number<double> D{ 0 };
				fibers::parallel::ForEach(obj, [&D](auto& iter) {
					D++;
				});
				EXPECT_EQ(2, D);


				D = 0;
				fibers::parallel::For(0, 1000, [&obj, &D](int i) {

					for (auto& iter : obj) {
						if (iter)
							D++;
					}
					obj.emplace(std::to_string(i), std::shared_ptr<int>());

				});

				auto printf = [](auto x) { std::cout << x << std::endl; };
				fibers::parallel::ForEach(obj, [&D, &printf](auto& iter) {
					if (iter)
						printf(iter->first.c_str());
					});

				try {
					D = 0;
					fibers::parallel::For(0, 1000, [&obj, &D](int i) {
						for (auto& iter : obj) {
							if (iter)
								D++;
						}
						obj.erase(std::to_string(i));
					});
				}
				catch (std::exception& e) {
					printf(e.what());
				}




			}
			delete (new int(5));
		}

		if (1) {
			delete (new int(5));
			if (true) {
				fibers::containers::Map2<std::string, std::shared_ptr<int>> m_functions;
			}
			delete (new int(5));
		}

		if (1) {
			delete (new int(5));
			if (true) {
				fibers::containers::Map2 <
					std::string, // Function Name (e.g. string). 
					std::shared_ptr<fibers::containers::Map2<
						scripting::Param_Types, // Function parameters (e.g. {string, Any}, or {Any, Any, Any}). 
					    scripting::Proxy_Function
					>>
				> m_functions;



			}
			delete (new int(5));
		}
#endif

#if 0
		if (1) {
			delete (new int(5));
			if (true) {
				scripting::Functions funcs; // LEAK
			}
			delete (new int(5));
		}

		if (1) {
			delete (new int(5));
			if (true) {
				scripting::Functions funcs;
				funcs.emplace("Foo", scripting::make_callable([](){}), true);
				funcs.emplace("Bar", scripting::make_callable([](int x){}), true);
			}
			delete (new int(5));
		}

		if (1) {
			delete (new int(5));
			if (true) {
				scripting::Functions funcs;
				funcs.emplace("Foo", scripting::make_callable([]() {}));
				funcs.emplace("Bar", scripting::make_callable([](int x) {}));
				funcs.emplace("Foo", scripting::make_callable([](int x) {}));
				funcs.emplace("Bar", scripting::make_callable([](int x, int y) {}));
			}
			delete (new int(5));
		}

		if (1) {
			delete (new int(5));
			if (true) {
				auto global_scope{ std::make_shared<scripting::Global>() };
				global_scope->p_self = global_scope;
				global_scope->AddBuiltIns();
			}
			delete (new int(5));
		}

		if (1) {
			delete (new int(5));
			if (true) {
				auto global_scope{ std::make_shared<scripting::Global>() };
				global_scope->p_self = global_scope;
				global_scope->AddBuiltIns();
				delete (new int(5));
			}
			delete (new int(5));
		}
#endif


		// Re-build 2 Test
		try {
			using namespace scripting;
			auto printf = [](auto x) { std::cout << x << std::endl; };
			Stopwatch sw{};

			fibers::containers::Map<std::string, std::shared_ptr<Global2>> imports;

			auto scope_1 = std::make_shared<Global2>(); // ::
			scope_1->SetSelf(scope_1);
			scope_1->AddBuiltIns();

			int numIterations = 1000000;

			// FindNamespace
			if (1) {
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto namespacePtr = scope_1->FindNamespace("string")) {

					}
					else {
						EXPECT_EQ(true, false);
					}
					});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			}

			// FindClass
			if (1) {
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto namespacePtr = scope_1->FindClass(user_type<std::string>())) {

					}
					else {
						EXPECT_EQ(true, false);
					}
					});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			}

			// Find Objects
			if (1) {
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto p = scope_1->FindObj("npos")) {
						EXPECT_EQ(p->cast<size_t>(), std::string::npos);
					}
					else {
						EXPECT_EQ(true, false);
					}
					});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto p = scope_1->FindObj("::string::npos")) {
						EXPECT_EQ(p->cast<size_t>(), std::string::npos);
					}
					else {
						EXPECT_EQ(true, false);
					}
					});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			}

			// Find Functions
			if (1) {
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto p = scope_1->FindFunctions("length")) {

					}
					else {
						EXPECT_EQ(true, false);
					}
					});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto p = scope_1->FindFunctions("::string::length")) {

					}
					else {
						EXPECT_EQ(true, false);
					}
					});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			}

			// Create and test the type conversion tree, which builds itself from the constructors of the various classes.
			if (1) {
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i)
					{
						auto tree = scope_1->GetTypeConverterTree(); // builds and caches the tree. Updates the tree only if the situation has changed (new functions, new classes, or new Using statements)

						if (!tree->Converts<int, long>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<float, long>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<int, double>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<bool, int>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<int, int>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<double, int>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<bool, double>()) { EXPECT_EQ(true, false); }

						EXPECT_EQ(100, tree->Convert<int>(100.0));
						EXPECT_EQ(100.0, tree->Convert<double>(100l));
						EXPECT_EQ(100l, tree->Convert<long>(100.0f));
						EXPECT_EQ(100.0f, tree->Convert<float>(100.0));
						EXPECT_EQ(true, tree->Convert<bool>(1));
						EXPECT_EQ(100.0f, tree->Convert<float>(100l));
						EXPECT_EQ(1.0, tree->Convert<double>(true));
					}
				);
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
			}

			// Create and test the type conversion tree, which builds itself from the constructors of the various classes.
			if (1) {
				std::vector<Any> params = { Any(fibers::containers::number<double>(0)), Any(1) };
				auto& n = params[0].cast<fibers::containers::number<double>&>();

				sw.Start();
				auto scope_outer = std::make_shared<Scope2>(scope_1);
				scope_outer->SetSelf(scope_outer);
				fibers::parallel::For(0, numIterations, [&](int i) {
					auto scope_inner = std::make_shared<Scope2>(scope_outer);
					scope_inner->SetSelf(scope_inner);
					scope_inner->AddObj("i", std::make_shared<Any>(i));
					{
						scope_inner->CallFunction("+=", params);
					}
					});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");

				{
					EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("==", { params[0], numIterations })));
				}

				EXPECT_EQ(n, (numIterations));
			}

			//EXPECT_EQ("100", scope_1->Cast<std::string>(scope_1->CallFunction("string", { 100 })));
			//EXPECT_EQ("200", scope_1->Cast<std::string>(scope_1->CallFunction("::string", { scope_1->Cast<int>(scope_1->CallFunction("+", { 100.0f, 100.0 })) })));

			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 100 })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 100.0f })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 100.0 })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { std::string("TEST") })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 'A' })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { Units::acre(1) })));

			//EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("!=", { scope_1->CallFunction("Type", { 100.0 }), scope_1->CallFunction("Type", { 100.0f }) })));
			//EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("==", { scope_1->CallFunction("Type", { 100.0 }), scope_1->CallFunction("Type", { 100.0 }) })));

#if 1
			// Units
			if (1) {
				// #include "Units"
				if (0) {
					auto global_scope2{ std::make_shared<Global2>() }; // global should always be a Namespace
					global_scope2->SetSelf(global_scope2);
					global_scope2->AddBuiltIns();

					// Create library...
					{
						auto std_namespace{ std::make_shared<Namespace2>(global_scope2, "Units") };
						std_namespace->SetSelf(std_namespace);
						global_scope2->AddChild(std_namespace);

						// the "std" namespace imports the "string" namespace...
						{
							auto value_namespace{ std::make_shared<Class2>(std_namespace, "value", scripting::user_type<Units::value>()) };
							value_namespace->SetSelf(value_namespace);
							std_namespace->AddChild(value_namespace);

							// which has the following types groups... 
							{
								// value -> double
								if (auto p = std_namespace->FindClass(user_type<double>())) {
									p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> double { return o(); }));
								}
								// value -> float
								if (auto p = std_namespace->FindClass(user_type<float>())) {
									p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> float { return o(); }));
								}
								// value -> int
								if (auto p = std_namespace->FindClass(user_type<int>())) {
									p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> int { return o(); }));
								}
								// value -> string
								if (auto p = std_namespace->FindClass(user_type<std::string>())) {
									p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> std::string { return o.ToString(); }));
								}

								value_namespace->AddFunction("abbreviation", make_callable([](Units::value const& x)->std::string {
									return x.Abbreviation();
								}));
								value_namespace->AddFunction("name", make_callable([](Units::value const& x)->std::string {
									return x.UnitName();
								}));
								value_namespace->AddFunction("to_string", make_callable([](Units::value const& x)->std::string {
									return x.ToString();
								}));

								// Constructors
								value_namespace->AddFunction("value", make_callable([]() -> Units::value { return Units::value{}; }));
								value_namespace->AddFunction("value", make_callable([](Units::value const& makeCopy) -> Units::value { return makeCopy; }));
								value_namespace->AddFunction("value", make_callable([](int const& o)->Units::value { return o; }));
								value_namespace->AddFunction("value", make_callable([](float const& o)->Units::value { return o; }));
								value_namespace->AddFunction("value", make_callable([](double const& o)->Units::value { return o; }));
								value_namespace->AddFunction("=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out = b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));

								// Comparisons & operators
								value_namespace->AddFunction("==", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x == y; }));
								value_namespace->AddFunction("!=", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x != y; }));
								value_namespace->AddFunction("<", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x < y; }));
								value_namespace->AddFunction(">", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x > y; }));
								value_namespace->AddFunction("<=", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x <= y; }));
								value_namespace->AddFunction(">=", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x >= y; }));
								value_namespace->AddFunction("+", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x + y; }));
								value_namespace->AddFunction("-", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x - y; }));
								value_namespace->AddFunction("*", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x * y; }));
								value_namespace->AddFunction("/", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x / y; }));
								value_namespace->AddFunction("+=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out += b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
								value_namespace->AddFunction("-=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out -= b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
								value_namespace->AddFunction("*=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out *= b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
								value_namespace->AddFunction("/=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out /= b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
							}

							if (1) {
								scripting::UnitsLibrary::UnitsLibrary::Part1(std_namespace, value_namespace);
								scripting::UnitsLibrary::UnitsLibrary::Part2(std_namespace, value_namespace);
								scripting::UnitsLibrary::UnitsLibrary::Part3(std_namespace, value_namespace);




							}

						}
					}

					// add it to our script...
					imports.emplace("github//scriptLanguage.Units", global_scope2); // the import map guarrantees lifetime...
					scope_1->AddUsing(global_scope2); // ... while "using" allows our global to share their global's custom namespaces and objects

					if (auto p = scope_1->FindFunctions("Units::value")) {
						EXPECT_EQ(true, true);
					}
					else {
						EXPECT_EQ(false, true);
					}

					std::vector<Any> params = { 100 };
					if (auto p = scope_1->BuildFunction("Units::value", params)) {
						EXPECT_EQ(true, true);
					}
					else {
						EXPECT_EQ(false, true);
					}
					if (auto p = scope_1->CallFunction("Units::value", params)) {
						EXPECT_EQ(user_type<Units::value>(), p.Type());
					}
					else {
						EXPECT_EQ(false, true);
					}

					std::vector<Any> params2 = { Units::value(100), 100 };
					if (auto p = scope_1->BuildFunction("==", params2)) {
						EXPECT_EQ(true, true);
					}
					else {
						EXPECT_EQ(false, true);
					}
					if (auto p = scope_1->CallFunction("==", params2)) {
						EXPECT_EQ(user_type<bool>(), p.Type());
					}
					else {
						EXPECT_EQ(false, true);
					}

				}

				// Basic conversions
				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<int, double>()));
				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<double, int>()));
				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<double, Units::value>()));
				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<int, Units::value>()));


				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<double, Units::foot>()));
				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<int, Units::foot>()));

				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<Units::foot, double>()));
				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<Units::foot, int>()));

				std::shared_ptr<Type_Converter_Tree> tempTree = std::make_shared<Type_Converter_Tree>();
				scope_1->CreateTypeConverterTree(tempTree, true);
				// EXPECT_EQ(true, (tempTree->Converts<double, Units::foot>()));



				std::vector<scripting::Type_Info> tempResult;
				EXPECT_EQ(true, (tempTree->TryCreateConversionPath2(user_type<int>(), user_type<double>(), tempResult)));
				EXPECT_EQ(true, (tempTree->TryCreateConversionPath2(user_type<double>(), user_type<int>(), tempResult)));
				EXPECT_EQ(true, (tempTree->TryCreateConversionPath2(user_type<int>(), user_type<Units::value>(), tempResult)));
				EXPECT_EQ(true, (tempTree->TryCreateConversionPath2(user_type<Units::value>(), user_type<Units::foot>(), tempResult)));
				EXPECT_EQ(true, (tempTree->TryCreateConversionPath2(user_type<double>(), user_type<Units::foot>(), tempResult)));
				EXPECT_EQ(true, (tempTree->TryCreateConversionPath2(user_type<int>(), user_type<Units::foot>(), tempResult)));
				EXPECT_EQ(false, (tempTree->TryCreateConversionPath2(user_type<std::string>(), user_type<int>(), tempResult)));

				





				// slowest
				if (1) {
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							EXPECT_EQ(true, scope_inner->Cast<bool>(scope_inner->CallFunction("==", { scope_inner->CallFunction("Units::value", { 100.0f }), 100l })));
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// OPTIMIZATION IDEA: when using for-loops with Using statements: move those Using statements to the temporary parent scope (scope_outer) of the for-loop to reduce the number of cache misses of the type conversion tree
				// fastest
				if (1) {
					// This one "uses" the Units namespace, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							EXPECT_EQ(true, scope_inner->Cast<bool>(scope_inner->CallFunction("==", { scope_inner->CallFunction("Units::value", { 100.0f }), 100l })));
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// for some reason, slightly slower!
				if (1) {
					// This one "uses" the Units namespace, but doesn't use the Units:: qualifier, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							EXPECT_EQ(true, scope_inner->Cast<bool>(scope_inner->CallFunction("==", { scope_inner->CallFunction("value", { 100.0f }), 100l })));
						}
					});
					// printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Using Units;
				// for (int i = 0; i < numIterations; i++){
				//     true == (Units::value(i) == i);
				// }
				if (1) {
					// This one "uses" the Units namespace, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							auto i_obj = scope_inner->FindObj("i");
							auto i_value = scope_inner->CallFunction("Units::value", { i_obj });
							EXPECT_EQ(true, scope_inner->Cast<bool>(scope_inner->CallFunction("==", { i_value, i_obj })));
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Using Units;
				// for (int i = 0; i < numIterations; i++){
				//     true == (Units::value(string::npos) == string::npos); 
				// }
				if (1) {
					// This one "uses" the Units namespace, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr
							auto j_obj = scope_inner->FindObj("string::npos"); // searching and failing to find does not throw, but returns an empty ptr
							EXPECT_EQ(true, scope_inner->Cast<bool>(scope_inner->CallFunction("!=", { i_obj, j_obj })));
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Using Units;
				// for (int i = 0; i < numIterations; i++){
				//	   true == ("ft" == Units::foot(i).abbreviation());
				// }
				if (1) {
					// Requires up-casting Units::foot to Units::value before calling 'abbreviation' and getting the result from the polymorphic type. 
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr
							auto i_ft = scope_inner->CallFunction("Units::foot", { i_obj });
							auto ft_abbrev = scope_inner->CallFunction("abbreviation", { i_ft });
							EXPECT_EQ(scope_inner->Cast<std::string>(ft_abbrev), "ft");
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Using Units;
				// for (int i = 0; i < numIterations; i++){
				//	   true == ("foot" == Units::foot(i).name());
				// }
				if (1) {
					// This one "uses" the Units namespace, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr
							auto i_ft = scope_inner->CallFunction("Units::foot", { i_obj });
							auto ft_abbrev = scope_inner->CallFunction("name", { i_ft });
							EXPECT_EQ(scope_inner->Cast<std::string>(ft_abbrev), "foot");
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Using Units;
				// for (int i = 0; i < numIterations; i++){
				//     true == ("${i} ft" == Units::foot(i).value().to_string());
				// }
				if (1) {
					// This one "uses" the Units namespace, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr
							auto i_ft = scope_inner->CallFunction("value", { scope_inner->CallFunction("Units::foot", { i_obj }) });
							auto ft_str = scope_inner->CallFunction("to_string", { i_ft });
							EXPECT_EQ(scope_inner->Cast<std::string>(ft_str), Units::printf("%i ft", i));
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Using Units;
				// for (int i = 0; i < numIterations; i++){
				//     true == (Units::foot(i) == Units::meter(Units::foot(i)));
				// }
				if (1) {
					// This one "uses" the Units namespace, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							auto i_obj = scope_inner->FindObj("i");
							EXPECT_EQ(true, scope_inner->Cast<bool>(scope_inner->CallFunction("==", { scope_inner->CallFunction("Units::foot", { i_obj }), scope_inner->CallFunction("Units::meter", { scope_inner->CallFunction("Units::foot", { i_obj }) }) })));
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Using Units;
				// for (int i = 0; i < numIterations; i++){
				//     (Units::foot(i) * Units::foot(i)).to_string == "${ i * i } sq_ft"
				// }
				if (1) {
					// This one "uses" the Units namespace, to see if it provides a speed boost at all.
					sw.Start();
					auto scope_outer = std::make_shared<Scope2>(scope_1);
					scope_outer->SetSelf(scope_outer);
					scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope2>(scope_outer);
						scope_inner->SetSelf(scope_inner);
						scope_inner->AddObj("i", std::make_shared<Any>(i));
						{
							auto i_obj = scope_inner->FindObj("i");
							auto pt1 = scope_inner->CallFunction("Units::foot", { i_obj });
							auto pt2 = scope_inner->CallFunction("Units::foot", { i_obj });

							auto multiplied = scope_inner->CallFunction("*", { pt1, pt2 });
							auto stringified = scope_inner->CallFunction("to_string", { multiplied });
						}
					});
					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}
			}



			// DateTime
			if (1) {






			}





#endif


#if 0
			EXPECT_EQ(true, scope_1->AddFunction("length", make_callable([]() -> size_t { return 0; }), true));
			EXPECT_EQ(true, scope_1->AddFunction("size", make_callable([]() -> size_t { return 0; }), true));

			{
				auto scope_t = std::make_shared<Namespace2>(scope_1, "fibers"); // ::fibers::
				scope_t->SetSelf(scope_t);
				scope_1->AddChild(scope_t);

				{
					auto scope_t2 = std::make_shared<Namespace2>(scope_t, "containers"); // ::fibers::containers::
					scope_t2->SetSelf(scope_t2);
					scope_t->AddChild(scope_t2);

					{
						auto scope_t3 = std::make_shared<Class2>(scope_t2, "Map", user_type<fibers::containers::Map<std::string, fibers::Any>>()); // ::fibers::containers::Map::
						scope_t3->SetSelf(scope_t3);
						scope_t2->AddChild(scope_t3);
					}
				}
			}
			auto scope_2 = std::make_shared<Namespace2>(scope_1, "std"); // ::std::
			scope_2->SetSelf(scope_2);
			scope_1->AddChild(scope_2);

			EXPECT_EQ(true, scope_2->AddFunction("length", make_callable([](int x) -> size_t { return sizeof(int); }), true));
			EXPECT_EQ(true, scope_2->AddFunction("size", make_callable([](int x) -> size_t { return sizeof(int); }), true));

			EXPECT_EQ(true, scope_2->AddFunction("length", make_callable([](double x) -> size_t { return sizeof(double); }), true));
			EXPECT_EQ(true, scope_2->AddFunction("size", make_callable([](double x) -> size_t { return sizeof(double); }), true));

			auto scope_3 = std::make_shared<Class2>(scope_2, "string", user_type<std::string>()); // ::std::string::
			scope_3->SetSelf(scope_3);
			scope_2->AddChild(scope_3);
			EXPECT_EQ(scope_3->AddObj("npos", std::make_shared<fibers::Any>(std::string::npos), true), true);
			
			auto scope_4 = std::make_shared<Scope2>(scope_3); // ::std::string::
			scope_4->SetSelf(scope_4);

			auto scope_5 = std::make_shared<Class2>(scope_4, "string"); // ::std::string::string::
			scope_5->SetSelf(scope_5);
			scope_4->AddChild(scope_5);




			if (0) {
				// support the default types
				if (1) {
					auto lambda = [&](auto type, std::string className) {
						using TypeTypename = decltype(type);

						auto defaultTypeClass = std::make_shared<Class2>(scope_1, className, user_type<TypeTypename>());
						defaultTypeClass->SetSelf(defaultTypeClass);
						scope_1->AddChild(defaultTypeClass);

						defaultTypeClass->AddFunction(className, make_callable([]() -> TypeTypename { return TypeTypename{}; }));
						defaultTypeClass->AddFunction(className, make_callable([](TypeTypename const& makeCopy) -> TypeTypename { return makeCopy; }));
						defaultTypeClass->AddFunction("=", make_callable([](Any const& a, TypeTypename const& b) -> Any { a.cast<TypeTypename&>() = b; return a; }), Param_Types({ {"a", user_type<TypeTypename>() }, {"b", user_type<TypeTypename>() } }));
						defaultTypeClass->AddFunction("==", make_callable([](TypeTypename const& a, TypeTypename const& b) -> bool { return a == b; }));
						defaultTypeClass->AddFunction("!=", make_callable([](TypeTypename const& a, TypeTypename const& b) -> bool { return a != b; }));
						defaultTypeClass->AddFunction(">", make_callable([](TypeTypename const& a, TypeTypename const& b) -> bool { return a > b; }));
						defaultTypeClass->AddFunction(">=", make_callable([](TypeTypename const& a, TypeTypename const& b) -> bool { return a >= b; }));
						defaultTypeClass->AddFunction("<", make_callable([](TypeTypename const& a, TypeTypename const& b) -> bool { return a < b; }));
						defaultTypeClass->AddFunction("<=", make_callable([](TypeTypename const& a, TypeTypename const& b) -> bool { return a <= b; }));
						defaultTypeClass->AddFunction("+", make_callable([](TypeTypename const& a, TypeTypename const& b) -> TypeTypename { return a + b; }));
						defaultTypeClass->AddFunction("-", make_callable([](TypeTypename const& a, TypeTypename const& b) -> TypeTypename { return a - b; }));
						defaultTypeClass->AddFunction("*", make_callable([](TypeTypename const& a, TypeTypename const& b) -> TypeTypename { return a * b; }));
						defaultTypeClass->AddFunction("/", make_callable([](TypeTypename const& a, TypeTypename const& b) -> TypeTypename { return a / b; }));
						defaultTypeClass->AddFunction("+=", make_callable([](Any const& a, TypeTypename const& b) -> Any { a.cast<TypeTypename&>() += b; return a; }), Param_Types({ {"a", user_type<TypeTypename>() }, {"b", user_type<TypeTypename>() } }));
						defaultTypeClass->AddFunction("-=", make_callable([](Any const& a, TypeTypename const& b) -> Any { a.cast<TypeTypename&>() -= b; return a; }), Param_Types({ {"a", user_type<TypeTypename>() }, {"b", user_type<TypeTypename>() } }));
						defaultTypeClass->AddFunction("*=", make_callable([](Any const& a, TypeTypename const& b) -> Any { a.cast<TypeTypename&>() *= b; return a; }), Param_Types({ {"a", user_type<TypeTypename>() }, {"b", user_type<TypeTypename>() } }));
						defaultTypeClass->AddFunction("/=", make_callable([](Any const& a, TypeTypename const& b) -> Any { a.cast<TypeTypename&>() /= b; return a; }), Param_Types({ {"a", user_type<TypeTypename>() }, {"b", user_type<TypeTypename>() } }));
					};

#define MakeClasses(className) lambda(##className(), #className)
					MakeClasses(bool);
					MakeClasses(char);
					MakeClasses(int);
					MakeClasses(long);
					MakeClasses(size_t);
					MakeClasses(float);
					MakeClasses(double);
					lambda(fibers::containers::number<double>(), "number");
#undef MakeClasses
				}
				if (1) {
					auto lambda_oneWay = [&](auto FromType, auto ToType) {
						using FromTypename = decltype(FromType);
						using ToTypename = decltype(ToType);

						if (auto ToClass = scope_1->FindClass(user_type<ToTypename>())) {
							ToClass->AddFunction(ToClass->GetName(), make_callable([](FromTypename const& from) -> ToTypename { return (ToTypename)from; }));
						}
						else {
							EXPECT_EQ(true, false);
						}
					};
					auto lambda = [&](auto FromType, auto ToType) {
						lambda_oneWay(FromType, ToType);
						lambda_oneWay(ToType, FromType);
					};

#define MakeConverter(from, to) lambda(from(), to())

#define MakeConverters(from) lambda(from(), bool()); \
                             lambda(from(), char()); \
                             lambda(from(), int()); \
                             lambda(from(), long()); \
                             lambda(from(), size_t()); \
                             lambda(from(), float()); \
                             lambda(from(), double())

					//lambda(char(), bool());
					//lambda(char(), int());
					//lambda(long(), int());
					//lambda(long(), size_t());
					//lambda(float(), size_t());
					//lambda(float(), double());
					//lambda(fibers::containers::number<double>(), double());

					MakeConverters(bool);
					MakeConverters(char);
					MakeConverters(int);
					MakeConverters(long);
					MakeConverters(size_t);
					MakeConverters(float);
					MakeConverters(double);
					MakeConverters(fibers::containers::number<double>);
#undef MakeConverters
#undef MakeConverter
				}
				if (1) {
					// constructors
					scope_3->AddFunction("string", make_callable([]() -> std::string { return std::string{}; }));
					scope_3->AddFunction("string", make_callable([](std::string const& makeCopy) -> std::string { return makeCopy; }));
					scope_3->AddFunction("string", make_callable([](bool from) -> std::string { if (from) return "true"; else return "false"; }));
					scope_3->AddFunction("string", make_callable([](int from) -> std::string { return std::to_string(from); }));
					scope_3->AddFunction("string", make_callable([](long from) -> std::string { return std::to_string(from); }));
					scope_3->AddFunction("string", make_callable([](size_t from) -> std::string { return std::to_string(from); }));
					scope_3->AddFunction("string", make_callable([](float from) -> std::string { return std::to_string(from); }));
					scope_3->AddFunction("string", make_callable([](double from) -> std::string { return std::to_string(from); }));
					scope_3->AddFunction("string", make_callable([](fibers::containers::number<double> from) -> std::string { return std::to_string(from); }));

					// operators
					scope_3->AddFunction("=", make_callable([](Any const& a, std::string const& b) -> Any { a.cast<std::string&>() = b; return a; }), Param_Types({ {"a", user_type<std::string>() }, {"b", user_type<std::string>() } }));
					scope_3->AddFunction("==", make_callable([](std::string const& a, std::string const& b) -> bool { return a == b; }));
					scope_3->AddFunction("!=", make_callable([](std::string const& a, std::string const& b) -> bool { return a != b; }));
					scope_3->AddFunction(">", make_callable([](std::string const& a, std::string const& b) -> bool { return a > b; }));
					scope_3->AddFunction(">=", make_callable([](std::string const& a, std::string const& b) -> bool { return a >= b; }));
					scope_3->AddFunction("<", make_callable([](std::string const& a, std::string const& b) -> bool { return a < b; }));
					scope_3->AddFunction("<=", make_callable([](std::string const& a, std::string const& b) -> bool { return a <= b; }));
					scope_3->AddFunction("+", make_callable([](std::string const& a, std::string const& b) -> std::string { return a + b; }));
					scope_3->AddFunction("+=", make_callable([](Any const& a, std::string const& b) -> Any { a.cast<std::string&>() += b; return a; }), Param_Types({ {"a", user_type<std::string>() }, {"b", user_type<std::string>() } }));

					// functions
					scope_3->AddFunction("length", make_callable([](std::string const& a) -> size_t { return a.length(); }));
					scope_3->AddFunction("size", make_callable([](std::string const& a) -> size_t { return a.size(); }));
					scope_3->AddFunction("[]", make_callable([](std::string const& a, size_t index) -> char { return a[index]; }));
					scope_3->AddFunction("front", make_callable([](std::string const& a) -> char { return a.front(); }));
					scope_3->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind) -> size_t { return a.find(toFind); }));
					scope_3->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind, size_t startPos) -> size_t { return a.find(toFind, startPos); }));

				}


			}





			// Demonstrate multiple classes with the same name being created, added to, and found from the same Scope
			if (1) {
				fibers::containers::Map<std::shared_ptr<Scope2>, std::shared_ptr<Scope2>, Scope2::Hasher> set;
				fibers::containers::number<int> counter{ 0 };
				fibers::containers::number<int> counter_failed{ 0 };
				fibers::parallel::For(0, 10000, [&](int i) {
					
					auto scope_6 = std::make_shared<Scope2>(scope_4); // ::std::string::
					scope_6->SetSelf(scope_6);
					{
						{
							auto scope_7 = std::make_shared<Class2>(scope_6, "Position"); // ::std::string::Position::
							scope_7->SetSelf(scope_7);
							scope_6->AddChild(scope_7);
						}

						// may (and is allowed to) discover any of the "Position" classes that are available in this Scope
						if (auto p = std::dynamic_pointer_cast<Scope2>(scope_6->FindNamespace("Position"))) {
							EXPECT_NE(p->GetQualifiedNamespace(true).find(scope_6->GetQualifiedNamespace(true)), std::string::npos);

							if (set.emplace(p, p, false)) {
								counter++;
							}
							else {
								counter_failed++;
							}
						}
						else {
							EXPECT_EQ(true, false);
						}
					}
				});
				EXPECT_EQ(counter.load(), set.size());
				EXPECT_EQ(counter_failed.load(), 0);
			}

			// Demonstrate multiple classes with the same name being created and added to unique - but connected - Namespaces, and discovered from those unique Namespaces
			// Bug to prevent is accidental discovery of another Namespace's class when an better-fit class is located more closely. 
			if (1) {
				auto scope_6 = std::make_shared<Namespace2>(scope_4, "For"); // Loop Start...
				scope_6->SetSelf(scope_6);
				scope_4->AddChild(scope_6);

				
				fibers::parallel::For(0, 1000, [&](int i) {
					auto scope_7 = std::make_shared<Namespace2>(scope_6, "Each"); // ForLoop Scope ...
					scope_7->SetSelf(scope_7);
					scope_6->AddChild(scope_7);

					{
						auto scope_8 = std::make_shared<Class2>(scope_7, "Position"); // ::std::string::string::
						scope_8->SetSelf(scope_8);
						scope_7->AddChild(scope_8);
					}

					if (auto p = scope_7->FindNearestNamespaceWhere([](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
						std::string tryFind = "Position";
						long long len = tryFind.length();
						auto qualifiedName = namespacePtr->GetQualifiedNamespace();

						// remove "::" from end
						while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

						long long qualifiedNameLen = qualifiedName.length();
						auto F = qualifiedName.find(tryFind);
						if (F != std::string::npos) {
							if (F == (qualifiedNameLen - len)) {
								return true;
							}
						}
						return false;
					})) {
						EXPECT_NE(p->GetQualifiedNamespace(true).find(scope_7->GetQualifiedNamespace(true)), std::string::npos);
					}
					else {
						EXPECT_EQ(true, false);
					}
				});
			}

			int numIterations = 100000;

			// this scope has a child AND parent, named "string". Expect return of parent. 
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNearestNamespaceWhere([](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					return namespacePtr->GetName() == "string";
					})) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNamespace("string")) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
				});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Expect quick discovery of parent. 
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNearestNamespaceWhere([](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					return namespacePtr->GetName() == "std";
				})) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNamespace("std")) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::");
				}
				else {
					EXPECT_EQ(true, false);
				}
				});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Qualified name requires a bit more work to find.
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNearestNamespaceWhere([](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					std::string tryFind = "std::string";
					long long len = tryFind.length();
					auto qualifiedName = namespacePtr->GetQualifiedNamespace();

					// remove "::" from end
					while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);
					if (F != std::string::npos) {
						if (F == (qualifiedNameLen - len)) {
							return true;
						}
					}
					return false;
				})) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNamespace("std::string")) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Qualified approach should work for unqualified search name. 
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNearestNamespaceWhere([](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					std::string tryFind = "string";
					long long len = tryFind.length();
					auto qualifiedName = namespacePtr->GetQualifiedNamespace();

					// remove "::" from end
					while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);
					if (F != std::string::npos) {
						if (F == (qualifiedNameLen - len)) {
							return true;
						}
					}
					return false;
					})) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNamespace("string")) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
				});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Expect scope to eventually try and check its own children.
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNearestNamespaceWhere([&](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					std::string tryFind = "std::string::string";
					long long len = tryFind.length();
					auto qualifiedName = namespacePtr->GetQualifiedNamespace();
				
					// remove "::" from end
					while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);
					if (F != std::string::npos) {
						if (F == (qualifiedNameLen - len)) {
							return true;
						}
					}
					return false;
				})) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNamespace("std::string::string")) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Expect scope to eventually try and check children from the parents, and (eventually) discover this far-away qualified namespace
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNearestNamespaceWhere([&](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					std::string tryFind = "fibers::containers::Map";
					long long len = tryFind.length();
					auto qualifiedName = namespacePtr->GetQualifiedNamespace();

					// remove "::" from end
					while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);
					if (F != std::string::npos) {
						if (F == (qualifiedNameLen - len)) {
							return true;
						}
					}
					return false;
				})) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::fibers::containers::Map::");
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindNamespace("fibers::containers::Map")) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::fibers::containers::Map::");
				}
				else {
					EXPECT_EQ(true, false);
				}
				});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Find Classes using TypeInfo tags
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindClass(user_type<std::string>())) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::std::string::");
				}
				else {
					EXPECT_EQ(true, false);
				}
				});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindClass(user_type<fibers::containers::Map<std::string, fibers::Any>>())) {
					EXPECT_EQ(p->GetQualifiedNamespace(), "::fibers::containers::Map::");
				}
				else {
					EXPECT_EQ(true, false);
				}
				});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Find Objects
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindObj("npos")) {
					EXPECT_EQ(p->cast<size_t>(), std::string::npos);
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindObj("::std::string::npos")) {
					EXPECT_EQ(p->cast<size_t>(), std::string::npos);
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindObj("::string::npos")) {
					EXPECT_EQ(p->cast<size_t>(), std::string::npos);
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Find Functions
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindFunctions("length")) {
					//EXPECT_EQ(p->cast<size_t>(), std::string::npos);
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindFunctions("::std::string::length")) {
					//EXPECT_EQ(p->cast<size_t>(), std::string::npos);
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			
			sw.Start();
			fibers::parallel::For(0, numIterations, [&](int i) {
				if (auto p = scope_4->FindFunctions("::string::length")) {
					//EXPECT_EQ(p->cast<size_t>(), std::string::npos);
				}
				else {
					EXPECT_EQ(true, false);
				}
			});
			printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");

			// Find and Call Functions
			if (1) {
				std::vector<Any> params = { Any(std::string("TESTING")) };
				
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto p = scope_4->FindFunction("length", scripting::Function_Params(params))) {
						EXPECT_EQ(true, p->first->call_match(params, {}));
						EXPECT_EQ(scripting::call(p->first, params, {}).cast<size_t>(), 7);
					}
					else {
						EXPECT_EQ(true, false);
					}
				});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
				
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto p = scope_4->FindFunction("::std::string::length", scripting::Function_Params(params))) {
						EXPECT_EQ(true, p->first->call_match(params, {}));
						EXPECT_EQ(scripting::call(p->first, params, {}).cast<size_t>(), 7);
					}
					else {
						EXPECT_EQ(true, false);
					}
				});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
				
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i) {
					if (auto p = scope_4->FindFunction("::string::length", scripting::Function_Params(params))) {
						EXPECT_EQ(true, p->first->call_match(params, {}));
						EXPECT_EQ(scripting::call(p->first, params, {}).cast<size_t>(), 7);
					}
					else {
						EXPECT_EQ(true, false);
					}
				});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations per " + Units::second(sw.Stop_s()).ToString() + ".");
			}
			// Get All Available Classes
			if (1) {
				sw.Start();
				fibers::parallel::For(0, numIterations, [&](int i)
				{
					auto classes = scope_4->GetLibrary()->GetAllAvailableClasses();
					EXPECT_EQ(true, classes->size() >= 10);
				}
				);
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
			}

			// Create and test the type conversion tree, which builds itself from the constructors of the various classes.
			if (1) {
				sw.Start();
				fibers::parallel::For(0, numIterations / 10, [&](int i)
					{
						auto tree = scope_4->GetTypeConverterTree(); // builds and caches the tree. Updates the tree only if the situation has changed (new functions, new classes, or new Using statements)

						if (!tree->Converts<int, long>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<float, long>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<int, double>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<bool, int>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<int, int>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<double, int>()) { EXPECT_EQ(true, false); }
						if (!tree->Converts<bool, double>()) { EXPECT_EQ(true, false); }

						EXPECT_EQ(100, tree->Convert<int>(100.0));
						EXPECT_EQ(100.0, tree->Convert<double>(100l));
						EXPECT_EQ(100l, tree->Convert<long>(100.0f));
						EXPECT_EQ(100.0f, tree->Convert<float>(100.0));
						EXPECT_EQ(true, tree->Convert<bool>(1));
						EXPECT_EQ(100.0f, tree->Convert<float>(100l));
						EXPECT_EQ(1.0, tree->Convert<double>(true));
					}
				);
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations / 10) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
			}

			// Create and test the type conversion tree, which builds itself from the constructors of the various classes.
			if (1) {

				std::vector<Any> params = { Any(fibers::containers::number<double>(0)), Any(1) };
				auto& n = params[0].cast<fibers::containers::number<double>&>();

				sw.Start();
				auto scope_outer = std::make_shared<Scope2>(scope_4);
				scope_outer->SetSelf(scope_outer);
				fibers::parallel::For(0, numIterations, [&](int i) {
					auto scope_inner = std::make_shared<Scope2>(scope_outer);
					scope_inner->SetSelf(scope_inner);
					scope_inner->AddObj("i", std::make_shared<Any>(i));
					{

						scope_inner->CallFunction("+=", params);

					}
				});
				printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");

				{
					EXPECT_EQ(true, scope_4->Cast<bool>(scope_4->CallFunction("==", { params[0], numIterations })));
				}

				{
					printf(scope_4->Cast<std::string>(
						scope_4->CallFunction("+", { 
							scope_4->Cast<std::string>(params[0])
							, std::string(" is the count") 
						})
					));
				}

				EXPECT_EQ(n, (numIterations));
			}

			EXPECT_EQ("100", scope_4->Cast<std::string>(scope_4->CallFunction("std::string", { 100 })));
			EXPECT_EQ("200", scope_4->Cast<std::string>(scope_1->CallFunction("::std::string", { scope_4->Cast<int>(scope_1->CallFunction("+", { 100.0f, 100.0 })) })));








			EXPECT_EQ(scope_1, scope_4->GetLibrary());

			scope_4->AddUsing(scope_1);
			scope_4->AddUsing(scope_4->GetLibrary());

			EXPECT_EQ(scope_4->p_using.size(), 1);

			scope_4->AddUsing(scope_3);
			scope_4->AddUsing(scope_4->GetNamespace());
			EXPECT_EQ(scope_4->p_using.size(), 2);


#endif
		}
		catch (std::exception& e) {
			printf(e.what());
		}


#if 0

		// MODERN TEST 2
		if (1) {
			using namespace scripting;
			auto printf = [](auto x) { std::cout << x << std::endl; };

			// Import Collection (to be owned and maintained by engine instance)
			fibers::containers::Map<std::string, std::shared_ptr<Namespace>> imports;

			// Global Namespace...
			auto global_scope{ std::make_shared<Global>() };
			global_scope->p_self = global_scope;
			global_scope->AddBuiltIns();

			//// Built-In Functions, Type, Conversions
			//if (1) {
			//	// template function (specified by the "Any" type) which be duplicated and instantiated whenever actually called by a "real" set of parameters
			//	global_scope->m_functions.emplace("Type", scripting::Param_Types({ { std::string("obj"), scripting::user_type<Any>() } }), scripting::make_callable(
			//		[](Any const& x) -> fibers::Type_Info {
			//			if (auto p = x.Type().lock())
			//				return *p;
			//			else
			//				return fibers::user_type<void>();
			//		}
			//	), true);
			//	global_scope->m_functions.emplace("+", scripting::make_callable([](double a, double b) -> double {
			//		return a + b;
			//	}), true);
			//	global_scope->m_functions.emplace("*", scripting::make_callable([](double a, double b) -> double {
			//		return a * b;
			//	}), true);
			//	global_scope->m_functions.emplace("-", scripting::make_callable([](double a, double b) -> double {
			//		return a - b;
			//	}), true);
			//	global_scope->m_functions.emplace("/", scripting::make_callable([](double a, double b) -> double {
			//		return a / b;
			//	}), true);
			//	global_scope->m_functions.emplace("^", scripting::make_callable([](double a, double b) -> double {
			//		return std::pow(a, b);
			//	}), true);
			//	global_scope->CallFunction("Type", { Any(100.0) });
			//	global_scope->CallFunction("+", { Any(100.0), Any(100.0) });
			//	global_scope->CallFunction("^", { Any(5.0), Any(2.0) });
			//}

			// Import Namespaces...
			if (1) {
				// #include "std"
				if (1) {
					auto global_scope2{ std::make_shared<Global>() }; 
					global_scope2->p_self = global_scope2;

					global_scope2->AddBuiltIns();

					// Create STD library...
					{
						auto std_namespace{ std::make_shared<Namespace>(global_scope2, "std") };
						std_namespace->p_self = std_namespace;
						global_scope2->AddChild(std_namespace);

						// the "std" namespace imports the "map" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::map<std::string, fibers::Any>>(), std_namespace, "map") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "std" namespace imports the "set" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::set<std::string>>(), std_namespace, "set") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "std" namespace imports the "vector" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::vector<fibers::Any>>(), std_namespace, "vector") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}
					}

					// add it to our script...
					imports.emplace("github//scriptLanguage.std", global_scope2); // the import map guarrantees lifetime...
					global_scope->AddUsing(global_scope2); // ... while "using" allows our global to share their global's custom namespaces and objects
				}

				// #include "fibers"
				if (1) {
					auto script_scope{ std::make_shared<Global>() }; // global should always be a Namespace
					script_scope->p_self = script_scope;

					script_scope->AddBuiltIns();

					{
						auto std_namespace{ std::make_shared<Namespace>(script_scope, "fibers") };
						std_namespace->p_self = std_namespace;
						script_scope->AddChild(std_namespace);

						// the "fibers" namespace imports the "Number" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<fibers::containers::number<double>>(), std_namespace, "Number") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...								
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							string_namespace->TypeConversionTree()->AddConverter<fibers::containers::number<double>, unsigned int>();
							string_namespace->TypeConversionTree()->AddConverter<fibers::containers::number<double>, int>();
							string_namespace->TypeConversionTree()->AddConverter<fibers::containers::number<double>, float>();
							string_namespace->TypeConversionTree()->AddConverter<fibers::containers::number<double>, double>();
							string_namespace->TypeConversionTree()->AddConverter<fibers::containers::number<double>, size_t>();
						}

						// the "fibers" namespace imports the "Pattern" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<fibers::containers::Pattern<double, double>>(), std_namespace, "Pattern") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "fibers" namespace imports the "Matrix" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(std_namespace, "Matrix") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "fibers" namespace imports the "UI" namespace...
						{
							auto string_namespace{ std::make_shared<Namespace>(std_namespace, "UI") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "Interactive" namespace...
							auto interactive_namespace{ std::make_shared<Class>(string_namespace, "Interactive") }; {
								interactive_namespace->p_self = interactive_namespace;
								string_namespace->AddChild(interactive_namespace);

								{
									auto impl_namespace{ std::make_shared<Namespace>(interactive_namespace, "InteractiveImpl") };
									impl_namespace->p_self = impl_namespace;
									interactive_namespace->AddChild(impl_namespace);
								}
							}

							// ... which imports the "Map" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Map", interactive_namespace) };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);

								{
									auto impl_namespace2{ std::make_shared<Namespace>(impl_namespace, "MapImpl") };
									impl_namespace2->p_self = impl_namespace2;
									impl_namespace->AddChild(impl_namespace2);
								}

								{
									auto impl_namespace2{ std::make_shared<Namespace>(impl_namespace, "Text") };
									impl_namespace2->p_self = impl_namespace2;
									impl_namespace->AddChild(impl_namespace2);
								}

								impl_namespace->m_functions.emplace("=", make_callable([](Any& lhs, Any const& rhs) -> Any {
									fibers::DynamicObject& LHS = lhs.cast();
									fibers::DynamicObject& RHS = rhs.cast();
									LHS = RHS;
									return Any(lhs);
									}), scripting::Param_Types({ { "lhs", impl_namespace->ClassType }, { "rhs", impl_namespace->ClassType } }));
							}

							// ... which imports the "Button" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Button", interactive_namespace) };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "Grid" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Grid") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "Plot" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Plot") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "Text" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Text") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "WebPage" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "WebPage") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

						}
					}

					// add it to our script...
					imports.emplace("github//scriptLanguage.fibers", script_scope); // the import map guarrantees lifetime...
					global_scope->AddUsing(script_scope); // ... while "using" allows our global to share their global's custom namespaces and objects
				}

				// #include "Units"
				if (1) {
					auto global_scope2{ std::make_shared<Global>() }; // global should always be a Namespace
					global_scope2->p_self = global_scope2;
					auto& tree = *global_scope2->TypeConversionTree();

					global_scope2->AddBuiltIns();

					// Create library...
					{
						auto std_namespace{ std::make_shared<Namespace>(global_scope2, "Units") };
						std_namespace->p_self = std_namespace;
						global_scope2->AddChild(std_namespace);

						// the "std" namespace imports the "string" namespace...
						{
							auto value_namespace{ std::make_shared<Class>(fibers::user_type<Units::value>(), std_namespace, "value") };
							value_namespace->p_self = value_namespace;
							std_namespace->AddChild(value_namespace);

							// which has the following types groups... 

							{
								tree.AddConverter<Units::value, double>();
								tree.AddConverter([](Units::value const& x) -> std::string { return x.ToString(); });
								value_namespace->m_functions.emplace("abbreviation", make_callable([](Units::value const& x)->std::string {
									return x.Abbreviation();
								}));
								value_namespace->m_functions.emplace("name", make_callable([](Units::value const& x)->std::string {
									return x.UnitName();
								}));
								value_namespace->m_functions.emplace("to_string", make_callable([](Units::value const& x)->std::string {
									return x.ToString();
								}));

								value_namespace->m_functions.emplace("*", make_callable([](Units::value const& x, Units::value const& y)->Units::value{
									return x * y;
								}));
								value_namespace->m_functions.emplace("+", make_callable([](Units::value const& x, Units::value const& y)->Units::value {
									return x + y;
								}));
								value_namespace->m_functions.emplace("-", make_callable([](Units::value const& x, Units::value const& y)->Units::value {
									return x - y;
								}));
								value_namespace->m_functions.emplace("/", make_callable([](Units::value const& x, Units::value const& y)->Units::value {
									return x / y;
								}));
								value_namespace->m_functions.emplace("+=", make_callable([](Units::value& x, Units::value const& y) -> void {
									x += y;
								}));
								value_namespace->m_functions.emplace("*=", make_callable([](Units::value& x, Units::value const& y) -> void {
									x *= y;
								}));
								value_namespace->m_functions.emplace("/=", make_callable([](Units::value& x, Units::value const& y) -> void {
									x /= y;
								}));
								value_namespace->m_functions.emplace("-=", make_callable([](Units::value& x, Units::value const& y) -> void {
									x -= y;
								}));

								// manually go through and add each unit type... 
								auto allUnitTypes = Units::UnitsDetail::GetValueTypes();
								auto addUnit = [&](auto impl, std::string const& name) -> void {
									auto impl_namespace{ std::make_shared<Class>(std_namespace, name, value_namespace, user_type<decltype(impl)>()) };
									impl_namespace->p_self = impl_namespace;
									std_namespace->AddChild(impl_namespace);

									// Polymorphic converter
									tree.AddConverter<decltype(impl), Units::value>();
									tree.AddConverter<Units::value, decltype(impl)>();
									tree.AddConverter<decltype(impl), double>();
									tree.AddConverter([](decltype(impl) const& x) -> std::string { return x.ToString(); });

									bool found{ false };
									for (auto& type_group : allUnitTypes) {
										if (found) break;
										for (auto& type : type_group) {
											if (std::get<1>(type) == name) {
												auto& abbrev = std::get<0>(type);

												// POSTFIX
												std_namespace->m_postfixes.emplace(abbrev, impl_namespace);

												// CONSTRUCT {}
												impl_namespace->m_functions.emplace(name, make_callable([/*thisT = impl*/]()->decltype(impl) {
													decltype(impl) out;
													out = 0;
													return out;
												}), scripting::Param_Types());

												// CONSTRUCT { double }
												impl_namespace->m_functions.emplace(name, make_callable([/*thisT = impl*/](double x)->decltype(impl) {
													decltype(impl) out;
													out = x;
													return out;
												}), scripting::Param_Types({ { "in", user_type<double>() } }));

												// CONSTRUCT { Units::value }
												impl_namespace->m_functions.emplace(name, make_callable([/*thisT = impl*/](Units::value const& x)->decltype(impl) {
													decltype(impl) out;
													out = x;
													return out;
												}), scripting::Param_Types({ { "in", user_type<Units::value>() } }));

												// abbreviation
												impl_namespace->m_functions.emplace("abbreviation", make_callable([thisT = abbrev]()->std::string {
													return thisT;
												}));

												// name
												impl_namespace->m_functions.emplace("name", make_callable([thisT = name]()->std::string {
													return thisT;
												}));

												std_namespace->AddUsing(impl_namespace);

												found = true;
												break;
											}
										}
									}
								};

								

#define AddUnit(unitname) addUnit(Units::##unitname(), #unitname)
								AddUnit(meter);
								AddUnit(foot);
								AddUnit(inch);
								AddUnit(mile);
								AddUnit(nauticalMile);
								AddUnit(astronicalUnit);
								AddUnit(yard);
								AddUnit(gram);
								AddUnit(metric_ton);
								AddUnit(pound);
								AddUnit(long_ton);
								AddUnit(short_ton);
								AddUnit(stone);
								AddUnit(ounce);
								AddUnit(carat);
								AddUnit(slug);
								AddUnit(second);
								AddUnit(minute);
								AddUnit(hour);
								AddUnit(day);
								AddUnit(week);
								AddUnit(year);
								AddUnit(month);
								AddUnit(julian_year);
								AddUnit(gregorian_year);
								AddUnit(ampere);
								AddUnit(Dollar);
								AddUnit(MillionDollar);
								AddUnit(hertz);
								AddUnit(meters_per_second);
								AddUnit(feet_per_second);
								AddUnit(feet_per_minute);
								AddUnit(feet_per_hour);
								AddUnit(miles_per_hour);
								AddUnit(kilometers_per_hour);
								AddUnit(knot);
								AddUnit(meters_per_second_squared);
								AddUnit(feet_per_second_squared);
								AddUnit(standard_gravity);
								AddUnit(newton);
								AddUnit(pound_f);
								AddUnit(dyne);
								AddUnit(kilopond);
								AddUnit(poundal);
								AddUnit(pascals);
								AddUnit(bar);
								AddUnit(atmosphere);
								AddUnit(pounds_per_square_inch);
								AddUnit(head);
								AddUnit(torr);
								AddUnit(coulomb);
								AddUnit(ampere_hour);
								AddUnit(watt);
								AddUnit(horsepower);
								AddUnit(joule);
								AddUnit(calorie);
								AddUnit(watt_minute);
								AddUnit(watt_hour);
								AddUnit(watt_day);
								AddUnit(british_thermal_unit);
								AddUnit(british_thermal_unit_iso);
								AddUnit(british_thermal_unit_59);
								AddUnit(therm);
								AddUnit(foot_pound);
								AddUnit(volt);
								AddUnit(ohm);
								AddUnit(siemens);
								AddUnit(square_meter);
								AddUnit(square_foot);
								AddUnit(square_inch);
								AddUnit(square_mile);
								AddUnit(square_kilometer);
								AddUnit(hectare);
								AddUnit(acre);
								AddUnit(cubic_meter);
								AddUnit(cubic_millimeter);
								AddUnit(cubic_kilometer);
								AddUnit(liter);
								AddUnit(cubic_inch);
								AddUnit(cubic_foot);
								AddUnit(cubic_yard);
								AddUnit(cubic_mile);
								AddUnit(gallon);
								AddUnit(imperial_gallon);
								AddUnit(million_gallon);
								AddUnit(imperial_million_gallon);
								AddUnit(acre_foot);
								AddUnit(quart);
								AddUnit(pint);
								AddUnit(cup);
								AddUnit(fluid_ounce);
								AddUnit(barrel);
								AddUnit(bushel);
								AddUnit(cord);
								AddUnit(tablespoon);
								AddUnit(teaspoon);
								AddUnit(pinch);
								AddUnit(dash);
								AddUnit(drop);
								AddUnit(fifth);
								AddUnit(dram);
								AddUnit(gill);
								AddUnit(peck);
								AddUnit(sack);
								AddUnit(shot);
								AddUnit(strike);
								AddUnit(gram_per_second);
								AddUnit(metric_ton_per_second);
								AddUnit(metric_ton_per_minute);
								AddUnit(metric_ton_per_hour);
								AddUnit(metric_ton_per_day);
								AddUnit(metric_ton_per_year);
								AddUnit(cubic_meter_per_second);
								AddUnit(cubic_meter_per_hour);
								AddUnit(cubic_meter_per_day);
								AddUnit(cubic_millimeter_per_second);
								AddUnit(liter_per_second);
								AddUnit(liter_per_minute);
								AddUnit(liter_per_day);
								AddUnit(megaliter_per_day);
								AddUnit(cubic_inch_per_second);
								AddUnit(cubic_inch_per_hour);
								AddUnit(cubic_foot_per_second);
								AddUnit(cubic_foot_per_hour);
								AddUnit(gallon_per_second);
								AddUnit(gallon_per_minute);
								AddUnit(gallon_per_hour);
								AddUnit(gallon_per_day);
								AddUnit(gallon_per_year);
								AddUnit(million_gallon_per_second);
								AddUnit(million_gallon_per_minute);
								AddUnit(million_gallon_per_hour);
								AddUnit(million_gallon_per_day);
								AddUnit(million_gallon_per_year);
								AddUnit(imperial_million_gallon_per_second);
								AddUnit(imperial_million_gallon_per_minute);
								AddUnit(imperial_million_gallon_per_hour);
								AddUnit(imperial_million_gallon_per_day);
								AddUnit(imperial_million_gallon_per_year);
								AddUnit(acre_foot_per_second);
								AddUnit(acre_foot_per_minute);
								AddUnit(acre_foot_per_hour);
								AddUnit(acre_foot_per_day);
								AddUnit(acre_foot_per_year);
								AddUnit(kilograms_per_cubic_meter);
								AddUnit(grams_per_milliliter);
								AddUnit(kilograms_per_liter);
								AddUnit(ounces_per_cubic_foot);
								AddUnit(ounces_per_cubic_inch);
								AddUnit(ounces_per_gallon);
								AddUnit(pounds_per_cubic_foot);
								AddUnit(pounds_per_cubic_inch);
								AddUnit(pounds_per_gallon);
								AddUnit(slugs_per_cubic_foot);
								AddUnit(Dollar_per_joule);
								AddUnit(Dollar_per_kilowatt_hour);
								AddUnit(Dollar_per_watt);
								AddUnit(Dollar_per_kilowatt);
								AddUnit(Dollar_per_cubic_meter);
								AddUnit(Dollar_per_gallon);
								AddUnit(kilowatt_hour_per_acre_foot);
								AddUnit(Dollar_per_mile);
								AddUnit(Dollar_per_ton);
								AddUnit(ton_per_kilowatt_hour);
#undef AddUnit
							}

							// std_namespace->AddUsing(value_namespace);
						}
					}

					// add it to our script...
					imports.emplace("github//scriptLanguage.Units", global_scope2); // the import map guarrantees lifetime...
					global_scope->AddUsing(global_scope2); // ... while "using" allows our global to share their global's custom namespaces and objects
				}
			}

			// TEST SHAREDPTR
			if (1) {
				fibers::utilities::SharedPtr<int> test;
				fibers::parallel::For(0, 100, [&](int i) {
					test = fibers::utilities::make_shared<int>(i);
					fibers::utilities::SharedPtr<int> localCopy{ test };
					if (localCopy) {
						// printf(*localCopy);
					}
				});
            }


			// TESTING...
			if (1) {
				// Simulate a complex, multithreaded ForLoop
				{
					auto ScriptScope = std::make_shared<Scope>(global_scope);
					ScriptScope->p_self = ScriptScope;

					ScriptScope->AddObject("x", std::make_shared<Any>(fibers::containers::number<double>(0)));

					{

						auto ForScope = std::make_shared<Scope>(ScriptScope);
						ForScope->p_self = ForScope;

						fibers::parallel::For(0, 100, [&](int i) {
							auto LoopScope = std::make_shared<Scope>(ForScope);
							LoopScope->p_self = LoopScope;

							LoopScope->AddObject("i", std::make_shared<Any>((int)i));

							if (auto i_obj = LoopScope->FindObject("i")) {
								auto LengthObj = LoopScope->CallFunction("length", { // returns a size_t
									LoopScope->CallFunction("string", { // returns a string
										LoopScope->CallFunction("double", { // returns a double
											*i_obj
										})
									})
								});

								printf(std::string("Length of ") + Impl::Cast<std::string>(*i_obj, LoopScope) + " is " + Impl::Cast<std::string>(LengthObj, LoopScope));

								if (auto x_obj = LoopScope->FindObject("x")) {
									LoopScope->CallFunction("+=", { *x_obj, LengthObj });
								}
							}
						});
					}
					printf(std::string("Final Answer: \t") + Impl::Cast<std::string>(*ScriptScope->FindObject("x"), ScriptScope));
				}

				// Simulate a complex, multithreaded ForLoop which Throws a runtime error during one of the evaluations
				{
					auto ScriptScope = std::make_shared<Scope>(global_scope);
					ScriptScope->p_self = ScriptScope;

					ScriptScope->AddObject("x", std::make_shared<Any>(fibers::containers::number<double>(0)));

					{
						auto ForScope = std::make_shared<Scope>(ScriptScope);
						ForScope->p_self = ForScope;

						try {
							fibers::parallel::For(0, 100, [&](int i) {
								auto LoopScope = std::make_shared<Scope>(ForScope);
								LoopScope->p_self = LoopScope;

								LoopScope->AddObject("i", std::make_shared<Any>((int)i));

								if (auto i_obj = LoopScope->FindObject("i")) {
									auto LengthObj = LoopScope->CallFunction("length", { // returns a size_t
										LoopScope->CallFunction("string", { // returns a string
											LoopScope->CallFunction("double", { // returns a double
												*i_obj
											})
										})
									});

									// printf(std::string("Length of ") + Impl::Cast<std::string>(*i_obj, LoopScope) + " is " + Impl::Cast<std::string>(LengthObj, LoopScope));

									if (Impl::Cast<int>(*LoopScope->FindObject("x"), LoopScope) > 800) {
										throw(std::runtime_error("x cannot be greater than 800 for some random reason!"));
									}

									if (auto x_obj = LoopScope->FindObject("x")) {
										LoopScope->CallFunction("+=", { *x_obj, LengthObj });

										if (Impl::Cast<bool>(LoopScope->CallFunction(">", { *x_obj, 800 }), LoopScope)) {
											throw(std::runtime_error("x cannot be greater than 800 for some random reason"));
										}
									}
								}
							});
							EXPECT_EQ(true, false); // we should not get here.
						}
						catch (std::runtime_error const& e) {
							printf(e.what());
						}
					}

					// the variable x should still be valid, and should have a value greater than 800
					printf(std::string("Final Answer: \t") + Impl::Cast<std::string>(*ScriptScope->FindObject("x"), ScriptScope));
				}

				// Simulate a simple string operation
				{
					// {
					auto ScriptScope = std::make_shared<Scope>(global_scope); ScriptScope->p_self = ScriptScope;
					// var x = "A";
					ScriptScope->AddObject("x", std::make_shared<Any>(std::string("A")));
					// var y = "B";
					ScriptScope->AddObject("y", std::make_shared<Any>(std::string("B")));
					// return x + y;
					printf(Impl::Cast<std::string>(ScriptScope->CallFunction("+", { *ScriptScope->FindObject("x"), *ScriptScope->FindObject("y") }), ScriptScope));
					// }
				}

				// Simulate a simple Units operation
				{
					// {
					auto ScriptScope = std::make_shared<Scope>(global_scope); ScriptScope->p_self = ScriptScope;
					// Using namespace "Units"
					ScriptScope->AddUsing(ScriptScope->FindNamespace("Units"));
					// var x = foot(int(10.4));
					ScriptScope->AddObject("x", std::make_shared<Any>(ScriptScope->CallFunction("foot", { ScriptScope->CallFunction("int", { 10.4 }) })));
					// var y = meter(100);
					ScriptScope->AddObject("y", std::make_shared<Any>(ScriptScope->CallFunction("meter", { 100 })));
					// var z = inch(12);
					ScriptScope->AddObject("z", std::make_shared<Any>(ScriptScope->CallFunction("inch", { 12 })));
					// return Units::gallon(x*y*z);
					auto result = ScriptScope->CallFunction("gallon", { ScriptScope->CallFunction("*", { ScriptScope->CallFunction("*", { *ScriptScope->FindObject("x"), *ScriptScope->FindObject("y") }), *ScriptScope->FindObject("z") }) });
					printf(Impl::Cast<std::string>(result, ScriptScope));
					// }
				}

				// Simulate a for-loop that 1: creates a new Class, 2: adds functions to it, 3: adds a conversion for it to std::string, and 4: uses that conversion. 
				{
					auto ScriptScope = std::make_shared<Scope>(global_scope);
					ScriptScope->p_self = ScriptScope;

					{
						auto ForScope = std::make_shared<Scope>(ScriptScope);
						ForScope->p_self = ForScope;

						try{
							fibers::containers::number<int> TTT{ 0 };
							fibers::parallel::For(0, 9, [&](int i) { // for (int i = 0; i < 100; i++){// 
								if ((++TTT) >= 9) {
									::Sleep(1000);
									std::cout << "test" << std::endl;
								}

								auto LoopScope = std::make_shared<Scope>(ForScope);
								LoopScope->p_self = LoopScope;
								LoopScope->AddObject("i", std::make_shared<Any>((int)i));

								Type_Info expectedType;
								if (1) {
									// Make the new class...
									auto Position = std::make_shared<Class>(LoopScope, "Position");
									Position->p_self = Position;

									if (1) {
										Position->m_functions.emplace("Position", make_callable([thisScope = std::weak_ptr<Scope>(Position), t = Position->ClassType]()->fibers::DynamicObject {
											fibers::DynamicObject out(t);
											if (auto ptr = thisScope.lock()) {
												out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>("longitude", std::make_shared<Any>(ptr->CallFunction("Number", {}))));
												out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>("latitude", std::make_shared<Any>(ptr->CallFunction("Number", {}))));
												out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>("elevation", std::make_shared<Any>(ptr->CallFunction("Units::foot", {}))));
											}
											else {
												throw(std::runtime_error("Class definition was no longer available"));
											}
											return out;
										}));
										Position->m_functions.emplace("Position", make_callable([thisScope = std::weak_ptr<Scope>(Position), t = Position->ClassType](Any const& tryCopy)->fibers::DynamicObject {
											fibers::DynamicObject out(t);
											// generic code to "Copy" the Dynamic Object
											if (auto ptr = thisScope.lock()) {
												auto& tryCopyObj = tryCopy.cast<fibers::DynamicObject>();
												for (auto& obj : tryCopyObj.m_objects) {
													auto& obj_name = obj.first; 
													Any& obj_obj = *obj.second;
													Type_Info obj_type = obj_obj.Type();
												
													// if we cannot find this type anymore, how can we be expected to copy it?
													if (auto ObjClass = ptr->FindClass(obj_type)) {
														try {
															out.m_objects.insert(
																std::pair<std::string, std::shared_ptr<Any>>{
																	obj_name,
																	std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj_obj })) // e.g. Number(number_object) , or, double(double_obj)
																}
															);
														}
														catch (exception::not_found_error) {
															// the function to actually copy the object was not found or doesn't exist -- so use the value as-is. Nothing more can be done without throwing the error upstream. 
															out.m_objects.insert(
																std::pair<std::string, std::shared_ptr<Any>>{
																	obj_name,
																	obj.second
																}
															);
														}

														// MAKE SURE THAT WE ACTUALLY MADE A COPY, AND NOT JUST PASSING IT ALONG
														Any& new_obj = *out.m_objects[obj_name];
														Any& old_obj = *tryCopyObj.m_objects[obj_name];
														EXPECT_NE(new_obj, old_obj);
													}
												}
											}
											else {
												throw(std::runtime_error("Class definition was no longer available"));
											}
											return out;
										}), Param_Types({ { "ToCopy", Position->ClassType } }));
										Position->m_functions.emplace("Position", make_callable([thisScope = std::weak_ptr<Scope>(Position), t = Position->ClassType](double longitude, double latitude, Units::foot elevation)->fibers::DynamicObject {
											fibers::DynamicObject out(t);
											if (auto ptr = thisScope.lock()) {
												out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>("longitude", std::make_shared<Any>(ptr->CallFunction("Number", {
													longitude
												}))));
												out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>("latitude", std::make_shared<Any>(ptr->CallFunction("Number", {
													latitude
												}))));
												out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>("elevation", std::make_shared<Any>(ptr->CallFunction("Units::foot", {
													elevation
												}))));
											}
											else {
												throw(std::runtime_error("Class definition was no longer available"));
											}
											return out;
										}), Param_Types({ { "longitude", user_type<double>() }, { "latitude", user_type<double>() }, { "elevation", user_type<Units::foot>() } }));
										Position->m_functions.emplace("to_string", make_callable([thisScope = std::weak_ptr<Scope>(Position)](Any const& parent)->std::string {
											std::string out;
											// generic code to "Print" the Dynamic Object
											auto& Parent = parent.cast<fibers::DynamicObject>();
											if (auto ptr = thisScope.lock()) {
												for (auto& obj : Parent.m_objects) {
													if (out.length() > 0) out += ",";
													auto conv = Impl::Cast<std::string>(*obj.second, ptr);
													out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
												}
											}
											else {
												throw(std::runtime_error("Class definition was no longer available"));
											}
											return std::string("{") + out + " }";
										}), Param_Types({ { "parent", Position->ClassType } }));
										bool successfulInsert = Position->TypeConversionTree()->AddConverter([thisScope = std::weak_ptr<Scope>(Position)](Any const& parent)->std::string {
											std::string out;
											auto& Parent = parent.cast<fibers::DynamicObject>();
											if (auto ptr = thisScope.lock()) {
												for (auto& obj : Parent.m_objects) {
													if (out.length() > 0) out += ",";
													auto conv = Impl::Cast<std::string>(*obj.second, ptr);
													out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
												}
											}
											else {
												throw(std::runtime_error("Class definition was no longer available"));
											}
											return std::string("{") + out + " }";
										}, Position->ClassType, user_type<std::string>());
										EXPECT_EQ(successfulInsert, true);
										Position->m_functions.emplace("longitude", make_callable([](Any const& parent)-> Any {
											auto& Parent = parent.cast<fibers::DynamicObject>();
											return *Parent.m_objects.at("longitude");
										}), Param_Types({ { "parent", Position->ClassType } }));
										Position->m_functions.emplace("latitude", make_callable([](Any const& parent)-> Any {
											auto& Parent = parent.cast<fibers::DynamicObject>();
											return *Parent.m_objects.at("latitude");
										}), Param_Types({ { "parent", Position->ClassType } }));
										Position->m_functions.emplace("elevation", make_callable([](Any const& parent)-> Any {
											auto& Parent = parent.cast<fibers::DynamicObject>();
											return *Parent.m_objects.at("elevation");
										}), Param_Types({ { "parent", Position->ClassType } }));
									}

									// ...and add it to the parent scope
									LoopScope->AddChild(Position);
									expectedType = Position->ClassType;
								}

								if (auto i_obj = LoopScope->FindObject("i")) {
									auto PositionInstance1 = LoopScope->CallFunction("Position", {  }); // create as instance
									auto PositionInstance2 = LoopScope->CallFunction("Position", { -121, 32, Units::foot(15) }); // create from objs
									auto PositionInstance3 = LoopScope->CallFunction("Position", { PositionInstance2 }); // create as copy

									EXPECT_EQ(expectedType, PositionInstance1.Type());
									EXPECT_EQ(expectedType, PositionInstance2.Type());
									EXPECT_EQ(expectedType, PositionInstance3.Type());

									EXPECT_EQ(LoopScope->TypeConversionTree()->nodes.contains(PositionInstance1.Type()), true);
									EXPECT_EQ(LoopScope->TypeConversionTree()->nodes.contains(PositionInstance2.Type()), true);
									EXPECT_EQ(LoopScope->TypeConversionTree()->nodes.contains(PositionInstance3.Type()), true);

									if (auto pppp = LoopScope->TypeConversionTree()->nodes.at_or(PositionInstance2.Type(), nullptr)) {
										EXPECT_EQ(true, pppp->connections.contains(user_type<std::string>()));
									}

									EXPECT_EQ(LoopScope->TypeConversionTree()->Converts(PositionInstance1, user_type<std::string>()), true);
									EXPECT_EQ(LoopScope->TypeConversionTree()->Converts(PositionInstance2, user_type<std::string>()), true);
									EXPECT_EQ(LoopScope->TypeConversionTree()->Converts(PositionInstance3, user_type<std::string>()), true);

									{
										auto tree = LoopScope->GetCombinedTypeConversionTree();
										if (!tree.nodes.contains(PositionInstance1.Type())) {
											tree = LoopScope->GetCombinedTypeConversionTree();
										}

										if (!EXPECT_EQ(tree.nodes.contains(PositionInstance1.Type()), true)) {
											printf(PositionInstance1.Type().lock()->raw_name());
										}
										if (!EXPECT_EQ(tree.nodes.contains(PositionInstance2.Type()), true)) {
											printf(PositionInstance2.Type().lock()->raw_name());
										}
										if (!EXPECT_EQ(tree.nodes.contains(PositionInstance3.Type()), true)) {
											printf(PositionInstance3.Type().lock()->raw_name());
										}

										if (auto pppp = tree.nodes.at_or(PositionInstance2.Type(), nullptr)) {
											EXPECT_EQ(true, pppp->connections.contains(user_type<std::string>()));
										}
										else {
											EXPECT_EQ(true, false);
										}

										EXPECT_EQ(tree.Converts(PositionInstance1, user_type<std::string>()), true);
										EXPECT_EQ(tree.Converts(PositionInstance2, user_type<std::string>()), true);
										EXPECT_EQ(tree.Converts(PositionInstance3, user_type<std::string>()), true);


									}
									// Gets an object with the type of THIS Position...							

									// ...which may have been replaced with a whole new version of Position by the time we get here...
									// ...and since "longitude" would only be found if the class type of Position is found, which it never will be, it'll fail right here. 
									EXPECT_EQ(-121, Impl::Cast<int>(LoopScope->CallFunction("longitude", { PositionInstance2 }), LoopScope));									
									EXPECT_EQ(-121, Impl::Cast<int>(LoopScope->CallFunction("longitude", { PositionInstance3 }), LoopScope));

									auto& longitude = LoopScope->CallFunction("longitude", { PositionInstance2 }).cast<fibers::containers::number<double>&>();
									EXPECT_EQ(-121, longitude);
									LoopScope->CallFunction("+=", { LoopScope->CallFunction("longitude", { PositionInstance2 }), -1 });
									EXPECT_EQ(-122, longitude);
									EXPECT_EQ(-122, LoopScope->CallFunction("longitude", { PositionInstance2 }).cast<fibers::containers::number<double>&>());
								
									EXPECT_EQ(-121, LoopScope->CallFunction("longitude", { PositionInstance3 }).cast<fibers::containers::number<double>&>());

									// Test that we can cast to std::string
									try {
										printf(Impl::Cast<std::string>(i_obj, LoopScope) + ": " + Impl::Cast<std::string>(PositionInstance2, LoopScope));
									} catch (std::exception& ee) {
										printf("PRINTING:");
										try {
											printf(Impl::Cast<std::string>(i_obj, LoopScope) + ": " + Impl::Cast<std::string>(PositionInstance2, LoopScope));
										}
										catch (std::exception& eee) {
											printf(Impl::Cast<std::string>(i_obj, LoopScope) + ": #NA: " + eee.what());
											for (auto& child : LoopScope->GetAvailableNamespaces(std::make_shared<std::set<std::shared_ptr<Scope>>>(), true, true)) {
												printf(child.first + "\t  ->  \t" + child.second.lock()->GetQualifiedNamespace(true));
											}
											printf("");
										}										
									}
								}
							});
						}
						catch (std::exception& e) {
							printf(e.what());
						}
					}

					// now, we are going to do a bad thing on purpose, and try to call that custom class and its functions. 
					if (1) {
						try {
							auto PositionInstance1 = ScriptScope->CallFunction("Position", {  }); // create as instance
							EXPECT_EQ(false, true); // should not happen
						}
						catch (exception::not_found_error const& e) {
							printf(e.what());
							EXPECT_EQ(true, true); // Good!
						}
					}
				}








				auto localScope = std::make_shared<Scope>(global_scope);
				localScope->p_self = localScope;

				if (1) {
					auto& tree = localScope->GetCombinedTypeConversionTree(); // Likely returns the tree used for the parent scope
					EXPECT_EQ(true, tree.Converts(scripting::user_type<int>(), scripting::user_type<float>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<double>(), scripting::user_type<float>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<unsigned char>(), scripting::user_type<char>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<long>(), scripting::user_type<double>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<unsigned int>(), scripting::user_type<long double>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<Units::foot>(), scripting::user_type<Units::value>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<Units::foot>(), scripting::user_type<Units::meter>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<float>(), scripting::user_type<std::string>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<int>(), scripting::user_type<std::string>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<double>(), scripting::user_type<std::string>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<size_t>(), scripting::user_type<std::string>()));
					EXPECT_EQ(true, tree.Converts(scripting::user_type<Units::foot>(), scripting::user_type<std::string>()));

					tree.Convert(100.0, scripting::user_type<Units::foot>());
					tree.Convert(Units::foot(100.0), scripting::user_type<Units::value>());
					tree.Convert(Units::foot(100.0), scripting::user_type<Units::meter>());
				}

				if (Any f = localScope->CallFunction("double", { Any(100.0) })) {
					printf(f.cast<double>());
				}
				if (Any f = localScope->CallFunction("int", { Any(100) })) {
					printf(f.cast<int>());
				}
				if (Any f = localScope->CallFunction("double", { Any(10) })) {
					printf(f.cast<double>());
				}
				if (Any f = localScope->CallFunction("int", { Any(10.0) })) {
					printf(f.cast<int>());
				}
				if (Any f = localScope->CallFunction("char", { Any(int('A'))})) {
					printf(f.cast<char>());
				}
				if (Any f = localScope->CallFunction("double", {})) {
					printf(f.cast<double>());
				}
				if (Any f = localScope->CallFunction("double", { Units::foot( 100.0 ) })) {
					printf(f.cast<double>());
				}
				if (Any f = localScope->CallFunction("double", { Units::meter(Units::foot(100.0)) })) {
					printf(f.cast<double>());
				}

				if (Any f = localScope->CallFunction("max", { 100.0 })) {
					printf(f.cast<double>());
				}
				if (Any f = localScope->CallFunction("max", { bool() })) {
					printf(f.cast<bool>());
				}
				if (Any f = localScope->CallFunction("min", { int() })) {
					printf(f.cast<int>());
				}
				if (Any f = localScope->CallFunction("int::min", {})) {
					printf(f.cast<int>());
				}
				if (Any f = localScope->CallFunction("int::min", { int() })) {
					printf(f.cast<int>());
				}
				if (Any f = localScope->CallFunction("int::min", { double() })) {
					printf(f.cast<int>());
				}
				if (Any f = localScope->CallFunction("to_string", { double(100.0) })) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("to_string", { (unsigned int)(55) })) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("to_string", { (Units::value)(54) })) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("to_string", { (Units::foot)(50) })) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("!=", { 50.0, 75.0 })) {
					printf(f.cast<bool>());
				}
				if (Any f = localScope->CallFunction("==", { 500, 500.0f })) {
					printf(f.cast<bool>());
				}
				if (Any f = localScope->CallFunction("==", { 'z', (int)('z') })) {
					printf(f.cast<bool>());
				}
				if (Any f = localScope->CallFunction("to_string", { localScope->CallFunction("*", { 5.0f, 5.0 }) })) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("string", { Any(36.0f) })) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("string", { localScope->CallFunction("ldouble", { 5555ll }) })) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("string", {})) {
					printf(f.cast<std::string>());
				}
				if (Any f = localScope->CallFunction("float", {})) {
					printf(f.cast<float>());
				}
				if (Any f = localScope->CallFunction("float", { 100.0 })) {
					printf(f.cast<float>());
				}

				if (auto foundScope = localScope->FindClass(user_type<float>())) {
					if (Any f = localScope->CallFunction(foundScope->GetName(), { 123.0 })) {
						printf(f.cast<float>());
					}
				}

				printf(Impl::Cast<float>(321, localScope));
				printf(Impl::Cast<std::string>(std::string("I am a TEST"), localScope));
				printf(Impl::Cast<std::string>(Units::gallon(100), localScope));
				printf(Impl::Cast<Units::gallon>(localScope->CallFunction("length", { std::string("TEST") }), localScope));

				printf(Impl::Cast(321, user_type<std::string>(), localScope).cast<std::string>());
				printf(Impl::Cast(std::string("I am a TEST"), user_type<std::string>(), localScope).cast<std::string>());
				printf(Impl::Cast(Units::gallon(100), user_type<std::string>(), localScope).cast<std::string>());
				printf(Impl::Cast(localScope->CallFunction("length", { std::string("TEST") }), user_type<std::string>(), localScope).cast<std::string>());

				localScope->Print();

				printf("PRINTING:");
				for (auto& child : localScope->GetAvailableNamespaces()) {
					printf(child.first + "\t  ->  \t" + child.second.lock()->GetQualifiedNamespace());
				}
				printf("");
				
				Any foot_v;
				{
					foot_v = localScope->CallFunction("Units::foot", { Any(100.0) });
					printf(foot_v.cast<Units::foot>().ToString());
				}

				if (auto nsFound = localScope->FindNamespace("Units")) {
					foot_v = nsFound->CallFunction("foot", { Any(100.0) });
					printf(foot_v.cast<Units::foot>().ToString());
				}

				// ISSUE: the wrong "type conversion tree" is being searched for potential conversions
				// SOLUTION: must search MULTIPLE trees to generate the correct solution. 
				{
					auto meter_v = localScope->CallFunction("Units::meter", { foot_v });
					printf(meter_v.cast<Units::meter>().ToString());
				}
				{
					auto meter_v = localScope->CallFunction("Units::meter", { foot_v });
					printf(meter_v.cast<Units::meter>().ToString());
				}

				// expect success
				{
					auto text_v = localScope->CallFunction("to_string", { foot_v });
					printf(text_v.cast<std::string>());
				}
				















			}
		}

#endif

#if 0
		// MODERN TEST
		if (1) {
			using namespace scripting;
			auto printf = [](auto x) { std::cout << x << std::endl; };

			// Import Collection...
			fibers::containers::Map<std::string, std::shared_ptr<Namespace>> imports;

			// Global Namespace...
			auto global_scope{ std::make_shared<Namespace>() }; // global should always be a Namespace
			global_scope->p_self = global_scope;

			{
				// "Import" the `STD` library...
				if (1) {
					// DEMONSTRATES HOW TO MAKE A INDEPENDANT IMPORT (e.g. from github), store it in its own, independant scope (global_scope2), and "add it" to another running scope without changing it.

					auto global_scope2{ std::make_shared<Namespace>() }; // global should always be a Namespace
					global_scope2->p_self = global_scope2;

					// Add Global Functions...
					{
						global_scope2->m_functions.emplace("Type", scripting::Param_Types({ { std::string("obj"), scripting::user_type<Any>() } }), scripting::make_callable(
							[](Any const& x) -> fibers::Type_Info {
								if (auto p = x.Type().lock())
									return *p;
								else
									return fibers::user_type<void>();
							}
						), true);
					}

					// Create STD library...
					{
						auto std_namespace{ std::make_shared<Namespace>(global_scope2, "std") };
						std_namespace->p_self = std_namespace;
						global_scope2->AddChild(std_namespace);

						// the "std" namespace imports the "string" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::string>(), std_namespace, "string") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...								
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which has a couple constructor functions ... 
							string_namespace->m_functions.emplace("string", scripting::make_callable([](std::string const& x) -> std::string { return x; }), scripting::Param_Types({ { std::string("parent"), string_namespace->ClassType } }));
							string_namespace->m_functions.emplace("string", scripting::make_callable([](Any const& x) -> std::string {  return x.TypeName(); }), scripting::Param_Types({ { std::string("parent"), scripting::user_type<float>() } }));
							string_namespace->m_functions.emplace("string", scripting::make_callable([](Any const& x) -> std::string {  return x.TypeName(); }), scripting::Param_Types({ { std::string("parent"), scripting::user_type<int>() } }));
							string_namespace->m_functions.emplace("string", scripting::make_callable([](Any const& x) -> std::string {  return x.TypeName(); }), scripting::Param_Types({ { std::string("parent"), scripting::user_type<double>() } }));						

							// ... and has "to_string" functions...
							std_namespace->m_functions.emplace("to_string", scripting::Param_Types({ { "o", string_namespace->ClassType } }), scripting::make_callable(
								[](std::string const& x) -> std::string { return x; }
							));
							std_namespace->m_functions.emplace("to_string", scripting::Param_Types({ { "o", user_type<int>() } }), scripting::make_callable(
								[](int const& x) -> std::string { return std::to_string(x); }
							));
							std_namespace->m_functions.emplace("to_string", scripting::Param_Types({ { "o", user_type<float>() } }), scripting::make_callable(
								[](float const& x) -> std::string { return std::to_string(x); }
							));
							std_namespace->m_functions.emplace("to_string", scripting::Param_Types({ { "o", user_type<double>() } }), scripting::make_callable(
								[](double const& x) -> std::string { return std::to_string(x); }
							));
							std_namespace->m_functions.emplace("to_string", scripting::Param_Types({ { "o", user_type<Any>() } }), scripting::make_callable(
								[](Any const& x) -> std::string { return Units::printf("`%s`", x.TypeName()); }
							));
						}

						// the "std" namespace imports the "map" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::map<std::string, fibers::Any>>(), std_namespace, "map") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "std" namespace imports the "set" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::set<std::string>>(), std_namespace, "set") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "std" namespace imports the "vector" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::vector<fibers::Any>>(), std_namespace, "vector") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}
					}

					// add it to our script...
					imports.emplace("github//scriptLanguage.std", global_scope2); // the import map guarrantees lifetime...
					global_scope->AddUsing(global_scope2); // ... while "using" allows our global to share their global's custom namespaces and objects
				}

				// "Import" the `fibers` library...
				if (1) {
				    auto script_scope{ std::make_shared<Namespace>() }; // global should always be a Namespace
					script_scope->p_self = script_scope;

					{
						auto std_namespace{ std::make_shared<Namespace>(script_scope, "fibers") };
						std_namespace->p_self = std_namespace;
						script_scope->AddChild(std_namespace);

						// the "fibers" namespace imports the "Number" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<fibers::containers::number<double>>(), std_namespace, "Number") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...								
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "fibers" namespace imports the "Pattern" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(fibers::user_type<fibers::containers::Pattern<double, double>>(), std_namespace, "Pattern") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "fibers" namespace imports the "Matrix" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(std_namespace, "Matrix") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}

						// the "fibers" namespace imports the "UI" namespace...
						{
							auto string_namespace{ std::make_shared<Namespace>(std_namespace, "UI") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// ... which imports the "Interactive" namespace...
							auto interactive_namespace{ std::make_shared<Class>(string_namespace, "Interactive") }; {
								interactive_namespace->p_self = interactive_namespace;
								string_namespace->AddChild(interactive_namespace);

								{
									auto impl_namespace{ std::make_shared<Namespace>(interactive_namespace, "InteractiveImpl") };
									impl_namespace->p_self = impl_namespace;
									interactive_namespace->AddChild(impl_namespace);
								}
							}

							// ... which imports the "Map" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Map", interactive_namespace) };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);

								{
									auto impl_namespace2{ std::make_shared<Namespace>(impl_namespace, "MapImpl") };
									impl_namespace2->p_self = impl_namespace2;
									impl_namespace->AddChild(impl_namespace2);
								}

								{
									auto impl_namespace2{ std::make_shared<Namespace>(impl_namespace, "Text") };
									impl_namespace2->p_self = impl_namespace2;
									impl_namespace->AddChild(impl_namespace2);
								}

								impl_namespace->m_functions.emplace("=", make_callable([](Any& lhs, Any const& rhs) -> Any {
									fibers::DynamicObject& LHS = lhs.cast();
									fibers::DynamicObject& RHS = rhs.cast();
									LHS = RHS;
									return Any(lhs);
								}), scripting::Param_Types({ { "lhs", impl_namespace->ClassType }, { "rhs", impl_namespace->ClassType } }));
							}

							// ... which imports the "Button" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Button", interactive_namespace) };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "Grid" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Grid") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "Plot" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Plot") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "Text" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "Text") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

							// ... which imports the "WebPage" namespace...
							{
								auto impl_namespace{ std::make_shared<Class>(string_namespace, "WebPage") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}

						}
					}

					// add it to our script...
					imports.emplace("github//scriptLanguage.fibers", script_scope); // the import map guarrantees lifetime...
					global_scope->AddUsing(script_scope); // ... while "using" allows our global to share their global's custom namespaces and objects
                }

				// "Import" the `Units` library...
				if (1) {
					Type_Converter_Tree tree;
					auto global_scope2{ std::make_shared<Namespace>() }; // global should always be a Namespace
					global_scope2->p_self = global_scope2;

					// Create STD library...
					{
						auto std_namespace{ std::make_shared<Namespace>(global_scope2, "Units") };
						std_namespace->p_self = std_namespace;
						global_scope2->AddChild(std_namespace);

						// the "std" namespace imports the "string" namespace...
						{
							auto value_namespace{ std::make_shared<Class>(fibers::user_type<Units::value>(), std_namespace, "value") };
							value_namespace->p_self = value_namespace;
							std_namespace->AddChild(value_namespace);

							// which has the following types groups... 
							
							{
								// default, built-in conversions...
								tree.AddConverter<int, double>();
								tree.AddConverter<int, float>();
								tree.AddConverter<double, float>();
								tree.AddConverter([](int x) -> std::string { return std::to_string(x); });
								tree.AddConverter([](float x) -> std::string { return std::to_string(x); });
								tree.AddConverter([](double x) -> std::string { return std::to_string(x); });
								tree.AddConverter([](std::string x) -> double { return std::atof(x.c_str()); });
								tree.AddConverter([](std::string x) -> float { return std::atof(x.c_str()); });
								tree.AddConverter([](std::string x) -> int { return std::atof(x.c_str()); });
							}

							{
								tree.AddConverter<Units::value, double>();
								tree.AddConverter([](Units::value const& x) -> std::string { return x.ToString(); });
								value_namespace->m_functions.emplace("abbreviation", make_callable([](Units::value const& x)->std::string {
									return x.Abbreviation();
								}), scripting::Param_Types());
								value_namespace->m_functions.emplace("name", make_callable([](Units::value const& x)->std::string {
									return x.UnitName();
								}), scripting::Param_Types());

								// manually go through and add each unit type... 
								auto allUnitTypes = Units::UnitsDetail::GetValueTypes();
								auto addUnit = [&](auto impl, std::string const& name) -> void {
									auto impl_namespace{ std::make_shared<Class>(std_namespace, name, value_namespace, user_type<decltype(impl)>()) };
									impl_namespace->p_self = impl_namespace;
									std_namespace->AddChild(impl_namespace);

									// Polymorphic converter
									tree.AddConverter<decltype(impl), Units::value>();
									tree.AddConverter<Units::value, decltype(impl)>();
									tree.AddConverter<decltype(impl), double>();
									tree.AddConverter([](decltype(impl) const& x) -> std::string { return x.ToString(); });

									bool found{ false };
									for (auto& type_group : allUnitTypes) {
										if (found) break;
										for (auto& type : type_group) {
											if (std::get<1>(type) == name) {
												auto& abbrev = std::get<0>(type);

												// POSTFIX
												std_namespace->m_postfixes.emplace(abbrev, impl_namespace);

												// CONSTRUCT {}
												impl_namespace->m_functions.emplace(name, make_callable([thisT = impl]()->Units::value {
													Units::value out = thisT;
													out = 0;
													return out;
												}), scripting::Param_Types());

												// CONSTRUCT { double }
												impl_namespace->m_functions.emplace(name, make_callable([thisT = impl](double x)->Units::value {
													Units::value out = thisT;
													out = x;
													return out;
												}), scripting::Param_Types({ { "in", user_type<double>() } }));

												// CONSTRUCT { Units::value }
												impl_namespace->m_functions.emplace(name, make_callable([thisT = impl](Units::value const& x)->Units::value {
													Units::value out = thisT;
													out = x;
													return out;
												}), scripting::Param_Types({ { "in", user_type<Units::value>() } }));

												// abbreviation
												impl_namespace->m_functions.emplace("abbreviation", make_callable([thisT = abbrev]()->std::string {
													return thisT;
												}), scripting::Param_Types({ { "parent", user_type<decltype(impl)>() } }));

												// name
												impl_namespace->m_functions.emplace("name", make_callable([thisT = name]()->std::string {
													return thisT;
												}), scripting::Param_Types({ { "parent", user_type<decltype(impl)>() } }));
												
												std_namespace->AddUsing(impl_namespace);

												found = true;
												break;
											}
										}
									}
								};

#define AddUnit(unitname) addUnit(Units::##unitname(), #unitname)
								AddUnit(meter);
								AddUnit(foot);
								AddUnit(inch);
								AddUnit(mile);
								AddUnit(nauticalMile);
								AddUnit(astronicalUnit);
								AddUnit(yard);
								AddUnit(gram);
								AddUnit(metric_ton);
								AddUnit(pound);
								AddUnit(long_ton);
								AddUnit(short_ton);
								AddUnit(stone);
								AddUnit(ounce);
								AddUnit(carat);
								AddUnit(slug);
								AddUnit(second);
								AddUnit(minute);
								AddUnit(hour);
								AddUnit(day);
								AddUnit(week);
								AddUnit(year);
								AddUnit(month);
								AddUnit(julian_year);
								AddUnit(gregorian_year);
								AddUnit(ampere);
								AddUnit(Dollar);
								AddUnit(MillionDollar);
								AddUnit(hertz);
								AddUnit(meters_per_second);
								AddUnit(feet_per_second);
								AddUnit(feet_per_minute);
								AddUnit(feet_per_hour);
								AddUnit(miles_per_hour);
								AddUnit(kilometers_per_hour);
								AddUnit(knot);
								AddUnit(meters_per_second_squared);
								AddUnit(feet_per_second_squared);
								AddUnit(standard_gravity);
								AddUnit(newton);
								AddUnit(pound_f);
								AddUnit(dyne);
								AddUnit(kilopond);
								AddUnit(poundal);
								AddUnit(pascals);
								AddUnit(bar);
								AddUnit(atmosphere);
								AddUnit(pounds_per_square_inch);
								AddUnit(head);
								AddUnit(torr);
								AddUnit(coulomb);
								AddUnit(ampere_hour);
								AddUnit(watt);
								AddUnit(horsepower);
								AddUnit(joule);
								AddUnit(calorie);
								AddUnit(watt_minute);
								AddUnit(watt_hour);
								AddUnit(watt_day);
								AddUnit(british_thermal_unit);
								AddUnit(british_thermal_unit_iso);
								AddUnit(british_thermal_unit_59);
								AddUnit(therm);
								AddUnit(foot_pound);
								AddUnit(volt);
								AddUnit(ohm);
								AddUnit(siemens);
								AddUnit(square_meter);
								AddUnit(square_foot);
								AddUnit(square_inch);
								AddUnit(square_mile);
								AddUnit(square_kilometer);
								AddUnit(hectare);
								AddUnit(acre);
								AddUnit(cubic_meter);
								AddUnit(cubic_millimeter);
								AddUnit(cubic_kilometer);
								AddUnit(liter);
								AddUnit(cubic_inch);
								AddUnit(cubic_foot);
								AddUnit(cubic_yard);
								AddUnit(cubic_mile);
								AddUnit(gallon);
								AddUnit(imperial_gallon);
								AddUnit(million_gallon);
								AddUnit(imperial_million_gallon);
								AddUnit(acre_foot);
								AddUnit(quart);
								AddUnit(pint);
								AddUnit(cup);
								AddUnit(fluid_ounce);
								AddUnit(barrel);
								AddUnit(bushel);
								AddUnit(cord);
								AddUnit(tablespoon);
								AddUnit(teaspoon);
								AddUnit(pinch);
								AddUnit(dash);
								AddUnit(drop);
								AddUnit(fifth);
								AddUnit(dram);
								AddUnit(gill);
								AddUnit(peck);
								AddUnit(sack);
								AddUnit(shot);
								AddUnit(strike);
								AddUnit(gram_per_second);
								AddUnit(metric_ton_per_second);
								AddUnit(metric_ton_per_minute);
								AddUnit(metric_ton_per_hour);
								AddUnit(metric_ton_per_day);
								AddUnit(metric_ton_per_year);
								AddUnit(cubic_meter_per_second);
								AddUnit(cubic_meter_per_hour);
								AddUnit(cubic_meter_per_day);
								AddUnit(cubic_millimeter_per_second);
								AddUnit(liter_per_second);
								AddUnit(liter_per_minute);
								AddUnit(liter_per_day);
								AddUnit(megaliter_per_day);
								AddUnit(cubic_inch_per_second);
								AddUnit(cubic_inch_per_hour);
								AddUnit(cubic_foot_per_second);
								AddUnit(cubic_foot_per_hour);
								AddUnit(gallon_per_second);
								AddUnit(gallon_per_minute);
								AddUnit(gallon_per_hour);
								AddUnit(gallon_per_day);
								AddUnit(gallon_per_year);
								AddUnit(million_gallon_per_second);
								AddUnit(million_gallon_per_minute);
								AddUnit(million_gallon_per_hour);
								AddUnit(million_gallon_per_day);
								AddUnit(million_gallon_per_year);
								AddUnit(imperial_million_gallon_per_second);
								AddUnit(imperial_million_gallon_per_minute);
								AddUnit(imperial_million_gallon_per_hour);
								AddUnit(imperial_million_gallon_per_day);
								AddUnit(imperial_million_gallon_per_year);
								AddUnit(acre_foot_per_second);
								AddUnit(acre_foot_per_minute);
								AddUnit(acre_foot_per_hour);
								AddUnit(acre_foot_per_day);
								AddUnit(acre_foot_per_year);
								AddUnit(kilograms_per_cubic_meter);
								AddUnit(grams_per_milliliter);
								AddUnit(kilograms_per_liter);
								AddUnit(ounces_per_cubic_foot);
								AddUnit(ounces_per_cubic_inch);
								AddUnit(ounces_per_gallon);
								AddUnit(pounds_per_cubic_foot);
								AddUnit(pounds_per_cubic_inch);
								AddUnit(pounds_per_gallon);
								AddUnit(slugs_per_cubic_foot);
								AddUnit(Dollar_per_joule);
								AddUnit(Dollar_per_kilowatt_hour);
								AddUnit(Dollar_per_watt);
								AddUnit(Dollar_per_kilowatt);
								AddUnit(Dollar_per_cubic_meter);
								AddUnit(Dollar_per_gallon);
								AddUnit(kilowatt_hour_per_acre_foot);
								AddUnit(Dollar_per_mile);
								AddUnit(Dollar_per_ton);
								AddUnit(ton_per_kilowatt_hour);
#undef AddUnit



							}

							// TESTING -> use those converters! 
							if (1) {
								Any obj; 

								obj = Units::foot(1); 
								EXPECT_EQ(true, tree.Converts(obj, user_type<Units::inch>()));
								
								obj = Units::foot(1); 
								EXPECT_EQ(true, tree.Converts(obj, user_type<Units::meter>()));
								
								obj = Units::second(1); 
								EXPECT_EQ(true, tree.Converts(obj, user_type<Units::year>()));
								
								obj = Units::gallon(1); 
								EXPECT_EQ(true, tree.Converts(obj, user_type<Units::million_gallon>()));
								
								obj = Units::foot(1) * Units::foot(1); 
								EXPECT_EQ(true, tree.Converts(obj, user_type<Units::acre>()));
								
								std::cout << tree.Convert(Units::foot(1), user_type<Units::inch>()).cast<Units::inch>() << std::endl;
								std::cout << tree.Convert(Units::foot(1) * Units::foot(1), user_type<Units::acre>()).cast<Units::acre>() << std::endl;
								std::cout << tree.Convert(Units::foot(1) * Units::inch(1) * Units::meter(1), user_type<Units::gallon>()).cast<Units::gallon>() << std::endl;

								obj = Units::foot(1) * Units::foot(1);
								EXPECT_EQ(true, tree.Converts(obj, user_type<Units::year>())); // technically, Units::value -> Units::year is a valid conversion, since we can't (until runtime) determine the actual unit type.

								EXPECT_EQ(true, tree.Convert(Units::foot(1), user_type<Units::inch>()).IsTypeOf(user_type<Units::inch>()));
								EXPECT_EQ(true, tree.Convert(Units::foot(1) * Units::foot(1), user_type<Units::acre>()).IsTypeOf(user_type<Units::acre>()));
								EXPECT_EQ(true, tree.Convert(Units::foot(1), user_type<Units::value>()).IsTypeOf(user_type<Units::value>()));

								auto tempScope = std::make_shared<scripting::Scope>(global_scope2);
								tempScope->p_self = tempScope;
								{
									tempScope->AddUsing(std_namespace);
									std::vector<Any> params{ Any(100.0) };
									if (auto constructor = tempScope->FindFunction("foot", Function_Params{ params }, tree)) {
										auto returned_foot = call(constructor, params, tree);

										if (auto foundClass = tempScope->FindClass("meter")) {
											auto returned = tree.Convert(returned_foot, foundClass->ClassType);

											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}

										if (auto foundClass = tempScope->FindClass("inch")) {
											auto returned = tree.Convert(returned_foot, foundClass->ClassType);

											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}

										if (auto foundClass = tempScope->FindClass("yard")) {
											auto returned = tree.Convert(returned_foot, foundClass->ClassType);

											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}

										// EXPECT FOOT -> YEAR TO FAIL. 
										if (auto foundClass = tempScope->FindClass("year")) {
											try {
												
												auto returned_year = tree.Convert(returned_foot, foundClass->ClassType);
												EXPECT_EQ(true, false);
											} catch (...) {}
										}
									}

									std::vector<Any> params2{ Any(Units::foot(100.0)) };
									if (auto constructor = tempScope->FindFunction("meter", Function_Params{ params2 }, tree)) {
										auto returned_foot = call(constructor, params2, tree);

										if (auto foundClass = tempScope->FindClass("meter")) {
											auto returned = tree.Convert(returned_foot, foundClass->ClassType);

											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}

										if (auto foundClass = tempScope->FindClass("inch")) {
											auto returned = tree.Convert(returned_foot, foundClass->ClassType);

											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}

										if (auto foundClass = tempScope->FindClass("yard")) {
											auto returned = tree.Convert(returned_foot, foundClass->ClassType);

											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}

										// EXPECT METER -> YEAR TO FAIL. 
										if (auto foundClass = tempScope->FindClass("year")) {
											try {

												auto returned_year = tree.Convert(returned_foot, foundClass->ClassType);
												EXPECT_EQ(true, false);
											}
											catch (...) {}
										}
									}

									Any params3{ Units::foot(10.0) * Units::foot(10.0) };
									{
										if (auto foundClass = tempScope->FindClass("acre")) {
											auto returned = tree.Convert(params3, foundClass->ClassType);
											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}
									}
									Any params4{ Units::foot(10.0) * Units::foot(10.0) * Units::foot(10.0) };
									{
										if (auto foundClass = tempScope->FindClass("gallon")) {
											auto returned = tree.Convert(params4, foundClass->ClassType);
											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}
										if (auto foundClass = tempScope->FindClass("million_gallon")) {
											auto returned = tree.Convert(params4, foundClass->ClassType);
											printf(tree.Convert(returned, user_type<std::string>()).cast<std::string>());
										}
									}

								}
							}
						}
					}

					// add it to our script...
					imports.emplace("github//scriptLanguage.Units", global_scope2); // the import map guarrantees lifetime...
					global_scope->AddUsing(global_scope2); // ... while "using" allows our global to share their global's custom namespaces and objects
				}

				// make sure it works...
				{
				    global_scope->Print();

					Type_Converter_Tree tree;
					{
						std::vector<Any> params = { Any(100) };
						if (auto ptr = global_scope->FindFunction("std::to_string", params, tree)) {
							auto returned = call(ptr, params, tree);
							EXPECT_EQ(true, returned.IsTypeOf<std::string>());
							printf(returned.cast<std::string>());
						}
					}
					{
						std::vector<Any> params = { Any(std::string("TEST")) };
						if (auto ptr = global_scope->FindFunction("std::to_string", params, tree)) {
							auto returned = call(ptr, params, tree);
							EXPECT_EQ(true, returned.IsTypeOf<std::string>());
							printf(returned.cast<std::string>());
						}
					}
					{
						printf("PRINTING:");
						for (auto& child : global_scope->GetAvailableNamespaces()) {
							printf(child.first + "\t  ->  \t" + child.second.lock()->GetQualifiedNamespace());
						}
						printf("");

						if (auto ptr = global_scope->FindNamespace("std::string::impl")) {
							global_scope->Print();
						}

						if (auto ptr = global_scope->FindNamespace("fibers::string::impl2")) {
							EXPECT_EQ(false, true);
						}
					}
					{
						if (auto ClassPtr = global_scope->FindClass("fibers::UI::Map")) {
							global_scope->m_functions.emplace("=", make_callable([](Any& lhs, Any const& rhs) -> Any {
								fibers::DynamicObject& LHS = lhs.cast();
								fibers::DynamicObject& RHS = rhs.cast();
								LHS = RHS;
								return Any(lhs);
							}), scripting::Param_Types({ { "lhs", ClassPtr->ClassType }, { "rhs", ClassPtr->ClassType } }));

							std::vector<Any> params { Any(fibers::DynamicObject(ClassPtr->ClassType)), Any(fibers::DynamicObject(ClassPtr->ClassType)) };

							params[0].cast< fibers::DynamicObject >().m_objects["a"] = nullptr;
							params[1].cast< fibers::DynamicObject >().m_objects["b"] = nullptr;

							if (auto func = global_scope->FindFunction("=", params, tree)) {
								auto returned = call(func, params, tree);
								EXPECT_EQ(returned.Type(), ClassPtr->ClassType);
								EXPECT_EQ(std::string(returned.TypeName()), std::string("Map"));
								EXPECT_EQ(returned.cast< fibers::DynamicObject>().m_objects.at("b"), nullptr);
							}
						}
						if (auto scope = global_scope->FindNamespace("impl::fibers::UI::Map")) {
							EXPECT_EQ(false, true);
						}
					}

					{
						EXPECT_EQ(true, global_scope->AddUsing(global_scope->FindNamespace("Units")));

						if (auto ClassPtr = global_scope->FindClass("foot")) {
							if (auto func = ClassPtr->FindFunction("foot", {}, tree)) {
								auto returned = call(func, {}, tree);
								EXPECT_EQ(returned.Type(), user_type<Units::value>());

								std::cout << returned.cast<Units::value&>().Abbreviation() << std::endl;
							}
						}

						global_scope->Print();
					}

				}
			}
		}

		if (1) {
			using namespace scripting;
			auto printf = [](auto x) { std::cout << x << std::endl; };

			auto global_scope{ std::make_shared<Namespace>() }; // global should always be a Namespace
			global_scope->p_self = global_scope;


			{
				// start a new script, which has a single scope, importing the "std" namespace, with LOTS of namespaces and classes
				{
					auto script_scope{ std::make_shared<Scope>(global_scope) };
					script_scope->p_self = script_scope;
					{
						// that script imports the "std" namespace...
						{
							auto std_namespace{ std::make_shared<Namespace>(script_scope, "std") };
							std_namespace->p_self = std_namespace;
							script_scope->AddChild(std_namespace);

							// the "std" namespace imports the "string" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::string>(), std_namespace, "string") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "impl" namespace...								
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}

								Type_Converter_Tree
									m_typeConverters;

								// ... which has a couple constructor functions ... 
								string_namespace->m_functions.emplace("string", scripting::make_callable([](std::string const& x) -> std::string { return x; }), scripting::Param_Types({ { std::string("parent"), string_namespace->ClassType } }));
								string_namespace->m_functions.emplace("string", scripting::make_callable([](Any const& x) -> std::string {  return x.TypeName(); }), scripting::Param_Types({ { std::string("parent"), scripting::user_type<float>() } }));
								string_namespace->m_functions.emplace("string", scripting::make_callable([](Any const& x) -> std::string {  return x.TypeName(); }), scripting::Param_Types({ { std::string("parent"), scripting::user_type<int>() } }));
								string_namespace->m_functions.emplace("string", scripting::make_callable([](Any const& x) -> std::string {  return x.TypeName(); }), scripting::Param_Types({ { std::string("parent"), scripting::user_type<double>() } }));

								{
									std::vector<fibers::Any> params = { fibers::Any(std::string("TEST")) };
									auto function = string_namespace->m_functions("string", scripting::Function_Params(params));
									auto returned = scripting::call(function, params, m_typeConverters);
									EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
									std::cout << returned.cast<std::string>() << std::endl;
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(100) };
									auto function = string_namespace->m_functions("string", scripting::Function_Params(params));
									auto returned = scripting::call(function, params, m_typeConverters);
									EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
									std::cout << returned.cast<std::string>() << std::endl;
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(100.0f) };
									auto function = string_namespace->m_functions("string", scripting::Function_Params(params));
									auto returned = scripting::call(function, params, m_typeConverters);
									EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
									std::cout << returned.cast<std::string>() << std::endl;
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(100.0) };
									auto function = string_namespace->m_functions("string", scripting::Function_Params(params));
									auto returned = scripting::call(function, params, m_typeConverters);
									EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
									std::cout << returned.cast<std::string>() << std::endl;
								}

								{
									auto functionName = "Type";
									global_scope->m_functions.emplace(functionName, scripting::Param_Types({ { std::string("obj"), scripting::user_type<Any>() } }), scripting::make_callable(
										[](Any const& x) -> fibers::Type_Info { 
											if (auto p = x.Type().lock())
												return *p;
											else
												return fibers::user_type<void>();
										}
									), true);

									{
										std::vector<fibers::Any> params = { fibers::Any(100.0f) };
										if (auto ptr = string_namespace->FindFunction(functionName, params, m_typeConverters)) {
											auto returned = scripting::call(ptr, params, m_typeConverters);
											EXPECT_EQ(returned.IsTypeOf<fibers::Type_Info>(), true);
											std::cout << returned.cast<fibers::Type_Info>().name() << std::endl;
										}
									}
									
									{
										std::vector<fibers::Any> params = { fibers::Any(100.0) };
										if (auto ptr = string_namespace->FindFunction(functionName, params, m_typeConverters)) {
											auto returned = scripting::call(ptr, params, m_typeConverters);
											EXPECT_EQ(returned.IsTypeOf<fibers::Type_Info>(), true);
											std::cout << returned.cast<fibers::Type_Info>().name() << std::endl;
										}
									}

									{
										std::vector<fibers::Any> params = { fibers::Any(100) };
										if (auto ptr = string_namespace->FindFunction(functionName, params, m_typeConverters)) {
											auto returned = scripting::call(ptr, params, m_typeConverters);
											EXPECT_EQ(returned.IsTypeOf<fibers::Type_Info>(), true);
											std::cout << returned.cast<fibers::Type_Info>().name() << std::endl;
										}
									}

									{
										std::vector<fibers::Any> params = { fibers::Any(std::string("TEST"))};
										if (auto ptr = string_namespace->FindFunction(functionName, params, m_typeConverters)) {
											auto returned = scripting::call(ptr, params, m_typeConverters);
											EXPECT_EQ(returned.IsTypeOf<fibers::Type_Info>(), true);
											std::cout << returned.cast<fibers::Type_Info>().name() << std::endl;
										}
									}

								}
							}

							{
								Type_Converter_Tree
									m_typeConverters;

								auto functionName = "to_string";
								// variadic template, which takes any type. Up to the user to handle the various types, however. Will cache the result for faster retrieval.  
								std_namespace->m_functions.emplace(functionName, scripting::Param_Types({ { std::string("any"), scripting::user_type<Any>() } }), scripting::make_callable(
									[](Any const& x) -> std::string {
										if (auto p = x.Type().lock()) {
											return std::string("Retrieved type of: ") + p->name();
										}
										else {
											return std::string("Retrieved type of: ") + fibers::user_type<void>().name();
										}
									}
								), true);
								// specialized functions, for when the parameters exactly match. 
								std_namespace->m_functions.emplace(functionName, scripting::Param_Types({ { std::string("i"), scripting::user_type<int>() } }), scripting::make_callable(
									[](int const& x) -> std::string { return std::to_string(x); }
								), true);
								std_namespace->m_functions.emplace(functionName, scripting::Param_Types({ { std::string("i"), scripting::user_type<float>() } }), scripting::make_callable(
									[](float const& x) -> std::string { return std::to_string(x); }
								), true);
								std_namespace->m_functions.emplace(functionName, scripting::Param_Types({ { std::string("i"), scripting::user_type<std::string>() } }), scripting::make_callable(
									[](std::string const& x) -> std::string { return x; }
								), true);
								std_namespace->m_functions.emplace(functionName, scripting::Param_Types({ { std::string("i"), scripting::user_type<void>() } }), scripting::make_callable(
									[](Any const& x) -> std::string { return "NULL"; }
								), true);

								{
									std::vector<fibers::Any> params = { fibers::Any(100.0f) };
									if (auto ptr = std_namespace->FindFunction(functionName, params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(100.0) };
									if (auto ptr = std_namespace->FindFunction(functionName, params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(100) };
									if (auto ptr = std_namespace->FindFunction(functionName, params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(std::string("TEST")) };
									if (auto ptr = std_namespace->FindFunction(functionName, params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any() };
									if (auto ptr = std_namespace->FindFunction(functionName, params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(std::string_view()) };
									if (auto ptr = std_namespace->FindFunction(functionName, params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}
							}

							// function access with namespace specification
							{
								Type_Converter_Tree m_typeConverters;
								m_typeConverters.AddConverter<float, int>();
								m_typeConverters.AddConverter<long, int>();
								m_typeConverters.AddConverter<short, int>();
								m_typeConverters.AddConverter<float, double>();
								m_typeConverters.AddConverter<char, short>();
								{
									std::vector<fibers::Any> params = { fibers::Any('a') };
									if (auto ptr = std_namespace->FindFunction("std::to_string", params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}
								{
									std::vector<fibers::Any> params = { fibers::Any(5) };
									if (auto ptr = std_namespace->FindFunction("std::to_string", params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}
								{
									std::vector<fibers::Any> params = { fibers::Any(50l) };
									if (auto ptr = std_namespace->FindFunction("std::to_string", params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}
								{
									std::vector<fibers::Any> params = { fibers::Any(500.0) };
									if (auto ptr = std_namespace->FindFunction("std::to_string", params, m_typeConverters)) {
										auto returned = scripting::call(ptr, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}
							}

							// the "std" namespace imports the "map" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::map<std::string, fibers::Any>>(), std_namespace, "map") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}
							}

							// the "std" namespace imports the "set" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::set<std::string>>(), std_namespace, "set") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}
							}

							// the "std" namespace imports the "vector" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(fibers::user_type<std::vector<fibers::Any>>(), std_namespace, "vector") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}
							}
						}

						// that script imports the "fibers" namespace...
						{
							auto std_namespace{ std::make_shared<Namespace>(script_scope, "fibers") };
							std_namespace->p_self = std_namespace;
							script_scope->AddChild(std_namespace);

							// the "fibers" namespace imports the "Number" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(fibers::user_type<fibers::containers::number<double>>(), std_namespace, "Number") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "impl" namespace...								
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}
							}

							// the "fibers" namespace imports the "Pattern" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(fibers::user_type<fibers::containers::Pattern<double, double>>(), std_namespace, "Pattern") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}
							}

							// the "fibers" namespace imports the "Matrix" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(std_namespace, "Matrix") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}

								Type_Converter_Tree
									m_typeConverters;

								// ... which has a couple constructor functions ... 
								string_namespace->m_functions.emplace("0_param", scripting::make_callable([]() -> std::string {  return "0 params"; }), scripting::Param_Types());
								string_namespace->m_functions.emplace("1_param", scripting::Param_Types({ { std::string("a"), string_namespace->ClassType } }), 
									scripting::make_callable([](Any const& a) -> std::string {
										return a.TypeName();
									})
								);
								string_namespace->m_functions.emplace("+", scripting::Param_Types({ { std::string("a"), string_namespace->ClassType }, { std::string("b"), string_namespace->ClassType } }), scripting::make_callable([](Any const& a, Any const& b) -> std::string {
									return std::string(a.TypeName()) + " + " + b.TypeName();
									}));

								{
									std::vector<fibers::Any> params;
									auto function = string_namespace->FindFunction("0_param", params, m_typeConverters);
									if (function) {
										auto returned = scripting::call(function, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(fibers::DynamicObject(string_namespace->ClassType)) };
									auto function = string_namespace->FindFunction("1_param", params, m_typeConverters);
									if (function) {
										auto returned = scripting::call(function, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(fibers::DynamicObject(string_namespace->ClassType)), fibers::Any(fibers::DynamicObject(string_namespace->ClassType)) };
									auto function = string_namespace->FindFunction("+", params, m_typeConverters);
									if (function) {
										auto returned = scripting::call(function, params, m_typeConverters);
										EXPECT_EQ(returned.IsTypeOf<std::string>(), true);
										std::cout << returned.cast<std::string>() << std::endl;
									}
								}

								{
									std::vector<fibers::Any> params = { fibers::Any(fibers::DynamicObject()), fibers::Any(fibers::DynamicObject()) };
									auto function = string_namespace->FindFunction("+", params, m_typeConverters);
									if (function) {
										// SHOULD NOT RETURN A VALID FUNCTION
										EXPECT_EQ(false, true);
									}
								}
							}

							// the "fibers" namespace imports the "UI" namespace...
							{
								auto string_namespace{ std::make_shared<Namespace>(std_namespace, "UI") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// ... which imports the "Interactive" namespace...
								auto interactive_namespace{ std::make_shared<Class>(string_namespace, "Interactive") }; {
									interactive_namespace->p_self = interactive_namespace;
									string_namespace->AddChild(interactive_namespace);

									{
										auto impl_namespace{ std::make_shared<Namespace>(interactive_namespace, "InteractiveImpl") };
										impl_namespace->p_self = impl_namespace;
										interactive_namespace->AddChild(impl_namespace);
									}
								}

								// ... which imports the "Map" namespace...
								{
									auto impl_namespace{ std::make_shared<Class>(string_namespace, "Map", interactive_namespace) };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);

									{
										auto impl_namespace2{ std::make_shared<Namespace>(impl_namespace, "MapImpl") };
										impl_namespace2->p_self = impl_namespace2;
										impl_namespace->AddChild(impl_namespace2);
									}

									{
										auto impl_namespace2{ std::make_shared<Namespace>(impl_namespace, "Text") };
										impl_namespace2->p_self = impl_namespace2;
										impl_namespace->AddChild(impl_namespace2);
									}
								}

								// ... which imports the "Button" namespace...
								{
									auto impl_namespace{ std::make_shared<Class>(string_namespace, "Button", interactive_namespace) };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}

								// ... which imports the "Grid" namespace...
								{
									auto impl_namespace{ std::make_shared<Class>(string_namespace, "Grid") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}

								// ... which imports the "Plot" namespace...
								{
									auto impl_namespace{ std::make_shared<Class>(string_namespace, "Plot") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}

								// ... which imports the "Text" namespace...
								{
									auto impl_namespace{ std::make_shared<Class>(string_namespace, "Text") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}

								// ... which imports the "WebPage" namespace...
								{
									auto impl_namespace{ std::make_shared<Class>(string_namespace, "WebPage") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}

							}
						}
					}


					printf("PRINTING:");
					for (auto& child : script_scope->GetAvailableNamespaces()) {
						printf(child.first + "\t  ->  \t" + child.second.lock()->GetQualifiedNamespace());
					}
					printf("");

					// TODO: rank these scopes based on distance from the target scope! 

					printf("PRINTING OBJECT ACCESS FROM TOP:");
					for (auto& child : script_scope->GetScopesForObjectSearch()) {
						if (auto ptr = child) {
							printf(ptr->GetQualifiedNamespace());
						}
					}
					printf("");

					if (auto foundScope = script_scope->FindNamespace("std")) {
						printf("PRINTING OBJECT ACCESS FROM STD:");
						for (auto& child : foundScope->GetScopesForObjectSearch()) {
							if (auto ptr = child) {
								printf(ptr->GetQualifiedNamespace());
							}
						}
						printf("");
					}

					if (auto foundScope = script_scope->FindNamespace("std")) {
						EXPECT_EQ(true, script_scope->AddUsing(std::dynamic_pointer_cast<Namespace>(foundScope)));

						printf("PRINTING OBJECT ACCESS FROM fibers::UI::Map::InteractiveImpl:");
						for (auto& child : foundScope->GetScopesForObjectSearch()) {
							if (auto ptr = child) {
								printf(ptr->GetQualifiedNamespace());
							}
						}
						printf("");
					}

					// lets say we now make a scope, wherein we are "using" the UI namespace...
					{
						auto script_scope2{ std::make_shared<Scope>(script_scope) };
						script_scope2->p_self = script_scope2;

						// that script then declares it is "using" the "fibers::UI" namespace...
						if (auto foundScope = script_scope2->FindNamespace("fibers::UI")){
							EXPECT_EQ(true, script_scope2->AddUsing(std::dynamic_pointer_cast<Namespace>(foundScope)));
						}

						printf("PRINTING 2:");
						for (auto& child : script_scope2->GetAvailableNamespaces()) {
							printf(child.first + "\t  ->  \t" + child.second.lock()->GetQualifiedNamespace());
						}
						printf("");
					}

					// access it...
					{


						if (auto foundScope = script_scope->FindNamespace("std")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("std::string")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("std::string::impl")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("std::vector")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("std::map")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("fibers::Pattern")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("fibers::UI::Map::InteractiveImpl")) {}
						else { EXPECT_EQ(true, false); }

						if (auto foundScope = script_scope->FindNamespace("::std::")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("::std::string::")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("::std::string::impl::")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("::std::vector::")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("::std::map::")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("::fibers::Pattern::")) {}
						else { EXPECT_EQ(true, false); }
						if (auto foundScope = script_scope->FindNamespace("::fibers::UI::Map::InteractiveImpl::")) {}
						else { EXPECT_EQ(true, false); }

						if (auto foundScope = script_scope->FindNamespace("fibx")) { EXPECT_EQ(true, false); }
						else {}
						if (auto foundScope = script_scope->FindNamespace("std::string::string_view")) { EXPECT_EQ(true, false); }
						else {}
						if (auto foundScope = script_scope->FindNamespace("std::string_view")) { EXPECT_EQ(true, false); }
						else {}
						
					}

				}

				// start a new script, which has a single scope, importing the "std" namespace, and accessing it.
				{
					auto script_scope{ std::make_shared<Scope>(global_scope) };
					script_scope->p_self = script_scope;
					{
						// that script imports the "std" namespace...
						{
							auto std_namespace{ std::make_shared<Namespace>(script_scope, "std") };
							std_namespace->p_self = std_namespace;
							script_scope->AddChild(std_namespace);

							// the "std" namespace imports the "string" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(std_namespace, "string") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// the "string" namespace imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}
							}
						}
					}
					printf("PRINTING:");
					for (auto& child : script_scope->GetAvailableNamespaces()) {
						printf(child.first + "\t  ->  \t" + child.second.lock()->GetQualifiedNamespace());
					}
					printf("");
				}
				global_scope->Print(); // ::
				printf("");

				// start a new script, which has multiple scope access it in parallel, and each importing the "std" namespace.
				{
					fibers::parallel::For(0, 100, [&](int scopeN) {
						auto script_scope{ std::make_shared<Scope>(global_scope) };
						script_scope->p_self = script_scope;
						{
							// that script imports the "std" namespace...
							{
								auto std_namespace{ std::make_shared<Namespace>(script_scope, "std") };
								std_namespace->p_self = std_namespace;
								script_scope->AddChild(std_namespace);

								// the "std" namespace imports the "string" namespace...
								{
									auto string_namespace{ std::make_shared<Class>(std_namespace, "string") };
									string_namespace->p_self = string_namespace;
									std_namespace->AddChild(string_namespace);

									// the "string" namespace imports the "impl" namespace...
									{
										auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
										impl_namespace->p_self = impl_namespace;
										string_namespace->AddChild(impl_namespace);
									}
								}
							}
						}
						// script_scope->Print(); // :: -> std -> string -> impl
						});
				}
				global_scope->Print(); // ::
				printf("");

				// start a new script, import the "std" namespace, and then multiple scope access it in parallel.
				{
					auto script_scope{ std::make_shared<Scope>(global_scope) };
					script_scope->p_self = script_scope;

					// that script imports the "std" namespace...
					{
						auto std_namespace{ std::make_shared<Namespace>(script_scope, "std") };
						std_namespace->p_self = std_namespace;
						script_scope->AddChild(std_namespace);

						// the "std" namespace imports the "string" namespace...
						{
							auto string_namespace{ std::make_shared<Class>(std_namespace, "string") };
							string_namespace->p_self = string_namespace;
							std_namespace->AddChild(string_namespace);

							// the "string" namespace imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
								impl_namespace->p_self = impl_namespace;
								string_namespace->AddChild(impl_namespace);
							}
						}
					}

					fibers::parallel::For(0, 100, [&](int scopeN) {
						auto script_scope2{ std::make_shared<Scope>(script_scope) };
						script_scope2->p_self = script_scope2;
						
						// script_scope->Print(); // :: -> std -> string -> impl
					});
				}
				global_scope->Print(); // ::
				printf("");

				// A seperate "thing" to manage imports is necessary, since their lifetimes must be guarranteed outside of the script (also to help reduce import re-compiling)
				fibers::containers::Map<std::string, std::shared_ptr<Namespace>> imports;

				// DEMONSTRATES HOW TO MAKE A INDEPENDANT IMPORT (e.g. from github), store it in its own, independant scope (global_scope2), and "add it" to another running scope without changing it.
				{
					auto script_scope{ std::make_shared<Scope>(global_scope) };
					script_scope->p_self = script_scope;

					{
						// DEMONSTRATES HOW TO MAKE A INDEPENDANT IMPORT (e.g. from github), store it in its own, independant scope (global_scope2), and "add it" to another running scope without changing it.

						auto global_scope2{ std::make_shared<Namespace>() }; // global should always be a Namespace
						global_scope2->p_self = global_scope2;

						{
							auto std_namespace{ std::make_shared<Namespace>(global_scope2, "std") }; // global should always be a Namespace
							std_namespace->p_self = std_namespace;
							global_scope2->AddChild(std_namespace);

							// the "std" namespace imports the "string" namespace...
							{
								auto string_namespace{ std::make_shared<Class>(std_namespace, "string") };
								string_namespace->p_self = string_namespace;
								std_namespace->AddChild(string_namespace);

								// the "string" namespace imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
									impl_namespace->p_self = impl_namespace;
									string_namespace->AddChild(impl_namespace);
								}
							}

						}

						// add it to our script...
						imports.emplace("github//scriptLanguage.std", global_scope2); // the import map guarrantees lifetime...
						script_scope->AddUsing(global_scope2); // ... while "using" allows our global to share their global's custom namespaces and objects
					}


					printf("PRINTING:");
					for (auto& child : script_scope->GetAvailableNamespaces()) {
						printf(child.first + "\t  ->  \t" + child.second.lock()->GetQualifiedNamespace());
					}
					printf("");

					


				}
				global_scope->Print(); // ::
				printf("");
			}






















			EXPECT_EQ(true, global_scope->AddObject("apple", std::make_shared<fibers::Any>(100)));
			{
				if (auto ptr = global_scope->FindObject("apple")) {}
				else { EXPECT_EQ(true, false); }
			}
			{
				auto script_scope{ std::make_shared<Scope>(global_scope) };
				script_scope->p_self = script_scope;

				if (auto ptr = script_scope->FindObject("apple")) {}
				else { EXPECT_EQ(true, false); }

				if (auto ptr = script_scope->FindObject("::apple")) {}
				else { EXPECT_EQ(true, false); }
			}
			{
				auto script_scope{ std::make_shared<Scope>(global_scope) };
				script_scope->p_self = script_scope;

				// that script imports the "std" namespace...
				{
					auto std_namespace{ std::make_shared<Namespace>(script_scope, "std") };
					std_namespace->p_self = std_namespace;
					script_scope->AddChild(std_namespace);

					// the "std" namespace imports the "string" namespace...
					{
						auto string_namespace{ std::make_shared<Class>(std_namespace, "string") };
						string_namespace->p_self = string_namespace;
						std_namespace->AddChild(string_namespace);

						// the "string" namespace imports the "impl" namespace...
						{
							auto impl_namespace{ std::make_shared<Namespace>(string_namespace, "impl") };
							impl_namespace->p_self = impl_namespace;
							string_namespace->AddChild(impl_namespace);
						}

						// the "string" namespace declares an object...
						EXPECT_EQ(true, string_namespace->AddObject("npos", std::make_shared<fibers::Any>(std::string::npos)));
					}
				}

				if (auto ptr = script_scope->FindObject("std::string::npos")) {} else { EXPECT_EQ(true, false); }


			}








		}
#endif

#if 0
		if (1) {
			using namespace scripting;
			auto printf = [](auto x) { std::cout << x << std::endl; };

			std::shared_ptr<Namespace> global_scope = std::make_shared<Namespace>();

			// start a new script, which has a single scope, importing the "std" namespace.
			{
				auto script_scope{ std::make_shared<Scope>(global_scope.get()) };
				{
					// that script imports the "std" namespace...
					{
						auto std_namespace{ std::make_shared<Namespace>(&script_scope, "std") };
						script_scope.AddChild(std_namespace);

						// the "std" namespace imports the "string" namespace...
						{
							auto string_namespace{ std::make_shared<Namespace>(std_namespace.get(), "string") };
							std_namespace->AddChild(string_namespace);

							// the "string" namespace imports the "impl" namespace...
							{
								auto impl_namespace{ std::make_shared<Namespace>(string_namespace.get(), "impl") };
								string_namespace->AddChild(impl_namespace);
							}
						}
					}

					// that script then calls the "std::string::impl" namespace directly...
					{
						Scope* foundScope{ nullptr };
						if (script_scope.TryFindScope(foundScope, "std::string::impl")) {
							// found it 
							printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
						}
						else if (foundScope) {
							// found where to put it, if not found directly
							printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
						}
						else {
							printf("Failed to find it");
						}
					}

					// that script then declares it is "using" the "std" namespace...
					{
						Scope* foundScope{ nullptr };
						if (script_scope.TryFindScope(foundScope, "std")) {
							// found it 
							script_scope.AddUsing(foundScope);
						}
					}

					// that script then calls the "string" namespace directly...
					{
						Scope* foundScope{ nullptr };
						if (script_scope.TryFindScope(foundScope, "string")) {
							// found it 
							printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
						}
						else if (foundScope) {
							// found where to put it, if not found directly
							printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
						}
						else {
							printf("Failed to find it");
						}
					}
				}
				script_scope.Print(); // :: -> std -> string -> impl
			}
			global_scope->Print(); // ::
			printf("");

			// then, start another script, which has multiple scopes, each importing the "std" namespace
			{
				auto script_scope{ std::make_shared<Scope>(global_scope.get()) };
				{
					Scope script_inner_scope_A(&script_scope);
					{
						// that script imports the "std" namespace...
						{
							auto std_namespace{ std::make_shared<Namespace>(&script_inner_scope_A, "std") };
							script_inner_scope_A.AddChild(std_namespace);

							// the "std" namespace imports the "string" namespace...
							{
								auto string_namespace{ std::make_shared<Namespace>(std_namespace.get(), "string") };
								std_namespace->AddChild(string_namespace);

								// the "string" namespace imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace.get(), "impl") };
									string_namespace->AddChild(impl_namespace);
								}
							}
						}

						// that script then declares it is "using" the "std" namespace...
						{
							Scope* foundScope{ nullptr };
							if (script_inner_scope_A.TryFindScope(foundScope, "std")) {
								// found it 
								script_inner_scope_A.AddUsing(foundScope);
							}
						}

						// that script then calls the "string" namespace directly...
						{
							Scope* foundScope{ nullptr };
							if (script_inner_scope_A.TryFindScope(foundScope, "string")) {
								// found it 
								printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
							}
							else if (foundScope) {
								// found where to put it, if not found directly
								printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
							}
							else {
								printf("Failed to find it");
							}
						}
					}

					Scope script_inner_scope_B(&script_scope);
					{
						// that script imports the "std" namespace...
						{
							auto std_namespace{ std::make_shared<Namespace>(&script_inner_scope_B, "std") };
							script_inner_scope_B.AddChild(std_namespace);

							// the "std" namespace imports the "string" namespace...
							{
								auto string_namespace{ std::make_shared<Namespace>(std_namespace.get(), "string") };
								std_namespace->AddChild(string_namespace);

								// the "string" namespace imports the "impl" namespace...
								{
									auto impl_namespace{ std::make_shared<Namespace>(string_namespace.get(), "impl") };
									string_namespace->AddChild(impl_namespace);
								}
							}
						}

						// that script then declares it is "using" the "std" namespace...
						{
							Scope* foundScope{ nullptr };
							if (script_inner_scope_B.TryFindScope(foundScope, "std")) {
								// found it 
								script_inner_scope_B.AddUsing(foundScope);
							}
						}

						// that script then calls the "string" namespace directly...
						{
							Scope* foundScope{ nullptr };
							if (script_inner_scope_B.TryFindScope(foundScope, "string")) {
								// found it 
								printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
							}
							else if (foundScope) {
								// found where to put it, if not found directly
								printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
							}
							else {
								printf("Failed to find it");
							}
						}
					}

					script_scope.Print(); // ::
				}
				script_scope.Print(); // ::
			}
			global_scope->Print(); // ::
			printf("");

			// then, start another script, which has a single scope that declares a new namespace
			{
				auto script_scope{ std::make_shared<Scope>(global_scope.get()) };
				{
					Scope script_inner_scope_A(&script_scope);
					{
						// that script declares a new namespace "std::map"
						{
							Impl::FindOrMakeNamespace(&script_inner_scope_A, "std::map");
						}

						// that script then calls the "std::map" namespace directly...
						{
							Scope* foundScope{ nullptr };
							if (script_inner_scope_A.TryFindScope(foundScope, "std::map")) {
								// found it 
								printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
							}
							else if (foundScope) {
								// found where to put it, if not found directly
								printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
							}
							else {
								printf("Failed to find it");
							}
						}
					}

					script_scope.Print(); // ::
				}
				script_scope.Print(); // ::
			}
			global_scope->Print(); // ::
			printf("");

			// then, start another script, which has a multiple scopes that declares a new namespaces
			{
				auto script_scope{ std::make_shared<Scope>(global_scope.get()) };
				{
					Scope script_inner_scope_A(&script_scope);
					{
						// that script declares a new namespace "std::map"
						{
							Impl::FindOrMakeNamespace(&script_inner_scope_A, "std::map");
						}

						// that script then calls the "std::map" namespace directly...
						{
							Scope* foundScope{ nullptr };
							if (script_inner_scope_A.TryFindScope(foundScope, "std::map")) {
								// found it 
								printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
							}
							else if (foundScope) {
								// found where to put it, if not found directly
								printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
							}
							else {
								printf("Failed to find it");
							}
						}
					}

					global_scope.Print(); // ::

					Scope script_inner_scope_B(&script_scope);
					{
						// that script declares a new namespace "std::map"
						{
							Impl::FindOrMakeNamespace(&script_inner_scope_B, "::std::map::");
						}

						// that script then calls the "std::map" namespace directly...
						{
							Scope* foundScope{ nullptr };
							if (script_inner_scope_B.TryFindScope(foundScope, "std::map")) {
								// found it 
								printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
							}
							else if (foundScope) {
								// found where to put it, if not found directly
								printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
							}
							else {
								printf("Failed to find it");
							}
						}
					}

					script_inner_scope_B.Print(); // :: 

					global_scope.Print(); // :: -> std -> map

					script_scope.Print(); // ::
				}
				script_scope.Print(); // ::
			}
			global_scope->Print(); // ::
			printf("");

			// In the ACTUAL global scope, we will declare and import the "std" namespace, and declare we are "using" it.
			{
				auto std_namespace{ std::make_shared<Namespace>(&global_scope, "std") };
				global_scope->AddChild(std_namespace);

				// the "std" namespace imports the "string" namespace...
				{
					auto string_namespace{ std::make_shared<Namespace>(std_namespace.get(), "string") };
					std_namespace->AddChild(string_namespace);

					// the "string" namespace imports the "impl" namespace...
					{
						auto impl_namespace{ std::make_shared<Namespace>(string_namespace.get(), "impl") };
						string_namespace->AddChild(impl_namespace);
					}
				}

				Scope* foundScope{ nullptr };
				if (global_scope->TryFindScope(foundScope, "std")) {
					// found it 
					global_scope->AddUsing(foundScope);
				}
			}
			global_scope->Print(); // :: -> std -> string -> impl
			printf("");

			// we may now start a new script, which can call "std::string" with just "string" very easily. 
			{
				auto script_scope{ std::make_shared<Scope>(global_scope.get()) };
				{
					// that script then calls the "string" namespace directly...
					{
						Scope* foundScope{ nullptr };
						if (script_scope.TryFindScope(foundScope, "string::impl")) {
							// found it 
							printf("Found it, with qualified namespace of: " + foundScope->GetQualifiedNamespace());
						}
						else if (foundScope) {
							// found where to put it, if not found directly
							printf("Failed to find it, but suggested to put it here: " + foundScope->GetQualifiedNamespace());
						}
						else {
							printf("Failed to find it");
						}
					}
				}
				script_scope.Print();
			}
			global_scope->Print(); // :: -> std -> string -> impl
			printf("");



		}



		if (1) {
			using namespace scripting;

			auto printf = [](auto x) { std::cout << x << std::endl; };

			Namespace global_scope(nullptr, "");
			auto std_namespace{ std::make_shared<Namespace>(&global_scope, "std") };
			global_scope.AddChild(std_namespace);
			defer(global_scope.p_children.erase(std_namespace->GetName()));

			auto std_string_namespace{ std::make_shared<Class>(&*std_namespace, "string") };
			std_namespace->AddChild(std_string_namespace);
			defer(std_namespace->p_children.erase(std_string_namespace->GetName()));

			Scope script_scope(&*std_namespace);
			Scope script_scope2(&script_scope);
			Namespace script_scope3(&script_scope2, "TEST"); // this namespace is temporary, since it's within a Scope. It can still find parent namespaces, but parent's cannot find it. 

			printf(global_scope.GetQualifiedNamespace()); // ::

			printf(std_namespace->GetQualifiedNamespace()); // ::std::

			printf(std_string_namespace->GetQualifiedNamespace()); // ::std::string::

			printf(script_scope.GetQualifiedNamespace()); // ::std::

			printf(script_scope2.GetQualifiedNamespace()); // ::std::

			printf(script_scope3.GetQualifiedNamespace()); // ::std::TEST::

			int progress = 0;
			Scope* foundScope{ nullptr };
			if (1) {
				printf(""); printf(progress++); // 0
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::string::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::string::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 1
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 2
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 3
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::fibers::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 4
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::fibers::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::std::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 5
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::string::fibers::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::std::string::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 6
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "std")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 7
				foundScope = nullptr; // 
				if (script_scope3.TryFindScope(foundScope, "string")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::string::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 8
				foundScope = nullptr; // expect success
				if (script_scope3.TryFindScope(foundScope, "fibers")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::std::TEST::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 9
				foundScope = nullptr; // expect success
				if (script_scope3.TryFindScope(foundScope, "TEST")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::TEST::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 10
				foundScope = nullptr; // expect failure, since 'string' is actually in ::std::string, which cannot be expected to have been found without a "using" statement?
				if (global_scope.TryFindScope(foundScope, "string")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 11
				foundScope = nullptr; // expect failure, since TEST is actually in ::std::TEST, which cannot be expected to have been found without a "using" statement?
				if (global_scope.TryFindScope(foundScope, "TEST")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find");
				}
			}
			// using statement
			if (1) {
				progress = 0;
				script_scope3.AddUsing(std_namespace.get());

				printf(""); printf(progress++); // 0
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::string::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::string::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 1
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 2
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 3
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::fibers::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 4
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::fibers::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::std::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 5
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "::std::string::fibers::")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::std::string::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 6
				foundScope = nullptr;
				if (script_scope3.TryFindScope(foundScope, "std")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 7
				foundScope = nullptr; // 
				if (script_scope3.TryFindScope(foundScope, "string")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::string::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 8
				foundScope = nullptr; // expect success
				if (script_scope3.TryFindScope(foundScope, "fibers")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::std::TEST::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 9
				foundScope = nullptr; // expect success
				if (script_scope3.TryFindScope(foundScope, "TEST")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace()); // ::std::TEST::
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace());
					else printf("Failed to find");
				}

				global_scope.AddUsing(std_namespace.get());

				printf(""); printf(progress++); // 10
				foundScope = nullptr; // expect failure, since 'string' is actually in ::std::string, which cannot be expected to have been found without a "using" statement?
				if (global_scope.TryFindScope(foundScope, "string")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find");
				}

				printf(""); printf(progress++); // 11
				foundScope = nullptr; // expect failure, since TEST is actually in ::std::TEST, which cannot be expected to have been found without a "using" statement?
				if (global_scope.TryFindScope(foundScope, "TEST")) {
					if (foundScope) printf(foundScope->GetQualifiedNamespace());
					else printf("Failed to find, but reported success??!?!?!");
				}
				else {
					if (foundScope) printf("Failed to find, but recommend placement at " + foundScope->GetQualifiedNamespace()); // ::
					else printf("Failed to find");
				}

			}

			global_scope.Print();

			if (1) {
				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "std");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "string"); 
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "fiber::parallel::impl");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "::std::string::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "TEST");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "::string::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "::std::string::impl::test::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "::impl::impl::impl::impl::impl::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");
			}
			if (1) {
				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "std");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "string"); // failed to find the namespace ::std::string::, and it made a new one.
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "fiber::parallel::impl");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "::std::string::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "TEST");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "::string::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&script_scope3), "::std::string::impl::test::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "::impl::impl::impl::impl::impl::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");
			}
			if (1) {
				Scope test_scope(&global_scope);

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "std");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "string"); // failed to find the namespace ::std::string::, and it made a new one.
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "fiber::parallel::impl");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "::std::string::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "TEST");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "::string::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&test_scope), "::std::string::impl::test::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");

				foundScope = Impl::FindOrMakeNamespace(dynamic_cast<Scope*>(&global_scope), "::impl::impl::impl::impl::impl::");
				if (foundScope) printf(foundScope->GetQualifiedNamespace());
				else printf("Failed to find");
			}

			global_scope.Print();

		}

#endif

#if 0
		if (1) {
			scripting::Namespace global_namespace("global");
			{
				fibers::Any temp;

				global_namespace.AddObject("x", stackThing("x", 100));

				auto newScope1 = global_namespace.push_scope();
				defer(global_namespace.pop_scope()); // automatically pop's the previous scope
				newScope1->AddObject("y", stackThing("y", 100));

				auto newScope2 = global_namespace.push_scope();
				defer(global_namespace.pop_scope()); // automatically pop's the previous scope
				newScope2->AddObject("z", stackThing("z", 100));

				EXPECT_EQ(global_namespace.NumChildrenScopes(), 2);

				auto namedScope1 = global_namespace.push_scope("fibers");
				defer(global_namespace.pop_scope()); // automatically pop's the previous scope
				namedScope1->AddObject("w", stackThing("w", 100));

				EXPECT_EQ(global_namespace.NumChildrenScopes(), 3);

				EXPECT_EQ(global_namespace.TryFindObject("x", temp), true); // found in self
				EXPECT_EQ(newScope1->TryFindObject("x", temp), true); // found in parent
				EXPECT_EQ(newScope2->TryFindObject("x", temp), true); // found in parent
				EXPECT_EQ(namedScope1->TryFindObject("x", temp), true); // found in parent

				EXPECT_EQ(global_namespace.TryFindObject("y", temp), false); // should not be found
				EXPECT_EQ(newScope1->TryFindObject("y", temp), true); // found in self
				EXPECT_EQ(newScope2->TryFindObject("y", temp), false); // should not be found
				EXPECT_EQ(namedScope1->TryFindObject("y", temp), false); // should not be found

				std::cout << newScope1->QualifiedName() << std::endl; // global
				std::cout << namedScope1->QualifiedName() << std::endl; // global::fibers
			}
			EXPECT_EQ(global_namespace.NumChildrenScopes(), 0);

		}
#endif

#if 0
		// Type Conversions
		if (1) {
			// Static
			if (1) {
				scripting::details::Static_Type_Conversion_Impl<int, float> converter;

				EXPECT_EQ(converter.to(), fibers::user_type<float>());
				EXPECT_EQ(converter.from(), fibers::user_type<int>());

				EXPECT_EQ(converter.bidir(), true);
				EXPECT_EQ(converter.polymorphic(), false);
				EXPECT_EQ(converter.convert(1).IsTypeOf<float>(), true);
				EXPECT_EQ(converter.convert(1).cast<float>() == 1.0f, true);
				EXPECT_EQ(converter.convert_down(100.0f).IsTypeOf<int>(), true);
				EXPECT_EQ(converter.convert_down(100.0f).cast<int>() == 100, true);
			}

			// Dynamic
			if (1) {
				scripting::details::Dynamic_Type_Conversion_Impl< Units::foot, Units::value> converter;

				EXPECT_EQ(converter.to(), fibers::user_type<Units::value>());
				EXPECT_EQ(converter.from(), fibers::user_type<Units::foot>());

				fibers::Any Child = Units::foot(100);
				EXPECT_EQ(converter.convert(Child).cast<Units::value>()(), 100.0f);
				EXPECT_EQ(converter.convert(Child).cast<Units::value>().Abbreviation(), "ft");
				try {
					converter.convert_down(Units::value(1));
					EXPECT_EQ(true, false);
				} catch (...) {}
				EXPECT_EQ(converter.convert(Child).IsTypeOf<Units::value>(), true);
			}

			// Custom
			if (1) {
				auto converter{ scripting::details::Custom_Type_Conversion_Impl([](Units::foot const& a)->Units::value {
					return a;
				})};

				EXPECT_EQ(converter.to(), fibers::user_type<Units::value>());
				EXPECT_EQ(converter.from(), fibers::user_type<Units::foot>());

				fibers::Any Child = Units::foot(100);
				EXPECT_EQ(converter.convert(Child).IsTypeOf<Units::value>(), true);
				EXPECT_EQ(converter.convert(Child).cast<Units::value>()(), 100.0f);
				EXPECT_EQ(converter.convert(Child).cast<Units::value>().Abbreviation(), "ft");
				try {
					converter.convert_down(Units::value(1));
					EXPECT_EQ(true, false);
				}
				catch (...) {}
			}

			// Custom 2
			if (1) {
				auto converter{ scripting::details::Custom_Type_Conversion_Impl([](fibers::Any const& a)-> int {
					return 100; // always returns 100
				}) };

				EXPECT_EQ(converter.to(), fibers::user_type<int>());
				EXPECT_EQ(converter.from(), fibers::user_type<fibers::Any>());

				fibers::Any Child = Units::foot(100);
				EXPECT_EQ(converter.convert(Child).IsTypeOf<int>(), true);
				EXPECT_EQ(converter.convert(Child).cast<int>(), 100);
				try {
					converter.convert_down(Child);
					EXPECT_EQ(true, false);
				}
				catch (...) {}
			}

			// Type_Converter_Tree
			if (1) {
				scripting::Type_Converter_Tree tree;

				fibers::Any result;
				EXPECT_EQ(true, tree.TryConvert(1.0f, scripting::user_type<float>(), result)); // no conversion = successful, no conversion necessary.
				EXPECT_EQ(false, tree.TryConvert(1, scripting::user_type<float>(), result)); // no conversion provided, so this should fail. 

				// Numbers conversions
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, bool>())); // int -> bool, bool -> int
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, float>())); 
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, uint64_t>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, unsigned long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, long long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, long double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, uint64_t>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, unsigned long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, long long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, long double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, uint64_t>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, unsigned long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, long long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, long double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<uint64_t, long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<uint64_t, unsigned long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<uint64_t, long long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<uint64_t, long double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<long, unsigned long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<long, long long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<long, long double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<unsigned long, long long>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<unsigned long, long double>()));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<long long, long double>()));
				// Units conversions
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, Units::value>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, Units::value>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, Units::value>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<uint64_t, Units::value>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<long double, Units::value>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::foot, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::meter, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::inch, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::yard, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::millimeter, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::second, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::minute, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::hour, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::day, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::year, Units::value>())); // polymorphic
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<DateTime, Units::second>())); // static

				EXPECT_EQ(true, tree.TryConvert(1, scripting::user_type<bool>(), result));
				EXPECT_EQ(true, tree.TryConvert(100, scripting::user_type<Units::value>(), result));
				EXPECT_EQ("100", result.cast< Units::value >().ToString());

				EXPECT_EQ(true, tree.TryConvert(100, scripting::user_type<Units::foot>(), result));
				EXPECT_EQ("100 ft", result.cast< Units::foot >().ToString());

				{
					using namespace literals;
					EXPECT_EQ(true, tree.TryConvert(1726254751_s, scripting::user_type<DateTime>(), result));
				}
				{
					using namespace literals;
					EXPECT_EQ(false, tree.TryConvert(1726254751_ft, scripting::user_type<DateTime>(), result));
				}

				EXPECT_EQ(true, tree.TryConvert(1, scripting::user_type<Units::second>(), result));

				// update the tree...
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](std::string_view const& r) -> std::string { return r.data(); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](std::string const& r) -> std::string_view { return std::string_view(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](std::string const& r) -> float { return atof(r.c_str()); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](int r) -> std::string { return std::to_string(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](float r) -> std::string { return std::to_string(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](double r) -> std::string { return std::to_string(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](long r) -> std::string { return std::to_string(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](long long r) -> std::string { return std::to_string(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](long double r) -> std::string { return std::to_string(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](uint64_t r) -> std::string { return std::to_string(r); })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](Units::value const& r) -> std::string { return r.ToString(); })));

				// ... which will now re-evaluate the conversion and will use the faster conversion from now on. 
				EXPECT_EQ(true, tree.TryConvert(1, scripting::user_type<Units::second>(), result));
				EXPECT_EQ(true, tree.TryConvert(1, scripting::user_type<std::string_view>(), result));
				EXPECT_EQ(true, tree.TryConvert(1.0, scripting::user_type<std::string_view>(), result));
				EXPECT_EQ(true, tree.TryConvert(Units::foot(1), scripting::user_type<std::string_view>(), result));

#if 0
				if (1) {
					std::vector<fibers::Any> inputs{ fibers::Any(10) };
					scripting::Param_Types function_required_params({ {"int", scripting::user_type<int>() } });
					auto converted_inputs = function_required_params.convert(scripting::Function_Params{ inputs }, tree);

					for (int i = 0; i < function_required_params.size(); i++) {
						EXPECT_EQ(true, converted_inputs[i].IsTypeOf(function_required_params[i].second));
					}
				}
				if (1) {
					std::vector<fibers::Any> inputs{ fibers::Any(10.0f), fibers::Any(10.0) };
					scripting::Param_Types function_required_params({ {"int1", scripting::user_type<int>() }, {"int2", scripting::user_type<int>() } });
					auto converted_inputs = function_required_params.convert(scripting::Function_Params{ inputs }, tree);

					for (int i = 0; i < function_required_params.size(); i++) {
						EXPECT_EQ(true, converted_inputs[i].IsTypeOf(function_required_params[i].second));
					}
				}
				if (1) {
					std::vector<fibers::Any> inputs{ fibers::Any(10.0f), fibers::Any(10.0), fibers::Any(10) };
					scripting::Param_Types function_required_params({ {"std::string_view", scripting::user_type<std::string_view>() }, {"std::string_view", scripting::user_type<std::string_view>() }, {"std::string_view", scripting::user_type<std::string_view>() } });
					auto converted_inputs = function_required_params.convert(scripting::Function_Params{ inputs }, tree);
				}

				if (1) {
					scripting::Param_Types function1({ {"double", scripting::user_type<double>() }, {"double", scripting::user_type<double>() }, {"double", scripting::user_type<double>() } });
					scripting::Param_Types function2({ {"float", scripting::user_type<float>() }, {"float", scripting::user_type<float>() }, {"float", scripting::user_type<float>() } });
					scripting::Param_Types function3({ {"double", scripting::user_type<double>() }, {"int", scripting::user_type<int>() }, {"float", scripting::user_type<float>() } });
					scripting::Param_Types function4({ {"seconds", scripting::user_type<Units::second>() }, {"seconds", scripting::user_type<Units::second>() }, {"seconds", scripting::user_type<Units::second>() } });

					std::vector<fibers::Any> inputs{ fibers::Any(10ull), fibers::Any(10ull), fibers::Any(10ull) };

					float cost1 = function1.conversion_cost(scripting::Function_Params{ inputs }, tree);
					float cost2 = function2.conversion_cost(scripting::Function_Params{ inputs }, tree);
					float cost3 = function3.conversion_cost(scripting::Function_Params{ inputs }, tree);
					float cost4 = function4.conversion_cost(scripting::Function_Params{ inputs }, tree);

					std::cout << Units::printf("Cost1: %f\nCost2: %f\nCost3: %f\nCost4: %f\n\n", cost1, cost2, cost3, cost4);

					float minCost = std::min({ cost1, cost2, cost3, cost4 });

					std::vector<fibers::Any> converted_inputs;
					if (cost1 == minCost) {
						converted_inputs = function1.convert(scripting::Function_Params{ inputs }, tree);
					}
					else if (cost2 == minCost) {
						converted_inputs = function2.convert(scripting::Function_Params{ inputs }, tree);
					}
					else if (cost3 == minCost) {
						converted_inputs = function3.convert(scripting::Function_Params{ inputs }, tree);
					}
					else if (cost4 == minCost) {
						converted_inputs = function4.convert(scripting::Function_Params{ inputs }, tree);
					}
				}

				if (1) {
					auto inputs{ std::vector<fibers::Any>{} };
					auto* function_impl = new scripting::details::Explicit_Function_Impl(
						[]() -> int { return 10; }
					);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);
					EXPECT_EQ(tree.Convert<int>(returned), 10);
				};

				if (1) {
					auto inputs{ std::vector<fibers::Any>{} };
					auto* function_impl = new scripting::details::Explicit_Function_Impl(
						[]() {}
					);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);
				};

				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { 1 }, { 2 } } };
					auto* function_impl = new scripting::details::Explicit_Function_Impl(
						[](int i, int j) -> int { return i + j; }
					);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);
					EXPECT_EQ(tree.Convert<int>(returned), 3);
				};

				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { 1.0 }, { 2.0f } } }; // provided types (do not need to match)
					auto* function_impl = new scripting::details::Explicit_Function_Impl(
						[](int i, int j) -> int { return i + j; } // callable
					);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);

					EXPECT_EQ(returned.cast<int>(), 3);
					EXPECT_EQ(tree.Convert<double>(returned), 3.0);
				};

				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { Units::inch(12) }, { Units::inch(12) } } }; // provided types (do not need to match)
					auto* function_impl = new scripting::details::Explicit_Function_Impl(
						[](Units::foot x1, Units::meter x2) -> Units::inch { return x1 + x2; } // callable
					);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);

					EXPECT_EQ(tree.Convert<Units::foot>(returned), 2.0);
				};

				try {
					auto inputs{ std::vector<fibers::Any>{ { Units::inch(12) }, { Units::year(12) } } }; // provided types (do not need to match)
					auto* function_impl = new scripting::details::Explicit_Function_Impl(
						[](Units::foot x1, Units::second x2) -> Units::value { return x1 + x2; } // callable
					);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);

					EXPECT_EQ(tree.Convert<Units::value>(returned), 2.0);
					
					EXPECT_EQ(false, true);
				}
				catch (std::runtime_error e) {
					std::cout << e.what() << std::endl;
					// EXPECTED TO CATCH A RUNTIME ERROR FOR THE BAD UNITS MANAGEMENT
				}


				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { DateTime::Now() }, { Units::year(1) } } }; // provided types (do not need to match)
					auto* function_impl = new scripting::details::Explicit_Function_Impl(
						[](DateTime const& x1, Units::second x2) -> DateTime { return x1 + x2; } // callable
					);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);

					EXPECT_EQ(tree.Convert<DateTime>(returned).tm_year() + 1900, DateTime::Now().tm_year()+1901);
				}
				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { stackThing("TEST", 100) } } }; // provided types (do not need to match)

					auto* function_impl = new scripting::details::Attribute_Access_Impl(&stackThing::varName);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };
					
					EXPECT_EQ(returned.IsTypeOf(ptr->ReturnType()), true);		
					EXPECT_EQ(tree.Convert<std::string>(returned), "TEST");
					returned.cast<std::string&>() = "TEST2";
					EXPECT_EQ(inputs[0].cast<stackThing&>().varName, "TEST2");
				}
				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { stackThing("TEST") } } }; // provided types (do not need to match)

					auto* function_impl = new scripting::details::Attribute_Access_Impl(&stackThing::var);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };

					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<void>()), true);
				}
				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { stackThing("TEST", 100) } } }; // provided types (do not need to match)

					auto* function_impl = new scripting::details::Attribute_Access_Impl(&stackThing::var);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };

					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<int>()), true);
					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<fibers::Any>()), false);
					EXPECT_EQ(returned.cast<int>(), 100);
				}
				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { stackThing("TEST", 100) } } }; // provided types (do not need to match)
					auto ptr = scripting::details::Member_Function_Impl(&stackThing::length);
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };

					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<int>()), true);
					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<fibers::Any>()), false);
					EXPECT_EQ(returned.cast<int>(), 4);
				}
				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { 1.0l } } }; // provided types (do not need to match)
					auto* function_impl = new scripting::details::Static_Function_Impl(&Units::CUBED);
					auto ptr{ std::static_pointer_cast<scripting::details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };

					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<long double>()), true);
					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<fibers::Any>()), false);
					EXPECT_EQ(returned.cast<long double>(), 1); // FAILURE
				}
				if (1) {
					auto inputs{ std::vector<fibers::Any>{ { stackThing("TEST", 100) } } }; // provided types (do not need to match)
					auto ptr = scripting::details::Member_Function_Impl(&stackThing::get_var_name);
					auto returned{ ptr->operator()(scripting::Function_Params{ inputs }, tree) };

					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<std::string>()), true);
					EXPECT_EQ(returned.IsTypeOf(fibers::user_type<fibers::Any>()), false);
					EXPECT_EQ(returned.cast<std::string>(), "TEST");
				}
#endif

				// PROXY FUNCTIONS
				if (1) {
					// PROXY_FUNCTION WRAPPERS
					std::string example = "EXAMPLE";
					auto caller0 = scripting::make_callable([example]() -> std::string { return example; }); // capturing lambda, no return
					auto caller1 = scripting::make_callable(&stackThing::varName); // member object 
					auto caller2 = scripting::make_callable(&stackThing::var); // member object 
					auto caller3 = scripting::make_callable(&stackThing::length); // member function, returns value
					auto caller4 = scripting::make_callable([](stackThing& o) -> void {}); // lambda, no return
					auto caller5 = scripting::make_callable([](stackThing& o)-> int { return o.length(); }); // lambda, returns
					auto caller6 = scripting::make_callable(&Units::CUBED); // static member function
					auto caller7 = scripting::make_callable(&stackThing::get_var_name); // member function, returns reference
					auto caller8 = scripting::make_callable(&Thing); // static function, returns
					auto caller9 = scripting::make_callable([](stackThing& a, stackThing const& b) -> stackThing& { return a = b; }); // static friend function

					// STATIC RESULT TESTS (WITHOUT RUNNING THE ACTUAL CODE)
					EXPECT_EQ(caller0->ReturnType(), fibers::user_type<std::string>());
					EXPECT_EQ(caller1->ReturnType(), fibers::user_type<std::string>());
					EXPECT_NE(caller2->ReturnType(), fibers::user_type<int>()); // Because the return type is not known at compile-time, the "return type" from the function is "Any".
					EXPECT_EQ(caller2->ReturnType(), fibers::user_type<fibers::Any>()); // Because the return type is not known at compile-time, the "return type" from the function is "Any".
					EXPECT_EQ(caller3->ReturnType(), fibers::user_type<int>());
					EXPECT_EQ(caller4->ReturnType(), fibers::user_type<void>());
					EXPECT_EQ(caller5->ReturnType(), fibers::user_type<int>());
					EXPECT_EQ(caller6->ReturnType(), fibers::user_type<long double>());
					EXPECT_EQ(caller7->ReturnType(), fibers::user_type<std::string>());
					EXPECT_EQ(caller8->ReturnType(), fibers::user_type<bool>());
					EXPECT_EQ(caller9->ReturnType(), fibers::user_type<stackThing>());

					// INPUTS
					auto inputs{ std::vector<fibers::Any>{ { stackThing("TEST", 100) } } }; // provided types (do not need to match)
					auto inputs2{ std::vector<fibers::Any>{ { stackThing("TEST", 100) }, { stackThing("TEST2", 200) } } }; // provided types (do not need to match)

#if 0
					// RETURNED RESULTS FROM PROXY_FUNCTIONS
					auto returned0{ scripting::call(caller0, {}, tree) };
					auto returned1{ scripting::call(caller1, inputs, tree) };
					auto returned2{ scripting::call(caller2, inputs, tree) };
					auto returned3{ scripting::call(caller3, inputs, tree) };
					auto returned4{ scripting::call(caller4, inputs, tree) };
					auto returned5{ scripting::call(caller5, inputs, tree) };
					auto returned6{ scripting::call(caller6, { { 100.0l } }, tree) };
					auto returned7{ scripting::call(caller7, inputs, tree) };
					auto returned8{ scripting::call(caller8, {}, tree) };
					auto returned9{ scripting::call(caller9, inputs2, tree) };

					// TEST RUNTIME RESULTS TO ENSURE TYPES ALIGN WITH EXPECTATIONS
					EXPECT_EQ(returned0.IsTypeOf(fibers::user_type<std::string>()), true);
					EXPECT_EQ(returned1.IsTypeOf(fibers::user_type<std::string>()), true);
					EXPECT_EQ(returned2.IsTypeOf(fibers::user_type<int>()), true); // the type is only known at runtime, after running the function
					returned2.cast<int&>() = 1000; // test assignment for an Any passed as a result
					EXPECT_EQ(inputs[0].cast<stackThing&>().var.cast<int&>(), 1000);
					EXPECT_EQ(returned3.IsTypeOf(fibers::user_type<int>()), true);
					EXPECT_EQ(returned4.IsTypeOf(fibers::user_type<void>()), true);
					EXPECT_EQ(returned5.IsTypeOf(fibers::user_type<int>()), true);
					EXPECT_EQ(returned6.IsTypeOf(fibers::user_type<long double>()), true);
					EXPECT_EQ(returned7.IsTypeOf(fibers::user_type<std::string>()), true);
					EXPECT_EQ(returned8.IsTypeOf(fibers::user_type<bool>()), true);
					returned1.cast<std::string&>() = "TEST2"; // test assignment for member object
					EXPECT_EQ(inputs[0].cast<stackThing&>().varName, "TEST2");
					returned7.cast<std::string&>() = "TEST3"; // test assignment for reference object passed as result for member function
					EXPECT_EQ(inputs[0].cast<stackThing&>().varName, "TEST3");
					EXPECT_EQ(returned9.IsTypeOf(fibers::user_type<stackThing>()), true);
					EXPECT_EQ(inputs2[0].cast<stackThing>().varName, "TEST2");
					
					// ensure type-conversion failures throws catchable errors
					try {
						scripting::call(caller1, {}, tree);
						EXPECT_EQ(true, false);
					}
					catch (std::exception e) {  }
					try {
						scripting::call(caller1, { fibers::Any{ 100 } }, tree);
						EXPECT_EQ(true, false);
					}
					catch (std::exception e) {  }

#endif
				}

				tree.Convert<DateTime>( 100ull );
				EXPECT_EQ(tree.Convert<std::string_view>(std::string("TEST")), "TEST");
				EXPECT_EQ(tree.Convert<std::string>(std::string_view(std::string("TEST"))), "TEST");
				EXPECT_EQ(tree.Convert<std::string>(100), "100");
				EXPECT_EQ(tree.Convert<std::string>(Units::meter(1)), Units::meter(1).ToString());

				EXPECT_EQ(tree.Convert<uint64_t>(100), 100);
				EXPECT_EQ(tree.Convert<long>(100ull), 100);




			}
		}

		// RingBuffer
		if (1) {
			fibers::containers::number<int> thing{ 0 };

			if (1) {
				fibers::synchronization::RingBuffer< stackThing, 256 > buffer;
				for (int i = 0; i < 256; i++) {
					if (!buffer.push_back(stackThing(Units::printf("%i", i), 10))) {
						std::cout << "FAILED TO PUSH" << std::endl;
					}
				}
				stackThing out;
				EXPECT_EQ(true, buffer.try_pop(out));
			}

			if (1) {
				fibers::synchronization::RingBuffer< stackThing, 50, false > buffer;

				fibers::parallel::For(0, 10000, [&](int i) {
					if (i % 2 == 0) {
						if (buffer.push_back(stackThing(Units::printf("%i", i), i))) {
							thing.Increment();
						}
					}
					else {
						stackThing out("FAIL", -1);
						if (buffer.try_pop(out)) { // does not *have( to succeed under specific contention cases
							thing.Decrement();
						}
					}
				});
				if (!EXPECT_EQ(thing.load(), buffer.size())) {
					std::cout << Units::printf("Count: %i;\tBufferSize: %i;\n", (int)thing.load(), (int)buffer.size());
				}

				thing = 0;
			}

			if (1) {
				fibers::synchronization::RingBuffer< stackThing, 50 > buffer;

				fibers::parallel::For(0, 10000, [&](int i) {
					if (i % 2 == 0) {
						if (buffer.push_back(stackThing(Units::printf("%i", i), i))) {
							thing.Increment();
						}
					}
					else {
						stackThing out("FAIL", -1);
						if (buffer.try_pop(out)) { // does not *have( to succeed under specific contention cases
							thing.Decrement();
						}
					}
				});
				if (!EXPECT_EQ(thing.load(), buffer.size())) {
					std::cout << Units::printf("Count: %i;\tBufferSize: %i;\n", (int)thing.load(), (int)buffer.size());
				}

				thing = 0;
			}

			if (1) {
				fibers::synchronization::RingBuffer< stackThing, 256, true> buffer;
				fibers::parallel::For(0, 255, [&](int i) {
					if (!buffer.push_back(stackThing(Units::printf("%i", i), 10))) {
						std::cout << "FAILED TO PUSH" << std::endl;
					}
					});
				stackThing out;
				EXPECT_EQ(true, buffer.try_pop(out));
				buffer.push_back(stackThing(Units::printf("%i", -1), 10));
				buffer.push_back(stackThing(Units::printf("%i", -2), 10));
				EXPECT_EQ(false, buffer.try_pop(out));
			}

			if (1) {
				fibers::synchronization::RingBuffer< stackThing, 256, true> buffer;
				fibers::parallel::For(0, 255, [&](int i) {
					if (!buffer.push_back(stackThing(Units::printf("%i", i), 10))) {
						std::cout << "FAILED TO PUSH" << std::endl;
					}
					});
				stackThing out;
				EXPECT_EQ(true, buffer.try_pop(out));
				buffer.push_back(stackThing(Units::printf("%i", -1), 10));
				buffer.push_back(stackThing(Units::printf("%i", -2), 10));
				EXPECT_EQ(false, buffer.try_pop(out));
			}
		}

		if (1) {
			fibers::synchronization::impl::atomic_shared_ptr< stackThing > testPtr{ nullptr };

			fibers::parallel::For(0, 1000, [&](int i) {
				testPtr = new stackThing(Units::printf("%i", i), 10); // gets exclusive access and sets the ptr
				fibers::synchronization::impl::atomic_shared_ptr< stackThing > ptr{ testPtr }; // shared access
				EXPECT_EQ(ptr->var.cast<int>(), 10); // shared access
			});

			fibers::parallel::For(0, 10000, [&](int i) {
				fibers::synchronization::impl::atomic_shared_ptr< stackThing > ptr{ testPtr }; // shared access
				EXPECT_EQ(ptr->var.cast<int>(), 10); // shared access
			});
		}

		if (1) {
			if (1) {
				fibers::utilities::Allocator<int> test1;
				auto* p123 = test1.Alloc();
				*p123 = 100;
			}
			if (1) {
				fibers::utilities::Allocator<int> test1;
				auto* p1234 = test1.Alloc();
				test1.Free(p1234);
			}
			if (1) {
				fibers::utilities::Allocator<double> test1;
				auto* p1235 = test1.Alloc();
				test1.Free(p1235);
			}
			if (1) {
				fibers::utilities::Allocator<std::string> test1;
				auto* p1236 = test1.Alloc();
				test1.Free(p1236);
			}
			if (1) {
				fibers::utilities::Allocator<std::string, 1024, false> test1;
				for (int i = 0; i < 2048; i++) {
					auto* p7 = test1.Alloc();
				}
			}

		}

		// NO LEAK w/ custom lock-free garbage collector
		if (1) {
			if (1) {
				fibers::utilities::EpochGarbageCollector _gc;
				stackThing* x = new stackThing("NAME HERE", 1);
				_gc.AddGarbage(x);
			}

			if (1) {
				fibers::utilities::EpochGarbageCollector _gc;
				for (int i = 0; i < 100; i++) {
					stackThing* x = new stackThing(std::to_string(i), i);
					_gc.AddGarbage(x);
				}
			}

			if (1) {
				fibers::utilities::EpochGarbageCollector _gc;
				std::vector<stackThing*> ptrs;
				for (int i = 0; i < 1000; i++) {
					stackThing* x = new stackThing(std::to_string(i), i);
					ptrs.push_back(x);
				}
				
				{
					const auto guard{ _gc.CreateEpochGuard() };

					fibers::parallel::ForEach(ptrs, [&](stackThing*& i) {
						_gc.AddGarbage(i);
					});

					for (int j = 0; j < 1000; j++) {
						(void)ptrs[j]->var.cast<int>(); // should still work, since NOTHING should have been deleted yet.
					}
				}
			}

			if (1) {
				fibers::utilities::EpochGarbageCollector _gc;
				fibers::containers::queue<stackThing*> queue;
				fibers::parallel::For(0, 1000, [&](int i) {
					queue.push(new stackThing(std::to_string(i) + ": " + std::to_string(-1), i));
				});

				fibers::parallel::For(0, 1000, [&](int i) {
					const auto guard{ _gc.CreateEpochGuard() };
					for (int j = 0; j < 10; j++) {
						const auto guard{ _gc.CreateEpochGuard() };

						queue.push(new stackThing(std::to_string(i) + ": " + std::to_string(j), i * j));
						
						stackThing* p{ nullptr };
						if (queue.try_pop(p)) {
							_gc.AddGarbage(p);
						}
					}
				});
				{
					stackThing* p{ nullptr };
					while (queue.try_pop(p)) {
						_gc.AddGarbage(p);
					}
				}
			}

			if (1) {
				fibers::utilities::Allocator<int> Allocator;

				fibers::JobGroup group;
				for (int i = 0; i < 100; i++) {
					auto* ptr = Allocator.Alloc(0);
					auto job = fibers::Job([&Allocator, ptr]() {});
					group.Queue(job);
				}
				group.Wait();
			
			}

			if (1) {
				fibers::utilities::GarbageCollectedAllocator< stackThing > Allocator;

				fibers::JobGroup group;
				for (int i = 0; i < 100; i++) {
					auto* ptr = Allocator.Alloc("GarbageCollectedAllocator1 " + std::to_string(i), i);
					auto job = fibers::Job([&Allocator, ptr]() {
						auto guard{ Allocator.CreateEpochGuard() };
						(void)ptr->var.cast<int>();
						Allocator.Free(ptr);
					});
					group.Queue(job);
				}
				group.Wait();
			}

			if (1) {
				fibers::utilities::GarbageCollectedAllocator< stackThing > Allocator;

				std::vector<fibers::synchronization::atomic_ptr<stackThing>> ptrs;

				for (int i = 0; i < 100; i++) {
					auto* ptr = Allocator.Alloc("GarbageCollectedAllocator2 " + std::to_string(i), i);
					ptrs.push_back(ptr);
				}

				fibers::parallel::For(0, 100, [&](int i) {
					fibers::parallel::ForEach(ptrs, [&](fibers::synchronization::atomic_ptr<stackThing>& ptr) {
						auto guard{ Allocator.CreateEpochGuard() };
						stackThing* toDo{ nullptr };
						if (i > 50) {
							toDo = ptr.load();
						}
						else {
							toDo = ptr.Set(nullptr);
							Allocator.Free(toDo);
						}

						if (toDo) {
							(void)toDo->var.cast<int>();
						}
					});
				});
			}

			if (1) {
				fibers::utilities::GarbageCollectedAllocator< stackThing > Allocator;

				(void)Allocator.Alloc("GarbageCollectedAllocator3 1", 1);
				(void)Allocator.Alloc("GarbageCollectedAllocator3 2", 1);
				(void)Allocator.Alloc("GarbageCollectedAllocator3 3", 1);
				(void)Allocator.Alloc("GarbageCollectedAllocator3 4", 1);
				(void)Allocator.Alloc("GarbageCollectedAllocator3 5", 1);
			}

			if (1) {
				fibers::utilities::GarbageCollectedAllocator< stackThing > Allocator;

				auto guard{ Allocator.CreateEpochGuard() };

				Allocator.Free(Allocator.Alloc("GarbageCollectedAllocator4 1", 1));
				Allocator.Free(Allocator.Alloc("GarbageCollectedAllocator4 2", 1));
				Allocator.Free(Allocator.Alloc("GarbageCollectedAllocator4 3", 1));
				Allocator.Free(Allocator.Alloc("GarbageCollectedAllocator4 4", 1));
				Allocator.Free(Allocator.Alloc("GarbageCollectedAllocator4 5", 1));
			}

			if (1) {
				fibers::utilities::GarbageCollectedAllocator< stackThing > Allocator;

				auto* p = Allocator.Alloc("GarbageCollectedAllocator5 1", 1);

				auto guard{ Allocator.CreateEpochGuard() };

				Allocator.Free(p);

				(void)p->var.cast<int>(); // safe to do since the epoch guard protects us
			}

			if (1) {
				fibers::utilities::GarbageCollectedAllocator<stackThing, sizeof(stackThing) * 3> Allocator;
				auto g{ Allocator.CreateEpochGuard() };
				for (int i = 0; i < 255; i++) {
					auto p2 = Allocator.Alloc(std::to_string(i), i);
					Allocator.Free(p2);
				}
			}
		}

		// NO LEAK w/ std::for_each
		if (1) {
			fibers::utilities::Sequence seq(1000); // 0..999
			std::for_each(seq.begin(), seq.end(), [](int& x) {
				x++;
			});
		}

		// NO LEAK w/atomic_ptr
		if (1) {
			fibers::synchronization::atomic_ptr<int> e{ nullptr };
			e.Set(new int(1));
			if (int* p = e.Set(nullptr)) {
				delete p;
			}
		}

		// NO LEAK w/atomic_ptr<std::exception_ptr>
		if (1) {
			try {
				fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };
				try {
					throw(std::runtime_error("Example Error"));
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
				if (auto* p = e.Set(nullptr)) {
					std::exception_ptr copy{ *p };
					delete p;
					std::rethrow_exception(std::move(copy));
				}
			}
			catch (std::runtime_error const& err) {}
		}

		// NO LEAK w/ std::for_each which catches user errors
		if (1) {
			try {
				auto todo = [](int& x) { // user throwable code
					throw(std::runtime_error("Example Error"));
				};

				fibers::utilities::Sequence seq(1000); // 0..999
				fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

				std::for_each(seq.begin(), seq.end(), [&](int& x) {
					try {
						if (!e) todo(x);
					}
					catch (...) {
						if (!e) {
							if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
						}
					}
					});
				if (auto* p = e.Set(nullptr)) {
					std::exception_ptr copy{ *p };
					delete p;
					std::rethrow_exception(std::move(copy));
				}
			}
			catch (std::runtime_error const& err) {}
		}

		// NO LEAK w/ fibers::parallel::For
		if (1) {
			try {
				fibers::parallel::For(0, 100, [](int& i) {
					throw(std::runtime_error("Example Error"));
				});
			}
			catch (std::runtime_error const& err) {}
		}

		// NO LEAK w/ fibers::parallel::ForEach
		if (1) {
			try {
				fibers::utilities::Sequence seq(1000); // 0..999
				fibers::parallel::ForEach(seq, [](int& i) {
					throw(std::runtime_error("Example Error"));
				});
			}
			catch (std::runtime_error const& err) {}
		}

		// CAS_Container
		if (1) {

			EXPECT_EQ(fibers::utilities::FloatWrapper{ 1 }, fibers::utilities::FloatWrapper{ 1 });
			EXPECT_EQ(fibers::utilities::FloatWrapper{ 100 }, fibers::utilities::FloatWrapper{ 100 });
			EXPECT_EQ(fibers::utilities::FloatWrapper{ -100 }, fibers::utilities::FloatWrapper{ -100 });
			EXPECT_EQ(fibers::utilities::FloatWrapper{ -100 }, -100.0);

			static constexpr auto size_is1 = sizeof(fibers::utilities::FloatWrapper);
			static constexpr auto size_is2 = sizeof(fibers::utilities::DoubleWrapper);

			EXPECT_EQ(fibers::utilities::DoubleWrapper{ 1 }, fibers::utilities::DoubleWrapper{ 1 });
			EXPECT_EQ(fibers::utilities::DoubleWrapper{ 100 }, fibers::utilities::DoubleWrapper{ 100 });
			EXPECT_EQ(fibers::utilities::DoubleWrapper{ -100 }, fibers::utilities::DoubleWrapper{ -100 });
			EXPECT_EQ(fibers::utilities::DoubleWrapper{ -100 }, -100.0);

			if (0) {
				fibers::utilities::DoubleWrapper wrapper;
				fibers::parallel::For(-1000.0, 1000.0, [&wrapper](double i) {
					double prevV = wrapper.Update([i](double in) { return in + i / 2.; });
					double newV = prevV + i / 2.;

					printf("%lf + %lf = %lf\n", prevV, i / 2., newV);

				});
				std::cout << wrapper.load() << std::endl << std::endl << std::endl;
			}


			if (0) {
				fibers::containers::number<double> wrapper;
				fibers::parallel::For(-1000.0, 1000.0, [&wrapper](double i) {
					double prevV = wrapper.Add(i / 2.);
					double newV = prevV + i / 2.;
					printf("%lf + %lf = %lf\n", prevV, i / 2., newV);
				});
				std::cout << wrapper.load() << std::endl << std::endl << std::endl;
			}





			//fibers::containers::Pattern<long double, double> tree;
			//fibers::parallel::For(-10000, 10000, 500, [&tree](int i) {
			//	EXPECT_EQ(true, tree.Insert(i, i));
			//});
			//fibers::parallel::For(-1000, 1000, 50, [&tree](int i) {
			//	EXPECT_EQ(true, tree.Insert(i, i));
			//});

			//for (auto& x : tree) {
			//	std::cout << fibers::utilities::CAS_Container<double>{x.second}.load() << std::endl;
			//	std::cout << Units::second{ x.second } << std::endl;
			//	std::cout << Units::cubic_meter_per_second{ x.second } << std::endl;
			//	std::cout << Units::gallon_per_minute{ x.second } << std::endl;
			//}


		}

		// NO LEAK w/ fibers::Any
		if (1) {
			fibers::Any test;
			test = 100.0f; // value assignment. Is now a float.
			test = std::string("TEST"); // value assignment. Is now a String.
			test = std::make_shared<float>(100.0f); // shared_ptr assignment. Is now a float.
			test = std::make_shared<std::string>("TEST"); // shared_ptr assignment. Is now a String.

			std::string obj1 = test.cast<std::string>(); // copy value
			std::string& obj2 = test.cast<std::string&>(); // reference capture
			std::string* obj3 = test.cast<std::string*>(); // pointer capture
			std::shared_ptr<std::string> obj4 = test.cast<std::shared_ptr<std::string>>(); // sharted_pointer capture

			std::string obj5 = test.cast(); // E-Z copy value
			std::string& obj6 = test.cast(); // E-Z reference capture
			std::string* obj7 = test.cast(); // E-Z pointer capture
			std::shared_ptr<std::string> obj8 = test.cast(); // E-Z sharted_pointer capture
			obj5 = test.cast<decltype(obj5)>(); // set to value (must be explicit since String has multiple operator=() functions)
			obj6 = test.cast<decltype(obj6)>(); // set to reference (must be explicit since String has multiple operator=() functions)
			obj7 = test.cast(); // set to pointer
			obj8 = test.cast<decltype(obj8)>(); // set to sharted_pointer

			test = std::make_shared<float>(100.0f); // shared_ptr assignment. Is now a float.

			float obj9 = test.cast(); // E-Z copy value
			float& obj10 = test.cast(); // E-Z reference capture
			float* obj11 = test.cast(); // E-Z pointer capture
			std::shared_ptr<float> obj12 = test.cast(); // E-Z sharted_pointer capture
			obj9 = test.cast(); // set to value
			obj10 = test.cast(); // set to reference
			obj11 = test.cast(); // set to pointer
			obj12 = test.cast<decltype(obj12)>(); // set to sharted_pointer
		}

		// NO LEAK w/ atomic_number<double>
		if (1) {
			fibers::utilities::Sequence seq(1000); // 0..999
			fibers::utilities::CAS_Container<double> D{ 1000 };
			if (!EXPECT_EQ(D, 1000.0)) { std::cout << D.load() << std::endl; }
			
			D.Add(1000);
			if (!EXPECT_EQ(D, 2000.0)) { std::cout << D.load() << std::endl; }
		}

		// NO LEAK w/ atomic_number<double>
		if (1) {
			fibers::utilities::Sequence seq(1000); // 0..999
			fibers::synchronization::atomic_number<double> D{ 0 };
			fibers::parallel::ForEach(seq, [&D](int i) {
				D.Add(1);
			});
			if (!EXPECT_EQ(D, 1000.0)) { std::cout << std::to_string(D.load()) << std::endl; }
		}

		// NO LEAK w/ atomic_number<double> and throwing ForEach iteration
		if (1) {
			try {
				fibers::utilities::Sequence seq(1000); // 0..999
				fibers::synchronization::atomic_number<double> D{ 0 };
				fibers::parallel::ForEach(seq, [&D](int i) {
					if (D.Decrement() < 500) {
						throw(true);
					}
				});
				EXPECT_EQ(D, -1000.0);
			}
			catch (bool v) {
				EXPECT_EQ(v, true);
			}
		}

		// NO LEAK w/ CAS_Container
		if (1) {
			fibers::utilities::DoubleWrapper value{ 0 };
			if (!EXPECT_EQ(value, 0)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(1);
			if (!EXPECT_EQ(value, 1)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(1);
			if (!EXPECT_EQ(value, 2)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(-5);
			if (!EXPECT_EQ(value, -3)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(2);
			if (!EXPECT_EQ(value, -1)) { std::cout << std::to_string(value.load()) << std::endl; };
		}
		if (1) {
			fibers::utilities::CAS_Container<double> value{ 0 };
			if (!EXPECT_EQ(value, 0)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(1);
			if (!EXPECT_EQ(value, 1)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(1);
			if (!EXPECT_EQ(value, 2)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(-5);
			if (!EXPECT_EQ(value, -3)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(2);
			if (!EXPECT_EQ(value, -1)) { std::cout << std::to_string(value.load()) << std::endl; };
		}
		if (1) { 
			fibers::utilities::CAS_Container<double> value{ 1 };
			if (!EXPECT_EQ(value, 1)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(1);
			if (!EXPECT_EQ(value, 2)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(1);
			if (!EXPECT_EQ(value, 3)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(-5);
			if (!EXPECT_EQ(value, -2)) { std::cout << std::to_string(value.load()) << std::endl; };

			value.Add(2);
			if (!EXPECT_EQ(value, 0)) { std::cout << std::to_string(value.load()) << std::endl; };
		}
				 
		// NO LEAK w/ atomic_number<double>
		if (1) {
			fibers::synchronization::atomic_number<double> value{ 0 };
			EXPECT_EQ(value, 0);
			for (int i = 0; i < 10000; i++) {
				value.Add(0.125);
			}
			EXPECT_EQ(value, 10000 * 0.125);
			for (int i = 0; i < 10000; i++) {
				value.Sub(0.125);
			}
			EXPECT_EQ(value, 0);
			value = 0;
		}
		
		// NO LEAK w/ atomic_number<double> under parallel access
		if (1) {
			fibers::synchronization::atomic_number<double> value;
			EXPECT_EQ(value.load(), 0);
			fibers::parallel::For(0, 10000, [&value](int jobNum) {
				value.Add(0.125);
				});
			EXPECT_EQ(value.load(), 10000 * 0.125);
			fibers::parallel::For(0, 10000, [&value](int jobNum) {
				value.Sub(0.125);
				});
			EXPECT_EQ(value.load(), 0);

			value = 0;
		}

		// NO LEAK w/ atomic Units. Initial allocation for unit system, but otherwise OK. 
		if (1) {
			using namespace literals;
			
			std::array< fibers::utilities::DoubleWrapper, 5> unitType_m{ 0, 0, 0, 0, 0 };
			fibers::utilities::DoubleWrapper ratio_m{ 0 };
			fibers::utilities::DoubleWrapper value_m{ 0 };
			
			auto test{ fibers::utilities::MultiItemCAS(&unitType_m[0], &unitType_m[1], &unitType_m[2], &unitType_m[3], &unitType_m[4], &ratio_m, &value_m) };
			fibers::parallel::For(0, 100, [&test](int i) {
				test.Update([](auto inputs)->auto {
					std::array< fibers::utilities::DoubleWrapper*, 5> unitType_m{
						&inputs.get<0>(), &inputs.get<1>(), &inputs.get<2>(), &inputs.get<3>(), &inputs.get<4>()
					};
					fibers::utilities::DoubleWrapper& ratio_m = inputs.get<5>();
					fibers::utilities::DoubleWrapper& value_m = inputs.get<6>();

					unitType_m[0]->Add(1);
					ratio_m.Add(1);
					value_m.Add(1);

					return inputs;
				});


			});
			EXPECT_EQ(ratio_m, 100);
			EXPECT_EQ(value_m, 100);
			EXPECT_EQ(unitType_m[0], 100);

			EXPECT_EQ((-5 / Units::foot(1_m)).ToString(), (5 / -1_m).ToString());
			EXPECT_EQ((1_ft).ToString(), "1 ft");
			EXPECT_EQ((10_m / 2_s).ToString(), "5 mps");
			EXPECT_EQ(((1_m).pow(3)).ToString(), "1 cu_m");
			EXPECT_EQ(((4_sq_m).pow(0.5)).ToString(), "2 m");
			EXPECT_EQ((16_cu_m / 2_m).ToString(), "8 sq_m");
			EXPECT_EQ((16_cu_ft / 2_ft).ToString(), "8 sq_ft"); 

			auto y1 = 1_ft;
			auto y2 = 1_m;
			auto y3 = y1 - y2;
			Units::meter test1 = y3; // forces the result to meter (default, SI unit for length)

			auto x1 = Units::foot(1_ft);
			auto x2 = Units::meter(-1_m);
			auto x3 = x1 + x2;
			Units::yard test2 = x3; // forces the result to yards
			auto test3 = Units::foot(1_ft) - Units::meter(1_m); // allows any resulting unit so long as the value is correct for the unit selected. E.g. could be foot, meter, cm, etc. In this case it'll be foot.

			EXPECT_EQ(test1, test2);
			EXPECT_EQ(test1, test3);
			EXPECT_EQ(test2, test3);

			// Test multi-threading
			if (1) { // simple atomic additions
				using namespace literals;

				Units::value shared = 0_ft;

				fibers::parallel::For((size_t)0, (size_t)(100), [&shared](size_t threadNum) {
					for (int i = 0; i < 100; i++) {
						switch (threadNum % 4) {
						default:
						case 0:
							shared += 1_ft;
							break;
						case 1:
							shared += Units::meter(1_ft);
							break;
						case 2:
							shared += Units::inch(1_ft);
							break;
						case 3:
							shared += Units::centimeter(1_ft);
							break;
						}
					}
				});
				if (!EXPECT_EQ(shared, 10000_ft)) { std::cout << shared << std::endl; }
			}
		}

		// NO LEAK w/ Unions
		if (1) {
			using namespace fibers::utilities;
			{
				Union<double, bool, float> obj;
				obj.get<0>() += 1.0;
				obj.get<1>() = true;
				obj.get<2>() += 1.0f;
			}
			{
				Union<double*, bool*, float*> obj;
				obj.get<0>() = (double*)(void*)(1);
				obj.get<1>() = (bool*)(void*)(1);
				obj.get<2>() = (float*)(void*)(1);
			}
		}

		// NO LEAK w/ MultiItemCAS
		if (1) {
			using namespace fibers::utilities;
			{
				uint64_t item1{ 0 };
				uint64_t item2{ 0 };

				auto container{ MultiItemCAS(
					&item1,
					&item2
				) };

				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();
				constexpr size_t kExecNum = 1e5;

				fibers::parallel::For((size_t)0, kThreadNum, [kExecNum, &container](size_t threadNum) {
					for (size_t i = 0; i < kExecNum; ++i) {
						container.Update([](auto input) {
							input.get<0>() += 1;
							input.get<1>() += 1;
							return input;
						});
					}
				});

				EXPECT_EQ(container.Read<0>(), 2000000);
				EXPECT_EQ(container.Read<1>(), 2000000);
			}
			{
				uint64_t item1{ 0 };
				uint64_t item2{ 0 };
				uint64_t item3{ 0 };

				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();
				constexpr size_t kExecNum = 1e5;

				fibers::parallel::For((size_t)0, kExecNum, [kThreadNum, &item1, &item2, &item3](size_t threadNum) {
					if (threadNum % 2 == 0) {
						auto container{ MultiItemCAS(
							&item1,
							&item3
						) };

						for (size_t i = 0; i < kThreadNum; ++i) {
							container.Update([](auto input) { 
								input.get<0>() += 2;
								input.get<1>() += 1;
								return input; 
							});
						}
					}
					else {
						auto container{ MultiItemCAS(
							&item2,
							&item3
						) };

						for (size_t i = 0; i < kThreadNum; ++i) {
							container.Update([](auto input) {
								input.get<0>() += 2;
								input.get<1>() += 1;
								return input;
							});
						}
					}
				});

				EXPECT_EQ(item1, 2000000);
				EXPECT_EQ(item2, 2000000);
				EXPECT_EQ(item3, 2000000);
			}
		}

		// NO LEAK w/ Atomic BW Tree
		if (1) {
			//  No leak
			if (1) {
				// No leak -->
				fibers::utilities::dbgroup::index::bw_tree::BwTree<uint64_t, uint64_t> tree;
				
				for (int i = 0; i < 100; i ++)
					tree.Write(i, i);
				
				for (int i = 0; i < 100; i++)
					tree.Delete(i);

				for (int i = 0; i < 100; i++)
					tree.Write(i, i);

				

				for (auto iter = tree.Scan(); iter; ++iter) {
					

				}
				// <-- No leak

			}
			// No leak
			if (1) {
				fibers::containers::Pattern<uint64_t, uint64_t> tree;
				EXPECT_EQ(tree.GetNumValues(), 0);

				for (int i = 0; i < 50; i++) {
					tree.Insert(i, i);
				}
				EXPECT_EQ(tree.GetNumValues(), 50);

				for (int i = 0; i < 50; i++) {
					tree.Delete(i);
				}
				EXPECT_EQ(tree.GetNumValues(), 0);

				for (int i = 0; i < 50; i++) {
					tree.Insert(i, i, false);
				}
				EXPECT_EQ(tree.GetNumValues(), 50);

				for (int i = 0; i < 50; i++) {
					tree.Insert(i, i + 1, true);
				}
				EXPECT_EQ(tree.GetNumValues(), 50);
				EXPECT_EQ(tree.GetMinValue().value(), 1);
				EXPECT_EQ(tree.GetMaxValue().value(), 50);
			}
			// No leak
			if (1) {
				fibers::containers::Pattern<uint64_t, uint64_t> tree;
				tree.Insert(5, 100);
				EXPECT_EQ(true, tree.Read(5).has_value());
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 50, [&tree](int i) {
					tree.Insert(i, i);
				});

				for (auto iter = tree.Scan(); iter; iter++) {
					(void)iter.GetKey();
				}
			}
			// No leak
			if (0) {
				fibers::containers::Pattern<int, double> tree;
				tree.Insert(5, 100);
				EXPECT_EQ(true, tree.Read(5).has_value());
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 500, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
				});

				for (auto iter = tree.Scan(); iter; iter++) {
					(void)iter.GetKey();
				}
			}
			// No leak
			if (1) {
				fibers::containers::Pattern<double, double> tree;
				tree.Insert(5, 100.0);
				EXPECT_EQ(true, tree.Read(5).has_value() && (tree.Read(5).value() == 100.0));
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 50, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
				});

				for (auto iter = tree.Scan(); iter; iter++) {
					(void)iter.GetKey();
				}

				EXPECT_EQ(tree.GetCurrentValue(0.5, fibers::containers::interp_t::LEFT), 0);
				EXPECT_EQ(tree.GetCurrentValue(0.5, fibers::containers::interp_t::RIGHT), 1);
				EXPECT_EQ(tree.GetCurrentValue(0.5, fibers::containers::interp_t::LINEAR), 0.5);
				EXPECT_EQ(tree.GetCurrentValue(0.5, fibers::containers::interp_t::SPLINE), 0.5);

				//for (double i = -5; i < 55; i += 0.25) {
				//	auto left1 = tree.FindNthLargestSmallerEqual(i, 1);
				//	auto left2 = tree.FindNthLargestSmallerEqual(i, 2);
				//	auto left3 = tree.FindNthLargestSmallerEqual(i, 3);
				//	auto left4 = tree.FindNthLargestSmallerEqual(i, 4);
				//	auto left5 = tree.FindNthLargestSmallerEqual(i, 5);
				//	std::cout << Units::printf("%f (left5) %f (left4) %f (left3) %f (left2) %f (left1) %f (left) %f (right) %f (linear) %f (spline) %f\n"
				//		, i
				//		, left5 ? (double)left5.GetPayload() : 0.0
				//		, left4 ? (double)left4.GetPayload() : 0.0
				//		, left3 ? (double)left3.GetPayload() : 0.0
				//		, left2 ? (double)left2.GetPayload() : 0.0
				//		, left1 ? (double)left1.GetPayload() : 0.0
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::LEFT).value_or(0)
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::RIGHT).value_or(0)
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::LINEAR).value_or(0)
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::SPLINE).value_or(0)
				//	);
				//}
			}
			// No leak
			if (1) {
				fibers::containers::Pattern<double, double> tree;

				fibers::parallel::For(0, 50, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, ::sin(i)));
				});

				//for (double i = -5; i < 55; i += 0.25) {
				//	auto left = tree.FindNthLargestSmallerEqual(i, 1);
				//	auto left2 = tree.FindNthLargestSmallerEqual(i, 2);
				//	auto left3 = tree.FindNthLargestSmallerEqual(i, 3);
				//	auto left4 = tree.FindNthLargestSmallerEqual(i, 4);
				//	auto left5 = tree.FindNthLargestSmallerEqual(i, 5);
				//	std::cout << Units::printf("%f (sin(t)) %f (left) %f (right) %f (linear) %f (spline) %f\n"
				//		, i
				//		, ::sin(i)
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::LEFT).value_or(0)
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::RIGHT).value_or(0)
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::LINEAR).value_or(0)
				//		, tree.GetCurrentValue(i, fibers::containers::interp_t::SPLINE).value_or(0)
				//	);
				//}

				for (auto& x : tree.GetKnotSeries(-5, 55)) {
					std::cout << Units::printf("%f (sin(t)) %f (knot for) %f\n"
						, x.first
						, ::sin(x.first)
						, x.second
					);
				}

				for (auto& x : tree.GetTimeSeries(-5, 55, 0.25, fibers::containers::interp_t::SPLINE)) {
					std::cout << Units::printf("%f (sin(t)) %f (spline for) %f\n"
						, x.first
						, ::sin(x.first)
						, x.second.value_or(0)
					);
				}

				fibers::parallel::ForEach(tree.GetKnotSeries(-5, 55), [](auto& x) {
					std::cout << Units::printf("%f (sin(t)) %f (knot foreach) %f\n"
						, x.first
						, ::sin(x.first)
						, x.second
					);
				});



			}
			// CustomizedSequence
			if (1) {
				if (1) {
					int count{ 0 };
					auto seq{ fibers::utilities::CustomizedSequence<double, double>([](double x) -> double { return 0; }, 0, 100, 1) };
					for (auto& item : seq) {
						EXPECT_EQ(item, 0);
						count++;
					}
					EXPECT_EQ(count, 100);
				}
				if (1) {
					int count{ 0 };
					auto seq{ fibers::utilities::CustomizedSequence<double, double>([](double x) -> double { return 10; }, 0, 100, 5) };
					for (auto& item : seq) {
						EXPECT_EQ(item, 10);
						count++;
					}
					EXPECT_EQ(count, 20);
				}
			}




			// No Leak
			if (1) {
				fibers::containers::Pattern<long double, double> tree;
				tree.Insert(5, 100.0);
				EXPECT_EQ(true, tree.Read(5).has_value() && (tree.Read(5).value() == 100.0));
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(-5, 500, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
					});

				auto futureObj = fibers::parallel::async([&tree]() {
					for (int i = 0; i < 500; i++) {
						if (i % 50 == 0) ::Sleep(2);

						tree.Insert(i - 1, i + 1);
						tree.Insert(i + 0.5, i + 0.5);
					}
					return 100.0f;
				});

				for (double D = -2.25; D <= 505; D += 1.25) {
					auto iter_Larger = tree.FindSmallestLargerEqual(D);
					auto iter_Smaller = tree.FindLargestSmallerEqual(D);

					if (iter_Larger && iter_Smaller) {
						//Sleep(1);
						//std::cout << cweeStr::printf("\t %f <= %f <= %f\n", (float)iter_Smaller.GetKey().load(), (float)D, (float)iter_Larger.GetKey().load());
					}
				}

				// wait until the job is completed and the result is returned.
				(void)futureObj.wait_get_ref();

				fibers::containers::number<long> count{ 0 };
				auto iter_end{ tree.end() };
				int initialCount{ 0 };

				count = 0;
				for (auto iter = tree.Scan(); iter; iter++) {
					count++;
				}
				EXPECT_EQ(true, count >= 500);
				initialCount = count.load();

				count = 0;
				for (auto iter = tree.Scan(5, 15); iter; iter++) { // works, surprisingly
					count++;
				}
				EXPECT_EQ(true, count >= 8);

				count = 0;
				for (auto iter = tree.FindSmallestLargerEqual(5); iter; ++iter) {
					count++;
				}
				EXPECT_EQ(true, count >= 495);

				count = 0;
				for (auto iter = tree.FindSmallestLargerEqual(5, 15); iter; ++iter) {
					count++;
				}
				EXPECT_EQ(true, count >= 8);

				count = 0;
				for (auto iter = tree.begin(5.25, 15.25); iter != iter_end; ++iter) { // neither 5 nor 15 exist
					count++;
				}
				EXPECT_EQ(true, count >= 8);

				count = 0;
				for (auto iter = tree.begin(5); iter != iter_end; ++iter) { // neither 5 nor 15 exist
					count++;
				}
				EXPECT_EQ(true, count >= 495);

				count = 0;
				for (auto iter = tree.begin(); iter != iter_end; ++iter) {
					count++;
				}
				EXPECT_EQ(true, count >= 500);
				EXPECT_EQ(initialCount, count.load());

				count = 0;
				for (auto iter = tree.begin(); iter != iter_end; std::advance(iter, 1)) {
					count++;
				}
				EXPECT_EQ(true, count >= 500);
				EXPECT_EQ(initialCount, count.load());

				count = 0;
				for (auto& x : tree) {
					count++;
				}
				EXPECT_EQ(true, count >= 500);
				EXPECT_EQ(initialCount, count.load());

				count = 0;
				fibers::parallel::ForEach(tree, [&count](std::pair<long double, double> const& iter) {
					count++;
				});
				EXPECT_EQ(true, count >= 500);
				EXPECT_EQ(initialCount, count.load());
			}
			// No leak
			if (1) {
				using xType = Units::second;
				using yType = Units::gallon_per_minute;

				fibers::containers::Pattern<long double, double> tree;
				
				// when under heavy contention, this creates a LOT of tracking pages that need collapse
				fibers::parallel::For(-100000, 100000, 1, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
				});

				fibers::parallel::For(-1000, 1000, 50, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
				});

				int count{ 0 };

				for (auto& x : tree) { count++; }

				for (auto x = tree.begin(); x != tree.end(); ++x) {}
				for (auto x = tree.cbegin(); x != tree.cend(); ++x) {}

			}
			// No leak
			if (1) {
				for (int j = 1; j < 10; j += 4) {
					int numLoops{ 100 * j * j };
					if (0) {
						fibers::containers::Pattern<double, double> pat;
						fibers::parallel::For(0, numLoops, [&](int i) {
							for (int k = 0; k < j; k++) {
								(void)pat.Insert((i* j) + k, 0);
							}
						});
					}
					if (0) {
						fibers::containers::Pattern<uint64_t, uint64_t> pat;
						fibers::parallel::For(0, numLoops, [&](int i) {
							for (int k = 0; k < j; k++) {
								(void)pat.Insert((i * j) + k, 0);
							}
						});
					}
					if (1){
						fibers::containers::Pattern<long double, double> pat;
						fibers::parallel::For(0, numLoops, [&](int i) {
							for (int k = 0; k < j; k++) {
								(void)pat.Insert((i * j) + k, 0);
							}
						});
					}
				}
			}
		}

		// NO LEAK w/ Jobs
		if (1) {
			fibers::JobGroup group;
			for (int j = 1; j < 10; j++) {
				group.Queue(fibers::Job([](int j) {
					int numLoops{ 100 * j * j };
					if (1) {
						fibers::containers::Pattern<double, double> pat;
						fibers::parallel::For(0, numLoops, [&](int i) {
							for (int k = 0; k < j; k++) {
								(void)pat.Insert((i * j) + k, 0);
							}
						});
					}
					if (1) {
						fibers::containers::Pattern<uint64_t, uint64_t> pat;
						fibers::parallel::For(0, numLoops, [&](int i) {
							for (int k = 0; k < j; k++) {
								(void)pat.Insert((i * j) + k, 0);
							}
						});
					}
					if (1) {
						fibers::containers::Pattern<long double, double> pat;
						fibers::parallel::For(0, numLoops, [&](int i) {
							for (int k = 0; k < j; k++) {
								(void)pat.Insert((i * j) + k, 0);
							}
						});
					}
				}, j));
			}
			group.Wait();
		}

		// static function
		fibers::Job([]() { return 10.0f; }).AsyncFireAndForget(); // will not Throw, since the function is stateless (which can be called without concern of us going out of scope)

		// non-capturing functions
		fibers::Job([](int i)->float { return i; }, 100).AsyncFireAndForget(); // will not Throw, since the function is stateless (which can be called without concern of us going out of scope)
		fibers::Job([](float& x)->float { return x - 10.0f; }, 55.0f).Invoke(); // Can also use lambdas instead of static function pointers.

		// capturing function (purposefully throwing due to implication of running after memory expires) ... Technically, capturing is OK so long as all of the caputres are by-value. However, I could not figure out how to test for that (if it is possible at all). 
		try {
			int xyz{ 0 };
			auto jobTest2 = fibers::Job([&xyz]() { return xyz; }); // capturing function -> This will fail at runtime, since it is not safe to run AsyncFireAndForget without some external guarrantees. 
			jobTest2.AsyncFireAndForget(); // will Throw, since the above function is not stateless
			return -1; // Bad!
		}
		catch (std::runtime_error) {
			// Good!
		}

		// fibers::parallel::async
		if (1) {
			fibers::parallel::async([]() { return; }).wait_get_ref();
			fibers::parallel::async([](float x) { return x + 100.0f; }, 1.0f).wait_get();
			fibers::parallel::async([](float x) { return x + 100.0f; }, 1.0f).wait_get_ref();
			fibers::parallel::async([](long double x) { return x + 100.0l; }, 1.0l).wait_get_shared();
		}

		// LongLongWrapper
		if (1) {
			fibers::utilities::LongLongWrapper wrap;
			wrap = 10;
			wrap++;
			EXPECT_EQ(11, wrap);
			EXPECT_EQ(21, wrap + 10);


		}

		// DateTime
		if (1) {
			using namespace literals;

			EXPECT_EQ(DateTime::make_time(2020, 1, 1).c_str(), "2020/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(2021, 1, 1).c_str(), "2021/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(2022, 1, 1).c_str(), "2022/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(2023, 1, 1).c_str(), "2023/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(2024, 1, 1).c_str(), "2024/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(2025, 1, 1).c_str(), "2025/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(2038, 1, 1).c_str(), "2038/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(2072, 1, 1).c_str(), "2072/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(1961, 1, 1).c_str(), "1961/1/1 0:0:0.000000");
			EXPECT_EQ(DateTime::make_time(1940, 1, 1, 1).c_str(), "1940/1/1 1:0:0.000000");
			EXPECT_EQ(DateTime::make_time(1940, 1, 1, 1, 1).c_str(), "1940/1/1 1:1:0.000000");
			EXPECT_EQ(DateTime::make_time(1940, 1, 1, 1, 1, 1).c_str(), "1940/1/1 1:1:1.000000");
			EXPECT_EQ((DateTime::make_time(1940, 1, 1, 1, 1, 1) + Units::second(1)).c_str(), "1940/1/1 1:1:2.000000");
			EXPECT_EQ((DateTime::make_time(1940, 1, 1, 1, 1, 1) + Units::day(1)).c_str(), "1940/1/2 1:1:1.000000");
			EXPECT_EQ((DateTime::make_time(1940, 1, 1, 1, 1, 1) - Units::year(1)).c_str(), "1939/1/1 1:1:1.000000");
			EXPECT_EQ(DateTime(Units::second(DateTime::make_time(2020, 1, 1))), DateTime::make_time(2020, 1, 1));

			DateTime time(0.0l);
			fibers::parallel::For(0, 1000001, [&](int i) {
				if (i % 2 == 0)
					time += 1;
				else
					time -= 1;
			});
			EXPECT_EQ(time, 1.0);
			time = 0.0l;

			fibers::JobGroup group;
			for (int i = 0; i < 1000; i++) {
				group.Queue(fibers::Job([&]() {
					time -= 1;
				}));
			}
			for (int i = 0; i < 1000; i++) {
				group.Queue(fibers::Job([&]() {
					time += 1;
				}));
			}
			group.Wait();

			EXPECT_EQ(time, 0.0);
			time += 1723572441; // adds seconds
			EXPECT_EQ(time, 1723572441.0);
			EXPECT_EQ(time.c_str(), "2024/8/13 11:7:21.000000");
			time += Units::day(1);
			EXPECT_EQ(time.c_str(), "2024/8/14 11:7:21.000000");
			time += Units::year(1);
			EXPECT_EQ(time.c_str(), "2025/8/14 11:7:21.000000");
			time += Units::year(1);
			EXPECT_EQ(time.c_str(), "2026/8/14 11:7:21.000000");

			time = 1709193600;
			if (!EXPECT_EQ(time.c_str(), "2024/2/29 0:0:0.000000")) std::cout << time << std::endl;
			time = 1709279999;
			if (!EXPECT_EQ(time.c_str(), "2024/2/29 23:59:59.000000")) std::cout << time << std::endl;
			time += 1;
			EXPECT_EQ(time.c_str(), "2024/3/1 0:0:0.000000");
		}
#endif

	}

	return 0;
};
#undef EXPECT_EQ
#undef EXPECT_NE