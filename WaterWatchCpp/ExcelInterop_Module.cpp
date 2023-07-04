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
#include "ExcelInterop_Module.h"

#include "cweeUnitedValue.h"
#include "enum.h"
#include "cweeUnitPattern.h"
#include "fileSystemH.h"
#include "cweeScheduler.h"
#include "InterpolatedMatrix.h"
#include "Engineering.h"
#include "../ExcelInterop/Wrapper.h" // the bulk of the Excel tool

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_Excel() {
            auto lib = chaiscript::make_shared<Module>();

            // cweeExcel
            {
                lib->add(fun([]() { return cweeExcel::OpenExcel(); }), "OpenExcel");
                lib->add(fun([](cweeStr filePath) { return cweeExcel::OpenExcel(filePath); }), "OpenExcel");
            }

#define ThrowIfBadAccess throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object."))
#define WorkbookPtr cweeSharedPtr<ExcelWorkbook>
#define WorksheetPtr cweeSharedPtr<ExcelWorksheet>
#define RangePtr cweeSharedPtr<ExcelRange>
#define CellPtr cweeSharedPtr<ExcelCell>

            // ExcelWorkbook
            {
                AddSharedPtrClass(, ExcelWorkbook);
				{
					lib->add(chaiscript::fun([](WorkbookPtr& a) { if (a) return a->create_sheet(); else ThrowIfBadAccess; }), "create_sheet");
					lib->add(chaiscript::fun([](WorkbookPtr& a, int index) { if (a) return a->create_sheet(index); else ThrowIfBadAccess; }), "create_sheet");
					lib->add(chaiscript::fun([](WorkbookPtr& a, WorksheetPtr worksheet) { if (a) return a->copy_sheet(worksheet); else ThrowIfBadAccess; }), "copy_sheet");
					lib->add(chaiscript::fun([](WorkbookPtr& a, WorksheetPtr worksheet, int index) { if (a) return a->copy_sheet(worksheet, index); else ThrowIfBadAccess; }), "copy_sheet");
					lib->add(chaiscript::fun([](WorkbookPtr& a) { if (a) return a->active_sheet(); else ThrowIfBadAccess; }), "active_sheet");
					lib->add(chaiscript::fun([](WorkbookPtr& a, int index) { if (a) a->active_sheet(index); else ThrowIfBadAccess; }), "active_sheet");
					lib->add(chaiscript::fun([](WorkbookPtr& a, cweeStr const& title) { if (a) return a->sheet_by_title(title); else ThrowIfBadAccess; }), "sheet_by_title");
					lib->add(chaiscript::fun([](WorkbookPtr& a, int index) { if (a) return a->sheet_by_index(index); else ThrowIfBadAccess; }), "sheet_by_index");
					lib->add(chaiscript::fun([](WorkbookPtr& a, int index) { if (a) return a->sheet_by_id(index); else ThrowIfBadAccess; }), "sheet_by_id");
					lib->add(chaiscript::fun([](WorkbookPtr& a, int index) { if (a) return a->sheet_hidden_by_index(index); else ThrowIfBadAccess; }), "sheet_hidden_by_index");
					lib->add(chaiscript::fun([](WorkbookPtr& a, const cweeStr& title) { if (a) return a->contains(title); else ThrowIfBadAccess; }), "contains");
					lib->add(chaiscript::fun([](WorkbookPtr& a, WorksheetPtr worksheet) { if (a) return a->index(worksheet); else ThrowIfBadAccess; }), "index");
					lib->add(chaiscript::fun([](WorkbookPtr& a, WorksheetPtr worksheet) { if (a) return a->remove_sheet(worksheet); else ThrowIfBadAccess; }), "remove_sheet");
				}

				AddSharedPtrClass(, ExcelWorksheet);
				{
					lib->add(chaiscript::fun([](WorksheetPtr& a) { if (a) return a->id(); else ThrowIfBadAccess; }), "id");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int id) { if (a) return a->id(id); else ThrowIfBadAccess; }), "id");
					lib->add(chaiscript::fun([](WorksheetPtr& a) { if (a) return a->title(); else ThrowIfBadAccess; }), "title");
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr title) { if (a) a->title(title); else ThrowIfBadAccess; }), "title");
					lib->add(chaiscript::fun([](WorksheetPtr& a, CellPtr cell) { if (a) return a->freeze_panes(*cell); else ThrowIfBadAccess; }), "freeze_panes");
					AddSharedPtrClassFunction(, ExcelWorksheet, unfreeze_panes);
					AddSharedPtrClassFunction(, ExcelWorksheet, has_frozen_panes);
					lib->add(chaiscript::fun([](WorksheetPtr& a, int column, int row) { if (a) return a->cell(column, row); else ThrowIfBadAccess; }), "cell");
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr range) { if (a) return a->range(range); else ThrowIfBadAccess; }), "range");
					lib->add(chaiscript::fun([](WorksheetPtr& a, bool skip_null) { if (a) return a->rows(skip_null); else ThrowIfBadAccess; }), "rows");
					lib->add(chaiscript::fun([](WorksheetPtr& a) { if (a) return a->rows(); else ThrowIfBadAccess; }), "rows");
					lib->add(chaiscript::fun([](WorksheetPtr& a, bool skip_null) { if (a) return a->columns(skip_null); else ThrowIfBadAccess; }), "columns");
					lib->add(chaiscript::fun([](WorksheetPtr& a) { if (a) return a->columns(); else ThrowIfBadAccess; }), "columns");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int row) { if (a) a->clear_row(row); else ThrowIfBadAccess; }), "clear_row");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int row, int amount) { if (a) a->insert_rows(row, amount); else ThrowIfBadAccess; }), "insert_rows");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int col, int amount) { if (a) a->insert_columns(col, amount); else ThrowIfBadAccess; }), "insert_columns");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int row, int amount) { if (a) a->delete_rows(row, amount); else ThrowIfBadAccess; }), "delete_rows");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int col, int amount) { if (a) a->delete_columns(col, amount); else ThrowIfBadAccess; }), "delete_columns");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int col) { if (a) return a->column_width(col); else ThrowIfBadAccess; }), "column_width");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int row) { if (a) return a->row_height(row); else ThrowIfBadAccess; }), "row_height");
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr name, cweeStr reference_string) { if (a) a->create_named_range(name, reference_string); else ThrowIfBadAccess; }), "create_named_range");
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr name) { if (a) return a->has_named_range(name); else ThrowIfBadAccess; }), "has_named_range");
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr name) { if (a) return a->named_range(name); else ThrowIfBadAccess; }), "named_range");
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr name) { if (a) a->remove_named_range(name); else ThrowIfBadAccess; }), "remove_named_range");
					AddSharedPtrClassFunction(, ExcelWorksheet, lowest_row);
					AddSharedPtrClassFunction(, ExcelWorksheet, lowest_row_or_props);
					AddSharedPtrClassFunction(, ExcelWorksheet, highest_row);
					AddSharedPtrClassFunction(, ExcelWorksheet, highest_row_or_props);
					AddSharedPtrClassFunction(, ExcelWorksheet, next_row);
					AddSharedPtrClassFunction(, ExcelWorksheet, lowest_column);
					AddSharedPtrClassFunction(, ExcelWorksheet, lowest_column_or_props);
					AddSharedPtrClassFunction(, ExcelWorksheet, highest_column);
					AddSharedPtrClassFunction(, ExcelWorksheet, highest_column_or_props);
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr name) { if (a) a->merge_cells(name); else ThrowIfBadAccess; }), "merge_cells");
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr name) { if (a) a->unmerge_cells(name); else ThrowIfBadAccess; }), "unmerge_cells");
					AddSharedPtrClassFunction(, ExcelWorksheet, has_page_setup);
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr name) { if (a) a->auto_filter(name); else ThrowIfBadAccess; }), "auto_filter");
					lib->add(chaiscript::fun([](WorksheetPtr& a, RangePtr range) { if (a) a->auto_filter(*range); else ThrowIfBadAccess; }), "auto_filter");
					AddSharedPtrClassFunction(, ExcelWorksheet, clear_auto_filter);
					AddSharedPtrClassFunction(, ExcelWorksheet, has_auto_filter);
					lib->add(chaiscript::fun([](WorksheetPtr& a, int n) { if (a) a->reserve(n); else ThrowIfBadAccess; }), "reserve");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int start, int end) { if (a) a->print_title_rows(start, end); else ThrowIfBadAccess; }), "print_title_rows");
					lib->add(chaiscript::fun([](WorksheetPtr& a, int start, int end) { if (a) a->print_title_cols(start, end); else ThrowIfBadAccess; }), "print_title_cols");
					AddSharedPtrClassFunction(, ExcelWorksheet, has_print_titles);
					AddSharedPtrClassFunction(, ExcelWorksheet, clear_print_titles);
					lib->add(chaiscript::fun([](WorksheetPtr& a, cweeStr print_area) { if (a) a->print_area(print_area); else ThrowIfBadAccess; }), "print_area");
					AddSharedPtrClassFunction(, ExcelWorksheet, clear_print_area);
					AddSharedPtrClassFunction(, ExcelWorksheet, has_print_area);
					AddSharedPtrClassFunction(, ExcelWorksheet, has_view);
					AddSharedPtrClassFunction(, ExcelWorksheet, has_active_cell);
					AddSharedPtrClassFunction(, ExcelWorksheet, clear_page_breaks);
					AddSharedPtrClassFunction(, ExcelWorksheet, page_break_rows);
					lib->add(chaiscript::fun([](WorksheetPtr& a, int row) { if (a) a->page_break_at_row(row); else ThrowIfBadAccess; }), "page_break_at_row");
					AddSharedPtrClassFunction(, ExcelWorksheet, page_break_columns);
					lib->add(chaiscript::fun([](WorksheetPtr& a, int column) { if (a) a->page_break_at_column(column); else ThrowIfBadAccess; }), "page_break_at_column");
					AddSharedPtrClassFunction(, ExcelWorksheet, has_drawing);
					AddSharedPtrClassFunction(, ExcelWorksheet, is_empty);
				}

				AddSharedPtrClass(, ExcelRange);
				{
					DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(CellPtr);

					AddSharedPtrClassFunction(, ExcelRange, clear_cells);
					lib->add(chaiscript::fun([](RangePtr& a, int n) { if (a) return a->vector(n); else ThrowIfBadAccess; }), "vector");
					lib->add(chaiscript::fun([](RangePtr& a, CellPtr cell) { if (a) return a->cell(cell->reference()); else ThrowIfBadAccess; }), "cell");
					AddSharedPtrClassFunction(, ExcelRange, target_worksheet);
					AddSharedPtrClassFunction(, ExcelRange, length);
					lib->add(chaiscript::fun([](RangePtr& a, CellPtr cell) { if (a) return a->contains(cell->reference()); else ThrowIfBadAccess; }), "contains");
					AddSharedPtrClassFunction(, ExcelRange, front);
					AddSharedPtrClassFunction(, ExcelRange, back);
					lib->add(chaiscript::fun([](RangePtr& a, int n) { if (a) return a->operator[](n); else ThrowIfBadAccess; }), "[]");
					lib->add(chaiscript::fun([](RangePtr& a, std::function<void(cweeSharedPtr<ExcelCell>)> f) { if (a) a->apply(f); else ThrowIfBadAccess; }), "apply");
					lib->add(chaiscript::fun([](RangePtr& a, cweeStr timeLocation) { if (a) {
						cweeList<cweeStr> positions; positions = { cweeStr("Top"), cweeStr("Left"), cweeStr("") };
						timeLocation = timeLocation.BestMatch(positions);
						
						AUTO timeHash = timeLocation.Hash();

						cweeThreadedMap<cweeStr, cweeUnitValues::cweeUnitPattern> patterns;

						bool LeftToRight = false;
						{
							switch (timeHash) {
							case cweeStr::Hash("Left"):
								// values go top-to-bottom
								LeftToRight = false;

								break;
							case cweeStr::Hash("Top"):
								// values go left-to-right
								LeftToRight = true;

								break;
							default:
							case cweeStr::Hash(""):
								// Unknown direction! Find out the firstion based on the dimensions
								auto numRows = a->length();
								auto numCols = std::numeric_limits<decltype(numRows)>::max();
								for (auto& rowOrCol : *a) {
									numCols = ::Min<decltype(numCols)>(rowOrCol.Num(), numCols);
								}

								if (numRows >= numCols) {
									LeftToRight = false;
								}
								else {
									LeftToRight = true;
								}

								break;
							}
						}

						int headerOffset = 0;
						cweeList<cweeStr> headers;
						{
							if (LeftToRight) {
								// IF there are any sites, they will be in the first column
								int firstIsString = 0;
								int firstIsNumber = 0;
								for (auto& rowOrCol : *a) {
									for (auto& cell : rowOrCol) {
										switch (cell->data_type()) {
										case CellType::boolean:
										case CellType::date:
										case CellType::number:
											firstIsNumber++;
											break;
										case CellType::error:
										case CellType::formula_string:
										case CellType::inline_string:
										case CellType::shared_string:
											firstIsString++;
											break;
										case CellType::empty:
										default:
											break;
										}
										break;
									}
									if (std::fabs(firstIsString - firstIsNumber) > 100) {
										break;
									}
								}
								if (firstIsString > firstIsNumber) {
									headerOffset = 1;
								}
								else {
									headerOffset = 0;
								}
							}
							else {
								// IF there are any sites, they will be in the first row
								for (auto& rowOrCol : *a) {
									int firstIsString = 0;
									int firstIsNumber = 0;
									for (auto& cell : rowOrCol) {
										switch (cell->data_type()) {
										case CellType::boolean:
										case CellType::date:
										case CellType::number:
											firstIsNumber++;
											break;
										case CellType::error:
										case CellType::formula_string:
										case CellType::inline_string:
										case CellType::shared_string:
											firstIsString++;
											break;
										case CellType::empty:
										default:
											break;
										}
										if (std::fabs(firstIsString - firstIsNumber) > 100) {
											break;
										}
									}
									if (firstIsString > firstIsNumber) {
										headerOffset = 1;
									}
									else {
										headerOffset = 0;
									}
									break;
								}
							}

							if (headerOffset >= 1) {
								// we have headers based on the cell values
								if (LeftToRight) {
									for (auto& rowOrCol : *a) {
										for (auto& cell : rowOrCol) {
											headers.Append(cell->to_string().c_str());
											break;
										}
									}
								}
								else {
									for (auto& rowOrCol : *a) {
										for (auto& cell : rowOrCol) {
											headers.Append(cell->to_string().c_str());
										}
										break;
									}
								}
							}
							else {
								// we have headers based on the row/column index
								if (LeftToRight) {
									for (auto& rowOrCol : *a) {
										for (auto& cell : rowOrCol) {
											headers.Append(cweeStr(headers.Num()+1).c_str());
											break;
										}
									}
								}
								else {
									for (auto& rowOrCol : *a) {
										for (auto& cell : rowOrCol) {
											headers.Append(cweeStr(headers.Num() + 1).c_str());
										}
										break;
									}
								}
							}
						}

						// collect the data
						cweeList<double> times;
						int rowNumber = 0;
						int colNumber;
						cweeStr* siteName = nullptr;
						double* thisTime = nullptr;
						for (auto& rowOrCol : *a) { // rowOrColumn
							rowNumber++;
							if (LeftToRight && rowNumber == 1 && timeHash != cweeStr::Hash("")) {
								// the first row is the time row
								colNumber = 0;
								for (auto& cell : rowOrCol) {
									colNumber++;
									if (LeftToRight && colNumber == 1 && headerOffset >= 1) {
										// the first column is the site names
										continue;
									}
									else {
										times.Append(cell->value<double>());
									}
								}
								continue;
							}
							else if (!LeftToRight && rowNumber == 1 && headerOffset >= 1) {
								// the first row is the site names
								continue;
							}
							else {
								colNumber = 0;
								for (auto& cell : rowOrCol) {
									colNumber++;
									if (!LeftToRight && colNumber == 1 && timeHash != cweeStr::Hash("")) {
										// the first column is the time row
										times.Append(cell->value<double>());
										continue;
									}
									else if (LeftToRight && colNumber == 1 && headerOffset >= 1) {
										// the first column is the site names
										continue;
									}
									else {
										if (LeftToRight) {
											siteName = &headers[rowNumber - 1];
											if (times.Num() <= (colNumber - 1)) { times.AssureSize(colNumber, colNumber); }
											thisTime = &times[colNumber - 1];
										}
										else {
											siteName = &headers[colNumber - 1];
											if (times.Num() <= (rowNumber - 1)) { times.AssureSize(rowNumber, rowNumber); }
											thisTime = &times[rowNumber - 1];
										}

										patterns.GetPtr(*siteName)->AddValue(*thisTime, cell->value<double>());
									}
								}
							}
						}

						std::map<std::string, chaiscript::Boxed_Value> out;
						for (auto& site : patterns) {
							out.emplace(std::string(site.first.c_str()), var(cweeUnitValues::cweeUnitPattern(*site.second)));
						}
						return out;
					} else ThrowIfBadAccess; }), "to_patterns");

				}

				AddSharedPtrClass(, ExcelCell); 
				{
					ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(CellType, CellType);

					AddSharedPtrClassFunction(, ExcelCell, has_value);
					lib->add(chaiscript::fun([](CellPtr& a) -> chaiscript::Boxed_Value { if (a) { 
						switch (a->data_type()) {
						case CellType::empty:
							return var(nullptr);
						case CellType::error:
							return var(a->error());
						default:
							if (a->is_date()) {
								return var(a->value<cweeTime>());
							}
							else {
								switch (a->data_type()) {
								case CellType::date:
									return var(a->value<cweeTime>());
								case CellType::formula_string:
								case CellType::inline_string:
								case CellType::shared_string:
									return var(a->value<cweeStr>());
								case CellType::number:
									return var(a->value<double>());
								case CellType::boolean:
									return var(a->value<bool>());
								default:
									return var(nullptr);
								}
							}
						}
						
					}  else ThrowIfBadAccess; }), "value");
					AddSharedPtrClassFunction(, ExcelCell, clear_value);
					lib->add(chaiscript::fun([](CellPtr& a, bool v) { if (a) a->value(v); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a, int v) { if (a) a->value((double)v); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a, float v) { if (a) a->value((double)v); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a, double v) { if (a) a->value((double)v); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a, cweeTime v) { if (a) a->value(v); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a, cweeStr v) { if (a) a->value(v); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a, CellPtr v) { if (a) a->value(*v); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a, cweeStr v, bool inferType) { if (a) a->value(v, inferType); else ThrowIfBadAccess; }), "value");
					lib->add(chaiscript::fun([](CellPtr& a) { if (a) return a->data_type(); else ThrowIfBadAccess; }), "data_type");
					lib->add(chaiscript::fun([](CellPtr& a, CellType type) { if (a) a->data_type(type); else ThrowIfBadAccess; }), "data_type");
					AddSharedPtrClassFunction(, ExcelCell, is_date);
					AddSharedPtrClassFunction(, ExcelCell, column);
					AddSharedPtrClassFunction(, ExcelCell, column_index);
					AddSharedPtrClassFunction(, ExcelCell, row);
					AddSharedPtrClassFunction(, ExcelCell, has_format);
					AddSharedPtrClassFunction(, ExcelCell, clear_format);
					lib->add(chaiscript::fun([](CellPtr& a) { if (a) return a->formula(); else ThrowIfBadAccess; }), "formula");
					lib->add(chaiscript::fun([](CellPtr& a, cweeStr formula) { if (a) a->formula(formula); else ThrowIfBadAccess; }), "formula");
					AddSharedPtrClassFunction(, ExcelCell, clear_formula);
					AddSharedPtrClassFunction(, ExcelCell, has_formula);
					AddSharedPtrClassFunction(, ExcelCell, to_string);
					lib->add(chaiscript::fun([](CellPtr& a) { if (a) return a->is_merged(); else ThrowIfBadAccess; }), "is_merged");
					lib->add(chaiscript::fun([](CellPtr& a) { if (a) return a->error(); else ThrowIfBadAccess; }), "error");
					lib->add(chaiscript::fun([](CellPtr& a, cweeStr v) { if (a) a->error(v); else ThrowIfBadAccess; }), "error");
					lib->add(chaiscript::fun([](CellPtr& a, int column, int row) { if (a) return a->offset(column, row); else ThrowIfBadAccess; }), "offset");
					AddSharedPtrClassFunction(, ExcelCell, worksheet);
					AddSharedPtrClassFunction(, ExcelCell, workbook);
					lib->add(chaiscript::fun([](CellPtr& a, cweeStr to_check) { if (a) return a->check_string(to_check); else ThrowIfBadAccess; }), "check_string");
					AddSharedPtrClassFunction(, ExcelCell, width);
					AddSharedPtrClassFunction(, ExcelCell, height);
				}

                lib->add(chaiscript::fun([](cweeStr filePathToData, cweeStr filePathToCoordinates) {
					// This is the script code called from C# to C++, that requests the underlying data to be used to draw into C#. 

					using TimeSeriesType = cweeUnitValues::cweeUnitPattern; // Time to Measurement .. setting measurement units to "dimensionless" for now.
					using SiteMeasurementType = cweeThreadedMap<cweeStr, TimeSeriesType>; // AL to TimeSeries
					using SiteCollectionType = cweeThreadedMap<cweeStr, SiteMeasurementType>; // AA-01 to SiteMeasurementType

					cweeSharedPtr<SiteCollectionType> SiteCollection = make_cwee_shared<SiteCollectionType>();
					cweeThreadedMap<cweeStr, cweePair<double, double>> Coordinates;

					// Parse Data
					{
						cweeSharedPtr<ExcelWorkbook> workbook;
						workbook = cweeExcel::OpenExcel();
						workbook->load(filePathToData);

						for (auto& worksheet : *workbook) {
							int rowNum = 0;
							cweeStr siteName = worksheet.title();
							AUTO siteMeasurement = SiteCollection->GetPtr(siteName); // gets a shared PTR
							if (siteMeasurement) {
								cweeList<cweeStr> header;
								try {
									AUTO rows = worksheet.rows();
									for (auto& row : *rows) {
										rowNum++;
										if (rowNum <= 1) {
											for (auto& cell : row) {
												header.Append(cell->to_string());
											}
										}
										else {
											int colNum = 0; cweeTime time;
											for (auto& cell : row) {
												colNum++;

												switch (colNum) {
												case 1: break;
												case 2: {
													try {
														time = cell->value<cweeTime>();
													}
													catch (...) {}
													break;
												}
												default: {
													cweeStr& headerForThisCell = header[colNum - 1];

													AUTO TM = siteMeasurement->GetPtr(headerForThisCell);
													if (TM) {
														try {
															TM->AddValue((u64)time, cell->value<double>());
														}
														catch (...) {}
													}
													break;
												}
												}
											}
										}
									}
								}
								catch (...) {}
							}
						}
					}
					// Parse Coordinates
					{
						cweeSharedPtr<ExcelWorkbook> workbook;
						workbook = cweeExcel::OpenExcel();
						workbook->load(filePathToCoordinates);

						for (auto& worksheet : *workbook) {
								try {
									int rowNum = 0;
									AUTO rows = worksheet.rows();
									for (auto& row : *rows) {
										rowNum++;
										if (rowNum <= 2) { /* skip the headers */ }
										else {
											int colNum = 0; cweeStr name; double longitude, latitude;
											for (auto& cell : row) {
												colNum++;

												switch (colNum) {
												case 1: 
													name = cell->value<cweeStr>();
													break;
												case 2: {
													longitude = cell->value<double>();
													break;
												}
												case 3: {
													latitude = cell->value<double>();
													break;
												}
												default: {
													break;
												}
												}
											}
											Coordinates.Emplace(name, cweePair<double, double>(longitude, latitude));
										}
									}
								}
								catch (...) {}
						}
					}

					// This is the overall display
					std::shared_ptr<UI_Grid> displayGrid = make_shared<UI_Grid>(); {
						displayGrid->RowDefinitions.push_back("*");
						displayGrid->ColumnDefinitions.push_back("2*");
						displayGrid->ColumnDefinitions.push_back("1*");
					}
					// This is the pattern display
					std::shared_ptr<UI_Grid> listPatternsContainer = make_shared<UI_Grid>(); {
						listPatternsContainer->ColumnDefinitions.push_back("*");
						listPatternsContainer->RowDefinitions.push_back("*");
					}
					displayGrid->AddChild(var(listPatternsContainer), 0, 1, 1, 1);
					
					// this function is called when the pop-up for each DOT or site is loaded -- and is called EVERY time it is loaded.
					AUTO updatePatterns = [=](UI_Grid& destination, cweeSharedPtr< SiteMeasurementType> measurements) {
						listPatternsContainer->Children.Clear();
						std::map<std::string, chaiscript::Boxed_Value> listPatterns;

						for (auto& meas : *measurements) {
							AUTO measurementName = meas.first;
							AUTO measurementData = meas.second;

							UI_Grid patContainer;
							patContainer.MinHeight = 180;
							patContainer.MinWidth = 600;
							{
								patContainer.AddChild(var(*measurementData), 0, 0, 1, 1);

								AUTO title = UI_TextBlock(measurementName);
								{
									title.HorizontalAlignment = "Left";
									title.VerticalAlignment = "Top";
									title.HorizontalTextAlignment = "Left";
									title.FontSize = 12;
									title.Foreground = UI_Color(cweeRandomFloat(0, 255), cweeRandomFloat(0, 255), cweeRandomFloat(0, 255), cweeRandomFloat(128, 200));
								}

								patContainer.AddChild(var(title), 0, 0, 1, 1);
							}
							listPatterns[measurementName.c_str()] = var(patContainer);
						}

						listPatternsContainer->AddChild(var(listPatterns), 0, 0, 1, 1);
						listPatternsContainer->Update();
					};

					{	
						UI_Map map;
						{
							UI_MapLayer layer;
							{
								for (auto& site : *SiteCollection) {
									UI_MapIcon icon;
									AUTO measurements = site.second;

									double longitude = -1;
									double latitude = -1;

									AUTO coords = Coordinates.TryGetPtr(site.first);
									if (coords) {
										longitude = coords->first;
										latitude = coords->second;
									}

									icon.color = UI_Color(cweeRandomFloat(0,255), cweeRandomFloat(0, 255), cweeRandomFloat(0, 255), cweeRandomFloat(128, 200));
									icon.longitude = longitude;
									icon.latitude = latitude;
									icon.size = 24;
									{
										// set the Tag object, which when "Loaded" will update the righthand side of the figure.
										UI_Grid panel; {
											panel.RowDefinitions.push_back("Auto");
											panel.RowDefinitions.push_back("Auto");
											panel.RowDefinitions.push_back("Auto");
											panel.OnLoaded = var(fun([=]() {
												updatePatterns(*listPatternsContainer, measurements);
											}));
											panel.AddChild(var(site.first), 0, 0, 1, 1);
											panel.AddChild(var(longitude), 1, 0, 1, 1);
											panel.AddChild(var(latitude), 2, 0, 1, 1);
										}
										icon.Tag = var(panel);
									}
									icon.HideOnCollision = false;

									layer.Children.push_back(var(icon));
								}
							}
							map.Layers.push_back(var(layer));
						}
						displayGrid->AddChild(var(map), 0, 0, 1, 1);	
					}
					return displayGrid;                    
                }), "WaterQualityMap");
            }

#undef ThrowIfBadAccess
#undef WorkbookPtr
#undef WorksheetPtr
#undef RangePtr
#undef CellPtr

            return lib;
        }
    }
}