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

int Example::ExampleF(int numTasks, int numSubTasks) {
	int* xyzwabc = new int[10000];
	defer(delete[] xyzwabc); // does clean-up on our behalf on scope end

#if 1
#define EXPECT_EQ(a, b) if (a == b) {} else { std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }
#define EXPECT_NE(a, b) if (a != b) {} else { std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }
	if (1) {
		// Check the atomic_number system is not broken
		if (1) {
			fibers::utilities::CAS_Container<double> value{ 1 };
			std::cout << value.load() << std::endl; // 1

			value.Add(1);
			std::cout << value.load() << std::endl; // 2

			value.Add(1);
			std::cout << value.load() << std::endl; // 3

			value.Add(-5);
			std::cout << value.load() << std::endl; // -2

			value.Add(1.999995);
			std::cout << value.load() << std::endl; // -0.000005

		}


		if (1) {
			fibers::synchronization::atomic_number<double> value;
			std::cout << "\n\tValue (Befor Add): " << value.load() << std::endl << std::endl;
			fibers::parallel::For(0, 10000, [&value](int jobNum) {
				value.Add(0.125);
			});
			std::cout << "\n\tValue (After Add): " << value.load() << std::endl << std::endl;
			fibers::parallel::For(0, 10000, [&value](int jobNum) {
				value.Sub(0.125);
			});
			std::cout << "\n\tValue (After Sub): " << value.load() << std::endl << std::endl;

			value = 0;

			{
				std::atomic<int> shared;
				std::atomic<int> largest;
				fibers::parallel::For((size_t)0, (size_t)(fibers::utilities::Hardware::GetNumCpuCores()), [kExecNum = 1e5, &shared, &largest](size_t threadNum) {
					auto thisLargest = shared.fetch_add(1);

					::Sleep(1000);

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

					::Sleep(1000);

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

					::Sleep(1000);

					if (largest.load() < thisLargest)
						largest.store(thisLargest);

					shared.fetch_add(-1);
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

			Units::meter test1 = 1_ft - 1_m; // forces the result to meter (default, SI unit for length)
			Units::yard test2 = Units::foot(1_ft) + Units::meter(-1_m); // forces the result to yards
			auto test3 = Units::foot(1_ft) - Units::meter(1_m); // allows any resulting unit so long as the value is correct for the unit selected. E.g. could be foot, meter, cm, etc. In this case it'll be foot.

			std::cout << test1 << std::endl;
			std::cout << test2 << std::endl;
			std::cout << test3 << std::endl;

			// test unit canceling (should become unitless, like a normal double)
			std::cout << (5_ft / Units::meter(2_ft)).ToString() << std::endl;

			// test exact unit discovery (should discover that m/s is a valid, known unit type and report it correctly)
			std::cout << (10_m / 2_s).ToString() << std::endl;

			// test close unit discovery (gal*min/s does not exist, and it's resulting ratio is still not going to be found). Should find a valid volume unit type, but we will not know which (e.g. could be L or mL or m^3 -- all would be legal as long as the volume conversion is correct still).
			std::cout << ((10_gal / -60_s) * -1_min).ToString() << std::endl;

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
		if (1) {
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





            }
		}

		// Atomic BW Tree
		if (1) {
			using namespace fibers::utilities::dbgroup::index::bw_tree;
			if (1) {
				BwTree<uint64_t, uint64_t> tree;
				tree.Insert(5, 100);
				EXPECT_EQ(true, tree.Read(5).has_value());
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 50, [&tree](int i) {
					tree.Insert(i, i);
				});
			}
			if (1) {
				BwTree<int, double> tree;
				tree.Insert(5, 100);
				EXPECT_EQ(true, tree.Read(5).has_value());
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 500, [&tree](int i) {
					tree.Insert(i, i);
				});
			}
			if (1) {
				BwTree<double, double> tree;
				tree.Insert(5, 100.0);
				EXPECT_EQ(true, tree.Read(5).has_value() && (tree.Read(5).value() == 100.0));
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 5000, [&tree](int i) {
					tree.Insert(i, i);
				});
			}
			if (1) {
				BwTree<long double, double> tree;
				tree.Insert(5, 100.0);
				EXPECT_EQ(true, tree.Read(5).has_value() && (tree.Read(5).value() == 100.0));
				tree.Delete(5);
				EXPECT_EQ(false, tree.Read(5).has_value());

				fibers::parallel::For(0, 500, [&tree](int i) {
					tree.Insert(i, i);
				});


				auto futureObj = fibers::parallel::async([&tree](){
					::Sleep(10);
					std::cout << "\nWorking... \n";
					for (int i = 0; i < 500; i++) {
						if (i % 50 == 0) ::Sleep(1);

						tree.Write(i - 1, i + 1);
						tree.Insert(i + 0.5, i + 0.5);						
					}
					std::cout << "\n ...Finished.\n";
				});

#if 1
				for (double D = -2.25; D <= 505; D += 1.25) {
					auto iter_Larger = tree.FindSmallestLargerEqual(D);
					auto iter_Smaller = tree.FindLargestSmallerEqual(D);
					
					if (iter_Larger && iter_Smaller) {
						std::cout << "\tTarget Key: " << D << ", Smaller Key/Value:" << iter_Smaller.GetKey() << "/" << iter_Smaller.GetPayload() << ", Larger Key/Value: " << iter_Larger.GetKey() << "/" << iter_Larger.GetPayload() << std::endl;
					}
				}

				futureObj.wait();
#else
				for (auto& x : tree) {
					std::cout << "\tKey: " << x.first << ", Value: " << x.second << std::endl;
					::Sleep(1);
				}
				futureObj.wait();
#endif

			}

			if (1) {
				BwTree<long double, double> tree;

				fibers::parallel::For(0, 500, [&tree](int i) {
					tree.Insert(i, i);
				});

				auto futureObj = fibers::parallel::async([&tree]() {
					std::cout << "\nWorking... \n";

					for (auto iter = tree.Scan(); iter; iter++) {
						tree.Write(iter.GetKey(), iter.GetPayload() + cweeRandomFloat(-1, 1));
					}
					for (auto iter = tree.Scan(); iter; iter++) {
						tree.Write(iter.GetKey(), iter.GetPayload() + cweeRandomFloat(-1, 1));
					}
					for (auto iter = tree.Scan(); iter; iter++) {
						tree.Write(iter.GetKey(), iter.GetPayload() + cweeRandomFloat(-1, 1));
					}
					for (auto iter = tree.Scan(); iter; iter++) {
						tree.Write(iter.GetKey(), iter.GetPayload() + cweeRandomFloat(-1, 1));
					}

					std::cout << "\n ...Finished.\n";
				});

				for (double D = -2.25; D <= 505; D += 1.25) {
					auto iter_Larger = tree.FindSmallestLargerEqual(D);
					auto iter_Smaller = tree.FindLargestSmallerEqual(D);

					if (iter_Larger && iter_Smaller) {
						std::cout << "\tTarget Key: " << D << ", Smaller Key/Value:" << iter_Smaller.GetKey() << "/" << iter_Smaller.GetPayload() << ", Larger Key/Value: " << iter_Larger.GetKey() << "/" << iter_Larger.GetPayload() << std::endl;
					}
				}

				futureObj.wait();
			}

		}





		// Multi-word Compare-and-Swap
		if (1) {
			// 2-word pairs
			{
				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();

				// the number of MwCAS operations in each thread
				constexpr size_t kExecNum = 1e6;

				// use an unsigned long type as MwCAS targets
				using Target = uint64_t;

				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				// targets of a 2wCAS example
				Target word_1 = 0;
				Target word_2 = 0;

				fibers::parallel::For((size_t)0, kThreadNum, [kExecNum, &word_1, &word_2](size_t threadNum) {
					for (size_t i = 0; i < kExecNum; ++i) {
						// continue until a MwCAS operation succeeds
						while (true) {
							// create a MwCAS descriptor
							MwCASDescriptor<2> desc{};

							// prepare expected/desired values
							const auto old_1 = MwCASDescriptor<2>::Read<Target>(&word_1);
							const auto new_1 = old_1 + 1;
							const auto old_2 = MwCASDescriptor<2>::Read<Target>(&word_2);
							const auto new_2 = old_2 + 1;

							// register MwCAS targets with the descriptor
							desc.AddMwCASTarget(&word_1, old_1, new_1);
							desc.AddMwCASTarget(&word_2, old_2, new_2);

							// try MwCAS
							if (desc.MwCAS()) break;
						}
					}
					});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\t1st field: " << word_1 << "\n\t2nd field: " << word_2 << std::endl << std::endl;
			}
			
			// 3-word pairs
			{
				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();

				// the number of MwCAS operations in each thread
				constexpr size_t kExecNum = 1e6;

				// use an unsigned long type as MwCAS targets
				using Target = uint64_t;

				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				// targets of a mwCAS example
				Target word_1 = 0; // back PTR
				Target word_2 = 0; // this data PTR
				Target word_3 = 0; // next PTR

				fibers::parallel::For((size_t)0, kThreadNum, [kExecNum, &word_1, &word_2, &word_3](size_t threadNum) {
					for (size_t i = 0; i < kExecNum; ++i) {
						// continue until a MwCAS operation succeeds
						while (true) {
							// create a MwCAS descriptor
							MwCASDescriptor<3> desc{};

							// prepare expected/desired values
							const auto old_1 = MwCASDescriptor<3>::Read<Target>(&word_1);
							const auto new_1 = old_1 + 1;
							const auto old_2 = MwCASDescriptor<3>::Read<Target>(&word_2);
							const auto new_2 = old_2 + 2;
							const auto old_3 = MwCASDescriptor<3>::Read<Target>(&word_3);
							const auto new_3 = old_3 + 4;

							// register MwCAS targets with the descriptor
							desc.AddMwCASTarget(&word_1, old_1, new_1);
							desc.AddMwCASTarget(&word_2, old_2, new_2);
							desc.AddMwCASTarget(&word_3, old_3, new_3);

							// try MwCAS
							if (desc.MwCAS()) break;

						}
					}
					});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\t1st field: " << word_1 << "\n\t2nd field: " << word_2 << "\n\t3rd field: " << word_3 << std::endl << std::endl;
			}

			// 3-word pairs, updated in 2-word batches (A&&C or B&&C)
			{
				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();

				// the number of MwCAS operations in each thread
				constexpr size_t kExecNum = 1e6;

				// use an unsigned long type as MwCAS targets
				using Target = uint64_t;

				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				// targets of a mwCAS example
				Target word_1 = 0; // back PTR
				Target word_2 = 0; // this data PTR
				Target word_3 = 0; // next PTR

				fibers::parallel::For((size_t)0, kThreadNum, [kExecNum, &word_1, &word_2, &word_3](size_t threadNum) {
					if (threadNum % 2 == 0) {
						// A && C
						for (size_t i = 0; i < kExecNum; ++i) {
							// continue until a MwCAS operation succeeds
							while (true) {
								// create a MwCAS descriptor
								MwCASDescriptor<2> desc{};

								// prepare expected/desired values
								const auto old_1 = MwCASDescriptor<2>::Read<Target>(&word_1);
								const auto new_1 = old_1 + 2;
								const auto old_3 = MwCASDescriptor<2>::Read<Target>(&word_3);
								const auto new_3 = old_3 + 4;

								// register MwCAS targets with the descriptor
								desc.AddMwCASTarget(&word_1, old_1, new_1);
								desc.AddMwCASTarget(&word_3, old_3, new_3);

								// try MwCAS
								if (desc.MwCAS()) break;

							}
						}
					}
					else {
						// B && C
						for (size_t i = 0; i < kExecNum; ++i) {
							// continue until a MwCAS operation succeeds
							while (true) {
								// create a MwCAS descriptor
								MwCASDescriptor<2> desc{};

								// prepare expected/desired values
								const auto old_2 = MwCASDescriptor<2>::Read<Target>(&word_2);
								const auto new_2 = old_2 + 4;
								const auto old_3 = MwCASDescriptor<2>::Read<Target>(&word_3);
								const auto new_3 = old_3 + 4;

								// register MwCAS targets with the descriptor
								desc.AddMwCASTarget(&word_2, old_2, new_2);
								desc.AddMwCASTarget(&word_3, old_3, new_3);

								// try MwCAS
								if (desc.MwCAS()) break;

							}
						}
					}
				});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\t1st field: " << word_1 << "\n\t2nd field: " << word_2 << "\n\t3rd field: " << word_3 << std::endl << std::endl;
			}

			// Generic multi-word compare and swap with types of various sizes
			{
				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();

				// the number of MwCAS operations in each thread
				constexpr size_t kExecNum = 1e5;

				// use an unsigned long type as MwCAS targets

				// target of a mwCAS example
				fibers::utilities::CAS_Container<int> data{ 0 };

				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::parallel::For((size_t)0, kThreadNum, [kExecNum, &data](size_t threadNum) {
					for (size_t i = 0; i < kExecNum; ++i) {
						data.Add(1);
					}
				});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\tValue: " << data.load() << std::endl << std::endl;
			}
			{
				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();

				// the number of MwCAS operations in each thread
				constexpr size_t kExecNum = 1e5;

				// use an unsigned long type as MwCAS targets

				// target of a mwCAS example
				fibers::utilities::CAS_Container<float> data{ 0 };

				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::parallel::For((size_t)0, kThreadNum, [kExecNum, &data](size_t threadNum) {
					for (size_t i = 0; i < kExecNum; ++i) {
						data.Add(1);
					}
				});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\tValue: " << data.load() << std::endl << std::endl;
			}
			{
				size_t kThreadNum = fibers::utilities::Hardware::GetNumCpuCores();

				// the number of MwCAS operations in each thread
				constexpr size_t kExecNum = 1e5;

				// use an unsigned long type as MwCAS targets

				// target of a mwCAS example
				fibers::utilities::CAS_Container<long double> data{ 0 };

				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::parallel::For((size_t)0, kThreadNum, [kExecNum, &data](size_t threadNum) {
					for (size_t i = 0; i < kExecNum; ++i) {
						data.Add(1);
					}
				});

				// check whether MwCAS operations are performed consistently
				std::cout << "\n\tValue: " << data.load() << std::endl << std::endl;
			}
		}


#if 0
		if (1) {
			{
				BalancedTreeNode<int, const char> parentOfParents{ 0 }; parentOfParents.Reset();
				BalancedTreeNodeParentReference<int, const char> parentOfParentsRef{ &parentOfParents };

				BalancedTreeNode<int, const char> parent1{ 0 }; parent1.Reset();
				BalancedTreeNodeParentReference<int, const char> parentRef1{ &parent1 };
				BalancedTreeNode<int, const char> child1{ 0 }; child1.Reset();
				BalancedTreeNode<int, const char> child2{ 0 }; child2.Reset();

				BalancedTreeNode<int, const char> parent2{ 0 }; parent2.Reset();
				BalancedTreeNodeParentReference<int, const char> parentRef2{ &parent2 };
				BalancedTreeNode<int, const char> child3{ 0 }; child3.Reset();
				BalancedTreeNode<int, const char> child4{ 0 }; child4.Reset();

				{
					parentOfParents.object = "parentOfParents";
					parent1.object = "Parent1";
					parent2.object = "Parent2";
					child1.object = "Child1";
					child2.object = "Child2";
					child3.object = "Child3";
					child4.object = "Child4";

					parent1.parent = &parentOfParentsRef;
					parent2.parent = &parentOfParentsRef;

					child1.parent = &parentRef1;
					child2.parent = &parentRef1;
					child1.prev = nullptr;
					child1.next = &child2;
					child2.prev = &child1;
					child2.next = nullptr;
					parent1.firstChild = &child1;
					parent1.lastChild = &child2;
					parent1.numChildren = 2;

					child3.parent = &parentRef2;
					child4.parent = &parentRef2;
					child3.prev = nullptr;
					child3.next = &child4;
					child4.prev = &child3;
					child4.next = nullptr;
					parent2.firstChild = &child3;
					parent2.lastChild = &child4;
					parent2.numChildren = 2;

					parent1.next = &parent2;
					parent2.prev = &parent1;

					parentOfParents.firstChild = &parent1;
					parentOfParents.lastChild = &parent2;
					parentOfParents.numChildren = 2;
				}

				std::cout << "\n\t\tChild1 Parent: " << child1.GetParent()->object << std::endl;
				std::cout << "\t\tChild2 Parent: " << child2.GetParent()->object << std::endl;
				std::cout << "\t\tChild3 Parent: " << child3.GetParent()->object << std::endl;
				std::cout << "\t\tChild4 Parent: " << child4.GetParent()->object << std::endl;

				std::cout << "\t\tParent1 Num Children: " << parent1.numChildren << std::endl;
				std::cout << "\t\tParent1 First Child: " << parent1.firstChild->object << std::endl;
				std::cout << "\t\tParent1 Last Child: " << parent1.lastChild->object << std::endl;

				std::cout << "\t\tParent2 Num Children: " << parent2.numChildren << std::endl;
				std::cout << "\t\tParent2 First Child: " << parent2.firstChild->object << std::endl;
				std::cout << "\t\tParent2 Last Child: " << parent2.lastChild->object << std::endl;

				std::cout << "\t\tParentOfParents NumChildren: " << parentOfParents.numChildren << std::endl;

				BalancedTreeNode<int, const char>::MergeNodes(&parent1, &parent2);

				std::cout << "\n\t\tChild1 Parent: " << child1.GetParent()->object << std::endl;
				std::cout << "\t\tChild2 Parent: " << child2.GetParent()->object << std::endl;
				std::cout << "\t\tChild3 Parent: " << child3.GetParent()->object << std::endl;
				std::cout << "\t\tChild4 Parent: " << child4.GetParent()->object << std::endl;

				std::cout << "\t\tParent1 Num Children: " << parent1.numChildren << std::endl;
				std::cout << "\t\tParent1 First Child: " << parent1.firstChild->object << std::endl;
				std::cout << "\t\tParent1 Last Child: " << parent1.lastChild->object << std::endl;

				std::cout << "\t\tParent2 Num Children: " << parent2.numChildren << std::endl;
				std::cout << "\t\tParent2 First Child: " << parent2.firstChild->object << std::endl;
				std::cout << "\t\tParent2 Last Child: " << parent2.lastChild->object << std::endl;

				std::cout << "\t\tParentOfParents NumChildren: " << parentOfParents.numChildren << std::endl;

			}
			{
				fibers::utilities::Allocator<cweeStr, 128> str_alloc;
				fibers::utilities::Allocator<BalancedTreeNode<int, const char>, 128> alloc1;
				fibers::utilities::Allocator<BalancedTreeNodeParentReference<int, const char>, 128> alloc2;

				fibers::synchronization::shared_mutex<fibers::synchronization::mutex> rootLock;
				fibers::synchronization::atomic_ptr< BalancedTreeNode<int, const char> > root = alloc1.Alloc();
				root->Reset();
				root->key = 0;
				root->object = "Root!";

				for (int i = 1; i < 50; i++) {
					auto str_ptr = str_alloc.Alloc();
					str_ptr->operator=(cweeStr::printf("Object%i", i));
					BalancedTreeNode<int, const char>::AddChild(rootLock, 10, root, str_ptr->c_str(), i, [&alloc1]() { return alloc1.Alloc(); }, [&alloc2](BalancedTreeNode<int, const char>* p) { auto x = alloc2.Alloc(); x->parent = p; return x; });
				}

				auto GetNameFrom = [](BalancedTreeNode<int, const char>* p) -> const char* { if (!p || !p->object) { return ""; } else { return p->object; }};
				BalancedTreeNode<int, const char>* child = root;
				while (child) {
					std::cout << std::endl;

					std::cout << "\t" << GetNameFrom(child) << std::endl;
					std::cout << "\t\t Key: " << child->key << std::endl;
					std::cout << "\t\t Parent: " << GetNameFrom(child->GetParent()) << std::endl;
					std::cout << "\t\t Prev <-: " << GetNameFrom(child->GetPrev()) << std::endl;
					std::cout << "\t\t Next ->: " << GetNameFrom(child->GetNext()) << std::endl;
					std::cout << "\t\t First Child /: " << GetNameFrom(child->GetFirstChild()) << std::endl;
					std::cout << "\t\t Last Child \\: " << GetNameFrom(child->GetLastChild()) << std::endl;
					std::cout << "\t\t Num Children #: " << child->GetNumChildren() << std::endl;

					child = BalancedTreeNode<int, const char>::GetNextLeaf(child);
				}







			}

			{
				fibers::utilities::Allocator<cweeStr, 128> str_alloc;
				fibers::utilities::Allocator<BalancedTreeNode<int, const char>, 128> alloc1;
				fibers::utilities::Allocator<BalancedTreeNodeParentReference<int, const char>, 128> alloc2;

				fibers::synchronization::shared_mutex<fibers::synchronization::mutex> rootLock;
				fibers::synchronization::atomic_ptr< BalancedTreeNode<int, const char> >  root = alloc1.Alloc();
				root->Reset();
				root->key = 0;
				root->object = "Root!";

				fibers::parallel::For(0, 50, [&str_alloc, &root, &alloc1, &alloc2, &rootLock](int i) {
					auto str_ptr = str_alloc.Alloc();
					str_ptr->operator=(cweeStr::printf("Object%i", i));
					BalancedTreeNode<int, const char>::AddChild(rootLock, 10, root, str_ptr->c_str(), i, [&alloc1]() { return alloc1.Alloc(); }, [&alloc2](BalancedTreeNode<int, const char>* p) { auto x = alloc2.Alloc(); x->parent = p; return x; });
				});

				auto GetNameFrom = [](BalancedTreeNode<int, const char>* p) -> const char* { if (!p || !p->object) { return ""; } else { return p->object; }};
				BalancedTreeNode<int, const char>* child = root;
				while (child) {
					std::cout << std::endl;

					std::cout << "\t" << GetNameFrom(child) << std::endl;
					std::cout << "\t\t Key: " << child->key << std::endl;
					std::cout << "\t\t Parent: " << GetNameFrom(child->GetParent()) << std::endl;
					std::cout << "\t\t Prev <-: " << GetNameFrom(child->GetPrev()) << std::endl;
					std::cout << "\t\t Next ->: " << GetNameFrom(child->GetNext()) << std::endl;
					std::cout << "\t\t First Child /: " << GetNameFrom(child->GetFirstChild()) << std::endl;
					std::cout << "\t\t Last Child \\: " << GetNameFrom(child->GetLastChild()) << std::endl;
					std::cout << "\t\t Num Children #: " << child->GetNumChildren() << std::endl;

					child = BalancedTreeNode<int, const char>::GetNextLeaf(child);
				}
			}
		}
#endif


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

	{
		int ijk = 0;
		for (ijk = 0; ijk < 100; ijk++) {
			fibers::containers::Stack<double> concurrent_set;

			concurrent_set.push(0);
			concurrent_set.push(1);
			concurrent_set.push(2);
			concurrent_set.push(3);
			concurrent_set.push(4);
			concurrent_set.push(5);

			concurrent_set.try_remove(3);

			if (concurrent_set.contains(3)) {
				std::cout << "TEST" << std::endl;
			}

			double x{ 0 };
			if (concurrent_set.try_pop(x)) {
				std::cout << x << std::endl;
			}

			fibers::parallel::For(0, 400, [&concurrent_set](int i) {
				if (i % 2 == 0) {
					for (int k = i; k < (i + 400); k++) {
						concurrent_set.push(k);
					}
				}
				else {
					for (int k = i; k < (i + 400); k += 2) {
						concurrent_set.try_remove(k);
					}
				}
			});

			if (concurrent_set.contains(1)) {
				concurrent_set.try_remove(1);
			}
		}
		for (ijk = 0; ijk < 100; ijk++) {
			{
				fibers::containers::Stack<stackThing> concurrent_set; {
					for (int i = 0; i < 100; i++) {
						concurrent_set.push(stackThing(cweeStr(i), (int)i));
					}
				}
			}
			{
				fibers::containers::Stack<stackThing> concurrent_set; {
					fibers::parallel::For(0, 100, [&](int i) {
						int index = i % 10;
						int index2 = (i+1) % 10;

						concurrent_set.push(stackThing(cweeStr(index), (int)index));
						concurrent_set.try_remove(stackThing(cweeStr(index2), (int)index2));
					});
				}
			}

			/*
			concurrent_set.push(stackThing("integer", 100));
			concurrent_set.push(stackThing("double", 100.0));
			concurrent_set.push(stackThing("long", 100l));
			concurrent_set.push(stackThing("long long", 100ll));
			concurrent_set.push(stackThing("long double", 100.0l));

			concurrent_set.try_remove(stackThing("long"));

			if (concurrent_set.contains(stackThing("long"))) {
				std::cout << "FAILED" << std::endl;
			}

			std::shared_ptr<stackThing> x;
			if (concurrent_set.try_pop(x)) {
				if (x) {
					std::cout << x->varName << std::endl;
				}
			}
			stackThing y;
			if (concurrent_set.try_pop(y)) {
				if (x) {
					std::cout << y.varName << std::endl;
				}
			}

			fibers::parallel::For(0, 400, [&concurrent_set](int i) {
				if (i % 2 == 0) {
					for (int k = i; k < (i + 400); k++) {
						concurrent_set.push(stackThing(cweeStr(k), k));
					}
				}
				else {
					for (int k = i; k < (i + 400); k += 2) {
						concurrent_set.try_remove(stackThing(cweeStr(k), k));
					}
				}
			});*/

		}
	}

	if (0) {
		for (int j = 1; j < 10; j += 2) {
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

			printf("\n");
		}
		

		for (int j = 1; j < 10; j += 2) {
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

			printf("\n");
		}
		for (int j = 1; j < 10; j += 2) {
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

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count << ")" << std::endl;
			}

			printf("SpeedTest (unit values) ");
			std::cout << j;
			printf(" (Threads) : ");
			{
				Units::gallon_per_minute count;
				Stopwatch sw; sw.Start();
				int i, k;

				std::vector<int> vec(numLoops, 0);
				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&count, &j](int& V) {
					for (int k = 0; k < j; k++)
						count += k;
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
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

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count << ")" << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 2) {
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
		for (int j = 1; j < 10; j += 2) {
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

			printf("SpeedTest (vector search) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr());
				fibers::parallel::For(0, numLoops, [&vec](int i) { vec[i] = cweeStr(i); });

				Stopwatch sw; sw.Start();

				for (int i = 0; i < numLoops; i++) {
					(void)fibers::parallel::Find(vec, [j = cweeStr((int)random_fast(0, numLoops - 1))](cweeStr const& x) ->bool { return x == j; });
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 2) {
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

			printf("SpeedTest (matrix overwriting) ");
			std::cout << j;
			printf(" (Fibers Foreach w/ Fibers as Combined Job Stack) : ");
			{
				std::vector<std::vector<cweeStr>> vec(numLoops, std::vector<cweeStr>(j, cweeStr("TEST")));

				Stopwatch sw; sw.Start();

				fibers::JobGroup group;
				fibers::parallel::ForEach(group, vec, [&j, &group](std::vector<cweeStr>& vec) {
					fibers::parallel::For(group, 0, j, [&vec](int k) {
						vec[k] = cweeStr(k);
					});
				});
				group.Wait(); // only one "wait" command, allowing the job system to better jump between jobs / cells of the matrix.

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
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

	auto jobTest = fibers::Job([]() { return 10.0f; });
	jobTest.AsyncFireAndForget(); // will not crash

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
	printf("Job 7b\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));

		fibers::JobGroup group;
		for (int i = 0; i < numTasks; ++i){
			group.Queue(fibers::Job([&numSubTasks, &counter](int i) {
				fibers::JobGroup group;
				for (int j = 0; j < numSubTasks; ++j) {
					group.Queue(fibers::Job([&counter](int j) {
						counter->fetch_add(1);
					}, (int)j));
				}
				group.Wait();
			}, (int)i));
		};
		group.Wait();
	}
	printf("Job 7c\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		auto job = cweeJob([&counter, &numTasks, &numSubTasks]() {
			fibers::JobGroup group;
			for (int i = 0; i < numTasks; ++i) {
				group.Queue(fibers::Job([&numSubTasks, &counter](int i) {
					fibers::JobGroup group;
					for (int j = 0; j < numSubTasks; ++j) {
						group.Queue(fibers::Job([&counter](int j) {
							counter->fetch_add(1);
						}, (int)j));
					}
					group.Wait();
				}, (int)i));
			};
			group.Wait();
		});
		job.AsyncInvoke();
		job.Await();
	}
	printf("Job 7d\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		fibers::containers::vector<int> list;
		auto job = cweeJob([&counter, &numTasks, &numSubTasks, &list]() {
			fibers::parallel::For(0, numTasks, [&counter, &list, numSubTasks](int i) {
				fibers::parallel::For(0, numSubTasks, [&counter, &list](int j) {
					list.push_back(
						counter->fetch_add(1)
					); // do work
				});
			});
		});
		job.AsyncInvoke();
		job.Await();
		return counter->load();
	}

	auto x = fibers::parallel::async([](int x) { return 100.0f; }, 10).wait_get(); // returns 100.0f
	size_t t = fibers::parallel::For(0, numTasks * numSubTasks, [](int i) { return i; }).size();

	int result1 = fibers::Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wrapper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = fibers::Job([](float& x)->float { return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	auto __awaiter__ = fibers::Job([]() { return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, and guarrantees its completion before the scope ends.
	fibers::Job([]() { return cweeStr("HELLO"); }).AsyncFireAndForget(); // Queues the job to take place on a fiber/thread, and it is the user's job to guarrantee it is safe to do so when the scope ends.

	try {
		fibers::Job([&]() { return t; }).AsyncFireAndForget(); // Queues the job to take place on a fiber/thread, and it is the user's job to guarrantee it is safe to do so when the scope ends.
		throw(std::runtime_error("This should not happen"));
	}catch(std::runtime_error){
		// we anticipate that it would fail!
	}




	printf("Loop done\n");

	return t;
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
		cweeStr str = input.wait_get();//Await().cast();

		if (str == "Fiber") {
			int x = 1000;
			int y = 10;
			int z;
			while (true) {
				z = Example::ExampleF(y, x);
				if (y * x != z) {
					cweeStr err = cweeStr::printf("Something went wrong with the job system and a job returned %i instead of %i", z, y * x);
					throw(std::runtime_error(err.c_str()));
				}


				auto job = cweeJob(Example::ExampleF, (int)y, (int)x);
				job.AsyncInvoke();
				job.Await();

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