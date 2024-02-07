#pragma once

#include "Wrapper.h"

#include "include/pugixml.hpp"
#include <iostream>

#include "duckx.hpp"
cweeStr WordInteropWrapper::test(cweeStr const& filePath) {
    using namespace docx;

    Document doc(filePath.c_str());

    doc.AppendParagraph();

    auto p1 = doc.AppendParagraph("Hello, World!", 12, "Times New Roman"); {
        auto p1r1 = p1.AppendRun("This is a ROCKWELL sentence. ", 18, "Rockwell");
        p1r1.SetCharacterSpacing(Pt2Twip(2));
        p1r1.SetFontStyle(Bold | Italic);
    }

    auto p4 = doc.AppendParagraph();
    p4.SetAlignment(Paragraph::Alignment::Centered); {
        auto p4r1 = p4.AppendRun("This is a simple sentence. ", 12, "Arial");
        p4r1.SetCharacterSpacing(Pt2Twip(2));
        p4r1.SetFontStyle(Bold | Italic);

        auto p4r2 = p4.AppendRun("This is also a simple sentence. ", 18, "Rockwell");
        p4r2.SetCharacterSpacing(Pt2Twip(2));
        p4r2.SetFontStyle(Italic);

        auto p4r3 = p4.AppendRun("This IS A SENTANCE. ", 18, "Rockwell");
        p4r3.SetCharacterSpacing(Pt2Twip(2));
        p4r3.SetFontStyle(Bold | Underline);
    }

    auto p5 = doc.AppendParagraph();
    p5.SetAlignment(Paragraph::Alignment::Centered); {
        auto p4r1 = p5.AppendRun("This is a simple sentence. ", 12, "Arial");
        p4r1.SetCharacterSpacing(Pt2Twip(2));
        p4r1.SetFontStyle(Bold | Italic);

        auto p4r2 = p5.AppendRun("This is also a simple sentence. ", 18, "Rockwell");
        p4r2.SetCharacterSpacing(Pt2Twip(2));
        p4r2.SetFontStyle(Italic);

        auto p4r3 = p5.AppendRun("This IS A SENTANCE. ", 18, "Rockwell");
        p4r3.SetCharacterSpacing(Pt2Twip(2));
        p4r3.SetFontStyle(Bold | Underline);
    }

    doc.Save();

    ::std::string out;
    for (Section sec = doc.FirstSection(); (bool)sec; sec = sec.Next()) {
        for (Paragraph para = sec.FirstParagraph(); para; para = para.Next()) {
            for (Run run = para.FirstRun(); run; run = run.Next()) {
                out += run.GetText();
            }
            out += "\n";
        }
        out += "\n\n";
    }
    return out.c_str();
};

BETTER_ENUM(VerticalAlignment, int, Top, Center, Bottom);
BETTER_ENUM(HorizontalAlignment, int, Left, Center, Right, Justified, Distributed);
BETTER_ENUM(BorderStyle, int, Single, Dotted, Dashed, DotDash, Double, Wave, None);
BETTER_ENUM(FontStyle, int, a, Bold, Italic,d, Underline,f,g,h, Strikethrough,j,l);
BETTER_ENUM(PageNumberFormat, int, Decimal, NumberInDash, CardinalText, OrdinalText, LowerLetter, UpperLetter, LowerRoman, UpperRoman);
BETTER_ENUM(BulletType, int, Alpha, Number, Bullet);
BETTER_ENUM(DocxFieldType, int, Table, Figure);
BETTER_ENUM(DocxTickMarkType, int, In, Out, Cross, None);
BETTER_ENUM(DocxBlankDisplayMode, int, gap, zero, span);
BETTER_ENUM(DocxAxisSide, int, left, right, top, bottom);
BETTER_ENUM(DocxAxisStyle, int, Value, Category, Date, None);
BETTER_ENUM(DocxTickLabelPosition, int, High, Low, NextTo);
BETTER_ENUM(DocxVertAlign, int, none, superscript);
BETTER_ENUM(DocxLegendPos, int, l, r, t, b);
BETTER_ENUM(DocxChartSeriesGrouping, int, Standard, Stacked);

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr MSWord_library() {
            auto lib = chaiscript::make_shared<Module>();

            using namespace docx;

            lib->AddFunction(, format, , SINGLE_ARG(
                auto numDigitsStr{ std::to_string(numDigits) };
                auto x = cweeStr("%0.") + cweeStr(numDigitsStr.c_str()) + "f";
                return cweeStr::printf(x.c_str(), number);
            ), double number, int numDigits);

            lib->add(chaiscript::user_type<Cell>(), "DocxCell");
            lib->add(chaiscript::constructor<Cell()>(), "DocxCell");
            lib->add(chaiscript::constructor<Cell(const Cell&)>(), "DocxCell");
            lib->add(chaiscript::fun([](Cell& a, Cell& b) { a = b; return a; }), "=");
            lib->add(chaiscript::fun(&Cell::row), "row");
            lib->add(chaiscript::fun(&Cell::col), "col");
            lib->add(chaiscript::fun(&Cell::rows), "rows");
            lib->add(chaiscript::fun(&Cell::cols), "cols");

            lib->add(chaiscript::user_type<TableCell>(), "DocxTableCell");
            lib->add(chaiscript::constructor<TableCell()>(), "DocxTableCell");
            lib->add(chaiscript::constructor<TableCell(const TableCell&)>(), "DocxTableCell");
            lib->add(chaiscript::constructor<bool(const TableCell&)>(), "bool");
            lib->add(chaiscript::fun([](TableCell& a, TableCell& b) { a = b; return a; }), "=");
            lib->AddFunction(, empty, , return o.empty(); , TableCell& o);
            lib->AddFunction(, SetWidth, , return o.SetWidth(pixels);, TableCell& o, int pixels);
            lib->AddFunction(, SetVerticalAlignment, , return o.SetVerticalAlignment(static_cast<TableCell::Alignment>(GetBetterEnum<VerticalAlignment>(input)._to_integral()));, TableCell& o, cweeStr const& input);
            lib->add(chaiscript::fun(&TableCell::AppendParagraph), "AppendParagraph");
            lib->add(chaiscript::fun(&TableCell::FirstParagraph), "FirstParagraph");
            lib->AddFunction(, SetCellSpanning_, , return o.SetCellSpanning_(cols);, TableCell& o, int cols);

            lib->add(chaiscript::user_type<Table>(), "DocxTable");
            lib->add(chaiscript::constructor<Table()>(), "DocxTable");
            lib->add(chaiscript::constructor<Table(const Table&)>(), "DocxTable");
            lib->add(chaiscript::fun([](Table& a, Table& b) { a = b; return a; }), "=");
            lib->add(chaiscript::fun(&Table::Create_), "Create_");
            lib->add(chaiscript::fun(&Table::GetCell), "GetCell");
            lib->add(chaiscript::fun(&Table::GetCell_), "GetCell_");
            lib->add(chaiscript::fun(&Table::MergeRow), "MergeRow");
            lib->add(chaiscript::fun(&Table::MergeCells), "MergeCells");
            lib->add(chaiscript::fun(&Table::RemoveCell_), "RemoveCell_");
            lib->add(chaiscript::fun(&Table::SetWidthAuto), "SetWidthAuto");
            lib->add(chaiscript::fun(&Table::SetWidthPercent), "SetWidthPercent");
            lib->AddFunction(, SetLeftIndent, , return o.SetLeftIndent(leftIndent); , Table& o, double leftIndent);
            lib->AddFunction(, SetWidth, , return o.SetWidth(w);, Table& o, int w);            
            lib->AddFunction(, SetCellMarginTop, , return o.SetCellMarginTop(w); , Table& o, int w);
            lib->AddFunction(, SetCellMarginBottom, , return o.SetCellMarginBottom(w); , Table& o, int w);
            lib->AddFunction(, SetCellMarginLeft, , return o.SetCellMarginLeft(w); , Table& o, int w);
            lib->AddFunction(, SetCellMarginRight, , return o.SetCellMarginRight(w); , Table& o, int w);
            lib->AddFunction(, SetHorizontalAlignment, , return o.SetAlignment(static_cast<Table::Alignment>(GetBetterEnum<HorizontalAlignment>(input)._to_integral()));, Table& o, cweeStr const& input);            
            lib->AddFunction(, SetTopBorders, , return o.SetTopBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral())); , Table& o, cweeStr const& input);
            lib->AddFunction(, SetBottomBorders, , return o.SetBottomBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Table& o, cweeStr const& input);
            lib->AddFunction(, SetLeftBorders, , return o.SetLeftBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Table& o, cweeStr const& input);
            lib->AddFunction(, SetRightBorders, , return o.SetRightBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Table& o, cweeStr const& input);
            lib->AddFunction(, SetInsideHBorders, , return o.SetInsideHBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Table& o, cweeStr const& input);
            lib->AddFunction(, SetInsideVBorders, , return o.SetInsideVBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Table& o, cweeStr const& input);
            lib->AddFunction(, SetInsideBorders, , return o.SetInsideBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Table& o, cweeStr const& input);
            lib->AddFunction(, SetOutsideBorders, , return o.SetOutsideBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Table& o, cweeStr const& input);
            lib->AddFunction(, SetAllBorders, , return o.SetAllBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral())); , Table& o, cweeStr const& input);

            lib->add(chaiscript::user_type<Axis>(), "DocxChartAxis");
            lib->add(chaiscript::constructor<Axis()>(), "DocxChartAxis");
            lib->add(chaiscript::constructor<Axis(const Axis&)>(), "DocxChartAxis");
            lib->add(chaiscript::constructor<bool(const Axis&)>(), "bool");
            lib->add(chaiscript::fun([](Axis& a, Axis& b) { a = b; return a; }), "=");
            lib->AddFunction(, SetTitle, , o.SetTitle(title); , Axis & o, cweeStr const& title);
            lib->AddFunction(, SetMajorTickMark, , o.SetMajorTickMark(static_cast<docx::Axis::TickMarkType>(GetBetterEnum<::DocxTickMarkType>(input)._to_integral()));, Axis & o, cweeStr const& input);
            lib->AddFunction(, SetMinorTickMark, , o.SetMinorTickMark(static_cast<docx::Axis::TickMarkType>(GetBetterEnum<::DocxTickMarkType>(input)._to_integral()));, Axis & o, cweeStr const& input);
            lib->add(chaiscript::fun(&Axis::SetNumFmt), "SetNumFmt");
            lib->add(chaiscript::fun(&Axis::SetMajorGridLine), "SetMajorGridLine");
            lib->AddFunction(, SetPosition, , o.SetPosition(static_cast<docx::Axis::AxisSide>(GetBetterEnum<::DocxAxisSide>(input)._to_integral())); , Axis& o, cweeStr const& input);
            lib->add(chaiscript::fun(&Axis::SetTextRotation), "SetTextRotation");
            lib->add(chaiscript::fun(&Axis::SetTitleRotation), "SetTitleRotation");
            lib->AddFunction(, SetAxisStyle, , o.SetAxisStyle(static_cast<docx::Axis::AxisStyle>(GetBetterEnum<::DocxAxisStyle>(input)._to_integral()));, Axis & o, cweeStr const& input);
            lib->AddFunction(, SetAxisScale, , o.SetAxisScale(min,max); , Axis & o, double min, double max);
            lib->add(chaiscript::fun(&Axis::SetMajorStep), "SetMajorStep");
            lib->AddFunction(, SetTickLabelPosition, , o.SetTickLabelPosition(static_cast<docx::Axis::TickLabelPosition>(GetBetterEnum<::DocxTickLabelPosition>(input)._to_integral())); , Axis & o, cweeStr const& input);
            lib->AddFunction(, GetAxisId, , o.GetAxisId();, Axis & o);
            lib->AddFunction(, CrossesAxis, , o.CrossesAxis(other); , Axis & o, Axis& other);
            lib->AddFunction(, CrossesAxisAt, , o.CrossesAxisAt(v); , Axis & o, float v);

            lib->add(chaiscript::user_type<Series>(), "DocxSeries");
            lib->add(chaiscript::constructor<Series()>(), "DocxSeries");
            lib->add(chaiscript::constructor<Series(const Series&)>(), "DocxSeries");
            lib->add(chaiscript::constructor<bool(const Series&)>(), "bool");
            lib->add(chaiscript::fun([](Series& a, Series& b) { a = b; return a; }), "=");
            lib->add(chaiscript::fun(&Series::SetColor), "SetColor");
            lib->add(chaiscript::fun(&Series::SetLineThickness), "SetLineThickness");
            lib->add(chaiscript::fun(&Series::SetName), "SetName");
            lib->add(chaiscript::fun(&Series::SetOrder), "SetOrder");
            lib->add(chaiscript::fun(&Series::SetPatternFill), "SetPatternFill");
            lib->AddFunction(, SetPatternFill, , o.SetPatternFill(fg,bg);, Series & o, UI_Color const& fg, UI_Color const& bg);
            lib->AddFunction(, SetPatternFill, , o.SetPatternFill(fg, bg,pat);, Series & o, UI_Color const& fg, UI_Color const& bg, cweeStr const& pat);

            lib->add(chaiscript::user_type<Chart>(), "DocxChart");
            lib->add(chaiscript::constructor<Chart()>(), "DocxChart");
            lib->add(chaiscript::constructor<Chart(const Chart&)>(), "DocxChart");
            lib->add(chaiscript::constructor<bool(const Chart&)>(), "bool");
            lib->add(chaiscript::fun([](Chart& a, Chart& b) { a = b; return a; }), "=");
            lib->AddFunction(, xAxis, , return o.xAxis();, Chart& o);
            lib->AddFunction(, yAxis, , return o.yAxis(); , Chart& o);
            lib->AddFunction(, xAxis, , o.xAxis(axis); , Chart& o, Axis& axis);
            lib->AddFunction(, yAxis, , o.yAxis(axis); , Chart& o, Axis& axis);
            lib->add(chaiscript::fun(&Chart::GetSeries), "GetSeries");
            lib->add(chaiscript::fun(&Chart::AddSeries), "AddSeries");
            lib->AddFunction(, SetGrouping, , o.SetGrouping(static_cast<docx::Chart::ChartSeriesGrouping>(GetBetterEnum<::DocxChartSeriesGrouping>(input)._to_integral()));, Chart & o, cweeStr const& input);

            lib->add(chaiscript::user_type<ExcelPlot>(), "DocxPlot");
            lib->add(chaiscript::constructor<ExcelPlot()>(), "DocxPlot");
            lib->add(chaiscript::constructor<ExcelPlot(const ExcelPlot&)>(), "DocxPlot");
            lib->add(chaiscript::constructor<bool(const ExcelPlot&)>(), "bool");
            lib->add(chaiscript::fun([](ExcelPlot& a, ExcelPlot& b) { a = b; return a; }), "=");
            lib->AddFunction(, SetTitle, , o.SetTitle(title);, ExcelPlot& o, cweeStr const& title);
            lib->AddFunction(, SetWidth, , o.SetWidth(width);, ExcelPlot & o, cweeUnitValues::unit_value const& width);
            lib->AddFunction(, SetHeight, , o.SetHeight(height);, ExcelPlot & o, cweeUnitValues::unit_value const& height);
            lib->add(chaiscript::fun(&ExcelPlot::AppendLineChart), "AppendLineChart");
            lib->add(chaiscript::fun(&ExcelPlot::AppendAreaChart), "AppendAreaChart");
            lib->add(chaiscript::fun(&ExcelPlot::SetPlotVisOnly), "SetPlotVisOnly");
            lib->AddFunction(, SetDispBlanksAs, , o.SetDispBlanksAs(static_cast<docx::ExcelPlot::BlankDisplayMode>(GetBetterEnum<::DocxBlankDisplayMode>(input)._to_integral())); , ExcelPlot& o, cweeStr const& input);            
            lib->AddFunction(, SetLayout, , o.SetLayout(x,y,w,h); , ExcelPlot & o, cweeUnitValues::unit_value const& x, cweeUnitValues::unit_value const& y, cweeUnitValues::unit_value const& w, cweeUnitValues::unit_value const& h);
            lib->AddFunction(, SetLayout, , o.SetLayout();, ExcelPlot & o);            
            lib->AddFunction(, AppendLegend, , o.AppendLegend(static_cast<docx::ExcelPlot::LegendPos>(GetBetterEnum<::DocxLegendPos>(input)._to_integral()), x, y, w, h);, ExcelPlot & o, cweeStr const& input, cweeUnitValues::unit_value const& x, cweeUnitValues::unit_value const& y, cweeUnitValues::unit_value const& w, cweeUnitValues::unit_value const& h);
            lib->AddFunction(, AppendLegend, , o.AppendLegend(static_cast<docx::ExcelPlot::LegendPos>(GetBetterEnum<::DocxLegendPos>(input)._to_integral())); , ExcelPlot & o, cweeStr const& input);

            lib->add(chaiscript::user_type<Run>(), "DocxRun");
            lib->add(chaiscript::constructor<Run()>(), "DocxRun");
            lib->add(chaiscript::constructor<Run(const Run&)>(), "DocxRun");
            lib->add(chaiscript::constructor<bool(const Run&)>(), "bool");
            lib->add(chaiscript::fun([](Run& a, Run& b) { a = b; return a; }), "=");
            lib->AddFunction(, Next, , return o.Next(); , Run& o);
            lib->add(chaiscript::fun(&Run::AppendText), "AppendText");
            lib->add(chaiscript::fun(&Run::AppendText), "SetText");
            lib->add(chaiscript::fun(&Run::GetText), "GetText");
            lib->add(chaiscript::fun(&Run::ClearText), "ClearText");
            lib->add(chaiscript::fun(&Run::AppendLineBreak), "AppendLineBreak");            
            lib->add(chaiscript::fun(&Run::SetFontSize), "SetFontSize");
            lib->add(chaiscript::fun(&Run::GetFontSize), "GetFontSize");
            lib->add(chaiscript::fun(&Run::InsertChart), "InsertChart");
            lib->AddFunction(, SetFont, , return o.SetFont(font.c_str()); , Run& o, cweeStr const& font);
            lib->AddFunction(, GetFont, , std::string fontAscii; std::string fontEastAsia; o.GetFont(fontAscii, fontEastAsia); return fontAscii, Run& o);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input)._to_integral())); , Run& o, cweeStr const& input);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral())); , Run& o, cweeStr const& input1, cweeStr const& input2);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input3)._to_integral())); , Run& o, cweeStr const& input1, cweeStr const& input2, cweeStr const& input3);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input3)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input4)._to_integral()));, Run& o, cweeStr const& input1, cweeStr const& input2, cweeStr const& input3, cweeStr const& input4);
            lib->AddFunction(, GetFontStyle, ->cweeStr , return ::FontStyle::_from_integral(static_cast<int>(o.GetFontStyle())).ToString();, Run& o, cweeStr const& input);
            lib->add(chaiscript::fun(&Run::SetCharacterSpacing), "SetCharacterSpacing");
            lib->add(chaiscript::fun(&Run::GetCharacterSpacing), "GetCharacterSpacing");
            lib->AddFunction(, SetVerticalAlignment, , return o.SetVerticalAlignment(static_cast<docx::Run::vertAlign>(GetBetterEnum<::DocxVertAlign>(input)._to_integral()));, Run& o, cweeStr const& input);
            lib->add(chaiscript::fun(&Run::Remove), "Remove");
            lib->add(chaiscript::fun(&Run::IsPageBreak), "IsPageBreak");

            lib->add(chaiscript::user_type<Section>(), "DocxSection");
            lib->add(chaiscript::constructor<Section()>(), "DocxSection");
            lib->add(chaiscript::constructor<Section(const Section&)>(), "DocxSection");
            lib->add(chaiscript::constructor<bool(const Section&)>(), "bool");
            lib->add(chaiscript::fun([](Section& a, Section& b) { a = b; return a; }), "=");            
            lib->add(chaiscript::fun(&Section::Next), "Next");
            lib->add(chaiscript::fun(&Section::Prev), "Prev");
            lib->add(chaiscript::fun(&Section::Split), "Split");
            lib->add(chaiscript::fun(&Section::IsSplit), "IsSplit");
            lib->add(chaiscript::fun(&Section::FirstParagraph), "FirstParagraph");
            lib->add(chaiscript::fun(&Section::LastParagraph), "LastParagraph");
            lib->AddFunction(, SetPageMargin, , return o.SetPageMargin(top, bottom, left, right);, Section& o, int top, int bottom, int left, int right);
            lib->AddFunction(, SetPageMargin, , return o.SetPageMargin(header, footer); , Section& o, int header, int footer);
            lib->AddFunction(, SetPageNumber, , return o.SetPageNumber(static_cast<Section::PageNumberFormat>(GetBetterEnum<::PageNumberFormat>(input)._to_integral()));, Section& o, cweeStr const& input);
            lib->AddFunction(, SetPageNumber, , return o.SetPageNumber(static_cast<Section::PageNumberFormat>(GetBetterEnum<::PageNumberFormat>(input)._to_integral()), start); , Section& o, cweeStr const& input, int start);
            lib->add(chaiscript::fun(&Section::RemovePageNumber), "RemovePageNumber");

            lib->add(chaiscript::user_type<Paragraph>(), "DocxParagraph");
            lib->add(chaiscript::constructor<Paragraph()>(), "DocxParagraph");
            lib->add(chaiscript::constructor<Paragraph(const Paragraph&)>(), "DocxParagraph");
            lib->add(chaiscript::constructor<bool(const Paragraph&)>(), "bool");
            lib->add(chaiscript::fun([](Paragraph& a, Paragraph& b) { a = b; return a; }), "=");
            lib->add(chaiscript::fun(&Paragraph::Next), "Next");
            lib->add(chaiscript::fun(&Paragraph::Prev), "Prev");
            lib->add(chaiscript::fun(&Paragraph::FirstRun), "FirstRun");
            lib->AddFunction(, AppendField, , return o.AppendField(static_cast<Paragraph::Field>(GetBetterEnum<DocxFieldType>(tableOrFigure)._to_integral())); , Paragraph& o, cweeStr const& tableOrFigure);
            lib->AddFunction(, SetHeading, , return o.SetHeading(headingLevel);, Paragraph& o, int headingLevel);
            lib->AddFunction(, AppendRun, , return o.AppendRun();, Paragraph& o);
            lib->AddFunction(, AppendRun, , return o.AppendRun(text);, Paragraph& o, const std::string& text);
            lib->AddFunction(, AppendRun, , return o.AppendRun(text, fontSize); , Paragraph& o, const std::string& text, const double fontSize);
            lib->AddFunction(, AppendRun, , return o.AppendRun(text, fontSize, fontAscii); , Paragraph& o, const std::string& text, const double fontSize, const std::string& fontAscii);
            lib->add(chaiscript::fun(&Paragraph::AppendPageBreak), "AppendPageBreak");
            lib->AddFunction(, SetNumberedList, , return o.SetNumberedList(listNumber, 0); , Paragraph& o, int listNumber);
            lib->AddFunction(, SetNumberedList, , return o.SetNumberedList(listNumber, indentLevel);, Paragraph& o, int listNumber, int indentLevel);
            lib->AddFunction(, SetNumberedList, , return o.SetNumberedList(listNumber, indentLevel, static_cast<Paragraph::BulletType>(GetBetterEnum<BulletType>(bulletType)._to_integral())); , Paragraph& o, int listNumber, int indentLevel, cweeStr const& bulletType);
            lib->AddFunction(, SetHorizontalAlignment, , return o.SetAlignment(static_cast<Paragraph::Alignment>(GetBetterEnum<HorizontalAlignment>(input)._to_integral())); , Paragraph& o, cweeStr const& input);
            lib->add(chaiscript::fun(&Paragraph::SetLineSpacingSingle), "SetLineSpacingSingle");
            lib->add(chaiscript::fun(&Paragraph::SetLineSpacingLines), "SetLineSpacingLines");
            lib->add(chaiscript::fun(&Paragraph::SetLineSpacingAtLeast), "SetLineSpacingAtLeast");
            lib->add(chaiscript::fun(&Paragraph::SetLineSpacingExactly), "SetLineSpacingExactly");
            lib->add(chaiscript::fun(&Paragraph::SetBeforeSpacingAuto), "SetBeforeSpacingAuto");
            lib->add(chaiscript::fun(&Paragraph::SetAfterSpacingAuto), "SetAfterSpacingAuto");
            lib->add(chaiscript::fun(&Paragraph::SetSpacingAuto), "SetSpacingAuto");
            lib->add(chaiscript::fun(&Paragraph::SetBeforeSpacingLines), "SetBeforeSpacingLines");
            lib->add(chaiscript::fun(&Paragraph::SetAfterSpacingLines), "SetAfterSpacingLines");
            lib->add(chaiscript::fun(&Paragraph::SetBeforeSpacing), "SetBeforeSpacing");
            lib->add(chaiscript::fun(&Paragraph::SetAfterSpacing), "SetAfterSpacing");
            lib->add(chaiscript::fun(&Paragraph::SetLeftIndentChars), "SetLeftIndentChars");
            lib->add(chaiscript::fun(&Paragraph::SetRightIndentChars), "SetRightIndentChars");
            lib->add(chaiscript::fun(&Paragraph::SetLeftIndent), "SetLeftIndent");
            lib->add(chaiscript::fun(&Paragraph::SetRightIndent), "SetRightIndent");
            lib->add(chaiscript::fun(&Paragraph::SetFirstLineChars), "SetFirstLineChars");
            lib->add(chaiscript::fun(&Paragraph::SetHangingChars), "SetHangingChars");
            lib->add(chaiscript::fun(&Paragraph::SetFirstLine), "SetFirstLine");
            lib->add(chaiscript::fun(&Paragraph::SetHanging), "SetHanging");
            lib->add(chaiscript::fun(&Paragraph::SetIndent), "SetIndent");
            lib->AddFunction(, SetTopBorder, , return o.SetTopBorder(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral()));, Paragraph& o, cweeStr const& input);
            lib->AddFunction(, SetBottomBorder, , return o.SetBottomBorder(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral())); , Paragraph& o, cweeStr const& input);
            lib->AddFunction(, SetLeftBorder, , return o.SetLeftBorder(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral())); , Paragraph& o, cweeStr const& input);
            lib->AddFunction(, SetRightBorder, , return o.SetRightBorder(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral())); , Paragraph& o, cweeStr const& input);
            lib->AddFunction(, SetBorders, , return o.SetBorders(static_cast<Box::BorderStyle>(GetBetterEnum<BorderStyle>(input)._to_integral())); , Paragraph& o, cweeStr const& input);          
            lib->add(chaiscript::fun(&Paragraph::SetFontSize), "SetFontSize");
            lib->AddFunction(, SetFont, , return o.SetFont(font.c_str());, Paragraph& o, cweeStr const& font);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input)._to_integral()));, Paragraph& o, cweeStr const& input);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral()));, Paragraph& o, cweeStr const& input1, cweeStr const& input2);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input3)._to_integral()));, Paragraph& o, cweeStr const& input1, cweeStr const& input2, cweeStr const& input3);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input3)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input4)._to_integral())); , Paragraph& o, cweeStr const& input1, cweeStr const& input2, cweeStr const& input3, cweeStr const& input4);
            lib->add(chaiscript::fun(&Paragraph::SetCharacterSpacing), "SetCharacterSpacing");
            lib->add(chaiscript::fun(&Paragraph::GetText), "GetText");
            lib->add(chaiscript::fun(&Paragraph::GetSection), "GetSection");
            lib->add(chaiscript::fun(&Paragraph::InsertSectionBreak), "InsertSectionBreak");
            lib->add(chaiscript::fun(&Paragraph::RemoveSectionBreak), "RemoveSectionBreak");
            lib->add(chaiscript::fun(&Paragraph::HasSectionBreak), "HasSectionBreak");

            lib->add(chaiscript::user_type<Document>(), "DocxDocument");
            lib->add(chaiscript::constructor<Document()>(), "DocxDocument");
            lib->add(chaiscript::constructor<Document(const Document&)>(), "DocxDocument");
            lib->add(chaiscript::constructor<Document(const std::string&)>(), "DocxDocument");
            lib->add(chaiscript::fun([](Document& a, Document& b) { a = b; return a; }), "=");
            lib->AddFunction(, Save, , return o.Save();, Document& o);
            lib->AddFunction(, Save, , return o.Save(path.c_str());, Document& o, const cweeStr& path);
            lib->add(chaiscript::fun(&Document::Open), "Open");
            lib->add(chaiscript::fun(&Document::FirstParagraph), "FirstParagraph");
            lib->add(chaiscript::fun(&Document::LastParagraph), "LastParagraph");
            lib->AddFunction(, AppendParagraph, , return o.AppendParagraph(); , Document& o);
            lib->AddFunction(, AppendParagraph, , return o.AppendParagraph(text); , Document& o, const std::string& text);
            lib->AddFunction(, AppendParagraph, , return o.AppendParagraph(text, fontSize);, Document& o, const std::string& text, const double fontSize);
            lib->AddFunction(, AppendParagraph, , return o.AppendParagraph(text, fontSize, fontAscii);, Document& o, const std::string& text, const double fontSize, const std::string& fontAscii);
            lib->AddFunction(, PrependParagraph, , return o.PrependParagraph();, Document& o);
            lib->AddFunction(, PrependParagraph, , return o.PrependParagraph(text);, Document& o, const std::string& text);
            lib->AddFunction(, PrependParagraph, , return o.PrependParagraph(text, fontSize); , Document& o, const std::string& text, const double fontSize);
            lib->AddFunction(, PrependParagraph, , return o.PrependParagraph(text, fontSize, fontAscii); , Document& o, const std::string& text, const double fontSize, const std::string& fontAscii);
            lib->add(chaiscript::fun(&Document::InsertParagraphBefore), "InsertParagraphBefore");
            lib->add(chaiscript::fun(&Document::InsertParagraphAfter), "InsertParagraphAfter");
            lib->add(chaiscript::fun(&Document::RemoveParagraph), "RemoveParagraph");            
            lib->add(chaiscript::fun(&Document::AppendPageBreak), "AppendPageBreak");
            lib->add(chaiscript::fun(&Document::FirstSection), "FirstSection");
            lib->add(chaiscript::fun(&Document::LastSection), "LastSection");
            lib->add(chaiscript::fun(&Document::AppendSectionBreak), "AppendSectionBreak");
            lib->add(chaiscript::fun(&Document::AppendTable), "AppendTable");
            lib->add(chaiscript::fun(&Document::RemoveTable), "RemoveTable");
            lib->add(chaiscript::fun(&Document::SetFontSize), "SetFontSize");
            lib->AddFunction(, SetHorizontalAlignment, , return o.SetAlignment(static_cast<Paragraph::Alignment>(GetBetterEnum<HorizontalAlignment>(input)._to_integral()));, Document& o, cweeStr const& input);
            lib->add(chaiscript::fun(&Document::SetLineSpacingSingle), "SetLineSpacingSingle");
            lib->add(chaiscript::fun(&Document::SetLineSpacingLines), "SetLineSpacingLines");
            lib->add(chaiscript::fun(&Document::SetLineSpacingAtLeast), "SetLineSpacingAtLeast");
            lib->add(chaiscript::fun(&Document::SetLineSpacingExactly), "SetLineSpacingExactly");
            lib->add(chaiscript::fun(&Document::SetBeforeSpacingAuto), "SetBeforeSpacingAuto");
            lib->add(chaiscript::fun(&Document::SetAfterSpacingAuto), "SetAfterSpacingAuto");
            lib->add(chaiscript::fun(&Document::SetSpacingAuto), "SetSpacingAuto");
            lib->add(chaiscript::fun(&Document::SetBeforeSpacingLines), "SetBeforeSpacingLines");
            lib->add(chaiscript::fun(&Document::SetAfterSpacingLines), "SetAfterSpacingLines");
            lib->add(chaiscript::fun(&Document::SetBeforeSpacing), "SetBeforeSpacing");
            lib->add(chaiscript::fun(&Document::SetAfterSpacing), "SetAfterSpacing");
            lib->add(chaiscript::fun(&Document::SetLeftIndentChars), "SetLeftIndentChars");
            lib->add(chaiscript::fun(&Document::SetRightIndentChars), "SetRightIndentChars");
            lib->add(chaiscript::fun(&Document::SetLeftIndent), "SetLeftIndent");
            lib->add(chaiscript::fun(&Document::SetRightIndent), "SetRightIndent");
            lib->add(chaiscript::fun(&Document::SetFirstLineChars), "SetFirstLineChars");
            lib->add(chaiscript::fun(&Document::SetHangingChars), "SetHangingChars");
            lib->add(chaiscript::fun(&Document::SetFirstLine), "SetFirstLine");
            lib->add(chaiscript::fun(&Document::SetHanging), "SetHanging");
            lib->add(chaiscript::fun(&Document::SetIndent), "SetIndent");

            lib->AddFunction(, SetFont, , return o.SetFont(font.c_str()); , Document& o, cweeStr const& font);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input)._to_integral())); , Document& o, cweeStr const& input);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral())); , Document& o, cweeStr const& input1, cweeStr const& input2);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input3)._to_integral())); , Document& o, cweeStr const& input1, cweeStr const& input2, cweeStr const& input3);
            lib->AddFunction(, SetFontStyle, , return o.SetFontStyle(static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input1)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input2)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input3)._to_integral()) | static_cast<docx::FontStyle>(GetBetterEnum<::FontStyle>(input4)._to_integral()));, Document& o, cweeStr const& input1, cweeStr const& input2, cweeStr const& input3, cweeStr const& input4);
            lib->add(chaiscript::fun(&Document::SetCharacterSpacing), "SetCharacterSpacing");
            lib->add(chaiscript::fun(&Document::GetText), "GetText");

            lib->add(chaiscript::fun([](cweeStr const& filePath) ->cweeStr { return WordInteropWrapper::test(filePath); }, { "filePath" }), "TestWordDoc");

            return lib;
        };
    };
}; // namespace chaiscript