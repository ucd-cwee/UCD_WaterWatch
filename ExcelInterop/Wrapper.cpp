#include <xlnt/xlnt.hpp>
#include "Wrapper.h"
#include "../ScriptLanguageTesting/units.h"

//#include "../WaterWatchCpp/cweeUnitedValue.h"
//#include "../WaterWatchCpp/enum.h"
//#include "../WaterWatchCpp/cweeUnitPattern.h"
//#include "../WaterWatchCpp/fileSystemH.h"
//#include "../WaterWatchCpp/cweeScheduler.h"
//#include "../WaterWatchCpp/InterpolatedMatrix.h"
//#include "../WaterWatchCpp/Engineering.h"

__forceinline std::shared_ptr<ExcelCellReference> To_ExcelCellReference(xlnt::cell_reference cell) {
    return std::shared_ptr<ExcelCellReference>(new ExcelCellReference(std::static_pointer_cast<void>(std::make_shared<xlnt::cell_reference>(cell))));
};
__forceinline std::shared_ptr<ExcelCell> To_ExcelCell(xlnt::cell cell) {
    return std::shared_ptr<ExcelCell>(new ExcelCell(std::static_pointer_cast<void>(std::make_shared<xlnt::cell>(cell))));
};
__forceinline std::shared_ptr<ExcelRangeReference> To_ExcelRangeReference(xlnt::range_reference  range) {
    return std::shared_ptr<ExcelRangeReference>(new ExcelRangeReference(std::static_pointer_cast<void>(std::make_shared<xlnt::range_reference >(range))));
};
__forceinline std::shared_ptr<ExcelRange> To_ExcelRange(xlnt::range range) {
    return std::shared_ptr<ExcelRange>(new ExcelRange(std::static_pointer_cast<void>(std::make_shared<xlnt::range>(range))));
};
__forceinline std::shared_ptr<ExcelWorksheet> To_ExcelWorkSheet(xlnt::worksheet worksheet) {
    return std::shared_ptr<ExcelWorksheet>(new ExcelWorksheet(std::static_pointer_cast<void>(std::make_shared<xlnt::worksheet>(worksheet))));
};
__forceinline std::shared_ptr<ExcelWorkbook> To_ExcelWorkBook(xlnt::workbook& range) {
    return std::shared_ptr<ExcelWorkbook>(new ExcelWorkbook(std::static_pointer_cast<void>(std::shared_ptr< xlnt::workbook >(&range, [](xlnt::workbook* p) { /* do not delete this ptr, b/c it's a reference. */ }))));
};

__forceinline ExcelRange::cell_vector To_CellVector(xlnt::cell_vector const& cv) {
    ExcelRange::cell_vector out;
    for (auto& x : cv) {
        out.push_back(To_ExcelCell(x));
    }
    return out;
};

__forceinline xlnt::cell_reference& To_CellReference(ExcelCellReference const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::cell_reference*>(c.data.get());
};
__forceinline xlnt::cell& To_Cell(ExcelCell const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::cell*>(c.data.get());
};
__forceinline xlnt::range_reference& To_RangeReference(ExcelRangeReference const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::range_reference*>(c.data.get());
};
__forceinline xlnt::range& To_Range(ExcelRange const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::range*>(c.data.get());
};
__forceinline xlnt::workbook& To_Workbook(ExcelWorkbook const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::workbook*>(c.data.get());
};
__forceinline xlnt::worksheet& To_Worksheet(ExcelWorksheet const& c) {
    using namespace xlnt;
    return *static_cast<xlnt::worksheet*>(c.data.get());
};

#pragma region ExcelRange

void ExcelRange::clear_cells() {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    ThisRange.clear_cells();
};
ExcelRange::cell_vector ExcelRange::vector(std::size_t n) {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange.vector(n));
};
const ExcelRange::cell_vector ExcelRange::vector(std::size_t n) const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange.vector(n));
};
std::shared_ptr<ExcelCell> ExcelRange::cell(std::shared_ptr<ExcelCellReference> ref) {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    xlnt::cell_reference& CellRef = To_CellReference(*ref);
    return To_ExcelCell(ThisRange.cell(CellRef));
};
const std::shared_ptr<ExcelCell> ExcelRange::cell(std::shared_ptr<ExcelCellReference> ref) const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    xlnt::cell_reference& CellRef = To_CellReference(*ref);
    return To_ExcelCell(ThisRange.cell(CellRef));
};
std::shared_ptr<ExcelWorksheet> ExcelRange::target_worksheet() const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_ExcelWorkSheet(ThisRange.target_worksheet());
};
std::shared_ptr<ExcelRangeReference> ExcelRange::ref() const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_ExcelRangeReference(ThisRange.reference());
};
std::size_t ExcelRange::length() const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return ThisRange.length();
};
bool ExcelRange::contains(std::shared_ptr<ExcelCellReference> ref) {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    xlnt::cell_reference& CellRef = To_CellReference(*ref);
    return ThisRange.contains(CellRef);
};
ExcelRange::cell_vector ExcelRange::front() {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange.front());
};
const ExcelRange::cell_vector ExcelRange::front() const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange.front());
};
ExcelRange::cell_vector ExcelRange::back() {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange.back());
};
const ExcelRange::cell_vector ExcelRange::back() const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange.back());
};
void ExcelRange::apply(std::function<void(std::shared_ptr<ExcelCell>)> f) {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    ThisRange.apply([f](xlnt::cell Cell) {
        f(To_ExcelCell(Cell));
    });
};
ExcelRange::cell_vector ExcelRange::operator[](std::size_t n) {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange[n]);
};
const ExcelRange::cell_vector ExcelRange::operator[](std::size_t n) const {
    using namespace xlnt;
    xlnt::range& ThisRange = To_Range(*this);
    return To_CellVector(ThisRange[n]);
};

#pragma endregion

#pragma region ExcelCell
std::string ExcelCell::address() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    auto str = ThisCell.reference().to_string();
    return str.c_str();
};
bool ExcelCell::has_value() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.has_value();
};
void ExcelCell::clear_value() {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    ThisCell.clear_value();
};
void ExcelCell::value(std::nullptr_t) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    ThisCell.value(nullptr);
};
void ExcelCell::value(bool boolean_value) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    ThisCell.value(boolean_value);
};
void ExcelCell::value(double float_value) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    ThisCell.value(float_value);
};
void ExcelCell::value(const GL::datetime& date_value) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    datetime dt = datetime(
        xlnt::date(
            date_value.tm_year()+1900, 
            date_value.tm_mon()+1, 
            date_value.tm_mday()
        ), 
        

        xlnt::time(
            date_value.tm_hour(), 
            date_value.tm_min(), 
            date_value.tm_sec(), 
            (float)((GL::microsecond)(GL::millisecond(date_value.tm_fractionalsec())))
        )
    );    
    ThisCell.value(dt);
};
void ExcelCell::value(const std::string& string_value) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    ThisCell.value(std::string(string_value.c_str()));
};
void ExcelCell::value(const ExcelCell& other_cell) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    xlnt::cell& OtherCell = To_Cell(other_cell);
    ThisCell.value(OtherCell);
};
void ExcelCell::value(const std::string& string_value, bool infer_type) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    ThisCell.value(string_value.c_str(), infer_type);
};
ExcelCell::type ExcelCell::data_type() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ExcelCell::type::_from_integral(static_cast<int>(ThisCell.data_type()));
};
void ExcelCell::data_type(ExcelCell::type t) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    ThisCell.data_type((xlnt::cell::type)(t._to_integral()));
};
bool ExcelCell::is_date() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.is_date();
};
std::shared_ptr<ExcelCellReference> ExcelCell::reference() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return To_ExcelCellReference(ThisCell.reference());
};
int ExcelCell::column() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.column().index;
};
int ExcelCell::column_index() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.column_index();
};
int ExcelCell::row() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.row();
};
std::pair<int, int> ExcelCell::anchor() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.anchor();
};
bool ExcelCell::has_format() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.has_format();
};
void ExcelCell::clear_format() {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.clear_format();
};
std::string ExcelCell::formula() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.formula().c_str();
};
void ExcelCell::formula(const std::string& formula) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.formula(formula.c_str());
};
void ExcelCell::clear_formula() {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.clear_formula();
};
bool ExcelCell::has_formula() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.has_formula();
};
std::string ExcelCell::to_string() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.to_string().c_str();
};
bool ExcelCell::is_merged() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.is_merged();
};
std::string ExcelCell::error() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.error().c_str();
};
void ExcelCell::error(const std::string& error) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.error(error.c_str());
};
std::shared_ptr<ExcelCell> ExcelCell::offset(int column, int row) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return To_ExcelCell(ThisCell.offset(column, row));
};
std::shared_ptr<ExcelWorksheet> ExcelCell::worksheet() {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return To_ExcelWorkSheet(ThisCell.worksheet());
};
const std::shared_ptr<ExcelWorksheet> ExcelCell::worksheet() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return To_ExcelWorkSheet(ThisCell.worksheet());
};
std::shared_ptr<ExcelWorkbook> ExcelCell::workbook() {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return To_ExcelWorkBook(ThisCell.workbook()); // NOT COPY
};
std::shared_ptr<ExcelWorkbook> ExcelCell::workbook() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return To_ExcelWorkBook(ThisCell.workbook()); // NOT COPY
};
std::string ExcelCell::check_string(const std::string& to_check) {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.check_string(to_check.c_str()).c_str();
};
double ExcelCell::width() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.width();
};
double ExcelCell::height() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.height();
};
template <> bool ExcelCell::value<bool>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.value<bool>();
};
template <> int ExcelCell::value<int>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.value<int>();
};
template <> unsigned int ExcelCell::value<unsigned int>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.value<unsigned int>();
};
template <> long long int ExcelCell::value<long long int>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.value<long long int>();
};
template <> unsigned long long ExcelCell::value<unsigned long long int>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.value<unsigned long long>();
};
template <> float ExcelCell::value<float>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.value<float>();
};
template <> double ExcelCell::value<double>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    return ThisCell.value<double>();
};
template <> GL::datetime ExcelCell::value<GL::datetime>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    xlnt::datetime dt = ThisCell.value<datetime>();
    return GL::datetime::make_time(dt.year, dt.month, dt.day, dt.hour, dt.minute, (float)((GL::second)(GL::second(dt.second) + GL::microsecond(dt.microsecond))));
};
template <> std::string ExcelCell::value<std::string>() const {
    using namespace xlnt;
    xlnt::cell& ThisCell = To_Cell(*this);
    std::string str = ThisCell.value<std::string>();
    return str.c_str();
};

#pragma endregion

#pragma region ExcelCellReference
bool ExcelCellReference::column_absolute() const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return ThisRef.column_absolute();
};
void ExcelCellReference::column_absolute(bool absolute_column) {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    ThisRef.column_absolute(absolute_column);
};
bool ExcelCellReference::row_absolute() const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return ThisRef.row_absolute();
};
void ExcelCellReference::row_absolute(bool absolute_row) {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    ThisRef.row_absolute(absolute_row);
};
std::shared_ptr< ExcelCellReference> ExcelCellReference::make_absolute(bool absolute_column, bool absolute_row) {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return To_ExcelCellReference(ThisRef.make_absolute(absolute_column, absolute_row));
};
int ExcelCellReference::column() const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return ThisRef.column().index;
};
void ExcelCellReference::column(const std::string& column_string) {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    ThisRef.column(column_string.c_str());
};
int ExcelCellReference::column_index() const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return ThisRef.column_index();
};
void ExcelCellReference::column_index(int column) {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    ThisRef.column_index(column);
};
int ExcelCellReference::row() const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return ThisRef.row();
};
void ExcelCellReference::row(int row) {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    ThisRef.row(row);
};
std::shared_ptr< ExcelCellReference> ExcelCellReference::make_offset(int column_offset, int row_offset) const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return To_ExcelCellReference(ThisRef.make_offset(column_offset, row_offset));
};
std::string ExcelCellReference::to_string() const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return ThisRef.to_string().c_str();
};
std::shared_ptr< ExcelRangeReference> ExcelCellReference::to_range() const {
    using namespace xlnt;
    xlnt::cell_reference& ThisRef = To_CellReference(*this);
    return To_ExcelRangeReference(ThisRef.to_range());
};
#pragma endregion

#pragma region ExcelRangeReference
std::shared_ptr<ExcelRangeReference> ExcelRangeReference::make_absolute() {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return To_ExcelRangeReference(range_reference::make_absolute(ThisRef));
};
bool ExcelRangeReference::is_single_cell() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return ThisRef.is_single_cell();
};
std::size_t ExcelRangeReference::width() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return ThisRef.width();
};
std::size_t ExcelRangeReference::height() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return ThisRef.height();
};
std::shared_ptr< ExcelCellReference > ExcelRangeReference::top_left() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return To_ExcelCellReference(ThisRef.top_left());
};
std::shared_ptr< ExcelCellReference > ExcelRangeReference::top_right() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return To_ExcelCellReference(ThisRef.top_right());
};
std::shared_ptr< ExcelCellReference > ExcelRangeReference::bottom_left() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return To_ExcelCellReference(ThisRef.bottom_left());
};
std::shared_ptr< ExcelCellReference > ExcelRangeReference::bottom_right() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return To_ExcelCellReference(ThisRef.bottom_right());
};
std::shared_ptr<ExcelRangeReference> ExcelRangeReference::make_offset(int column_offset, int row_offset) const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return To_ExcelRangeReference(ThisRef.make_offset(column_offset, row_offset));
};
std::string ExcelRangeReference::to_string() const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return ThisRef.to_string().c_str();
};
bool ExcelRangeReference::contains(std::shared_ptr< ExcelCellReference > ref) const {
    using namespace xlnt;
    xlnt::range_reference& ThisRef = To_RangeReference(*this);
    return ThisRef.contains(To_CellReference(*ref));
};
#pragma endregion

#pragma region ExcelWorksheet
std::size_t ExcelWorksheet::id() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.id();
};
void ExcelWorksheet::id(std::size_t id) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.id(id);
};
std::string ExcelWorksheet::title() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.title().c_str();
};
void ExcelWorksheet::title(std::string const& title) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.title(title.c_str());
};
void ExcelWorksheet::freeze_panes(ExcelCell const& top_left_cell) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    xlnt::cell& cell = To_Cell(top_left_cell);
    worksheet.freeze_panes(cell);
};
void ExcelWorksheet::unfreeze_panes() {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.unfreeze_panes();
};
bool ExcelWorksheet::has_frozen_panes() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_frozen_panes();
};
std::shared_ptr<ExcelCell> ExcelWorksheet::cell(int column, int row) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    xlnt::cell thisCell = worksheet.cell(column, row);
    return To_ExcelCell(thisCell);
};
std::shared_ptr<ExcelCell> ExcelWorksheet::cell(int column, int row) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    xlnt::cell thisCell = worksheet.cell(column, row);
    return To_ExcelCell(thisCell);
};
std::shared_ptr<ExcelCell> ExcelWorksheet::cell(std::string const& address) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    xlnt::cell thisCell = worksheet.cell(address.c_str());
    return To_ExcelCell(thisCell);
};
std::shared_ptr<ExcelRange> ExcelWorksheet::range(std::string reference_string) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    auto thisObj = worksheet.range(reference_string.c_str());
    return To_ExcelRange(thisObj);
};
const std::shared_ptr<ExcelRange> ExcelWorksheet::range(std::string reference_string) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    auto thisObj = worksheet.range(reference_string.c_str());
    return To_ExcelRange(thisObj);
};
std::shared_ptr<ExcelRange> ExcelWorksheet::rows(bool skip_null) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    auto thisObj = worksheet.rows(skip_null);
    return To_ExcelRange(thisObj);
};
const std::shared_ptr<ExcelRange> ExcelWorksheet::rows(bool skip_null) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    auto thisObj = worksheet.rows(skip_null);
    return To_ExcelRange(thisObj);
};
std::shared_ptr<ExcelRange> ExcelWorksheet::columns(bool skip_null) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    auto thisObj = worksheet.columns(skip_null);
    return To_ExcelRange(thisObj);
};
const std::shared_ptr<ExcelRange> ExcelWorksheet::columns(bool skip_null) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    auto thisObj = worksheet.columns(skip_null);
    return To_ExcelRange(thisObj);
};
void ExcelWorksheet::clear_row(int row) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.clear_row(row);
};
void ExcelWorksheet::insert_rows(int row, std::uint32_t amount) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.insert_rows(row, amount);
};
void ExcelWorksheet::insert_columns(int column, std::uint32_t amount) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.insert_columns(column, amount);
};
void ExcelWorksheet::delete_rows(int row, std::uint32_t amount) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.delete_rows(row, amount);
};
void ExcelWorksheet::delete_columns(int column, std::uint32_t amount) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.delete_columns(column, amount);
};
double ExcelWorksheet::column_width(int column) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.column_width(column);
};
double ExcelWorksheet::row_height(int row) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.row_height(row);
};
void ExcelWorksheet::create_named_range(std::string name, std::string reference_string) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.create_named_range(name.c_str(), reference_string.c_str());
};
bool ExcelWorksheet::has_named_range(std::string name) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_named_range(name.c_str());
};
std::shared_ptr<ExcelRange> ExcelWorksheet::named_range(std::string name) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return To_ExcelRange(worksheet.named_range(name.c_str()));
};
std::shared_ptr<ExcelRange> ExcelWorksheet::named_range(std::string name) const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.named_range(name.c_str());
};
void ExcelWorksheet::remove_named_range(std::string name) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.remove_named_range(name.c_str());
};
int ExcelWorksheet::lowest_row() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.lowest_row();
};
int ExcelWorksheet::lowest_row_or_props() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.lowest_row_or_props();
};
int ExcelWorksheet::highest_row() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.highest_row();
};
int ExcelWorksheet::highest_row_or_props() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.highest_row_or_props();
};
int ExcelWorksheet::next_row() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.next_row();
};
int ExcelWorksheet::lowest_column() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.lowest_column().index;
};
int ExcelWorksheet::lowest_column_or_props() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.lowest_column_or_props().index;
};
int ExcelWorksheet::highest_column() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.highest_column().index;
};
int ExcelWorksheet::highest_column_or_props() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.highest_column_or_props().index;
};
void ExcelWorksheet::merge_cells(std::string reference_string) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.merge_cells(reference_string.c_str());
};
void ExcelWorksheet::unmerge_cells(std::string reference_string) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.unmerge_cells(reference_string.c_str());
};
bool ExcelWorksheet::has_page_setup() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_page_setup();
};
void ExcelWorksheet::auto_filter(std::string range_string) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.auto_filter(range_string.c_str());
};
void ExcelWorksheet::auto_filter(const ExcelRange& range) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.auto_filter(To_Range(range));
};
void ExcelWorksheet::clear_auto_filter() {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.clear_auto_filter();
};
bool ExcelWorksheet::has_auto_filter() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_auto_filter();
};
void ExcelWorksheet::reserve(std::size_t n) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.reserve(n);
};
void ExcelWorksheet::print_title_rows(int start, int end) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.print_title_rows(start, end);
};
void ExcelWorksheet::print_title_cols(int start, int end) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.print_title_cols(start, end);
};
bool ExcelWorksheet::has_print_titles() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_print_titles();
};
void ExcelWorksheet::clear_print_titles() {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.clear_print_titles();
};
void ExcelWorksheet::print_area(std::string print_area) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.print_area(print_area.c_str());
};
void ExcelWorksheet::clear_print_area() {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.clear_print_area();
};
bool ExcelWorksheet::has_print_area() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_print_area();
};
bool ExcelWorksheet::has_view() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_view();
};
bool ExcelWorksheet::has_active_cell() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_active_cell();
};
void ExcelWorksheet::clear_page_breaks() {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.clear_page_breaks();
};
std::vector<int> ExcelWorksheet::page_break_rows() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    std::vector<int> out;
    for (auto& x : worksheet.page_break_rows()) {
        out.push_back(x);
    }
    return out;
};
void ExcelWorksheet::page_break_at_row(int row) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    worksheet.page_break_at_row(row);
};
std::vector<int> ExcelWorksheet::page_break_columns() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    std::vector<int> out; 
    for (auto& x : worksheet.page_break_columns()) {
        out.push_back(x.index);
    }
    return out;
};
void ExcelWorksheet::page_break_at_column(int column) {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.page_break_at_column(column);
};
bool ExcelWorksheet::has_drawing() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.has_drawing();
};
bool ExcelWorksheet::is_empty() const {
    using namespace xlnt;
    xlnt::worksheet& worksheet = To_Worksheet(*this);
    return worksheet.is_empty();
};
#pragma endregion

#pragma region ExcelWorkbook
void ExcelWorkbook::calculate_now() {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::create_sheet() {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.create_sheet());
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::create_sheet(std::size_t index) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.create_sheet(index));
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::copy_sheet(std::shared_ptr<ExcelWorksheet> worksheet) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.copy_sheet(To_Worksheet(*worksheet)));
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::copy_sheet(std::shared_ptr<ExcelWorksheet> worksheet, std::size_t index) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.copy_sheet(To_Worksheet(*worksheet), index));
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::active_sheet() {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.active_sheet());
};
void ExcelWorkbook::active_sheet(std::size_t index) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.active_sheet(index);
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::sheet_by_title(const std::string& title) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.sheet_by_title(title.c_str()));
};
const std::shared_ptr<ExcelWorksheet> ExcelWorkbook::sheet_by_title(const std::string& title) const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.sheet_by_title(title.c_str()));
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::sheet_by_index(std::size_t index) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.sheet_by_index(index));
};
const std::shared_ptr<ExcelWorksheet> ExcelWorkbook::sheet_by_index(std::size_t index) const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.sheet_by_index(index));
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::sheet_by_id(std::size_t id) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.sheet_by_id(id));
};
const std::shared_ptr<ExcelWorksheet> ExcelWorkbook::sheet_by_id(std::size_t id) const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return To_ExcelWorkSheet(workbook.sheet_by_id(id));
};
bool ExcelWorkbook::sheet_hidden_by_index(std::size_t index) const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return workbook.sheet_hidden_by_index(index);
};
bool ExcelWorkbook::contains(const std::string& title) const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return workbook.contains(title.c_str());
};
std::size_t ExcelWorkbook::index(std::shared_ptr<ExcelWorksheet> worksheet) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return workbook.index(To_Worksheet(*worksheet));
};
void ExcelWorkbook::remove_sheet(std::shared_ptr<ExcelWorksheet> worksheet) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.remove_sheet(To_Worksheet(*worksheet));
};
void ExcelWorkbook::clear() {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.clear();
};
void ExcelWorkbook::apply_to_cells(std::function<void(std::shared_ptr<ExcelCell>)> f) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.apply_to_cells([f](xlnt::cell Cell) {
        f(To_ExcelCell(Cell));
    });
};
std::vector<std::string> ExcelWorkbook::sheet_titles() const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    std::vector<std::string> strs;
    for (auto& x : workbook.sheet_titles()) {
        strs.push_back(x.c_str());
    }
    return strs;
};
std::size_t ExcelWorkbook::sheet_count() const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return workbook.sheet_count();
};
bool ExcelWorkbook::has_title() const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return workbook.has_title();
};
std::string ExcelWorkbook::title() const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    return workbook.title().c_str();
};
void ExcelWorkbook::title(const std::string& title) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.title(title.c_str());
};
void ExcelWorkbook::abs_path(const std::string& path) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.abs_path(path.c_str());
};
void ExcelWorkbook::save(const std::string& filename) const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.save(filename.c_str());
};
void ExcelWorkbook::save(const std::string& filename, const std::string& password) const {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.save(filename.c_str(), password.c_str());
};
void ExcelWorkbook::load(const std::string& filename) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.load(filename.c_str());
};
void ExcelWorkbook::load_limited(const std::string& filename, const std::string& sheetTitle) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.load_limited(filename.c_str(), sheetTitle.c_str());
};
void ExcelWorkbook::load(const std::string& filename, const std::string& password) {
    using namespace xlnt;
    xlnt::workbook& workbook = To_Workbook(*this);
    workbook.load(filename.c_str(), password.c_str());
};
std::shared_ptr<ExcelWorksheet> ExcelWorkbook::operator[](std::size_t n) {
    return sheet_by_index(n);
};
const std::shared_ptr<ExcelWorksheet> ExcelWorkbook::operator[](std::size_t n) const {
    return sheet_by_index(n);
};

#pragma endregion

#pragma region Excel
std::shared_ptr<ExcelWorkbook> cweeExcel::OpenExcel(std::string filePath, std::string sheetTitle) {
    using namespace xlnt;
    auto* workbook = new xlnt::workbook();
    workbook->load(filePath.c_str());
    // workbook->load_limited(filePath.c_str(), sheetTitle.c_str());
    for (int i = workbook->sheet_count() - 1; i >= 0; --i) {
        auto worksheet = workbook->sheet_by_index(i);
        if (worksheet.title().c_str() != sheetTitle) {
            workbook->remove_sheet(worksheet);
        }
    }
    return std::shared_ptr<ExcelWorkbook>(new ExcelWorkbook(std::static_pointer_cast<void>(std::shared_ptr<xlnt::workbook>(workbook))));
};
std::shared_ptr<ExcelWorkbook> cweeExcel::OpenExcel(std::string filePath) {
    using namespace xlnt;
    auto* workbook = new xlnt::workbook();
    workbook->load(filePath.c_str());
    return std::shared_ptr<ExcelWorkbook>(new ExcelWorkbook(std::static_pointer_cast<void>(std::shared_ptr<xlnt::workbook>(workbook))));
};
std::shared_ptr<ExcelWorkbook> cweeExcel::OpenExcel() {
    using namespace xlnt;
    auto* workbook = new xlnt::workbook();
    return std::shared_ptr<ExcelWorkbook>(new ExcelWorkbook(std::static_pointer_cast<void>(std::shared_ptr<xlnt::workbook>(workbook))));
};
#pragma endregion