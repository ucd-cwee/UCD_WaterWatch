#include "duckx.hpp"
#include "include/pugixml.hpp"
#include <cstring> 
#include <cstdlib>
#include <cctype>
#include "include/pugixml.hpp"
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/FileSystemH.h"
#include "../WaterWatchCpp/ZipLib.h"

#define _RELS R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>)"
#define DOCUMENT_XML R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas" xmlns:cx="http://schemas.microsoft.com/office/drawing/2014/chartex" xmlns:cx1="http://schemas.microsoft.com/office/drawing/2015/9/8/chartex" xmlns:cx2="http://schemas.microsoft.com/office/drawing/2015/10/21/chartex" xmlns:cx3="http://schemas.microsoft.com/office/drawing/2016/5/9/chartex" xmlns:cx4="http://schemas.microsoft.com/office/drawing/2016/5/10/chartex" xmlns:cx5="http://schemas.microsoft.com/office/drawing/2016/5/11/chartex" xmlns:cx6="http://schemas.microsoft.com/office/drawing/2016/5/12/chartex" xmlns:cx7="http://schemas.microsoft.com/office/drawing/2016/5/13/chartex" xmlns:cx8="http://schemas.microsoft.com/office/drawing/2016/5/14/chartex" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" xmlns:aink="http://schemas.microsoft.com/office/drawing/2016/ink" xmlns:am3d="http://schemas.microsoft.com/office/drawing/2017/model3d" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:oel="http://schemas.microsoft.com/office/2019/extlst" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:w15="http://schemas.microsoft.com/office/word/2012/wordml" xmlns:w16cex="http://schemas.microsoft.com/office/word/2018/wordml/cex" xmlns:w16cid="http://schemas.microsoft.com/office/word/2016/wordml/cid" xmlns:w16="http://schemas.microsoft.com/office/word/2018/wordml" xmlns:w16sdtdh="http://schemas.microsoft.com/office/word/2020/wordml/sdtdatahash" xmlns:w16se="http://schemas.microsoft.com/office/word/2015/wordml/symex" xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" xmlns:wpi="http://schemas.microsoft.com/office/word/2010/wordprocessingInk" xmlns:wne="http://schemas.microsoft.com/office/word/2006/wordml" xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" mc:Ignorable="w14 w15 w16se w16cid w16 w16cex w16sdtdh wp14"><w:body><w:sectPr><w:pgSz w:w="11906" w:h="16838" /><w:pgMar w:top="1440" w:right="1800" w:bottom="1440" w:left="1800" w:header="851" w:footer="992" w:gutter="0" /><w:cols w:space="425" /><w:docGrid w:type="lines" w:linePitch="312" /></w:sectPr></w:body></w:document>)"
#define CONTENT_TYPES_XML R"(<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="xml" ContentType="application/xml"/><Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/><Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/><Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/><Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/><Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/><Override PartName="/word/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/></Types>)"
#define DOCUMENT_XML_RELS R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/><Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/><Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/><Relationship Id="rId6" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/></Relationships>)"
#define FOOTER1_XML R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:ftr xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas" xmlns:cx="http://schemas.microsoft.com/office/drawing/2014/chartex" xmlns:cx1="http://schemas.microsoft.com/office/drawing/2015/9/8/chartex" xmlns:cx2="http://schemas.microsoft.com/office/drawing/2015/10/21/chartex" xmlns:cx3="http://schemas.microsoft.com/office/drawing/2016/5/9/chartex" xmlns:cx4="http://schemas.microsoft.com/office/drawing/2016/5/10/chartex" xmlns:cx5="http://schemas.microsoft.com/office/drawing/2016/5/11/chartex" xmlns:cx6="http://schemas.microsoft.com/office/drawing/2016/5/12/chartex" xmlns:cx7="http://schemas.microsoft.com/office/drawing/2016/5/13/chartex" xmlns:cx8="http://schemas.microsoft.com/office/drawing/2016/5/14/chartex" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" xmlns:aink="http://schemas.microsoft.com/office/drawing/2016/ink" xmlns:am3d="http://schemas.microsoft.com/office/drawing/2017/model3d" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:oel="http://schemas.microsoft.com/office/2019/extlst" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:w15="http://schemas.microsoft.com/office/word/2012/wordml" xmlns:w16cex="http://schemas.microsoft.com/office/word/2018/wordml/cex" xmlns:w16cid="http://schemas.microsoft.com/office/word/2016/wordml/cid" xmlns:w16="http://schemas.microsoft.com/office/word/2018/wordml" xmlns:w16du="http://schemas.microsoft.com/office/word/2023/wordml/word16du" xmlns:w16sdtdh="http://schemas.microsoft.com/office/word/2020/wordml/sdtdatahash" xmlns:w16se="http://schemas.microsoft.com/office/word/2015/wordml/symex" xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" xmlns:wpi="http://schemas.microsoft.com/office/word/2010/wordprocessingInk" xmlns:wne="http://schemas.microsoft.com/office/word/2006/wordml" xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" mc:Ignorable="w14 w15 w16se w16cid w16 w16cex w16sdtdh wp14"><w:p><w:pPr><w:jc w:val="center" /></w:pPr><w:r><w:fldChar w:fldCharType="begin" /></w:r><w:r><w:instrText>PAGE \* MERGEFORMAT</w:instrText></w:r><w:r><w:fldChar w:fldCharType="end" /></w:r></w:p></w:ftr>)"
// #define NUMBERING_XML R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:numbering xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas" xmlns:cx="http://schemas.microsoft.com/office/drawing/2014/chartex" xmlns:cx1="http://schemas.microsoft.com/office/drawing/2015/9/8/chartex" xmlns:cx2="http://schemas.microsoft.com/office/drawing/2015/10/21/chartex" xmlns:cx3="http://schemas.microsoft.com/office/drawing/2016/5/9/chartex" xmlns:cx4="http://schemas.microsoft.com/office/drawing/2016/5/10/chartex" xmlns:cx5="http://schemas.microsoft.com/office/drawing/2016/5/11/chartex" xmlns:cx6="http://schemas.microsoft.com/office/drawing/2016/5/12/chartex" xmlns:cx7="http://schemas.microsoft.com/office/drawing/2016/5/13/chartex" xmlns:cx8="http://schemas.microsoft.com/office/drawing/2016/5/14/chartex" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" xmlns:aink="http://schemas.microsoft.com/office/drawing/2016/ink" xmlns:am3d="http://schemas.microsoft.com/office/drawing/2017/model3d" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:w15="http://schemas.microsoft.com/office/word/2012/wordml" xmlns:w16cid="http://schemas.microsoft.com/office/word/2016/wordml/cid" xmlns:w16se="http://schemas.microsoft.com/office/word/2015/wordml/symex" xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" xmlns:wpi="http://schemas.microsoft.com/office/word/2010/wordprocessingInk" xmlns:wne="http://schemas.microsoft.com/office/word/2006/wordml" xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" mc:Ignorable="w14 w15 w16se w16cid wp14"><w:abstractNum w:abstractNumId="0" w15:restartNumberingAfterBreak="0"><w:nsid w:val="316E7BC8"/><w:multiLevelType w:val="hybridMultilevel"/><w:tmpl w:val="F4EA5848"/><w:lvl w:ilvl="0" w:tplc="04090001"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val=""/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="1" w:tplc="04090003"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val="o"/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="1440" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" w:cs="Courier New" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="2" w:tplc="04090005" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val=""/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="2160" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Wingdings" w:hAnsi="Wingdings" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="3" w:tplc="04090001" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val=""/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="2880" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="4" w:tplc="04090003" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val="o"/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="3600" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" w:cs="Courier New" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="5" w:tplc="04090005" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val=""/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="4320" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Wingdings" w:hAnsi="Wingdings" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="6" w:tplc="04090001" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val=""/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="5040" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="7" w:tplc="04090003" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val="o"/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="5760" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" w:cs="Courier New" w:hint="default"/></w:rPr></w:lvl><w:lvl w:ilvl="8" w:tplc="04090005" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="bullet"/><w:lvlText w:val=""/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="6480" w:hanging="360"/></w:pPr><w:rPr><w:rFonts w:ascii="Wingdings" w:hAnsi="Wingdings" w:hint="default"/></w:rPr></w:lvl></w:abstractNum><w:abstractNum w:abstractNumId="1" w15:restartNumberingAfterBreak="0"><w:nsid w:val="325E017B"/><w:multiLevelType w:val="hybridMultilevel"/><w:tmpl w:val="E164460E"/><w:lvl w:ilvl="0" w:tplc="0409000F"><w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="1" w:tplc="04090019"><w:start w:val="1"/><w:numFmt w:val="lowerLetter"/><w:lvlText w:val="%2."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="1440" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="2" w:tplc="0409001B" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerRoman"/><w:lvlText w:val="%3."/><w:lvlJc w:val="right"/><w:pPr><w:ind w:left="2160" w:hanging="180"/></w:pPr></w:lvl><w:lvl w:ilvl="3" w:tplc="0409000F" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%4."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="2880" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="4" w:tplc="04090019" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerLetter"/><w:lvlText w:val="%5."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="3600" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="5" w:tplc="0409001B" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerRoman"/><w:lvlText w:val="%6."/><w:lvlJc w:val="right"/><w:pPr><w:ind w:left="4320" w:hanging="180"/></w:pPr></w:lvl><w:lvl w:ilvl="6" w:tplc="0409000F" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%7."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="5040" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="7" w:tplc="04090019" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerLetter"/><w:lvlText w:val="%8."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="5760" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="8" w:tplc="0409001B" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerRoman"/><w:lvlText w:val="%9."/><w:lvlJc w:val="right"/><w:pPr><w:ind w:left="6480" w:hanging="180"/></w:pPr></w:lvl></w:abstractNum><w:abstractNum w:abstractNumId="2" w15:restartNumberingAfterBreak="0"><w:nsid w:val="34CC2ED3"/><w:multiLevelType w:val="hybridMultilevel"/><w:tmpl w:val="2A4AA970"/><w:lvl w:ilvl="0" w:tplc="0409000F"><w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="1" w:tplc="04090019"><w:start w:val="1"/><w:numFmt w:val="lowerLetter"/><w:lvlText w:val="%2."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="1440" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="2" w:tplc="0409001B" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerRoman"/><w:lvlText w:val="%3."/><w:lvlJc w:val="right"/><w:pPr><w:ind w:left="2160" w:hanging="180"/></w:pPr></w:lvl><w:lvl w:ilvl="3" w:tplc="0409000F" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%4."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="2880" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="4" w:tplc="04090019" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerLetter"/><w:lvlText w:val="%5."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="3600" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="5" w:tplc="0409001B" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerRoman"/><w:lvlText w:val="%6."/><w:lvlJc w:val="right"/><w:pPr><w:ind w:left="4320" w:hanging="180"/></w:pPr></w:lvl><w:lvl w:ilvl="6" w:tplc="0409000F" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%7."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="5040" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="7" w:tplc="04090019" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerLetter"/><w:lvlText w:val="%8."/><w:lvlJc w:val="left"/><w:pPr><w:ind w:left="5760" w:hanging="360"/></w:pPr></w:lvl><w:lvl w:ilvl="8" w:tplc="0409001B" w:tentative="1"><w:start w:val="1"/><w:numFmt w:val="lowerRoman"/><w:lvlText w:val="%9."/><w:lvlJc w:val="right"/><w:pPr><w:ind w:left="6480" w:hanging="180"/></w:pPr></w:lvl></w:abstractNum></w:numbering>)"
#define NUMBERING_XML R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:numbering xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas" xmlns:cx="http://schemas.microsoft.com/office/drawing/2014/chartex" xmlns:cx1="http://schemas.microsoft.com/office/drawing/2015/9/8/chartex" xmlns:cx2="http://schemas.microsoft.com/office/drawing/2015/10/21/chartex" xmlns:cx3="http://schemas.microsoft.com/office/drawing/2016/5/9/chartex" xmlns:cx4="http://schemas.microsoft.com/office/drawing/2016/5/10/chartex" xmlns:cx5="http://schemas.microsoft.com/office/drawing/2016/5/11/chartex" xmlns:cx6="http://schemas.microsoft.com/office/drawing/2016/5/12/chartex" xmlns:cx7="http://schemas.microsoft.com/office/drawing/2016/5/13/chartex" xmlns:cx8="http://schemas.microsoft.com/office/drawing/2016/5/14/chartex" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" xmlns:aink="http://schemas.microsoft.com/office/drawing/2016/ink" xmlns:am3d="http://schemas.microsoft.com/office/drawing/2017/model3d" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:oel="http://schemas.microsoft.com/office/2019/extlst" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:w15="http://schemas.microsoft.com/office/word/2012/wordml" xmlns:w16cex="http://schemas.microsoft.com/office/word/2018/wordml/cex" xmlns:w16cid="http://schemas.microsoft.com/office/word/2016/wordml/cid" xmlns:w16="http://schemas.microsoft.com/office/word/2018/wordml" xmlns:w16sdtdh="http://schemas.microsoft.com/office/word/2020/wordml/sdtdatahash" xmlns:w16se="http://schemas.microsoft.com/office/word/2015/wordml/symex" xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" xmlns:wpi="http://schemas.microsoft.com/office/word/2010/wordprocessingInk" xmlns:wne="http://schemas.microsoft.com/office/word/2006/wordml" xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" mc:Ignorable="w14 w15 w16se w16cid w16 w16cex w16sdtdh wp14"/>)"
#define STYLES_XML_1 R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:styles xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:w15="http://schemas.microsoft.com/office/word/2012/wordml" xmlns:w16cex="http://schemas.microsoft.com/office/word/2018/wordml/cex" xmlns:w16cid="http://schemas.microsoft.com/office/word/2016/wordml/cid" xmlns:w16="http://schemas.microsoft.com/office/word/2018/wordml" xmlns:w16sdtdh="http://schemas.microsoft.com/office/word/2020/wordml/sdtdatahash" xmlns:w16se="http://schemas.microsoft.com/office/word/2015/wordml/symex" mc:Ignorable="w14 w15 w16se w16cid w16 w16cex w16sdtdh"><w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:asciiTheme="minorHAnsi" w:eastAsiaTheme="minorEastAsia" w:hAnsiTheme="minorHAnsi" w:cstheme="minorBidi"/><w:kern w:val="2"/><w:sz w:val="22"/><w:szCs w:val="22"/><w:lang w:val="en-US" w:eastAsia="en-US" w:bidi="ar-SA"/><w14:ligatures w14:val="standardContextual"/></w:rPr></w:rPrDefault><w:pPrDefault><w:pPr><w:spacing w:after="160" w:line="259" w:lineRule="auto"/></w:pPr></w:pPrDefault></w:docDefaults><w:latentStyles w:defLockedState="0" w:defUIPriority="99" w:defSemiHidden="0" w:defUnhideWhenUsed="0" w:defQFormat="0" w:count="376"><w:lsdException w:name="Normal" w:uiPriority="0" w:qFormat="1"/><w:lsdException w:name="heading 1" w:uiPriority="9" w:qFormat="1"/><w:lsdException w:name="heading 2" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="heading 3" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="heading 4" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="heading 5" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="heading 6" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="heading 7" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="heading 8" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="heading 9" w:semiHidden="1" w:uiPriority="9" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="index 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 6" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 7" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 8" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index 9" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 1" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 2" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 3" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 4" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 5" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 6" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 7" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 8" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="toc 9" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1"/><w:lsdException w:name="Normal Indent" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="footnote text" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="annotation text" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="header" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="footer" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="index heading" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="caption" w:semiHidden="1" w:uiPriority="35" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="table of figures" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="envelope address" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="envelope return" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="footnote reference" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="annotation reference" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="line number" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="page number" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="endnote reference" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="endnote text" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="table of authorities" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="macro" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="toa heading" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Bullet" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Number" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Bullet 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Bullet 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Bullet 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Bullet 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Number 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Number 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Number 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Number 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Title" w:uiPriority="10" w:qFormat="1"/><w:lsdException w:name="Closing" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Signature" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Default Paragraph Font" w:semiHidden="1" w:uiPriority="1")"
#define STYLES_XML_2 R"( w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text Indent" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Continue" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Continue 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Continue 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Continue 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="List Continue 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Message Header" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Subtitle" w:uiPriority="11" w:qFormat="1"/><w:lsdException w:name="Salutation" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Date" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text First Indent" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text First Indent 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Note Heading" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text Indent 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Body Text Indent 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Block Text" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Hyperlink" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="FollowedHyperlink" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Strong" w:uiPriority="22" w:qFormat="1"/><w:lsdException w:name="Emphasis" w:uiPriority="20" w:qFormat="1"/><w:lsdException w:name="Document Map" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Plain Text" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="E-mail Signature" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Top of Form" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Bottom of Form" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Normal" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Acronym" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Address" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Cite" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Code" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Definition" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Keyboard" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Preformatted" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Sample" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Typewriter" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="HTML Variable" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Normal Table" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="annotation subject" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="No List" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Outline List 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Outline List 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Outline List 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Simple 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Simple 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Simple 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Classic 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Classic 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Classic 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Classic 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Colorful 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Colorful 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Colorful 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Columns 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Columns 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Columns 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Columns 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Columns 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 6" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 7" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid 8" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 4" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 5" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 6" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 7" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table List 8" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table 3D effects 1" w:semiHidden="1" )"
#define STYLES_XML_3 R"(w:unhideWhenUsed="1"/><w:lsdException w:name="Table 3D effects 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table 3D effects 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Contemporary" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Elegant" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Professional" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Subtle 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Subtle 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Web 1" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Web 2" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Web 3" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Balloon Text" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Table Grid" w:uiPriority="39"/><w:lsdException w:name="Table Theme" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Placeholder Text" w:semiHidden="1"/><w:lsdException w:name="No Spacing" w:uiPriority="1" w:qFormat="1"/><w:lsdException w:name="Light Shading" w:uiPriority="60"/><w:lsdException w:name="Light List" w:uiPriority="61"/><w:lsdException w:name="Light Grid" w:uiPriority="62"/><w:lsdException w:name="Medium Shading 1" w:uiPriority="63"/><w:lsdException w:name="Medium Shading 2" w:uiPriority="64"/><w:lsdException w:name="Medium List 1" w:uiPriority="65"/><w:lsdException w:name="Medium List 2" w:uiPriority="66"/><w:lsdException w:name="Medium Grid 1" w:uiPriority="67"/><w:lsdException w:name="Medium Grid 2" w:uiPriority="68"/><w:lsdException w:name="Medium Grid 3" w:uiPriority="69"/><w:lsdException w:name="Dark List" w:uiPriority="70"/><w:lsdException w:name="Colorful Shading" w:uiPriority="71"/><w:lsdException w:name="Colorful List" w:uiPriority="72"/><w:lsdException w:name="Colorful Grid" w:uiPriority="73"/><w:lsdException w:name="Light Shading Accent 1" w:uiPriority="60"/><w:lsdException w:name="Light List Accent 1" w:uiPriority="61"/><w:lsdException w:name="Light Grid Accent 1" w:uiPriority="62"/><w:lsdException w:name="Medium Shading 1 Accent 1" w:uiPriority="63"/><w:lsdException w:name="Medium Shading 2 Accent 1" w:uiPriority="64"/><w:lsdException w:name="Medium List 1 Accent 1" w:uiPriority="65"/><w:lsdException w:name="Revision" w:semiHidden="1"/><w:lsdException w:name="List Paragraph" w:uiPriority="34" w:qFormat="1"/><w:lsdException w:name="Quote" w:uiPriority="29" w:qFormat="1"/><w:lsdException w:name="Intense Quote" w:uiPriority="30" w:qFormat="1"/><w:lsdException w:name="Medium List 2 Accent 1" w:uiPriority="66"/><w:lsdException w:name="Medium Grid 1 Accent 1" w:uiPriority="67"/><w:lsdException w:name="Medium Grid 2 Accent 1" w:uiPriority="68"/><w:lsdException w:name="Medium Grid 3 Accent 1" w:uiPriority="69"/><w:lsdException w:name="Dark List Accent 1" w:uiPriority="70"/><w:lsdException w:name="Colorful Shading Accent 1" w:uiPriority="71"/><w:lsdException w:name="Colorful List Accent 1" w:uiPriority="72"/><w:lsdException w:name="Colorful Grid Accent 1" w:uiPriority="73"/><w:lsdException w:name="Light Shading Accent 2" w:uiPriority="60"/><w:lsdException w:name="Light List Accent 2" w:uiPriority="61"/><w:lsdException w:name="Light Grid Accent 2" w:uiPriority="62"/><w:lsdException w:name="Medium Shading 1 Accent 2" w:uiPriority="63"/><w:lsdException w:name="Medium Shading 2 Accent 2" w:uiPriority="64"/><w:lsdException w:name="Medium List 1 Accent 2" w:uiPriority="65"/><w:lsdException w:name="Medium List 2 Accent 2" w:uiPriority="66"/><w:lsdException w:name="Medium Grid 1 Accent 2" w:uiPriority="67"/><w:lsdException w:name="Medium Grid 2 Accent 2" w:uiPriority="68"/><w:lsdException w:name="Medium Grid 3 Accent 2" w:uiPriority="69"/><w:lsdException w:name="Dark List Accent 2" w:uiPriority="70"/><w:lsdException w:name="Colorful Shading Accent 2" w:uiPriority="71"/><w:lsdException w:name="Colorful List Accent 2" w:uiPriority="72"/><w:lsdException w:name="Colorful Grid Accent 2" w:uiPriority="73"/><w:lsdException w:name="Light Shading Accent 3" w:uiPriority="60"/><w:lsdException w:name="Light List Accent 3" w:uiPriority="61"/><w:lsdException w:name="Light Grid Accent 3" w:uiPriority="62"/><w:lsdException w:name="Medium Shading 1 Accent 3" w:uiPriority="63"/><w:lsdException w:name="Medium Shading 2 Accent 3" w:uiPriority="64"/><w:lsdException w:name="Medium List 1 Accent 3" w:uiPriority="65"/><w:lsdException w:name="Medium List 2 Accent 3" w:uiPriority="66"/><w:lsdException w:name="Medium Grid 1 Accent 3" w:uiPriority="67"/><w:lsdException w:name="Medium Grid 2 Accent 3" w:uiPriority="68"/><w:lsdException w:name="Medium Grid 3 Accent 3" w:uiPriority="69"/><w:lsdException w:name="Dark List Accent 3" w:uiPriority="70"/><w:lsdException w:name="Colorful Shading Accent 3" w:uiPriority="71"/><w:lsdException w:name="Colorful List Accent 3" w:uiPriority="72"/><w:lsdException w:name="Colorful Grid Accent 3" w:uiPriority="73"/><w:lsdException w:name="Light Shading Accent 4" w:uiPriority="60"/><w:lsdException w:name="Light List Accent 4" w:uiPriority="61"/><w:lsdException w:name="Light Grid Accent 4" w:uiPriority="62"/><w:lsdException w:name="Medium Shading 1 Accent 4" w:uiPriority="63"/><w:lsdException w:name="Medium Shading 2 Accent 4" w:uiPriority="64"/><w:lsdException w:name="Medium List 1 Accent 4" w:uiPriority="65"/><w:lsdException w:name="Medium List 2 Accent 4" w:uiPriority="66"/><w:lsdException w:name="Medium Grid 1 Accent 4" w:uiPriority="67"/><w:lsdException w:name="Medium Grid 2 Accent 4" w:uiPriority="68"/><w:lsdException w:name="Medium Grid 3 Accent 4" w:uiPriority="69"/><w:lsdException w:name="Dark List Accent 4" w:uiPriority="70"/><w:lsdException w:name="Colorful Shading Accent 4" w:uiPriority="71"/><w:lsdException w:name="Colorful List Accent 4" w:uiPriority="72"/><w:lsdException w:name="Colorful Grid Accent 4" w:uiPriority="73"/><w:lsdException w:name="Light Shading Accent 5" w:uiPriority="60"/><w:lsdException w:name="Light List Accent 5" w:uiPriority="61"/><w:lsdException w:name="Light Grid Accent 5" w:uiPriority="62"/><w:lsdException w:name="Medium Shading 1 Accent 5" w:uiPriority="63"/><w:lsdException w:name="Medium Shading 2 Accent 5" w:uiPriority="64"/><w:lsdException w:name="Medium List 1 Accent 5" w:uiPriority="65"/><w:lsdException w:name="Medium List 2 Accent 5" w:uiPriority="66"/><w:lsdException w:name="Medium)"
#define STYLES_XML_4 R"( Grid 1 Accent 5" w:uiPriority="67"/><w:lsdException w:name="Medium Grid 2 Accent 5" w:uiPriority="68"/><w:lsdException w:name="Medium Grid 3 Accent 5" w:uiPriority="69"/><w:lsdException w:name="Dark List Accent 5" w:uiPriority="70"/><w:lsdException w:name="Colorful Shading Accent 5" w:uiPriority="71"/><w:lsdException w:name="Colorful List Accent 5" w:uiPriority="72"/><w:lsdException w:name="Colorful Grid Accent 5" w:uiPriority="73"/><w:lsdException w:name="Light Shading Accent 6" w:uiPriority="60"/><w:lsdException w:name="Light List Accent 6" w:uiPriority="61"/><w:lsdException w:name="Light Grid Accent 6" w:uiPriority="62"/><w:lsdException w:name="Medium Shading 1 Accent 6" w:uiPriority="63"/><w:lsdException w:name="Medium Shading 2 Accent 6" w:uiPriority="64"/><w:lsdException w:name="Medium List 1 Accent 6" w:uiPriority="65"/><w:lsdException w:name="Medium List 2 Accent 6" w:uiPriority="66"/><w:lsdException w:name="Medium Grid 1 Accent 6" w:uiPriority="67"/><w:lsdException w:name="Medium Grid 2 Accent 6" w:uiPriority="68"/><w:lsdException w:name="Medium Grid 3 Accent 6" w:uiPriority="69"/><w:lsdException w:name="Dark List Accent 6" w:uiPriority="70"/><w:lsdException w:name="Colorful Shading Accent 6" w:uiPriority="71"/><w:lsdException w:name="Colorful List Accent 6" w:uiPriority="72"/><w:lsdException w:name="Colorful Grid Accent 6" w:uiPriority="73"/><w:lsdException w:name="Subtle Emphasis" w:uiPriority="19" w:qFormat="1"/><w:lsdException w:name="Intense Emphasis" w:uiPriority="21" w:qFormat="1"/><w:lsdException w:name="Subtle Reference" w:uiPriority="31" w:qFormat="1"/><w:lsdException w:name="Intense Reference" w:uiPriority="32" w:qFormat="1"/><w:lsdException w:name="Book Title" w:uiPriority="33" w:qFormat="1"/><w:lsdException w:name="Bibliography" w:semiHidden="1" w:uiPriority="37" w:unhideWhenUsed="1"/><w:lsdException w:name="TOC Heading" w:semiHidden="1" w:uiPriority="39" w:unhideWhenUsed="1" w:qFormat="1"/><w:lsdException w:name="Plain Table 1" w:uiPriority="41"/><w:lsdException w:name="Plain Table 2" w:uiPriority="42"/><w:lsdException w:name="Plain Table 3" w:uiPriority="43"/><w:lsdException w:name="Plain Table 4" w:uiPriority="44"/><w:lsdException w:name="Plain Table 5" w:uiPriority="45"/><w:lsdException w:name="Grid Table Light" w:uiPriority="40"/><w:lsdException w:name="Grid Table 1 Light" w:uiPriority="46"/><w:lsdException w:name="Grid Table 2" w:uiPriority="47"/><w:lsdException w:name="Grid Table 3" w:uiPriority="48"/><w:lsdException w:name="Grid Table 4" w:uiPriority="49"/><w:lsdException w:name="Grid Table 5 Dark" w:uiPriority="50"/><w:lsdException w:name="Grid Table 6 Colorful" w:uiPriority="51"/><w:lsdException w:name="Grid Table 7 Colorful" w:uiPriority="52"/><w:lsdException w:name="Grid Table 1 Light Accent 1" w:uiPriority="46"/><w:lsdException w:name="Grid Table 2 Accent 1" w:uiPriority="47"/><w:lsdException w:name="Grid Table 3 Accent 1" w:uiPriority="48"/><w:lsdException w:name="Grid Table 4 Accent 1" w:uiPriority="49"/><w:lsdException w:name="Grid Table 5 Dark Accent 1" w:uiPriority="50"/><w:lsdException w:name="Grid Table 6 Colorful Accent 1" w:uiPriority="51"/><w:lsdException w:name="Grid Table 7 Colorful Accent 1" w:uiPriority="52"/><w:lsdException w:name="Grid Table 1 Light Accent 2" w:uiPriority="46"/><w:lsdException w:name="Grid Table 2 Accent 2" w:uiPriority="47"/><w:lsdException w:name="Grid Table 3 Accent 2" w:uiPriority="48"/><w:lsdException w:name="Grid Table 4 Accent 2" w:uiPriority="49"/><w:lsdException w:name="Grid Table 5 Dark Accent 2" w:uiPriority="50"/><w:lsdException w:name="Grid Table 6 Colorful Accent 2" w:uiPriority="51"/><w:lsdException w:name="Grid Table 7 Colorful Accent 2" w:uiPriority="52"/><w:lsdException w:name="Grid Table 1 Light Accent 3" w:uiPriority="46"/><w:lsdException w:name="Grid Table 2 Accent 3" w:uiPriority="47"/><w:lsdException w:name="Grid Table 3 Accent 3" w:uiPriority="48"/><w:lsdException w:name="Grid Table 4 Accent 3" w:uiPriority="49"/><w:lsdException w:name="Grid Table 5 Dark Accent 3" w:uiPriority="50"/><w:lsdException w:name="Grid Table 6 Colorful Accent 3" w:uiPriority="51"/><w:lsdException w:name="Grid Table 7 Colorful Accent 3" w:uiPriority="52"/><w:lsdException w:name="Grid Table 1 Light Accent 4" w:uiPriority="46"/><w:lsdException w:name="Grid Table 2 Accent 4" w:uiPriority="47"/><w:lsdException w:name="Grid Table 3 Accent 4" w:uiPriority="48"/><w:lsdException w:name="Grid Table 4 Accent 4" w:uiPriority="49"/><w:lsdException w:name="Grid Table 5 Dark Accent 4" w:uiPriority="50"/><w:lsdException w:name="Grid Table 6 Colorful Accent 4" w:uiPriority="51"/><w:lsdException w:name="Grid Table 7 Colorful Accent 4" w:uiPriority="52"/><w:lsdException w:name="Grid Table 1 Light Accent 5" w:uiPriority="46"/><w:lsdException w:name="Grid Table 2 Accent 5" w:uiPriority="47"/><w:lsdException w:name="Grid Table 3 Accent 5" w:uiPriority="48"/><w:lsdException w:name="Grid Table 4 Accent 5" w:uiPriority="49"/><w:lsdException w:name="Grid Table 5 Dark Accent 5" w:uiPriority="50"/><w:lsdException w:name="Grid Table 6 Colorful Accent 5" w:uiPriority="51"/><w:lsdException w:name="Grid Table 7 Colorful Accent 5" w:uiPriority="52"/><w:lsdException w:name="Grid Table 1 Light Accent 6" w:uiPriority="46"/><w:lsdException w:name="Grid Table 2 Accent 6" w:uiPriority="47"/><w:lsdException w:name="Grid Table 3 Accent 6" w:uiPriority="48"/><w:lsdException w:name="Grid Table 4 Accent 6" w:uiPriority="49"/><w:lsdException w:name="Grid Table 5 Dark Accent 6" w:uiPriority="50"/><w:lsdException w:name="Grid Table 6 Colorful Accent 6" w:uiPriority="51"/><w:lsdException w:name="Grid Table 7 Colorful Accent 6" w:uiPriority="52"/><w:lsdException w:name="List Table 1 Light" w:uiPriority="46"/><w:lsdException w:name="List Table 2" w:uiPriority="47"/><w:lsdException w:name="List Table 3" w:uiPriority="48"/><w:lsdException w:name="List Table 4" w:uiPriority="49"/><w:lsdException w:name="List Table 5 Dark" w:uiPriority="50"/><w:lsdException w:name="List Table 6 Colorful" w:uiPriority="51"/><w:lsdException w:name="List Table 7 Colorful" w:uiPriority="52"/><w:lsdException w:name="List Table 1 Light Accent 1" w:uiPriority="46"/><w:lsdException w:name="List Table 2 Accent 1" w:uiPriority="47"/><w:lsdException w:name="List Table 3 Accent 1" w:uiPriority="48"/><w:lsdException w:name="List Table 4 Accent 1" w:uiPriority="49"/><w:lsdException w:name="List Table 5 Dark Accent 1" w:uiPriority="50"/><w:lsdException w:name="List Table 6)"
#define STYLES_XML_5 R"( Colorful Accent 1" w:uiPriority="51"/><w:lsdException w:name="List Table 7 Colorful Accent 1" w:uiPriority="52"/><w:lsdException w:name="List Table 1 Light Accent 2" w:uiPriority="46"/><w:lsdException w:name="List Table 2 Accent 2" w:uiPriority="47"/><w:lsdException w:name="List Table 3 Accent 2" w:uiPriority="48"/><w:lsdException w:name="List Table 4 Accent 2" w:uiPriority="49"/><w:lsdException w:name="List Table 5 Dark Accent 2" w:uiPriority="50"/><w:lsdException w:name="List Table 6 Colorful Accent 2" w:uiPriority="51"/><w:lsdException w:name="List Table 7 Colorful Accent 2" w:uiPriority="52"/><w:lsdException w:name="List Table 1 Light Accent 3" w:uiPriority="46"/><w:lsdException w:name="List Table 2 Accent 3" w:uiPriority="47"/><w:lsdException w:name="List Table 3 Accent 3" w:uiPriority="48"/><w:lsdException w:name="List Table 4 Accent 3" w:uiPriority="49"/><w:lsdException w:name="List Table 5 Dark Accent 3" w:uiPriority="50"/><w:lsdException w:name="List Table 6 Colorful Accent 3" w:uiPriority="51"/><w:lsdException w:name="List Table 7 Colorful Accent 3" w:uiPriority="52"/><w:lsdException w:name="List Table 1 Light Accent 4" w:uiPriority="46"/><w:lsdException w:name="List Table 2 Accent 4" w:uiPriority="47"/><w:lsdException w:name="List Table 3 Accent 4" w:uiPriority="48"/><w:lsdException w:name="List Table 4 Accent 4" w:uiPriority="49"/><w:lsdException w:name="List Table 5 Dark Accent 4" w:uiPriority="50"/><w:lsdException w:name="List Table 6 Colorful Accent 4" w:uiPriority="51"/><w:lsdException w:name="List Table 7 Colorful Accent 4" w:uiPriority="52"/><w:lsdException w:name="List Table 1 Light Accent 5" w:uiPriority="46"/><w:lsdException w:name="List Table 2 Accent 5" w:uiPriority="47"/><w:lsdException w:name="List Table 3 Accent 5" w:uiPriority="48"/><w:lsdException w:name="List Table 4 Accent 5" w:uiPriority="49"/><w:lsdException w:name="List Table 5 Dark Accent 5" w:uiPriority="50"/><w:lsdException w:name="List Table 6 Colorful Accent 5" w:uiPriority="51"/><w:lsdException w:name="List Table 7 Colorful Accent 5" w:uiPriority="52"/><w:lsdException w:name="List Table 1 Light Accent 6" w:uiPriority="46"/><w:lsdException w:name="List Table 2 Accent 6" w:uiPriority="47"/><w:lsdException w:name="List Table 3 Accent 6" w:uiPriority="48"/><w:lsdException w:name="List Table 4 Accent 6" w:uiPriority="49"/><w:lsdException w:name="List Table 5 Dark Accent 6" w:uiPriority="50"/><w:lsdException w:name="List Table 6 Colorful Accent 6" w:uiPriority="51"/><w:lsdException w:name="List Table 7 Colorful Accent 6" w:uiPriority="52"/><w:lsdException w:name="Mention" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Smart Hyperlink" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Hashtag" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Unresolved Mention" w:semiHidden="1" w:unhideWhenUsed="1"/><w:lsdException w:name="Smart Link" w:semiHidden="1" w:unhideWhenUsed="1"/></w:latentStyles><w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:qFormat/></w:style><w:style w:type="character" w:default="1" w:styleId="DefaultParagraphFont"><w:name w:val="Default Paragraph Font"/><w:uiPriority w:val="1"/><w:semiHidden/><w:unhideWhenUsed/></w:style><w:style w:type="table" w:default="1" w:styleId="TableNormal"><w:name w:val="Normal Table"/><w:uiPriority w:val="99"/><w:semiHidden/><w:unhideWhenUsed/><w:tblPr><w:tblInd w:w="0" w:type="dxa"/><w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="108" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/><w:right w:w="108" w:type="dxa"/></w:tblCellMar></w:tblPr></w:style><w:style w:type="numbering" w:default="1" w:styleId="NoList"><w:name w:val="No List"/><w:uiPriority w:val="99"/><w:semiHidden/><w:unhideWhenUsed/></w:style></w:styles>)"
#define SETTINGS_XML R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:settings xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" xmlns:w15="http://schemas.microsoft.com/office/word/2012/wordml" xmlns:w16cex="http://schemas.microsoft.com/office/word/2018/wordml/cex" xmlns:w16cid="http://schemas.microsoft.com/office/word/2016/wordml/cid" xmlns:w16="http://schemas.microsoft.com/office/word/2018/wordml" xmlns:w16sdtdh="http://schemas.microsoft.com/office/word/2020/wordml/sdtdatahash" xmlns:w16se="http://schemas.microsoft.com/office/word/2015/wordml/symex" xmlns:sl="http://schemas.openxmlformats.org/schemaLibrary/2006/main" mc:Ignorable="w14 w15 w16se w16cid w16 w16cex w16sdtdh"><w:zoom w:percent="100"/><w:proofState w:spelling="clean" w:grammar="clean"/><w:defaultTabStop w:val="720"/><w:characterSpacingControl w:val="doNotCompress"/><w:compat><w:useFELayout/><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="12"/><w:compatSetting w:name="useWord2013TrackBottomHyphenation" w:uri="http://schemas.microsoft.com/office/word" w:val="1"/></w:compat><w:rsids><w:rsidRoot w:val="00711754"/><w:rsid w:val="005A16A0"/><w:rsid w:val="00711754"/></w:rsids><m:mathPr><m:mathFont m:val="Cambria Math"/><m:brkBin m:val="before"/><m:brkBinSub m:val="--"/><m:smallFrac m:val="0"/><m:dispDef/><m:lMargin m:val="0"/><m:rMargin m:val="0"/><m:defJc m:val="centerGroup"/><m:wrapIndent m:val="1440"/><m:intLim m:val="subSup"/><m:naryLim m:val="undOvr"/></m:mathPr><w:themeFontLang w:val="en-US"/><w:clrSchemeMapping w:bg1="light1" w:t1="dark1" w:bg2="light2" w:t2="dark2" w:accent1="accent1" w:accent2="accent2" w:accent3="accent3" w:accent4="accent4" w:accent5="accent5" w:accent6="accent6" w:hyperlink="hyperlink" w:followedHyperlink="followedHyperlink"/><w:shapeDefaults><o:shapedefaults v:ext="edit" spidmax="1026"/><o:shapelayout v:ext="edit"><o:idmap v:ext="edit" data="1"/></o:shapelayout></w:shapeDefaults><w:decimalSymbol w:val="."/><w:listSeparator w:val=","/><w14:docId w14:val="13A67980"/><w15:docId w15:val="{329DCD25-C76C-4DF3-A865-B9F3B644E005}"/></w:settings>)"
#define THEME_XML R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="Office Theme"><a:themeElements><a:clrScheme name="Office"><a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1><a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1><a:dk2><a:srgbClr val="44546A"/></a:dk2><a:lt2><a:srgbClr val="E7E6E6"/></a:lt2><a:accent1><a:srgbClr val="4472C4"/></a:accent1><a:accent2><a:srgbClr val="ED7D31"/></a:accent2><a:accent3><a:srgbClr val="A5A5A5"/></a:accent3><a:accent4><a:srgbClr val="FFC000"/></a:accent4><a:accent5><a:srgbClr val="5B9BD5"/></a:accent5><a:accent6><a:srgbClr val="70AD47"/></a:accent6><a:hlink><a:srgbClr val="0563C1"/></a:hlink><a:folHlink><a:srgbClr val="954F72"/></a:folHlink></a:clrScheme><a:fontScheme name="Office"><a:majorFont><a:latin typeface="Calibri Light" panose="020F0302020204030204"/><a:ea typeface=""/><a:cs typeface=""/><a:font script="Jpan" typeface="游ゴシック Light"/><a:font script="Hang" typeface="맑은 고딕"/><a:font script="Hans" typeface="等线 Light"/><a:font script="Hant" typeface="新細明體"/><a:font script="Arab" typeface="Times New Roman"/><a:font script="Hebr" typeface="Times New Roman"/><a:font script="Thai" typeface="Angsana New"/><a:font script="Ethi" typeface="Nyala"/><a:font script="Beng" typeface="Vrinda"/><a:font script="Gujr" typeface="Shruti"/><a:font script="Khmr" typeface="MoolBoran"/><a:font script="Knda" typeface="Tunga"/><a:font script="Guru" typeface="Raavi"/><a:font script="Cans" typeface="Euphemia"/><a:font script="Cher" typeface="Plantagenet Cherokee"/><a:font script="Yiii" typeface="Microsoft Yi Baiti"/><a:font script="Tibt" typeface="Microsoft Himalaya"/><a:font script="Thaa" typeface="MV Boli"/><a:font script="Deva" typeface="Mangal"/><a:font script="Telu" typeface="Gautami"/><a:font script="Taml" typeface="Latha"/><a:font script="Syrc" typeface="Estrangelo Edessa"/><a:font script="Orya" typeface="Kalinga"/><a:font script="Mlym" typeface="Kartika"/><a:font script="Laoo" typeface="DokChampa"/><a:font script="Sinh" typeface="Iskoola Pota"/><a:font script="Mong" typeface="Mongolian Baiti"/><a:font script="Viet" typeface="Times New Roman"/><a:font script="Uigh" typeface="Microsoft Uighur"/><a:font script="Geor" typeface="Sylfaen"/><a:font script="Armn" typeface="Arial"/><a:font script="Bugi" typeface="Leelawadee UI"/><a:font script="Bopo" typeface="Microsoft JhengHei"/><a:font script="Java" typeface="Javanese Text"/><a:font script="Lisu" typeface="Segoe UI"/><a:font script="Mymr" typeface="Myanmar Text"/><a:font script="Nkoo" typeface="Ebrima"/><a:font script="Olck" typeface="Nirmala UI"/><a:font script="Osma" typeface="Ebrima"/><a:font script="Phag" typeface="Phagspa"/><a:font script="Syrn" typeface="Estrangelo Edessa"/><a:font script="Syrj" typeface="Estrangelo Edessa"/><a:font script="Syre" typeface="Estrangelo Edessa"/><a:font script="Sora" typeface="Nirmala UI"/><a:font script="Tale" typeface="Microsoft Tai Le"/><a:font script="Talu" typeface="Microsoft New Tai Lue"/><a:font script="Tfng" typeface="Ebrima"/></a:majorFont><a:minorFont><a:latin typeface="Calibri" panose="020F0502020204030204"/><a:ea typeface=""/><a:cs typeface=""/><a:font script="Jpan" typeface="游明朝"/><a:font script="Hang" typeface="맑은 고딕"/><a:font script="Hans" typeface="等线"/><a:font script="Hant" typeface="新細明體"/><a:font script="Arab" typeface="Arial"/><a:font script="Hebr" typeface="Arial"/><a:font script="Thai" typeface="Cordia New"/><a:font script="Ethi" typeface="Nyala"/><a:font script="Beng" typeface="Vrinda"/><a:font script="Gujr" typeface="Shruti"/><a:font script="Khmr" typeface="DaunPenh"/><a:font script="Knda" typeface="Tunga"/><a:font script="Guru" typeface="Raavi"/><a:font script="Cans" typeface="Euphemia"/><a:font script="Cher" typeface="Plantagenet Cherokee"/><a:font script="Yiii" typeface="Microsoft Yi Baiti"/><a:font script="Tibt" typeface="Microsoft Himalaya"/><a:font script="Thaa" typeface="MV Boli"/><a:font script="Deva" typeface="Mangal"/><a:font script="Telu" typeface="Gautami"/><a:font script="Taml" typeface="Latha"/><a:font script="Syrc" typeface="Estrangelo Edessa"/><a:font script="Orya" typeface="Kalinga"/><a:font script="Mlym" typeface="Kartika"/><a:font script="Laoo" typeface="DokChampa"/><a:font script="Sinh" typeface="Iskoola Pota"/><a:font script="Mong" typeface="Mongolian Baiti"/><a:font script="Viet" typeface="Arial"/><a:font script="Uigh" typeface="Microsoft Uighur"/><a:font script="Geor" typeface="Sylfaen"/><a:font script="Armn" typeface="Arial"/><a:font script="Bugi" typeface="Leelawadee UI"/><a:font script="Bopo" typeface="Microsoft JhengHei"/><a:font script="Java" typeface="Javanese Text"/><a:font script="Lisu" typeface="Segoe UI"/><a:font script="Mymr" typeface="Myanmar Text"/><a:font script="Nkoo" typeface="Ebrima"/><a:font script="Olck" typeface="Nirmala UI"/><a:font script="Osma" typeface="Ebrima"/><a:font script="Phag" typeface="Phagspa"/><a:font script="Syrn" typeface="Estrangelo Edessa"/><a:font script="Syrj" typeface="Estrangelo Edessa"/><a:font script="Syre" typeface="Estrangelo Edessa"/><a:font script="Sora" typeface="Nirmala UI"/><a:font script="Tale" typeface="Microsoft Tai Le"/><a:font script="Talu" typeface="Microsoft New Tai Lue"/><a:font script="Tfng" typeface="Ebrima"/></a:minorFont></a:fontScheme><a:fmtScheme name="Office"><a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:gradFill rotWithShape="1"><a:gsLst><a:gs pos="0"><a:schemeClr val="phClr"><a:lumMod val="110000"/><a:satMod val="105000"/><a:tint val="67000"/></a:schemeClr></a:gs><a:gs pos="50000"><a:schemeClr val="phClr"><a:lumMod val="105000"/><a:satMod val="103000"/><a:tint val="73000"/></a:schemeClr></a:gs><a:gs pos="100000"><a:schemeClr val="phClr"><a:lumMod val="105000"/><a:satMod val="109000"/><a:tint val="81000"/></a:schemeClr></a:gs></a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill><a:gradFill rotWithShape="1"><a:gsLst><a:gs pos="0"><a:schemeClr val="phClr"><a:satMod val="103000"/><a:lumMod val="102000"/><a:tint val="94000"/></a:schemeClr></a:gs><a:gs pos="50000"><a:schemeClr val="phClr"><a:satMod val="110000"/><a:lumMod val="100000"/><a:shade val="100000"/></a:schemeClr></a:gs><a:gs pos="100000"><a:schemeClr val="phClr"><a:lumMod val="99000"/><a:satMod val="120000"/><a:shade val="78000"/></a:schemeClr></a:gs></a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill></a:fillStyleLst><a:lnStyleLst><a:ln w="6350" cap="flat" cmpd="sng" algn="ctr"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/><a:miter lim="800000"/></a:ln><a:ln w="12700" cap="flat" cmpd="sng" algn="ctr"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/><a:miter lim="800000"/></a:ln><a:ln w="19050" cap="flat" cmpd="sng" algn="ctr"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:prstDash val="solid"/><a:miter lim="800000"/></a:ln></a:lnStyleLst><a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst><a:outerShdw blurRad="57150" dist="19050" dir="5400000" algn="ctr" rotWithShape="0"><a:srgbClr val="000000"><a:alpha val="63000"/></a:srgbClr></a:outerShdw></a:effectLst></a:effectStyle></a:effectStyleLst><a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"><a:tint val="95000"/><a:satMod val="170000"/></a:schemeClr></a:solidFill><a:gradFill rotWithShape="1"><a:gsLst><a:gs pos="0"><a:schemeClr val="phClr"><a:tint val="93000"/><a:satMod val="150000"/><a:shade val="98000"/><a:lumMod val="102000"/></a:schemeClr></a:gs><a:gs pos="50000"><a:schemeClr val="phClr"><a:tint val="98000"/><a:satMod val="130000"/><a:shade val="90000"/><a:lumMod val="103000"/></a:schemeClr></a:gs><a:gs pos="100000"><a:schemeClr val="phClr"><a:shade val="63000"/><a:satMod val="120000"/></a:schemeClr></a:gs></a:gsLst><a:lin ang="5400000" scaled="0"/></a:gradFill></a:bgFillStyleLst></a:fmtScheme></a:themeElements><a:objectDefaults/><a:extraClrSchemeLst/><a:extLst><a:ext uri="{05A4C25C-085E-4340-85A3-A5531E510DB2}"><thm15:themeFamily xmlns:thm15="http://schemas.microsoft.com/office/thememl/2012/main" name="Office Theme" id="{62F939B6-93AF-4DB8-9C6B-D6C7DFDC589F}" vid="{4A3C46E8-61CC-4603-A589-7422A47A8E4A}"/></a:ext></a:extLst></a:theme>)"

// #define NUMBERING_ALPHA_XML R"(<w:abstractNum w:abstractNumId="0" w15:restartNumberingAfterBreak="0"> <w:nsid w:val="0E4350D6"/> <w:multiLevelType w:val="hybridMultilevel"/> <w:tmpl w:val="C9D0A764"/> <w:lvl w:ilvl="0" w:tplc="04090015"> <w:start w:val="1"/> <w:numFmt w:val="upperLetter"/> <w:lvlText w:val="%1."/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="720" w:hanging="360"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="1" w:tplc="04090019" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="lowerLetter"/> <w:lvlText w:val="%2."/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="1440" w:hanging="360"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="2" w:tplc="0409001B" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="lowerRoman"/> <w:lvlText w:val="%3."/> <w:lvlJc w:val="right"/> <w:pPr> <w:ind w:left="2160" w:hanging="180"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="3" w:tplc="0409000F" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%4."/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="2880" w:hanging="360"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="4" w:tplc="04090019" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="lowerLetter"/> <w:lvlText w:val="%5."/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="3600" w:hanging="360"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="5" w:tplc="0409001B" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="lowerRoman"/> <w:lvlText w:val="%6."/> <w:lvlJc w:val="right"/> <w:pPr> <w:ind w:left="4320" w:hanging="180"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="6" w:tplc="0409000F" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%7."/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="5040" w:hanging="360"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="7" w:tplc="04090019" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="lowerLetter"/> <w:lvlText w:val="%8."/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="5760" w:hanging="360"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="8" w:tplc="0409001B" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="lowerRoman"/> <w:lvlText w:val="%9."/> <w:lvlJc w:val="right"/> <w:pPr> <w:ind w:left="6480" w:hanging="180"/> </w:pPr> </w:lvl> </w:abstractNum>)"
// #define NUMBERING_DECIMAL_XML R"(<w:abstractNum w:abstractNumId="1" w15:restartNumberingAfterBreak="0"> <w:nsid w:val="4ADE7E68"/> <w:multiLevelType w:val="multilevel"/> <w:tmpl w:val="4F34F910"/> <w:lvl w:ilvl="0"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%1."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="720"/> </w:tabs> <w:ind w:left="720" w:hanging="720"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="1"> <w:start w:val="1"/> <w:numFmt w:val="lowerLetter"/> <w:lvlText w:val="%2."/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="1080" w:hanging="360"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="2"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%3."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="2160"/> </w:tabs> <w:ind w:left="2160" w:hanging="720"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="3"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%4."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="2880"/> </w:tabs> <w:ind w:left="2880" w:hanging="720"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="4"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%5."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="3600"/> </w:tabs> <w:ind w:left="3600" w:hanging="720"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="5"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%6."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="4320"/> </w:tabs> <w:ind w:left="4320" w:hanging="720"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="6"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%7."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="5040"/> </w:tabs> <w:ind w:left="5040" w:hanging="720"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="7"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%8."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="5760"/> </w:tabs> <w:ind w:left="5760" w:hanging="720"/> </w:pPr> </w:lvl> <w:lvl w:ilvl="8"> <w:start w:val="1"/> <w:numFmt w:val="decimal"/> <w:lvlText w:val="%9."/> <w:lvlJc w:val="left"/> <w:pPr> <w:tabs> <w:tab w:val="num" w:pos="6480"/> </w:tabs> <w:ind w:left="6480" w:hanging="720"/> </w:pPr> </w:lvl> </w:abstractNum>)"
// #define NUMBERING_BULLET_XML R"(<w:abstractNum w:abstractNumId="2" w15:restartNumberingAfterBreak="0"> <w:nsid w:val="5A7E5C08"/> <w:multiLevelType w:val="hybridMultilevel"/> <w:tmpl w:val="B67C5632"/> <w:lvl w:ilvl="0" w:tplc="04090001"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="ï‚·"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="720" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="1" w:tplc="04090003" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="o"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="1440" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" w:cs="Courier New" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="2" w:tplc="04090005" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="ï‚§"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="2160" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Wingdings" w:hAnsi="Wingdings" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="3" w:tplc="04090001" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="ï‚·"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="2880" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="4" w:tplc="04090003" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="o"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="3600" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" w:cs="Courier New" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="5" w:tplc="04090005" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="ï‚§"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="4320" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Wingdings" w:hAnsi="Wingdings" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="6" w:tplc="04090001" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="ï‚·"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="5040" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="7" w:tplc="04090003" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="o"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="5760" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Courier New" w:hAnsi="Courier New" w:cs="Courier New" w:hint="default"/> </w:rPr> </w:lvl> <w:lvl w:ilvl="8" w:tplc="04090005" w:tentative="1"> <w:start w:val="1"/> <w:numFmt w:val="bullet"/> <w:lvlText w:val="ï‚§"/> <w:lvlJc w:val="left"/> <w:pPr> <w:ind w:left="6480" w:hanging="360"/> </w:pPr> <w:rPr> <w:rFonts w:ascii="Wingdings" w:hAnsi="Wingdings" w:hint="default"/> </w:rPr> </w:lvl> </w:abstractNum>)"

namespace docx {
    int Pt2Twip(const double pt) {
        return pt * 20;
    };
    double Twip2Pt(const int twip) {
        return twip / 20.0;
    };
    double Inch2Pt(const double inch) {
        return inch * 72;
    };
    double Pt2Inch(const double pt) {
        return pt / 72;
    };
    double MM2Inch(const int mm) {
        return mm / 25.4;
    };
    int Inch2MM(const double inch) {
        return inch * 25.4;
    };
    double CM2Inch(const double cm) {
        return cm / 2.54;
    };
    double Inch2CM(const double inch) {
        return inch * 2.54;
    };
    int Inch2Twip(const double inch) {
        return inch * 1440;
    };
    double Twip2Inch(const int twip) {
        return twip / 1440.0;
    };
    int MM2Twip(const int mm) {
        return Inch2Twip(MM2Inch(mm));
    };
    int Twip2MM(const int twip) {
        return Inch2MM(Twip2Inch(twip));
    };
    int CM2Twip(const double cm) {
        return Inch2Twip(CM2Inch(cm));
    };
    double Twip2CM(const int twip) {
        return Inch2CM(Twip2Inch(twip));
    };

    struct xml_string_writer : pugi::xml_writer {
        ::std::string result;

        virtual void write(const void* data, size_t size)
        {
            result.append(static_cast<const char*>(data), size);
        }
    };
    class Document::Impl {
    public:
        ::std::string      path_;
        pugi::xml_document numbers_;
        pugi::xml_document doc_;
        pugi::xml_node     w_body_;
        pugi::xml_node     w_sectPr_;

        TextFormat format;
    };
    class Paragraph::Impl {
    public:
        pugi::xml_node w_body_;
        pugi::xml_node w_p_;
        pugi::xml_node w_pPr_;

        TextFormat format;
        cweeSharedPtr<class Document::Impl> Document;
    };
    class TextFrame::Impl {
    public:
        pugi::xml_node w_framePr_;
    };
    class Section::Impl {
    public:
        pugi::xml_node w_body_;
        pugi::xml_node w_p_;      // current paragraph
        pugi::xml_node w_pPr_;
        pugi::xml_node w_p_last_; // the last paragraph of the section
        pugi::xml_node w_pPr_last_;
        pugi::xml_node w_sectPr_;

        cweeSharedPtr<class Document::Impl> Document;
    };
    class Table::Impl {
    public:
        pugi::xml_node w_body_;
        pugi::xml_node w_tbl_;
        pugi::xml_node w_tblPr_;
        pugi::xml_node w_tblGrid_;

        int rows_ = 0;
        int cols_ = 0;
        Grid grid_; // logical grid

        cweeSharedPtr<class Document::Impl> Document;
    };
    class Run::Impl {
    public:
        pugi::xml_node w_p_;
        pugi::xml_node w_r_;
        pugi::xml_node w_rPr_;

        TextFormat format;
    };
    class TableCell::Impl {
    public:
        Cell* c_;
        pugi::xml_node w_tr_;
        pugi::xml_node w_tc_;
        pugi::xml_node w_tcPr_;

        cweeSharedPtr<class Document::Impl> Document;
    };

    pugi::xml_node GetLastChild(pugi::xml_node node, const char* name)
    {
        pugi::xml_node child = node.last_child();
        while (!child.empty() && ::strcmp(name, child.name()) != 0) {
            child = child.previous_sibling(name);
        }
        return child;
    }
    void SetBorders(pugi::xml_node& w_bdrs, const char* elemName, const Box::BorderStyle style, const double width, const char* color) {
        auto w_bdr = w_bdrs.child(elemName);
        if (!w_bdr) {
            w_bdr = w_bdrs.append_child(elemName);
        }

        const char* val = nullptr;
        switch (style) {
        case Box::BorderStyle::Single:
            val = "single";
            break;
        case Box::BorderStyle::Dotted:
            val = "dotted";
            break;
        case Box::BorderStyle::DotDash:
            val = "dotDash";
            break;
        case Box::BorderStyle::Dashed:
            val = "dashed";
            break;
        case Box::BorderStyle::Double:
            val = "double";
            break;
        case Box::BorderStyle::Wave:
            val = "wave";
            break;
        case Box::BorderStyle::None:
            val = "none";
            break;
        }

        auto w_val = w_bdr.attribute("w:val");
        if (!w_val) {
            w_val = w_bdr.append_attribute("w:val");
        }
        w_val.set_value(val);

        auto w_sz = w_bdr.attribute("w:sz");
        if (!w_sz) {
            w_sz = w_bdr.append_attribute("w:sz");
        }
        w_sz.set_value(width * 8);

        auto w_color = w_bdr.attribute("w:color");
        if (!w_color) {
            w_color = w_bdr.append_attribute("w:color");
        }
        w_color.set_value(color);
    };

    ::std::ostream& operator<<(::std::ostream& out, const Document& doc)
    {
        if (doc.doc_impl_) {
            xml_string_writer writer;
            doc.impl_()->w_body_.print(writer, "  ");
            out << writer.result;
        }
        else {
            out << "<document />";
        }
        return out;
    }
    ::std::ostream& operator<<(::std::ostream& out, const Paragraph& p)
    {
        if (p.impl) {
            xml_string_writer writer;
            p.impl_()->w_p_.print(writer, "  ");
            out << writer.result;
        }
        else {
            out << "<paragraph />";
        }
        return out;
    }
    ::std::ostream& operator<<(::std::ostream& out, const Run& r)
    {
        if (r.impl) {
            xml_string_writer writer;
            r.impl_()->w_r_.print(writer, "  ");
            out << writer.result;
        }
        else {
            out << "<run />";
        }
        return out;
    }
    ::std::ostream& operator<<(::std::ostream& out, const Section& s)
    {
        if (s.impl) {
            xml_string_writer writer;
            s.impl_()->w_p_.print(writer, "  ");
            out << writer.result;
        }
        else {
            out << "<section />";
        }
        return out;
    }

    // Document
    Document::Document() {
        doc_impl_ = cweeSharedPtr<void>(cweeSharedPtr<Document::Impl>(new Document::Impl(), [](void* p) { return p; }));

        AUTO fp{ fileSystem->createRandomFile(".docx") }; // creates the file itself and returns the path. this will be an un-initialized file, however, and it must be initialized before it's able to be used as a docx format (atypical for most file formats). 
        
        impl_()->numbers_.load_buffer(NUMBERING_XML, ::std::strlen(NUMBERING_XML), pugi::parse_declaration);
        impl_()->doc_.load_buffer(DOCUMENT_XML, ::std::strlen(DOCUMENT_XML), pugi::parse_declaration);
        impl_()->w_body_ = impl_()->doc_.child("w:document").child("w:body");
        impl_()->w_sectPr_ = impl_()->w_body_.child("w:sectPr");
        impl_()->path_ = fp.c_str();

        Save(fp.c_str());
    };
    Document::Document(Document const& right) : doc_impl_(right.doc_impl_) {};
    Document& Document::operator=(Document const& right) {
        doc_impl_ = right.doc_impl_;
        return *this;
    };
    Document::Document(const ::std::string& path) {
        doc_impl_ = cweeSharedPtr<void>(cweeSharedPtr<Document::Impl>(new Document::Impl(), [](void* p) { return p; }));
        impl_()->numbers_.load_buffer(NUMBERING_XML, ::std::strlen(NUMBERING_XML), pugi::parse_declaration);
        impl_()->doc_.load_buffer(DOCUMENT_XML, ::std::strlen(DOCUMENT_XML), pugi::parse_declaration);
        impl_()->w_body_ = impl_()->doc_.child("w:document").child("w:body");
        impl_()->w_sectPr_ = impl_()->w_body_.child("w:sectPr");
        impl_()->path_ = path;

        Open(path);
    };
    Document::Impl* Document::impl_() const {
       return static_cast<Document::Impl*>(doc_impl_.Get());
    };
    Document::~Document() {};
    bool Document::Save() {
        if (!doc_impl_) return false;

        ::std::string original_file = impl_()->path_;
        ::std::string temp_file = impl_()->path_ + ".tmp";

        zipper new_zip;
        // Create the file
        if (new_zip.open(temp_file.c_str(), false)) {
            if (new_zip.addEntry("_rels/.rels")) {
                new_zip << _RELS;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/document.xml")) {
                xml_string_writer writer;
                impl_()->doc_.save(writer, "", pugi::format_raw);
                const char* buf = writer.result.c_str();

                new_zip << buf;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/numbering.xml")) {
                xml_string_writer writer;
                impl_()->numbers_.save(writer, "", pugi::format_raw);
                const char* buf = writer.result.c_str();

                new_zip << buf;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/_rels/document.xml.rels")) {
                new_zip << DOCUMENT_XML_RELS;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/footer1.xml")) {
                new_zip << FOOTER1_XML;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/settings.xml")) {
                new_zip << SETTINGS_XML;
                new_zip.closeEntry();
            }            

            if (new_zip.addEntry("word/theme/theme1.xml")) {
                new_zip << THEME_XML;
                new_zip.closeEntry();
            }            

            if (new_zip.addEntry("word/styles.xml")) {
                new_zip << STYLES_XML_1;
                new_zip << STYLES_XML_2;
                new_zip << STYLES_XML_3;
                new_zip << STYLES_XML_4;
                new_zip << STYLES_XML_5;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("[Content_Types].xml")) {
                new_zip << CONTENT_TYPES_XML;
                new_zip.closeEntry();
            }



            unzipper orig_zip;
            // Open the original zip and copy all files which are not replaced by duckX
            if (orig_zip.open(original_file.c_str())) {
                // Loop & copy each relevant entry in the original zip
                for (auto& name : orig_zip.getFilenames()) {
                    if (name == "_rels/.rels") continue;
                    if (name == "word/document.xml") continue;
                    if (name == "word/numbering.xml") continue;
                    if (name == "word/_rels/document.xml.rels") continue;
                    if (name == "word/footer1.xml") continue;
                    if (name == "word/styles.xml") continue;
                    if (name == "word/theme/theme1.xml") continue;
                    if (name == "word/settings.xml") continue;
                    if (name == "[Content_Types].xml") continue;

                    // Skip copying the original file
                    if (orig_zip.openEntry(name.c_str())) {
                        // Read the old content
                        cweeSharedPtr<char> ptr; unsigned int bufSize;
                        if (orig_zip.ReadEntry(&ptr, &bufSize)) {
                            if (new_zip.addEntry(name.c_str())) {
                                new_zip.setEntry(ptr, bufSize);
                                new_zip.closeEntry();
                            }
                        }
                        orig_zip.closeEntry();
                    }

                }
                orig_zip.close();
            }

            new_zip.close();
        }

        // Remove original zip, rename to correct name
        remove(original_file.c_str());
        rename(temp_file.c_str(), original_file.c_str());

        return true;
    };
    bool Document::Save(const std::string& path) {
        if (!doc_impl_) return false;

        ::std::string original_file = impl_()->path_;
        ::std::string temp_file = impl_()->path_ + ".tmp";

        xml_string_writer writer;
        impl_()->doc_.save(writer, "", pugi::format_raw);
        const char* buf = writer.result.c_str();

        zipper new_zip;
        // Create the file
        if (new_zip.open(temp_file.c_str(), false)) {
            if (new_zip.addEntry("_rels/.rels")) {
                new_zip << _RELS;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/document.xml")) {
                xml_string_writer writer;
                impl_()->doc_.save(writer, "", pugi::format_raw);
                const char* buf = writer.result.c_str();

                new_zip << buf;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/numbering.xml")) {
                xml_string_writer writer;
                impl_()->numbers_.save(writer, "", pugi::format_raw);
                const char* buf = writer.result.c_str();

                new_zip << buf;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/_rels/document.xml.rels")) {
                new_zip << DOCUMENT_XML_RELS;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/footer1.xml")) {
                new_zip << FOOTER1_XML;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/settings.xml")) {
                new_zip << SETTINGS_XML;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/theme/theme1.xml")) {
                new_zip << THEME_XML;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("word/styles.xml")) {
                new_zip << STYLES_XML_1;
                new_zip << STYLES_XML_2;
                new_zip << STYLES_XML_3;
                new_zip << STYLES_XML_4;
                new_zip << STYLES_XML_5;
                new_zip.closeEntry();
            }

            if (new_zip.addEntry("[Content_Types].xml")) {
                new_zip << CONTENT_TYPES_XML;
                new_zip.closeEntry();
            }


            unzipper orig_zip;
            // Open the original zip and copy all files which are not replaced by duckX
            if (orig_zip.open(original_file.c_str())) {
                // Loop & copy each relevant entry in the original zip
                for (auto& name : orig_zip.getFilenames()) {
                    if (name == "_rels/.rels") continue;
                    if (name == "word/document.xml") continue;
                    if (name == "word/numbering.xml") continue;
                    if (name == "word/_rels/document.xml.rels") continue;
                    if (name == "word/footer1.xml") continue;
                    if (name == "word/styles.xml") continue;
                    if (name == "word/theme/theme1.xml") continue;
                    if (name == "word/settings.xml") continue;
                    if (name == "[Content_Types].xml") continue;

                    // Skip copying the original file
                    if (orig_zip.openEntry(name.c_str())) {
                        // Read the old content
                        cweeSharedPtr<char> ptr; unsigned int bufSize;
                        if (orig_zip.ReadEntry(&ptr, &bufSize)) {
                            if (new_zip.addEntry(name.c_str())) {
                                new_zip.setEntry(ptr, bufSize);
                                new_zip.closeEntry();
                            }
                        }
                        orig_zip.closeEntry();
                    }

                }
                orig_zip.close();
            }

            new_zip.close();
        }

        // Remove original zip, rename to correct name
        remove(original_file.c_str());
        rename(temp_file.c_str(), path.c_str());

        impl_()->path_ = path; // set the new save path

        return true;
    };
    bool Document::Open(const ::std::string& path) {
        if (!doc_impl_) return false;

        void* buf = NULL;
        size_t bufsize;

        unzipper unzip;
        if (unzip.open(path.c_str())) {
            if (unzip.openEntry("word/document.xml")) {
                cweeSharedPtr<char> ptr;
                unsigned int bufSize;
                if (unzip.ReadEntry(&ptr, &bufSize)) {
                    buf = (void*)(char*)(ptr.Get());
                    bufsize = bufSize;

                    impl_()->doc_.load_buffer(buf, bufsize, pugi::parse_declaration);
                    impl_()->w_body_ = impl_()->doc_.child("w:document").child("w:body");
                    impl_()->w_sectPr_ = impl_()->w_body_.child("w:sectPr");
                }
                unzip.closeEntry();
            }
            if (unzip.openEntry("word/numbering.xml")) {
                cweeSharedPtr<char> ptr;
                unsigned int bufSize;
                if (unzip.ReadEntry(&ptr, &bufSize)) {
                    buf = (void*)(char*)(ptr.Get());
                    bufsize = bufSize;

                    impl_()->numbers_.load_buffer(buf, bufsize, pugi::parse_declaration);
                }
                unzip.closeEntry();
            }
            unzip.close();
        }

        return true;
    };
    Paragraph Document::FirstParagraph() {
        if (!doc_impl_) return Paragraph();
        auto w_p = impl_()->w_body_.child("w:p");
        if (!w_p) return Paragraph();

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_p.child("w:pPr");
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = this->doc_impl_;
        return out;
    };
    Paragraph Document::LastParagraph() {
        if (!doc_impl_) return Paragraph();
        auto w_p = GetLastChild(impl_()->w_body_, "w:p");
        if (!w_p) return Paragraph();

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_p.child("w:pPr");
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = this->doc_impl_;
        return out;
    };
    Section Document::FirstSection() {
        Paragraph firstParagraph = FirstParagraph();
        if (firstParagraph) return firstParagraph.GetSection();

        Section::Impl* impl = new Section::Impl;
        impl->w_body_ = impl_()->w_body_;
        Section section(impl);
        section.FindSectionProperties();
        section.impl_()->Document = this->doc_impl_;
        return section;
    };
    Section Document::LastSection() {
        Paragraph lastParagraph = LastParagraph();
        if (lastParagraph) return lastParagraph.GetSection();

        Section::Impl* impl = new Section::Impl;
        impl->w_body_ = impl_()->w_body_;
        Section section(impl);
        section.FindSectionProperties();
        section.impl_()->Document = this->doc_impl_;
        return section;
    };
    Paragraph Document::AppendParagraph() {
        if (!doc_impl_) return Paragraph();

        auto w_p = impl_()->w_body_.insert_child_before("w:p", impl_()->w_sectPr_);
        auto w_pPr = w_p.append_child("w:pPr");

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_pPr;
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = this->doc_impl_;
        return out;
    };
    Paragraph Document::AppendParagraph(const ::std::string& text) {
        auto p = AppendParagraph();
        p.AppendRun(text);
        return p;
    };
    Paragraph Document::AppendParagraph(const ::std::string& text, const double fontSize) {
        auto p = AppendParagraph();
        p.AppendRun(text, fontSize);
        return p;
    };
    Paragraph Document::AppendParagraph(const ::std::string& text, const double fontSize, const ::std::string& fontAscii, const ::std::string& fontEastAsia) {
        auto p = AppendParagraph();
        p.AppendRun(text, fontSize, fontAscii, fontEastAsia);
        return p;
    };
    Paragraph Document::PrependParagraph() {
        if (!doc_impl_) return Paragraph();

        auto w_p = impl_()->w_body_.prepend_child("w:p");
        auto w_pPr = w_p.append_child("w:pPr");

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_pPr;
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = this->doc_impl_;
        return out;
    };
    Paragraph Document::PrependParagraph(const ::std::string& text) {
        auto p = PrependParagraph();
        p.AppendRun(text);
        return p;
    };
    Paragraph Document::PrependParagraph(const ::std::string& text, const double fontSize) {
        auto p = PrependParagraph();
        p.AppendRun(text, fontSize);
        return p;
    };
    Paragraph Document::PrependParagraph(const ::std::string& text, const double fontSize, const ::std::string& fontAscii, const ::std::string& fontEastAsia) {
        auto p = PrependParagraph();
        p.AppendRun(text, fontSize, fontAscii, fontEastAsia);
        return p;
    };
    Paragraph Document::InsertParagraphBefore(const Paragraph& p) {
        if (!doc_impl_) return Paragraph();

        auto w_p = impl_()->w_body_.insert_child_before("w:p", p.impl_()->w_p_);
        auto w_pPr = w_p.append_child("w:pPr");

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_pPr;
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = this->doc_impl_;
        return out;
    };
    Paragraph Document::InsertParagraphAfter(const Paragraph& p) {
        if (!doc_impl_) return Paragraph();

        auto w_p = impl_()->w_body_.insert_child_after("w:p", p.impl_()->w_p_);
        auto w_pPr = w_p.append_child("w:pPr");

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_pPr;
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = this->doc_impl_;
        return out;
    };
    bool Document::RemoveParagraph(Paragraph& p) {
        if (!doc_impl_) return false;
        return impl_()->w_body_.remove_child(p.impl_()->w_p_);
    };
    Paragraph Document::AppendPageBreak() {
        auto p = AppendParagraph();
        p.AppendPageBreak();
        return p;
    };
    Section Document::AppendSectionBreak() {
        auto p = AppendParagraph();
        return p.InsertSectionBreak();
    };
    Table Document::AppendTable(const int rows, const int cols) {
        if (!doc_impl_) return Table();

        auto w_tbl = impl_()->w_body_.insert_child_before("w:tbl", impl_()->w_sectPr_);
        auto w_tblPr = w_tbl.append_child("w:tblPr");
        auto w_tblGrid = w_tbl.append_child("w:tblGrid");

        Table::Impl* impl = new Table::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_tbl_ = w_tbl;
        impl->w_tblPr_ = w_tblPr;
        impl->w_tblGrid_ = w_tblGrid;
        impl->Document = this->doc_impl_;
        Table tbl(impl);

        tbl.Create_(rows, cols);
        tbl.SetAllBorders();
        tbl.SetWidthPercent(100);
        tbl.SetCellMarginLeft(CM2Twip(0.19));
        tbl.SetCellMarginRight(CM2Twip(0.19));
        tbl.SetAlignment(Table::Alignment::Centered);
        return tbl;
    };
    void Document::RemoveTable(Table& tbl) {
        if (!doc_impl_) return;
        impl_()->w_body_.remove_child(tbl.impl_()->w_tbl_);
    };
    void Document::SetFontSize(const double fontSize) {
        if (!doc_impl_) return;
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetFontSize(fontSize);
        }
        impl_()->format.fontSize = fontSize;
    };
    void Document::SetFont(const std::string& fontAscii, const std::string& fontEastAsia) {
        if (!doc_impl_) return;
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetFont(fontAscii, fontEastAsia);
        }
        impl_()->format.fontFamily = fontAscii;
    };
    void Document::SetFontStyle(const FontStyle fontStyle) {
        if (!doc_impl_) return;
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetFontStyle(fontStyle);
        }
        impl_()->format.fontStyle = fontStyle;
    };
    void Document::SetCharacterSpacing(const int characterSpacing) {
        if (!doc_impl_) return;
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetCharacterSpacing(characterSpacing);
        }
        impl_()->format.characterSpacing = characterSpacing;
    };
    std::string Document::GetText() {
        if (!doc_impl_) return "";

        ::std::string text;
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            text += r.GetText();
        }
        return text;
    };
    TextFormat* Document::Format() {
        if (!doc_impl_) return nullptr;
    };
    TextFrame Document::AppendTextFrame(const int w, const int h)
    {
        if (!doc_impl_) return TextFrame();

        auto w_p = impl_()->w_body_.insert_child_before("w:p", impl_()->w_sectPr_);
        auto w_pPr = w_p.append_child("w:pPr");
        auto w_framePr = w_pPr.append_child("w:framePr");


        Paragraph::Impl* paragraph_impl = new Paragraph::Impl;
        paragraph_impl->w_body_ = impl_()->w_body_;
        paragraph_impl->w_p_ = w_p;
        paragraph_impl->w_pPr_ = w_pPr;

        TextFrame::Impl* impl = new TextFrame::Impl;
        impl->w_framePr_ = w_framePr;
        auto textFrame = TextFrame(impl, paragraph_impl);

        *textFrame.Format() = impl_()->format;

        textFrame.SetSize(w, h);
        textFrame.SetBorders();
        return textFrame;
    }
    void Document::SetAlignment(const Paragraph::Alignment alignment){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetAlignment(alignment);
        }
    };
    void Document::SetLineSpacingSingle(){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetLineSpacingSingle();
        }
    };               // Single
    void Document::SetLineSpacingLines(const double at){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetLineSpacingLines(at);
        }
    }; // 1.5 lines, Double (2 lines), Multiple (3 lines)
    void Document::SetLineSpacingAtLeast(const int at){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetLineSpacingAtLeast(at);
        }
    };  // At Least
    void Document::SetLineSpacingExactly(const int at){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetLineSpacingExactly(at);
        }
    };  // Exactly
    void Document::SetLineSpacing(const int at, std::string const& lineRule){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetLineSpacing(at, lineRule);
        }
    };
    void Document::SetBeforeSpacingAuto(){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetBeforeSpacingAuto();
        }
    };
    void Document::SetAfterSpacingAuto(){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetAfterSpacingAuto();
        }
    };
    void Document::SetSpacingAuto(std::string const& attrNameAuto){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetSpacingAuto(attrNameAuto);
        }
    };
    void Document::SetBeforeSpacingLines(const double beforeSpacing){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetBeforeSpacingLines(beforeSpacing);
        }
    };
    void Document::SetAfterSpacingLines(const double afterSpacing){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetAfterSpacingLines(afterSpacing);
        }
    };
    void Document::SetBeforeSpacing(const int beforeSpacing){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetBeforeSpacing(beforeSpacing);
        }
    };
    void Document::SetAfterSpacing(const int afterSpacing){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetAfterSpacing(afterSpacing);
        }
    };
    void Document::SetSpacing(const int twip, std::string const& attrNameAuto, std::string const& attrName){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetSpacing(twip, attrNameAuto, attrName);
        }
    };
    void Document::SetLeftIndentChars(const double leftIndent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetLeftIndentChars(leftIndent);
        }
    };
    void Document::SetRightIndentChars(const double rightIndent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetRightIndentChars(rightIndent);
        }
    };
    void Document::SetLeftIndent(const int leftIndent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetLeftIndent(leftIndent);
        }
    };
    void Document::SetRightIndent(const int rightIndent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetRightIndent(rightIndent);
        }
    };
    void Document::SetFirstLineChars(const double indent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetFirstLineChars(indent);
        }
    };
    void Document::SetHangingChars(const double indent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetHangingChars(indent);
        }
    };
    void Document::SetFirstLine(const int indent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetFirstLine(indent);
        }
    };
    void Document::SetHanging(const int indent){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetHanging(indent);
        }
    };
    void Document::SetIndent(const int indent, std::string const& attrName){
        for (auto r = FirstParagraph(); r; r = r.Next()) {
            r.SetIndent(indent, attrName);
        }
    };

    // Paragraph
    Paragraph::Paragraph() : impl(nullptr) {};
    Paragraph::Paragraph(Impl* implP) : impl(cweeSharedPtr<void>(cweeSharedPtr<Impl>(implP, [](void* p) { return p; }))) {};
    Paragraph::Paragraph(const Paragraph& p) : impl(p.impl) {};
    Paragraph::~Paragraph() {};
    Run Paragraph::FirstRun()
    {
        if (!impl) return Run();
        auto w_r = impl_()->w_p_.child("w:r");
        if (!w_r) return Run();

        Run::Impl* impl = new Run::Impl;
        impl->w_p_ = impl_()->w_p_;
        impl->w_r_ = w_r;
        impl->w_rPr_ = w_r.child("w:rPr");
        impl->format = impl_()->format;
        auto out{ Run(impl) };

        out.SetCharacterSpacing(impl_()->format.characterSpacing);
        out.SetFont(impl_()->format.fontFamily, "");
        out.SetFontSize(impl_()->format.fontSize);
        out.SetFontStyle(impl_()->format.fontStyle);

        return out;
    }
    Run Paragraph::AppendRun()
    {
        if (!impl) return Run();
        auto w_r = impl_()->w_p_.append_child("w:r");
        auto w_rPr = w_r.append_child("w:rPr");

        Run::Impl* impl = new Run::Impl;
        impl->w_p_ = impl_()->w_p_;
        impl->w_r_ = w_r;
        impl->w_rPr_ = w_rPr;
        impl->format = impl_()->format;
        auto out{ Run(impl) };

        out.SetCharacterSpacing(impl_()->format.characterSpacing);
        out.SetFont(impl_()->format.fontFamily, "");
        out.SetFontSize(impl_()->format.fontSize);
        out.SetFontStyle(impl_()->format.fontStyle);

        return out;
    }
    Run Paragraph::AppendRun(const ::std::string& text)
    {
        auto r = AppendRun();
        if (!text.empty()) {
            r.AppendText(text);
        }
        return r;
    }
    Run Paragraph::AppendRun(const ::std::string& text, const double fontSize)
    {
        if (impl) impl_()->format.fontSize = fontSize;

        auto r = AppendRun(text);
        if (fontSize != 0) {
            r.SetFontSize(fontSize);
        }
        return r;
    }
    Run Paragraph::AppendRun(const ::std::string& text, const double fontSize, const ::std::string& fontAscii, const ::std::string& fontEastAsia)
    {
        if (impl) impl_()->format.fontSize = fontSize;
        if (impl) impl_()->format.fontFamily = fontAscii;

        auto r = AppendRun(text, fontSize);
        if (!fontAscii.empty()) {
            r.SetFont(fontAscii, fontEastAsia);
        }
        return r;
    }
    Run Paragraph::AppendPageBreak()
    {
        if (!impl) return Run();
        auto w_r = impl_()->w_p_.append_child("w:r");
        auto w_br = w_r.append_child("w:br");
        w_br.append_attribute("w:type") = "page";

        Run::Impl* impl = new Run::Impl;
        impl->w_p_ = impl_()->w_p_;
        impl->w_r_ = w_r;
        impl->w_rPr_ = w_br;
        auto out{ Run(impl) };

        out.SetCharacterSpacing(impl_()->format.characterSpacing);
        out.SetFont(impl_()->format.fontFamily, "");
        out.SetFontSize(impl_()->format.fontSize);
        out.SetFontStyle(impl_()->format.fontStyle);

        return out;
    }    
 
    void SetLvlNode(pugi::xml_node& w_lvl, int level, Paragraph::BulletType type) {
        w_lvl.emplace_attribute("w:ilvl").set_value(level);
        w_lvl.emplace_child("w:start").emplace_attribute("w:val").set_value(1);

        double indent = 360.0;
        double hanging = 360.0;

        if (type == Paragraph::BulletType::Number) {
            if (level == 1) {
                w_lvl.emplace_attribute("w:tplc").set_value("0409000F");
                w_lvl.emplace_child("w:numFmt").emplace_attribute("w:val").set_value("lowerLetter");
            }
            else {
                w_lvl.emplace_attribute("w:tplc").set_value("04090019");
                w_lvl.emplace_child("w:numFmt").emplace_attribute("w:val").set_value("decimal");
            }
            cweeStr w_lvltext = cweeStr("%") + cweeStr::printf("%i.", level + 1);
            w_lvl.emplace_child("w:lvlText").emplace_attribute("w:val").set_value(w_lvltext.c_str());

            w_lvl.emplace_child("w:lvlJc").emplace_attribute("w:val").set_value("left");

            auto w_ind = w_lvl.emplace_child("w:pPr").emplace_child("w:ind");
            w_ind.emplace_attribute("w:left").set_value(indent + (level * indent));
            w_ind.emplace_attribute("w:hanging").set_value(hanging);
        }
        else if (type == Paragraph::BulletType::Bullet) {
            w_lvl.emplace_child("w:numFmt").emplace_attribute("w:val").set_value("bullet");
            w_lvl.emplace_child("w:lvlJc").emplace_attribute("w:val").set_value("left");

            if ((level == 0) || level % 3 == 0) {
                w_lvl.emplace_child("w:lvlText").emplace_attribute("w:val").set_value("●");
                w_lvl.emplace_attribute("w:tplc").set_value("04090005");
            }
            else if (level % 2 == 0) {
                w_lvl.emplace_child("w:lvlText").emplace_attribute("w:val").set_value("-");
                w_lvl.emplace_attribute("w:tplc").set_value("04090003");
            }
            else {
                w_lvl.emplace_child("w:lvlText").emplace_attribute("w:val").set_value("o");
                w_lvl.emplace_attribute("w:tplc").set_value("04090001");
            }

            auto w_ind = w_lvl.emplace_child("w:pPr").emplace_child("w:ind");
            w_ind.emplace_attribute("w:left").set_value(indent + (level * indent));
            w_ind.emplace_attribute("w:hanging").set_value(hanging);
        }
        else if (type == Paragraph::BulletType::Alpha) {
            if (level == 0) {
                w_lvl.emplace_attribute("w:tplc").set_value("04090015");
                w_lvl.emplace_child("w:numFmt").emplace_attribute("w:val").set_value("upperLetter");
                w_lvl.emplace_child("w:lvlJc").emplace_attribute("w:val").set_value("left");

                auto w_ind = w_lvl.emplace_child("w:pPr").emplace_child("w:ind");
                w_ind.emplace_attribute("w:left").set_value(indent + (level * indent));
                w_ind.emplace_attribute("w:hanging").set_value(hanging);
            }
            else {
                if (level % 3 == 0) { // decimal
                    w_lvl.emplace_attribute("w:tplc").set_value("04090019");
                    w_lvl.emplace_child("w:numFmt").emplace_attribute("w:val").set_value("decimal");
                    w_lvl.emplace_child("w:lvlJc").emplace_attribute("w:val").set_value("left");

                    auto w_ind = w_lvl.emplace_child("w:pPr").emplace_child("w:ind");
                    w_ind.emplace_attribute("w:left").set_value(indent + (level * indent));
                    w_ind.emplace_attribute("w:hanging").set_value(hanging);
                }
                else if (level % 2 == 0) { // lowerRoman
                    w_lvl.emplace_attribute("w:tplc").set_value("0409001B");
                    w_lvl.emplace_child("w:numFmt").emplace_attribute("w:val").set_value("lowerRoman");
                    w_lvl.emplace_child("w:lvlJc").emplace_attribute("w:val").set_value("right");

                    auto w_ind = w_lvl.emplace_child("w:pPr").emplace_child("w:ind");
                    w_ind.emplace_attribute("w:left").set_value(indent + (level * indent));
                    w_ind.emplace_attribute("w:hanging").set_value(180);
                }
                else { // lowerLetter
                    w_lvl.emplace_attribute("w:tplc").set_value("0409000F");
                    w_lvl.emplace_child("w:numFmt").emplace_attribute("w:val").set_value("lowerLetter");
                    w_lvl.emplace_child("w:lvlJc").emplace_attribute("w:val").set_value("left");

                    auto w_ind = w_lvl.emplace_child("w:pPr").emplace_child("w:ind");
                    w_ind.emplace_attribute("w:left").set_value(indent + (level * indent));
                    w_ind.emplace_attribute("w:hanging").set_value(hanging);
                }
            }
            cweeStr w_lvltext = cweeStr("%") + cweeStr::printf("%i.", level + 1);
            w_lvl.emplace_child("w:lvlText").emplace_attribute("w:val").set_value(w_lvltext.c_str());
        }
    };

    pugi::xml_node AppendAbstractNumList(pugi::xml_node& w_numbering, const int listNumber, const int indentLevel, Paragraph::BulletType type, cweeStr NSID) {
        int newAbstractNum = -1;
        for (auto& child : w_numbering.children("w:abstractNum")) {
            auto w_abstractNumId = child.attribute("w:abstractNumId");
            if (w_abstractNumId) {
                int abstractNum = std::atoi(w_abstractNumId.value());
                if (abstractNum >= newAbstractNum) newAbstractNum = abstractNum;
            }
        }
        newAbstractNum += 1;

        auto type_str = std::to_string(static_cast<int>(type));

        auto w_abstractNum = w_numbering.prepend_child("w:abstractNum");
        w_abstractNum.append_attribute("w:abstractNumId").set_value(newAbstractNum);
        w_abstractNum.append_attribute("w15:restartNumberingAfterBreak").set_value(0);

        w_abstractNum.append_child("w:multiLevelType").append_attribute("w:val").set_value("hybridMultilevel");

        w_abstractNum.append_child("w:nsid").append_attribute("w:val").set_value(NSID.c_str());
        w_abstractNum.append_child("w:tmpl").append_attribute("w:val").set_value("E164460E");

        for (int level = 0; level <= 8; level++) {
            pugi::xml_node w_lvl = w_abstractNum.append_child("w:lvl");            
            SetLvlNode(w_lvl, level, type);
        }

        return w_abstractNum;
    };      

    void Paragraph::SetNumberedList(const int listNumber, const int indentLevel, Paragraph::BulletType type) {
        if (!impl) return;
        if (impl_()->Document && impl_()->Document->numbers_) {
            auto numbers = impl_()->Document->numbers_.child("w:numbering");
            if (numbers) { // add new nodes
                cweeStr desiredNSID = cweeStr::printf("%08d", listNumber);                
                desiredNSID[0] = '0' + static_cast<int>(type);                
                pugi::xml_node abstractChild;
                for (auto& child : numbers.children("w:abstractNum")) {
                    if (child) {
                        auto w_nsid = child.child("w:nsid");
                        if (w_nsid) {
                            AUTO nsid = w_nsid.attribute("w:val").value();
                            if (desiredNSID == nsid) {
                                abstractChild = child;
                                break;
                            }
                        }
                    }
                }
                if (!abstractChild) {
                    abstractChild = AppendAbstractNumList(numbers, listNumber, indentLevel, type, desiredNSID);
                }
                
                auto toFind = std::to_string(listNumber);
                auto nonAbstractChild = numbers.find_child_by_attribute("w:numId", toFind.c_str());
                if (!nonAbstractChild) {
                    nonAbstractChild = numbers.append_child("w:num"); {
                        nonAbstractChild.append_attribute("w:numId").set_value(listNumber);
                        nonAbstractChild.append_child("w:abstractNumId").append_attribute("w:val").set_value(abstractChild.attribute("w:abstractNumId").value());
                    }
                }
                else {
                    toFind = std::to_string(indentLevel);
                    pugi::xml_node w_lvl = abstractChild.find_child_by_attribute("w:ilvl", toFind.c_str());
                    SetLvlNode(w_lvl, indentLevel, type);
                }
            }
        }

        auto elemNumPr = impl_()->w_pPr_.child("w:numPr");
        if (!elemNumPr) elemNumPr = impl_()->w_pPr_.append_child("w:numPr");        
        {
            auto wilvl = elemNumPr.child("w:ilvl");
            if (!wilvl) wilvl = elemNumPr.append_child("w:ilvl");            
            {
                auto styleVal = wilvl.attribute("w:val");
                if (!styleVal) {
                    styleVal = wilvl.append_attribute("w:val");
                }
                styleVal.set_value(indentLevel);
            }

            auto wiNum = elemNumPr.child("w:numId");
            if (!wiNum) wiNum = elemNumPr.append_child("w:numId");
            {
                auto styleVal = wiNum.attribute("w:val");
                if (!styleVal) styleVal = wiNum.append_attribute("w:val");                
                styleVal.set_value(listNumber);
            }
        }
    };

    void Paragraph::SetNumberedList(const int listNumber, const int indentLevel) {
        if (!impl) return;
        if (impl_()->Document && impl_()->Document->numbers_) {
            auto numbers = impl_()->Document->numbers_.child("w:numbering");
            if (numbers) { // add new nodes
                cweeStr desiredNSID = cweeStr::printf("%08d", listNumber);                
                pugi::xml_node abstractChild;
                Paragraph::BulletType type = Paragraph::BulletType::Number;

                for (auto& child : numbers.children("w:abstractNum")) {
                    if (child) {
                        auto w_nsid = child.child("w:nsid");
                        if (w_nsid) {
                            AUTO nsid = w_nsid.attribute("w:val").value();
                            if (desiredNSID.Right(7) == cweeStr(nsid).Right(7)) {
                                abstractChild = child;
                                type = static_cast<decltype(type)>((int)(cweeStr((char)(nsid[0])).ReturnNumeric()));
                                break;
                            }
                        }
                    }
                }
                
                desiredNSID[0] = '0' + static_cast<int>(type);

                if (!abstractChild) {
                    abstractChild = AppendAbstractNumList(numbers, listNumber, indentLevel, type, desiredNSID);
                }
                AUTO nonAbstractChild = numbers.append_child("w:num"); {
                    nonAbstractChild.append_attribute("w:numId").set_value(listNumber);
                    nonAbstractChild.append_child("w:abstractNumId").append_attribute("w:val").set_value(abstractChild.attribute("w:abstractNumId").value());
                }
            }
        }
        auto elemNumPr = impl_()->w_pPr_.child("w:numPr");
        if (!elemNumPr) elemNumPr = impl_()->w_pPr_.append_child("w:numPr");
        {
            auto wilvl = elemNumPr.child("w:ilvl");
            if (!wilvl) wilvl = elemNumPr.append_child("w:ilvl");
            {
                auto styleVal = wilvl.attribute("w:val");
                if (!styleVal) {
                    styleVal = wilvl.append_attribute("w:val");
                }
                styleVal.set_value(indentLevel);
            }

            auto wiNum = elemNumPr.child("w:numId");
            if (!wiNum) wiNum = elemNumPr.append_child("w:numId");
            {
                auto styleVal = wiNum.attribute("w:val");
                if (!styleVal) styleVal = wiNum.append_attribute("w:val");
                styleVal.set_value(listNumber);
            }
        }
    };

    void Paragraph::SetAlignment(const Alignment alignment) {
        if (!impl) return;

        const char* val{ nullptr };
        switch (alignment) {
        case Alignment::Left:
            val = "start";
            break;
        case Alignment::Right:
            val = "end";
            break;
        case Alignment::Centered:
            val = "center";
            break;
        case Alignment::Justified:
            val = "both";
            break;
        case Alignment::Distributed:
            val = "distribute";
            break;
        }

        auto jc = impl_()->w_pPr_.child("w:jc");
        if (!jc) {
            jc = impl_()->w_pPr_.append_child("w:jc");
        }
        auto jcVal = jc.attribute("w:val");
        if (!jcVal) {
            jcVal = jc.append_attribute("w:val");
        }
        jcVal.set_value(val);
    }
    void Paragraph::SetLineSpacingSingle()
    {
        if (!impl) return;
        auto spacing = impl_()->w_pPr_.child("w:spacing");
        if (!spacing) return;
        auto spacingLineRule = spacing.attribute("w:lineRule");
        if (spacingLineRule) {
            spacing.remove_attribute(spacingLineRule);
        }
        auto spacingLine = spacing.attribute("w:line");
        if (spacingLine) {
            spacing.remove_attribute(spacingLine);
        }
    }
    void Paragraph::SetLineSpacingLines(const double at)
    {
        // A normal single-spaced paragaph has a w:line value of 240, or 12 points.
        // 
        // If the value of lineRule is auto, then the value of line 
        // is interpreted as 240th of a line, e.g. 360 = 1.5 lines.
        SetLineSpacing(at * 240, "auto");
    }
    void Paragraph::SetLineSpacingAtLeast(const int at)
    {
        // If the value of the lineRule attribute is atLeast or exactly, 
        // then the value of the line attribute is interpreted as 240th of a point.
        // (Not really. Actually, values are in twentieths of a point, e.g. 240 = 12 pt.)
        SetLineSpacing(at, "atLeast");
    }
    void Paragraph::SetLineSpacingExactly(const int at)
    {
        SetLineSpacing(at, "exact");
    }
    void Paragraph::SetLineSpacing(const int at, std::string const& lineRule)
    {
        if (!impl) return;
        auto spacing = impl_()->w_pPr_.child("w:spacing");
        if (!spacing) {
            spacing = impl_()->w_pPr_.append_child("w:spacing");
        }

        auto spacingLineRule = spacing.attribute("w:lineRule");
        if (!spacingLineRule) {
            spacingLineRule = spacing.append_attribute("w:lineRule");
        }

        auto spacingLine = spacing.attribute("w:line");
        if (!spacingLine) {
            spacingLine = spacing.append_attribute("w:line");
        }

        spacingLineRule.set_value(lineRule.c_str());
        spacingLine.set_value(at);
    }
    void Paragraph::SetBeforeSpacingAuto()
    {
        SetSpacingAuto("w:beforeAutospacing");
    }
    void Paragraph::SetAfterSpacingAuto()
    {
        SetSpacingAuto("w:afterAutospacing");
    }
    void Paragraph::SetSpacingAuto(std::string const& attrNameAuto)
    {
        if (!impl) return;
        auto spacing = impl_()->w_pPr_.child("w:spacing");
        if (!spacing) {
            spacing = impl_()->w_pPr_.append_child("w:spacing");
        }
        auto spacingAuto = spacing.attribute(attrNameAuto.c_str());
        if (!spacingAuto) {
            spacingAuto = spacing.append_attribute(attrNameAuto.c_str());
        }
        // Any value for before or beforeLines is ignored.
        spacingAuto.set_value("true");
    }
    void Paragraph::SetBeforeSpacingLines(const double beforeSpacing)
    {
        // To specify units in hundreths of a line, 
        // use attributes 'afterLines'/'beforeLines'.
        SetSpacing(beforeSpacing * 100, "w:beforeAutospacing", "w:beforeLines");
    }
    void Paragraph::SetAfterSpacingLines(const double afterSpacing)
    {
        SetSpacing(afterSpacing * 100, "w:afterAutospacing", "w:afterLines");
    }
    void Paragraph::SetBeforeSpacing(const int beforeSpacing)
    {
        SetSpacing(beforeSpacing, "w:beforeAutospacing", "w:before");
    }
    void Paragraph::SetAfterSpacing(const int afterSpacing)
    {
        SetSpacing(afterSpacing, "w:afterAutospacing", "w:after");
    }
    void Paragraph::SetSpacing(const int twip, std::string const& attrNameAuto, std::string const& attrName)
    {
        if (!impl) return;
        auto elemSpacing = impl_()->w_pPr_.child("w:spacing");
        if (!elemSpacing) {
            elemSpacing = impl_()->w_pPr_.append_child("w:spacing");
        }

        auto attrSpacingAuto = elemSpacing.attribute(attrNameAuto.c_str());
        if (attrSpacingAuto) {
            elemSpacing.remove_attribute(attrSpacingAuto);
        }

        auto attrSpacing = elemSpacing.attribute(attrName.c_str());
        if (!attrSpacing) {
            attrSpacing = elemSpacing.append_attribute(attrName.c_str());
        }
        attrSpacing.set_value(twip);
    }
    void Paragraph::SetLeftIndentChars(const double leftIndent)
    {
        // To specify units in hundreths of a character, 
        // use attributes leftChars/endChars, rightChars/endChars, etc. 
        SetIndent(leftIndent * 100, "w:leftChars");
    }
    void Paragraph::SetRightIndentChars(const double rightIndent)
    {
        SetIndent(rightIndent * 100, "w:rightChars");
    }
    void Paragraph::SetLeftIndent(const int leftIndent)
    {
        SetIndent(leftIndent, "w:left");
    }
    void Paragraph::SetRightIndent(const int rightIndent)
    {
        SetIndent(rightIndent, "w:right");
    }
    void Paragraph::SetFirstLineChars(const double indent)
    {
        SetIndent(indent * 100, "w:firstLineChars");
    }
    void Paragraph::SetHangingChars(const double indent)
    {
        SetIndent(indent * 100, "w:hangingChars");
    }
    void Paragraph::SetFirstLine(const int indent)
    {
        SetIndent(indent, "w:firstLine");
    }
    void Paragraph::SetHanging(const int indent)
    {
        SetIndent(indent, "w:hanging");
        SetLeftIndent(indent);
    }
    void Paragraph::SetIndent(const int indent, std::string const& attrName) {
        if (!impl) return;
        auto elemIndent = impl_()->w_pPr_.child("w:ind");
        if (!elemIndent) {
            elemIndent = impl_()->w_pPr_.append_child("w:ind");
        }

        auto attrIndent = elemIndent.attribute(attrName.c_str());
        if (!attrIndent) {
            attrIndent = elemIndent.append_attribute(attrName.c_str());
        }
        attrIndent.set_value(indent);
    }
    void Paragraph::SetTopBorder(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:top", style, width, color);
    }
    void Paragraph::SetBottomBorder(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:bottom", style, width, color);
    }
    void Paragraph::SetLeftBorder(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:left", style, width, color);
    }
    void Paragraph::SetRightBorder(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:right", style, width, color);
    }
    void Paragraph::SetBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetTopBorder(style, width, color);
        SetBottomBorder(style, width, color);
        SetLeftBorder(style, width, color);
        SetRightBorder(style, width, color);
    }
    void Paragraph::SetBorders_(std::string const& elemName, const BorderStyle style, const double width, std::string const& color)
    {
        if (!impl) return;
        auto w_pBdr = impl_()->w_pPr_.child("w:pBdr");
        if (!w_pBdr) {
            w_pBdr = impl_()->w_pPr_.append_child("w:pBdr");
        }
        docx::SetBorders(w_pBdr, elemName.c_str(), style, width, color.c_str());
    }
    void Paragraph::SetFontSize(const double fontSize)
    {
        if (impl) impl_()->format.fontSize = fontSize;
        for (auto r = FirstRun(); r; r = r.Next()) {
            r.SetFontSize(fontSize);
        }
    }
    void Paragraph::SetFont(const ::std::string& fontAscii, const ::std::string& fontEastAsia)
    {
        if (impl) impl_()->format.fontFamily = fontAscii;
        for (auto r = FirstRun(); r; r = r.Next()) {
            r.SetFont(fontAscii, fontEastAsia);
        }
    }
    void Paragraph::SetFontStyle(const FontStyle fontStyle)
    {
        if (impl) impl_()->format.fontStyle = fontStyle;
        for (auto r = FirstRun(); r; r = r.Next()) {
            r.SetFontStyle(fontStyle);
        }
    }
    void Paragraph::SetCharacterSpacing(const int characterSpacing)
    {
        if (impl) impl_()->format.characterSpacing = characterSpacing;
        for (auto r = FirstRun(); r; r = r.Next()) {
            r.SetCharacterSpacing(characterSpacing);
        }
    }
    TextFormat* Paragraph::Format() {
        if (!impl) return nullptr;
        return &impl_()->format;
    };
    ::std::string Paragraph::GetText()
    {
        ::std::string text;
        for (auto r = FirstRun(); r; r = r.Next()) {
            text += r.GetText();
        }
        return text;
    }
    Paragraph Paragraph::Next()
    {
        if (!impl) return Paragraph();
        auto w_p = impl_()->w_p_.next_sibling("w:p");
        if (!w_p) return Paragraph();

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_p.child("w:pPr");
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = impl_()->Document;
        return out;
    }
    Paragraph Paragraph::Prev()
    {
        if (!impl) return Paragraph();
        auto w_p = impl_()->w_p_.previous_sibling("w:p");
        if (!w_p) return Paragraph();

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_p.child("w:pPr");
        auto out{ Paragraph(impl) };
        out.impl_()->format = impl_()->format;
        out.impl_()->Document = impl_()->Document;
        return out;
    }
    Paragraph::operator bool() const
    {
        return impl != nullptr && impl_()->w_p_;
    }
    bool Paragraph::operator==(const Paragraph& p)
    {
        if (!impl && !p.impl) return true;
        if (impl && p.impl) return impl_()->w_p_ == p.impl_()->w_p_;
        return false;
    }
    void Paragraph::operator=(const Paragraph& right) { impl = right.impl; };
    Section Paragraph::GetSection()
    {
        if (!impl) return Section();
        Section::Impl* impl = new Section::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = impl_()->w_p_;
        impl->w_pPr_ = impl_()->w_pPr_;
        Section section(impl);
        section.FindSectionProperties();
        section.impl_()->Document = this->impl_()->Document;
        return section;
    }
    Section Paragraph::InsertSectionBreak()
    {
        Section section = GetSection();
        // this paragraph will be the last paragraph of the new section
        if (section) section.Split();
        return section;
    }
    Section Paragraph::RemoveSectionBreak()
    {
        Section section = GetSection();
        if (section && section.IsSplit()) section.Merge();
        return section;
    }
    bool Paragraph::HasSectionBreak()
    {
        return GetSection().IsSplit();
    }

    // Section
    Section::Section() : impl(nullptr) {};
    Section::Section(Impl* implP) : impl(cweeSharedPtr<void>(cweeSharedPtr<Impl>(implP, [](void* p) { return p; }))) {};
    Section::Section(const Section& s) :impl(s.impl) {};
    Section::~Section() {};
    void Section::FindSectionProperties()
    {
        if (!impl) return;
        pugi::xml_node w_p_next = impl_()->w_p_, w_p, w_pPr, w_sectPr;
        do {
            w_p = w_p_next;
            w_pPr = w_p.child("w:pPr");
            w_sectPr = w_pPr.child("w:sectPr");

            w_p_next = w_p.next_sibling("w:p");
        } while (w_sectPr.empty() && !w_p_next.empty());

        impl_()->w_p_last_ = w_p;
        impl_()->w_pPr_last_ = w_pPr;
        impl_()->w_sectPr_ = w_sectPr;

        if (impl_()->w_sectPr_.empty()) {
            impl_()->w_sectPr_ = impl_()->w_body_.child("w:sectPr");
        }
    }
    void Section::Split()
    {
        if (!impl) return;
        if (IsSplit()) return;
        impl_()->w_p_last_ = impl_()->w_p_;
        impl_()->w_pPr_last_ = impl_()->w_pPr_;
        impl_()->w_sectPr_ = impl_()->w_pPr_.append_copy(impl_()->w_sectPr_);
    }
    bool Section::IsSplit()
    {
        if (!impl) return false;
        return impl_()->w_pPr_.child("w:sectPr");
    }
    void Section::Merge()
    {
        if (!impl) return;
        if (impl_()->w_pPr_.child("w:sectPr").empty()) return;
        impl_()->w_pPr_last_.remove_child(impl_()->w_sectPr_);
        FindSectionProperties();
    }
    void Section::SetPageSize(const int w, const int h)
    {
        if (!impl) return;
        auto pgSz = impl_()->w_sectPr_.child("w:pgSz");
        if (!pgSz) {
            pgSz = impl_()->w_sectPr_.append_child("w:pgSz");
        }
        auto pgSzW = pgSz.attribute("w:w");
        if (!pgSzW) {
            pgSzW = pgSz.append_attribute("w:w");
        }
        auto pgSzH = pgSz.attribute("w:h");
        if (!pgSzH) {
            pgSzH = pgSz.append_attribute("w:h");
        }
        pgSzW.set_value(w);
        pgSzH.set_value(h);
    }
    void Section::GetPageSize(int& w, int& h)
    {
        if (!impl) return;
        w = h = 0;
        auto pgSz = impl_()->w_sectPr_.child("w:pgSz");
        if (!pgSz) return;
        auto pgSzW = pgSz.attribute("w:w");
        if (!pgSzW) return;
        auto pgSzH = pgSz.attribute("w:h");
        if (!pgSzH) return;
        w = pgSzW.as_int();
        h = pgSzH.as_int();
    }
    void Section::SetPageOrient(const Orientation orient)
    {
        if (!impl) return;
        auto pgSz = impl_()->w_sectPr_.child("w:pgSz");
        if (!pgSz) {
            pgSz = impl_()->w_sectPr_.append_child("w:pgSz");
        }
        auto pgSzH = pgSz.attribute("w:h");
        if (!pgSzH) {
            pgSzH = pgSz.append_attribute("w:h");
        }
        auto pgSzW = pgSz.attribute("w:w");
        if (!pgSzW) {
            pgSzW = pgSz.append_attribute("w:w");
        }
        auto pgSzOrient = pgSz.attribute("w:orient");
        if (!pgSzOrient) {
            pgSzOrient = pgSz.append_attribute("w:orient");
        }
        int hVal = pgSzH.as_int();
        int wVal = pgSzW.as_int();
        switch (orient) {
        case Orientation::Landscape:
            if (hVal < wVal) return;
            pgSzOrient.set_value("landscape");
            pgSzH.set_value(wVal);
            pgSzW.set_value(hVal);
            break;
        case Orientation::Portrait:
            if (hVal > wVal) return;
            pgSzOrient.set_value("portrait");
            pgSzH.set_value(wVal);
            pgSzW.set_value(hVal);
            break;
        }
    }
    Section::Orientation Section::GetPageOrient()
    {
        if (!impl) return Orientation::Unknown;
        Orientation orient = Orientation::Portrait;
        auto pgSz = impl_()->w_sectPr_.child("w:pgSz");
        if (!pgSz) return orient;
        auto pgSzOrient = pgSz.attribute("w:orient");
        if (!pgSzOrient) return orient;
        if (::std::string(pgSzOrient.value()).compare("landscape") == 0) {
            orient = Orientation::Landscape;
        }
        return orient;
    }
    void Section::SetPageMargin(const int top, const int bottom, const int left, const int right)
    {
        if (!impl) return;
        auto pgMar = impl_()->w_sectPr_.child("w:pgMar");
        if (!pgMar) {
            pgMar = impl_()->w_sectPr_.append_child("w:pgMar");
        }
        auto pgMarTop = pgMar.attribute("w:top");
        if (!pgMarTop) {
            pgMarTop = pgMar.append_attribute("w:top");
        }
        auto pgMarBottom = pgMar.attribute("w:bottom");
        if (!pgMarBottom) {
            pgMarBottom = pgMar.append_attribute("w:bottom");
        }
        auto pgMarLeft = pgMar.attribute("w:left");
        if (!pgMarLeft) {
            pgMarLeft = pgMar.append_attribute("w:left");
        }
        auto pgMarRight = pgMar.attribute("w:right");
        if (!pgMarRight) {
            pgMarRight = pgMar.append_attribute("w:right");
        }
        pgMarTop.set_value(top);
        pgMarBottom.set_value(bottom);
        pgMarLeft.set_value(left);
        pgMarRight.set_value(right);
    }
    void Section::GetPageMargin(int& top, int& bottom, int& left, int& right)
    {
        if (!impl) return;
        top = bottom = left = right = 0;
        auto pgMar = impl_()->w_sectPr_.child("w:pgMar");
        if (!pgMar) return;
        auto pgMarTop = pgMar.attribute("w:top");
        if (!pgMarTop) return;
        auto pgMarBottom = pgMar.attribute("w:bottom");
        if (!pgMarBottom) return;
        auto pgMarLeft = pgMar.attribute("w:left");
        if (!pgMarLeft) return;
        auto pgMarRight = pgMar.attribute("w:right");
        if (!pgMarRight) return;
        top = pgMarTop.as_int();
        bottom = pgMarBottom.as_int();
        left = pgMarLeft.as_int();
        right = pgMarRight.as_int();
    }
    void Section::SetPageMargin(const int header, const int footer)
    {
        if (!impl) return;
        auto pgMar = impl_()->w_sectPr_.child("w:pgMar");
        if (!pgMar) {
            pgMar = impl_()->w_sectPr_.append_child("w:pgMar");
        }
        auto pgMarHeader = pgMar.attribute("w:header");
        if (!pgMarHeader) {
            pgMarHeader = pgMar.append_attribute("w:header");
        }
        auto pgMarFooter = pgMar.attribute("w:footer");
        if (!pgMarFooter) {
            pgMarFooter = pgMar.append_attribute("w:footer");
        }
        pgMarHeader.set_value(header);
        pgMarFooter.set_value(footer);
    }
    void Section::GetPageMargin(int& header, int& footer)
    {
        if (!impl) return;
        header = footer = 0;
        auto pgMar = impl_()->w_sectPr_.child("w:pgMar");
        if (!pgMar) return;
        auto pgMarHeader = pgMar.attribute("w:header");
        if (!pgMarHeader) return;
        auto pgMarFooter = pgMar.attribute("w:footer");
        if (!pgMarFooter) return;
        header = pgMarHeader.as_int();
        footer = pgMarFooter.as_int();
    }
    void Section::SetPageNumber(const PageNumberFormat fmt, const unsigned int start)
    {
        if (!impl) return;

        auto footerReference = impl_()->w_sectPr_.child("w:footerReference");
        if (!footerReference) {
            footerReference = impl_()->w_sectPr_.append_child("w:footerReference");
        }

        auto footerReferenceId = footerReference.attribute("r:id");
        if (!footerReferenceId) {
            footerReferenceId = footerReference.append_attribute("r:id");
        }
        footerReferenceId.set_value("rId1");

        auto footerReferenceType = footerReference.attribute("w:type");
        if (!footerReferenceType) {
            footerReferenceType = footerReference.append_attribute("w:type");
        }
        footerReferenceType.set_value("default");

        auto pgNumType = impl_()->w_sectPr_.child("w:pgNumType");
        if (!pgNumType) {
            pgNumType = impl_()->w_sectPr_.append_child("w:pgNumType");
        }

        auto pgNumTypeFmt = pgNumType.attribute("w:fmt");
        if (!pgNumTypeFmt) {
            pgNumTypeFmt = pgNumType.append_attribute("w:fmt");
        }
        const char* fmtVal = "";
        switch (fmt) {
        case PageNumberFormat::Decimal:
            fmtVal = "decimal";
            break;
        case PageNumberFormat::NumberInDash:
            fmtVal = "numberInDash";
            break;
        case PageNumberFormat::CardinalText:
            fmtVal = "cardinalText";
            break;
        case PageNumberFormat::OrdinalText:
            fmtVal = "ordinalText";
            break;
        case PageNumberFormat::LowerLetter:
            fmtVal = "lowerLetter";
            break;
        case PageNumberFormat::UpperLetter:
            fmtVal = "upperLetter";
            break;
        case PageNumberFormat::LowerRoman:
            fmtVal = "lowerRoman";
            break;
        case PageNumberFormat::UpperRoman:
            fmtVal = "upperRoman";
            break;
        }
        pgNumTypeFmt.set_value(fmtVal);

        auto pgNumTypeStart = pgNumType.attribute("w:start");
        if (start > 0) {
            if (!pgNumTypeStart) {
                pgNumTypeStart = pgNumType.append_attribute("w:start");
                pgNumTypeStart.set_value(start);
            }
        }
        else {
            if (pgNumTypeStart) {
                pgNumType.remove_attribute(pgNumTypeStart);
            }
        }
    }
    void Section::RemovePageNumber()
    {
        if (!impl) return;

        auto footerReference = impl_()->w_sectPr_.child("w:footerReference");
        if (footerReference) {
            impl_()->w_sectPr_.remove_child(footerReference);
        }

        auto pgNumType = impl_()->w_sectPr_.child("w:pgNumType");
        if (pgNumType) {
            impl_()->w_sectPr_.remove_child(pgNumType);
        }
    }
    Paragraph Section::FirstParagraph()
    {
        return Prev().LastParagraph().Next();
    }
    Paragraph Section::LastParagraph()
    {
        if (!impl) return Paragraph();
        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = impl_()->w_p_last_;
        impl->w_pPr_ = impl_()->w_pPr_last_;
        impl->Document = impl_()->Document;
        if (impl_()->Document) impl->format = impl_()->Document->format;
        return Paragraph(impl);
    }
    Section Section::Next()
    {
        if (!impl) return Section();
        auto w_p = impl_()->w_p_last_.next_sibling();
        if (w_p.empty()) return Section();

        Section::Impl* impl = new Section::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_p.child("w:pPr");
        Section s(impl);
        s.FindSectionProperties();
        s.impl_()->Document = this->impl_()->Document;
        return s;
    }
    Section Section::Prev()
    {
        if (!impl) return Section();

        pugi::xml_node w_p_prev, w_p, w_pPr, w_sectPr;

        w_p_prev = impl_()->w_p_.previous_sibling();
        if (w_p_prev.empty()) return Section();

        do {
            w_p = w_p_prev;
            w_pPr = w_p.child("w:pPr");
            w_sectPr = w_pPr.child("w:sectPr");
            w_p_prev = w_p.previous_sibling();
        } while (w_sectPr.empty() && !w_p_prev.empty());

        Section::Impl* impl = new Section::Impl;
        impl->w_body_ = impl_()->w_body_;
        impl->w_p_ = impl->w_p_last_ = w_p;
        impl->w_pPr_ = impl->w_pPr_last_ = w_pPr;
        impl->w_sectPr_ = w_sectPr;
        impl->Document = this->impl_()->Document;
        return Section(impl);
    }
    Section::operator bool() const {
        return impl != nullptr && impl_()->w_sectPr_;
    };
    bool Section::operator==(const Section& s) {
        if (!impl && !s.impl) return true;
        if (impl && s.impl) return impl_()->w_sectPr_ == s.impl_()->w_sectPr_;
        return false;
    };
    void Section::operator=(const Section& right) { impl = right.impl; };

    // Run
    Run::Run() : impl(nullptr) {};
    Run::Run(Impl* implP) : impl(cweeSharedPtr<void>(cweeSharedPtr<Impl>(implP, [](void* p) { return p; }))) {};
    Run::Run(const Run& r) : impl(r.impl) {};
    Run::~Run() {};
    void Run::AppendText(const ::std::string& text) {
        if (!impl) return;
        auto t = impl_()->w_r_.append_child("w:t");
        if (::std::isspace(static_cast<unsigned char>(text.front()))) {
            t.append_attribute("xml:space") = "preserve";
        }
        else if (::std::isspace(static_cast<unsigned char>(text.back()))) {
            t.append_attribute("xml:space") = "preserve";
        }
        t.text().set(text.c_str());
    };
    ::std::string Run::GetText()
    {
        if (!impl) return "";
        ::std::string text;
        for (auto t = impl_()->w_r_.child("w:t"); t; t = t.next_sibling("w:t")) {
            text += t.text().get();
        }
        return text;
    }
    void Run::ClearText()
    {
        if (!impl) return;
        impl_()->w_r_.remove_children();
    }
    void Run::SetText(const ::std::string& text)
    {
        ClearText();
        AppendText(text);
    }
    void Run::AppendLineBreak()
    {
        if (!impl) return;
        impl_()->w_r_.append_child("w:br");
    }
    void Run::SetFontSize(const double fontSize)
    {
        if (!impl) return;
        auto sz = impl_()->w_rPr_.child("w:sz");
        if (!sz) {
            sz = impl_()->w_rPr_.append_child("w:sz");
        }
        auto szVal = sz.attribute("w:val");
        if (!szVal) {
            szVal = sz.append_attribute("w:val");
        }
        // font size in half-points (1/144 of an inch)
        szVal.set_value(fontSize * 2);

        impl_()->format.fontSize = fontSize;
    }
    double Run::GetFontSize()
    {
        if (!impl) return -1;
        auto sz = impl_()->w_rPr_.child("w:sz");
        if (!sz) return 0;
        auto szVal = sz.attribute("w:val");
        if (!szVal) return 0;
        return szVal.as_int() / 2.0;
    }
    void Run::SetFont(const ::std::string& fontAscii, const ::std::string& fontEastAsia)
    {
        if (!impl) return;
        auto rFonts = impl_()->w_rPr_.child("w:rFonts");
        if (!rFonts) {
            rFonts = impl_()->w_rPr_.append_child("w:rFonts");
        }
        auto rFontsAscii = rFonts.attribute("w:ascii");
        if (!rFontsAscii) {
            rFontsAscii = rFonts.append_attribute("w:ascii");
        }
        auto rFontsEastAsia = rFonts.attribute("w:eastAsia");
        if (!rFontsEastAsia) {
            rFontsEastAsia = rFonts.append_attribute("w:eastAsia");
        }
        rFontsAscii.set_value(fontAscii.c_str());
        rFontsEastAsia.set_value(fontEastAsia.empty()
            ? fontAscii.c_str()
            : fontEastAsia.c_str());

        impl_()->format.fontFamily = fontAscii;
    }
    void Run::GetFont(::std::string& fontAscii, ::std::string& fontEastAsia)
    {
        if (!impl) return;
        auto rFonts = impl_()->w_rPr_.child("w:rFonts");
        if (!rFonts) return;

        auto rFontsAscii = rFonts.attribute("w:ascii");
        if (rFontsAscii) fontAscii = rFontsAscii.value();

        auto rFontsEastAsia = rFonts.attribute("w:eastAsia");
        if (rFontsEastAsia) fontEastAsia = rFontsEastAsia.value();
    }
    void Run::SetFontStyle(const FontStyle f)
    {
        if (!impl) return;
        auto b = impl_()->w_rPr_.child("w:b");
        if (f & Bold) {
            if (b.empty()) impl_()->w_rPr_.append_child("w:b");
        }
        else {
            impl_()->w_rPr_.remove_child(b);
        }

        auto i = impl_()->w_rPr_.child("w:i");
        if (f & Italic) {
            if (i.empty()) impl_()->w_rPr_.append_child("w:i");
        }
        else {
            impl_()->w_rPr_.remove_child(i);
        }

        auto u = impl_()->w_rPr_.child("w:u");
        if (f & Underline) {
            if (u.empty())
                impl_()->w_rPr_.append_child("w:u").append_attribute("w:val") = "single";
        }
        else {
            impl_()->w_rPr_.remove_child(u);
        }

        auto strike = impl_()->w_rPr_.child("w:strike");
        if (f & Strikethrough) {
            if (strike.empty())
                impl_()->w_rPr_.append_child("w:strike").append_attribute("w:val") = "true";
        }
        else {
            impl_()->w_rPr_.remove_child(strike);
        }

        impl_()->format.fontStyle = f;
    }
    FontStyle Run::GetFontStyle()
    {
        FontStyle fontStyle = 0;
        if (!impl) return fontStyle;
        if (impl_()->w_rPr_.child("w:b")) fontStyle |= Bold;
        if (impl_()->w_rPr_.child("w:i")) fontStyle |= Italic;
        if (impl_()->w_rPr_.child("w:u")) fontStyle |= Underline;
        if (impl_()->w_rPr_.child("w:strike")) fontStyle |= Strikethrough;
        return fontStyle;
    }
    void Run::SetCharacterSpacing(const int characterSpacing)
    {
        if (!impl) return;
        auto spacing = impl_()->w_rPr_.child("w:spacing");
        if (!spacing) {
            spacing = impl_()->w_rPr_.append_child("w:spacing");
        }
        auto spacingVal = spacing.attribute("w:val");
        if (!spacingVal) {
            spacingVal = spacing.append_attribute("w:val");
        }
        spacingVal.set_value(characterSpacing);

        impl_()->format.characterSpacing = characterSpacing;
    }
    int Run::GetCharacterSpacing()
    {
        if (!impl) return -1;
        return impl_()->w_rPr_.child("w:spacing").attribute("w:val").as_int();
    }
    bool Run::IsPageBreak()
    {
        if (!impl) return false;
        return impl_()->w_r_.find_child_by_attribute("w:br", "w:type", "page");
    }
    void Run::Remove()
    {
        if (!impl) return;
        impl_()->w_p_.remove_child(impl_()->w_r_);
    }
    Run Run::Next()
    {
        if (!impl) return Run();
        auto w_r = impl_()->w_r_.next_sibling("w:r");
        if (!w_r) return Run();

        Run::Impl* impl = new Run::Impl;
        impl->w_p_ = impl_()->w_p_;
        impl->w_r_ = w_r;
        impl->w_rPr_ = w_r.child("w:rPr");
        impl->format = impl_()->format;
        auto out{ Run(impl) };

        out.SetCharacterSpacing(impl_()->format.characterSpacing);
        out.SetFont(impl_()->format.fontFamily, "");
        out.SetFontSize(impl_()->format.fontSize);
        out.SetFontStyle(impl_()->format.fontStyle);

        return out;
    }
    Run::operator bool() const
    {
        return impl != nullptr && impl_()->w_r_;
    }
    void Run::operator=(const Run& right) { impl = right.impl; };

    // class Table
    Table::Table() : impl(nullptr) {}; 
    Table::Table(Impl* implP) : impl(cweeSharedPtr<void>(cweeSharedPtr<Impl>(implP, [](void* p) { return p; }))) {};    
    Table::Table(const Table& t) : impl(t.impl) {};
    Table::~Table(){}
    void Table::operator=(const Table& right) { impl = right.impl; };
    void Table::Create_(const int rows, const int cols)
    {
        if (!impl) return;
        impl_()->rows_ = rows;
        impl_()->cols_ = cols;

        // init grid
        for (int i = 0; i < rows; i++) {
            Row row;
            for (int j = 0; j < cols; j++) {
                Cell cell = { i, j, 1, 1 };
                row.push_back(cell);
            }
            impl_()->grid_.push_back(row);
        }

        // init table
        for (int i = 0; i < rows; i++) {
            auto w_gridCol = impl_()->w_tblGrid_.append_child("w:gridCol");
            auto w_tr = impl_()->w_tbl_.append_child("w:tr");

            for (int j = 0; j < cols; j++) {
                auto w_tc = w_tr.append_child("w:tc");
                auto w_tcPr = w_tc.append_child("w:tcPr");

                TableCell::Impl* impl = new TableCell::Impl;
                impl->c_ = &impl_()->grid_[i][j];
                impl->w_tr_ = w_tr;
                impl->w_tc_ = w_tc;
                impl->w_tcPr_ = w_tcPr;
                impl->Document = impl_()->Document;
                TableCell tc(impl);
                // A table cell must contain at least one block-level element, 
                // even if it is an empty <p/>.
                tc.AppendParagraph();
            }
        }
    }
    TableCell Table::GetCell(const int row, const int col)
    {
        if (!impl) return TableCell();
        if (row < 0 || row >= impl_()->rows_ || col < 0 || col >= impl_()->cols_) {
            return TableCell();
        }

        Cell* c = &impl_()->grid_[row][col];
        return GetCell_(c->row, c->col);
    }
    TableCell Table::GetCell_(const int row, const int col)
    {
        if (!impl) return TableCell();
        int i = 0;
        auto w_tr = impl_()->w_tbl_.child("w:tr");
        while (i < row && !w_tr.empty()) {
            i++;
            w_tr = w_tr.next_sibling("w:tr");
        }
        if (w_tr.empty()) {
            return TableCell();
        }

        int j = 0;
        auto w_tc = w_tr.child("w:tc");
        while (j < col && !w_tc.empty()) {
            j += impl_()->grid_[row][j].cols;
            w_tc = w_tc.next_sibling("w:tc");
        }
        if (w_tc.empty()) {
            return TableCell();
        }

        TableCell::Impl* impl = new TableCell::Impl;
        impl->c_ = &impl_()->grid_[row][col];
        impl->w_tr_ = w_tr;
        impl->w_tc_ = w_tc;
        impl->w_tcPr_ = w_tc.child("w:tcPr");
        impl->Document = impl_()->Document;
        return TableCell(impl);
    }    
    bool Table::MergeCells(TableCell tc1, TableCell tc2)
    {
        if (tc1.empty() || tc2.empty()) {
            return false;
        }

        Cell* c1 = tc1.impl_()->c_;
        Cell* c2 = tc2.impl_()->c_;

        if (c1->row == c2->row && c1->col != c2->col && c1->rows == c2->rows) {
            Cell* left_cell, * right_cell;
            TableCell* left_tc, * right_tc;
            if (c1->col < c2->col) {
                left_cell = c1;
                left_tc = &tc1;
                right_cell = c2;
                right_tc = &tc2;
            }
            else {
                left_cell = c2;
                left_tc = &tc2;
                right_cell = c1;
                right_tc = &tc1;
            }

            const int col = left_cell->col;
            if ((right_cell->col - col) == left_cell->cols) {
                const int cols = left_cell->cols + right_cell->cols;

                // update right grid
                const int right_col = right_cell->col;
                const int right_cols = right_cell->cols;
                for (int i = 0; i < right_cell->rows; i++) {
                    const int y = right_cell->row + i;
                    for (int j = 0; j < right_cols; j++) {
                        Cell& c = impl_()->grid_[y][right_col + j];
                        c.col = col;
                        c.cols = cols;
                    }
                }

                // update cells
                for (int i = 0; i < right_cell->rows; i++) {
                    RemoveCell_(GetCell_(right_cell->row + i, right_cell->col));
                }
                for (int i = 0; i < left_cell->rows; i++) {
                    GetCell_(left_cell->row + i, left_cell->col).SetCellSpanning_(cols);
                }

                // update left grid
                const int left_cols = left_cell->cols;
                for (int i = 0; i < left_cell->rows; i++) {
                    const int y = left_cell->row + i;
                    for (int j = 0; j < left_cols; j++) {
                        Cell& c = impl_()->grid_[y][left_cell->col + j];
                        c.cols = cols;
                    }
                }

                right_tc->impl_()->c_ = left_cell;
                right_tc->impl_()->w_tc_ = left_tc->impl_()->w_tc_;
                right_tc->impl_()->w_tcPr_ = left_tc->impl_()->w_tcPr_;
                return true;
            }
        }
        else if (c1->col == c2->col && c1->row != c2->row && c1->cols == c2->cols) {
            Cell* top_cell, * bottom_cell;
            TableCell* top_tc, * bottom_tc;
            if (c1->row < c2->row) {
                top_cell = c1;
                top_tc = &tc1;
                bottom_cell = c2;
                bottom_tc = &tc2;
            }
            else {
                top_cell = c2;
                top_tc = &tc2;
                bottom_cell = c1;
                bottom_tc = &tc1;
            }

            const int row = top_cell->row;
            if ((bottom_cell->row - top_cell->row) == top_cell->rows) {
                const int rows = top_cell->rows + bottom_cell->rows;

                // update cells
                if (top_cell->rows == 1) {
                    auto w_vMerge = top_tc->impl_()->w_tcPr_.append_child("w:vMerge");
                    auto w_val = w_vMerge.append_attribute("w:val");
                    w_val.set_value("restart");
                }
                if (bottom_cell->rows == 1) {
                    bottom_tc->impl_()->w_tcPr_.append_child("w:vMerge");
                }
                else {
                    bottom_tc->impl_()->w_tcPr_.remove_child("w:vMerge");
                    bottom_tc->impl_()->w_tcPr_.append_child("w:vMerge");
                }

                // update top grid
                const int top_rows = top_cell->rows;
                for (int i = 0; i < top_rows; i++) {
                    const int x = top_cell->row + i;
                    for (int j = 0; j < top_cell->cols; j++) {
                        Cell& c = impl_()->grid_[x][top_cell->col + j];
                        c.rows = rows;
                    }
                }

                // update bottom grid
                const int bottom_row = bottom_cell->row;
                const int bottom_rows = bottom_cell->rows;
                for (int i = 0; i < bottom_rows; i++) {
                    const int x = bottom_row + i;
                    for (int j = 0; j < bottom_cell->cols; j++) {
                        Cell& c = impl_()->grid_[x][bottom_cell->col + j];
                        c.row = row;
                        c.rows = rows;
                    }
                }

                bottom_tc->impl_()->c_ = top_cell;
                bottom_tc->impl_()->w_tc_ = top_tc->impl_()->w_tc_;
                bottom_tc->impl_()->w_tcPr_ = top_tc->impl_()->w_tcPr_;
                return true;
            }
        }

        return false;
    }
    bool Table::MergeRow(int row) {
        int numC = this->impl_()->cols_;
        int numR = this->impl_()->rows_;

        if (row >= 0 && row < numR) {
            // from right-to-left
            for (int c = numC - 1; c >= 1; c--) {
                MergeCells(GetCell(row, c - 1), GetCell(row, c));
            }
        }
        return true;
    };
    void Table::RemoveCell_(TableCell tc)
    {
        if (!impl) return;
        tc.impl_()->w_tr_.remove_child(tc.impl_()->w_tc_);
    }
    void Table::SetWidthAuto() {
        SetWidth(0, "auto");
    }
    void Table::SetWidthPercent(const double w)
    {
        SetWidth(w / 0.02, "pct");
    }
    void Table::SetWidth(const int w, std::string const& units)
    {
        if (!impl) return;
        auto w_tblW = impl_()->w_tblPr_.child("w:tblW");
        if (!w_tblW) {
            w_tblW = impl_()->w_tblPr_.append_child("w:tblW");
        }

        auto w_w = w_tblW.attribute("w:w");
        if (!w_w) {
            w_w = w_tblW.append_attribute("w:w");
        }

        auto w_type = w_tblW.attribute("w:type");
        if (!w_type) {
            w_type = w_tblW.append_attribute("w:type");
        }

        w_w.set_value(w);
        w_type.set_value(units.c_str());
    }
    void Table::SetCellMarginTop(const int w, std::string const& units)
    {
        SetCellMargin("w:top", w, units);
    }
    void Table::SetCellMarginBottom(const int w, std::string const& units)
    {
        SetCellMargin("w:bottom", w, units);
    }
    void Table::SetCellMarginLeft(const int w, std::string const& units)
    {
        SetCellMargin("w:start", w, units);
    }
    void Table::SetCellMarginRight(const int w, std::string const& units)
    {
        SetCellMargin("w:end", w, units);
    }
    void Table::SetCellMargin(std::string const& elemName, const int w, std::string const& units)
    {
        if (!impl) return;
        auto w_tblCellMar = impl_()->w_tblPr_.child("w:tblCellMar");
        if (!w_tblCellMar) {
            w_tblCellMar = impl_()->w_tblPr_.append_child("w:tblCellMar");
        }

        auto w_tblCellMarChild = w_tblCellMar.child(elemName.c_str());
        if (!w_tblCellMarChild) {
            w_tblCellMarChild = w_tblCellMar.append_child(elemName.c_str());
        }

        auto w_w = w_tblCellMarChild.attribute("w:w");
        if (!w_w) {
            w_w = w_tblCellMarChild.append_attribute("w:w");
        }

        auto w_type = w_tblCellMarChild.attribute("w:type");
        if (!w_type) {
            w_type = w_tblCellMarChild.append_attribute("w:type");
        }

        w_w.set_value(w);
        w_type.set_value(units.c_str());
    }
    void Table::SetAlignment(const Alignment alignment)
    {
        if (!impl) return;
        const char* val = nullptr;
        switch (alignment) {
        case Alignment::Left:
            val = "start";
            break;
        case Alignment::Right:
            val = "end";
            break;
        case Alignment::Centered:
            val = "center";
            break;
        }

        auto w_jc = impl_()->w_tblPr_.child("w:jc");
        if (!w_jc) {
            w_jc = impl_()->w_tblPr_.append_child("w:jc");
        }
        auto w_val = w_jc.attribute("w:val");
        if (!w_val) {
            w_val = w_jc.append_attribute("w:val");
        }
        w_val.set_value(val);
    }
    void Table::SetTopBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:top", style, width, color);
    }
    void Table::SetBottomBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:bottom", style, width, color);
    }
    void Table::SetLeftBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:start", style, width, color);
    }
    void Table::SetRightBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:end", style, width, color);
    }
    void Table::SetInsideHBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:insideH", style, width, color);
    }
    void Table::SetInsideVBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetBorders_("w:insideV", style, width, color);
    }
    void Table::SetLeftIndent(const int leftIndent) {
        if (!impl) return;
        auto w_tblInd = impl_()->w_tblPr_.child("w:tblInd");
        if (!w_tblInd) {
            w_tblInd = impl_()->w_tblPr_.append_child("w:tblInd");
        }

        auto w_w = w_tblInd.attribute("w:w");
        if (!w_w) {
            w_w = w_tblInd.append_attribute("w:w");
        }

        auto w_type = w_tblInd.attribute("w:type");
        if (!w_type) {
            w_type = w_tblInd.append_attribute("w:type");
        }
        
        w_w.set_value(leftIndent);
        w_type.set_value("dxa");
    };
    void Table::SetInsideBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetInsideHBorders(style, width, color);
        SetInsideVBorders(style, width, color);
    }
    void Table::SetOutsideBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetTopBorders(style, width, color);
        SetBottomBorders(style, width, color);
        SetLeftBorders(style, width, color);
        SetRightBorders(style, width, color);
    }
    void Table::SetAllBorders(const BorderStyle style, const double width, std::string const& color)
    {
        SetOutsideBorders(style, width, color);
        SetInsideBorders(style, width, color);
    }
    void Table::SetBorders_(std::string const& elemName, const BorderStyle style, const double width, std::string const& color)
    {
        if (!impl) return;
        auto w_tblBorders = impl_()->w_tblPr_.child("w:tblBorders");
        if (!w_tblBorders) {
            w_tblBorders = impl_()->w_tblPr_.append_child("w:tblBorders");
        }
        SetBorders(w_tblBorders, elemName.c_str(), style, width, color.c_str());
    }

    // class TableCell    
    TableCell::TableCell() : impl(nullptr) {};
    TableCell::TableCell(Impl* implP) : impl(cweeSharedPtr<void>(cweeSharedPtr<Impl>(implP, [](void* p) { return p; }))) {};
    TableCell::TableCell(const TableCell& tc) : impl(tc.impl) { }
    TableCell::~TableCell(){}
    void TableCell::operator=(const TableCell& right) {
        impl = right.impl;
    };
    TableCell::operator bool() const {
        return impl != nullptr && impl_()->w_tc_;
    };
    bool TableCell::empty() const {
        return impl == nullptr || !impl_()->w_tc_;
    };
    void TableCell::SetWidth(const int w, std::string const& units) {
        if (!impl) return;
        auto w_tcW = impl_()->w_tcPr_.child("w:tcW");
        if (!w_tcW) {
            w_tcW = impl_()->w_tcPr_.append_child("w:tcW");
        }

        auto w_w = w_tcW.attribute("w:w");
        if (!w_w) {
            w_w = w_tcW.append_attribute("w:w");
        }

        auto w_type = w_tcW.attribute("w:type");
        if (!w_type) {
            w_type = w_tcW.append_attribute("w:type");
        }

        w_w.set_value(w);
        w_type.set_value(units.c_str());
    }
    void TableCell::SetVerticalAlignment(const Alignment align)
    {
        if (!impl) return;
        auto w_vAlign = impl_()->w_tcPr_.child("w:vAlign");
        if (!w_vAlign) {
            w_vAlign = impl_()->w_tcPr_.append_child("w:vAlign");
        }

        auto w_val = w_vAlign.attribute("w:val");
        if (!w_val) {
            w_val = w_vAlign.append_attribute("w:val");
        }

        const char* val = nullptr;
        switch (align) {
        case Alignment::Top:
            val = "top";
            break;
        case Alignment::Center:
            val = "center";
            break;
        case Alignment::Bottom:
            val = "bottom";
            break;
        }
        w_val.set_value(val);
    }
    void TableCell::SetCellSpanning_(const int cols)
    {
        if (!impl) return;
        auto w_gridSpan = impl_()->w_tcPr_.child("w:gridSpan");
        if (cols == 1) {
            if (w_gridSpan) {
                impl_()->w_tcPr_.remove_child(w_gridSpan);
            }
            return;
        }
        if (!w_gridSpan) {
            w_gridSpan = impl_()->w_tcPr_.append_child("w:gridSpan");
        }

        auto w_val = w_gridSpan.attribute("w:val");
        if (!w_val) {
            w_val = w_gridSpan.append_attribute("w:val");
        }

        w_val.set_value(cols);
    }
    Paragraph TableCell::AppendParagraph()
    {
        if (!impl) return Paragraph();
        auto w_p = impl_()->w_tc_.append_child("w:p");
        auto w_pPr = w_p.append_child("w:pPr");

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_tc_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_pPr;
        impl->Document = impl_()->Document;
        if (impl_()->Document) impl->format = impl_()->Document->format;
        return Paragraph(impl);
    }
    Paragraph TableCell::FirstParagraph()
    {
        if (!impl) return Paragraph();
        auto w_p = impl_()->w_tc_.child("w:p");
        auto w_pPr = w_p.child("w:pPr");

        Paragraph::Impl* impl = new Paragraph::Impl;
        impl->w_body_ = impl_()->w_tc_;
        impl->w_p_ = w_p;
        impl->w_pPr_ = w_pPr;
        impl->Document = impl_()->Document;
        if (impl_()->Document) impl->format = impl_()->Document->format;
        return Paragraph(impl);
    }

    // class TextFrame
    TextFrame::TextFrame() : Paragraph(), impl(nullptr) {};
    TextFrame::TextFrame(Impl* implP, Paragraph::Impl* paragraph_impl) : Paragraph(paragraph_impl), impl(cweeSharedPtr<void>(cweeSharedPtr<Impl>(implP, [](void* p) { return p; }))) {};
    TextFrame::TextFrame(const TextFrame& tf) : Paragraph(tf), impl(tf.impl) {};
    TextFrame::~TextFrame() {};
    void TextFrame::SetSize(const int w, const int h)
    {
        if (!impl) return;
        auto w_w = impl_()->w_framePr_.attribute("w:w");
        if (!w_w) {
            w_w = impl_()->w_framePr_.append_attribute("w:w");
        }
        auto w_h = impl_()->w_framePr_.attribute("w:h");
        if (!w_h) {
            w_h = impl_()->w_framePr_.append_attribute("w:h");
        }

        w_w.set_value(w);
        w_h.set_value(h);
    }
    void TextFrame::SetPositionX(const Position align, const Anchor ralativeTo)
    {
        SetAnchor_("w:hAnchor", ralativeTo);
        SetPosition_("w:xAlign", align);
    }
    void TextFrame::SetPositionY(const Position align, const Anchor ralativeTo)
    {
        SetAnchor_("w:vAnchor", ralativeTo);
        SetPosition_("w:yAlign", align);
    }
    void TextFrame::SetPositionX(const int x, const Anchor ralativeTo)
    {
        SetAnchor_("w:hAnchor", ralativeTo);
        SetPosition_("w:x", x);
    }
    void TextFrame::SetPositionY(const int y, const Anchor ralativeTo)
    {
        SetAnchor_("w:vAnchor", ralativeTo);
        SetPosition_("w:y", y);
    }
    void TextFrame::SetAnchor_(std::string const& attrName, const Anchor anchor)
    {
        if (!impl) return;
        auto w_anchor = impl_()->w_framePr_.attribute(attrName.c_str());
        if (!w_anchor) {
            w_anchor = impl_()->w_framePr_.append_attribute(attrName.c_str());
        }

        const char* val;
        switch (anchor) {
        case Anchor::Page:
            val = "page";
            break;
        case Anchor::Margin:
            val = "margin";
            break;
        }
        w_anchor.set_value(val);
    }
    void TextFrame::SetPosition_(std::string const& attrName, const Position align)
    {
        if (!impl) return;
        auto w_align = impl_()->w_framePr_.attribute(attrName.c_str());
        if (!w_align) {
            w_align = impl_()->w_framePr_.append_attribute(attrName.c_str());
        }

        const char* val;
        switch (align) {
        case Position::Left:
            val = "left";
            break;
        case Position::Center:
            val = "center";
            break;
        case Position::Right:
            val = "right";
            break;
        case Position::Top:
            val = "top";
            break;
        case Position::Bottom:
            val = "bottom";
            break;
        }
        w_align.set_value(val);
    }
    void TextFrame::SetPosition_(std::string const& attrName, const int twip)
    {
        if (!impl) return;
        auto w_Align = impl_()->w_framePr_.attribute(attrName.c_str());
        if (!w_Align) {
            w_Align = impl_()->w_framePr_.append_attribute(attrName.c_str());
        }
        w_Align.set_value(twip);
    }
    void TextFrame::SetTextWrapping(const Wrapping wrapping)
    {
        if (!impl) return;
        auto w_wrap = impl_()->w_framePr_.attribute("w:wrap");
        if (!w_wrap) {
            w_wrap = impl_()->w_framePr_.append_attribute("w:wrap");
        }

        const char* val;
        switch (wrapping) {
        case Wrapping::Around:
            val = "around";
            break;
        case Wrapping::None:
            val = "none";
            break;
        }
        w_wrap.set_value(val);
    }

} // namespace docx

#undef _RELS
#undef DOCUMENT_XML
#undef CONTENT_TYPES_XML
#undef DOCUMENT_XML_RELS
#undef FOOTER1_XML