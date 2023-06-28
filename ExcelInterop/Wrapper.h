
#pragma once
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/SharedPtr.h"

class ExcelWorksheet {
public:
	ExcelWorksheet() : data() {};
	ExcelWorksheet(cweeSharedPtr<void> d) : data(d) {};
	cweeSharedPtr<void> data; // could be anything! represents the worksheet ACTUALLY

	cweeStr ReadCell(cweeStr cellName);
};

class ExcelWorkbook {
public:
	ExcelWorkbook() : data() {};
	ExcelWorkbook(cweeSharedPtr<void> d) : data(d) {};
	cweeSharedPtr<void> data; // could be anything! represents the workbook ACTUALLY

	cweeSharedPtr<ExcelWorksheet> OpenWorksheet(int i);
	cweeSharedPtr<ExcelWorksheet> OpenWorksheet();
};

class cweeExcel {
public:
	static cweeSharedPtr<ExcelWorkbook> OpenExcel(cweeStr filePath);

};