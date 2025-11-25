#pragma once
#include <memory>
#include "../WaterWatchCpp/enum.h"
#include "../ScriptLanguageTesting/datetime.h"

BETTER_ENUM(CellType, uint8_t, empty, boolean, date, error, inline_string, number, shared_string, formula_string);

class ExcelWorksheet;
class ExcelWorkbook;
class ExcelCell;
class ExcelRangeReference;

class ExcelCellReference {
public:
    ExcelCellReference() : data() {};
    ExcelCellReference(std::shared_ptr<void> d) : data(std::move(d)) {};
    std::shared_ptr<void> data; // could be anything! represents the worksheet ACTUALLY

    /// <summary>
    /// Returns true if the reference refers to an absolute column, otherwise false.
    /// </summary>
    bool column_absolute() const;

    /// <summary>
    /// Makes this reference have an absolute column if absolute_column is true,
    /// otherwise not absolute.
    /// </summary>
    void column_absolute(bool absolute_column);

    /// <summary>
    /// Returns true if the reference refers to an absolute row, otherwise false.
    /// </summary>
    bool row_absolute() const;

    /// <summary>
    /// Makes this reference have an absolute row if absolute_row is true,
    /// otherwise not absolute.
    /// </summary>
    void row_absolute(bool absolute_row);

    /// <summary>
    /// Converts a coordinate to an absolute coordinate string (e.g. B12 -> $B$12)
    /// Defaulting to true, absolute_column and absolute_row can optionally control
    /// whether the resulting cell_reference has an absolute column (e.g. B12 -> $B12)
    /// and absolute row (e.g. B12 -> B$12) respectively.
    /// </summary>
    /// <remarks>
    /// This is functionally equivalent to:
    /// cell_reference copy(*this);
    /// copy.column_absolute(absolute_column);
    /// copy.row_absolute(absolute_row);
    /// return copy;
    /// </remarks>
    std::shared_ptr< ExcelCellReference> make_absolute(bool absolute_column = true, bool absolute_row = true);

    /// <summary>
    /// Returns a string that identifies the column of this reference
    /// (e.g. second column from left is "B")
    /// </summary>
    int column() const;

    /// <summary>
    /// Sets the column of this reference from a string that identifies a particular column.
    /// </summary>
    void column(const std::string& column_string);

    /// <summary>
    /// Returns a 1-indexed numeric index of the column of this reference.
    /// </summary>
    int column_index() const;

    /// <summary>
    /// Sets the column of this reference from a 1-indexed number that identifies a particular column.
    /// </summary>
    void column_index(int column);

    /// <summary>
    /// Returns a 1-indexed numeric index of the row of this reference.
    /// </summary>
    int row() const;

    /// <summary>
    /// Sets the row of this reference from a 1-indexed number that identifies a particular row.
    /// </summary>
    void row(int row);

    /// <summary>
    /// Returns a cell_reference offset from this cell_reference by
    /// the number of columns and rows specified by the parameters.
    /// A negative value for column_offset or row_offset results
    /// in a reference above or left of this cell_reference, respectively.
    /// </summary>
    std::shared_ptr< ExcelCellReference> make_offset(int column_offset, int row_offset) const;

    /// <summary>
    /// Returns a string like "A1" for cell_reference(1, 1).
    /// </summary>
    std::string to_string() const;

    /// <summary>
    /// Returns a 1x1 range_reference containing only this cell_reference.
    /// </summary>
    std::shared_ptr< ExcelRangeReference> to_range() const;
};

class ExcelRangeReference {
public:
    ExcelRangeReference() : data() {};
    ExcelRangeReference(std::shared_ptr<void> d) : data(d) {};
    std::shared_ptr<void> data; // could be anything! represents the worksheet ACTUALLY

    /// <summary>
    /// Converts relative reference coordinates to absolute coordinates (B12 -> $B$12)
    /// </summary>
    std::shared_ptr<ExcelRangeReference> make_absolute();

    /// <summary>
    /// Returns true if the range has a width and height of 1 cell.
    /// </summary>
    bool is_single_cell() const;

    /// <summary>
    /// Returns the number of columns encompassed by this range.
    /// </summary>
    std::size_t width() const;

    /// <summary>
    /// Returns the number of rows encompassed by this range.
    /// </summary>
    std::size_t height() const;

    /// <summary>
    /// Returns the coordinate of the top left cell of this range.
    /// </summary>
    std::shared_ptr< ExcelCellReference > top_left() const;

    /// <summary>
    /// Returns the coordinate of the top right cell of this range.
    /// </summary>
    std::shared_ptr< ExcelCellReference > top_right() const;

    /// <summary>
    /// Returns the coordinate of the bottom left cell of this range.
    /// </summary>
    std::shared_ptr< ExcelCellReference > bottom_left() const;

    /// <summary>
    /// Returns the coordinate of the bottom right cell of this range.
    /// </summary>
    std::shared_ptr< ExcelCellReference > bottom_right() const;

    /// <summary>
    /// Returns a new range reference with the same width and height as this
    /// range but shifted by the given number of columns and rows.
    /// </summary>
    std::shared_ptr<ExcelRangeReference> make_offset(int column_offset, int row_offset) const;

    /// <summary>
    /// Returns a string representation of this range.
    /// </summary>
    std::string to_string() const;

    /// <summary>
    /// Returns true if the given cell reference is within the bounds of this range reference.
    /// </summary>
    bool contains(std::shared_ptr< ExcelCellReference > ref) const;
};

class ExcelCell {
public:
    using type = CellType;

	ExcelCell() : data() {};
	ExcelCell(std::shared_ptr<void> d) : data(d) {};
	std::shared_ptr<void> data; // could be anything! represents the worksheet ACTUALLY

    std::string address() const;

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
    void value(const GL::datetime& date_value);

    /// <summary>
    /// Sets the value of this cell to the given value.
    /// </summary>
    void value(const std::string& string_value);

    /// <summary>
    /// Sets the value and formatting of this cell to that of other_cell.
    /// </summary>
    void value(const ExcelCell& other_cell);

    /// <summary>
    /// Analyzes string_value to determine its type, convert it to that type,
    /// and set the value of this cell to that converted value.
    /// </summary>
    void value(const std::string& string_value, bool infer_type);

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
    std::shared_ptr<ExcelCellReference> reference() const;

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
    std::string formula() const;

    /// <summary>
    /// Sets the formula of this cell to the given value.
    /// This formula string should begin with '='.
    /// </summary>
    void formula(const std::string& formula);

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
    std::string to_string() const;

    // merging

    /// <summary>
    /// Returns true iff this cell has been merged with one or more
    /// surrounding cells.
    /// </summary>
    bool is_merged() const;

    /// <summary>
    /// Returns the error string that is stored in this cell.
    /// </summary>
    std::string error() const;

    /// <summary>
    /// Directly assigns the value of this cell to be the given error.
    /// </summary>
    void error(const std::string& error);

    /// <summary>
    /// Returns a cell from this cell's parent workbook at
    /// a relative offset given by the parameters.
    /// </summary>
    std::shared_ptr<ExcelCell> offset(int column, int row);

    /// <summary>
    /// Returns the worksheet that owns this cell.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> worksheet();

    /// <summary>
    /// Returns the worksheet that owns this cell.
    /// </summary>
    const std::shared_ptr<ExcelWorksheet> worksheet() const;

    /// <summary>
    /// Returns the workbook of the worksheet that owns this cell.
    /// </summary>
    std::shared_ptr<ExcelWorkbook> workbook();

    /// <summary>
    /// Returns the workbook of the worksheet that owns this cell.
    /// </summary>
    std::shared_ptr<ExcelWorkbook> workbook() const;

    /// <summary>
    /// Returns to_check after verifying and fixing encoding, size, and illegal characters.
    /// </summary>
    std::string check_string(const std::string& to_check);

    /// <summary>
    /// Returns the width of this cell in pixels.
    /// </summary>
    double width() const;

    /// <summary>
    /// Returns the height of this cell in pixels.
    /// </summary>
    double height() const;
};
template <> bool ExcelCell::value<bool>() const;
template <> int ExcelCell::value<int>() const;
template <> unsigned int ExcelCell::value<unsigned int>() const;
template <> long long int ExcelCell::value<long long int>() const;
template <> unsigned long long ExcelCell::value<unsigned long long int>() const;
template <> float ExcelCell::value<float>() const;
template <> double ExcelCell::value<double>() const;
template <> GL::datetime ExcelCell::value<GL::datetime>() const;
template <> std::string ExcelCell::value<std::string>() const;

class ExcelRange {
public:
    ExcelRange() : data() {};
    ExcelRange(std::shared_ptr<void> d) : data(d) {};
    std::shared_ptr<void> data; // could be anything! represents the worksheet ACTUALLY

    using cell_vector = std::vector<std::shared_ptr<ExcelCell>>;

    /// <summary>
    /// Erases all cell data from the worksheet for cells within this range.
    /// </summary>
    void clear_cells();

    /// <summary>
    /// Returns a vector pointing to the n-th row or column in this range (depending
    /// on major order).
    /// </summary>
    cell_vector vector(std::size_t n);

    /// <summary>
    /// Returns a vector pointing to the n-th row or column in this range (depending
    /// on major order).
    /// </summary>
    const cell_vector vector(std::size_t n) const;

    /// <summary>
    /// Returns a cell in the range relative to its top left cell.
    /// </summary>
    std::shared_ptr<ExcelCell> cell(std::shared_ptr<ExcelCellReference> ref);

    /// <summary>
    /// Returns a cell in the range relative to its top left cell.
    /// </summary>
    const std::shared_ptr<ExcelCell> cell(std::shared_ptr<ExcelCellReference> ref) const;

    /// <summary>
    /// The worksheet this range targets
    /// </summary>
    std::shared_ptr<ExcelWorksheet> target_worksheet() const;

    /// <summary>
    /// Returns the reference defining the bounds of this range.
    /// </summary>
    std::shared_ptr<ExcelRangeReference> ref() const;

    /// <summary>
    /// Returns the number of rows or columns in this range (depending on major order).
    /// </summary>
    std::size_t length() const;

    /// <summary>
    /// Returns true if the given cell exists in the parent worksheet of this range.
    /// </summary>
    bool contains(std::shared_ptr<ExcelCellReference> ref);

    /// <summary>
    /// Returns the first row or column in this range.
    /// </summary>
    cell_vector front();

    /// <summary>
    /// Returns the first row or column in this range.
    /// </summary>
    const cell_vector front() const;

    /// <summary>
    /// Returns the last row or column in this range.
    /// </summary>
    cell_vector back();

    /// <summary>
    /// Returns the last row or column in this range.
    /// </summary>
    const cell_vector back() const;

    /// <summary>
    /// Applies function f to all cells in the range
    /// </summary>
    void apply(std::function<void(std::shared_ptr<ExcelCell>)> f);

    /// <summary>
    /// Returns the n-th row or column in this range.
    /// </summary>
    cell_vector operator[](std::size_t n);

    /// <summary>
    /// Returns the n-th row or column in this range.
    /// </summary>
    const cell_vector operator[](std::size_t n) const;

};

class ExcelWorksheet {
public:
	ExcelWorksheet() : data() {};
	ExcelWorksheet(std::shared_ptr<void> d) : data(d) {};
	std::shared_ptr<void> data; // could be anything! represents the worksheet ACTUALLY

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
    std::string title() const;

    /// <summary>
    /// Sets the title of this sheet.
    /// </summary>
    void title(std::string const& title);

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
    std::shared_ptr<ExcelCell> cell(int column, int row);

    /// <summary>
    /// Returns the cell at the given column and row. If the cell doesn't exist, an
    /// invalid_parameter exception will be thrown.
    /// </summary>
    std::shared_ptr<ExcelCell> cell(int column, int row) const;

    /// <summary>
    /// Returns the cell at the given column and row. If the cell doesn't exist, an
    /// invalid_parameter exception will be thrown.
    /// </summary>
    std::shared_ptr<ExcelCell> cell(std::string const& address) const;

    /// <summary>
    /// Returns the range defined by reference string. If reference string is the name of
    /// a previously-defined named range in the sheet, it will be returned.
    /// </summary>
    std::shared_ptr<ExcelRange> range(std::string reference_string);

    /// <summary>
    /// Returns the range defined by reference string. If reference string is the name of
    /// a previously-defined named range in the sheet, it will be returned.
    /// </summary>
    const std::shared_ptr<ExcelRange> range(std::string reference_string) const;

    /// <summary>
    /// Returns a range encompassing all cells in this sheet which will
    /// be iterated upon in row-major order. If skip_null is true (default),
    /// empty rows and cells will be skipped during iteration of the range.
    /// </summary>
    std::shared_ptr<ExcelRange> rows(bool skip_null = true);

    /// <summary>
    /// Returns a range encompassing all cells in this sheet which will
    /// be iterated upon in row-major order. If skip_null is true (default),
    /// empty rows and cells will be skipped during iteration of the range.
    /// </summary>
    const std::shared_ptr<ExcelRange> rows(bool skip_null = true) const;

    /// <summary>
    /// Returns a range ecompassing all cells in this sheet which will
    /// be iterated upon in column-major order. If skip_null is true (default),
    /// empty columns and cells will be skipped during iteration of the range.
    /// </summary>
    std::shared_ptr<ExcelRange> columns(bool skip_null = true);

    /// <summary>
    /// Returns a range ecompassing all cells in this sheet which will
    /// be iterated upon in column-major order. If skip_null is true (default),
    /// empty columns and cells will be skipped during iteration of the range.
    /// </summary>
    const std::shared_ptr<ExcelRange> columns(bool skip_null = true) const;

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
    void create_named_range(std::string name, std::string reference_string);

    /// <summary>
    /// Returns true if this worksheet contains a named range with the given name.
    /// </summary>
    bool has_named_range(std::string name) const;

    /// <summary>
    /// Returns the named range with the given name. Throws key_not_found
    /// exception if the named range doesn't exist.
    /// </summary>
    std::shared_ptr<ExcelRange> named_range(std::string name);

    /// <summary>
    /// Returns the named range with the given name. Throws key_not_found
    /// exception if the named range doesn't exist.
    /// </summary>
    std::shared_ptr<ExcelRange> named_range(std::string name) const;

    /// <summary>
    /// Removes a named range with the given name.
    /// </summary>
    void remove_named_range(std::string name);

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
    void merge_cells(std::string reference_string);

    /// <summary>
    /// Removes the merging of the cells in the range represented by the given string.
    /// </summary>
    void unmerge_cells(std::string reference_string);

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
    void auto_filter(std::string range_string);

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
    void print_area(std::string print_area);

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
    std::vector<int> page_break_rows() const;

    /// <summary>
    /// Add a page break at the given row.
    /// </summary>
    void page_break_at_row(int row);

    /// <summary>
    /// Returns vector where each element represents a column which will break a page to the right.
    /// </summary>
    std::vector<int> page_break_columns() const;

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
};

class ExcelWorkbook {
public:
	ExcelWorkbook() : data() {};
	ExcelWorkbook(std::shared_ptr<void> d) : data(d) {};
	std::shared_ptr<void> data; // could be anything! represents the workbook ACTUALLY

    /// <summary>
    /// Evaluate all sheets and all cells immediately
    /// </summary>
    void calculate_now();

    /// <summary>
    /// Creates and returns a sheet after the last sheet in this workbook.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> create_sheet();

    /// <summary>
    /// Creates and returns a sheet at the specified index.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> create_sheet(std::size_t index);

    /// <summary>
    /// Creates and returns a new sheet after the last sheet initializing it
    /// with all of the data from the provided worksheet.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> copy_sheet(std::shared_ptr<ExcelWorksheet> worksheet);

    /// <summary>
    /// Creates and returns a new sheet at the specified index initializing it
    /// with all of the data from the provided worksheet.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> copy_sheet(std::shared_ptr<ExcelWorksheet> worksheet, std::size_t index);

    /// <summary>
    /// Returns the worksheet that is determined to be active. An active
    /// sheet is that which is initially shown by the spreadsheet editor.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> active_sheet();

    /// <summary>
    /// Sets the worksheet that is determined to be active. An active
    /// sheet is that which is initially shown by the spreadsheet editor.
    /// </summary>
    void active_sheet(std::size_t index);

    /// <summary>
    /// Returns the worksheet with the given name. This may throw an exception
    /// if the sheet isn't found. Use workbook::contains(const std::string &)
    /// to make sure the sheet exists before calling this method.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> sheet_by_title(const std::string& title);

    /// <summary>
    /// Returns the worksheet with the given name. This may throw an exception
    /// if the sheet isn't found. Use workbook::contains(const std::string &)
    /// to make sure the sheet exists before calling this method.
    /// </summary>
    const std::shared_ptr<ExcelWorksheet> sheet_by_title(const std::string& title) const;

    /// <summary>
    /// Returns the worksheet at the given index. This will throw an exception
    /// if index is greater than or equal to the number of sheets in this workbook.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> sheet_by_index(std::size_t index);

    /// <summary>
    /// Returns the worksheet at the given index. This will throw an exception
    /// if index is greater than or equal to the number of sheets in this workbook.
    /// </summary>
    const std::shared_ptr<ExcelWorksheet> sheet_by_index(std::size_t index) const;

    /// <summary>
    /// Returns the worksheet with a sheetId of id. Sheet IDs are arbitrary numbers
    /// that uniquely identify a sheet. Most users won't need this.
    /// </summary>
    std::shared_ptr<ExcelWorksheet> sheet_by_id(std::size_t id);

    /// <summary>
    /// Returns the worksheet with a sheetId of id. Sheet IDs are arbitrary numbers
    /// that uniquely identify a sheet. Most users won't need this.
    /// </summary>
    const std::shared_ptr<ExcelWorksheet> sheet_by_id(std::size_t id) const;

    /// <summary>
    /// Returns the hidden identifier of the worksheet at the given index.
    /// This will throw an exception if index is greater than or equal to the
    /// number of sheets in this workbook.
    /// </summary>
    bool sheet_hidden_by_index(std::size_t index) const;

    /// <summary>
    /// Returns true if this workbook contains a sheet with the given title.
    /// </summary>
    bool contains(const std::string& title) const;

    /// <summary>
    /// Returns the index of the given worksheet. The worksheet must be owned by this workbook.
    /// </summary>
    std::size_t index(std::shared_ptr<ExcelWorksheet> worksheet);

    // remove worksheets
    /// <summary>
    /// Removes the given worksheet from this workbook.
    /// </summary>
    void remove_sheet(std::shared_ptr<ExcelWorksheet> worksheet);

    /// <summary>
    /// Sets the contents of this workbook to be equivalent to that of
    /// a workbook returned by workbook::empty().
    /// </summary>
    void clear();

    /// <summary>
    /// Applies the function "f" to every non-empty cell in every worksheet in this workbook.
    /// </summary>
    void apply_to_cells(std::function<void(std::shared_ptr<ExcelCell>)> f);

    /// <summary>
    /// Returns a temporary vector containing the titles of each sheet in the order
    /// of the sheets in the workbook.
    /// </summary>
    std::vector<std::string> sheet_titles() const;

    /// <summary>
    /// Returns the number of sheets in this workbook.
    /// </summary>
    std::size_t sheet_count() const;

    /// <summary>
    /// Returns true if this workbook has had its title set.
    /// </summary>
    bool has_title() const;

    /// <summary>
    /// Returns the title of this workbook.
    /// </summary>
    std::string title() const;

    /// <summary>
    /// Sets the title of this workbook to title.
    /// </summary>
    void title(const std::string& title);

    /// <summary>
    /// Sets the absolute path of this workbook to path.
    /// </summary>
    void abs_path(const std::string& path);

    /// <summary>
    /// Serializes the workbook into an XLSX file and saves the data into a file
    /// named filename.
    /// </summary>
    void save(const std::string& filename) const;

    /// <summary>
    /// Serializes the workbook into an XLSX file encrypted with the given password
    /// and loads the bytes into a file named filename.
    /// </summary>
    void save(const std::string& filename, const std::string& password) const;


    /// <summary>
    /// Interprets file with the given filename as an XLSX file and sets
    /// the content of this workbook to match that file.
    /// </summary>
    void load(const std::string& filename);

    /// <summary>
    /// Interprets file with the given filename as an XLSX file and sets
    /// the content of this workbook to match that file.
    /// </summary>
    void load_limited(const std::string& filename, const std::string& sheetTitle);

    /// <summary>
    /// Interprets file with the given filename as an XLSX file encrypted with the
    /// given password and sets the content of this workbook to match that file.
    /// </summary>
    void load(const std::string& filename, const std::string& password);

    /// <summary>
    /// Returns the n-th worksheet
    /// </summary>
    std::shared_ptr<ExcelWorksheet> operator[](std::size_t n);

    /// <summary>
    /// Returns the n-th row or column in this range.
    /// </summary>
    const std::shared_ptr<ExcelWorksheet> operator[](std::size_t n) const;

};

class cweeExcel {
public:
    static std::shared_ptr<ExcelWorkbook> OpenExcel(std::string filePath, std::string sheetTitle);
	static std::shared_ptr<ExcelWorkbook> OpenExcel(std::string filePath);
    static std::shared_ptr<ExcelWorkbook> OpenExcel();
};