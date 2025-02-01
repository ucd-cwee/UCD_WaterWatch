// FibersDebugConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <string>
#include "../GoodLang/Any.h"
#include "../GoodLang/Proxy_Function.h"
#include "../GoodLang/ThreadSafeContainers.h"
#include "../GoodLang/Units.h"
#include "../GoodLang/DateTime.h"
#include "../GoodLang/Parallel.h"
#include "../WaterWatchCpp/Clock.h"

//#include "../FiberTasks/Fibers.h"
//#include "../FiberTasks/UnitsLibrary.h"
//#include "../FiberTasks/ScriptingLanguage2.h"
//#include <execution>
//#include "../WaterWatchCpp/Clock.h"
//#include "../FiberTasks/Actions2.h"
//class stackThing {
//public:
//	std::string varName;
//	fibers::Any var;
//	bool perform_cout;
//
//public:
//	stackThing() : varName(), var(), perform_cout{ true }{};
//	stackThing(std::string const& name) : varName(name), var(), perform_cout{ true } {};
//	template<typename T> stackThing(std::string const& name, T const& obj) : varName(name), var(obj), perform_cout{ true } {};
//	template<typename T> stackThing(std::string const& name, T&& obj) : varName(name), var(std::forward<T>(obj)), perform_cout{ true } {};
//	template<typename T> stackThing(std::string const& name, T&& obj, bool doCout) : varName(name), var(std::forward<T>(obj)), perform_cout(doCout) {};
//	stackThing(stackThing const& r) = default;
//	stackThing(stackThing&& r) = default;
//	stackThing& operator=(stackThing const& r) = default;
//	stackThing& operator=(stackThing&& r) = default;
//	~stackThing() { 
//		if (perform_cout && (!varName.empty())) {
//			std::cout << Units::printf("DELETING %s\n", varName.c_str());
//		}
//	};
//	int length() const { return varName.length(); };
//	std::string& get_var_name() { return varName; };
//	bool operator==(stackThing const& a) const { return varName == a.varName; };
//	bool operator!=(stackThing const& a) const { return varName != a.varName; };
//};
//class stackThing2 {
//public:
//	std::string varName;
//	GoodLang::Any var;
//	bool perform_cout;
//
//public:
//	stackThing2() : varName(), var(), perform_cout{ true }{};
//	stackThing2(std::string const& name) : varName(name), var(), perform_cout{ true } {};
//	template<typename T> stackThing2(std::string const& name, T const& obj) : varName(name), var(obj), perform_cout{ true } {};
//	template<typename T> stackThing2(std::string const& name, T&& obj) : varName(name), var(std::forward<T>(obj)), perform_cout{ true } {};
//	template<typename T> stackThing2(std::string const& name, T&& obj, bool doCout) : varName(name), var(std::forward<T>(obj)), perform_cout(doCout) {};
//	stackThing2(stackThing2 const& r) = default;
//	stackThing2(stackThing2&& r) = default;
//	stackThing2& operator=(stackThing2 const& r) = default;
//	stackThing2& operator=(stackThing2&& r) = default;
//	~stackThing2() {
//		if (perform_cout && (!varName.empty())) {
//			std::cout << Units::printf("DELETING %s\n", varName.c_str());
//		}
//	};
//	int length() const { return varName.length(); };
//	std::string& get_var_name() { return varName; };
//	bool operator==(stackThing const& a) const { return varName == a.varName; };
//	bool operator!=(stackThing const& a) const { return varName != a.varName; };
//};
//
//static bool Thing() { return true; };
//

// *
#define SINGLE_ARG(...) __VA_ARGS__
#define EXPECT_EQ_PRINTF(A,B) [a = (A), b = (B)]()->bool{ \
    if (a == b) { return true; } else { \
        std::string tempA{std::to_string(a)}, tempB{std::to_string(b)}; \
        std::cout << GoodLang::printf("FAILURE AT LINE %i: (%s != %s)\n", (int)__LINE__, tempA.c_str(), tempB.c_str()); \
    return false; } \
}()
#define print(a) std::cout << a << std::endl
#define EXPECT_EQ(a, b) if (a != b){ print(GoodLang::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#define EXPECT_NE(a, b) if (a == b){ print(GoodLang::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }

int main() {
	using namespace GoodLang;

	// pre-warm the heap
	for (int i = 0; i < 100000; i++) delete (new int(5));

	auto f{ GoodLang::Scripted_Type_Info() };

	auto& type_int = GoodLang::user_type<int>();
	type_int.IsBuiltInType();

	Any x = 100;
	print(x.TypeName());

	x = 500;

	utilities::FastAllocator<int> xxx;
	xxx.Alloc();

	(void)x.cast<int>();

	auto func = make_callable([](int x) -> int { return x * x; });
	TypeConverter converter;
	auto result = call(func, { 100 }, converter);
	EXPECT_EQ(true, result.IsTypeOf<int>());

	Functions F;
	F.emplace("foo", make_callable([](double x) -> double { return x * x; }), true);

	func = F.BuildMatch("foo", { 100.0 }, converter);
	result = call(func, { 100.0 }, converter);
	EXPECT_EQ(true, result.IsTypeOf<double>());

	Union<int, int> y;
	y.get<0>() += 1;

	DoubleWrapper xyz{ 5 };
	xyz += 1;
	xyz -= 2.0;
	xyz /= 5.0f;
	print(xyz);


	Units::UnitDefinition def;
	def.IsSI();
	print(def.HashCategory());

	Units::scalar S;
	S += 10;
	S /= 5;
	S = S + Units::scalar{ 55 };
	print(S);

	auto length = Units::foot{ 1 } + Units::meter{ 1 };
	print(length);


	auto length2 = Units::meter{ 1 } + Units::petameter{ 1 };
	print(length2);


	auto length3 = Units::meter{ 1 } + Units::femtometer{ 10000 };
	print(length3);

	print(Units::foot{ 5 }); // 5 ft

	print(Units::foot{ 5 } *Units::foot{ 5 }); // 25 sq_ft

	print(Units::foot{ 5 }.pow(2)); // 25 sq_ft

	print(Units::foot{ 5 }.pow(3)); // 125.000029 cu_ft

	print(Units::foot{ 5 } *Units::foot{ 5 } *Units::foot{ 5 }); // 125.000029 cu_ft

	print(Units::inch{ 5 } *Units::foot{ 5 } *Units::meter{ 5 }); // 109.848622 pk

	print(Units::foot{ 5 }.pow(2).pow(0.5)); // 5 ft

	print(Units::foot{ 5 }.pow(0.25)); // 1.111082 m^0.25

	auto rate = Units::gallon{ 5 } / Units::minute{ 1 };
	auto volume = rate * Units::year{ 1 };

	print(rate); // Units::gallon{ 5 } / Units::minute{ 1 }
	print(Units::gallon_per_minute{ 5 }); // identical to the manual type-calculation
	print(volume);
	print(Units::million_gallon{ volume });

	using namespace literals;

	print(1234_MG);
	print(1234_ac_ft);

	(void)(Units::gallon{ 5 } != Units::cubic_foot{ 5 });
	(void)(Units::gallon{ 50 } != Units::cubic_foot{ 50 });

	Units::mile h = 200_acre / 1.25_mi;
	print(h);

	Units::meters_per_second V0 = -12.0_mps;
	Units::meter X0 = 4.6_m;
	auto accelerationFormula = [](Units::second t) -> Units::meters_per_second_squared {
		auto a = 0.3_m / (1_s * 1_s * 1_s);
		Units::meters_per_second_squared b = 2.4_mps_sq;
		return (a * t) + b;
	};
	auto velocityFormula = [](Units::second t, Units::meters_per_second V0) -> Units::meters_per_second {
		auto a = 0.3_m / (1_s * 1_s * 1_s);
		Units::meters_per_second_squared b = 2.4_mps_sq;
		return (0.5 * a * t * t) + (b * t) + V0;
	};

	Units::meter pos = X0;
	Units::meters_per_second velocity = V0;
	Units::second timeStep = 0.1_s;
	for (Units::second t = 0; t < 8_s; t += timeStep) {
		velocity = velocityFormula(t, V0);

		auto posStr = pos.ToString();
		auto tStr = t.ToString();
		auto vStr = velocity.ToString();
		print(GoodLang::printf("pos = %s \t t = %s \t v = %s;", posStr.c_str(), tStr.c_str(), vStr.c_str()));

		pos += velocity * timeStep;
	}

	print((Units::constants::g() * 12_m / 2).sqrt()); // should be ~ 7.67 mps
	print((0_mps * 32.8_s) + (0.5 * 3.2_mps_sq * (32.8_s).pow(2))); // should be ~ 1720 m
	print((65_mps).pow(2) / (2 * 3_mps_sq)); // should be ~ 704 m

	(void)Units::value::GetValueTypes();

	print(DateTime::Now());
	print(DateTime::Now() + 30_d);
	print(DateTime::Now() + 365_yr);
	print(DateTime(2014, 1, 1));
	print((Units::second)(DateTime::Now() - DateTime(2014, 1, 1)));

	// Direct Invoke
	print(GoodLang::Job([](int x)->int { return x; }, 5).Invoke().cast<int>());
	// Async and Await
	print(GoodLang::Job([](int x)->int { return x; }, 5).AsyncInvoke().Wait_Get<int>());
	// Async, do stuff, then Await
	if (1) {
		auto group = GoodLang::Job([](int x)->int { return x; }, 5).AsyncInvoke();
		Sleep(10);
		print(group.Wait_Get<int>());
	}
	// 100 parallel jobs
	if (1) {
		Units::scalar V{ 0 };
		parallel::For(0, 100, [&](int i) {
			V++;
			});
		print(V);
	}
	// 100,000 parallel jobs
	if (1) {
		std::atomic<int> V{ 0 };
		Units::scalar V2{ 0 }; // At high levels of parallelism, the Units lock will choke the threads
		Stopwatch sw;

		sw.Start();
		parallel::For(0, 100000, [&](int i) {
			V++;
		});
		print(Units::second(sw.Stop_s()));

		sw.Start();
		parallel::For(0, 100000, [&](int i) {
			V2++;
		});
		print(Units::second(sw.Stop_s()));

		print(V);
		print(V2);
	}
	// 10,000,000 parallel jobs
	if (1) {
		Stopwatch sw;

		std::atomic<int> V{ 0 };
		Units::scalar V2{ 0 }; // At high levels of parallelism, the Units lock will choke the threads

		sw.Start();
		parallel::For(0, 10000000, [&](int i) {
			V++;
		});
		print(Units::second(sw.Stop_s()));

		sw.Start();
		parallel::For(0, 10000000, [&](int i) {
			V2++;
		});
		print(Units::second(sw.Stop_s()));

		print(V);
		print(V2);
	}
	// parallel jobs that each ALSO dispatch parallel jobs, for a total of 10,000,000 parallel jobs
	if (1) {
		std::atomic<int> V{ 0 };
		parallel::For(0, 100, [&](int i) {
			parallel::For(0, 100000, [&](int j) {
				V++;
			});
		});
		print(V);		
	}
	// catch exceptions thrown from inside of a job
	if (1) {
		Units::foot V{ 0 };
		try {
			parallel::For(0, 100, [&](int i) {
				parallel::For(0, 100000, [&](int j) {
					V += 5_gpm; // will fail due to incompatable units. 
				});
			});
			print(V);
		} catch (std::exception& e) {
			print(e.what());
		} // note that catching exceptions is a slow process (relatively), but is preferred to general crash. 
	}
	//// parallel jobs iterating over sequences and iterators
	if (1) {
		auto seq{ Sequence(0, 10000000, 1) }; // sequence is an iterator with little to no memory usage, but allows iterating on a counter
		std::atomic<int> V{ 0 };
		parallel::ForEach(seq, [&](int i) {
			V++;
		});
		print(V);
	}
	// iterate over Sequence wrapper, for basic count-from-0-to-10 operations
	if (1) {
		auto seq{ Sequence(10000000, 0, -1) }; // sequence is an iterator with little to no memory usage, but allows iterating on a counter
		std::atomic<int> V{ 0 };
		parallel::ForEach(seq, [&](int i) {
			V++;
		});
		print(V);
	}
	// iterate over IteratorSequence wrapper (allows looping over iterators randomly while tracking the correct index)
	if (1) {
		std::vector<std::string> data(100000, "TEST");
		auto seq{ IteratorSequence(data.begin(), data.end()) };
		std::atomic<int> V{ 0 };
		parallel::ForEach(seq, [&](std::pair<
			int, // index
			std::string* // data
		> const& i) {
			V++;
		});
		print(V);
	}
	// iterate over container
	if (1) {
		std::vector<std::string> data(100000, "TEST");
		std::atomic<int> V{ 0 };
		parallel::ForEach(data, [&](std::string const& i) {
			V++;
		});
		print(V);
	}

	// Map
	if (1) {
		GoodLang::Map<int, Units::scalar> map;
		*map[0] = 1;
		*map[1] = 2;

		parallel::For(0, 100000, [&](int i) {
			(void)map.size(); // OK
			map.try_emplace(i, i); // OK
			map.at(0)->operator++(); // OK
			*map[i] = i; // OK
			try {
				map.erase(100000.0 * (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX)));
			} catch (std::out_of_range&) {}

		});
		print(map.size());

		//parallel::ForEach(map, [&](auto& i) {
		//	print(i.second);
		//});


		
	}



#if 0
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

		// REVISED SCRIPTING LANGUAGE (starting over, to try and prevent random crashes)
		while (1) {
			auto printf = [](auto x) { std::cout << x << std::endl; };

			// Units
			if (1) {
				Units::value x = 10;
				Units::value y = 10;
				Units::foot z = 10;
				Units::meter w = 10;
				Units::gallon a = 10;

				auto xy = x + y;
				auto zx = z + x;
				auto xz = x + z;
				auto zz = z + z;
				auto zw = z + w;
				auto wz = w + z;
				try {
					auto za = z + a;
					EXPECT_EQ(true, false);
				}
				catch (...) {}

				//printf(x.ToString()); // 10
				//printf(y.ToString()); // 10
				//printf(z.ToString()); // 10 ft
				//printf(w.ToString()); // 10 m
				//printf(a.ToString()); // 10 gal
				//printf(xy.ToString()); // 20
				//printf(zx.ToString()); // 20 ft
				//printf(xz.ToString()); // 20 ft
				//printf(zz.ToString()); // 20 ft
				//printf(zw.ToString()); // 42.808399 ft
				//printf(wz.ToString()); // 13.048 m
			}

			// GoodLang::user_type
			if (1) {
				auto& voidType = GoodLang::user_type<void>();
				auto& intType = GoodLang::user_type<int>();
				auto& floatType = GoodLang::user_type<float>();
				auto& constFloatType = GoodLang::user_type<const float>();
				auto& constRefFloatType = GoodLang::user_type<const float&>();

				EXPECT_EQ(voidType, voidType);
				EXPECT_NE(voidType, intType);
				EXPECT_NE(intType, floatType);
				EXPECT_NE(floatType, constFloatType);
				EXPECT_EQ(constFloatType, constFloatType);
				EXPECT_NE(constFloatType, constRefFloatType);
				EXPECT_EQ(constRefFloatType.CanCast(floatType), false); // would be true if we could teach it how to make copies. 
				EXPECT_EQ(floatType.CanCast(constRefFloatType), true);  // const float& x = 0.0f; 			
				EXPECT_EQ(constRefFloatType.CanCast(constFloatType), true); // const float x = (const float&)0.0f; 
				EXPECT_EQ(constFloatType.CanCast(constRefFloatType), true); // const float& x = (const float)0.0f; 
			}

			// GoodLang::user_type_shared
			if (1) {
				auto voidType = GoodLang::user_type_shared<void>();
				auto intType = GoodLang::user_type_shared<int>();
				auto floatType = GoodLang::user_type_shared<float>();
				auto constFloatType = GoodLang::user_type_shared<const float>();
				auto constRefFloatType = GoodLang::user_type_shared<const float&>();

				EXPECT_EQ(voidType, voidType);
				EXPECT_NE(voidType, intType);
				EXPECT_NE(intType, floatType);
				EXPECT_NE(floatType, constFloatType);
				EXPECT_EQ(constFloatType, constFloatType);
				EXPECT_NE(constFloatType, constRefFloatType);

				if (1) {
					std::map< std::weak_ptr<GoodLang::Type_Info>, std::string> Map;
					Map[voidType] = "void";
					Map[intType] = "int";
					Map[floatType] = "float";
					Map[constFloatType] = "const float";
					Map[constRefFloatType] = "const float&";
					EXPECT_EQ(Map.size(), 5);
				}
				if (1) {
					std::set< std::weak_ptr<GoodLang::Type_Info>> Map;
					Map.insert(voidType);
					Map.insert(intType);
					Map.insert(floatType);
					Map.insert(constFloatType);
					Map.insert(constRefFloatType);
					EXPECT_EQ(Map.size(), 5);
				}
				if (1) {
					std::unordered_map< std::weak_ptr<GoodLang::Type_Info>, std::string> Map;
					Map[voidType] = "void";
					Map[intType] = "int";
					Map[floatType] = "float";
					Map[constFloatType] = "const float";
					Map[constRefFloatType] = "const float&";
					EXPECT_EQ(Map.size(), 5);
				}
				if (1) {
					concurrency::concurrent_unordered_map< std::weak_ptr<GoodLang::Type_Info>, std::string> Map;
					Map[voidType] = "void";
					Map[intType] = "int";
					Map[floatType] = "float";
					Map[constFloatType] = "const float";
					Map[constRefFloatType] = "const float&";
					EXPECT_EQ(Map.size(), 5);
				}
			}

			// test AnyData // WORKS
			if (1) {
				if (1) {
					auto instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<std::string>>("TEST1"));
					instanced_any->SetSelf(instanced_any);

					if (auto* ptr = instanced_any->cast<std::string>()) {
						EXPECT_EQ("TEST1", *ptr);
					}
					else {
						EXPECT_EQ(false, true);
					}
				}
				if (1) {
					auto instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<const std::string>>("TEST2"));
					instanced_any->SetSelf(instanced_any);

					if (auto* ptr = instanced_any->cast<const std::string>()) {
						EXPECT_EQ("TEST2", *ptr);
					}
					else {
						EXPECT_EQ(false, true);
					}
				}
				if (1) {
					auto instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<const std::string>>("TEST3"));
					instanced_any->SetSelf(instanced_any);

					if (auto* ptr = instanced_any->cast<std::string>()) {
						EXPECT_EQ(false, true);
					}
				}
				if (1) {
					auto instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<std::string>>("TEST4"));
					instanced_any->SetSelf(instanced_any);

					if (auto* ptr = instanced_any->cast<const std::string>()) {
						EXPECT_EQ("TEST4", *ptr);
					}
					else {
						EXPECT_EQ(false, true);
					}
				}
				if (1) {
					auto instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<std::string>>("TEST5"));
					instanced_any->SetSelf(instanced_any);

					if (auto ptr = instanced_any->cast_shared<std::string>()) {
						EXPECT_EQ("TEST5", *ptr);
					}
					else {
						EXPECT_EQ(false, true);
					}
				}
				if (1) {
					auto instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<const std::string>>("TEST6"));
					instanced_any->SetSelf(instanced_any);

					if (auto ptr = instanced_any->cast_shared<const std::string>()) {
						EXPECT_EQ("TEST6", *ptr);
					}
					else {
						EXPECT_EQ(false, true);
					}
				}
				if (1) {
					std::weak_ptr< GoodLang::AnyData> test;
					if (1) {
						std::shared_ptr<std::string> temp;
						if (1) {
							auto instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<std::string>>("TEST7"));
							instanced_any->SetSelf(instanced_any);

							temp = instanced_any->cast_shared<std::string>();
							test = instanced_any;
						}
						if (temp) {
							EXPECT_EQ("TEST7", *temp);
						}
						if (auto ptr = test.lock()) {

						}
						else {
							EXPECT_EQ(false, true);
						}
					}

					// this should now be out-of-scope and it should be destroyed. 
					if (auto ptr = test.lock()) {
						EXPECT_EQ(false, true);
					}
				}
			}

			// test Any (local instance) // WORKS
			if (1) {
				std::weak_ptr<GoodLang::AnyData> wrapped;
				if (1) {
					std::shared_ptr<std::string> string_ptr;
					if (1) {
						auto anyObj{ GoodLang::Any(std::string("TEST")) };
						wrapped = anyObj.impl();

						string_ptr = anyObj.cast<std::shared_ptr<std::string>>();
						if (string_ptr) {
							EXPECT_EQ(*string_ptr, "TEST");
						}

						EXPECT_EQ(anyObj.cast<std::string>(), "TEST");
						EXPECT_EQ(anyObj.cast<const std::string>(), "TEST");
						EXPECT_EQ((*anyObj.cast<std::string*>()), "TEST");
						EXPECT_EQ((*anyObj.cast<const std::string*>()), "TEST");
						EXPECT_EQ((anyObj.Type()), (GoodLang::user_type_shared<std::string>()));
						EXPECT_EQ((anyObj.Type()), (GoodLang::user_type<std::string>()));
						EXPECT_EQ((anyObj.TypeHash()), GoodLang::GetHash(GoodLang::user_type<std::string>()));
						EXPECT_EQ(wrapped.expired(), false);
					}
					if (string_ptr) {
						EXPECT_EQ(*string_ptr, "TEST");
					}
					EXPECT_EQ(wrapped.expired(), false); // wrapped is kept alive by its connection to the string_ptr, which itself is just referencing an object living inside of wrapped.
				}
				EXPECT_EQ(wrapped.expired(), true);
			}

			// test Any (shared impl) // WORKS
			if (1) {
				std::weak_ptr<GoodLang::AnyData> wrapped;
				if (1) {
					std::shared_ptr<std::string> string_ptr;
					if (1) {
						auto anyObj{ GoodLang::Any(std::make_shared<std::string>("TEST")) };
						wrapped = anyObj.impl();

						string_ptr = anyObj.cast<std::shared_ptr<std::string>>();
						if (string_ptr) {
							EXPECT_EQ(*string_ptr, "TEST");
						}

						EXPECT_EQ(anyObj.cast<std::string>(), "TEST");
						EXPECT_EQ(anyObj.cast<const std::string>(), "TEST");
						EXPECT_EQ((*anyObj.cast<std::string*>()), "TEST");
						EXPECT_EQ((*anyObj.cast<const std::string*>()), "TEST");
						EXPECT_EQ((anyObj.Type()), (GoodLang::user_type_shared<std::string>()));
						EXPECT_EQ((anyObj.Type()), (GoodLang::user_type<std::string>()));
						EXPECT_EQ((anyObj.TypeHash()), GoodLang::GetHash(GoodLang::user_type<std::string>()));
						EXPECT_EQ(wrapped.expired(), false);
					}
					if (string_ptr) {
						EXPECT_EQ(*string_ptr, "TEST");
					}
					EXPECT_EQ(wrapped.expired(), true); // wrapped is not kept alive since string_ptr is independant and has the data itself.
				}
				EXPECT_EQ(wrapped.expired(), true);
			}

			// test Any auto-casting and assignment // WORKS
			if (1) {
				std::shared_ptr<stackThing> a5;
				if (1) {
					GoodLang::Any anyObj;
					anyObj = stackThing("TEST1", bool(), false);

					stackThing& a2 = anyObj.cast();
					const stackThing& a3 = anyObj.cast();

					a5 = anyObj.cast<std::shared_ptr<stackThing>>();
				}
				if (1) {
					GoodLang::Any anyObj;
					anyObj = std::make_shared<stackThing>("TEST2", bool(), false);

					stackThing& a2 = anyObj.cast();
					const stackThing& a3 = anyObj.cast();
					a5 = anyObj.cast<std::shared_ptr<stackThing>>();
				}
				a5 = nullptr;
			}

			// test Any auto-casting and assignment on a threaded race-condition // WORKS
			if (1) {
				GoodLang::Any anyObj;
				fibers::parallel::For(0, 10000, [&anyObj](int i) {
					anyObj = stackThing(Units::printf("%i", i), i, false);

					GoodLang::Any copy = anyObj;
					GoodLang::Any copy2{ anyObj };
					GoodLang::Any copy3; copy3 = anyObj;

					stackThing& a1 = anyObj.cast<stackThing&>();

					auto caster = anyObj.cast();

					stackThing& a2 = caster; // note -- this is somewhat dangerous, since the Any object is bring overwritten in multiple threads.
					const stackThing& a3 = anyObj.cast();
					std::shared_ptr<stackThing> a4 = anyObj.cast();
					std::shared_ptr<stackThing> a5 = anyObj.cast<std::shared_ptr<stackThing>>();
				});
				fibers::parallel::For(0, 10000, [&anyObj](int i) {
					stackThing& a2 = anyObj.cast();
					const stackThing& a3 = anyObj.cast();
					std::shared_ptr<stackThing> a4 = anyObj.cast();
					std::shared_ptr<stackThing> a5 = anyObj.cast<std::shared_ptr<stackThing>>();
				});
			}

			// test Instance memory lifetime on a threaded race-condition with assignment of different class types // WORKS
			if (1) {
				fibers::containers::number<long> N{ 0 };
				fibers::containers::number<long> M{ 0 };
				GoodLang::Any anyObj;
				fibers::parallel::For(0, 10000, [&anyObj, &N, &M](int i) {
					// thread-safe to overwrite
					if (i % 2 == 0) {
						anyObj = Units::printf("%i", i);
					}
					else {
						anyObj = stackThing(Units::printf("%i", i), i, false);
					}

					// thread-safe to cast

					// If trying to cast to a reference, it may throw an error if the type doesn't match
					try {
						std::string& stringRef = anyObj.cast();
					}
					catch (...) {}

					// If trying to cast to a ptr, it may return nullptr if the type doesn't match
					std::string* stringPtr = anyObj.cast(); // note that this doesn't prevent the underlying object from being changed!

					// If trying to cast to a shared_ptr, it may return nullptr if the type doesn't match
					if (auto ptr = anyObj.cast<std::shared_ptr<std::string>>()) {
						// now that we have access to a shared_ptr, the ptr will remain valid for as long as I have it. The Any object may be changed still
						(void)ptr->c_str();
						N++;
					}
					if (auto ptr = anyObj.cast<std::shared_ptr<stackThing>>()) {
						// now that we have access to a shared_ptr, the ptr will remain valid for as long as I have it. The Any object may be changed still
						(void)ptr->get_var_name();
						M++;
					}

					});
				// printf(Units::printf("Num Valid String Ptrs: %i\nNum Valid StackThing Ptrs: %i\nTotal: %i", (int)N, (int)M, (int)(N+M)));
				EXPECT_EQ(N > (10000.0 * 0.4), true);
				EXPECT_EQ(M > (10000.0 * 0.4), true);
			}

			// test a custom type info 
			if (1) {
				std::weak_ptr<GoodLang::Type_Info> stdMapRefType;
				if (1) {
					auto stdMapType = std::make_shared<GoodLang::Scripted_Type_Info>("std", "map");
					stdMapType->SetSelf(stdMapType);

					stdMapRefType = stdMapType->MakeRef();
					auto stdMapConstRefType = stdMapRefType.lock()->MakeConst();

					auto obj{ GoodLang::DynamicObject(stdMapType) };

					auto voidType = GoodLang::user_type_shared<void>();
					auto intType = GoodLang::user_type_shared<int>();
					auto floatType = GoodLang::user_type_shared<float>();
					auto constFloatType = GoodLang::user_type_shared<const float>();
					auto constRefFloatType = GoodLang::user_type_shared<const float&>();

					EXPECT_EQ(voidType, voidType);
					EXPECT_NE(voidType, intType);
					EXPECT_NE(intType, floatType);
					EXPECT_NE(floatType, constFloatType);
					EXPECT_EQ(constFloatType, constFloatType);
					EXPECT_NE(constFloatType, constRefFloatType);
					EXPECT_NE(constFloatType, stdMapType);
					EXPECT_NE(stdMapType, stdMapRefType);

					if (1) {
						std::map< std::weak_ptr<GoodLang::Type_Info>, std::string> Map;
						Map[voidType] = "void";
						Map[intType] = "int";
						Map[floatType] = "float";
						Map[constFloatType] = "const float";
						Map[constRefFloatType] = "const float&";
						Map[stdMapType] = "stdMapType";
						Map[stdMapRefType] = "stdMapRefType";
						EXPECT_EQ(Map.size(), 7);
					}
					EXPECT_EQ("map", (stdMapType->name()));
					EXPECT_EQ("map&", (stdMapRefType.lock()->name()));
					EXPECT_EQ("const map&", (stdMapConstRefType.lock()->name()));

					EXPECT_EQ(false, stdMapRefType.expired());
				}
				EXPECT_EQ(true, stdMapRefType.expired());

			}

			// pretend that a function wants a "const ref" version of a type -- can we cast from our Type_Info to it? Can we detect it's different? 
			if (1) {
				// Built-In types offer unlimited lifetimes for their casts
				if (1) {
					std::weak_ptr<GoodLang::Type_Info> TypeConst;
					if (1) {
						auto Type = GoodLang::user_type_shared<float>();
						TypeConst = Type.lock()->MakeConst();
						auto TypeRef = Type.lock()->MakeRef();
						auto TypeConstRef = TypeConst.lock()->MakeRef();

						EXPECT_EQ(true, Type.lock()->CanCast(*TypeConstRef.lock()));
						EXPECT_EQ(true, Type.lock()->CanCast(*TypeConst.lock()));
						EXPECT_EQ(true, TypeRef.lock()->CanCast(*TypeConstRef.lock()));
						EXPECT_EQ(false, TypeConst.lock()->CanCast(*Type.lock()));
						EXPECT_EQ(false, Type.lock()->CanCast(*TypeRef.lock()));
					}
					EXPECT_EQ(false, TypeConst.expired());
				}
				// Scripted_Type_Info offer limited lifetimes, tied to the lifetime of the parent or original Scripted_Type_Info.
				if (1) {
					std::weak_ptr<GoodLang::Type_Info> TypeConst;
					if (1) {
						auto Type = std::make_shared<GoodLang::Scripted_Type_Info>("std", "map");
						Type->SetSelf(Type);

						TypeConst = Type->MakeConst();
						auto TypeRef = Type->MakeRef();
						auto TypeConstRef = TypeConst.lock()->MakeRef();

						EXPECT_EQ(true, Type->CanCast(*TypeConstRef.lock()));
						EXPECT_EQ(true, Type->CanCast(*TypeConst.lock()));
						EXPECT_EQ(true, TypeRef.lock()->CanCast(*TypeConstRef.lock()));
						EXPECT_EQ(false, TypeConst.lock()->CanCast(*Type));
						EXPECT_EQ(false, Type->CanCast(*TypeRef.lock()));
					}
					EXPECT_EQ(true, TypeConst.expired());
				}
			}

			// GoodLang::ParamTypes
			if (1) {
				using namespace GoodLang;

				auto types0 = ParamTypes();
				auto types1 = ParamTypes({ user_type_shared<int>() });
				auto types1c = ParamTypes({ user_type_shared<const int>() });
				auto types1cr = ParamTypes({ types1[0].lock()->MakeConst().lock()->MakeRef() });
				auto types2 = ParamTypes({ user_type_shared<int>(), user_type_shared<int>() });

				for (auto& type : types0) {}
				for (auto& type : types1) {}
				for (auto& type : types1c) {}
				for (auto& type : types1cr) {}
				for (auto& type : types2) {}

				EXPECT_EQ(types0.size(), 0);
				EXPECT_EQ(types1.size(), 1);
				EXPECT_EQ(types1c.size(), 1);
				EXPECT_EQ(types1cr.size(), 1);
				EXPECT_EQ(types2.size(), 2);

				EXPECT_EQ(types2.CanCast(types2), true);
				EXPECT_EQ(types2.CanCast(types0), true);
				EXPECT_EQ(types0.CanCast(types2), false);
				EXPECT_EQ(types1.CanCast(types0), true);
				EXPECT_EQ(types1.CanCast(types2), false);
				EXPECT_EQ(types1.CanCast(types1cr), true);
				EXPECT_EQ(types1.CanCast(types1c), true);
				EXPECT_EQ(types1c.CanCast(types1cr), true);
				EXPECT_EQ(types1cr.CanCast(types1c), true);
				EXPECT_EQ(types1cr.CanCast(types1), false);

				std::map< ParamTypes, int> test1;
				std::unordered_map< ParamTypes, int> test2;
				concurrency::concurrent_unordered_map< ParamTypes, int> test3;

				test1[types0] = 0;
				test1[types1] = 0;
				test1[types1c] = 0;
				test1[types1cr] = 0;
				test1[types2] = 0;
				EXPECT_EQ(test1.size(), 5);

				test2[types0] = 0;
				test2[types1] = 0;
				test2[types1c] = 0;
				test2[types1cr] = 0;
				test2[types2] = 0;
				EXPECT_EQ(test2.size(), 5);

				test3[types0] = 0;
				test3[types1] = 0;
				test3[types1c] = 0;
				test3[types1cr] = 0;
				test3[types2] = 0;
				EXPECT_EQ(test3.size(), 5);
			}

			// GoodLang::FunctionArgs
			if (1) {
				using namespace GoodLang;

				auto types0 = FunctionArgs();
				auto types1 = FunctionArgs(ParamTypes({ user_type_shared<int>() }));
				auto types1c = FunctionArgs(ParamTypes({ user_type_shared<const int>() }));
				auto types1cr = FunctionArgs(ParamTypes({ types1.Type(0).lock()->MakeConst().lock()->MakeRef() }));
				auto types2 = FunctionArgs(ParamTypes({ user_type_shared<int>(), user_type_shared<int>() }));

				for (auto& type : types0) {}
				for (auto& type : types1) {}
				for (auto& type : types1c) {}
				for (auto& type : types1cr) {}
				for (auto& type : types2) {}

				EXPECT_EQ(types0.size(), 0);
				EXPECT_EQ(types1.size(), 1);
				EXPECT_EQ(types1c.size(), 1);
				EXPECT_EQ(types1cr.size(), 1);
				EXPECT_EQ(types2.size(), 2);

				EXPECT_EQ(types2.CanCastTo(types2), true);
				EXPECT_EQ(types2.CanCastTo(types0), true);
				EXPECT_EQ(types0.CanCastTo(types2), false);
				EXPECT_EQ(types1.CanCastTo(types0), true);
				EXPECT_EQ(types1.CanCastTo(types2), false);
				EXPECT_EQ(types1.CanCastTo(types1cr), true);
				EXPECT_EQ(types1.CanCastTo(types1c), true);
				EXPECT_EQ(types1c.CanCastTo(types1cr), true);
				EXPECT_EQ(types1cr.CanCastTo(types1c), true);
				EXPECT_EQ(types1cr.CanCastTo(types1), false);

				std::map< FunctionArgs, int> test1;
				std::unordered_map< FunctionArgs, int> test2;
				concurrency::concurrent_unordered_map< FunctionArgs, int> test3;

				test1[types0] = 0;
				test1[types1] = 0;
				test1[types1c] = 0;
				test1[types1cr] = 0;
				test1[types2] = 0;
				EXPECT_EQ(test1.size(), 5);

				test2[types0] = 0;
				test2[types1] = 0;
				test2[types1c] = 0;
				test2[types1cr] = 0;
				test2[types2] = 0;
				EXPECT_EQ(test2.size(), 5);

				test3[types0] = 0;
				test3[types1] = 0;
				test3[types1c] = 0;
				test3[types1cr] = 0;
				test3[types2] = 0;
				EXPECT_EQ(test3.size(), 5);
			}

			// GoodLang::FunctionSignature
			if (1) {
				using namespace GoodLang;

				auto types0 = FunctionSignature();
				auto types1 = FunctionSignature(user_type_shared<int>(), ParamTypes({ user_type_shared<int>() }), "Namespace", "Name1");
				auto types1c = FunctionSignature(user_type_shared<int>(), ParamTypes({ user_type_shared<const int>() }), "Namespace", "Name1");
				auto types1cr = FunctionSignature(user_type_shared<int>(), ParamTypes({ user_type_shared<const int&>() }), "Namespace", "Name1");
				auto types2 = FunctionSignature(user_type_shared<int>(), ParamTypes({ user_type_shared<int>(), user_type_shared<int>() }), "Namespace", "Name1");

				for (auto& type : types0.Arguments()) {}
				for (auto& type : types1.Arguments()) {}
				for (auto& type : types1c.Arguments()) {}
				for (auto& type : types1cr.Arguments()) {}
				for (auto& type : types2.Arguments()) {}

				EXPECT_EQ(types0.Arguments().size(), 0);
				EXPECT_EQ(types1.Arguments().size(), 1);
				EXPECT_EQ(types1c.Arguments().size(), 1);
				EXPECT_EQ(types1cr.Arguments().size(), 1);
				EXPECT_EQ(types2.Arguments().size(), 2);

				EXPECT_EQ(types2.Arguments().CanCastTo(types2.Arguments()), true);
				EXPECT_EQ(types2.Arguments().CanCastTo(types0.Arguments()), true);
				EXPECT_EQ(types0.Arguments().CanCastTo(types2.Arguments()), false);
				EXPECT_EQ(types1.Arguments().CanCastTo(types0.Arguments()), true);
				EXPECT_EQ(types1.Arguments().CanCastTo(types2.Arguments()), false);
				EXPECT_EQ(types1.Arguments().CanCastTo(types1cr.Arguments()), true);
				EXPECT_EQ(types1.Arguments().CanCastTo(types1c.Arguments()), true);
				EXPECT_EQ(types1c.Arguments().CanCastTo(types1cr.Arguments()), true);
				EXPECT_EQ(types1cr.Arguments().CanCastTo(types1c.Arguments()), true);
				EXPECT_EQ(types1cr.Arguments().CanCastTo(types1.Arguments()), false);

				std::map< FunctionSignature, int> test1;
				std::unordered_map< FunctionSignature, int> test2;
				concurrency::concurrent_unordered_map< FunctionSignature, int> test3;

				test1[types0] = 0;
				test1[types1] = 0;
				test1[types1c] = 0;
				test1[types1cr] = 0;
				test1[types2] = 0;
				EXPECT_EQ(test1.size(), 5);

				test2[types0] = 0;
				test2[types1] = 0;
				test2[types1c] = 0;
				test2[types1cr] = 0;
				test2[types2] = 0;
				EXPECT_EQ(test2.size(), 5);

				test3[types0] = 0;
				test3[types1] = 0;
				test3[types1c] = 0;
				test3[types1cr] = 0;
				test3[types2] = 0;
				EXPECT_EQ(test3.size(), 5);
			}

			// GoodLang::details::Custom_Type_Conversion_Impl
			if (1) {
				using namespace GoodLang;

				auto type_converter = std::shared_ptr< GoodLang::details::Type_Conversion_Base >(new GoodLang::details::Custom_Type_Conversion_Impl([](int x)->double { return x; }));
				double answer1 = type_converter->convert(10).cast<double>();
				double answer2 = type_converter->convert(10).cast<const double>();
				double answer3 = type_converter->convert(10).cast<const double&>();
				const float* answer4 = type_converter->convert(10).cast<const float*>(); // will be null

				EXPECT_EQ(answer1, 10.0);
				EXPECT_EQ(answer2, 10.0);
				EXPECT_EQ(answer3, 10.0);
				EXPECT_EQ(answer4, nullptr);
			}

			// GoodLang::details::Static_Type_Conversion_Impl
			if (1) {
				using namespace GoodLang;

				auto type_converter = std::shared_ptr< GoodLang::details::Type_Conversion_Base >(new GoodLang::details::Static_Type_Conversion_Impl<int, double>());
				double answer1 = type_converter->convert(10).cast<double>();
				double answer2 = type_converter->convert(10).cast<const double>();
				double answer3 = type_converter->convert(10).cast<const double&>();
				const float* answer4 = type_converter->convert(10).cast<const float*>(); // will be null

				EXPECT_EQ(answer1, 10.0);
				EXPECT_EQ(answer2, 10.0);
				EXPECT_EQ(answer3, 10.0);
				EXPECT_EQ(answer4, nullptr);
			}

			// Any(int&)
			if (1) {
				using namespace GoodLang;
				int from = 1;
				auto int_Any = Any(from);
				int& t1 = int_Any.cast<int>();
				int& t2 = int_Any.cast<int&>();
				int const& t3 = int_Any.cast<const int&>();
				int* t4 = int_Any.cast<int*>();
				const int* t5 = int_Any.cast<const int*>();

				EXPECT_EQ(t1, 1);
				EXPECT_EQ(t2, 1);
				EXPECT_EQ(t3, 1);
				EXPECT_EQ(*t4, 1);
				EXPECT_EQ(*t5, 1);
			}

			// GoodLang::details::Dynamic_Type_Conversion_Impl
			if (1) {
				using namespace GoodLang;

				auto type_converter = std::shared_ptr< GoodLang::details::Type_Conversion_Base >(new GoodLang::details::Dynamic_Type_Conversion_Impl<GoodLang::exception::bad_any_cast, std::bad_cast>());

				auto from = GoodLang::exception::bad_any_cast(user_type_shared<double>(), user_type_shared<double>(), __LINE__);

				const std::bad_cast* answer3 = type_converter->convert(from).cast<const std::bad_cast*>();
				const float* answer4 = type_converter->convert(from).cast<const float*>(); // will be null

				EXPECT_NE(answer3, nullptr);
				EXPECT_EQ(answer4, nullptr);
			}

			// GoodLang::details::MakeConversionFunc
			if (1) {
				using namespace GoodLang;

				if (auto p = GoodLang::details::MakeConversionFunc([](int i) -> double { return i; })) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<double>(), 10.0);
					EXPECT_EQ(p->convert(10).cast<double>(), 10.0);
				}
				if (auto p = GoodLang::details::MakeConversionFunc([](int const& i) -> double { return i; })) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<double>(), 10.0);
					EXPECT_EQ(p->convert(10).cast<double>(), 10.0);
				}
				if (auto p = GoodLang::details::MakeConversionFunc([](int& i) -> double { return i; })) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<double>(), 10.0);
					EXPECT_EQ(p->convert(10).cast<double>(), 10.0);
				}
				if (auto p = GoodLang::details::MakeConversionFunc<int, double>()) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<double>(), 10.0);
					EXPECT_EQ(p->convert(10).cast<double>(), 10.0);
				}
				if (auto p = GoodLang::details::MakeConversionFunc<int&, double>()) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<double>(), 10.0);
					EXPECT_EQ(p->convert(10).cast<double>(), 10.0);
				}
				if (auto p = GoodLang::details::MakeConversionFunc<int const&, double>()) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<double>(), 10.0);
					EXPECT_EQ(p->convert(10).cast<double>(), 10.0);
				}
				if (auto p = GoodLang::details::MakeConversionFunc([](int i) -> std::string { return std::to_string(i); })) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<std::string>(), "10");
					EXPECT_EQ(p->convert(10).cast<std::string>(), "10");
				}

				// Any -> string
				if (auto p = GoodLang::details::MakeConversionFunc([](Any const& i) -> std::string { return i.TypeName(); })) {
					auto x = Any(10);
					p->convert_in_place(x);
					EXPECT_EQ(x.cast<std::string>(), "int");
					EXPECT_EQ(p->convert(10).cast<std::string>(), "int");
				}

				// just a simple pass-through. Should get what you give.
				if (auto p = GoodLang::details::MakeConversionFunc([](Any& i) -> Any { return i; })) {
					EXPECT_EQ(p->convert(10).IsTypeOf<int>(), true);
					EXPECT_EQ(p->convert(10.0).IsTypeOf<double>(), true);
					EXPECT_EQ(p->convert(10.0f).IsTypeOf<float>(), true);
				}

				// just a simple pass-through. Should get what you give.
				if (auto p = GoodLang::details::MakeConversionFunc([](std::shared_ptr<int> i) -> Any { return i; })) {
					EXPECT_EQ(p->convert(10).IsTypeOf<int>(), true);
					EXPECT_EQ(p->convert(10.0).IsTypeOf<double>(), false); // this will be false! It returns a nullptr, and a nullptr to Any should be empty...
					EXPECT_EQ(p->convert(10.0f).IsTypeOf<float>(), false); // this will be false! It returns a nullptr, and a nullptr to Any should be empty...
					EXPECT_EQ(p->convert(10.0f).IsTypeOf<int>(), false); // this will be false! It returns a nullptr, and a nullptr to Any should be empty...
					EXPECT_EQ(p->convert(nullptr).IsTypeOf<int>(), false); // this will be false! It returns a nullptr, and a nullptr to Any should be empty...

				}


			}

			// GoodLang::TypeConverter
			if (1) {
				using namespace GoodLang;

				TypeConverter tree;
				tree.AddConverter<int, char>();

				//printf(tree.print());

				tree.AddConverter<bool, int>();
				tree.AddConverter<float, double>();
				tree.AddConverter<fibers::synchronization::atomic_number<double>, double>();
				tree.AddConverter<int, float>();

				//printf(tree.print());

				tree.AddConverter([](int i) -> std::string { return std::to_string(i); });
				tree.AddConverter([](float i) -> std::string { return std::to_string(i); });
				tree.AddConverter([](double i) -> std::string { return std::to_string(i); });
				tree.AddConverter([](char i) -> std::string { return std::to_string(i); });
				tree.AddConverter([](bool i) -> std::string { return std::to_string(i); });
				tree.AddConverter([](fibers::synchronization::atomic_number<double> const& i) -> std::string { return std::to_string(i.GetValue()); });

				//printf(tree.print());

				EXPECT_EQ(tree.Convert<double>(10.0f), 10.0);
				EXPECT_EQ(tree.Convert<bool>(1), true);

				EXPECT_EQ(tree.Convert<double>(100), 100.0); // int -> float -> double
				EXPECT_EQ(tree.Convert<double>(true), 1.0); // bool -> int -> float -> double, which utilizes multiple daisy-chain conversions in a row. 
				EXPECT_EQ(tree.Convert<std::string>(10), "10");

				EXPECT_EQ(tree.Convert<std::string>(100.0), "100.000000");

				EXPECT_EQ(tree.Convert<double>(fibers::synchronization::atomic_number<double>(100)), 100.0); // int -> float -> double
				EXPECT_EQ(tree.Convert<std::string>(fibers::synchronization::atomic_number<double>(100)), "100.000000"); // int -> float -> double
			}
		
			// GoodLang::TypeConverter, with converters being added (and used) in multi-threaded context
			if (1) {
				using namespace GoodLang;
				try {
					TypeConverter tree;
					for (int i = 0; i < 10000; i++){//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 1; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 2; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 3; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );
				}
				catch (std::exception& e) {
					printf(std::string(e.what()) + " at line " + std::to_string(__LINE__));
				}
					
				// 

				try {
					TypeConverter tree;
					for (int i = 0; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 2: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 3: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 1; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 2: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 3: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 2; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 2: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 3: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 3; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 2: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 3: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );
				}
				catch (std::exception& e) {
					printf(std::string(e.what()) + " at line " + std::to_string(__LINE__));
				}

				//

				try {
					TypeConverter tree;
					for (int i = 0; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 3: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 1; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 3: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 2; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 3: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 3; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 3: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 0: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );
				}
				catch (std::exception& e) {
					printf(std::string(e.what()) + " at line " + std::to_string(__LINE__));
				}

				// 

				try {
					TypeConverter tree;
					for (int i = 0; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 1: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 1; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 1: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 2; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 1: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 3; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 1: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );
				}
				catch (std::exception& e) {
					printf(std::string(e.what()) + " at line " + std::to_string(__LINE__));
				}

				// 

				try {
					TypeConverter tree;
					for (int i = 0; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 0: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 1; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 0: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 2; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 0: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );

					for (int i = 3; i < 10000; i++) {//  fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 1: {
							tree.AddConverter<int, double>();
							EXPECT_EQ(tree.Convert<int>(10), 10);
							EXPECT_EQ(tree.Convert<const int>(10), 10);
							EXPECT_EQ(tree.Convert<const int&>(10), 10);
							EXPECT_EQ(tree.Convert<double>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<int>(10.0), 10)) {
								printf(tree.Convert<int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							if (!EXPECT_EQ(tree.Convert<const int>(10.0), 10)) {
								printf(tree.Convert<const int>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							EXPECT_EQ(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ(tree.Convert<double&>(10), 10.0);
							if (!EXPECT_EQ(tree.Convert<const int&>(10.0), 10)) {
								printf(tree.Convert<const int&>(10.0));
								printf(tree.print());
							}; // occassional, random failure (not throw, just not matched)
							break;
						}
						case 2: {
							tree.AddConverter<float, int>();
							EXPECT_EQ(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 0: {
							tree.AddConverter<bool, int>();
							EXPECT_EQ(tree.Convert<bool>(1), true);
							EXPECT_EQ(tree.Convert<int>(true), 1);
							EXPECT_EQ(tree.Convert<const int>(true), 1); //
							EXPECT_EQ(tree.Convert<const bool>(1), true);
							EXPECT_EQ(tree.Convert<bool&>(1), true);
							EXPECT_EQ(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							tree.AddConverter<bool, double>();
							EXPECT_EQ(tree.Convert<bool>(1.0), true);
							EXPECT_EQ(tree.Convert<double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					} // );
				}
				catch (std::exception& e) {
					printf(std::string(e.what()) + " at line " + std::to_string(__LINE__));
				}
			}
			if (1) {
				using namespace GoodLang;
				try {
					TypeConverter tree;
#define AddConv(a, b) tree.AddConverter<a, b>()
#define AddConvs(a) \
					AddConv(a, char); \
					AddConv(a, bool); \
					AddConv(a, int); \
					AddConv(a, long); \
					AddConv(a, float); \
					AddConv(a, long long); \
					AddConv(a, long double); \
					AddConv(a, double); \
					AddConv(a, unsigned int); \
					AddConv(a, unsigned long); \
					AddConv(a, fibers::synchronization::atomic_number<double>);
					
					AddConvs(char);
					AddConvs(bool);
					AddConvs(int);
					AddConvs(long);
					AddConvs(float);
					AddConvs(long long);
					AddConvs(long double);
					AddConvs(double);
					AddConvs(unsigned int);
					AddConvs(unsigned long);
					AddConvs(fibers::synchronization::atomic_number<double>);
#undef AddConvs
#undef AddConv
					tree.AddConverter([](char const& x)->std::string { return std::to_string(x); });
					tree.AddConverter([](int const& x)->std::string { return std::to_string(x); });
					tree.AddConverter([](long const& x)->std::string { return std::to_string(x); });
					tree.AddConverter([](float const& x)->std::string { return std::to_string(x); });
					tree.AddConverter([](double const& x)->std::string { return std::to_string(x); });
					tree.AddConverter([](std::string const& x)->int { return std::atol(x.c_str()); });
					tree.AddConverter([](std::string const& x)->long { return std::atol(x.c_str()); });
					tree.AddConverter([](std::string const& x)->float { return std::atof(x.c_str()); });
					tree.AddConverter([](std::string const& x)->double { return std::atof(x.c_str()); });

					EXPECT_EQ(100, tree.Convert<int>(tree.Convert<std::string>(100.0)));
					EXPECT_EQ(97, tree.Convert< fibers::synchronization::atomic_number<double>>('a').GetValue());
					EXPECT_EQ('a', tree.Convert<char>((long double)(int)('a')));

					fibers::parallel::For(0, 10000, [&](int i) { // 
						switch (i % 4) {
						case 0: {
							//tree.AddConverter<int, double>();
							EXPECT_EQ_PRINTF(tree.Convert<int>(10), 10);
							EXPECT_EQ_PRINTF(tree.Convert<const int>(10), 10);
							EXPECT_EQ_PRINTF(tree.Convert<const int&>(10), 10);
							EXPECT_EQ_PRINTF(tree.Convert<double>(10), 10.0);
							EXPECT_EQ_PRINTF(tree.Convert<int>(10.0), 10); // occassional, random failure (not throw, just not matched)
							EXPECT_EQ_PRINTF(tree.Convert<const int>(10.0), 10); // occassional, random failure (not throw, just not matched)
							EXPECT_EQ_PRINTF(tree.Convert<const double>(10), 10.0);
							EXPECT_EQ_PRINTF(tree.Convert<double&>(10), 10.0);
							EXPECT_EQ_PRINTF(tree.Convert<const int&>(10.0), 10); // occassional, random failure (not throw, just not matched)
							break;
						}
						case 1: {
							//tree.AddConverter<float, int>();
							EXPECT_EQ_PRINTF(tree.Convert<float>(10), 10.0f);
							EXPECT_EQ_PRINTF(tree.Convert<int>(10.0f), 10);
							EXPECT_EQ_PRINTF(tree.Convert<const int>(10.0f), 10);
							EXPECT_EQ_PRINTF(tree.Convert<const float>(10), 10.0f);
							EXPECT_EQ_PRINTF(tree.Convert<float&>(10), 10.0f);
							EXPECT_EQ_PRINTF(tree.Convert<const int&>(10.0f), 10);
							break;
						}
						case 2: {
							//tree.AddConverter<bool, int>();
							EXPECT_EQ_PRINTF(tree.Convert<bool>(1), true);
							EXPECT_EQ_PRINTF(tree.Convert<int>(true), 1);
							EXPECT_EQ_PRINTF(tree.Convert<const int>(true), 1); //
							EXPECT_EQ_PRINTF(tree.Convert<const bool>(1), true);
							EXPECT_EQ_PRINTF(tree.Convert<bool&>(1), true);
							EXPECT_EQ_PRINTF(tree.Convert<const int&>(true), 1); //
							break;
						}
						case 3: {
							//tree.AddConverter<bool, double>();
							EXPECT_EQ_PRINTF(tree.Convert<bool>(1.0), true);
							EXPECT_EQ_PRINTF(tree.Convert<double>(true), 1.0);
							EXPECT_EQ_PRINTF(tree.Convert<const double>(true), 1.0);
							EXPECT_EQ_PRINTF(tree.Convert<const bool>(1.0), true);
							EXPECT_EQ_PRINTF(tree.Convert<bool&>(1.0), true);
							EXPECT_EQ_PRINTF(tree.Convert<const double&>(true), 1.0);
							break;
						}
						}
					});
				}
				catch (std::exception& e) {
					printf(std::string(e.what()) + " at line " + std::to_string(__LINE__));
				}
			}
			
			// GoodLang::TypeConverter, correctly handling references as well as copies
			if (1) {
				using namespace GoodLang;
				TypeConverter tree;
#define AddConv(a, b) tree.AddConverter<a, b>()
#define AddConvs(a) \
					AddConv(a, char); \
					AddConv(a, bool); \
					AddConv(a, int); \
					AddConv(a, long); \
					AddConv(a, float); \
					AddConv(a, long long); \
					AddConv(a, long double); \
					AddConv(a, double); \
					AddConv(a, unsigned int); \
					AddConv(a, unsigned long); \
					AddConv(a, fibers::synchronization::atomic_number<double>);

				AddConvs(char);
				AddConvs(bool);
				AddConvs(int);
				AddConvs(long);
				AddConvs(float);
				AddConvs(long long);
				AddConvs(long double);
				AddConvs(double);
				AddConvs(unsigned int);
				AddConvs(unsigned long);
				AddConvs(fibers::synchronization::atomic_number<double>);
#undef AddConvs
#undef AddConv
				tree.AddConverter([](char const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](int const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](long const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](float const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](double const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](std::string const& x)->int { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->long { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->float { return std::atof(x.c_str()); });
				tree.AddConverter([](std::string const& x)->double { return std::atof(x.c_str()); });

				Any D = 0.0;
				Any D_ref = tree.Convert(D, user_type_shared<double&>().lock());
				D_ref.cast<double&>() += 1;

				Any D_copy = tree.Convert(D_ref, user_type_shared<double>().lock());
				D_copy.cast<double&>() += 1;

				EXPECT_EQ_PRINTF(D.cast<double&>(), 1);
				EXPECT_EQ_PRINTF(D_copy.cast<double&>(), 2);
			}

			// GoodLang::details::Explicit_Function_Impl
			if (1) {
				using namespace GoodLang;
				TypeConverter tree;
#define AddConv(a, b) tree.AddConverter<a, b>()
#define AddConvs(a) \
					AddConv(a, char); \
					AddConv(a, bool); \
					AddConv(a, int); \
					AddConv(a, long); \
					AddConv(a, float); \
					AddConv(a, long long); \
					AddConv(a, long double); \
					AddConv(a, double); \
					AddConv(a, unsigned int); \
					AddConv(a, unsigned long); \
					AddConv(a, fibers::synchronization::atomic_number<double>);

				AddConvs(char);
				AddConvs(bool);
				AddConvs(int);
				AddConvs(long);
				AddConvs(float);
				AddConvs(long long);
				AddConvs(long double);
				AddConvs(double);
				AddConvs(unsigned int);
				AddConvs(unsigned long);
				AddConvs(fibers::synchronization::atomic_number<double>);
#undef AddConvs
#undef AddConv
				tree.AddConverter([](char const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](int const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](long const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](float const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](double const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](std::string const& x)->int { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->long { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->float { return std::atof(x.c_str()); });
				tree.AddConverter([](std::string const& x)->double { return std::atof(x.c_str()); });

				if (1) {
					auto funcPtr = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Explicit_Function_Impl([](int& from)-> int { return from; }));
					EXPECT_EQ_PRINTF(funcPtr->operator()({ 10 }, tree).IsTypeOf<int>(), true);
					EXPECT_EQ_PRINTF(funcPtr->operator()({ 10.0 }, tree).IsTypeOf<int>(), true);
				}
				if (1) {
					auto funcPtr = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Explicit_Function_Impl([](int& from)-> int& { return from; }));
					EXPECT_EQ_PRINTF(funcPtr->operator()({ 10 }, tree).IsTypeOf<int>(), true);
					EXPECT_EQ_PRINTF(funcPtr->operator()({ 10.0 }, tree).IsTypeOf<int>(), true);

					Any result = funcPtr->operator()({ 10.0 }, tree);
					int& result_intRef = result.cast<int&>();
					EXPECT_EQ(result_intRef, 10);
				}
				if (1) {
					auto funcPtr = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Explicit_Function_Impl([](std::string& from)-> char& { return from[0]; }));
					Any from = std::string("TEST");

					EXPECT_EQ_PRINTF(funcPtr->operator()({ from }, tree).IsTypeOf<char>(), true);
					EXPECT_EQ_PRINTF(funcPtr->operator()({ from }, tree).IsTypeOf<char&>(), true);
					EXPECT_EQ_PRINTF(funcPtr->operator()({ from }, tree).cast<char&>(), 'T');
					Any result = funcPtr->operator()({ from }, tree);
					result.cast<char&>() = 'F';
					EXPECT_EQ(from.cast<std::string>(), "FEST");
					EXPECT_EQ(result.cast<char>(), 'F');


					Any result2 = funcPtr->operator()({ 10.0 }, tree);
					char& result_Ref = result2.cast<char>();
					EXPECT_EQ_PRINTF(result_Ref, '1');
				}
				// Test "Explicit_Function_Impl" supporting reference types by copying the function params and carrying them with the result, and NOT deleting it.
				if (1) {
					auto funcPtr = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Explicit_Function_Impl(
						[](std::string& from)-> char& { return from[0]; }
					));
					Any firstChar;
					{
						Any from = std::string("TEST");
						firstChar = funcPtr->operator()({ from }, tree);
					}
					EXPECT_EQ_PRINTF(firstChar.cast<char>(), 'T');
				}

				// Test "Explicit_Function_Impl" supporting reference types by copying the function params and carrying them with the result, and NOT deleting it.
				if (1) {
					auto varNameFunc = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Explicit_Function_Impl([](stackThing2& x) -> std::string& {
						return x.varName;
					}));
					auto varFunc = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Explicit_Function_Impl([](stackThing2& x) -> Any& {
						return x.var;
					}));

					Any obj = stackThing2("THING", 10, false); // make "true" if you want to see that this is correctly deleted.
					auto varNameObj = varNameFunc->operator()({ obj }, tree);

					auto varObj = varFunc->operator()({ obj }, tree);
					EXPECT_EQ(varNameObj.IsTypeOf<std::string>(), true);
					EXPECT_EQ(varObj.IsTypeOf<int>(), true);
					EXPECT_EQ(varNameObj.cast<std::string>(), "THING");
					EXPECT_EQ(varObj.cast<int>(), 10);
				}
				// Test "Attribute_Access_Impl" supporting reference types by copying the function params and carrying them with the result, and NOT deleting it.
				if (1) {
					auto varNameFunc = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Attribute_Access_Impl(&stackThing2::varName));
					auto varFunc = std::shared_ptr<GoodLang::details::Proxy_Function_Base>(new GoodLang::details::Attribute_Access_Impl(&stackThing2::var));

					Any obj = stackThing2("THING", 10, false); // make "true" if you want to see that this is correctly deleted.
					auto varNameObj = varNameFunc->operator()({ obj }, tree);

					auto varObj = varFunc->operator()({ obj }, tree);
					EXPECT_EQ(varNameObj.IsTypeOf<std::string>(), true);
					EXPECT_EQ(varObj.IsTypeOf<int>(), true);
					EXPECT_EQ(varNameObj.cast<std::string>(), "THING");
					EXPECT_EQ(varObj.cast<int>(), 10);
				}

				// Test "Member_Function_Impl" 
				if (1) {
					auto func_ptr = GoodLang::details::Member_Function_Impl(&stackThing2::length);
					Any obj = stackThing2("THING", 10, false);
					EXPECT_EQ(func_ptr->operator()({ obj }, tree).cast<int>(), 5);
					EXPECT_EQ(func_ptr->operator()({ obj }, tree).cast<int const&>(), 5);

					Any obj2 = func_ptr->operator()({ obj }, tree);
					EXPECT_EQ(obj2.cast<int>(), 5);
					EXPECT_EQ(obj2.cast<int const&>(), 5);
				}
				if (1) {
					auto func_ptr = GoodLang::details::Member_Function_Impl(&stackThing2::get_var_name);
					Any obj = stackThing2("THING", 10, false);
					EXPECT_EQ(func_ptr->operator()({ obj }, tree).cast<std::string>(), "THING");

					Any obj2 = func_ptr->operator()({ obj }, tree);
					EXPECT_EQ(obj2.cast<std::string>(), "THING");
				}

			}

			// make_callable(...), call(...)
			if (1) {
				using namespace GoodLang;
				TypeConverter tree;
#define AddConv(a, b) tree.AddConverter<a, b>()
#define AddConvs(a) \
					AddConv(a, char); \
					AddConv(a, bool); \
					AddConv(a, int); \
					AddConv(a, long); \
					AddConv(a, float); \
					AddConv(a, long long); \
					AddConv(a, long double); \
					AddConv(a, double); \
					AddConv(a, unsigned int); \
					AddConv(a, unsigned long); \
					AddConv(a, fibers::synchronization::atomic_number<double>);

				AddConvs(char);
				AddConvs(bool);
				AddConvs(int);
				AddConvs(long);
				AddConvs(float);
				AddConvs(long long);
				AddConvs(long double);
				AddConvs(double);
				AddConvs(unsigned int);
				AddConvs(unsigned long);
				AddConvs(fibers::synchronization::atomic_number<double>);
#undef AddConvs
#undef AddConv
				tree.AddConverter([](char const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](int const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](long const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](float const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](double const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](std::string const& x)->int { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->long { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->float { return std::atof(x.c_str()); });
				tree.AddConverter([](std::string const& x)->double { return std::atof(x.c_str()); });

				auto randFunc0 = make_callable([]()-> double { return 0.0; }); // 0
				auto randFunc1 = make_callable([]()-> double { return (double)std::rand() / (double)RAND_MAX; }); // rand 0-1
				auto randFunc2 = make_callable([](double const& max)-> double { return max * ((double)std::rand() / (double)RAND_MAX); }); // rand 0-1
				auto randFunc3 = make_callable([](double const& max, double const& min)-> double { return min + (max-min)*((double)std::rand() / (double)RAND_MAX); }); // rand 0-1

				EXPECT_EQ(tree.Convert<double>(randFunc0->operator()({}, tree)), 0.0);
				EXPECT_EQ((bool)randFunc1->operator()({}, tree), true);
				EXPECT_EQ((bool)randFunc1->operator()({ 1.0 }, tree), true);
				EXPECT_EQ((bool)randFunc1->operator()({ 1.0, 0.0 }, tree), true);
				EXPECT_EQ((bool)randFunc2->operator()({ 1.0 }, tree), true);
				EXPECT_EQ((bool)randFunc3->operator()({ 1.0, 0.0 }, tree), true);
				EXPECT_EQ((bool)randFunc1->operator()({}, tree), true);
				EXPECT_EQ((bool)randFunc2->operator()({ 1.0 }, tree), true);
				EXPECT_EQ((bool)randFunc3->operator()({ 1.0, 0.0 }, tree), true);
				EXPECT_EQ((bool)randFunc1->operator()({ 1 }, tree), true);
				EXPECT_EQ((bool)randFunc2->operator()({ 1 }, tree), true);
				EXPECT_EQ((bool)randFunc3->operator()({ 1, 0 }, tree), true);
				EXPECT_EQ((bool)randFunc3->operator()({ true, false }, tree), true);

				EXPECT_EQ((bool)call(randFunc1, {}, tree), true);
				EXPECT_EQ((bool)call(randFunc2, { 1.0 }, tree), true);
				EXPECT_EQ((bool)call(randFunc3, { 1.0, 0.0 }, tree), true);
				EXPECT_EQ((bool)call(randFunc1, { 1.0, 0.0 }, tree), true);
				EXPECT_EQ((bool)call(randFunc2, { 1.0, 0.0 }, tree), true);
				EXPECT_EQ((bool)call(randFunc3, { 1, 0 }, tree), true);
				EXPECT_EQ((bool)call(randFunc3, { true, false }, tree), true);

				auto get_var_name = make_callable(&stackThing2::get_var_name);
				auto length = make_callable(&stackThing2::length);
				auto varName = make_callable(&stackThing2::varName);
				auto var = make_callable(&stackThing2::var);

				Any obj = stackThing2("TEST", 100.0, false);

				Any varNameObj1 = get_var_name->operator()({ obj }, tree);
				EXPECT_EQ(varNameObj1.cast<std::string&>(), "TEST");

				Any varNameObj2 = get_var_name->operator()({ obj }, tree);
				EXPECT_EQ(varNameObj2.cast<std::string&>(), "TEST");

				Any varNameObj3 = get_var_name->operator()({ obj }, tree);
				EXPECT_EQ(varNameObj3.cast<std::string&>(), "TEST");

				// tree.AddConverter<stackThing2>();

				Any varNameObj = call(get_var_name, { obj }, tree);
				varNameObj.cast<std::string&>() = "TESTING";

				EXPECT_EQ(varNameObj1.cast<std::string&>(), "TESTING");
				EXPECT_EQ(varNameObj2.cast<std::string&>(), "TESTING");
				EXPECT_EQ(varNameObj3.cast<std::string&>(), "TESTING");

				EXPECT_EQ(call(var, { obj }, tree).IsTypeOf<double>(), true);
				EXPECT_EQ(call(varName, { obj }, tree).cast<std::string>(), "TESTING");
				EXPECT_EQ(tree.Convert<int>(call(length, { obj }, tree)), 7);
				
			}

			// Function & Functions
			if (1) {
				using namespace GoodLang;
				TypeConverter tree;
#define AddConv(a, b) tree.AddConverter<a, b>()
#define AddConvs(a) \
					AddConv(a, char); \
					AddConv(a, bool); \
					AddConv(a, int); \
					AddConv(a, long); \
					AddConv(a, float); \
					AddConv(a, long long); \
					AddConv(a, long double); \
					AddConv(a, double); \
					AddConv(a, unsigned int); \
					AddConv(a, unsigned long); \
					AddConv(a, fibers::synchronization::atomic_number<double>);

				AddConvs(char);
				AddConvs(bool);
				AddConvs(int);
				AddConvs(long);
				AddConvs(float);
				AddConvs(long long);
				AddConvs(long double);
				AddConvs(double);
				AddConvs(unsigned int);
				AddConvs(unsigned long);
				AddConvs(fibers::synchronization::atomic_number<double>);
#undef AddConvs
#undef AddConv
				tree.AddConverter([](char const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](int const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](long const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](float const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](double const& x)->std::string { return std::to_string(x); });
				tree.AddConverter([](std::string const& x)->int { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->long { return std::atol(x.c_str()); });
				tree.AddConverter([](std::string const& x)->float { return std::atof(x.c_str()); });
				tree.AddConverter([](std::string const& x)->double { return std::atof(x.c_str()); });

				Functions funcs;
				// emplace & at TEST in parallel
				fibers::parallel::For(0, 10000, [&](int i) {
					// emplace
					switch (i % 5) {
					case 0:
						funcs.emplace("Foo", Function(make_callable([]() -> double { return 0.0; }), false));
						break;
					case 1:
						// funcs.emplace("Foo", Function(make_callable([](int const& i) -> double { return i; }, tree), true)); // explicit only - no conversions
						funcs.emplace(
							"Foo"
							, Function(
								make_callable(
									[](Any const& i) -> Any { return (double)i.cast<int const&>(); } // call this function
									, ParamTypes({ user_type_shared<int const&>(), user_type_shared<double>() }) // use these params
								)
								, true // explicit only - no conversions
							)
						); 
						break;
					case 2:
						funcs.emplace("Foo", Function(make_callable([](double const& i) -> double { return i; }), true)); // explicit only - no conversions
						break;
					case 3:
						funcs.emplace("Foo", Function(make_callable([](float const& i) -> double { return i; }), true)); // explicit only - no conversions
						break;
					case 4:
						funcs.emplace("Foo", Function(make_callable([](Any const& i) -> Any { return i; }), false));
						break;
					};

					// at
					switch (std::rand() % 4) {
					case 0:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{}))) {
							(void)func->m_function->operator()(std::vector<Any>{  }, tree);
						}
						break;
					case 1:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{ 10 }))) {
							(void)func->m_function->operator()(std::vector<Any>{ 10 }, tree);
						}						
						break;
					case 2:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{ 10.0 }))) {
							(void)func->m_function->operator()(std::vector<Any>{ 10.0 }, tree);
						}						
						break;
					case 3:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{ 10.0f }))) {
							(void)func->m_function->operator()(std::vector<Any>{ 10.0f }, tree);
						}
						break;
					}
				});
				// at TEST in parallel
				fibers::parallel::For(0, 10000, [&](int i) {
					// at
					switch (std::rand() % 4) {
					case 0:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{}))) {
							(void)func->m_function->operator()(std::vector<Any>{  }, tree);
						}
						break;
					case 1:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{ 10 }))) {
							(void)func->m_function->operator()(std::vector<Any>{ 10 }, tree);
						}
						break;
					case 2:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{ 10.0 }))) {
							(void)func->m_function->operator()(std::vector<Any>{ 10.0 }, tree);
						}
						break;
					case 3:
						if (auto func = funcs.at("Foo", ParamTypes(std::vector<Any>{ 10.0f }))) {
							(void)func->m_function->operator()(std::vector<Any>{ 10.0f }, tree);
						}
						break;
					}
				});
				
				// build TEST
				if (auto func = funcs.BuildMatch("Foo", {}, tree)) {
					(void)func->operator()({}, tree);
				}
				if (auto func = funcs.BuildMatch("Foo", { 10.0 }, tree)) {
					EXPECT_EQ(10.0, tree.Convert<double>(func->operator()({ 10.0 }, tree)));
				}
				if (auto func = funcs.BuildMatch("Foo", { 10.0f }, tree)) {
					EXPECT_EQ(10.0f, tree.Convert<double>(func->operator()({ 10.0f }, tree)));
				}
				if (auto func = funcs.BuildMatch("Foo", { 10 }, tree)) {
					EXPECT_EQ(10, tree.Convert<double>(func->operator()({ 10 }, tree)));
				}
				if (auto func = funcs.BuildMatch("Foo", { 10l }, tree)) {
					EXPECT_EQ("long", (func->operator()({ 10l }, tree).TypeName()));
				}
				if (auto func = funcs.BuildMatch("Foo", { 10.0l }, tree)) {
					EXPECT_EQ("long double", (func->operator()({ 10.0l }, tree).TypeName()));
				}
				if (auto func = funcs.BuildMatch("Foo", { 'A' }, tree)) {
					EXPECT_EQ("char", (func->operator()({'A'}, tree).TypeName()));
				}

				// at TEST in parallel
				fibers::parallel::For(0, 100000, [&](int i) {
					// at
					switch (std::rand() % 9) {
					case 0:
						(void)funcs.Call("Foo", {}, tree);
						break;
					case 1:
						(void)funcs.Call("Foo", { 10 }, tree);
						break;
					case 2:
						(void)funcs.Call("Foo", { 10.0 }, tree);
						break;
					case 3:
						(void)funcs.Call("Foo", { 10.0f }, tree);
						break;
					case 4:
						(void)funcs.Call("Foo", { 10l }, tree);
						break;
					case 5:
						(void)funcs.Call("Foo", { 10.0l }, tree);
						break;
					case 6:
						(void)funcs.Call("Foo", { 'A' }, tree);
						break;
					case 7:
						(void)funcs.Call("Foo", { (long long)i }, tree);
						break;
					case 8:
						(void)funcs.Call("Foo", { (unsigned int)i }, tree);
						break;
					};
				});
			}

			// TypeConverter with template types
			if (1) {
				using namespace GoodLang;
				TypeConverter tree;
#define AddConv(a, b) tree.AddConverter<a, b>()
#define AddConvs(a) \
					AddConv(a, char); \
					AddConv(a, bool); \
					AddConv(a, int); \
					AddConv(a, long); \
					AddConv(a, float); \
					AddConv(a, long long); \
					AddConv(a, long double); \
					AddConv(a, double); \
					AddConv(a, unsigned int); \
					AddConv(a, unsigned long); \
					AddConv(a, fibers::synchronization::atomic_number<double>);

				AddConvs(char);
				AddConvs(bool);
				AddConvs(int);
				AddConvs(long);
				AddConvs(float);
				AddConvs(long long);
				AddConvs(long double);
				AddConvs(double);
				AddConvs(unsigned int);
				AddConvs(unsigned long);
				AddConvs(fibers::synchronization::atomic_number<double>);
#undef AddConvs
#undef AddConv

				tree.AddConverter([](Any const& rhs) -> Any {
					return std::to_string(rhs.cast<double>());
				}, user_type_shared<double const&>(), user_type_shared<std::string>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return std::to_string(rhs.cast<int>());
				}, user_type_shared<int const&>(), user_type_shared<std::string>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return std::to_string(rhs.cast<long>());
				}, user_type_shared<long const&>(), user_type_shared<std::string>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return std::to_string(rhs.cast<float>());
				}, user_type_shared<float const&>(), user_type_shared<std::string>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return std::to_string(rhs.cast<char>());
				}, user_type_shared<char const&>(), user_type_shared<std::string>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return (int)std::atol(rhs.cast<std::string>().c_str());
				}, user_type_shared<std::string const&>(), user_type_shared<int>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return (long)std::atol(rhs.cast<std::string>().c_str());
				}, user_type_shared<std::string const&>(), user_type_shared<long>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return (float)std::atof(rhs.cast<std::string>().c_str());
				}, user_type_shared<std::string const&>(), user_type_shared<float>());
				tree.AddConverter([](Any const& rhs) -> Any {
					return (double)std::atof(rhs.cast<std::string>().c_str());
				}, user_type_shared<std::string const&>(), user_type_shared<double>());

				EXPECT_EQ(100, tree.Convert<int>(tree.Convert<std::string>(100.0)));
				EXPECT_EQ(97, tree.Convert< fibers::synchronization::atomic_number<double>>('a').GetValue());
				EXPECT_EQ('a', tree.Convert<char>((long double)(int)('a')));
				EXPECT_EQ("100.000000", tree.Convert<std::string>(100.0));
				EXPECT_EQ(100, tree.Convert<float>(tree.Convert<std::string>(100.0)));
			}

			// automatic object copying
			if (1) {
				using namespace GoodLang;
				auto intType = user_type_shared<int>().lock();
				Any startingObj = (int)100;

				Any copiedObj = intType->GetCopyConstructor()(startingObj);

				EXPECT_EQ(startingObj.IsTypeOf<int>(), true); // ensure the basics
				EXPECT_EQ(copiedObj.IsTypeOf<int>(), true); // ensure copy succeeded
				EXPECT_EQ(copiedObj.cast<int>(), 100); // ensure copy succeeded
				startingObj.cast<int>() = 50;
				EXPECT_EQ(startingObj.cast<int>(), 50); // ensure change happened
				EXPECT_EQ(copiedObj.cast<int>(), 100); // should still be 100, as it's a seperate copy
			}

			// Scopes
			if (1) {
				using namespace GoodLang;
				fibers::containers::Map<std::string, std::shared_ptr<Global>> imports;

				auto scope_1 = std::make_shared<Global>(); // ::
				scope_1->SetSelf(scope_1);
				scope_1->AddBuiltIns();

				(void)scope_1->Cast<int>(100);
				auto intObj = scope_1->CallFunction("int", { 100 });
				auto doubleObj = scope_1->CallFunction("double", { intObj });
				auto stringObj = scope_1->Cast<std::string>(doubleObj);
				EXPECT_EQ("100.000000", stringObj);
				EXPECT_EQ("int", scope_1->Cast<std::string>(scope_1->Cast<std::weak_ptr<Type_Info>>(100)));
				EXPECT_EQ("ldouble", scope_1->Cast<std::string>(scope_1->Cast<std::weak_ptr<Type_Info>>(100.0l)));
				EXPECT_EQ("char", scope_1->Cast<std::string>(scope_1->Cast<std::weak_ptr<Type_Info>>('c')));

				EXPECT_EQ(20, scope_1->Cast<int>(scope_1->CallFunction("+", { 10.0, 10 })));

				auto ft_obj = scope_1->CallFunction("Units::foot", { 100.0 });
				auto m_obj = scope_1->CallFunction("Units::meter", { Units::foot(100.0) });
				EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("==", { scope_1->CallFunction("+", {ft_obj, m_obj}), Units::foot(200) })));


				auto DT = scope_1->CallFunction("DateTime::Now", {  });
				auto DT_time = scope_1->CallFunction("time", { DT });
				//printf(scope_1->Cast<std::string>(DT));
				//printf(scope_1->Cast<std::string>(DT_time));
				auto time1 = scope_1->Cast<Units::second>(DT_time);

				scope_1->CallFunction("+=", { DT, Units::day(7) });
				//printf(scope_1->Cast<std::string>(DT));
				//printf(scope_1->Cast<std::string>(DT_time));
				auto time2 = scope_1->Cast<Units::second>(DT_time);
				EXPECT_NE(time1, time2);
			}

			// Scopes, Namespaces, Classes
			if (1) {
				int numIterations = 100000;
				auto sw = Stopwatch();

				using namespace GoodLang;
				fibers::containers::Map<std::string, std::shared_ptr<Global>> imports;

				auto scope_1 = std::make_shared<Global>(); // ::
				scope_1->SetSelf(scope_1);
				scope_1->AddBuiltIns();

				// to_hash
				if (1) {					
					fibers::parallel::For(0, numIterations, [&](int i) {
						scope_1->Cast<size_t>(scope_1->CallFunction("to_hash", { 100 }));
						scope_1->Cast<size_t>(scope_1->CallFunction("to_hash", { std::string("TEST") }));
						scope_1->Cast<size_t>(scope_1->CallFunction("to_hash", { 100ll }));
						scope_1->Cast<size_t>(scope_1->CallFunction("to_hash", { Var(100) }));
						scope_1->Cast<size_t>(scope_1->CallFunction("to_hash", { Var(100) }));
					});
				}

				// to_string
				if (1) {
					fibers::parallel::For(0, numIterations, [&](int i) {
						scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { 100 }));
						scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { std::string("TEST") }));
						scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { 100ll }));
						scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { Var(100) }));
						scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { Var(100) }));
					});
				}

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
						if (auto namespacePtr = scope_1->FindClass(user_type_shared<std::string>())) {

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
					auto scope_outer = std::make_shared<Scope>(scope_1);
					scope_outer->SetSelf(scope_outer);
					fibers::parallel::For(0, numIterations, [&](int i) {
						auto scope_inner = std::make_shared<Scope>(scope_outer);
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

				EXPECT_EQ("100", scope_1->Cast<std::string>(scope_1->CallFunction("string", { 100 })));
				EXPECT_EQ("200", scope_1->Cast<std::string>(scope_1->CallFunction("::string", { scope_1->Cast<int>(scope_1->CallFunction("+", { 100.0f, 100.0 })) })));

				EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("!=", { scope_1->CallFunction("Type", { 100.0 }), scope_1->CallFunction("Type", { 100.0f }) })));
				EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("==", { scope_1->CallFunction("Type", { 100.0 }), scope_1->CallFunction("Type", { 100.0 }) })));

				// Units
				if (1) {
					// slowest
					if (1) {
						sw.Start();
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
#if 0
					if (1) {
						// Requires up-casting Units::foot to Units::value before calling 'abbreviation' and getting the result from the polymorphic type. 
						sw.Start();
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) { // for (int i = 0; i < numIterations;i++){// 
							auto scope_inner = std::make_shared<Scope>(scope_outer);
							scope_inner->SetSelf(scope_inner);
							scope_inner->AddObj("i", std::make_shared<Any>(i));
							{
								auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr

								auto [func, tree] = scope_inner->BuildFunction("Units::foot", { i_obj });
								if (func) {
									(void)call(func, { i_obj }, *tree);
								}
							}
							});
						printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
					}
#endif

					// Using Units;
					// for (int i = 0; i < numIterations; i++){
					//	   true == ("ft" == Units::foot(i).abbreviation());
					// }
					if (1) {
						// Requires up-casting Units::foot to Units::value before calling 'abbreviation' and getting the result from the polymorphic type. 
						sw.Start();
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) { // for (int i = 0; i < numIterations;i++){// 
							auto scope_inner = std::make_shared<Scope>(scope_outer);
							scope_inner->SetSelf(scope_inner);
							scope_inner->AddObj("i", std::make_shared<Any>(i));
							{
								auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr

								auto i_ft = scope_inner->CallFunction("Units::foot", { i_obj });
								//auto ft_abbrev = scope_inner->CallFunction("abbreviation", { i_ft });
								//EXPECT_EQ(scope_inner->Cast<std::string>(ft_abbrev), "ft");
							}
						});
						printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
					}

					// NO CRASH
					if (1) {
						// This one "uses" the Units namespace, to see if it provides a speed boost at all.
						sw.Start();
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
							scope_inner->SetSelf(scope_inner);
							scope_inner->AddObj("i", std::make_shared<Any>(i));
							{
								auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr
								auto i_ft = scope_inner->CallFunction("Units::foot", { i_obj });

								Any ft_abbrev;
								if (1) {
									std::vector<Any> params = { i_ft };
									auto tree = scope_inner->GetTypeConverterTree(); // builds and caches the tree. Updates the tree only if the situation has changed (new functions, new classes, or new Using statements)
									if (tree) {

									}
									else {
										throw std::runtime_error("Scope was invalid");
									}
								}
							}
							});
						printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
					}

					// CRASH AT BUILDING THE FUNCTION. Issue might be the conversion for foot -> value 
#if 0
					if (1) {
						// This one "uses" the Units namespace, to see if it provides a speed boost at all.
						sw.Start();
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
							scope_inner->SetSelf(scope_inner);
							scope_inner->AddObj("i", std::make_shared<Any>(i));
							{
								auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr
								auto i_ft = scope_inner->CallFunction("Units::foot", { i_obj });

								Any ft_abbrev;
								if (1) {
									std::vector<Any> params = { i_ft };
									auto m_conversionTree = scope_outer->GetTypeConverterTree(); // builds and caches the tree. Updates the tree only if the situation has changed (new functions, new classes, or new Using statements)
									if (m_conversionTree) {
										auto value_ref_converted = m_conversionTree->Convert(params[0], user_type_shared<Units::value&>());
										/*printf(*/value_ref_converted.cast< Units::value&>().UnitName();//);


										Proxy_Function func;

										// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
										if (!func) {
											Proxy_Function out{ nullptr };
											if (scope_outer->TryFindFunctionImpl("name", params, m_conversionTree, out)) {
												func = out;
											}
											else {
												func = nullptr;
											}
										}

										if (func) { //auto func = scope_inner->BuildFunction("name", params, m_conversionTree)) {
											//ft_abbrev = call(func, params, *tree);
										}
									}
									else {
										throw std::runtime_error("Scope was invalid");
									}
								}
							}
							});
						printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
					}
#endif

					// Using Units;
					// for (int i = 0; i < numIterations; i++){
					//	   true == ("foot" == Units::foot(i).name());
					// }
					if (1) {
						// This one "uses" the Units namespace, to see if it provides a speed boost at all.
						sw.Start();
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
							scope_inner->SetSelf(scope_inner);
							scope_inner->AddObj("i", std::make_shared<Any>(i));
							{
								auto i_obj = scope_inner->FindObj("i"); // searching and failing to find does not throw, but returns an empty ptr
								auto i_ft = scope_inner->CallFunction("Units::foot", { i_obj });
								auto ft_abbrev = scope_inner->CallFunction("name", { i_ft });
								// SOMETHING IS WRONG HERE, AND IT IS CRASHING... 
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
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
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
					EXPECT_EQ(true, (scope_1->CallFunction("DateTime", {}).IsTypeOf<DateTime>()));
					EXPECT_EQ(true, (scope_1->CallFunction("DateTime", { std::string("2024/12/5 18:58:59.576000") }).IsTypeOf<DateTime>()));
					EXPECT_EQ(true, (scope_1->CallFunction("DateTime", { scope_1->CallFunction("Units::second", { 1733454336 }) }).IsTypeOf<DateTime>()));

					EXPECT_EQ(true, (scope_1->CallFunction("DateTime", { 1733454336.0l }).IsTypeOf<DateTime>()));
					EXPECT_EQ(true, (scope_1->CallFunction("Units::second", { DateTime::Now() }).IsTypeOf<Units::second>()));
					EXPECT_EQ(true, (scope_1->CallFunction("Units::value", { DateTime::Now() }).IsTypeOf<Units::value>()));
					EXPECT_EQ(true, (scope_1->CallFunction("double", { DateTime::Now() }).IsTypeOf<double>()));

					//EXPECT_EQ(true, (scope_1->CallFunction("Now", { DateTime() }).IsTypeOf<DateTime>()));
					EXPECT_EQ(true, (scope_1->CallFunction("time", { DateTime::Now() }).IsTypeOf<Units::day>()));
					//EXPECT_EQ(true, (scope_1->CallFunction("DateTime::Now", { DateTime() }).IsTypeOf<DateTime>()));
					EXPECT_EQ(true, (scope_1->CallFunction("DateTime::Now", { }).IsTypeOf<DateTime>()));

					//printf(scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { scope_1->CallFunction("+", { scope_1->CallFunction("time", { DateTime::Now() }), scope_1->CallFunction("Units::year", { 1 }) }) })));
					//printf(scope_1->Cast<std::string>(scope_1->CallFunction("getNumDaysInSameMonth", { scope_1->CallFunction("DateTime::Now", {}) })));

					EXPECT_EQ(true, (scope_1->CallFunction("DateTime::Epoch", { }).IsTypeOf<DateTime>()));
					try {
						scope_1->CallFunction("tm_year", { }); // it will fail to find the function with those params and throw an error
						EXPECT_EQ(true, false);
					}
					catch (...) {}
					EXPECT_EQ(true, (scope_1->CallFunction("tm_year", { DateTime::Now() }).IsTypeOf<int>()));
					EXPECT_EQ(true, (scope_1->CallFunction("getNumDaysInSameMonth", { DateTime::Now() }).IsTypeOf<int>()));
					// EXPECT_EQ(true, (scope_1->CallFunction("getNumDaysInSameMonth", { DateTime::Now(), DateTime::Now() }).IsTypeOf<int>()));
					EXPECT_EQ(true, (scope_1->Cast<bool>(scope_1->CallFunction("==", { scope_1->CallFunction("DateTime::Epoch", { }), scope_1->CallFunction("DateTime::Epoch", { }) }))));
				}

				// Simulate a complex, multithreaded ForLoop
				{
					auto ScriptScope = std::make_shared<Scope>(scope_1);
					ScriptScope->SetSelf(ScriptScope);

					ScriptScope->AddObj("x", std::make_shared<Any>(fibers::containers::number<double>(0)));

					{
						auto ForScope = std::make_shared<Scope>(ScriptScope);
						ForScope->SetSelf(ForScope);

						fibers::parallel::For(0, 100, [&](int i) {
							auto LoopScope = std::make_shared<Scope>(ForScope);
							LoopScope->SetSelf(LoopScope);

							LoopScope->AddObj("i", std::make_shared<Any>((int)i));

							if (auto i_obj = LoopScope->FindObj("i")) {
								auto LengthObj = LoopScope->CallFunction("length", { // returns a size_t
									LoopScope->CallFunction("string", { // returns a string
										LoopScope->CallFunction("double", { // returns a double
											i_obj
										})
									})
								});

								if (auto x_obj = LoopScope->FindObj("x")) {
									LoopScope->CallFunction("+=", { *x_obj, LengthObj });
								}
							}
						});
					}
				}

				// Simulate a complex, multithreaded ForLoop which Throws a runtime error during one (or multiple) evaluations
				{
					auto ScriptScope = std::make_shared<Scope>(scope_1);
					ScriptScope->SetSelf(ScriptScope);

					ScriptScope->AddObj("x", std::make_shared<Any>(fibers::containers::number<double>(0)));

					{
						auto ForScope = std::make_shared<Scope>(ScriptScope);
						ForScope->SetSelf(ForScope);

						try {
							fibers::parallel::For(0, 100, [&](int i) {
								auto LoopScope = std::make_shared<Scope>(ForScope);
								LoopScope->SetSelf(LoopScope);

								LoopScope->AddObj("i", std::make_shared<Any>((int)i));

								if (auto i_obj = LoopScope->FindObj("i")) {
									auto LengthObj = LoopScope->CallFunction("length", { // returns a size_t
										LoopScope->CallFunction("string", { // returns a string
											LoopScope->CallFunction("double", { // returns a double
												i_obj
											})
										})
										});

									// printf(std::string("Length of ") + Impl::Cast<std::string>(*i_obj, LoopScope) + " is " + Impl::Cast<std::string>(LengthObj, LoopScope));

									if (auto x_obj = LoopScope->FindObj("x")) {
										if (LoopScope->Cast<bool>(LoopScope->CallFunction(">", { x_obj, 800 }))) {
											throw(std::runtime_error("x cannot be greater than 800 for some random reason!"));
										}

										LoopScope->CallFunction("+=", { *x_obj, LengthObj });
									}
								}
								});
							EXPECT_EQ(true, false); // we should not get here.
						}
						catch (std::runtime_error const& e) {}
					}
				}

				// Simulate a simple string operation
				{
					// {
					auto ScriptScope = std::make_shared<Scope>(scope_1); ScriptScope->SetSelf(ScriptScope);
					// var x = "A";
					ScriptScope->AddObj("x", std::make_shared<Any>(std::string("A")));
					// var y = "B";
					ScriptScope->AddObj("y", std::make_shared<Any>(std::string("B")));
					// return x + y;
					EXPECT_EQ("AB", (ScriptScope->Cast<std::string>(ScriptScope->CallFunction("+", { ScriptScope->FindObj("x"), ScriptScope->FindObj("y") }))));
					// }
				}

				// Simulate a simple Units operation
				{
					// {
					auto ScriptScope = std::make_shared<Scope>(scope_1); ScriptScope->SetSelf(ScriptScope);
					// Using namespace "Units"
					ScriptScope->AddUsing(ScriptScope->FindNamespace("Units"));
					// var x = foot(int(10.4));
					ScriptScope->AddObj("x", std::make_shared<Any>(ScriptScope->CallFunction("foot", { ScriptScope->CallFunction("int", { 10.4 }) })));
					// var y = meter(100);
					ScriptScope->AddObj("y", std::make_shared<Any>(ScriptScope->CallFunction("meter", { 100 })));
					// var z = inch(12);
					ScriptScope->AddObj("z", std::make_shared<Any>(ScriptScope->CallFunction("inch", { 12 })));
					// return Units::gallon(x*y*z);
					auto result = ScriptScope->CallFunction("gallon", { ScriptScope->CallFunction("*", { ScriptScope->CallFunction("*", { ScriptScope->FindObj("x"), ScriptScope->FindObj("y") }), ScriptScope->FindObj("z") }) });
					EXPECT_EQ("24542.398314 gal", (ScriptScope->Cast<std::string>(result)));
					// }
				}

				// Test some basic function calls on various types
				if (1) {
					sw.Start();
					// {}
					{					
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);

						// Using namespace "Units"
						scope_outer->AddUsing(scope_outer->FindNamespace("Units"));

						// parallel_for (int i = 0; i < numIterations; i++)
						try {
							fibers::parallel::For(0, numIterations, [&](int i) {
								auto scope_inner = std::make_shared<Scope>(scope_outer);
								scope_inner->SetSelf(scope_inner);
								scope_inner->AddObj("i", std::make_shared<Any>((const int)i)); // make scope aware of "i" as being available to it. It'll be a copy to prevent fucking about with the parallel function
							
								// var x = foot(i) * foot(i); // e.g. sq_ft
								scope_inner->AddObj("x", std::make_shared<Any>(scope_inner->CallFunction("*", {
									scope_inner->CallFunction("foot", { scope_inner->FindObj("i") }), 
									scope_inner->CallFunction("foot", { scope_inner->FindObj("i") })
								})));

								// var y = foot(i) * i; // e.g. ft
								scope_inner->AddObj("y", std::make_shared<Any>(scope_inner->CallFunction("*", {
									scope_inner->CallFunction("foot", { scope_inner->FindObj("i") }),
									scope_inner->FindObj("i")
								})));

								// x + y; // expect to fail!
								(void)scope_inner->CallFunction("+", { scope_inner->FindObj("x"), scope_inner->FindObj("y") });								
								EXPECT_EQ(true, false);
							});
						}
						catch (std::exception& e) {
							// catch error
							EXPECT_EQ(true, true);
						}
					}

					// {}
					{
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);

						// parallel_for (int i = 0; i < numIterations; i++)
						try {
							fibers::parallel::For(0, numIterations, [&](int i) {
								auto scope_inner = std::make_shared<Scope>(scope_outer);
								scope_inner->SetSelf(scope_inner);
								scope_inner->AddObj("i", std::make_shared<Any>((const int)i));

								// var x = Units::foot(i) * Units::foot(i); // e.g. sq_ft
								scope_inner->AddObj("x", std::make_shared<Any>(scope_inner->CallFunction("*", {
									scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") }),
									scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") })
								})));

								// var y = Units::foot(i) * i; // e.g. ft
								scope_inner->AddObj("y", std::make_shared<Any>(scope_inner->CallFunction("*", {
									scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") }),
									scope_inner->FindObj("i")
								})));

								// x + y; (expect to fail)
								(void)scope_inner->CallFunction("+", { scope_inner->FindObj("x"), scope_inner->FindObj("y") });
								EXPECT_EQ(true, false);
							});
						}
						catch (std::exception& e) {
							// catch error
							EXPECT_EQ(true, true);
						}
					}

					// {}
					{
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);

						// parallel_for (int i = 0; i < numIterations; i++)
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
							scope_inner->SetSelf(scope_inner);
							scope_inner->AddObj("i", std::make_shared<Any>((const int)i));

							// var x = Units::foot(i) + Units::foot(i); // e.g. ft
							scope_inner->AddObj("x", std::make_shared<Any>(scope_inner->CallFunction("+", {
								scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") }),
								scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") })
							})));

							// var y = Units::foot(i) * i; // e.g. ft
							scope_inner->AddObj("y", std::make_shared<Any>(scope_inner->CallFunction("*", {
								scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") }),
								scope_inner->FindObj("i")
							})));

							// x + y;
							(void)scope_inner->CallFunction("+", { scope_inner->FindObj("x"), scope_inner->FindObj("y") });
						});
					}

					// {}
					{
						auto scope_outer = std::make_shared<Scope>(scope_1);
						scope_outer->SetSelf(scope_outer);

						// parallel_for (int i = 0; i < numIterations; i++)
						fibers::parallel::For(0, numIterations, [&](int i) {
							auto scope_inner = std::make_shared<Scope>(scope_outer);
							scope_inner->SetSelf(scope_inner);
							scope_inner->AddObj("i", std::make_shared<Any>((const int)i));

							// var x = Units::foot(i) + Units::foot(i); // e.g. ft
							scope_inner->AddObj("x", std::make_shared<Any>(scope_inner->CallFunction("+", {
								scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") }),
								scope_inner->CallFunction("Units::foot", { scope_inner->FindObj("i") })
							})));

							// print(x.to_string);
							(void)scope_inner->Cast<std::string>(scope_inner->CallFunction("to_string", { scope_inner->FindObj("x") }));

							// std::string y = 10; -> std::string y; y = 10;
							scope_inner->AddObj("y", std::make_shared<Any>(scope_inner->CallFunction("string", {})));
							try {
								scope_inner->CallFunction("=", { scope_inner->FindObj("y"), 10 });
							}
							catch (std::exception& e) {
								printf(e.what());
							}
						});
					}

					printf(std::to_string(__LINE__) + ": \t" + std::to_string(numIterations) + " operations (on 10,000 classes) per " + Units::second(sw.Stop_s()).ToString() + ".");
				}

				// Test a basic custom class 
				if (1) {
					if (1) {
						auto ScopedObj = std::make_shared<Class>(scope_1, "ScopedObj");
						ScopedObj->SetSelf(ScopedObj);
						scope_1->AddChild(ScopedObj);

						// Default Constructors
						ScopedObj->AddDefaultConstructors();

						// Define the "member objects" for this class
						ScopedObj->DeclareMemberObject("name", user_type_shared<std::string>()); // if no default is provided, it will make its own at runtime
						ScopedObj->DeclareMemberObject("number", user_type_shared<double>()); // if no default is provided, it will make its own at runtime
						ScopedObj->DeclareMemberObject("value", user_type_shared<Var>()); // if no default is provided, it will make its own at runtime

						
					}

					auto instance = scope_1->CallFunction("ScopedObj", {});
					auto numberV = scope_1->CallFunction("number", { instance });
					EXPECT_EQ(0, scope_1->Cast<int>(numberV));
					scope_1->CallFunction("=", { numberV , 100 });
					EXPECT_EQ(100, scope_1->Cast<int>(numberV));
					auto anyV = scope_1->CallFunction("value", { instance });
					printf(anyV.Type().lock()->name());
					scope_1->CallFunction("=", { anyV, 100 });
					printf(anyV.Type().lock()->name());
					auto anyV2 = scope_1->CallFunction("value", { instance });
					EXPECT_EQ(true, anyV.cast<Var&>().p_data->IsTypeOf<int>());
					EXPECT_EQ(true, anyV2.cast<Var&>().p_data->IsTypeOf<int>());
					EXPECT_EQ(100, scope_1->Cast<int>(anyV));

					auto instance2 = scope_1->CallFunction("ScopedObj", { instance });
					auto anyV3 = scope_1->CallFunction("value", { instance2 });
					EXPECT_EQ(true, anyV3.cast<Var&>().p_data->IsTypeOf<int>());
					
					if (auto* VarContainerPtr = anyV3.container->cast<Var>()) {
						if (auto* p = &*VarContainerPtr->p_data) {
							if (int* p_int_ptr = p->container->cast<int>()) {
								printf(*p_int_ptr);
							}
							else {
								printf(p->container->GetType().name());
							}
						}
						else {
							printf("VarContainerPtr was empty");
						}
					}
					else {
						printf(anyV3.container->GetType().name());
					}


					const int& test = anyV3.cast<const int&>();
					printf(test);

					const int& test2 = anyV3.cast();
					printf(test2);

					try {
						scope_1->CallFunction("=", { anyV3, std::string("TEST") }); // fails, since was already an int
					}
					catch (...) {}

					// EXPECT_EQ(true, anyV3.cast<Var&>().p_data->IsTypeOf<std::string>());
					EXPECT_EQ(true, anyV.cast<Var&>().p_data->IsTypeOf<int>());
				}
				
				// Test an inherited custom class 
				if (1) {
					if (auto parentClass = scope_1->FindClass("ScopedObj")) {
						auto ScopedObj = std::make_shared<Class>(scope_1, "ScopedObj2", user_type_shared<void>().lock(), parentClass);
						ScopedObj->SetSelf(ScopedObj);
						scope_1->AddChild(ScopedObj);

						// Default Constructors
						ScopedObj->AddDefaultConstructors();

						// Define the "member objects" for this class
						//ScopedObj->DeclareMemberObject("name", user_type_shared<std::string>()); // if no default is provided, it will make its own at runtime
						//ScopedObj->DeclareMemberObject("number", user_type_shared<double>()); // if no default is provided, it will make its own at runtime
						//ScopedObj->DeclareMemberObject("value", user_type_shared<Var>()); // if no default is provided, it will make its own at runtime
					}

					auto instance = scope_1->CallFunction("ScopedObj2", {});
					auto numberV = scope_1->CallFunction("number", { instance });
					EXPECT_EQ(0, scope_1->Cast<int>(numberV));
					scope_1->CallFunction("=", { numberV , 100 });
					EXPECT_EQ(100, scope_1->Cast<int>(numberV));
					auto anyV = scope_1->CallFunction("value", { instance });
					scope_1->CallFunction("=", { anyV, 100 });
					auto anyV2 = scope_1->CallFunction("value", { instance });
					EXPECT_EQ(true, anyV.cast<Var&>().p_data->IsTypeOf<int>());
					EXPECT_EQ(true, anyV2.cast<Var&>().p_data->IsTypeOf<int>());
					EXPECT_EQ(100, scope_1->Cast<int>(anyV));

					auto instance2 = scope_1->CallFunction("ScopedObj2", { instance });
					auto anyV3 = scope_1->CallFunction("value", { instance2 });
					EXPECT_EQ(true, anyV3.cast<Var&>().p_data->IsTypeOf<int>());

					//scope_1->CallFunction("=", { anyV3, std::string("TEST") });
					//EXPECT_EQ(true, anyV3.cast<Var&>().p_data->IsTypeOf<std::string>());
					EXPECT_EQ(true, anyV.cast<Var&>().p_data->IsTypeOf<int>());
				}

				// Test a inherited-then-inherited custom class (e.g:)
				// class A{}
				// class B : A {}
				// class C : B {}
				if (1) {
					if (auto parentClass = scope_1->FindClass("ScopedObj2")) {
						auto ScopedObj = std::make_shared<Class>(scope_1, "ScopedObj3", user_type_shared<void>().lock(), parentClass);
						ScopedObj->SetSelf(ScopedObj);
						scope_1->AddChild(ScopedObj);

						// Default Constructors
						ScopedObj->AddDefaultConstructors();

						// Define the "member objects" for this class
						ScopedObj->DeclareMemberObject("name2", user_type_shared<std::string>(), std::make_shared<Any>(ScopedObj->GetName())); // if no default is provided, it will make its own at runtime
						//ScopedObj->DeclareMemberObject("number", user_type_shared<double>()); // if no default is provided, it will make its own at runtime
						//ScopedObj->DeclareMemberObject("value", user_type_shared<Var>()); // if no default is provided, it will make its own at runtime
					}

					auto instance = scope_1->CallFunction("ScopedObj3", {});
					auto numberV = scope_1->CallFunction("number", { instance });
					EXPECT_EQ(0, scope_1->Cast<int>(numberV));
					scope_1->CallFunction("=", { numberV , 100 });
					EXPECT_EQ(100, scope_1->Cast<int>(numberV));
					auto anyV = scope_1->CallFunction("value", { instance });
					scope_1->CallFunction("=", { anyV, 100 });
					auto anyV2 = scope_1->CallFunction("value", { instance });
					EXPECT_EQ(true, anyV.cast<Var&>().p_data->IsTypeOf<int>());
					EXPECT_EQ(true, anyV2.cast<Var&>().p_data->IsTypeOf<int>());
					EXPECT_EQ(100, scope_1->Cast<int>(anyV));

					auto name2 = scope_1->CallFunction("name2", { instance });
					auto equality = scope_1->CallFunction("==", { name2, std::string("ScopedObj3") });
					EXPECT_EQ(true, scope_1->Cast<bool>(equality));

					// printf(scope_1->Cast<std::string>(scope_1->CallFunction("name2", { instance })));

					auto instance2 = scope_1->CallFunction("ScopedObj3", { instance });
					auto anyV3 = scope_1->CallFunction("value", { instance2 });
					EXPECT_EQ(true, anyV3.cast<Var&>().p_data->IsTypeOf<int>());

					//scope_1->CallFunction("=", { anyV3, std::string("TEST") });
					//EXPECT_EQ(true, anyV3.cast<Var&>().p_data->IsTypeOf<std::string>());
					EXPECT_EQ(true, anyV.cast<Var&>().p_data->IsTypeOf<int>());

					EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("==", { scope_1->CallFunction("name2", { instance2 }), std::string("ScopedObj3") })));



				}

				// Test a multi-inherited custom class (e.g:)
				// class A{}
			    // class B{}
				// class C : A, B {}
				// Additionally, this demo's overloaded functions (from C) being accessed in parent classes (e.g. A and B) after casting
				if (1) {
					// Class A
					if (1) {
						if (1) {
							auto ScopedObj = std::make_shared<Class>(scope_1, "A");
							ScopedObj->SetSelf(ScopedObj);
							scope_1->AddChild(ScopedObj);

							// Default Constructors
							ScopedObj->AddDefaultConstructors();

							// Define the "member objects" for this class
							ScopedObj->DeclareMemberObject("name", user_type_shared<std::string>()); // if no default is provided, it will make its own at runtime
							// member functions
							ScopedObj->AddFunction("name_len", make_callable([selfPtr = std::weak_ptr<Class>(ScopedObj)](Any const& from) -> Any {
								if (auto self = selfPtr.lock()) {
									DynamicObject& From = from.cast<DynamicObject&>();
									return self->CallFunction("length", { From.m_objects->at("name") });
								}
								throw(exception::not_found_error("Custom class type was no longer available"));								
							}, ParamTypes({ ScopedObj->GetClassType().lock()->MakeConstRef() }), user_type_shared<size_t>() ));
						}

						auto instance = scope_1->CallFunction("A", {});
						auto nameV = scope_1->CallFunction("name", { instance });

						scope_1->CallFunction("=", { scope_1->CallFunction("name", { instance }), std::string("test")});
						EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction(">=", { scope_1->CallFunction("name_len", {instance }), 4 })));

					}
					// Class B
					if (1) {
						if (1) {
							auto ScopedObj = std::make_shared<Class>(scope_1, "B");
							ScopedObj->SetSelf(ScopedObj);
							scope_1->AddChild(ScopedObj);

							// Default Constructors
							ScopedObj->AddDefaultConstructors();

							// Define the "member objects" for this class
							ScopedObj->DeclareMemberObject("value", user_type_shared<double>()); // if no default is provided, it will make its own at runtime
							ScopedObj->DeclareMemberObject("var", user_type_shared<double>()); // if no default is provided, it will make its own at runtime

							ScopedObj->AddFunction("name_len2", make_callable([selfPtr = std::weak_ptr<Class>(ScopedObj)](Any const& from)->Any {
								if (auto self = selfPtr.lock()) {
									DynamicObject& From = from.cast<DynamicObject&>();
									return 100;
								}
								throw(exception::not_found_error("Custom class type was no longer available"));
							}, ParamTypes({ ScopedObj->GetClassType().lock()->MakeConstRef() }), user_type_shared<size_t>()));
						}

						auto instance = scope_1->CallFunction("B", {});
						auto valueV = scope_1->CallFunction("value", { instance });
						auto varV = scope_1->CallFunction("var", { instance }); // no cast -- and should NOT have returned a double.

						EXPECT_EQ(valueV.IsTypeOf<double>(), true);
						EXPECT_EQ(varV.IsTypeOf<double>(), true);
						EXPECT_EQ(100, scope_1->Cast<size_t>(scope_1->CallFunction("name_len2", { instance })));

					}
					// Class C
					if (1) {
						auto classA = scope_1->FindClass("A");
						auto classB = scope_1->FindClass("B");
						if (classA && classB) {
							auto ScopedObj = std::make_shared<Class>(scope_1, "C", user_type_shared<void>().lock(), std::vector<std::weak_ptr<Class>>{ classA, classB });
							ScopedObj->SetSelf(ScopedObj);
							scope_1->AddChild(ScopedObj);

							// Default Constructors
							ScopedObj->AddDefaultConstructors();

							// Define the "member objects" for this class
							ScopedObj->DeclareMemberObject("var", user_type_shared<Var>()); // if no default is provided, it will make its own at runtime

							ScopedObj->AddFunction("name_len2", make_callable([selfPtr = std::weak_ptr<Class>(ScopedObj)](Any const& from)->Any {
								if (auto self = selfPtr.lock()) {
									DynamicObject& From = from.cast<DynamicObject&>();
									return 500;
								}
								throw(exception::not_found_error("Custom class type was no longer available"));
							}, ParamTypes({ ScopedObj->GetClassType().lock()->MakeConstRef() }), user_type_shared<size_t>()));
						}

						auto instance = scope_1->CallFunction("C", {});
						auto nameV = scope_1->CallFunction("name", { instance }); // casts to A
						auto valueV = scope_1->CallFunction("value", { instance }); // casts to B
						auto varV = scope_1->CallFunction("var", { instance }); // no cast -- and should NOT have returned a double.

						EXPECT_EQ(valueV.IsTypeOf<double>(), true);
						EXPECT_EQ(varV.IsTypeOf<double>(), false);
						EXPECT_EQ(varV.IsTypeOf<Var>(), true);

						scope_1->CallFunction("=", { scope_1->CallFunction("name", { instance }), std::string("test") });
						EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction(">=", { scope_1->CallFunction("name_len", {instance }), 4 })));
						EXPECT_EQ(500, scope_1->Cast<size_t>(scope_1->CallFunction("name_len2", { instance })));
						
						// Demonstrate "overwritten" functions
						auto B_Cast = scope_1->CallFunction("B", { instance });
						auto varV_from_BCast = scope_1->CallFunction("var", { B_Cast }); 
						EXPECT_EQ(varV_from_BCast.IsTypeOf<double>(), false);
						EXPECT_EQ(varV_from_BCast.IsTypeOf<Var>(), true); 

						// Demonstrate calling specialized (e.g. from "C") functions from parent classes ("A")
						auto A_Cast = scope_1->CallFunction("A", { instance });
						auto varV_from_ACast = scope_1->CallFunction("var", { B_Cast }); // Normally not possible to call this since "A" does not have this function, but since the child does have it, it'll succeed.
						EXPECT_EQ(varV_from_ACast.IsTypeOf<double>(), false); 
						EXPECT_EQ(varV_from_ACast.IsTypeOf<Var>(), true);
					}
				}

				// the engine does NOT support inheriting from built-in types, and will silently fail if you do attempt to do this.
				
				// Namespace with classes e.g.
				// UI::StackPanel().Background.R = 255
				// except "Background" is a member object of UI::Panel, and requires automatic casting
				if (1) {
					if (1) {
						auto UI = std::make_shared<Namespace>(scope_1, "UI");
						UI->SetSelf(UI);
						scope_1->AddChild(UI);

						// add child classes or child namespaces
						auto Color = std::make_shared<Class>(UI, "Color");
						if (Color) {
							Color->SetSelf(Color);
							UI->AddChild(Color);

							Color->AddDefaultConstructors();

							// Define the "member objects" for this class
							Color->DeclareMemberObject("R", user_type_shared<float>());
							Color->DeclareMemberObject("G", user_type_shared<float>());
							Color->DeclareMemberObject("B", user_type_shared<float>());
							Color->DeclareMemberObject("A", user_type_shared<float>());

							Color->AddFunction("to_string", make_callable([selfPtr = std::weak_ptr<Class>(Color)](Any const& from) -> std::string {
								if (auto self = selfPtr.lock()) {
									DynamicObject& From = from.cast<DynamicObject&>();
									
									auto R_str = self->Cast<std::string>(self->CallFunction("to_string", { From.m_objects->at("R") }));
									auto G_str = self->Cast<std::string>(self->CallFunction("to_string", { From.m_objects->at("G") }));
									auto B_str = self->Cast<std::string>(self->CallFunction("to_string", { From.m_objects->at("B") }));
									auto A_str = self->Cast<std::string>(self->CallFunction("to_string", { From.m_objects->at("A") }));
									
									return Units::printf("{ %s, %s, %s, %s }"
										, R_str.c_str()
										, G_str.c_str()
										, B_str.c_str()
										, A_str.c_str()
									);
								}
								else throw(exception::not_found_error("Custom class type was no longer available"));
							}, ParamTypes({ Color->GetClassType().lock()->MakeConstRef() })));

							Color->AddFunction("max", make_callable([selfPtr = std::weak_ptr<Class>(Color)](Any const& from)-> float {
								if (auto self = selfPtr.lock()) {
									DynamicObject& From = from.cast<DynamicObject&>();

									auto R = self->Cast<float>(From.m_objects->at("R"));
									auto G = self->Cast<float>(From.m_objects->at("G"));
									auto B = self->Cast<float>(From.m_objects->at("B"));
									auto A = self->Cast<float>(From.m_objects->at("A"));

									return std::max(std::max(std::max(R, G), B), A);
								}
								else throw(exception::not_found_error("Custom class type was no longer available"));
							}, ParamTypes({ Color->GetClassType().lock()->MakeConstRef() })));
						}

						auto FrameworkElement = std::make_shared<Class>(UI, "FrameworkElement");
						if (FrameworkElement) {
							FrameworkElement->SetSelf(FrameworkElement);
							UI->AddChild(FrameworkElement);

							FrameworkElement->AddDefaultConstructors();

							// Define the "member objects" for this class
							FrameworkElement->DeclareMemberObject("UniqueName", user_type_shared<int>()); 
							FrameworkElement->DeclareMemberObject("Version", user_type_shared<int>(), std::make_shared<Any>(0));
							FrameworkElement->DeclareMemberObject("Opacity", user_type_shared<double>(), std::make_shared<Any>(-1.0));
							FrameworkElement->DeclareMemberObject("Width", user_type_shared<double>(), std::make_shared<Any>(-1.0));
							FrameworkElement->DeclareMemberObject("Height", user_type_shared<double>(), std::make_shared<Any>(-1.0));
							FrameworkElement->DeclareMemberObject("VerticalAlignment", user_type_shared<std::string>(), std::make_shared<Any>(std::string("Stretch")));
							FrameworkElement->DeclareMemberObject("HorizontalAlignment", user_type_shared<std::string>(), std::make_shared<Any>(std::string("Stretch")));
							FrameworkElement->DeclareMemberObject("Tag", user_type_shared<Var>());
							FrameworkElement->DeclareMemberObject("Name", user_type_shared<std::string>());
							FrameworkElement->DeclareMemberObject("MinWidth", user_type_shared<double>(), std::make_shared<Any>(-1.0));
							FrameworkElement->DeclareMemberObject("MinHeight", user_type_shared<double>(), std::make_shared<Any>(-1.0));
							FrameworkElement->DeclareMemberObject("MaxWidth", user_type_shared<double>(), std::make_shared<Any>(-1.0));
							FrameworkElement->DeclareMemberObject("MaxHeight", user_type_shared<double>(), std::make_shared<Any>(-1.0));
							FrameworkElement->DeclareMemberObject("Margin", user_type_shared<std::string>(), std::make_shared<Any>(std::string("0,0,0,0")));
							FrameworkElement->DeclareMemberObject("OnLoaded", user_type_shared<Var>()); // function as variable?
							FrameworkElement->DeclareMemberObject("OnUnloaded", user_type_shared<Var>()); // function as variable?

							FrameworkElement->AddFunction("Update", make_callable([selfPtr = std::weak_ptr<Class>(FrameworkElement)](Any const& from) {
								if (auto self = selfPtr.lock()) {
									DynamicObject& From = from.cast<DynamicObject&>();
									// do nothing?
								}else throw(exception::not_found_error("Custom class type was no longer available"));
							}, ParamTypes({ FrameworkElement->GetClassType().lock()->MakeConstRef() })));
						}

						auto Panel = std::make_shared<Class>(UI, "Panel", user_type_shared<void>().lock(), FrameworkElement);
						if (Panel) {
							Panel->SetSelf(Panel);
							UI->AddChild(Panel);

							Panel->AddDefaultConstructors();

							// Define the "member objects" for this class
							Panel->DeclareMemberObject("BorderBrush", Color->GetClassType());
							Panel->DeclareMemberObject("Background", Color->GetClassType());
							Panel->DeclareMemberObject("Padding", user_type_shared<std::string>(), std::make_shared<Any>(std::string("0,0,0,0")));
							Panel->DeclareMemberObject("BorderThickness", user_type_shared<std::string>(), std::make_shared<Any>(std::string("0,0,0,0")));
						}

						auto StackPanel = std::make_shared<Class>(UI, "StackPanel", user_type_shared<void>().lock(), Panel);
						if (StackPanel) {
							StackPanel->SetSelf(StackPanel);
							UI->AddChild(StackPanel);

							StackPanel->AddDefaultConstructors();

							// Define the "member objects" for this class
							StackPanel->DeclareMemberObject("Orientation", user_type_shared<std::string>(), std::make_shared<Any>(std::string("Vertical")));
							StackPanel->DeclareMemberObject("Spacing", user_type_shared<double>(), std::make_shared<Any>(0.0));
						}
					}

					if (1) {
						auto stackPanelInstance = scope_1->CallFunction("UI::StackPanel", {});
						auto stackPanelBackground = scope_1->CallFunction("Background", { stackPanelInstance });
						auto stackPanelBackground_R = scope_1->CallFunction("R", { stackPanelBackground });
						(void)scope_1->CallFunction("=", { stackPanelBackground_R, 255 });
						EXPECT_EQ(255, scope_1->Cast<int>(stackPanelBackground_R));
					}

					if (1) {
						auto tempScope = std::make_shared<Scope>(scope_1);
						tempScope->SetSelf(tempScope);

						auto stackPanelInstance = tempScope->CallFunction("UI::StackPanel", {});
						auto stackPanelBackground = tempScope->CallFunction("Background", { stackPanelInstance });
						auto stackPanelBackground_R = tempScope->CallFunction("R", { stackPanelBackground });
						(void)tempScope->CallFunction("=", { stackPanelBackground_R, 255 });
						EXPECT_EQ(255, scope_1->Cast<int>(stackPanelBackground_R));
					}
					
					if (1) {
						auto tempScope = std::make_shared<Scope>(scope_1);
						tempScope->SetSelf(tempScope);
						tempScope->AddUsing(tempScope->FindNamespace("UI"));

						auto stackPanelInstance = tempScope->CallFunction("UI::StackPanel", {});
						auto stackPanelBackground = tempScope->CallFunction("Background", { stackPanelInstance });
						auto stackPanelBackground_R = tempScope->CallFunction("R", { stackPanelBackground });
						(void)tempScope->CallFunction("=", { stackPanelBackground_R, 255 });
						EXPECT_EQ(255, scope_1->Cast<int>(stackPanelBackground_R));
					}

					if (1) {
						auto tempScope = std::make_shared<Scope>(scope_1);
						tempScope->SetSelf(tempScope);
						tempScope->AddUsing(tempScope->FindNamespace("UI"));

						auto stackPanelInstance = tempScope->CallFunction("StackPanel", {});
						auto stackPanelBackground = tempScope->CallFunction("Background", { stackPanelInstance });
						auto stackPanelBackground_R = tempScope->CallFunction("R", { stackPanelBackground });
						(void)tempScope->CallFunction("=", { stackPanelBackground_R, 255 });
						EXPECT_EQ(255, scope_1->Cast<int>(stackPanelBackground_R));
					}
				}

				// pair, Var testing
				if (1) {
					auto Instance = scope_1->CallFunction("pair", { 100, 200 });

					printf(scope_1->Cast<int>(scope_1->CallFunction("first", { Instance })));
					scope_1->CallFunction("=", { scope_1->CallFunction("first", {Instance}), 50 });
					printf(scope_1->Cast<int>(scope_1->CallFunction("first", { Instance })));

					printf(scope_1->Cast<int>(scope_1->CallFunction("int", { scope_1->CallFunction("first", {Instance}) })));

					printf(scope_1->Cast<std::string>(scope_1->CallFunction("name", { scope_1->CallFunction("Type_Info", { scope_1->CallFunction("first", { Instance }) }) })));




					// printf(scope_1->Cast<int const&>(scope_1->CallFunction("first", { Instance })));

					printf(scope_1->Cast<int>(scope_1->CallFunction("first", { Instance })));
					printf(scope_1->Cast<int>(scope_1->CallFunction("second", { Instance })));

					auto Instance2 = scope_1->CallFunction("int", { 0 });
					scope_1->CallFunction("=", { Instance2, scope_1->CallFunction("first", { Instance }) });
					printf(scope_1->Cast<int>(Instance2));

					if (scope_1->Cast<bool>(scope_1->CallFunction("try_reset", { scope_1->CallFunction("first", { Instance }) }))) {
						printf("try_reset succeeded");
					}
					else {
						printf("try_reset failed");
					}

					scope_1->CallFunction("=", { scope_1->CallFunction("first", { Instance }), std::string("TEST")});

					printf(scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { scope_1->CallFunction("first", { Instance }) })));
				}

				// Var test with custom class
				if (1) {
					if (1) {
						// Var x;
						auto VarInstance = scope_1->CallFunction("Var", {});
						// x = UI::StackPanel();
						scope_1->CallFunction("=", { VarInstance, scope_1->CallFunction("UI::StackPanel", {}) });
						// x.Background.R = 255;
						scope_1->CallFunction("=", { scope_1->CallFunction("R", { scope_1->CallFunction("Background", { VarInstance }) }), 255 });
						// assert(x.Background.R == 255);
						EXPECT_EQ(255, scope_1->Cast<int>(scope_1->CallFunction("R", { scope_1->CallFunction("Background", { VarInstance }) })));
					}
					if (1) {
						// Var x = UI::StackPanel();
						auto VarInstance = scope_1->CallFunction("Var", { scope_1->CallFunction("UI::StackPanel", {}) });
						// x.Background.R = 255;
						scope_1->CallFunction("=", { scope_1->CallFunction("R", { scope_1->CallFunction("Background", { VarInstance }) }), 255 });
						// assert(x.Background.R == 255);
						EXPECT_EQ(255, scope_1->Cast<int>(scope_1->CallFunction("R", { scope_1->CallFunction("Background", { VarInstance }) })));
					}
					if (1) {
						// Var(UI::StackPanel()).Background.R = 255;
						scope_1->CallFunction("=", { scope_1->CallFunction("R", { scope_1->CallFunction("Background", { scope_1->CallFunction("Var", { scope_1->CallFunction("UI::StackPanel", {}) }) }) }), 255 });
					}
				}

				// Map. Allows mixed-type keys and values. Keys must support the to_hash function. Be careful mixing key-types, though, since there's no guarrantee the hashes won't overlap.
				if (1) {
					// Map x;
					auto Instance = scope_1->CallFunction("Map", {});
					// x[100] = "TEST";
					scope_1->CallFunction("=", { scope_1->CallFunction("[]", { Instance, 100 }), std::string("TEST")});
					// x["TEST"] = 100;
					scope_1->CallFunction("=", { scope_1->CallFunction("[]", { Instance, std::string("TEST") }), 100 });
					// x["TEST"] = 200;
					scope_1->CallFunction("=", { scope_1->CallFunction("[]", { Instance, std::string("TEST") }), 200 });
					// auto v = x.at(100);
					auto ObjAtKey = scope_1->CallFunction("at", { Instance, 100 });
					// print(v);
					printf(scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { ObjAtKey })));
					// print(x);
					printf(scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { Instance })));
					// print(to_hash(x));
					printf(scope_1->Cast<size_t>(scope_1->CallFunction("to_hash", { Instance })));

					auto iter = scope_1->CallFunction("begin", { Instance });
					auto iter_end = scope_1->CallFunction("end", { Instance });
					while (scope_1->Cast<bool>(scope_1->CallFunction("!=", { iter, iter_end }))) {
						printf(
							scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { 
								scope_1->CallFunction("first", { scope_1->CallFunction("get", { iter }) }) 
							}))
						);
						std::cout << "\t";
						printf(
							scope_1->Cast<std::string>(scope_1->CallFunction("to_string", {
								scope_1->CallFunction("second", { iter })
							}))
						);
						scope_1->CallFunction("++", { iter });
					}
				}

#if 0
				// Set. Allows mixed-type keys. Keys must support the to_hash function. Be careful mixing key-types, though, since there's no guarrantee the hashes won't overlap.
				if (1) {
					// Map x;
					auto Instance = scope_1->CallFunction("Set", {});

					scope_1->CallFunction("emplace", { Instance, 100 });
					scope_1->CallFunction("emplace", { Instance, std::string("TEST") });
					scope_1->CallFunction("emplace", { Instance, std::string("TESTING") });
					try {
						scope_1->CallFunction("emplace", { Instance, 200.0 }); // will fail, because doubles are not hashable
						EXPECT_EQ(true, false);
					} catch (...) {}

					printf(scope_1->Cast<bool>(scope_1->CallFunction("contains", { Instance, std::string("TESTING") })));

					// print(x);
					printf(scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { Instance })));

					// print(to_hash(x));
					printf(scope_1->Cast<size_t>(scope_1->CallFunction("to_hash", { Instance })));

					auto iter = scope_1->CallFunction("begin", { Instance });
					auto iter_end = scope_1->CallFunction("end", { Instance });
					for (; scope_1->Cast<bool>(scope_1->CallFunction("!=", { iter, iter_end })); (void)scope_1->CallFunction("++", { iter })) {
						printf(
							scope_1->Cast<std::string>(scope_1->CallFunction("to_string", {
								scope_1->CallFunction("get", { iter })
							}))
						);
					}
				}
#endif








			}
		}
		














#if 0
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
#endif

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

#if 0
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
#endif

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

		// Re-build 2 Test
		try {
			using namespace scripting;
			auto printf = [](auto x) { std::cout << x << std::endl; };
			Stopwatch sw{};

			fibers::containers::Map<std::string, std::shared_ptr<Global2>> imports;

			auto scope_1 = std::make_shared<Global2>(); // ::
			scope_1->SetSelf(scope_1);
			scope_1->AddBuiltIns();

			int numIterations = 100000;

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

			EXPECT_EQ("100", scope_1->Cast<std::string>(scope_1->CallFunction("string", { 100 })));
			EXPECT_EQ("200", scope_1->Cast<std::string>(scope_1->CallFunction("::string", { scope_1->Cast<int>(scope_1->CallFunction("+", { 100.0f, 100.0 })) })));

			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 100 })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 100.0f })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 100.0 })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { std::string("TEST") })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { 'A' })));
			//printf(scope_1->Cast<std::string>(scope_1->CallFunction("Type", { Units::acre(1) })));

			EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("!=", { scope_1->CallFunction("Type", { 100.0 }), scope_1->CallFunction("Type", { 100.0f }) })));
			EXPECT_EQ(true, scope_1->Cast<bool>(scope_1->CallFunction("==", { scope_1->CallFunction("Type", { 100.0 }), scope_1->CallFunction("Type", { 100.0 }) })));

			// Units
			if (1) {
				// #include "Units"
				// Trying to have Units be built-in, but this still demo's how to make a seperate library and make it included as-if it were built-in. 

				// Conversion Tree Test
				{
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<int, double>()));
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<double, int>()));
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<double, Units::value>()));
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<int, Units::value>()));
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<double, Units::foot>()));
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<int, Units::foot>()));
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<Units::foot, double>()));
					EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<Units::foot, int>()));

					std::shared_ptr<Type_Converter_Tree> tempTree = std::make_shared<Type_Converter_Tree>();
					scope_1->CreateTypeConverterTree(tempTree/*, false*/);
					std::vector<scripting::Type_Info> tempResult;
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<int>(), user_type<double>(), tempResult)));
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<double>(), user_type<int>(), tempResult)));
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<int>(), user_type<Units::value>(), tempResult)));
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<Units::value>(), user_type<Units::foot>(), tempResult)));
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<double>(), user_type<Units::foot>(), tempResult)));
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<int>(), user_type<Units::foot>(), tempResult)));
					EXPECT_EQ(false, (tempTree->TryCreateConversionPath(user_type<std::string>(), user_type<int>(), tempResult))); // int(string) is not built-in
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<int>(), user_type<DateTime>(), tempResult)));
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<double>(), user_type<DateTime>(), tempResult)));
					EXPECT_EQ(false, (tempTree->TryCreateConversionPath(user_type<std::string>(), user_type<DateTime>(), tempResult))); // DateTime(string) is explicit, and must be requested directly
					EXPECT_EQ(true, (tempTree->TryCreateConversionPath(user_type<DateTime>(), user_type<std::string>(), tempResult)));

					// return (string)(DateTime)(double)(Units::second)DateTime::Now();
					(void)tempTree->Convert< std::string>(tempTree->Convert< DateTime >((double)(Units::second)DateTime::Now()));
				}

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
				EXPECT_EQ(true, (scope_1->CallFunction("DateTime", Function_Params{}).IsTypeOf<DateTime>()));
				EXPECT_EQ(true, (scope_1->CallFunction("DateTime", { std::string("2024/12/5 18:58:59.576000") }).IsTypeOf<DateTime>()));
				EXPECT_EQ(true, (scope_1->CallFunction("DateTime", { scope_1->CallFunction("Units::second", { 1733454336 }) }).IsTypeOf<DateTime>()));

				EXPECT_EQ(true, (scope_1->GetTypeConverterTree()->Converts<long double, DateTime>()));

				EXPECT_EQ(true, (scope_1->CallFunction("DateTime", { 1733454336.0l }).IsTypeOf<DateTime>()));
				EXPECT_EQ(true, (scope_1->CallFunction("Units::second", { DateTime::Now() }).IsTypeOf<Units::second>()));
				EXPECT_EQ(true, (scope_1->CallFunction("Units::value", { DateTime::Now() }).IsTypeOf<Units::value>()));
				EXPECT_EQ(true, (scope_1->CallFunction("double", { DateTime::Now() }).IsTypeOf<double>()));

				EXPECT_EQ(true, (scope_1->CallFunction("Now", { DateTime() }).IsTypeOf<DateTime>()));
				EXPECT_EQ(true, (scope_1->CallFunction("time", { DateTime::Now() }).IsTypeOf<Units::day>())); 
				EXPECT_EQ(true, (scope_1->CallFunction("DateTime::Now", { DateTime() }).IsTypeOf<DateTime>()));
				EXPECT_EQ(true, (scope_1->CallFunction("DateTime::Now", Function_Params{ }).IsTypeOf<DateTime>()));
				
				(void)scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { scope_1->CallFunction("+", { scope_1->CallFunction("time", { DateTime::Now() }), scope_1->CallFunction("Units::year", { 1 }) }) }));
				
				EXPECT_EQ(true, (scope_1->CallFunction("DateTime::Epoch", Function_Params{ }).IsTypeOf<DateTime>()));
				try {
					scope_1->CallFunction("tm_year", Function_Params{ }); // it will fail to find the function with those params and throw an error
					EXPECT_EQ(true, false);
				}
				catch (...) {}
				EXPECT_EQ(true, (scope_1->CallFunction("tm_year", { DateTime::Now() }).IsTypeOf<int>()));
				EXPECT_EQ(true, (scope_1->CallFunction("getNumDaysInSameMonth", { DateTime::Now() }).IsTypeOf<int>()));
				EXPECT_EQ(true, (scope_1->Cast<bool>(scope_1->CallFunction("==", { scope_1->CallFunction("DateTime::Epoch", Function_Params{ }), scope_1->CallFunction("DateTime::Epoch", Function_Params{ }) }))));
			}

			// Simulate a complex, multithreaded ForLoop
			{
				auto ScriptScope = std::make_shared<Scope2>(scope_1);
				ScriptScope->SetSelf(ScriptScope);

				ScriptScope->AddObj("x", std::make_shared<Any>(fibers::containers::number<double>(0)));

				{
					auto ForScope = std::make_shared<Scope2>(ScriptScope);
					ForScope->SetSelf(ForScope);

					fibers::parallel::For(0, 100, [&](int i) {
						auto LoopScope = std::make_shared<Scope2>(ForScope);
						LoopScope->SetSelf(LoopScope);

						LoopScope->AddObj("i", std::make_shared<Any>((int)i));

						if (auto i_obj = LoopScope->FindObj("i")) {
							auto LengthObj = LoopScope->CallFunction("length", { // returns a size_t
								LoopScope->CallFunction("string", { // returns a string
									LoopScope->CallFunction("double", { // returns a double
										i_obj
									})
								})
							});

							//printf(std::string("Length of ") + LoopScope->Cast<std::string>(i_obj) + " is " + LoopScope->Cast<std::string>(LengthObj));

							if (auto x_obj = LoopScope->FindObj("x")) {
								LoopScope->CallFunction("+=", { *x_obj, LengthObj });
							}
						}
					});
				}
			}

			// Simulate a complex, multithreaded ForLoop which Throws a runtime error during one (or multiple) evaluations
			{
				auto ScriptScope = std::make_shared<Scope2>(scope_1);
				ScriptScope->SetSelf(ScriptScope);

				ScriptScope->AddObj("x", std::make_shared<Any>(fibers::containers::number<double>(0)));

				{
					auto ForScope = std::make_shared<Scope2>(ScriptScope);
					ForScope->SetSelf(ForScope);

					try {
						fibers::parallel::For(0, 100, [&](int i) {
							auto LoopScope = std::make_shared<Scope2>(ForScope);
							LoopScope->SetSelf(LoopScope);

							LoopScope->AddObj("i", std::make_shared<Any>((int)i));

							if (auto i_obj = LoopScope->FindObj("i")) {
								auto LengthObj = LoopScope->CallFunction("length", { // returns a size_t
									LoopScope->CallFunction("string", { // returns a string
										LoopScope->CallFunction("double", { // returns a double
											i_obj
										})
									})
								});

								// printf(std::string("Length of ") + Impl::Cast<std::string>(*i_obj, LoopScope) + " is " + Impl::Cast<std::string>(LengthObj, LoopScope));

								if (auto x_obj = LoopScope->FindObj("x")) {
									if (LoopScope->Cast<bool>(LoopScope->CallFunction(">", { x_obj, 800 }))) {
										throw(std::runtime_error("x cannot be greater than 800 for some random reason!"));
									}

									LoopScope->CallFunction("+=", { *x_obj, LengthObj });
								}
							}
						});
						EXPECT_EQ(true, false); // we should not get here.
					}
					catch (std::runtime_error const& e) {}
				}
			}

			// Simulate a simple string operation
			{
				// {
				auto ScriptScope = std::make_shared<Scope2>(scope_1); ScriptScope->SetSelf(ScriptScope);
				// var x = "A";
				ScriptScope->AddObj("x", std::make_shared<Any>(std::string("A")));
				// var y = "B";
				ScriptScope->AddObj("y", std::make_shared<Any>(std::string("B")));
				// return x + y;
				EXPECT_EQ("AB", (ScriptScope->Cast<std::string>(ScriptScope->CallFunction("+", {ScriptScope->FindObj("x"), ScriptScope->FindObj("y")}))));
				// }
			}

			// Simulate a simple Units operation
			{
				// {
				auto ScriptScope = std::make_shared<Scope2>(scope_1); ScriptScope->SetSelf(ScriptScope);
				// Using namespace "Units"
				ScriptScope->AddUsing(ScriptScope->FindNamespace("Units"));
				// var x = foot(int(10.4));
				ScriptScope->AddObj("x", std::make_shared<Any>(ScriptScope->CallFunction("foot", { ScriptScope->CallFunction("int", { 10.4 }) })));
				// var y = meter(100);
				ScriptScope->AddObj("y", std::make_shared<Any>(ScriptScope->CallFunction("meter", { 100 })));
				// var z = inch(12);
				ScriptScope->AddObj("z", std::make_shared<Any>(ScriptScope->CallFunction("inch", { 12 })));
				// return Units::gallon(x*y*z);
				auto result = ScriptScope->CallFunction("gallon", { ScriptScope->CallFunction("*", { ScriptScope->CallFunction("*", { ScriptScope->FindObj("x"), ScriptScope->FindObj("y") }), ScriptScope->FindObj("z") }) });
				EXPECT_EQ("24542.398314 gal", (ScriptScope->Cast<std::string>(result)));
				// }
			}

			// Test a user-defined type ... 
			if (1) {
				auto UI_Namespace = std::make_shared<Namespace2>(scope_1, "UI");
				UI_Namespace->SetSelf(UI_Namespace);
				scope_1->AddChild(UI_Namespace);

				if (1) {
					// ... which imports the "Interactive" namespace...
					auto interactive_namespace{ std::make_shared<Class2>(UI_Namespace, "Interactive") }; {
						interactive_namespace->SetSelf(interactive_namespace);
						UI_Namespace->AddChild(interactive_namespace);

						{
							auto impl_namespace{ std::make_shared<Namespace2>(interactive_namespace, "InteractiveImpl") };
							impl_namespace->SetSelf(impl_namespace);
							interactive_namespace->AddChild(impl_namespace);
						}

						// self-construct
						interactive_namespace->AddFunction(interactive_namespace->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(interactive_namespace)]()->fibers::DynamicObject {
							if (auto classPtr = thisClass.lock()) {
								auto out{ fibers::DynamicObject(classPtr->GetClassType()) };
								/* add stuff to the dynamic object ... */
								return out;
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
						}));
						// construct from other
						interactive_namespace->AddFunction(interactive_namespace->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(interactive_namespace)](Any const& rhs)->fibers::DynamicObject {
							if (auto classPtr = thisClass.lock()) {
								fibers::DynamicObject& RHS = rhs.cast();
								fibers::DynamicObject LHS(RHS.m_classType); {
									for (auto& obj : RHS.m_objects) {
										if (obj.second) {
											if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
												LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
											}
											else {
												LHS.m_objects[obj.first] = obj.second;
											}
										}
										else {
											LHS.m_objects[obj.first] = std::make_shared<Any>();
										}
									}
								}
								return LHS;
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
						}), scripting::Param_Types({ { "rhs", interactive_namespace->GetClassType() } }));
						// copy other
						interactive_namespace->AddFunction("=", make_callable([thisClass = std::weak_ptr<Class2>(interactive_namespace)](std::shared_ptr<Any> lhs, Any const& rhs)->std::shared_ptr<Any> {
							if (auto classPtr = thisClass.lock()) {
								fibers::DynamicObject& RHS = rhs.cast();
								fibers::DynamicObject LHS(RHS.m_classType); {
									for (auto& obj : RHS.m_objects) {
										if (obj.second) {
											if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
												LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
											}
											else {
												LHS.m_objects[obj.first] = obj.second;
											}
										}
										else {
											LHS.m_objects[obj.first] = std::make_shared<Any>();
										}
									}
								}
								return lhs;
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
						}), scripting::Param_Types({ { "lhs", interactive_namespace->GetClassType() }, { "rhs", interactive_namespace->GetClassType() } }));
						// to_string
						interactive_namespace->AddFunction("to_string", make_callable([thisScope = std::weak_ptr<Class2>(interactive_namespace)](Any const& parent)->std::string {
							std::string out;
							// generic code to "Print" the Dynamic Object
							fibers::DynamicObject& Parent = parent.cast(false);
							if (auto ptr = thisScope.lock()) {
								for (auto& obj : Parent.m_objects) {
									if (out.length() > 0) out += ",";
									auto conv = ptr->Cast<std::string>(*obj.second);
									out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
								}
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
							return std::string("{") + out + " }";
						}), Param_Types({ { "parent", interactive_namespace->GetClassType() } }));
						// explicit std::string(thisClass)...
						if (auto stringClass = UI_Namespace->FindClass("string")) {
							stringClass->AddFunction(stringClass->GetName(), make_callable([thisScope = std::weak_ptr<Class2>(interactive_namespace)](Any const& parent)->std::string {
								std::string out;
								// generic code to "Print" the Dynamic Object
								fibers::DynamicObject& Parent = parent.cast(false);
								if (auto ptr = thisScope.lock()) {
									for (auto& obj : Parent.m_objects) {
										if (out.length() > 0) out += ",";
										auto conv = ptr->Cast<std::string>(*obj.second);
										out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
									}
								}
								else {
									throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
								}
								return std::string("{") + out + " }";
							}), Param_Types({ { "parent", interactive_namespace->GetClassType() } }), true, true);
						}

						interactive_namespace->AddFunction("X", make_callable([](Any const& parent)-> double {
							return (double)std::rand() / (double)RAND_MAX;
						}), Param_Types({ { "parent", interactive_namespace->GetClassType() } }));
						interactive_namespace->AddFunction("Y", make_callable([](Any const& parent)-> double {
							return (double)std::rand() / (double)RAND_MAX;
						}), Param_Types({ { "parent", interactive_namespace->GetClassType() } }));
						interactive_namespace->AddFunction("print", make_callable([thisScope = std::weak_ptr<Class2>(interactive_namespace)](Any const& parent)-> std::string {
							std::string out;
							// generic code to "Print" the Dynamic Object
							fibers::DynamicObject& Parent = parent.cast(false);
							if (auto ptr = thisScope.lock()) {
								for (auto& obj : Parent.m_objects) {
									if (out.length() > 0) out += ",";
									auto conv = ptr->Cast<std::string>(*obj.second);
									out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
								}
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
							return std::string("{") + out + " }";
						}), Param_Types({ { "parent", interactive_namespace->GetClassType() } }));
					}

					// ... which is the parent type for children types ... 
					if (1) {
						auto child_namespace{ std::make_shared<Class2>(UI_Namespace, "Map", user_type<void>(), interactive_namespace) }; {
							child_namespace->SetSelf(child_namespace);
							UI_Namespace->AddChild(child_namespace);
							{
								auto impl_namespace{ std::make_shared<Namespace2>(child_namespace, "MapImpl") };
								impl_namespace->SetSelf(impl_namespace);
								child_namespace->AddChild(impl_namespace);
							}
							{
								auto impl_namespace{ std::make_shared<Namespace2>(child_namespace, "Text") };
								impl_namespace->SetSelf(impl_namespace);
								child_namespace->AddChild(impl_namespace);
							}

							// self-construct
							child_namespace->AddFunction(child_namespace->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(child_namespace)]() -> fibers::DynamicObject {
								if (auto classPtr = thisClass.lock()) {
									auto out{ fibers::DynamicObject(classPtr->GetClassType()) };
									/* add stuff to the dynamic object ... */
									return out;
								}
								else {
									throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
								}								
							}));
							// construct from other
							child_namespace->AddFunction(child_namespace->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(child_namespace)](Any const& rhs) -> fibers::DynamicObject {
								if (auto classPtr = thisClass.lock()) {
									fibers::DynamicObject& RHS = rhs.cast();
									fibers::DynamicObject LHS(RHS.m_classType); {
										for (auto& obj : RHS.m_objects) {
											if (obj.second) {
												if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
													LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
												}
												else {
													LHS.m_objects[obj.first] = obj.second;
												}
											}
											else {
												LHS.m_objects[obj.first] = std::make_shared<Any>();
											}
										}
									}
									return LHS;
								}
								else {
									throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
								}
							}), scripting::Param_Types({ { "rhs", child_namespace->GetClassType() } }));
							// copy other
							child_namespace->AddFunction("=", make_callable([thisClass = std::weak_ptr<Class2>(child_namespace)](std::shared_ptr<Any> lhs, Any const& rhs) -> std::shared_ptr<Any> {
								if (auto classPtr = thisClass.lock()) {
									fibers::DynamicObject& RHS = rhs.cast();
									fibers::DynamicObject LHS(RHS.m_classType); {
										for (auto& obj : RHS.m_objects) {
											if (obj.second) {
												if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
													LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
												}
												else {
													LHS.m_objects[obj.first] = obj.second;
												}
											}
											else {
												LHS.m_objects[obj.first] = std::make_shared<Any>();
											}
										}
									}
									return lhs;
								}
								else {
									throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
								}
							}), scripting::Param_Types({ { "lhs", child_namespace->GetClassType() }, { "rhs", child_namespace->GetClassType() } }));
							// to_string
							child_namespace->AddFunction("to_string", make_callable([thisScope = std::weak_ptr<Class2>(child_namespace)](Any const& parent) -> std::string {
								std::string out;
								// generic code to "Print" the Dynamic Object
								fibers::DynamicObject& Parent = parent.cast(false);
								if (auto ptr = thisScope.lock()) {
									for (auto& obj : Parent.m_objects) {
										if (out.length() > 0) out += ",";
										auto conv = ptr->Cast<std::string>(*obj.second);
										out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
									}
								}
								else {
									throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
								}
								return std::string("{") + out + " }";
							}), Param_Types({ { "parent", child_namespace->GetClassType() } }));
							// explicit std::string(thisClass)...
							if (auto stringClass = UI_Namespace->FindClass("string")) {
								stringClass->AddFunction(stringClass->GetName(), make_callable([thisScope = std::weak_ptr<Class2>(child_namespace)](Any const& parent)->std::string {
									std::string out;
									// generic code to "Print" the Dynamic Object
									fibers::DynamicObject& Parent = parent.cast(false);
									if (auto ptr = thisScope.lock()) {
										for (auto& obj : Parent.m_objects) {
											if (out.length() > 0) out += ",";
											auto conv = ptr->Cast<std::string>(*obj.second);
											out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
										}
									}
									else {
										throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
									}
									return std::string("{") + out + " }";
								}), Param_Types({ { "parent", child_namespace->GetClassType() } }), true, true);
							}

							// parent(child)
							interactive_namespace->AddFunction(interactive_namespace->GetName(), scripting::make_callable([](Any const& parent) -> Any {
								std::shared_ptr<fibers::DynamicObject> Parent = parent.cast(false);
								return Parent;								
							}), scripting::Param_Types({ { "from", child_namespace->GetClassType() } }));

						}
					}
					if (1) {
						auto child_namespace{ std::make_shared<Class2>(UI_Namespace, "Text", user_type<void>(), interactive_namespace) }; {
							child_namespace->SetSelf(child_namespace);
							UI_Namespace->AddChild(child_namespace);
							{
								auto impl_namespace{ std::make_shared<Namespace2>(child_namespace, "TextImpl") };
								impl_namespace->SetSelf(impl_namespace);
								child_namespace->AddChild(impl_namespace);
							}
						}

						// self-construct
						child_namespace->AddFunction(child_namespace->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(child_namespace)]()->fibers::DynamicObject {
							if (auto classPtr = thisClass.lock()) {
								auto out{ fibers::DynamicObject(classPtr->GetClassType()) };
								/* add stuff to the dynamic object ... */
								return out;
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
						}));
						// construct from other
						child_namespace->AddFunction(child_namespace->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(child_namespace)](Any const& rhs)->fibers::DynamicObject {
							if (auto classPtr = thisClass.lock()) {
								fibers::DynamicObject& RHS = rhs.cast();
								fibers::DynamicObject LHS(RHS.m_classType); {
									for (auto& obj : RHS.m_objects) {
										if (obj.second) {
											if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
												LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
											}
											else {
												LHS.m_objects[obj.first] = obj.second;
											}
										}
										else {
											LHS.m_objects[obj.first] = std::make_shared<Any>();
										}
									}
								}
								return LHS;
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
						}), scripting::Param_Types({ { "rhs", child_namespace->GetClassType() } }));
						// copy other
						child_namespace->AddFunction("=", make_callable([thisClass = std::weak_ptr<Class2>(child_namespace)](std::shared_ptr<Any> lhs, Any const& rhs)->std::shared_ptr<Any> {
							if (auto classPtr = thisClass.lock()) {
								fibers::DynamicObject& RHS = rhs.cast();
								fibers::DynamicObject LHS(RHS.m_classType); {
									for (auto& obj : RHS.m_objects) {
										if (obj.second) {
											if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
												LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
											}
											else {
												LHS.m_objects[obj.first] = obj.second;
											}
										}
										else {
											LHS.m_objects[obj.first] = std::make_shared<Any>();
										}
									}
								}
								return lhs;
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
						}), scripting::Param_Types({ { "lhs", child_namespace->GetClassType() }, { "rhs", child_namespace->GetClassType() } }));
						// to_string
						child_namespace->AddFunction("to_string", make_callable([thisScope = std::weak_ptr<Class2>(child_namespace)](Any const& parent)->std::string {
							std::string out;
							// generic code to "Print" the Dynamic Object
							fibers::DynamicObject& Parent = parent.cast(false);
							if (auto ptr = thisScope.lock()) {
								for (auto& obj : Parent.m_objects) {
									if (out.length() > 0) out += ",";
									auto conv = ptr->Cast<std::string>(*obj.second);
									out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
								}
							}
							else {
								throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
							}
							return std::string("{") + out + " }";
						}), Param_Types({ { "parent", child_namespace->GetClassType() } }));
						// explicit std::string(thisClass)...
						if (auto stringClass = UI_Namespace->FindClass("string")) {
							stringClass->AddFunction(stringClass->GetName(), make_callable([thisScope = std::weak_ptr<Class2>(child_namespace)](Any const& parent)->std::string {
								std::string out;
								// generic code to "Print" the Dynamic Object
								fibers::DynamicObject& Parent = parent.cast(false);
								if (auto ptr = thisScope.lock()) {
									for (auto& obj : Parent.m_objects) {
										if (out.length() > 0) out += ",";
										auto conv = ptr->Cast<std::string>(*obj.second);
										out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
									}
								}
								else {
									throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
								}
								return std::string("{") + out + " }";
							}), Param_Types({ { "parent", child_namespace->GetClassType() } }), true, true);
						}

						// parent(child)
						interactive_namespace->AddFunction(interactive_namespace->GetName(), scripting::make_callable([](Any const& parent) -> Any {
							std::shared_ptr<fibers::DynamicObject> Parent = parent.cast(false);
							return Parent;
						}), scripting::Param_Types({ { "from", child_namespace->GetClassType() } }));
					}

				}

				// TESTS
				if (1) {
					auto mapObj = scope_1->CallFunction("UI::Map", Function_Params{});
					mapObj.cast<fibers::DynamicObject>().m_objects.insert({ std::string{"title"}, std::make_shared<Any>(std::string("I AM A TITLE")) });

					EXPECT_EQ(true, ((bool)scope_1->FindNamespace("UI::Map::Text"))); // correct!
					if (auto ptr = scope_1->FindNamespace("UI::Map::Text")) { // correct!
						EXPECT_EQ("::UI::Map::Text::", ptr->GetQualifiedNamespace());
					}
					if (auto ptr = scope_1->FindNamespace("UI::Text")) { // correct!
						EXPECT_EQ("::UI::Text::", ptr->GetQualifiedNamespace());
					}
					if (auto ptr = scope_1->FindNamespace("Text")) { // correct!
						EXPECT_EQ("::UI::Text::", ptr->GetQualifiedNamespace());
					}

					EXPECT_EQ("{ { \"title\": I AM A TITLE } }", (scope_1->Cast<std::string>(scope_1->CallFunction("to_string", {mapObj})))); // { { "title": I AM A TITLE } }
					EXPECT_EQ("{ { \"title\": I AM A TITLE } }", (scope_1->Cast<std::string>(scope_1->CallFunction("string", { mapObj }))));

					auto map2Obj = scope_1->CallFunction("UI::Map", { mapObj });
					EXPECT_EQ("{ { \"title\": I AM A TITLE } }", (scope_1->Cast<std::string>(scope_1->CallFunction("to_string", { map2Obj }))));

					map2Obj.cast<fibers::DynamicObject>().m_objects["title"]->cast<std::string>() = "I AM A NEW TITLE";
					EXPECT_EQ("{ { \"title\": I AM A NEW TITLE } }", (scope_1->Cast<std::string>(scope_1->CallFunction("string", { map2Obj })))); // { { "title": I AM A NEW TITLE } }
					EXPECT_EQ("{ { \"title\": I AM A TITLE } }", (scope_1->Cast<std::string>(scope_1->CallFunction("string", { mapObj })))); // { { "title": I AM A TITLE } }

					// Calling the parent's inherited functions
					// These functions aren't available anywhere else, so if they don't throw, they worked. 
					(void)scope_1->Cast<std::string>(scope_1->CallFunction("X", { mapObj })); // rand number
					(void)scope_1->Cast<std::string>(scope_1->CallFunction("Y", { mapObj })); // rand number
					(void)scope_1->Cast<std::string>(scope_1->CallFunction("X", { map2Obj })); // rand number
					(void)scope_1->Cast<std::string>(scope_1->CallFunction("Y", { map2Obj })); // rand number
					(void)scope_1->Cast<std::string>(scope_1->CallFunction("print", { mapObj })); // { { "title": I AM A TITLE } }
					(void)scope_1->Cast<std::string>(scope_1->CallFunction("print", { map2Obj })); // { { "title": I AM A NEW TITLE } }
				}
			}

			// Simulate a for-loop that 1: creates a new Class, 2: adds functions to it, 3: adds a conversion for it to std::string, and 4: uses that conversion. 
			if (1) {
				auto ScriptScope = std::make_shared<Scope2>(scope_1);
				ScriptScope->SetSelf(ScriptScope);
				{
					auto ForScope = std::make_shared<Scope2>(ScriptScope);
					ForScope->SetSelf(ForScope);

					fibers::parallel::For(0, 100, [&](int i) {
						auto LoopScope = std::make_shared<Scope2>(ForScope);
						LoopScope->SetSelf(LoopScope);
						LoopScope->AddObj("i", std::make_shared<Any>((int)i));

						if (auto i_obj = LoopScope->FindObj("i")) {
							// Make the new class...
							auto Position = std::make_shared<Class2>(LoopScope, "Position");
							Position->SetSelf(Position);
							LoopScope->AddChild(Position);
							if (1) {
								// self-construct
								Position->AddFunction(Position->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(Position)]()->fibers::DynamicObject {
									if (auto classPtr = thisClass.lock()) {
										auto out{ fibers::DynamicObject(classPtr->GetClassType()) };
										/* add stuff to the dynamic object ... */
										{
											out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>(
												"longitude", std::make_shared<Any>(classPtr->CallFunction("Number", Function_Params{}))
											));
											out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>(
												"latitude", std::make_shared<Any>(classPtr->CallFunction("Number", Function_Params{}))
											));
											out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>(
												"elevation", std::make_shared<Any>(classPtr->CallFunction("Units::foot", Function_Params{}))
											));
										}
										return out;
									}
									else {
										throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
									}
								}));
								// construct from params
								Position->AddFunction(Position->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(Position)](Any const& longitude, Any const& latitude, Any const& elevation)->fibers::DynamicObject {
									if (auto classPtr = thisClass.lock()) {
										auto out{ fibers::DynamicObject(classPtr->GetClassType()) };
										/* add stuff to the dynamic object ... */
										{
											out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>(
												"longitude", std::make_shared<Any>(classPtr->CallFunction("double", { longitude }))
											));
											out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>(
												"latitude", std::make_shared<Any>(classPtr->CallFunction("double", { latitude }))
											));
											out.m_objects.insert(std::pair<std::string, std::shared_ptr<Any>>(
												"elevation", std::make_shared<Any>(classPtr->CallFunction("Units::foot", { elevation }))
											));
										}
										return out;
									}
									else {
										throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
									}
								}), scripting::Param_Types({ 
									{ "longitude", Position->FindClass("double")->GetClassType() }
									, { "latitude", Position->FindClass("double")->GetClassType() }
								    , { "elevation", Position->FindClass("Units::foot")->GetClassType() }
								}));
								// construct from other
								Position->AddFunction(Position->GetName(), make_callable([thisClass = std::weak_ptr<Class2>(Position)](Any const& rhs)->fibers::DynamicObject {
									if (auto classPtr = thisClass.lock()) {
										fibers::DynamicObject& RHS = rhs.cast();
										fibers::DynamicObject LHS(RHS.m_classType); {
											for (auto& obj : RHS.m_objects) {
												if (obj.second) {
													if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
														LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
													}
													else {
														LHS.m_objects[obj.first] = obj.second;
													}
												}
												else {
													LHS.m_objects[obj.first] = std::make_shared<Any>();
												}
											}
										}
										return LHS;
									}
									else {
										throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
									}
								}), scripting::Param_Types({ { "rhs", Position->GetClassType() } }));
								// copy other
								Position->AddFunction("=", make_callable([thisClass = std::weak_ptr<Class2>(Position)](std::shared_ptr<Any> lhs, Any const& rhs)->std::shared_ptr<Any> {
									if (auto classPtr = thisClass.lock()) {
										fibers::DynamicObject& RHS = rhs.cast();
										fibers::DynamicObject LHS(RHS.m_classType); {
											for (auto& obj : RHS.m_objects) {
												if (obj.second) {
													if (auto ObjClass = classPtr->FindClass(obj.second->Type())) {
														LHS.m_objects[obj.first] = std::make_shared<Any>(ObjClass->CallFunction(ObjClass->GetName(), { obj.second }));
													}
													else {
														LHS.m_objects[obj.first] = obj.second;
													}
												}
												else {
													LHS.m_objects[obj.first] = std::make_shared<Any>();
												}
											}
										}
										return lhs;
									}
									else {
										throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
									}
								}), scripting::Param_Types({ { "lhs", Position->GetClassType() }, { "rhs", Position->GetClassType() } }));
								// to_string
								Position->AddFunction("to_string", make_callable([thisScope = std::weak_ptr<Class2>(Position)](Any const& parent)->std::string {
									std::string out;
									// generic code to "Print" the Dynamic Object
									fibers::DynamicObject& Parent = parent.cast(false);
									if (auto ptr = thisScope.lock()) {
										for (auto& obj : Parent.m_objects) {
											if (out.length() > 0) out += ",";
											auto conv = ptr->Cast<std::string>(*obj.second);
											out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
										}
									}
									else {
										throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
									}
									return std::string("{") + out + " }";
								}), Param_Types({ { "parent", Position->GetClassType() } }));
								// explicit std::string(thisClass)...
								if (auto stringClass = Position->FindClass("string")) {
									stringClass->AddFunction(stringClass->GetName(), make_callable([thisScope = std::weak_ptr<Class2>(Position)](Any const& parent)->std::string {
										std::string out;
										// generic code to "Print" the Dynamic Object
										fibers::DynamicObject& Parent = parent.cast(false);
										if (auto ptr = thisScope.lock()) {
											for (auto& obj : Parent.m_objects) {
												if (out.length() > 0) out += ",";
												auto conv = ptr->Cast<std::string>(*obj.second);
												out += std::string(Units::printf(" { \"%s\": %s }", obj.first.c_str(), conv.c_str()));
											}
										}
										else {
											throw scripting::exception::not_found_error("Could not find the class ptr for the requested class");
										}
										return std::string("{") + out + " }";
									}), Param_Types({ { "parent", Position->GetClassType() } }), true, true);
								}

								Position->AddFunction("longitude", make_callable([](Any const& parent)-> Any {
									auto& Parent = parent.cast<fibers::DynamicObject>(false);
									return Parent.m_objects.at("longitude");
								}), Param_Types({ { "parent", Position->GetClassType() } }));
								Position->AddFunction("latitude", make_callable([](Any const& parent)-> Any {
									auto& Parent = parent.cast<fibers::DynamicObject>(false);
									return Parent.m_objects.at("latitude");
								}), Param_Types({ { "parent", Position->GetClassType() } }));
								Position->AddFunction("elevation", make_callable([](Any const& parent)-> Any {
									auto& Parent = parent.cast<fibers::DynamicObject>(false);
									return Parent.m_objects.at("elevation");
								}), Param_Types({ { "parent", Position->GetClassType() } }));
							}


							// Instance the class, and make sure we get the class we expect... 
							if (1) {
								auto PositionInstance1 = LoopScope->CallFunction("Position", Function_Params{  }); // create as instance
								auto PositionInstance2 = LoopScope->CallFunction("Position", { -121, 32, Units::foot(15) }); // create from objs
								auto PositionInstance3 = LoopScope->CallFunction("Position", { PositionInstance2 }); // create as copy

								EXPECT_EQ((Position->GetClassType()), (PositionInstance1.Type()));
								EXPECT_EQ((Position->GetClassType()), (PositionInstance2.Type()));
								EXPECT_EQ((Position->GetClassType()), (PositionInstance3.Type()));

								// Gets an object with the type of THIS Position...							

								// ...which may have been replaced with a whole new version of Position by the time we get here...
								// ...and since "longitude" would only be found if the class type of Position is found, which it never will be, it'll fail right here. 
								EXPECT_EQ(-121, LoopScope->Cast<int>(LoopScope->CallFunction("longitude", { PositionInstance2 })));
								EXPECT_EQ(-121, LoopScope->Cast<int>(LoopScope->CallFunction("longitude", { PositionInstance3 })));

								auto& longitude = LoopScope->CallFunction("longitude", { PositionInstance2 }).cast<double>();
								EXPECT_EQ(-121, longitude);
								LoopScope->CallFunction("+=", { LoopScope->CallFunction("longitude", { PositionInstance2 }), -1 });
								EXPECT_EQ(-122, longitude);
								EXPECT_EQ(-122, LoopScope->CallFunction("longitude", { PositionInstance2 }).cast<double>());

								EXPECT_EQ(-121, LoopScope->CallFunction("longitude", { PositionInstance3 }).cast<double>());

								EXPECT_EQ("{ { \"elevation\": 15 ft }, { \"longitude\": -122.000000 }, { \"latitude\": 32.000000 } }", (LoopScope->Cast<std::string>(PositionInstance2)));
							}
						}
					});
				}
			}

			if (1) {
				// UI::Map::Text and similar namespaces should still be available
				EXPECT_EQ(true, ((bool)scope_1->FindNamespace("UI::Map::Text")));
				 
				// But, the 100 "Position" classes we made should no longer be available or in-scope
				EXPECT_EQ(false, ((bool)scope_1->FindNamespace("Position")));
				EXPECT_EQ(false, ((bool)scope_1->FindClass("Position")));

				try {
					(void)scope_1->CallFunction("Position", Function_Params{});
					EXPECT_EQ(false, true);
				} catch(std::exception& e1) {}
			}

		}
		catch (std::exception& e) {
			printf(e.what());
		}

#endif

#if 0

		// MODERN TEST 2
		if (1) {
			
			// TESTING...
			if (1) {
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
#endif

	return 0;
};
#undef EXPECT_EQ
#undef EXPECT_NE