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
#include "EPANET_COMPILED.h"

namespace epanet {
    namespace epanet_postcompiled {
        void		initrules(EN_Project pr);
        void		initpointers(EN_Project pr);
        int		allocrules(EN_Project pr);
        int		allocdata(EN_Project pr);
        void		freedemands(Pnode node);
        void		freelinkvertices(Plink link);
        void		freerules(EN_Project pr);
        void		freedata(EN_Project pr);
        void		writelogo(EN_Project pr);
        int		openfiles(EN_Project pr, const char* f1, const char* f2, const char* f3);
        int		openhydfile(EN_Project pr);
        void		closeoutfile(EN_Project pr);
        int		savenetdata(EN_Project pr);
        int		saveenergy(EN_Project pr);
        int		openoutfile(EN_Project pr);
        Sdemand* finddemand(Pnode n, int index);
        int		incontrols(EN_Project pr, int objType, int index);
        void		adjustpatterns(EN_Network network, int index);
        void		adjustcurves(EN_Network network, int index);
        int		getcomment(EN_Network network, int object, int index, char* comment);
        int		setcomment(EN_Network network, int object, int index, const char* newcomment);
        int		adjustpumpparams(EN_Project pr, int curveIndex);
        void     addrule(Parser* parser, char* tok);
        int      addpattern(EN_Network network, char* id);
        int      addcurve(EN_Network network, char* id);
        int      netsize(EN_Project pr);
        void inperrmsg(EN_Project pr, int err, int sect, char* line);

        bool   TryFindValidDiameter(EN_Project pr, Plink link, int nodeIndex, foot_t& diameter);
        bool     IsLinkControlled(EN_Project const& pr, int LinkN);
        bool     IsLinkControlled(EN_Project const& pr, Plink Link);

        /* Used to create the list of pressure zones. Note that this function must be as light-weight (in terms of memory overhead) as possible due to the recursive function used. */
        void     parseZone_Basic(EN_Project const& pr, Pzone const& zone, cweeList<Pnode>& nodes, int nodeIndex, u64 const& startTime);

        void     combineZonesActual(cweeList<Pzone>& zones, Pzone const& first, Pzone const& second);
        int      getAllSharedLinks(Pzone const& zone_1, Pzone const& zone_2, cweeList<Plink>& out, std::unordered_map<Slink*, bool>& map);

        bool     combineZones(EN_Project const& pr, cweeList<Pzone>& zones, cweeList<Plink>& sharedLinks, std::unordered_map<Slink*, bool>& sharedLinksMap, std::unordered_map<Slink*, StatusType>& sharedLinksStatusMap, std::unordered_map<Slink*, bool>& sharedLinksContolledMap);

        bool     isZoneWaterFlowPossible(EN_Project const& pr, Pzone& zone, std::unordered_map<Slink*, bool>& sharedLinksContolledMap, std::unordered_map<Slink*, StatusType>& sharedLinksStatusMap);

        int      parseNetwork(EN_Project pr);

        int readdata(EN_Project pr);
        int readdata(EN_Project pr, SectionType targetedSection);
        int getdata(EN_Project pr);
        int setError(Parser* parser, int tokindex, int errcode);
        int juncdata(EN_Project pr);
        int tankdata(EN_Project pr);
        int pipedata(EN_Project pr);
        int getpumpcurve(EN_Project pr, int n);
        int pumpdata(EN_Project pr);
        int valvedata(EN_Project pr);
        int patterndata(EN_Project pr);
        int curvedata(EN_Project pr);
        int coordata(EN_Project pr);
        int vertexdata(EN_Project pr);
        int demanddata(EN_Project pr);
        int controldata(EN_Project pr);
        int sourcedata(EN_Project pr);
        int emitterdata(EN_Project pr);
        int qualdata(EN_Project pr);
        int reactdata(EN_Project pr);
        int mixingdata(EN_Project pr);
        void changestatus(EN_Project pr, int j, StatusType status, SCALER y);
        int statusdata(EN_Project pr);
        int energydata(EN_Project pr);
        int reportdata(EN_Project pr);
        int timedata(EN_Project pr);
        int optionchoice(EN_Project pr, int n);
        int optionvalue(EN_Project pr, int n);
        int optiondata(EN_Project pr);
        void newrule(EN_Project pr);
        int newpremise(EN_Project pr, int logop);
        int newaction(EN_Project pr);
        int newpriority(EN_Project pr);
        void ruleerrmsg(EN_Project pr);
        int ruledata(EN_Project pr);
        int newline(EN_Project pr, int sect, char* line);
        int getpumpparams(EN_Project pr);
        int addnodeID(EN_Network net, int n, char* id);
        int addlinkID(EN_Network net, int n, char* id);

        int setreport(EN_Project pr, char* s);

        void deleterule(EN_Project pr, int index);
        void adjustrules(EN_Project pr, int objtype, int index);
        void adjusttankrules(EN_Project pr);
        Spremise* getpremise(Spremise* premises, int i);
        Saction* getaction(Saction* actions, int i);
        int clearreport(EN_Project pr);
        int copyreport(EN_Project pr, char* filename);
        void writelimits(EN_Project pr, int j1, int j2);
        int checklimits(Report* rpt, SCALER* y, int j1, int j2);
        void writetime(EN_Project pr, char* fmt);
        void writeheader(EN_Project pr, int type, int contin);
        void writeenergy(EN_Project pr);
        void writesummary(EN_Project pr);
        void writenodetable(EN_Project pr, Pfloat* x);
        void writelinktable(EN_Project pr, Pfloat* x);
        int writeresults(EN_Project pr);
        int writereport(EN_Project pr);
    };
};