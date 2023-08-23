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
#include "SharedPtr.h"
#include "UnorderedList.h"
#include "BalancedPattern.h"
#include "Pattern.h"
#include "InterpolatedMatrix.h"
#include "cweeUnitedValue.h"
#include "cweeUnitPattern.h"
#include "cweeThreadedMap.h"

class cweeData {
private:
	cweeThreadedMap<int, cweeStr> strings; cweeSysInterlockedInteger nStrings;
	cweeThreadedMap<int, cweeUnitValues::cweeUnitPattern> patterns; cweeSysInterlockedInteger nPatterns;
	cweeThreadedMap<int, cweeInterpolatedMatrix<float>> matrixes; cweeSysInterlockedInteger nMatrixes;

	AUTO GetStringPtr(int index) { return strings.GetPtr(index); };
	AUTO GetPatternPtr(int index) { return patterns.GetPtr(index); };
	AUTO GetMatrixPtr(int index) { return matrixes.GetPtr(index); };

public:
	int CreateString() {
		int index = nStrings.Increment();
		strings.Emplace(index, cweeStr());
		return index;
	};
	int CreatePattern() {
		int index = nPatterns.Increment();
		patterns.Emplace(index, cweeUnitValues::cweeUnitPattern());
		return index;
	};
	int CreateMatrix() {
		int index = nMatrixes.Increment();
		matrixes.Emplace(index, cweeInterpolatedMatrix<float>());
		return index;
	};
	
	bool CheckString(int index) {
		return strings.count(index) > 0;
	};
	bool CheckPattern(int index) {
		return patterns.count(index) > 0;
	};
	bool CheckMatrix(int index) {
		return matrixes.count(index) > 0;
	};

	void DeleteString(int index) {
		strings.Erase(index);
	};
	void DeletePattern(int index) {
		patterns.Erase(index);
	};
	void DeleteMatrix(int index) {
		matrixes.Erase(index);
	};

	void ClearPattern(int index) {
		GetPatternPtr(index)->Clear();
	};
	void ClearMatrix(int index) {
		GetMatrixPtr(index)->Clear();
	};

	void AppendData(int index, cweeStr const& value) {
		GetStringPtr(index)->operator=(value);
	};
	void AppendData(int index, const u64& time, float value) {
		GetPatternPtr(index)->AddUniqueValue(time, value);
	};
	void AppendData(int index, const u64& column, const u64& row, float value) {
		GetMatrixPtr(index)->InsertValue(column, row, value);
	};

	cweeStr GetValue(int index) {
		return GetStringPtr(index)->c_str();
	};
	cweeUnitValues::unit_value GetValue(int index, const cweeUnitValues::unit_value& time) {
		return GetPatternPtr(index)->GetCurrentValue(time);
	};
	float GetValue(int index, const u64& column, const u64& row) {
		return GetMatrixPtr(index)->GetValue(column, row);
	};

	cweeUnitValues::cweeUnitPattern* GetPatternRef(int index) {
		return GetPatternPtr(index).Get();
	};
	cweeInterpolatedMatrix<float>* GetMatrixRef(int index) {
		return GetMatrixPtr(index).Get();
	};
	cweeStr* GetStringRef(int index) {
		return GetStringPtr(index).Get();
	};

	/*! Row1Column1, Row1Column2, ... Row1ColumnN, Row2Column1 ... etc. */
	cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> GetPattern(int index) {
		return GetPatternPtr(index)->GetKnotSeries();
	};
	cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> GetPattern(int index, const cweeUnitValues::unit_value& From, const cweeUnitValues::unit_value& Till) {
		return GetPatternPtr(index)->GetKnotSeries(From, Till);
	};
	cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> GetPattern(int index, const cweeUnitValues::unit_value& From, const cweeUnitValues::unit_value& Till, const cweeUnitValues::unit_value& step) {
		return GetPatternPtr(index)->GetTimeSeries(From, Till, step);
	};

	std::vector<float> GetMatrix(int index, const u64& Left, const u64& Top, const u64& Right, const u64& Bottom, int numColumns, int numRows) {
		return GetMatrixPtr(index)->GetMatrix(Left, Top, Right, Bottom, numColumns, numRows);
	};
	void SetMatrix(int index, const cweeInterpolatedMatrix<float>& other) {
		GetMatrixPtr(index)->operator=(other);
	};
	void SetPattern(int index, const cweeUnitValues::cweeUnitPattern& other) {
		AUTO safeContainer = GetPatternPtr(index);
		if (safeContainer) {
			safeContainer->Clear();
			safeContainer->SetInterpolationType(other.GetInterpolationType());
			safeContainer->SetBoundaryType(other.GetBoundaryType());
			for (auto& x : other.GetKnotSeries()) {
				safeContainer->AddValue(x.first, x.second);
			}
		}
	};
};

extern cweeSharedPtr< cweeData> external_data;
