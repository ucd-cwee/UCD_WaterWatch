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