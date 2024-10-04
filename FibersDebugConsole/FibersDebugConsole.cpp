// FibersDebugConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include "../FiberTasks/Fibers.h"
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
			// std::cout << Units::printf("DELETING %s\n", varName.c_str());
		}
	};

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

		// Type Conversions
		if (1) {
			// Static
			if (1) {
				fibers::details::Static_Type_Conversion_Impl<int, float> converter;

				EXPECT_EQ(converter.to(), boost::typeindex::type_id<float>().type_info());
				EXPECT_EQ(converter.from(), boost::typeindex::type_id<int>().type_info());

				EXPECT_EQ(converter.bidir(), true);
				EXPECT_EQ(converter.polymorphic(), false);
				EXPECT_EQ(converter.convert(1).IsTypeOf<float>(), true);
				EXPECT_EQ(converter.convert(1).cast<float>() == 1.0f, true);
				EXPECT_EQ(converter.convert_down(100.0f).IsTypeOf<int>(), true);
				EXPECT_EQ(converter.convert_down(100.0f).cast<int>() == 100, true);
			}

			// Dynamic
			if (1) {
				fibers::details::Dynamic_Type_Conversion_Impl< Units::foot, Units::value> converter;

				EXPECT_EQ(converter.to(), boost::typeindex::type_id<Units::value>().type_info());
				EXPECT_EQ(converter.from(), boost::typeindex::type_id<Units::foot>().type_info());

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
				auto converter{ fibers::details::Custom_Type_Conversion_Impl([](Units::foot const& a)->Units::value {
					return a;
				})};

				EXPECT_EQ(converter.to(), boost::typeindex::type_id<Units::value>().type_info());
				EXPECT_EQ(converter.from(), boost::typeindex::type_id<Units::foot>().type_info());

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
				auto converter{ fibers::details::Custom_Type_Conversion_Impl([](fibers::Any const& a)-> int {
					return 100; // always returns 100
				}) };

				EXPECT_EQ(converter.to(), boost::typeindex::type_id<int>().type_info());
				EXPECT_EQ(converter.from(), boost::typeindex::type_id<fibers::Any>().type_info());

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
				fibers::details::Type_Converter_Tree tree;

				fibers::Any result;
				EXPECT_EQ(true, tree.TryConvert(1.0f, boost::typeindex::type_id<float>().type_info(), result)); // no conversion = successful, no conversion necessary.

				EXPECT_EQ(false, tree.TryConvert(1, boost::typeindex::type_id<float>().type_info(), result)); // no conversion provided, so this should fail. 

				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, float>())); // {int -> float} AND {float -> int}

				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<float>().type_info(), result)); // int -> float 
				EXPECT_EQ(true, tree.TryConvert(1.0f, boost::typeindex::type_id<int>().type_info(), result)); // float -> int

				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, double>())); // {double -> float} AND {float -> double}
				
				EXPECT_EQ(true, tree.TryConvert(1.0, boost::typeindex::type_id<float>().type_info(), result)); // double -> float 
				EXPECT_EQ(true, tree.TryConvert(1.0f, boost::typeindex::type_id<double>().type_info(), result)); // float -> double
				
				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<double>().type_info(), result)); // int -> double
				EXPECT_EQ(true, tree.TryConvert(1.0, boost::typeindex::type_id<int>().type_info(), result)); // double -> int 







				
				
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, uint64_t>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<uint64_t, DateTime>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<DateTime, Units::second>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<Units::foot, Units::value>())); // polymorphic
				EXPECT_EQ(false, tree.AddConverter([](float v) -> double { return v; })); // expect false because this conversion should already exist

				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, Units::value>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, Units::foot>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, Units::meter>())); // static
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, Units::inch>())); // static


				EXPECT_EQ(true, tree.TryConvert(100, boost::typeindex::type_id<Units::value>().type_info(), result));
				EXPECT_EQ("100", result.cast< Units::value >().ToString());

				EXPECT_EQ(true, tree.TryConvert(100, fibers::Any::TypeOf<Units::foot>(), result));
				EXPECT_EQ("100 ft", result.cast< Units::foot >().ToString());





				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result));

				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result)); // int -> float -> double -> uint64_t -> DateTime -> Units::second
				// will NOT re-evaluate the conversion since it's been cached, and the tree has not been updated. 
				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result)); // int -> float -> double -> uint64_t -> DateTime -> Units::second

				// update the tree...
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<double, Units::second>()));
				// ... which will now re-evaluate the conversion and will use the faster conversion from now on. 
				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result)); // int -> float -> double -> Units::second

				// update the tree...
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<int, Units::second>()));
				// ... which will now re-evaluate the conversion and will use the faster conversion from now on. 
				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result)); // int -> Units::second

				// update the tree...
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter<float, Units::second>()));
				// ... which will now re-evaluate the conversion and will use the faster conversion from now on. 
				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result)); // int -> Units::second
				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result)); // int -> Units::second
				// update the tree...
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](std::string const& r) -> std::string_view { return r; })));
				EXPECT_EQ(true, SINGLE_ARG(tree.AddConverter([](std::string_view const& r) -> const char* { return r.data(); })));
				// ... which will now re-evaluate the conversion and will use the faster conversion from now on. 
				EXPECT_EQ(true, tree.TryConvert(1, boost::typeindex::type_id<Units::second>().type_info(), result)); // int -> Units::second
				EXPECT_EQ(false, tree.TryConvert(1, boost::typeindex::type_id<std::string_view>().type_info(), result)); // no conversion possible
				EXPECT_EQ(false, tree.TryConvert(1, boost::typeindex::type_id<std::string_view>().type_info(), result)); // no conversion possible, AND that result should have been cached














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



		









	}

	return 0;
};
#undef EXPECT_EQ
#undef EXPECT_NE