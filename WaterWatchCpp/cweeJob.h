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

#pragma once
#include "Precompiled.h"
#include "Strings.h"
#include "cweeAny.h"
#include "SharedPtr.h"
#include "cweeInterlocked.h"
#include "InterlockedValues.h"
#include "cweeThreadedMap.h"
#include "UnorderedList.h"
#include "Mutex.h"

class cweeJob; // forward decl
template<typename T> struct count_arg;
template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };

enum jobType {
	IO_thread = 0,
	SIM_thread = 1,
	OPT_thread = 2,
	PRIORITY_thread = 3,
	SCRIPT_thread = 4,
	thread5,
	thread6,
	thread7,
	thread8,
	thread9,
	thread10,
	thread11,
	thread12,
	thread13,
	thread14,
	thread15,
	thread16,
	thread17,
	thread18,
	thread19,
	thread20,
	thread21,
	thread22,
	thread23,
	thread24,
	thread25,
	thread26,
	thread27,
	thread28,
	thread29,
	thread30,
	thread31,
	thread32,
	MAX_NUM_PARALLEL_THREADS = 5000				// Must be a large value
};

template <typename F = void()> class cweeFunction {
public:
	typedef F Type;
	typedef typename std::function<F>::result_type ResultType;
	typedef typename function_traits<std::function<F>>::arguments Arguments;

	cweeFunction() noexcept
		: _function()
		, _data()
		, Result()
		, IsFinished(false)
	{};

	cweeFunction(const cweeFunction& copy) noexcept
		: _function(copy._function)
		, _data(copy._data)
		, Result(copy.Result)
		, IsFinished(copy.IsFinished.load())
	{};

	cweeFunction(cweeFunction&& copy) noexcept
		: _function(std::move(copy._function))
		, _data(std::move(copy._data))
		, Result(std::move(copy.Result))
		, IsFinished(copy.IsFinished.load())
	{};

	template <typename... Args>
	cweeFunction(const std::function<F>& function, Args... Fargs) noexcept
		: _function(function)
		, _data(GetData(Fargs...))
		, Result()
		, IsFinished(false)		
	{};

private:
	static void AddData(std::vector<cweeAny>& d) { return; };
	template<typename T, typename... Targs> static void AddData(std::vector<cweeAny>& d, const T& value, Targs... Fargs) // recursive function
	{
		if constexpr (std::is_same<T, void>::value) {
			AddData(d, Fargs...);
			return;
		}
		else {
			d.push_back(value);
			AddData(d, Fargs...);
			return;
		}
	};
	template <typename... Args> std::vector<cweeAny> GetData(Args... Fargs) {
		constexpr size_t NumNeededInputs = NumInputs();
		constexpr size_t NumProvidedInputs = sizeof...(Args);
		static_assert(NumNeededInputs <= NumProvidedInputs, "Providing fewer inputs than required is unsupported. C++ Lambdas cannot support default arguments and therefore all arguments must be provided for.");

		std::vector<cweeAny> out;
		AddData(out, Fargs...);
		return out;
	};

public:
	static cweeFunction Finished() {
		cweeFunction to_return;

		to_return.IsFinished.store(true);

		return to_return;
	};
	template <typename T> static cweeFunction Finished(const T& returnMe) {
		cweeFunction to_return;

		to_return.Result = returnMe;
		to_return.IsFinished.store(true);

		return to_return;
	};

	cweeAny& Invoke(int iterationNumber = 0) {
		DoJob(iterationNumber);
		return Result;
	};
	cweeAny& ForceInvoke(int iterationNumber = 0) {
		ForceDoJob(iterationNumber);
		return Result;
	};

	static constexpr size_t NumInputs() noexcept {
		constexpr size_t numArgs = count_arg<std::function<F>>::value;
		return numArgs;
	};
	static constexpr bool ReturnsNothing() {
		return  std::is_same<typename std::function<F>::result_type, void>::value;
	};

	const char* FunctionName() const {
		return _function.target_type().name();
	};

	cweeAny& GetResult() {
		return Result;
	};
	cweeAny& GetResult() const {
		return Result;
	};

private:
	void						DoJob(int iterationNumber = 0) {
		static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a cweeFunction without further specialization.");

		while (!InvokeTryLock()) {}

		if (!IsFinished.load()) {
			FixQueuedInputs();

			if constexpr (NumInputs() == 0) {
				DoJob_Internal_0();
			}
			else if constexpr (NumInputs() == 1) {
				DoJob_Internal_1();
			}
			else if constexpr (NumInputs() == 2) {
				DoJob_Internal_2();
			}
			else if constexpr (NumInputs() == 3) {
				DoJob_Internal_3();
			}
			else if constexpr (NumInputs() == 4) {
				DoJob_Internal_4();
			}
			else if constexpr (NumInputs() == 5) {
				DoJob_Internal_5();
			}
			else if constexpr (NumInputs() == 6) {
				DoJob_Internal_6();
			}
			else if constexpr (NumInputs() == 7) {
				DoJob_Internal_7();
			}
			else if constexpr (NumInputs() == 8) {
				DoJob_Internal_8();
			}
			else if constexpr (NumInputs() == 9) {
				DoJob_Internal_9();
			}
			else if constexpr (NumInputs() == 10) {
				DoJob_Internal_10();
			}
			else if constexpr (NumInputs() == 11) {
				DoJob_Internal_11();
			}
			else if constexpr (NumInputs() == 12) {
				DoJob_Internal_12();
			}
			else if constexpr (NumInputs() == 13) {
				DoJob_Internal_13();
			}
			else if constexpr (NumInputs() == 14) {
				DoJob_Internal_14();
			}
			else if constexpr (NumInputs() == 15) {
				DoJob_Internal_15();
			}
			else if constexpr (NumInputs() == 16) {
				DoJob_Internal_16();
			}

			if (GetResult().template IsTypeOf<cweeJob>()) {
				AUTO r = GetResult();
				cweeAnyAutoCast auto_r = r.cast();
				//auto_r.operator cweeJob& ();

				cweeJob& j = GetResult().cast();
				if (iterationNumber < 20) {
					Result = j.Invoke(iterationNumber + 1);
				}
				else {
					Result.Clear();
				}
				SetAsFinished();
			}
			else {
				SetAsFinished();
			}
		}
		InvokeUnlock();
	};
	void						ForceDoJob(int iterationNumber = 0) {
		static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a cweeFunction without further specialization.");

		while (!InvokeTryLock()) {}

		if (true) {
			FixQueuedInputs();

			if constexpr (NumInputs() == 0) {
				DoJob_Internal_0();
			}
			else if constexpr (NumInputs() == 1) {
				DoJob_Internal_1();
			}
			else if constexpr (NumInputs() == 2) {
				DoJob_Internal_2();
			}
			else if constexpr (NumInputs() == 3) {
				DoJob_Internal_3();
			}
			else if constexpr (NumInputs() == 4) {
				DoJob_Internal_4();
			}
			else if constexpr (NumInputs() == 5) {
				DoJob_Internal_5();
			}
			else if constexpr (NumInputs() == 6) {
				DoJob_Internal_6();
			}
			else if constexpr (NumInputs() == 7) {
				DoJob_Internal_7();
			}
			else if constexpr (NumInputs() == 8) {
				DoJob_Internal_8();
			}
			else if constexpr (NumInputs() == 9) {
				DoJob_Internal_9();
			}
			else if constexpr (NumInputs() == 10) {
				DoJob_Internal_10();
			}
			else if constexpr (NumInputs() == 11) {
				DoJob_Internal_11();
			}
			else if constexpr (NumInputs() == 12) {
				DoJob_Internal_12();
			}
			else if constexpr (NumInputs() == 13) {
				DoJob_Internal_13();
			}
			else if constexpr (NumInputs() == 14) {
				DoJob_Internal_14();
			}
			else if constexpr (NumInputs() == 15) {
				DoJob_Internal_15();
			}
			else if constexpr (NumInputs() == 16) {
				DoJob_Internal_16();
			}

			if (GetResult().template IsTypeOf<cweeJob>()) {
				AUTO r = GetResult();
				cweeAnyAutoCast auto_r = r.cast();
				// auto_r.operator cweeJob& ();

				cweeJob& j = GetResult().cast();
				if (iterationNumber < 20) {
					Result = j.ForceInvoke(iterationNumber + 1);
				}
				else {
					Result.Clear();
				}
				SetAsFinished();
			}
			else {
				SetAsFinished();
			}
		}
		InvokeUnlock();
	};

	void						FixQueuedInputs() {
		bool failed = true;
		while (failed) {
			failed = false;
			if constexpr (NumInputs() >= 1) {
				constexpr int i = 0;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 2) {
				constexpr int i = 1;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 3) {
				constexpr int i = 2;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 4) {
				constexpr int i = 3;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 5) {
				constexpr int i = 4;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 6) {
				constexpr int i = 5;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 7) {
				constexpr int i = 6;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 8) {
				constexpr int i = 7;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 9) {
				constexpr int i = 8;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 10) {
				constexpr int i = 9;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 11) {
				constexpr int i = 10;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 12) {
				constexpr int i = 11;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 13) {
				constexpr int i = 12;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 14) {
				constexpr int i = 13;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 15) {
				constexpr int i = 14;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 16) {
				constexpr int i = 15;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}

			//for (int i = _data.size() - 1; i >= 0; i--) {
			//	if (_data[i].IsTypeOf<cweeJob>()) {
			//		_data[i] = _data[i].cast<cweeJob&>().Await(); // GetResult();
			//		failed = true;
			//	}
			//}
		}
	};
	void						DoJob_Internal_16() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
		}
	};
	void						DoJob_Internal_15() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
		}
	};
	void						DoJob_Internal_14() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
		}
	};
	void						DoJob_Internal_13() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
		}
	};
	void						DoJob_Internal_12() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
		}
	};
	void						DoJob_Internal_11() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
		}
	};
	void						DoJob_Internal_10() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
		}
	};
	void						DoJob_Internal_9() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
		}
	};
	void						DoJob_Internal_8() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
		}
	};
	void						DoJob_Internal_7() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
		}
	};
	void						DoJob_Internal_6() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
		}
	};
	void						DoJob_Internal_5() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
		}
	};
	void						DoJob_Internal_4() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
		}
	};
	void						DoJob_Internal_3() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
		}
	};
	void						DoJob_Internal_2() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast());
		}
	};
	void						DoJob_Internal_1() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast());
		}
	};
	void						DoJob_Internal_0() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function();
			cweeAny().swap(Result);
		}
		else {
			Result = _function();
		}
	};

private:
	std::function<F>			_function;
	std::vector<cweeAny>		_data;
public:
	mutable cweeAny				Result;
	mutable std::atomic<bool>	IsFinished;

private:
	void						SetAsFinished() {
		IsFinished.store(true);
	};
	// mutable cweeSysInterlockedInteger	InvokeLock;

	bool						InvokeTryLock() const {
		//if (InvokeLock.Increment() == 1) {
		return true;
		//}
		//else {
		//	InvokeLock.Decrement();
		//	return false;
		//}
	};
	void						InvokeUnlock() const {
		//InvokeLock.Decrement();
	};
};

class cweeAction_Interface {
public:
	explicit cweeAction_Interface() {};
	explicit cweeAction_Interface(cweeAction_Interface const&) = delete;
	explicit cweeAction_Interface(cweeAction_Interface&&) = delete;
	cweeAction_Interface& operator=(cweeAction_Interface const&) = delete;
	cweeAction_Interface& operator=(cweeAction_Interface&&) = delete;
	virtual ~cweeAction_Interface() noexcept {};

	virtual boost::typeindex::type_info const& type() const noexcept = 0;
	virtual const char* typeName() const noexcept = 0;
	virtual cweeSharedPtr<cweeAction_Interface> clone() const noexcept = 0;
	virtual cweeAny& Invoke(int iterationNumber = 0) noexcept = 0;
	virtual cweeAny& ForceInvoke(int iterationNumber = 0) noexcept = 0;
	virtual const char* FunctionName() const noexcept = 0;
	virtual cweeAny& Result() const noexcept = 0;
	virtual bool IsFinished() const noexcept = 0;
};
template<typename ValueType> class cweeAction_Impl final : public cweeAction_Interface {
public:
	explicit cweeAction_Impl() = delete;
	explicit cweeAction_Impl(cweeFunction<ValueType> const& f) noexcept : data(f) {};
	explicit cweeAction_Impl(cweeFunction<ValueType>&& f) noexcept : data(std::forward<cweeFunction<ValueType>>(f)) {};
	virtual ~cweeAction_Impl() noexcept {};

	virtual boost::typeindex::type_info const& type() const noexcept final {
		return boost::typeindex::type_id<cweeFunction<ValueType>>().type_info();
	};
	virtual const char* typeName() const noexcept final {
		return boost::typeindex::type_id<cweeFunction<ValueType>>().type_info().name();
	};
	virtual cweeSharedPtr<cweeAction_Interface> clone() const noexcept final {
		return make_cwee_shared<cweeAction_Impl<ValueType>>(data).template CastReference<cweeAction_Interface>();
	};
	virtual cweeAny& Invoke(int iterationNumber = 0) noexcept final {
		return data.Invoke(iterationNumber);
	};
	virtual cweeAny& ForceInvoke(int iterationNumber = 0) noexcept final {
		return data.ForceInvoke(iterationNumber);
	};
	virtual const char* FunctionName() const noexcept final {
		return data.FunctionName();
	};
	virtual cweeAny& Result() const noexcept final {
		return data.GetResult();
	};
	virtual bool IsFinished() const noexcept final {
		return data.IsFinished.load();
	};

	cweeFunction<ValueType> data;
};
class cweeAction {
public: // structors
	/*! Init */ cweeAction() noexcept : content(BasePtr()) {};
	/*! Copy */ cweeAction(const cweeAction& other) noexcept : content(BasePtr()) { cweeSharedPtr<cweeAction_Interface> c = other.content; content = c->clone(); };
	/*! Data Assignment */ template<typename ValueType> explicit cweeAction(const cweeFunction<ValueType>& value) : content(ToPtr<ValueType>(value)) {};
	~cweeAction() noexcept {};

public: // modifiers
	/*! Swap Data */ cweeAction& swap(cweeAction& rhs) noexcept {
		cweeSharedPtr<cweeAction_Interface> c1 = this->content;
		cweeSharedPtr<cweeAction_Interface> c2 = rhs.content;

		auto copy1 = c1->clone();
		auto copy2 = c2->clone();

		rhs.content = copy1;
		content = copy2;

		return *this;
	}
	/*! Copy Data */ cweeAction& operator=(const cweeAction& rhs) noexcept { cweeAction(rhs).swap(*this); return *this; };
	/* Perfect forwarding of ValueType */ template <class ValueType> cweeAction& operator=(const cweeFunction<ValueType>& rhs) noexcept { cweeAction(rhs).swap(*this); return *this; };

public: // queries
	explicit operator bool() { return !IsEmpty(); };
	explicit operator bool() const { return !IsEmpty(); };

	/*! Checks if the cweeAction has been assigned something */
	bool IsEmpty() const noexcept { AUTO c = content; return !c; };

	/*! Empties the cweeAction and frees the memory. */
	void Clear() noexcept { cweeAction().swap(*this); };

	template <typename ValueT> static constexpr const char* TypeNameOf() { return typenames::type_name<ValueT>(); };
	template <typename ValueT> static const boost::typeindex::type_info& TypeOf() { return boost::typeindex::type_id<ValueT>().type_info(); };

	const char* TypeName() const noexcept { cweeSharedPtr<cweeAction_Interface> c = content; if (c) return c->typeName(); return typenames::type_name<void>(); };
	const boost::typeindex::type_info& Type() const noexcept { cweeSharedPtr<cweeAction_Interface> c = content; return c ? c->type() : boost::typeindex::type_id<void>().type_info(); };

private:
	template <class ValueType> static cweeSharedPtr<cweeAction_Interface> ToPtr(const cweeFunction<ValueType>& rhs) noexcept { return make_cwee_shared<cweeAction_Impl<ValueType>>(rhs).template CastReference<cweeAction_Interface>(); };
	template <class ValueType> static cweeSharedPtr<cweeAction_Interface> ToPtr(cweeFunction<ValueType>&& rhs) noexcept { return make_cwee_shared<cweeAction_Impl<ValueType>>(std::forward<cweeFunction<ValueType>>(rhs)).template CastReference<cweeAction_Interface>(); };
	static cweeSharedPtr<cweeAction_Interface> BasePtr() noexcept { return ToPtr(cweeFunction<void()>()); };

public:
	cweeSharedPtr<cweeAction_Interface> content;

public:
	cweeAny* Invoke(int iterationNumber = 0) noexcept { cweeSharedPtr<cweeAction_Interface> c = content; if (c) { return &c->Invoke(iterationNumber); } return nullptr; };
	cweeAny* ForceInvoke(int iterationNumber = 0) noexcept { cweeSharedPtr<cweeAction_Interface> c = content; if (c) { return &c->ForceInvoke(iterationNumber); } return nullptr; };
	const char* FunctionName() const {
		cweeSharedPtr<cweeAction_Interface> c = content;
		if (c)
		{
			return c->FunctionName();
		}
		return "No Function";
	};
	bool     IsFinished() const {
		cweeSharedPtr<cweeAction_Interface> c = content;
		if (c)
		{
			return c->IsFinished();
		}
		return false;
	};
	cweeAny* Result() const { cweeSharedPtr<cweeAction_Interface> c = content; if (c) { return &c->Result(); } return nullptr; };
	static cweeAction Finished() { return cweeAction(cweeFunction<void()>::Finished()); };
	template <typename T> static cweeAction Finished(const T& returnMe) { return cweeAction(cweeFunction<T()>::Finished(returnMe)); };

};

class cweeJob {
public:
	class cweeJob_Impl {
	public:
		cweeJob_Impl() : todo(new cweeAction()), _ContinueWith(make_cwee_shared<cweeThreadedList < cweeJob >>()) {};
		cweeJob_Impl(const cweeJob_Impl& other) : todo(other.todo), _ContinueWith(other._ContinueWith) {};
		cweeJob_Impl(cweeJob_Impl&& other) : todo(other.todo), _ContinueWith(other._ContinueWith) {};

		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<cweeJob_Impl, std::decay_t<T>> && !std::is_same_v<cweeAny, std::decay_t<T>> >>
		explicit cweeJob_Impl(T function, Args... Fargs) : todo(new cweeAction(cweeFunction(std::function(function), Fargs...))), _ContinueWith(make_cwee_shared<cweeThreadedList < cweeJob >>()) {};

		cweeJob_Impl& operator=(const cweeJob_Impl& other) {
			todo = other.todo;
			_ContinueWith = other._ContinueWith;

			return *this;
		};
		cweeJob_Impl& operator=(cweeJob_Impl&& other) {
			todo = cweeSharedPtr < cweeAction >().Swap(other.todo);
			_ContinueWith = decltype(_ContinueWith)().Swap(other._ContinueWith);

			return *this;
		};

		cweeAny Invoke(int iterationNumber = 0) {
			cweeAny out; bool previouslyFinished(true);
			AUTO job = todo.Get();
			if (job) {
				previouslyFinished = job->IsFinished();
				AUTO reply = job->Invoke(iterationNumber);
				if (reply) {
					out = *reply;
				}
			}

			if (!previouslyFinished) {
				cweeThreadedList < cweeJob > L;
				_ContinueWith.Lock();
				AUTO toLoop = _ContinueWith.UnsafeGet();
				if (toLoop) L = *toLoop;
				_ContinueWith.Unlock();

				if (L.Num() > 0) {
					for (auto& j : L) {
						j.Invoke();
					}
				}
			}

			return out;
		};
		cweeAny ForceInvoke(int iterationNumber = 0) {
			cweeAny out;
			AUTO job = todo.Get();
			if (job) {
				AUTO reply = job->ForceInvoke(iterationNumber);
				if (reply) {
					out = *reply;
				}
			}
			if (true) {
				cweeThreadedList < cweeJob > L;
				_ContinueWith.Lock();
				AUTO toLoop = _ContinueWith.UnsafeGet();
				if (toLoop) L = *toLoop;
				_ContinueWith.Unlock();

				if (L.Num() > 0) {
					for (auto& j : L) {
						j.ForceInvoke();
					}
				}
			}
			return out;
		};
		cweeAny GetResult() {
			return Invoke();
		};
		cweeAny operator()() {
			return Invoke();
		};

		bool IsFinished() const {
			AUTO job = todo.Get();
			if (job) {
				return job->IsFinished();
			}
			return true;
		};

		cweeJob ContinueWith(const cweeJob& next) {
			if (IsFinished()) {
				const_cast<cweeJob&>(next).Invoke();
			}
			_ContinueWith.Lock();
			AUTO cont = _ContinueWith.UnsafeGet();
			if (cont) {
				cont->Append(next);
			}
			_ContinueWith.Unlock();
			return next;
		};
		cweeJob ContinueWith(cweeJob&& next) {
			if (IsFinished()) {
				next.Invoke();
			}
			_ContinueWith.Lock();
			AUTO cont = _ContinueWith.UnsafeGet();
			if (cont) {
				cont->Append(std::forward<cweeJob>(next));
			}
			_ContinueWith.Unlock();
			return next;
		};

		template < typename T, typename... Args, typename = std::enable_if_t<!std::is_same_v<cweeJob_Impl, std::decay_t<T>>>>
		cweeJob ContinueWith(T function, Args... Fargs) {
			return ContinueWith(cweeJob(cweeJob_Impl(function, Fargs...)));
		};

		const char* FunctionName() const {
			AUTO job = todo.Get();
			if (job) {
				return job->FunctionName();
			}
			return cweeStr("No Function").c_str();
		};

	public:
		cweeSharedPtr < cweeAction > todo;
		cweeSharedPtr < cweeThreadedList < cweeJob > > _ContinueWith;
	};

	static cweeJob Finished() {
		AUTO toReturn = cweeJob();

		AUTO ptrp = toReturn.GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			ptr->todo = cweeSharedPtr<cweeAction>(new cweeAction(cweeAction::Finished()));
		}

		return toReturn;
	};
	template <typename T> static cweeJob Finished(const T& returnMe) {
		AUTO toReturn = cweeJob();

		AUTO ptrp = toReturn.GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			ptr->todo = std::make_shared<cweeAction>(cweeAction::Finished(returnMe));
		}

		return toReturn;
	};

public:
	cweeJob() : impl(new cweeJob_Impl()) {};
	cweeJob(const cweeJob& other) : impl(other.GetImpl()) {};
	cweeJob(cweeJob&& other) : impl(other.GetImpl()) {};
	cweeJob& operator=(const cweeJob& other) {
		impl = other.GetImpl();
		return *this;
	};
	cweeJob& operator=(cweeJob&& other) {
		impl = other.GetImpl();
		return *this;
	};

	template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<cweeJob, std::decay_t<T>> && !std::is_same_v<cweeAny, std::decay_t<T>> >>
	explicit cweeJob(T function, Args... Fargs) : impl(new cweeJob_Impl(function, Fargs...)) {};

	cweeJob& AsyncInvoke();

	cweeJob& AsyncForceInvoke();

	uintptr_t DelayedAsyncInvoke(u64 milliseconds_delay);
	uintptr_t DelayedAsyncForceInvoke(u64 milliseconds_delay);

	cweeAny Invoke(int iterationNumber = 0) {
		AUTO ptr = GetImpl();
		AUTO ptrI = ptr.Get();
		if (ptrI) {
			return ptrI->Invoke(iterationNumber);
		}
		return cweeAny();
	};
	cweeAny ForceInvoke(int iterationNumber = 0) {
		AUTO ptr = GetImpl();
		AUTO ptrI = ptr.Get();
		if (ptrI) {
			return ptrI->ForceInvoke(iterationNumber);
		}
		return cweeAny();
	};
	cweeAny GetResult() const {
		AUTO ptr = GetImpl();
		AUTO ptrR = ptr.Get();
		if (ptrR) {
			return ptrR->GetResult();
		}
		return cweeAny();
	};
	cweeAny operator()() {
		return Invoke();
	};

	bool IsFinished() const {
		AUTO ptrp = GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			return ptr->IsFinished();
		}
		return false;
	};

	cweeAny Await();
	cweeAny AwaitAll() {
		Await();

		AUTO ptrAp = GetImpl();
		AUTO ptrA = ptrAp.Get();
		if (ptrA) {
			cweeThreadedList < cweeJob > L;
			ptrA->_ContinueWith.Lock();
			AUTO toLoop = ptrA->_ContinueWith.UnsafeGet();
			if (toLoop) L = *toLoop;
			ptrA->_ContinueWith.Unlock();

			if (L.Num() > 0) {
				for (auto& j : L) {
					j.AwaitAll();
				}
			}
		}

		return std::move(GetResult());
	};

	cweeJob ContinueWith(const cweeJob& next) {
		AUTO ptrp = GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			return ptr->ContinueWith(next);
		}
		return next;
	};
	cweeJob ContinueWith(cweeJob&& next) {
		AUTO ptrp = GetImpl();
		AUTO ptr = ptrp.Get();
		if (ptr) {
			return ptr->ContinueWith(std::forward<cweeJob>(next));
		}
		return next;
	};
	template < typename T, typename... Args, typename = std::enable_if_t<!std::is_same_v<cweeJob, std::decay_t<T>>>>
	cweeJob ContinueWith(T function, Args... Fargs) {
		return ContinueWith(cweeJob(function, Fargs...));
	};
	const char* FunctionName() const {
		AUTO ptr = GetImpl();
		AUTO ptrF = ptr.Get();
		if (ptrF) {
			return ptrF->FunctionName();
		}
		return "Unknown Function";
	};

protected:
	mutable cweeSharedPtr< cweeJob_Impl > impl;

private:
	cweeSharedPtr< cweeJob_Impl > GetImpl() const {
		return impl;
	};
};

#pragma region Parallel Job Processor
// #define jobListTypeAsList

class cweeJobThreads {
public:
#ifdef jobListTypeAsList
	typedef cweeUnorderedList< cweeJob > jobListType;
#else
	typedef cweeThreadedMap<u64, cweeJob> jobListType;
#endif

	class cweeJobThreadsData {
	public:
		class cweeCpuThread {
		public:
			class cweeCpuThreadData {
			private:
				cweeSysInterlockedInteger														m_Waiting;
				cweeSysInterlockedInteger														m_Terminate;
				cweeSysSignal																	m_Signal;
				cweeSysInterlockedInteger														m_Running;
				cweeSysInterlockedInteger														m_Working;
				cweeUnpooledInterlocked<cweeUnion<cweeSysInterlockedPointer<void>>>				m_Content;
				cweeSharedPtr<jobListType>														m_SharedJobs;
				cweeSharedPtr<cweeSysInterlockedInteger>										m_NumActiveThreads;

			public:
				static void ThreadDoWork(cweeCpuThreadData* threadData, cweeJob& todo) {
					// primary responsibility... 
					while (threadData->TryExtractNextJob(threadData->m_SharedJobs, todo)) { // for any and all jobs in the queue that we are sharing...						
						todo.Invoke(); // do the work. 		
						todo = cweeJob(); // forget about the work.
					}
				};
				static void ThreadSleep(cweeCpuThreadData* threadData) {
					if (1) {
						threadData->m_Waiting.Increment();
						threadData->m_Working.Decrement();
					}
					if (1){
						threadData->m_Signal.Wait(cweeSysSignal::WAIT_INFINITE);
					}
					if (1) {
						threadData->m_Working.Increment();
						threadData->m_Waiting.Decrement();
					}
				};
				static int ThreadProc(cweeCpuThreadData* threadData) {
					cweeJob todo;
					while (1) {
						if (1) {
							ThreadSleep(threadData);
						}
						if (threadData->m_Terminate.Decrement() == 0) break;
						else threadData->m_Terminate.Increment();
						if (1) {
							ThreadDoWork(threadData, todo);
						}
						threadData->m_Signal.Clear();
					}
					threadData->m_Running.Decrement();
					threadData->m_NumActiveThreads->Decrement();
					return 0;
				};
				static bool	TryExtractNextJob(cweeSharedPtr<jobListType> jobs, cweeJob& nextJob) {					
					bool out;
#ifdef jobListTypeAsList
					jobs->Lock();
					out = jobs->UnsafeExtractAny(nextJob);
					jobs->Unlock();
#else
					out = jobs->ExtractAny(nextJob);
#endif
					return out;
				};

			private:
				void	TryStartThread() { m_Signal.Raise(); };
				void	ClearThreadHandle() {
					m_Content.Lock();
					uintptr_t out = (uintptr_t)m_Content.UnsafeRead()->get<0>().Set(nullptr);
					m_Content.Unlock();
					if (out != 0) {
						m_Terminate.Increment(); // let the thread know it needs to die				
						TryStartThread(); // raise the signal, in case the thread is asleep						
						cweeSysThreadTools::Sys_DestroyThread(out); // spin-wait for it to die. 
					}
				};

			public:
				explicit cweeCpuThreadData(
					cweeSharedPtr<jobListType> const& p_sharedJobs,
					cweeSharedPtr<cweeSysInterlockedInteger> numActive) :
					m_Waiting(0),
					m_Terminate(0),
					m_Signal(true),
					m_Running(1),
					m_Working(1),
					m_Content(cweeUnion<cweeSysInterlockedPointer<void>>(nullptr)),
					m_SharedJobs(p_sharedJobs),
					m_NumActiveThreads(numActive)
				{
					m_Content = cweeUnion<cweeSysInterlockedPointer<void>>(
						(void*)cweeSysThreadTools::Sys_CreateThread((xthread_t)ThreadProc, this, 512 * 1024)
					);
				};
				void WakeUp() { TryStartThread(); };
				bool IsRunning() { 
					if (m_Signal.Wait(0)){
						// m_Signal.Raise();
						return true;
					}
					else {
						return !(m_Working.GetValue() == 0);
					}
				};
				~cweeCpuThreadData() { ClearThreadHandle(); };

			};

		private:
			cweeSharedPtr<cweeCpuThreadData>	m_Data;
			cweeCpuThread& swap(cweeCpuThread& a) {
				if (&a == this) { return *this; }
				m_Data.swap(a.m_Data);
				return *this;
			};

		public:
			cweeCpuThread() : m_Data(nullptr) {};
			explicit cweeCpuThread(cweeSharedPtr<jobListType> const& p_sharedJobs, cweeSharedPtr<cweeSysInterlockedInteger> numActive) : 
				m_Data(make_cwee_shared<cweeCpuThreadData>(p_sharedJobs, numActive)) {};
			cweeCpuThread(cweeCpuThread const& a) : m_Data(a.m_Data) {};
			cweeCpuThread(cweeCpuThread&& a) : m_Data(a.m_Data) { a.m_Data = nullptr; };
			cweeCpuThread& operator=(cweeCpuThread const& a) { m_Data = a.m_Data; return *this; };
			cweeCpuThread& operator=(cweeCpuThread&& a) { m_Data = a.m_Data; return *this; };
			~cweeCpuThread() {};

		public:
			void WakeUp() {
				cweeSharedPtr<cweeCpuThreadData>	data = m_Data;
				cweeCpuThreadData* p = data.Get();
				if (p) {
					p->WakeUp();
				}
			};
			bool IsRunning() {
				cweeSharedPtr<cweeCpuThreadData>	data = m_Data;
				cweeCpuThreadData* p = data.Get();
				if (p) {
					return p->IsRunning();
				}
				return false;
			};

		};

	public:
		cweeJobThreadsData(int const& numLogicalCores = 4) : // CPU_DATA const& d = CPU_DATA()
			// m_numPhysicalCpuCores(d.m_numPhysicalCpuCores),
			m_numLogicalCpuCores(numLogicalCores), // m_numLogicalCpuCores(d.m_numLogicalCpuCores),
			// m_numCpuPackages(d.m_numCpuPackages),
			m_Jobs(make_cwee_shared<jobListType>()),
			m_NumActiveThreads(make_cwee_shared<cweeSysInterlockedInteger>(0)),
			m_Threads(),
			m_numJobs(0)
		{
			m_Threads.Append(cweeCpuThread(m_Jobs, m_NumActiveThreads)); // minimum of one thread
			for (int i = 1; i < m_numLogicalCpuCores - 1; i++) { // one thread less to allow for the "UI" thread
				m_Threads.Append(cweeCpuThread(m_Jobs, m_NumActiveThreads));
			}
		};
		cweeJobThreadsData(cweeJobThreadsData const&) = delete;
		cweeJobThreadsData(cweeJobThreadsData&&) = delete;
		cweeJobThreadsData& operator=(cweeJobThreadsData const&) = delete;
		cweeJobThreadsData& operator=(cweeJobThreadsData&&) = delete;
		~cweeJobThreadsData() { Wait(); };

	public:
		void WakeUpAll() {
			int n = m_Jobs->Num();
			for (auto& thread : m_Threads) {
				if (!thread.IsRunning()) {
					if (--n >= 0) {
						thread.WakeUp();
					}
					else {
						break;
					}
				}
			}
			if (n > 0) {
				for (auto& thread : m_Threads) {
					if (--n >= 0) {
						thread.WakeUp();
					}
					else {
						break;
					}
				}
			}
		};
		void AddTask(cweeJob const& j) {
#ifdef jobListTypeAsList
			m_Jobs->Append(j);
#else
			m_Jobs->Emplace(m_numJobs.Increment(), j); 
#endif
			WakeUpAll();
		};
		void Wait() {
			cweeJob out;
			WakeUpAll();
			while (1) {
				bool stop = true;
				while (cweeJobThreadsData::cweeCpuThread::cweeCpuThreadData::TryExtractNextJob(m_Jobs, out)) { out.Invoke(); WakeUpAll(); }
				for (auto& thread : m_Threads) {
					if (thread.IsRunning()) {
						stop = false;
						break;
					};
				}
				if (stop) {
					break;
				}
			}
		};
		void Wait_Once() {
			cweeJob out;
			WakeUpAll();
			if (1) {
				if (cweeJobThreadsData::cweeCpuThread::cweeCpuThreadData::TryExtractNextJob(m_Jobs, out)) { out.Invoke(); WakeUpAll(); }
			}
		};

	public:
		const int								m_numLogicalCpuCores;
		cweeSharedPtr<jobListType>				m_Jobs;
		cweeSharedPtr<cweeSysInterlockedInteger> m_NumActiveThreads;
		cweeThreadedList<cweeCpuThread>			m_Threads;
		cweeSysInterlockedInteger				m_numJobs;

	};

private:
	cweeSharedPtr<cweeJobThreadsData> m_data;

public:
	cweeJobThreads(int numLogicalCores = 4) : m_data(new cweeJobThreadsData(numLogicalCores)) {};
	cweeJobThreads(cweeJobThreads const& a) : m_data(a.m_data) {};
	cweeJobThreads(cweeJobThreads&& a) : m_data(a.m_data) {};
	cweeJobThreads& operator=(cweeJobThreads const& a) { m_data = a.m_data; return *this; };
	cweeJobThreads& operator=(cweeJobThreads&& a) { m_data = a.m_data; return *this; };
	~cweeJobThreads() { /* Does not await because that responsibility is owned by the destruction of the m_data */ };

public:
	void Queue(cweeJob j) const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			p->AddTask(j);
		}
		else {
			j.Invoke();
		}
	};
	void Await() const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			p->Wait();
		}
	};
	void Await_Once() const {
		cweeSharedPtr<cweeJobThreadsData> d = m_data;
		cweeJobThreadsData* p = d.Get();
		if (p) {
			p->Wait_Once();
		}
	};
};
extern cweeSharedPtr<cweeJobThreads> cweeSharedJobThreads;
#pragma endregion

INLINE cweeJob& cweeJob::AsyncInvoke() { if (!IsFinished()) { cweeSharedJobThreads->Queue(*this); } return *this; };
INLINE cweeJob& cweeJob::AsyncForceInvoke() { cweeSharedJobThreads->Queue(cweeJob([](cweeJob& todo) { todo.ForceInvoke(); }, *this)); return *this; };
INLINE cweeAny cweeJob::Await() { while (!this->IsFinished()) { cweeSharedJobThreads->Await_Once(); } return GetResult(); };
INLINE uintptr_t cweeJob::DelayedAsyncInvoke(u64 milliseconds_delay) {
	cweeUnion< u64, cweeJob >* data = new cweeUnion<u64, cweeJob>(milliseconds_delay, *this);
	return cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
		if (_anon_ptr != nullptr) {
			cweeUnion< u64, cweeJob >* T = static_cast<cweeUnion< u64, cweeJob>*>(_anon_ptr);
			::Sleep(T->get<0>());
			T->get<1>().AsyncInvoke();
			delete T;
		}
		return 0;
		}), (void*)data, 1024);
};
INLINE uintptr_t cweeJob::DelayedAsyncForceInvoke(u64 milliseconds_delay) {
	cweeUnion< u64, cweeJob >* data = new cweeUnion<u64, cweeJob>(milliseconds_delay, *this);
	return cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
		if (_anon_ptr != nullptr) {
			cweeUnion< u64, cweeJob >* T = static_cast<cweeUnion< u64, cweeJob>*>(_anon_ptr);
			::Sleep(T->get<0>());
			T->get<1>().AsyncForceInvoke();
			delete T;
		}
		return 0;
		}), (void*)data, 1024);
};