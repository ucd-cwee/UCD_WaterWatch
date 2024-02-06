#pragma once

#include <cstdio>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include "include/pugixml.hpp"
#include "../WaterWatchCpp/enum.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../ExcelInterop/Wrapper.h"
#include "../WaterWatchCpp/cweeUnitedValue.h"

/* 
 minidocx 0.5.0 - C++ library for creating Microsoft Word Document (.docx).
 * --------------------------------------------------------
 * Copyright (C) 2022-2023, by Xie Zequn (totravel@foxmail.com)
 * Report bugs and download new versions at https://github.com/totravel/minidocx
*/

namespace docx {
    const int PPI = 72;

    // inches
    const double A0_W = 33.1;
    const double A0_H = 46.8;

    const double A1_W = 23.4;
    const double A1_H = 33.1;

    const double A2_W = 16.5;
    const double A2_H = 23.4;

    const double A3_W = 11.7;
    const double A3_H = 16.5;

    const double A4_W = 8.3;
    const double A4_H = 11.7;

    const double A5_W = 5.8;
    const double A5_H = 8.3;

    const double A6_W = 4.1;
    const double A6_H = 5.8;

    const double LETTER_W = 8.5;
    const double LETTER_H = 11;

    const double LEGAL_W = 8.5;
    const double LEGAL_H = 14;

    const double TABLOID_W = 11;
    const double TABLOID_H = 17;

    // pixels
    const unsigned int A0_COLS = 2384;
    const unsigned int A0_ROWS = 3370;

    const unsigned int A1_COLS = 1684;
    const unsigned int A1_ROWS = 2384;

    const unsigned int A2_COLS = 1191;
    const unsigned int A2_ROWS = 1684;

    const unsigned int A3_COLS = 842;
    const unsigned int A3_ROWS = 1190;

    const unsigned int A4_COLS = 595;
    const unsigned int A4_ROWS = 842;

    const unsigned int A5_COLS = 420;
    const unsigned int A5_ROWS = 595;

    const unsigned int A6_COLS = 297;
    const unsigned int A6_ROWS = 420;

    const unsigned int LETTER_COLS = 612;
    const unsigned int LETTER_ROWS = 792;

    const unsigned int LEGAL_COLS = 612;
    const unsigned int LEGAL_ROWS = 1008;

    const unsigned int TABLOID_COLS = 792;
    const unsigned int TABLOID_ROWS = 1224;


    int Pt2Twip(const double pt);
    double Twip2Pt(const int twip);

    double Inch2Pt(const double inch);
    double Pt2Inch(const double pt);

    double MM2Inch(const int mm);
    int Inch2MM(const double inch);

    double CM2Inch(const double cm);
    double Inch2CM(const double inch);

    int Inch2Twip(const double inch);
    double Twip2Inch(const int twip);

    int MM2Twip(const int mm);
    int Twip2MM(const int twip);

    int CM2Twip(const double cm);
    double Twip2CM(const int twip);


    class Document;
    class Paragraph;
    class Section;
    class Run;
    class Table;
    class TableCell;
    class TextFrame;
    class TextFormat;

    class Box
    {
    public:
        enum class BorderStyle { Single, Dotted, Dashed, DotDash, Double, Wave, None };
    };


    struct Cell {
        int row, col; // position
        int rows, cols; // size
    };
    using Row = std::vector<Cell>;
    using Grid = std::vector<Row>;
    using FontStyle = unsigned int;
    enum : FontStyle { Bold = 1, Italic = 2, Underline = 4, Strikethrough = 8 };

    class TableCell {
        friend class Table;

    public:
        // constructs an empty cell
        TableCell();
        TableCell(const TableCell& tc);
        ~TableCell();
        void operator=(const TableCell& right);

        operator bool() const;
        bool empty() const;

        void SetWidth(const int w, std::string const& units = "dxa");

        enum class Alignment { Top, Center, Bottom };
        void SetVerticalAlignment(const Alignment align);

        void SetCellSpanning_(const int cols);

        Paragraph AppendParagraph();
        Paragraph FirstParagraph();

    private:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs a table from existing xml node
        TableCell(Impl* impl);
    }; // class TableCell

    class Table : public Box {
        friend class Document;

    public:
        // constructs an empty table
        Table();
        Table(const Table& t);
        ~Table();
        void operator=(const Table& right);

        void Create_(const int rows, const int cols);

        TableCell GetCell(const int row, const int col);
        TableCell GetCell_(const int row, const int col);
        bool MergeRow(int row);
        bool MergeCells(TableCell tc1, TableCell tc2);
        void RemoveCell_(TableCell tc);

        // units: 
        //   auto - Specifies that width is determined by the overall table layout algorithm.
        //   dxa  - Specifies that the value is in twentieths of a point (1/1440 of an inch).
        //   nil  - Specifies a value of zero.
        //   pct  - Specifies a value as a percent of the table width.
        void SetWidthAuto();
        void SetWidthPercent(const double w); // 0-100
        void SetWidth(const int w, std::string const& units = "dxa");

        // the distance between the cell contents and the cell borders
        void SetCellMarginTop(const int w, std::string const& units = "dxa");
        void SetCellMarginBottom(const int w, std::string const& units = "dxa");
        void SetCellMarginLeft(const int w, std::string const& units = "dxa");
        void SetCellMarginRight(const int w, std::string const& units = "dxa");
        void SetCellMargin(std::string const& elemName, const int w, std::string const& units = "dxa");

        // table formatting
        enum class Alignment { Left, Centered, Right };
        void SetAlignment(const Alignment alignment);

        // style - Specifies the style of the border.
        // width - Specifies the width of the border in points.
        // color - Specifies the color of the border. 
        //         Values are given as hex values (in RRGGBB format). 
        //         No #, unlike hex values in HTML/CSS. E.g., color="FFFF00". 
        //         A value of auto is also permitted and will allow the 
        //         consuming word processor to determine the color.
        void SetTopBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetBottomBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetLeftBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetRightBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetInsideHBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetInsideVBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetInsideBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetOutsideBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetAllBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetBorders_(std::string const& elemName, const BorderStyle style, const double width, std::string const& color);

        void SetLeftIndent(const int leftIndent);

    private:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs a table from existing xml node
        Table(Impl* impl);
    }; // class Table

    class TextFormat {
    public:
        double fontSize = 12;
        std::string fontFamily = "Ariel";
        FontStyle fontStyle = 0;
        int characterSpacing = 0;
    };

    class Axis {
        friend class Document;
        friend class ExcelPlot;
        friend class Chart;

    public:
        Axis();
        Axis(Axis const& p);
        ~Axis();
        void operator=(Axis const& right);
        operator bool() const;

        void SetTitle(cweeStr const& title);
        enum class TickMarkType { in, out, cross, none };
        void SetMajorTickMark(TickMarkType type);
        void SetMajorStep(double majorstep);
        void SetMinorTickMark(TickMarkType type);
        enum class TickLabelPosition { High, Low, NextTo };
        void SetTickLabelPosition(TickLabelPosition pos);
        void SetNumFmt(const cweeStr& fmt);
        void SetMajorGridLine(double weight);
        enum class AxisSide { left,right,top,bottom };
        void SetPosition(AxisSide side);
        void SetTextRotation(int degrees);
        void SetTitleRotation(int degrees);
        enum class AxisStyle { Value, Category, Date, None };
        void SetAxisStyle(AxisStyle style);
        void SetAxisScale(double min, double max);
        cweeStr GetAxisId();

    private:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs from xml node
        Axis(Impl* impl);
    };

    class Series {
        friend class Chart;
        friend class Document;

    public:
        Series();
        Series(Series const& p);
        ~Series();
        void operator=(Series const& right);
        operator bool() const;

        void SetPatternFill(chaiscript::UI_Color const& foreground, chaiscript::UI_Color const& background, cweeStr const& pattern = "pct75");
        void SetColor(chaiscript::UI_Color const& col);
        void SetLineThickness(double weight);
        void SetName(cweeStr const& name);
        void SetOrder(int order);
        // void HideFromLegend();

    protected:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        Series(Series::Impl* impl, cweeSharedPtr<ExcelRange> const& xrange, cweeSharedPtr<ExcelRange> const& yrange);  // constructs from xml node

        cweeSharedPtr< void > impl;
    };

    class Chart {
        friend class ExcelPlot;
        friend class Document;

    public:
        Chart();
        Chart(Chart const& p);
        virtual ~Chart();
        void operator=(Chart const& right);
        operator bool() const;

        void xAxis(Axis& axis);
        void yAxis(Axis& axis);
        Axis xAxis();
        Axis yAxis();

        Series GetSeries(int i);
        Series AddSeries(cweeSharedPtr<ExcelRange> const& xrange, cweeSharedPtr<ExcelRange> const& yrange);

        enum class ChartSeriesGrouping { Standard, Stacked };
        void SetGrouping(ChartSeriesGrouping grouping);

    protected:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        Chart(Chart::Impl* impl, cweeSharedPtr<ExcelRange> const& xrange, cweeSharedPtr<ExcelRange> const& yrange);  // constructs from xml node

        cweeSharedPtr< void > impl;
    };
    
    class LineChart final : public Chart {
        friend class ExcelPlot;

    public:
        LineChart();
        LineChart(LineChart const& p);
        virtual ~LineChart();

    private:
        LineChart(Chart::Impl* impl, cweeSharedPtr<ExcelRange> const& xrange, cweeSharedPtr<ExcelRange> const& yrange);  // constructs from xml node

    };
    class AreaChart final : public Chart {
        friend class ExcelPlot;

    public:
        AreaChart();
        AreaChart(AreaChart const& p);
        virtual ~AreaChart();

    private:
        AreaChart(Chart::Impl* impl, cweeSharedPtr<ExcelRange> const& xrange, cweeSharedPtr<ExcelRange> const& yrange); // constructs from xml node

    };

    class ExcelPlot {
        friend class Run;
        friend class Document;
        friend class Axis;

    public:
        ExcelPlot();
        ExcelPlot(ExcelPlot const& p);
        ~ExcelPlot();
        void operator=(ExcelPlot const& right);
        operator bool() const;       
        void SetTitle(cweeStr const& title);
        void SetPlotVisOnly(bool shouldPlotVisOnly);
        enum class BlankDisplayMode { gap, zero, span };
        void SetDispBlanksAs(BlankDisplayMode mode);
        Chart AppendLineChart(cweeSharedPtr<ExcelRange> const& xrange, cweeSharedPtr<ExcelRange> const& yrange);
        Chart AppendAreaChart(cweeSharedPtr<ExcelRange> const& xrange, cweeSharedPtr<ExcelRange> const& yrange);
        void SetWidth(cweeUnitValues::inch width);
        void SetHeight(cweeUnitValues::inch height);
        void SetLayout(cweeUnitValues::inch x = 0.13780757733712981, cweeUnitValues::inch y = 9.8103417785904798E-2, cweeUnitValues::inch w = 0.81542477392173396, cweeUnitValues::inch h = 0.68267880615409293);
        enum class LegendPos { l, r, t, b };
        void AppendLegend(LegendPos pos, cweeUnitValues::inch x, cweeUnitValues::inch y, cweeUnitValues::inch w, cweeUnitValues::inch h);
        void AppendLegend(LegendPos pos);

    private:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs from xml node
        ExcelPlot(Impl* impl);
    };



    class Run {
        friend class ExcelPlot;
        friend class Paragraph;
        friend std::ostream& operator<<(std::ostream& out, const Run& r);

    public:
        // constructs an empty run
        Run();
        Run(const Run& r);
        ~Run();
        void operator=(const Run& right);

        operator bool() const;
        Run Next();

        // text
        void SetText(const ::std::string& text);
        void AppendText(const std::string& text);
        std::string GetText();
        void ClearText();
        void AppendLineBreak();
        //void InsertPicture(const std::string& filepath);
        ExcelPlot InsertChart(cweeSharedPtr<ExcelWorkbook> const& workbook);

        // text formatting
        void SetFontSize(const double fontSize);
        double GetFontSize();

        void SetFont(const std::string& fontAscii, const std::string& fontEastAsia = "");
        void GetFont(std::string& fontAscii, std::string& fontEastAsia);

        void SetFontStyle(const FontStyle fontStyle);
        FontStyle GetFontStyle();

        void SetCharacterSpacing(const int characterSpacing);
        int GetCharacterSpacing();

        enum class vertAlign { none, superscript };
        void SetVerticalAlignment(vertAlign alignment);

        // Run
        void Remove();
        bool IsPageBreak();

    private:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs run from existing xml node
        Run(Impl* impl);
    }; // class Run

    class Section
    {
        friend class Document;
        friend class Paragraph;
        friend std::ostream& operator<<(std::ostream& out, const Section& s);

    public:
        // constructs an empty section
        Section();
        Section(const Section& s);
        ~Section();
        void operator=(const Section& right);
        bool operator==(const Section& s);

        operator bool() const;
        Section Next();
        Section Prev();

        // section
        void Split();
        bool IsSplit();
        void Merge();

        // page formatting
        enum class Orientation { Landscape, Portrait, Unknown };
        void SetPageSize(const int w, const int h);
        void GetPageSize(int& w, int& h);

        void SetPageOrient(const Orientation orient);
        Orientation GetPageOrient();

        void SetPageMargin(const int top, const int bottom, const int left, const int right);
        void GetPageMargin(int& top, int& bottom, int& left, int& right);

        void SetPageMargin(const int header, const int footer);
        void GetPageMargin(int& header, int& footer);

        enum class PageNumberFormat {
            Decimal,      // e.g., 1, 2, 3, 4, etc.
            NumberInDash, // e.g., -1-, -2-, -3-, -4-, etc.
            CardinalText, // In English, One, Two, Three, etc.
            OrdinalText,  // In English, First, Second, Third, etc.
            LowerLetter,  // e.g., a, b, c, etc.
            UpperLetter,  // e.g., A, B, C, etc.
            LowerRoman,   // e.g., i, ii, iii, iv, etc.
            UpperRoman    // e.g., I, II, III, IV, etc.
        };

        /**
         * Specifies the page numbers for pages in the section.
         *
         * @param fmt   Specifies the number format to be used for page numbers in the section.
         *
         * @param start Specifies the page number that appears on the first page of the section.
         *              If the value is omitted, numbering continues from the highest page number in the previous section.
         */
        void SetPageNumber(const PageNumberFormat fmt = PageNumberFormat::Decimal, const unsigned int start = 0);
        void RemovePageNumber();

        // paragraph
        Paragraph FirstParagraph();
        Paragraph LastParagraph();

    private:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs section from existing xml node
        Section(Impl* impl);
        void FindSectionProperties();
    }; // class Section

    class Paragraph : public Box
    {
        friend class Document;
        friend class Section;
        friend class TableCell;
        friend std::ostream& operator<<(std::ostream& out, const Paragraph& p);

    public:
        // constructs an empty paragraph
        Paragraph();
        Paragraph(const Paragraph& p);
        ~Paragraph();
        void operator=(const Paragraph& right);
        bool operator==(const Paragraph& p);

        operator bool() const;
        Paragraph Next();
        Paragraph Prev();

        // get run
        Run FirstRun();

        // add field
        enum class Field { Table, Figure };
        Paragraph AppendField(Field field);// const std::string& category = " SEQ Table \\* ARABIC ");

        // Set as Heading
        Paragraph SetHeading(int level);

        // add run
        Run AppendRun();
        Run AppendRun(const std::string& text);
        Run AppendRun(const std::string& text, const double fontSize);
        Run AppendRun(const std::string& text, const double fontSize, const std::string& fontAscii, const std::string& fontEastAsia = "");
        Run AppendPageBreak();

        // paragraph formatting
        enum class Alignment { Left, Centered, Right, Justified, Distributed };
        void SetAlignment(const Alignment alignment);

        enum class BulletType { Alpha, Number, Bullet };
        void SetNumberedList(const int listNumber, const int indentLevel, BulletType type);
        void SetNumberedList(const int listNumber, const int indentLevel);

        void SetLineSpacingSingle();               // Single
        void SetLineSpacingLines(const double at); // 1.5 lines, Double (2 lines), Multiple (3 lines)
        void SetLineSpacingAtLeast(const int at);  // At Least
        void SetLineSpacingExactly(const int at);  // Exactly
        void SetLineSpacing(const int at, std::string const& lineRule);

        void SetBeforeSpacingAuto();
        void SetAfterSpacingAuto();
        void SetSpacingAuto(std::string const& attrNameAuto);
        void SetBeforeSpacingLines(const double beforeSpacing);
        void SetAfterSpacingLines(const double afterSpacing);
        void SetBeforeSpacing(const int beforeSpacing);
        void SetAfterSpacing(const int afterSpacing);
        void SetSpacing(const int twip, std::string const& attrNameAuto, std::string const& attrName);

        void SetLeftIndentChars(const double leftIndent);
        void SetRightIndentChars(const double rightIndent);
        void SetLeftIndent(const int leftIndent);
        void SetRightIndent(const int rightIndent);
        void SetFirstLineChars(const double indent);
        void SetHangingChars(const double indent);
        void SetFirstLine(const int indent);
        void SetHanging(const int indent);
        void SetIndent(const int indent, std::string const& attrName);

        void SetTopBorder(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetBottomBorder(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetLeftBorder(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetRightBorder(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetBorders(const BorderStyle style = BorderStyle::Single, const double width = 0.5, std::string const& color = "auto");
        void SetBorders_(std::string const& elemName, const BorderStyle style, const double width, std::string const& color);

        // helper
        void SetFontSize(const double fontSize);
        void SetFont(const std::string& fontAscii, const std::string& fontEastAsia = "");
        void SetFontStyle(const FontStyle fontStyle);
        void SetCharacterSpacing(const int characterSpacing);
        std::string GetText();
        TextFormat* Format();

        // section
        Section GetSection();
        Section InsertSectionBreak();
        Section RemoveSectionBreak();
        bool HasSectionBreak();

    protected:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs paragraph from existing xml node
        Paragraph(Impl* impl);
    }; // class Paragraph

    class TextFrame : public Paragraph
    {
        friend class Document;

    public:
        // constructs an empty text frame
        TextFrame();
        TextFrame(const TextFrame& tf);
        ~TextFrame();

        void SetSize(const int w, const int h);

        enum class Anchor { Page, Margin };
        enum class Position { Left, Center, Right, Top, Bottom };
        void SetAnchor_(std::string const& attrName, const Anchor anchor);
        void SetPosition_(std::string const& attrName, const Position align);
        void SetPosition_(std::string const& attrName, const int twip);

        void SetPositionX(const Position align, const Anchor ralativeTo);
        void SetPositionY(const Position align, const Anchor ralativeTo);
        void SetPositionX(const int x, const Anchor ralativeTo);
        void SetPositionY(const int y, const Anchor ralativeTo);

        enum class Wrapping { Around, None };
        void SetTextWrapping(const Wrapping wrapping);

    private:
        class Impl;
        Impl* impl_() const { return static_cast<Impl*>(impl.Get()); };
        cweeSharedPtr< void > impl;

        // constructs text frame from existing xml node
        TextFrame(Impl* impl, Paragraph::Impl* p_impl);
    }; // class TextFrame

    class Document
    {
        friend std::ostream& operator<<(std::ostream& out, const Document& doc);

    public:
        // constructs an empty document
        Document();
        Document(Document const&);
        Document& operator=(Document const&);
        Document(const std::string& path);
        ~Document();

        // save document to file
        bool Save();
        bool Save(const std::string& path);
        bool Open(const std::string& path);

        // get paragraph
        Paragraph FirstParagraph();
        Paragraph LastParagraph();

        // add paragraph
        Paragraph AppendParagraph();
        Paragraph AppendParagraph(const std::string& text);
        Paragraph AppendParagraph(const std::string& text, const double fontSize);
        Paragraph AppendParagraph(const std::string& text, const double fontSize, const std::string& fontAscii, const std::string& fontEastAsia = "");
        Paragraph PrependParagraph();
        Paragraph PrependParagraph(const std::string& text);
        Paragraph PrependParagraph(const std::string& text, const double fontSize);
        Paragraph PrependParagraph(const std::string& text, const double fontSize, const std::string& fontAscii, const std::string& fontEastAsia = "");

        Paragraph InsertParagraphBefore(const Paragraph& p);
        Paragraph InsertParagraphAfter(const Paragraph& p);
        bool RemoveParagraph(Paragraph& p);

        Paragraph AppendPageBreak();

        // get section
        Section FirstSection();
        Section LastSection();

        // add section
        Section AppendSectionBreak();

        // add table
        Table AppendTable(const int rows, const int cols);
        void RemoveTable(Table& tbl);

        // helper
        void SetFontSize(const double fontSize);
        void SetFont(const std::string& fontAscii, const std::string& fontEastAsia = "");
        void SetFontStyle(const FontStyle fontStyle);
        void SetCharacterSpacing(const int characterSpacing);
        std::string GetText();
        TextFormat* Format();

        // paragraph formatting
        void SetAlignment(const Paragraph::Alignment alignment);
        void SetLineSpacingSingle();               // Single
        void SetLineSpacingLines(const double at); // 1.5 lines, Double (2 lines), Multiple (3 lines)
        void SetLineSpacingAtLeast(const int at);  // At Least
        void SetLineSpacingExactly(const int at);  // Exactly
        void SetLineSpacing(const int at, std::string const& lineRule);
        void SetBeforeSpacingAuto();
        void SetAfterSpacingAuto();
        void SetSpacingAuto(std::string const& attrNameAuto);
        void SetBeforeSpacingLines(const double beforeSpacing);
        void SetAfterSpacingLines(const double afterSpacing);
        void SetBeforeSpacing(const int beforeSpacing);
        void SetAfterSpacing(const int afterSpacing);
        void SetSpacing(const int twip, std::string const& attrNameAuto, std::string const& attrName);
        void SetLeftIndentChars(const double leftIndent);
        void SetRightIndentChars(const double rightIndent);
        void SetLeftIndent(const int leftIndent);
        void SetRightIndent(const int rightIndent);
        void SetFirstLineChars(const double indent);
        void SetHangingChars(const double indent);
        void SetFirstLine(const int indent);
        void SetHanging(const int indent);
        void SetIndent(const int indent, std::string const& attrName);

        // add text frame
        TextFrame AppendTextFrame(const int w, const int h);

    protected:
        class Impl;
        Impl* impl_() const;
        cweeSharedPtr< void > doc_impl_;
    }; // class Document

}; // namespace docx