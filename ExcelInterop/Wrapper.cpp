#include <xlnt/xlnt.hpp>
#include "Wrapper.h"

xlnt::workbook& WorkBook(ExcelWorkbook const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::workbook*>(c.data.Get());
};
xlnt::worksheet& WorkSheet(ExcelWorksheet const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::worksheet*>(c.data.Get());
};
cweeStr ExcelWorksheet::ReadCell(cweeStr cellName) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = WorkSheet(*this);
    AUTO cell = worksheet.cell(cellName.c_str());
    AUTO DataType = cell.data_type();

    switch (DataType) {
    /// no value
    case cell::type::empty: {
        break;
    }
    /// value is TRUE or FALSE
    case cell::type::boolean: {
        break;
    }
    /// value is an ISO 8601 formatted date
    case cell::type::date: {
        break;
    }
    /// value is a known error code such as \#VALUE!
    case cell::type::error: {
        break;
    }
    /// value is a string stored in the cell
    case cell::type::inline_string: {
        break;
    }
    /// value is a number
    case cell::type::number: {
        AUTO as_number = cell.value<double>();

        break;
    }
    /// value is a string shared with other cells to save space
    case cell::type::shared_string: {
        break;
    }
    /// value is the string result of a formula
    case cell::type::formula_string: {
        break;
    }
    }

    return "I AM EMPTY";
};
cweeSharedPtr<ExcelWorksheet> ExcelWorkbook::OpenWorksheet(int i) {
    using namespace xlnt;
    xlnt::workbook& workbook = WorkBook(*this);
    workbook.active_sheet(i);
    auto ws = workbook.active_sheet();
    return make_cwee_shared<ExcelWorksheet>(new ExcelWorksheet(cweeSharedPtr<void>(make_cwee_shared<xlnt::worksheet>(ws), [](void* p) { return p; })));
};
cweeSharedPtr<ExcelWorksheet> ExcelWorkbook::OpenWorksheet() {
    using namespace xlnt;
    xlnt::workbook& workbook = WorkBook(*this);
    auto ws = workbook.active_sheet();
    return make_cwee_shared<ExcelWorksheet>(new ExcelWorksheet(cweeSharedPtr<void>(make_cwee_shared<xlnt::worksheet>(ws), [](void* p) { return p; })));
};
cweeSharedPtr<ExcelWorkbook> cweeExcel::OpenExcel(cweeStr filePath) {
    using namespace xlnt;
    auto* workbook = new xlnt::workbook();
    workbook->load(filePath.c_str());
    return make_cwee_shared<ExcelWorkbook>(new ExcelWorkbook(cweeSharedPtr<void>(make_cwee_shared<xlnt::workbook>(workbook), [](void* p) { return p; })));
};
