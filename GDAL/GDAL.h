#pragma once
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/Units.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/cweeTime.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/enum.h"
#include "../WaterWatchCpp/Iterator.h"
#include "../WaterWatchCpp/DelayedInstantiation.h"

class GDAL_Data {
public:
	virtual cweeStr TestGDAL(cweeStr filePath) = 0;
};
extern cweeSharedPtr<GDAL_Data> gdal_data;

namespace cweeGeo {
	class Field {
	protected:
		cweeSharedPtr<void> data;

	public:
		Field() : data(nullptr) {};
		Field(cweeSharedPtr<void> dataSource) : data(dataSource) {};
		Field(Field const& other) : data(other.data) {};
		Field(Field&& other) : data(other.data) {};
		Field& operator=(Field const& other) { data = other.data; return *this; };
		Field& operator=(Field&& other) { data = other.data; return *this; };



	};



	class ShapeFile {
	public:



	};



};