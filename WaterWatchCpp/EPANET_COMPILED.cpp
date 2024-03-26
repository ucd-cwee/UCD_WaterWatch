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
#include "EPANET_COMPILED.h"
#include "InterpolatedMatrix.h"
#include "FileSystemH.h"
#include "../FiberTasks/Fibers.h"

namespace epanet {
    namespace epanet_shared {
        int      updatepumpparams(EN_Project const& pr, int pumpindex)
            /*
            **-------------------------------------------------------------
            **  Input:   pumpindex = index of a pump
            **  Output:  returns error code
            **  Purpose: computes & checks a pump's head curve coefficients
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Ppump pump;
            Pcurve curve;

            int m;
            int curveindex;
            int npts = 0;
            int errcode = 0;
            SCALER a, b, c, h0 = 0.0, h1 = 0.0, h2 = 0.0, q1 = 0.0, q2 = 0.0;

            pump = net->Pump[pumpindex];
            if (pump->Ptype == CONST_HP)  // Constant Hp pump
            {
                pump->H0 = 0.0_ft;
                pump->R = -8.814 * pump->Km;
                pump->N = -1.0;
                pump->Hmax = (foot_t)(double)BIG; // No head limit
                pump->Qmax = (cubic_foot_per_second_t)(double)BIG; // No flow limit
                pump->InitialFlow(pr->times.GetSimStartTime()) = 1.0_cfs;   // Init. flow = 1 cfs
                return errcode;
            }

            else if (pump->Ptype == NOCURVE) // Pump curve specified
            {
                curveindex = pump->Hcurve;
                if (curveindex == 0) return 226;
                curve = net->Curve[curveindex];
                curve->Type = PUMP_CURVE;
                npts = curve->Curve.GetNumValues();

                // Generic power function curve
                if (npts == 1)
                {
                    pump->Ptype = POWER_FUNC;
                    q1 = curve->Curve.GetMinTime();
                    h1 = curve->Curve.GetCurrentValue(curve->Curve.GetMinTime());
                    h0 = 1.33334 * h1;
                    q2 = 2.0 * q1;
                    h2 = 0.0;
                }

                // 3 point curve with shutoff head
                else if (npts == 3 && curve->Curve.GetMinTime() == 0.0)
                {
                    pump->Ptype = POWER_FUNC;

                    h0 = curve->Curve.GetValueAtIndex(0);
                    q1 = curve->Curve.GetTimeAtIndex(1);
                    h1 = curve->Curve.GetValueAtIndex(1);
                    q2 = curve->Curve.GetTimeAtIndex(2);
                    h2 = curve->Curve.GetValueAtIndex(2);
                }
                // Custom pump curve
                else
                {
                    pump->Ptype = CUSTOM;
                    for (m = 1; m < npts; m++) {
                        if (curve->Curve.GetValueAtIndex(m) >= curve->Curve.GetValueAtIndex(m - 1)) return 227;
                    }

                    pump->Qmax = pr->convertToUnit< cubic_foot_per_second_t>(curve->Curve.GetTimeForValue(0));
                    pump->InitialFlow(pr->times.GetSimStartTime()) = pr->convertToUnit< cubic_foot_per_second_t>((curve->Curve.GetMinTime() + curve->Curve.GetMaxTime()) / 2.0); // assumes the initial operating point is halfway between the min and max flows in the curve
                    pump->Hmax = pr->convertToUnit < foot_t>(curve->Curve.GetCurrentValue(0));
                    // H0, R, and N will be set from the headlosscoeff path during the simulation. 
                }

                // Compute shape factors & limits of power function curves
                if (pump->Ptype == POWER_FUNC)
                {
                    if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c)) return 227;
                    else
                    {
                        pump->H0 = pr->convertToUnit < foot_t>(-a);
                        pump->R = -b;
                        pump->N = c;
                        pump->InitialFlow(pr->times.GetSimStartTime()) = pr->convertToUnit< cubic_foot_per_second_t>(q1); // assumes the initial operating point is the middle point of the 3 provided points
                        pump->Qmax = pr->convertToUnit< cubic_foot_per_second_t>(::epanet::pow((-a / b), (1.0 / c)));
                        pump->Hmax = pr->convertToUnit < foot_t>(h0);
                    }
                }
            }
            return 0;
        };
        int		findpattern(EN_Network network, char* id)
            /*----------------------------------------------------------------
            **  Input:   id = time pattern ID
            **  Output:  none
            **  Returns: time pattern index, or -1 if pattern not found
            **  Purpose: finds index of time pattern given its ID
            **----------------------------------------------------------------
            */
        {
            int i;

            // Don't forget to include the "dummy" pattern 0 in the search
            for (i = 0; i <= network->Npats; i++)
            {
                if (strcmp(id, network->Pattern[i]->ID.c_str()) == 0) return i;
            }
            return -1;
        };
        void     writeline(EN_Project const& pr, char* s)
            /*
            **--------------------------------------------------------------
            **   Input:   *s = text string
            **   Output:  none
            **   Purpose: writes a line of output to report file
            **--------------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;

            if (rpt->RptFile == NULL) return;
            if (rpt->Rptflag)
            {
                if (rpt->LineNum == (long)rpt->PageSize)
                {
                    rpt->PageNum++;
                    if (fprintf(rpt->RptFile, FMT82, (int)rpt->PageNum, pr->Title[0]) < 0)
                    {
                        rpt->Fprinterr = TRUE;
                    }
                    rpt->LineNum = 3;
                }
            }
            if (fprintf(rpt->RptFile, "\n  %s", s) < 0) rpt->Fprinterr = TRUE;
            rpt->LineNum++;
        };
        int		findtank(EN_Network network, int index)
            /*----------------------------------------------------------------
            **  Input:   index = node index
            **  Output:  none
            **  Returns: index of tank with given node id, or NOTFOUND if tank not found
            **  Purpose: for use in the deletenode function
            **----------------------------------------------------------------
            */
        {
            int i;
            for (i = 1; i <= network->Ntanks; i++)
            {
                if (network->Tank[i]->Node == index) return i;
            }
            return NOTFOUND;
        };
        int		findpump(EN_Network network, int index)
            /*----------------------------------------------------------------
            **  Input:   index = link ID
            **  Output:  none
            **  Returns: index of pump with given link id, or NOTFOUND if pump not found
            **  Purpose: for use in the deletelink function
            **----------------------------------------------------------------
            */
        {
            int i;
            for (i = 1; i <= network->Npumps; i++)
            {
                if (network->Pump[i]->Link == index) return i;
            }
            return NOTFOUND;
        };
        int		findvalve(EN_Network network, int index)
            /*----------------------------------------------------------------
            **  Input:   index = link ID
            **  Output:  none
            **  Returns: index of valve with given link id, or NOTFOUND if valve not found
            **  Purpose: for use in the deletelink function
            **----------------------------------------------------------------
            */
        {
            int i;
            for (i = 1; i <= network->Nvalves; i++)
            {
                if (network->Valve[i]->Link == index) return i;
            }
            return NOTFOUND;
        };
        int		findnode(EN_Network network, char* id)
            /*----------------------------------------------------------------
            **  Input:   id = node ID
            **  Output:  none
            **  Returns: index of node with given ID, or 0 if ID not found
            **  Purpose: uses hash table to find index of node with given ID
            **----------------------------------------------------------------
            */
        {
            return (hashtable_t::hashtable_find(network->NodeHashTable, id));
        };
        int		findlink(EN_Network network, char* id)
            /*----------------------------------------------------------------
            **  Input:   id = link ID
            **  Output:  none
            **  Returns: index of link with given ID, or 0 if ID not found
            **  Purpose: uses hash table to find index of link with given ID
            **----------------------------------------------------------------
            */
        {
            return (hashtable_t::hashtable_find(network->LinkHashTable, id));
        };
        int		findcurve(EN_Network network, char* id)
            /*----------------------------------------------------------------
            **  Input:   id = data curve ID
            **  Output:  none
            **  Returns: data curve index, or 0 if curve not found
            **  Purpose: finds index of data curve given its ID
            **----------------------------------------------------------------
            */
        {
            int i;
            for (i = 1; i <= network->Ncurves; i++)
            {
                if (strcmp(id, network->Curve[i]->ID) == 0) return i;
            }
            return 0;
        };
        int		adddemand(Pnode node, cubic_foot_per_second_t dbase, int dpat, char* dname, Ppattern TimePat)
            /*----------------------------------------------------------------
            **  Input:   node = a network junction node
            **           dbase = base demand value
            **           dpat = demand pattern index
            **           dname = name of demand category
            **  Output:  returns TRUE if successful, FALSE if not
            **  Purpose: adds a new demand category to a node.
            **----------------------------------------------------------------
            */
        {
            // Create a new demand struct
            Sdemand demand;

            // Assign it the designated properties
            demand.Base = dbase;
            demand.Pat = dpat;
            demand.TimePat = TimePat;

            demand.Name = NULL;
            if (dname && strlen(dname) > 0) demand.Name = xstrcpy(&demand.Name, dname, MAXID);

            node->D.Append(demand);

            return TRUE;
        };
        int		valvecheck(EN_Project const& pr, int index, int type, int j1, int j2)
            /*
            **--------------------------------------------------------------
            **  Input:   index = link index
            **           type = valve type
            **           j1   = index of upstream node
            **           j2   = index of downstream node
            **  Output:  returns an error code
            **  Purpose: checks for illegal connections between valves
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            int k, vj1, vj2;
            LinkType vtype;
            Pvalve valve;

            if (type == PRV || type == PSV || type == FCV)
            {
                // Can't be connected to a fixed grade node
                if (j1 > net->Njuncs || j2 > net->Njuncs) return 219;

                // Examine each existing valve
                for (k = 1; k <= net->Nvalves; k++)
                {
                    valve = net->Valve[k];
                    if (valve->Link == index) continue;
                    vj1 = valve->N1;
                    vj2 = valve->N2;
                    vtype = valve->Type;

                    // Cannot have two PRVs sharing downstream nodes or in series
                    if (vtype == PRV && type == PRV)
                    {
                        if (vj2 == j2 || vj2 == j1 || vj1 == j2) return 220;
                    }

                    // Cannot have two PSVs sharing upstream nodes or in series
                    if (vtype == PSV && type == PSV)
                    {
                        if (vj1 == j1 || vj1 == j2 || vj2 == j1) return 220;
                    }

                    // Cannot have PSV connected to downstream node of PRV
                    if (vtype == PSV && type == PRV && vj1 == j2) return 220;
                    if (vtype == PRV && type == PSV && vj2 == j1) return 220;

                    // Cannot have PSV connected to downstream node of FCV
                    // nor have PRV connected to upstream node of FCV
                    if (vtype == FCV && type == PSV && vj2 == j1) return 220;
                    if (vtype == FCV && type == PRV && vj1 == j2) return 220;
                    if (vtype == PSV && type == FCV && vj1 == j2) return 220;
                    if (vtype == PRV && type == FCV && vj2 == j1) return 220;
                }
            }
            return 0;
        };
        int		resizecurve(Pcurve curve, int size)
            /*----------------------------------------------------------------
            **  Input:   curve = a data curve object
            **           size = desired number of curve data points
            **  Output:  error code
            **  Purpose: resizes a data curve to a desired size
            **----------------------------------------------------------------
            */
        {
            return 0;
        };
        int		addlinkvertex(Plink link, SCALER x, SCALER y)
            /*----------------------------------------------------------------
            **  Input:   link = pointer to a network link
            **           x = x-coordinate of a new vertex
            **           y = y-coordiante of a new vertex
            **  Returns: an error code
            **  Purpose: adds to a link's collection of vertex points.
            **----------------------------------------------------------------
            */
        {
            int CHUNKSIZE = 5;
            int n;
            Pvertices vertices;
            if (link->Vertices == nullptr)
            {
                vertices = make_cwee_shared<Svertices>();
                link->Vertices = vertices;
            }
            vertices = link->Vertices;
            vertices->Array.Append(cweePair<SCALER, SCALER>(x, y));
            return 0;
        };
        void		clearrule(EN_Project const& pr, int i)
            //-----------------------------------------------------------
            //  Clears memory used by a rule for premises & actions
            //-----------------------------------------------------------
        {
            EN_Network const& net = pr->network;

            Spremise* p;
            Spremise* pnext;
            Saction* a;
            Saction* anext;

            p = net->Rule[i]->Premises;
            while (p != NULL)
            {
                pnext = p->next;
                free(p);
                p = pnext;
            }
            a = net->Rule[i]->ThenActions;
            while (a != NULL)
            {
                anext = a->next;
                free(a);
                a = anext;
            }
            a = net->Rule[i]->ElseActions;
            while (a != NULL)
            {
                anext = a->next;
                free(a);
                a = anext;
            }
        };
        void		errmsg(EN_Project const& pr, int errcode)
            /*----------------------------------------------------------------
            **  Input:   errcode = error code
            **  Output:  none
            **  Purpose: writes error message to report file
            **----------------------------------------------------------------
            */
        {
            char errmsg[MAXMSG + 1] = "";
            if (errcode == 309) /* Report file write error -  */
            {                   /* Do not write msg to file.  */

            }
            else if (pr->report.RptFile != NULL && pr->report.Messageflag && errcode > 100)
            {
                sprintf(pr->Msg, "Error %d: %s", errcode, geterrmsg(errcode, errmsg));
                writeline(pr, pr->Msg);
            }
        };
        int		unlinked(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code if any unlinked junctions found
            ** Purpose: checks for unlinked junctions in network
            **
            ** NOTE: unlinked tanks have no effect on computations.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            int i, count = 0;
            char errmsg[MAXMSG + 1] = "";

            for (i = 1; i <= net->Njuncs; i++)
            {
                if (pr->network->Adjlist[i] == nullptr)
                {
                    count++;
                    sprintf(pr->Msg, "Error 234: %s %s", geterrmsg(234, errmsg), net->Node[i]->ID);
                    writeline(pr, pr->Msg);
                }
                if (count >= 10) break;
            }
            if (count > 0) return 233;
            return 0;
        };
        void     setdefaults(EN_Project const& pr)
            /*
            **----------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: assigns default values to a project's variables
            **----------------------------------------------------------------
            */
        {
            Parser* parser = &pr->parser;
            Report* rpt = &pr->report;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Times* time = &pr->times;
            Outfile* out = &pr->outfile;

            strncpy(pr->Title[0], "", TITLELEN);
            strncpy(pr->Title[1], "", TITLELEN);
            strncpy(pr->Title[2], "", TITLELEN);
            strncpy(out->HydFname, "", MAXFNAME);
            strncpy(pr->MapFname, "", MAXFNAME);
            strncpy(qual->ChemName, t_CHEMICAL, MAXID);
            strncpy(qual->ChemUnits, u_MGperL, MAXID);
            strncpy(parser->DefPatID, DEFPATID, MAXID);

            pr->Warnflag = FALSE;       // Warning flag is off
            parser->Unitsflag = US;     // US unit system
            parser->Flowflag = GPM;     // Flow units are gpm
            parser->Pressflag = PSI;    // Pressure units are psi
            parser->DefPat = 0;         // Default demand pattern index
            out->Hydflag = SCRATCH;     // No external hydraulics file
            rpt->Tstatflag = SERIES;    // Generate time series output

            hyd->Formflag = HW;         // Use Hazen-Williams formula
            hyd->Htol = HTOL;           // Default head tolerance
            hyd->Qtol = QTOL;           // Default flow tolerance
            hyd->Hacc = HACC;           // Default hydraulic accuracy
            hyd->FlowChangeLimit = 0.0_cfs; // Default flow change limit
            hyd->HeadErrorLimit = 0.0_ft;  // Default head error limit
            hyd->DemandModel = DDA;     // Demand driven analysis
            hyd->Pmin = 0.0;            // Minimum demand pressure (ft)
            hyd->Preq = MINPDIFF;       // Required demand pressure (ft)
            hyd->Pexp = 0.5;            // Pressure function exponent
            hyd->MaxIter = MAXITER;     // Default max. hydraulic trials
            hyd->ExtraIter = -1;        // Stop if network unbalanced
            hyd->Viscos = (double)MISSING;      // Temporary viscosity
            hyd->SpGrav = SPGRAV;       // Default specific gravity
            hyd->Epat = 0;              // No energy price pattern
            hyd->Ecost = 0.0;           // Zero unit energy cost
            hyd->Dcost = 0.0;           // Zero energy demand charge
            hyd->Epump = EPUMP;         // Default pump efficiency
            hyd->Emax = 0.0_kW;            // Zero peak energy usage
            hyd->Etotal = 0.0_kW;
            hyd->Qexp = 2.0;            // Flow exponent for emitters
            hyd->Dmult = 1.0;           // Demand multiplier
            hyd->RQtol = RQTOL;         // Default hydraulics parameters
            hyd->CheckFreq = CHECKFREQ;
            hyd->MaxCheck = MAXCHECK;
            hyd->DampLimit = DAMPLIMIT;

            qual->Qualflag = NONE;      // No quality simulation
            qual->Ctol = MISSING;       // No pre-set quality tolerance
            qual->TraceNode = "";        // No source tracing
            qual->BulkOrder = 1.0;      // 1st-order bulk reaction rate
            qual->WallOrder = 1.0;      // 1st-order wall reaction rate
            qual->TankOrder = 1.0;      // 1st-order tank reaction rate
            qual->Kbulk = 0.0;          // No global bulk reaction
            qual->Kwall = 0.0;          // No global wall reaction
            qual->Climit = 0.0;         // No limiting potential quality
            qual->Diffus = (double)MISSING;     // Temporary diffusivity
            qual->Rfactor = 0.0;        // No roughness-reaction factor
            qual->MassBalance.ratio = 0.0;

            time->Dur = 0;              // 0 sec duration (steady state)
            time->Tstart = 0;           // Starting time of day
            time->Pstart = 0;           // Starting pattern period
            time->Hstep = 3600;         // 1 hr hydraulic time step
            time->Qstep = 0;            // No pre-set quality time step
            time->Pstep = 3600;         // 1 hr time pattern period
            time->Rstep = 3600;         // 1 hr reporting period
            time->Rstep_JunctionsPipes = time->Rstep * 1.0;         // 1 hr reporting period
            time->Rulestep = 0;         // No pre-set rule time step
            time->Rstart = 0;           // Start reporting at time 0
        };
        void     initreport(Report* rpt)
            /*
            **----------------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: initializes reporting options
            **----------------------------------------------------------------------
            */
        {
            int i;
            strncpy(rpt->Rpt2Fname, "", MAXFNAME);

            // Initialize general reporting options
            rpt->PageSize = PAGESIZE;      // Default page size for report
            rpt->Summaryflag = TRUE;       // Write summary report
            rpt->Messageflag = TRUE;       // Report error/warning messages
            rpt->Statflag = FALSE;         // No hydraulic status reports
            rpt->Energyflag = FALSE;       // No energy usage report
            rpt->Nodeflag = 0;             // No reporting on nodes
            rpt->Linkflag = 0;             // No reporting on links

            // Initialize options for each reported variable field
            for (i = 0; i < MAXVAR; i++)
            {
                strncpy(rpt->Field[i].Name, Fldname[i], MAXID);
                rpt->Field[i].Enabled = FALSE;        // Not included in report
                rpt->Field[i].Precision = 2;          // 2 decimal precision
                rpt->Field[i].RptLim[LOW] = SQR(BIG); // No reporting limits
                rpt->Field[i].RptLim[HI] = -SQR(BIG);
            }
            rpt->Field[FRICTION].Precision = 3;

            // Set default set of variables reported on
            for (i = DEMAND; i <= QUALITY; i++)
            {
                rpt->Field[i].Enabled = TRUE;
            }
            for (i = FLOW; i <= HEADLOSS; i++)
            {
                rpt->Field[i].Enabled = TRUE;
            }
        };
        void     adjustdata(EN_Project const& pr)
            /*
            **----------------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: adjusts project data after input file has been processed
            **----------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Times* time = &pr->times;
            Parser* parser = &pr->parser;
            Report* rpt = &pr->report;

            int i;
            SCALER ucf;     // Unit conversion factor
            Plink link;
            Pnode node;
            Ptank tank;

            // Use 1 hr pattern & report time step if none specified
            if (time->Pstep <= 0_s) time->Pstep = 3600_s;
            if (time->Rstep == 0_s) time->Rstep = time->Pstep;
            time->Rstep_JunctionsPipes = time->Rstep * 1.0;

            // Hydraulic time step cannot be greater than pattern or report time step
            if (time->Hstep <= 0_s) time->Hstep = 3600_s;
            if (time->Hstep > time->Pstep) time->Hstep = time->Pstep;
            if (time->Hstep > time->Rstep) time->Hstep = time->Rstep;

            // Report start time cannot be greater than simulation duration
            if (time->Rstart > time->Dur) time->Rstart = 0;

            // No water quality analysis for single period run
            if (time->Dur == 0_s) qual->Qualflag = NONE;

            // If no quality timestep, then make it 1/10 of hydraulic timestep
            if (time->Qstep == 0_s) time->Qstep = time->Hstep / 10;

            // If no rule time step then make it 1/10 of hydraulic time step
            // but not greater than hydraulic time step
            if (time->Rulestep == 0_s) time->Rulestep = time->Hstep / 10;
            time->Rulestep = fMIN(time->Rulestep, time->Hstep);

            // Quality timestep cannot exceed hydraulic timestep
            time->Qstep = fMIN(time->Qstep, time->Hstep);

            // If no quality tolerance, then use default values
            if (qual->Ctol == MISSING)
            {
                if (qual->Qualflag == AGE) qual->Ctol = AGETOL;
                else qual->Ctol = CHEMTOL;
            }

            // Determine units system based on flow units
            switch (parser->Flowflag)
            {
            case LPS: // Liters/sec
            case LPM: // Liters/min
            case MLD: // megaliters/day
            case CMH: // cubic meters/hr
            case CMD: // cubic meters/day
                parser->Unitsflag = SI;
                break;
            default:
                parser->Unitsflag = US;
            }

            // Revise pressure units depending on flow units
            if (parser->Unitsflag != SI) parser->Pressflag = PSI;
            else if (parser->Pressflag == PSI) parser->Pressflag = METERS;

            // Store value of viscosity & diffusivity
            ucf = 1.0;
            if (parser->Unitsflag == SI) ucf = SQR(MperFT);
            if (hyd->Viscos == (squared_foot_per_second_t)(double)MISSING)
            {
                hyd->Viscos = VISCOS;               // No viscosity supplied
            }
            else if (hyd->Viscos > (squared_foot_per_second_t)1.e-3)
            {
                hyd->Viscos = (SCALER)(double)hyd->Viscos * VISCOS; // Multiplier supplied
            }
            else hyd->Viscos = hyd->Viscos / ucf;   // Actual value supplied
            if (qual->Diffus == (squared_foot_per_second_t)(double)MISSING)
            {
                qual->Diffus = DIFFUS;              // No viscosity supplied
            }
            else if (qual->Diffus > (squared_foot_per_second_t)1.e-4)
            {
                qual->Diffus = (SCALER)(double)qual->Diffus * DIFFUS; // Multiplier supplied
            }
            else qual->Diffus = qual->Diffus / ucf;   //  Actual value supplied

            // Set exponent in head loss equation and adjust flow-resistance tolerance.
            if (hyd->Formflag == HW) hyd->Hexp = 1.852;
            else hyd->Hexp = 2.0;

            // See if default reaction coeffs. apply
            for (i = 1; i <= net->Nlinks; i++)
            {
                link = net->Link[i];
                if (link->Type > PIPE) continue;
                if (link->Kb == MISSING) link->Kb = qual->Kbulk;   // Bulk coeff.
                if (link->Kw == MISSING)                           // Wall coeff.
                {
                    // Rfactor is the pipe roughness correlation factor
                    if (qual->Rfactor == 0.0) link->Kw = qual->Kwall;
                    else if ((link->Kc > 0.0) && (link->Diam > 0.0_ft))
                    {
                        if (hyd->Formflag == HW) link->Kw = qual->Rfactor / link->Kc;
                        if (hyd->Formflag == DW)
                        {
                            link->Kw = qual->Rfactor / ABS(::epanet::log(link->Kc / link->Diam));
                        }
                        if (hyd->Formflag == CM) link->Kw = qual->Rfactor * link->Kc;
                    }
                    else link->Kw = 0.0;
                }
            }
            for (i = 1; i <= net->Ntanks; i++)
            {
                tank = net->Tank[i];
                if (tank->Kb == MISSING) tank->Kb = qual->Kbulk;
            }

            // Use default pattern if none assigned to a demand
            parser->DefPat = findpattern(net, parser->DefPatID);
            if (parser->DefPat > 0) for (i = 1; i <= net->Nnodes; i++)
            {
                node = net->Node[i];
                for (auto& demand : node->D) {
                    if (demand.Pat == 0) {
                        demand.Pat = parser->DefPat;
                        demand.TimePat = net->Pattern[parser->DefPat];
                    }
                }
            }

            // Remove QUALITY as a reporting variable if no WQ analysis
            if (qual->Qualflag == NONE) rpt->Field[QUALITY].Enabled = FALSE;
        };
        void     initunits(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: determines unit conversion factors
            **--------------------------------------------------------------
            */
        {
            Parser* parser = &pr->parser;
            Report* rpt = &pr->report;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Times* time = &pr->times;

            SCALER dcf,    // distance conversion factor
                ccf,    // concentration conversion factor
                qcf,    // flow conversion factor
                hcf,    // head conversion factor
                pcf,    // pressure conversion factor
                wcf;    // energy conversion factor

            if (parser->Unitsflag == SI)  // SI units
            {
                ::strcpy(rpt->Field[DEMAND].Units, RptFlowUnitsTxt[parser->Flowflag]);
                ::strcpy(rpt->Field[ELEV].Units, u_METERS);
                ::strcpy(rpt->Field[HEAD].Units, u_METERS);
                if (parser->Pressflag == METERS) ::strcpy(rpt->Field[PRESSURE].Units, u_METERS);
                else                             ::strcpy(rpt->Field[PRESSURE].Units, u_KPA);
                ::strcpy(rpt->Field[LENGTH].Units, u_METERS);
                ::strcpy(rpt->Field[DIAM].Units, u_MMETERS);
                ::strcpy(rpt->Field[FLOW].Units, RptFlowUnitsTxt[parser->Flowflag]);
                ::strcpy(rpt->Field[VELOCITY].Units, u_MperSEC);
                ::strcpy(rpt->Field[HEADLOSS].Units, u_per1000M);
                ::strcpy(rpt->Field[FRICTION].Units, "");
                ::strcpy(rpt->Field[POWER].Units, u_KW);

                dcf = 1000.0 * MperFT;
                qcf = LPSperCFS;
                if (parser->Flowflag == LPM) qcf = LPMperCFS;
                if (parser->Flowflag == MLD) qcf = MLDperCFS;
                if (parser->Flowflag == CMH) qcf = CMHperCFS;
                if (parser->Flowflag == CMD) qcf = CMDperCFS;

                hcf = MperFT;
                if (parser->Pressflag == METERS) pcf = MperFT * hyd->SpGrav;
                else pcf = KPAperPSI * PSIperFT * hyd->SpGrav;
                wcf = KWperHP;
            }
            else  // US units
            {
                ::strcpy(rpt->Field[DEMAND].Units, RptFlowUnitsTxt[parser->Flowflag]);
                ::strcpy(rpt->Field[ELEV].Units, u_FEET);
                ::strcpy(rpt->Field[HEAD].Units, u_FEET);
                ::strcpy(rpt->Field[PRESSURE].Units, u_PSI);
                ::strcpy(rpt->Field[LENGTH].Units, u_FEET);
                ::strcpy(rpt->Field[DIAM].Units, u_INCHES);
                ::strcpy(rpt->Field[FLOW].Units, RptFlowUnitsTxt[parser->Flowflag]);
                ::strcpy(rpt->Field[VELOCITY].Units, u_FTperSEC);
                ::strcpy(rpt->Field[HEADLOSS].Units, u_per1000FT);
                ::strcpy(rpt->Field[FRICTION].Units, "");
                ::strcpy(rpt->Field[POWER].Units, u_HP);

                dcf = 12.0;
                qcf = 1.0;
                if (parser->Flowflag == GPM) qcf = GPMperCFS;
                if (parser->Flowflag == MGD) qcf = MGDperCFS;
                if (parser->Flowflag == IMGD) qcf = IMGDperCFS;
                if (parser->Flowflag == AFD)  qcf = AFDperCFS;
                hcf = 1.0;
                pcf = PSIperFT * hyd->SpGrav;
                wcf = 1.0;
            }

            ::strcpy(rpt->Field[QUALITY].Units, "");
            ccf = 1.0;
            if (qual->Qualflag == CHEM)
            {
                ccf = 1.0 / LperFT3;
                strncpy(rpt->Field[QUALITY].Units, qual->ChemUnits, MAXID);
                strncpy(rpt->Field[REACTRATE].Units, qual->ChemUnits, MAXID);
                strcat(rpt->Field[REACTRATE].Units, t_PERDAY);
            }
            if (qual->Qualflag == ENERGYINTENSITY)
            {
                ccf = 1.0;
                strncpy(rpt->Field[QUALITY].Units, qual->ChemUnits, MAXID);
                strncpy(rpt->Field[REACTRATE].Units, qual->ChemUnits, MAXID);
            }
            else if (qual->Qualflag == AGE) ::strcpy(rpt->Field[QUALITY].Units, u_HOURS);
            else if (qual->Qualflag == TRACE_QUAL) ::strcpy(rpt->Field[QUALITY].Units, u_PERCENT);

            pr->Ucf[DEMAND] = qcf;
            pr->Ucf[ELEV] = hcf;
            pr->Ucf[HEAD] = hcf;
            pr->Ucf[PRESSURE] = pcf;
            pr->Ucf[QUALITY] = ccf;
            pr->Ucf[LENGTH] = hcf;
            pr->Ucf[DIAM] = dcf;
            pr->Ucf[FLOW] = qcf;
            pr->Ucf[VELOCITY] = hcf;
            pr->Ucf[HEADLOSS] = hcf;
            pr->Ucf[LINKQUAL] = ccf;
            pr->Ucf[REACTRATE] = ccf;
            pr->Ucf[FRICTION] = 1.0;
            pr->Ucf[POWER] = wcf;
            pr->Ucf[VOLUME] = hcf * hcf * hcf;

            // Report time in minutes if hyd. time step < 1/2 hr.
            if (time->Hstep < 1800_s)
            {
                pr->Ucf[TIME] = 1.0 / 60.0;
                ::strcpy(rpt->Field[TIME].Units, u_MINUTES);
            }
            else
            {
                pr->Ucf[TIME] = 1.0 / 3600.0;
                ::strcpy(rpt->Field[TIME].Units, u_HOURS);
            }
        };
        void     convertunits(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: converts units of input data
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Parser* parser = &pr->parser;

            int i, j, k;
            SCALER ucf;     // Unit conversion factor
            Pnode node;
            Ptank tank;
            Plink link;
            Ppump pump;
            Pcontrol control;

            // Convert nodal elevations & initial WQ
            // (WQ source units are converted in QUALITY.C
            for (i = 1; i <= net->Nnodes; i++)
            {
                node = net->Node[i];
                // node->El /= pr->Ucf[ELEV];
                node->C0(pr->times.GetSimStartTime()) /= pr->Ucf[QUALITY];
            }

            // Convert PDA pressure limits
            hyd->Pmin /= pr->Ucf[PRESSURE];
            hyd->Preq /= pr->Ucf[PRESSURE];

            // Convert emitter discharge coeffs. to head loss coeff.
            ucf = ::epanet::pow(pr->Ucf[FLOW], hyd->Qexp) / pr->Ucf[PRESSURE];
            for (i = 1; i <= net->Njuncs; i++)
            {
                node = net->Node[i];
                if (node->Ke > 0.0) node->Ke = ucf / ::epanet::pow(node->Ke, hyd->Qexp);
            }

            // Initialize tank variables (convert tank levels to elevations)
            for (j = 1; j <= net->Ntanks; j++) {
                tank = net->Tank[j];
                tank->InitialHead(pr->times.GetSimStartTime()) += tank->El;
                tank->Hmin += tank->El;
                tank->Hmax += tank->El;
                tank->Kb /= SECperDAY;
                hyd->TankVolume[j] = tank->Volume(pr, tank->InitialHead(pr->times.GetSimStartTime()));
                //tank->C = node->C0(pr->times.GetSimStartTime());
                qual->TankConcentration[j] = node->C0(pr->times.GetSimStartTime());
            }

            // Convert water quality concentration options
            qual->Climit /= pr->Ucf[QUALITY];
            qual->Ctol /= pr->Ucf[QUALITY];

            // Convert global reaction coeffs.
            qual->Kbulk /= SECperDAY;
            qual->Kwall /= SECperDAY;

            // Convert units of link parameters
            for (k = 1; k <= net->Nlinks; k++) {
                link = net->Link[k];
                if (link->Type <= PIPE)
                {
                    // Convert D-W roughness from millifeet (or mm) to ft
                    if (hyd->Formflag == DW) link->Kc /= (1000.0 * pr->Ucf[ELEV]);

                    // Convert minor loss coeff. from V^2/2g basis to Q^2 basis
                    link->Km = (double)(0.02517 * link->Km / SQR(link->Diam) / SQR(link->Diam));

                    // Convert units on reaction coeffs.
                    link->Kb /= SECperDAY;
                    link->Kw /= SECperDAY;
                }

                else if (link->Type == PUMP)
                {
                    // Convert units for pump curve parameters
                    i = findpump(net, k);
                    pump = net->Pump[i];
                    if (pump->Ptype == CONST_HP)
                    {
                        // For constant hp pump, convert kw to hp
                        if (parser->Unitsflag == SI) pump->R /= pr->Ucf[POWER];
                    }
                    else
                    {
                        // For power curve pumps, convert shutoff head and flow coeff.
                        if (pump->Ptype == POWER_FUNC)
                        {
                            pump->R *= (::epanet::pow(pr->Ucf[FLOW], pump->N) / pr->Ucf[HEAD]);
                        }
                    }
                }
                else
                {
                    // For flow control valves, convert flow setting
                    // while for other valves convert pressure setting
                    link->Km = (double)(0.02517 * link->Km / SQR(link->Diam) / SQR(link->Diam));
                    if (link->Kc != MISSING) switch (link->Type)
                    {
                    case FCV:
                        link->Kc /= pr->Ucf[FLOW];
                        break;
                    case PRV:
                    case PSV:
                    case PBV:
                        link->Kc /= pr->Ucf[PRESSURE];
                        break;
                    default:
                        break;
                    }
                }
            }

            // Convert units on control settings
            for (i = 1; i <= net->Ncontrols; i++) {
                control = net->Control[i];
                if ((k = control->Link) == 0) continue;
                link = net->Link[k];
                if ((j = control->Node) > 0)
                {
                    node = net->Node[j];
                    // control is based on tank level
                    if (j > net->Njuncs)
                    {
                        control->Grade = (double)(node->El + pr->convertToUnit<foot_t>(control->Grade));
                    }
                    // control is based on nodal pressure
                    else control->Grade = (double)(node->El) + (double)(control->Grade / pr->Ucf[PRESSURE]);
                }

                // Convert units on valve settings
                if (control->Setting != MISSING)
                {
                    switch (link->Type)
                    {
                    case PRV:
                    case PSV:
                    case PBV:
                        control->Setting /= pr->Ucf[PRESSURE];
                        break;
                    case FCV:
                        control->Setting /= pr->Ucf[FLOW];
                    default:
                        break;
                    }
                }
            }
        };
        int      inittanks(EN_Project const& pr)
            /*
            **---------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: initializes volumes in non-cylindrical tanks
            **---------------------------------------------------------------
            */
        {
            // Hmin, Hmax, and H0 are in LVL, not HEAD, at this point until the conversion in convertunits

            EN_Network const& net = pr->network;

            int i, j, n = 0;
            SCALER a;
            int errcode = 0, levelerr;
            char errmsg[MAXMSG + 1] = "";
            Ptank tank;
            Pcurve curve;

            for (j = 1; j <= net->Ntanks; j++)
            {
                tank = net->Tank[j];
                if (tank->Diameter == 0.0_ft) continue;  // Skip reservoirs

                // Check for valid lower/upper tank levels
                levelerr = 0;
                if (tank->InitialHead(pr->times.GetSimStartTime()) > tank->Hmax || tank->Hmin > tank->Hmax || tank->InitialHead(pr->times.GetSimStartTime()) < tank->Hmin) levelerr = 1;

                // Check that tank heights are within volume curve
                i = tank->Vcurve;
                if (i > 0)
                {
                    curve = net->Curve[i];
                    n = curve->Curve.GetNumValues() - 1;
                    if (tank->Hmin < pr->convertToUnit<foot_t>(curve->Curve.GetMinTime()) || tank->Hmax > pr->convertToUnit<foot_t>(curve->Curve.GetMaxTime()))
                    {
                        levelerr = 1;
                    }

                    else
                    {
                        // Find a "nominal" diameter for tank
                        auto a_sqft = pr->convertToUnit<square_foot_t>((curve->Curve.GetCurrentValue(curve->Curve.GetMaxTime()) - curve->Curve.GetCurrentValue(curve->Curve.GetMinTime())) / (curve->Curve.GetMaxTime() - curve->Curve.GetMinTime()));
                        tank->Diameter = units::math::sqrt(a_sqft * 4.0 / PI);
                    }
                }

                // Report error in levels if found
                if (levelerr)
                {
                    sprintf(pr->Msg, "Error 225: %s node %s", geterrmsg(225, errmsg),
                        tank->ID);
                    writeline(pr, pr->Msg);
                    errcode = 200;
                }
            }
            return errcode;
        };
        void     resetpumpflow(EN_Project const& pr, int i)
            /*
            **-------------------------------------------------------------------
            **  Input:   i = link index
            **  Output:  none
            **  Purpose: resets flow in a constant HP pump to its initial value.
            **-------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Ppump pump = net->Pump[findpump(net, i)];
            if (pump->Ptype == CONST_HP)
                pr->hydraul.LinkFlow[i] = pump->InitialFlow(pr->times.GetSimStartTime());
        };
        void     setlinksetting(EN_Project const& pr, int index, SCALER value, SCALER* s, SCALER* k)
            /*----------------------------------------------------------------
            **  Input:   index  = link index
            **           value  = pump speed or valve setting
            **           s      = pointer to link status
            **           k      = pointer to link setting
            **  Output:  none
            **  Purpose: sets pump speed or valve setting, adjusting link
            **           status and flow when necessary
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            Plink link = net->Link[index];
            LinkType t = link->Type;

            // For a pump, status is OPEN if speed > 0, CLOSED otherwise
            if (t == PUMP)
            {
                *k = value;
                if (value > (SCALER)0 && *s <= (double)CLOSED)
                {
                    // Check if a re-opened pump needs its flow reset
                    resetpumpflow(pr, index);
                    *s = (double)OPEN;
                }
                if (value == (SCALER)0 && *s > (double)CLOSED) *s = (double)CLOSED;
            }

            // For FCV, activate it
            else if (t == FCV)
            {
                *k = value;
                *s = (double)ACTIVE;
            }

            // Open closed control valve with fixed status (setting = MISSING)
            else
            {
                if (*k == MISSING && *s <= (double)CLOSED) *s = (double)OPEN;
                *k = value;
            }
        };
        void     setlinksetting(EN_Project const& pr, int index, SCALER value, StatusType* s, SCALER* k)
            /*----------------------------------------------------------------
            **  Input:   index  = link index
            **           value  = pump speed or valve setting
            **           s      = pointer to link status
            **           k      = pointer to link setting
            **  Output:  none
            **  Purpose: sets pump speed or valve setting, adjusting link
            **           status and flow when necessary
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            Plink link = net->Link[index];
            LinkType t = link->Type;

            // For a pump, status is OPEN if speed > 0, CLOSED otherwise
            if (t == PUMP)
            {
                *k = value;
                if (value > (SCALER)0 && *s <= CLOSED)
                {
                    // Check if a re-opened pump needs its flow reset
                    resetpumpflow(pr, index);
                    *s = OPEN;
                }
                if (value == (SCALER)0 && *s > CLOSED) *s = CLOSED;
            }

            // For FCV, activate it
            else if (t == FCV)
            {
                *k = value;
                *s = ACTIVE;
            }

            // Open closed control valve with fixed status (setting = MISSING)
            else
            {
                if (*k == MISSING && *s <= CLOSED) *s = OPEN;
                *k = value;
            }
        };
        int      checktime(EN_Project const& pr, Spremise* p)
            //------------------------------------------------------------
            //    Checks if condition on system time holds
            //------------------------------------------------------------
        {
            Times* time = &pr->times;
            Rules* rules = &pr->rules;

            char flag;
            units::time::second_t t1, t2, x;

            // Get start and end of rule evaluation time interval
            if (p->variable == r_TIME)
            {
                t1 = (u64)(rules->Time_t - time->GetCalibrationDateTime()); // rules->Time1;
                t2 = (u64)(time->GetCurrentRealHtime() - time->GetCalibrationDateTime()); // time->Htime;
            }
            else if (p->variable == r_CLOCKTIME)
            {
                t1 = (double)units::math::fmod((SCALER)(double)(rules->Time1 + time->Tstart), SECperDAY); // ((long)(u64)(rules->Time1 + time->Tstart)) % (long)SECperDAY;
                t2 = (double)units::math::fmod((SCALER)(double)(time->Htime + time->Tstart), SECperDAY); // ((long)(u64)(time->Htime + time->Tstart)) % (long)SECperDAY;
            }
            else return (0);

            // Test premise's time
            x = (double)p->value;
            switch (p->relop) {
                // For inequality, test against current time
            case LT:
                if (t2 >= x) return (0);
                break;
            case LE:
                if (t2 > x) return (0);
                break;
            case GT:
                if (t2 <= x) return (0);
                break;
            case GE:
                if (t2 < x) return (0);
                break;

                // For equality, test if within interval
            case EQ:
            case NE:
                flag = FALSE;
                if (t2 < t1) // E.g., 11:00 am to 1:00 am
                {
                    if (x >= t1 || x <= t2)
                        flag = TRUE;
                }
                else
                {
                    if (x >= t1 && x <= t2)
                        flag = TRUE;
                }
                if (p->relop == EQ && flag == FALSE) return (0);
                if (p->relop == NE && flag == TRUE)  return (0);
                break;
            }

            // If we get to here then premise was satisfied
            return 1;
        };
        int      checkstatus(EN_Project const& pr, Spremise* p)
            //------------------------------------------------------------
            //    Checks if condition on link status holds
            //------------------------------------------------------------
        {
            Hydraul* hyd = &pr->hydraul;

            char i;
            int j;

            switch (p->status)
            {
            case IS_OPEN:
            case IS_CLOSED:
            case IS_ACTIVE:
                i = hyd->LinkStatus[p->index];
                if (i <= CLOSED) j = IS_CLOSED;
                else if (i == ACTIVE) j = IS_ACTIVE;
                else                  j = IS_OPEN;
                if (j == p->status && p->relop == EQ) return 1;
                if (j != p->status && p->relop == NE) return 1;
            }
            return 0;
        };
        int      checkvalue(EN_Project const& pr, Spremise* p)
            //----------------------------------------------------------
            //    Checks if numerical condition on a variable is true.
            //    Uses tolerance of 0.001 when testing conditions.
            //----------------------------------------------------------
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int     i, j, v;
            SCALER  x,            // A variable's value
                tol = 1.e-3;  // Equality tolerance
            int    Njuncs = net->Njuncs;
            SCALER* Ucf = pr->Ucf;
            auto& NodeDemand = hyd->NodeDemand;
            auto& LinkFlow = hyd->LinkFlow;
            auto& LinkSetting = hyd->LinkSetting;
            auto& Node = net->Node;
            auto& Link = net->Link;
            auto& Tank = net->Tank;

            // Find the value being checked
            i = p->index;
            v = p->variable;
            switch (v)
            {
            case r_DEMAND:
                if (p->object == r_SYSTEM) x = (SCALER)(double)hyd->Dsystem * Ucf[DEMAND];
                else                       x = (SCALER)(double)NodeDemand[i] * Ucf[DEMAND];
                break;

            case r_HEAD:
            case r_GRADE:
                x = (double)(hyd->NodeHead[i] * Ucf[HEAD]);
                break;

            case r_PRESSURE:
                x = (double)(hyd->NodeHead[i] - Node[i]->El) * Ucf[PRESSURE];
                break;

            case r_LEVEL:
                x = (double)(hyd->NodeHead[i] - Node[i]->El) * Ucf[HEAD];
                break;

            case r_FLOW:
                x = (double)(ABS(LinkFlow[i]) * Ucf[FLOW]);
                break;

            case r_SETTING:
                if (LinkSetting[i] == MISSING) return 0;
                x = LinkSetting[i];
                switch (Link[i]->Type)
                {
                case PRV:
                case PSV:
                case PBV:
                    x = x * Ucf[PRESSURE];
                    break;
                case FCV:
                    x = x * Ucf[FLOW];
                    break;
                default:
                    break;
                }
                break;

            case r_FILLTIME:
                if (i <= Njuncs) return 0;
                j = i - Njuncs;
                if (Tank[j]->Diameter == 0.0_ft) return 0;
                if (NodeDemand[i] <= (cubic_foot_per_second_t)(double)(TINY)) return 0;
                x = (double)((Tank[j]->Vmax(pr) - hyd->TankVolume[j]) / NodeDemand[i]);
                break;

            case r_DRAINTIME:
                if (i <= Njuncs) return 0;
                j = i - Njuncs;
                if (Tank[j]->Diameter == 0.0_ft) return 0;
                if (NodeDemand[i] >= (cubic_foot_per_second_t)(double)(-TINY)) return 0;
                x = (double)((Tank[j]->Vmin(pr) - hyd->TankVolume[j]) / NodeDemand[i]);
                break;

            default:
                return 0;
            }

            // Compare value x against the premise
            switch (p->relop)
            {
            case EQ: if (ABS(x - p->value) > tol) return 0; break;
            case NE: if (ABS(x - p->value) < tol) return 0; break;
            case LT: if (x > p->value + tol) return 0;      break;
            case LE: if (x > p->value - tol) return 0;      break;
            case GT: if (x < p->value - tol) return 0;      break;
            case GE: if (x < p->value + tol) return 0;      break;
            }
            return 1;
        };
        int      onactionlist(EN_Project const& pr, int i, Saction* a)
            //-----------------------------------------------------------------------------
            //  Checks if action a from rule i can be added to the action list
            //-----------------------------------------------------------------------------
        {
            EN_Network const& net = pr->network;

            int link, i1;
            SactionList* actionItem;
            Saction* a1;

            // Search action list for link included in action a
            link = a->link;
            actionItem = pr->rules.ActionList;
            while (actionItem != NULL)
            {
                a1 = actionItem->action;
                i1 = actionItem->ruleIndex;

                // Link appears in list
                if (link == a1->link)
                {
                    // Replace its action with 'a' if rule i has higher priority
                    if (net->Rule[i]->priority > net->Rule[i1]->priority)
                    {
                        actionItem->action = a;
                        actionItem->ruleIndex = i;
                    }

                    // Return indicating that 'a' should not be added to action list
                    return 1;
                }
                actionItem = actionItem->next;
            }

            // Return indicating that it's ok to add 'a' to the action list
            return 0;
        };
        int      checkpremise(EN_Project const& pr, Spremise* p)
            //----------------------------------------------------------
            //    Checks if a particular premise is true
            //----------------------------------------------------------
        {
            if (p->variable == r_TIME ||
                p->variable == r_CLOCKTIME) return (checktime(pr, p));
            else if (p->status > IS_NUMBER) return (checkstatus(pr, p));
            else return (checkvalue(pr, p));
        };
        void     updateactionlist(EN_Project const& pr, int i, Saction* actions)
            //---------------------------------------------------
            //    Adds rule's actions to action list
            //--------------------------------------------------
        {
            Rules* rules = &pr->rules;

            SactionList* actionItem;
            Saction* a;

            // Iterate through each action of Rule i
            a = actions;
            while (a != NULL)
            {
                // Add action to list if its link not already on it
                if (!onactionlist(pr, i, a))
                {
                    actionItem = (SactionList*)malloc(sizeof(SactionList));
                    if (actionItem != NULL)
                    {
                        actionItem->action = a;
                        actionItem->ruleIndex = i;
                        actionItem->next = rules->ActionList;
                        rules->ActionList = actionItem;
                    }
                }
                a = a->next;
            }
        };
        int      evalpremises(EN_Project const& pr, int i)
            //----------------------------------------------------------
            //    Checks if premises to rule i are true
            //----------------------------------------------------------
        {
            EN_Network const& net = pr->network;

            int result;
            Spremise* p;

            result = TRUE;
            p = net->Rule[i]->Premises;
            while (p != NULL)
            {
                if (p->logop == r_OR)
                {
                    if (result == FALSE) result = checkpremise(pr, p);
                }
                else
                {
                    if (result == FALSE) return (FALSE);
                    result = checkpremise(pr, p);
                }
                p = p->next;
            }
            return result;
        };
        void    setlinkstatus(EN_Project const& pr, int index, char value, SCALER* s, SCALER* k)
            /*----------------------------------------------------------------
            **  Input:   index  = link index
            **           value  = 0 (CLOSED) or 1 (OPEN)
            **           s      = pointer to link status
            **           k      = pointer to link setting
            **  Output:  none
            **  Purpose: sets link status to OPEN or CLOSED
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            Plink link = net->Link[index];
            LinkType t = link->Type;

            // Status set to open
            if (value == 1)
            {
                // Adjust link setting for pumps & valves
                if (t == PUMP)
                {
                    *k = 1.0;
                    // Check if a re-opened pump needs its flow reset            
                    if (*s == (double)CLOSED) resetpumpflow(pr, index);
                }
                if (t > PUMP && t != GPV) *k = MISSING;
                *s = (double)OPEN;
            }

            // Status set to closed
            else if (value == 0)
            {
                // Adjust link setting for pumps & valves
                if (t == PUMP) *k = 0.0;
                if (t > PUMP && t != GPV) *k = MISSING;
                *s = (double)CLOSED;
            }
        };
        void    setlinkstatus(EN_Project const& pr, int index, char value, StatusType* s, SCALER* k)
            /*----------------------------------------------------------------
            **  Input:   index  = link index
            **           value  = 0 (CLOSED) or 1 (OPEN)
            **           s      = pointer to link status
            **           k      = pointer to link setting
            **  Output:  none
            **  Purpose: sets link status to OPEN or CLOSED
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            Plink link = net->Link[index];
            LinkType t = link->Type;

            // Status set to open
            if (value == 1)
            {
                // Adjust link setting for pumps & valves
                if (t == PUMP)
                {
                    *k = 1.0;
                    // Check if a re-opened pump needs its flow reset            
                    if (*s == CLOSED) resetpumpflow(pr, index);
                }
                if (t > PUMP && t != GPV) *k = MISSING;
                *s = OPEN;
            }

            // Status set to closed
            else if (value == 0)
            {
                // Adjust link setting for pumps & valves
                if (t == PUMP) *k = 0.0;
                if (t > PUMP && t != GPV) *k = MISSING;
                *s = CLOSED;
            }
        };
        void    writeruleaction(EN_Project const& pr, int k, char* ruleID)
            /*
            **--------------------------------------------------------------
            **   Input:   k  = link index
            **            *ruleID  = rule ID
            **   Output:  none
            **   Purpose: writes rule action taken to status report
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            auto& Link = net->Link;

            sprintf(pr->Msg, FMT63, clocktime(rpt->Atime, time->Htime),
                LinkTxt[Link[k]->Type], Link[k]->ID, ruleID);
            writeline(pr, pr->Msg);
        };
        int     takeactions(EN_Project const& pr)
            //-----------------------------------------------------------
            //    Implements actions on action list
            //-----------------------------------------------------------
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;
            Rules* rules = &pr->rules;

            char flag;
            int k, s, n;
            SCALER tol = 1.e-3, v, x;
            Saction* a;
            SactionList* actionItem;

            n = 0;
            actionItem = rules->ActionList;
            while (actionItem != NULL)
            {
                flag = FALSE;
                a = actionItem->action;
                k = a->link;
                s = hyd->LinkStatus[k];
                v = hyd->LinkSetting[k];
                x = a->setting;

                // Switch link from closed to open
                if (a->status == IS_OPEN && s <= CLOSED)
                {
                    setlinkstatus(pr, k, 1, &hyd->LinkStatus[k], &hyd->LinkSetting[k]);
                    flag = TRUE;
                }

                // Switch link from not closed to closed
                else if (a->status == IS_CLOSED && s > CLOSED)
                {
                    setlinkstatus(pr, k, 0, &hyd->LinkStatus[k], &hyd->LinkSetting[k]);
                    flag = TRUE;
                }

                // Change link's setting
                else if (x != MISSING)
                {
                    switch (net->Link[k]->Type)
                    {
                    case PRV:
                    case PSV:
                    case PBV:
                        x = x / pr->Ucf[PRESSURE];
                        break;
                    case FCV:
                        x = x / pr->Ucf[FLOW];
                        break;
                    default:
                        break;
                    }
                    if (ABS(x - v) > tol)
                    {
                        setlinksetting(pr, k, x, &hyd->LinkStatus[k],
                            &hyd->LinkSetting[k]);
                        flag = TRUE;
                    }
                }

                // Report rule action
                if (flag == TRUE)
                {
                    n++;
                    if (rpt->Statflag)
                    {
                        writeruleaction(pr, k, net->Rule[actionItem->ruleIndex]->label);
                    }
                }

                // Move to next action on list
                actionItem = actionItem->next;
            }
            return n;
        };
        void    clearactionlist(Rules* rules)
            //----------------------------------------------------------
            //    Clears memory used for action list
            //----------------------------------------------------------
        {
            SactionList* nextItem;
            SactionList* actionItem;
            actionItem = rules->ActionList;
            while (actionItem != NULL)
            {
                nextItem = actionItem->next;
                free(actionItem);
                actionItem = nextItem;
            }
        };
        int     checkrules(EN_Project const& pr, units::time::second_t dt)
            //-----------------------------------------------------
            //    Checks which rules should fire at current time.
            //-----------------------------------------------------
        {
            EN_Network const& net = pr->network;
            Times* time = &pr->times;
            Rules* rules = &pr->rules;

            int i;
            int actionCount = 0;    // Number of actions actually taken

            // Start of rule evaluation time interval
            rules->Time1 = time->Htime - dt + (units::time::second_t)1;
            rules->Time_t = time->GetCurrentRealHtime() - (u64)(dt + (units::time::second_t)1);

            // Iterate through each rule
            rules->ActionList = NULL;
            for (i = 1; i <= net->Nrules; i++)
            {
                // If premises true, add THEN clauses to action list
                if (evalpremises(pr, i) == TRUE)
                {
                    updateactionlist(pr, i, net->Rule[i]->ThenActions);
                }

                // If premises false, add ELSE actions to list
                else
                {
                    if (net->Rule[i]->ElseActions != NULL)
                    {
                        updateactionlist(pr, i, net->Rule[i]->ElseActions);
                    }
                }
            }

            // Execute actions then clear action list
            if (rules->ActionList != NULL) actionCount = takeactions(pr);
            clearactionlist(rules);
            return actionCount;
        };
        void    writepremise(Spremise* p, FILE* f, EN_Network net)
            //-----------------------------------------------------------------------------
            //  Write a rule's premise clause to an INP file.
            //-----------------------------------------------------------------------------
        {
            char s_obj[20];
            char s_id[MAXID + 1];
            char s_value[20];
            int  subtype;

            // Get the type name & ID of object referred to in the premise
            if (p->object == r_NODE)
            {
                subtype = net->Node[p->index]->Type;
                getobjtxt(r_NODE, subtype, s_obj);
                strcpy(s_id, net->Node[p->index]->ID);
            }
            else if (p->object == r_LINK)
            {
                subtype = net->Link[p->index]->Type;
                getobjtxt(r_LINK, subtype, s_obj);
                strcpy(s_id, net->Link[p->index]->ID);
            }
            else
            {
                strcpy(s_obj, "SYSTEM");
                strcpy(s_id, "");
            }

            // If premise has no value field, use its status field as a value
            if (p->value == MISSING) strcpy(s_value, Value[p->status]);

            // Otherwise get text of premise's value field
            else
            {
                // For time values convert from seconds to hr:min:sec
                switch (p->variable)
                {
                case r_CLOCKTIME:
                case r_DRAINTIME:
                case r_FILLTIME:
                case r_TIME:
                    gettimetxt(p->value, s_value);
                    break;
                default: sprintf(s_value, "%.4f", (double)p->value);
                }
            }

            // Write the premise clause to the file
            fprintf(f, "%s %s %s %s %s", s_obj, s_id, Varword[p->variable],
                Operator[p->relop], s_value);
        };
        void    writeaction(Saction* a, FILE* f, EN_Network net)
            //-----------------------------------------------------------------------------
            //  Write a rule's action clause to an INP file.
            //-----------------------------------------------------------------------------
        {
            char s_id[MAXID + 1];
            char s_obj[20];
            char s_var[20];
            char s_value[20];
            int subtype;

            subtype = net->Link[a->link]->Type;
            getobjtxt(r_LINK, subtype, s_obj);
            strcpy(s_id, net->Link[a->link]->ID);
            if (a->setting == MISSING)
            {
                strcpy(s_var, "STATUS");
                strcpy(s_value, Value[a->status]);
            }
            else
            {
                strcpy(s_var, "SETTING");
                sprintf(s_value, "%.4f", (double)a->setting);
            }
            fprintf(f, "%s %s %s = %s", s_obj, s_id, s_var, s_value);
        };
        int     writerule(EN_Project const& pr, FILE* f, int ruleIndex)
            //-----------------------------------------------------------------------------
            //  Write a rule to an INP file.
            //-----------------------------------------------------------------------------
        {
            EN_Network const& net = pr->network;

            Prule rule = net->Rule[ruleIndex];
            Spremise* p;
            Saction* a;

            // Write each premise clause to the file
            p = rule->Premises;
            fprintf(f, "\nIF   ");
            while (p != NULL)
            {
                writepremise(p, f, net);
                p = p->next;
                if (p) fprintf(f, "\n%-5s", Ruleword[p->logop]);
            }

            // Write each THEN action clause to the file
            a = rule->ThenActions;
            if (a) fprintf(f, "\nTHEN ");
            while (a != NULL)
            {
                writeaction(a, f, net);
                a = a->next;
                if (a) fprintf(f, "\nAND  ");
            }

            // Write each ELSE action clause to the file
            a = rule->ElseActions;
            if (a) fprintf(f, "\nELSE ");
            while (a != NULL)
            {
                writeaction(a, f, net);
                a = a->next;
                if (a) fprintf(f, "\nAND  ");
            }

            // Write the rule's priority to the file
            if (rule->priority > (SCALER)0) fprintf(f, "\nPRIORITY %f", (double)rule->priority);
            return 0;
        };
        void    writestatchange(EN_Project const& pr, int k, char s1, char s2)
            /*
            **--------------------------------------------------------------
            **   Input:   k  = link index
            **            s1 = old link status
            **            s2 = new link status
            **   Output:  none
            **   Purpose: writes change in link status to output report
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int j1, j2;
            SCALER setting;
            SCALER* Ucf = pr->Ucf;
            auto& LinkSetting = hyd->LinkSetting;
            auto& Link = net->Link;

            // We have a pump/valve setting change instead of a status change
            if (s1 == s2)
            {
                setting = LinkSetting[k];
                switch (Link[k]->Type)
                {
                case PRV:
                case PSV:
                case PBV:
                    setting *= Ucf[PRESSURE];
                    break;
                case FCV:
                    setting *= Ucf[FLOW];
                    break;
                default:
                    break;
                }
                sprintf(pr->Msg, FMT56, LinkTxt[Link[k]->Type], Link[k]->ID, setting);
                writeline(pr, pr->Msg);
                return;
            }

            // We have a status change  - write the old & new status types
            if (s1 == ACTIVE) j1 = ACTIVE;
            else if (s1 <= CLOSED) j1 = CLOSED;
            else                   j1 = OPEN;
            if (s2 == ACTIVE) j2 = ACTIVE;
            else if (s2 <= CLOSED) j2 = CLOSED;
            else                   j2 = OPEN;
            if (j1 != j2)
            {
                sprintf(pr->Msg, FMT57, LinkTxt[Link[k]->Type], Link[k]->ID, StatTxt[j1],
                    StatTxt[j2]);
                writeline(pr, pr->Msg);
            }
        };
        void    writerelerr(EN_Project const& pr, int iter, SCALER relerr)
            /*
            **-----------------------------------------------------------------
            **   Input:   iter   = current iteration of hydraulic solution
            **            relerr = current convergence error
            **   Output:  none
            **   Purpose: writes out convergence status of hydraulic solution
            **-----------------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            if (iter == 0)
            {
                sprintf(pr->Msg, FMT64, clocktime(rpt->Atime, time->Htime));
                writeline(pr, pr->Msg);
            }
            else
            {
                sprintf(pr->Msg, FMT65, iter, relerr);
                writeline(pr, pr->Msg);
            }
        };
        void    writehydstat(EN_Project const& pr, int iter, SCALER relerr)
            /*
            **--------------------------------------------------------------
            **   Input:   iter   = # iterations to find hydraulic solution
            **            relerr = convergence error in hydraulic solution
            **   Output:  none
            **   Purpose: writes hydraulic status report for solution found
            **            at current time period to report file
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            int i, n;
            char s1[MAXLINE + 1];
            char atime[13];
            StatusType newstat;
            auto& Tank = net->Tank;
            auto& Link = net->Link;

            // Display system status
            strcpy(atime, clocktime(rpt->Atime, time->Htime));
            if (iter > 0)
            {
                if (relerr <= hyd->Hacc) sprintf(s1, FMT58, atime, iter);
                else sprintf(s1, FMT59, atime, iter, relerr);
                writeline(pr, s1);
                if (hyd->DemandModel == PDA && hyd->DeficientNodes > 0)
                {
                    if (hyd->DeficientNodes == 1)
                        sprintf(s1, FMT69a, hyd->DemandReduction);
                    else
                        sprintf(s1, FMT69b, hyd->DeficientNodes, hyd->DemandReduction);
                    writeline(pr, s1);
                }
            }

            // Display status changes for tanks:
            //   D[n] is net inflow to tank at node n;
            //   old tank status is stored in OldStatus[]
            //   at indexes Nlinks+1 to Nlinks+Ntanks.
            for (i = 1; i <= net->Ntanks; i++)
            {
                n = net->Tank[i]->Node;
                auto& NodeDemand = hyd->NodeDemand;
                if (ABS(NodeDemand[n]) < 0.001_cfs) newstat = CLOSED;
                else if (NodeDemand[n] < 0.0_cfs)   newstat = EMPTYING;
                else if (NodeDemand[n] > 0.0_cfs)
                {
                    if (Tank[i]->Diameter != 0.0_ft && ABS(hyd->NodeHead[n] - Tank[i]->Hmax) < 0.001_ft)
                        newstat = OVERFLOWING;
                    else newstat = FILLING;
                }
                else newstat = hyd->OldStatus[net->Nlinks + i];
                if (newstat != hyd->OldStatus[net->Nlinks + i])
                {
                    if (Tank[i]->Diameter != 0.0_ft)
                    {
                        snprintf(s1, MAXLINE, FMT50, atime, net->Node[n]->ID, StatTxt[newstat], (hyd->NodeHead[n] - net->Node[n]->El) * pr->Ucf[HEAD], rpt->Field[HEAD].Units);
                    }
                    else
                    {
                        snprintf(s1, MAXLINE, FMT51, atime, net->Node[n]->ID, StatTxt[newstat]);
                    }
                    writeline(pr, s1);
                    hyd->OldStatus[net->Nlinks + i] = newstat;
                }
            }

            // Display status changes for links
            for (i = 1; i <= net->Nlinks; i++)
            {
                if (hyd->LinkStatus[i] != hyd->OldStatus[i])
                {
                    if (time->Htime == 0_s)
                    {
                        sprintf(s1, FMT52, atime, LinkTxt[(int)net->Link[i]->Type],
                            net->Link[i]->ID, StatTxt[(int)hyd->LinkStatus[i]]);
                    }
                    else sprintf(s1, FMT53, atime, LinkTxt[Link[i]->Type], net->Link[i]->ID,
                        StatTxt[hyd->OldStatus[i]], StatTxt[hyd->LinkStatus[i]]);
                    writeline(pr, s1);
                    hyd->OldStatus[i] = hyd->LinkStatus[i];
                }
            }
            writeline(pr, (char*)" ");
        };
        void    marknodes(EN_Project const& pr, int m, int* nodelist, char* marked)
            /*
            **----------------------------------------------------------------
            **   Input:   m = number of source nodes
            **            nodelist[] = list of nodes to be traced from
            **            marked[]   = TRUE if node connected to source
            **   Output:  None.
            **   Purpose: Marks all junction nodes connected to tanks.
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int i, j, k, n;
            Padjlist alink;

            // Scan each successive entry of node list
            n = 1;
            while (n <= m)
            {
                // Scan all nodes connected to current node
                i = nodelist[n];
                for (alink = net->Adjlist[i]; alink != nullptr; alink = alink->next)
                {
                    // Get indexes of connecting link and node
                    k = alink->link;
                    j = alink->node;
                    if (marked[j]) continue;

                    // Check if valve connection is in correct direction
                    switch (net->Link[k]->Type)
                    {
                    case CVPIPE:
                    case PRV:
                    case PSV:
                        if (j == net->Link[k]->N1) continue;
                        break;
                    default:
                        break;
                    }

                    // Mark connection node if link not closed
                    if (hyd->LinkStatus[k] > CLOSED)
                    {
                        marked[j] = 1;
                        m++;
                        nodelist[m] = j;
                    }
                }
                n++;
            }
        };
        void    getclosedlink(EN_Project const& pr, int i, char* marked)
            /*
            **----------------------------------------------------------------
            **   Input:   i = junction index
            **            marked[] = marks nodes already examined
            **   Output:  None.
            **   Purpose: Determines if a closed link connects to junction i.
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            int j, k;
            Padjlist alink;

            marked[i] = 2;
            for (alink = net->Adjlist[i]; alink != nullptr; alink = alink->next)
            {
                k = alink->link;
                j = alink->node;
                if (marked[j] == 2) continue;
                if (marked[j] == 1)
                {
                    sprintf(pr->Msg, WARN03c, net->Link[k]->ID);
                    writeline(pr, pr->Msg);
                    return;
                }
                else getclosedlink(pr, j, marked);
            }
        };
        int     disconnected(EN_Project const& pr)
            /*
            **-------------------------------------------------------------------
            **   Input:   None
            **   Output:  Returns number of disconnected nodes
            **   Purpose: Tests current hydraulic solution to see if any closed
            **            links have caused the network to become disconnected.
            **-------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            int i, j;
            int count, mcount;
            int errcode = 0;
            int* nodelist;
            char* marked;
            Pnode node;

            // Allocate memory for node list & marked list
            nodelist = (int*)calloc(net->Nnodes + 1, sizeof(int));
            marked = (char*)calloc(net->Nnodes + 1, sizeof(char));
            ERRCODE(MEMCHECK(nodelist));
            ERRCODE(MEMCHECK(marked));

            // If allocation fails return with 0 nodes disconnected
            if (errcode)
            {
                free(nodelist);
                free(marked);
                return (0);
            }

            // Place tanks on node list and marked list
            for (i = 1; i <= net->Ntanks; i++)
            {
                j = net->Njuncs + i;
                nodelist[i] = j;
                marked[j] = 1;
            }

            // Place junctions with negative demands on the lists
            mcount = net->Ntanks;
            for (i = 1; i <= net->Njuncs; i++)
            {
                if (hyd->NodeDemand[i] < 0.0_cfs)
                {
                    mcount++;
                    nodelist[mcount] = i;
                    marked[i] = 1;
                }
            }

            // Mark all nodes that can be connected to tanks
            // and count number of nodes remaining unmarked
            marknodes(pr, mcount, nodelist, marked);
            j = 0;
            count = 0;
            for (i = 1; i <= net->Njuncs; i++)
            {
                node = net->Node[i];
                if (!marked[i] && hyd->NodeDemand[i] != 0.0_cfs)
                {
                    count++;
                    if (count <= MAXCOUNT && rpt->Messageflag)
                    {
                        sprintf(pr->Msg, WARN03a, node->ID,
                            clocktime(rpt->Atime, time->Htime));
                        writeline(pr, pr->Msg);
                    }
                    j = i; // Last unmarked node
                }
            }

            // Report number of unmarked nodes and find closed link
            // on path from node j back to a tank
            if (count > 0 && rpt->Messageflag)
            {
                if (count > MAXCOUNT)
                {
                    sprintf(pr->Msg, WARN03b, count - MAXCOUNT,
                        clocktime(rpt->Atime, time->Htime));
                    writeline(pr, pr->Msg);
                }
                getclosedlink(pr, j, marked);
            }

            // Free allocated memory
            free(nodelist);
            free(marked);
            return count;
        };
        void    writehyderr(EN_Project const& pr, int errnode)
            /*
            **-----------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: outputs status & checks connectivity when
            **            network hydraulic equations cannot be solved.
            **-----------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            auto& Node = net->Node;

            if (rpt->Messageflag)
            {
                sprintf(pr->Msg, FMT62, clocktime(rpt->Atime, time->Htime),
                    Node[errnode]->ID);
                writeline(pr, pr->Msg);
            }
            writehydstat(pr, 0, 0);
            disconnected(pr);
        };
        int     writehydwarn(EN_Project const& pr, int iter, SCALER relerr)
            /*
            **--------------------------------------------------------------
            **   Input:   iter = # iterations to find hydraulic solution
            **   Output:  warning flag code
            **   Purpose: writes hydraulic warning message to report file
            **
            **   Note: Warning conditions checked in following order:
            **         1. System balanced but unstable
            **         2. Negative pressures
            **         3. FCV cannot supply flow or PRV/PSV cannot maintain pressure
            **         4. Pump out of range
            **         5. Network disconnected
            **         6. System unbalanced
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            int i, j;
            char flag = 0;
            int s;
            Pnode node;
            Plink link;
            Ppump pump;

            // Check if system unstable
            if (iter > hyd->MaxIter && relerr <= hyd->Hacc)
            {
                sprintf(pr->Msg, WARN02, clocktime(rpt->Atime, time->Htime));
                if (rpt->Messageflag) writeline(pr, pr->Msg);
                flag = 2;
            }

            // Check for pressure deficient nodes
            if (hyd->DemandModel == DDA)
            {
                hyd->DeficientNodes = 0;
                for (i = 1; i <= net->Njuncs; i++)
                {
                    node = net->Node[i];
                    if (hyd->NodeHead[i] < node->El && hyd->NodeDemand[i] > 0.0_cfs)
                        hyd->DeficientNodes++;
                }
                if (hyd->DeficientNodes > 0)
                {
                    if (rpt->Messageflag)
                    {
                        sprintf(pr->Msg, WARN06, clocktime(rpt->Atime, time->Htime));
                        writeline(pr, pr->Msg);
                    }
                    flag = 6;
                }
            }

            // Check for abnormal valve condition
            for (i = 1; i <= net->Nvalves; i++)
            {
                j = net->Valve[i]->Link;
                link = net->Link[j];
                if (hyd->LinkStatus[j] >= XFCV)
                {
                    if (rpt->Messageflag)
                    {
                        sprintf(pr->Msg, WARN05, LinkTxt[link->Type], link->ID,
                            StatTxt[hyd->LinkStatus[j]],
                            clocktime(rpt->Atime, time->Htime));
                        writeline(pr, pr->Msg);
                    }
                    flag = 5;
                }
            }

            // Check for abnormal pump condition
            for (i = 1; i <= net->Npumps; i++)
            {
                pump = net->Pump[i];
                j = pump->Link;
                s = hyd->LinkStatus[j];
                if (hyd->LinkStatus[j] >= OPEN)
                {
                    if (hyd->LinkFlow[j] > hyd->LinkSetting[j] * pump->Qmax) s = XFLOW;
                    if (hyd->LinkFlow[j] < 0.0_cfs) s = XHEAD;
                }
                if (s == XHEAD || s == XFLOW)
                {
                    if (rpt->Messageflag)
                    {
                        sprintf(pr->Msg, WARN04, net->Link[j]->ID, StatTxt[s],
                            clocktime(rpt->Atime, time->Htime));
                        writeline(pr, pr->Msg);
                    }
                    flag = 4;
                }
            }

            // Check if system is unbalanced
            if (iter > hyd->MaxIter && relerr > hyd->Hacc)
            {
                if (rpt->Messageflag)
                {
                    sprintf(pr->Msg, WARN01, clocktime(rpt->Atime, time->Htime));
                    if (hyd->ExtraIter == -1) strcat(pr->Msg, t_HALTED);
                    writeline(pr, pr->Msg);
                }
                flag = 1;
            }

            // Check for disconnected network & update project's warning flag
            if (flag > 0)
            {
                disconnected(pr);
                pr->Warnflag = flag;
                if (rpt->Messageflag) writeline(pr, (char*)" ");
            }
            return flag;
        };
        int     getnodetype(EN_Network net, int i)
            /*
            **---------------------------------------------------------
            **  Determines type of node with index i
            **  (junction = 0, reservoir = 1, tank = 2).
            **---------------------------------------------------------
            */
        {
            if (i <= net->Njuncs) return 0;
            if (net->Tank[i - net->Njuncs]->Diameter == 0.0_ft) return 1;
            return 2;
        };
        void    writecontrolaction(EN_Project const& pr, int k, int i)
            /*
            ----------------------------------------------------------------
            **   Input:   k  = link index
            **            i  = control index
            **   Output:  none
            **   Purpose: writes control action taken to status report
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            int n;
            auto& Node = net->Node;
            auto& Link = net->Link;
            auto& Control = net->Control;

            switch (Control[i]->Type)
            {
            case LOWLEVEL:
            case HILEVEL:
                n = Control[i]->Node;
                sprintf(pr->Msg, FMT54, clocktime(rpt->Atime, time->Htime),
                    LinkTxt[Link[k]->Type], Link[k]->ID,
                    NodeTxt[getnodetype(net, n)], Node[n]->ID);
                break;

            case TIMER:
            case TIMEOFDAY:
                sprintf(pr->Msg, FMT55, clocktime(rpt->Atime, time->Htime),
                    LinkTxt[Link[k]->Type], Link[k]->ID);
                break;
            default:
                return;
            }
            writeline(pr, pr->Msg);
        };
        void    writemassbalance(EN_Project const& pr)
            /*
            **-------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: writes water quality mass balance ratio
            **            (Outflow + Final Storage) / Inflow + Initial Storage)
            **            to report file.
            **-------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;

            char s1[MAXMSG + 1];
            char* units[] = { (char*)"", (char*)" (mg)", (char*)" (ug)", (char*)" (hrs)" };
            int  kunits = 0;

            if (qual->Qualflag == TRACE_QUAL) kunits = 1;
            else if (qual->Qualflag == AGE)   kunits = 3;
            else
            {
                if (match(qual->ChemUnits, "mg")) kunits = 1;
                else if (match(qual->ChemUnits, "ug")) kunits = 2;
            }

            snprintf(s1, MAXMSG, "Water Quality Mass Balance%s", units[kunits]);
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "================================");
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "Initial Mass:      %12.5e", (double)qual->MassBalance.initial);
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "Mass Inflow:       %12.5e", (double)qual->MassBalance.inflow);
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "Mass Outflow:      %12.5e", (double)qual->MassBalance.outflow);
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "Mass Reacted:      %12.5e", (double)qual->MassBalance.reacted);
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "Final Mass:        %12.5e", (double)qual->MassBalance.final);
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "Mass Ratio:         %-.5f", (double)qual->MassBalance.ratio);
            writeline(pr, s1);
            snprintf(s1, MAXMSG, "================================\n");
            writeline(pr, s1);
        };
        void    DWpipecoeff(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  none
            **   Purpose: computes pipe head loss coeffs. for Darcy-Weisbach
            **            formula.
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Plink link = pr->network->Link[k];

            SCALER q = (double)ABS(hyd->LinkFlow[k]);
            const auto& r = link->R_FlowResistance;                         // Resistance coeff.
            const auto& ml = link->Km;                       // Minor loss coeff.
            SCALER e = (double)(link->Kc / link->Diam);           // Relative roughness
            SCALER s = (double)(hyd->Viscos * link->Diam);        // Viscosity / diameter = ft2/s * ft = cfs
            SCALER hloss, hgrad, f, dfdq;

            // Compute head loss and its derivative
            // ... use Hagen-Poiseuille formula for laminar flow (Re <= 2000)
            if (q <= A2 * s)
            {
                hloss = (double)hyd->LinkFlow[k] * ((16.0 * PI * s * r) + ml * q);
                hgrad = (16.0 * PI * s * r) + 2.0 * ml * q;
            }

            // ... otherwise use Darcy-Weisbach formula with friction factor
            else
            {
                dfdq = 0.0;
                f = frictionFactor(q, e, s, &dfdq);
                hloss = (f * r + ml) * q * (double)hyd->LinkFlow[k];
                hgrad = (2.0 * (f * r + ml) * q) + (dfdq * r * q * q);
            }

            // Compute P and Y coefficients
            hyd->P[k] = (double)(1.0 / hgrad);
            hyd->Y[k] = (double)(hloss / hgrad);
        };
        void    pipecoeff(EN_Project const& pr, int k, cweeList<Plink> const& link)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  none
            **   Purpose:  computes P & Y coefficients for pipe k.
            **
            **    P = inverse head loss gradient = 1/hgrad
            **    Y = flow correction term = hloss / hgrad
            **--------------------------------------------------------------
            */

        {
            Hydraul* hyd = &pr->hydraul;

            foot_t
                hloss;     // Head loss
            ft_per_cfs_t
                hgrad;     // Head loss gradient
            cubic_foot_per_second_t
                q;         // Abs. value of flow

            // For closed pipe use headloss formula: hloss = CBIG*q
            if (hyd->LinkStatus[k] <= CLOSED)
            {
                hyd->P[k] = (double)(1.0 / CBIG);
                hyd->Y[k] = hyd->LinkFlow[k];
                return;
            }

            // Use custom function for Darcy-Weisbach formula
            if (hyd->Formflag == DW)
            {
                DWpipecoeff(pr, k);
                return;
            }

            q = ABS(hyd->LinkFlow[k]);
            const auto& ml = link[k]->Km;

            // Friction head loss gradient
            hgrad = hyd->Hexp();
            hgrad *= link[k]->R_FlowResistance();
            hgrad *= std::pow(q(), hyd->Hexp() - 1.0);

            // Friction head loss:
            // ... use linear function for very small gradient
            if ((double)hgrad < (double)hyd->RQtol)
            {
                hgrad = (double)hyd->RQtol;
                hloss = hgrad * q;
            }
            // ... otherwise use original formula
            else hloss = hgrad * q / hyd->Hexp;

            // Contribution of minor head loss
            if (ml > 0.0)
            {
                hloss += (foot_t)(double)(ml * q * q);
                hgrad += (ft_per_cfs_t)(double)(2.0 * ml * q);
            }

            // Adjust head loss sign for flow direction
            hloss *= SGN(hyd->LinkFlow[k]);

            // P and Y coeffs.
            hyd->P[k] = 1.0 / hgrad;
            hyd->Y[k] = hloss / hgrad;
        };
        void    pumpcoeff(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  none
            **   Purpose: computes P & Y coeffs. for pump in link k
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            int    p;                // Pump index

            SCALER h0,               // Shutoff head
                q,                // Abs. value of flow
                r,                // Flow resistance coeff.
                n,                // Flow exponent coeff.
                setting,          // Pump speed setting
                hloss,            // Head loss across pump
                hgrad;            // Head loss gradient
            Ppump pump;

            // Use high resistance pipe if pump closed or cannot deliver head
            setting = hyd->LinkSetting[k];
            if (hyd->LinkStatus[k] <= CLOSED || setting == 0.0)
            {
                hyd->P[k] = (double)(1.0 / CBIG);
                hyd->Y[k] = hyd->LinkFlow[k];
            }
            else {
                // Obtain reference to pump object
                q = (double)ABS(hyd->LinkFlow[k]);
                p = findpump(pr->network, k);
                pump = pr->network->Pump[p];

                // If no pump curve treat pump as an open valve
                if (pump->Ptype == NOCURVE)
                {
                    hyd->P[k] = (double)(1.0 / CSMALL);
                    hyd->Y[k] = hyd->LinkFlow[k];
                }
                else {

                    // Get pump curve coefficients for custom pump curve (Other pump types have pre-determined coeffs.)
                    if (pump->Ptype == CUSTOM)
                    {
                        // Find intercept (h0) & slope (r) of pump curve line segment which contains speed-adjusted flow.
                        curvecoeff(pr, pump->Hcurve, q / setting, &h0, &r);

                        // Determine head loss coefficients (negative sign converts from pump curve's head gain to head loss)
                        //pump->H0 = (double)(-h0); // already converted in curve coeff!
                        //pump->R = -r;
                        //pump->N = 1.0;

                        // Compute head loss and its gradient (with speed adjustment)
                        hgrad = (double)(-r * setting);
                        hloss = (double)(-h0 * SQR(setting) + hgrad * (double)hyd->LinkFlow[k]);
                    }
                    else
                    {
                        // Adjust head loss coefficients for pump speed
                        h0 = (double)(SQR(setting) * pump->H0);
                        n = pump->N;
                        if (ABS(n - 1.0) < TINY) n = 1.0;
                        r = pump->R * ::epanet::pow(setting, 2.0 - n);

                        // Constant HP pump
                        if (pump->Ptype == CONST_HP)
                        {
                            // ... compute pump curve's gradient
                            hgrad = -r / q / q;
                            // ... use linear curve if gradient too large or too small
                            if (hgrad > CBIG)
                            {
                                hgrad = CBIG;
                                hloss = -hgrad * (double)hyd->LinkFlow[k];
                            }
                            else if (hgrad < hyd->RQtol)
                            {
                                hgrad = hyd->RQtol;
                                hloss = -hgrad * (double)hyd->LinkFlow[k];
                            }
                            // ... otherwise compute head loss from pump curve
                            else
                            {
                                hloss = r / (double)hyd->LinkFlow[k];
                            }
                        }

                        // Compute head loss and its gradient
                        // ... pump curve is nonlinear
                        else if (n != 1.0)
                        {
                            // ... compute pump curve's gradient
                            hgrad = n * r * ::epanet::pow(q, n - 1.0);
                            // ... use linear pump curve if gradient too small
                            if (hgrad < hyd->RQtol)
                            {
                                hgrad = hyd->RQtol;
                                hloss = h0 + hgrad * (double)hyd->LinkFlow[k];
                            }
                            // ... otherwise compute head loss from pump curve
                            else hloss = h0 + hgrad * (double)hyd->LinkFlow[k] / n;
                        }
                        // ... pump curve is linear
                        else
                        {
                            hgrad = r;
                            hloss = h0 + hgrad * (double)hyd->LinkFlow[k];
                        }
                    }

                    // P and Y coeffs.
                    hyd->P[k] = (double)(1.0 / hgrad);
                    hyd->Y[k] = (double)(hloss / hgrad);
                }
            }

            if (!pump) pump = pr->network->Pump[findpump(pr->network, k)];
        };
        void    curvecoeff(EN_Project const& pr, int i, SCALER q, SCALER* h0, SCALER* r)
            /*
            **-------------------------------------------------------------------
            **   Input:   i   = curve index
            **            q   = flow rate
            **   Output:  *h0  = head at zero flow (y-intercept)
            **            *r  = dHead/dFlow (slope)
            **   Purpose: computes intercept and slope of head v. flow curve
            **            at current flow.
            **-------------------------------------------------------------------
            */
        {
            int   k1, k2, npts;
            Pcurve curve;
            curve = pr->network->Curve[i];

            // Remember that curve is stored in untransformed units
            q *= pr->Ucf[FLOW];

            npts = curve->Curve.GetNumValues();

            // Find linear segment of curve that brackets flow q
            k2 = 0;
            k2 = curve->Curve.GetLargestSmallerOrEqualTime(q);
            // while (k2 < npts && curve->Curve.GetTimeAtIndex(k2) < q) k2++;
            if (k2 == 0) k2++;
            else if (k2 == npts)  k2--;
            k1 = k2 - 1;

            *r = (curve->Curve.GetValueAtIndex(k2) - curve->Curve.GetValueAtIndex(k1)) / (curve->Curve.GetTimeAtIndex(k2) - curve->Curve.GetTimeAtIndex(k1));
            *h0 = curve->Curve.GetValueAtIndex(k1) - (*r) * curve->Curve.GetTimeAtIndex(k1);

            // Convert units
            *h0 = (*h0) / pr->Ucf[HEAD];
            *r = (*r) * pr->Ucf[FLOW] / pr->Ucf[HEAD];
        };
        void    gpvcoeff(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  none
            **   Purpose: computes P & Y coeffs. for general purpose valve
            **--------------------------------------------------------------
            */
        {
            int    i;
            SCALER h0,        // Intercept of head loss curve segment
                r,         // Slope of head loss curve segment
                q;         // Abs. value of flow

            Hydraul* hyd = &pr->hydraul;

            // Treat as a pipe if valve closed
            if (hyd->LinkStatus[k] == CLOSED) valvecoeff(pr, k);

            // Otherwise utilize segment of head loss curve
            // bracketing current flow (curve index is stored
            // in valve's setting)
            else
            {
                // Index of valve's head loss curve
                i = (int)ROUND(hyd->LinkSetting[k]);

                // Adjusted flow rate
                q = (double)ABS(hyd->LinkFlow[k]);
                q = fMAX(q, TINY);

                // Intercept and slope of curve segment containing q
                curvecoeff(pr, i, q, &h0, &r);
                r = fMAX(r, TINY);

                // Resulting P and Y coeffs.
                hyd->P[k] = (double)(1.0 / r);
                hyd->Y[k] = (double)((h0 / r + q) * SGN(hyd->LinkFlow[k]));
            }
        };
        void    pbvcoeff(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  none
            **   Purpose: computes P & Y coeffs. for pressure breaker valve
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Plink link = pr->network->Link[k];

            // If valve fixed OPEN or CLOSED then treat as a pipe
            if (hyd->LinkSetting[k] == MISSING || hyd->LinkSetting[k] == 0.0)
            {
                valvecoeff(pr, k);
            }

            // If valve is active
            else
            {
                // Treat as a pipe if minor loss > valve setting
                if (link->Km * (double)SQR(hyd->LinkFlow[k]) > hyd->LinkSetting[k])
                {
                    valvecoeff(pr, k);
                }
                // Otherwise force headloss across valve to be equal to setting
                else
                {
                    hyd->P[k] = (double)CBIG;
                    hyd->Y[k] = (double)(hyd->LinkSetting[k] * CBIG);
                }
            }
        };
        void    tcvcoeff(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  none
            **   Purpose: computes P & Y coeffs. for throttle control valve
            **--------------------------------------------------------------
            */
        {
            SCALER km;
            Hydraul* hyd = &pr->hydraul;
            Plink link = pr->network->Link[k];

            // Save original loss coeff. for open valve
            km = link->Km;

            // If valve not fixed OPEN or CLOSED, compute its loss coeff.
            if (hyd->LinkSetting[k] != MISSING)
            {
                link->Km = (double)(0.02517 * hyd->LinkSetting[k] / (SQR(link->Diam) * SQR(link->Diam)));
            }

            // Then apply usual valve formula
            valvecoeff(pr, k);

            // Restore original loss coeff.
            link->Km = km;
        };
        void    prvcoeff(EN_Project const& pr, int k, int n1, int n2)
            /*
            **--------------------------------------------------------------
            **   Input:   k    = link index
            **            n1   = upstream node of valve
            **            n2   = downstream node of valve
            **   Output:  none
            **   Purpose: computes solution matrix coeffs. for pressure
            **            reducing valves
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            int   i, j;                        // Rows of solution matrix
            SCALER hset;                       // Valve head setting

            i = sm->Row[n1];                  // Matrix rows of nodes
            j = sm->Row[n2];
            hset = (double)(pr->network->Node[n2]->El + (foot_t)(double)hyd->LinkSetting[k]);        // Valve setting

            if (hyd->LinkStatus[k] == ACTIVE)
            {

                // Set coeffs. to force head at downstream
                // node equal to valve setting & force flow
                // to equal to flow excess at downstream node.

                hyd->P[k] = 0.0;
                hyd->Y[k] = hyd->LinkFlow[k] + hyd->Xflow[n2];   // Force flow balance
                sm->F[j] += (cubic_foot_per_second_t)(double)(hset * CBIG);                        // Force head = hset
                sm->Aii[j] += (cfs_p_ft_t)(double)CBIG;                               // at downstream node
                if (hyd->Xflow[n2] < 0.0_cfs)
                {
                    sm->F[i] += hyd->Xflow[n2];
                }
                return;
            }

            // For OPEN, CLOSED, or XPRESSURE valve compute matrix coeffs. using the valvecoeff() function.
            valvecoeff(pr, k);
            sm->Aij[sm->Ndx[k]] -= hyd->P[k];
            sm->Aii[i] += hyd->P[k];
            sm->Aii[j] += hyd->P[k];
            sm->F[i] += (hyd->Y[k] - hyd->LinkFlow[k]);
            sm->F[j] -= (hyd->Y[k] - hyd->LinkFlow[k]);
        };
        void    psvcoeff(EN_Project const& pr, int k, int n1, int n2)
            /*
            **--------------------------------------------------------------
            **   Input:   k    = link index
            **            n1   = upstream node of valve
            **            n2   = downstream node of valve
            **   Output:  none
            **   Purpose: computes solution matrix coeffs. for pressure
            **            sustaining valve
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            int   i, j;                        // Rows of solution matrix
            SCALER hset;                       // Valve head setting

            i = sm->Row[n1];                   // Matrix rows of nodes
            j = sm->Row[n2];
            hset = (double)(pr->network->Node[n1]->El + (foot_t)(double)hyd->LinkSetting[k]);        // Valve setting

            if (hyd->LinkStatus[k] == ACTIVE)
            {
                // Set coeffs. to force head at upstream
                // node equal to valve setting & force flow
                // equal to flow excess at upstream node.

                hyd->P[k] = 0.0;
                hyd->Y[k] = hyd->LinkFlow[k] - hyd->Xflow[n1];   // Force flow balance
                sm->F[i] += (cubic_foot_per_second_t)(double)(hset * CBIG);                        // Force head = hset
                sm->Aii[i] += (cfs_p_ft_t)(double)CBIG;                               // at upstream node
                if (hyd->Xflow[n1] > 0.0_cfs)
                {
                    sm->F[j] += hyd->Xflow[n1];
                }
                return;
            }

            // For OPEN, CLOSED, or XPRESSURE valve
            // compute matrix coeffs. using the valvecoeff() function.

            valvecoeff(pr, k);
            sm->Aij[sm->Ndx[k]] -= hyd->P[k];
            sm->Aii[i] += hyd->P[k];
            sm->Aii[j] += hyd->P[k];
            sm->F[i] += hyd->Y[k] - hyd->LinkFlow[k];
            sm->F[j] -= hyd->Y[k] - hyd->LinkFlow[k];
        };
        void    fcvcoeff(EN_Project const& pr, int k, int n1, int n2)
            /*
            **--------------------------------------------------------------
            **   Input:   k    = link index
            **            n1   = upstream node of valve
            **            n2   = downstream node of valve
            **   Output:  none
            **   Purpose: computes solution matrix coeffs. for flow control
            **            valve
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            int   i, j;                   // Rows in solution matrix
            SCALER q;                     // Valve flow setting

            q = hyd->LinkSetting[k];
            i = sm->Row[n1];
            j = sm->Row[n2];

            // If valve active, break network at valve and treat
            // flow setting as external demand at upstream node
            // and external supply at downstream node.

            if (hyd->LinkStatus[k] == ACTIVE)
            {
                hyd->Xflow[n1] -= (cubic_foot_per_second_t)(double)q;
                hyd->Xflow[n2] += (cubic_foot_per_second_t)(double)q;
                hyd->Y[k] = (hyd->LinkFlow[k] - (cubic_foot_per_second_t)(double)q);
                sm->F[i] -= (cubic_foot_per_second_t)(double)q;
                sm->F[j] += (cubic_foot_per_second_t)(double)q;
                hyd->P[k] = (double)(1.0 / CBIG);
                sm->Aij[sm->Ndx[k]] -= hyd->P[k];
                sm->Aii[i] += hyd->P[k];
                sm->Aii[j] += hyd->P[k];
            }

            // Otherwise treat valve as an open pipe

            else
            {
                valvecoeff(pr, k);
                sm->Aij[sm->Ndx[k]] -= hyd->P[k];
                sm->Aii[i] += hyd->P[k];
                sm->Aii[j] += hyd->P[k];
                sm->F[i] += hyd->Y[k] - hyd->LinkFlow[k];
                sm->F[j] -= hyd->Y[k] - hyd->LinkFlow[k];
            }
        };
        void    valvecoeff(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k    = link index
            **   Output:  none
            **   Purpose: computes solution matrix coeffs. for a completely
            **            open, closed, or throttled control valve.
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Plink link = pr->network->Link[k];

            SCALER flow, q, hloss, hgrad;

            flow = (double)hyd->LinkFlow[k];

            // Valve is closed. Use a very small matrix coeff.
            if (hyd->LinkStatus[k] <= CLOSED)
            {
                hyd->P[k] = (double)(1.0 / CBIG);
                hyd->Y[k] = hyd->LinkFlow[k];
                return;
            }

            // Account for any minor headloss through the valve
            if (link->Km > 0.0)
            {
                q = ABS(flow);
                hgrad = 2.0 * link->Km * q;

                // Guard against too small a head loss gradient
                if (hgrad < hyd->RQtol)
                {
                    hgrad = hyd->RQtol;
                    hloss = flow * hgrad;
                }
                else hloss = flow * hgrad / 2.0;

                // P and Y coeffs.
                hyd->P[k] = (double)(1.0 / hgrad);
                hyd->Y[k] = (double)(hloss / hgrad);
            }

            // If no minor loss coeff. specified use a
            // low resistance linear head loss relation
            else
            {
                hyd->P[k] = (double)(1.0 / CSMALL);
                hyd->Y[k] = (double)flow;
            }
        };
        void    headlosscoeffs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: computes coefficients P (1 / head loss gradient)
            **            and Y (head loss / gradient) for all links.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            auto& link = net->Link;
            for (int k = 1; k <= net->Nlinks; k++) {
                switch (link[k]->Type) {
                case CVPIPE:
                case PIPE:
                    pipecoeff(pr, k, link);
                    break;
                case PUMP:
                    pumpcoeff(pr, k);
                    break;
                case PBV:
                    pbvcoeff(pr, k);
                    break;
                case TCV:
                    tcvcoeff(pr, k);
                    break;
                case GPV:
                    gpvcoeff(pr, k);
                    break;
                case FCV:
                case PRV:
                case PSV:
                    if (hyd->LinkSetting[k] == MISSING) valvecoeff(pr, k);
                    else hyd->P[k] = 0.0;
                }
            }
        };

        void     linkcoeffs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: computes coefficients contributed by links to the
            **            linearized system of hydraulic equations.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            int   k, n1, n2, nL;
            // Examine each link of network
            nL = net->Nlinks;
            auto& links = net->Link;
            Slink* link;
            for (k = 1; k <= nL; k++)
            {
                if (hyd->P[k] == (cfs_p_ft_t)0.0) continue;
                link = links[k].Get();
                n1 = link->N1;           // Start node of link
                n2 = link->N2;           // End node of link

                // Update nodal flow excess (Xflow)
                // (Flow out of node is (-), flow into node is (+))
                hyd->Xflow[n1] -= hyd->LinkFlow[k];
                hyd->Xflow[n2] += hyd->LinkFlow[k];

                // Add to off-diagonal coeff. of linear system matrix
                sm->Aij[sm->Ndx[k]] -= hyd->P[k];

                // Update linear system coeffs. associated with start node n1
                // ... node n1 is junction
                if (n1 <= net->Njuncs)
                {
                    sm->Aii[sm->Row[n1]] += hyd->P[k];   // Diagonal coeff.
                    sm->F[sm->Row[n1]] += hyd->Y[k];     // RHS coeff.
                }

                // ... node n1 is a tank/reservoir
                else sm->F[sm->Row[n2]] += hyd->P[k] * hyd->NodeHead[n1];

                // Update linear system coeffs. associated with end node n2
                // ... node n2 is junction
                if (n2 <= net->Njuncs)
                {
                    sm->Aii[sm->Row[n2]] += hyd->P[k];   // Diagonal coeff.
                    sm->F[sm->Row[n2]] -= hyd->Y[k];     // RHS coeff.
                }

                // ... node n2 is a tank/reservoir
                else sm->F[sm->Row[n1]] += hyd->P[k] * hyd->NodeHead[n2];
            }
        };
        void     emittercoeffs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: computes coeffs. of the linearized hydraulic eqns.
            **            contributed by emitters.
            **
            **   Note: Emitters consist of a fictitious pipe connected to
            **         a fictitious reservoir whose elevation equals that
            **         of the junction. The headloss through this pipe is
            **         Ke*(Flow)^hyd->Qexp, where Ke = emitter headloss coeff.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            int    i, row, nN;
            foot_t hloss;
            ft_per_cfs_t hgrad;
            auto& nodes = net->Node;
            nN = net->Nnodes;
            for (i = 1; i <= nN; i++) {
                auto& node = nodes[i];
                if (node->Ke == 0.0) continue;

                // Find emitter head loss and gradient
                emitterheadloss(pr, i, &hloss, &hgrad);

                // Row of solution matrix
                row = sm->Row[i];

                // Addition to matrix diagonal & r.h.s
                sm->Aii[row] += 1.0 / hgrad;
                sm->F[row] += (hloss + node->El) / hgrad;

                // Update to node flow excess
                hyd->Xflow[i] -= hyd->EmitterFlow[i];
            }
        };
        void     demandcoeffs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: computes coeffs. of the linearized hydraulic eqns.
            **            contributed by pressure dependent demands.
            **
            **   Note: Pressure dependent demands are modelled like emitters
            **         with Hloss = Preq * (D / Dfull)^(1/Pexp)
            **         where D (actual demand) is zero for negative pressure
            **         and is Dfull above pressure Preq.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            int i, row;
            foot_t
                dp;         // pressure range over which demand can vary (ft)
            SCALER
                n;          // exponent in head loss v. demand function
            foot_t
                hloss;      // head loss in supplying demand (ft)
            ft_per_cfs_t
                hgrad;      // gradient of demand head loss (ft/cfs)

            // Get demand function parameters
            if (hyd->DemandModel == DDA) return;
            dp = (foot_t)(double)(hyd->Preq - hyd->Pmin);
            n = 1.0 / hyd->Pexp;

            // Examine each junction node
            for (i = 1; i <= net->Njuncs; i++)
            {
                // Skip junctions with non-positive demands
                if (hyd->NodeDemand[i] <= 0.0_cfs) continue;

                // Find head loss for demand outflow at node's elevation
                demandheadloss(pr, i, dp, n, &hloss, &hgrad);

                // Update row of solution matrix A & its r.h.s. F
                if (hgrad > (ft_per_cfs_t)0.0)
                {
                    row = sm->Row[i];
                    sm->Aii[row] += 1.0 / hgrad;
                    sm->F[row] += (hloss + net->Node[i]->El + (foot_t)(double)hyd->Pmin) / hgrad;
                }
            }
        };
        void     nodecoeffs(EN_Project const& pr)
            /*
            **----------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: completes calculation of nodal flow balance array
            **           (Xflow) & r.h.s. (F) of linearized hydraulic eqns.
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            int   i;

            // For junction nodes, subtract demand flow from net
            // flow excess & add flow excess to RHS array F
            for (i = 1; i <= net->Njuncs; i++)
            {
                hyd->Xflow[i] -= hyd->DemandFlow[i];
                sm->F[sm->Row[i]] += hyd->Xflow[i];
            }
        };
        void     valvecoeffs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: computes coeffs. of the linearized hydraulic eqns.
            **            contributed by PRVs, PSVs & FCVs whose status is
            **            not fixed to OPEN/CLOSED
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int i, k, n1, n2;
            Plink link;
            Pvalve valve;

            // Examine each valve
            for (i = 1; i <= net->Nvalves; i++)
            {
                // Find valve's link index
                valve = net->Valve[i];
                k = valve->Link;

                // Coeffs. for fixed status valves have already been computed
                if (hyd->LinkSetting[k] == MISSING) continue;

                // Start & end nodes of valve's link
                link = net->Link[k];
                n1 = link->N1;
                n2 = link->N2;

                // Call valve-specific function
                switch (link->Type)
                {
                case PRV:
                    prvcoeff(pr, k, n1, n2);
                    break;
                case PSV:
                    psvcoeff(pr, k, n1, n2);
                    break;
                case FCV:
                    fcvcoeff(pr, k, n1, n2);
                    break;
                default:   continue;
                }
            }
        };
        void     emitterheadloss(EN_Project const& pr, int i, foot_t* hloss, ft_per_cfs_t* hgrad)
            /*
            **-------------------------------------------------------------
            **   Input:   i = node index
            **   Output:  hloss = head loss across node's emitter
            **            hgrad = head loss gradient
            **   Purpose: computes an emitters's head loss and gradient.
            **-------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            SCALER  ke;
            cubic_foot_per_second_t  q;

            // Set adjusted emitter coeff.
            ke = fMAX(CSMALL, pr->network->Node[i]->Ke);

            // Compute gradient of head loss through emitter
            q = hyd->EmitterFlow[i];
            *hgrad = (double)(hyd->Qexp * ke * ::epanet::pow(ABS(q), hyd->Qexp - 1.0));

            // Use linear head loss function for small gradient
            if ((double)(*hgrad) < hyd->RQtol)
            {
                *hgrad = (double)hyd->RQtol;
                *hloss = (*hgrad) * q;
            }

            // Otherwise use normal emitter head loss function
            else *hloss = (*hgrad) * q / hyd->Qexp;
        };

        void     demandheadloss(EN_Project const& pr, int i, foot_t dp, SCALER n, foot_t* hloss, ft_per_cfs_t* hgrad)
            /*
            **--------------------------------------------------------------
            **   Input:   i  = junction index
            **            dp = pressure range for demand function (ft)
            **            n  = exponent in head v. demand function
            **   Output:  hloss = pressure dependent demand head loss (ft)
            **            hgrad = gradient of head loss (ft/cfs)
            **  Purpose:  computes head loss and its gradient for delivering
            **            a pressure dependent demand flow.
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            auto d = hyd->DemandFlow[i];
            auto dfull = hyd->NodeDemand[i];
            SCALER r = d / dfull;

            // Use lower barrier function for negative demand
            if (r <= (SCALER)0)
            {
                *hgrad = (double)CBIG;
                *hloss = (double)(CBIG * d);
            }

            // Use power head loss function for demand less than full
            else if (r < 1.0)
            {
                *hgrad = n * dp * ::epanet::pow(r, n - 1.0) / dfull;
                // ... use linear function for very small gradient
                if (((double)(*hgrad)) < hyd->RQtol)
                {
                    *hgrad = (double)hyd->RQtol;
                    *hloss = (*hgrad) * d;
                }
                else *hloss = (*hgrad) * d / n;
            }

            // Use upper barrier function for demand above full value
            else
            {
                *hgrad = (double)CBIG;
                *hloss = dp + (foot_t)(double)(CBIG * (d - dfull));
            }
        };

        void     matrixcoeffs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: computes coefficients of linearized network eqns.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;

            // Reset values of all diagonal coeffs. (Aii), off-diagonal
            // coeffs. (Aij), r.h.s. coeffs. (F) and node excess flow (Xflow)
            for (auto& x : sm->Aii) x = 0;
            for (auto& x : sm->Aij) x = 0;
            for (auto& x : sm->F) x = 0;
            for (auto& x : hyd->Xflow) x = 0;

            // Compute matrix coeffs. from links, emitters, and nodal demands
            linkcoeffs(pr);
            emittercoeffs(pr);
            demandcoeffs(pr);

            // Update nodal flow balances with demands and add onto r.h.s. coeffs.
            nodecoeffs(pr);

            // Finally, find coeffs. for PRV/PSV/FCV control valves whose status is not fixed to OPEN/CLOSED
            valvecoeffs(pr);
        };
        void     resistcoeff(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------------
            **  Input:   k = link index
            **  Output:  none
            **  Purpose: computes link flow resistance coefficient
            **--------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            SCALER e, d, L;
            Plink link = net->Link[k];

            switch (link->Type) {

                // ... Link is a pipe. Compute resistance based on headloss formula.
                //     Friction factor for D-W formula gets included during head loss
                //     calculation.
            case CVPIPE:
            case PIPE:
                e = link->Kc;                 // Roughness coeff.
                d = (double)link->Diam;               // Diameter
                L = (double)link->Len;                // Length
                switch (hyd->Formflag)
                {
                case HW:
                    link->R_FlowResistance = 4.727 * L / ::epanet::pow(e, hyd->Hexp) / ::epanet::pow(d, 4.871);
                    break;
                case DW:
                    link->R_FlowResistance = L / 2.0 / 32.2 / d / SQR(PI * SQR(d) / 4.0);
                    break;
                case CM:
                    link->R_FlowResistance = SQR(4.0 * e / (1.49 * PI * SQR(d))) *
                        ::epanet::pow((d / 4.0), -1.333) * L;
                }
                break;

                // ... Link is a pump. Use huge resistance.
            case PUMP:
                link->R_FlowResistance = CBIG;
                break;

                // ... For all other links (e.g. valves) use a small resistance
            default:
                link->R_FlowResistance = CSMALL;
                break;
            }
        };

        StatusType next_prv_status(EN_Project const& pr, int k, StatusType s, SCALER hset, SCALER h1, SCALER h2)
            /*
            **-----------------------------------------------------------
            **  Input:   k    = link index
            **           s    = current status
            **           hset = valve head setting
            **           h1   = head at upstream node
            **           h2   = head at downstream node
            **  Output:  returns new valve status
            **  Purpose: updates status of a pressure reducing valve.
            **-----------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            StatusType status;             // Valve's new status
            SCALER  hml;                   // Head loss when fully opened
            SCALER  htol;
            Plink link;

            htol = (double)hyd->Htol;
            link = pr->network->Link[k];

            // Head loss when fully open
            hml = link->Km * (double)SQR(hyd->LinkFlow[k]);

            // Rules for updating valve's status from current value s
            status = s;
            switch (s)
            {
            case StatusType::ACTIVE:
                if (hyd->LinkFlow[k] < (-hyd->Qtol))  status = StatusType::CLOSED;            // if the valve was active, but the flow is small, CLOSE the valve.
                else if (h1 - hml < hset - htol)     status = StatusType::OPEN;             // if the valve was active, flow is not small, but (upstream head - (headloss@fullyOpen)) < desiredHead) then simply set the valve to OPEN. 
                else                                 status = StatusType::ACTIVE;           // if the valve is ACTIVE and headloss is available, the the flow should be considered ACTIVE. 
                break;
            case StatusType::OPEN:
                if (hyd->LinkFlow[k] < (-hyd->Qtol))  status = StatusType::CLOSED;            // if the valve was fully open, but the flow is small, close the valve.
                else if (h2 >= hset + htol)          status = StatusType::ACTIVE;           // if the valve was fully open, and some excess head is available to blow off, then set to ACTIVE
                else                                 status = StatusType::OPEN;             // if the valve was fully open, but still not enough head is available for blowoff, then set to OPEN
                break;
            case StatusType::CLOSED:
                if (h1 >= hset + htol && h2 < hset - htol)   status = StatusType::ACTIVE;   // if the valve was closed, and upstream is greater than my setting, and downstread is lower than my setting, then ACTIVATE. 
                else if (h1 < hset - htol && h1 > h2 + htol) status = StatusType::OPEN;     // if the valve was closed, and the head upstream is less than my setting, yet the upstream is still greater than downstream, then FULLY OPEN
                else                                         status = StatusType::CLOSED;   // if the valve was closed but the pressure conditions aren't good enough to open or activate, then remain closed. 
                break;
            case StatusType::XPRESSURE:
                if (hyd->LinkFlow[k] < (-hyd->Qtol)) status = StatusType::CLOSED;             // the valve could not provide the requested pressure and must close. 
                break;
            default:
                // do nothing -> status remains the same. 
                break;
            }
            return status;
        };
        StatusType next_psv_status(EN_Project const& pr, int k, StatusType s, SCALER hset, SCALER h1, SCALER h2)
            /*
            **-----------------------------------------------------------
            **  Input:   k    = link index
            **           s    = current status
            **           hset = valve head setting
            **           h1   = head at upstream node
            **           h2   = head at downstream node
            **  Output:  returns new valve status
            **  Purpose: updates status of a pressure sustaining valve.
            **-----------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            StatusType status;             // Valve's new status
            SCALER  hml;                   // Head loss when fully opened
            SCALER  htol;
            Plink link;

            htol = (double)hyd->Htol;
            link = pr->network->Link[k];

            // Head loss when fully open
            hml = link->Km * (double)SQR(hyd->LinkFlow[k]);

            // Rules for updating valve's status from current value s
            status = s;
            switch (s)
            {
            case StatusType::ACTIVE:
                if (hyd->LinkFlow[k] < (-hyd->Qtol))              status = StatusType::CLOSED;
                else if (h2 + hml > hset + htol)                status = StatusType::OPEN;
                else                                            status = StatusType::ACTIVE;
                break;

            case StatusType::OPEN:
                if (hyd->LinkFlow[k] < (-hyd->Qtol))              status = StatusType::CLOSED;
                else if (h1 < hset - htol)                      status = StatusType::ACTIVE;
                else                                            status = StatusType::OPEN;
                break;

            case StatusType::CLOSED:
                if (h2 > hset + htol && h1 > h2 + htol)         status = StatusType::OPEN;
                else if (h1 >= hset + htol && h1 > h2 + htol)   status = StatusType::ACTIVE;
                else                                            status = StatusType::CLOSED;
                break;

            case StatusType::XPRESSURE:
                if (hyd->LinkFlow[k] < (-hyd->Qtol)) status = StatusType::CLOSED;
                break;

            default:
                break;
            }
            return status;
        };
        int      calc_and_set_prv_and_psv_status(EN_Project const& pr)
            /*
            **-----------------------------------------------------------------
            **  Input:   none
            **  Output:  returns 1 if any pressure or flow control valve
            **           changes status, 0 otherwise
            **  Purpose: updates status for PRVs & PSVs whose status
            **           is not fixed to OPEN/CLOSED
            **-----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;

            int    change = FALSE,   // Status change flag
                i, k,             // Valve & link indexes
                n1, n2;           // Start & end nodes
            SCALER hset;             // Valve head setting
            StatusType status;       // Valve status settings
            Plink link;

            // Examine each valve
            for (i = 1; i <= net->Nvalves; i++)
            {
                // Get valve's link and its index
                k = net->Valve[i]->Link;
                link = net->Link[k];

                // Ignore valve if its status is fixed to OPEN/CLOSED
                if (hyd->LinkSetting[k] == MISSING) continue;

                // Get start/end node indexes & save current status
                n1 = link->N1;
                n2 = link->N2;
                status = hyd->LinkStatus[k];

                // Evaluate valve's new status
                switch (link->Type)
                {
                case PRV:
                    hset = (double)(net->Node[n2]->El + (foot_t)(double)hyd->LinkSetting[k]);
                    hyd->LinkStatus[k] = next_prv_status(pr, k, status, hset, (double)hyd->NodeHead[n1], (double)hyd->NodeHead[n2]);
                    break;
                case PSV:
                    hset = (double)(net->Node[n1]->El + (foot_t)(double)hyd->LinkSetting[k]);
                    hyd->LinkStatus[k] = next_psv_status(pr, k, status, hset, (double)hyd->NodeHead[n1], (double)hyd->NodeHead[n2]);
                    break;
                default:
                    continue;
                }

                // Check for a status change
                if (status != hyd->LinkStatus[k])
                {
                    if ((SCALER)rpt->Statflag == FULL)
                    {
                        writestatchange(pr, k, status, hyd->LinkStatus[k]);
                    }
                    change = TRUE;
                }
            }
            return change;
        };
        StatusType cvstatus(EN_Project const& pr, StatusType s, SCALER dh, SCALER q)
            /*
            **--------------------------------------------------
            **  Input:   s  = current link status
            **           dh = head loss across link
            **           q  = link flow
            **  Output:  returns new link status
            **  Purpose: updates status of a check valve link.
            **--------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            // Prevent reverse flow through CVs
            if (ABS(dh) > (SCALER)(double)hyd->Htol)
            {
                if (dh < (SCALER)(double)(-hyd->Htol))     return CLOSED;
                else if (q < (SCALER)(double)(-hyd->Qtol)) return CLOSED;
                else                     return OPEN;
            }
            else
            {
                if (q < (SCALER)(double)(-hyd->Qtol)) return CLOSED;
                else                return s;
            }
        };
        StatusType pumpstatus(EN_Project const& pr, int k, SCALER dh)
            /*
            **--------------------------------------------------
            **  Input:   k  = link index
            **           dh = head gain across link
            **  Output:  returns new pump status
            **  Purpose: updates status of an open pump.
            **--------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            EN_Network const& net = pr->network;

            int   p;
            foot_t hmax;

            // Find maximum head (hmax) pump can deliver
            p = findpump(net, k);
            if (net->Pump[p]->Ptype == CONST_HP)
            {
                // Use huge value for constant HP pump
                hmax = (foot_t)(double)BIG;
            }
            else
            {
                // Use speed-adjusted shut-off head for other pumps
                hmax = SQR(hyd->LinkSetting[k]) * net->Pump[p]->Hmax;
            }

            // Check if currrent head gain exceeds pump's max. head
            if ((foot_t)(double)dh > hmax + hyd->Htol) return XHEAD;

            // No check is made to see if flow exceeds pump's max. flow
            return OPEN;
        };
        StatusType next_fcv_status(EN_Project const& pr, int k, StatusType s, SCALER h1, SCALER h2)
            /*
            **-----------------------------------------------------------
            **  Input:   k    = link index
            **           s    = current status
            **           h1   = head at upstream node
            **           h2   = head at downstream node
            **  Output:  returns new valve status
            **  Purpose: updates status of a flow control valve.
            **
            **    Valve status changes to XFCV if flow reversal.
            **    If current status is XFCV and current flow is
            **    above setting, then valve becomes active.
            **    If current status is XFCV, and current flow
            **    positive but still below valve setting, then
            **    status remains same.
            **-----------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            StatusType status;            // New valve status

            status = s;
            if (h1 - h2 < (SCALER)(double)(-hyd->Htol)) // if the upstream head is less than the downstream head, simply exit early and say we cannot do this. 
            {
                status = XFCV;
            }
            else if (hyd->LinkFlow[k] < (-hyd->Qtol)) // else if the flow through the link is negative, exit early and say we cannot do this.  
            {
                status = XFCV;
            }
            else if (s == XFCV && hyd->LinkFlow[k] >= (cubic_foot_per_second_t)(double)hyd->LinkSetting[k]) // else if the flowis currently in a bad state, see if the flow has returned to the desired range, and set to ACTIVE to clamp the flow. 
            {
                status = ACTIVE;
            }
            return status;
        };
        void     tankstatus(EN_Project const& pr, int k, int n1, int n2)
            /*
            **----------------------------------------------------------------
            **  Input:   k  = link index
            **           n1 = start node of link
            **           n2 = end node of link
            **  Output:  none
            **  Purpose: closes link flowing into full or out of empty tank
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int   i, n;
            SCALER h, q;
            Ptank tank;
            Plink link = net->Link[k];

            // Return if link is closed
            if (hyd->LinkStatus[k] <= CLOSED) return;

            // Make node n1 be the tank, reversing flow (q) if need be
            q = (double)hyd->LinkFlow[k];
            i = n1 - net->Njuncs;
            if (i <= 0)
            {
                i = n2 - net->Njuncs;
                if (i <= 0) return;
                n = n1;
                n1 = n2;
                n2 = n;
                q = -q;
            }

            // Ignore reservoirs
            tank = net->Tank[i];
            if (tank->Diameter == 0.0_ft) return;

            // Find head difference across link
            h = (double)(hyd->NodeHead[n1] - hyd->NodeHead[n2]);

            // If tank is full, then prevent flow into it
            if (hyd->NodeHead[n1] >= (tank->Hmax - hyd->Htol) && !tank->CanOverflow)
            {
                // Case 1: Link is a pump discharging into tank
                if (link->Type == PUMP)
                {
                    if (link->N2 == n1) hyd->LinkStatus[k] = TEMPCLOSED;
                }

                // Case 2: Downstream head > tank head
                // (e.g., an open outflow check valve would close)
                else if (cvstatus(pr, OPEN, h, q) == CLOSED)
                {
                    hyd->LinkStatus[k] = TEMPCLOSED;
                }
            }

            // If tank is empty, then prevent flow out of it
            if (hyd->NodeHead[n1] <= (tank->Hmin + hyd->Htol))
            {
                // Case 1: Link is a pump discharging from tank
                if (link->Type == PUMP)
                {
                    if (link->N1 == n1) hyd->LinkStatus[k] = TEMPCLOSED;
                }

                // Case 2: Tank head > downstream head
                // (e.g., a closed outflow check valve would open)
                else if (cvstatus(pr, CLOSED, h, q) == OPEN)
                {
                    hyd->LinkStatus[k] = TEMPCLOSED;
                }
            }
        };
        int      linkstatus(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns 1 if any link changes status, 0 otherwise
            **  Purpose: determines new status for pumps, CVs, FCVs & pipes
            **           to tanks.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;

            int change = FALSE,             // Status change flag
                k,                          // Link index
                n1,                         // Start node index
                n2;                         // End node index
            foot_t dh;                      // Head difference across link
            StatusType  status;             // Current status
            Slink* link;
            // Examine each link
            for (k = 1; k <= net->Nlinks; k++)
            {
                link = net->Link[k].Get();
                n1 = link->N1;
                n2 = link->N2;
                dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];

                // Re-open temporarily closed links (status = XHEAD or TEMPCLOSED)
                status = hyd->LinkStatus[k];
                if (status == XHEAD || status == TEMPCLOSED)
                {
                    hyd->LinkStatus[k] = OPEN;
                }

                // Check for status changes in CVs and pumps
                if (link->Type == CVPIPE)
                {
                    hyd->LinkStatus[k] = cvstatus(pr, hyd->LinkStatus[k], (double)dh, (double)hyd->LinkFlow[k]);
                }
                if (link->Type == PUMP && hyd->LinkStatus[k] >= OPEN &&
                    hyd->LinkSetting[k] > 0.0)
                {
                    hyd->LinkStatus[k] = pumpstatus(pr, k, (double)(-dh));
                }

                // Check for status changes in non-fixed FCVs
                if (link->Type == FCV && hyd->LinkSetting[k] != MISSING)
                {
                    hyd->LinkStatus[k] = next_fcv_status(pr, k, status, (double)hyd->NodeHead[n1], (double)hyd->NodeHead[n2]);
                }

                // Check for flow into (out of) full (empty) tanks
                if (n1 > net->Njuncs || n2 > net->Njuncs)
                {
                    tankstatus(pr, k, n1, n2);
                }

                // Note any change in link status; do not revise link flow
                if (status != hyd->LinkStatus[k])
                {
                    change = TRUE;
                    if ((SCALER)rpt->Statflag == FULL)
                    {
                        writestatchange(pr, k, status, hyd->LinkStatus[k]);
                    }
                }
            }
            return change;
        };



        int      badvalve(EN_Project const& pr, int n)
            /*
            **-----------------------------------------------------------------
            **  Input:   n = node index
            **  Output:  returns 1 if node n belongs to an active control valve,
            **           0 otherwise
            **  Purpose: determines if a node belongs to an active control valve
            **           whose setting causes an inconsistent set of eqns. If so,
            **           the valve status is fixed open and a warning condition
            **           is generated.
            **-----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            int i, k, n1, n2;
            Plink link;
            LinkType t;

            for (i = 1; i <= net->Nvalves; i++)
            {
                k = net->Valve[i]->Link;
                link = net->Link[k];
                n1 = link->N1;
                n2 = link->N2;
                if (n == n1 || n == n2)
                {
                    t = link->Type;
                    if (t == PRV || t == PSV || t == FCV)
                    {
                        if (hyd->LinkStatus[k] == ACTIVE)
                        {
                            if ((SCALER)rpt->Statflag == FULL)
                            {
                                sprintf(pr->Msg, FMT61,
                                    clocktime(rpt->Atime, time->Htime), link->ID);
                                writeline(pr, pr->Msg);
                            }
                            if (link->Type == FCV) hyd->LinkStatus[k] = XFCV;
                            else                   hyd->LinkStatus[k] = XPRESSURE;
                            return 1;
                        }
                    }
                    return 0;
                }
            }
            return 0;
        };
        int      pswitch(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns 1 if status of any link changes, 0 if not
            **  Purpose: adjusts settings of links controlled by junction
            **           pressures after a hydraulic solution is found
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;

            int   i,                 // Control statement index
                k,                 // Index of link being controlled
                n,                 // Node controlling link k
                reset,             // Flag on control conditions
                change,            // Flag for status or setting change
                anychange = 0;     // Flag for 1 or more control actions
            char  s;                 // Current link status
            Plink link;

            // Check each control statement
            for (i = 1; i <= net->Ncontrols; i++)
            {
                reset = 0;
                k = net->Control[i]->Link;
                if (k <= 0) continue;

                // Determine if control based on a junction, not a tank
                n = net->Control[i]->Node;
                if (n > 0 && n <= net->Njuncs)
                {
                    // Determine if control conditions are satisfied
                    if (net->Control[i]->Type == LOWLEVEL &&
                        hyd->NodeHead[n] <= ((foot_t)(double)net->Control[i]->Grade + hyd->Htol))
                    {
                        reset = 1;
                    }
                    if (net->Control[i]->Type == HILEVEL &&
                        hyd->NodeHead[n] >= ((foot_t)(double)net->Control[i]->Grade - hyd->Htol))
                    {
                        reset = 1;
                    }
                }

                // Determine if control forces a status or setting change
                if (reset == 1)
                {
                    link = net->Link[k];
                    change = 0;
                    s = hyd->LinkStatus[k];
                    if (link->Type == PIPE)
                    {
                        if (s != net->Control[i]->Status) change = 1;
                    }
                    if (link->Type == PUMP)
                    {
                        if (hyd->LinkSetting[k] != net->Control[i]->Setting) change = 1;
                    }
                    if (link->Type >= PRV)
                    {
                        if (hyd->LinkSetting[k] != net->Control[i]->Setting) change = 1;
                        else if (hyd->LinkSetting[k] == MISSING && s != net->Control[i]->Status)
                        {
                            change = 1;
                        }
                    }

                    // If a change occurs, update status & setting
                    if (change)
                    {
                        hyd->LinkStatus[k] = net->Control[i]->Status;
                        if (link->Type > PIPE)
                        {
                            hyd->LinkSetting[k] = net->Control[i]->Setting;
                        }
                        if ((SCALER)rpt->Statflag == FULL)
                        {
                            writestatchange(pr, k, s, hyd->LinkStatus[k]);
                        }
                        anychange = 1;
                    }
                }
            }
            return anychange;
        };
        SCALER   newflows(EN_Project const& pr, Hydbalance* hbal)
            /*
            **----------------------------------------------------------------
            **  Input:   hbal = ptr. to hydraulic balance information
            **  Output:  returns solution convergence error
            **  Purpose: updates link, emitter & demand flows after new
            **           nodal heads are computed.
            **----------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            cubic_foot_per_second_t
                dqsum,                 // Network flow change
                qsum;                  // Network total flow

        // Initialize sum of flows & corrections
            qsum = 0.0;
            dqsum = 0.0;
            hbal->maxflowchange = 0.0_cfs;
            hbal->maxflowlink = 1;
            hbal->maxflownode = -1;

            // Update flows in all real and virtual links
            newlinkflows(pr, hbal, &qsum, &dqsum);
            newemitterflows(pr, hbal, &qsum, &dqsum);
            newdemandflows(pr, hbal, &qsum, &dqsum);

            // Return ratio of total flow corrections to total flow
            if ((SCALER)(double)qsum > hyd->Hacc) return (dqsum / qsum);
            else return (SCALER)(double)dqsum;
        };
        void     newlinkflows(EN_Project const& pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum)
            /*
            **----------------------------------------------------------------
            **  Input:   hbal = ptr. to hydraulic balance information
            **           qsum = sum of current system flows
            **           dqsum = sum of system flow changes
            **  Output:  updates hbal, qsum and dqsum
            **  Purpose: updates link flows after new nodal heads computed
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            foot_t
                dh;                    /* Link head loss       */
            cubic_foot_per_second_t
                dq;                    /* Link flow change     */
            int     k, n, n1, n2;

            // Initialize net inflows (i.e., demands) at fixed grade nodes
            for (n = net->Njuncs + 1; n <= net->Nnodes; n++)
            {
                hyd->NodeDemand[n] = 0.0_cfs;
            }
            auto& links = net->Link;
            Slink* link;
            // Examine each link
            for (k = 1; k <= net->Nlinks; k++)
            {
                link = links[k].Get();
                n1 = link->N1;
                n2 = link->N2;

                // Apply flow update formula:
                //   dq = Y - P * (new head loss)
                //    P = 1 / (previous head loss gradient)
                //    Y = P * (previous head loss)
                // where P & Y were computed in hlosscoeff() in hydcoeffs.c
                dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];
                dq = hyd->Y[k] - (hyd->P[k] * dh);

                // Adjust flow change by the relaxation factor
                dq *= hyd->RelaxFactor;

                // Prevent flow in constant HP pumps from going negative
                if (link->Type == PUMP)
                {
                    n = findpump(net, k);
                    if (net->Pump[n]->Ptype == CONST_HP && dq > hyd->LinkFlow[k])
                    {
                        dq = hyd->LinkFlow[k] / 2.0;
                    }
                }

                // Update link flow and system flow summation
                hyd->LinkFlow[k] -= dq;
                *qsum += ABS(hyd->LinkFlow[k]);
                *dqsum += ABS(dq);

                // Update identity of element with max. flow change
                if (ABS(dq) > hbal->maxflowchange)
                {
                    hbal->maxflowchange = ABS(dq);
                    hbal->maxflowlink = k;
                    hbal->maxflownode = -1;
                }

                // Update net flows to fixed grade nodes
                if (hyd->LinkStatus[k] > CLOSED)
                {
                    if (n1 > net->Njuncs) hyd->NodeDemand[n1] -= hyd->LinkFlow[k];
                    if (n2 > net->Njuncs) hyd->NodeDemand[n2] += hyd->LinkFlow[k];
                }
            }
        };
        void     newemitterflows(EN_Project const& pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum)
            /*
            **----------------------------------------------------------------
            **  Input:   hbal = ptr. to hydraulic balance information
            **           qsum = sum of current system flows
            **           dqsum = sum of system flow changes
            **  Output:  updates hbal, qsum and dqsum
            **  Purpose: updates nodal emitter flows after new nodal heads computed
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int     i;
            foot_t  hloss;
            ft_per_cfs_t  hgrad;
            foot_t  dh;
            cubic_foot_per_second_t  dq;
            auto& Node = net->Node;

            // Examine each network junction
            for (i = 1; i <= net->Njuncs; i++)
            {
                // Skip junction if it does not have an emitter
                if (Node[i]->Ke == 0.0) continue;

                // Find emitter head loss and gradient
                emitterheadloss(pr, i, &hloss, &hgrad);

                // Find emitter flow change
                dh = hyd->NodeHead[i] - net->Node[i]->El;
                dq = (hloss - dh) / hgrad;
                dq *= hyd->RelaxFactor;
                hyd->EmitterFlow[i] -= dq;

                // Update system flow summation
                *qsum += ABS(hyd->EmitterFlow[i]);
                *dqsum += ABS(dq);

                // Update identity of element with max. flow change
                if (ABS(dq) > hbal->maxflowchange)
                {
                    hbal->maxflowchange = ABS(dq);
                    hbal->maxflownode = i;
                    hbal->maxflowlink = -1;
                }
            }
        };
        void     newdemandflows(EN_Project const& pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum)
            /*
            **----------------------------------------------------------------
            **  Input:   hbal = ptr. to hydraulic balance information
            **           qsum = sum of current system flows
            **           dqsum = sum of system flow changes
            **  Output:  updates hbal, qsum and dqsum
            **  Purpose: updates nodal pressure dependent demand flows after
            **           new nodal heads computed
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            foot_t
                dp;         // pressure range over which demand can vary (ft)
            cubic_foot_per_second_t
                dq;         // change in demand flow (cfs)
            SCALER
                n;          // exponent in head loss v. demand  function
            foot_t
                hloss;      // current head loss through outflow junction (ft)
            ft_per_cfs_t
                hgrad;      // head loss gradient with respect to flow (ft/cfs)
            foot_t
                dh;         // new head loss through outflow junction (ft)
            int     i;

            // Get demand function parameters
            if (hyd->DemandModel == DDA) return;
            dp = (foot_t)(double)fMAX((hyd->Preq - hyd->Pmin), MINPDIFF);
            n = 1.0 / hyd->Pexp;

            // Examine each junction
            for (i = 1; i <= net->Njuncs; i++)
            {
                // Skip junctions with no positive demand
                if (hyd->NodeDemand[i] <= 0.0_cfs) continue;

                // Find change in demand flow (see hydcoeffs.c)
                demandheadloss(pr, i, dp, n, &hloss, &hgrad);
                dh = (hyd->NodeHead[i] - net->Node[i]->El - (foot_t)(double)hyd->Pmin);
                dq = (hloss - dh) / hgrad;
                dq *= hyd->RelaxFactor;
                hyd->DemandFlow[i] -= dq;

                // Update system flow summation
                *qsum += ABS(hyd->DemandFlow[i]);
                *dqsum += ABS(dq);

                // Update identity of element with max. flow change
                if (ABS(dq) > hbal->maxflowchange)
                {
                    hbal->maxflowchange = ABS(dq);
                    hbal->maxflownode = i;
                    hbal->maxflowlink = -1;
                }
            }
        };
        void     checkhydbalance(EN_Project const& pr, Hydbalance* hbal)
            /*
            **--------------------------------------------------------------
            **   Input:   hbal = hydraulic balance errors
            **   Output:  none
            **   Purpose: finds the link with the largest head imbalance
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            foot_t dh, headerror, headloss;
            Slink* link;

            hbal->maxheaderror = 0.0_ft;
            hbal->maxheadlink = 1;
            headlosscoeffs(pr);
            for (int k = 1; k <= net->Nlinks; k++)
            {
                if (hyd->LinkStatus[k] <= CLOSED) continue;
                if (hyd->P[k] == (cfs_p_ft_t)0.0) continue;
                link = net->Link[k].Get();
                dh = hyd->NodeHead[link->N1] - hyd->NodeHead[link->N2];
                headloss = hyd->Y[k] / hyd->P[k];
                headerror = ABS(dh - headloss);
                if (headerror > hbal->maxheaderror)
                {
                    hbal->maxheaderror = headerror;
                    hbal->maxheadlink = k;
                }
            }
        };

        int      pdaconverged(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns 1 if PDA converged, 0 if not
            **   Purpose: checks if pressure driven analysis has converged
            **            and updates total demand deficit
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            const SCALER TOL = 0.001;
            int i, converged = 1;
            SCALER totalDemand = 0.0, totalReduction = 0.0;

            hyd->DeficientNodes = 0;
            hyd->DemandReduction = 0.0;

            // Add up number of junctions with demand deficits
            for (i = 1; i <= pr->network->Njuncs; i++)
            {
                // Skip nodes whose required demand is non-positive
                if (hyd->NodeDemand[i] <= 0.0_cfs) continue;

                // Check for negative demand flow or positive demand flow at negative pressure
                if (hyd->DemandFlow[i] < (cubic_foot_per_second_t)(double)(-TOL)) converged = 0;
                if (hyd->DemandFlow[i] > (cubic_foot_per_second_t)(double)(TOL) && hyd->NodeHead[i] - pr->network->Node[i]->El - (foot_t)(double)hyd->Pmin < (foot_t)(double)(-TOL))
                    converged = 0;

                // Accumulate total required demand and demand deficit
                if (hyd->DemandFlow[i] + 0.0001_cfs < hyd->NodeDemand[i])
                {
                    hyd->DeficientNodes++;
                    totalDemand += (SCALER)(double)hyd->NodeDemand[i];
                    totalReduction += (SCALER)(double)(hyd->NodeDemand[i] - hyd->DemandFlow[i]);
                }
            }
            if (totalDemand > 0.0)
                hyd->DemandReduction = totalReduction / totalDemand * 100.0;
            return converged;
        };
        void     reporthydbal(EN_Project const& pr, Hydbalance* hbal)
            /*
            **--------------------------------------------------------------
            **   Input:   hbal   = current hydraulic balance errors
            **   Output:  none
            **   Purpose: identifies links with largest flow change and
            **            largest head loss error.
            **--------------------------------------------------------------
            */
        {
            SCALER qchange = (double)hbal->maxflowchange * pr->Ucf[FLOW];
            SCALER herror = (double)hbal->maxheaderror * pr->Ucf[HEAD];
            int    qlink = hbal->maxflowlink;
            int    qnode = hbal->maxflownode;
            int    hlink = hbal->maxheadlink;
            if (qlink >= 1)
            {
                sprintf(pr->Msg, FMT66, qchange, pr->network->Link[qlink]->ID);
                writeline(pr, pr->Msg);
            }
            else if (qnode >= 1)
            {
                sprintf(pr->Msg, FMT67, qchange, pr->network->Node[qnode]->ID);
                writeline(pr, pr->Msg);
            }
            if (hlink >= 1)
            {
                sprintf(pr->Msg, FMT68, herror, pr->network->Link[hlink]->ID);
                writeline(pr, pr->Msg);
            }
        };
        int      hasconverged(EN_Project const& pr, SCALER* relerr, Hydbalance* hbal)
            /*
            **--------------------------------------------------------------
            **   Input:   relerr = current total relative flow change
            **            hbal   = current hydraulic balance errors
            **   Output:  returns 1 if system has converged or 0 if not
            **   Purpose: checks various criteria to see if system has
            **            become hydraulically balanced
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;

            // Check that total relative flow change is small enough
            if (*relerr > hyd->Hacc) return 0;

            // Find largest head loss error and absolute flow change
            checkhydbalance(pr, hbal);
            if ((SCALER)pr->report.Statflag == FULL)
            {
                reporthydbal(pr, hbal);
            }

            // Check that head loss error and flow change criteria are met
            if (hyd->HeadErrorLimit > 0.0_ft &&
                hbal->maxheaderror > hyd->HeadErrorLimit) return 0;
            if (hyd->FlowChangeLimit > 0.0_cfs &&
                hbal->maxflowchange > hyd->FlowChangeLimit) return 0;

            // Check for pressure driven analysis convergence
            if (hyd->DemandModel == PDA) return pdaconverged(pr);
            return 1;
        };

        void     SaveResultsForTimeStep(EN_Project const& pr, HydraulicSimulationQuality simQuality) {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            u64 currentTime = (u64)pr->times.GetCurrentRealHtime();
            units::time::second_t t(0);
            if (simQuality == HydraulicSimulationQuality::HIGHESTRES) {
                t = 0; // units::math::fmod(time->Htime, time->Rstep);
            }
            else if (simQuality == HydraulicSimulationQuality::HIGHRES) {
                t = units::math::fmod(time->Htime, time->Rstep_JunctionsPipes);
            }
            else {
                t = units::math::fmod(time->Htime, time->Rstep_JunctionsPipes * 4);
            }
            asset_t A_t;

            auto& nodes = net->Node;
            auto& links = net->Link;
            auto numN = net->Nnodes;
            for (auto i = 1; i <= numN; i++) {
                auto& obj = nodes[i];
                if (obj) {
                    A_t = obj->Type_p;
                    if ((t == 0_s) || A_t == asset_t::RESERVOIR) {
                        AUTO pat = obj->GetValue<_HEAD_>();
                        if (pat) {
                            pat->AddUniqueValue(currentTime, hyd->NodeHead[i]);
                            pat->CompressLastValueAdded();
                        }
                    }
                    if ((t == 0_s) || A_t == asset_t::RESERVOIR) {
                        AUTO pat = obj->GetValue<_DEMAND_>();
                        if (pat) {
                            pat->AddUniqueValue(currentTime, hyd->NodeDemand[i]);
                            pat->CompressLastValueAdded();
                        }
                    }
                }
            }
            numN = net->Nlinks;
            
#if 0
            fibers::parallel::For(1, numN, [&links, &currentTime, &hyd, &t, &pr, &net](int i) {
                auto& obj = links[i];
                if (obj) {
                    auto& A_t = obj->Type_p;
                    if (A_t == asset_t::PIPE || A_t == asset_t::PUMP || A_t == asset_t::VALVE) {
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_FLOW_>();
                            if (pat) {
                                pat->AddUniqueValue(currentTime, hyd->LinkFlow[i]);
                                pat->CompressLastValueAdded();
                            }
                        }
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_VELOCITY_>();
                            if (pat) {
                                if (hyd->LinkStatus[i] <= CLOSED) {
                                    pat->AddUniqueValue(currentTime, 0_fps);
                                }
                                else {
                                    pat->AddUniqueValue(currentTime, hyd->LinkFlow[i] / obj->Area());
                                }
                                pat->CompressLastValueAdded();
                            }
                        }
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_HEADLOSS_>();
                            if (pat) {
                                auto h = hyd->NodeHead[obj->N1] - hyd->NodeHead[obj->N2];
                                if (obj->Type != PUMP) h = ABS(h);
                                pat->AddUniqueValue(currentTime, h);
                                pat->CompressLastValueAdded();
                            }
                        }
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_STATUS_>();
                            if (pat) {
                                pat->AddUniqueValue(currentTime, (double)hyd->LinkStatus[i]);
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                    if (A_t == asset_t::PUMP || A_t == asset_t::VALVE) {
                        {
                            AUTO pat = obj->GetValue<_SETTING_>();
                            if (pat) {
                                if (hyd->LinkSetting[i] == MISSING) pat->AddUniqueValue(currentTime, 0);
                                else {
                                    switch (obj->Type)
                                    {
                                    case PRV:
                                    case PSV:
                                    case PBV:
                                        // pressure setting, internally set as 'head' above elevation
                                        pat->AddUniqueValue(currentTime, (double)(pounds_per_square_inch_t)(head_t)(double)hyd->LinkSetting[i]);
                                        break;
                                    case FCV:
                                        // flow setting - internally set as cfs
                                        pat->AddUniqueValue(currentTime, (double)(cubic_foot_per_second_t)(double)hyd->LinkSetting[i]);
                                        break;
                                    default:
                                        pat->AddUniqueValue(currentTime, (double)hyd->LinkSetting[i]);
                                        break;
                                    }
                                }
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                    if (A_t == asset_t::PUMP) {
                        {
                            AUTO pat = obj->GetValue<_ENERGY_>();
                            if (pat) {
                                kilowatt_t kw;
                                SCALER eff;

                                getenergy(pr, i, &kw, &eff);
                                pat->AddUniqueValue(currentTime, kw);
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                    if (A_t == asset_t::VALVE) {
                        {
                            if (AUTO valve = net->Valve[findvalve(net, i)]) {
                                AUTO pat = obj->GetValue<_ENERGY_>();
                                if (valve->ProducesElectricity) {
                                    kilowatt_t kw;
                                    SCALER eff;
                                    getenergy(pr, i, &kw, &eff);
                                    pat->AddUniqueValue(currentTime, -kw);

                                    // pat->AddUniqueValue(currentTime, -1.0 * valve->energy_generation_potential(hyd->LinkFlow[i], obj->GetCurrentValue<_HEADLOSS_>(currentTime)));
                                }
                                else {
                                    pat->AddUniqueValue(currentTime, 0_kW);
                                }
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                }
            });
#else
            for (auto i = 1; i <= numN; i++) {
                auto& obj = links[i];
                if (obj) {
                    A_t = obj->Type_p;
                    if (A_t == asset_t::PIPE || A_t == asset_t::PUMP || A_t == asset_t::VALVE) {
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_FLOW_>();
                            if (pat) {
                                pat->AddUniqueValue(currentTime, hyd->LinkFlow[i]);
                                pat->CompressLastValueAdded();
                            }
                        }
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_VELOCITY_>();
                            if (pat) {
                                if (hyd->LinkStatus[i] <= CLOSED) {
                                    pat->AddUniqueValue(currentTime, 0_fps);
                                }
                                else {
                                    pat->AddUniqueValue(currentTime, hyd->LinkFlow[i] / obj->Area());
                                }
                                pat->CompressLastValueAdded();
                            }
                        }
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_HEADLOSS_>();
                            if (pat) {
                                auto h = hyd->NodeHead[obj->N1] - hyd->NodeHead[obj->N2];
                                if (obj->Type != PUMP) h = ABS(h);
                                pat->AddUniqueValue(currentTime, h);
                                pat->CompressLastValueAdded();
                            }
                        }
                        if ((t == 0_s) || A_t != asset_t::PIPE) {
                            AUTO pat = obj->GetValue<_STATUS_>();
                            if (pat) {
                                pat->AddUniqueValue(currentTime, (double)hyd->LinkStatus[i]);
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                    if (A_t == asset_t::PUMP || A_t == asset_t::VALVE) {
                        {
                            AUTO pat = obj->GetValue<_SETTING_>();
                            if (pat) {
                                if (hyd->LinkSetting[i] == MISSING) pat->AddUniqueValue(currentTime, 0);
                                else {
                                    switch (obj->Type)
                                    {
                                    case PRV:
                                    case PSV:
                                    case PBV:
                                        // pressure setting, internally set as 'head' above elevation
                                        pat->AddUniqueValue(currentTime, (double)(pounds_per_square_inch_t)(head_t)(double)hyd->LinkSetting[i]);
                                        break;
                                    case FCV:
                                        // flow setting - internally set as cfs
                                        pat->AddUniqueValue(currentTime, (double)(cubic_foot_per_second_t)(double)hyd->LinkSetting[i]);
                                        break;
                                    default:
                                        pat->AddUniqueValue(currentTime, (double)hyd->LinkSetting[i]);
                                        break;
                                    }
                                }
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                    if (A_t == asset_t::PUMP) {
                        {
                            AUTO pat = obj->GetValue<_ENERGY_>();
                            if (pat) {
                                kilowatt_t kw;
                                SCALER eff;

                                getenergy(pr, i, &kw, &eff);
                                pat->AddUniqueValue(currentTime, kw);
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                    if (A_t == asset_t::VALVE) {
                        {
                            if (AUTO valve = net->Valve[findvalve(net, i)]) {
                                AUTO pat = obj->GetValue<_ENERGY_>();
                                if (valve->ProducesElectricity) {
                                    kilowatt_t kw;
                                    SCALER eff;
                                    getenergy(pr, i, &kw, &eff);
                                    pat->AddUniqueValue(currentTime, -kw);

                                    // pat->AddUniqueValue(currentTime, -1.0 * valve->energy_generation_potential(hyd->LinkFlow[i], obj->GetCurrentValue<_HEADLOSS_>(currentTime)));
                                }
                                else {
                                    pat->AddUniqueValue(currentTime, 0_kW);
                                }
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                }
            }
#endif
            // on the reporting timestep, evaluate the pressure zones: 
            if (t == 0_s) {
                for (auto& obj : net->Zone) {
                    if (obj) {
                        bool hasWaterDemand = obj->HasWaterDemand();
                        // sum of all customer demands
                        cubic_foot_per_second_t demand = 0; // we will approximate the demand using the reservoirs and inflow/outflows. 
                        cubic_foot_per_second_t flowrate = 0; // Flowrate is the next inflow or net outflow. I.e. NOT including the reservoirs. 
                        foot_t head = 0; int numSamples = 0;

                        for (auto& link : obj->Boundary_Link) {
                            switch (link.second) {
                            case direction_t::FLOW_WITHIN_DMA: break;
                            case direction_t::FLOW_IN_DMA:
                                demand += link.first->GetCurrentValue<_FLOW_>(currentTime);
                                break;
                            case direction_t::FLOW_OUT_DMA:
                                demand -= link.first->GetCurrentValue<_FLOW_>(currentTime);
                                break;
                            }
                        }
                        flowrate = demand;
                        for (auto& node : obj->Node) {
                            if (!hasWaterDemand || node->Type_p == asset_t::RESERVOIR || node->HasWaterDemand())
                                cweeMath::rollingAverageRef(head, node->GetCurrentValue<_HEAD_>(currentTime), numSamples);
                            if (node->Type_p == asset_t::RESERVOIR) demand -= node->GetCurrentValue<_DEMAND_>(currentTime);
                        }
                        {
                            AUTO pat = obj->GetValue<_DEMAND_>();
                            if (pat) {
                                pat->AddUniqueValue(currentTime, demand);
                                pat->CompressLastValueAdded();
                            }
                        }
                        {
                            AUTO pat = obj->GetValue<_HEAD_>();
                            if (pat) {
                                pat->AddUniqueValue(currentTime, head);
                                pat->CompressLastValueAdded();
                            }
                        }
                        {
                            AUTO pat = obj->GetValue<_FLOW_>();
                            if (pat) {
                                pat->AddUniqueValue(currentTime, flowrate);
                                pat->CompressLastValueAdded();
                            }
                        }
                    }
                }
            }

            net->System->GetValue<_DEMAND_>()->AddUniqueValue(currentTime, hyd->Dsystem);
            net->System->GetValue<_ENERGY_>()->AddUniqueValue(currentTime, hyd->Etotal);
        };
        int      hydsolve(EN_Project const& pr, int* iter, SCALER* relerr, HydraulicSimulationQuality simQuality)
            /*
            **-------------------------------------------------------------------
            **  Input:   none
            **  Output:  *iter   = # of iterations to reach solution
            **           *relerr = convergence error in solution
            **           returns error code
            **  Purpose: solves network nodal equations for heads and flows
            **           using Todini's Gradient algorithm
            **
            **  Notes:   Status checks on CVs, pumps and pipes to tanks are made
            **           every CheckFreq iteration, up until MaxCheck iterations
            **           are reached. Status checks on control valves are made
            **           every iteration if DampLimit = 0 or only when the
            **           convergence error is at or below DampLimit. If DampLimit
            **           is > 0 then future computed flow changes are only 60% of
            **           their full value. A complete status check on all links
            **           is made when convergence is achieved. If convergence is
            **           not achieved in MaxIter trials and ExtraIter > 0 then
            **           another ExtraIter trials are made with no status changes
            **           made to any links and a warning message is generated.
            **
            **   This procedure calls linsolve() which appears in SMATRIX.C.
            **-------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;
            Report* rpt = &pr->report;

            int    i;                     // Node index
            int    errcode = 0;           // Node causing solution error
            int    nextcheck;             // Next status check trial
            int    maxtrials;             // Max. trials for convergence
            SCALER newerr;                // New convergence error
            int    valveChange;           // Valve status change flag
            int    statChange;            // Non-valve status change flag
            Hydbalance hydbal;            // Hydraulic balance errors
            cubic_foot_per_second_t fullDemand;            // Full demand for a node (cfs)

            // Initialize status checking & relaxation factor
            nextcheck = hyd->CheckFreq;
            hyd->RelaxFactor = 1.0;

            // Initialize convergence criteria and PDA results
            hydbal.maxheaderror = 0.0_ft;
            hydbal.maxflowchange = 0.0_cfs;
            hyd->DeficientNodes = 0;
            hyd->DemandReduction = 0.0;

            // Repeat iterations until convergence or trial limit is exceeded.
            // (ExtraIter used to increase trials in case of status cycling.)
            if ((SCALER)rpt->Statflag == FULL) writerelerr(pr, 0, 0);
            maxtrials = hyd->MaxIter;
            if (hyd->ExtraIter > 0) maxtrials += hyd->ExtraIter;
            *iter = 1;
            while (*iter <= maxtrials)
            {
                // Compute coefficient matrices A & F and solve A*H = F
                // where H = heads, A = Jacobian coeffs. derived from
                // head loss gradients, & F = flow correction terms.
                // Solution for H is returned in F from call to linsolve().
                headlosscoeffs(pr);
                matrixcoeffs(pr);
                errcode = smatrix_t::linsolve(sm, net->Njuncs);
                // Matrix ill-conditioning problem - if control valve causing problem,
                // fix its status & continue, otherwise quit with no solution.
                if (errcode > 0)
                {
                    if (badvalve(pr, sm->Order[errcode])) continue;
                    else break;
                }

                // Update current solution.
                // (Row[i] = row of solution matrix corresponding to node i)
                for (i = 1; i <= net->Njuncs; i++)
                {
                    hyd->NodeHead[i] = sm->B_ft[sm->Row[i]];   // Update heads
                }
                newerr = newflows(pr, &hydbal);             // Update flows
                *relerr = newerr;

                // Write convergence error to status report if called for
                if ((SCALER)rpt->Statflag == FULL)
                {
                    writerelerr(pr, *iter, *relerr);
                }

                // Apply solution damping & check for change in valve status
                hyd->RelaxFactor = 1.0;
                valveChange = FALSE;
                if (hyd->DampLimit > 0.0)
                {
                    if (*relerr <= hyd->DampLimit)
                    {
                        hyd->RelaxFactor = 0.6;
                        valveChange = calc_and_set_prv_and_psv_status(pr);
                    }
                }
                else
                {
                    valveChange = calc_and_set_prv_and_psv_status(pr);
                }

                // Check for convergence
                if (hasconverged(pr, relerr, &hydbal))
                {
                    // We have convergence - quit if we are into extra iterations
                    if (*iter > hyd->MaxIter) break;

                    // Quit if no status changes occur
                    statChange = FALSE;
                    if (valveChange)    statChange = TRUE;
                    if (linkstatus(pr)) statChange = TRUE;
                    if (pswitch(pr))    statChange = TRUE;
                    if (!statChange)    break;

                    // We have a status change so continue the iterations
                    nextcheck = *iter + hyd->CheckFreq;
                }

                // No convergence yet - see if its time for a periodic status
                // check  on pumps, CV's, and pipes connected to tank
                else if (*iter <= hyd->MaxCheck && *iter == nextcheck)
                {
                    linkstatus(pr);
                    nextcheck += hyd->CheckFreq;
                }
                (*iter)++;
            }

            // Iterations ended - report any errors.
            if (errcode > 0)
            {
                writehyderr(pr, sm->Order[errcode]);    // Ill-conditioned matrix error
                errcode = 110;
            }

            // Store actual junction outflow in NodeDemand & full demand in DemandFlow
            for (i = 1; i <= net->Njuncs; i++)
            {
                fullDemand = hyd->NodeDemand[i];
                hyd->NodeDemand[i] = hyd->DemandFlow[i] + hyd->EmitterFlow[i];
                hyd->DemandFlow[i] = fullDemand;
            }

            // Save the simulation data for this timestep            
            SaveResultsForTimeStep(pr, simQuality);

            // Save convergence info
            hyd->RelativeError = *relerr;
            hyd->MaxHeadError = hydbal.maxheaderror;
            hyd->MaxFlowChange = hydbal.maxflowchange;
            hyd->Iterations = *iter;
            return errcode;
        };

        int      openhyd(EN_Project const& pr)
            /*
                *--------------------------------------------------------------
                *  Input:   none
                *  Output:  returns error code
                *  Purpose: opens hydraulics solver system
                *--------------------------------------------------------------
            */
        {
            int  i;
            int  errcode = 0;
            Plink link;

            // Check for too few nodes & no fixed grade nodes
            if (pr->network->Nnodes < 2) errcode = 223;
            else if (pr->network->Ntanks == 0) errcode = 224;

            // Allocate memory for sparse matrix structures (see SMATRIX.C)
            ERRCODE(smatrix_t::createsparse(pr));

            // Allocate memory for hydraulic variables
            ERRCODE(allocmatrix(pr));

            // Check for unconnected nodes
            ERRCODE(unlinked(pr));

            // Initialize link flows
            if (!errcode) for (i = 1; i <= pr->network->Nlinks; i++)
            {
                link = pr->network->Link[i];
                initlinkflow(pr, i, (StatusType)(double)link->Status(pr->times.GetSimStartTime()), link->Kc);
            }
            return errcode;
        };
        void     inithyd(EN_Project const& pr, int initflag)
            /*
            **--------------------------------------------------------------
            **  Input:   initflag > 0 if link flows should be re-initialized
            **                    = 0 if not
            **  Output:  none
            **  Purpose: initializes hydraulics solver system
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Outfile* out = &pr->outfile;
            Times* time = &pr->times;

            int i;
            Ptank tank;
            Plink link;
            Ppump pump;

            // Initialize tanks
            for (i = 1; i <= net->Ntanks; i++)
            {
                tank = net->Tank[i];
                hyd->TankVolume[i] = tank->Volume(pr, tank->InitialHead(pr->times.GetSimStartTime()));
                hyd->NodeHead[tank->Node] = tank->InitialHead(pr->times.GetSimStartTime());
                hyd->NodeDemand[tank->Node] = 0.0_cfs;
                hyd->OldStatus[net->Nlinks + i] = TEMPCLOSED;
            }

            // Initialize emitter flows
            for (auto& x : hyd->EmitterFlow) x = 0;
            for (i = 1; i <= net->Nnodes; i++)
            {
                net->Node[i]->ResultIndex = i;
                if (net->Node[i]->Ke > 0.0) hyd->EmitterFlow[i] = 1.0_cfs;
            }

            // Initialize links
            for (i = 1; i <= net->Nlinks; i++)
            {
                link = net->Link[i];
                link->ResultIndex = i;

                // Initialize status and setting
                hyd->LinkStatus[i] = (StatusType)(double)link->Status(pr->times.GetSimStartTime());
                hyd->LinkSetting[i] = link->Kc;

                // Compute flow resistance
                resistcoeff(pr, i);

                // Start active control valves in ACTIVE position
                if (
                    (link->Type == PRV || link->Type == PSV
                        || link->Type == FCV) && (link->Kc != MISSING)
                    ) hyd->LinkStatus[i] = ACTIVE;

                // Initialize flows if necessary
                if (hyd->LinkStatus[i] <= CLOSED)
                {
                    hyd->LinkFlow[i] = QZERO;
                }
                else if (ABS(hyd->LinkFlow[i]) <= QZERO || initflag > 0)
                {
                    initlinkflow(pr, i, hyd->LinkStatus[i], hyd->LinkSetting[i]);
                }

                // Save initial status
                hyd->OldStatus[i] = hyd->LinkStatus[i];
            }

            // Initialize pump energy usage
            for (i = 1; i <= net->Npumps; i++)
            {
                pump = net->Pump[i];
                pump->Energy.Efficiency = 0.0;
                pump->Energy.TimeOnLine = 0.0_s;
                pump->Energy.KwHrs = 0.0_kWh;
                pump->Energy.KwHrsPerFlow = 0.0;
                pump->Energy.MaxKwatts = 0.0_kW;
                pump->Energy.TotalCost = 0.0_USD;
                pump->Energy.CurrentPower = 0.0_kW;
                pump->Energy.CurrentEffic = 0.0;
            }

            // Re-position hydraulics file
            if (pr->outfile.Saveflag)
            {
                fseek(out->HydFile, out->HydOffset, SEEK_SET);
            }

            // Initialize current time
            hyd->Haltflag = 0;
            time->Htime = 0;
            time->Hydstep = 0;
            time->Rtime = time->Rstep;
            time->Rtime_JunctionsPipes = time->Rstep_JunctionsPipes;

        };
        int      runhyd(EN_Project const& pr, units::time::second_t* t, HydraulicSimulationQuality simQuality)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  t = pointer to current time (in seconds)
            **  Returns: error code
            **  Purpose: solves network hydraulics in a single time period
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Times* time = &pr->times;
            Report* rpt = &pr->report;

            int   iter;          // Iteration count
            int   errcode;       // Error code
            SCALER relerr;       // Solution accuracy

            // Find new demands & control actions
            *t = time->Htime;
            demands(pr);
            controls(pr);

            // Solve network hydraulic equations
            errcode = hydsolve(pr, &iter, &relerr, simQuality);
            if (!errcode)
            {
                // Report new status & save results
                if (rpt->Statflag) writehydstat(pr, iter, relerr);

                // If system unbalanced and no extra trials
                // allowed, then activate the Haltflag
                if (relerr > hyd->Hacc && hyd->ExtraIter == -1)
                {
                    hyd->Haltflag = 1;
                }

                // Report any warning conditions
                if (!errcode) errcode = writehydwarn(pr, iter, relerr);
            }
            return errcode;
        };
        int      nexthyd(EN_Project const& pr, units::time::second_t* tstep)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  tstep = pointer to time step (in seconds)
            **  Returns: error code
            **  Purpose: finds length of next time step & updates tank
            **           levels and rule-based contol actions
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Times* time = &pr->times;

            units::time::second_t  hydstep;         // Actual time step
            int   errcode = 0;     // Error code

            // Compute current power and efficiency of all pumps
            getallpumpsenergy(pr);

            // Save current results to hydraulics file and
            // force end of simulation if Haltflag is active
            if (pr->outfile.Saveflag) errcode = savehyd(pr, &time->Htime);
            if (hyd->Haltflag) time->Htime = time->Dur;

            // Compute next time step & update tank levels
            *tstep = 0;
            hydstep = 0;
            if (time->Htime < time->Dur) hydstep = timestep(pr);
            if (pr->outfile.Saveflag) errcode = savehydstep(pr, &hydstep);

            // Accumulate pumping energy
            if (time->Dur == 0_s) addenergy(pr, 0);
            else if (time->Htime < time->Dur) addenergy(pr, hydstep);

            // More time remains - update current time
            if (time->Htime < time->Dur)
            {
                time->Htime += hydstep;
                if (!pr->quality.OpenQflag)
                {
                    if (time->Htime >= time->Rtime) time->Rtime += time->Rstep;
                    if (time->Htime >= time->Rtime_JunctionsPipes) time->Rtime_JunctionsPipes += time->Rstep_JunctionsPipes;
                }
            }

            // No more time remains - force completion of analysis
            else
            {
                time->Htime++;
                if (pr->quality.OpenQflag) time->Qtime++;
            }
            *tstep = hydstep;
            return errcode;
        };
        void     closehyd(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: closes hydraulics solver system
            **--------------------------------------------------------------
            */
        {
            smatrix_t::freesparse(pr);
            freematrix(pr);

            for (auto& x : pr->network->Asset) if (x) x->ReduceMemoryOfValues();
        };
        void     freematrix(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: frees memory used for solution matrix coeffs.
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            hyd->P.Clear();
            hyd->Y.Clear();
            hyd->Xflow.Clear();
            hyd->OldStatus.Clear();
        };
        int      allocmatrix(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: allocates memory used for solution matrix coeffs.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            freematrix(pr);

            hyd->P.AssureSize(net->Nlinks + 1);
            hyd->Y.AssureSize(net->Nlinks + 1);
            hyd->Xflow.AssureSize(fMAX((net->Nnodes + 1), (net->Nlinks + 1)));
            hyd->OldStatus.AssureSize(net->Nlinks + net->Ntanks + 1);

            return 0;
        };

        void     initlinkflow(EN_Project const& pr, int i, char s, SCALER k)
            /*
            **--------------------------------------------------------------------
            **  Input:   i = link index
            **           s = link status
            **           k = link setting (i.e., pump speed)
            **  Output:  none
            **  Purpose: sets initial flow in link to QZERO if link is closed,
            **           to design flow for a pump, or to flow at velocity of
            **           1 fps for other links.
            **--------------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            EN_Network n = pr->network;

            Plink link = n->Link[i];

            if (s == CLOSED)
            {
                hyd->LinkFlow[i] = QZERO;
            }
            else if (link->Type == PUMP)
            {
                hyd->LinkFlow[i] = k * n->Pump[findpump(n, i)]->InitialFlow(pr->times.GetSimStartTime());
            }
            else
            {
                hyd->LinkFlow[i] = (PI * SQR(link->Diam) / 4.0) * 1_fps;
            }
        };
        void     demands(EN_Project const& pr)
            /*
            **--------------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: computes demands at nodes during current time period
            **--------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Times* time = &pr->times;

            int  i, j, n;
            cubic_foot_per_second_t djunc;
            cubic_foot_per_second_t sum;

            // Determine total elapsed number of pattern periods
            AUTO t = (u64)time->GetCurrentRealHtime();
            // Update demand at each node according to its assigned pattern
            hyd->Dsystem = 0.0_cfs;          // System-wide demand
            int nN = net->Njuncs;
            auto& nodes = net->Node;
            for (n = 1; n <= nN; n++) {
                sum = 0.0_cfs;
                for (auto& demand : nodes[n]->D) {
                    // pattern period (k) = (elapsed periods) modulus (periods per pattern)
                    if (demand.TimePat) {
                        djunc = demand.Base;
                        djunc *= hyd->Dmult;
                        djunc *= demand.TimePat->Pat.GetCurrentValue(t);
                        if (djunc > 0.0_cfs) hyd->Dsystem += djunc;
                        sum += djunc;
                    }
                }
                hyd->NodeDemand[n] = sum;

                // Initialize pressure dependent demand
                hyd->DemandFlow[n] = sum;
            }

            // Update head at fixed grade nodes with time patterns
            nN = net->Ntanks;
            for (n = 1; n <= nN; n++)
            {
                Ptank tank = net->Tank[n];
                if (tank->Diameter == 0.0_ft)
                {
                    if (tank->TimePat) {
                        i = tank->Node;
                        hyd->NodeHead[i] = tank->El * tank->TimePat->Pat((u64)time->GetCurrentRealHtime());
                    }
                }
            }

            // Update status of pumps with utilization patterns
            nN = net->Npumps;
            for (n = 1; n <= nN; n++)
            {
                Ppump pump = net->Pump[n];
                if (pump->TimeUpat) {
                    i = pump->Link;
                    setlinksetting(pr, i, pump->TimeUpat->Pat((u64)time->GetCurrentRealHtime()), &hyd->LinkStatus[i], &hyd->LinkSetting[i]);
                }
            }
        };
        int      controls(EN_Project const& pr)
            /*
            **---------------------------------------------------------------------
            **  Input:   none
            **  Output:  number of links whose setting changes
            **  Purpose: implements simple controls based on time or tank levels
            **---------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Times* time = &pr->times;

            int i, k, n, reset, setsum;
            SCALER h, vplus;
            SCALER v1, v2;
            SCALER k1, k2;
            char  s1, s2;
            Plink link;
            Pcontrol control;

            // Examine each control statement
            setsum = 0;
            for (i = 1; i <= net->Ncontrols; i++)
            {
                // Make sure that link is defined
                control = net->Control[i];
                reset = 0;
                if ((k = control->Link) <= 0) continue;
                link = net->Link[k];

                // Link is controlled by tank level
                if ((n = control->Node) > 0 && n > net->Njuncs)
                {
                    h = (double)hyd->NodeHead[n];
                    vplus = (double)ABS(hyd->NodeDemand[n]);
                    v1 = (double)tankvolume(pr, n - net->Njuncs, (foot_t)(double)h);
                    v2 = (double)tankvolume(pr, n - net->Njuncs, (foot_t)(double)control->Grade);
                    if (control->Type == LOWLEVEL && v1 <= v2 + vplus) reset = 1;
                    if (control->Type == HILEVEL && v1 >= v2 - vplus)  reset = 1;
                }

                // Link is time-controlled
                if (control->Type == TIMER)
                {
                    if (control->Time_Adjusted(pr) == time->GetCurrentRealHtime()) reset = 1;
                }

                //* Link is time-of-day controlled
                if (control->Type == TIMEOFDAY)
                {
                    if (units::math::fmod((SCALER)(double)(time->Htime + time->Tstart), SECperDAY) == (SCALER)(double)control->Time) {
                        reset = 1;
                    }
                }

                // Update link status & pump speed or valve setting
                if (reset == 1)
                {
                    if (hyd->LinkStatus[k] <= CLOSED) s1 = CLOSED;
                    else s1 = OPEN;
                    s2 = control->Status;
                    k1 = hyd->LinkSetting[k];
                    k2 = k1;
                    if (link->Type > PIPE) k2 = control->Setting;

                    // Check if a re-opened pump needs its flow reset
                    if (link->Type == PUMP && s1 == CLOSED && s2 == OPEN)
                        resetpumpflow(pr, k);

                    if (s1 != s2 || k1 != k2)
                    {
                        hyd->LinkStatus[k] = (StatusType)s2;
                        hyd->LinkSetting[k] = k2;
                        if (pr->report.Statflag) writecontrolaction(pr, k, i);
                        setsum++;
                    }
                }
            }
            return setsum;
        };
        units::time::second_t timestep(EN_Project const& pr)
            /*
            **----------------------------------------------------------------
            **  Input:   none
            **  Output:  returns time step until next change in hydraulics
            **  Purpose: computes time step to advance hydraulic simulation
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Times* time = &pr->times;

            SCALER n;
            units::time::second_t t;
            units::time::second_t tstep;

            // Normal time step is hydraulic time step
            tstep = time->Hstep;

            // Revise time step based on time until next demand period
            // (n = next pattern period, t = time till next period)
            units::time::second_t s = time->Htime + (units::time::second_t)time->Pstart;
            AUTO m = s / time->Pstep;
            n = (SCALER)(m)+(SCALER)1;
            t = n * time->Pstep - time->Htime;
            if (t > 0_s && t < tstep) tstep = t;

            // Revise time step based on time until next reporting period
            t = time->Rtime - time->Htime;
            // t = time->Rtime_JunctionsPipes - time->Htime;

            if (t > 0_s && t < tstep) tstep = t;

            // Revise time step based on smallest time to fill or drain a tank
            tanktimestep(pr, &tstep);

            // Revise time step based on smallest time to activate a control
            controltimestep(pr, &tstep);

            // Evaluate rule-based controls (which will also update tank levels)
            if (net->Nrules > 0) ruletimestep(pr, &tstep);
            else tanklevels(pr, tstep);
            return tstep;
        };
        int      tanktimestep(EN_Project const& pr, units::time::second_t* tstep)
            /*
            **-----------------------------------------------------------------
            **  Input:   *tstep = current time step
            **  Output:  *tstep = modified current time step
            **  Purpose: revises time step based on shortest time to fill or
            **           drain a tank
            **-----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int     i, n, tankIdx = 0;
            SCALER  h, q, v;
            units::time::second_t    t;
            Ptank tank;

            // Examine each tank
            for (i = 1; i <= net->Ntanks; i++)
            {
                // Skip reservoirs
                tank = net->Tank[i];
                if (tank->Diameter == 0.0_ft) continue;

                // Get current tank grade (h) & inflow (q)
                n = tank->Node;
                h = (double)hyd->NodeHead[n];
                q = (double)hyd->NodeDemand[n];
                if ((cubic_foot_per_second_t)(double)ABS(q) <= QZERO) continue;

                // Find volume to fill/drain tank
                if (q > 0.0 && (foot_t)(double)h < tank->Hmax) v = (double)(tank->Vmax(pr) - hyd->TankVolume[i]);
                else if (q < 0.0 && (foot_t)(double)h > tank->Hmin) v = (double)(tank->Vmin(pr) - hyd->TankVolume[i]);
                else continue;

                // Find time to fill/drain tank
                t = (double)ROUND(v / q);
                if (t > 0_s && t < *tstep)
                {
                    *tstep = t;
                    tankIdx = n;
                }
            }
            return tankIdx;
        };
        void     controltimestep(EN_Project const& pr, units::time::second_t* tstep)
            /*
            **------------------------------------------------------------------
            **  Input:   *tstep = current time step
            **  Output:  *tstep = modified current time step
            **  Purpose: revises time step based on shortest time to activate
            **           a simple control
            **------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int    i, j, k, n;
            SCALER h, q, v;
            units::time::second_t   t, t1, t2;
            Plink link;
            Pcontrol control;

            // Examine each control
            for (i = 1; i <= net->Ncontrols; i++)
            {
                t = 0;
                control = net->Control[i];

                // Control depends on a tank level
                if ((n = control->Node) > 0)
                {
                    // Skip node if not a tank or reservoir
                    if ((j = n - net->Njuncs) <= 0) continue;

                    // Find current head and flow into tank
                    h = (double)hyd->NodeHead[n];
                    q = (double)hyd->NodeDemand[n];
                    if ((cubic_foot_per_second_t)(double)ABS(q) <= QZERO) continue;

                    // Find time to reach upper or lower control level
                    if ((h < control->Grade && control->Type == HILEVEL && q > 0.0)
                        || (h > control->Grade && control->Type == LOWLEVEL && q < 0.0))
                    {
                        v = (double)(tankvolume(pr, j, (foot_t)(double)control->Grade) - hyd->TankVolume[j]);
                        t = (double)ROUND(v / q);
                    }
                }

                // Control is based on elapsed time
                if (control->Type == TIMER)
                {
                    if (control->Time_Adjusted(pr) > pr->times.GetCurrentRealHtime())
                    {
                        t = (u64)(control->Time_Adjusted(pr) - pr->times.GetCurrentRealHtime());
                    }
                }

                // Control is based on time of day
                if (control->Type == TIMEOFDAY)
                {
                    t1 = (double)units::math::fmod((SCALER)(double)(pr->times.Htime + pr->times.Tstart), SECperDAY);
                    // t1 = (long)(double)(pr->times.Htime + pr->times.Tstart) % (long)SECperDAY;
                    t2 = control->Time;
                    if (t2 >= t1) t = t2 - t1;
                    else          t = (units::time::second_t)(double)SECperDAY - t1 + t2;
                }

                // Revise the current estimated next time step
                if (t > 0_s && t < *tstep)
                {
                    // Check if rule actually changes link status or setting
                    k = control->Link;
                    link = net->Link[k];
                    if ((link->Type > PIPE && hyd->LinkSetting[k] != control->Setting)
                        || (hyd->LinkStatus[k] != control->Status)) *tstep = t;
                }
            }
        };
        void     ruletimestep(EN_Project const& pr, units::time::second_t* tstep)
            /*
            **--------------------------------------------------------------
            **  Input:   *tstep = current time step (sec)
            **  Output:  *tstep = modified time step
            **  Purpose: updates next time step by checking if any rules
            **           will fire before then; also updates tank levels.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Times* time = &pr->times;

            units::time::second_t tnow,      // Start of time interval for rule evaluation
                tmax,      // End of time interval for rule evaluation
                dt,        // Normal time increment for rule evaluation
                dt1;       // Actual time increment for rule evaluation

            // Find interval of time for rule evaluation
            tnow = time->Htime;
            tmax = tnow + *tstep;

            // If no rules, then time increment equals current time step
            if (net->Nrules == 0)
            {
                dt = *tstep;
                dt1 = dt;
            }

            // Otherwise, time increment equals rule evaluation time step and
            // first actual increment equals time until next even multiple of
            // Rulestep occurs.
            else
            {
                dt = time->Rulestep;
                dt1 = time->Rulestep - (units::time::second_t)((long)(double)tnow % (long)(double)time->Rulestep);
            }

            // Make sure time increment is no larger than current time step
            dt = fMIN(dt, *tstep);
            dt1 = fMIN(dt1, *tstep);
            if (dt1 == 0_s) dt1 = dt;

            // Step through time, updating tank levels, until either
            // a rule fires or we reach the end of evaluation period.
            //
            // Note: we are updating the global simulation time (Htime)
            //       here because it is used by functions in RULES.C
            //       to evaluate rules when checkrules() is called.
            //       It is restored to its original value after the
            //       rule evaluation process is completed (see below).
            //       Also note that dt1 will equal dt after the first
            //       time increment is taken.
            //
            do
            {
                time->Htime += dt1;                // Update simulation clock
                tanklevels(pr, dt1);                // Find new tank levels
                if (checkrules(pr, dt1)) break;     // Stop if any rule fires
                dt = fMIN(dt, tmax - time->Htime);  // Update time increment
                dt1 = dt;                           // Update actual increment
            } while (dt > 0_s);                       // Stop if no time left

            // Compute an updated simulation time step (*tstep)
            // and return simulation time to its original value
            *tstep = time->Htime - tnow;
            time->Htime = tnow;
        };
        void     addenergy(EN_Project const& pr, units::time::second_t hstep)
            /*
            **-------------------------------------------------------------
            **  Input:   hstep = time step (sec)
            **  Output:  none
            **  Purpose: accumulates pump energy usage
            **-------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Times* time = &pr->times;

            int    i, j, k;
            long   m, n;
            hour_t
                dt;               // Time interval (hr)

            Dollar_per_kilowatt_hour_t c0, c;  // Energy cost (cost/kwh)

            SCALER
                f0,               // Energy cost factor               
                e;                // Pump efficiency (fraction)

            cubic_foot_per_second_t
                q;                // Pump flow (cfs)              

            kilowatt_t
                p = 0.0_kW,                // Pump energy (kw)
                psum = 0.0_kW;       // Total energy (kw)

            Ppump pump;
            Pvalve valve;

            // Determine current time interval in hours
            if (time->Dur == 0_s) dt = 1.0_hr;
            else if (time->Htime < time->Dur)
            {
                dt = hstep;
            }
            else dt = 0.0_s;
            if (dt == 0.0_hr) return;
            // Compute default energy cost at current time
            c0 = hyd->Ecost;
            f0 = 1.0;
            if (hyd->Epat > 0)
            {
                f0 = net->Pattern[hyd->Epat]->Pat((u64)time->GetCurrentRealHtime());
            }

            // Examine each pump
            for (j = 1; j <= net->Npumps; j++)
            {
                // Skip closed pumps
                pump = net->Pump[j];
                k = pump->Link;
                if (hyd->LinkStatus[k] <= CLOSED) continue;
                q = fMAX(QZERO, ABS(hyd->LinkFlow[k]));

                // Find pump-specific energy cost
                if (pump->Ecost > (Dollar_per_kilowatt_hour_t)0.0) c = pump->Ecost;
                else c = c0;
                if (pump->TimeEpat)
                {
                    c *= pump->TimeEpat->Pat((u64)time->GetCurrentRealHtime());
                }
                else c *= f0;

                // Update pump's cumulative statistics
                p = pump->Energy.CurrentPower;
                e = pump->Energy.CurrentEffic;
                psum += p;
                pump->Energy.TimeOnLine += dt;
                pump->Energy.Efficiency += (double)(e * dt);
                pump->Energy.KwHrsPerFlow += (double)(p / q * dt);
                pump->Energy.KwHrs += p * dt;
                pump->Energy.MaxKwatts = fMAX(pump->Energy.MaxKwatts, p);
                pump->Energy.TotalCost += c * p * dt;
            }

            // Examine each valve
            for (j = 1; j <= net->Nvalves; j++)
            {
                valve = net->Valve[j];
                if (valve->ProducesElectricity) {
                    // Skip closed valves 
                    k = valve->Link;
                    if (hyd->LinkStatus[k] <= CLOSED) continue;
                    q = fMAX(QZERO, ABS(hyd->LinkFlow[k]));

                    // Find valve-specific energy cost
                    c = c0 * f0;

                    // Update valve's cumulative statistics
                    p = valve->Energy.CurrentPower;
                    e = valve->Energy.CurrentEffic;
                    psum -= p;
                    valve->Energy.TimeOnLine += dt;
                    valve->Energy.Efficiency += (double)(e * dt);
                    valve->Energy.KwHrsPerFlow = (double)(p / q * dt);
                    valve->Energy.KwHrs -= p * dt;
                    valve->Energy.MaxKwatts = fMAX(valve->Energy.MaxKwatts, p);
                    valve->Energy.TotalCost -= c * p * dt;
                }
            }

            // Update maximum kW value
            hyd->Emax = fMAX(hyd->Emax, psum);
            hyd->Etotal = psum;
        };
        void     getenergy(EN_Project const& pr, int k, kilowatt_t* kw, SCALER* eff)
            /*
            **----------------------------------------------------------------
            **  Input:   k    = link index
            **  Output:  *kw  = kwatt energy used
            **           *eff = efficiency (pumps only)
            **  Purpose: computes flow energy associated with link k
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int    i,       // efficiency curve index
                j;          // pump index
            foot_t dh;      // head across pump (ft)
            cubic_foot_per_second_t q; // flow through pump (cfs)
            SCALER e;       // pump efficiency
            SCALER q4eff;   // flow at nominal pump speed of 1.0
            SCALER speed;   // current speed setting
            Pcurve curve;
            Plink link = net->Link[k];

            // No energy if link is closed
            if (hyd->LinkStatus[k] <= CLOSED)
            {
                *kw = 0.0_kW;
                *eff = 0.0;
                return;
            }

            // Determine flow and head difference
            q = ABS(hyd->LinkFlow[k]);
            dh = ABS(hyd->NodeHead[link->N1] - hyd->NodeHead[link->N2]);

            // For pumps, find effic. at current flow
            if (link->Type == PUMP)
            {
                j = findpump(net, k);
                e = hyd->Epump;
                speed = hyd->LinkSetting[k];
                if ((i = net->Pump[j]->Ecurve) > 0)
                {
                    q4eff = (SCALER)(double)(q / speed * pr->Ucf[FLOW]);
                    curve = net->Curve[i];
                    e = curve->Curve.GetCurrentValue(q4eff);

                    // Sarbu and Borza pump speed adjustment
                    e = 100.0 - ((100.0 - e) * ::epanet::pow(1.0 / speed, 0.1));
                }
                e = fMIN(e, 100.0);
                e = fMAX(e, 1.0);
                e /= 100.0;
            }
            else if (link->Type >= PRV) {
                e = 1.31;
            }
            else { // Pipe?
                e = 1.0;
            }

            // Compute energy
            *kw = cweeEng::CentrifugalPumpEnergyDemand_kW(q, dh, e * 100.0) * hyd->SpGrav; // ((kilowatt_t)(double)(dh * q / 8.814 / e * KWperHP))* hyd->SpGrav;

            *eff = e;
        };
        void     getallpumpsenergy(EN_Project const& pr)
            /*
            **-------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: finds the current power and efficiency for each pump.
            **-------------------------------------------------------------
            */
        {
            int  j;
            Ppump pump;

            for (j = 1; j <= pr->network->Npumps; j++)
            {
                pump = pr->network->Pump[j];
                getenergy(pr, pump->Link, &pump->Energy.CurrentPower, &pump->Energy.CurrentEffic);
            }
            for (j = 1; j <= pr->network->Nvalves; j++)
            {
                AUTO valve = pr->network->Valve[j];
                if (valve->ProducesElectricity)
                    getenergy(pr, valve->Link, &valve->Energy.CurrentPower, &valve->Energy.CurrentEffic);
                else {
                    valve->Energy.CurrentPower = 0;
                    valve->Energy.CurrentEffic = 0;
                }
            }
        };
        void     tanklevels(EN_Project const& pr, units::time::second_t tstep)
            /*
            **----------------------------------------------------------------
            **  Input:   tstep = current time step
            **  Output:  none
            **  Purpose: computes new water levels in tanks after current
            **           time step
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;

            int    i, n;

            for (i = 1; i <= net->Ntanks; i++)
            {
                Ptank tank = net->Tank[i];
                if (tank->Diameter == 0.0_ft) continue;    // Skip reservoirs

                // Update the tank's volume & water elevation
                n = tank->Node;
                AUTO dv = hyd->NodeDemand[n] * tstep;
                hyd->TankVolume[i] += dv;

                // Check if tank full/empty within next second
                if (hyd->TankVolume[i] + (hyd->NodeDemand[n] * 1_s) >= tank->Vmax(pr))
                {
                    hyd->TankVolume[i] = tank->Vmax(pr);
                }
                else if (hyd->TankVolume[i] - (hyd->NodeDemand[n] * 1_s) <= tank->Vmin(pr))
                {
                    hyd->TankVolume[i] = tank->Vmin(pr);
                }

                hyd->NodeHead[n] = tankgrade(pr, i, hyd->TankVolume[i]);
            }
        };
        cubic_foot_t tankvolume(EN_Project const& pr, int i, foot_t h)
            /*
            **--------------------------------------------------------------------
            **  Input:   i = tank index
            **           h = water elevation in tank
            **  Output:  returns water volume in tank
            **  Purpose: finds water volume in tank i corresponding to elev. h.
            **--------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            int j;
            SCALER y, v;
            Ptank tank = net->Tank[i];
            Pcurve curve;

            // Use level*area if no volume curve
            j = tank->Vcurve;
            if (j == 0) return tank->Vmin(pr) + (h - tank->Hmin) * tank->Area();

            // If curve exists, interpolate on h to find volume v remembering that volume curve is in original units.
            else
            {
                curve = net->Curve[j];
                y = (double)(h - tank->El) * pr->Ucf[HEAD];
                return pr->convertToUnit<cubic_foot_t>(curve->Curve.GetCurrentValue(y));
            }
        };
        foot_t   tankgrade(EN_Project const& pr, int i, cubic_foot_t v)
            /*
            **-------------------------------------------------------------------
            **  Input:   i = tank index
            **           v = volume in tank
            **  Output:  returns water level in tank
            **  Purpose: finds water level in tank i corresponding to volume v.
            **-------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            int j;
            SCALER y;
            foot_t h;
            Ptank tank = net->Tank[i];

            // Use area if no volume curve
            j = tank->Vcurve;
            if (j == 0) return tank->Hmin + (v - tank->Vmin(pr)) / tank->Area();

            // If curve exists, interpolate on volume (originally the Y-variable but used here as the X-variable) to find new level above bottom. Remember that volume curve is stored in original units.
            else
            {
                Pcurve curve = net->Curve[j];
                // X are units of (original units) height and Y are units of (original units) volume
                y = curve->Curve.GetTimeForValue(((double)v * pr->Ucf[VOLUME]));
                h = tank->El + pr->convertToUnit<foot_t>(y);
                return h;
            }
        };
        SCALER   findsourcequal(EN_Project const& pr, int n, SCALER volout, units::time::second_t tstep)
            /*
            **---------------------------------------------------------------------
            **   Input:   n = node index
            **            volout = volume of node outflow over time step
            **            tstep = current quality time step
            **   Output:  returns concentration added by an external quality source.
            **   Purpose: computes contribution (if any) of mass addition from an
            **            external quality source at a node.
            **---------------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Times* time = &pr->times;

            SCALER massadded = 0.0, c;
            Psource source;

            // Sources only apply to CHEMICAL analyses
            if (qual->Qualflag == CHEM) {

                // Return 0 if node is not a quality source or has no outflow
                source = net->Node[n]->S;
                if (source == NULL)    return 0.0;
                if (source->Concentration == 0.0) return 0.0;
                if ((cubic_foot_per_second_t)(double)(volout / tstep) <= Q_STAGNANT) return 0.0;

                // Added source concentration depends on source type
                c = sourcequal(pr, source);
                switch (source->Type)
                {
                    // Concentration Source:
                case CONCEN:
                    if (net->Node[n]->Type == JUNCTION)
                    {
                        // ... source requires a negative demand at the node
                        if (hyd->NodeDemand[n] < 0.0_cfs)
                        {
                            c = -c * hyd->NodeDemand[n] * tstep / (cubic_foot_t)(double)volout;
                        }
                        else c = 0.0;
                    }
                    break;

                    // Mass Inflow Booster Source:
                case MASS:
                    // ... convert source input from mass/sec to concentration
                    c = c * (double)tstep / volout;
                    break;

                    // Setpoint Booster Source:
                    // Source quality is difference between source strength
                    // & node quality
                case SETPOINT:
                    c = fMAX(c - qual->NodeQual[n], 0.0);
                    break;

                    // Flow-Paced Booster Source:
                    // Source quality equals source strength
                case FLOWPACED:
                    break;
                }

                // Source mass added over time step = source concen. * outflow volume
                massadded = c * volout;

                // Update source's total mass added
                source->Smass += massadded;

                // Update Wsource
                if (time->Htime >= time->Rstart)
                {
                    qual->Wsource += massadded;
                }
                return c;
            }
            if (qual->Qualflag == ENERGYINTENSITY) {

                // Return 0 if node is not a quality source or has no outflow
                source = net->Node[n]->S;
                if (source == NULL)    return 0.0;
                if (source->Concentration == 0.0) return 0.0;
                if ((cubic_foot_per_second_t)(double)(volout / tstep) <= Q_STAGNANT) return 0.0;

                // Added source concentration depends on source type
                c = sourcequal(pr, source);
                switch (source->Type)
                {
                    // Concentration Source:
                case CONCEN:
                    if (net->Node[n]->Type == JUNCTION)
                    {
                        // ... source requires a negative demand at the node
                        if (hyd->NodeDemand[n] < 0.0_cfs)
                        {
                            c = -c * hyd->NodeDemand[n] * tstep / (cubic_foot_t)(double)volout;
                        }
                        else c = 0.0;
                    }
                    break;

                    // Mass Inflow Booster Source:
                case MASS:
                    // ... convert source input from mass/sec to concentration
                    c = c * (double)tstep / volout;
                    break;

                    // Setpoint Booster Source:
                    // Source quality is difference between source strength
                    // & node quality
                case SETPOINT:
                    c = fMAX(c - qual->NodeQual[n], 0.0);
                    break;

                    // Flow-Paced Booster Source:
                    // Source quality equals source strength
                case FLOWPACED:
                    break;
                }

                // Source mass added over time step = source concen. * outflow volume
                massadded = c * volout;

                // Update source's total mass added
                source->Smass += massadded;

                // Update Wsource
                if (time->Htime >= time->Rstart)
                {
                    qual->Wsource += massadded;
                }
                return c;
            }

            return 0.0;
        };
        void     reversesegs(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  none
            **   Purpose: re-orients a link's segments when flow reverses.
            **--------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;
            Pseg  seg, nseg, pseg;

            seg = qual->FirstSeg[k];
            qual->FirstSeg[k] = qual->LastSeg[k];
            qual->LastSeg[k] = seg;
            pseg = NULL;
            while (seg != NULL)
            {
                nseg = seg->prev;
                seg->prev = pseg;
                pseg = seg;
                seg = nseg;
            }
        };
        void     addseg(EN_Project const& pr, int k, SCALER v, SCALER c)
            /*
            **-------------------------------------------------------------
            **   Input:   k = segment chain index
            **            v = segment volume
            **            c = segment quality
            **   Output:  none
            **   Purpose: adds a segment to the start of a link
            **            upstream of its current last segment.
            **-------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;
            Pseg seg;

            // Grab the next free segment from the segment pool if available
            if (qual->FreeSeg != NULL)
            {
                seg = qual->FreeSeg;
                qual->FreeSeg = seg->prev;
            }

            // Otherwise allocate a new segment
            else
            {
                seg = (struct Sseg*)Mempool_t::mempool_alloc(qual->SegPool, sizeof(struct Sseg));
                if (seg == NULL)
                {
                    qual->OutOfMemory = TRUE;
                    return;
                }
            }

            // Assign volume and quality to the segment
            seg->v = v;
            seg->c = c;

            // Add the new segment to the end of the segment chain
            seg->prev = NULL;
            if (qual->FirstSeg[k] == NULL) qual->FirstSeg[k] = seg;
            if (qual->LastSeg[k] != NULL)  qual->LastSeg[k]->prev = seg;
            qual->LastSeg[k] = seg;
        };
        char     setreactflag(EN_Project const& pr)
            /*
            **-----------------------------------------------------------
            **   Input:   none
            **   Output:  returns 1 for reactive WQ constituent, 0 otherwise
            **   Purpose: checks if reactive chemical being simulated
            **-----------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            int i;

            if (pr->quality.Qualflag == ENERGYINTENSITY) return 0;
            if (pr->quality.Qualflag == TRACE_QUAL) return 0;
            else if (pr->quality.Qualflag == AGE)   return 1;
            else
            {
                for (i = 1; i <= net->Nlinks; i++)
                {
                    if (net->Link[i]->Type <= PIPE)
                    {
                        if (net->Link[i]->Kb != 0.0 || net->Link[i]->Kw != 0.0) return 1;
                    }
                }
                for (i = 1; i <= net->Ntanks; i++)
                {
                    if (net->Tank[i]->Kb != 0.0) return 1;
                }
            }
            return 0;
        };
        void     ratecoeffs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: determines wall reaction coeff. for each pipe
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int k;
            SCALER kw;

            for (k = 1; k <= net->Nlinks; k++)
            {
                kw = net->Link[k]->Kw;
                if (kw != 0.0)  kw = piperate(pr, k);
                net->Link[k]->Rc = kw;
                qual->PipeRateCoeff[k] = 0.0;
            }
        };
        void     reactpipes(EN_Project const& pr, units::time::second_t dt)
            /*
            **--------------------------------------------------------------
            **   Input:   dt = time step
            **   Output:  none
            **   Purpose: reacts water within each pipe over a time step.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int k;
            Pseg seg;
            SCALER cseg, rsum, vsum;

            // Examine each link in network
            for (k = 1; k <= net->Nlinks; k++)
            {
                // Skip non-pipe links (pumps & valves)
                if (net->Link[k]->Type != PIPE) continue;
                rsum = 0.0;
                vsum = 0.0;

                // Examine each segment of the pipe
                seg = qual->FirstSeg[k];
                while (seg != NULL)
                {
                    // React segment over time dt
                    cseg = seg->c;
                    seg->c = pipereact(pr, k, seg->c, seg->v, dt);

                    // Update reaction component of mass balance
                    qual->MassBalance.reacted += (cseg - seg->c) * seg->v;

                    // Accumulate volume-weighted reaction rate
                    if (qual->Qualflag == CHEM)
                    {
                        rsum += ABS(seg->c - cseg) * seg->v;
                        vsum += seg->v;
                    }
                    if (qual->Qualflag == ENERGYINTENSITY)
                    {
                        rsum += ABS(seg->c - cseg) * seg->v;
                        vsum += seg->v;
                    }
                    seg = seg->prev;
                }

                // Normalize volume-weighted reaction rate
                if (vsum > 0.0) qual->PipeRateCoeff[k] = rsum / vsum / (double)dt * SECperDAY;
                else qual->PipeRateCoeff[k] = 0.0;
            }
        };
        void     reacttanks(EN_Project const& pr, units::time::second_t dt)
            /*
            **--------------------------------------------------------------
            **   Input:   dt = time step
            **   Output:  none
            **   Purpose: reacts water within each tank over a time step.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int i, k;
            SCALER c;
            Pseg seg;
            Ptank tank;

            // Examine each tank in network
            for (i = 1; i <= net->Ntanks; i++)
            {
                // Skip reservoirs
                tank = net->Tank[i];
                if (tank->Diameter == 0.0_ft) continue;

                // k is segment chain belonging to tank i
                k = net->Nlinks + i;

                // React each volume segment in the chain
                seg = qual->FirstSeg[k];
                while (seg != NULL)
                {
                    c = seg->c;
                    seg->c = tankreact(pr, seg->c, seg->v, tank->Kb, dt);
                    qual->MassBalance.reacted += (c - seg->c) * seg->v;
                    seg = seg->prev;
                }
            }
        };
        SCALER   piperate(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  returns reaction rate coeff. for 1st-order wall
            **            reactions or mass transfer rate coeff. for 0-order
            **            reactions
            **   Purpose: finds wall reaction rate coeffs.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;

            SCALER a, d, u, q, kf, kw, y, Re, Sh;

            d = (double)net->Link[k]->Diam;   // Pipe diameter, ft

            // Ignore mass transfer if Schmidt No. is 0
            if (qual->Sc == 0.0)
            {
                if (qual->WallOrder == 0.0) return BIG;
                else return (net->Link[k]->Kw * (4.0 / d) / pr->Ucf[ELEV]);
            }

            // Compute Reynolds No.
            // Flow rate made consistent with how its saved to hydraulics file
            q = (double)((hyd->LinkStatus[k] <= CLOSED) ? 0.0_cfs : hyd->LinkFlow[k]);
            a = PI * d * d / 4.0;         // pipe area
            u = ABS(q) / a;              // flow velocity
            Re = (double)(u * d / hyd->Viscos);     // Reynolds number

            // Compute Sherwood No. for stagnant flow
            // (mass transfer coeff. = Diffus./radius)
            if (Re < 1.0) Sh = 2.0;

            // Compute Sherwood No. for turbulent flow using the Notter-Sleicher formula.
            else if (Re >= 2300.0) Sh = 0.0149 * ::epanet::pow(Re, 0.88) * ::epanet::pow(qual->Sc, 0.333);

            // Compute Sherwood No. for laminar flow using Graetz solution formula.
            else
            {
                y = d / (SCALER)(double)net->Link[k]->Len * Re * qual->Sc;
                Sh = 3.65 + 0.0668 * y / (1.0 + 0.04 * ::epanet::pow(y, 0.667));
            }

            // Compute mass transfer coeff. (in ft/sec)
            kf = (double)(Sh * qual->Diffus / d);

            // For zero-order reaction, return mass transfer coeff.
            if (qual->WallOrder == 0.0) return kf;

            // For first-order reaction, return apparent wall coeff.
            kw = net->Link[k]->Kw / pr->Ucf[ELEV];        // Wall coeff, ft/sec
            kw = (4.0 / d) * kw * kf / (kf + ABS(kw));   // Wall coeff, 1/sec
            return kw;
        };
        SCALER   pipereact(EN_Project const& pr, int k, SCALER c, SCALER v, units::time::second_t dt)
            /*
            **------------------------------------------------------------
            **   Input:   k = link index
            **            c = current quality in segment
            **            v = segment volume
            **            dt = time step
            **   Output:  returns new WQ value
            **   Purpose: computes new quality in a pipe segment after
            **            reaction occurs
            **------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            SCALER cnew, dc, dcbulk, dcwall, rbulk, rwall;

            // For water age (hrs), update concentration by timestep
            if (qual->Qualflag == AGE)
            {
                dc = (double)dt / 3600.0;
                cnew = c + dc;
                cnew = fMAX(0.0, cnew);
                return cnew;
            }

            // Otherwise find bulk & wall reaction rates
            rbulk = bulkrate(pr, c, net->Link[k]->Kb, qual->BulkOrder) * qual->Bucf;
            rwall = wallrate(pr, c, (double)net->Link[k]->Diam, net->Link[k]->Kw, net->Link[k]->Rc);

            // Find change in concentration over timestep
            dcbulk = rbulk * (double)dt;
            dcwall = rwall * (double)dt;

            // Update cumulative mass reacted
            if (pr->times.Htime >= pr->times.Rstart)
            {
                qual->Wbulk += ABS(dcbulk) * v;
                qual->Wwall += ABS(dcwall) * v;
            }

            // Update concentration
            dc = dcbulk + dcwall;
            cnew = c + dc;
            cnew = fMAX(0.0, cnew);
            return cnew;
        };
        SCALER   tankreact(EN_Project const& pr, SCALER c, SCALER v, SCALER kb, units::time::second_t dt)
            /*
            **-------------------------------------------------------
            **   Input:   c = current quality in tank
            **            v = tank volume
            **            kb = reaction coeff.
            **            dt = time step
            **   Output:  returns new WQ value
            **   Purpose: computes new quality in a tank after
            **            reaction occurs
            **-------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;

            SCALER cnew, dc, rbulk;

            // For water age, update concentration by timestep
            if (qual->Qualflag == AGE)
            {
                dc = (double)dt / 3600.0;
            }

            // For chemical analysis apply bulk reaction rate
            else
            {
                // Find bulk reaction rate
                rbulk = bulkrate(pr, c, kb, qual->TankOrder) * qual->Tucf;

                // Find concentration change & update quality
                dc = rbulk * (double)dt;
                if (pr->times.Htime >= pr->times.Rstart)
                {
                    qual->Wtank += ABS(dc) * v;
                }
            }
            cnew = c + dc;
            cnew = fMAX(0.0, cnew);
            return cnew;
        };
        SCALER   bulkrate(EN_Project const& pr, SCALER c, SCALER kb, SCALER order)
            /*
            **-----------------------------------------------------------
            **   Input:   c = current WQ concentration
            **            kb = bulk reaction coeff.
            **            order = bulk reaction order
            **   Output:  returns bulk reaction rate
            **   Purpose: computes bulk reaction rate (mass/volume/time)
            **-----------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;

            SCALER c1;

            // Find bulk reaction potential taking into account
            // limiting potential & reaction order.

            // Zero-order kinetics:
            if (order == 0.0) c = 1.0;

            // Michaelis-Menton kinetics:
            else if (order < 0.0)
            {
                c1 = qual->Climit + SGN(kb) * c;
                if (ABS(c1) < TINY) c1 = SGN(c1) * TINY;
                c = c / c1;
            }

            // N-th order kinetics:
            else
            {
                // Account for limiting potential
                if (qual->Climit == 0.0) c1 = c;
                else c1 = fMAX(0.0, SGN(kb) * (qual->Climit - c));

                // Compute concentration potential
                if (order == 1.0) c = c1;
                else if (order == 2.0) c = c1 * c;
                else c = c1 * ::epanet::pow(fMAX(0.0, c), order - 1.0);
            }

            // Reaction rate = bulk coeff. * potential
            if (c < (SCALER)0) c = 0;
            return kb * c;
        };
        SCALER   wallrate(EN_Project const& pr, SCALER c, SCALER d, SCALER kw, SCALER kf)
            /*
            **------------------------------------------------------------
            **   Input:   c = current WQ concentration
            **            d = pipe diameter
            **            kw = intrinsic wall reaction coeff.
            **            kf = mass transfer coeff. for 0-order reaction
            **                 (ft/sec) or apparent wall reaction coeff.
            **                 for 1-st order reaction (1/sec)
            **   Output:  returns wall reaction rate in mass/ft3/sec
            **   Purpose: computes wall reaction rate
            **------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;

            if (kw == 0.0 || d == 0.0) return (0.0);

            if (qual->WallOrder == 0.0)             // 0-order reaction */
            {
                kf = SGN(kw) * c * kf;              //* Mass transfer rate (mass/ft2/sec)
                kw = kw * SQR(pr->Ucf[ELEV]);       // Reaction rate (mass/ft2/sec)
                if (ABS(kf) < ABS(kw)) kw = kf;   // Reaction mass transfer limited
                return (kw * 4.0 / d);              // Reaction rate (mass/ft3/sec)
            }
            else return (c * kf);                   // 1st-order reaction
        };
        SCALER   mixtank(EN_Project const& pr, int n, SCALER volin, SCALER massin, SCALER volout)
            /*
            **------------------------------------------------------------
            **   Input:   n      = node index
            **            volin  = inflow volume to tank over time step
            **            massin = mass inflow to tank over time step
            **            volout = outflow volume from tank over time step
            **   Output:  returns new quality for tank
            **   Purpose: mixes inflow with tank's contents to update its quality.
            **------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            int i;
            SCALER vnet;
            i = n - net->Njuncs;
            vnet = volin - volout;
            switch (net->Tank[i]->MixModel)
            {
            case MIX1: tankmix1(pr, i, volin, massin, vnet); break;
            case MIX2: tankmix2(pr, i, volin, massin, vnet); break;
            case FIFO: tankmix3(pr, i, volin, massin, vnet); break;
            case LIFO: tankmix4(pr, i, volin, massin, vnet); break;
            }
            return pr->quality.TankConcentration[i]; //  net->Tank[i]->C;
        };
        void     tankmix1(EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet)
            /*
            **---------------------------------------------
            **   Input:   i = tank index
            **            vin = inflow volume
            **            win = mass inflow
            **            vnet = inflow - outflow
            **   Output:  none
            **   Purpose: updates quality in a complete mix tank model
            **---------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int k;
            SCALER vnew;
            Pseg seg;
            Ptank tank = net->Tank[i];

            k = net->Nlinks + i;
            seg = qual->FirstSeg[k];
            if (seg)
            {
                vnew = seg->v + vin;
                if (vnew > 0.0) seg->c = (seg->c * seg->v + win) / vnew;
                seg->v += vnet;
                seg->v = fMAX(0.0, seg->v);
                // tank->C = seg->c;
                qual->TankConcentration[i] = seg->c;
            }
        };
        void     tankmix2(EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet)
            /*
            **------------------------------------------------
            **   Input:   i = tank index
            **            vin = inflow volume
            **            win = mass inflow
            **            vnet = inflow - outflow
            **   Output:  none
            **   Purpose: updates quality in a 2-compartment tank model
            **------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int    k;
            SCALER vt,          // Transferred volume
                vmz;         // Full mixing zone volume
            Pseg   mixzone,     // Mixing zone segment
                stagzone;    // Stagnant zone segment
            Ptank tank = pr->network->Tank[i];

            // Identify segments for each compartment
            k = net->Nlinks + i;
            mixzone = qual->LastSeg[k];
            stagzone = qual->FirstSeg[k];
            if (mixzone == NULL || stagzone == NULL) return;

            // Full mixing zone volume
            vmz = (double)(tank->V1frac * tank->Vmax(pr));

            // Tank is filling
            vt = 0.0;
            if (vnet > 0.0)
            {
                vt = fMAX(0.0, (mixzone->v + vnet - vmz));
                if (vin > 0.0)
                {
                    mixzone->c = ((mixzone->c) * (mixzone->v) + win) /
                        (mixzone->v + vin);
                }
                if (vt > 0.0)
                {
                    stagzone->c = ((stagzone->c) * (stagzone->v) +
                        (mixzone->c) * vt) / (stagzone->v + vt);
                }
            }

            // Tank is emptying
            else if (vnet < 0.0)
            {
                if (stagzone->v > 0.0) vt = fMIN(stagzone->v, (-vnet));
                if (vin + vt > 0.0)
                {
                    mixzone->c = ((mixzone->c) * (mixzone->v) + win +
                        (stagzone->c) * vt) / (mixzone->v + vin + vt);
                }
            }

            // Update segment volumes
            if (vt > 0.0)
            {
                mixzone->v = vmz;
                if (vnet > 0.0) stagzone->v += vt;
                else            stagzone->v = fMAX(0.0, ((stagzone->v) - vt));
            }
            else
            {
                mixzone->v += vnet;
                mixzone->v = fMIN(mixzone->v, vmz);
                mixzone->v = fMAX(0.0, mixzone->v);
                stagzone->v = 0.0;
            }

            // Use quality of mixing zone to represent quality of tank since this is where outflow begins to flow from
            //tank->C = mixzone->c;
            qual->TankConcentration[i] = mixzone->c;
        };
        void     tankmix3(EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet)
            /*
            **----------------------------------------------------------
            **   Input:   i = tank index
            **            vin = inflow volume
            **            win = mass inflow
            **            vnet = inflow - outflow
            **   Output:  none
            **   Purpose: Updates quality in a First-In-First-Out (FIFO) tank model.
            **----------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int k;
            SCALER vout, vseg;
            SCALER cin, vsum, wsum;
            Pseg seg;
            Ptank tank = pr->network->Tank[i];

            k = net->Nlinks + i;
            if (qual->LastSeg[k] == NULL || qual->FirstSeg[k] == NULL) return;

            // Add new last segment for flow entering the tank
            if (vin > 0.0)
            {
                // ... increase segment volume if inflow has same quality as segment
                cin = win / vin;
                seg = qual->LastSeg[k];
                if (ABS(seg->c - cin) < qual->Ctol) seg->v += vin;

                // ... otherwise add a new last segment to the tank
                else addseg(pr, k, vin, cin);
            }

            // Withdraw flow from first segment
            vsum = 0.0;
            wsum = 0.0;
            vout = vin - vnet;
            while (vout > 0.0)
            {
                seg = qual->FirstSeg[k];
                if (seg == NULL)  break;
                vseg = seg->v;            // Flow volume from leading seg
                vseg = fMIN(vseg, vout);
                if (seg == qual->LastSeg[k]) vseg = vout;
                vsum += vseg;
                wsum += (seg->c) * vseg;
                vout -= vseg;                       // Remaining flow volume
                if (vout >= 0.0 && vseg >= seg->v)  // Seg used up
                {
                    if (seg->prev)
                    {
                        qual->FirstSeg[k] = seg->prev;
                        seg->prev = qual->FreeSeg;
                        qual->FreeSeg = seg;
                    }
                }
                else seg->v -= vseg;      // Remaining volume in segment
            }

            // Use quality withdrawn from 1st segment
            // to represent overall quality of tank
            if (vsum > 0.0) {
                //tank->C = wsum / vsum;
                qual->TankConcentration[i] = wsum / vsum;
            }
            else if (qual->FirstSeg[k] == NULL) {
                //tank->C = 0.0;
                qual->TankConcentration[i] = 0;
            }
            else {
                //tank->C = qual->FirstSeg[k]->c;
                qual->TankConcentration[i] = qual->FirstSeg[k]->c;
            }
        };
        void     tankmix4(EN_Project const& pr, int i, SCALER vin, SCALER win, SCALER vnet)
            /*
            **----------------------------------------------------------
            **   Input:   i = tank index
            **            vin = inflow volume
            **            win = mass inflow
            **            vnet = inflow - outflow
            **   Output:  none
            **   Purpose: Updates quality in a Last In-First Out (LIFO) tank model.
            **----------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int k;
            SCALER cin, vsum, wsum, vseg;
            Pseg seg;
            Ptank tank = pr->network->Tank[i];

            k = net->Nlinks + i;
            if (qual->LastSeg[k] == NULL || qual->FirstSeg[k] == NULL) return;

            // Find inflows & outflows
            if (vin > 0.0) cin = win / vin;
            else           cin = 0.0;

            // If tank filling, then create new last seg
            //tank->C = qual->LastSeg[k]->c;
            qual->TankConcentration[i] = qual->LastSeg[k]->c;
            seg = qual->LastSeg[k];
            if (vnet > 0.0)
            {
                // ... inflow quality is same as last segment's quality,
                //     so just add inflow volume to last segment
                if (ABS(seg->c - cin) < qual->Ctol) seg->v += vnet;

                // ... otherwise add a new last segment with inflow quality
                else addseg(pr, k, vnet, cin);

                // Update reported tank quality
                //tank->C = qual->LastSeg[k]->c;
                qual->TankConcentration[i] = qual->LastSeg[k]->c;
            }

            // If tank emptying then remove last segments until vnet consumed
            else if (vnet < 0.0)
            {
                vsum = 0.0;
                wsum = 0.0;
                vnet = -vnet;

                // Reverse segment chain so segments are processed from last to first
                reversesegs(pr, k);

                // While there is still volume to remove
                while (vnet > 0.0)
                {
                    // ... start with reversed first segment
                    seg = qual->FirstSeg[k];
                    if (seg == NULL) break;

                    // ... find volume to remove from it
                    vseg = seg->v;
                    vseg = fMIN(vseg, vnet);
                    if (seg == qual->LastSeg[k]) vseg = vnet;

                    // ... update total volume & mass removed
                    vsum += vseg;
                    wsum += (seg->c) * vseg;

                    // ... update remiaing volume to remove
                    vnet -= vseg;

                    // ... if no more volume left in current segment
                    if (vnet >= 0.0 && vseg >= seg->v)
                    {
                        // ... replace current segment with previous one
                        if (seg->prev)
                        {
                            qual->FirstSeg[k] = seg->prev;
                            seg->prev = qual->FreeSeg;
                            qual->FreeSeg = seg;
                        }
                    }

                    // ... otherwise reduce volume of current segment
                    else seg->v -= vseg;
                }

                // Restore original orientation of segment chain
                reversesegs(pr, k);

                // Reported tank quality is mixture of flow released and any inflow
                //tank->C = (wsum + win) / (vsum + vin);
                qual->TankConcentration[i] = (wsum + win) / (vsum + vin);
            }
        };
        void     transport(EN_Project const& pr, units::time::second_t tstep)
            /*
            **--------------------------------------------------------------
            **   Input:   tstep = length of current time step
            **   Output:  none
            **   Purpose: transports constituent mass through the pipe network
            **            under a period of constant hydraulic conditions.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;

            int j, k, m, n;
            SCALER volin, massin, volout, nodequal;
            Padjlist  alink;

            // React contents of each pipe and tank
            if (qual->Reactflag)
            {
                reactpipes(pr, tstep);
                reacttanks(pr, tstep);
            }

            // Analyze each node in topological order
            for (j = 1; j <= net->Nnodes; j++)
            {
                // ... index of node to be processed
                n = qual->SortedNodes[j];

                // ... zero out mass & flow volumes for this node
                volin = 0.0;
                massin = 0.0;
                volout = 0.0;

                // ... examine each link with flow into the node
                for (alink = net->Adjlist[n]; alink != nullptr; alink = alink->next)
                {
                    // ... k is index of next link incident on node n
                    k = alink->link;

                    // ... link has flow into node - add it to node's inflow
                    //     (m is index of link's downstream node)
                    m = net->Link[k]->N2;
                    if (qual->FlowDir[k] < 0) m = net->Link[k]->N1;
                    if (m == n)
                    {
                        evalnodeinflow(pr, k, tstep, &volin, &massin);
                    }

                    // ... link has flow out of node - add it to node's outflow
                    else volout += (double)ABS(((hyd->LinkStatus[k] <= CLOSED) ? 0.0_cfs : hyd->LinkFlow[k]));
                }

                // ... if node is a junction, add on any external outflow (e.g., demands)
                if (net->Node[n]->Type == JUNCTION)
                {
                    volout += (double)fMAX(0.0_cfs, hyd->NodeDemand[n]);
                }

                // ... convert from outflow rate to volume
                volout *= (double)tstep;

                // ... find the concentration of flow leaving the node
                nodequal = findnodequal(pr, n, volin, massin, volout, tstep);

                // ... examine each link with flow out of the node
                for (alink = net->Adjlist[n]; alink != nullptr; alink = alink->next)
                {
                    // ... link k incident on node n has upstream node m equal to n
                    k = alink->link;
                    m = net->Link[k]->N1;
                    if (qual->FlowDir[k] < 0) m = net->Link[k]->N2;
                    if (m == n)
                    {
                        // ... send flow at new node concen. into link
                        evalnodeoutflow(pr, k, nodequal, tstep);
                    }
                }
                updatemassbalance(pr, n, massin, volout, tstep);
            }
        };
        void     evalnodeinflow(EN_Project const& pr, int k, units::time::second_t tstep, SCALER* volin, SCALER* massin)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **            tstep = quality routing time step
            **   Output:  volin = flow volume entering a node
            **            massin = constituent mass entering a node
            **   Purpose: adds the contribution of a link's outflow volume
            **            and constituent mass to the total inflow into its
            **            downstream node over a time step.
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;

            SCALER q, v, vseg;
            Pseg seg;

            // Get flow rate (q) and flow volume (v) through link
            q = (double)((hyd->LinkStatus[k] <= CLOSED) ? 0.0_cfs : hyd->LinkFlow[k]);
            v = ABS(q) * (double)tstep;

            // Transport flow volume v from link's leading segments into downstream
            // node, removing segments once their full volume is consumed
            while (v > 0.0)
            {
                seg = qual->FirstSeg[k];
                if (!seg) break;

                // ... volume transported from first segment is smaller of
                //     remaining flow volume & segment volume
                vseg = seg->v;
                vseg = fMIN(vseg, v);

                // ... update total volume & mass entering downstream node
                *volin += vseg;
                *massin += vseg * seg->c;

                // ... reduce remaining flow volume by amount transported
                v -= vseg;

                // ... if all of segment's volume was transferred
                if (v >= 0.0 && vseg >= seg->v)
                {
                    // ... replace this leading segment with the one behind it
                    qual->FirstSeg[k] = seg->prev;
                    if (qual->FirstSeg[k] == NULL) qual->LastSeg[k] = NULL;

                    // ... recycle the used up segment
                    seg->prev = qual->FreeSeg;
                    qual->FreeSeg = seg;
                }

                // ... otherwise just reduce this segment's volume
                else seg->v -= vseg;
            }
        };
        SCALER   findnodequal(EN_Project const& pr, int n, SCALER volin, SCALER massin, SCALER volout, units::time::second_t tstep)
            /*
            **--------------------------------------------------------------
            **   Input:   n = node index
            **            volin = flow volume entering node
            **            massin = mass entering node
            **            volout = flow volume leaving node
            **            tstep = length of current time step
            **   Output:  returns water quality in a node's outflow
            **   Purpose: computes a node's new quality from its inflow
            **            volume and mass, including any source contribution.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;

            // Node is a junction - update its water quality
            if (net->Node[n]->Type == JUNCTION)
            {
                // ... dilute inflow with any external negative demand
                volin -= (double)(fMIN(0.0_cfs, hyd->NodeDemand[n]) * tstep);

                // ... new concen. is mass inflow / volume inflow
                if (volin > 0.0) qual->NodeQual[n] = massin / volin;

                // ... if no inflow adjust quality for reaction in connecting pipes
                else if (qual->Reactflag) qual->NodeQual[n] = noflowqual(pr, n);
            }

            // Node is a tank - use its mixing model to update its quality
            else if (net->Node[n]->Type == TANK)
            {
                qual->NodeQual[n] = mixtank(pr, n, volin, massin, volout);
            }

            // Add any external quality source onto node's concen.
            qual->SourceQual = 0.0;

            // For source tracing analysis find tracer added at source node
            if (qual->Qualflag == TRACE_QUAL)
            {
                if (net->Node[n]->ID == qual->TraceNode) {
                    // ... quality added to network is difference between tracer
                    //     concentration (100 mg/L) and current node quality
                    if (net->Node[n]->Type == RESERVOIR) qual->SourceQual = 100.0;
                    else qual->SourceQual = fMAX(100.0 - qual->NodeQual[n], 0.0);
                    qual->NodeQual[n] = 100.0;
                }
                return qual->NodeQual[n];
            }

            // Find quality contribued by any external chemical source
            else qual->SourceQual = findsourcequal(pr, n, volout, tstep);
            if (qual->SourceQual == 0.0) return qual->NodeQual[n];

            // Combine source quality with node quality
            switch (net->Node[n]->Type)
            {
            case JUNCTION:
                qual->NodeQual[n] += qual->SourceQual;
                return qual->NodeQual[n];

            case TANK:
                return qual->NodeQual[n] + qual->SourceQual;

            case RESERVOIR:
                qual->NodeQual[n] = qual->SourceQual;
                return qual->SourceQual;
            }
            return qual->NodeQual[n];
        };
        SCALER   noflowqual(EN_Project const& pr, int n)
            /*
            **--------------------------------------------------------------
            **   Input:   n = node index
            **   Output:  quality for node n
            **   Purpose: sets the quality for a junction node that has no
            **            inflow to the average of the quality in its
            **            adjoining link segments.
            **   Note:    this function is only used for reactive substances.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int k, inflow, kount = 0;
            SCALER c = 0.0;
            FlowDirection dir;
            Padjlist  alink;

            // Examine each link incident on the node
            for (alink = net->Adjlist[n]; alink != nullptr; alink = alink->next)
            {
                // ... index of an incident link
                k = alink->link;
                dir = qual->FlowDir[k];

                // Node n is link's downstream node - add quality
                // of link's first segment to average
                if (net->Link[k]->N2 == n && dir >= 0) inflow = TRUE;
                else if (net->Link[k]->N1 == n && dir < 0)  inflow = TRUE;
                else inflow = FALSE;
                if (inflow == TRUE && qual->FirstSeg[k] != NULL)
                {
                    c += qual->FirstSeg[k]->c;
                    kount++;
                }

                // Node n is link's upstream node - add quality
                // of link's last segment to average
                else if (inflow == FALSE && qual->LastSeg[k] != NULL)
                {
                    c += qual->LastSeg[k]->c;
                    kount++;
                }
            }
            if (kount > 0) c = c / (SCALER)kount;
            return c;
        };
        void     evalnodeoutflow(EN_Project const& pr, int k, SCALER c, units::time::second_t tstep)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **            c = quality from upstream node
            **            tstep = time step
            **   Output:  none
            **   Purpose: releases flow volume and mass from the upstream
            **            node of a link over a time step.
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;

            SCALER v;
            Pseg seg;

            // Find flow volume (v) released over time step
            v = (double)(ABS(((hyd->LinkStatus[k] <= CLOSED) ? 0.0_cfs : hyd->LinkFlow[k])) * tstep);
            if (v == 0.0) return;

            // Release flow and mass into upstream end of the link

            // ... case where link has a last (most upstream) segment
            seg = qual->LastSeg[k];
            if (seg)
            {
                // ... if node quality close to segment quality then mix
                //     the nodal outflow volume with the segment's volume
                if (ABS(seg->c - c) < qual->Ctol)
                {
                    seg->c = (seg->c * seg->v + c * v) / (seg->v + v);
                    seg->v += v;
                }

                // ... otherwise add a new segment at upstream end of link
                else addseg(pr, k, v, c);
            }

            // ... link has no segments so add one
            else addseg(pr, k, v, c);
        };
        void     updatemassbalance(EN_Project const& pr, int n, SCALER massin, SCALER volout, units::time::second_t tstep)
            /*
            **--------------------------------------------------------------
            **   Input:   n = node index
            **            massin = mass inflow to node
            **            volout = outflow volume from node
            **   Output:  none
            **   Purpose: Adds a node's external mass inflow and outflow
            **            over the current time step to the network's
            **            overall mass balance.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;

            SCALER masslost = 0.0,
                massadded = 0.0;

            switch (net->Node[n]->Type)
            {
                // Junctions lose mass from outflow demand & gain it from source inflow
            case JUNCTION:
                masslost = (SCALER)(double)(fMAX(0.0_cfs, hyd->NodeDemand[n]) * tstep) * qual->NodeQual[n];
                massadded = qual->SourceQual * volout;
                break;

                // Reservoirs add mass from quality source if specified or from a fixed
                // initial quality
            case RESERVOIR:
                masslost = massin;
                if (qual->SourceQual > 0.0) massadded = qual->SourceQual * volout;
                else                        massadded = qual->NodeQual[n] * volout;
                break;

                // Tanks add mass only from external source inflow
            case TANK:
                massadded = qual->SourceQual * volout;
                break;
            }
            qual->MassBalance.outflow += masslost;
            qual->MassBalance.inflow += massadded;
        };
        int      sortnodes(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns an error code
            **   Purpose: topologically sorts nodes from upstream to downstream.
            **   Note:    links with negligible flow are ignored since they can
            **            create spurious cycles that cause the sort to fail.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int i, j, k, n;
            int* indegree = NULL;
            int* stack = NULL;
            int stacksize = 0;
            int numsorted = 0;
            int errcode = 0;
            FlowDirection dir;
            Padjlist  alink;

            // Allocate an array to count # links with inflow to each node
            // and for a stack to hold nodes waiting to be processed
            indegree = (int*)calloc(net->Nnodes + 1, sizeof(int));
            stack = (int*)calloc(net->Nnodes + 1, sizeof(int));
            if (indegree && stack)
            {
                // Count links with "non-negligible" inflow to each node
                for (k = 1; k <= net->Nlinks; k++)
                {
                    dir = qual->FlowDir[k];
                    if (dir == POSITIVE) n = net->Link[k]->N2;
                    else if (dir == NEGATIVE) n = net->Link[k]->N1;
                    else continue;
                    indegree[n]++;
                }

                // Place nodes with no inflow onto a stack
                for (i = 1; i <= net->Nnodes; i++)
                {
                    if (indegree[i] == 0)
                    {
                        stacksize++;
                        stack[stacksize] = i;
                    }
                }

                // Examine each node on the stack until none are left
                while (numsorted < net->Nnodes)
                {
                    // ... if stack is empty then a cycle exists
                    if (stacksize == 0)
                    {
                        //  ... add a non-sorted node connected to a sorted one to stack
                        j = selectnonstacknode(pr, numsorted, indegree);
                        if (j == 0) break;  // This shouldn't happen.
                        indegree[j] = 0;
                        stacksize++;
                        stack[stacksize] = j;
                    }

                    // ... make the last node added to the stack the next
                    //     in sorted order & remove it from the stack
                    i = stack[stacksize];
                    stacksize--;
                    numsorted++;
                    qual->SortedNodes[numsorted] = i;

                    // ... for each outflow link from this node reduce the in-degree
                    //     of its downstream node
                    for (alink = net->Adjlist[i]; alink != nullptr; alink = alink->next)
                    {
                        // ... k is the index of the next link incident on node i
                        k = alink->link;

                        // ... skip link if flow is negligible
                        if (qual->FlowDir[k] == 0) continue;

                        // ... link has flow out of node (downstream node n not equal to i)
                        n = net->Link[k]->N2;
                        if (qual->FlowDir[k] < 0) n = net->Link[k]->N1;

                        // ... reduce degree of node n
                        if (n != i && indegree[n] > 0)
                        {
                            indegree[n]--;

                            // ... no more degree left so add node n to stack
                            if (indegree[n] == 0)
                            {
                                stacksize++;
                                stack[stacksize] = n;
                            }
                        }
                    }
                }
            }
            else errcode = 101;
            if (numsorted < net->Nnodes) errcode = 120;
            FREE(indegree);
            FREE(stack);
            return errcode;
        };
        int      selectnonstacknode(EN_Project const& pr, int numsorted, int* indegree)
            /*
            **--------------------------------------------------------------
            **   Input:   numsorted = number of nodes that have been sorted
            **            indegree = number of inflow links to each node
            **   Output:  returns a node index
            **   Purpose: selects a next node for sorting when a cycle exists.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int i, m, n;
            Padjlist  alink;

            // Examine each sorted node in last in - first out order
            for (i = numsorted; i > 0; i--)
            {
                // For each link connected to the sorted node
                m = qual->SortedNodes[i];
                for (alink = net->Adjlist[m]; alink != nullptr; alink = alink->next)
                {
                    // ... n is the node of link k opposite to node m
                    n = alink->node;

                    // ... select node n if it still has inflow links
                    if (indegree[n] > 0) return n;
                }
            }

            // If no node was selected by the above process then return the
            // first node that still has inflow links remaining
            for (i = 1; i <= net->Nnodes; i++)
            {
                if (indegree[i] > 0) return i;
            }

            // If all else fails return 0 indicating that no node was selected
            return 0;
        };
        void     initsegs(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: initializes water quality volume segments in each
            **            pipe and tank.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int j, k;
            SCALER c, v, v1;

            // Add one segment with assigned downstream node quality to each pipe
            for (k = 1; k <= net->Nlinks; k++)
            {
                qual->FirstSeg[k] = NULL;
                qual->LastSeg[k] = NULL;
                if (net->Link[k]->Type == PIPE)
                {
                    v = (double)(0.785398 * net->Link[(k)]->Len * SQR(net->Link[(k)]->Diam));
                    j = net->Link[k]->N2;
                    c = qual->NodeQual[j];
                    addseg(pr, k, v, c);
                }
            }

            // Initialize segments in tanks
            for (j = 1; j <= net->Ntanks; j++)
            {
                // Skip reservoirs
                if (net->Tank[j]->Diameter == 0.0_ft) continue;

                // Establish initial tank quality & volume
                k = net->Tank[j]->Node;
                c = net->Node[k]->C0(pr->times.GetSimStartTime());
                auto& tank = net->Tank[j];
                v = (double)tank->Volume(pr, tank->InitialHead(pr->times.GetSimStartTime()));

                // Create one volume segment for entire tank
                k = net->Nlinks + j;
                qual->FirstSeg[k] = NULL;
                qual->LastSeg[k] = NULL;
                addseg(pr, k, v, c);

                // Create a 2nd segment for the 2-compartment tank model
                if (net->Tank[j]->MixModel == MIX2)
                {
                    // ... mixing zone segment
                    v1 = (double)fMAX(
                        0_cu_ft,
                        ((cubic_foot_t)(double)v) - (net->Tank[j]->Vmax(pr) * net->Tank[j]->V1frac)
                    );
                    qual->FirstSeg[k]->v = v1;

                    // ... stagnant zone segment
                    v = v - v1;
                    addseg(pr, k, v, c);
                }
            }
        };
        int      openqual(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: opens water quality solver
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int errcode = 0;
            int n;

            // Return if no quality analysis requested
            if (qual->Qualflag == NONE) return errcode;

            // Build nodal adjacency lists if they don't already exist
            if (net->Adjlist.Num() == 0)
            {
                // Check for too few nodes & no fixed grade nodes
                if (net->Nnodes < 2) return 223;
                if (net->Ntanks == 0) return 224;

                // Build adjacency lists
                errcode = smatrix_t::buildadjlists(net);
                if (errcode) return errcode;

                // Check for unconnected nodes
                if (errcode = unlinked(pr)) return errcode;
            }

            // Create a memory pool for water quality segments
            qual->OutOfMemory = FALSE;
            qual->SegPool = Mempool_t::mempool_create();
            if (qual->SegPool == NULL) errcode = 101;

            // Allocate arrays for link flow direction & reaction rates
            n = net->Nlinks + 1;
            qual->FlowDir.ClearedResize(n);
            qual->PipeRateCoeff.ClearedResize(n);

            // Allocate arrays used for volume segments in links & tanks
            n = net->Nlinks + net->Ntanks + 1;
            qual->FirstSeg = (Pseg*)calloc(n, sizeof(Pseg));
            qual->LastSeg = (Pseg*)calloc(n, sizeof(Pseg));

            // Allocate memory for topologically sorted nodes
            qual->SortedNodes.ClearedResize(n);

            ERRCODE(MEMCHECK(qual->FirstSeg));
            ERRCODE(MEMCHECK(qual->LastSeg));
            return errcode;
        };
        int      initqual(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: re-initializes water quality solver
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Times* time = &pr->times;

            int i;
            int errcode = 0;

            // Re-position hydraulics file
            if (!hyd->OpenHflag)
            {
                fseek(pr->outfile.HydFile, pr->outfile.HydOffset, SEEK_SET);
            }

            // Set elapsed times to zero
            time->Qtime = 0;
            time->Htime = 0;
            time->Rtime = time->Rstart;
            time->Rtime_JunctionsPipes = time->Rstart;
            pr->report.Nperiods = 0;

            // Initialize node quality
            for (i = 1; i <= net->Nnodes; i++)
            {
                if (qual->Qualflag == TRACE_QUAL) qual->NodeQual[i] = 0.0;
                else                         qual->NodeQual[i] = net->Node[i]->C0(pr->times.GetSimStartTime());
                if (net->Node[i]->S != NULL) net->Node[i]->S->Smass = 0.0;
            }
            if (qual->Qualflag == NONE) return errcode;

            // Initialize tank quality
            for (i = 1; i <= net->Ntanks; i++)
            {
                //net->Tank[i]->C = qual->NodeQual[net->Tank[i]->Node];
                qual->TankConcentration[i] = qual->NodeQual[net->Tank[i]->Node];
            }

            // Initialize quality at trace node (if applicable)
            if (qual->Qualflag == TRACE_QUAL) {
                for (i = 1; i <= net->Nnodes; i++) {
                    if (net->Node[i]->ID == qual->TraceNode) {
                        qual->NodeQual[i] = 100.0;
                        break;
                    }
                }
            }

            // Compute Schmidt number
            if (qual->Diffus > (squared_foot_per_second_t)0.0) qual->Sc = (double)(hyd->Viscos / qual->Diffus);
            else                    qual->Sc = 0.0;

            // Compute unit conversion factor for bulk react. coeff.
            qual->Bucf = getucf(qual->BulkOrder);
            qual->Tucf = getucf(qual->TankOrder);

            // Check if modeling a reactive substance
            qual->Reactflag = setreactflag(pr);

            // Reset memory pool used for pipe & tank segments
            qual->FreeSeg = NULL;
            Mempool_t::mempool_reset(qual->SegPool);

            // Create initial set of pipe & tank segments
            initsegs(pr);

            // Initialize link flow direction indicator
            for (i = 1; i <= net->Nlinks; i++) qual->FlowDir[i] = ZERO_FLOW;

            // Initialize avg. reaction rates
            qual->Wbulk = 0.0;
            qual->Wwall = 0.0;
            qual->Wtank = 0.0;
            qual->Wsource = 0.0;

            // Initialize mass balance components
            qual->MassBalance.initial = findstoredmass(pr);
            qual->MassBalance.inflow = 0.0;
            qual->MassBalance.outflow = 0.0;
            qual->MassBalance.reacted = 0.0;
            qual->MassBalance.final = 0.0;
            qual->MassBalance.ratio = 0.0;
            return errcode;
        };
        int      runqual(EN_Project const& pr, units::time::second_t* t)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  t = current simulation time (sec)
            **   Returns: error code
            **   Purpose: retrieves hydraulics for next hydraulic time step
            **            (at time t) and saves current results to file
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Times* time = &pr->times;

            units::time::second_t hydtime = 0;       // Hydraulic solution time
            units::time::second_t hydstep = 0;       // Hydraulic time step
            int errcode = 0;

            // Update reported simulation time
            *t = time->Qtime;

            // Read hydraulic solution from hydraulics file
            if (time->Qtime == time->Htime)
            {
                // Read hydraulic results from file
                if (!hyd->OpenHflag)
                {
                    if (!readhyd(pr, &hydtime)) return 307;
                    if (!readhydstep(pr, &hydstep)) return 307;
                    time->Htime = hydtime;
                }

                // Save current results to output file
                if (time->Htime >= time->Rtime)
                {
                    if (pr->outfile.Saveflag)
                    {
                        errcode = saveoutput(pr);
                        pr->report.Nperiods++;
                    }
                    time->Rtime += time->Rstep;
                }
                if (time->Htime >= time->Rtime_JunctionsPipes) {
                    time->Rtime_JunctionsPipes += time->Rstep_JunctionsPipes;
                }
                if (errcode) return errcode;

                // If simulating water quality
                if (qual->Qualflag != NONE && time->Qtime < time->Dur)
                {
                    // ... compute reaction rate coeffs.
                    if (qual->Reactflag && qual->Qualflag != AGE) ratecoeffs(pr);

                    // ... topologically sort network nodes if flow directions change
                    if (flowdirchanged(pr) == TRUE)
                    {
                        errcode = sortnodes(pr);
                    }
                }
                if (!hyd->OpenHflag) time->Htime = hydtime + hydstep;
            }
            return errcode;
        };

        void     SaveResultsForQualityStep(EN_Project const& pr) {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Smatrix* sm = &hyd->smatrix;
            Report* rpt = &pr->report;
            Times* time = &pr->times;
            auto* qual = &pr->quality;

            u64 currentTime = (u64)pr->times.GetCurrentRealQtime();
            AUTO t = units::math::fmod(time->Qtime, time->Rstep_JunctionsPipes);
            for (auto i = 1; i <= net->Nnodes; i++) {
                auto& obj = net->Node[i];
                if (obj) {
                    if ((t == 0_s) || obj->Type_p == asset_t::RESERVOIR) {
                        AUTO pat = obj->GetValue<_QUALITY_>();
                        if (pat) {
                            pat->AddUniqueValue(currentTime, qual->NodeQual[i]);
                            pat->CompressLastValueAdded();
                        }
                    }
                }
            }
        };
        int      nextqual(EN_Project const& pr, units::time::second_t* tstep)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  tstep = time step (sec) over which quality was updated
            **   Returns: error code
            **   Purpose: updates water quality in network until next hydraulic
            **            event occurs (after tstep secs.)
            **--------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;
            Times* time = &pr->times;

            units::time::second_t hydstep;            // Time step until next hydraulic event
            units::time::second_t dt, qtime;
            int errcode = 0;

            // Find time step till next hydraulic event
            *tstep = 0;
            hydstep = 0;
            if (time->Htime <= time->Dur) hydstep = time->Htime - time->Qtime;

            // Perform water quality routing over this time step
            if (qual->Qualflag != NONE && hydstep > 0_s)
            {
                // Repeat over each quality time step until tstep is reached
                qtime = 0;
                while (!qual->OutOfMemory && qtime < hydstep)
                {
                    dt = fMIN(time->Qstep, hydstep - qtime);
                    qtime += dt;
                    transport(pr, dt);
                }
                if (qual->OutOfMemory) errcode = 101;
            }

            SaveResultsForQualityStep(pr);

            // Update mass balance ratio
            evalmassbalance(pr);

            // Update current time
            if (!errcode) *tstep = hydstep;
            time->Qtime += hydstep;

            // If no more time steps remain
            if (!errcode && *tstep == 0_s)
            {
                // ... report overall mass balance
                if (qual->Qualflag != NONE && pr->report.Statflag)
                {
                    writemassbalance(pr);
                }

                // ... write the final portion of the binary output file
                if (pr->outfile.Saveflag) errcode = savefinaloutput(pr);
            }
            return errcode;
        };
        int      stepqual(EN_Project const& pr, units::time::second_t* tleft)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  tleft = time left in simulation
            **   Returns: error code
            **   Purpose: updates quality conditions over a single
            **            quality time step
            **--------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;
            Times* time = &pr->times;

            units::time::second_t dt, hstep, t, tstep;
            int errcode = 0;

            tstep = time->Qstep;
            do
            {
                // Set local time step to quality time step
                dt = tstep;

                // Find time step until next hydraulic event
                hstep = time->Htime - time->Qtime;

                // If next hydraulic event occurs before end of local time step
                if (hstep < dt)
                {
                    // ... adjust local time step to next hydraulic event
                    dt = hstep;

                    // ... transport quality over local time step
                    if (qual->Qualflag != NONE) transport(pr, dt);
                    time->Qtime += dt;

                    // ... quit if running quality concurrently with hydraulics
                    if (pr->hydraul.OpenHflag) break;

                    // ... otherwise call runqual() to update hydraulics
                    errcode = runqual(pr, &t);
                    time->Qtime = t;
                }

                // Otherwise transport quality over current local time step
                else
                {
                    if (qual->Qualflag != NONE) transport(pr, dt);
                    time->Qtime += dt;
                }

                // Reduce quality time step by local time step
                tstep -= dt;
                if (qual->OutOfMemory) errcode = 101;

            } while (!errcode && tstep > 0_s);

            // Update mass balance ratio
            evalmassbalance(pr);

            // Update total simulation time left
            *tleft = time->Dur - time->Qtime;

            // If no more time steps remain
            if (!errcode && *tleft == 0_s)
            {
                // ... report overall mass balance
                if (qual->Qualflag != NONE && pr->report.Statflag)
                {
                    writemassbalance(pr);
                }

                // ... write the final portion of the binary output file
                if (pr->outfile.Saveflag) errcode = savefinaloutput(pr);
            }
            return errcode;
        };
        int      closequal(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: closes water quality solver
            **--------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;
            int errcode = 0;

            if (qual->Qualflag != NONE)
            {
                if (qual->SegPool) Mempool_t::mempool_delete(qual->SegPool);
                FREE(qual->FirstSeg);
                FREE(qual->LastSeg);
                qual->PipeRateCoeff.Clear();
                qual->FlowDir.Clear();
                qual->SortedNodes.Clear();
            }
            return errcode;
        };
        SCALER   avgqual(EN_Project const& pr, int k)
            /*
            **--------------------------------------------------------------
            **   Input:   k = link index
            **   Output:  returns quality concentration
            **   Purpose: computes current average quality in link k
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            SCALER vsum = 0.0, msum = 0.0;
            Pseg seg;

            if (qual->Qualflag == NONE) return 0.0;

            // Sum up the quality and volume in each segment of the link
            if (qual->FirstSeg != NULL)
            {
                seg = qual->FirstSeg[k];
                while (seg != NULL)
                {
                    vsum += seg->v;
                    msum += (seg->c) * (seg->v);
                    seg = seg->prev;
                }
            }

            // Compute average quality if link has volume
            if (vsum > 0.0) return (msum / vsum);

            // Otherwise use the average quality of the link's end nodes
            else
            {
                return ((qual->NodeQual[net->Link[k]->N1] +
                    qual->NodeQual[net->Link[k]->N2]) / 2.);
            }
        };
        SCALER   sourcequal(EN_Project const& pr, Psource source)
            /*
            **--------------------------------------------------------------
            **   Input:   source = a water quality source object
            **   Output:  returns strength of quality source
            **   Purpose: determines source strength in current time period
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Times* time = &pr->times;

            long k;
            SCALER c;

            // Get source concentration (or mass flow) in original units
            c = source->Concentration;

            // Convert mass flow rate from min. to sec.
            // and convert concen. from liters to cubic feet
            if (source->Type == MASS) c /= 60.0;
            else                      c /= pr->Ucf[QUALITY];

            // Apply time pattern if assigned
            if (source->TimePat) {
                return (c * source->TimePat->Pat((u64)time->GetCurrentRealQtime()));
            }
            else {
                return c;
            }
        };
        void     evalmassbalance(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: computes the overall mass balance ratio of a
            **            quality constituent.
            **--------------------------------------------------------------
            */
        {
            Quality* qual = &pr->quality;

            SCALER massin;
            SCALER massout;
            SCALER massreacted;

            if (qual->Qualflag == NONE) qual->MassBalance.ratio = 1.0;
            else
            {
                qual->MassBalance.final = findstoredmass(pr);
                massin = qual->MassBalance.initial + qual->MassBalance.inflow;
                massout = qual->MassBalance.outflow + qual->MassBalance.final;
                massreacted = qual->MassBalance.reacted;
                if (massreacted > 0.0) massout += massreacted;
                else                   massin -= massreacted;
                if (massin == 0.0) qual->MassBalance.ratio = 1.0;
                else               qual->MassBalance.ratio = massout / massin;
            }
        };
        SCALER   findstoredmass(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns total constituent mass stored in the network
            **   Purpose: finds the current mass of a constituent stored in
            **            all pipes and tanks.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Quality* qual = &pr->quality;

            int    i, k;
            SCALER totalmass = 0.0;
            Pseg   seg;

            // Mass residing in each pipe
            for (k = 1; k <= net->Nlinks; k++)
            {
                // Sum up the quality and volume in each segment of the link
                seg = qual->FirstSeg[k];
                while (seg != NULL)
                {
                    totalmass += (seg->c) * (seg->v);
                    seg = seg->prev;
                }
            }

            // Mass residing in each tank
            for (i = 1; i <= net->Ntanks; i++)
            {
                // ... skip reservoirs
                if (net->Tank[i]->Diameter == 0.0_ft) continue;

                // ... add up mass in each volume segment
                else
                {
                    k = net->Nlinks + i;
                    seg = qual->FirstSeg[k];
                    while (seg != NULL)
                    {
                        totalmass += seg->c * seg->v;
                        seg = seg->prev;
                    }
                }
            }
            return totalmass;
        };
        int      flowdirchanged(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns TRUE if flow direction changes in any link
            **   Purpose: finds new flow directions for each network link.
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;

            int k;
            int result = FALSE;
            int newdir;
            int olddir;
            SCALER q;

            // Examine each network link
            for (k = 1; k <= pr->network->Nlinks; k++)
            {
                // Determine sign (+1 or -1) of new flow rate
                olddir = qual->FlowDir[k];
                q = (double)((hyd->LinkStatus[k] <= CLOSED) ? 0.0_cfs : hyd->LinkFlow[k]);
                newdir = SGN(q);

                // Indicate if flow is negligible
                if ((cubic_foot_per_second_t)(double)ABS(q) < Q_STAGNANT) newdir = 0;

                // Reverse link's volume segments if flow direction changes sign
                if (newdir * olddir < 0) reversesegs(pr, k);

                // If flow direction changes either sign or magnitude then set
                // result to true (e.g., if a link's positive flow becomes
                // negligible then the network still needs to be re-sorted)
                if (newdir != olddir) result = TRUE;

                // ... replace old flow direction with the new direction
                qual->FlowDir[k] = (FlowDirection)newdir;
            }
            return result;
        };
        int      savehyd(EN_Project const& pr, units::time::second_t* htime)
            /*
            **--------------------------------------------------------------
            **   Input:   *htime   = current time
            **   Output:  returns error code
            **   Purpose: saves current hydraulic solution to file HydFile
            **            in binary format
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Outfile* out = &pr->outfile;
            Hydraul* hyd = &pr->hydraul;

            int i;
            INT4 t;
            int errcode = 0;
            REAL4* x;
            FILE* HydFile = out->HydFile;

            x = (REAL4*)calloc(fMAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
            if (x == NULL) return 101;

            // Save current time (htime)
            t = (INT4)(*htime);
            fwrite(&t, sizeof(INT4), 1, HydFile);

            // Save current nodal demands (D)
            for (i = 1; i <= net->Nnodes; i++) x[i] = (REAL4)hyd->NodeDemand[i];
            fwrite(x + 1, sizeof(REAL4), net->Nnodes, HydFile);
            //f_save(x, net->Nnodes, HydFile);

            // Save current nodal heads
            for (i = 1; i <= net->Nnodes; i++) x[i] = (REAL4)hyd->NodeHead[i];
            fwrite(x + 1, sizeof(REAL4), net->Nnodes, HydFile);
            //f_save(x, net->Nnodes, HydFile);

            // Force flow in closed links to be zero then save flows
            for (i = 1; i <= net->Nlinks; i++)
            {
                if (hyd->LinkStatus[i] <= CLOSED) x[i] = 0.0f;
                else x[i] = (REAL4)hyd->LinkFlow[i];
            }
            fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile);
            //f_save(x, net->Nlinks, HydFile);

            // Save link status
            for (i = 1; i <= net->Nlinks; i++) x[i] = (REAL4)hyd->LinkStatus[i];
            fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile);
            //f_save(x, net->Nlinks, HydFile);

            // Save link settings & check for successful write-to-disk
            // (We assume that if any of the previous fwrites failed,
            // then this one will also fail.)
            for (i = 1; i <= net->Nlinks; i++) x[i] = (REAL4)hyd->LinkSetting[i];
            if (fwrite(x + 1, sizeof(REAL4), net->Nlinks, HydFile) <
                (unsigned)net->Nlinks
                ) errcode = 308;
            //if (f_save(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) errcode = 308;
            free(x);
            fflush(HydFile);
            return errcode;
        };
        int      savehydstep(EN_Project const& pr, units::time::second_t* hydstep)
            /*
            **--------------------------------------------------------------
            **   Input:   *hydstep = next time step
            **   Output:  returns error code
            **   Purpose: saves next hydraulic timestep to file HydFile
            **            in binary format
            **--------------------------------------------------------------
            */
        {
            Outfile* out = &pr->outfile;

            INT4 t;
            int errcode = 0;

            t = (INT4)(*hydstep);
            if (fwrite(&t, sizeof(INT4), 1, out->HydFile) < 1) errcode = 308;
            if (t == 0) fputc(EOFMARK, out->HydFile);
            fflush(out->HydFile);
            return errcode;
        };
        int      readhyd(EN_Project const& pr, units::time::second_t* hydtime)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  *hydtime = time of hydraulic solution
            **   Returns: 1 if successful, 0 if not
            **   Purpose: reads hydraulic solution from file HydFile
            **
            **   NOTE: A hydraulic solution consists of the current time
            **         (hydtime), nodal demands (D) and heads (H), link
            **         flows (Q), link status (S), and link settings (K).
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Outfile* out = &pr->outfile;

            int i;
            INT4 t;
            int result = 1;
            REAL4* x;
            FILE* HydFile = out->HydFile;

            x = (REAL4*)calloc(fMAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
            if (x == NULL) return 0;

            if (fread(&t, sizeof(INT4), 1, HydFile) < 1) result = 0;
            *hydtime = t;

            if (f_read(x, net->Nnodes, HydFile) < (unsigned)net->Nnodes) result = 0;
            else for (i = 1; i <= net->Nnodes; i++) hyd->NodeDemand[i] = x[i];

            if (f_read(x, net->Nnodes, HydFile) < (unsigned)net->Nnodes) result = 0;
            else for (i = 1; i <= net->Nnodes; i++) hyd->NodeHead[i] = x[i];

            if (f_read(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) result = 0;
            else for (i = 1; i <= net->Nlinks; i++) hyd->LinkFlow[i] = x[i];

            if (f_read(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) result = 0;
            else for (i = 1; i <= net->Nlinks; i++) hyd->LinkStatus[i] = (StatusType)x[i];

            if (f_read(x, net->Nlinks, HydFile) < (unsigned)net->Nlinks) result = 0;
            else for (i = 1; i <= net->Nlinks; i++) hyd->LinkSetting[i] = x[i];

            free(x);
            return result;
        };
        int      readhydstep(EN_Project const& pr, units::time::second_t* hydstep)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  *hydstep = next hydraulic time step (sec)
            **   Returns: 1 if successful, 0 if not
            **   Purpose: reads hydraulic time step from file HydFile
            **--------------------------------------------------------------
            */
        {
            FILE* hydFile = pr->outfile.HydFile;
            INT4 t;

            if (fread(&t, sizeof(INT4), 1, hydFile) < 1) return 0;
            *hydstep = t;
            return 1;
        };
        int      saveoutput(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: writes simulation results to output file
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;

            int j;
            int errcode = 0;
            REAL4* x;

            x = (REAL4*)calloc(fMAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
            if (x == NULL) return 101;

            // Write out node results, then link results
            for (j = DEMAND; j <= QUALITY; j++) ERRCODE(nodeoutput(pr, j, x, pr->Ucf[j]));
            for (j = FLOW; j <= FRICTION; j++) ERRCODE(linkoutput(pr, j, x, pr->Ucf[j]));
            free(x);
            return errcode;
        };
        int      nodeoutput(EN_Project const& pr, int j, REAL4* x, SCALER ucf)
            /*
            **--------------------------------------------------------------
            **   Input:   j   = type of node variable
            **            *x  = buffer for node values
            **            ucf = units conversion factor
            **   Output:  returns error code
            **   Purpose: writes results for node variable j to output file
            **-----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Outfile* out = &pr->outfile;

            int i;
            FILE* outFile = out->TmpOutFile;

            // Load computed results (in proper units) into buffer x
            switch (j)
            {
            case DEMAND:
                for (i = 1; i <= net->Nnodes; i++)
                {
                    x[i] = (REAL4)(hyd->NodeDemand[i] * ucf);
                }
                break;

            case HEAD:
                for (i = 1; i <= net->Nnodes; i++)
                {
                    x[i] = (REAL4)(hyd->NodeHead[i] * ucf);
                }
                break;

            case PRESSURE:
                for (i = 1; i <= net->Nnodes; i++)
                {
                    x[i] = (REAL4)((hyd->NodeHead[i] - net->Node[i]->El) * ucf);
                }
                break;

            case QUALITY:
                for (i = 1; i <= net->Nnodes; i++)
                {
                    x[i] = (REAL4)(qual->NodeQual[i] * ucf);
                }
            }

            // Write x[1] to x[net->Nnodes] to output file
            if (f_save(x, net->Nnodes, outFile) < (unsigned)net->Nnodes) return 308;
            return 0;
        };
        int      linkoutput(EN_Project const& pr, int j, REAL4* x, SCALER ucf)
            /*
            **----------------------------------------------------------------
            **   Input:   j   = type of link variable
            **            *x  = buffer for link values
            **            ucf = units conversion factor
            **   Output:  returns error code
            **   Purpose: writes results for link variable j to output file
            **----------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Outfile* out = &pr->outfile;

            int i;
            SCALER a, h, q, f, setting;
            FILE* outFile = out->TmpOutFile;

            // Load computed results (in proper units) into buffer x
            switch (j)
            {
            case FLOW:
                for (i = 1; i <= net->Nlinks; i++)
                {
                    x[i] = (REAL4)(hyd->LinkFlow[i] * ucf);
                }
                break;

            case VELOCITY:
                for (i = 1; i <= net->Nlinks; i++)
                {
                    if (net->Link[i]->Type == PUMP) x[i] = 0.0f;
                    else
                    {
                        q = (double)ABS(hyd->LinkFlow[i]);
                        a = (double)(PI * SQR(net->Link[i]->Diam) / 4.0);
                        x[i] = (REAL4)(q / a * ucf);
                    }
                }
                break;

            case HEADLOSS:
                for (i = 1; i <= net->Nlinks; i++)
                {
                    if (hyd->LinkStatus[i] <= CLOSED) x[i] = 0.0f;
                    else
                    {
                        h = (double)(hyd->NodeHead[net->Link[i]->N1] - hyd->NodeHead[net->Link[i]->N2]);
                        if (net->Link[i]->Type != PUMP) h = ABS(h);
                        if (net->Link[i]->Type <= PIPE)
                        {
                            x[i] = (REAL4)(1000.0 * h / net->Link[i]->Len);
                        }
                        else x[i] = (REAL4)(h * ucf);
                    }
                }
                break;

            case LINKQUAL:
                for (i = 1; i <= net->Nlinks; i++)
                {
                    x[i] = (REAL4)(avgqual(pr, i) * ucf);
                }
                break;

            case STATUS:
                for (i = 1; i <= net->Nlinks; i++)
                {
                    x[i] = (REAL4)hyd->LinkStatus[i];
                }
                break;

            case SETTING:
                for (i = 1; i <= net->Nlinks; i++)
                {
                    setting = hyd->LinkSetting[i];
                    if (setting != MISSING) switch (net->Link[i]->Type)
                    {
                    case CVPIPE:
                    case PIPE:
                        x[i] = (REAL4)setting; break;
                    case PUMP:
                        x[i] = (REAL4)setting; break;
                    case PRV:
                    case PSV:
                    case PBV:
                        x[i] = (REAL4)(setting * pr->Ucf[PRESSURE]); break;
                    case FCV:
                        x[i] = (REAL4)(setting * pr->Ucf[FLOW]); break;
                    case TCV:
                        x[i] = (REAL4)setting; break;
                    default: x[i] = 0.0f;
                    }
                    else x[i] = 0.0f;
                }
                break;

            case REACTRATE: // Overall reaction rate in mass/L/day
                if (qual->Qualflag == NONE)
                {
                    memset(x, 0, (net->Nlinks + 1) * sizeof(REAL4));
                }
                else for (i = 1; i <= net->Nlinks; i++)
                {
                    x[i] = (REAL4)(qual->PipeRateCoeff[i] * ucf);
                }
                break;

            case FRICTION:  // Friction factor
                // f = 2ghd/(Lu^2) where f = friction factor
                // u = velocity, g = grav. accel., h = head loss
                //loss, d = diam., & L = pipe length
                for (i = 1; i <= net->Nlinks; i++)
                {
                    if (net->Link[i]->Type <= PIPE && ABS(hyd->LinkFlow[i]) > (cubic_foot_per_second_t)(double)TINY)
                    {
                        h = (double)ABS(hyd->NodeHead[net->Link[i]->N1] - hyd->NodeHead[net->Link[i]->N2]);
                        f = (double)(39.725 * h * units::math::pow<5>(net->Link[i]->Diam) / net->Link[i]->Len / SQR(hyd->LinkFlow[i]));
                        x[i] = (REAL4)f;
                    }
                    else x[i] = 0.0f;
                }
                break;
            }

            // Write x[1] to x[net->Nlinks] to output file
            if (f_save(x, net->Nlinks, outFile) < (unsigned)net->Nlinks) return 308;
            return 0;
        };
        int      savefinaloutput(EN_Project const& pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: saves time series statistics, reaction rates &
            **            epilog to output file.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Outfile* out = &pr->outfile;
            Report* rpt = &pr->report;
            Quality* qual = &pr->quality;

            int errcode = 0;
            REAL4* x;
            FILE* outFile = out->OutFile;

            // Save time series statistic if computed
            if (rpt->Tstatflag != SERIES && out->TmpOutFile != NULL)
            {
                x = (REAL4*)calloc(fMAX(net->Nnodes, net->Nlinks) + 1, sizeof(REAL4));
                if (x == NULL) return 101;
                ERRCODE(savetimestat(pr, x, NODEHDR));
                ERRCODE(savetimestat(pr, x, LINKHDR));
                if (!errcode) rpt->Nperiods = 1;
                fclose(out->TmpOutFile);
                out->TmpOutFile = NULL;
                free(x);
            }

            // Save avg. reaction rates & file epilog
            if (outFile != NULL)
            {
                ERRCODE(savenetreacts(pr, qual->Wbulk, qual->Wwall, qual->Wtank,
                    qual->Wsource));
                ERRCODE(saveepilog(pr));
            }
            return errcode;
        };
        int      savetimestat(EN_Project const& pr, REAL4* x, HdrType objtype)
            /*
            **--------------------------------------------------------------
            **   Input:   *x  = buffer for node values
            **            objtype = NODEHDR (for nodes) or LINKHDR (for links)
            **   Output:  returns error code
            **   Purpose: computes time series statistic for nodes or links
            **            and saves to normal output file.
            **
            **   NOTE: This routine is dependent on how the output reporting
            **         variables were assigned to FieldType in TYPES.H.
            **--------------------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Outfile* out = &pr->outfile;
            Report* rpt = &pr->report;

            int n, n1, n2;
            int i, j, p, errcode = 0;
            long startbyte, skipbytes;
            float* stat1, * stat2, xx;
            FILE* outFile = out->OutFile;

            // Compute number of bytes in temp output file to skip over (skipbytes)
            // when moving from one time period to the next for a particular variable
            if (objtype == NODEHDR)
            {
                // For nodes, we start at 0 and skip over node output for all
                // node variables minus 1 plus link output for all link variables.
                startbyte = 0;
                skipbytes = (net->Nnodes * (QUALITY - DEMAND) +
                    net->Nlinks * (FRICTION - FLOW + 1)) * sizeof(REAL4);
                n = net->Nnodes;
                n1 = DEMAND;
                n2 = QUALITY;
            }
            else
            {
                // For links, we start at the end of all node variables and skip
                // over node output for all node variables plus link output for
                // all link variables minus 1
                startbyte = net->Nnodes * (QUALITY - DEMAND + 1) * sizeof(REAL4);
                skipbytes = (net->Nnodes * (QUALITY - DEMAND + 1) +
                    net->Nlinks * (FRICTION - FLOW)) * sizeof(REAL4);
                n = net->Nlinks;
                n1 = FLOW;
                n2 = FRICTION;
            }
            stat1 = (float*)calloc(n + 1, sizeof(float));
            stat2 = (float*)calloc(n + 1, sizeof(float));
            ERRCODE(MEMCHECK(stat1));
            ERRCODE(MEMCHECK(stat2));

            // Process each output reporting variable
            if (!errcode) for (j = n1; j <= n2; j++)
            {
                // Initialize stat arrays
                if (rpt->Tstatflag == AVG) memset(stat1, 0, (n + 1) * sizeof(float));
                else for (i = 1; i <= n; i++)
                {
                    stat1[i] = -MISSING;
                    stat2[i] = MISSING;
                }

                // Position temp output file at start of output
                fseek(out->TmpOutFile, startbyte + (j - n1) * n * sizeof(REAL4),
                    SEEK_SET);

                // Process each time period
                for (p = 1; p <= rpt->Nperiods; p++)
                {
                    // Get output results for time period & update stats
                    f_read(x, n, out->TmpOutFile);
                    for (i = 1; i <= n; i++)
                    {
                        xx = x[i];
                        if (objtype == LINKHDR)
                        {
                            if (j == FLOW) xx = ABS(xx);
                            if (j == STATUS)
                            {
                                if (xx >= OPEN) xx = 1.0;
                                else            xx = 0.0;
                            }
                        }
                        if (rpt->Tstatflag == AVG) stat1[i] += xx;
                        else
                        {
                            stat1[i] = fMIN(stat1[i], xx);
                            stat2[i] = fMAX(stat2[i], xx);
                        }
                    }

                    // Advance file to next period
                    if (p < rpt->Nperiods) fseek(out->TmpOutFile, skipbytes, SEEK_CUR);
                }

                // Compute resultant stat & save to regular output file
                switch (rpt->Tstatflag)
                {
                case AVG:
                    for (i = 1; i <= n; i++) x[i] = stat1[i] / (float)rpt->Nperiods;
                    break;
                case MIN:
                    for (i = 1; i <= n; i++) x[i] = stat1[i];
                    break;
                case MAX:
                    for (i = 1; i <= n; i++) x[i] = stat2[i];
                    break;
                case RANGE:
                    for (i = 1; i <= n; i++) x[i] = stat2[i] - stat1[i];
                    break;
                }
                if (objtype == LINKHDR && j == STATUS)
                {
                    for (i = 1; i <= n; i++)
                    {
                        if (x[i] < 0.5f) x[i] = CLOSED;
                        else             x[i] = OPEN;
                    }
                }
                if (f_save(x, n, outFile) < (unsigned)n) errcode = 308;

                // Update internal output variables where applicable
                if (objtype == NODEHDR) switch (j)
                {
                case DEMAND:
                    for (i = 1; i <= n; i++) hyd->NodeDemand[i] = pr->convertToUnit<cubic_foot_per_second_t>(x[i]);
                    break;
                case HEAD:
                    for (i = 1; i <= n; i++) hyd->NodeHead[i] = pr->convertToUnit<foot_t>(x[i]);
                    break;
                case QUALITY:
                    for (i = 1; i <= n; i++) qual->NodeQual[i] = x[i] / pr->Ucf[QUALITY];
                    break;
                }
                else if (j == FLOW)
                {
                    for (i = 1; i <= n; i++) hyd->LinkFlow[i] = (double)(x[i] / pr->Ucf[FLOW]);
                }
            }

            // Free allocated memory
            free(stat1);
            free(stat2);
            return errcode;
        };
        int      savenetreacts(EN_Project const& pr, SCALER wbulk, SCALER wwall, SCALER wtank, SCALER wsource)
            /*
            **-----------------------------------------------------
            **  Writes average network-wide reaction rates (in
            **  mass/hr) to binary output file.
            **-----------------------------------------------------
            */
        {
            Outfile* out = &pr->outfile;
            Times* time = &pr->times;

            int errcode = 0;
            SCALER t;
            REAL4 w[4];
            FILE* outFile = out->OutFile;

            if (time->Dur > 0_s) t = (double)(time->Dur) / 3600.;
            else t = 1.;
            w[0] = (REAL4)(wbulk / t);
            w[1] = (REAL4)(wwall / t);
            w[2] = (REAL4)(wtank / t);
            w[3] = (REAL4)(wsource / t);
            if (fwrite(w, sizeof(REAL4), 4, outFile) < 4) errcode = 308;
            return errcode;
        };
        int      saveepilog(EN_Project const& pr)
            /*
            **-------------------------------------------------
            **  Writes Nperiods, Warnflag, & Magic Number to
            **  end of binary output file.
            **-------------------------------------------------
            */
        {
            Outfile* out = &pr->outfile;
            Report* rpt = &pr->report;

            int errcode = 0;
            INT4 i;
            FILE* outFile = out->OutFile;

            i = rpt->Nperiods;
            if (fwrite(&i, sizeof(INT4), 1, outFile) < 1) errcode = 308;
            i = pr->Warnflag;
            if (fwrite(&i, sizeof(INT4), 1, outFile) < 1) errcode = 308;
            i = MAGICNUMBER; if (fwrite(&i, sizeof(INT4), 1, outFile) < 1) errcode = 308;
            return errcode;
        };
        void     saveauxdata(EN_Project const& pr, FILE* f)
            /*
            ------------------------------------------------------------
                Writes auxilary data from original input file to new file.
            ------------------------------------------------------------
            */
        {
            int sect, newsect;
            char* tok;
            char write;
            char line[MAXLINE + 1];
            char s[MAXLINE + 1];
            FILE* InFile = pr->parser.InFile;

            // Re-open the input file
            if (InFile == NULL)
            {
                InFile = fopen(pr->parser.InpFname, "rt");
                if (InFile == NULL) return;
            }
            rewind(InFile);
            sect = -1;

            // Read each line of the input file
            while (fgets(line, MAXLINE, InFile) != NULL)
            {
                strcpy(s, line);
                tok = strtok(s, SEPSTR.c_str());
                if (tok == NULL) continue;

                // Check if line begins with a new section heading
                if (*tok == '[')
                {
                    newsect = findmatch(tok, SectTxt);
                    if (newsect >= 0)
                    {
                        sect = newsect;
                        if (sect == _END) break;

                        // Write section heading to file
                        switch (sect)
                        {
                        case _LABELS:
                        case _BACKDROP:
                        case _TAGS:
                            fprintf(f, "\n%s", line);
                        }
                    }
                }

                // Write line of auxilary data to file
                else
                {
                    write = FALSE;
                    switch (sect)
                    {
                    case _TAGS:
                        if (*tok == ';' ||
                            (match("NODE", tok) && findnode(pr->network, strtok(NULL, SEPSTR.c_str()))) ||
                            (match("LINK", tok) && findlink(pr->network, strtok(NULL, SEPSTR.c_str()))))
                            write = TRUE;
                        break;
                    case _LABELS:
                        write = TRUE;
                        break;
                    case _BACKDROP: {
                        if (match("DIMENSIONS", tok)) {
                            // get and write the project bounds.
                            cweeInterpolatedMatrix<float> x; // handles collection of sparse 2D coordinates

                            for (auto& node : pr->network->Node) {
                                if (node) {
                                    x.AddValue(node->X, node->Y, node->El());
                                }
                            }
                            for (auto& link : pr->network->Link) {
                                if (link && link->Vertices) {
                                    for (auto& vert : link->Vertices->Array) {
                                        x.AddValue(vert.first(), vert.second(), 0);
                                    }
                                }
                            }

                            AUTO minX = x.GetMinX();
                            AUTO minY = x.GetMinY();
                            AUTO width = x.GetMaxX() - minX;
                            AUTO height = x.GetMaxY() - minY;
                            AUTO widthD10 = width / 10.0;
                            AUTO heightD10 = height / 10.0;
                            minX -= widthD10 / 2.0;
                            minY -= heightD10 / 2.0;
                            width += widthD10;
                            height += heightD10;

                            fprintf(f, "%s", cweeStr::printf("DIMENSIONS %f %f %f %f",
                                (float)minX,
                                (float)minY,
                                (float)minX + width,
                                (float)minY + height
                            ).c_str());

                            write = FALSE;
                        }
                        else {
                            write = TRUE;
                        }
                        break;
                    }
                    default:
                        break;
                    }
                    if (write) fprintf(f, "%s", line);
                }
            }
            fclose(InFile);
            InFile = NULL;
        };;
        int      saveinpfile(EN_Project const& pr, const char* fname)
            /*
            -------------------------------------------------
                Writes network data to text file.
            -------------------------------------------------
            */
        {
            EN_Network const& net = pr->network;
            Parser* parser = &pr->parser;
            Report* rpt = &pr->report;
            Outfile* out = &pr->outfile;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Times* time = &pr->times;

            int i, j, n;
            SCALER d, kc, ke, km, ucf;
            char s[MAXLINE + 1], s1[MAXLINE + 1], s2[MAXLINE + 1];
            Psource source;
            FILE* f;
            Plink link;
            Ptank tank;
            Pnode node;
            Ppump pump;
            Pcontrol control;
            Pcurve curve;

            // Guard the new text file (prevent multiple threads from writing to the same filepath... which may result in a deadlock or bad writes)
            AUTO fileGuard = fileSystem->GuardFile(fname);

            // Open the new text file
            if ((f = fopen(fname, "wt")) == NULL) return 302;

            // Write [TITLE] section
            fprintf(f, s_TITLE);
            for (i = 0; i < 3; i++)
            {
                if (strlen(pr->Title[i]) > 0) fprintf(f, "\n%s", pr->Title[i]);
            }

            // Write [JUNCTIONS] section
            // (Leave demands for [DEMANDS] section)
            fprintf(f, "\n\n");
            fprintf(f, s_JUNCTIONS);
            for (i = 1; i <= net->Njuncs; i++)
            {
                node = net->Node[i];
                fprintf(f, "\n %-31s %12.4f", node->ID, (double)(node->El));
                if (node->Comment) fprintf(f, "  ;%s", node->Comment);
            }

            // Write [RESERVOIRS] section
            fprintf(f, "\n\n");
            fprintf(f, s_RESERVOIRS);
            for (i = 1; i <= net->Ntanks; i++)
            {
                tank = net->Tank[i];
                if (tank->Diameter == 0.0_ft)
                {
                    sprintf(s, " %-31s %12.4f", tank->ID, (double)(tank->El));
                    if (tank->TimePat) sprintf(s1, " %s", tank->TimePat->ID.c_str());
                    else strcpy(s1, " ");
                    fprintf(f, "\n%s %-31s", s, s1);
                    if (tank->Comment) fprintf(f, " ;%s", tank->Comment);
                }
            }

            // Write [TANKS] section
            fprintf(f, "\n\n");
            fprintf(f, s_TANKS);
            for (i = 1; i <= net->Ntanks; i++)
            {
                tank = net->Tank[i];
                if (tank->Diameter != 0.0_ft)
                {
                    sprintf(s, " %-31s %12.4f %12.4f %12.4f %12.4f %12.4f %12.4f",
                        tank->ID,
                        (double)(tank->El),
                        (double)((tank->InitialHead(pr->times.GetSimStartTime()) - tank->El)),
                        (double)((tank->Hmin - tank->El)),
                        (double)((tank->Hmax - tank->El)),
                        (double)(tank->Diameter),
                        (double)(tank->Vmin(pr)));
                    if ((j = tank->Vcurve) > 0) sprintf(s1, "%s", net->Curve[j]->ID);
                    else if (tank->CanOverflow) strcpy(s1, "*");
                    else strcpy(s1, " ");
                    fprintf(f, "\n%s %-31s", s, s1);
                    if (tank->CanOverflow) fprintf(f, "  YES  ");
                    if (tank->Comment) fprintf(f, " ;%s", tank->Comment);
                }
            }

            // Write [PIPES] section
            fprintf(f, "\n\n");
            fprintf(f, s_PIPES);
            for (i = 1; i <= net->Nlinks; i++)
            {
                link = net->Link[i];
                if (link->Type <= PIPE)
                {
                    d = (double)link->Diam;
                    kc = link->Kc;
                    if (hyd->Formflag == DW)  kc = kc * pr->Ucf[ELEV] * 1000.0;
                    km = link->Km * SQR(d) * SQR(d) / 0.02517;

                    sprintf(s, " %-31s %-31s %-31s %12.4f %12.4f %12.4f %12.4f",
                        link->ID,
                        net->Node[link->N1]->ID,
                        net->Node[link->N2]->ID,
                        (double)(link->Len * pr->Ucf[LENGTH]),
                        (double)(d * pr->Ucf[DIAM]),
                        (double)kc,
                        (double)km);

                    if (link->Type == CVPIPE) sprintf(s2, "CV");
                    else if ((StatusType)(double)link->Status(pr->times.GetSimStartTime()) == CLOSED) sprintf(s2, "Closed");
                    else strcpy(s2, "Open");
                    fprintf(f, "\n%s %-6s", s, s2);
                    if (link->Comment) fprintf(f, " ;%s", link->Comment);
                }
            }

            // Write [PUMPS] section
            fprintf(f, "\n\n");
            fprintf(f, s_PUMPS);
            for (i = 1; i <= net->Npumps; i++)
            {
                n = net->Pump[i]->Link;
                link = net->Link[n];
                pump = net->Pump[i];
                sprintf(s, " %-31s %-31s %-31s", link->ID, net->Node[link->N1]->ID,
                    net->Node[link->N2]->ID);

                // Pump has constant power
                if (pump->Ptype == CONST_HP) sprintf(s1, "  POWER %.4f", (double)link->Km);

                // Pump has a head curve
                else if ((j = pump->Hcurve) > 0)
                {
                    sprintf(s1, "  HEAD %s", net->Curve[j]->ID);
                }

                // Old format used for pump curve
                else
                {
                    fprintf(f, "\n%s %12.4f %12.4f %12.4f          0.0 %12.4f", s,
                        (double)(-pump->H0 * pr->Ucf[HEAD]),
                        (double)(((SCALER)(double)(-pump->H0) - pump->R * ::epanet::pow((SCALER)(double)pump->InitialFlow(pr->times.GetSimStartTime()), pump->N)) * pr->Ucf[HEAD]),
                        (double)(pump->InitialFlow(pr->times.GetSimStartTime()) * pr->Ucf[FLOW]),
                        (double)(pump->Qmax * pr->Ucf[FLOW]));
                    continue;
                }
                strcat(s, s1);

                // Optional speed pattern
                if (pump->TimeUpat) {
                    sprintf(s1, "  PATTERN  %s", pump->TimeUpat->ID.c_str());
                    strcat(s, s1);
                }

                // Optional speed setting
                if (link->Kc != 1.0)
                {
                    sprintf(s1, "  SPEED %.4f", (double)link->Kc);
                    strcat(s, s1);
                }

                fprintf(f, "\n%s", s);
                if (link->Comment) fprintf(f, "  ;%s", link->Comment);

            }

            // Write [VALVES] section
            fprintf(f, "\n\n");
            fprintf(f, s_VALVES);
            for (i = 1; i <= net->Nvalves; i++)
            {
                AUTO thisValve = net->Valve[i];
                n = net->Valve[i]->Link;
                link = net->Link[n];
                d = (double)link->Diam;

                // Valve setting
                kc = link->Kc;
                if (kc == MISSING) kc = 0.0;
                switch (link->Type)
                {
                case FCV:
                    kc *= pr->Ucf[FLOW];
                    break;
                case PRV:
                case PSV:
                case PBV:
                    kc *= pr->Ucf[PRESSURE];
                    break;
                default:
                    break;
                }
                km = link->Km * SQR(d) * SQR(d) / 0.02517;

                sprintf(s, " %-31s %-31s %-31s %12.4f %5s",
                    link->ID,
                    net->Node[link->N1]->ID,
                    net->Node[link->N2]->ID,
                    (double)(d * pr->Ucf[DIAM]),
                    LinkTxt[link->Type]);

                // For GPV, setting = head curve index
                if (link->Type == GPV && (j = ROUND(link->Kc)) > 0)
                {
                    sprintf(s1, "%-31s %12.4f", net->Curve[j]->ID, (double)km);
                }
                else sprintf(s1, "%12.4f %12.4f", (double)kc, (double)km);

                // Optional speed setting
                if (thisValve->ProducesElectricity)
                {
                    sprintf(s2, "  PAT %i", (int)(thisValve->ProducesElectricity ? 1 : 0));
                    strcat(s1, s2);
                }

                fprintf(f, "\n%s %s", s, s1);
                if (link->Comment) fprintf(f, " ;%s", link->Comment);
            }


            // Write [DEMANDS] section
            fprintf(f, "\n\n");
            fprintf(f, s_DEMANDS);
            ucf = pr->Ucf[DEMAND];
            for (i = 1; i <= net->Njuncs; i++)
            {
                node = net->Node[i];
                for (auto& demand : node->D) {
                    sprintf(s, " %-31s %14.6f", node->ID, (double)(ucf * demand.Base));
                    if (demand.TimePat) sprintf(s1, " %s", demand.TimePat->ID.c_str());
                    else strcpy(s1, " ");
                    fprintf(f, "\n%s %-31s", s, s1);
                    if (demand.Name) fprintf(f, " ;%s", demand.Name);
                }
            }


            // Write [EMITTERS] section
            fprintf(f, "\n\n");
            fprintf(f, s_EMITTERS);
            for (i = 1; i <= net->Njuncs; i++)
            {
                node = net->Node[i];
                if (node->Ke == 0.0) continue;
                ke = pr->Ucf[FLOW] / ::epanet::pow(pr->Ucf[PRESSURE] * node->Ke, (1.0 / hyd->Qexp));
                fprintf(f, "\n %-31s %14.6f", node->ID, (double)ke);
            }

            // Write [STATUS] section
            fprintf(f, "\n\n");
            fprintf(f, s_STATUS);
            for (i = 1; i <= net->Nlinks; i++)
            {
                link = net->Link[i];
                if (link->Type <= PIPE) {
                    // pipes's and check valve's initial status is written in the pipe section, not here. 
                    // Also, by this design, check valves can not be initially closed, ever? Interesting oversight.
                }
                else if (link->Type == PUMP)
                {

                    if ((StatusType)(double)link->Status(pr->times.GetSimStartTime()) == CLOSED)
                    {
                        fprintf(f, "\n %-31s %s", link->ID, "Closed");
                    }

                    // Write pump speed here for pumps with old-style pump curve input
                    else if (link->Type == PUMP)
                    {
                        n = findpump(net, i);
                        pump = net->Pump[n];
                        if (pump->Hcurve == 0 && pump->Ptype != CONST_HP &&
                            link->Kc != 1.0)
                        {
                            fprintf(f, "\n %-31s %-.4f", link->ID, (double)link->Kc);
                        }
                    }
                }

                // Write fixed-status PRVs & PSVs (setting = MISSING)
                else if (link->Kc == MISSING)
                {
                    if ((StatusType)(double)link->Status(pr->times.GetSimStartTime()) == OPEN)
                    {
                        fprintf(f, "\n %-31s %s", link->ID, "Open");
                    }
                    if ((StatusType)(double)link->Status(pr->times.GetSimStartTime()) == CLOSED)
                    {
                        fprintf(f, "\n%-31s %s", link->ID, "Closed");
                    }
                }
            }

            // Write [PATTERNS] section
            // (Use 6 pattern factors per line)
            fprintf(f, "\n\n");
            fprintf(f, s_PATTERNS);
            for (i = 1; i <= net->Npats; i++) {
                if (net->Pattern[i]->Comment.Length() > 0) fprintf(f, "\n;%s", net->Pattern[i]->Comment.c_str());
                else fprintf(f, "\n;%s", " ");
                AUTO pat = net->Pattern[i];
                if (pat) {
                    cweeStr line;
                    j = 0;
                    for (auto& knot : net->Pattern[i]->Pat.GetTimeSeries((u64)pr->times.GetPatternStartTime(), (u64)(pr->times.GetPatternStartTime() + (u64)pr->times.Dur - (u64)pr->times.Pstep), (u64)pr->times.Pstep)) {
                        ++j;

                        line.AddToDelimiter(cweeStr(knot.second()), " ");

                        if ((j % 6) == 0) {
                            line = pat->ID + " " + line;

                            fprintf(f, "\n %s", line.c_str());

                            line.Clear();
                        }
                    }
                    if ((j % 6) != 0) { // there is some value left
                        line = pat->ID + " " + line;
                        fprintf(f, "\n %s", line.c_str());
                        line.Clear();
                    }
                }
            }

            // Write [CURVES] section
            fprintf(f, "\n\n");
            fprintf(f, s_CURVES);
            for (i = 1; i <= net->Ncurves; i++)
            {
                if (net->Curve[i]->Comment.Length() > 0) fprintf(f, "\n;%s", net->Curve[i]->Comment.c_str());

                for (auto& knot : net->Curve[i]->Curve.GetKnotSeries()) {
                    curve = net->Curve[i];
                    fprintf(f, "\n %-31s %12.4f %12.4f", curve->ID, (double)knot.first, (double)knot.second);
                }
            }

            // Write [CONTROLS] section
            fprintf(f, "\n\n");
            fprintf(f, s_CONTROLS);
            for (i = 1; i <= net->Ncontrols; i++)
            {
                // Check that controlled link exists
                control = net->Control[i];
                if ((j = control->Link) <= 0) continue;
                link = net->Link[j];

                // Get text of control's link status/setting
                if (control->Setting == MISSING)
                {
                    sprintf(s, " LINK %s %s ", link->ID, StatTxt[control->Status]);
                }
                else
                {
                    kc = control->Setting;
                    switch (link->Type)
                    {
                    case PRV:
                    case PSV:
                    case PBV:
                        kc *= pr->Ucf[PRESSURE];
                        break;
                    case FCV:
                        kc *= pr->Ucf[FLOW];
                        break;
                    default:
                        break;
                    }
                    sprintf(s, " LINK %s %.4f", link->ID, (double)kc);
                }

                switch (control->Type)
                {
                    // Print level control
                case LOWLEVEL:
                case HILEVEL:
                    n = control->Node;
                    node = net->Node[n];
                    kc = (double)((foot_t)(double)control->Grade - node->El);
                    if (n > net->Njuncs) kc *= pr->Ucf[HEAD];
                    else kc *= pr->Ucf[PRESSURE];
                    fprintf(f, "\n%s IF NODE %s %s %.4f", s, node->ID,
                        ControlTxt[control->Type], (double)kc);
                    break;

                    // Print timer control
                case TIMER:
                    fprintf(f, "\n%s AT %s %.4f HOURS", s, ControlTxt[TIMER],
                        (double)(control->Time / 3600.));
                    break;

                    // Print time-of-day control
                case TIMEOFDAY:
                    fprintf(f, "\n%s AT %s %s", s, ControlTxt[TIMEOFDAY],
                        clocktime(rpt->Atime, control->Time));
                    break;
                }
            }

            // Write [RULES] section
            fprintf(f, "\n\n");
            fprintf(f, s_RULES);
            for (i = 1; i <= net->Nrules; i++)
            {
                fprintf(f, "\nRULE %s", pr->network->Rule[i]->label);
                writerule(pr, f, i);  // see RULES.C
                fprintf(f, "\n");
            }

            // Write [QUALITY] section
            // (Skip nodes with default quality of 0)
            fprintf(f, "\n\n");
            fprintf(f, s_QUALITY);
            for (i = 1; i <= net->Nnodes; i++)
            {
                node = net->Node[i];
                if (node->C0(pr->times.GetSimStartTime()) == 0.0) continue;
                fprintf(f, "\n %-31s %14.6f", node->ID, (double)(node->C0(pr->times.GetSimStartTime()) * pr->Ucf[QUALITY]));
            }

            // Write [SOURCES] section
            fprintf(f, "\n\n");
            fprintf(f, s_SOURCES);
            for (i = 1; i <= net->Nnodes; i++)
            {
                node = net->Node[i];
                source = node->S;
                if (source == NULL) continue;
                sprintf(s, " %-31s %-8s %14.6f", node->ID, SourceTxt[source->Type],
                    (double)source->Concentration);
                if (source->TimePat)
                {
                    sprintf(s1, "%s", source->TimePat->ID.c_str());
                }
                else strcpy(s1, "");
                fprintf(f, "\n%s %s", s, s1);
            }

            // Write [MIXING] section
            fprintf(f, "\n\n");
            fprintf(f, s_MIXING);
            for (i = 1; i <= net->Ntanks; i++)
            {
                tank = net->Tank[i];
                if (tank->Diameter == 0.0_ft) continue;
                fprintf(f, "\n %-31s %-8s %12.4f", tank->ID,
                    MixTxt[tank->MixModel], (double)tank->V1frac);
            }

            // Write [REACTIONS] section
            fprintf(f, "\n\n");
            fprintf(f, s_REACTIONS);

            // General parameters
            fprintf(f, "\n ORDER  BULK            %-.2f", (double)qual->BulkOrder);
            fprintf(f, "\n ORDER  WALL            %-.0f", (double)qual->WallOrder);
            fprintf(f, "\n ORDER  TANK            %-.2f", (double)qual->TankOrder);
            fprintf(f, "\n GLOBAL BULK            %-.6f", (double)(qual->Kbulk * SECperDAY));
            fprintf(f, "\n GLOBAL WALL            %-.6f", (double)(qual->Kwall * SECperDAY));

            if (qual->Climit > 0.0)
            {
                fprintf(f, "\n LIMITING POTENTIAL     %-.6f", (double)(qual->Climit * pr->Ucf[QUALITY]));
            }
            if (qual->Rfactor != MISSING && qual->Rfactor != 0.0)
            {
                fprintf(f, "\n ROUGHNESS CORRELATION  %-.6f", (double)(qual->Rfactor));
            }

            // Pipe-specific parameters
            for (i = 1; i <= net->Nlinks; i++)
            {
                link = net->Link[i];
                if (link->Type > PIPE) continue;
                if (link->Kb != qual->Kbulk)
                {
                    fprintf(f, "\n BULK   %-31s %-.6f", link->ID, (double)(link->Kb * SECperDAY));
                }
                if (link->Kw != qual->Kwall)
                {
                    fprintf(f, "\n WALL   %-31s %-.6f", link->ID, (double)(link->Kw * SECperDAY));
                }
            }

            // Tank parameters
            for (i = 1; i <= net->Ntanks; i++)
            {
                tank = net->Tank[i];
                if (tank->Diameter == 0.0_ft) continue;
                if (tank->Kb != qual->Kbulk)
                {
                    fprintf(f, "\n TANK   %-31s %-.6f", tank->ID,
                        (double)(tank->Kb * SECperDAY));
                }
            }

            // Write [ENERGY] section
            fprintf(f, "\n\n");
            fprintf(f, s_ENERGY);

            // General parameters
            if (hyd->Ecost != (Dollar_per_kilowatt_hour_t)0.0)
            {
                fprintf(f, "\n GLOBAL PRICE        %-.4f", (double)hyd->Ecost);
            }
            if (hyd->Epat != 0)
            {
                fprintf(f, "\n GLOBAL PATTERN      %s", net->Pattern[hyd->Epat]->ID.c_str());
            }
            fprintf(f, "\n GLOBAL EFFIC        %-.4f", (double)hyd->Epump);
            fprintf(f, "\n DEMAND CHARGE       %-.4f", (double)hyd->Dcost);

            // Pump-specific parameters
            for (i = 1; i <= net->Npumps; i++)
            {
                pump = net->Pump[i];
                if (pump->Ecost > (Dollar_per_kilowatt_hour_t)0.0)
                {
                    fprintf(f, "\n PUMP %-31s PRICE   %-.4f", pump->ID, (double)pump->Ecost);
                }
                if (pump->TimeEpat)
                {
                    fprintf(f, "\n PUMP %-31s PATTERN %s", pump->ID, pump->TimeEpat->ID.c_str());
                }
                if (pump->Ecurve > 0.0)
                {
                    fprintf(f, "\n PUMP %-31s EFFIC   %s", pump->ID, net->Curve[pump->Ecurve]->ID);
                }
            }

            // Write [TIMES] section
            fprintf(f, "\n\n");
            fprintf(f, s_TIMES);
            fprintf(f, "\n DURATION            %s", clocktime(rpt->Atime, time->Dur));
            fprintf(f, "\n HYDRAULIC TIMESTEP  %s", clocktime(rpt->Atime, time->Hstep));
            fprintf(f, "\n QUALITY TIMESTEP    %s", clocktime(rpt->Atime, time->Qstep));
            fprintf(f, "\n REPORT TIMESTEP     %s", clocktime(rpt->Atime, time->Rstep));
            fprintf(f, "\n REPORT START        %s", clocktime(rpt->Atime, time->Rstart));
            fprintf(f, "\n PATTERN TIMESTEP    %s", clocktime(rpt->Atime, time->Pstep));
            fprintf(f, "\n PATTERN START       %s", clocktime(rpt->Atime, time->Pstart));
            fprintf(f, "\n RULE TIMESTEP       %s", clocktime(rpt->Atime, time->Rulestep));
            fprintf(f, "\n START CLOCKTIME     %s", clocktime(rpt->Atime, time->Tstart));
            fprintf(f, "\n STATISTIC           %s", TstatTxt[rpt->Tstatflag]);

            // Write [OPTIONS] section
            fprintf(f, "\n\n");
            fprintf(f, s_OPTIONS);
            fprintf(f, "\n UNITS               %s", FlowUnitsTxt[parser->Flowflag]);
            fprintf(f, "\n PRESSURE            %s", PressUnitsTxt[parser->Pressflag]);
            fprintf(f, "\n HEADLOSS            %s", FormTxt[hyd->Formflag]);
            switch (out->Hydflag)
            {
            case USE:
                fprintf(f, "\n HYDRAULICS USE      %s", out->HydFname);
                break;
            case SAVE:
                fprintf(f, "\n HYDRAULICS SAVE     %s", out->HydFname);
                break;
            }
            if (hyd->ExtraIter == -1)
            {
                fprintf(f, "\n UNBALANCED          STOP");
            }
            if (hyd->ExtraIter >= 0)
            {
                fprintf(f, "\n UNBALANCED          CONTINUE %d", hyd->ExtraIter);
            }

            switch (qual->Qualflag)
            {
            case QualType::ENERGYINTENSITY:
            case QualType::CHEM:
                fprintf(f, "\n QUALITY             %s %s",
                    qual->ChemName, qual->ChemUnits);
                break;
            case QualType::TRACE_QUAL:
                fprintf(f, "\n QUALITY             TRACE %-31s",
                    net->Node[findnode(net, (char*)qual->TraceNode.c_str())]->ID);
                break;
            case QualType::AGE:
                fprintf(f, "\n QUALITY             AGE");
                break;
            case QualType::NONE:
                fprintf(f, "\n QUALITY             NONE");
                break;
            }

            fprintf(f, "\n DEMAND MULTIPLIER   %-.4f", (double)hyd->Dmult);
            fprintf(f, "\n EMITTER EXPONENT    %-.4f", (double)(1.0 / hyd->Qexp));
            fprintf(f, "\n VISCOSITY           %-.6f", (double)(hyd->Viscos / VISCOS));
            fprintf(f, "\n DIFFUSIVITY         %-.6f", (double)(qual->Diffus / DIFFUS));
            fprintf(f, "\n SPECIFIC GRAVITY    %-.6f", (double)hyd->SpGrav);
            fprintf(f, "\n TRIALS              %-d", hyd->MaxIter);
            fprintf(f, "\n ACCURACY            %-.8f", (double)hyd->Hacc);
            fprintf(f, "\n TOLERANCE           %-.8f", (double)(qual->Ctol * pr->Ucf[QUALITY]));
            fprintf(f, "\n CHECKFREQ           %-d", hyd->CheckFreq);
            fprintf(f, "\n MAXCHECK            %-d", hyd->MaxCheck);
            fprintf(f, "\n DAMPLIMIT           %-.8f", (double)hyd->DampLimit);
            if (hyd->HeadErrorLimit > 0.0_ft)
            {
                fprintf(f, "\n HEADERROR           %-.8f",
                    (double)(hyd->HeadErrorLimit * pr->Ucf[HEAD]));
            }
            if (hyd->FlowChangeLimit > 0.0_cfs)
            {
                fprintf(f, "\n FLOWCHANGE          %-.8f",
                    (double)(hyd->FlowChangeLimit * pr->Ucf[FLOW]));
            }
            if (hyd->DemandModel == PDA)
            {
                fprintf(f, "\n DEMAND MODEL        PDA");
                fprintf(f, "\n MINIMUM PRESSURE    %-.4f", (double)(hyd->Pmin * pr->Ucf[PRESSURE]));
                fprintf(f, "\n REQUIRED PRESSURE   %-.4f", (double)(hyd->Preq * pr->Ucf[PRESSURE]));
                fprintf(f, "\n PRESSURE EXPONENT   %-.4f", (double)hyd->Pexp);
            }

            // Write [REPORT] section
            fprintf(f, "\n\n");
            fprintf(f, s_REPORT);

            // General options
            fprintf(f, "\n PAGESIZE            %d", rpt->PageSize);
            fprintf(f, "\n STATUS              %s", RptFlagTxt[rpt->Statflag]);
            fprintf(f, "\n SUMMARY             %s", RptFlagTxt[rpt->Summaryflag]);
            fprintf(f, "\n ENERGY              %s", RptFlagTxt[rpt->Energyflag]);
            fprintf(f, "\n MESSAGES            %s", RptFlagTxt[rpt->Messageflag]);
            if (strlen(rpt->Rpt2Fname) > 0)
            {
                fprintf(f, "\n FILE                %s", rpt->Rpt2Fname);
            }

            // Node reporting
            switch (rpt->Nodeflag)
            {
            case 0:
                fprintf(f, "\n NODES               NONE");
                break;
            case 1:
                fprintf(f, "\n NODES               ALL");
                break;
            default:
                j = 0;
                for (i = 1; i <= net->Nnodes; i++)
                {
                    node = net->Node[i];
                    if (node->Rpt == 1)
                    {
                        if (j % 5 == 0) fprintf(f, "\n NODES               ");
                        fprintf(f, "%s ", node->ID);
                        j++;
                    }
                }
            }

            // Link reporting
            switch (rpt->Linkflag)
            {
            case 0:
                fprintf(f, "\n LINKS               NONE");
                break;
            case 1:
                fprintf(f, "\n LINKS               ALL");
                break;
            default:
                j = 0;
                for (i = 1; i <= net->Nlinks; i++)
                {
                    link = net->Link[i];
                    if (link->Rpt == 1)
                    {
                        if (j % 5 == 0) fprintf(f, "\n LINKS               ");
                        fprintf(f, "%s ", link->ID);
                        j++;
                    }
                }
            }

            // Field formatting options
            for (i = 0; i < FRICTION; i++)
            {
                SField* field = &rpt->Field[i];
                if (field->Enabled == TRUE)
                {
                    fprintf(f, "\n %-20sPRECISION %d", field->Name, field->Precision);
                    if (field->RptLim[LOW] < BIG)
                    {
                        fprintf(f, "\n %-20sBELOW %.6f", field->Name, (double)field->RptLim[LOW]);
                    }
                    if (field->RptLim[HI] > -BIG)
                    {
                        fprintf(f, "\n %-20sABOVE %.6f", field->Name, (double)field->RptLim[HI]);
                    }
                }
                else fprintf(f, "\n %-20sNO", field->Name);
            }

            // Write [COORDINATES] section
            fprintf(f, "\n\n");
            fprintf(f, s_COORDS);
            for (i = 1; i <= net->Nnodes; i++)
            {
                node = net->Node[i];
                if (node->X == MISSING || node->Y == MISSING) continue;
                fprintf(f, "\n %-31s %14.6f %14.6f", node->ID, (double)node->X, (double)node->Y);
            }

            // Write [VERTICES] section
            fprintf(f, "\n\n");
            fprintf(f, s_VERTICES);
            for (i = 1; i <= net->Nlinks; i++)
            {
                link = net->Link[i];
                if (link->Vertices != NULL)
                {
                    for (j = 0; j < link->Vertices->Array.Num(); j++)
                        fprintf(f, "\n %-31s %14.6f %14.6f",
                            link->ID, (double)link->Vertices->Array[j].first, (double)link->Vertices->Array[j].second);
                }
            }

            // Save auxilary data to new input file
            fprintf(f, "\n");
            saveauxdata(pr, f);

            // Close the new input file
            fprintf(f, "\n%s\n", (char*)s_END.c_str());
            fclose(f);
            return 0;
        };








    };
};