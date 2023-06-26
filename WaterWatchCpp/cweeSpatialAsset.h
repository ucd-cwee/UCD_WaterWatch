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
#include "BalancedPattern.h"
#include "Pattern.h"
#include "Strings.h"
#include "List.h"
#include "UnorderedList.h"
#include "Mutex.h"
#include "SharedPtr.h"
#include "Curve.h"
#include "Parser.h"

/*! Generic object intended to host constant and time-varying strings as well as measurements associated to a real-world site.  */
class cweeThreadedSpatialAsset {
public: // ENUMS AND TYPEDEFS
	enum class SpatialAssetParam
	{
		missingParam = -1,
		constantParam = 0,
		timeseriesParam = 1,
		measurement = 2
	};
	typedef cweePair<SpatialAssetParam, int> assetParameter;

public: // PUBLIC FUNCTIONS

	cweeThreadedSpatialAsset& operator=(const cweeThreadedSpatialAsset& in) {
		SetName(in.GetName());

		Lock();
		in.Lock();
		{
			prev_index = in.prev_index;
			prev_search = in.prev_search;
			parameters = in.parameters;
		}
		in.Unlock();
		Unlock();

		timeseries->Copy(*in.timeseries.get());
		measurements->Copy(*in.measurements.get());
		constants->Copy(*in.constants.get());

		return *this;
	};
	void Clear() {
		Lock();
		prev_index = assetParameter();
		prev_search = "";
		parameters.clear();
		Unlock();

		measurements->Clear();
		timeseries->Clear();
		constants->Clear();
	};
	cweeThreadedSpatialAsset() {
		//measurements->SetGranularity(1); // set small so as not to suddenly require 2GB of memory on use of 1 pattern
		//timeseries->SetGranularity(2);
		//constants->SetGranularity(4);
	};

	/*! Add a parameter to the object of the specified category with the provided string key */
	void						AddParameter(const SpatialAssetParam& type, const cweeStr& name) {
		_addParameter(type, name);
	};
	/*! Add a constant string parameter to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const cweeStr& param) {
		auto p = _addParameter(SpatialAssetParam::constantParam, name);
		auto x = GetConstParameter(p);
		if (x) {
			x->Lock();
			*x->UnsafeRead() = param;
			x->Unlock();
		}
	};
	/*! Add a time-verying string parameter to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const cweeCurve<cweeStr>& param) {
		auto p = _addParameter(SpatialAssetParam::timeseriesParam, name);
		auto x = GetTimeSeriesParameter(p);
		if (x) {
			x->Lock();
			*x->UnsafeRead() = param;
			x->Unlock();
		}
	};
	/*! Add a time-varying measurement to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const Pattern& param) {
		auto p = _addParameter(SpatialAssetParam::measurement, name);
		auto x = GetMeasurement(p);
		if (x) {
			x = param;
		}
	};
	/*! Get the number of parameters in the object */
	int							Num() const {
		return constants->Num() + timeseries->Num() + measurements->Num();
	};
	/*! Get the nuber of parameters in the object of the requested type  */
	int							Num(const SpatialAssetParam& which) const {
		switch (which) {
		case SpatialAssetParam::constantParam:
			return constants->Num();
			break;
		case SpatialAssetParam::timeseriesParam:
			return timeseries->Num();
			break;
		case SpatialAssetParam::measurement:
			return measurements->Num();
			break;
		}
		return -1;
	};
	/*! Get a list of all parameter names in the object  */
	cweeThreadedList<cweeStr>	GetParameters() const {
		cweeThreadedList<cweeStr> out;

		Lock();
		out.SetGranularity(UnsafeGetParameters().size() + 16);
		for (auto& x : UnsafeGetParameters()) out.Append(x.first.c_str());
		Unlock();

		return out;
	};
	/*! Get a list of all parameter names in the object of the specified type */
	cweeThreadedList<cweeStr>	GetParameters(const SpatialAssetParam& which) const {
		switch (which) {
		case SpatialAssetParam::constantParam:
			return GetConstParameters();
			break;
		case SpatialAssetParam::timeseriesParam:
			return GetTimeSeriesParameters();
			break;
		case SpatialAssetParam::measurement:
			return GetMeasurementParameters();
			break;
		}
		return cweeThreadedList<cweeStr>();
	};
	/*! Get the type of the parameter key name */
	SpatialAssetParam			GetParameter(const cweeStr& name) {
		assetParameter out;
		_getParameter(name, out);
		return out.first;
	};
	/*! Replace the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const cweeStr& in) {
		GetConstParameter(name) = in;
	};
	/*! Append the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const u64& time, const cweeStr& in) {
		auto x = GetTimeSeriesParameter(name);
		x->Lock();
		auto ptr = x->UnsafeRead();
		if (ptr) {
			ptr->AddUniqueValue(time, in);
		}
		x->Unlock();
	};
	/*! Append the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const u64& time, float in) {
		GetMeasurement(name)->AddUniqueValue(time, in);
	};
	/*! Directly access the parameter with the provided name */
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>> GetConstParameter(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeThreadedSpatialAsset::SpatialAssetParam::constantParam, name);
			_getParameter(name, index);
		}
		return GetConstParameter(index);
	};
	/*! Directly access the parameter with the provided name */
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>> GetTimeSeriesParameter(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeThreadedSpatialAsset::SpatialAssetParam::timeseriesParam, name);
			_getParameter(name, index);
		}
		return GetTimeSeriesParameter(index);
	};
	/*! Directly access the parameter with the provided name */
	cweeUnorderedListReferenceObject<Pattern> GetMeasurement(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeThreadedSpatialAsset::SpatialAssetParam::measurement, name);
			_getParameter(name, index);
		}
		return GetMeasurement(index);
	};
	/*! Save the object to a cweeStr */
	cweeStr						Serialize(int option = -1) {
		cweeStr delim = ":CUSTOM_in_DELIM:";
		cweeStr out; cweeStr temp; cweeStr temp2;

		if (1) {
			auto list = constants->GetList();
			list.Sort();
			temp = ""; for (auto& x : list) {
				constants->PreventDeletion(x);
				constants->Lock();
				auto ptr = constants->UnsafeRead(x);
				constants->Unlock();
				if (ptr) {
					ptr->Lock();
					temp2 = cweeStr(x) + "," + *ptr->UnsafeRead(); // 1,STRING
					ptr->Unlock();
				}
				constants->AllowDeletion(x);

				// temp2 = cweeStr(x) + "," + (cweeStr)constants->Read(x); // 1,STRING
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim); if (out.IsEmpty()) out = " ";
		}
		if (1) {
			auto list = timeseries->GetList();
			list.Sort();
			temp = ""; for (auto& x : list) {
				timeseries->PreventDeletion(x);
				timeseries->Lock();
				auto ptr = timeseries->UnsafeRead(x);
				timeseries->Unlock();
				if (ptr) {
					ptr->Lock();
					temp2 = cweeStr(x) + "," + ptr->UnsafeRead()->Serialize(); // 1,STRING
					ptr->Unlock();
				}
				timeseries->AllowDeletion(x);

				// temp2 = cweeStr(x) + "," + timeseries->Read(x)->Serialize(); 
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim);
		}
		if (1) {
			auto list = measurements->GetList();
			list.Sort();
			temp = ""; for (auto& x : list) {
				measurements->PreventDeletion(x);
				measurements->Lock();
				auto ptr = measurements->UnsafeRead(x);
				measurements->Unlock();
				if (ptr) {
					temp2 = cweeStr(x) + "," + ptr->Serialize(); // 1,STRING
				}
				measurements->AllowDeletion(x);
				// temp2 = cweeStr(x) + "," + measurements->Read(x).Serialize(option); // 1,STRING
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim);
		}

		temp = "";
		Lock();
		for (auto& x : UnsafeGetParameters()) {
			cweeStr temp2 = cweeStr(static_cast<int>(x.second.first)) + "," + cweeStr(x.second.second);
			temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:");
		}
		Unlock();
		out.AddToDelimiter(temp, delim);

		temp = "";
		Lock();
		for (auto& x : UnsafeGetParameters()) {
			temp.AddToDelimiter(x.first.c_str(), ":CUSTOM_in_DELIM_in:");
		}
		Unlock();
		out.AddToDelimiter(temp, delim);

		temp = "";
		out.AddToDelimiter(GetName(), delim);

		return out;
	};
	/*! Load the object from a cweeStr */
	void						Deserialize(cweeStr& in) {
		if (in.IsEmpty()) {
			Clear();
			return;
		}

		cweeParser obj(in, ":CUSTOM_in_DELIM:", true);
		in.Clear(); cweeParser s, r;

		cweeThreadedList<cweeStr> names;
		{
			cweeStr& t = obj[4];
			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);

			Lock();
			names.SetGranularity((int)(s.getNumVars() + 16));
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;
				names.Append(x);
			}
			Unlock();
		}
		{
			cweeStr& t = obj[3];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);

			Lock();
			auto& c = UnsafeGetParameters();
			c.clear(); c.reserve((int)(s.getNumVars() + 16)); cweePair<SpatialAssetParam, int> q;
			int namesIndex = 0;

			for (auto& x : s)
			{
				if (x.IsEmpty() || names.Num() <= namesIndex) continue;
				r.Parse(x, ",", true);

				int newIndex = UnsafeGetNumParameters(static_cast<SpatialAssetParam>((int)r[0])) + 1;

				q = cweePair<SpatialAssetParam, int>(static_cast<SpatialAssetParam>((int)r[0]), newIndex);  //  (int)r[1]);
				c[names[namesIndex].c_str()] = q;
				namesIndex++;
			}
			Unlock();
		}

		{
			cweeStr& t = obj[0];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = constants;
			auto params = UnsafeGetParameters(SpatialAssetParam::constantParam);  int progress = 0;
			cweePair<int, cweeStr> q;
			for (auto& x : s)
			{
				if (x.IsEmpty() || x == " ") continue;
				r.ParseFirstDelimiterOnly(x, ",");

				if (params.Num() > progress) {
					c->Emplace(params[progress].second, r[1]);
				}
				progress++;
			}
		}
		{
			cweeStr& t = obj[1];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = timeseries;
			auto params = UnsafeGetParameters(SpatialAssetParam::timeseriesParam);  int progress = 0;
			cweePair<int, cweeCurve<cweeStr>> q; cweeCurve<cweeStr> o;
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;

				r.ParseFirstDelimiterOnly(x, ",");
				cweeStr& r2 = r[1];
				cweeStr& r1 = r[0];
				o.Deserialize(r[1]);

				if (params.Num() > progress) {
					c->Emplace(params[progress].second, o);
				}
				progress++;
			}
		}
		{
			cweeStr& t = obj[2];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = measurements;
			auto params = UnsafeGetParameters(SpatialAssetParam::measurement);  int progress = 0;
			cweePair<int, Pattern> q; Pattern o;
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;

				r.ParseFirstDelimiterOnly(x, ",");
				o.Deserialize(r[1]);

				if (params.Num() > progress) {
					c->Emplace(params[progress].second, o);
				}
				progress++;
			}

		}

		SetName(obj[5]);

		prev_search.Clear();
	};

	bool						ParameterExists(const cweeStr& name) {
		if (GetParameter(name) == SpatialAssetParam::missingParam) return false;
		return true;
	};
	bool						RemoveParameter(const cweeStr& name) {
		return _removeParameter(GetParameter(name), name);
	};

	cweeStr						GetName(void) const {
		cweeStr out;

		Lock();
		out = _name;
		Unlock();

		return out;
	};
	void						SetName(const cweeStr& newName) {
		Lock();
		_name = newName;
		Unlock();
	};





















































private: // PRIVATE FUNCTIONS
	assetParameter												_newParameter(const SpatialAssetParam& type) {
		assetParameter out;
		out.first = type;
		switch (type) {
		case SpatialAssetParam::constantParam:
			out.second = constants->Append();
			break;
		case SpatialAssetParam::timeseriesParam:
			out.second = timeseries->Append();
			break;
		case SpatialAssetParam::measurement:
			out.second = measurements->Append();
			break;
		}
		return out;
	};
	assetParameter												_addParameter(const SpatialAssetParam& type, const cweeStr& name) {
		Lock();
		auto found = UnsafeGetParameters().find(name.c_str());
		bool success = (found != UnsafeGetParameters().end());
		Unlock();

		if (!success) {
			// new object;
			auto newP = _newParameter(type); // first is type, second is unorderedMap index

			Lock();
			UnsafeGetParameters()[name.c_str()] = newP;
			Unlock();

			return newP;
		}
		else {
			// existing object;
			assetParameter out;

			Lock();
			if (found != UnsafeGetParameters().end()) {
				out = found->second;
			}
			Unlock();

			return out;
		}
	};
	bool														_getParameter(const cweeStr& name, assetParameter& out) const {
		bool earlyExit = false;
		Lock();
		if (name == UnsafeGetPrevSearch()) {
			earlyExit = true;
			out = UnsafeGetPrevIndex();
		}
		Unlock();
		if (earlyExit) return true;

		Lock();
		auto found = UnsafeGetParameters().find(name.c_str());
		bool success = (found != UnsafeGetParameters().end());
		Unlock();

		if (!success) {
			out.first = SpatialAssetParam::missingParam;
			return false;
		}
		else {
			Lock();
			if (found != UnsafeGetParameters().end()) {
				out = found->second;
				UnsafeGetPrevSearch() = name;
				UnsafeGetPrevIndex() = out;
			}
			Unlock();

			return true;
		}
	}
	bool														_removeParameter(const SpatialAssetParam& type, const cweeStr& name) {
		bool success = false;
		Lock();

		auto found = UnsafeGetParameters().find(name.c_str());
		if (found != UnsafeGetParameters().end()) {
			UnsafeGetPrevSearch() = "";

			switch (found->second.first) {
			case SpatialAssetParam::constantParam:
				constants->Erase(found->second.second);
				break;
			case SpatialAssetParam::timeseriesParam:
				timeseries->Erase(found->second.second);
				break;
			case SpatialAssetParam::measurement:
				measurements->Erase(found->second.second);
				break;
			}

			success = true;
		}

		Unlock();

		return success;
	}
protected: // PROTECTED FUNCTIONS
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>					GetConstParameter(const assetParameter& index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>				temp(constants, index.second);
		return temp;
	};
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>		GetTimeSeriesParameter(const assetParameter& index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>	temp(timeseries, index.second);
		return temp;
	};
	cweeUnorderedListReferenceObject<Pattern>					GetMeasurement(const assetParameter& index) {
		cweeUnorderedListReferenceObject<Pattern>				temp(measurements, index.second);
		return temp;
	};

	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>					GetConstParameter(int index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>				temp(constants, index);
		return temp;
	};
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>		GetTimeSeriesParameter(int index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>	temp(timeseries, index);
		return temp;
	};
	cweeUnorderedListReferenceObject<Pattern>					GetMeasurement(int index) {
		cweeUnorderedListReferenceObject<Pattern>				temp(measurements, index);
		return temp;
	};

	cweeThreadedList<cweeStr>									GetConstParameters() const {
		cweeThreadedList<cweeStr> out(constants->Num() + 1);

		Lock();
		int n = UnsafeGetParameters().size();
		for (auto& x : UnsafeGetParameters()) {
			if (x.second.first == SpatialAssetParam::constantParam) {
				out.Append(x.first.c_str());
			}
		}
		Unlock();

		return out;
	};
	cweeThreadedList<cweeStr>									GetTimeSeriesParameters() const {
		cweeThreadedList<cweeStr> out(timeseries->Num() + 1);

		Lock();
		int n = UnsafeGetParameters().size();
		for (auto& x : UnsafeGetParameters()) {
			if (x.second.first == SpatialAssetParam::timeseriesParam) {
				out.Append(x.first.c_str());
			}
		}
		Unlock();

		return out;
	};
	cweeThreadedList<cweeStr>									GetMeasurementParameters() const {
		cweeThreadedList<cweeStr> out(measurements->Num() + 1);

		Lock();
		int n = UnsafeGetParameters().size();
		for (auto& x : UnsafeGetParameters()) {
			if (x.second.first == SpatialAssetParam::measurement) {
				out.Append(x.first.c_str());
			}
		}
		Unlock();

		return out;
	};
	tsl::robin_map<std::string, assetParameter, robin_hood::hash<std::string>, std::equal_to<std::string>, std::allocator<std::pair<std::string, assetParameter>>, true>& UnsafeGetParameters() {
		return parameters;
	};
	const tsl::robin_map<std::string, assetParameter, robin_hood::hash<std::string>, std::equal_to<std::string>, std::allocator<std::pair<std::string, assetParameter>>, true>& UnsafeGetParameters() const {
		return parameters;
	};
	cweeThreadedList< assetParameter >							UnsafeGetParameters(const SpatialAssetParam& type) const {
		cweeThreadedList< assetParameter > out;
		out.SetGranularity(parameters.size() + 1);
		for (auto& x : parameters) {
			if (x.second.first == type) {
				out.Append(x.second);
			}
		}
		return out;
	};
	int															UnsafeGetNumParameters(const SpatialAssetParam& type) const {
		int out = 0;
		for (auto& x : parameters) {
			if (x.second.first == type) {
				out++;
			}
		}
		return out;
	};
	cweeStr& UnsafeGetPrevSearch() const {
		return prev_search;
	};
	assetParameter& UnsafeGetPrevIndex() const {
		return prev_index;
	};

	cweeSharedPtr < cweeUnorderedList< cweeUnpooledInterlocked<cweeStr> > >				constants = make_cwee_shared < cweeUnorderedList< cweeUnpooledInterlocked<cweeStr> > >(); // site name, site address, etc. 
	cweeSharedPtr < cweeUnorderedList< cweeUnpooledInterlocked<cweeCurve<cweeStr>> > >	timeseries = make_cwee_shared < cweeUnorderedList< cweeUnpooledInterlocked<cweeCurve<cweeStr>> > >(); // customer name, customer account number, etc. 
	cweeSharedPtr < cweeUnorderedList< Pattern > >				measurements = make_cwee_shared < cweeUnorderedList< Pattern > >(); // water usage, temperature, etc.

private: // PRIVATE DATA

	mutable cweeSysMutex										lock;
	tsl::robin_map<std::string, assetParameter, robin_hood::hash<std::string>, std::equal_to<std::string>, std::allocator<std::pair<std::string, assetParameter>>, true> parameters;
	// std::unordered_map<std::string, assetParameter>				parameters;

	mutable cweeStr												prev_search; // optimizaiton for searching the same key over and over
	mutable assetParameter										prev_index; // optimizaiton for searching the same key over and over

	void														Lock() const {
		lock.Lock();
	};
	void														Unlock() const {
		lock.Unlock();
	};

public: // PUBLIC DATA
	cweeStr														_name = "No Name";
};