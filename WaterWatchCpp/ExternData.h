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

class cweeData {
private:
	cweeSharedPtr < cweeUnorderedList < cweeStr > > strings = make_cwee_shared < cweeUnorderedList < cweeStr > >();;
	cweeSharedPtr < cweeUnorderedList < cweeUnitValues::cweeUnitPattern > >						patterns = make_cwee_shared < cweeUnorderedList < cweeUnitValues::cweeUnitPattern > >();;
	cweeSharedPtr < cweeUnorderedList < cweeInterpolatedMatrix<float> > > matrixes = make_cwee_shared < cweeUnorderedList < cweeInterpolatedMatrix<float> > >();;

public:
	int CreateString() {
		return strings->Append();
	};

	int CreatePattern() {
		return patterns->Append();
	};

	int CreateMatrix() {
		return matrixes->Append();
	};
	
	void DeleteString(int index) {
		strings->Erase(index);
	};

	void DeletePattern(int index) {
		patterns->Erase(index);
	};

	void DeleteMatrix(int index) {
		matrixes->Erase(index);
	};

	void ClearPattern(int index) {
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) safeContainer->Clear();
	};

	void ClearMatrix(int index) {
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) safeContainer->Clear();
	};

	void AppendData(int index, cweeStr const& value) {
		cweeUnorderedListReferenceObject< cweeStr > safeContainer(strings, index);
		if (safeContainer) safeContainer->operator=(value);
	};

	void AppendData(int index, const u64& time, float value) {
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) safeContainer->AddUniqueValue(time, value);
	};

	void AppendData(int index, const u64& column, const u64& row, float value) {
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) safeContainer->InsertValue(column, row, value);
	};

	cweeStr GetValue(int index) {
		cweeStr out;
		cweeUnorderedListReferenceObject< cweeStr > safeContainer(strings, index);
		if (safeContainer) out = *safeContainer;
		return out;
	};

	cweeUnitValues::unit_value GetValue(int index, const cweeUnitValues::unit_value& time) {
		cweeUnitValues::unit_value out = 0;
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) out = safeContainer->GetCurrentValue(time);
		return out;
	};

	float GetValue(int index, const u64& column, const u64& row) {
		float out = 0;
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) out = safeContainer->GetValue(column, row);
		return out;
	};

	cweeUnitValues::cweeUnitPattern* GetPatternRef(int index) {
		cweeUnitValues::cweeUnitPattern* out = nullptr;
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) {
			out = &*safeContainer;
		}
		return out;
	};
	cweeInterpolatedMatrix<float>* GetMatrixRef(int index) {
		cweeInterpolatedMatrix<float>* out = nullptr;
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) {
			out = &*safeContainer;
		}
		return out;
	};
	cweeStr* GetStringRef(int index) {
		cweeStr* out = nullptr;
		cweeUnorderedListReferenceObject< cweeStr > safeContainer(strings, index);
		if (safeContainer) {
			out = &*safeContainer;
		}
		return out;
	};

	/*! Row1Column1, Row1Column2, ... Row1ColumnN, Row2Column1 ... etc. */
	cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> GetPattern(int index) {
		cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> out;
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) {
			out = safeContainer->GetKnotSeries();
		}
		return out;
	};
	cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> GetPattern(int index, const cweeUnitValues::unit_value& From, const cweeUnitValues::unit_value& Till) {
		cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> out;
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) {
			out = safeContainer->GetKnotSeries(From, Till);
		}
		return out;
	};
	cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> GetPattern(int index, const cweeUnitValues::unit_value& From, const cweeUnitValues::unit_value& Till, const cweeUnitValues::unit_value& step) {
		cweeThreadedList<std::pair<cweeUnitValues::unit_value, cweeUnitValues::unit_value>> out;
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) {
			out = safeContainer->GetTimeSeries(From, Till, step);
		}
		return out;
	};

	std::vector<float> GetMatrix(int index, const u64& Left, const u64& Top, const u64& Right, const u64& Bottom, int numColumns, int numRows) {
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) {
			return safeContainer->GetMatrix(Left, Top, Right, Bottom, numColumns, numRows);
		}
		else {
			return std::vector<float>();
		}
	};
	void SetMatrix(int index, const cweeInterpolatedMatrix<float>& other) {
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) {
			safeContainer->operator=(other);
		}
	};
	void SetPattern(int index, const cweeUnitValues::cweeUnitPattern& other) {
		cweeUnorderedListReferenceObject< cweeUnitValues::cweeUnitPattern > safeContainer(patterns, index);
		if (safeContainer) {
			safeContainer->Clear();
			for (auto& x : other.GetKnotSeries()) {
				safeContainer->AddValue(x.first, x.second);
			}
		}
	};

};

extern cweeSharedPtr< cweeData> external_data;
