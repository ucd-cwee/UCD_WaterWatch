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
#include "EPANET_Precompiled.h"

namespace epanet {
    namespace epanet_shared {
        int      updatepumpparams(::epanet::EN_Project const& pr, int pumpindex);
        int		findpattern(::epanet::EN_Network network, char* id);
        void     writeline(::epanet::EN_Project const& pr, char* s);
        int		findtank(::epanet::EN_Network network, int index);
        int		findpump(::epanet::EN_Network network, int index);
        int		findvalve(::epanet::EN_Network network, int index);
        int		findnode(::epanet::EN_Network network, char* id);
        int		findlink(::epanet::EN_Network network, char* id);
        int		findcurve(::epanet::EN_Network network, char* id);
        int		adddemand(::epanet::Pnode node, cubic_foot_per_second_t dbase, int dpat, char* dname, Ppattern TimePat);
        int		valvecheck(::epanet::EN_Project const& pr, int index, int type, int j1, int j2);
        int		resizecurve(::epanet::Pcurve curve, int size);
        int		addlinkvertex(::epanet::Plink link, SCALER x, SCALER y);
        void		clearrule(::epanet::EN_Project const& pr, int i);
        void		errmsg(::epanet::EN_Project const& pr, int errcode);
        int		unlinked(::epanet::EN_Project const& pr);
        void     setdefaults(::epanet::EN_Project const& pr);
        void     initreport(::epanet::Report* rpt);
        void     adjustdata(::epanet::EN_Project const& pr);
        void     initunits(::epanet::EN_Project const& pr);
        void     convertunits(::epanet::EN_Project const& pr);
        int      inittanks(::epanet::EN_Project const& pr);
        void     resetpumpflow(::epanet::EN_Project const& pr, int i);
        void     setlinksetting(::epanet::EN_Project const& pr, int index, SCALER value, SCALER* s, SCALER* k);
        void     setlinksetting(::epanet::EN_Project const& pr, int index, SCALER value, StatusType* s, SCALER* k);
        int      checktime(::epanet::EN_Project const& pr, Spremise* p);
        int      checkstatus(::epanet::EN_Project const& pr, Spremise* p);
        int      checkvalue(::epanet::EN_Project const& pr, Spremise* p);
        int      onactionlist(::epanet::EN_Project const& pr, int i, Saction* a);
        int      checkpremise(::epanet::EN_Project const& pr, Spremise* p);
        void     updateactionlist(::epanet::EN_Project const& pr, int i, Saction* actions);
        int      evalpremises(::epanet::EN_Project const& pr, int i);
        void     setlinkstatus(::epanet::EN_Project const& pr, int index, char value, SCALER* s, SCALER* k);
        void     setlinkstatus(::epanet::EN_Project const& pr, int index, char value, StatusType* s, SCALER* k);
        void     writeruleaction(::epanet::EN_Project const& pr, int k, char* ruleID);
        int      takeactions(::epanet::EN_Project const& pr);
        void     clearactionlist(Rules* rules);
        int      checkrules(::epanet::EN_Project const& pr, units::time::second_t dt);
        void     writepremise(Spremise* p, FILE* f, ::epanet::EN_Network net);
        void     writeaction(Saction* a, FILE* f, ::epanet::EN_Network net);
        int      writerule(::epanet::EN_Project const& pr, FILE* f, int ruleIndex);
        void     writestatchange(::epanet::EN_Project const& pr, int k, char s1, char s2);
        void     writerelerr(::epanet::EN_Project const& pr, int iter, SCALER relerr);
        void     writehydstat(::epanet::EN_Project const& pr, int iter, SCALER relerr);
        void     marknodes(::epanet::EN_Project const& pr, int m, int* nodelist, char* marked);
        void     getclosedlink(::epanet::EN_Project const& pr, int i, char* marked);
        int      disconnected(::epanet::EN_Project const& pr);
        void     writehyderr(::epanet::EN_Project const& pr, int errnode);
        int      writehydwarn(::epanet::EN_Project const& pr, int iter, SCALER relerr);
        int      getnodetype(::epanet::EN_Network net, int i);
        void     writecontrolaction(::epanet::EN_Project const& pr, int k, int i);
        void     writemassbalance(::epanet::EN_Project const& pr);
        void     DWpipecoeff(::epanet::EN_Project const& pr, int k);
        void     pipecoeff(::epanet::EN_Project const& pr, int k, cweeList<::epanet::Plink> const& link);
        void     pumpcoeff(::epanet::EN_Project const& pr, int k);
        void     curvecoeff(::epanet::EN_Project const& pr, int i, SCALER q, SCALER* h0, SCALER* r);
        void     gpvcoeff(::epanet::EN_Project const& pr, int k);
        void     pbvcoeff(::epanet::EN_Project const& pr, int k);
        void     tcvcoeff(::epanet::EN_Project const& pr, int k);
        void     prvcoeff(::epanet::EN_Project const& pr, int k, int n1, int n2);
        void     psvcoeff(::epanet::EN_Project const& pr, int k, int n1, int n2);
        void     fcvcoeff(::epanet::EN_Project const& pr, int k, int n1, int n2);
        void     valvecoeff(::epanet::EN_Project const& pr, int k);
        void     headlosscoeffs(::epanet::EN_Project const& pr);

        void     linkcoeffs(::epanet::EN_Project const& pr);
        void     emittercoeffs(::epanet::EN_Project const& pr);
        void     demandcoeffs(::epanet::EN_Project const& pr);
        void     nodecoeffs(::epanet::EN_Project const& pr);
        void     valvecoeffs(::epanet::EN_Project const& pr);
        void     emitterheadloss(::epanet::EN_Project const& pr, int i, foot_t* hloss, ft_per_cfs_t* hgrad);

        void     demandheadloss(::epanet::EN_Project const& pr, int i, foot_t dp, SCALER n, foot_t* hloss, ft_per_cfs_t* hgrad);

        void     matrixcoeffs(::epanet::EN_Project const& pr);
        void     resistcoeff(::epanet::EN_Project const& pr, int k);

        StatusType   next_prv_status(::epanet::EN_Project const& pr, int k, StatusType s, SCALER hset, SCALER h1, SCALER h2);
        StatusType   next_psv_status(::epanet::EN_Project const& pr, int k, StatusType s, SCALER hset, SCALER h1, SCALER h2);
        int          calc_and_set_prv_and_psv_status(::epanet::EN_Project const& pr);
        StatusType  cvstatus(::epanet::EN_Project const& pr, StatusType s, SCALER dh, SCALER q);
        StatusType  pumpstatus(::epanet::EN_Project const& pr, int k, SCALER dh);
        StatusType  next_fcv_status(::epanet::EN_Project const& pr, int k, StatusType s, SCALER h1, SCALER h2);
        void     tankstatus(::epanet::EN_Project const& pr, int k, int n1, int n2);
        int      linkstatus(::epanet::EN_Project const& pr);

        int      badvalve(::epanet::EN_Project const& pr, int n);
        int      pswitch(::epanet::EN_Project const& pr);
        SCALER   newflows(::epanet::EN_Project const& pr, Hydbalance* hbal);
        void     newlinkflows(::epanet::EN_Project const& pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum);
        void     newemitterflows(::epanet::EN_Project const& pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum);
        void     newdemandflows(::epanet::EN_Project const& pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum);
        void     checkhydbalance(::epanet::EN_Project const& pr, Hydbalance* hbal);

        int      pdaconverged(::epanet::EN_Project const& pr);
        void     reporthydbal(::epanet::EN_Project const& pr, Hydbalance* hbal);
        int      hasconverged(::epanet::EN_Project const& pr, SCALER* relerr, Hydbalance* hbal);

        template<int type>
        cweeSharedPtr<Sasset> FindOrMakeAssetPtr(::epanet::EN_Project const& pr, cweeStr name) {
            AUTO ptr = pr->network->Asset.Find(Sasset::Hash(asset_t::_from_integral(type), name));
            if (!ptr) {
                cweeSharedPtr<Sasset> a = make_cwee_shared<Sasset>(type, cweeAssetValueCollection<type>::Values(), name);
                ptr = pr->network->Asset.Add(a);
            }
            return *ptr;
        };

        void     SaveResultsForTimeStep(::epanet::EN_Project const& pr, HydraulicSimulationQuality simQuality = HydraulicSimulationQuality::HIGHRES);
        int      hydsolve(::epanet::EN_Project const& pr, int* iter, SCALER* relerr, HydraulicSimulationQuality simQuality = HydraulicSimulationQuality::HIGHRES);

        int      openhyd(::epanet::EN_Project const& pr);
        void     inithyd(::epanet::EN_Project const& pr, int initflag);
        int      runhyd(::epanet::EN_Project const& pr, units::time::second_t* t, HydraulicSimulationQuality simQuality = HydraulicSimulationQuality::HIGHRES);
        int      nexthyd(::epanet::EN_Project const& pr, units::time::second_t* tstep);
        void     closehyd(::epanet::EN_Project const& pr);
        void     freematrix(::epanet::EN_Project const& pr);
        int      allocmatrix(::epanet::EN_Project const& pr);

        void     initlinkflow(::epanet::EN_Project const& pr, int i, char s, SCALER k);
        void     demands(::epanet::EN_Project const& pr);
        int      controls(::epanet::EN_Project const& pr);
        units::time::second_t  timestep(::epanet::EN_Project const& pr);
        int      tanktimestep(::epanet::EN_Project const& pr, units::time::second_t* tstep);
        void     controltimestep(::epanet::EN_Project const& pr, units::time::second_t* tstep);
        void     ruletimestep(::epanet::EN_Project const& pr, units::time::second_t* tstep);
        void     addenergy(::epanet::EN_Project const& pr, units::time::second_t hstep);
        void     getenergy(::epanet::EN_Project const& pr, int k, kilowatt_t* kw, SCALER* eff);
        void     getallpumpsenergy(::epanet::EN_Project const& pr);
        void     tanklevels(::epanet::EN_Project const& pr, units::time::second_t tstep);
        cubic_foot_t  tankvolume(::epanet::EN_Project const& pr, int i, foot_t h);
        foot_t   tankgrade(::epanet::EN_Project const& pr, int i, cubic_foot_t v);
        SCALER   findsourcequal(::epanet::EN_Project const& pr, int n, SCALER volout, units::time::second_t tstep);
        void     reversesegs(::epanet::EN_Project const& pr, int k);
        void     addseg(::epanet::EN_Project const& pr, int k, SCALER v, SCALER c);
        char     setreactflag(::epanet::EN_Project const& pr);
        void     ratecoeffs(::epanet::EN_Project const& pr);
        void     reactpipes(::epanet::EN_Project const& pr, units::time::second_t dt);
        void     reacttanks(::epanet::EN_Project const& pr, units::time::second_t dt);
        SCALER   piperate(::epanet::EN_Project const& pr, int k);
        SCALER   pipereact(::epanet::EN_Project const& pr, int k, SCALER c, SCALER v, units::time::second_t dt);
        SCALER   tankreact(::epanet::EN_Project const& pr, SCALER c, SCALER v, SCALER kb, units::time::second_t dt);
        SCALER   bulkrate(::epanet::EN_Project const& pr, SCALER c, SCALER kb, SCALER order);
        SCALER   wallrate(::epanet::EN_Project const& pr, SCALER c, SCALER d, SCALER kw, SCALER kf);
        SCALER   mixtank(::epanet::EN_Project const& pr, int n, SCALER volin, SCALER massin, SCALER volout);
        void     tankmix1(::epanet::EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet);
        void     tankmix2(::epanet::EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet);
        void     tankmix3(::epanet::EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet);
        void     tankmix4(::epanet::EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet);
        void     transport(::epanet::EN_Project const& pr, units::time::second_t tstep);
        void     evalnodeinflow(::epanet::EN_Project const& pr, int k, units::time::second_t tstep, SCALER* volin, SCALER* massin);
        SCALER   findnodequal(::epanet::EN_Project const& pr, int n, SCALER volin, SCALER massin, SCALER volout, units::time::second_t tstep);
        SCALER   noflowqual(::epanet::EN_Project const& pr, int n);
        void     evalnodeoutflow(::epanet::EN_Project const& pr, int k, SCALER c, units::time::second_t tstep);
        void     updatemassbalance(::epanet::EN_Project const& pr, int n, SCALER massin, SCALER volout, units::time::second_t tstep);
        int      sortnodes(::epanet::EN_Project const& pr);
        int      selectnonstacknode(::epanet::EN_Project const& pr, int numsorted, int* indegree);
        void     initsegs(::epanet::EN_Project const& pr);
        int      openqual(::epanet::EN_Project const& pr);
        int      initqual(::epanet::EN_Project const& pr);
        int      runqual(::epanet::EN_Project const& pr, units::time::second_t* t);

        void     SaveResultsForQualityStep(::epanet::EN_Project const& pr);
        int      nextqual(::epanet::EN_Project const& pr, units::time::second_t* tstep);
        int      stepqual(::epanet::EN_Project const& pr, units::time::second_t* tleft);
        int      closequal(::epanet::EN_Project const& pr);
        SCALER   avgqual(::epanet::EN_Project const& pr, int k);
        SCALER   sourcequal(::epanet::EN_Project const& pr, Psource source);
        void     evalmassbalance(::epanet::EN_Project const& pr);
        SCALER   findstoredmass(::epanet::EN_Project const& pr);
        int      flowdirchanged(::epanet::EN_Project const& pr);
        int      savehyd(::epanet::EN_Project const& pr, units::time::second_t* htime);
        int      savehydstep(::epanet::EN_Project const& pr, units::time::second_t* hydstep);
        int      readhyd(::epanet::EN_Project const& pr, units::time::second_t* hydtime);
        int      readhydstep(::epanet::EN_Project const& pr, units::time::second_t* hydstep);
        int      saveoutput(::epanet::EN_Project const& pr);
        int      nodeoutput(::epanet::EN_Project const& pr, int j, REAL4* x, SCALER ucf);
        int      linkoutput(::epanet::EN_Project const& pr, int j, REAL4* x, SCALER ucf);
        int      savefinaloutput(::epanet::EN_Project const& pr);
        int      savetimestat(::epanet::EN_Project const& pr, REAL4* x, HdrType objtype);
        int      savenetreacts(::epanet::EN_Project const& pr, SCALER wbulk, SCALER wwall, SCALER wtank, SCALER wsource);
        int      saveepilog(::epanet::EN_Project const& pr);
        void     saveauxdata(::epanet::EN_Project const& pr, FILE* f);
        int      saveinpfile(::epanet::EN_Project const& pr, const char* fname);
    };
};