/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

 Project:      OWA EPANET
 Description:  API function declarations
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE

 History: RTG	/	2023		1. Modified original source code to use WaterWatch tools, and to support Zones and other analysis tasks.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "EPANET_POSTCOMPILED.h"

namespace epanet {
    namespace epanet_cpp {
        EN_Project EN_createproject();
        int EN_clearproject(EN_Project p);
        int EN_runproject(EN_Project p, const char* inpFile, const char* rptFile, const char* outFile, void (*pviewprog)(char*));
        int EN_init(EN_Project p, const char* rptFile, const char* outFile, int unitsType, int headLossType);
        int EN_open(EN_Project p, const char* inpFile, const char* rptFile = "", const char* outFile = "");
        int EN_gettitle(EN_Project p, char* line1, char* line2, char* line3);
        int EN_settitle(EN_Project p, char* line1, char* line2, char* line3);
        int EN_getcomment(EN_Project p, int object, int index, char* comment);
        int EN_setcomment(EN_Project p, int object, int index, char* comment);
        int EN_getcount(EN_Project p, int object, int* count);
        int EN_saveinpfile(EN_Project p, const char* filename);
        int EN_close(EN_Project p);

        int EN_solveH(EN_Project p);
        int EN_saveH(EN_Project p);
        int EN_openH(EN_Project p);
        int EN_initH(EN_Project p, int initFlag);
        int EN_runH(EN_Project p, units::time::second_t* currentTime, HydraulicSimulationQuality simQuality = HydraulicSimulationQuality::HIGHRES);
        int EN_nextH(EN_Project p, units::time::second_t* tStep);
        int EN_closeH(EN_Project p);
        int EN_savehydfile(EN_Project p, const char* filename);
        int EN_usehydfile(EN_Project p, const char* filename);

        int EN_solveQ(EN_Project p);
        int EN_openQ(EN_Project p);
        int EN_initQ(EN_Project p, int saveFlag);
        int EN_runQ(EN_Project p, units::time::second_t* currentTime);
        int EN_nextQ(EN_Project p, units::time::second_t* tStep);
        int EN_stepQ(EN_Project p, units::time::second_t* timeLeft);
        int EN_closeQ(EN_Project p);

        int EN_writeline(EN_Project p, char* line);
        int EN_report(EN_Project p);
        int EN_copyreport(EN_Project p, char* filename);
        int EN_clearreport(EN_Project p);
        int EN_resetreport(EN_Project p);
        int EN_setreport(EN_Project p, char* format);
        int EN_setstatusreport(EN_Project p, int level);
        int EN_getversion(int* version);
        int EN_geterror(int errcode, char* errmsg, int maxLen);
        int EN_getstatistic(EN_Project p, int type, SCALER* value);
        int EN_getresultindex(EN_Project p, int type, int index, int* value);

        int EN_getoption(EN_Project p, int option, SCALER* value);
        int EN_setoption(EN_Project p, int option, SCALER value);
        int EN_getflowunits(EN_Project p, int* units);
        int EN_setflowunits(EN_Project p, int units);
        int EN_gettimeparam(EN_Project p, int param, units::time::second_t* value);
        int EN_settimeparam(EN_Project p, int param, units::time::second_t value);
        int EN_getqualinfo(EN_Project p, int* qualType, char* chemName, char* chemUnits, int* traceNode);
        int EN_getqualtype(EN_Project p, int* qualType, int* traceNode);
        int EN_setqualtype(EN_Project p, int qualType, char* chemName, char* chemUnits, char* traceNode);

        int EN_addnode(EN_Project p, char* id, int nodeType, int* index);
        int EN_deletenode(EN_Project p, int index, int actionCode);
        int EN_getnodeindex(EN_Project p, char* id, int* index);
        int EN_getnodeid(EN_Project p, int index, char* id);
        int EN_setnodeid(EN_Project p, int index, char* newid);
        int EN_getnodetype(EN_Project p, int index, int* nodeType);
        int EN_getnodevalue(EN_Project p, int index, int property, SCALER* value);
        int EN_setnodevalue(EN_Project p, int index, int property, SCALER value);
        int EN_setjuncdata(EN_Project p, int index, SCALER elev, SCALER dmnd, char* dmndpat);

        int EN_settankdata(EN_Project p, int index, SCALER elev, SCALER initlvl, SCALER minlvl, SCALER maxlvl, SCALER diam, SCALER minvol, char* volcurve);
        int EN_getcoord(EN_Project p, int index, SCALER* x, SCALER* y);
        int EN_setcoord(EN_Project p, int index, SCALER x, SCALER y);

        int EN_getConnectedLinks(EN_Project p, int index, cweeList<::epanet::Plink>* out);

        int EN_getdemandmodel(EN_Project p, int* model, SCALER* pmin, SCALER* preq, SCALER* pexp);
        int EN_setdemandmodel(EN_Project p, int model, SCALER pmin, SCALER preq, SCALER pexp);
        int EN_adddemand(EN_Project p, int nodeIndex, SCALER baseDemand, char* demandPattern, char* demandName);
        int EN_deletedemand(EN_Project p, int nodeIndex, int demandIndex);
        int EN_getdemandindex(EN_Project p, int nodeIndex, char* demandName, int* demandIndex);
        int EN_getnumdemands(EN_Project p, int nodeIndex, int* numDemands);
        int EN_getbasedemand(EN_Project p, int nodeIndex, int demandIndex, SCALER* baseDemand);
        int EN_setbasedemand(EN_Project p, int nodeIndex, int demandIndex, SCALER baseDemand);
        int EN_getdemandname(EN_Project p, int nodeIndex, int demandIndex, char* demandName);
        int EN_setdemandname(EN_Project p, int nodeIndex, int demandIndex, char* demandName);
        int EN_getdemandpattern(EN_Project p, int nodeIndex, int demandIndex, int* patIndex);
        int EN_setdemandpattern(EN_Project p, int nodeIndex, int demandIndex, int patIndex);

        int EN_addlink(EN_Project p, char* id, int linkType, char* fromNode, char* toNode, int* index);
        int EN_deletelink(EN_Project p, int index, int actionCode);
        int EN_getlinkindex(EN_Project p, char* id, int* index);
        int EN_getlinkid(EN_Project p, int index, char* id);
        int EN_setlinkid(EN_Project p, int index, char* newid);
        int EN_getlinktype(EN_Project p, int index, int* linkType);
        int EN_setlinktype(EN_Project p, int* index, int linkType, int actionCode);
        int EN_getlinknodes(EN_Project p, int index, int* node1, int* node2);
        int EN_setlinknodes(EN_Project p, int index, int node1, int node2);
        int EN_getlinkvalue(EN_Project p, int index, int property, SCALER* value);
        int EN_setlinkvalue(EN_Project p, int index, int property, SCALER value);
        int EN_setpipedata(EN_Project p, int index, SCALER length, SCALER diam, SCALER rough, SCALER mloss);
        int EN_getvertexcount(EN_Project p, int index, int* count);
        int EN_getvertex(EN_Project p, int index, int vertex, SCALER* x, SCALER* y);
        int EN_setvertex(EN_Project p, int index, int vertex, SCALER x, SCALER y);
        int EN_setvertices(EN_Project p, int index, SCALER* x, SCALER* y, int count);

        int EN_getpumptype(EN_Project p, int linkIndex, int* pumpType);
        int EN_getheadcurveindex(EN_Project p, int linkIndex, int* curveIndex);
        int EN_setheadcurveindex(EN_Project p, int linkIndex, int curveIndex);

        int EN_addpattern(EN_Project p, char* id);
        int EN_deletepattern(EN_Project p, int index);
        int EN_getpatternindex(EN_Project p, char* id, int* index);
        int EN_getpatternid(EN_Project p, int index, char* id);
        int EN_setpatternid(EN_Project p, int index, char* id);
        int EN_getpatternlen(EN_Project p, int index, int* len);
        int EN_getpatternvalue(EN_Project p, int index, int period, SCALER* value);
        int EN_setpatternvalue(EN_Project p, int index, int period, SCALER value);
        int EN_getaveragepatternvalue(EN_Project p, int index, SCALER* value);
        int EN_setpattern(EN_Project p, int index, SCALER* values, int len);

        int EN_addcurve(EN_Project p, char* id);
        int EN_deletecurve(EN_Project p, int index);
        int EN_getcurveindex(EN_Project p, char* id, int* index);
        int EN_getcurveid(EN_Project p, int index, char* id);
        int EN_setcurveid(EN_Project p, int index, char* id);
        int EN_getcurvelen(EN_Project p, int index, int* len);
        int EN_getcurvetype(EN_Project p, int index, int* type);
        int EN_setcurvetype(EN_Project p, int index, int type);
        int EN_getcurvevalue(EN_Project p, int curveIndex, int pointIndex, SCALER* x, SCALER* y);
        int EN_setcurvevalue(EN_Project p, int curveIndex, int pointIndex, SCALER x, SCALER y);
        int EN_getcurve(EN_Project p, int index, char* id, int* nPoints, SCALER* xValues, SCALER* yValues);
        int EN_setcurve(EN_Project p, int index, SCALER* xValues, SCALER* yValues, int nPoints);

        int EN_addcontrol(EN_Project p, int type, int linkIndex, SCALER setting, int nodeIndex, SCALER level, int* index);
        int EN_deletecontrol(EN_Project p, int index);
        int EN_getcontrol(EN_Project p, int index, int* type, int* linkIndex, SCALER* setting, int* nodeIndex, SCALER* level);
        int EN_setcontrol(EN_Project p, int index, int type, int linkIndex, SCALER setting, int nodeIndex, SCALER level);

        int EN_addrule(EN_Project p, char* rule);
        int EN_deleterule(EN_Project p, int index);
        int EN_getrule(EN_Project p, int index, int* nPremises, int* nThenActions, int* nElseActions, SCALER* priority);
        int EN_getruleID(EN_Project p, int index, char* id);
        int EN_getpremise(EN_Project p, int ruleIndex, int premiseIndex, int* logop, int* object, int* objIndex, int* variable, int* relop, int* status, SCALER* value);
        int EN_setpremise(EN_Project p, int ruleIndex, int premiseIndex, int logop, int object, int objIndex, int variable, int relop, int status, SCALER value);
        int EN_setpremiseindex(EN_Project p, int ruleIndex, int premiseIndex, int objIndex);
        int EN_setpremisestatus(EN_Project p, int ruleIndex, int premiseIndex, int status);
        int EN_setpremisevalue(EN_Project p, int ruleIndex, int premiseIndex, SCALER value);
        int EN_getthenaction(EN_Project p, int ruleIndex, int actionIndex, int* linkIndex, int* status, SCALER* setting);
        int EN_setthenaction(EN_Project p, int ruleIndex, int actionIndex, int linkIndex, int status, SCALER setting);
        int EN_getelseaction(EN_Project p, int ruleIndex, int actionIndex, int* linkIndex, int* status, SCALER* setting);
        int EN_setelseaction(EN_Project p, int ruleIndex, int actionIndex, int linkIndex, int status, SCALER setting);
        int EN_setrulepriority(EN_Project p, int index, SCALER priority);
    };
};