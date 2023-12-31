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
#include "../WaterWatchCpp/Precompiled.h"
#include "../WindowsPlatformTools/WindowsPlatformTools.h"
#include "../WaterWatchCpp/Clock.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/AppLayerRequests.h"
#include "../WaterWatchCpp/Toasts.h"
#include "../WaterWatchCpp/FileSystemH.h"
#include "../WaterWatchCpp/Geocoding.h"
#include "../WaterWatchCpp/MachineLearning.h"
#include "../WaterWatchCpp/cweeJob.h"
#include "../WaterWatchCpp/Chaiscript_WaterWatch_Module.h"
#include "../WaterWatchCpp/ExternData.h"
#include "../WaterWatchCpp/AlgLibWrapper.h"
#include "cwee.h"
#include <Python.h>

#pragma region cweeDateTime
cweeDateTime::cweeDateTime() : unixTime(0), year(0), month(0), day(0), hour(0), minute(0), second(0), milliseconds(0) {};
cweeDateTime::cweeDateTime(double x) : unixTime(x), year(0), month(0), day(0), hour(0), minute(0), second(0), milliseconds(0) {
	cweeTime t(x);
	year = t.tm_year() + 1900;
	month = t.tm_mon() + 1;
	day = t.tm_mday();
	hour = t.tm_hour();
	minute = t.tm_min();
	second = t.tm_sec();
	milliseconds = t.tm_fractionalsec();
};
#pragma endregion

#pragma region AWAITER
bool Awaiter::IsFinished() { return *(this->isFinished_m); };
std::string Awaiter::Result() { if (this->data_m) return *this->data_m; return ""; };
#pragma endregion

#pragma region Shared Data

SharedMatrix::SharedMatrix() : index_p(external_data->CreateMatrix()), ptr(nullptr) {
	ptr = cweeSharedPtr<void>(cweeSharedPtr<bool>(new bool(true), [=](bool* d) {
		delete d;
		external_data->DeleteMatrix(index_p);
		}), [](void* p) { return p; });
};
SharedMatrix::SharedMatrix(int index, bool deleteDataWhenScopeEnds) : index_p(index), ptr(nullptr) {
	// check that this index is valid
	if (!external_data->CheckMatrix(index_p)) {
		throw(std::runtime_error(""));
	}

	if (deleteDataWhenScopeEnds) {
		ptr = cweeSharedPtr<void>(cweeSharedPtr<bool>(new bool(true), [=](bool* d) {
			delete d;
			external_data->DeleteMatrix(index_p);
			}), [](void* p) { return p; });
	}
};
SharedMatrix::SharedMatrix(SharedMatrix const& other) : index_p(other.index_p), ptr(other.ptr) {};
SharedMatrix::~SharedMatrix() {
	if (ptr) {
		ptr = nullptr;
	}
};
void    SharedMatrix::Clear() { external_data->ClearMatrix(index_p); };
void    SharedMatrix::AppendData(double X, double Y, float value) { external_data->AppendData(index_p, X, Y, value); };
double  SharedMatrix::GetValue(double X, double Y) { return (double)external_data->GetValue(index_p, X, Y); };
std::vector<double> SharedMatrix::GetKnotSeries(double Left, double Top, double Right, double Bottom, int numColumns, int numRows) {
	std::vector<double> out;
	AUTO v = external_data->GetMatrix(index_p, Left, Top, Right, Bottom, numColumns, numRows);
	out.reserve(v.size() + 1);
	for (auto& x : v) out.push_back(x);
	return out;
};
std::vector<double> SharedMatrix::GetTimeSeries(double Left, double Top, double Right, double Bottom, int numColumns, int numRows) {
	constexpr int
		reductionRatio = 4;
	std::vector<double>
		out;
	out.reserve(numColumns * numRows + 12);

	//Stopwatch sw; 
	//sw.Start();

	constexpr bool Multithreaded = true;
	if constexpr (Multithreaded) {
		out.resize(numColumns * numRows);

		u64
			columnStep = (Right - Left) / numColumns,
			rowStep = (Top - Bottom) / numRows;

		cweeInterpolatedMatrix<float> tempMatrix;

		auto* knots = external_data->GetMatrixRef(index_p);

		// Get or create the underlying interpolation model
		cweeSharedPtr<alglib::rbfmodel> model;
		double avgDistanceBetweenKnots = 1.0;
		if (!knots->Tag) {
			avgDistanceBetweenKnots = knots->EstimateDistanceBetweenKnots();
			knots->Lock();
		}
		if (!knots->Tag) {
			model = make_cwee_shared<alglib::rbfmodel>();
			{
				alglib::real_2d_array arr;
				cweeThreadedList<cweeUnion<double, double, double>> data;
				auto& knotSeries = knots->UnsafeGetSource();
				knotSeries.Lock();
				for (auto& knot : knotSeries.UnsafeGetValues()) {
					if (knot.object) {
						data.Append(cweeUnion<double, double, double>(knot.object->x, knot.object->y, knot.object->z));
					}
				}
				knotSeries.Unlock();

				{
					arr.attach_to_ptr(data.Num(), 3, (double*)(void*)data.Ptr());
					{
						alglib::rbfcreate(2, 1, *model);
						rbfsetpoints(*model, arr);
						alglib::rbfreport rep;
						alglib::rbfsetalgohierarchical(*model, avgDistanceBetweenKnots * 10.0, 4, 0.0); // (*model, avgDistanceBetweenKnots * 10.0, 4, 0.0);
						alglib::rbfbuildmodel(*model, rep, alglib::parallel);
					}
				}
			}
			knots->Tag = cweeSharedPtr<void>(model, [](void* p) { return p; });
			knots->Unlock();
		}
		else {
			model = cweeSharedPtr<alglib::rbfmodel>(knots->Tag, [](void* p) -> alglib::rbfmodel* { return static_cast<alglib::rbfmodel*>(p); });
		}

		cweeList<cweeJob> jobs(numRows + 1);
		cweeSysInterlockedInteger count = 0; // numColumns
		tempMatrix.Reserve(numColumns * numRows + 12);
		int AsyncRows = ::Max<double>(0, numRows * 10.0 / 12.0); // 2/3 resulted in no waiting

		for (int r = 0; r < AsyncRows; r++) {
			jobs.Append(cweeJob([&](alglib::rbfmodel& Model, int R, alglib::real_1d_array& results) {
				static thread_local alglib::rbfcalcbuffer buf;

				// attach pointers for arrays for the interpolation
				alglib::rbfcreatecalcbuffer(Model, buf, alglib::parallel);

				alglib::real_1d_array arr;
				cweeUnion<double, double> coords;
				arr.attach_to_ptr(2, (double*)(void*)(&coords));

				cweeList< cweeUnion<double, double, double> > out(numColumns + 1);

				int C;
				for (
					coords.get<0>() = Left,
					coords.get<1>() = Top - R * rowStep,
					C = 0;
					C < numColumns;
					C++,
					coords.get<0>() += columnStep)
				{
					if ((::Max(R, C) - ::Min(R, C)) % reductionRatio == 0) {
						// get "real" interpolated results for 1/3 of the requested pixels using a slow, complex model, distributed diagonally along the grid								
						alglib::rbftscalcbuf(Model, buf, arr, results, alglib::parallel);

						// add it to the matrix
						// tempMatrix.AddValue(C, R, results[0], false); // the lock in the matrix is our issue. 
						out.Append(cweeUnion<double, double, double>((double)C, (double)R, (double)results[0]));
					}
					else {
						out.Append(cweeUnion<double, double, double>(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()));
					}
				}
				cweeSharedPtr<int> incrementer = cweeSharedPtr<int>(&C, [&](int* p) { count.Increment(); });
				return out;
				}, model, (int)r, alglib::real_1d_array()).AsyncInvoke());
		}

		{
			alglib::real_1d_array results;
			for (int r = AsyncRows; r < numRows; r++) {
				static thread_local alglib::rbfcalcbuffer buf;

				// attach pointers for arrays for the interpolation
				alglib::rbfcreatecalcbuffer(*model, buf, alglib::parallel);

				alglib::real_1d_array arr;
				cweeUnion<double, double> coords;
				arr.attach_to_ptr(2, (double*)(void*)(&coords));

				int C;
				for (
					coords.get<0>() = Left,
					coords.get<1>() = Top - r * rowStep,
					C = 0;
					C < numColumns;
					C++,
					coords.get<0>() += columnStep)
				{
					if ((::Max(r, C) - ::Min(r, C)) % reductionRatio == 0) {
						// get "real" interpolated results for 1/3 of the requested pixels using a slow, complex model, distributed diagonally along the grid								
						alglib::rbftscalcbuf(*model, buf, arr, results, alglib::parallel);

						// add it to the matrix
						tempMatrix.AddValue(C, r, results[0]);
						out[numColumns * r + C] = results[0];
					}
				}
				count.Increment();
			}
		}

		while (count.GetValue() != numRows) {}
		for (auto& job : jobs) {
			cweeAny any = job.Await();
			cweeList< cweeUnion<double, double, double> >* coords = any.cast();
			if (coords) {
				for (auto& coord : *coords) {
					if (coord.get<0>() != std::numeric_limits<double>::max()
						&& coord.get<1>() != std::numeric_limits<double>::max()
						&& coord.get<2>() != std::numeric_limits<double>::max()) {
						tempMatrix.AddValue(coord.get<0>(), coord.get<1>(), coord.get<2>(), false);
						out[numColumns * coord.get<1>() + coord.get<0>()] = coord.get<2>();
					}
				}
			}
		}

		// do a faster, local interpolation of those results using the Hilbert curve for the last 2/3 components. 
		if (true) {
			double R, C;
			for (R = 0; R < numRows; R++) {
				for (C = 0; C < numColumns; C++) {
					// out.push_back(tempMatrix.GetCurrentValue(C, R)); // tempMatrix.GetCurrentValue(C, R));

					if (((int)(::Max(R, C) - ::Min(R, C))) % reductionRatio != 0) {
						out[numColumns * R + C] = tempMatrix.GetCurrentValue(C, R);
					}
				}
			}
		}
	}
	else {
		out.resize(numColumns * numRows);

		cweeUnion<double, double>
			coords(Left, Top);
		u64
			columnStep = (Right - Left) / numColumns,
			rowStep = (Top - Bottom) / numRows;
		int
			R,
			C;
		alglib::real_1d_array
			arr;
		alglib::real_1d_array
			results;// , derivativeResults;
		static thread_local alglib::rbfcalcbuffer
			buf;
		cweeInterpolatedMatrix<float>
			tempMatrix;

		auto* knots = external_data->GetMatrixRef(index_p);

		// Get or create the underlying interpolation model
		cweeSharedPtr<alglib::rbfmodel> model;
		double avgDistanceBetweenKnots = 1.0;
		if (!knots->Tag) {
			avgDistanceBetweenKnots = knots->EstimateDistanceBetweenKnots();
			knots->Lock();
		}
		if (!knots->Tag) {
			model = make_cwee_shared<alglib::rbfmodel>();
			{
				alglib::real_2d_array arr;
				cweeThreadedList<cweeUnion<double, double, double>> data;
				auto& knotSeries = knots->UnsafeGetSource();
				knotSeries.Lock();
				for (auto& knot : knotSeries.UnsafeGetValues()) {
					if (knot.object) {
						data.Append(cweeUnion<double, double, double>(knot.object->x, knot.object->y, knot.object->z));
					}
				}
				knotSeries.Unlock();

				{
					arr.attach_to_ptr(data.Num(), 3, (double*)(void*)data.Ptr());
					{
						alglib::rbfcreate(2, 1, *model);
						rbfsetpoints(*model, arr);
						alglib::rbfreport rep;
						alglib::rbfsetalgohierarchical(*model, avgDistanceBetweenKnots * 10.0, 3, 0.0); // 4, 0.0);
						alglib::rbfbuildmodel(*model, rep, alglib::parallel);
					}
				}
			}
			knots->Tag = cweeSharedPtr<void>(model, [](void* p) { return p; });
			knots->Unlock();
		}
		else {
			model = cweeSharedPtr<alglib::rbfmodel>(knots->Tag, [](void* p) -> alglib::rbfmodel* { return static_cast<alglib::rbfmodel*>(p); });
		}

		// attach pointers for arrays for the interpolation
		arr.attach_to_ptr(2, (double*)(void*)(&coords));
		alglib::rbfcreatecalcbuffer(*model, buf);
		tempMatrix.Reserve(numColumns * numRows + 12);

		// get "real" interpolated results for 1/3 of the requested pixels using a slow, complex model, distributed diagonally along the grid
		for (coords.get<1>() = Top, R = 0; R < numRows; R++, coords.get<1>() -= rowStep) {
			for (coords.get<0>() = Left, C = 0; C < numColumns; C++, coords.get<0>() += columnStep) {
				if ((::Max(R, C) - ::Min(R, C)) % reductionRatio == 0) {
					alglib::rbftscalcbuf(*model, buf, arr, results, alglib::parallel);
					tempMatrix.AddValue(C, R, results[0]);
					out[numColumns * R + C] = results[0];
				}
			}
		}

		// do a faster, local interpolation of those results using the Hilbert curve for the last components. 
		for (R = 0; R < numRows; R++) {
			for (C = 0; C < numColumns; C++) {
				if ((::Max(R, C) - ::Min(R, C)) % reductionRatio != 0) {
					out[numColumns * R + C] = tempMatrix.GetCurrentValue(C, R);
				}
			}
		}

	}

	//sw.Stop();
	//cweeToasts->submitToast("Seconds Required", cweeUnitValues::second(sw.Seconds_Passed()).ToString());

	return out;
};
double  SharedMatrix::GetMinX() { return external_data->GetMatrixRef(index_p)->GetMinX(); };
double  SharedMatrix::GetMaxX() { return external_data->GetMatrixRef(index_p)->GetMaxX(); };
double  SharedMatrix::GetMinY() { return external_data->GetMatrixRef(index_p)->GetMinY(); };
double  SharedMatrix::GetMaxY() { return external_data->GetMatrixRef(index_p)->GetMaxY(); };
double  SharedMatrix::GetMinValue() { return external_data->GetMatrixRef(index_p)->GetMinValue(); };
double  SharedMatrix::GetMaxValue() { return external_data->GetMatrixRef(index_p)->GetMaxValue(); };
int     SharedMatrix::GetNumValues() { return external_data->GetMatrixRef(index_p)->Num(); };
int     SharedMatrix::Index() { return index_p; };


SharedTimeSeriesPattern::SharedTimeSeriesPattern() : index_p(external_data->CreatePattern()), deleteDataWhenScopeEnds_p(true) {};
SharedTimeSeriesPattern::SharedTimeSeriesPattern(int index, bool deleteDataWhenScopeEnds) : index_p(index), deleteDataWhenScopeEnds_p(deleteDataWhenScopeEnds) {};
SharedTimeSeriesPattern::~SharedTimeSeriesPattern() {
	if (deleteDataWhenScopeEnds_p) {
		external_data->DeletePattern(index_p);
	}
};
void    SharedTimeSeriesPattern::Clear() { external_data->ClearPattern(index_p); };
void    SharedTimeSeriesPattern::AppendData(double time, float value) { external_data->AppendData(index_p, time, value); };
float   SharedTimeSeriesPattern::GetValue(double time) { return (double)external_data->GetValue(index_p, time); };
float   SharedTimeSeriesPattern::GetAvgValue(double time1, double time2) { return (double)(external_data->GetPatternRef(index_p)->GetAvgValue(time1, time2)); };
int     SharedTimeSeriesPattern::Index() { return index_p; };
std::vector<Pair<double, double>> SharedTimeSeriesPattern::GetTimeSeries() {
	std::vector<Pair<double, double>> out;
	for (auto& x : external_data->GetPattern(index_p)) {
		out.push_back(Pair<double, double>((double)x.first, (double)x.second));
	}
	return out;
};
double  SharedTimeSeriesPattern::GetMinTime() { return external_data->GetPatternRef(index_p)->GetMinTime()(); };
double  SharedTimeSeriesPattern::GetMaxTime() { return external_data->GetPatternRef(index_p)->GetMaxTime()(); };
int     SharedTimeSeriesPattern::GetNumValues() { return external_data->GetPatternRef(index_p)->GetNumValues(); };
uwp_patternInterpType     SharedTimeSeriesPattern::GetInterpolationType() {
	switch (external_data->GetPatternRef(index_p)->GetInterpolationType()) {
	default:
	case interpolation_t::LEFT:
		return uwp_patternInterpType::LEFT;
	case interpolation_t::RIGHT:
		return uwp_patternInterpType::RIGHT;
	case interpolation_t::LINEAR:
		return uwp_patternInterpType::LINEAR;
	case interpolation_t::SPLINE:
		return uwp_patternInterpType::SPLINE;
	}
};
std::string SharedTimeSeriesPattern::X_Units() {
	return external_data->GetPatternRef(index_p)->X_Type().Abbreviation();
};
std::string SharedTimeSeriesPattern::Y_Units() {
	return external_data->GetPatternRef(index_p)->Y_Type().Abbreviation();
};


SharedString::SharedString() : index_p(external_data->CreateString()), deleteDataWhenScopeEnds_p(true) {};
SharedString::SharedString(int index, bool deleteDataWhenScopeEnds) : index_p(index), deleteDataWhenScopeEnds_p(deleteDataWhenScopeEnds) {};
SharedString::~SharedString() {
	if (deleteDataWhenScopeEnds_p) {
		external_data->DeleteString(index_p);
	}
};
void    SharedString::Set(std::string v) { external_data->AppendData(index_p, v.c_str()); };
std::string   SharedString::Get() {
	auto* p = external_data->GetStringRef(index_p);
	if (p) {
		return std::string(p->c_str());
	}
	else {
		return std::string("");
	}
};
int     SharedString::Index() { return index_p; };
#pragma endregion

#pragma region ScriptingNode

class ScriptingNodeImpl
{
public:
	ScriptingNodeImpl() :
		mutex(),
		startLine(0),
		startColumn(0),
		endLine(0),
		endColumn(0),
		text(""),
		type(WaterWatchEnums::ScriptNodeType::Noop),
		typeHint(""),
		depth(0)
	{};
	~ScriptingNodeImpl() {};

	std::string			text_get() const {
		AUTO g{ mutex.Guard() };
		return std::string(text.c_str());
	};
	int			        startLine_get() const {
		AUTO g{ mutex.Guard() };
		return startLine;
	};
	int			        startColumn_get() const {
		AUTO g{ mutex.Guard() };
		return startColumn;
	};
	int			        endLine_get() const {
		AUTO g{ mutex.Guard() };
		return endLine;
	};
	int			        endColumn_get() const {
		AUTO g{ mutex.Guard() };
		return endColumn;
	};
	WaterWatchEnums::ScriptNodeType	    type_get() const {
		AUTO g{ mutex.Guard() };
		return type;
	};
	std::string			typeHint_get() const {
		AUTO g{ mutex.Guard() };
		return std::string(typeHint.c_str());
	};
	int			        depth_get() const {
		AUTO g{ mutex.Guard() };
		return depth;
	};
	void			    text_set(std::string s) {
		AUTO g{ mutex.Guard() };
		text = s;
	};
	void		        startLine_set(int s) {
		AUTO g{ mutex.Guard() };
		startLine = s;
	};
	void		        startColumn_set(int s) {
		AUTO g{ mutex.Guard() };
		startColumn = s;
	};
	void		        endLine_set(int s) {
		AUTO g{ mutex.Guard() };
		endLine = s;
	};
	void		        endColumn_set(int s) {
		AUTO g{ mutex.Guard() };
		endColumn = s;
	};
	void        	    type_set(WaterWatchEnums::ScriptNodeType s) {
		AUTO g{ mutex.Guard() };
		type = s;
	};
	void	    		typeHint_set(std::string s) {
		AUTO g{ mutex.Guard() };
		typeHint = s;
	};
	void		        depth_set(int s) {
		AUTO g{ mutex.Guard() };
		depth = s;
	};

private:
	mutable cweeSysMutex        mutex;
	int			        startLine;
	int			        startColumn;
	int			        endLine;
	int			        endColumn;
	std::string			text;
	WaterWatchEnums::ScriptNodeType	    type;
	std::string			typeHint;
	int			        depth;
};

ScriptingNode::ScriptingNode() : data(cweeSharedPtr<void>(cweeSharedPtr<ScriptingNodeImpl>(new ScriptingNodeImpl()), [](void* p) { return p; })) {};
ScriptingNode::~ScriptingNode() {};

std::string			ScriptingNode::text_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->text_get();
};
int			        ScriptingNode::startLine_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->startLine_get();
};
int			        ScriptingNode::startColumn_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->startColumn_get();
};
int			        ScriptingNode::endLine_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->endLine_get();
};
int			        ScriptingNode::endColumn_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->endColumn_get();
};
WaterWatchEnums::ScriptNodeType	    ScriptingNode::type_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->type_get();
};
std::string			ScriptingNode::typeHint_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->typeHint_get();
};
int			        ScriptingNode::depth_get() const {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	return ptr->depth_get();
};
void			    ScriptingNode::text_set(std::string s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->text_set(s);
};
void		        ScriptingNode::startLine_set(int s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->startLine_set(s);
};
void		        ScriptingNode::startColumn_set(int s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->startColumn_set(s);
};
void		        ScriptingNode::endLine_set(int s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->endLine_set(s);
};
void		        ScriptingNode::endColumn_set(int s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->endColumn_set(s);
};
void        	    ScriptingNode::type_set(WaterWatchEnums::ScriptNodeType s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->type_set(s);
};
void	    		ScriptingNode::typeHint_set(std::string s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->typeHint_set(s);
};
void		        ScriptingNode::depth_set(int s) {
	cweeSharedPtr<ScriptingNodeImpl> ptr(data, [](void* p) { return static_cast<ScriptingNodeImpl*>(p); });
	ptr->depth_set(s);
};

#pragma endregion






#pragma region Script Engine
ScriptEngine::ScriptEngine() : ptr(cweeSharedPtr<void>(cweeSharedPtr< chaiscript::WaterWatch_ChaiScript>(new chaiscript::WaterWatch_ChaiScript()), [](void* p) { return p; })) {};
ScriptEngine::~ScriptEngine() {};
Awaiter    ScriptEngine::DoScriptAsync(std::string command) {
	Awaiter toReturn;

	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> enginePtr(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	cweeJob task1([](std::string const& cmd, cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine, Awaiter& place_results)-> std::string {
		cweeStr STR;

		try {
			auto bv = engine->eval(cmd.c_str(), chaiscript::Exception_Handler());
			if (!bv.get_type_info().is_void()) STR = cweeStr::printf("Retrieved Object of Type: '%s'.", bv.get_type_info().name());
			else  STR = "";

			if (!bv.is_type(chaiscript::user_type<void>())) {
				try {
					auto str = engine->call_function(engine->eval("to_string"), bv);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = chaiscript::boxed_cast<cweeStr>(str);
					}
				}
				catch (...) {}
			}
		}
		catch (chaiscript::Boxed_Value e) {
			if (!e.is_type(chaiscript::user_type<void>())) {
				try {
					auto str = engine->call_function(engine->eval("to_string"), e);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
					}
				}
				catch (chaiscript::exception::eval_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
				}
				catch (chaiscript::exception::dispatch_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arity_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arithmetic_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::runtime_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::exception const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (...) {
					STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
				}
			}
		}
		catch (chaiscript::exception::eval_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
		}
		catch (chaiscript::exception::dispatch_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arity_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arithmetic_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::runtime_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::exception const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (...) {}

		*place_results.data_m = STR.c_str();
		*place_results.isFinished_m = true;

		return "";
		}, command, enginePtr, toReturn);
	task1.AsyncInvoke();

	return toReturn;
};
std::string    ScriptEngine::DoScript(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	cweeStr STR;
	try {
		auto bv = engine->eval(command.c_str(), chaiscript::Exception_Handler());

		auto typeName = engine->type_name(bv);
		if (!bv.get_type_info().is_void()) STR = cweeStr::printf("Retrieved Object of Type: '%s'.", typeName.c_str());
		else  STR = "";

		if (!bv.is_type(chaiscript::user_type<void>())) {
			try {
				auto str = engine->call_function(engine->eval("to_string"), bv);
				if (str.is_type(chaiscript::user_type<std::string>())) {
					STR = chaiscript::boxed_cast<std::string>(str).c_str();
				}
				else if (str.is_type(chaiscript::user_type<cweeStr>())) {
					STR = chaiscript::boxed_cast<cweeStr>(str);
				}
			}
			catch (...) {}
		}
	}
	catch (chaiscript::Boxed_Value e) {
		if (!e.is_type(chaiscript::user_type<void>())) {
			if (e.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(e).pretty_print().c_str();
			}
			else if (e.is_type(chaiscript::user_type<chaiscript::exception::dispatch_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::dispatch_error>(e).pretty_print().c_str();
			}
			else if (e.is_type(chaiscript::user_type<chaiscript::exception::arity_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::arity_error>(e).what();
			}
			else if (e.is_type(chaiscript::user_type<chaiscript::exception::arithmetic_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::arithmetic_error>(e).what();
			}
			else if (e.is_type(chaiscript::user_type<std::runtime_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::runtime_error>(e).what();
			}
			else if (e.is_type(chaiscript::user_type<std::exception>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::exception>(e).what();
			}
			else {
				try {
					STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
					auto str = engine->call_function(engine->eval("to_string"), e);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
					}
					else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
					}
				}
				catch (...) {
					STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
				}
			}
		}
		else {
			STR = "";
			return STR.c_str();
		}
	}
	catch (chaiscript::exception::eval_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
	}
	catch (chaiscript::exception::dispatch_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
	}
	catch (chaiscript::exception::arity_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (chaiscript::exception::arithmetic_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::runtime_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::exception const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (...) {}
	return STR.c_str();
};
std::vector<std::string> ScriptEngine::DoScript_Cast_VectorStrings(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	std::vector<std::string> result;
	try {
		auto bv = engine->eval(command.c_str());
		if (bv.is_type(chaiscript::user_type<void>())) {
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<std::vector<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<std::string>())) {
					result.push_back(chaiscript::boxed_cast<std::string>(x));
				}
				else if (x.is_type(chaiscript::user_type<cweeStr>())) {
					result.push_back(chaiscript::boxed_cast<cweeStr>(x).c_str());
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<cweeList<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<cweeList<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<std::string>())) {
					result.push_back(chaiscript::boxed_cast<std::string>(x));
				}
				else if (x.is_type(chaiscript::user_type<cweeStr>())) {
					result.push_back(chaiscript::boxed_cast<cweeStr>(x).c_str());
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<std::string>>())) {
			result = chaiscript::boxed_cast<std::vector<std::string>>(bv);
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<cweeStr>>())) {
			auto res = chaiscript::boxed_cast<std::vector<cweeStr>>(bv);
			for (auto& x : res) {
				result.push_back(x.c_str());
			}
			return result;
		}
		else {
			// do what we can... 




		}
	}
	catch (chaiscript::Boxed_Value bv) {
		if (bv.is_type(chaiscript::user_type<void>())) {
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<std::vector<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<std::string>())) {
					result.push_back(chaiscript::boxed_cast<std::string>(x));
				}
				else if (x.is_type(chaiscript::user_type<cweeStr>())) {
					result.push_back(chaiscript::boxed_cast<cweeStr>(x).c_str());
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<std::string>>())) {
			result = chaiscript::boxed_cast<std::vector<std::string>>(bv);
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<cweeStr>>())) {
			auto res = chaiscript::boxed_cast<std::vector<cweeStr>>(bv);
			for (auto& x : res) {
				result.push_back(x.c_str());
			}
			return result;
		}
	}
	catch (...) {}
	return result;
};
std::vector<float> ScriptEngine::DoScript_Cast_VectorFloats(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	std::vector<float> result;
	try {
		auto bv = engine->eval(command.c_str());
		if (bv.is_type(chaiscript::user_type<void>())) {
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<std::vector<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<float>())) {
					result.push_back(chaiscript::boxed_cast<float>(x));
				}
				else if (x.is_type(chaiscript::user_type<double>())) {
					result.push_back(chaiscript::boxed_cast<double>(x));
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<cweeList<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<cweeList<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<float>())) {
					result.push_back(chaiscript::boxed_cast<float>(x));
				}
				else if (x.is_type(chaiscript::user_type<double>())) {
					result.push_back(chaiscript::boxed_cast<double>(x));
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<float>>())) {
			result = *chaiscript::boxed_cast<std::vector<float>*>(bv);
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<double>>())) {
			auto res = chaiscript::boxed_cast<std::vector<double>*>(bv);
			for (auto& x : *res) {
				result.push_back(x);
			}
			return result;
		}
		else {
			// do what we can... 

		}
	}
	catch (chaiscript::Boxed_Value bv) {
		if (bv.is_type(chaiscript::user_type<void>())) {
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<std::vector<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<float>())) {
					result.push_back(chaiscript::boxed_cast<float>(x));
				}
				else if (x.is_type(chaiscript::user_type<double>())) {
					result.push_back(chaiscript::boxed_cast<double>(x));
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<cweeList<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<cweeList<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<float>())) {
					result.push_back(chaiscript::boxed_cast<float>(x));
				}
				else if (x.is_type(chaiscript::user_type<double>())) {
					result.push_back(chaiscript::boxed_cast<double>(x));
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<float>>())) {
			result = *chaiscript::boxed_cast<std::vector<float>*>(bv);
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<double>>())) {
			auto res = chaiscript::boxed_cast<std::vector<double>*>(bv);
			for (auto& x : *res) {
				result.push_back(x);
			}
			return result;
		}
	}
	catch (...) {}
	return result;
};
std::vector< ScriptingNode > ScriptEngine::PreParseScript(std::string command) {
	std::vector< ScriptingNode > out;
	cweeStr STR = "";
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });
	try {
		try {
			AUTO parserFunc = engine->eval("`DoParse`");

			AUTO bv_of_list_of_bv = engine->call_function(parserFunc, chaiscript::var(cweeStr(command.c_str())), chaiscript::var(cweeStr("WaterWatchParse")));

			try {
				AUTO list_of_bv = engine->boxed_cast<std::vector<chaiscript::Boxed_Value>*>(bv_of_list_of_bv);
				if (list_of_bv) {
					out.reserve(list_of_bv->size() + 1);
					for (auto& x : *list_of_bv) {
						try {
							AUTO containerPTR = engine->boxed_cast<std::map<std::string, chaiscript::Boxed_Value>*>(x);
							if (containerPTR) {
								if (containerPTR->find("node") != containerPTR->end()) {
									try {
										AUTO nodePtr = engine->boxed_cast<chaiscript::AST_Node*>(containerPTR->operator[]("node"));
										if (nodePtr) {
											if (nodePtr->filename() != "WaterWatchParse") {
												// this is likely a case of from-scratch evaluation within an evaluation. (i.e. "10 == ${ 10.0 }";
												// to fix this would require upating the "ListNodes" script function to "fix" the column and line notices based on the parenthood and/or based on the filename of the parser.
												continue;
											}
											bool WrongNamespace = false;
											for (auto& child : nodePtr->get_children()) {
												if (child.get().filename() != "WaterWatchParse") {
													WrongNamespace = true;
													break;
												}
											}
											if (WrongNamespace) continue;

											auto temp = ScriptingNode();
											if (containerPTR->find("type") != containerPTR->end()) {
												try {
													std::string* type_str_ptr = engine->boxed_cast<std::string*>(containerPTR->operator[]("type"));
													if (type_str_ptr) {
														temp.typeHint_set(*type_str_ptr);
													}
													else {
														temp.typeHint_set("");
													}
												}
												catch (...) { temp.typeHint_set(""); }
											}
											else {
												temp.typeHint_set("");
											}

											if (containerPTR->find("depth") != containerPTR->end()) {
												try {
													int* depth_ptr = engine->boxed_cast<int*>(containerPTR->operator[]("depth"));
													if (depth_ptr) {
														temp.depth_set(*depth_ptr);
													}
													else {
														temp.depth_set(0);
													}
												}
												catch (...) { temp.depth_set(0); }
											}
											else {
												temp.depth_set(0);
											}

											temp.text_set(nodePtr->text.c_str());
											temp.startLine_set(nodePtr->start().line - 1);
											temp.startColumn_set(nodePtr->start().column);
											temp.endLine_set(nodePtr->end().line - 1);
											temp.endColumn_set(nodePtr->end().column);
											temp.type_set(static_cast<WaterWatchEnums::ScriptNodeType>(static_cast<int>(nodePtr->identifier)));

											out.push_back(temp);
										}
										else {
										}
									}
									catch (...) {}
								}
								else {
								}
							}
							else {
							}
						}
						catch (...) {}
					}
				}
				return out;
			}
			catch (chaiscript::Boxed_Value e) {
				if (!e.is_type(chaiscript::user_type<void>())) {
					try {
						STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
						auto str = engine->call_function(engine->eval("to_string"), e);
						if (str.is_type(chaiscript::user_type<std::string>())) {
							STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
						}
						else if (str.is_type(chaiscript::user_type<cweeStr>())) {
							STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
						}
						else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
							STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
						}
					}
					catch (chaiscript::exception::eval_error const& e) {
						STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
					}
					catch (chaiscript::exception::dispatch_error const& e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (chaiscript::exception::arity_error const& e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (chaiscript::exception::arithmetic_error const& e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (std::runtime_error const& e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (std::exception const& e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (...) {
						STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
					}
				}
			}
			catch (chaiscript::exception::eval_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
			}
			catch (chaiscript::exception::dispatch_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arity_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arithmetic_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::runtime_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::exception const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (...) {
				STR = (cweeStr("Error: ") + cweeStr("UNKNOWN ERR 1"));
			}
			if (STR != "") {
				ScriptingNode node;
				node.depth_set(0);
				node.type_set(WaterWatchEnums::ScriptNodeType::Error);
				node.text_set(STR.c_str());
				auto splitter = STR.Split(" at (");
				if (splitter.getNumVars() >= 2) {
					auto splitter2 = splitter[1].Split(")");
					if (splitter2.getNumVars() >= 1) {
						auto line_col = splitter2[0].Split(",");
						for (auto& c : line_col) c.Replace(" ", "");
						if (line_col.getNumVars() >= 2) {
							node.startLine_set(0);
							node.endLine_set((int)(line_col[0]));
							node.startColumn_set(0);
							node.endColumn_set((int)(line_col[1]));
						}
					}
				}
				out.push_back(node);
				STR = "";
			}

			return out;
		}
		catch (chaiscript::Boxed_Value e) {
			if (!e.is_type(chaiscript::user_type<void>())) {
				try {
					STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
					auto str = engine->call_function(engine->eval("to_string"), e);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
					}
					else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
					}
				}
				catch (chaiscript::exception::eval_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
				}
				catch (chaiscript::exception::dispatch_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arity_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arithmetic_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::runtime_error const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::exception const& e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (...) {
					STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
				}
			}
		}
		catch (chaiscript::exception::eval_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
		}
		catch (chaiscript::exception::dispatch_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arity_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arithmetic_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::runtime_error const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::exception const& e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (...) {
			STR = (cweeStr("Error: ") + cweeStr("UNKNOWN ERR 2"));
		}
		if (STR != "") {
			ScriptingNode node;
			node.depth_set(0);
			node.type_set(WaterWatchEnums::ScriptNodeType::Error);
			node.text_set(STR.c_str());
			auto splitter = STR.Split(" at (");
			if (splitter.getNumVars() >= 2) {
				auto splitter2 = splitter[1].Split(")");
				if (splitter2.getNumVars() >= 1) {
					auto line_col = splitter2[0].Split(",");
					for (auto& c : line_col) c.Replace(" ", "");
					if (line_col.getNumVars() >= 2) {
						node.startLine_set(0);
						node.endLine_set((int)(line_col[0]));
						node.startColumn_set(0);
						node.endColumn_set((int)(line_col[1]));
					}
				}
			}
			out.push_back(node);
			STR = "";
		}
	}
	catch (chaiscript::Boxed_Value e) {
		if (!e.is_type(chaiscript::user_type<void>())) {
			try {
				STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
				auto str = engine->call_function(engine->eval("to_string"), e);
				if (str.is_type(chaiscript::user_type<std::string>())) {
					STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
				}
				else if (str.is_type(chaiscript::user_type<cweeStr>())) {
					STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
				}
				else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
					STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
				}
			}
			catch (chaiscript::exception::eval_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
			}
			catch (chaiscript::exception::dispatch_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arity_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arithmetic_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::runtime_error const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::exception const& e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (...) {
				STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
			}
		}
	}
	catch (chaiscript::exception::eval_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
	}
	catch (chaiscript::exception::dispatch_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (chaiscript::exception::arity_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (chaiscript::exception::arithmetic_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::runtime_error const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::exception const& e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (...) {
		STR = (cweeStr("Error: ") + cweeStr("UNKNOWN ERR 3"));
	}
	if (STR != "") {
		ScriptingNode node;
		node.depth_set(0);
		node.type_set(WaterWatchEnums::ScriptNodeType::Error);
		node.text_set(STR.c_str());
		auto splitter = STR.Split(" at (");
		if (splitter.getNumVars() >= 2) {
			auto splitter2 = splitter[1].Split(")");
			if (splitter2.getNumVars() >= 1) {
				auto line_col = splitter2[0].Split(",");
				for (auto& c : line_col) c.Replace(" ", "");
				if (line_col.getNumVars() >= 2) {
					node.startLine_set(0);
					node.endLine_set((int)(line_col[0]));
					node.startColumn_set(0);
					node.endColumn_set((int)(line_col[1]));
				}
			}
		}
		out.push_back(node);
		STR = "";
	}

	return out;
};
std::vector<std::string> ScriptEngine::CompatibleFunctions(std::string TypeName) {
	return DoScript_Cast_VectorStrings(cweeStr::printf("return `%s`.get_compatible_functions().keys();", TypeName.c_str()).c_str());
};
std::vector<std::string> ScriptEngine::FunctionsThatStartWith(std::string startsWith) {
	return DoScript_Cast_VectorStrings(cweeStr::printf("get_functions_that_start_with(\"%s\")", startsWith.c_str()).c_str());
};
void ScriptEngine::StopCurrentScript() {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });
	if (engine) {
		engine->get_eval_engine().CancelCurrentScript();
	}
};


std::string ScriptEngine::Cast_String(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	AUTO bv = engine->eval(command.c_str(), chaiscript::Exception_Handler());
	std::string out;
	AUTO val = chaiscript::boxed_cast<std::string*>(bv);
	if (val) {
		out = *val;
	}
	return out;
};
std::vector<float> ScriptEngine::Cast_VectorFloats(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	AUTO boxed = engine->eval(command.c_str(), chaiscript::Exception_Handler());
	std::vector<float> out;
	AUTO val2 = chaiscript::boxed_cast<std::vector<float>*>(boxed);
	if (val2) {
		out = *val2;
	}
	return out;
};

#pragma endregion

#pragma region GEOCOING
std::map<std::string, std::string> Geocoding::Geocode(std::string address) {
	std::map<std::string, std::string> out;

	AUTO long_lat = geocoding->GetLongLat(address.c_str());
	AUTO corrected_address = geocoding->GetAddress(long_lat);
	out["longitude"] = std::to_string(long_lat.x);
	out["latitude"] = std::to_string(long_lat.y);
	out["elevation (ft)"] = std::to_string(geocoding->GetElevation(long_lat));
	out["input address"] = address;
	out["address"] = std::string(corrected_address.c_str());

	return out;
};
std::map<std::string, std::string> Geocoding::Geocode(double longitude, double latitude) {
	std::map<std::string, std::string> out;

	AUTO corrected_address = geocoding->GetAddress(vec2d(longitude, latitude));
	out["longitude"] = std::to_string(longitude);
	out["latitude"] = std::to_string(latitude);
	out["elevation (ft)"] = std::to_string(geocoding->GetElevation(vec2d(longitude, latitude)));
	out["address"] = std::string(corrected_address.c_str());

	return out;
};

double Geocoding::Elevation_ft(std::string address) {
	double out;
	out = geocoding->GetElevation(geocoding->GetLongLat(address.c_str()));
	return out;
};
double Geocoding::Elevation_ft(double longitude, double latitude) {
	double out;
	out = geocoding->GetElevation(vec2d(longitude, latitude));
	return out;
};
#pragma endregion



#pragma region WATERWATCH

std::vector<double> WaterWatch::half1(const std::vector<double>& v) {
	std::vector<double> w(v);
	for (unsigned int i = 0; i < w.size(); i++)
		w[i] /= 2.0;
	return w;
};
std::vector<int> WaterWatch::half2(const std::vector<int>& v) {
	std::vector<int> w(v);
	for (unsigned int i = 0; i < w.size(); i++)
		w[i] /= 2.0;
	return w;
};
std::vector<float> WaterWatch::half3(const std::vector<float>& v) {
	std::vector<float> w(v);
	for (unsigned int i = 0; i < w.size(); i++)
		w[i] /= 2.0;
	return w;
};


std::vector<float>  WaterWatch::TestVector() {
	//AUTO cpp_list = std::vector<float>({ 1,2,3,4,5 });
	//AUTO list = PyList_New(0);
	//for (auto& x : cpp_list) {
		// PyList_Append(list, PyFloat_FromDouble(x));
	//}
	//return list;
	return std::vector<float>({1,2,3,4,5});
};
int WaterWatch::TestVector2(std::vector<float> const& a) {
	return a.size();
};
int WaterWatch::TestVector2(std::vector<double> const& a) {
	return a.size();
};
int WaterWatch::TestVector2(std::vector<int> const& a) {
	return a.size();
};


void WaterWatch::SubmitToast(std::string title, std::string content) {
	cweeToasts->submitToast(title.c_str(), content.c_str());
};
Pair<bool, Pair<std::string, std::string>> WaterWatch::TryGetToast() {
	static auto empty = Pair<bool, Pair<std::string, std::string>>(false, Pair<std::string, std::string>("", ""));
	cweeStr title, desc;

	if (cweeToasts->tryGetToast(title, desc))
		return Pair<bool, Pair<std::string, std::string>>(true, Pair<std::string, std::string>(std::string(title.c_str()), std::string(desc.c_str())));
	else
		return empty;
};
Pair<int, Pair<std::string, std::vector<std::string>>> WaterWatch::TryGetAppRequest() {
	static auto empty = Pair<int, Pair<std::string, std::vector<std::string>>>(-1, Pair<std::string, std::vector<std::string>>("", std::vector<std::string>()));

	std::pair<
		int, // ID
		cweeUnion<
		cweeStr, // Function Name
		cweeThreadedList<cweeStr>, // Arguments (optional)
		cweeSharedPtr<cweeStr> // result ptr?
		>
	> request = AppLayerRequests->DequeueRequest();
	if (request.first >= 0) {
		std::vector<std::string> params; for (auto& p : request.second.get<1>()) params.push_back(p.c_str());
		return Pair<int, Pair<std::string, std::vector<std::string>>>(request.first, Pair<std::string, std::vector<std::string>>(request.second.get<0>().c_str(), params));
	}
	else {
		return empty;
	}
};
void WaterWatch::CompleteAppRequest(int ID, std::string reply) {
	AppLayerRequests->FinishRequest(ID, reply.c_str());
};

std::string WaterWatch::GetDataDirectory() {
	return fileSystem->getDataFolder().c_str();
};
void WaterWatch::SetDataDirectory(std::string dir) {
	fileSystem->setDataFolder(dir.c_str());
};
std::string WaterWatch::GetTemporaryFilePath(std::string extension) {
	return fileSystem->createRandomFile(extension.c_str()).c_str();
};

Pair<double, double> WaterWatch::GeocodeAddress(std::string address) {
	auto t = geocoding->GetLongLat(address.c_str());
	return std::make_pair(t.x, t.y);
};
std::string WaterWatch::GeocodeAddress(double longitude, double latitude) {
	return geocoding->GetAddress(vec2d(longitude, latitude)).c_str();
};
double WaterWatch::GeocodeElevation(double longitude, double latitude) {
	return geocoding->GetElevation(vec2d(longitude, latitude));
};
Pair<double, double> WaterWatch::ValidateCoordinates(double longitude, double latitude) {
	if (longitude >= -180 && longitude <= 180 && latitude >= -90 && latitude <= 90) {
		return Pair<double, double>(longitude, latitude);
	}
	else {
		AUTO coords = geocoding->GetLongLat(longitude, latitude);
		return Pair<double, double>(coords.x, coords.y);
	}
};

double WaterWatch::Encode_2D_to_1D(double x, double y, double H) {
	return cweeInterpolatedMatrix<float>::Encode_2D_to_1D(x, y, H);
};
Pair<double, double> WaterWatch::Decode_1D_to_2D(double l, double H) {
	auto x = cweeInterpolatedMatrix<float>::Decode_1D_to_2D(l, H);
	return std::make_pair((double)x.first, (double)x.second);
};

std::vector<float> WaterWatch::PredictNext(std::vector<double> const& data, int nextN) {

	cweeList<float> labels = data;
	cweeList<cweeList<float>> features; {
		features.Append((cweeList<float>)data);
		features.Append((cweeList<float>)data);
	}

	std::pair<vec2, vec2> fit;

	AUTO ml_result = cweeMachineLearning::Learn(labels, features, &fit);
	return { ml_result.performance.x, ml_result.performance.y, ml_result.performance.z, ml_result.performance.w };
};
cweeDateTime WaterWatch::getCurrentTime() {
	return cweeDateTime((u64)fileSystem->getCurrentTime());
};

int WaterWatch::RandomInt(int min, int max) { return cweeRandomFloat(min, max); };
float WaterWatch::RandomFloat(float min, float max) { return cweeRandomFloat(min, max); };
std::string WaterWatch::GetBestMatch(std::string const& input, std::vector<std::string> const& options) {
	cweeStr a = input.c_str();
	cweeThreadedList<cweeStr> op(options.size() + 16);

	for (auto& x : options) op.Append(x.c_str());

	cweeStr bestMatch = a.BestMatch(op);
	return bestMatch.c_str();
};

int WaterWatch::GetNumMultithreadingCores() {
	return WindowsPlatform::GetCPUInfo().logicalProcessorCount;
};
int WaterWatch::GetNumLogicalCoresOnMachine() {
	return WindowsPlatform::GetCPUInfo().logicalProcessorCount;
};
int WaterWatch::GetNumPhysicalCoresOnMachine() {
	return WindowsPlatform::GetCPUInfo().processorCoreCount;
};
float WaterWatch::GetPercentMemoryUsedOfMachine() {
	return Computer_Usage().PercentMemoryUsed();
};
float WaterWatch::GetPercentCpuUsedOfMachine() {
	return WindowsPlatform::GetCPULoad();
};

void WaterWatch::AddToLog(std::string filePath, std::string content) {
	cweeStr previousFile;
	fileSystem->readFileAsCweeStr(previousFile, filePath.c_str());
	fileSystem->writeFileFromCweeStr(filePath.c_str(), previousFile.AddToDelimiter(content.c_str(), "\n"));
};
double WaterWatch::GetNanosecondsSinceStart() {
	return clock_ns();
};


std::string WaterWatch::DoScriptImmediately(std::string command) {

	static cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine = cweeSharedPtr<chaiscript::WaterWatch_ChaiScript>(new chaiscript::WaterWatch_ChaiScript(), [](chaiscript::WaterWatch_ChaiScript* p) {
		delete p;
		});
	try {
		auto bv = engine->eval(command);
		if (!bv.is_type(chaiscript::user_type<void>())) {
			try {
				return engine->to_string(bv).c_str();
			}
			catch (std::runtime_error e) {
				return (cweeStr("Error: ") + cweeStr(e.what())).c_str();
			}
			catch (...) {
				return "";
			}
		}
	}
	catch (std::runtime_error const& e) {
		return (cweeStr("Error: ") + cweeStr(e.what())).c_str();
	}
	catch (...) {
		return "";
	}
};
Awaiter WaterWatch::DoScript(std::string command) {
	Awaiter toReturn;

	cweeJob task1([](std::string const& cmd)-> std::string {
		static cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine = cweeSharedPtr<chaiscript::WaterWatch_ChaiScript>(new chaiscript::WaterWatch_ChaiScript(), [](chaiscript::WaterWatch_ChaiScript* p) {
			delete p;
			});
		try {
			auto bv = engine->eval(cweeStr(cmd.c_str()).c_str());
			if (!bv.is_type(chaiscript::user_type<void>())) {
				try {
					auto str = engine->call_function(engine->eval("to_string"), bv);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						std::string STR = chaiscript::boxed_cast<std::string>(str);
						return STR.c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						cweeStr STR = chaiscript::boxed_cast<cweeStr>(str);
						return STR.c_str();
					}
				}
				catch (...) {}
			}
		}
		catch (std::runtime_error const& e) {
			return (cweeStr("Error: ") + cweeStr(e.what())).c_str();
		}
		catch (...) {}

		return "";
		}, command);
	task1.ContinueWith([task1](Awaiter& place_results) ->void {
		*place_results.data_m = task1.GetResult().cast<std::string>();
		*place_results.isFinished_m = true;
		}, toReturn);
	task1.AsyncInvoke();

	return toReturn;
};

/*
int WaterWatch::Data_CreatePattern();
void WaterWatch::Data_DeletePattern(int index);
void WaterWatch::Data_ClearPattern(int index);
void WaterWatch::Data_AppendData(int index, double time, float value);
float WaterWatch::Data_GetValue(int index, double time);
int WaterWatch::Data_CreateMatrix();
void WaterWatch::Data_DeleteMatrix(int index);
void WaterWatch::Data_ClearMatrix(int index);
void WaterWatch::Data_AppendData(int index, double column, double row, float value);
float WaterWatch::Data_GetValue(int index, double column, double row);
std::vector<float> WaterWatch::Data_GetMatrix(int index, double left, double top, double right, double bottom, int nCol, int nRow);
int WaterWatch::Data_CreateSpatialAsset();
void WaterWatch::Data_DeleteSpatialAsset(int index);
void WaterWatch::Data_ClearSpatialAsset(int index);
void WaterWatch::Data_AppendData(int index, std::string const& name, std::string const& value);
void WaterWatch::Data_AppendData(int index, std::string const& name, cweeDateTime const& time, std::string const& value);
void WaterWatch::Data_AppendData(int index, std::string const& name, cweeDateTime const& time, float value);
std::vector<Pair<cweeDateTime, float>>	WaterWatch::Data_GetPattern(int index);
std::vector<Pair<cweeDateTime, float>>	WaterWatch::Data_GetPattern(int index, double from, double till);
std::vector<Pair<cweeDateTime, float>>	WaterWatch::Data_GetPattern(int index, double from, double till, double step);
*/



#pragma endregion

