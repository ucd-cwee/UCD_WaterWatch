#ifndef __PATTERNS_CPP__
#define __PATTERNS_CPP__

#pragma region "INCLUDES"
#pragma hdrstop
#include "precompiled.h"
#pragma endregion

template< class type >
cweeTime cweePattern_CatmullRomSpline<type>::getLocalTime(const u64& time) {
#if 1

	return fileSystem->localtime(time);

#else

	tm* out; cweeTime out1 {}; time_t t = time;
	if (t < 0) t = 0;
	out = std::localtime(&t);
	if (out) {
		out1 = *out;
		return out1;
	}
	else {
		return out1;
	}
#endif
};

template< class type >
cweePattern_CatmullRomSpline<type>			cweePattern_CatmullRomSpline<type>::GetTimePattern(bool returnHour) const {
	cweePattern_CatmullRomSpline<type> out;
	out.Copy(*this);
	if (returnHour == true) {
		// return 0 - 23 value representing the hour (0 is midnight)
		out.Lock();
		for (int i = 0; i < out.UnsafeGetValues().Num(); i++) {
			time_t time = out.UnsafeGetTimes()[i];
			cweeTime tmp = getLocalTime(time);
			out.UnsafeGetValues()[i] = (int)tmp.tm_hour();
		}
		out.Unlock();
	}
	else {
		// return 0 - 6 value representing the day of the week (0 is Sunday)
		out.Lock();
		for (int i = 0; i < out.UnsafeGetValues().Num(); i++) {
			time_t time = out.UnsafeGetTimes()[i];
			cweeTime tmp = getLocalTime(time);
			out.UnsafeGetValues()[i] = (int)tmp.tm_wday();
		}
		out.Unlock();
	}
	return out;
};

template< class type >
int											cweePattern_CatmullRomSpline<type>::GetMonthOfYear(const u64& Time) {
	time_t localTime = (u64)Time;
	cweeTime tmp = getLocalTime(localTime);
	return tmp.tm_mon();
};

template< class type >
int											cweePattern_CatmullRomSpline<type>::GetDayOfMonth(const u64& time) {
	time_t temp = time;
	cweeTime tmp = getLocalTime(temp);
	return tmp.tm_mday();
};

template< class type >
int											cweePattern_CatmullRomSpline<type>::GetDayOfWeek(const u64& time) {
	time_t temp = time;
	cweeTime tmp = getLocalTime(temp);
	return tmp.tm_wday();
};

template< class type >
float										cweePattern_CatmullRomSpline<type>::GetHourOfDay(const u64& time) {
	time_t temp = time;
	cweeTime tmp = getLocalTime(temp);
	return (((float)tmp.tm_hour()) + (((float)tmp.tm_min()) / 60.0f) + (((float)tmp.tm_sec()) / 3600.0f));
};

template< class type >
int											cweePattern_CatmullRomSpline<type>::GetSeason(const u64& time) {
	int month = GetMonthOfYear(time);
	switch (month) {
	case 0: { // jan
		return 0;
		break;
	}
	case 1: { // feb
		return 0;
		break;
	}
	case 2: { // mar
		return 0;
		break;
	}
	case 3: { // apr
		return 1;
		break;
	}
	case 4: { // may
		return 1;
		break;
	}
	case 5: { // june
		return 1;
		break;
	}
	case 6: { // july
		return 2;
		break;
	}
	case 7: { // aug
		return 2;
		break;
	}
	case 8: { // sep
		return 2;
		break;
	}
	case 9: { // oct
		return 3;
		break;
	}
	case 10: { // nov
		return 3;
		break;
	}
	case 11: { // dec
		return 3;
		break;
	}
	}
};


template< class type >
type								cweePattern_CatmullRomSpline<type>::GetTransformedCurrentValue(const u64& time, patternModifier mod, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent) const {
	switch (mod) {
	case patternModifier::None: {
		return this->GetCurrentValue(time, Parent);
		break;
	}
	case patternModifier::DayOfWeek: {
		return (type)this->GetDayOfWeek(time);
		break;
	}
	case patternModifier::HourOfDay: {
		return (type)this->GetHourOfDay(time);
		break;
	}
	case patternModifier::Velocity: {
		int i = this->IndexForTime(time);
		if (i + 1 < this->GetNumValues()) {
			u64 tP = this->TimeForIndex(i + 1) - this->TimeForIndex(i);
			if (tP == 0.0f) return 0.0f;
			return (this->ValueForIndex(i + 1) - this->ValueForIndex(i)) / (tP);
		}
		else {
			type toReturn;
			toReturn = 0;
			return toReturn;
		}
		break;
	}
	case patternModifier::Acceleration: {
		int i = this->IndexForTime(time);
		if (i + 1 < this->GetNumValues()) {
			u64 tP = this->TimeForIndex(i + 1) - this->TimeForIndex(i);
			if (tP == 0.0f) return 0.0f;
			return
				(
					GetTransformedCurrentValue(this->TimeForIndex(i + 1), patternModifier::Velocity, Parent)
					-
					GetTransformedCurrentValue(this->TimeForIndex(i), patternModifier::Velocity, Parent)
					) / (
						tP
						);
		}
		else {
			type toReturn;
			toReturn = 0;
			return toReturn;
		}
		break;
	}
	case patternModifier::MovingAverage: {
		return this->GetCurrentMovingAverage(time);
		break;
	}
	case patternModifier::Normalize: {
		return this->GetNormalizedValue(time);
		break;
	}
	}
};

template< class type >
void cweePattern<type>::RemoveTimes(const u64& greaterThan, const u64& lessThenEqualTo) {
	int lowerLimit, upperLimit, index; cweeThreadedList<int> indexesToDelete;
	Lock();
	{
		lowerLimit = UnsafeIndexForTime(greaterThan) - 1;
		upperLimit = UnsafeIndexForTime(lessThenEqualTo) + 1;
		indexesToDelete.SetGranularity(cweeMath::max(16, (upperLimit - lowerLimit) + 16));
		for (index = upperLimit; index >= lowerLimit; index--) {
			if (index < times.Num() && index >= 0) {
				auto& pos = times[index];
				if (pos > greaterThan && pos <= lessThenEqualTo) {
					indexesToDelete.Append(index);
				}
			}
		}

		times.RemoveIndexes(indexesToDelete); // safe if outside bounds
		values.RemoveIndexes(indexesToDelete); // safe if outside bounds
	}
	Unlock();
};

template< class type >
void cweePattern<type>::Lock() const {
	this->lock.Lock();
};

template< class type >
void cweePattern<type>::Unlock() const {
	this->lock.Unlock();
};

template< class type >
vec2 cweePattern_CatmullRomSpline<type>::Learn_Automated(const cweeThreadedList<std::pair<cweeStr, vec2>> FeaturePatterns, const cweeUnorderedList<cweePattern_CatmullRomSpline<type>>* Parent, const u64& timeStart, const u64& timeEnd) {
	// find the source of the feature patterns. if they cannot be found, they cannot be used. 
	int parentIndex; 
	for (auto& x : FeaturePatterns) {	
		if (x.first != this->GetName()) {
			if (Parent != nullptr) {
				parentIndex = Parent->FindIndexWithName(x.first);
				if (parentIndex > 0) {				
					patternSource source = patternSource::Parent;
					this->Lock();
					machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
					this->Unlock();
					continue;
				}
			}
			parentIndex = Globals->FindIndexWithName(x.first);
			if (parentIndex >= 0) {
				patternSource source = patternSource::Global;
				this->Lock();
				machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
				this->Unlock();
				continue;
			}
			parentIndex = Scada->FindIndexWithName(x.first);
			if (parentIndex >= 0) {
				patternSource source = patternSource::Scada;
				this->Lock();
				machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
				this->Unlock();
				continue;
			}
			parentIndex = Customers->FindIndexWithName(x.first);
			if (parentIndex >= 0) {
				patternSource source = patternSource::Customers;
				this->Lock();
				machineLearn_features.Append(vec4(static_cast<int>(source), parentIndex, x.second.x, x.second.y));
				this->Unlock();
				continue;
			}
			// not found anywhere. Bad luck, it cannot be used in the future.
		}
	}
	return Relearn_Automated(Parent, timeStart, timeEnd);
};

template< class type >
void SaveLearn(const cweePattern_CatmullRomSpline<type>* ptr, int numSamples, const cweeThreadedList< std::pair<u64, type> >& labels, const cweeThreadedList < cweeThreadedList< std::pair<u64, type> > >&	features, const vec4& perf); // dec
template< class type >
INLINE void SaveLearn(const cweePattern_CatmullRomSpline<type>* ptr, int numSamples, const cweeThreadedList< std::pair<u64, type> >& labels, const cweeThreadedList < cweeThreadedList< std::pair<u64, type> > >&	features, const vec4& perf) { // def
	// ATTEMPT 1. 
	if (1) {
		auto timeNow = cweeStr(fileSystem->getCurrentTime());
		//		Convert labels into a CSV document that can be appended to with each re-learn.  

		/*
		TIME (INT)		TIME (STRING)		LABELS_0		LABELS_1		LABELS_2
		16000000		Jan 1 2021			15				""				""
		16000000		Jan 2 2021			15				15				16
		16000000		Jan 3 2021			""				16				16
		16000000		Jan 4 2021			15				15				17
		...
		*/

		cweeStr rowDelim = "\r"; cweeStr colDelim = ",";
		cweeStr fileName = "RelearnPattern_" + ptr->GetName();
		cweeStr filePath = fileSystem->createFilePath(fileSystem->getDataFolder(), fileName, CSV);
		cweeStr prevContent; fileSystem->readFileAsCweeStr(prevContent, filePath);

		cweeThreadedList < cweeStr > headers;
		cweeThreadedList < cweeThreadedList< std::pair<u64, type> > > labelHistory; 
		{
			cweeParser a(prevContent, rowDelim, true);
			cweeParser b;
			for (int j = 0; j < a.getNumVars(); j++) {
				auto& row = a[j];
				b.Parse(row, colDelim, true);
				if (j == 0) {
					for (int i = 2; i < b.getNumVars(); i++)
						headers.Append(b[i]);
				}
				else {					
					std::pair<u64, type> v;
					for (int i = 0; i < b.getNumVars(); i++) {
						switch (i) {
						case 0:
							v.first = (float)b[i];
							break;
						case 1:
							// string version of t -- ignored.
							break;
						default:
							// column representing a label.
						{
							int colNum = i - 2; // 0, 1, 2
							while (labelHistory.Num() < colNum) labelHistory.Append(cweeThreadedList< std::pair<u64, type> >());
							if (!b[i].IsEmpty()) {
								v.second = (float)b[i];
								labelHistory[colNum].Append(v);
							}
						}
						break;
						}
					}
				}
			}
		}

		// add new 'column' to label history
		labelHistory.Append(labels);

		// new file content
		cweeStr newContent;
		{
			// header
			{
				cweeStr header;
				header.AddToDelimiter("TIME (int)", colDelim);
				header.AddToDelimiter("TIME (text)", colDelim);
				for (auto& x : headers)
					header.AddToDelimiter(x, colDelim);

				header.AddToDelimiter(timeNow + cweeStr::printf(" (%i samples) (%f | %f | %f | %f)", numSamples, perf.x, perf.y, perf.z, perf.w), colDelim);
				newContent.AddToDelimiter(header, rowDelim);
			}

			// ensure same number of rows throughout
			cweeThreadedList < cweeCurve<cweeStr> > out;
			for (int i = 0; i < labelHistory.Num(); i++) {
				out.Append(cweeCurve<cweeStr>());
				for (auto& x : labelHistory[i]) {
					out[i].AddUniqueValue(x.first, cweeStr((float)x.second));
				}
			}
			for (int i = 0; i < out.Num(); i++) {
				for (int j = 0; j < out.Num(); j++) {
					if (j == i) continue;

					auto& a = out[i];
					auto& b = out[j];

					for (auto& knot : a.GetKnotSeries()) {
						if (b.FindExactX(knot.first) < 0) {
							// value does not exist. 
							b.AddUniqueValue(knot.first, "");
						}
					}
				}
			}

			//// by this point, there should be an exact number of points in all out[] curves. 
			//int numRows = 0;
			//for (int i = 0; i < out.Num(); i++) {
			//	for (int j = 0; j < out.Num(); j++) {
			//		if (out[i].GetNumValues() != out[j].GetNumValues()) {
			//			fileSystem->submitToast("EDMS Warning", "Number of curve points do not match as Pattern.cpp, line 155.");
			//		}
			//		numRows = out[i].GetNumValues();
			//	}
			//}

			// Write to new file content
			for (auto& t : out[0].GetKnotSeries()) {
				cweeStr row;
				row.AddToDelimiter(t.first, colDelim);
				row.AddToDelimiter(cweeStr((time_t)t.first), colDelim);

				for (auto& label : out) {
					row.AddToDelimiter(label.GetCurrentValue(t.first), colDelim);				
				}
				newContent.AddToDelimiter(row, rowDelim);
			}
		}

		// save to OS
		fileSystem->writeFileFromCweeStr(filePath, newContent);
	}
};

template< class type >
vec2 cweePattern_CatmullRomSpline<type>::Relearn_Automated(const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent, const u64& timeStart, const u64& timeEnd)  {
	// instantiate the 'learned' object 
	this->Lock();
	if (!learned) {
		learned = new cweeML_learned_parameters();
	}
	this->Unlock();
	
	float splitSample = 20;

	if (this->isLearned()) {
		vec2 learnedPeriod = this->GetLearnedPeriod();
		this->RemoveUnnecessaryKnots(0, learnedPeriod.x);
		this->RemoveUnnecessaryKnots(learnedPeriod.y);
	}
	else {
		this->RemoveUnnecessaryKnots();
	}
		
	// override the current learned parameters based on the content of the machineLearn_features
	constexpr int MaxNumSamples = 80640; // 2688
	cweeThreadedList< std::pair<u64, type> >							labels;
	cweeThreadedList < cweeThreadedList< std::pair<u64, type> > >		features;

	bool learnWithExpansion = false;

	this->Lock();
	u64 start; 
	if (learned && learned->learned && learned->learnPeriod.x != 0 && timeStart == 0) {
		start = learned->learnPeriod.x;
		this->Unlock();
		learnWithExpansion = false;
	}
	else {
		this->Unlock();
		start = ((timeStart == 0) ? this->GetMinTime() : timeStart);
		learnWithExpansion = true;
	}

	this->Lock();
	u64 end; 
	if (learned && learned->learned && learned->learnPeriod.y != 0 && timeEnd == 0) {
		end = learned->learnPeriod.y;
		this->Unlock();
		learnWithExpansion = false;
	}
	else {
		this->Unlock();
		end = ((timeEnd == 0) ? this->GetMaxTime() : timeEnd);
		learnWithExpansion = true;
	}

	if (end <= start) {
		end = start + 7 * 24 * 60 * 60;
	}

	u64 minTimestep = this->GetMinimumTimeStep(); // seconds
	minTimestep /= 3600.0; // hours
	u64 _learnHourDelta = learnHourDelta; // 0.25 hours
	this->Lock();
	if (learned && learned->learned && learned->learnPeriod.z > 0) {
		_learnHourDelta = ::Max(_learnHourDelta, (u64)learned->learnPeriod.z);
		this->Unlock();
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Min(_learnHourDelta, (u64)2.0);
	}
	else if (learned && learned->learned && learned->learnPeriod.z <= 0) {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
	}
	else {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Min(_learnHourDelta, (u64)2.0);
	}
	

	float hr;	
	u64 resolutionSeconds = _learnHourDelta * 60.0f * 60.0f; // x-seconds step to achieve the desired resolution
	resolutionSeconds = ::Max(resolutionSeconds, minTimestep);

	resolutionSeconds *= (1.0f - (splitSample / 100.0f)); // i.e. 20% greater number of samples to counter-act the samples removed by the learn/test operation.
	float numSamples = (end - start) / resolutionSeconds;

	if (numSamples > MaxNumSamples) {
#if 0
		// instead of choosing to reduce the resolution, instead we will choose the most recent range at full resolution. 
		labels = GetTimeSeries(end - (MaxNumSamples * resolutionSeconds), end, resolutionSeconds);  // 'most recent data' with maximum resolution. 

#else
		if ((numSamples * _learnHourDelta) < MaxNumSamples) {
			auto prevInterp = this->GetInterpolationType();
			this->SetInterpolationType(interpolation_t::IT_LINEAR);
			labels = GetTimeSeries(start, end, resolutionSeconds / _learnHourDelta);
			this->SetInterpolationType(prevInterp);
		}
		else if (((numSamples * _learnHourDelta) / 24.0f) < MaxNumSamples) {
			auto prevInterp = this->GetInterpolationType();
			this->SetInterpolationType(interpolation_t::IT_LINEAR);
			labels = GetTimeSeries(start, end, (resolutionSeconds / _learnHourDelta) * 24);
			this->SetInterpolationType(prevInterp);
		}
		else {
			auto prevInterp = this->GetInterpolationType();
			this->SetInterpolationType(interpolation_t::IT_LINEAR);
			labels = GetTimeSeries(start, end, (end - start) / MaxNumSamples);
			this->SetInterpolationType(prevInterp);
		}
#endif
	}
	else {
		auto prevInterp = this->GetInterpolationType();
		this->SetInterpolationType(interpolation_t::IT_LINEAR);
		labels = GetTimeSeries(start, end, resolutionSeconds);  // great!
		this->SetInterpolationType(prevInterp);
	}

	if (0) {
		fileSystem->submitToast("Machine Learning " + this->GetName(), cweeStr::printf("Targetting %i samples, using %i samples.", (int)numSamples, labels.Num()));
	}

	// develop exclusion list as we go.	
	cweeThreadedList<bool> exclusionList;
	
	this->Lock();
	cweeThreadedList<vec4> MachineLearnFeatures = machineLearn_features;
	this->Unlock();

	// user provided features
	for (auto& x : MachineLearnFeatures) {
		switch (static_cast<patternSource>((int)x.x)) {
		case patternSource::Parent: {
			if (Parent != nullptr) {
				cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
				std::pair<u64, type> tempHold;
				Parent->PreventDeletion(x.y);
				Parent->Lock();
				cweePattern_CatmullRomSpline<type>* ptr = Parent->UnsafeRead(x.y);
				Parent->Unlock();
				if (ptr) {
					for (std::pair<u64, type>& y : labels) {
						tempHold.first = y.first; tempHold.second = (type)(ptr->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z), Parent));
						feature1.Append(tempHold);
					}
				}

				Parent->AllowDeletion(x.y);
				//bool ex = DetermineIfExcludeFeature(feature1);
				//if (!ex) 
				if (feature1.Num() > 0)
				features.Append(feature1);
			}
			break;
		}
		case patternSource::Global: {
			cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
			std::pair<u64, type> tempHold;
			Globals->PreventDeletion(x.y);
			Globals->Lock();
			auto ptr = Globals->UnsafeRead(x.y);
			Globals->Unlock();
			if (ptr) {
				for (std::pair<u64, type>& y : labels) {
					tempHold.first = y.first; tempHold.second = (type)(ptr->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z), Parent));
					feature1.Append(tempHold);
				}
			}
			Globals->AllowDeletion(x.y);
			//bool ex = DetermineIfExcludeFeature(feature1);
			//if (!ex) 
			if (feature1.Num() > 0)
			features.Append(feature1);
			break;
		}
		case patternSource::Scada: {
			cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
			std::pair<u64, type> tempHold;
			Scada->PreventDeletion(x.y);
			Scada->Lock();
			auto ptr = Scada->UnsafeRead(x.y);
			Scada->Unlock();
			if (ptr) {
				for (std::pair<u64, type>& y : labels) {
					tempHold.first = y.first; tempHold.second = (type)(ptr->GetMeasurement("value")->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z), Parent));
					feature1.Append(tempHold);
				}
			}
			Scada->AllowDeletion(x.y);
			//bool ex = DetermineIfExcludeFeature(feature1);
			//if (!ex) 
			if (feature1.Num() > 0)
			features.Append(feature1);
			break;
		}
		case patternSource::Customers: {
			cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
			std::pair<u64, type> tempHold;
			Customers->Lock();
			auto ptr = Customers->UnsafeRead(x.y);
			if (ptr) {
				for (std::pair<u64, type>& y : labels) {
					tempHold.first = y.first; tempHold.second = (type)(ptr->GetMeasurement("usage_gpm")->GetTransformedCurrentValue(y.first - x.w, static_cast<patternModifier>((int)x.z), Parent));
					feature1.Append(tempHold);
				}
			}
			Customers->Unlock();
			//bool ex = DetermineIfExcludeFeature(feature1);
			//if (!ex) 
			if (feature1.Num() > 0)
			features.Append(feature1);
			break;
		}
		}
	}

	// more complicated but corrected regression design based on M&V diff-diff concept
	if (MachineLearnFeatures.Num() <= 0 || features.Num() <= 0) {
		{
			// Hours
			for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
				std::vector < std::pair<u64, type>> feature1;
				for (auto& x : labels) {
					hr = this->GetHourOfDay(x.first);
					//cweeStr tempp;
					//tempp = cweeStr((time_t)x.first);
					int v = (int)(hr >= t_s && hr < (t_s + _learnHourDelta));
					std::pair<u64, type> t = std::make_pair(x.first, (type)(v));
					feature1.push_back(t);
				}
				features.Append(feature1);
			}
		}
		{
			// Day of Week
			bool excludeDays = false;
			{
				cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
				for (auto& x : labels) feature1.Append(std::make_pair(x.first, this->GetDayOfWeek(x.first)));
				excludeDays = DetermineIfExcludeFeature(feature1);
				exclusionList.Append(excludeDays);
			}

			// 0 automatically accounted for as the 'default'
			// 0
			if (!excludeDays) {
				for (int j = 0; j < 7; j++) {
					for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
						std::vector < std::pair<u64, type>> feature1;
						for (auto& x : labels) {
							hr = this->GetHourOfDay(x.first);
							//cweeStr tempp;
							//tempp = cweeStr((time_t)x.first);
							int v = (int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (this->GetDayOfWeek(x.first) == j));
							std::pair<u64, type> t = std::make_pair(x.first, (type)(v));
							feature1.push_back(t);
						}
						features.Append(feature1);
					}
				}
			}
		}
		{
			// Quarter of the Year
			bool excludeQuarters = false;
			{
				cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
				for (auto& x : labels) feature1.Append(std::make_pair(x.first,
					cweeMath::Floor(((float)this->GetMonthOfYear(x.first)) / 3.0f)
				));
				excludeQuarters = DetermineIfExcludeFeature(feature1);
				exclusionList.Append(excludeQuarters);
			}
			if (!excludeQuarters) {
				for (int j = 0; j < 4; j++) {
					cweeThreadedList < std::pair<u64, type>> feature1(labels.Num());
					for (auto& x : labels) {
						if (cweeMath::Floor(((float)this->GetMonthOfYear(x.first)) / 3.0f) == j)
							feature1.Append(std::make_pair(x.first, (type)1));
						else
							feature1.Append(std::make_pair(x.first, (type)0));
					}
					features.Append(feature1);
				}
			}
		}
	}

	// perform learning
	cweeML_learned_parameters attempt1; std::pair<vec2, vec2> fit1;
	attempt1 = cweeMachineLearning::Learn(labels, features, &fit1, splitSample); // actual learn activity - note that this is done async to actually accessing the pattern itself.
	attempt1.performance.x = fit1.first.x;
	attempt1.performance.y = fit1.first.y;
	attempt1.performance.z = fit1.second.x;
	attempt1.performance.w = fit1.second.y;
	attempt1.learnPeriod.x = start;
	attempt1.learnPeriod.y = end;
	attempt1.learnPeriod.z = _learnHourDelta;

	float mT = (float)this->GetMinimumDecimals();
	this->Lock(); {
		machineLearn_roundNearest = mT;		
	} this->Unlock();

	mT = (float)this->GetMinValue();	
	this->Lock(); {
		machineLearn_min = mT;
	} this->Unlock();

	mT = (float)this->GetMaxValue();	
	this->Lock(); {
		machineLearn_max = mT;
	} this->Unlock();
	
	this->Lock();
	if (splitSample > 0) {
		if (learned) {
			if (attempt1.performance.w > learned->performance.y) { // Mean Squared Error is larger 		
				vec2 err = cweeMachineLearning::CalculateError(labels, cweeMachineLearning::Forecast(*learned, features, machineLearn_roundNearest)); // adjusted previous error
				if (attempt1.performance.w > err.y) { // the older learned object is better
					this->Unlock();
					return vec2(fit1.first.x, fit1.second.x); // return early - this was not approved for use. 
				}
			}
		}
	}
	else {
		if (learned) {
			if (attempt1.performance.y > learned->performance.y) { // Mean Squared Error is larger 		
				vec2 err = cweeMachineLearning::CalculateError(labels, cweeMachineLearning::Forecast(*learned, features, machineLearn_roundNearest)); // adjusted previous error
				if (attempt1.performance.y > err.y) { // the older learned object is better
					this->Unlock();
					return vec2(fit1.first.x, fit1.second.x); // return early - this was not approved for use. 
				}
			}
		}
	}
	this->Unlock();

	if (learnWithExpansion) {
		// SaveLearn(this, numSamples, labels, features, attempt1.performance);
		// add 'column' with the input data
		if (0) {
			auto timeNow = cweeStr(fileSystem->getCurrentTime());
			//		Convert labels into a CSV document that can be appended to with each re-learn.  

			/*
			TIME (INT)		TIME (STRING)		LABELS_0		LABELS_1		LABELS_2
			16000000		Jan 1 2021			15				""				""
			16000000		Jan 2 2021			15				15				16
			16000000		Jan 3 2021			""				16				16
			16000000		Jan 4 2021			15				15				17
			...
			*/

			cweeStr rowDelim = "\r"; cweeStr colDelim = ",";
			cweeStr fileName = "RelearnPattern_" + this->GetName();
			cweeStr filePath = fileSystem->createFilePath(fileSystem->getDataFolder(), fileName, CSV);
			cweeStr prevContent; fileSystem->readFileAsCweeStr(prevContent, filePath);

			cweeThreadedList < cweeStr > headers;
			cweeThreadedList < cweeThreadedList< std::pair<u64, type> > > labelHistory;
			{
				cweeParser a(prevContent, rowDelim, true);
				cweeParser b;
				for (int j = 0; j < a.getNumVars(); j++) {
					auto& row = a[j];
					b.Parse(row, colDelim, true);
					if (j == 0) {
						for (int i = 2; i < b.getNumVars(); i++)
							headers.Append(b[i]);
					}
					else {
						std::pair<u64, type> v;
						for (int i = 0; i < b.getNumVars(); i++) {
							switch (i) {
							case 0:
								v.first = (float)b[i];
								break;
							case 1:
								// string version of t -- ignored.
								break;
							default:
								// column representing a label.
							{
								int colNum = i - 2; // 0, 1, 2
								while (labelHistory.Num() <= colNum) labelHistory.Append(cweeThreadedList< std::pair<u64, type> >());
								if (!b[i].IsEmpty()) {
									v.second = (float)b[i];
									labelHistory[colNum].Append(v);
								}
							}
							break;
							}
						}
					}
				}
			}

			// add new 'column' to label history
			labelHistory.Append(labels);

			// new file content
			cweeStr newContent;
			{
				// header
				{
					cweeStr header;
					header.AddToDelimiter("TIME (int)", colDelim);
					header.AddToDelimiter("TIME (text)", colDelim);
					for (auto& x : headers)
						header.AddToDelimiter(x, colDelim);

					header.AddToDelimiter(timeNow + cweeStr::printf(" (%i samples)", (int)numSamples), colDelim);
					newContent.AddToDelimiter(header, rowDelim);
				}

				// ensure same number of rows throughout
				cweeThreadedList < cweeCurve<cweeStr> > out;
				for (int i = 0; i < labelHistory.Num(); i++) {
					out.Append(cweeCurve<cweeStr>());
					for (auto& x : labelHistory[i]) {
						out[i].AddUniqueValue(x.first, cweeStr((float)x.second));
					}
				}
				for (int i = 0; i < out.Num(); i++) {
					for (int j = 0; j < out.Num(); j++) {
						if (j == i) continue;

						auto& a = out[i];
						auto& b = out[j];

						for (auto& knot : a.GetKnotSeries()) {
							if (b.FindExactX(knot.first) < 0) {
								// value does not exist. 
								b.AddUniqueValue(knot.first, "");
							}
						}
					}
				}

				//// by this point, there should be an exact number of points in all out[] curves. 
				//int numRows = 0;
				//for (int i = 0; i < out.Num(); i++) {
				//	for (int j = 0; j < out.Num(); j++) {
				//		if (out[i].GetNumValues() != out[j].GetNumValues()) {
				//			fileSystem->submitToast("EDMS Warning", "Number of curve points do not match as Pattern.cpp, line 155.");
				//		}
				//		numRows = out[i].GetNumValues();
				//	}
				//}

				// Write to new file content
				for (auto& t : out[0].GetKnotSeries()) {
					cweeStr row;
					row.AddToDelimiter(t.first, colDelim);
					row.AddToDelimiter(cweeStr((time_t)t.first), colDelim);

					for (auto& label : out) {
						row.AddToDelimiter(label.GetCurrentValue(t.first), colDelim);
					}
					newContent.AddToDelimiter(row, rowDelim);
				}
			}

			// save to OS
			fileSystem->writeFileFromCweeStr(filePath, newContent);
		}

		// add 'column' with the output/forecast data
		if (0) {
			auto timeNow = cweeStr(fileSystem->getCurrentTime());
			//		Convert labels into a CSV document that can be appended to with each re-learn.  

			/*
			TIME (INT)		TIME (STRING)		LABELS_0		LABELS_1		LABELS_2
			16000000		Jan 1 2021			15				""				""
			16000000		Jan 2 2021			15				15				16
			16000000		Jan 3 2021			""				16				16
			16000000		Jan 4 2021			15				15				17
			...
			*/

			cweeStr rowDelim = "\r"; cweeStr colDelim = ",";
			cweeStr fileName = "RelearnPattern_" + this->GetName();
			cweeStr filePath = fileSystem->createFilePath(fileSystem->getDataFolder(), fileName, CSV);
			cweeStr prevContent; fileSystem->readFileAsCweeStr(prevContent, filePath);

			cweeThreadedList < cweeStr > headers;
			cweeThreadedList < cweeThreadedList< std::pair<u64, type> > > labelHistory;
			{
				cweeParser a(prevContent, rowDelim, true);
				cweeParser b;
				for (int j = 0; j < a.getNumVars(); j++) {
					auto& row = a[j];
					b.Parse(row, colDelim, true);
					if (j == 0) {
						for (int i = 2; i < b.getNumVars(); i++)
							headers.Append(b[i]);
					}
					else {
						std::pair<u64, type> v;
						for (int i = 0; i < b.getNumVars(); i++) {
							switch (i) {
							case 0:
								v.first = (float)b[i];
								break;
							case 1:
								// string version of t -- ignored.
								break;
							default:
								// column representing a label.
							{
								int colNum = i - 2; // 0, 1, 2
								while (labelHistory.Num() <= colNum) labelHistory.Append(cweeThreadedList< std::pair<u64, type> >());
								if (!b[i].IsEmpty()) {
									v.second = (float)b[i];
									labelHistory[colNum].Append(v);
								}
							}
							break;
							}
						}
					}
				}
			}

			// add new 'column' to label history
			this->Lock();
			labelHistory.Append(cweeMachineLearning::Forecast(attempt1, features, machineLearn_roundNearest));
			this->Unlock();

			// new file content
			cweeStr newContent;
			{
				// header
				{
					cweeStr header;
					header.AddToDelimiter("TIME (int)", colDelim);
					header.AddToDelimiter("TIME (text)", colDelim);
					for (auto& x : headers)
						header.AddToDelimiter(x, colDelim);

					header.AddToDelimiter(cweeStr::printf("FORECAST (%f | %f | %f | %f)", attempt1.performance.x, attempt1.performance.y, attempt1.performance.z, attempt1.performance.w), colDelim);
					newContent.AddToDelimiter(header, rowDelim);
				}

				// ensure same number of rows throughout
				cweeThreadedList < cweeCurve<cweeStr> > out;
				for (int i = 0; i < labelHistory.Num(); i++) {
					out.Append(cweeCurve<cweeStr>());
					for (auto& x : labelHistory[i]) {
						out[i].AddUniqueValue(x.first, cweeStr((float)x.second));
					}
				}
				for (int i = 0; i < out.Num(); i++) {
					for (int j = 0; j < out.Num(); j++) {
						if (j == i) continue;

						auto& a = out[i];
						auto& b = out[j];

						for (auto& knot : a.GetKnotSeries()) {
							if (b.FindExactX(knot.first) < 0) {
								// value does not exist. 
								b.AddUniqueValue(knot.first, "");
							}
						}
					}
				}

				//// by this point, there should be an exact number of points in all out[] curves. 
				//int numRows = 0;
				//for (int i = 0; i < out.Num(); i++) {
				//	for (int j = 0; j < out.Num(); j++) {
				//		if (out[i].GetNumValues() != out[j].GetNumValues()) {
				//			fileSystem->submitToast("EDMS Warning", "Number of curve points do not match as Pattern.cpp, line 155.");
				//		}
				//		numRows = out[i].GetNumValues();
				//	}
				//}

				// Write to new file content
				for (auto& t : out[0].GetKnotSeries()) {
					cweeStr row;
					row.AddToDelimiter(t.first, colDelim);
					row.AddToDelimiter(cweeStr((time_t)t.first), colDelim);

					for (auto& label : out) {
						row.AddToDelimiter(label.GetCurrentValue(t.first), colDelim);
					}
					newContent.AddToDelimiter(row, rowDelim);
				}
			}

			// save to OS
			fileSystem->writeFileFromCweeStr(filePath, newContent);
		}

		// serialize, deserialize, then forecast 1
		if (0) {
			auto timeNow = cweeStr(fileSystem->getCurrentTime());
			//		Convert labels into a CSV document that can be appended to with each re-learn.  

			/*
			TIME (INT)		TIME (STRING)		LABELS_0		LABELS_1		LABELS_2
			16000000		Jan 1 2021			15				""				""
			16000000		Jan 2 2021			15				15				16
			16000000		Jan 3 2021			""				16				16
			16000000		Jan 4 2021			15				15				17
			...
			*/

			cweeStr rowDelim = "\r"; cweeStr colDelim = ",";
			cweeStr fileName = "RelearnPattern_" + this->GetName();
			cweeStr filePath = fileSystem->createFilePath(fileSystem->getDataFolder(), fileName, CSV);
			cweeStr prevContent; fileSystem->readFileAsCweeStr(prevContent, filePath);

			cweeThreadedList < cweeStr > headers;
			cweeThreadedList < cweeThreadedList< std::pair<u64, type> > > labelHistory;
			{
				cweeParser a(prevContent, rowDelim, true);
				cweeParser b;
				for (int j = 0; j < a.getNumVars(); j++) {
					auto& row = a[j];
					b.Parse(row, colDelim, true);
					if (j == 0) {
						for (int i = 2; i < b.getNumVars(); i++)
							headers.Append(b[i]);
					}
					else {
						std::pair<u64, type> v;
						for (int i = 0; i < b.getNumVars(); i++) {
							switch (i) {
							case 0:
								v.first = (float)b[i];
								break;
							case 1:
								// string version of t -- ignored.
								break;
							default:
								// column representing a label.
							{
								int colNum = i - 2; // 0, 1, 2
								while (labelHistory.Num() <= colNum) labelHistory.Append(cweeThreadedList< std::pair<u64, type> >());
								if (!b[i].IsEmpty()) {
									v.second = (float)b[i];
									labelHistory[colNum].Append(v);
								}
							}
							break;
							}
						}
					}
				}
			}

			// add new 'column' to label history
			int numChar = 0;
			cweeML_learned_parameters attempt2;
			// attempt2 = attempt1;

			{
				if (1) {
					cweeStr ser = attempt1.Serialize();
					numChar = ser.Length();
					attempt2.Deserialize(ser);
				}
				if (0) {
					typedef dlib::matrix<float, 0, 1>								sample_type;
					typedef dlib::radial_basis_kernel<sample_type>					kernel_type;

					std::stringstream of1(std::stringstream::out | std::stringstream::binary);
					//std::stringstream of2(std::stringstream::out | std::stringstream::binary);
					//std::stringstream of3(std::stringstream::out | std::stringstream::binary);
					std::stringstream of4(std::stringstream::out | std::stringstream::binary);

					dlib::serialize(attempt1.svr_param.df2.alpha, of1);
					cweeStr ser_B = cweeStr(attempt1.svr_param.df2.b); // dlib::serialize(attempt1.svr_param.df2.b, of2);
					cweeStr ser_Gamma = cweeStr(attempt1.svr_param.df2.kernel_function.gamma); // dlib::serialize(attempt1.svr_param.df2.kernel_function.gamma, of3);
					dlib::serialize(attempt1.svr_param.df2.basis_vectors, of4);

					std::stringstream iF1(std::stringstream::in | std::stringstream::binary); iF1.str(of1.str());
					//std::stringstream iF2(std::stringstream::in | std::stringstream::binary); iF2.str(of2.str());
					//std::stringstream iF3(std::stringstream::in | std::stringstream::binary); iF3.str(of3.str());
					std::stringstream iF4(std::stringstream::in | std::stringstream::binary); iF4.str(of4.str());

					dlib::deserialize(attempt2.svr_param.df2.alpha, iF1);
					attempt2.svr_param.df2.b = (float)ser_B; // dlib::deserialize(attempt2.svr_param.df2.b, iF2);
					attempt2.svr_param.df2.kernel_function = kernel_type((float)ser_Gamma); // dlib::deserialize(attempt2.svr_param.df2.kernel_function.gamma, iF3);
					dlib::deserialize(attempt2.svr_param.df2.basis_vectors, iF4);
				}
			}

			this->Lock();
			labelHistory.Append(cweeMachineLearning::Forecast(attempt2, features, machineLearn_roundNearest));
			this->Unlock();

			// new file content
			cweeStr newContent;
			{
				// header
				{
					cweeStr header;
					header.AddToDelimiter("TIME (int)", colDelim);
					header.AddToDelimiter("TIME (text)", colDelim);
					for (auto& x : headers)
						header.AddToDelimiter(x, colDelim);

					header.AddToDelimiter(cweeStr::printf("SER/DES TEST FORECAST (CWEE: %i char)", numChar), colDelim);
					newContent.AddToDelimiter(header, rowDelim);
				}

				// ensure same number of rows throughout
				cweeThreadedList < cweeCurve<cweeStr> > out;
				for (int i = 0; i < labelHistory.Num(); i++) {
					out.Append(cweeCurve<cweeStr>());
					for (auto& x : labelHistory[i]) {
						out[i].AddUniqueValue(x.first, cweeStr((float)x.second));
					}
				}
				for (int i = 0; i < out.Num(); i++) {
					for (int j = 0; j < out.Num(); j++) {
						if (j == i) continue;

						auto& a = out[i];
						auto& b = out[j];

						for (auto& knot : a.GetKnotSeries()) {
							if (b.FindExactX(knot.first) < 0) {
								// value does not exist. 
								b.AddUniqueValue(knot.first, "");
							}
						}
					}
				}

				//// by this point, there should be an exact number of points in all out[] curves. 
				//int numRows = 0;
				//for (int i = 0; i < out.Num(); i++) {
				//	for (int j = 0; j < out.Num(); j++) {
				//		if (out[i].GetNumValues() != out[j].GetNumValues()) {
				//			fileSystem->submitToast("EDMS Warning", "Number of curve points do not match as Pattern.cpp, line 155.");
				//		}
				//		numRows = out[i].GetNumValues();
				//	}
				//}

				// Write to new file content
				for (auto& t : out[0].GetKnotSeries()) {
					cweeStr row;
					row.AddToDelimiter(t.first, colDelim);
					row.AddToDelimiter(cweeStr((time_t)t.first), colDelim);

					for (auto& label : out) {
						row.AddToDelimiter(label.GetCurrentValue(t.first), colDelim);
					}
					newContent.AddToDelimiter(row, rowDelim);
				}
			}

			// save to OS
			fileSystem->writeFileFromCweeStr(filePath, newContent);
		}

		// serialize, deserialize, then forecast 2
		if (0) {
			auto timeNow = cweeStr(fileSystem->getCurrentTime());
			//		Convert labels into a CSV document that can be appended to with each re-learn.  

			/*
			TIME (INT)		TIME (STRING)		LABELS_0		LABELS_1		LABELS_2
			16000000		Jan 1 2021			15				""				""
			16000000		Jan 2 2021			15				15				16
			16000000		Jan 3 2021			""				16				16
			16000000		Jan 4 2021			15				15				17
			...
			*/

			cweeStr rowDelim = "\r"; cweeStr colDelim = ",";
			cweeStr fileName = "RelearnPattern_" + this->GetName();
			cweeStr filePath = fileSystem->createFilePath(fileSystem->getDataFolder(), fileName, CSV);
			cweeStr prevContent; fileSystem->readFileAsCweeStr(prevContent, filePath);

			cweeThreadedList < cweeStr > headers;
			cweeThreadedList < cweeThreadedList< std::pair<u64, type> > > labelHistory;
			{
				cweeParser a(prevContent, rowDelim, true);
				cweeParser b;
				for (int j = 0; j < a.getNumVars(); j++) {
					auto& row = a[j];
					b.Parse(row, colDelim, true);
					if (j == 0) {
						for (int i = 2; i < b.getNumVars(); i++)
							headers.Append(b[i]);
					}
					else {
						std::pair<u64, type> v;
						for (int i = 0; i < b.getNumVars(); i++) {
							switch (i) {
							case 0:
								v.first = (float)b[i];
								break;
							case 1:
								// string version of t -- ignored.
								break;
							default:
								// column representing a label.
							{
								int colNum = i - 2; // 0, 1, 2
								while (labelHistory.Num() <= colNum) labelHistory.Append(cweeThreadedList< std::pair<u64, type> >());
								if (!b[i].IsEmpty()) {
									v.second = (float)b[i];
									labelHistory[colNum].Append(v);
								}
							}
							break;
							}
						}
					}
				}
			}

			// add new 'column' to label history
			cweeML_learned_parameters attempt2; 
			if (0) { // move all data except for the df2
				attempt2.learned = attempt1.learned;
				attempt2.learnedAdjustment = attempt1.learnedAdjustment;
				attempt2.learnPeriod = attempt1.learnPeriod;
				attempt2.method_name = attempt1.method_name;
				attempt2.performance = attempt1.performance;
			}

			int numChar = 0;
			if (1) {
				// test dlib serialization
				//cweeStr_ostringstream out;
				
				if (0) {
					// this works.
					cweeStr filepath = fileSystem->createFilePath(fileSystem->getDataFolder(), "mlSave" + cweeStr(cweeRandomInt(100, 10000)), TXT);
					dlib::serialize(filepath.c_str()) << attempt1.svr_param.df2;
					dlib::deserialize(filepath.c_str()) >> attempt2.svr_param.df2;
				}
				if (0) {
					// this appears to work but produces the exact same result as my serialization ... 
					std::stringstream of(std::stringstream::out | std::stringstream::binary);
					dlib::serialize(attempt1.svr_param.df2.alpha, of);
					dlib::serialize(attempt1.svr_param.df2.b, of);
					dlib::serialize(attempt1.svr_param.df2.basis_vectors, of);

					
					std::stringstream iF(std::stringstream::in | std::stringstream::binary);
					iF.str(of.str());

					dlib::deserialize(attempt2.svr_param.df2.alpha, iF);
					dlib::deserialize(attempt2.svr_param.df2.b, iF);
					dlib::deserialize(attempt2.svr_param.df2.basis_vectors, iF);
				}
				if (0) {
					// this works.
					std::stringstream of(std::stringstream::out | std::stringstream::binary);
					dlib::serialize(attempt1.svr_param.df2, of);

					std::stringstream iF(std::stringstream::in | std::stringstream::binary);
					iF.str(of.str());

					dlib::deserialize(attempt2.svr_param.df2, iF);
				}
				if (1) {
					std::stringstream of1(std::stringstream::out | std::stringstream::binary);
					std::stringstream of2(std::stringstream::out | std::stringstream::binary);
					std::stringstream of3(std::stringstream::out | std::stringstream::binary);
					std::stringstream of4(std::stringstream::out | std::stringstream::binary);

					dlib::serialize(attempt1.svr_param.df2.alpha, of1);
					dlib::serialize(attempt1.svr_param.df2.b, of2);
					dlib::serialize(attempt1.svr_param.df2.kernel_function, of3);
					dlib::serialize(attempt1.svr_param.df2.basis_vectors, of4);		

					std::stringstream iF1(std::stringstream::in | std::stringstream::binary); iF1.str(of1.str());
					std::stringstream iF2(std::stringstream::in | std::stringstream::binary); iF2.str(of2.str());
					std::stringstream iF3(std::stringstream::in | std::stringstream::binary); iF3.str(of3.str());
					std::stringstream iF4(std::stringstream::in | std::stringstream::binary); iF4.str(of4.str());

					dlib::deserialize(attempt2.svr_param.df2.alpha, iF1);
					dlib::deserialize(attempt2.svr_param.df2.b, iF2);
					dlib::deserialize(attempt2.svr_param.df2.kernel_function, iF3);
					dlib::deserialize(attempt2.svr_param.df2.basis_vectors, iF4);
				}				
			}

			this->Lock();
			labelHistory.Append(cweeMachineLearning::Forecast(attempt2, features, machineLearn_roundNearest));
			this->Unlock();

			// new file content
			cweeStr newContent;
			{
				// header
				{
					cweeStr header;
					header.AddToDelimiter("TIME (int)", colDelim);
					header.AddToDelimiter("TIME (text)", colDelim);
					for (auto& x : headers)
						header.AddToDelimiter(x, colDelim);

					header.AddToDelimiter(cweeStr::printf("SER/DES TEST FORECAST (Dlib: %i char)", numChar), colDelim);
					newContent.AddToDelimiter(header, rowDelim);
				}

				// ensure same number of rows throughout
				cweeThreadedList < cweeCurve<cweeStr> > out;
				for (int i = 0; i < labelHistory.Num(); i++) {
					out.Append(cweeCurve<cweeStr>());
					for (auto& x : labelHistory[i]) {
						out[i].AddUniqueValue(x.first, cweeStr((float)x.second));
					}
				}
				for (int i = 0; i < out.Num(); i++) {
					for (int j = 0; j < out.Num(); j++) {
						if (j == i) continue;

						auto& a = out[i];
						auto& b = out[j];

						for (auto& knot : a.GetKnotSeries()) {
							if (b.FindExactX(knot.first) < 0) {
								// value does not exist. 
								b.AddUniqueValue(knot.first, "");
							}
						}
					}
				}

				//// by this point, there should be an exact number of points in all out[] curves. 
				//int numRows = 0;
				//for (int i = 0; i < out.Num(); i++) {
				//	for (int j = 0; j < out.Num(); j++) {
				//		if (out[i].GetNumValues() != out[j].GetNumValues()) {
				//			fileSystem->submitToast("EDMS Warning", "Number of curve points do not match as Pattern.cpp, line 155.");
				//		}
				//		numRows = out[i].GetNumValues();
				//	}
				//}

				// Write to new file content
				for (auto& t : out[0].GetKnotSeries()) {
					cweeStr row;
					row.AddToDelimiter(t.first, colDelim);
					row.AddToDelimiter(cweeStr((time_t)t.first), colDelim);

					for (auto& label : out) {
						row.AddToDelimiter(label.GetCurrentValue(t.first), colDelim);
					}
					newContent.AddToDelimiter(row, rowDelim);
				}
			}

			// save to OS
			fileSystem->writeFileFromCweeStr(filePath, newContent);
		}
	}

	this->Lock(); {
		if (learned) *learned = attempt1; // save results	
		exclusion_list = exclusionList;
		machineLearn_MAD = vec2(1, 0); // reset this! We just finished learning and this shouldn't be necessary anymore.
	} this->Unlock();
	return vec2(fit1.first.x, fit1.second.x); // return the fit
};

template< class type >
type cweePattern_CatmullRomSpline<type>::Forecast_Value_Automated(const u64& time, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent) const {
	float hr; int exclusionCheck = 0;

	u64 _learnHourDelta = learnHourDelta;
	this->Lock();
	if (learned && learned->learned && learned->learnPeriod.z > 0) {
		_learnHourDelta = ::Max(_learnHourDelta, (u64)learned->learnPeriod.z);
		this->Unlock();		
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}
	else if (learned && learned->learned && learned->learnPeriod.z <= 0) {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
	}
	else {
		this->Unlock();
		u64 minTimestep = this->GetMinimumTimeStep();
		_learnHourDelta = learnHourDelta;
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}

	cweeThreadedList < float > features(cweeMath::max(16, (int)(learnHourMaxTime / _learnHourDelta))); {
		this->Lock();
		cweeThreadedList<vec4> MachineLearnFeatures = machineLearn_features;
		this->Unlock();

		// more complicated but corrected regression design based on M&V diff-diff concept
		if (MachineLearnFeatures.Num() <= 0) {
			if (1) { // Hours
				hr = GetHourOfDay(time);
				for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					features.Append((int)(hr >= t_s && hr < (t_s + _learnHourDelta)));
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) {
				// Day of Week
				for (int j = 0; j < 7; j++) {
					for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
						features.Append((int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (GetDayOfWeek(time) == j)));
					}


					//features.Append((int)(GetDayOfWeek(time) == j));
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) {
				// Quarter of the Year
				for (int j = 0; j < 4; j++) {
					//for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					//	features.Append((int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (cweeMath::Floor(((float)this->GetMonthOfYear(time)) / 3.0f) == j)));
					//}


					features.Append((int)(cweeMath::Floor(((float)this->GetMonthOfYear(time)) / 3.0f) == j));
				}
			}
		}

		{
			float ts(0);
			for (auto& x : MachineLearnFeatures) {
				//if (!DetermineIfExcludeFeature(exclusionCheck)) {
					switch (static_cast<patternSource>((int)x.x)) {
					case patternSource::Parent: {
						if (Parent != nullptr) {
							Parent->PreventDeletion(x.y);
							Parent->Lock();
							auto ptr = Parent->UnsafeRead(x.y);
							Parent->Unlock(); 
							if (ptr) {
								ts = (float)ptr->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z), Parent);
							}							
							Parent->AllowDeletion(x.y);
							features.Append(ts);
						}
						break;
					}
					case patternSource::Global: {
						Globals->PreventDeletion(x.y);
						Globals->Lock();
						auto ptr = Globals->UnsafeRead(x.y);
						Globals->Unlock();
						if (ptr) {
							ts = ((float)ptr->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z), Parent));
						}
						Globals->AllowDeletion(x.y);

						features.Append(ts);
						break;
					}
					case patternSource::Scada: {
						Scada->PreventDeletion(x.y);
						Scada->Lock();
						auto ptr = Scada->UnsafeRead(x.y);
						Scada->Unlock();
						if (ptr) {
							ts = ((float)ptr->GetMeasurement("value")->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z), Parent));
						}
						Scada->AllowDeletion(x.y);

						features.Append(ts);
						break;
					}
					case patternSource::Customers: {
						Customers->Lock();
						auto ptr = Customers->UnsafeRead(x.y);
						if (ptr) {
							ts = ((float)ptr->GetMeasurement("usage_gpm")->GetTransformedCurrentValue(time - x.w, static_cast<patternModifier>((int)x.z), Parent));
						}
						Customers->Unlock();
						features.Append(ts);
						break;
					}
					}
				//}
			}
		}

	}

	this->Lock();
	type out;
	out = 0.0f;
	if (learned) {
		out = (type)cweeMachineLearning::Forecast(*learned, features, machineLearn_roundNearest);
	}
	out = (type)cweeMath::Fmax(cweeMath::Fmin((float)out, machineLearn_max), machineLearn_min);
	if (machineLearn_MAD != vec2(1, 0)) { out *= (type)machineLearn_MAD[0]; out += (type)machineLearn_MAD[1]; }
	this->Unlock();
	return out;
};

template< class type >
cweeThreadedList< std::pair<u64, type> > cweePattern_CatmullRomSpline<type>::Forecast_Series_Automated(const u64& timeStart, const u64& timeEnd, const u64& timeStep, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent) const {
	float hr;  int exclusionCheck = 0; 
	u64 realStep = cweeMath::Fmax(1, timeStep);

	u64 _learnHourDelta = learnHourDelta;
	this->Lock();
	if (learned && learned->learned && learned->learnPeriod.z > 0) {
		_learnHourDelta = ::Max(_learnHourDelta, (u64)learned->learnPeriod.z);
		this->Unlock();
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}
	else if (learned && learned->learned && learned->learnPeriod.z <= 0) {
		this->Unlock();
		_learnHourDelta = learnHourDelta;
	}
	else {
		this->Unlock();
		u64 minTimestep = this->GetMinimumTimeStep();
		_learnHourDelta = learnHourDelta;
		_learnHourDelta = ::Max(_learnHourDelta, minTimestep);
		_learnHourDelta = cweeMath::roundNearest(_learnHourDelta, learnHourDelta);
		_learnHourDelta = ::Max(_learnHourDelta, learnHourDelta);
	}

	cweeThreadedList< std::pair<u64, type> > feature1(((timeEnd - timeStart) / realStep));
	cweeThreadedList < cweeThreadedList< std::pair<u64, type> > > features; {	
		this->Lock();
		cweeThreadedList<vec4> MachineLearnFeatures = machineLearn_features;
		this->Unlock();


		// more complicated but corrected regression design based on M&V diff-diff concept
		if (MachineLearnFeatures.Num() <= 0) {
			if (1) { // Hours
				for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					feature1.Clear();
					for (u64 t = timeStart; t < timeEnd; t += realStep) {
						hr = GetHourOfDay(t);
						feature1.Append(std::make_pair(t, (type)(
							(int)(hr >= t_s && hr < (t_s + _learnHourDelta))
							)));
					}
					features.Append(feature1);
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) { // Day of Week
				for (int j = 0; j < 7; j++) {
					for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
						feature1.Clear();
						for (u64 t = timeStart; t < timeEnd; t += realStep) {
							hr = GetHourOfDay(t);
							feature1.Append(std::make_pair(t, (type)(
								(int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (GetDayOfWeek(t) == j))
								)));
						}
						features.Append(feature1);
					}

					//feature1.Clear();
					//for (u64 t = timeStart; t < timeEnd; t += realStep)
					//	feature1.Append(std::make_pair(t, (type)(
					//		(int)(GetDayOfWeek(t) == j)
					//		)));
					//features.Append(feature1);
				}
			}
			if (!DetermineIfExcludeFeature(exclusionCheck)) { // Quarter of the Year
				// 0 automatically accounted for as the 'default'
				// 0
				for (int j = 0; j < 4; j++) {
					//for (u64 t_s = 0; t_s < learnHourMaxTime; t_s += _learnHourDelta) {
					//	feature1.Clear();
					//	for (u64 t = timeStart; t < timeEnd; t += realStep) {
					//		hr = GetHourOfDay(t);
					//		feature1.Append(std::make_pair(t, (type)(
					//			(int)((hr >= t_s && hr < (t_s + _learnHourDelta)) && (cweeMath::Floor(((float)this->GetMonthOfYear(t)) / 3.0f) == j))
					//			)));
					//	}
					//	features.Append(feature1);
					//}

					feature1.Clear();
					for (u64 t = timeStart; t < timeEnd; t += realStep)
						feature1.Append(std::make_pair(t, (type)(
							(int)(cweeMath::Floor(((float)this->GetMonthOfYear(t)) / 3.0f) == j)
							)));
					features.Append(feature1);
				}
			}
		}
		{
			for (auto& x : MachineLearnFeatures) {
				//if (!DetermineIfExcludeFeature(exclusionCheck)) {
					switch (static_cast<patternSource>((int)x.x)) {
					case patternSource::Parent: {
						if (Parent != nullptr) {
							Parent->PreventDeletion(x.y);
							Parent->Lock();
							cweePattern_CatmullRomSpline<type>* ptr = Parent->UnsafeRead(x.y);
							Parent->Unlock();
							if (ptr) {
								switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
								case ::TrainedModel_Modifier::None: {
									feature1 = ptr->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
									break; }
								case ::TrainedModel_Modifier::DayOfWeek: {
									feature1 = ptr->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
									break; }
								case ::TrainedModel_Modifier::HourOfDay: {
									feature1 = ptr->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
									break; }
								case ::TrainedModel_Modifier::Velocity: {
									feature1 = ptr->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
									break; }
								case ::TrainedModel_Modifier::Acceleration: {
									feature1 = ptr->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
									break; }
								case ::TrainedModel_Modifier::MovingAverage: {
									feature1 = ptr->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
									break; }
								case ::TrainedModel_Modifier::Normalize: {
									feature1 = ptr->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
									break; }
								}							
							}							
							Parent->AllowDeletion(x.y);
						}
						break;
					}
					case patternSource::Global: {
						Globals->PreventDeletion(x.y);
						Globals->Lock();
						auto ptr = Globals->UnsafeRead(x.y);
						Globals->Unlock();
						if (ptr) {
							switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
							case ::TrainedModel_Modifier::None: {
								feature1 = ptr->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::DayOfWeek: {
								feature1 = ptr->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::HourOfDay: {
								feature1 = ptr->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Velocity: {
								feature1 = ptr->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Acceleration: {
								feature1 = ptr->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::MovingAverage: {
								feature1 = ptr->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Normalize: {
								feature1 = ptr->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							}
						}
						Globals->AllowDeletion(x.y);

						break;
					}
					case patternSource::Scada: {
						Scada->PreventDeletion(x.y);
						Scada->Lock();
						auto ptr = Scada->UnsafeRead(x.y);
						Scada->Unlock();
						if (ptr) {
							switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
							case ::TrainedModel_Modifier::None: {
								feature1 = ptr->GetMeasurement("value")->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::DayOfWeek: {
								feature1 = ptr->GetMeasurement("value")->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::HourOfDay: {
								feature1 = ptr->GetMeasurement("value")->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Velocity: {
								feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Acceleration: {
								feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::MovingAverage: {
								feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Normalize: {
								feature1 = ptr->GetMeasurement("value")->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							}
						}
						Scada->AllowDeletion(x.y);
						break;
					}
					case patternSource::Customers: {
						Customers->Lock();
						auto ptr = Customers->UnsafeRead(x.y);
						if (ptr) {
							switch (static_cast<::TrainedModel_Modifier>((int)x.z)) {
							case ::TrainedModel_Modifier::None: {
								feature1 = ptr->GetMeasurement("usage_gpm")->GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::DayOfWeek: {
								feature1 = ptr->GetMeasurement("usage_gpm")->GetTimePattern(false).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::HourOfDay: {
								feature1 = ptr->GetMeasurement("usage_gpm")->GetTimePattern(true).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Velocity: {
								feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(0).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Acceleration: {
								feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(1).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::MovingAverage: {
								feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(2).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							case ::TrainedModel_Modifier::Normalize: {
								feature1 = ptr->GetMeasurement("usage_gpm")->GetTransformedPattern(3).GetTimeSeries(timeStart - x.w, timeEnd - x.w, realStep);
								break; }
							}
						}
						Customers->Unlock();						
						break;
					}
					}	
					features.Append(feature1);
				//}
			}
		}
	}

	cweePattern_CatmullRomSpline<type> out; 
	out.SetInterpolationType(this->GetInterpolationType()); out.SetBoundaryType(this->GetBoundaryType());
	
	this->Lock(); {
		if (learned) {
			for (auto& x : cweeMachineLearning::Forecast(*learned, features, machineLearn_roundNearest)) {
				out.AddUniqueValue(x.first, (float)x.second, false);
			}
		}
		out.ClampValues((type)machineLearn_min, (type)machineLearn_max);
		if (machineLearn_MAD != vec2(1, 0)) { out *= (type)machineLearn_MAD[0]; out += (type)machineLearn_MAD[1]; }
	} this->Unlock();

	return out.GetTimeSeries(timeStart, timeEnd, realStep);
};

template< class type >
type cweePattern_CatmullRomSpline<type>::GetCurrentValue(const u64& time, const cweeUnorderedList< cweePattern_CatmullRomSpline<type> >* Parent, const measurement_t& outboundUnit) const {

	int i = 0, j = 0, k = 0;
	u64 bvals[4], clampedTime;
	type v;
	
	j = this->GetNumValues();

	if (j < 1) {
		v = 0;
		return v;
	}
	if (j == 1) {
		return this->ValueForIndex(0);
	}

	clampedTime = this->ClampedTime(time);
	clampedTime = this->LoopedTime(clampedTime);
	
	this->Lock();
	if (learned) {
		i = (int)learned->learned;
	}
	this->Unlock();

	if ((i > 0) && clampedTime != time) {
		// clamped time != time is indication that we are outside bounds of real dataset / knots.
		v = Forecast_Value_Automated(time, Parent);
	}
	else {
		i = this->IndexForTime(clampedTime);
		Basis(i - 1, clampedTime, bvals);
		v = 0;
		for (j = 0; j < 4; j++) {
			k = i + j - 2;
			v += (this->ValueForIndex(k) * (float)bvals[j]);
		}
	}

	if (outboundUnit != measurement_t::_end_)
	{
		vec3 mad0 = cweeUnits::GetMadConversion(this->GetMeasurement(), outboundUnit);
		v *= mad0[0];
		v += mad0[1];
	}
	return v;

};

template< class type >
void cweePattern_CatmullRomSpline<type>::ReduceMemory(float percentToRemove, const u64& start, const u64& end) const {
	if (!this->GetChanged()) return;

	if (start >= end) return;

	RemoveUnnecessaryKnots(start, end);
	
	if (percentToRemove <= 0) return;
#if 0
	int num = this->GetNumValues();

	// reduction based on (lack of) change in y-axis.
	if (num > 3) {
		type maxValue;
		type minValue;

		//cweeThreadedList<float> fast_quartiles; fast_quartiles = { 0.05, 0.95 };
		//fast_quartiles = ValueQuantiles(fast_quartiles, start, end, 100);
		//if (fast_quartiles.Num() > 0)
		//	minValue = fast_quartiles[0];
		//else 
		minValue = this->GetMinValue();

		//if (fast_quartiles.Num() > 1)
		//	maxValue = fast_quartiles[1];
		//else 
		maxValue = this->GetMaxValue();

		type epsilon = cweeMath::Fabs(maxValue - minValue) * (((float)percentToRemove) / 100.0f);
		// where step = 1 -> 1% removal. step = 2 -> 2% removal, etc. 		

		cweeThreadedList<int> indexesToDelete(this->GetNumValues() + 16);

		int currentPos = this->IndexForTime(start) + 1; // should be "1" under most conditions

		type val = this->ValueForIndex(currentPos - 1);
		type val2 = this->ValueForIndex(currentPos);
		type val3 = this->ValueForIndex(currentPos + 1);
		do {
			if (this->TimeForIndex(currentPos + 2) > end) break;
			if (this->GetNumValues() < (currentPos + 2)) break;

			if (
#if 1
				(cweeMath::Fabs((float)(val - val2)) <= epsilon)
				&& (cweeMath::Fabs((float)(val3 - val2)) <= epsilon)
				&& (cweeMath::Fabs((float)(val - val3)) <= epsilon)
#else
				(cweeMath::Fabs((float)(val - val3)) <= epsilon) // 
				&&
				(
					(cweeMath::Fabs((float)(val - val2)) <= epsilon)
					||
					(cweeMath::Fabs((float)(val2 - val3)) <= epsilon)
					)
#endif
				) { // ==				
				if (((val >= val2) && (val2 >= val3)) || ((val <= val2) && (val2 <= val3))) { // i.e. the 'middle' value is neither the max or min value in this relationship

					indexesToDelete.Append(currentPos);
					++currentPos;
					val2 = val3;
					val3 = this->ValueForIndex(currentPos + 1);
					continue; // don't move the currentPos forward. Repeat the analysis from this spot. 		

				}
			}
			++currentPos;
			val = val2;
			val2 = val3;
			val3 = this->ValueForIndex(currentPos + 1);
		} while (1);

		this->Lock();
		this->UnsafeGetTimes().RemoveIndexes(indexesToDelete);
		this->UnsafeGetValues().RemoveIndexes(indexesToDelete);
		this->Unlock();
	}

#else
	type maxValue;
	type minValue;
	minValue = this->GetMinValue();
	maxValue = this->GetMaxValue();
	type epsilon = cweeMath::Fabs(maxValue - minValue) * (((float)percentToRemove) / 100.0f);
	int num, currentPos;
	cweeThreadedList<int> indexesToDelete;
	type* val = nullptr; 
	type* val2 = nullptr;
	type* val3 = nullptr;

	this->Lock();
	{
		auto& _values = this->UnsafeGetValues();
		auto& _times = this->UnsafeGetTimes();

		num = _values.Num();

		// reduction based on (lack of) change in y-axis.
		if (num > 3) {
			// where step = 1 -> 1% removal. step = 2 -> 2% removal, etc. 		
			indexesToDelete.SetGranularity(num + 16);
			{
				currentPos = this->UnsafeIndexForTime(start) + 1; // should be "1" under most conditions
				if (currentPos <= 0) currentPos++;
				if ((currentPos + 3) < num) {
					val = &_values[currentPos - 1];
					val2 = &_values[currentPos];
					do {
						if (num <= (currentPos + 2)) break;
						if (_times[currentPos + 2] > end) break;	
						val3 = &_values[currentPos + 1];

						if (
							(cweeMath::Fabs((float)(*val - *val2)) <= epsilon) && (cweeMath::Fabs((float)(*val3 - *val2)) <= epsilon) && (cweeMath::Fabs((float)(*val - *val3)) <= epsilon) 
							) { // ==				
							if (
								((*val >= *val2) && (*val2 >= *val3)) || ((*val <= *val2) && (*val2 <= *val3))								
							) { // i.e. the 'middle' value is neither the max or min value in this relationship
								indexesToDelete.Append(currentPos);
								++currentPos;
								val2 = val3;								
								continue; // don't move the currentPos forward. Repeat the analysis from this spot. 		
							}
						}
						++currentPos;
						val = val2;
						val2 = val3;
					} while (1);
				}
			}
			_times.RemoveIndexes(indexesToDelete); // safe if outside bounds
			_values.RemoveIndexes(indexesToDelete); // safe if outside bounds
			
		}		
	}
	this->Unlock();
	this->SetChanged(false);
	
#endif
}


#endif