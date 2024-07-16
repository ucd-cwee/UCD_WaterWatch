/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#include "WaterWatch_Win32_Console.h"

#include "../FiberTasks/Fibers.h"
#include <execution>

static int staticFunctionExample() noexcept {
	return 2;
};
class Thing {
public:
	int memberFunctionExample() { // if static this will no longer be considered a "member function" and just a normal function
		return 2;
	};
	static int staticMemberFunctionExample() { // if static this will no longer be considered a "member function" and just a normal function
		return 2;
	};
};

template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<fibers::Job, std::decay_t<T>> && !std::is_same_v<fibers::Any, std::decay_t<T>> >> 
static constexpr const bool IsStatelessTest() {
	using return_type = typename std::invoke_result<T, Args...>::type;
	using ftype = return_type(*)(Args...);
	return std::is_convertible<T, ftype>::value;
};

class stackThing {
public:
	cweeStr varName;
	fibers::Any var;

public:
	stackThing() : varName(), var() {};
	stackThing(cweeStr const& name) : varName(name), var() {};
	template<typename T> stackThing(cweeStr const& name, T const& obj) : varName(name), var(obj) {};
	template<typename T> stackThing(cweeStr const& name, T&& obj) : varName(name), var(std::forward<T>(obj)) {};
	stackThing(stackThing const& r) : varName(r.varName), var(r.var) {};
	stackThing(stackThing&& r) : varName(std::move(r.varName)), var(std::move(r.var)) {};
	stackThing& operator=(stackThing const& r) { varName = r.varName; var = r.var; }
	stackThing& operator=(stackThing&& r) { varName = std::move(r.varName); var = std::move(r.var); }
	~stackThing() { if (!varName.IsEmpty()) { std::cout << "DELETING " << varName << std::endl; } };

	bool operator==(stackThing const& a) const { return varName == a.varName; };
	bool operator!=(stackThing const& a) const { return varName != a.varName; };
};


//constexpr static auto sizeOfPart1 = sizeof(std::array< fibers::utilities::UnsignedWrapper<float>, unitTypes::units_type::_size_constant>);
//constexpr static auto sizeOfPart2 = sizeof(fibers::utilities::UnsignedWrapper<double>);
//constexpr static auto sizeOfPart3 = sizeof(char*);
//constexpr static auto sizeOfPart4 = sizeof(bool);
//constexpr static auto sizeOfStruct = sizeof(Units::AtomicUnitStruct);
//
//
//
//
//
//
//struct AtomicUnitStruct {
//	static constexpr size_t NumUnits = unitTypes::units_type::_size_constant;
//	
//	char* abbreviation_m;
//	// unit data
//	fibers::utilities::UnsignedWrapper<double> ratio_m;
//	// SI-unit's actual value
//	fibers::utilities::UnsignedWrapper<double> value_m;
//	std::array< fibers::utilities::UnsignedWrapper<float>, NumUnits> unitType_m;
//	bool isScalar_m : 1;
//	bool isSI_m : 1;
//	
//};
//
//constexpr static auto sizeOfStruct2 = sizeof(AtomicUnitStruct);

int Example::ExampleF(int numTasks, int numSubTasks) {
	// looking for memory leaks
#define EXPECT_EQ(a,b) [&]()->bool{ if ((a) == (b)) { return true; } else { std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; return false; } }()
#define EXPECT_NE(a, b) [&]()->bool{ if ((a) != (b)) { return true; } else { std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; return false; } }()

	if (1) {
		// TODO; Test the new fibers::parallel::ForEach system to make sure it works as intended and is not broken unintentionally. 
		if (1) {
			fibers::parallel::For(0, 10, [](int i) {});
		}
		if (1) {
			fibers::utilities::Sequence seq(1000); // 0..999
			fibers::parallel::ForEach(seq, [](int i) {  });
		}
		if (1) {
			fibers::utilities::Sequence seq(1000); // 0..999
			fibers::synchronization::atomic_number<double> D{ 0 };
			fibers::parallel::ForEach(seq, [&D](int i) {
				D.Increment();
			});
			EXPECT_EQ(D, 1000.0);
		}
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
		if (1) {
			fibers::parallel::For(0, 10000, [](int i) {});
		}
		if (1) {
			try {
				std::vector<cweeStr> vec(500, cweeStr("TEST"));
				fibers::synchronization::atomic_number<double> D{ 0 };
				fibers::parallel::ForEach(vec, [&D](cweeStr& i) {
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

		// Check the atomic_number system is not broken
		if (1) {
			fibers::utilities::CAS_Container<double> value{ 1 };
			EXPECT_EQ(value.load(), 1);

			value.Add(1);
			EXPECT_EQ(value.load(), 2);

			value.Add(1);
			EXPECT_EQ(value.load(), 3);

			value.Add(-5);
			EXPECT_EQ(value.load(), -2);

			value.Add(2);
			EXPECT_EQ(value.load(), 0);
		}
		if (1) {
			fibers::utilities::CAS_Container<double> value{ 1 };
			EXPECT_EQ(value.load(), 1);

			value.Add(-50);
			EXPECT_EQ(value.load(), -49);

			value.Add(48.5);
			EXPECT_EQ(value.load(), -0.5);

			value.Add(5.5);
			EXPECT_EQ(value.load(), 5);
		}
		if (1) {
			fibers::synchronization::atomic_number<double> value;
			EXPECT_EQ(value.load(), 0);
			for (int i = 0; i < 10000; i++){
				value.Add(0.125);
			}
			EXPECT_EQ(value.load(), 10000 * 0.125);
			for (int i = 0; i < 10000; i++) {
				value.Sub(0.125);
			}
			EXPECT_EQ(value.load(), 0);
			value = 0;
		}
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

		// Unit Values
		if (1) {
			using namespace literals;

			EXPECT_EQ((-5 / Units::foot(1_m)).ToString(), (5 / -1_m).ToString());
			EXPECT_EQ((1_ft).ToString(), "1 ft");
			EXPECT_EQ((5_ft / Units::meter(2_ft)).ToString(), "2.5");
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

		// Unions 
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

		// MultiItemCAS, which leverages the Unions for organizing N-number of types of pointers. 
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
						container.SwapWithFunctions(
							[](uint64_t item1)->uint64_t { return item1 + 1; },
							[](uint64_t item2)->uint64_t { return item2 + 1; }
						);
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
							container.SwapWithFunctions(
								[](uint64_t item1)->uint64_t { return item1 + 2; },
								[](uint64_t item2)->uint64_t { return item2 + 1; }
							);
						}
					}
					else {
						auto container{ MultiItemCAS(
							&item2,
							&item3
						) };

						for (size_t i = 0; i < kThreadNum; ++i) {
							container.SwapWithFunctions(
								[](uint64_t item1)->uint64_t { return item1 + 2; },
								[](uint64_t item2)->uint64_t { return item2 + 1; }
							);
						}
					}
					});

				EXPECT_EQ(item1, 2000000);
				EXPECT_EQ(item2, 2000000);
				EXPECT_EQ(item3, 2000000);
			}
		}

		// Epoch-based garbage collector examples
		if (0) {
			if (1) {
				using namespace fibers::utilities::dbgroup::memory;


				// using a garbage collector across multiple threads
				{
					constexpr size_t kGCInterval = 1E3;  // increment an epoch value every 1ms
					constexpr size_t kThreadNum = 1;     // use one thread to release garbage

					EpochBasedGC <
						fibers::utilities::dbgroup::memory::Target<stackThing>,
						fibers::utilities::dbgroup::memory::Target < float >
					> gc{ kGCInterval, kThreadNum };
					gc.StartGC();
					{
						for (int k = 0; k < 50; k++) {
							// 50 iterations 
							fibers::parallel::For(0, 50, [&gc](int i) {
								{
									const auto& guard = gc.CreateEpochGuard(); // try to guard this "generation" of stuff since it's being worked on

									for (int j = 0; j < 5; j++) {
										// 5 children per generation
										{
											auto* testPtr = new stackThing(cweeStr::printf("CHILD %i FROM GENERATION %i", j, i), (float)i);
											gc.Push(testPtr);
										}

										// 5 children per generation
										{
											auto* testPtr = new float((float)i);
											gc.Push(testPtr);
										}
									}
								}
								});
						}
					}
				}

				// using multple garbage collectors across multiple threads simultaneously
				{

					fibers::parallel::For(0, 50, [](int i) {
						constexpr size_t kGCInterval = 1E3;  // increment an epoch value every 1ms
						constexpr size_t kThreadNum = 1;     // use one thread to release garbage

						EpochBasedGC < fibers::utilities::dbgroup::memory::Target<stackThing> > gc{ kGCInterval, kThreadNum }; // may be constexpr

						gc.StartGC();

						for (int k = 0; k < 50; k++) {
							auto* testPtr = new stackThing(cweeStr::printf("CHILD %i FROM GENERATION %i", k, i), (float)i);
							gc.Push(testPtr);
						}
						});
				}


				// using a garbage collector across multiple threads
				{
					fibers::utilities::GarbageCollectedAllocator< stackThing > gc;
					{
						for (int k = 0; k < 50; k++) {
							// 50 iterations 
							fibers::parallel::For(0, 50, [&gc](int i) {
								{
									const auto& guard = gc.CreateEpochGuard(); // try to guard this "generation" of stuff since it's being worked on

									for (int j = 0; j < 5; j++) {
										// 5 children per generation
										{
											stackThing* testPtr = gc.Alloc(cweeStr::printf("CHILD %i FROM GENERATION %i", j, i), (float)i);
											gc.Free(testPtr);
											testPtr->var = 100.0f; // ptr will remain available until at least 3 epochs after this one, which is protected currently.
										}
									}
								}
								});
						}
					}
				}

				// using multple garbage collectors across multiple threads simultaneously
				{

					fibers::parallel::For(0, 50, [](int i) {
						fibers::utilities::GarbageCollectedAllocator< stackThing > gc;
						for (int k = 0; k < 50; k++) {
							const auto& guard = gc.CreateEpochGuard(); // try to guard this "generation" of stuff since it's being worked on
							for (int j = 0; j < 5; j++) {
								stackThing* testPtr = gc.Alloc(cweeStr::printf("CHILD %i FROM GENERATION %i", (k * 5) + j, i), (float)i);
								gc.Free(testPtr); // may be instantly cleared since we are not protecting this Epoch
							}
						}
						});
				}


			}
		}

		// Atomic BW Tree
		if (1) {
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
			if (1) {
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
			if (1) {
				fibers::containers::Pattern<double, double> tree;
				tree.Insert(5, 100.0);
				EXPECT_EQ(true, tree.Read(5).has_value() && (tree.Read(5).value() == 100.0));
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 5000, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
					});

				for (auto iter = tree.Scan(); iter; iter++) {
					(void)iter.GetKey();
				}
			}
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
					//Stopwatch sw;
					//sw.Start();

					//std::cout << "\nWorking... \n";
					for (int i = 0; i < 500; i++) {
						if (i % 50 == 0) ::Sleep(1);

						tree.Insert(i - 1, i + 1);
						tree.Insert(i + 0.5, i + 0.5);
					}
					//std::cout << "\n ...Finished.\n";

					//sw.Stop();
					//return Units::second(sw.Seconds_Passed());
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
				futureObj.wait();

				// std::cout << cweeStr::printf("MinTime = %f, MaxTime = %f\n", (float)tree.GetMinTime().value_or(0), (float)tree.GetMaxTime().value_or(0)) << std::endl;

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
		}
	}



	return 0;














	



































































	int* xyzwabc = new int[10000];
	defer(delete[] xyzwabc); // does clean-up on our behalf on scope end

#if 1
	if (1) {
		// TODO; Ensure the job system supports basic queue and wait features.
		{
			fibers::JobGroup jobGroup(fibers::Job([]() { return 100.0f; }).AsyncInvoke());
			EXPECT_EQ(jobGroup.Wait_Get<float>(), 100.0f);
		}

		// TODO; Test the new fibers::parallel::ForEach system to make sure it works as intended and is not broken unintentionally. 
		{
			fibers::utilities::Sequence seq(1000); // 0..999
			fibers::synchronization::atomic_number<double> D{ 0 };
			fibers::parallel::ForEach(seq, [&D](int i) {
				D.Increment();
			});
			EXPECT_EQ(D, 1000.0);
		}

		try{
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

		try {
			std::vector<cweeStr> vec(500, cweeStr("TEST"));
			fibers::synchronization::atomic_number<double> D{ 0 };
			fibers::parallel::ForEach(vec, [&D](cweeStr& i) {
				if (D.Decrement() < 500) {
					throw(true);
				}
			});
			EXPECT_EQ(D, -1000.0);
		}
		catch (bool v) {
			EXPECT_EQ(v, true);
		}

		// Check the atomic_number system is not broken
		if (1) {
			fibers::utilities::CAS_Container<double> value{ 1 };
			EXPECT_EQ(value.load(), 1);

			value.Add(1);
			EXPECT_EQ(value.load(), 2);

			value.Add(1);
			EXPECT_EQ(value.load(), 3);

			value.Add(-5);
			EXPECT_EQ(value.load(), -2);

			value.Add(2);
			EXPECT_EQ(value.load(), 0);
		}
		if (1) {
			fibers::utilities::CAS_Container<double> value{ 1 };
			EXPECT_EQ(value.load(), 1);

			value.Add(-50);
			EXPECT_EQ(value.load(), -49);

			value.Add(48.5);
			EXPECT_EQ(value.load(), -0.5);

			value.Add(5.5);
			EXPECT_EQ(value.load(), 5);

			// value.Add(-5);
			// value.Add(2e10);
			// EXPECT_EQ(value.load(), 2e10);
		}
		if (1) {
			fibers::utilities::UnsignedWrapper<long double> value{ 0 };

			value = std::numeric_limits<double>::max();

			std::cout << value.load() << std::endl;
			std::cout << std::numeric_limits<double>::max() << std::endl;
		}

		// Check the atomic_number system is not broken
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

			{
				std::atomic<int> shared;
				std::atomic<int> largest;
				fibers::parallel::For((size_t)0, (size_t)(fibers::utilities::Hardware::GetNumCpuCores()), [kExecNum = 1e5, &shared, &largest](size_t threadNum) {
					auto thisLargest = shared.fetch_add(1);

					::Sleep(100);

					if (largest.load() < thisLargest)
						largest.store(thisLargest);

					shared.fetch_add(-1);
				});
				std::cout << cweeStr::printf("\tResult (atomic) = %f\n", (float)largest.load());
			}
			{
				fibers::synchronization::atomic_number<int> shared;
				std::atomic<int> largest;
				fibers::parallel::For((size_t)0, (size_t)(fibers::utilities::Hardware::GetNumCpuCores()), [kExecNum = 1e5, &shared, &largest](size_t threadNum) {
					auto thisLargest = shared.fetch_add(1);

					::Sleep(100);

					if (largest.load() < thisLargest)
						largest.store(thisLargest);

					shared.fetch_add(-1);
				});
				std::cout << cweeStr::printf("\tResult (int) = %f\n", (float)largest.load());
			}
			{
				fibers::synchronization::atomic_number<double> shared;
				std::atomic<int> largest;
				fibers::parallel::For((size_t)0, (size_t)(fibers::utilities::Hardware::GetNumCpuCores()), [kExecNum = 1e5, &shared, &largest](size_t threadNum) {
					auto thisLargest = shared.fetch_add(1);

					::Sleep(100);

					if (largest.load() < thisLargest)
						largest.store(thisLargest);

					shared.fetch_add(-1);
				});
				std::cout << cweeStr::printf("\tResult (double) = %f\n", (float)largest.load());
			}
			{
				fibers::synchronization::atomic_number<double> shared;
				std::atomic<int> largest;
				fibers::parallel::For((size_t)0, (size_t)(fibers::utilities::Hardware::GetNumCpuCores()), [kExecNum = 1e5, &shared, &largest](size_t threadNum) {
					auto thisLargest = std::abs(shared.fetch_add(-1));

					::Sleep(100);

					if (largest.load() < thisLargest)
						largest.store(thisLargest);

					shared.fetch_add(1);
				});
				std::cout << cweeStr::printf("\tResult (double) = %f\n", (float)largest.load());
			}
		}

		// Unit Values
		if (1) {
			using namespace literals;

			std::cout << -5 / Units::foot(1_m) << std::endl; // 1/ft is not a standard type so it'll default to SI units, which is the desired behavior
			std::cout << 5 / -1_m << std::endl; 

			std::cout << 1_ft << std::endl;

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

			// test unit canceling (should become unitless, like a normal double)
			std::cout << (5_ft / Units::meter(2_ft)).ToString() << std::endl; // 2.5

			// test exact unit discovery (should discover that m/s is a valid, known unit type and report it correctly)
			std::cout << (10_m / 2_s).ToString() << std::endl; // 5 mps

			// test close unit discovery (gal*min/s does not exist, and it's resulting ratio is still not going to be found). Should find a valid volume unit type, but we will not know which (e.g. could be L or mL or m^3 -- all would be legal as long as the volume conversion is correct still).
			std::cout << ((10_gal / -60_s) * -1_min).ToString() << std::endl; // e.g. 0.2381 bl, or gallons, or MG, etc. all are valid results

			// test generation of a unit through multiplication / power functions
			std::cout << (1_m).pow(3) << std::endl; // 1 cu_m
			std::cout << (4_sq_m).pow(0.5) << std::endl; // 2 m
			std::cout << (4_ft).pow(0.5) << std::endl; // 0.8204 m^0.5
			std::cout << (2_m).pow(1.5) << std::endl; //  2.8284 m^1.5
			
			std::cout << 16_cu_m / 2_m << std::endl; // 8 sq_m
			std::cout << (16_cu_m / 2_m) / 2_m << std::endl; // 4 m

			std::cout << 16_cu_ft / 2_ft << std::endl; // 8 sq_ft
			std::cout << (16_cu_ft / 2_ft) / 2_ft << std::endl; // 4 ft

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
				std::cout << "Result = " << shared << std::endl;
			}
			if (1) { // complex atomic math
				using namespace literals;

				Units::value shared = 0_ft;

				fibers::parallel::For((size_t)0, (size_t)(100), [&shared](size_t threadNum) {
					for (int i = 0; i < 100; i++) {
						switch (i % 4) {
						default:
						case 0:
							shared *= 2;
							break;
						case 1:
							shared /= 2;
							break;
						case 2:
							shared += 2;
							break;
						case 3:
							shared -= 2;
							break;
						}

						if ((threadNum * 1000 + i) % 750 == 0) {
							auto str = shared.ToString();
							std::cout << cweeStr::printf("\tResult (Intermediate)= %s\n", str.c_str());
						}
					}
				});
				std::cout << "Result = " << shared << std::endl;
			}
			if (1) { // swapping and modification of units
				using namespace literals;

				Units::value shared = 0_ft;

				Units::foot(100) + Units::inch(10000.123) + Units::meter(10);

				fibers::parallel::For((size_t)0, (size_t)(100), [&shared](size_t threadNum) {
					for (int i = 0; i < 100; i++) {
						switch ((cweeRandomInt(0,4) + i) % 4) {
						default:
						case 0:
							shared.Swap(1_ft);
							break;
						case 1:
							shared += 2; // whatever the unit is, add two
							break;
						case 2:
							shared.Swap(1_m * 1_ft);
							break;
						case 3:
							shared.Swap(1_gal * 1_min * 1_yr / 1_d / 1_acre);
							break;
						}

						if ((threadNum * 1000 + i) % 250 == 0) {
							auto str = shared.ToString();
							std::cout << cweeStr::printf("\tResult (Intermediate)= %s\n", str.c_str());
						}
					}
				});
				std::cout << "Result = " << shared << std::endl;
			}

		}

		// Unions 
		if (0) {
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

		// MultiItemCAS, which leverages the Unions for organizing N-number of types of pointers. 
		if (0) {
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
						container.SwapWithFunctions(
							[](uint64_t item1)->uint64_t { return item1 + 1; },
							[](uint64_t item2)->uint64_t { return item2 + 1; }
						);
					}
				});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\t1st field: " << container.Read<0>() << "\n\t2nd field: " << container.Read<1>() << std::endl << std::endl;
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
							container.SwapWithFunctions(
								[](uint64_t item1)->uint64_t { return item1 + 2; },
								[](uint64_t item2)->uint64_t { return item2 + 1; }
							);
						}
					}
					else {
						auto container{ MultiItemCAS(
							&item2,
							&item3
						) };

						for (size_t i = 0; i < kThreadNum; ++i) {
							container.SwapWithFunctions(
								[](uint64_t item1)->uint64_t { return item1 + 2; },
								[](uint64_t item2)->uint64_t { return item2 + 1; }
							);
						}
					}
				});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\t1st field: " << item1 << "\n\t2nd field: " << item2 << "\n\t3rd field: " << item3 << std::endl << std::endl;
			}
		}

		// Epoch-based garbage collector examples
		if (0) {
			if (1) {
			    using namespace fibers::utilities::dbgroup::memory;

		
				// using a garbage collector across multiple threads
				{
					constexpr size_t kGCInterval = 1E3;  // increment an epoch value every 1ms
					constexpr size_t kThreadNum = 1;     // use one thread to release garbage

					EpochBasedGC <
						fibers::utilities::dbgroup::memory::Target<stackThing>,
						fibers::utilities::dbgroup::memory::Target < float >
					> gc{ kGCInterval, kThreadNum };
					gc.StartGC();
					{
						for (int k = 0; k < 50; k++) {
							// 50 iterations 
							fibers::parallel::For(0, 50, [&gc](int i) {
								{
									const auto& guard = gc.CreateEpochGuard(); // try to guard this "generation" of stuff since it's being worked on

									for (int j = 0; j < 5; j++) {
										// 5 children per generation
										{
											auto* testPtr = new stackThing(cweeStr::printf("CHILD %i FROM GENERATION %i", j, i), (float)i);
											gc.Push(testPtr);
										}

										// 5 children per generation
										{
											auto* testPtr = new float((float)i);
											gc.Push(testPtr);
										}
									}
								}
								});
						}
					}
				}

				// using multple garbage collectors across multiple threads simultaneously
				{

					fibers::parallel::For(0, 50, [](int i) {
						constexpr size_t kGCInterval = 1E3;  // increment an epoch value every 1ms
						constexpr size_t kThreadNum = 1;     // use one thread to release garbage

						EpochBasedGC < fibers::utilities::dbgroup::memory::Target<stackThing> > gc{ kGCInterval, kThreadNum }; // may be constexpr

						gc.StartGC();

						for (int k = 0; k < 50; k++) {
							auto* testPtr = new stackThing(cweeStr::printf("CHILD %i FROM GENERATION %i", k, i), (float)i);
							gc.Push(testPtr);
						}
					});
				}


				// using a garbage collector across multiple threads
				{
					fibers::utilities::GarbageCollectedAllocator< stackThing > gc;
					{
						for (int k = 0; k < 50; k++) {
							// 50 iterations 
							fibers::parallel::For(0, 50, [&gc](int i) {
								{
									const auto& guard = gc.CreateEpochGuard(); // try to guard this "generation" of stuff since it's being worked on

									for (int j = 0; j < 5; j++) {
										// 5 children per generation
										{
											stackThing* testPtr = gc.Alloc(cweeStr::printf("CHILD %i FROM GENERATION %i", j, i), (float)i);
											gc.Free(testPtr);
											testPtr->var = 100.0f; // ptr will remain available until at least 3 epochs after this one, which is protected currently.
										}
									}
								}
							});
						}
					}
				}

				// using multple garbage collectors across multiple threads simultaneously
				{

					fibers::parallel::For(0, 50, [](int i) {
						fibers::utilities::GarbageCollectedAllocator< stackThing > gc;
						for (int k = 0; k < 50; k++) {
							const auto& guard = gc.CreateEpochGuard(); // try to guard this "generation" of stuff since it's being worked on
							for (int j = 0; j < 5; j++) {
								stackThing* testPtr = gc.Alloc(cweeStr::printf("CHILD %i FROM GENERATION %i", (k * 5) + j, i), (float)i);
								gc.Free(testPtr); // may be instantly cleared since we are not protecting this Epoch
							}
						}
					});
				}


            }
		}

		// Atomic BW Tree
		if (1) {
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
			if (1) {
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
			if (1) {
				fibers::containers::Pattern<double, double> tree;
				tree.Insert(5, 100.0);
				EXPECT_EQ(true, tree.Read(5).has_value() && (tree.Read(5).value() == 100.0));
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 5000, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
				});

				for (auto iter = tree.Scan(); iter; iter++) {
					(void)iter.GetKey();
				}
			}
			if (1) {
				fibers::containers::Pattern<long double, double> tree;
				tree.Insert(5, 100.0);
				EXPECT_EQ(true, tree.Read(5).has_value() && (tree.Read(5).value() == 100.0));
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(-5, 500, [&tree](int i) {
					EXPECT_EQ(true, tree.Insert(i, i));
				});

				auto futureObj = fibers::parallel::async([&tree](){
					Stopwatch sw; 
					sw.Start();

					std::cout << "\nWorking... \n";
					for (int i = 0; i < 500; i++) {
						if (i % 50 == 0) ::Sleep(1);

						tree.Insert(i - 1, i + 1);
						tree.Insert(i + 0.5, i + 0.5);						
					}
					std::cout << "\n ...Finished.\n";

					sw.Stop();
					return Units::second(sw.Seconds_Passed());
				});

				for (double D = -2.25; D <= 505; D += 1.25) {
					auto iter_Larger = tree.FindSmallestLargerEqual(D);
					auto iter_Smaller = tree.FindLargestSmallerEqual(D);
					
					if (iter_Larger && iter_Smaller) {
						Sleep(1);
						std::cout << cweeStr::printf("\t %f <= %f <= %f\n", (float)iter_Smaller.GetKey().load(), (float)D, (float)iter_Larger.GetKey().load());
					}
				}

				// wait until the job is completed and the result is returned.
				std::cout << futureObj.wait_get_ref() << std::endl; 

				std::cout << cweeStr::printf("MinTime = %f, MaxTime = %f\n", (float)tree.GetMinTime().value_or(0), (float)tree.GetMaxTime().value_or(0)) << std::endl;

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
		}
	}
#endif

	// Allocator
	if (0) {
		int index = 0;
		for (index = 0; index < 10; index++)
		{
			fibers::utilities::Allocator<double, 128> alloc;
			fibers::containers::queue<double*> ptrs;

			auto* p1 = alloc.Alloc();
			alloc.Free(p1);

			auto* p2 = alloc.Alloc();
			auto* p3 = alloc.Alloc();
			alloc.Free(p2);
			alloc.Free(p3);

			fibers::parallel::For(0, 400, [&alloc, &ptrs](int i) {
				if (i % 2 == 0) {
					for (int k = i; k < (i + 400); k++) {
						auto* p = alloc.Alloc();
						*p = random_fast(-100, 100);
						ptrs.push(p);
					}
				}
				else {
					for (int k = i; k < (i + 400); k += 2) {
						double* ptr{ nullptr };
						if (ptrs.try_pop(ptr)) {
							alloc.Free(ptr);
						}
					}
				}
			});
		}
		for (index = 0; index < 10; index++)
		{
			fibers::utilities::Allocator<double, 128> alloc;

			fibers::parallel::For(0, 400, [&alloc](int i) {
				if (i % 2 == 0) {
					for (int k = i; k < (i + 400); k++) {
						auto ptr = alloc.AllocShared();
						*ptr = random_fast(-100, 100);
					}
				}
			});
		}
		for (index = 0; index < 10; index++)
		{
			fibers::utilities::Allocator<stackThing, 128> alloc;
			fibers::containers::queue<stackThing*> ptrs;

			auto* p1 = alloc.Alloc();
			alloc.Free(p1);

			auto* p2 = alloc.Alloc();
			auto* p3 = alloc.Alloc();
			alloc.Free(p2);
			alloc.Free(p3);

			fibers::parallel::For(0, 400, [&alloc, &ptrs](int i) {
				if (i % 2 == 0) {
					for (int k = i; k < (i + 400); k++) {
						auto* p = alloc.Alloc();
						p->var = random_fast(-100, 100);
						p->varName = cweeStr(p->var.cast<float>());
						ptrs.push(p);
					}
				}
				else {
					for (int k = i; k < (i + 400); k += 2) {
						stackThing* ptr{ nullptr };
						if (ptrs.try_pop(ptr)) {
							alloc.Free(ptr);
						}
					}
				}
			});
		}
		for (index = 0; index < 10; index++)
		{
			fibers::utilities::Allocator<stackThing, 128> alloc;

			fibers::parallel::For(0, 400, [&alloc](int i) {
				if (i % 2 == 0) {
					for (int k = i; k < (i + 400); k++) {
						auto p = alloc.AllocShared();
						p->var = random_fast(-100, 100);
						p->varName = cweeStr(p->var.cast<float>());
					}
				}
			});
		}
	}

	// across multiple threads, submitting lots of jobs all at the same time. Then waiting for them.
	if (1) {
		for (int numJobs = 400; numJobs <= 1600; numJobs += 400) {
			{
				Stopwatch sw; sw.Start();
				fibers::containers::number<long long> Num{ 0 };
				defer(std::cout << cweeStr::printf("ForEachN:  %f seconds: %i.\n", (float)Units::second(sw.Stop() / 1000000000.0l)(), (int)Num.load()));

				fibers::utilities::Sequence<int> seq{ numJobs };
				std::for_each_n(std::execution::par, seq.begin(), numJobs, [&Num, &numJobs](int& i) {
					fibers::utilities::Sequence<int> seq2{ numJobs };
					std::for_each_n(std::execution::par, seq2.begin(), numJobs, [&Num](int& i) {
						//Num.Increment();
					});
				});
			}
			{
				Stopwatch sw; sw.Start();
				fibers::containers::number<long long> Num{ 0 };
				defer(std::cout << cweeStr::printf("JobList A: %f seconds: %i.\n", (float)Units::second(sw.Stop() / 1000000000.0l)(), (int)Num.load()));

				fibers::parallel::For(0, numJobs, [&Num, &numJobs](int i) {
					fibers::parallel::For(0, numJobs, [&Num](int i) {
						//Num.Increment();
					});
				});
			}
		}
		for (int numJobs = 400; numJobs <= 1600; numJobs += 400) {
			{
				Stopwatch sw; sw.Start();
				fibers::containers::number<long long> Num{ 0 };
				defer(std::cout << cweeStr::printf("ForEachN:  %f seconds: %i.\n", (float)Units::second(sw.Stop() / 1000000000.0l)(), (int)Num.load()));

				fibers::utilities::Sequence<int> seq{ numJobs };
				std::for_each_n(std::execution::par, seq.begin(), numJobs, [&Num, &numJobs](int& i) {
					fibers::utilities::Sequence<int> seq2{ numJobs };
					std::for_each_n(std::execution::par, seq2.begin(), numJobs, [&Num](int& i) {
						Num.Increment();
					});
				});
			}
			{
				Stopwatch sw; sw.Start();
				fibers::containers::number<long long> Num{ 0 };
				defer(std::cout << cweeStr::printf("JobList A: %f seconds: %i.\n", (float)Units::second(sw.Stop() / 1000000000.0l)(), (int)Num.load()));

				fibers::parallel::For(0, numJobs, [&Num, &numJobs](int i) {
					fibers::parallel::For(0, numJobs, [&Num](int i) {
						Num.Increment();
					});
				});
			}
		}
	}

	// fibers::containers::Stack
	if (0) {
		int ijk = 0;
		for (ijk = 0; ijk < 5; ijk++) {
			fibers::containers::Stack<double> concurrent_set;

			concurrent_set.push(0);
 			concurrent_set.push(1);
			concurrent_set.push(2);
			concurrent_set.push(3);
			concurrent_set.push(4);
			concurrent_set.push(5);

			concurrent_set.try_remove(3);

			EXPECT_EQ(false, concurrent_set.contains(3));

			double x{ 0 };
			EXPECT_EQ(true, concurrent_set.try_pop(x));

			fibers::parallel::For(0, 400, [&concurrent_set](int i) {
				if (i % 2 == 0) {
					for (int k = i; k < (i + 400); k++) {
						concurrent_set.push(k);
					}
				}
			});

			if (concurrent_set.contains(1)) {
				double T;
				concurrent_set.try_pop(T);
			}
		}
	}

	// fibers::containers::AtomicQueue
	if (0) {
		fibers::containers::AtomicQueue<long long> concurrent_queue;
		fibers::containers::number<int> jobs{ 0 };
		while (jobs.load() < 100) {
			concurrent_queue.push(jobs.Increment());
		}
		jobs = 0;

		fibers::parallel::For(0, fibers::utilities::Hardware::GetNumCpuCores() + 1, [&concurrent_queue, &jobs](int i) {
			if (i == 0) {
				// add items to the back of the queue
				while (jobs.load() < 100) {
					concurrent_queue.push(jobs.Increment());
				}
			}
			else {
				long long Task{ 0 };
				while (concurrent_queue.front(Task)) {
					::Sleep(1);
					if (concurrent_queue.try_remove_front_if([Task](long long const& actualFront)->bool { return Task == actualFront; })) {
						std::cout << cweeStr::printf("\t\tThread %i Removed Task %i.\n", (int)i, (int)Task);
					}
					else {
						std::cout << cweeStr::printf("\t\t\tThread %i Worked on Task %i but failed to remove it.\n", (int)i, (int)Task);
					}
				}
			}
		});
	}

	std::cout << "Starting Speed Tests:\n";

	if (1) {
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl);

				for (int i = 0; i < numLoops; i++) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				}
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Threads) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl);

				std::vector<int> vec(numLoops, 0);
				fibers::parallel::For(0, numLoops, [&vec](int i) { vec[i] = i; });
				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++) {
						pat.AddValue(i + k, i + k);
					}
				});
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();				
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl);

				fibers::parallel::For(0, numLoops, [&pat, &j](int i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Threads (Fibers Sequence)) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl);

				fibers::utilities::Sequence<int> seq{ numLoops };
				std::for_each_n(std::execution::par, seq.begin(), numLoops, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++) {
						pat.AddValue(i + k, i + k);
					}
				});
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers (Fibers Sequence)) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl);

				fibers::utilities::Sequence<int> seq{ numLoops };
				fibers::parallel::ForEach(seq, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (Atomic Pattern) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				fibers::containers::Pattern<long double, double> pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << std::endl);

				for (int i = 0; i < numLoops; i++) {
					for (int k = 0; k < j; k++)
						pat.Insert(i + k, i + k);
				}
			}

			printf("SpeedTest (Atomic Pattern) ");
			std::cout << j;
			printf(" (Threads) : ");
			{
				fibers::containers::Pattern<long double, double> pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << std::endl);

				std::vector<int> vec(numLoops, 0);
				fibers::parallel::For(0, numLoops, [&vec](int i) { vec[i] = i; });
				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++) {
						pat.Insert(i + k, i + k);
					}
				});
			}

			printf("SpeedTest (Atomic Pattern) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				fibers::containers::Pattern<long double, double> pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << std::endl);

				fibers::parallel::For(0, numLoops, [&pat, &j](int i) {
					for (int k = 0; k < j; k++)
						pat.Insert(i + k, i + k);
				});
			}

			printf("SpeedTest (Atomic Pattern) ");
			std::cout << j;
			printf(" (Threads (Fibers Sequence)) : ");
			{
				fibers::containers::Pattern<long double, double> pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << std::endl);

				fibers::utilities::Sequence<int> seq{ numLoops };
				std::for_each_n(std::execution::par, seq.begin(), numLoops, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++) {
						pat.Insert(i + k, i + k);
					}
				});
			}

			printf("SpeedTest (Atomic Pattern) ");
			std::cout << j;
			printf(" (Fibers (Fibers Sequence)) : ");
			{
				fibers::containers::Pattern<long double, double> pat;
				Stopwatch sw; sw.Start();
				defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << std::endl);

				fibers::utilities::Sequence<int> seq{ numLoops };
				fibers::parallel::ForEach(seq, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++)
						pat.Insert(i + k, i + k);
				});
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				fibers::containers::number<int> count;
				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++)
						count.Add(k);
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.GetValue() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Threads) : ");
			{
				
				fibers::containers::number<int> count;
				Stopwatch sw; sw.Start();
				int i, k;

				std::vector<int> vec(numLoops, 0);
				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&count, &j](int& V) {
					for (int k = 0; k < j; k++)
						count.Add(k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.GetValue() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				fibers::containers::number<int> count;
				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&count, &j](int i) {
					for (int k = 0; k < j; k++)
						count.Add(k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.GetValue() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Threads (Fibers Sequence)) : ");
			{
				fibers::containers::number<int> count;
				Stopwatch sw; sw.Start();

				fibers::utilities::Sequence<int> seq{ numLoops };
				std::for_each_n(std::execution::par, seq.begin(), numLoops, [&count, &j](int& V) {
					for (int k = 0; k < j; k++) {
						count.Add(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.GetValue() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers (Fibers Sequence)) : ");
			{
				fibers::containers::number<int> count;
				Stopwatch sw; sw.Start();

				fibers::utilities::Sequence<int> seq{ numLoops };
				fibers::parallel::ForEach(seq, [&count, &j](int& V) {
					for (int k = 0; k < j; k++) {
						count.Add(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.GetValue() << ")" << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (unit values) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				Units::gallon_per_minute count;
				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++)
						count += k;
				}

				Units::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count << ")" << std::endl;
			}

			printf("SpeedTest (unit values) ");
			std::cout << j;
			printf(" (Threads) : ");
			{
				Units::gallon_per_minute count;
				Stopwatch sw; sw.Start();
				int i, k;

				fibers::utilities::Sequence<int> seq{ numLoops };
				std::for_each_n(std::execution::par, seq.begin(), numLoops, [&count, &j](int& V) {
					for (int k = 0; k < j; k++)
						count += k;
				});

				Units::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count << ")" << std::endl;
			}

			printf("SpeedTest (unit values) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				Units::gallon_per_minute count;
				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&count, &j](int i) {
					for (int k = 0; k < j; k++)
						count += k;
				});

				Units::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count << ")" << std::endl;
			}

			printf("SpeedTest (unit values) ");
			std::cout << j;
			printf(" (Fibers, Locked) : ");
			{
				fibers::synchronization::mutex lock;
				Units::gallon_per_minute count;
				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&lock, &count, &j](int i) {
					for (int k = 0; k < j; k++) {
						auto locked{ std::scoped_lock{ lock } };
						count += k;
					}
				});

				Units::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count << ")" << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++) {
						vec[i] = cweeStr(k);
					}
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Threads) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				int i, k;

				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&vec, &j](cweeStr& V) {
					for (int k = 0; k < j; k++) {
						V = cweeStr(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&vec, &j](int i) {
					for (int k = 0; k < j; k++) {
						vec[i] = cweeStr(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers ForEach) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				fibers::parallel::ForEach(vec, [&j](cweeStr& v) {
					for (int k = 0; k < j; k++) {
						v = cweeStr(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (vector search) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr());
				fibers::parallel::For(0, numLoops, [&vec](int i) { vec[i] = cweeStr(i); });

				Stopwatch sw; sw.Start();

				for (int i = 0; i < numLoops; i++) {
					(void)std::find_if(vec.begin(), vec.end(), [j = cweeStr((int)random_fast(0, numLoops-1))](cweeStr const& x) ->bool { return x == j; });
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector search) ");
			std::cout << j;
			printf(" (Threads) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr());
				fibers::parallel::For(0, numLoops, [&vec](int i) { vec[i] = cweeStr(i); });

				Stopwatch sw; sw.Start();

				for (int i = 0; i < numLoops; i++) {
					(void)std::find_if(std::execution::par, vec.begin(), vec.end(), [j = cweeStr((int)random_fast(0, numLoops - 1))](cweeStr const& x) ->bool { return x == j; });
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++) {
						vec[i][k] = cweeStr(k);
					}
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (Threads w/ For-Loop) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();
				int i, k;

				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&vec, &j](std::vector<cweeStr>& V) {
					for (int k = 0; k < j; k++) {
						V[k] = cweeStr(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (Threads w/ Threads) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();
				int i, k;

				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&vec, &j](std::vector<cweeStr>& V) {
					fibers::containers::number<int> n{ 0 };
					std::for_each_n(std::execution::par, V.begin(), j, [&V, &n](cweeStr& V) {
						V = cweeStr(n.Increment());
					});
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (Fibers w/ For-Loop) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&vec, &j](int i) {
					for (int k = 0; k < j; k++) {
						vec[i][k] = cweeStr(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (Fibers w/ Fibers) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&vec, &j](int i) {
					fibers::parallel::For(0, j, [&vec, &j, &i](int k) {
						vec[i][k] = cweeStr(k);
					});
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (Fibers Foreach w/ For-Loop) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();
				fibers::parallel::ForEach(vec, [&j](std::vector<cweeStr>& vec) {
					for (int k = 0; k < j; k++) {
						vec[k] = cweeStr(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (Fibers Foreach w/ Fibers) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();
				fibers::parallel::ForEach(vec, [&j](std::vector<cweeStr>& vec) {
					fibers::parallel::For(0, j, [&vec, &j](int k) {
						vec[k] = cweeStr(k);
					});
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 4) {
			int numLoops = 400 * j * j;
			bool successfulErrorCatch = false;

			printf("SpeedTest (catching errors) ");
			std::cout << j;
			printf(" (No Fibers) : "); {
				try {
					std::vector<int> vec(numLoops, 0);
					Stopwatch sw;
					defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << std::endl);
					sw.Start();

					int i;
					for (i = 0; i < numLoops; i++) {
						throw(std::runtime_error("Purposeful error"));
					}
				}
				catch (std::runtime_error const& e) { successfulErrorCatch = true; }
				catch (...) {}
				EXPECT_EQ(true, successfulErrorCatch);
				successfulErrorCatch = false;
			}

			printf("SpeedTest (catching errors) ");
			std::cout << j;
			printf(" (Fibers) : "); {
				try {
					std::vector<int> vec(numLoops, 0);
					Stopwatch sw;
					defer(std::cout << cweeUnitValues::second(sw.Stop() / 1000000000.0).ToString() << std::endl);
					sw.Start();

					fibers::parallel::For(0, numLoops, [](int i) {
						throw(std::runtime_error("Purposeful error"));
					});
				}
				catch (std::runtime_error const& e) { successfulErrorCatch = true; }
				catch (...) {}
				EXPECT_EQ(true, successfulErrorCatch);
				successfulErrorCatch = false;
			}

			printf("\n");
		}
	}

	auto static_lambda = []()->cweeStr { return cweeStr("2"); };
	auto dynamic_lambda = [xyzwabc]()->void{ xyzwabc[0]; return; };
	auto static_function = &staticFunctionExample;
	auto member_function = &Thing::memberFunctionExample;	
	auto local_static_stdFunc = std::function([]()->int { throw(std::runtime_error("ERR")); return 2; });
	auto local_dynamic_stdFunc = std::function([xyzwabc]()->int { return xyzwabc[0]; });
	auto static_stdFunc = std::function(staticFunctionExample);
	auto member_stdFunc = std::function(Thing::staticMemberFunctionExample);




	int v = 1;
	auto lambda1 = [](int i)->void {};
	auto lambda2 = [v]()->void {};

	constexpr const bool v_1234 = IsStatelessTest<decltype(lambda2)>();

	auto jobTest = fibers::Job([]() { return 10.0f; }); // static function
	jobTest.AsyncFireAndForget(); // will not crash, since the above function is stateless (which can be called without concern of us going out of scope)

	printf("Job 0\n");
	if (true) {
		try {
			fibers::parallel::For(0, numTasks * numSubTasks, [](int j) {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}
				throw(std::move(sw)); // throws early, and (most) other threads early-exit. Threads in the middle of their work will still need to complete their work, however.
			});

			return -1; // should never happen
		}
		catch (...) {
			// Great!
		}
	}
	printf("Job 1");
	if (true) {
		fibers::containers::number<int> V{ 0 };
		fibers::parallel::For(0, numTasks * numSubTasks, [&V](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {
				V.Increment();
			}
		});
		printf(": %i\n", (int)(V.GetValue()));
	}
	printf("Job 1.1");
	if (true) {
		fibers::containers::number<unsigned long long> V{ 0ull };
		fibers::parallel::For(0, numTasks * numSubTasks, [&V](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {
				V.Increment();
			}
			});
		printf(": %i\n", (int)(V.GetValue()));
	}
	printf("Job 1.2");
	if (true) {
		fibers::containers::number<double> V{ 0.0 };
		fibers::parallel::For(0, numTasks * numSubTasks, [&V](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {
				V.Increment();
			}
			});
		printf(": %f\n", (float)(V.GetValue()));
	}
	printf("Job 1.3");
	if (true) {
		fibers::containers::number<long double> V{ 0.0l };
		fibers::parallel::For(0, numTasks * numSubTasks, [&V](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {
				V.Increment();
			}
			});
		printf(": %f\n", (float)(V.GetValue()));
	}
	printf("Job 2\n");
	if (true) {
		cweeBalancedPattern pat;
		fibers::JobGroup group;
		for (int i = 0; i < numTasks * numSubTasks; i++) {
			auto job = fibers::Job([&pat](int j) {
				pat.AddUniqueValue(j, j);
			}, (int)i);
			group.Queue(job);
		}
		group.Wait();

		int sizeIs = pat.GetNumValues();
		if (sizeIs == 0) throw(std::runtime_error("Something went wrong"));
	}
	printf("Job 3\n");
	if (true) {
		fibers::parallel::For(0, numTasks * numSubTasks * 2, [](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {}
		});
	}
	printf("Job 4\n");
	if (true) {
		fibers::JobGroup group;
		//fibers::ftl_wrapper::TaskScheduler scheduler;
		for (int i = 0; i < numTasks; i++) {
			auto job = fibers::Job([]() {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}
			});
			group.Queue(job);
			//scheduler.AddTask(job);
		}
		group.Wait();
		//scheduler.Wait();
	}
	printf("Job 5\n");
	if (true) {
		std::atomic<int> numJobsDone;
		fibers::parallel::For(0, numTasks, [&numSubTasks, &numJobsDone](int i) {
			fibers::parallel::For(0, numSubTasks, [&numJobsDone](int j) {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}

				numJobsDone.fetch_add(1);
			});
		});
		if (numJobsDone.load() != numTasks * numSubTasks) {
			printf("Job 5 failed");
		}
	}
	printf("Job 6\n");
	if (true) {
		cweeBalancedPattern pat;
		fibers::parallel::For(0, numTasks, [&numSubTasks, &pat](int i) {
			fibers::parallel::For(0, numSubTasks, [&pat, &i, &numSubTasks](int j) {
				pat.AddUniqueValue(i*numSubTasks + j, i* numSubTasks + j);
			});
		});
		if (pat.GetNumValues() != numSubTasks * numTasks) {
			auto err = cweeStr::printf("Pattern had %i values instead of %i", pat.GetNumValues(), numSubTasks * numTasks);
			printf(err);
		}
	}
	printf("Job 7a\n");
	if (true) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		fibers::containers::vector<int> list;

		fibers::parallel::For(0, numTasks, [&counter, &list, numSubTasks](int i) {
			fibers::parallel::For(0, numSubTasks, [&counter, &list](int j) {
				list.push_back(
					counter->fetch_add(1)
				); // do work
			});
		});
	}

	auto x = fibers::parallel::async([](int x) { return 100.0f; }, 10).wait_get(); // returns 100.0f
	int result1 = fibers::Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wrapper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = fibers::Job([](float& x)->float { return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	auto __awaiter__ = fibers::Job([]() { return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, and guarrantees its completion before the scope ends.
	(void)fibers::Job([]() { return cweeStr("HELLO"); }).AsyncFireAndForget(); // Queues the job to take place on a fiber/thread, and it is the user's job to guarrantee it is safe to do so when the scope ends.

	try {
		fibers::Job([&]() { return x; }).AsyncFireAndForget(); // Queues the job to take place on a fiber/thread, and it is the user's job to guarrantee it is safe to do so when the scope ends.
		throw(std::runtime_error("This should not happen"));
	}catch(std::runtime_error){
		// we anticipate that it would fail! The user attempted to cast a capturing lambda which may have gone out-of-scope. 
	}

	printf("Loop done\n");

	return 10;



#undef EXPECT_EQ
#undef EXPECT_NE
};












#include "../ExcelInterop/Wrapper.h"

class Win32ConsoleSupport {
public:
	static void clearLine() {
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen;
		DWORD written;

		GetConsoleScreenBufferInfo(console, &screen);

		COORD pos = { 0, screen.dwCursorPosition.Y };

		FillConsoleOutputCharacterA(
			console, ' ', screen.dwSize.X * 1, pos, &written
		);
		FillConsoleOutputAttribute(
			console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
			screen.dwSize.X * 1, pos, &written
		);
		SetConsoleCursorPosition(console, pos);
	};
};

/* Linux style terminal for multithread friendly processing. */
static auto GetUserInput() {
	return fibers::parallel::async([]() {
		std::cout << std::endl; // new line, since we'll be refreshing constantly on the current line, then using carriage return to reset the origin.
		cweeStr temp;
		while (1) {
			// if the user hasn't typed anything, keep thinking.                
			Win32ConsoleSupport::clearLine();
			Win32ConsoleSupport::clearLine();
			std::cout << "\r" << temp; // print the current text here, so that it doesn't appear to flash on screen.
			while (!_kbhit()) {
				cweeSysThreadTools::Sys_Yield();
			}
			unsigned char character = _getch(); // grab user key press. 	
			// bool shift_pressed = GetAsyncKeyState(VK_SHIFT);

			if (character == '\n' || character == '\r') { // new line
				std::cout << std::endl;
				break;
			}
			if (character == '\b') { // backspace
				temp = temp.Left(cweeMath::max(0, temp.Length() - 1)); // remove one character
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if ((int)character == 27) { // escape
				temp.Clear();
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if (character == 0 || character == 0xE0) {
				character = _getch();

				if (character == 72) { // up arrow		
					continue;
				}
				if (character == 80) { // down arrow	
					continue;
				}
				if (character == 75) { // left arrow	
					continue;
				}
				if (character == 77) { // right arrow	
					continue;
				}
				continue; // If another not-programmed key was pressed, skip. (i.e. Home button, PGUP, END, PGDOWN)
			}

			temp += (char)character; // all other keys, record to temp repo
		}
		return temp;
	});
};
static cweeStr UserMustSelectFile(fileType_t fileType = fileType_t::ANY_EXT) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().wait_get();// Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return fileSystem->getDataFolder() + "\\" + files[(int)reply];
	}
	else {
		return fileSystem->getDataFolder() + "\\" + reply.BestMatch(files);
	}
};
static cweeStr UserMustSelectFile(cweeStr fileType) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().wait_get();//Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return files[(int)reply];
	}
	else {
		return reply.BestMatch(files);
	}
};

/* Parallel thread to occasionally look for and process toast messages. Sleeps most of the time and wakes up to check for toasts. */
#if 1
static Timer parallel_toast_manager = Timer(0.1, Action(std::function([](cweeStr& title, cweeStr& content) {
	while (cweeToasts->tryGetToast(title, content)) std::cout << cweeStr::printf("\n/* WaterWatch Toast: \t\"%s\": \t\"%s\" */\n\n", title.c_str(), content.c_str());
}), cweeStr(), cweeStr()));
#endif

// Handle async or scripted AppRequests. 
#if 1
static Timer AppLayerRequestProcessor = Timer(0.01, Action(std::function([]() {
	std::pair<
		int, // ID
		cweeUnion<
			cweeStr, // Function Name
			cweeThreadedList<cweeStr>, // Arguments (optional)
			cweeSharedPtr<cweeStr> // result ptr?
		>
	> request = AppLayerRequests->DequeueRequest();
	if (request.first >= 0) {
#pragma warning(disable : 4573)			// the usage of 'cweeStr::Hash' requires the compiler to capture 'this' but the current default capture mode does not allow it
		cweeStr result;
		cweeStr& funcName = request.second.get<0>();
		cweeThreadedList<cweeStr>& args = request.second.get<1>();
		switch (static_cast<size_t>(funcName.Hash())) {
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFile")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFolder")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SavePassword")): // server, username, password
			if (args.Num() >= 3) fileSystem->saveWindowsPassword(args[0], args[1], args[2]);
			else result = "Arguments required: 'account_name', 'user_name', 'password'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_LoadPassword")):
			if (args.Num() >= 2) result = fileSystem->retrieveWindowsPassword(args[0], args[1]);
			else result = "Arguments required: 'account_name', 'user_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_ThemeColor")):
			if (args.Num() >= 1) result = "[255,255,255,255]";
			else result = "Arguments required: 'color_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetUserName")):
			result = "Win32 Project";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetMousePosition")):
			result = "[0,0]";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SaveSetting")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetSetting")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber")): 
		    {
				if (args.Num() >= 2) result = std::to_string(Example::ExampleF(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
				//if (args.Num() >= 2) result = std::to_string(Example::ExampleF(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
				else result = "Arguments required: 'num_tasks', 'num_subtasks'";
			}
			break;
		default:
			// unknown function.
			result = "Function Not Found";
			break;
		}
		AppLayerRequests->FinishRequest(request.first, result); // let the app set the result and end this dequeue
#pragma warning(default : 4573)	
	}
})));
#endif

static cweeStr GetHeaderString() {
	cweeStr toRet = "Welcome to the WaterWatch Sample App.\n";
	toRet += "Data Directory = " + fileSystem->getDataFolder() + "\n";
	toRet += "Begin Scripting:\n\n";
	return toRet;
}

#define AddFunctionToLib(lib, name, todo, ...){ \
	auto varNames = cweeStr(#__VA_ARGS__).RemoveBetween("<", ">").ReplaceInline("*", " ").ReplaceInline("&", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").Split(",").Trim(' ').ReplaceInline("  ", " ").SplitAgain(" ").Trim(' ').GetEveryOtherVar(); \
	lib->add(fun([](__VA_ARGS__) { todo; }, varNames), #name);	\
}

int main() {
	using namespace cwee_units;
	
	std::cout << GetHeaderString() << std::endl;

	cweeStr prevLine = "";
	cweeStr command = "";
	std::shared_ptr<chaiscript::WaterWatch_ChaiScript> engine = std::make_shared<chaiscript::WaterWatch_ChaiScript>();
	while (true) {
		AUTO input = GetUserInput();
		cweeStr str = input.wait_get();

		if (str == "Fiber") {
			int x = 1000;
			int y = 10;
			
			try {
				fibers::Job(Example::ExampleF, (int)y, (int)x).AsyncInvoke().Wait();
			}
			catch (std::runtime_error err) {
				std::cout << "FIBER TEST ERROR: " << err.what() << std::endl;
			}
			
		}
		if (str == "Exit")
		{
			return 0;
		}
		if (str == "Reset") { 
			engine = std::make_shared<chaiscript::WaterWatch_ChaiScript>();
			command = ""; 
			continue; 
		} 
		if (str == "" && prevLine == "") {
			Stopwatch sw;
			sw.Start();

			cweeStr result;
			try {
				AUTO bv = engine->eval(command.c_str());
				result = engine->to_string(bv).c_str();
			} catch (std::exception e) {
				result = e.what();
			}

			sw.Stop();
			std::cout << "Time Elapsed; " << (second_t)sw.Seconds_Passed() << std::endl;
			std::cout << "Current DateTime: " << ((cweeTime)fileSystem->getCurrentTime()).c_str() << std::endl;

			std::cout << result << std::endl;

			command = "";
		}

		command += str;
		command += "\n";
		prevLine = str;
	}
}