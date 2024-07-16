// FibersDebugConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include "../FiberTasks/Fibers.h"
#include <execution>

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
	stackThing& operator=(stackThing const& r) { varName = r.varName; var = r.var; }
	stackThing& operator=(stackThing&& r) { varName = std::move(r.varName); var = std::move(r.var); }
	~stackThing() { if (!varName.empty()) { std::cout << "DELETING " << varName << std::endl; } };

	bool operator==(stackThing const& a) const { return varName == a.varName; };
	bool operator!=(stackThing const& a) const { return varName != a.varName; };
};

#define EXPECT_EQ(a,b) [&]()->bool{ if ((a) == (b)) { return true; } else { std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; return false; } }()
#define EXPECT_NE(a, b) [&]()->bool{ if ((a) != (b)) { return true; } else { std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; return false; } }()
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

	std::allocator<int> alloc;

	while (1) {
		std::this_thread::yield();

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

		// NO LEAK w/ std::for_each which catches user errors
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

		// NO LEAK w/ fibers::parallel::For
		try {
			fibers::parallel::For(0, 100, [](int& i) {
				throw(std::runtime_error("Example Error"));
			});
		}
		catch (std::runtime_error const& err) {}

		// NO LEAK w/ fibers::parallel::ForEach
		try {
			fibers::utilities::Sequence seq(1000); // 0..999
			fibers::parallel::ForEach(seq, [](int& i) {
				throw(std::runtime_error("Example Error"));
			});
		}
		catch (std::runtime_error const& err) {}

		// NO LEAK w/ fibers::Any
		if (0) {
			fibers::Any test;
			test = 100.0f;
			test = std::string("TEST");
			test = std::make_shared<float>(100.0f);
			test = std::make_shared<std::string>("TEST");
			test = std::make_shared<float>(100.0f);
			test = std::make_shared<std::string>("TEST");
		}

		// NO LEAK w/ atomic_number<double>
		if (1) {
			fibers::utilities::Sequence seq(1000); // 0..999
			fibers::synchronization::atomic_number<double> D{ 0 };
			fibers::parallel::ForEach(seq, [&D](int i) {
				D.Increment();
			});
			EXPECT_EQ(D, 1000.0);
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
				 
		// NO LEAK w/ atomic_number<double>
		if (1) {
			fibers::synchronization::atomic_number<double> value;
			EXPECT_EQ(value.load(), 0);
			for (int i = 0; i < 10000; i++) {
				value.Add(0.125);
			}
			EXPECT_EQ(value.load(), 10000 * 0.125);
			for (int i = 0; i < 10000; i++) {
				value.Sub(0.125);
			}
			EXPECT_EQ(value.load(), 0);
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

			EXPECT_EQ((-5 / Units::foot(1_m)).ToString(), (5 / -1_m).ToString());
			EXPECT_EQ((1_ft).ToString(), "1 ft");
			EXPECT_EQ((5_ft / Units::meter(2_ft)).ToString(), "2.5");
			EXPECT_EQ((10_m / 2_s).ToString(), "5 mps");
			EXPECT_EQ(((1_m).pow(3)).ToString(), "1 cu_m");
			EXPECT_EQ(((4_sq_m).pow(0.5)).ToString(), "2 m");
			EXPECT_EQ((16_cu_m / 2_m).ToString(), "8 sq_m");
			// EXPECT_EQ((16_cu_ft / 2_ft).ToString(), "8 sq_ft"); // UGH

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

		// Atomic BW Tree
		if (1) {
			//
			if (1) {

				using treeType = fibers::utilities::dbgroup::index::bw_tree::BwTree<uint64_t, uint64_t>;
				// No Leak -->
				fibers::utilities::dbgroup::memory::EpochBasedGC<treeType::NodePage, treeType::DeltaPage> gc_{ fibers::utilities::dbgroup::index::bw_tree::kDefaultGCTime , fibers::utilities::dbgroup::index::bw_tree::kDefaultGCThreadNum };
				gc_.StartGC();
				// <-- No Leak

				// LEAK --> ... potential solution is to use an allocator / deallocator, since it SEEMS (need to confirm) that the mapping table does all of its own allocation / deallocations, and it is not using a garbage collector. (Maybe  it should!)
				treeType::MappingTable_t* mapping_table_ = new treeType::MappingTable_t();
				auto root_id = mapping_table_->GetNewPageID();
				auto* root_ptr = mapping_table_->GetLogicalPtr(root_id);
				delete mapping_table_;
				// <-- LEAK


				//auto root_id = mapping_table_.GetNewPageID();
				//auto* root_ptr = mapping_table_.GetLogicalPtr(root_id);
				









				//Node_t



				
				//auto* root_node = new (GetNodePage()) Node_t{};
				
				
				
				
				
				
				
				
				
				
				//root_ptr->Store(root_node);







				// create an empty Bw-tree
				// auto* root_node = new (GetNodePage()) Node_t{};
				//auto root_id = mapping_table_.GetNewPageID();
				//auto* root_ptr = mapping_table_.GetLogicalPtr(root_id);
				//root_ptr->Store(root_node);
				//root_.store(root_id, std::memory_order_relaxed);

				

			}
			// 
			if (0) {
				fibers::utilities::dbgroup::index::bw_tree::BwTree<uint64_t, uint64_t> tree;
			}
			// 
			if (0) {
				fibers::containers::Pattern<uint64_t, uint64_t> tree;
			}
			// 
			if (0) {
				fibers::containers::Pattern<uint64_t, uint64_t> tree;
				for (int i = 0; i < 50; i++) {
					tree.Insert(i, i);
				}
			}
			// 
			if (0) {
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
			// 
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
			// 
			if (0) {
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
			// 
			if (0) {
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
};
#undef EXPECT_EQ
#undef EXPECT_NE