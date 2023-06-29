
#if 0

#pragma once
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/Units.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/cweeTime.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/enum.h"



BETTER_ENUM(CellType, uint8_t, empty, boolean, date, error, inline_string, number, shared_string, formula_string);


class ExcelWorksheet;
class ExcelWorkbook;
class ExcelCell;

class ExcelCellReference {
public:
    ExcelCellReference() : data() {};
    ExcelCellReference(cweeSharedPtr<void> d) : data(d) {};
    cweeSharedPtr<void> data; // could be anything! represents the worksheet ACTUALLY

};

class ExcelCell {
public:
    using type = CellType;

	ExcelCell() : data() {};
	ExcelCell(cweeSharedPtr<void> d) : data(d) {};
	cweeSharedPtr<void> data; // could be anything! represents the worksheet ACTUALLY

    /// <summary>
    /// Returns true if value has been set and has not been cleared using cell::clear_value().
    /// </summary>
    bool has_value() const;

    /// <summary>
    /// Returns the value of this cell as an instance of type T.
    /// Overloads exist for most C++ fundamental types like bool, int, etc. as well
    /// as for std::string and xlnt datetime types: date, time, datetime, and timedelta.
    /// </summary>
    template <typename T>
    T value() const;

    /// <summary>
    /// Makes this cell have a value of type null.
    /// All other cell attributes are retained.
    /// </summary>
    void clear_value();

    /// <summary>
    /// Sets the type of this cell to null.
    /// </summary>
    void value(std::nullptr_t);

    /// <summary>
    /// Sets the value of this cell to the given boolean value.
    /// </summary>
    void value(bool boolean_value);

    /// <summary>
    /// Sets the value of this cell to the given value.
    /// </summary>
    void value(double float_value);

    /// <summary>
    /// Sets the value of this cell to the given value.
    /// </summary>
    void value(const cweeTime& date_value);

    /// <summary>
    /// Sets the value of this cell to the given value.
    /// </summary>
    void value(const cweeStr& string_value);

    /// <summary>
    /// Sets the value and formatting of this cell to that of other_cell.
    /// </summary>
    void value(const ExcelCell& other_cell);

    /// <summary>
    /// Analyzes string_value to determine its type, convert it to that type,
    /// and set the value of this cell to that converted value.
    /// </summary>
    void value(const cweeStr& string_value, bool infer_type);

    /// <summary>
    /// Returns the type of this cell.
    /// </summary>
    type data_type() const;

    /// <summary>
    /// Sets the type of this cell. This should usually be done indirectly
    /// by setting the value of the cell to a value of that type.
    /// </summary>
    void data_type(type t);

    // properties
    /// <summary>
    /// Returns true iff this cell's number format matches a date format.
    /// </summary>
    bool is_date() const;

    // position
    /// <summary>
    /// Returns a cell_reference that points to the location of this cell.
    /// </summary>
    cweeSharedPtr<ExcelCellReference> reference() const;

    /// <summary>
    /// Returns the column of this cell.
    /// </summary>
    int column() const;

    /// <summary>
    /// Returns the numeric index (A == 1) of the column of this cell.
    /// </summary>
    int column_index() const;

    /// <summary>
    /// Returns the row of this cell.
    /// </summary>
    int row() const;

    /// <summary>
    /// Returns the location of this cell as an ordered pair (left, top).
    /// </summary>
    std::pair<int, int> anchor() const;

    // format

    /// <summary>
    /// Returns true if this cell has had a format applied to it.
    /// </summary>
    bool has_format() const;

    /// <summary>
    /// Removes the cell-level formatting from this cell.
    /// This doesn't affect the style that may also be applied to the cell.
    /// Throws an invalid_attribute exception if no format is applied.
    /// </summary>
    void clear_format();

    // formula

    /// <summary>
    /// Returns the string representation of the formula applied to this cell.
    /// </summary>
    cweeStr formula() const;

    /// <summary>
    /// Sets the formula of this cell to the given value.
    /// This formula string should begin with '='.
    /// </summary>
    void formula(const cweeStr& formula);

    /// <summary>
    /// Removes the formula from this cell. After this is called, has_formula() will return false.
    /// </summary>
    void clear_formula();

    /// <summary>
    /// Returns true if this cell has had a formula applied to it.
    /// </summary>
    bool has_formula() const;

    // printing

    /// <summary>
    /// Returns a string representing the value of this cell. If the data type is not a string,
    /// it will be converted according to the number format.
    /// </summary>
    cweeStr to_string() const;

    // merging

    /// <summary>
    /// Returns true iff this cell has been merged with one or more
    /// surrounding cells.
    /// </summary>
    bool is_merged() const;

    /// <summary>
    /// Makes this a merged cell iff merged is true.
    /// Generally, this shouldn't be called directly. Instead,
    /// use worksheet::merge_cells on its parent worksheet.
    /// </summary>
    void merged(bool merged);

    /// <summary>
    /// Returns the error string that is stored in this cell.
    /// </summary>
    cweeStr error() const;

    /// <summary>
    /// Directly assigns the value of this cell to be the given error.
    /// </summary>
    void error(const cweeStr& error);

    /// <summary>
    /// Returns a cell from this cell's parent workbook at
    /// a relative offset given by the parameters.
    /// </summary>
    cweeSharedPtr<ExcelCell> offset(int column, int row);

    /// <summary>
    /// Returns the worksheet that owns this cell.
    /// </summary>
    cweeSharedPtr<ExcelWorksheet> worksheet();

    /// <summary>
    /// Returns the worksheet that owns this cell.
    /// </summary>
    const cweeSharedPtr<ExcelWorksheet> worksheet() const;

    /// <summary>
    /// Returns the workbook of the worksheet that owns this cell.
    /// </summary>
    cweeSharedPtr<ExcelWorkbook> workbook();

    /// <summary>
    /// Returns the workbook of the worksheet that owns this cell.
    /// </summary>
    cweeSharedPtr<ExcelWorkbook> workbook() const;

    /// <summary>
    /// Returns to_check after verifying and fixing encoding, size, and illegal characters.
    /// </summary>
    cweeStr check_string(const cweeStr& to_check);

    /// <summary>
    /// Returns the width of this cell in pixels.
    /// </summary>
    double width() const;

    /// <summary>
    /// Returns the height of this cell in pixels.
    /// </summary>
    double height() const;
};

template <>
bool ExcelCell::value<bool>() const;

template <>
int ExcelCell::value<int>() const;

template <>
unsigned int ExcelCell::value<unsigned int>() const;

template <>
long long int ExcelCell::value<long long int>() const;

template <>
unsigned long long ExcelCell::value<unsigned long long int>() const;

template <>
float ExcelCell::value<float>() const;

template <>
double ExcelCell::value<double>() const;

template <>
cweeTime ExcelCell::value<cweeTime>() const;

template <>
cweeStr ExcelCell::value<cweeStr>() const;




class ExcelRange {
public:
    ExcelRange() : data() {};
    ExcelRange(cweeSharedPtr<void> d) : data(d) {};
    cweeSharedPtr<void> data; // could be anything! represents the worksheet ACTUALLY

};

class ExcelWorksheet {
public:
	ExcelWorksheet() : data() {};
	ExcelWorksheet(cweeSharedPtr<void> d) : data(d) {};
	cweeSharedPtr<void> data; // could be anything! represents the worksheet ACTUALLY

public:
    /// <summary>
    /// Returns the unique numeric identifier of this worksheet. This will sometimes but not necessarily
    /// be the index of the worksheet in the workbook.
    /// </summary>
    std::size_t id() const;

    /// <summary>
    /// Set the unique numeric identifier. The id defaults to the lowest unused id in the workbook
    /// so this should not be called without a good reason.
    /// </summary>
    void id(std::size_t id);

    /// <summary>
    /// Returns the title of this sheet.
    /// </summary>
    cweeStr title() const;

    /// <summary>
    /// Sets the title of this sheet.
    /// </summary>
    void title(cweeStr title);

    // freeze panes
    /// <summary>
    /// Freeze panes above and to the left of top_left_cell.
    /// </summary>
    void freeze_panes(ExcelCell const& top_left_cell);

    /// <summary>
    /// Remove frozen panes. The data in those panes will be unaffected--this affects only the view.
    /// </summary>
    void unfreeze_panes();

    /// <summary>
    /// Returns true if this sheet has a frozen row, frozen column, or both.
    /// </summary>
    bool has_frozen_panes() const;

    // container
    /// <summary>
    /// Returns the cell at the given column and row. If the cell doesn't exist, it
    /// will be initialized to null before being returned.
    /// </summary>
    cweeSharedPtr<ExcelCell> cell(int column, int row);

    /// <summary>
    /// Returns the cell at the given column and row. If the cell doesn't exist, an
    /// invalid_parameter exception will be thrown.
    /// </summary>
    cweeSharedPtr<ExcelCell> cell(int column, int row) const;

    /// <summary>
    /// Returns the range defined by reference string. If reference string is the name of
    /// a previously-defined named range in the sheet, it will be returned.
    /// </summary>
    cweeSharedPtr<ExcelRange> range(cweeStr reference_string);

    /// <summary>
    /// Returns the range defined by reference string. If reference string is the name of
    /// a previously-defined named range in the sheet, it will be returned.
    /// </summary>
    const cweeSharedPtr<ExcelRange> range(cweeStr reference_string) const;

    /// <summary>
    /// Returns a range encompassing all cells in this sheet which will
    /// be iterated upon in row-major order. If skip_null is true (default),
    /// empty rows and cells will be skipped during iteration of the range.
    /// </summary>
    cweeSharedPtr<ExcelRange> rows(bool skip_null = true);

    /// <summary>
    /// Returns a range encompassing all cells in this sheet which will
    /// be iterated upon in row-major order. If skip_null is true (default),
    /// empty rows and cells will be skipped during iteration of the range.
    /// </summary>
    const cweeSharedPtr<ExcelRange> rows(bool skip_null = true) const;

    /// <summary>
    /// Returns a range ecompassing all cells in this sheet which will
    /// be iterated upon in column-major order. If skip_null is true (default),
    /// empty columns and cells will be skipped during iteration of the range.
    /// </summary>
    cweeSharedPtr<ExcelRange> columns(bool skip_null = true);

    /// <summary>
    /// Returns a range ecompassing all cells in this sheet which will
    /// be iterated upon in column-major order. If skip_null is true (default),
    /// empty columns and cells will be skipped during iteration of the range.
    /// </summary>
    const cweeSharedPtr<ExcelRange> columns(bool skip_null = true) const;

    /// <summary>
    /// Clears memory used by all cells in the given row.
    /// </summary>
    void clear_row(int row);

    /// <summary>
    /// Insert empty rows before the given row index
    /// </summary>
    void insert_rows(int row, std::uint32_t amount);

    /// <summary>
    /// Insert empty columns before the given column index
    /// </summary>
    void insert_columns(int column, std::uint32_t amount);

    /// <summary>
    /// Delete rows before the given row index
    /// </summary>
    void delete_rows(int row, std::uint32_t amount);

    /// <summary>
    /// Delete columns before the given column index
    /// </summary>
    void delete_columns(int column, std::uint32_t amount);

    // properties

    /// <summary>
    /// Calculates the width of the given column. This will be the default column width if
    /// a custom width is not set on this column's column_properties.
    /// </summary>
    double column_width(int column) const;

    /// <summary>
    /// Calculate the height of the given row. This will be the default row height if
    /// a custom height is not set on this row's row_properties.
    /// </summary>
    double row_height(int row) const;

    // named range

    /// <summary>
    /// Creates a new named range with the given name encompassing the string representing a range.
    /// </summary>
    void create_named_range(cweeStr name, cweeStr reference_string);

    /// <summary>
    /// Returns true if this worksheet contains a named range with the given name.
    /// </summary>
    bool has_named_range(cweeStr name) const;

    /// <summary>
    /// Returns the named range with the given name. Throws key_not_found
    /// exception if the named range doesn't exist.
    /// </summary>
    cweeSharedPtr<ExcelRange> named_range(cweeStr name);

    /// <summary>
    /// Returns the named range with the given name. Throws key_not_found
    /// exception if the named range doesn't exist.
    /// </summary>
    cweeSharedPtr<ExcelRange> named_range(cweeStr name) const;

    /// <summary>
    /// Removes a named range with the given name.
    /// </summary>
    void remove_named_range(cweeStr name);

    // extents

    /// <summary>
    /// Returns the row of the first non-empty cell in the worksheet.
    /// </summary>
    int lowest_row() const;

    /// <summary>
    /// Returns the row of the first non-empty cell or lowest row with properties in the worksheet.
    /// </summary>
    int lowest_row_or_props() const;

    /// <summary>
    /// Returns the row of the last non-empty cell in the worksheet.
    /// </summary>
    int highest_row() const;

    /// <summary>
    /// Returns the row of the last non-empty cell or highest row with properties in the worksheet.
    /// </summary>
    int highest_row_or_props() const;

    /// <summary>
    /// Returns the row directly below the last non-empty cell in the worksheet.
    /// </summary>
    int next_row() const;

    /// <summary>
    /// Returns the column of the first non-empty cell in the worksheet.
    /// </summary>
    int lowest_column() const;

    /// <summary>
    /// Returns the column of the first non-empty cell or lowest column with properties in the worksheet.
    /// </summary>
    int lowest_column_or_props() const;

    /// <summary>
    /// Returns the column of the last non-empty cell in the worksheet.
    /// </summary>
    int highest_column() const;

    /// <summary>
    /// Returns the column of the last non-empty cell or highest column with properties in the worksheet.
    /// </summary>
    int highest_column_or_props() const;

    // cell merge

    /// <summary>
    /// Merges the cells within the range represented by the given string.
    /// </summary>
    void merge_cells(cweeStr reference_string);


    /// <summary>
    /// Removes the merging of the cells in the range represented by the given string.
    /// </summary>
    void unmerge_cells(cweeStr reference_string);

    // operators

    // page

    /// <summary>
    /// Returns true if this worksheet has a page setup.
    /// </summary>
    bool has_page_setup() const;

    // auto filter

    /// <summary>
    /// Sets the auto-filter of this sheet to the range defined by range_string.
    /// </summary>
    void auto_filter(cweeStr range_string);

    /// <summary>
    /// Sets the auto-filter of this sheet to the given range.
    /// </summary>
    void auto_filter(const ExcelRange& range);

    /// <summary>
    /// Clear the auto-filter from this sheet.
    /// </summary>
    void clear_auto_filter();

    /// <summary>
    /// Returns true if this sheet has an auto-filter set.
    /// </summary>
    bool has_auto_filter() const;

    /// <summary>
    /// Reserve n rows. This can be optionally called before adding many rows
    /// to improve performance.
    /// </summary>
    void reserve(std::size_t n);

    /// <summary>
    /// Sets rows to repeat at top during printing.
    /// </summary>
    void print_title_rows(int start, int end);

    /// <summary>
    /// Sets columns to repeat at left during printing.
    /// </summary>
    void print_title_cols(int start, int end);

    /// <summary>
    /// Returns true if the sheet has print titles defined.
    /// </summary>
    bool has_print_titles() const;

    /// <summary>
    /// Remove all print titles.
    /// </summary>
    void clear_print_titles();

    /// <summary>
    /// Sets the print area of this sheet to print_area.
    /// </summary>
    void print_area(cweeStr print_area);

    /// <summary>
    /// Clear the print area of this sheet.
    /// </summary>
    void clear_print_area();

    /// <summary>
    /// Returns true if the print area is defined for this sheet.
    /// </summary>
    bool has_print_area() const;

    /// <summary>
    /// Returns true if this sheet has any number of views defined.
    /// </summary>
    bool has_view() const;

    /// <summary>
    /// Returns true if the worksheet has a view and the view has an active cell.
    /// </summary>
    bool has_active_cell() const;

    // page breaks

    /// <summary>
    /// Remove all manual column and row page breaks (represented as dashed
    /// blue lines in the page view in Excel).
    /// </summary>
    void clear_page_breaks();

    /// <summary>
    /// Returns vector where each element represents a row which will break a page below it.
    /// </summary>
    cweeList<int> page_break_rows() const;

    /// <summary>
    /// Add a page break at the given row.
    /// </summary>
    void page_break_at_row(int row);

    /// <summary>
    /// Returns vector where each element represents a column which will break a page to the right.
    /// </summary>
    cweeList<int> page_break_columns() const;

    /// <summary>
    /// Add a page break at the given column.
    /// </summary>
    void page_break_at_column(int column);

    /// <summary>
    /// Returns true if this worksheet has a page setup.
    /// </summary>
    bool has_drawing() const;

    /// <summary>
    /// Returns true if this worksheet is empty.
    /// A worksheet is considered empty if it doesn't have any cells.
    /// </summary>
    bool is_empty() const;

public:

	cweeSharedPtr<ExcelCell> GetCell(cweeStr const& cellName);

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
    static cweeSharedPtr<ExcelWorkbook> OpenExcel();
};


#endif