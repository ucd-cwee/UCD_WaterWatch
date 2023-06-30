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