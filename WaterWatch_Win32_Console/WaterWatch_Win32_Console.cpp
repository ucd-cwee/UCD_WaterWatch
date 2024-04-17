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


int Example::ExampleF(int numTasks, int numSubTasks) {
	int* xyzwabc = new int[10000];
	defer(delete[] xyzwabc); // does clean-up on our behalf on scope end
	
	{
		for (int j = 1; j < 10; j += 2) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				
				for (int i = 0; i < numLoops; i++) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Threads) : ");
			{

				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				int i, k;

				std::vector<int> vec(numLoops, 0);
				fibers::parallel::For(0, numLoops, [&vec](int i) { vec[i] = i; });
				std::for_each_n(std::execution::par, vec.begin(), numLoops, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();				

				fibers::parallel::For(0, numLoops, [&pat, &j](int i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
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