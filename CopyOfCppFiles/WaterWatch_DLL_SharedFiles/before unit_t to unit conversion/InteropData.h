#pragma once
#include "Precompiled.h"

class cweeData {
public:
	int CreatePattern() {
		return patterns->Append();
	};
	int CreateMatrix() {
		return matrixes->Append();
	};
	void DeletePattern(int index) {
		patterns->Erase(index);
	};
	void DeleteMatrix(int index) {
		matrixes->Erase(index);
	};
	void ClearPattern(int index) {
		cweeUnorderedListReferenceObject< Pattern > safeContainer(patterns, index);
		if (safeContainer) safeContainer->Clear();
	};
	void ClearMatrix(int index) {
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) safeContainer->Clear();
	};
	void AppendData(int index, const u64& time, float value) {
		cweeUnorderedListReferenceObject< Pattern > safeContainer(patterns, index);
		if (safeContainer) safeContainer->AddUniqueValue(time, value);
	};
	void AppendData(int index, const u64& column, const u64& row, float value) {
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) safeContainer->InsertValue(column, row, value);
	};
	float GetValue(int index, const u64& time) {
		float out = 0;
		cweeUnorderedListReferenceObject< Pattern > safeContainer(patterns, index);
		if (safeContainer) out = safeContainer->GetCurrentValue(time);
		return out;
	};
	float GetValue(int index, const u64& column, const u64& row) {
		float out = 0;
		cweeUnorderedListReferenceObject< cweeInterpolatedMatrix<float> > safeContainer(matrixes, index);
		if (safeContainer) out = safeContainer->GetValue(column, row);
		return out;
	};

	/*! Row1Column1, Row1Column2, ... Row1ColumnN, Row2Column1 ... etc. */
	cweeThreadedList<std::pair<u64, float>> GetPattern(int index) {
		cweeThreadedList<std::pair<u64, float>> out;
		cweeUnorderedListReferenceObject< Pattern > safeContainer(patterns, index);
		if (safeContainer) {
			out = safeContainer->GetKnotSeries();
		}
		return out;
	};
	cweeThreadedList<std::pair<u64, float>> GetPattern(int index, const u64& From, const u64& Till) {
		cweeThreadedList<std::pair<u64, float>> out;
		cweeUnorderedListReferenceObject< Pattern > safeContainer(patterns, index);
		if (safeContainer) {
			out = safeContainer->GetKnotSeries(From, Till);
		}
		return out;
	};
	cweeThreadedList<std::pair<u64, float>> GetPattern(int index, const u64& From, const u64& Till, const u64& step) {
		cweeThreadedList<std::pair<u64, float>> out;
		cweeUnorderedListReferenceObject< Pattern > safeContainer(patterns, index);
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
	void SetPattern(int index, const Pattern& other) {
		cweeUnorderedListReferenceObject< Pattern > safeContainer(patterns, index);
		if (safeContainer) {
			safeContainer->operator=(other);
		}
	};

	int CreateSpatialAsset() {
		return spatialAssets->Append();
	};
	void DeleteSpatialAsset(int index) {
		spatialAssets->Erase(index);
	};
	void ClearSpatialAsset(int index) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > safeContainer(spatialAssets, index);
		if (safeContainer) safeContainer->Clear();
	};
	void AppendData(int index, const cweeStr& name, const cweeStr& constValue) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > safeContainer(spatialAssets, index);
		if (safeContainer) safeContainer->Append(name, constValue);
	};
	void AppendData(int index, const cweeStr& name, const u64& time, const cweeStr& value) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > safeContainer(spatialAssets, index);
		if (safeContainer) safeContainer->Append(name, time, value);
	};
	void AppendData(int index, const cweeStr& name, const u64& time, float value) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > safeContainer(spatialAssets, index);
		if (safeContainer) safeContainer->Append(name, time, value);
	};
	cweeStr GetValue(int index, const cweeStr& name, const u64& time = 0) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > ptr(spatialAssets, index);

		cweeStr out;
		if (ptr) {
			auto type = ptr->GetParameter(name);
			switch (type) {
			case cweeThreadedSpatialAsset::SpatialAssetParam::constantParam: {
				auto got = ptr->GetConstParameter(name);
				if (got) {
					got->Lock();
					auto ptr2 = got->UnsafeRead();
					if (ptr2) {
						out = *ptr2;
					}
					got->Unlock();
				}
				break;
			}
			case cweeThreadedSpatialAsset::SpatialAssetParam::timeseriesParam: {
				auto got = ptr->GetTimeSeriesParameter(name);
				if (got) {
					got->Lock();
					auto ptr2 = got->UnsafeRead();
					if (ptr2) {
						out = ptr2->GetCurrentValue(time);
					}
					got->Unlock();
				}
				break;
			}
			case cweeThreadedSpatialAsset::SpatialAssetParam::measurement: {
				auto got = ptr->GetMeasurement(name);
				if (got) {
					out = cweeStr(got->GetCurrentValue(time));
				}
				break;
			}
			}
		}
		return out;
	};
	cweeThreadedList<std::pair<cweeStr, cweeThreadedSpatialAsset::SpatialAssetParam>> GetSpatialAssetParameters(int index) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > ptr(spatialAssets, index);

		cweeThreadedList<std::pair<cweeStr, cweeThreadedSpatialAsset::SpatialAssetParam>> out;
		if (ptr) {
			cweeThreadedList<cweeStr> p = ptr->GetParameters();
			for (auto& x : p) {
				out.Append(std::pair<cweeStr, cweeThreadedSpatialAsset::SpatialAssetParam>(x, ptr->GetParameter(x)));
			}
		}
		return out;
	};
	cweeStr GetSpatialAssetConstant(int index, const cweeStr& name) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > ptr(spatialAssets, index);

		cweeStr out;
		if (ptr) {
			auto type = ptr->GetParameter(name);
			switch (type) {
			case cweeThreadedSpatialAsset::SpatialAssetParam::constantParam: {
				auto got = ptr->GetConstParameter(name);
				if (got) {
					got->Lock();
					auto ptr2 = got->UnsafeRead();
					if (ptr2) {
						out = *ptr2;
					}
					got->Unlock();
				}
				break;
			}
			}
		}

		out.ReplaceInline("\n", "\r");
		out.ReplaceInline("\r\r", "\r");

		return out;
	};
	cweeThreadedList<std::pair<u64, cweeStr>> GetSpatialAssetTimeseries(int index, const cweeStr& name) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > ptr(spatialAssets, index);

		cweeThreadedList<std::pair<u64, cweeStr>> out;
		if (ptr) {
			auto type = ptr->GetParameter(name);
			switch (type) {
			case cweeThreadedSpatialAsset::SpatialAssetParam::timeseriesParam: {
				auto got = ptr->GetTimeSeriesParameter(name);
				if (got) {
					got->Lock();
					auto ptr2 = got->UnsafeRead();
					if (ptr2) {
						out = ptr2->GetKnotSeries();
					}
					got->Unlock();
				}
				break;
			}
			}
		}
		return out;
	};
	cweeThreadedList<std::pair<u64, float>> GetSpatialAssetMeasurement(int index, const cweeStr& name) {
		cweeUnorderedListReferenceObject< cweeThreadedSpatialAsset > ptr(spatialAssets, index);

		cweeThreadedList<std::pair<u64, float>> out;
		if (ptr) {
			auto type = ptr->GetParameter(name);
			switch (type) {
			case cweeThreadedSpatialAsset::SpatialAssetParam::measurement: {
				auto got = ptr->GetMeasurement(name);
				if (got) {
					got->RemoveUnnecessaryKnots();
					out = got->GetKnotSeries();
				}
				break;
			}
			}
		}
		return out;
	};

private:
	cweeSharedPtr < cweeUnorderedList < Pattern > >							patterns = make_cwee_shared < cweeUnorderedList < Pattern > >();
	cweeSharedPtr < cweeUnorderedList < cweeInterpolatedMatrix<float> > >	matrixes = make_cwee_shared < cweeUnorderedList < cweeInterpolatedMatrix<float> > >();
	cweeSharedPtr < cweeUnorderedList < cweeThreadedSpatialAsset > >		spatialAssets = make_cwee_shared < cweeUnorderedList < cweeThreadedSpatialAsset > >();

};
static cweeSharedPtr<cweeData> externalData = make_cwee_shared<cweeData>();