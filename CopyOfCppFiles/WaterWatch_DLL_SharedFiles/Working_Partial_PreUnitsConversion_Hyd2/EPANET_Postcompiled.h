#pragma once
#include "Precompiled.h"

namespace epanet {
    class epanet_postcompiled { public:
        static void		initrules(EN_Project pr)
                //--------------------------------------------------------------
                //    Initializes rule base.
                //--------------------------------------------------------------
            {
                pr->rules.RuleState = r_PRIORITY;
                pr->rules.LastPremise = NULL;
                pr->rules.LastThenAction = NULL;
                pr->rules.LastElseAction = NULL;
                pr->rules.ActionList = NULL;
                pr->network->Rule = NULL;
            };
        static void		initpointers(EN_Project pr)
            /*----------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: initializes data array pointers to NULL
            **----------------------------------------------------------------
            */
        {
            EN_Network nw = pr->network;
            nw->Nnodes = 0;
            nw->Ntanks = 0;
            nw->Njuncs = 0;
            nw->Nlinks = 0;
            nw->Npipes = 0;
            nw->Npumps = 0;
            nw->Nvalves = 0;
            nw->Ncontrols = 0;
            nw->Nrules = 0;
            nw->Npats = 0;
            nw->Ncurves = 0;

            pr->hydraul.NodeDemand.Clear();
            pr->hydraul.NodeHead.Clear();
            pr->hydraul.LinkFlow.Clear();
            pr->hydraul.LinkStatus .Clear();
            pr->hydraul.LinkSetting.Clear();
            pr->hydraul.OldStatus .Clear();
            pr->hydraul.P.Clear();
            pr->hydraul.Y.Clear();
            pr->hydraul.Xflow.Clear();
            pr->hydraul.DemandFlow.Clear();
            pr->hydraul.EmitterFlow.Clear();

            pr->quality.NodeQual.Clear();
            pr->quality.PipeRateCoeff.Clear();

            pr->network->Node.Clear();
            pr->network->Link.Clear();
            pr->network->Tank.Clear();
            pr->network->Pump.Clear();
            pr->network->Valve.Clear();
            pr->network->Pattern.Clear();
            pr->network->Curve.Clear();
            pr->network->Control.Clear();
            pr->network->Adjlist.Clear();
            pr->network->NodeHashTable = NULL;
            pr->network->LinkHashTable = NULL;

            pr->hydraul.smatrix.Aii.Clear();
            pr->hydraul.smatrix.Aij.Clear();
            pr->hydraul.smatrix.F.Clear();
            pr->hydraul.smatrix.Order.Clear();
            pr->hydraul.smatrix.Row.Clear();
            pr->hydraul.smatrix.Ndx.Clear();
            pr->hydraul.smatrix.XLNZ.Clear();
            pr->hydraul.smatrix.NZSUB.Clear();
            pr->hydraul.smatrix.LNZ.Clear();

            initrules(pr);
        };
        static int		allocrules(EN_Project pr)
            //--------------------------------------------------------------
            //    Allocates memory for rule-based controls.
            //--------------------------------------------------------------
        {
            EN_Network net = pr->network;
            int n = pr->parser.MaxRules + 1;

            net->Rule.ClearedResize(n);

            return 0;
        };
        static int		allocdata(EN_Project pr)
            /*----------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Returns: error code
            **  Purpose: allocates memory for network data structures
            **----------------------------------------------------------------
            */
        {
            int n;
            int errcode = 0;

            // Allocate node & link ID hash tables
            pr->network->NodeHashTable = hashtable_t::hashtable_create();
            pr->network->LinkHashTable = hashtable_t::hashtable_create();
            ERRCODE(MEMCHECK(pr->network->NodeHashTable));
            ERRCODE(MEMCHECK(pr->network->LinkHashTable));

            // Allocate memory for network nodes
            //*************************************************************
            // NOTE: Because network components of a given type are indexed
            // starting from 1, their arrays must be sized 1
            // element larger than the number of components.
            //*************************************************************
            if (!errcode)
            {
                n = pr->parser.MaxNodes + 1;
                pr->network->Node.ClearedResize(n); for (auto& x : pr->network->Node) { if (!x) { x = make_cwee_shared<Snode>(); } }
                pr->hydraul.NodeDemand.ClearedResize(n);
                pr->hydraul.NodeHead.ClearedResize(n);
                pr->quality.NodeQual.ClearedResize(n);
                pr->hydraul.DemandFlow.ClearedResize(n);
                pr->hydraul.EmitterFlow.ClearedResize(n);
            }

            // Allocate memory for network links
            if (!errcode)
            {
                n = pr->parser.MaxLinks + 1;
                pr->network->Link.ClearedResize(n); for (auto& x : pr->network->Link) { if (!x) x = make_cwee_shared<Slink>(); }
                pr->hydraul.LinkFlow.ClearedResize(n);
                pr->hydraul.LinkSetting.ClearedResize(n);
                pr->hydraul.LinkStatus.ClearedResize(n);
            }

            // Allocate memory for tanks, sources, pumps, valves, and controls
            // (memory for Patterns and Curves arrays expanded as each is added)
            if (!errcode)
            {
                pr->network->Tank.ClearedResize(pr->parser.MaxTanks + 1); for (auto& x : pr->network->Tank) if (!x) x = make_cwee_shared<Stank>();
                pr->network->Pump.ClearedResize(pr->parser.MaxPumps + 1); for (auto& x : pr->network->Pump) if (!x) x = make_cwee_shared<Spump>();
                pr->network->Valve.ClearedResize(pr->parser.MaxValves + 1); for (auto& x : pr->network->Valve) if (!x) x = make_cwee_shared<Svalve>();
                pr->network->Control.ClearedResize(pr->parser.MaxControls + 1);
            }

            // Initialize pointers used in nodes and links
            if (!errcode)
            {
                for (n = 0; n <= pr->parser.MaxNodes; n++)
                {
                    pr->network->Node[n]->D = NULL;    // node demand
                    pr->network->Node[n]->S = nullptr;    // node source
                    pr->network->Node[n]->Comment = NULL;
                }
                for (n = 0; n <= pr->parser.MaxLinks; n++)
                {
                    pr->network->Link[n]->Vertices = nullptr;
                    pr->network->Link[n]->Comment = NULL;
                }
            }

            // Allocate memory for rule base (see RULES.C)
            if (!errcode) errcode = allocrules(pr);
            return errcode;
        };
        static void		freedemands(Pnode node)
            /*----------------------------------------------------------------
            **  Input:   node = a network junction node
            **  Output:  node
            **  Purpose: frees the memory used for a node's list of demands.
            **----------------------------------------------------------------
            */
        {
            Pdemand nextdemand;
            Pdemand demand = node->D;
            while (demand != NULL)
            {
                nextdemand = demand->next;
                free(demand->Name);
                free(demand);
                demand = nextdemand;
            }
            node->D = NULL;
        };
        static void		freelinkvertices(Plink link)
            /*----------------------------------------------------------------
            **  Input:   vertices = list of link vertex points
            **  Output:  none
            **  Purpose: frees the memory used for a link's list of vertices.
            **----------------------------------------------------------------
            */
        {
            if (link->Vertices)
            {
                link->Vertices->X.Clear();
                link->Vertices->Y.Clear();
                link->Vertices = nullptr;
            }
        };
        static void		freerules(EN_Project pr)
            //--------------------------------------------------------------
            //    Frees memory used for rule-based controls.
            //--------------------------------------------------------------
        {
            int i;

            // Already freed
            if (pr->network->Rule == NULL)
                return;

            for (i = 1; i <= pr->network->Nrules; i++) epanet_shared::clearrule(pr, i);
            pr->network->Rule.Clear();
        };
        static void		freedata(EN_Project pr)
            /*----------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: frees memory allocated for network data structures.
            **----------------------------------------------------------------
            */
        {
            int j;

            // Free memory for computed results
            pr->hydraul.NodeDemand.Clear();
            pr->hydraul.NodeHead.Clear();
            pr->hydraul.LinkFlow.Clear();
            pr->hydraul.LinkSetting.Clear();
            pr->hydraul.LinkStatus.Clear();
            pr->hydraul.DemandFlow.Clear();
            pr->hydraul.EmitterFlow.Clear();
            pr->quality.NodeQual.Clear();

            // Free memory used for nodal adjacency lists
            smatrix_t::freeadjlists(pr->network);

            // Free memory for node data
            //if (pr->network->Node != NULL)
            {
                for (j = 1; j <= pr->network->Nnodes; j++)
                {
                    // Free memory used for demands and WQ source data
                    freedemands(pr->network->Node[j]);
                    pr->network->Node[j]->S = nullptr;
                    free(pr->network->Node[j]->Comment);
                }
                pr->network->Node.Clear();
            }

            // Free memory for link data
            //if (pr->network->Link != NULL)
            {
                for (j = 1; j <= pr->network->Nlinks; j++)
                {
                    freelinkvertices(pr->network->Link[j]);
                    free(pr->network->Link[j]->Comment);
                }
            }
            pr->network->Link.Clear();

            // Free memory for other network objects
            pr->network->Tank.Clear();
            pr->network->Pump.Clear();
            pr->network->Valve.Clear();
            pr->network->Control.Clear();

            // Free memory for time patterns
            if (pr->network->Pattern != NULL)
            {
                for (j = 0; j <= pr->network->Npats; j++)
                {
                    pr->network->Pattern[j].Pat.Clear();
                    free(pr->network->Pattern[j].Comment);
                }
                pr->network->Pattern.Clear();
            }

            // Free memory for curves
            if (pr->network->Curve != NULL)
            {
                // There is no Curve[0]
                for (j = 1; j <= pr->network->Ncurves; j++)
                {
                    free(pr->network->Curve[j].X);
                    free(pr->network->Curve[j].Y);
                    free(pr->network->Curve[j].Comment);
                }
                pr->network->Curve.Clear();
            }

            // Free memory for rule base (see RULES.C)
            freerules(pr);

            // Free hash table memory
            if (pr->network->NodeHashTable != NULL)
            {
                hashtable_t::hashtable_free(pr->network->NodeHashTable);
            }
            if (pr->network->LinkHashTable != NULL)
            {
                hashtable_t::hashtable_free(pr->network->LinkHashTable);
            }
        };
        static void		writelogo(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: writes program logo to report file.
            **--------------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;

            int version;
            int major;
            int minor;
            // char s[80];
            time_t timer; // time_t structure & functions time() & ctime() are defined in time.h

            version = CODEVERSION;
            major = version / 10000;
            minor = (version % 10000) / 100;

            ::time(&timer);
            ::strcpy(rpt->DateStamp, ctime(&timer));
            rpt->PageNum = 1;
            rpt->LineNum = 2;
        };
        static int		openfiles(EN_Project pr, const char* f1, const char* f2, const char* f3)
            /*----------------------------------------------------------------
            **  Input:   f1 = pointer to name of input file
            **           f2 = pointer to name of report file
            **           f3 = pointer to name of binary output file
            **  Output:  none
            **  Returns: error code
            **  Purpose: opens input & report files
            **----------------------------------------------------------------
            */
        {
            // Initialize file pointers to NULL
            pr->parser.InFile = NULL;
            pr->report.RptFile = NULL;
            pr->outfile.OutFile = NULL;
            pr->outfile.HydFile = NULL;
            pr->outfile.TmpOutFile = NULL;

            // Save file names
            strncpy(pr->parser.InpFname, f1, MAXFNAME);
            strncpy(pr->report.Rpt1Fname, f2, MAXFNAME);
            strncpy(pr->outfile.OutFname, f3, MAXFNAME);
            if (strlen(f3) > 0) pr->outfile.Outflag = SAVE;
            else
            {
                pr->outfile.Outflag = SCRATCH;
                strcpy(pr->outfile.OutFname, pr->TmpOutFname.c_str());
            }

            // Check that file names are not identical
            if (strlen(f1) > 0 && (strcomp(f1, f2) || strcomp(f1, f3))) return 301;
            if (strlen(f3) > 0 && strcomp(f2, f3)) return 301;

            // Attempt to open input and report files
            if (strlen(f1) > 0)
            {
                if ((pr->parser.InFile = fopen(f1, "rt")) == NULL) return 302;
            }
            if (strlen(f2) == 0) pr->report.RptFile = stdout;
            else
            {
                pr->report.RptFile = fopen(f2, "wt");
                if (pr->report.RptFile == NULL) return 303;
            }
            writelogo(pr);
            return 0;
        };
        static int		openhydfile(EN_Project pr)
            /*----------------------------------------------------------------
            ** Input:   none
            ** Output:  none
            ** Returns: error code
            ** Purpose: opens file that saves hydraulics solution
            **----------------------------------------------------------------
            */
        {
            const int Nnodes = pr->network->Nnodes;
            const int Ntanks = pr->network->Ntanks;
            const int Nlinks = pr->network->Nlinks;
            const int Nvalves = pr->network->Nvalves;
            const int Npumps = pr->network->Npumps;

            INT4 nsize[6]; // Temporary array
            INT4 magic;
            INT4 version;
            int errcode = 0;

            // If HydFile currently open, then close it if its not a scratch file
            if (pr->outfile.HydFile != NULL)
            {
                if (pr->outfile.Hydflag == SCRATCH) return 0;
                fclose(pr->outfile.HydFile);
                pr->outfile.HydFile = NULL;
            }

            // Use Hydflag to determine the type of hydraulics file to use.
            // Write error message if the file cannot be opened.
            pr->outfile.HydFile = NULL;
            switch (pr->outfile.Hydflag)
            {
            case SCRATCH:
                strcpy(pr->outfile.HydFname, pr->TmpHydFname.c_str());
                pr->outfile.HydFile = fopen(pr->outfile.HydFname, "w+b");
                break;
            case SAVE:
                pr->outfile.HydFile = fopen(pr->outfile.HydFname, "w+b");
                break;
            case USE:
                pr->outfile.HydFile = fopen(pr->outfile.HydFname, "rb");
                break;
            }
            if (pr->outfile.HydFile == NULL) return 305;

            // If a previous hydraulics solution is not being used, then
            // save the current network size parameters to the file.
            if (pr->outfile.Hydflag != USE)
            {
                magic = MAGICNUMBER;
                version = ENGINE_VERSION;
                nsize[0] = Nnodes;
                nsize[1] = Nlinks;
                nsize[2] = Ntanks;
                nsize[3] = Npumps;
                nsize[4] = Nvalves;
                nsize[5] = (int)pr->times.Dur;
                fwrite(&magic, sizeof(INT4), 1, pr->outfile.HydFile);
                fwrite(&version, sizeof(INT4), 1, pr->outfile.HydFile);
                fwrite(nsize, sizeof(INT4), 6, pr->outfile.HydFile);
            }

            // If a previous hydraulics solution is being used, then
            // make sure its network size parameters match those of
            // the current network
            if (pr->outfile.Hydflag == USE)
            {
                fread(&magic, sizeof(INT4), 1, pr->outfile.HydFile);
                if (magic != MAGICNUMBER) return 306;
                fread(&version, sizeof(INT4), 1, pr->outfile.HydFile);
                if (version != ENGINE_VERSION) return 306;
                if (fread(nsize, sizeof(INT4), 6, pr->outfile.HydFile) < 6) return 306;
                if (nsize[0] != Nnodes || nsize[1] != Nlinks || nsize[2] != Ntanks ||
                    nsize[3] != Npumps || nsize[4] != Nvalves ||
                    nsize[5] != (long)(double)pr->times.Dur
                    ) return 306;
                pr->outfile.SaveHflag = TRUE;
            }

            // Save current position in hydraulics file
            // where storage of hydraulic results begins
            pr->outfile.HydOffset = ftell(pr->outfile.HydFile);
            return errcode;
        };
        static void		closeoutfile(EN_Project pr)
            /*----------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Purpose: closes binary output file.
            **----------------------------------------------------------------
            */
        {
            if (pr->outfile.TmpOutFile != pr->outfile.OutFile)
            {
                if (pr->outfile.TmpOutFile != NULL)
                {
                    fclose(pr->outfile.TmpOutFile);
                    pr->outfile.TmpOutFile = NULL;
                }
            }
            if (pr->outfile.OutFile != NULL)
            {
                if (pr->outfile.OutFile == pr->outfile.TmpOutFile)
                {
                    pr->outfile.TmpOutFile = NULL;
                }
                fclose(pr->outfile.OutFile);
                pr->outfile.OutFile = NULL;
            }
        };
        static int		savenetdata(EN_Project pr)
            /*
            **---------------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: saves input data in original units to binary
            **            output file using fixed-sized (4-byte) records
            **---------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Outfile* out = &pr->outfile;
            Report* rpt = &pr->report;
            Quality* qual = &pr->quality;
            Parser* parser = &pr->parser;
            Times* time = &pr->times;

            int i, nmax;
            int errcode = 0;
            INT4* ibuf;
            REAL4* x;
            Pnode node;
            FILE* outFile = out->OutFile;

            // Allocate buffer arrays
            nmax = fMAX(net->Nnodes, net->Nlinks) + 1;
            nmax = fMAX(nmax, 15);
            ibuf = (INT4*)calloc(nmax, sizeof(INT4));
            x = (REAL4*)calloc(nmax, sizeof(REAL4));
            ERRCODE(MEMCHECK(ibuf));
            ERRCODE(MEMCHECK(x));

            // Write prolog section of binary output file
            if (!errcode)
            {
                // Write integer variables to outFile
                ibuf[0] = MAGICNUMBER;
                ibuf[1] = 20012;  // keep version at 2.00.12 so that GUI will run
                ibuf[2] = net->Nnodes;
                ibuf[3] = net->Ntanks;
                ibuf[4] = net->Nlinks;
                ibuf[5] = net->Npumps;
                ibuf[6] = net->Nvalves;
                ibuf[7] = qual->Qualflag;
                ibuf[8] = epanet_shared::findnode(net, (char*)qual->TraceNode.c_str()); //  qual->TraceNode;
                ibuf[9] = parser->Flowflag;
                ibuf[10] = parser->Pressflag;
                ibuf[11] = rpt->Tstatflag;
                ibuf[12] = (INT4)time->Rstart;
                ibuf[13] = (INT4)time->Rstep;
                ibuf[14] = (INT4)time->Dur;
                fwrite(ibuf, sizeof(INT4), 15, outFile);

                // Write string variables to outFile
                fwrite(pr->Title[0], sizeof(char), TITLELEN + 1, outFile);
                fwrite(pr->Title[1], sizeof(char), TITLELEN + 1, outFile);
                fwrite(pr->Title[2], sizeof(char), TITLELEN + 1, outFile);
                fwrite(parser->InpFname, sizeof(char), MAXFNAME + 1, outFile);
                fwrite(rpt->Rpt2Fname, sizeof(char), MAXFNAME + 1, outFile);
                fwrite(qual->ChemName, sizeof(char), MAXID + 1, outFile);
                fwrite(rpt->Field[QUALITY].Units, sizeof(char), MAXID + 1, outFile);

                // Write node ID information to outFile
                for (i = 1; i <= net->Nnodes; i++)
                {
                    node = net->Node[i];
                    fwrite(node->ID, MAXID + 1, 1, outFile);
                }

                // Write link information to outFile
                // (Note: first transfer values to buffer array,
                // then fwrite buffer array at offset of 1 )
                for (i = 1; i <= net->Nlinks; i++)
                {
                    fwrite(net->Link[i]->ID, MAXID + 1, 1, outFile);
                }

                for (i = 1; i <= net->Nlinks; i++) ibuf[i] = net->Link[i]->N1;
                fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

                for (i = 1; i <= net->Nlinks; i++) ibuf[i] = net->Link[i]->N2;
                fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

                for (i = 1; i <= net->Nlinks; i++) ibuf[i] = net->Link[i]->Type;
                fwrite(ibuf + 1, sizeof(INT4), net->Nlinks, outFile);

                // Write tank information to outFile
                for (i = 1; i <= net->Ntanks; i++) ibuf[i] = net->Tank[i]->Node;
                fwrite(ibuf + 1, sizeof(INT4), net->Ntanks, outFile);

                for (i = 1; i <= net->Ntanks; i++) x[i] = (REAL4)net->Tank[i]->A;
                f_save(x, net->Ntanks, outFile);

                // Save node elevations to outFile
                for (i = 1; i <= net->Nnodes; i++)
                {
                    x[i] = (REAL4)(net->Node[i]->El * pr->Ucf[ELEV]);
                }
                f_save(x, net->Nnodes, outFile);

                // Save link lengths & diameters to outFile
                for (i = 1; i <= net->Nlinks; i++)
                {
                    x[i] = (REAL4)(net->Link[i]->Len * pr->Ucf[ELEV]);
                }
                f_save(x, net->Nlinks, outFile);

                for (i = 1; i <= net->Nlinks; i++)
                {
                    if (net->Link[i]->Type != PUMP)
                    {
                        x[i] = (REAL4)(net->Link[i]->Diam * pr->Ucf[DIAM]);
                    }
                    else x[i] = 0.0f;
                }
                if (f_save(x, net->Nlinks, outFile) < (unsigned)net->Nlinks) errcode = 308;
            }

            // Free memory used for buffer arrays
            free(ibuf);
            free(x);
            return errcode;
        };
        static int		saveenergy(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: saves energy usage by each pump to outFile
            **            in binary format
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Outfile* out = &pr->outfile;
            Parser* parser = &pr->parser;
            Times* time = &pr->times;

            int    i;
            INT4   index;
            REAL4  x[6];                // work array
            SCALER hdur,               // total simulation duration in hours
                t;                  // total pumping time duration
            Ppump pump;
            FILE* outFile = out->OutFile;

            hdur = (double)(time->Dur) / 3600.0;
            for (i = 1; i <= net->Npumps; i++)
            {
                pump = net->Pump[i];
                if (hdur == 0.0) pump->Energy.TotalCost *= 24.0;
                else
                {
                    // ... convert total hrs. online to fraction of total time online
                    t = pump->Energy.TimeOnLine;  //currently holds total hrs. online
                    pump->Energy.TimeOnLine = t / hdur;

                    // ... convert cumulative values to time-averaged ones
                    if (t > 0.0)
                    {
                        pump->Energy.Efficiency /= t;
                        pump->Energy.KwHrsPerFlow /= t;
                        pump->Energy.KwHrs /= t;
                    }

                    // ... convert total cost to cost per day
                    pump->Energy.TotalCost *= 24.0 / hdur;
                }

                // ... express time online and avg. efficiency as percentages
                pump->Energy.TimeOnLine *= 100.0;
                pump->Energy.Efficiency *= 100.0;

                // ... compute KWH per Million Gallons or per Cubic Meter
                if (parser->Unitsflag == SI)
                {
                    pump->Energy.KwHrsPerFlow *= (1000. / LPSperCFS / 3600.);
                }
                else pump->Energy.KwHrsPerFlow *= (1.0e6 / GPMperCFS / 60.);

                // ... save energy stats to REAL4 work array
                x[0] = (REAL4)pump->Energy.TimeOnLine;
                x[1] = (REAL4)pump->Energy.Efficiency;
                x[2] = (REAL4)pump->Energy.KwHrsPerFlow;
                x[3] = (REAL4)pump->Energy.KwHrs;
                x[4] = (REAL4)pump->Energy.MaxKwatts;
                x[5] = (REAL4)pump->Energy.TotalCost;

                // ... save energy results to output file
                index = pump->Link;
                if (fwrite(&index, sizeof(INT4), 1, outFile) < 1) return 308;
                if (fwrite(x, sizeof(REAL4), 6, outFile) < 6) return 308;
            }

            // ... compute and save demand charge
            x[0] = (REAL4)(hyd->Emax * hyd->Dcost);
            if (fwrite(&x[0], sizeof(REAL4), 1, outFile) < 1) return 308;
            return (0);
        };
        static int		openoutfile(EN_Project pr)
            /*----------------------------------------------------------------
            **  Input:   none
            **  Output:  none
            **  Returns: error code
            **  Purpose: opens binary output file.
            **----------------------------------------------------------------
            */
        {
            int errcode = 0;

            // Close output file if already opened
            closeoutfile(pr);

            // Try to open binary output file
            pr->outfile.OutFile = fopen(pr->outfile.OutFname, "w+b");
            if (pr->outfile.OutFile == NULL) return 304;

            // Save basic network data & energy usage results
            ERRCODE(savenetdata(pr));
            pr->outfile.OutOffset1 = ftell(pr->outfile.OutFile);
            ERRCODE(saveenergy(pr));
            pr->outfile.OutOffset2 = ftell(pr->outfile.OutFile);

            // Open temporary file if computing time series statistic
            if (!errcode)
            {
                if (pr->report.Tstatflag != SERIES)
                {
                    pr->outfile.TmpOutFile = fopen(pr->TmpStatFname, "w+b");
                    if (pr->outfile.TmpOutFile == NULL) errcode = 304;
                }
                else pr->outfile.TmpOutFile = pr->outfile.OutFile;
            }
            return errcode;
        };
        static Pdemand	finddemand(Pdemand d, int index)
            /*----------------------------------------------------------------
            **  Input:   d = pointer to start of a list of demands
            **           index = the position of the demand to retrieve
            **  Output:  none
            **  Returns: the demand at the requested position
            **  Purpose: finds the demand at a given position in a demand list
            **----------------------------------------------------------------
            */
        {
            int n = 1;
            if (index <= 0)return NULL;
            while (d)
            {
                if (n == index) break;
                n++;
                d = d->next;
            }
            return d;
        };
        static int		incontrols(EN_Project pr, int objType, int index)
            /*----------------------------------------------------------------
            **  Input:   objType = type of object (either NODE or LINK)
            **           index  = the object's index
            **  Output:  none
            **  Returns: 1 if any controls contain the object; 0 if not
            **  Purpose: determines if any simple or rule-based controls
            **           contain a particular node or link.
            **----------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;

            int i, ruleObject;
            Spremise* premise;
            Saction* action;

            // Check simple controls
            for (i = 1; i <= net->Ncontrols; i++)
            {
                if (objType == NODE && net->Control[i].Node == index) return 1;
                if (objType == LINK && net->Control[i].Link == index) return 1;
            }

            // Check rule-based controls
            for (i = 1; i <= net->Nrules; i++)
            {
                // Convert objType to a rule object type
                if (objType == NODE) ruleObject = 6;
                else                 ruleObject = 7;

                // Check rule's premises
                premise = net->Rule[i].Premises;
                while (premise != NULL)
                {
                    if (ruleObject == premise->object && premise->index == index) return 1;
                    premise = premise->next;
                }

                // Rule actions only need to be checked for link objects
                if (objType == LINK)
                {
                    // Check rule's THEN actions
                    action = net->Rule[i].ThenActions;
                    while (action != NULL)
                    {
                        if (action->link == index) return 1;
                        action = action->next;
                    }

                    // Check rule's ELSE actions
                    action = net->Rule[i].ElseActions;
                    while (action != NULL)
                    {
                        if (action->link == index) return 1;
                        action = action->next;
                    }
                }
            }
            return 0;
        };
        static void		adjustpatterns(EN_Network network, int index)
            /*----------------------------------------------------------------
            **  Input:   index = index of time pattern being deleted
            **  Output:  none
            **  Purpose: modifies references made to a deleted time pattern
            **----------------------------------------------------------------
            */
        {
            int j;
            Pdemand demand;
            Psource source;

            // Adjust patterns used by junctions
            for (j = 1; j <= network->Nnodes; j++)
            {
                // Adjust demand patterns
                for (demand = network->Node[j]->D; demand != NULL; demand = demand->next)
                {
                    adjustpattern(&demand->Pat, index);
                }
                // Adjust WQ source patterns
                source = network->Node[j]->S;
                if (source) adjustpattern(&source->Pat, index);
            }

            // Adjust patterns used by reservoir tanks
            for (j = 1; j <= network->Ntanks; j++)
            {
                adjustpattern(&network->Tank[j]->Pat, index);
            }

            // Adjust patterns used by pumps
            for (j = 1; j <= network->Npumps; j++)
            {
                adjustpattern(&network->Pump[j]->Upat, index);
                adjustpattern(&network->Pump[j]->Epat, index);
            }
        };
        static void		adjustcurves(EN_Network network, int index)
            /*----------------------------------------------------------------
            **  Input:   index = index of data curve being deleted
            **  Output:  none
            **  Purpose: modifies references made to a deleted data curve
            **----------------------------------------------------------------
            */
        {
            int j, k, setting;

            // Adjust tank volume curves
            for (j = 1; j <= network->Ntanks; j++)
            {
                adjustcurve(&network->Tank[j]->Vcurve, index);
            }

            // Adjust pump curves
            for (j = 1; j <= network->Npumps; j++)
            {
                adjustcurve(&network->Pump[j]->Hcurve, index);
                adjustcurve(&network->Pump[j]->Ecurve, index);
            }

            // Adjust GPV curves
            for (j = 1; j <= network->Nvalves; j++)
            {
                k = network->Valve[j]->Link;
                if (network->Link[k]->Type == GPV)
                {
                    setting = INT(network->Link[k]->Kc);
                    adjustcurve(&setting, index);
                    network->Link[k]->Kc = setting;
                }
            }
        };
        static int		getcomment(EN_Network network, int object, int index, char* comment)
            //----------------------------------------------------------------
            //  Input:   object = a type of network object
            //           index = index of the specified object
            //           comment = the object's comment string
            //  Output:  error code
            //  Purpose: gets the comment string assigned to an object.
            //----------------------------------------------------------------
        {
            char* currentcomment;

            // Get pointer to specified object's comment
            switch (object)
            {
            case NODE:
                if (index < 1 || index > network->Nnodes) return 251;
                currentcomment = network->Node[index]->Comment;
                break;
            case LINK:
                if (index < 1 || index > network->Nlinks) return 251;
                currentcomment = network->Link[index]->Comment;
                break;
            case TIMEPAT:
                if (index < 1 || index > network->Npats) return 251;
                currentcomment = network->Pattern[index].Comment;
                break;
            case CURVE:
                if (index < 1 || index > network->Ncurves) return 251;
                currentcomment = network->Curve[index].Comment;
                break;
            default:
                strcpy(comment, "");
                return 251;
            }

            // Copy the object's comment to the returned string
            if (currentcomment) strcpy(comment, currentcomment);
            else comment[0] = '\0';
            return 0;
        };
        static int		setcomment(EN_Network network, int object, int index, const char* newcomment)
            //----------------------------------------------------------------
            //  Input:   object = a type of network object
            //           index = index of the specified object
            //           newcomment = new comment string
            //  Output:  error code
            //  Purpose: sets the comment string of an object.
            //----------------------------------------------------------------
        {
            char* comment;

            switch (object)
            {
            case NODE:
                if (index < 1 || index > network->Nnodes) return 251;
                comment = network->Node[index]->Comment;
                network->Node[index]->Comment = xstrcpy(&comment, newcomment, MAXMSG);
                return 0;

            case LINK:
                if (index < 1 || index > network->Nlinks) return 251;
                comment = network->Link[index]->Comment;
                network->Link[index]->Comment = xstrcpy(&comment, newcomment, MAXMSG);
                return 0;

            case TIMEPAT:
                if (index < 1 || index > network->Npats) return 251;
                comment = network->Pattern[index].Comment;
                network->Pattern[index].Comment = xstrcpy(&comment, newcomment, MAXMSG);
                return 0;

            case CURVE:
                if (index < 1 || index > network->Ncurves) return 251;
                comment = network->Curve[index].Comment;
                network->Curve[index].Comment = xstrcpy(&comment, newcomment, MAXMSG);
                return 0;

            default: return 251;
            }
        };
        static int		adjustpumpparams(EN_Project pr, int curveIndex)
            /*----------------------------------------------------------------
            **  Input:   curveIndex = index of a data curve
            **  Output:  returns an error code
            **  Purpose: updates head curve parameters for pumps using a
            **           curve whose data have been modified.
            **----------------------------------------------------------------
            */
        {
            EN_Network network = pr->network;

            SCALER* Ucf = pr->Ucf;
            int j, err = 0;
            Ppump pump;

            // Check each pump
            for (j = 1; j <= network->Npumps; j++)
            {
                // Pump uses curve as head curve
                pump = network->Pump[j];
                if (curveIndex == pump->Hcurve)
                {
                    // Update its head curve parameters
                    pump->Ptype = NOCURVE;
                    err = epanet_shared::updatepumpparams(pr, curveIndex);
                    if (err > 0) break;

                    // Convert parameters to internal units
                    if (pump->Ptype == POWER_FUNC)
                    {
                        pump->H0 /= Ucf[HEAD];
                        pump->R *= (::epanet::pow(Ucf[FLOW], pump->N) / Ucf[HEAD]);
                    }
                    pump->Q0 /= Ucf[FLOW];
                    pump->Qmax /= Ucf[FLOW];
                    pump->Hmax /= Ucf[HEAD];
                }
            }
            return err;
        };
        static void     addrule(Parser* parser, char* tok)
            //--------------------------------------------------------------
            //    Updates rule count if RULE keyword found in line of input.
            //--------------------------------------------------------------
        {
            if (match(tok, w_RULE)) parser->MaxRules++;
        };
        static int      addpattern(EN_Network network, char* id)
            /*
            **-------------------------------------------------------------
            **  Input:   id = pattern ID label
            **  Output:  returns error code
            **  Purpose: adds a new pattern to the database
            **--------------------------------------------------------------
            */
        {
            int n = network->Npats;
            Spattern* pattern;

            // Check if pattern was already created
            if (n > 0)
            {
                if (strcmp(id, network->Pattern[n].ID) == 0) return 0;
                if (epanet_shared::findpattern(network, id) > 0) return 0;
            }
            if (strlen(id) > MAXID) return 252;

            // Update pattern count & add a new pattern to the database
            n = n + 2;
            network->Pattern.AssureSize(n);
            if (network->Pattern == NULL) return 101;
            (network->Npats)++;

            // Initialize the pattern
            pattern = &network->Pattern[network->Npats];
            strncpy(pattern->ID, id, MAXID);
            pattern->Comment = NULL;
            pattern->Pat.Clear();
            return 0;
        };
        static int      addcurve(EN_Network network, char* id)
            /*
            **-------------------------------------------------------------
            **  Input:   id = curve ID label
            **  Output:  returns error code
            **  Purpose: adds a new curve to the database
            **--------------------------------------------------------------
            */
        {
            int n = network->Ncurves;
            Scurve* curve;

            // Check if was already created
            if (n > 0)
            {
                if (strcmp(id, network->Curve[n].ID) == 0) return 0;
                if (epanet_shared::findcurve(network, id) > 0) return 0;
            }
            if (strlen(id) > MAXID) return 252;

            n = n + 2;
            network->Curve.AssureSize(n);
            (network->Ncurves)++;

            // Initialize the curve
            curve = &network->Curve[network->Ncurves];
            strncpy(curve->ID, id, MAXID);
            curve->Type = GENERIC_CURVE;
            curve->Comment = NULL;
            curve->Capacity = 0;
            curve->Npts = 0;
            curve->X = NULL;
            curve->Y = NULL;
            return 0;
        };
        static int      netsize(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: determines number of network objects
            **--------------------------------------------------------------
            */
        {
            Parser* parser = &pr->parser;

            char line[MAXLINE + 1]; // Line from input data file
            char* tok;              // First token of line
            int sect, newsect;      // Input data sections
            int errcode = 0;        // Error code
            Spattern* pattern;

            // Initialize object counts
            parser->MaxJuncs = 0;
            parser->MaxTanks = 0;
            parser->MaxPipes = 0;
            parser->MaxPumps = 0;
            parser->MaxValves = 0;
            parser->MaxControls = 0;
            parser->MaxRules = 0;
            parser->MaxCurves = 0;
            sect = -1;


            // Add a "dummy" time pattern with index of 0 and a single multiplier
            // of 1.0 to be used by all demands not assigned a pattern
            pr->network->Npats = -1;
            errcode = addpattern(pr->network, (char*)"");
            if (errcode) return errcode;
            pattern = &pr->network->Pattern[0];
            pattern[0].Pat.AddUniqueValue((u64)pr->times.GetPatternStartTime(), 1.0); 
            parser->MaxPats = pr->network->Npats;

            // Make a pass through input file counting number of each object
            if (parser->InFile == NULL) return 0;
            while (fgets(line, MAXLINE, parser->InFile) != NULL)
            {
                // Skip blank lines & those beginning with a comment
                tok = strtok(line, SEPSTR.c_str());
                if (tok == NULL) continue;
                if (*tok == ';') continue;

                // Check if line begins with a new section heading
                if (tok[0] == '[')
                {
                    newsect = findmatch(tok, SectTxt);
                    if (newsect >= 0)
                    {
                        sect = newsect;
                        if (sect == _END) break;
                        continue;
                    }
                    else continue;
                }

                // Add to count of current object
                switch (sect)
                {
                case _JUNCTIONS: parser->MaxJuncs++;    break;
                case _RESERVOIRS:
                case _TANKS:     parser->MaxTanks++;    break;
                case _PIPES:     parser->MaxPipes++;    break;
                case _PUMPS:     parser->MaxPumps++;    break;
                case _VALVES:    parser->MaxValves++;   break;
                case _CONTROLS:  parser->MaxControls++; break;
                case _RULES:     addrule(parser, tok);   break;
                case _PATTERNS:
                    errcode = addpattern(pr->network, tok);
                    parser->MaxPats = pr->network->Npats;
                    break;
                case _CURVES:
                    errcode = addcurve(pr->network, tok);
                    parser->MaxCurves = pr->network->Ncurves;
                    break;
                }
                if (errcode) break;
            }

            parser->MaxNodes = parser->MaxJuncs + parser->MaxTanks;
            parser->MaxLinks = parser->MaxPipes + parser->MaxPumps + parser->MaxValves;
            if (parser->MaxPats < 1) parser->MaxPats = 1;
            return errcode;
        };
        static void inperrmsg(EN_Project pr, int err, int sect, char* line)
            /*
            **-------------------------------------------------------------
            **  Input:   err   = error code
            **           sect  = input data section
            **           *line = line from input file
            **  Output:  none
            **  Purpose: displays input reader error message
            **-------------------------------------------------------------
            */
        {
            Parser* parser = &pr->parser;

            char errStr[MAXMSG + 1] = "";
            char tok[MAXMSG + 1];

            // Get token associated with input error
            if (parser->ErrTok >= 0) strcpy(tok, parser->Tok[parser->ErrTok]);
            else strcpy(tok, "");

            // write error message to report file
            sprintf(pr->Msg, "Error %d: %s %s in %s section:",
                err, geterrmsg(err, errStr), tok, SectTxt[sect]);
            epanet_shared::writeline(pr, pr->Msg);

            // Echo input line
            epanet_shared::writeline(pr, line);
        };
        static int readdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: reads contents of input data file
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            char line[MAXLINE + 1],  // Line from input data file
                wline[MAXLINE + 1]; // Working copy of input line
            char errmsg[MAXMSG + 1] = "";
            int  sect, newsect,      // Data sections
                errcode = 0,        // Error code
                inperr, errsum;     // Error code & total error count

            // Allocate input buffer
            parser->X = (SCALER*)calloc(MAXTOKS, sizeof(SCALER));
            ERRCODE(MEMCHECK(parser->X));
            if (errcode) return errcode;

            // Initialize actual number of network components
            parser->Ntitle = 0;
            net->Nnodes = 0;
            net->Njuncs = 0;
            net->Ntanks = 0;
            net->Nlinks = 0;
            net->Npipes = 0;
            net->Npumps = 0;
            net->Nvalves = 0;
            net->Ncontrols = 0;
            net->Nrules = 0;

            // Patterns & Curves were created previously in netsize()
            parser->MaxPats = net->Npats;
            parser->MaxCurves = net->Ncurves;
            parser->PrevPat = NULL;
            parser->PrevCurve = NULL;

            // Initialize full line comment, input data section and error count
            parser->LineComment[0] = '\0';
            sect = -1;
            errsum = 0;

            // Read each line from input file
            while (fgets(line, MAXLINE, parser->InFile) != NULL)
            {
                // Make copy of line and scan for tokens
                ::strcpy(wline, line);
                parser->Ntokens = gettokens(wline, parser->Tok, MAXTOKS, parser->Comment);

                // Skip blank lines and those filled with a comment
                parser->ErrTok = -1;
                if (parser->Ntokens == 0)
                {
                    // Store full line comment for Patterns and Curves
                    if (sect == _PATTERNS || sect == _CURVES)
                    {
                        strncpy(parser->LineComment, parser->Comment, MAXMSG);
                    }
                    continue;
                }

                // Apply full line comment for Patterns and Curves
                if (sect == _PATTERNS || sect == _CURVES)
                {
                    ::strcpy(parser->Comment, parser->LineComment);
                }
                parser->LineComment[0] = '\0';

                // Check if max. line length exceeded
                if (strlen(line) >= MAXLINE)
                {
                    sprintf(pr->Msg, "%s section: %s", geterrmsg(214, errmsg), SectTxt[sect]);
                    epanet_shared::writeline(pr, pr->Msg);
                    epanet_shared::writeline(pr, line);
                    errsum++;
                }

                // Check if at start of a new input section
                if (parser->Tok[0][0] == '[')
                {
                    newsect = findmatch(parser->Tok[0], SectTxt);
                    if (newsect >= 0)
                    {
                        sect = newsect;
                        if (sect == _END) break;
                        continue;
                    }
                    else
                    {
                        inperrmsg(pr, 201, sect, line);
                        errsum++;
                        break;
                    }
                }

                // Otherwise process next line of input in current section
                else
                {
                    if (sect >= 0)
                    {
                        inperr = newline(pr, sect, line);
                        if (inperr > 0)
                        {
                            inperrmsg(pr, inperr, sect, line);
                            errsum++;
                        }
                    }
                    else
                    {
                        errcode = 200;
                        break;
                    }
                }

                // Stop if reach end of file or max. error count
                if (errsum == MAXERRS)  break;
            }

            // Check for errors
            if (errsum > 0) errcode = 200;

            // Determine pump curve parameters
            if (!errcode) errcode = getpumpparams(pr);

            // Free input buffer
            free(parser->X);
            return errcode;
        };
        static int readdata(EN_Project pr, SectionType targetedSection)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: reads contents of input data file
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            char line[MAXLINE + 1],  // Line from input data file
                wline[MAXLINE + 1]; // Working copy of input line
            char errmsg[MAXMSG + 1] = "";
            int  sect, newsect,      // Data sections
                errcode = 0,        // Error code
                inperr, errsum;     // Error code & total error count

            // Patterns & Curves were created previously in netsize()
            parser->MaxPats = net->Npats;
            parser->MaxCurves = net->Ncurves;
            parser->PrevPat = NULL;
            parser->PrevCurve = NULL;

            // Initialize full line comment, input data section and error count
            parser->LineComment[0] = '\0';
            sect = -1;
            errsum = 0;

            // Read each line from input file
            while (fgets(line, MAXLINE, parser->InFile) != NULL)
            {
                // Make copy of line and scan for tokens
                ::strcpy(wline, line);
                parser->Ntokens = gettokens(wline, parser->Tok, MAXTOKS, parser->Comment);

                // Skip blank lines and those filled with a comment
                parser->ErrTok = -1;
                if (parser->Ntokens == 0)
                {
                    // Store full line comment for Patterns and Curves
                    if (sect == _PATTERNS || sect == _CURVES)
                    {
                        strncpy(parser->LineComment, parser->Comment, MAXMSG);
                    }
                    continue;
                }

                // Apply full line comment for Patterns and Curves
                if (sect == _PATTERNS || sect == _CURVES)
                {
                    ::strcpy(parser->Comment, parser->LineComment);
                }
                parser->LineComment[0] = '\0';

                // Check if max. line length exceeded
                if (strlen(line) >= MAXLINE)
                {
                    sprintf(pr->Msg, "%s section: %s", geterrmsg(214, errmsg), SectTxt[sect]);
                    epanet_shared::writeline(pr, pr->Msg);
                    epanet_shared::writeline(pr, line);
                    errsum++;
                }

                // Check if at start of a new input section
                if (parser->Tok[0][0] == '[')
                {
                    newsect = findmatch(parser->Tok[0], SectTxt);
                    if (newsect >= 0)
                    {
                        sect = newsect;
                        if (sect == _END) break;
                        continue;
                    }
                    else
                    {
                        inperrmsg(pr, 201, sect, line);
                        errsum++;
                        break;
                    }
                }

                // Otherwise process next line of input in current section
                else
                {
                    if (sect >= 0)
                    {
                        if (targetedSection == sect) {
                            inperr = newline(pr, sect, line);
                            if (inperr > 0)
                            {
                                inperrmsg(pr, inperr, sect, line);
                                errsum++;
                            }
                        }
                    }
                    else
                    {
                        errcode = 200;
                        break;
                    }
                }

                // Stop if reach end of file or max. error count
                if (errsum == MAXERRS)  break;
            }

            // Check for errors
            if (errsum > 0) errcode = 200;

            return errcode;
        };
        static int getdata(EN_Project pr)
            /*
            **----------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: reads in network data from disk file
            **----------------------------------------------------------------
            */
        {
            int errcode = 0;

            // Assign default data values & reporting options
            epanet_shared::setdefaults(pr);
            epanet_shared::initreport(&pr->report);

            // Read in network data
#if 0
            rewind(pr->parser.InFile);
            ERRCODE(readdata(pr));
#else
            cweeList< SectionType> SectionOrder;
            SectionOrder = {
                _OPTIONS, _TIMES, _TITLE, _JUNCTIONS, _RESERVOIRS, _TANKS, _PIPES, _PUMPS,
                _VALVES, _PATTERNS, _CURVES, _DEMANDS,_COORDS, _VERTICES, _ENERGY, _MIXING,
                _STATUS, _ROUGHNESS, _LABELS, _TAGS, _BACKDROP, _EMITTERS, _CONTROLS, _RULES,                
                _SOURCES, _QUALITY, _REACTIONS, _REPORT, _END
            };

            // Allocate input buffer
            pr->parser.X = (SCALER*)calloc(MAXTOKS, sizeof(SCALER));
            ERRCODE(MEMCHECK(pr->parser.X));
            if (errcode) return errcode;

            // Initialize actual number of network components
            pr->parser.Ntitle = 0;
            pr->network->Nnodes = 0;
            pr->network->Njuncs = 0;
            pr->network->Ntanks = 0;
            pr->network->Nlinks = 0;
            pr->network->Npipes = 0;
            pr->network->Npumps = 0;
            pr->network->Nvalves = 0;
            pr->network->Ncontrols = 0;
            pr->network->Nrules = 0;

            for (auto& section : SectionOrder) {
                rewind(pr->parser.InFile);
                ERRCODE(readdata(pr, section));
            }            
            if (!errcode) errcode = getpumpparams(pr); // Determine pump curve parameters            
            free(pr->parser.X); // Free input buffer
#endif


            // Adjust data and convert it to internal units
            if (!errcode)  epanet_shared::adjustdata(pr);
            if (!errcode)  epanet_shared::initunits(pr);
            ERRCODE(epanet_shared::inittanks(pr));
            if (!errcode)  epanet_shared::convertunits(pr);
            return errcode;
        };
        static int setError(Parser* parser, int tokindex, int errcode)
            /*
            **--------------------------------------------------------------
            **  Input:   tokindex = index of input line token
            **           errcode = an error code
            **  Output:  returns error code
            **  Purpose: records index of token from line of input associated
            **           with an error
            **--------------------------------------------------------------
            */
        {
            parser->ErrTok = tokindex;
            return errcode;
        };
        static int juncdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes junction data
            **  Format:
            **    [JUNCTIONS]
            **      id  elev.  (demand)  (demand pattern)
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;
            Hydraul* hyd = &pr->hydraul;

            int p = 0;                  // time pattern index
            int n;                      // number of tokens
            int njuncs;                 // number of network junction nodes
            SCALER el,                  // elevation
                y = 0.0;             // base demand
            Pnode node;
            int err = 0;

            // Add new junction to data base
            n = parser->Ntokens;
            if (net->Nnodes == parser->MaxNodes) return 200;
            net->Njuncs++;
            net->Nnodes++;
            njuncs = net->Njuncs;

            // Check for valid data
            if (n < 2) return 201;
            if (!getfloat(parser->Tok[1], &el)) return setError(parser, 1, 202);
            if (n >= 3 && !getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);
            if (n >= 4)
            {
                p = epanet_shared::findpattern(net, parser->Tok[3]);
                if (p < 0) return setError(parser, 3, 205);
            }

            // Save junction data
            net->Node[njuncs] = make_cwee_shared<Snode>(asset_t::JUNCTION, cweeAssetValueCollection<asset_t::JUNCTION>().Values, parser->Tok[0]);
            net->Node[njuncs]->Name_p = parser->Tok[0];

            err = addnodeID(net, net->Njuncs, parser->Tok[0]);
            if (err) return setError(parser, 0, err);

            net->Asset.AddOrReplace(net->Node[njuncs].CastReference<Sasset>());


            node = net->Node[njuncs];
            node->X = MISSING;
            node->Y = MISSING;
            node->El = pr->convertToUnit<foot_t>((double)el);
            node->C0 = 0.0;
            node->S = nullptr;
            node->Ke = 0.0;
            node->Rpt = 0;
            node->ResultIndex = 0;
            node->Type = JUNCTION;
            node->Comment = xstrcpy(&node->Comment, parser->Comment, MAXMSG);

            // Create a demand for the junction and use NodeDemand as an indicator
            // to be used when processing demands from the [DEMANDS] section
            if (!epanet_shared::adddemand(node, y, p, NULL)) return 101;
            hyd->NodeDemand[njuncs] = pr->convertToUnit<cubic_foot_per_second_t>(y); // Right? Wrong? 
            return 0;
        };
        static int tankdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes tank & reservoir data
            **  Format:
            **   [RESERVOIRS]
            **     id elev (pattern)
            **   [TANKS]
            **     id elev (pattern)
            **     id elev initlevel minlevel maxlevel diam (minvol vcurve)
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int    i,               // Node index
                n,               // # data items
                pattern = 0,     // Time pattern index
                curve = 0,       // Curve index
                overflow = FALSE;// Overflow indicator

            SCALER el = 0.0,        // Elevation
                initlevel = 0.0, // Initial level
                minlevel = 0.0,  // Minimum level
                maxlevel = 0.0,  // Maximum level
                minvol = 0.0,    // Minimum volume
                diam = 0.0;      // Diameter
            Pnode node;
            Ptank tank;

            int err = 0;

            // Add new tank to data base
            n = parser->Ntokens;
            if (net->Ntanks == parser->MaxTanks ||
                net->Nnodes == parser->MaxNodes) return 200;
            net->Ntanks++;
            net->Nnodes++;

            i = parser->MaxJuncs + net->Ntanks;

            // Check for valid data
            if (n < 2) return 201;
            if (!getfloat(parser->Tok[1], &el)) return setError(parser, 1, 202);

            // Tank is reservoir
            if (n <= 3)
            {
                // Head pattern supplied
                if (n == 3)
                {
                    pattern = epanet_shared::findpattern(net, parser->Tok[2]);
                    if (pattern < 0) return setError(parser, 2, 205);
                }
            }
            else if (n < 6) return 201;

            // Tank is a storage tank
            else
            {
                if (!getfloat(parser->Tok[2], &initlevel)) return setError(parser, 2, 202);
                if (!getfloat(parser->Tok[3], &minlevel))  return setError(parser, 3, 202);
                if (!getfloat(parser->Tok[4], &maxlevel))  return setError(parser, 4, 202);
                if (!getfloat(parser->Tok[5], &diam))      return setError(parser, 5, 202);
                if (n >= 7 && !getfloat(parser->Tok[6], &minvol)) return setError(parser, 6, 202);

                // If volume curve supplied check it exists
                if (n >= 8)
                {
                    if (strlen(parser->Tok[7]) > 0 && *(parser->Tok[7]) != '*')
                    {
                        curve = epanet_shared::findcurve(net, parser->Tok[7]);
                        if (curve == 0) return setError(parser, 7, 206);
                        net->Curve[curve].Type = VOLUME_CURVE;
                    }
                }

                // Parse overflow indicator if present
                if (n >= 9)
                {
                    if (match(parser->Tok[8], w_YES)) overflow = TRUE;
                    else if (match(parser->Tok[8], w_NO)) overflow = FALSE;
                    else return setError(parser, 8, 213);
                }

                if (initlevel < 0.0) return setError(parser, 2, 209);
                if (minlevel < 0.0) return setError(parser, 3, 209);
                if (maxlevel < 0.0) return setError(parser, 4, 209);
                if (diam < 0.0) return setError(parser, 5, 209);
                if (minvol < 0.0) return setError(parser, 6, 209);
            }

            tank = net->Tank[net->Ntanks];
            tank->Name_p = parser->Tok[0];
            net->Node[i] = tank.CastReference<Snode>();

            err = addnodeID(net, i, parser->Tok[0]);
            if (err) return setError(parser, 0, err);

            net->Asset.AddOrReplace(tank.CastReference<Sasset>());

            node = net->Node[i];
            
            node->X = MISSING;
            node->Y = MISSING;
            node->Rpt = 0;
            node->ResultIndex = 0;
            node->El = pr->convertToUnit<foot_t>((double)el);
            node->C0 = 0.0;
            node->S = nullptr;
            node->Ke = 0.0;
            node->Type = (diam == (SCALER)0) ? RESERVOIR : TANK;
            node->Comment = xstrcpy(&node->Comment, parser->Comment, MAXMSG);
            tank->Node = i;
            tank->H0 = pr->convertToUnit<foot_t>((double)initlevel); // fixed during units conversion
            tank->Hmin = pr->convertToUnit<foot_t>((double)minlevel); // fixed during units conversion
            tank->Hmax = pr->convertToUnit<foot_t>((double)maxlevel); // fixed during units conversion
            tank->Diameter = pr->convertToUnit<foot_t>((double)diam);
            tank->A = PI * SQR(tank->Diameter) / 4.0; // fixed during units conversion
            tank->Pat = pattern;
            tank->Kb = MISSING;
            tank->CanOverflow = overflow;

            //*******************************************************************
            // NOTE: The min, max, & initial volumes set here are based on a
            //    nominal tank diameter. They will be modified in INPUT1.C if
            //    a volume curve is supplied for this tank.
            //*******************************************************************
            tank->Vmin = tank->A * pr->convertToUnit<foot_t>((double)minlevel);
            if (minvol > 0.0) tank->Vmin = pr->convertToUnit<cubic_foot_t>((double)minvol);
            tank->V0 = tank->Vmin + tank->A * (pr->convertToUnit<foot_t>((double)initlevel) - pr->convertToUnit<foot_t>((double)minlevel));
            tank->Vmax = tank->Vmin + tank->A * (pr->convertToUnit<foot_t>((double)maxlevel) - pr->convertToUnit<foot_t>((double)minlevel));

            tank->Vcurve = curve;
            tank->MixModel = MIX1; // Completely mixed
            tank->V1frac = 1.0;    // Mixing compartment size fraction
            return 0;
        };
        static int pipedata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes pipe data
            **  Format:
            **    [PIPE]
            **    id  node1  node2  length  diam  rcoeff (lcoeff) (status)
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int      j1,               // Start-node index
                j2,               // End-node index
                n;                // # data items
            SCALER   length,           // Pipe length
                diam,             // Pipe diameter
                rcoeff,           // Roughness coeff.
                lcoeff = 0.0;     // Minor loss coeff
            LinkType type = PIPE;      // Link type
            StatusType status = OPEN;  // Link status
            Plink link;
            int err = 0;

            // Add new pipe to data base
            n = parser->Ntokens;
            if (net->Nlinks == parser->MaxLinks) return 200;
            net->Npipes++;
            net->Nlinks++;

            // Check for valid data
            if (n < 6) return 201;
            if ((j1 = epanet_shared::findnode(net, parser->Tok[1])) == 0) return setError(parser, 1, 203);
            if ((j2 = epanet_shared::findnode(net, parser->Tok[2])) == 0) return setError(parser, 2, 203);
            if (j1 == j2) return setError(parser, 0, 222);

            if (!getfloat(parser->Tok[3], &length)) return setError(parser, 3, 202);
            if (length <= 0.0) return setError(parser, 3, 211);
            if (!getfloat(parser->Tok[4], &diam)) return  setError(parser, 4, 202);
            if (diam <= 0.0) return setError(parser, 4, 211);
            if (!getfloat(parser->Tok[5], &rcoeff)) return setError(parser, 5, 202);
            if (rcoeff <= 0.0) setError(parser, 5, 211);

            // Either a loss coeff. or a status is supplied
            if (n == 7)
            {
                if (match(parser->Tok[6], w_CV)) type = CVPIPE;
                else if (match(parser->Tok[6], w_CLOSED)) status = CLOSED;
                else if (match(parser->Tok[6], w_OPEN))   status = OPEN;
                else if (!getfloat(parser->Tok[6], &lcoeff)) return setError(parser, 6, 202);
            }

            // Both a loss coeff. and a status is supplied
            if (n == 8)
            {
                if (!getfloat(parser->Tok[6], &lcoeff)) return setError(parser, 6, 202);
                if (match(parser->Tok[7], w_CV))  type = CVPIPE;
                else if (match(parser->Tok[7], w_CLOSED)) status = CLOSED;
                else if (match(parser->Tok[7], w_OPEN))   status = OPEN;
                else return setError(parser, 7, 213);
            }
            if (lcoeff < 0.0) return setError(parser, 6, 211);

            // Save pipe data

            net->Link[net->Nlinks] = make_cwee_shared<Slink>(asset_t::PIPE, cweeAssetValueCollection<asset_t::PIPE>().Values, parser->Tok[0]);
            net->Link[net->Nlinks]->Name_p = parser->Tok[0];
            err = addlinkID(net, net->Nlinks, parser->Tok[0]);
            if (err) return setError(parser, 0, err);
            net->Asset.AddOrReplace(net->Link[net->Nlinks].CastReference<Sasset>());

            link = net->Link[net->Nlinks];
            link->N1 = j1;
            link->N2 = j2;
            link->Len = pr->convertToUnit<foot_t>(length);
            link->Diam = pr->convertToUnit<foot_t>(diam, true);
            link->Kc = rcoeff;
            link->Km = lcoeff;
            link->Kb = MISSING;
            link->Kw = MISSING;
            link->Type = type;
            link->Status = status;
            link->Rpt = 0;
            link->ResultIndex = 0;
            link->Comment = xstrcpy(&link->Comment, parser->Comment, MAXMSG);
            return 0;
        };
        static int getpumpcurve(EN_Project pr, int n)
            /*
            **--------------------------------------------------------
            **  Input:   n = number of parameters for pump curve
            **  Output:  returns error code
            **  Purpose: processes pump curve data for Version 1.1-
            **           style input data
            **  Notes:
            **    1. Called by pumpdata() in INPUT3.C
            **    2. Current link index & pump index of pump being
            **       processed is found in network variables Nlinks
            **       and Npumps, respectively
            **    3. Curve data read from input line is found in
            **       parser's array X[0],...X[n-1]
            **---------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            SCALER a, b, c, h0, h1, h2, q1, q2;
            Ppump pump = net->Pump[net->Npumps];

            // Constant HP curve
            if (n == 1)
            {
                if (parser->X[0] <= 0.0) return 202;
                pump->Ptype = CONST_HP;
                net->Link[net->Nlinks]->Km = parser->X[0];
            }

            // Power function curve
            else
            {
                // Single point power curve
                if (n == 2)
                {
                    q1 = parser->X[1];
                    h1 = parser->X[0];
                    h0 = 1.33334 * h1;
                    q2 = 2.0 * q1;
                    h2 = 0.0;
                }

                // 3-point power curve
                else if (n >= 5)
                {
                    h0 = parser->X[0];
                    h1 = parser->X[1];
                    q1 = parser->X[2];
                    h2 = parser->X[3];
                    q2 = parser->X[4];
                }
                else return 202;
                pump->Ptype = POWER_FUNC;
                if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c)) return 206;
                pump->H0 = -a;
                pump->R = -b;
                pump->N = c;
                pump->Q0 = q1;
                pump->Qmax = ::epanet::pow((-a / b), (1.0 / c));
                pump->Hmax = h0;
            }
            return 0;
        };
        static int pumpdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code
            ** Purpose: processes pump data
            ** Formats:
            **  [PUMP]
            **   (Version 1.x Format):
            **   id  node1  node2  power
            **   id  node1  node2  h1    q1
            **   id  node1  node2  h0    h1   q1   h2   q2
            **   (Version 2 Format):
            **   id  node1  node2  KEYWORD value {KEYWORD value ...}
            **   where KEYWORD = [POWER,HEAD,PATTERN,SPEED]
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int    j, m,  // Token array indexes
                j1,    // Start-node index
                j2,    // End-node index
                n,     // # data items
                c, p;  // Curve & Pattern indexes
            SCALER y;
            Plink link;
            Ppump pump;
            int err = 0;

            /* Add new pump to data base */
            n = parser->Ntokens;
            if (net->Nlinks == parser->MaxLinks ||
                net->Npumps == parser->MaxPumps) return 200;
            net->Nlinks++;
            net->Npumps++;

            // Check for valid data
            if (n < 3) return 201;
            if ((j1 = epanet_shared::findnode(net, parser->Tok[1])) == 0) return setError(parser, 1, 203);
            if ((j2 = epanet_shared::findnode(net, parser->Tok[2])) == 0) return setError(parser, 2, 203);
            if (j1 == j2) return setError(parser, 0, 222);

            // Save pump data
            pump = net->Pump[net->Npumps];

            net->Link[net->Nlinks] = pump.CastReference<Slink>();
            net->Link[net->Nlinks]->Name_p = parser->Tok[0];
            err = addlinkID(net, net->Nlinks, parser->Tok[0]);
            if (err) return setError(parser, 0, err);
            net->Asset.AddOrReplace(pump.CastReference<Sasset>());

            link = net->Link[net->Nlinks];            

            link->N1 = j1;
            link->N2 = j2;
            link->Diam = 0_in;
            link->Len = 0.0_ft;
            link->Kc = 1.0;
            link->Km = 0.0;
            link->Kb = 0.0;
            link->Kw = 0.0;
            link->Type = PUMP;
            link->Status = OPEN;
            link->Rpt = 0;
            link->ResultIndex = 0;
            link->Comment = xstrcpy(&link->Comment, parser->Comment, MAXMSG);
            pump->Link = net->Nlinks;
            pump->Ptype = NOCURVE; // NOCURVE is a placeholder
            pump->Hcurve = 0;
            pump->Ecurve = 0;
            pump->Upat = 0;
            pump->Ecost = 0.0;
            pump->Epat = 0;
            if (n < 4) return 0;

            // If 4-th token is a number then input follows Version 1.x format
            // so retrieve pump curve parameters
            if (getfloat(parser->Tok[3], &parser->X[0]))
            {
                m = 1;
                for (j = 4; j < n; j++)
                {
                    if (!getfloat(parser->Tok[j], &parser->X[m])) return setError(parser, j, 202);
                    m++;
                }
                return (getpumpcurve(pr, m));
            }

            // Otherwise input follows Version 2 format
            // so retrieve keyword/value pairs
            m = 4;
            while (m < n)
            {
                if (match(parser->Tok[m - 1], w_POWER)) // Const. HP curve
                {
                    y = atof(parser->Tok[m]);
                    if (y <= 0.0) return setError(parser, m, 202);
                    pump->Ptype = CONST_HP;
                    link->Km = y;
                }
                else if (match(parser->Tok[m - 1], w_HEAD))  // Custom pump curve
                {
                    c = epanet_shared::findcurve(net, parser->Tok[m]);
                    if (c == 0) return setError(parser, m, 206);
                    pump->Hcurve = c;
                }
                else if (match(parser->Tok[m - 1], w_PATTERN))  // Speed/status pattern
                {
                    p = epanet_shared::findpattern(net, parser->Tok[m]);
                    if (p < 0) return setError(parser, m, 205);
                    pump->Upat = p;
                }
                else if (match(parser->Tok[m - 1], w_SPEED))   // Speed setting
                {
                    if (!getfloat(parser->Tok[m], &y)) return setError(parser, m, 202);
                    if (y < 0.0) return setError(parser, m, 211);
                    link->Kc = y;
                }
                else return 201;
                m = m + 2;  // Move to next keyword token
            }
            return 0;
        };
        static int valvedata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes valve data
            **  Format:
            **     [VALVE]
            **        id  node1  node2  diam  type  setting (lcoeff)
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int c,                     // Curve index
                j1,                    // Start-node index
                j2,                    // End-node index
                n;                     // # data items
            char  status = ACTIVE,     // Valve status
                type;                // Valve type
            SCALER diam = 0.0,         // Valve diameter
                setting,            // Valve setting
                lcoeff = 0.0;       // Minor loss coeff.
            Plink link;
            Pvalve valve;
            int err = 0;

            // Add new valve to data base
            n = parser->Ntokens;
            if (net->Nlinks == parser->MaxLinks ||
                net->Nvalves == parser->MaxValves) return 200;
            net->Nvalves++;
            net->Nlinks++;

            // Check for valid data
            if (n < 6)
                return 201;
            if ((j1 = epanet_shared::findnode(net, parser->Tok[1])) == 0)
                return setError(parser, 1, 203);
            if ((j2 = epanet_shared::findnode(net, parser->Tok[2])) == 0)
                return setError(parser, 2, 203);
            if (j1 == j2)
                return setError(parser, 0, 222);

            if (match(parser->Tok[4], w_PRV))
                type = PRV;
            else if (match(parser->Tok[4], w_PSV))
                type = PSV;
            else if (match(parser->Tok[4], w_PBV))
                type = PBV;
            else if (match(parser->Tok[4], w_FCV))
                type = FCV;
            else if (match(parser->Tok[4], w_TCV))
                type = TCV;
            else if (match(parser->Tok[4], w_GPV))
                type = GPV;
            else
                return setError(parser, 4, 213);

            if (!getfloat(parser->Tok[3], &diam)) return setError(parser, 3, 202);
            if (diam <= 0.0) return setError(parser, 3, 211);

            // Find headloss curve for GPV
            if (type == GPV)
            {
                c = epanet_shared::findcurve(net, parser->Tok[5]);
                if (c == 0) return setError(parser, 5, 206);
                setting = c;
                net->Curve[c].Type = HLOSS_CURVE;
                status = OPEN;
            }
            else if (!getfloat(parser->Tok[5], &setting)) return setError(parser, 5, 202);
            if (n >= 7 && !getfloat(parser->Tok[6], &lcoeff)) return setError(parser, 6, 202);

            // Check for illegal connections
            if (epanet_shared::valvecheck(pr, net->Nlinks, type, j1, j2))
            {
                if (j1 > net->Njuncs) return setError(parser, 1, 219);
                else if (j2 > net->Njuncs) return setError(parser, 2, 219);
                else                       return setError(parser, -1, 220);
            }

            // Save valve data
            valve = net->Valve[net->Nvalves];

            net->Link[net->Nlinks] = valve.CastReference<Slink>();
            net->Link[net->Nlinks]->Name_p = parser->Tok[0];
            err = addlinkID(net, net->Nlinks, parser->Tok[0]);
            if (err) return setError(parser, 0, err);
            net->Asset.AddOrReplace(valve.CastReference<Sasset>());

            link = net->Link[net->Nlinks];

            link->N1 = j1;
            link->N2 = j2;
            link->Diam = pr->convertToUnit<foot_t>(diam, true);
            link->Len = 0.0_ft;
            link->Kc = setting;
            link->Km = lcoeff;
            link->Kb = 0.0;
            link->Kw = 0.0;
            link->Type = (LinkType)type;
            link->Status = (StatusType)status;
            link->Rpt = 0;
            link->ResultIndex = 0;
            link->Comment = xstrcpy(&link->Comment, parser->Comment, MAXMSG);
            valve->Link = net->Nlinks;

            return 0;
        };
        static int patterndata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes time pattern data
            **  Format:
            **     [PATTERNS]
            **        id  mult1  mult2 .....
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int i, j, n, n1;
            SCALER x;
            Spattern* pattern;

            // "n" is the number of pattern factors contained in the line
            n = parser->Ntokens - 1;
            if (n < 1) return 201;

            // Check if previous input line was for the same pattern
            if (parser->PrevPat && strcmp(parser->Tok[0], parser->PrevPat->ID) == 0)
            {
                pattern = parser->PrevPat;
            }

            // Otherwise retrieve pattern from the network's Pattern array
            else
            {
                i = epanet_shared::findpattern(net, parser->Tok[0]);
                if (i <= 0) return setError(parser, 0, 205);
                pattern = &(net->Pattern[i]);
                if (pattern->Comment == NULL && parser->Comment[0])
                {
                    pattern->Comment = xstrcpy(&pattern->Comment, parser->Comment, MAXMSG);
                }
            }

            // Expand the pattern's factors array
            j = 1;
            for (auto t0 = pr->times.GetPatternStartTime() + (u64)((pattern->Pat.GetNumValues() - 1) * pr->times.Pstep); j <= n; t0 += (u64)pr->times.Pstep) {
                if (!getfloat(parser->Tok[j], &x)) return setError(parser, j, 202);
                pattern->Pat.AddUniqueValue((u64)t0, x);
                j++;
            }

            // Save a reference to this pattern for processing additional pattern data
            parser->PrevPat = pattern;
            return 0;
        };
        static int curvedata(EN_Project pr)
            /*
            **------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes curve data
            **  Format:
            **     [CURVES]
            **      CurveID   x-value  y-value
            **------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int i;
            SCALER x, y;
            Scurve* curve;

            // Check for valid data
            if (parser->Ntokens < 3) return 201;
            if (!getfloat(parser->Tok[1], &x)) return setError(parser, 1, 202);
            if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);

            // Check if previous input line was for the same curve
            if (parser->PrevCurve && strcmp(parser->Tok[0], parser->PrevCurve->ID) == 0)
            {
                curve = parser->PrevCurve;
            }

            // Otherwise retrieve curve from the network's Curve array
            else
            {
                i = epanet_shared::findcurve(net, parser->Tok[0]);
                if (i == 0) return setError(parser, 0, 206);
                curve = &(net->Curve[i]);
                if (curve->Comment == NULL && parser->Comment[0])
                {
                    curve->Comment = xstrcpy(&curve->Comment, parser->Comment, MAXMSG);
                }
            }

            // Expand size of data arrays if need be
            if (curve->Capacity == curve->Npts)
            {
                if (epanet_shared::resizecurve(curve, curve->Capacity + 10) > 0) return 101;
            }

            // Add new data point to curve
            curve->X[curve->Npts] = x;
            curve->Y[curve->Npts] = y;
            curve->Npts++;

            // Save a reference to this curve for processing additional curve data
            parser->PrevCurve = curve;
            return 0;
        };
        static int coordata(EN_Project pr)
            /*
                **--------------------------------------------------------------
                **  Input:   none
                **  Output:  returns error code
                **  Purpose: processes coordinate data
                **  Format:
                **    [COORD]
                **      id  x  y
                **--------------------------------------------------------------
                */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int j;
            SCALER x, y;
            Pnode node;

            // Check for valid node ID
            if (parser->Ntokens < 3) return 201;
            if ((j = epanet_shared::findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);

            // Check for valid data
            if (!getfloat(parser->Tok[1], &x)) return setError(parser, 1, 202);
            if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);

            // Save coord data
            node = net->Node[j];
            node->X = x;
            node->Y = y;
            return 0;
        };
        static int vertexdata(EN_Project pr)
            /*
                **--------------------------------------------------------------
                **  Input:   none
                **  Output:  returns error code
                **  Purpose: processes link vertex data
                **  Format:
                **    [VERTICES]
                **      id  x  y
                **--------------------------------------------------------------
                */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int j;
            SCALER x, y;

            // Check for valid link ID
            if (parser->Ntokens < 3) return 201;
            if ((j = epanet_shared::findlink(net, parser->Tok[0])) == 0) return setError(parser, 0, 204);

            // Check for valid coordinate data
            if (!getfloat(parser->Tok[1], &x)) return setError(parser, 1, 202);
            if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);

            // Add to link's list of vertex points
            return  epanet_shared::addlinkvertex(net->Link[j], x, y);
        };
        static int demanddata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes node demand data
            **  Format:
            **     [DEMANDS]
            **        MULTIPLY  factor
            **        node  base_demand  (pattern)
            **
            **  NOTE: Demands entered in this section replace those
            **        entered in the [JUNCTIONS] section
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Parser* parser = &pr->parser;

            int j, n, p = 0;
            SCALER y;

            Pdemand demand;

            // Extract data from tokens
            n = parser->Ntokens;
            if (n < 2) return 201;
            if (!getfloat(parser->Tok[1], &y)) return setError(parser, 1, 202);

            // If MULTIPLY command, save multiplier
            if (match(parser->Tok[0], w_MULTIPLY))
            {
                if (y <= 0.0) return setError(parser, 1, 213);
                else hyd->Dmult = y;
                return 0;
            }

            // Otherwise find node (and pattern) being referenced
            if ((j = epanet_shared::findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);
            if (j > net->Njuncs) return 0;
            if (n >= 3)
            {
                p = epanet_shared::findpattern(net, parser->Tok[2]);
                if (p < 0) return setError(parser, 2, 205);
            }

            // Replace any demand entered in [JUNCTIONS] section
            demand = net->Node[j]->D;
            if (demand && hyd->NodeDemand[j] != (cubic_foot_per_second_t)(double)MISSING)
            {
                // First category encountered will overwrite demand category
                // created when junction was read from [JUNCTIONS] section
                demand->Base = y;
                demand->Pat = p;
                if (parser->Comment[0])
                {
                    demand->Name = xstrcpy(&demand->Name, parser->Comment, MAXID);
                }
                hyd->NodeDemand[j] = (cubic_foot_per_second_t)(double)MISSING; // marker - next iteration will append a new category.
            }

            // Otherwise add new demand to junction
            // else if (!adddemand(&net->Node[j], y, p, parser->Comment) > 0) return 101;
            else if (!epanet_shared::adddemand(net->Node[j], y, p, parser->Comment)) return 101;
            return 0;
        };
        static int controldata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes simple controls
            **  Formats:
            **  [CONTROLS]
            **  LINK  linkID  setting IF NODE      nodeID {BELOW/ABOVE}  level
            **  LINK  linkID  setting AT TIME      value  (units)
            **  LINK  linkID  setting AT CLOCKTIME value  (units)
            **   (0)   (1)      (2)   (3) (4)       (5)     (6)          (7)
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int          i = 0,                // Node index
                k,                    // Link index
                n;                    // # data items
            SCALER       setting = MISSING,    // Link setting
                time = 0.0,           // Simulation time
                level = 0.0;          // Pressure or tank level
            StatusType   status = ACTIVE;      // Link status
            ControlType  ctltype;              // Control type
            LinkType     linktype;             // Link type
            Scontrol* control;

            // Check for sufficient number of input tokens
            n = parser->Ntokens;
            if (n < 6) return 201;

            // Check that controlled link exists
            k = epanet_shared::findlink(net, parser->Tok[1]);
            if (k == 0) return setError(parser, 1, 204);

            // Cannot control a check valve
            linktype = net->Link[k]->Type;
            if (linktype == CVPIPE)  return setError(parser, 1, 207);

            // Parse control setting into a status level or numerical setting
            if (match(parser->Tok[2], w_OPEN))
            {
                status = OPEN;
                if (linktype == PUMP) setting = 1.0;
                if (linktype == GPV)  setting = net->Link[k]->Kc;
            }
            else if (match(parser->Tok[2], w_CLOSED))
            {
                status = CLOSED;
                if (linktype == PUMP) setting = 0.0;
                if (linktype == GPV)  setting = net->Link[k]->Kc;
            }
            else if (linktype == GPV) return setError(parser, 1, 207);
            else if (!getfloat(parser->Tok[2], &setting)) return setError(parser, 2, 202);

            // Set status for pump in case speed setting was supplied
            // or for pipe if numerical setting was supplied
            if (linktype == PUMP || linktype == PIPE)
            {
                if (setting != MISSING)
                {
                    if (setting < 0.0)       return setError(parser, 2, 211);
                    else if (setting == 0.0) status = CLOSED;
                    else                     status = OPEN;
                }
            }

            // Determine type of control
            if (match(parser->Tok[4], w_TIME))           ctltype = TIMER;
            else if (match(parser->Tok[4], w_CLOCKTIME)) ctltype = TIMEOFDAY;
            else
            {
                if (n < 8) return 201;
                if ((i = epanet_shared::findnode(net, parser->Tok[5])) == 0) return setError(parser, 5, 203);
                if (match(parser->Tok[6], w_BELOW))      ctltype = LOWLEVEL;
                else if (match(parser->Tok[6], w_ABOVE)) ctltype = HILEVEL;
                else return setError(parser, 6, 213);
            }

            // Parse control level or time
            switch (ctltype)
            {
            case TIMER:
            case TIMEOFDAY:
                if (n == 6) time = hour(parser->Tok[5], (char*)"");
                if (n == 7) time = hour(parser->Tok[5], parser->Tok[6]);
                if (time < 0.0) return setError(parser, 5, 213);
                break;
            case LOWLEVEL:
            case HILEVEL:
                if (!getfloat(parser->Tok[7], &level)) return setError(parser, 7, 202);
                break;
            }

            // Fill in fields of control data structure
            net->Ncontrols++;
            if (net->Ncontrols > parser->MaxControls) return 200;
            control = &net->Control[net->Ncontrols];
            control->Link = k;
            control->Node = i;
            control->Type = ctltype;
            control->Status = status;
            control->Setting = setting;
            control->Time = (long)(3600.0 * time);
            if (ctltype == TIMEOFDAY) control->Time = (double)units::math::fmod((SCALER)(double)control->Time, SECperDAY); // (long)(double)control->Time % (long)SECperDAY;
            control->Grade = level;
            return 0;
        };
        static int sourcedata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes water quality source data
            **  Formats:
            **     [SOURCE]
            **        node  sourcetype  quality  (pattern)
            **
            **  NOTE: units of mass-based source are mass/min
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int i,              // Token with quality value
                j,              // Node index
                n,              // # data items
                p = 0;          // Time pattern index
            char type = CONCEN; // Source type
            SCALER c0 = 0;      // Initial quality
            Psource source;

            // Check for enough tokens & that source node exists
            n = parser->Ntokens;
            if (n < 2) return 201;
            if ((j = epanet_shared::findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);

            // Parse source type
            // NOTE: Under old 1.1 format, SourceType not supplied so
            //       let i = index of token that contains quality value
            i = 2;
            if (match(parser->Tok[1], w_CONCEN))         type = CONCEN;
            else if (match(parser->Tok[1], w_MASS))      type = MASS;
            else if (match(parser->Tok[1], w_SETPOINT))  type = SETPOINT;
            else if (match(parser->Tok[1], w_FLOWPACED)) type = FLOWPACED;
            else i = 1;

            // Parse source quality
            if (!getfloat(parser->Tok[i], &c0))
            {
                if (i == 1) return setError(parser, i, 213);
                else        return setError(parser, i, 202);
            }

            // Parse optional source time pattern
            if (n > i + 1 && strlen(parser->Tok[i + 1]) > 0 &&
                strcmp(parser->Tok[i + 1], "*") != 0)
            {
                p = epanet_shared::findpattern(net, parser->Tok[i + 1]);
                if (p < 0) return setError(parser, i + 1, 205);
            }

            // Destroy any existing source assigned to node
            if (net->Node[j]->S != nullptr) net->Node[j]->S = nullptr;

            // Create a new source & assign it to the node
            source = make_cwee_shared< Ssource>();
            if (source == nullptr) return 101;
            source->C0 = c0;
            source->Pat = p;
            source->Type = (SourceType)type;
            net->Node[j]->S = source;
            return 0;
        };
        static int emitterdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes junction emitter data
            **  Format:
            **     [EMITTER]
            **        node   Ke
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int j,       // Node index
                n;       // # data items
            SCALER k;    // Flow coeff.

            // Check that node exists & is a junction
            n = parser->Ntokens;
            if (n < 2) return 201;
            if ((j = epanet_shared::findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);
            if (j > net->Njuncs) return 0;

            // Parse emitter flow coeff.
            if (!getfloat(parser->Tok[1], &k)) return setError(parser, 1, 202);
            if (k < 0.0) return setError(parser, 1, 209);
            net->Node[j]->Ke = k;
            return 0;
        };
        static int qualdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes initial water quality data
            **  Formats:
            **     [QUALITY]
            **        node   initqual
            **        node1  node2    initqual
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int j, n;
            long i, i1, i2;
            SCALER c0;
            auto& Node = net->Node;

            if (net->Nnodes == 0) return setError(parser, 0, 203);  // No nodes defined yet
            n = parser->Ntokens;
            if (n < 2) return 0;

            // Single node name supplied
            if (n == 2)
            {
                if ((j = epanet_shared::findnode(net, parser->Tok[0])) == 0) return setError(parser, 0, 203);
                if (!getfloat(parser->Tok[1], &c0)) return setError(parser, 1, 202);
                if (c0 < 0.0) return setError(parser, 1, 209);
                Node[j]->C0 = c0;
            }

            // Range of node names supplied
            else
            {
                // Parse quality value
                if (!getfloat(parser->Tok[2], &c0)) return setError(parser, 2, 202);
                if (c0 < 0.0) return setError(parser, 2, 209);

                // If numerical node names supplied, then use numerical comparison
                // to find which nodes are assigned the quality value
                if ((i1 = atol(parser->Tok[0])) > 0 &&
                    (i2 = atol(parser->Tok[1])) > 0)
                {
                    for (j = 1; j <= net->Nnodes; j++)
                    {
                        i = atol(Node[j]->ID);
                        if (i >= i1 && i <= i2) Node[j]->C0 = c0;
                    }
                }

                // Otherwise use lexicographic comparison
                else
                {
                    for (j = 1; j <= net->Nnodes; j++)
                    {
                        if ((strcmp(parser->Tok[0], Node[j]->ID) <= 0) &&
                            (strcmp(parser->Tok[1], Node[j]->ID) >= 0)
                            ) Node[j]->C0 = c0;
                    }
                }
            }
            return 0;
        };
        static int reactdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes reaction coeff. data
            **  Formats:
            **     [REACTIONS]
            **        ORDER     {BULK/WALL/TANK} value
            **        GLOBAL    BULK             coeff
            **        GLOBAL    WALL             coeff
            **        BULK      link1  (link2)   coeff
            **        WALL      link1  (link2)   coeff
            **        TANK      node1  (node2)   coeff
            **        LIMITING  POTENTIAL        value
            **        ROUGHNESS CORRELATION      value
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Quality* qual = &pr->quality;
            Parser* parser = &pr->parser;

            int item, j, n;
            long i, i1, i2;
            SCALER y;

            // Skip line if insufficient data
            n = parser->Ntokens;
            if (n < 3) return 0;

            // Keyword is ORDER
            if (match(parser->Tok[0], w_ORDER))
            {
                if (!getfloat(parser->Tok[n - 1], &y))  return setError(parser, n - 1, 202);
                if (match(parser->Tok[1], w_BULK))      qual->BulkOrder = y;
                else if (match(parser->Tok[1], w_TANK)) qual->TankOrder = y;
                else if (match(parser->Tok[1], w_WALL))
                {
                    if (y == 0.0)      qual->WallOrder = 0.0;
                    else if (y == 1.0) qual->WallOrder = 1.0;
                    else return setError(parser, n - 1, 213);
                }
                else return setError(parser, 1, 213);
                return 0;
            }

            // Keyword is ROUGHNESS
            if (match(parser->Tok[0], w_ROUGHNESS))
            {
                if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);
                qual->Rfactor = y;
                return 0;
            }

            // Keyword is LIMITING
            if (match(parser->Tok[0], w_LIMITING))
            {
                if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);
                qual->Climit = y;
                return 0;
            }

            // Keyword is GLOBAL
            if (match(parser->Tok[0], w_GLOBAL))
            {
                if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);
                if (match(parser->Tok[1], w_BULK))      qual->Kbulk = y;
                else if (match(parser->Tok[1], w_WALL)) qual->Kwall = y;
                else return setError(parser, 1, 213);
                return 0;
            }

            // Keyword is BULK, WALL or TANK
            if (match(parser->Tok[0], w_BULK))      item = 1;
            else if (match(parser->Tok[0], w_WALL)) item = 2;
            else if (match(parser->Tok[0], w_TANK)) item = 3;
            else return setError(parser, 0, 213);

            // Case where tank rate coeffs. are being set
            if (item == 3)
            {
                // Get the rate coeff. value
                if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);

                // Case where just a single tank is specified
                if (n == 3)
                {
                    if ((j = epanet_shared::findnode(net, parser->Tok[1])) <= net->Njuncs) return 0;
                    net->Tank[j - net->Njuncs]->Kb = y;
                }

                // Case where a numerical range of tank IDs is specified
                else if ((i1 = atol(parser->Tok[1])) > 0 &&
                    (i2 = atol(parser->Tok[2])) > 0)
                {
                    for (j = net->Njuncs + 1; j <= net->Nnodes; j++)
                    {
                        i = atol(net->Node[j]->ID);
                        if (i >= i1 && i <= i2) net->Tank[j - net->Njuncs]->Kb = y;
                    }
                }

                // Case where a general range of tank IDs is specified
                else for (j = net->Njuncs + 1; j <= net->Nnodes; j++)
                {
                    if ((strcmp(parser->Tok[1], net->Node[j]->ID) <= 0) &&
                        (strcmp(parser->Tok[2], net->Node[j]->ID) >= 0)
                        ) net->Tank[j - net->Njuncs]->Kb = y;
                }
            }

            // Case where pipe rate coeffs. are being set
            else
            {
                // Get the rate coeff. value
                if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);
                if (net->Nlinks == 0) return 0;

                // Case where just a single link is specified
                if (n == 3)
                {
                    if ((j = epanet_shared::findlink(net, parser->Tok[1])) == 0) return 0;
                    if (item == 1) net->Link[j]->Kb = y;
                    else           net->Link[j]->Kw = y;
                }

                // Case where a numerical range of link IDs is specified
                else if ((i1 = atol(parser->Tok[1])) > 0 &&
                    (i2 = atol(parser->Tok[2])) > 0)
                {
                    for (j = 1; j <= net->Nlinks; j++)
                    {
                        i = atol(net->Link[j]->ID);
                        if (i >= i1 && i <= i2)
                        {
                            if (item == 1)  net->Link[j]->Kb = y;
                            else            net->Link[j]->Kw = y;
                        }
                    }
                }

                // Case where a general range of link IDs is specified
                else for (j = 1; j <= net->Nlinks; j++)
                {
                    if ((strcmp(parser->Tok[1], net->Link[j]->ID) <= 0) &&
                        (strcmp(parser->Tok[2], net->Link[j]->ID) >= 0))
                    {
                        if (item == 1) net->Link[j]->Kb = y;
                        else           net->Link[j]->Kw = y;
                    }
                }
            }
            return 0;
        };
        static int mixingdata(EN_Project pr)
            /*
            **-------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes tank mixing data
            **  Format:
            **    [MIXING]
            **     TankID  MixModel  FractVolume
            **-------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int    i,     // Tank index
                j,     // Node index
                m,     // Type of mixing model
                n;     // Number of data items
            SCALER v;     // Mixing zone volume fraction

            // Check for valid data
            if (net->Nnodes == 0) return setError(parser, 0, 203);
            n = parser->Ntokens;
            if (n < 2) return 0;
            j = epanet_shared::findnode(net, parser->Tok[0]);
            if (j == 0) return setError(parser, 0, 203);
            if (j <= net->Njuncs) return 0;
            if ((m = findmatch(parser->Tok[1], MixTxt)) < 0) return setError(parser, 1, 213);

            // Find mixing zone volume fraction (which can't be 0)
            v = 1.0;
            if ((m == MIX2) && (n == 3) &&
                (!getfloat(parser->Tok[2], &v))) return setError(parser, 2, 202);
            if (v == 0.0) v = 1.0;

            // Assign mixing data to tank (return if tank is a reservoir)
            i = j - net->Njuncs;
            if (net->Tank[i]->A == 0.0_sq_ft) return 0;
            net->Tank[i]->MixModel = (MixType)m;
            net->Tank[i]->V1frac = v;
            return 0;
        };
        static void changestatus(EN_Network net, int j, StatusType status, SCALER y)
            /*
            **--------------------------------------------------------------
            **  Input:   j      = link index
            **           status = status setting (OPEN, CLOSED)
            **           y      = numerical setting (pump speed, valve
            **                    setting)
            **  Output:  none
            **  Purpose: changes status or setting of a link
            **
            **  NOTE: If status = ACTIVE, then a numerical setting (y) was
            **        supplied. If status = OPEN/CLOSED, then numerical
            **        setting is 0.
            **--------------------------------------------------------------
            */
        {
            Plink link = net->Link[j];

            if (link->Type == PIPE || link->Type == GPV)
            {
                if (status != ACTIVE) link->Status = status;
            }
            else if (link->Type == PUMP)
            {
                if (status == ACTIVE)
                {
                    link->Kc = y;
                    status = OPEN;
                    if (y == 0.0) status = CLOSED;
                }
                else if (status == OPEN) link->Kc = 1.0;
                link->Status = status;
            }
            else if (link->Type >= PRV)
            {
                link->Kc = y;
                link->Status = status;
                if (status != ACTIVE) link->Kc = MISSING;
            }
        };
        static int statusdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes link initial status data
            **  Formats:
            **    [STATUS]
            **       link   value
            **       link1  (link2)  value
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;

            int j, n;
            long i, i1, i2;
            SCALER y = 0.0;
            char status = ACTIVE;

            if (net->Nlinks == 0) return setError(parser, 0, 204);
            n = parser->Ntokens - 1;
            if (n < 1) return 201;

            // Check for legal status setting
            if (match(parser->Tok[n], w_OPEN))  status = OPEN;
            else if (match(parser->Tok[n], w_CLOSED)) status = CLOSED;
            else
            {
                if (!getfloat(parser->Tok[n], &y)) return setError(parser, n, 202);
                if (y < 0.0) return setError(parser, n, 211);
            }

            // A single link ID was supplied
            if (n == 1)
            {
                if ((j = epanet_shared::findlink(net, parser->Tok[0])) == 0) return setError(parser, 0, 204);

                // Cannot change status of a Check Valve
                if (net->Link[j]->Type == CVPIPE) return setError(parser, 0, 207);

                // Cannot change setting for a GPV
                if (net->Link[j]->Type == GPV && status == ACTIVE) return setError(parser, 0, 207);
                changestatus(net, j, (StatusType)status, y);
            }

            // A range of numerical link ID's was supplied
            else if ((i1 = atol(parser->Tok[0])) > 0 &&
                (i2 = atol(parser->Tok[1])) > 0)
            {
                for (j = 1; j <= net->Nlinks; j++)
                {
                    i = atol(net->Link[j]->ID);
                    if (i >= i1 && i <= i2) changestatus(net, j, (StatusType)status, y);
                }
            }

            // A range of general link ID's was supplied
            else for (j = 1; j <= net->Nlinks; j++)
            {
                if ((strcmp(parser->Tok[0], net->Link[j]->ID) <= 0) &&
                    (strcmp(parser->Tok[1], net->Link[j]->ID) >= 0)
                    ) changestatus(net, j, (StatusType)status, y);
            }
            return 0;
        };
        static int energydata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes pump energy data
            **  Formats:
            **    [ENERGY]
            **       GLOBAL         {PRICE/PATTERN/EFFIC}  value
            **       PUMP   id      {PRICE/PATTERN/EFFIC}  value
            **       DEMAND CHARGE  value
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Parser* parser = &pr->parser;

            int j, k, n, p, c;
            SCALER y;

            auto& Link = net->Link;
            auto& Pump = net->Pump;

            // Check for sufficient data
            n = parser->Ntokens;
            if (n < 3) return 201;

            // First keyword is DEMAND
            if (match(parser->Tok[0], w_DMNDCHARGE))
            {
                if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);
                if (y < 0.0) return setError(parser, 2, 213);
                hyd->Dcost = y;
                return 0;
            }

            // First keyword is GLOBAL (remaining data refer to global options)
            if (match(parser->Tok[0], w_GLOBAL))
            {
                j = 0;
            }

            // First keyword is PUMP (remaining data refer to a specific pump)
            else if (match(parser->Tok[0], w_PUMP))
            {
                if (n < 4) return 201;
                k = epanet_shared::findlink(net, parser->Tok[1]);
                if (k == 0) return setError(parser, 1, 216);
                if (Link[k]->Type != PUMP) return setError(parser, 1, 216);
                j = epanet_shared::findpump(net, k);
            }
            else return setError(parser, 0, 213);

            // PRICE parameter being set
            if (match(parser->Tok[n - 2], w_PRICE))
            {
                if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);
                if (y < 0.0) return setError(parser, n - 1, 217);
                if (j == 0) hyd->Ecost = y;
                else        Pump[j]->Ecost = y;
                return 0;
            }

            // Price PATTERN being set
            else if (match(parser->Tok[n - 2], w_PATTERN))
            {
                p = epanet_shared::findpattern(net, parser->Tok[n - 1]);
                if (p < 0) return setError(parser, n - 1, 205);
                if (j == 0) hyd->Epat = p;
                else        Pump[j]->Epat = p;
                return 0;
            }

            // Pump EFFIC being set
            else if (match(parser->Tok[n - 2], w_EFFIC))
            {
                if (j == 0)
                {
                    if (!getfloat(parser->Tok[n - 1], &y)) return setError(parser, n - 1, 202);
                    if (y <= 0.0) return setError(parser, n - 1, 217);
                    hyd->Epump = y;
                }
                else
                {
                    c = epanet_shared::findcurve(net, parser->Tok[n - 1]);
                    if (c == 0) return setError(parser, n - 1, 206);
                    Pump[j]->Ecurve = c;
                    net->Curve[c].Type = EFFIC_CURVE;
                }
                return 0;
            }
            return 201;
        };
        static int reportdata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes report options data
            **  Formats:
            **    PAGE     linesperpage
            **    STATUS   {NONE/YES/FULL}
            **    SUMMARY  {YES/NO}
            **    MESSAGES {YES/NO}
            **    ENERGY   {NO/YES}
            **    NODES    {NONE/ALL}
            **    NODES    node1  node2 ...
            **    LINKS    {NONE/ALL}
            **    LINKS    link1  link2 ...
            **    FILE     filename
            **    variable {YES/NO}
            **    variable {BELOW/ABOVE/PRECISION}  value
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Report* rpt = &pr->report;
            Parser* parser = &pr->parser;

            int i, j, n;
            SCALER y;

            n = parser->Ntokens - 1;
            if (n < 1) return 201;

            // Value for page size
            if (match(parser->Tok[0], w_PAGE))
            {
                if (!getfloat(parser->Tok[n], &y)) return setError(parser, n, 202);
                if (y < 0.0 || y > 255.0) return setError(parser, n, 213);
                rpt->PageSize = (int)y;
                return 0;
            }

            // Request that status reports be written
            if (match(parser->Tok[0], w_STATUS))
            {
                if (match(parser->Tok[n], w_NO))   rpt->Statflag = FALSE;
                if (match(parser->Tok[n], w_YES))  rpt->Statflag = TRUE;
                if (match(parser->Tok[n], w_FULL)) rpt->Statflag = FULL;
                return 0;
            }

            // Request summary report
            if (match(parser->Tok[0], w_SUMMARY))
            {
                if (match(parser->Tok[n], w_NO)) rpt->Summaryflag = FALSE;
                if (match(parser->Tok[n], w_YES))   rpt->Summaryflag = TRUE;
                return 0;
            }

            // Request error/warning message reporting
            if (match(parser->Tok[0], w_MESSAGES))
            {
                if (match(parser->Tok[n], w_NO))  rpt->Messageflag = FALSE;
                if (match(parser->Tok[n], w_YES)) rpt->Messageflag = TRUE;
                return 0;
            }

            // Request an energy usage report
            if (match(parser->Tok[0], w_ENERGY))
            {
                if (match(parser->Tok[n], w_NO))  rpt->Energyflag = FALSE;
                if (match(parser->Tok[n], w_YES)) rpt->Energyflag = TRUE;
                return 0;
            }

            // Particular reporting nodes specified
            if (match(parser->Tok[0], w_NODE))
            {
                if (match(parser->Tok[n], w_NONE))     rpt->Nodeflag = 0; // No nodes
                else if (match(parser->Tok[n], w_ALL)) rpt->Nodeflag = 1; // All nodes
                else
                {
                    if (net->Nnodes == 0) return setError(parser, 1, 203);
                    for (i = 1; i <= n; i++)
                    {
                        if ((j = epanet_shared::findnode(net, parser->Tok[i])) == 0) return setError(parser, i, 203);
                        net->Node[j]->Rpt = 1;
                    }
                    rpt->Nodeflag = 2;
                }
                return 0;
            }

            // Particular reporting links specified
            if (match(parser->Tok[0], w_LINK))
            {
                if (match(parser->Tok[n], w_NONE))     rpt->Linkflag = 0;
                else if (match(parser->Tok[n], w_ALL)) rpt->Linkflag = 1;
                else
                {
                    if (net->Nlinks == 0) return setError(parser, 1, 204);
                    for (i = 1; i <= n; i++)
                    {
                        if ((j = epanet_shared::findlink(net, parser->Tok[i])) == 0) return setError(parser, i, 204);
                        net->Link[j]->Rpt = 1;
                    }
                    rpt->Linkflag = 2;
                }
                return 0;
            }

            // Report fields specified
            // Special case needed to distinguish "HEAD" from "HEADLOSS"
            if (strcomp(parser->Tok[0], t_HEADLOSS)) i = HEADLOSS;
            else i = findmatch(parser->Tok[0], Fldname);
            if (i >= 0)
            {
                if (i > FRICTION) return setError(parser, 0, 213);
                if (parser->Ntokens == 1 || match(parser->Tok[1], w_YES))
                {
                    rpt->Field[i].Enabled = TRUE;
                    return 0;
                }

                if (match(parser->Tok[1], w_NO))
                {
                    rpt->Field[i].Enabled = FALSE;
                    return 0;
                }

                // Get field qualifier type
                if (parser->Ntokens < 3) return 201;
                if (match(parser->Tok[1], w_BELOW))     j = LOW;
                else if (match(parser->Tok[1], w_ABOVE))     j = HI;
                else if (match(parser->Tok[1], w_PRECISION)) j = PREC;
                else return setError(parser, 1, 213);

                // Get field qualifier value
                if (!getfloat(parser->Tok[2], &y)) return setError(parser, 2, 202);
                if (j == PREC)
                {
                    rpt->Field[i].Enabled = TRUE;
                    rpt->Field[i].Precision = ROUND(y);
                }
                else rpt->Field[i].RptLim[j] = y;
                return (0);
            }

            // Name of external report file
            if (match(parser->Tok[0], w_FILE))
            {
                strncpy(rpt->Rpt2Fname, parser->Tok[1], MAXFNAME);
                return 0;
            }

            // If get to here then return error condition
            return 201;
        };
        static int timedata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes time options data
            **  Formats:
            **    STATISTIC                  {NONE/AVERAGE/MIN/MAX/RANGE}
            **    DURATION                   value   (units)
            **    HYDRAULIC TIMESTEP         value   (units)
            **    QUALITY TIMESTEP           value   (units)
            **    MINIMUM TRAVELTIME         value   (units)
            **    RULE TIMESTEP              value   (units)
            **    PATTERN TIMESTEP           value   (units)
            **    PATTERN START              value   (units)
            **    REPORT TIMESTEP            value   (units)
            **    REPORT START               value   (units)
            **    START CLOCKTIME            value   (AM PM)
            **-------------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;
            Parser* parser = &pr->parser;
            Times* time = &pr->times;

            int n;
            long t;
            SCALER y;

            n = parser->Ntokens - 1;
            if (n < 1) return 201;

            // Check if setting report time statistic flag
            if (match(parser->Tok[0], w_STATISTIC))
            {
                if (match(parser->Tok[n], w_NONE))  rpt->Tstatflag = SERIES;
                else if (match(parser->Tok[n], w_NO))    rpt->Tstatflag = SERIES;
                else if (match(parser->Tok[n], w_AVG))   rpt->Tstatflag = AVG;
                else if (match(parser->Tok[n], w_MIN))   rpt->Tstatflag = MIN;
                else if (match(parser->Tok[n], w_MAX))   rpt->Tstatflag = MAX;
                else if (match(parser->Tok[n], w_RANGE)) rpt->Tstatflag = RANGE;
                else return setError(parser, n, 213);
                return 0;
            }

            // Convert text time value to numerical value in seconds
            // Examples:
            //    5           = 5 * 3600 sec
            //    5 MINUTES   = 5 * 60   sec
            //    13:50       = 13*3600 + 50*60 sec
            //    1:50 pm     = (12+1)*3600 + 50*60 sec

            if (!getfloat(parser->Tok[n], &y))
            {
                if ((y = hour(parser->Tok[n], (char*)"")) < 0.0)
                {
                    if ((y = hour(parser->Tok[n - 1], parser->Tok[n])) < 0.0)
                    {
                        return setError(parser, n - 1, 213);
                    }
                }
            }
            t = (long)(3600.0 * y + 0.5);

            /// Process the value assigned to the matched parameter
            if (match(parser->Tok[0], w_DURATION))  time->Dur = t;
            else if (match(parser->Tok[0], w_HYDRAULIC)) time->Hstep = t;
            else if (match(parser->Tok[0], w_QUALITY))  time->Qstep = t;
            else if (match(parser->Tok[0], w_RULE))      time->Rulestep = t;
            else if (match(parser->Tok[0], w_MINIMUM))   return 0; // Not used anymore
            else if (match(parser->Tok[0], w_PATTERN))
            {
                if (match(parser->Tok[1], w_TIME))  time->Pstep = t;
                else if (match(parser->Tok[1], w_START)) time->Pstart = t;
                else return setError(parser, 1, 213);
            }
            else if (match(parser->Tok[0], w_REPORT))
            {
                if (match(parser->Tok[1], w_TIME))  time->Rstep = t;
                else if (match(parser->Tok[1], w_START)) time->Rstart = t;
                else return setError(parser, 1, 213);
            }
            else if (match(parser->Tok[0], w_START)) time->Tstart = (double)units::math::fmod((SCALER)t, SECperDAY); // t % (long)SECperDAY;
            else return setError(parser, 0, 213);
            return 0;
        };
        static int optionchoice(EN_Project pr, int n)
            /*
            **--------------------------------------------------------------
            **  Input:   n = index of last input token
            **  Output:  returns error code or 0 if option belongs to
            **           those listed below, or -1 otherwise
            **  Purpose: processes fixed choice [OPTIONS] data
            **  Formats:
            **    UNITS               CFS/GPM/MGD/IMGD/AFD/LPS/LPM/MLD/CMH/CMD/SI
            **    PRESSURE            PSI/KPA/M
            **    HEADLOSS            H-W/D-W/C-M
            **    HYDRAULICS          USE/SAVE  filename
            **    QUALITY             NONE/AGE/TRACE/CHEMICAL  (TraceNode)
            **    MAP                 filename
            **    VERIFY              filename
            **    UNBALANCED          STOP/CONTINUE {Niter}
            **    PATTERN             id
            **    DEMAND MODEL        DDA/PDA
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Parser* parser = &pr->parser;
            Outfile* out = &pr->outfile;

            int choice;

            // Check if 1st token matches a parameter name and
            // process the input for the matched parameter
            if (n < 0) return 201;

            // Flow UNITS
            if (match(parser->Tok[0], w_UNITS))
            {
                if (n < 1) return 0;
                else if (match(parser->Tok[1], w_CFS))  parser->Flowflag = CFS;
                else if (match(parser->Tok[1], w_GPM))  parser->Flowflag = GPM;
                else if (match(parser->Tok[1], w_AFD))  parser->Flowflag = AFD;
                else if (match(parser->Tok[1], w_MGD))  parser->Flowflag = MGD;
                else if (match(parser->Tok[1], w_IMGD)) parser->Flowflag = IMGD;
                else if (match(parser->Tok[1], w_LPS))  parser->Flowflag = LPS;
                else if (match(parser->Tok[1], w_LPM))  parser->Flowflag = LPM;
                else if (match(parser->Tok[1], w_CMH))  parser->Flowflag = CMH;
                else if (match(parser->Tok[1], w_CMD))  parser->Flowflag = CMD;
                else if (match(parser->Tok[1], w_MLD))  parser->Flowflag = MLD;
                else if (match(parser->Tok[1], w_SI))   parser->Flowflag = LPS;
                else return setError(parser, 1, 213);
            }

            // PRESSURE units
            else if (match(parser->Tok[0], w_PRESSURE))
            {
                if (n < 1) return 0;
                else if (match(parser->Tok[1], w_EXPONENT)) return -1;
                else if (match(parser->Tok[1], w_PSI))    parser->Pressflag = PSI;
                else if (match(parser->Tok[1], w_KPA))    parser->Pressflag = KPA;
                else if (match(parser->Tok[1], w_METERS)) parser->Pressflag = METERS;
                else return setError(parser, 1, 213);
            }

            // HEADLOSS formula
            else if (match(parser->Tok[0], w_HEADLOSS))
            {
                if (n < 1)  return 0;
                else if (match(parser->Tok[1], w_HW))   hyd->Formflag = HW;
                else if (match(parser->Tok[1], w_DW))   hyd->Formflag = DW;
                else if (match(parser->Tok[1], w_CM))   hyd->Formflag = CM;
                else return setError(parser, 1, 213);
            }

            // HYDRUALICS USE/SAVE file option
            else if (match(parser->Tok[0], w_HYDRAULIC))
            {
                if (n < 2) return 0;
                else if (match(parser->Tok[1], w_USE))  out->Hydflag = USE;
                else if (match(parser->Tok[1], w_SAVE)) out->Hydflag = SAVE;
                else return setError(parser, 1, 213);
                strncpy(out->HydFname, parser->Tok[2], MAXFNAME);
            }

            // Water QUALITY option
            else if (match(parser->Tok[0], w_QUALITY))
            {
                if (n < 1) return 0;
                else if (match(parser->Tok[1], w_NONE))  qual->Qualflag = NONE;
                else if (match(parser->Tok[1], w_CHEM))  qual->Qualflag = CHEM;
                else if (match(parser->Tok[1], w_ENERGYINTENSITY))  qual->Qualflag = ENERGYINTENSITY;
                else if (match(parser->Tok[1], w_AGE))   qual->Qualflag = AGE;
                else if (match(parser->Tok[1], w_TRACE)) qual->Qualflag = TRACE_QUAL;
                else
                {
                    qual->Qualflag = CHEM;
                    strncpy(qual->ChemName, parser->Tok[1], MAXID);
                    if (n >= 2) strncpy(qual->ChemUnits, parser->Tok[2], MAXID);
                }
                if (qual->Qualflag == TRACE_QUAL)
                {
                    // Copy Trace Node ID to parser->Tok[0] for error reporting
                    ::strcpy(parser->Tok[0], "");
                    if (n < 2) return 201;
                    ::strcpy(parser->Tok[0], parser->Tok[2]);
                    qual->TraceNode = parser->Tok[2];
                    // if (qual->TraceNode == 0) return setError(parser, 2, 212);
                    strncpy(qual->ChemName, u_PERCENT, MAXID);
                    strncpy(qual->ChemUnits, parser->Tok[2], MAXID);
                }
                if (qual->Qualflag == AGE)
                {
                    strncpy(qual->ChemName, w_AGE, MAXID);
                    strncpy(qual->ChemUnits, u_HOURS, MAXID);
                }
            }

            // MAP file name
            else if (match(parser->Tok[0], w_MAP))
            {
                if (n < 1) return 0;
                strncpy(pr->MapFname, parser->Tok[1], MAXFNAME);
            }

            else if (match(parser->Tok[0], w_VERIFY))
            {
                // Deprecated
            }

            // Hydraulics UNBALANCED option
            else if (match(parser->Tok[0], w_UNBALANCED))
            {
                if (n < 1) return 0;
                if (match(parser->Tok[1], w_STOP)) hyd->ExtraIter = -1;
                else if (match(parser->Tok[1], w_CONTINUE))
                {
                    if (n >= 2)  hyd->ExtraIter = atoi(parser->Tok[2]);
                    else         hyd->ExtraIter = 0;
                }
                else return setError(parser, 1, 213);
            }

            // Default demand PATTERN
            else if (match(parser->Tok[0], w_PATTERN))
            {
                if (n < 1) return 0;
                strncpy(parser->DefPatID, parser->Tok[1], MAXID);
            }

            // DEMAND model
            else if (match(parser->Tok[0], w_DEMAND))
            {
                if (n < 2) return 0;
                if (!match(parser->Tok[1], w_MODEL)) return -1;
                choice = findmatch(parser->Tok[2], DemandModelTxt);
                if (choice < 0) return setError(parser, 2, 213);
                hyd->DemandModel = choice;
            }

            // Return -1 if keyword did not match any option
            else return -1;
            return 0;
        };
        static int optionvalue(EN_Project pr, int n)
            /*
            **-------------------------------------------------------------
            **  Input:   *line = line read from input file
            **  Output:  returns error code
            **  Purpose: processes numerical value [OPTIONS] data
            **  Formats:
            **    DEMAND MULTIPLIER   value
            **    EMITTER EXPONENT    value
            **    VISCOSITY           value
            **    DIFFUSIVITY         value
            **    SPECIFIC GRAVITY    value
            **    TRIALS              value
            **    ACCURACY            value

            **    HEADERROR           value
            **    FLOWCHANGE          value
            **    MINIMUM PRESSURE    value
            **    REQUIRED PRESSURE   value
            **    PRESSURE EXPONENT   value

            **    TOLERANCE           value
            **    SEGMENTS            value  (not used)
            **  ------ Undocumented Options -----
            **    HTOL                value
            **    QTOL                value
            **    RQTOL               value
            **    CHECKFREQ           value
            **    MAXCHECK            value
            **    DAMPLIMIT           value
            **--------------------------------------------------------------
            */
        {
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Parser* parser = &pr->parser;

            int    nvalue = 1;   // Index of token with numerical value
            SCALER y;
            char* tok0 = parser->Tok[0];

            // Check for deprecated SEGMENTS keyword
            if (match(tok0, w_SEGMENTS)) return 0;

            // Check for missing value (which is permissible)
            if (match(tok0, w_SPECGRAV) || match(tok0, w_EMITTER) ||
                match(tok0, w_DEMAND) || match(tok0, w_MINIMUM) ||
                match(tok0, w_REQUIRED) || match(tok0, w_PRESSURE) ||
                match(tok0, w_PRECISION)
                ) nvalue = 2;
            if (n < nvalue) return 0;

            // Check for valid numerical input
            if (!getfloat(parser->Tok[nvalue], &y)) return setError(parser, nvalue, 202);

            // Quality tolerance option (which can be 0)
            if (match(tok0, w_TOLERANCE))
            {
                if (y < 0.0) return setError(parser, nvalue, 213);
                qual->Ctol = y;
                return 0;
            }

            // Diffusivity
            if (match(tok0, w_DIFFUSIVITY))
            {
                if (y < 0.0) return setError(parser, nvalue, 213);
                qual->Diffus = (double)y;
                return 0;
            }

            // Hydraulic damping limit option */
            if (match(tok0, w_DAMPLIMIT))
            {
                hyd->DampLimit = y;
                return 0;
            }

            // Flow change limit
            else if (match(tok0, w_FLOWCHANGE))
            {
                if (y < 0.0) return setError(parser, nvalue, 213);
                hyd->FlowChangeLimit = pr->convertToUnit<cubic_foot_per_second_t>(y);
                return 0;
            }

            // Head loss error limit
            else if (match(tok0, w_HEADERROR))
            {
                if (y < 0.0) return setError(parser, nvalue, 213);
                hyd->HeadErrorLimit = pr->convertToUnit<foot_t>(y);
                return 0;
            }

            // Pressure dependent demand parameters
            else if (match(tok0, w_MINIMUM))
            {
                if (y < 0.0) return setError(parser, nvalue, 213);
                // Required pressure still at default value
                if (hyd->Preq == MINPDIFF)
                    hyd->Preq = y + MINPDIFF;
                // Required pressure already entered
                else if (hyd->Preq - y < MINPDIFF)
                    return setError(parser, nvalue, 208);
                hyd->Pmin = y;
                return 0;
            }
            else if (match(tok0, w_REQUIRED))
            {
                if (y < 0.0) return setError(parser, nvalue, 213);
                if (y - hyd->Pmin < MINPDIFF)
                    return setError(parser, nvalue, 208);
                hyd->Preq = y;
                return 0;
            }
            else if (match(tok0, w_PRESSURE))
            {
                if (y < 0.0) return setError(parser, nvalue, 213);
                hyd->Pexp = y;
                return 0;
            }

            // All other options must be > 0
            if (y <= 0.0) return setError(parser, nvalue, 213);

            // Assign value to all other options
            if (match(tok0, w_VISCOSITY))     hyd->Viscos = (double)y;
            else if (match(tok0, w_SPECGRAV)) hyd->SpGrav = y;
            else if (match(tok0, w_TRIALS))   hyd->MaxIter = (int)y;
            else if (match(tok0, w_ACCURACY))
            {
                y = fMAX(y, 1.e-5);
                y = fMIN(y, 1.e-1);
                hyd->Hacc = y;
            }
            else if (match(tok0, w_HTOL))  hyd->Htol = (double)y;
            else if (match(tok0, w_QTOL))  hyd->Qtol = (double)y;
            else if (match(tok0, w_RQTOL))
            {
                if (y >= 1.0) return 213;
                hyd->RQtol = y;
            }
            else if (match(tok0, w_CHECKFREQ)) hyd->CheckFreq = (int)y;
            else if (match(tok0, w_MAXCHECK))  hyd->MaxCheck = (int)y;
            else if (match(tok0, w_EMITTER))   hyd->Qexp = 1.0 / y;
            else if (match(tok0, w_DEMAND))    hyd->Dmult = y;
            else return 201;
            return 0;
        };
        static int optiondata(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: processes [OPTIONS] data
            **--------------------------------------------------------------
            */
        {
            int i, n;
            Parser* parser = &pr->parser;

            // Option is a named choice
            n = parser->Ntokens - 1;
            i = optionchoice(pr, n);
            if (i >= 0) return i;

            // Option is a numerical value
            return (optionvalue(pr, n));
        };
        static void newrule(EN_Project pr)
            //----------------------------------------------------------
            //    Adds a new rule to the project
            //----------------------------------------------------------
        {
            EN_Network net = pr->network;

            char** Tok = pr->parser.Tok;
            Srule* rule = &net->Rule[net->Nrules];

            strncpy(rule->label, Tok[1], MAXID);
            rule->Premises = NULL;
            rule->ThenActions = NULL;
            rule->ElseActions = NULL;
            rule->priority = 0.0;
            pr->rules.LastPremise = NULL;
            pr->rules.LastThenAction = NULL;
            pr->rules.LastElseAction = NULL;
        };
        static int newpremise(EN_Project pr, int logop)
            //--------------------------------------------------------------------
            //   Adds new premise to current rule.
            //   Formats are:
            //     IF/AND/OR <object> <id> <variable> <operator> <value>
            //     IF/AND/OR  SYSTEM <variable> <operator> <value> (units)
            //---------------------------------------------------------------------
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;
            Rules* rules = &pr->rules;

            int i, j, k, m, r, s, v;
            SCALER x;
            char** Tok = parser->Tok;
            Spremise* p;

            // Check for correct number of tokens
            if (parser->Ntokens != 5 && parser->Ntokens != 6) return 201;

            // Find network object & id if present
            i = findmatch(Tok[1], Object);
            if (i == r_SYSTEM)
            {
                j = 0;
                v = findmatch(Tok[2], Varword);
                if (v != r_DEMAND && v != r_TIME && v != r_CLOCKTIME) return 201;
            }
            else
            {
                v = findmatch(Tok[3], Varword);
                if (v < 0) return (201);
                switch (i)
                {
                case r_NODE:
                case r_JUNC:
                case r_RESERV:
                case r_TANK:
                    k = r_NODE;
                    break;
                case r_LINK:
                case r_PIPE:
                case r_PUMP:
                case r_VALVE:
                    k = r_LINK;
                    break;
                default:
                    return 201;
                }
                i = k;
                if (i == r_NODE)
                {
                    j = epanet_shared::findnode(net, Tok[2]);
                    if (j == 0) return 203;
                    switch (v)
                    {
                    case r_DEMAND:
                    case r_HEAD:
                    case r_GRADE:
                    case r_LEVEL:
                    case r_PRESSURE:
                        break;
                    case r_FILLTIME:
                    case r_DRAINTIME:
                        if (j <= net->Njuncs) return 201;
                        break;
                    default:
                        return 201;
                    }
                }
                else
                {
                    j = epanet_shared::findlink(net, Tok[2]);
                    if (j == 0) return 204;
                    switch (v)
                    {
                    case r_FLOW:
                    case r_STATUS:
                    case r_SETTING:
                        break;
                    default:
                        return 201;
                    }
                }
            }

            // Parse relational operator (r) and check for synonyms
            if (i == r_SYSTEM) m = 3;
            else m = 4;
            k = findmatch(Tok[m], Operator);
            if (k < 0) return 201;
            switch (k)
            {
            case IS:
                r = EQ;
                break;
            case NOT:
                r = NE;
                break;
            case BELOW:
                r = LT;
                break;
            case ABOVE:
                r = GT;
                break;
            default:
                r = k;
            }

            // Parse for status (s) or numerical value (x)
            s = 0;
            x = MISSING;
            if (v == r_TIME || v == r_CLOCKTIME)
            {
                if (parser->Ntokens == 6) x = hour(Tok[4], Tok[5]) * 3600.;
                else                      x = hour(Tok[4], (char*)"") * 3600.;
                if (x < 0.0) return 202;
            }
            else if ((k = findmatch(Tok[parser->Ntokens - 1], Value)) > IS_NUMBER) s = k;
            else
            {
                if (!getfloat(Tok[parser->Ntokens - 1], &x))
                    return (202);
                if (v == r_FILLTIME || v == r_DRAINTIME) x = x * 3600.0;
            }

            // Create new premise structure
            p = (Spremise*)malloc(sizeof(Spremise));
            if (p == NULL) return 101;
            p->object = i;
            p->index = j;
            p->variable = v;
            p->relop = r;
            p->logop = logop;
            p->status = s;
            p->value = x;

            // Add premise to current rule's premise list
            p->next = NULL;
            if (rules->LastPremise == NULL) net->Rule[net->Nrules].Premises = p;
            else rules->LastPremise->next = p;
            rules->LastPremise = p;
            return 0;
        };
        static int newaction(EN_Project pr)
            //----------------------------------------------------------
            //   Adds new action to current rule.
            //   Format is:
            //      THEN/ELSE/AND LINK <id> <variable> IS <value>
            //----------------------------------------------------------
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;
            Rules* rules = &pr->rules;

            int j, k, s;
            SCALER x;
            Saction* a;
            char** Tok = parser->Tok;

            // Check for correct number of tokens
            if (parser->Ntokens != 6) return 201;

            // Check that link exists
            j = epanet_shared::findlink(net, Tok[2]);
            if (j == 0) return 204;

            // Cannot control a CV
            if (net->Link[j]->Type == CVPIPE) return 207;

            // Find value for status or setting
            s = -1;
            x = MISSING;
            if ((k = findmatch(Tok[5], Value)) > IS_NUMBER) s = k;
            else
            {
                if (!getfloat(Tok[5], &x)) return 202;
                if (x < 0.0) return 202;
            }

            // Cannot change setting for a GPV
            if (x != MISSING && net->Link[j]->Type == GPV) return 202;

            // Set status for pipe in case setting was specified
            if (x != MISSING && net->Link[j]->Type == PIPE)
            {
                if (x == 0.0) s = IS_CLOSED;
                else          s = IS_OPEN;
                x = MISSING;
            }

            // Create a new action structure
            a = (Saction*)malloc(sizeof(Saction));
            if (a == NULL) return 101;
            a->link = j;
            a->status = s;
            a->setting = x;

            // Add action to current rule's action list
            if (rules->RuleState == r_THEN)
            {
                a->next = NULL;
                if (rules->LastThenAction == NULL)
                {
                    net->Rule[net->Nrules].ThenActions = a;
                }
                else rules->LastThenAction->next = a;
                rules->LastThenAction = a;
            }
            else
            {
                a->next = NULL;
                if (rules->LastElseAction == NULL)
                {
                    net->Rule[net->Nrules].ElseActions = a;
                }
                else rules->LastElseAction->next = a;
                rules->LastElseAction = a;
            }
            return 0;
        };
        static int newpriority(EN_Project pr)
            //---------------------------------------------------
            //    Adds priority rating to current rule
            //---------------------------------------------------
        {
            EN_Network net = pr->network;

            SCALER x;
            char** Tok = pr->parser.Tok;

            if (!getfloat(Tok[1], &x)) return 202;
            net->Rule[net->Nrules].priority = x;
            return 0;
        };
        static void ruleerrmsg(EN_Project pr)
            //-----------------------------------------------------------
            //    Report a rule parsing error message
            //-----------------------------------------------------------
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;
            Rules* rules = &pr->rules;

            int i;
            char label[MAXMSG + 1];
            char msg[MAXLINE + 1];
            char** Tok = parser->Tok;

            // Get text of error message
            switch (rules->Errcode)
            {
            case 201: strcpy(msg, R_ERR201); break;
            case 202: strcpy(msg, R_ERR202); break;
            case 203: strcpy(msg, R_ERR203); break;
            case 204: strcpy(msg, R_ERR204); break;
            case 207: strcpy(msg, R_ERR207); break;
            case 221: strcpy(msg, R_ERR221); break;
            default: return;
            }

            // Get label of rule being parsed
            if (net->Nrules > 0)
            {
                strncpy(label, t_RULE, MAXMSG);
                strncat(label, " ", MAXMSG);
                strncat(label, net->Rule[net->Nrules].label, MAXMSG);
            }
            else strncpy(label, t_RULES_SECT, MAXMSG);

            // Write rule label and error message to status report
            _snprintf(pr->Msg, MAXMSG, "%s", msg);
            strncat(pr->Msg, label, MAXMSG);
            strncat(pr->Msg, ":", MAXMSG);
            epanet_shared::writeline(pr, pr->Msg);

            // Write text of rule clause being parsed to status report
            strcpy(msg, Tok[0]);
            for (i = 1; i < parser->Ntokens; i++)
            {
                strncat(msg, " ", MAXLINE);
                strncat(msg, Tok[i], MAXLINE);
            }
            epanet_shared::writeline(pr, msg);
        };
        static int ruledata(EN_Project pr)
            //--------------------------------------------------------------
            //    Parses a line from [RULES] section of input.
            //--------------------------------------------------------------
        {
            EN_Network net = pr->network;
            Parser* parser = &pr->parser;
            Rules* rules = &pr->rules;

            int key,      // Keyword code
                err;
            char** Tok = parser->Tok;  // Tokenized line of a rule statement

            // Exit if current rule has an error
            if (rules->RuleState == r_ERROR) return 0;

            // Find the key word that begins the rule statement
            err = 0;
            key = findmatch(Tok[0], Ruleword);
            switch (key)
            {
            case -1:
                err = 201;   // Unrecognized keyword
                break;

            case r_RULE:
                // Missing the rule label
                if (parser->Ntokens != 2)
                {
                    err = 201;
                    break;
                }
                net->Nrules++;
                newrule(pr);
                rules->RuleState = r_RULE;
                rules->Errcode = 0;
                break;

            case r_IF:
                if (rules->RuleState != r_RULE)
                {
                    err = 221;   // Mis-placed IF clause
                    break;
                }
                rules->RuleState = r_IF;
                err = newpremise(pr, r_AND);
                break;

            case r_AND:
                if (rules->RuleState == r_IF) err = newpremise(pr, r_AND);
                else if (rules->RuleState == r_THEN || rules->RuleState == r_ELSE)
                {
                    err = newaction(pr);
                }
                else err = 221;
                break;

            case r_OR:
                if (rules->RuleState == r_IF) err = newpremise(pr, r_OR);
                else err = 221;
                break;

            case r_THEN:
                if (rules->RuleState != r_IF)
                {
                    err = 221;   // Mis-placed THEN clause
                    break;
                }
                rules->RuleState = r_THEN;
                err = newaction(pr);
                break;

            case r_ELSE:
                if (rules->RuleState != r_THEN)
                {
                    err = 221;   // Mis-placed ELSE clause
                    break;
                }
                rules->RuleState = r_ELSE;
                err = newaction(pr);
                break;

            case r_PRIORITY:
                if (rules->RuleState != r_THEN && rules->RuleState != r_ELSE)
                {
                    err = 221;
                    break;
                }
                rules->RuleState = r_PRIORITY;
                err = newpriority(pr);
                break;

            default:
                err = 201;
            }

            // Set RuleState to r_ERROR if errors found
            if (err)
            {
                rules->RuleState = r_ERROR;
                rules->Errcode = err;
                err = 200;
            }
            return err;
        };
        static int newline(EN_Project pr, int sect, char* line)
            /*
            **--------------------------------------------------------------
            **  Input:   sect  = current section of input file
            **           *line = line read from input file
            **  Output:  returns error code or 0 if no error found
            **  Purpose: processes a new line of data from input file
            **
            **  Note: The xxxdata() functions appear in INPUT3.c.
            **--------------------------------------------------------------
            */
        {
            Parser* parser = &pr->parser;
            int n;

            switch (sect)
            {
            case _TITLE:
                if (parser->Ntitle < 3)
                {
                    n = (int)strlen(line);
                    if (line[n - 1] == 10)
                        line[n - 1] = '\0';
                    strncpy(pr->Title[parser->Ntitle], line, TITLELEN);
                    parser->Ntitle++;
                }
                return 0;
            case _JUNCTIONS:   return (juncdata(pr));
            case _RESERVOIRS:
            case _TANKS:       return (tankdata(pr));
            case _PIPES:       return (pipedata(pr));
            case _PUMPS:       return (pumpdata(pr));
            case _VALVES:      return (valvedata(pr));
            case _PATTERNS:    return (patterndata(pr));
            case _CURVES:      return (curvedata(pr));
            case _DEMANDS:     return (demanddata(pr));
            case _CONTROLS:    return (controldata(pr));
            case _RULES:
                if (ruledata(pr) > 0)
                {
                    ruleerrmsg(pr);
                    return 200;
                }
                else return 0;
            case _SOURCES:     return (sourcedata(pr));
            case _EMITTERS:    return (emitterdata(pr));
            case _QUALITY:     return (qualdata(pr));
            case _STATUS:      return (statusdata(pr));
            case _ROUGHNESS:   return (0);
            case _ENERGY:      return (energydata(pr));
            case _REACTIONS:   return (reactdata(pr));
            case _MIXING:      return (mixingdata(pr));
            case _REPORT:      return (reportdata(pr));
            case _TIMES:       return (timedata(pr));
            case _OPTIONS:     return (optiondata(pr));
            case _COORDS:      return (coordata(pr));
            case _VERTICES:    return (vertexdata(pr));

                // Data in these sections are not used for any computations
            case _LABELS:
            case _TAGS:
            case _BACKDROP:
                return (0);
            }
            return 201;
        };
        static int getpumpparams(EN_Project pr)
            /*
            **-------------------------------------------------------------
            **  Input:   none
            **  Output:  returns error code
            **  Purpose: computes pump curve coefficients for all pumps
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            int i, k, errcode = 0;
            char errmsg[MAXMSG + 1];

            for (i = 1; i <= net->Npumps; i++)
            {
                errcode = epanet_shared::updatepumpparams(pr, i);
                if (errcode)
                {
                    k = net->Pump[i]->Link;
                    sprintf(pr->Msg, "Error %d: %s %s",
                        errcode, geterrmsg(errcode, errmsg), net->Link[k]->ID);
                    epanet_shared::writeline(pr, pr->Msg);
                    return 200;
                }
            }
            return 0;
        };
        static int addnodeID(EN_Network net, int n, char* id)
            /*
            **-------------------------------------------------------------
            **  Input:   n = node index
            **           id = ID label
            **  Output:  returns 0 if ID already in use, 1 if not
            **  Purpose: adds a node ID to the Node Hash Table
            **--------------------------------------------------------------
            */
        {
            if (epanet_shared::findnode(net, id))
                return 215;  // duplicate id
            if (strlen(id) > MAXID)
                return 252;  // invalid format (too long)
            strncpy(net->Node[n]->ID, id, MAXID);
            hashtable_t::hashtable_insert(net->NodeHashTable, net->Node[n]->ID, n);
            return 0;
        };
        static int addlinkID(EN_Network net, int n, char* id)
            /*
            **-------------------------------------------------------------
            **  Input:   n = link index
            **           id = ID label
            **  Output:  returns 0 if ID already in use, 1 if not
            **  Purpose: adds a link ID to the Link Hash Table
            **--------------------------------------------------------------
            */
        {
            if (epanet_shared::findlink(net, id))
                return 215;  // duplicate id
            if (strlen(id) > MAXID)
                return 252; // invalid formt (too long);
            strncpy(net->Link[n]->ID, id, MAXID);
            hashtable_t::hashtable_insert(net->LinkHashTable, net->Link[n]->ID, n);
            return 0;
        };

        static int setreport(EN_Project pr, char* s)
            /*
            **-----------------------------------------------------------
            **  Input:   *s = report format command
            **  Output:  none
            **  Returns: error code
            **  Purpose: processes a report formatting command
            **           issued by the ENsetreport function
            **-----------------------------------------------------------
            */
        {
            Parser* parser = &pr->parser;
            parser->Ntokens = gettokens(s, parser->Tok, MAXTOKS, parser->Comment);
            return reportdata(pr);
        };

        static void deleterule(EN_Project pr, int index)
            //-----------------------------------------------------------
            //    Deletes a specific rule
            //-----------------------------------------------------------
        {
            EN_Network net = pr->network;

            int i;
            Srule* lastRule;

            // Free memory allocated to rule's premises & actions
            epanet_shared::clearrule(pr, index);

            // Shift position of higher indexed rules down one
            for (i = index; i <= net->Nrules - 1; i++)
            {
                net->Rule[i] = net->Rule[i + 1];
            }

            // Remove premises & actions from last (inactive) entry in Rule array
            lastRule = &net->Rule[net->Nrules];
            lastRule->Premises = NULL;
            lastRule->ThenActions = NULL;
            lastRule->ElseActions = NULL;

            // Reduce active rule count by one
            net->Nrules--;
        };
        static void adjustrules(EN_Project pr, int objtype, int index)
            //-----------------------------------------------------------
            //    Adjusts rules when a specific node or link is deleted.
            //-----------------------------------------------------------
        {
            EN_Network net = pr->network;

            int i, Delete;
            Spremise* p;
            Saction* a;

            // Delete rules that refer to objtype and index
            for (i = net->Nrules; i >= 1; i--)
            {
                Delete = FALSE;
                p = net->Rule[i].Premises;
                while (p != NULL && !Delete)
                {
                    if (objtype == p->object && p->index == index) Delete = TRUE;
                    p = p->next;
                }
                if (objtype == r_LINK)
                {
                    a = net->Rule[i].ThenActions;
                    while (a != NULL && !Delete)
                    {
                        if (a->link == index) Delete = TRUE;
                        a = a->next;
                    }
                    a = net->Rule[i].ElseActions;
                    while (a != NULL && !Delete)
                    {
                        if (a->link == index) Delete = TRUE;
                        a = a->next;
                    }
                }
                if (Delete) deleterule(pr, i);
            }

            // Adjust all higher object indices to reflect deletion of object index
            for (i = 1; i <= net->Nrules; i++)
            {
                p = net->Rule[i].Premises;
                while (p != NULL)
                {
                    if (objtype == p->object && p->index > index) p->index--;
                    p = p->next;
                }
                if (objtype == r_LINK)
                {
                    a = net->Rule[i].ThenActions;
                    while (a != NULL)
                    {
                        if (a->link > index) a->link--;
                        a = a->next;
                    }
                    a = net->Rule[i].ElseActions;
                    while (a != NULL)
                    {
                        if (a->link > index) a->link--;
                        a = a->next;
                    }
                }
            }
        };
        static void adjusttankrules(EN_Project pr)
            //-----------------------------------------------------------
            //    Adjusts tank indices in rule premises.
            //-----------------------------------------------------------
        {
            EN_Network net = pr->network;

            int i, njuncs;
            Spremise* p;

            njuncs = net->Njuncs;
            for (i = 1; i <= net->Nrules; i++)
            {
                p = net->Rule[i].Premises;
                while (p != NULL)
                {
                    if (p->object == r_NODE && p->index > njuncs) p->index++;
                    p = p->next;
                }
            }
        };
        static Spremise* getpremise(Spremise* premises, int i)
            //----------------------------------------------------------
            //    Return the i-th premise in a rule
            //----------------------------------------------------------
        {
            int count = 0;
            Spremise* p;

            p = premises;
            while (p != NULL)
            {
                count++;
                if (count == i) break;
                p = p->next;
            }
            return p;
        };
        static Saction* getaction(Saction* actions, int i)
            //----------------------------------------------------------
            //    Return the i-th action from a rule's action list
            //----------------------------------------------------------
        {
            int count = 0;
            Saction* a;

            a = actions;
            while (a != NULL)
            {
                count++;
                if (count == i) break;
                a = a->next;
            }
            return a;
        };
        static int clearreport(EN_Project pr)
            /*
            **------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: clears contents of a project's report file
            **------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;
            if (rpt->RptFile == NULL) return 0;
            if (freopen(rpt->Rpt1Fname, "w", rpt->RptFile) == NULL) return 303;
            //writelogo(pr);
            return 0;
        };
        static int copyreport(EN_Project pr, char* filename)
            /*
            **------------------------------------------------------
            **   Input:   filename = name of file to copy to
            **   Output:  returns error code
            **   Purpose: copies contents of a project's report file
            **------------------------------------------------------
            */
        {
            FILE* tfile;
            int c;
            Report* rpt = &pr->report;

            // Check that project's report file exists
            if (rpt->RptFile == NULL) return 0;

            // Open the new destination file
            tfile = fopen(filename, "w");
            if (tfile == NULL) return 303;

            // Re-open project's report file in read mode
            fclose(rpt->RptFile);
            rpt->RptFile = fopen(rpt->Rpt1Fname, "r");

            // Copy contents of project's report file
            if (rpt->RptFile)
            {
                while ((c = fgetc(rpt->RptFile)) != EOF) fputc(c, tfile);
                fclose(rpt->RptFile);
            }

            // Close destination file
            fclose(tfile);

            // Re-open project's report file in append mode
            rpt->RptFile = fopen(rpt->Rpt1Fname, "a");
            if (rpt->RptFile == NULL) return 303;
            return 0;
        };
        static void writelimits(EN_Project pr, int j1, int j2)
            /*
            **--------------------------------------------------------------
            **   Input:   j1 = index of first output variable
            **            j2 = index of last output variable
            **   Output:  none
            **   Purpose: writes reporting criteria to output report
            **--------------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;
            int j;

            for (j = j1; j <= j2; j++)
            {
                if (rpt->Field[j].RptLim[LOW] < BIG)
                {
                    sprintf(pr->Msg, FMT47, rpt->Field[j].Name,
                        rpt->Field[j].RptLim[LOW],
                        rpt->Field[j].Units);
                    epanet_shared::writeline(pr, pr->Msg);
                }
                if (rpt->Field[j].RptLim[HI] > -BIG)
                {
                    sprintf(pr->Msg, FMT48, rpt->Field[j].Name,
                        rpt->Field[j].RptLim[HI],
                        rpt->Field[j].Units);
                    epanet_shared::writeline(pr, pr->Msg);
                }
            }
        };
        static int checklimits(Report* rpt, SCALER* y, int j1, int j2)
            /*
            **--------------------------------------------------------------
            **   Input:   *y = array of output results
            **            j1 = index of first output variable
            **            j2 = index of last output variable
            **   Output:  returns 1 if criteria met, 0 otherwise
            **   Purpose: checks if output reporting criteria is met
            **--------------------------------------------------------------
            */
        {
            int j;
            for (j = j1; j <= j2; j++)
            {
                if (y[j] > rpt->Field[j].RptLim[LOW] ||
                    y[j] < rpt->Field[j].RptLim[HI]
                    ) return 0;
            }
            return 1;
        };
        static void writetime(EN_Project pr, char* fmt)
            /*
            **----------------------------------------------------------------
            **   Input:   fmt = format string
            **   Output:  none
            **   Purpose: writes starting/ending time of a run to report file
            **----------------------------------------------------------------
            */
        {
            time_t timer;
            ::time(&timer);
            sprintf(pr->Msg, fmt, ctime(&timer));
            epanet_shared::writeline(pr, pr->Msg);
        };
        static void writeheader(EN_Project pr, int type, int contin)
            /*
            **--------------------------------------------------------------
            **   Input:   type   = table type
            **            contin = table continuation flag
            **   Output:  none
            **   Purpose: writes column headings for output report tables
            **--------------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;
            Quality* qual = &pr->quality;
            Parser* parser = &pr->parser;
            Times* time = &pr->times;

            char s[MAXLINE + 1], s1[MAXLINE + 1], s2[MAXLINE + 1], s3[MAXLINE + 1];
            int i, n;

            // Move to next page if < 11 lines remain on current page
            if (rpt->Rptflag && rpt->LineNum + 11 > (long)rpt->PageSize)
            {
                while (rpt->LineNum < (long)rpt->PageSize)  epanet_shared::writeline(pr, (char*)" ");
            }
            epanet_shared::writeline(pr, (char*)" ");

            // Hydraulic Status Table
            if (type == STATHDR)
            {
                sprintf(s, FMT49);
                if (contin) strcat(s, t_CONTINUED);
                epanet_shared::writeline(pr, s);
                fillstr(s, '-', 70);
                epanet_shared::writeline(pr, s);
            }

            // Energy Usage Table
            if (type == ENERHDR)
            {
                if (parser->Unitsflag == SI) strcpy(s1, t_perM3);
                else                         strcpy(s1, t_perMGAL);
                sprintf(s, FMT71);
                if (contin) strcat(s, t_CONTINUED);
                epanet_shared::writeline(pr, s);
                fillstr(s, '-', 63);
                epanet_shared::writeline(pr, s);
                sprintf(s, FMT72);
                epanet_shared::writeline(pr, s);
                sprintf(s, FMT73, s1);
                epanet_shared::writeline(pr, s);
                fillstr(s, '-', 63);
                epanet_shared::writeline(pr, s);
            }

            // Node Results Table
            if (type == NODEHDR)
            {
                if (rpt->Tstatflag == RANGE) sprintf(s, FMT76, t_DIFFER);
                else if (rpt->Tstatflag != SERIES)
                {
                    sprintf(s, FMT76, TstatTxt[rpt->Tstatflag]);
                }
                else if (time->Dur == 0_s) sprintf(s, FMT77);
                else sprintf(s, FMT78, clocktime(rpt->Atime, time->Htime));
                if (contin) strcat(s, t_CONTINUED);
                epanet_shared::writeline(pr, s);

                n = 15;
                sprintf(s2, "%15s", "");
                strcpy(s, t_NODEID);
                sprintf(s3, "%-15s", s);

                for (i = ELEV; i < QUALITY; i++)
                {
                    if (rpt->Field[i].Enabled == TRUE)
                    {
                        n += 10;
                        sprintf(s, "%10s", rpt->Field[i].Name);
                        strcat(s2, s);
                        sprintf(s, "%10s", rpt->Field[i].Units);
                        strcat(s3, s);
                    }
                }

                if (rpt->Field[QUALITY].Enabled == TRUE)
                {
                    n += 10;
                    sprintf(s, "%10s", qual->ChemName);
                    strcat(s2, s);
                    sprintf(s, "%10s", qual->ChemUnits);
                    strcat(s3, s);
                }
                fillstr(s1, '-', n);
                epanet_shared::writeline(pr, s1);
                epanet_shared::writeline(pr, s2);
                epanet_shared::writeline(pr, s3);
                epanet_shared::writeline(pr, s1);
            }

            // Link Results Table
            if (type == LINKHDR)
            {
                if (rpt->Tstatflag == RANGE) sprintf(s, FMT79, t_DIFFER);
                else if (rpt->Tstatflag != SERIES)
                {
                    sprintf(s, FMT79, TstatTxt[rpt->Tstatflag]);
                }
                else if (time->Dur == 0_s) sprintf(s, FMT80);
                else  sprintf(s, FMT81, clocktime(rpt->Atime, time->Htime));
                if (contin) strcat(s, t_CONTINUED);
                epanet_shared::writeline(pr, s);

                n = 15;
                sprintf(s2, "%15s", "");
                strcpy(s, t_LINKID);
                sprintf(s3, "%-15s", s);
                for (i = LENGTH; i <= FRICTION; i++)
                {
                    if (rpt->Field[i].Enabled == TRUE)
                    {
                        n += 10;
                        sprintf(s, "%10s", rpt->Field[i].Name);
                        strcat(s2, s);
                        sprintf(s, "%10s", rpt->Field[i].Units);
                        strcat(s3, s);
                    }
                }
                fillstr(s1, '-', n);
                epanet_shared::writeline(pr, s1);
                epanet_shared::writeline(pr, s2);
                epanet_shared::writeline(pr, s3);
                epanet_shared::writeline(pr, s1);
            }
        };
        static void writeenergy(EN_Project pr)
            /*
            **-------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: writes energy usage report to report file
            **-------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Report* rpt = &pr->report;

            int j;
            SCALER csum;
            char s[MAXLINE + 1];
            Ppump pump;

            if (net->Npumps == 0) return;
            epanet_shared::writeline(pr, (char*)" ");
            writeheader(pr, ENERHDR, 0);

            csum = 0.0;
            for (j = 1; j <= net->Npumps; j++)
            {
                pump = net->Pump[j];
                csum += pump->Energy.TotalCost;
                if (rpt->LineNum == (long)rpt->PageSize) writeheader(pr, ENERHDR, 1);

                sprintf(s, "%-8s  %6.2f %6.2f %9.2f %9.2f %9.2f %9.2f",
                    net->Link[pump->Link]->ID,
                    (double)pump->Energy.TimeOnLine,
                    (double)pump->Energy.Efficiency,
                    (double)pump->Energy.KwHrsPerFlow,
                    (double)pump->Energy.KwHrs,
                    (double)pump->Energy.MaxKwatts,
                    (double)pump->Energy.TotalCost);
                epanet_shared::writeline(pr, s);
            }

            fillstr(s, '-', 63);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT74, "", hyd->Emax * hyd->Dcost);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT75, "", csum + (double)hyd->Emax * hyd->Dcost);
            epanet_shared::writeline(pr, s);
            epanet_shared::writeline(pr, (char*)" ");
        };
        static void writesummary(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  none
            **   Purpose: writes summary system information to report file
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Hydraul* hyd = &pr->hydraul;
            Quality* qual = &pr->quality;
            Report* rpt = &pr->report;
            Parser* parser = &pr->parser;
            Times* time = &pr->times;

            char s[MAXLINE + 1];
            int i;
            int nres = 0;

            for (i = 0; i < 3; i++)
            {
                if (strlen(pr->Title[i]) > 0)
                {
                    sprintf(s, "%-.70s", pr->Title[i]);
                    epanet_shared::writeline(pr, s);
                }
            }
            epanet_shared::writeline(pr, (char*)" ");
            sprintf(s, FMT19, parser->InpFname);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT20, net->Njuncs);
            epanet_shared::writeline(pr, s);
            for (i = 1; i <= net->Ntanks; i++) if (net->Tank[i]->A == 0.0_sq_ft) nres++;
            sprintf(s, FMT21a, nres);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT21b, net->Ntanks - nres);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT22, net->Npipes);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT23, net->Npumps);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT24, net->Nvalves);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT25, RptFormTxt[hyd->Formflag]);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT25a, DemandModelTxt[hyd->DemandModel]);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT26, time->Hstep * pr->Ucf[TIME], rpt->Field[TIME].Units);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT27, hyd->Hacc);
            epanet_shared::writeline(pr, s);

            if (hyd->HeadErrorLimit > 0.0_ft)
            {
                sprintf(s, FMT27d, hyd->HeadErrorLimit * pr->Ucf[HEAD], rpt->Field[HEAD].Units);
                epanet_shared::writeline(pr, s);
            }
            if (hyd->FlowChangeLimit > 0.0_cfs)
            {
                sprintf(s, FMT27e, hyd->FlowChangeLimit * pr->Ucf[FLOW], rpt->Field[FLOW].Units);
                epanet_shared::writeline(pr, s);
            }

            sprintf(s, FMT27a, hyd->CheckFreq);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT27b, hyd->MaxCheck);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT27c, hyd->DampLimit);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT28, hyd->MaxIter);
            epanet_shared::writeline(pr, s);

            if (qual->Qualflag == NONE || time->Dur == 0.0_s) sprintf(s, FMT29);
            else if (qual->Qualflag == CHEM)  sprintf(s, FMT30, qual->ChemName);
            else if (qual->Qualflag == ENERGYINTENSITY)  sprintf(s, FMT30, qual->ChemName);
            else if (qual->Qualflag == TRACE_QUAL) sprintf(s, FMT31, qual->TraceNode.c_str());
            else if (qual->Qualflag == AGE)   printf(s, FMT32);
            epanet_shared::writeline(pr, s);
            if (qual->Qualflag != NONE && time->Dur > 0_s)
            {
                sprintf(s, FMT33, (float)time->Qstep / 60.0);
                epanet_shared::writeline(pr, s);
                sprintf(s, FMT34, qual->Ctol * pr->Ucf[QUALITY], rpt->Field[QUALITY].Units);
                epanet_shared::writeline(pr, s);
            }

            sprintf(s, FMT36, hyd->SpGrav);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT37a, hyd->Viscos / VISCOS);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT37b, qual->Diffus / DIFFUS);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT38, hyd->Dmult);
            epanet_shared::writeline(pr, s);
            sprintf(s, FMT39, time->Dur * pr->Ucf[TIME], rpt->Field[TIME].Units);
            epanet_shared::writeline(pr, s);

            if (rpt->Rptflag)
            {
                sprintf(s, FMT40);
                epanet_shared::writeline(pr, s);
                if (rpt->Nodeflag == 0)   epanet_shared::writeline(pr, (char*)FMT41.c_str());
                if (rpt->Nodeflag == 1)   epanet_shared::writeline(pr, (char*)FMT42.c_str());
                if (rpt->Nodeflag == 2)   epanet_shared::writeline(pr, (char*)FMT43.c_str());
                writelimits(pr, DEMAND, QUALITY);
                if (rpt->Linkflag == 0)   epanet_shared::writeline(pr, (char*)FMT44.c_str());
                if (rpt->Linkflag == 1)   epanet_shared::writeline(pr, (char*)FMT45.c_str());
                if (rpt->Linkflag == 2)   epanet_shared::writeline(pr, (char*)FMT46.c_str());
                writelimits(pr, DIAM, HEADLOSS);
            }
            epanet_shared::writeline(pr, (char*)" ");
        };
        static void writenodetable(EN_Project pr, Pfloat* x)
            /*
            **---------------------------------------------------------------
            **   Input:   x = pointer to node results for current time
            **   Output:  none
            **   Purpose: writes node results for current time to report file
            **---------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Report* rpt = &pr->report;

            int i, j;
            char s[MAXLINE + 1], s1[16];
            SCALER y[MAXVAR];
            Pnode node;

            // Write table header
            writeheader(pr, NODEHDR, 0);

            // For each node:
            for (i = 1; i <= net->Nnodes; i++)
            {
                // Place node's results for each variable in y
                node = net->Node[i];
                y[ELEV] = (double)(node->El * pr->Ucf[ELEV]);
                for (j = DEMAND; j <= QUALITY; j++) y[j] = *((x[j - DEMAND]) + i);

                // Check if node gets reported on
                if ((rpt->Nodeflag == 1 || node->Rpt) &&
                    checklimits(rpt, y, ELEV, QUALITY))
                {
                    // Check if new page needed
                    if (rpt->LineNum == (long)rpt->PageSize) writeheader(pr, NODEHDR, 1);

                    // Add node ID and each reported field to string s
                    sprintf(s, "%-15s", node->ID);
                    for (j = ELEV; j <= QUALITY; j++)
                    {
                        if (rpt->Field[j].Enabled == TRUE)
                        {
                            if (ABS(y[j]) > 1.e6) sprintf(s1, "%10.2e", (double)y[j]);
                            else sprintf(s1, "%10.*f", rpt->Field[j].Precision, (double)y[j]);
                            strcat(s, s1);
                        }
                    }

                    // Note if node is a reservoir/tank
                    if (i > net->Njuncs)
                    {
                        strcat(s, "  ");
                        strcat(s, NodeTxt[epanet_shared::getnodetype(net, i)]);
                    }

                    // Write results for node to report file
                    epanet_shared::writeline(pr, s);
                }
            }
            epanet_shared::writeline(pr, (char*)" ");
        };
        static void writelinktable(EN_Project pr, Pfloat* x)
            /*
            **---------------------------------------------------------------
            **   Input:   x = pointer to link results for current time
            **   Output:  none
            **   Purpose: writes link results for current time to report file
            **---------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Report* rpt = &pr->report;

            int i, j, k;
            char s[MAXLINE + 1], s1[16];
            SCALER y[MAXVAR];
            SCALER* Ucf = pr->Ucf;
            auto& Link = net->Link;

            // Write table header
            writeheader(pr, LINKHDR, 0);

            // For each link:
            for (i = 1; i <= net->Nlinks; i++)
            {
                // Place results for each link variable in y
                y[LENGTH] = (double)(Link[i]->Len * Ucf[LENGTH]);
                y[DIAM] = (double)(Link[i]->Diam * Ucf[DIAM]);
                for (j = FLOW; j <= FRICTION; j++) y[j] = *((x[j - FLOW]) + i);

                // Check if link gets reported on
                if ((rpt->Linkflag == 1 || Link[i]->Rpt) && checklimits(rpt, y, DIAM, FRICTION))
                {
                    // Check if new page needed
                    if (rpt->LineNum == (long)rpt->PageSize) writeheader(pr, LINKHDR, 1);

                    // Add link ID and each reported field to string s
                    sprintf(s, "%-15s", Link[i]->ID);
                    for (j = LENGTH; j <= FRICTION; j++)
                    {
                        if (rpt->Field[j].Enabled == TRUE)
                        {
                            if (j == STATUS)
                            {
                                if (y[j] <= (SCALER)CLOSED) k = CLOSED;
                                else if (y[j] == (SCALER)ACTIVE) k = ACTIVE;
                                else                     k = OPEN;
                                sprintf(s1, "%10s", StatTxt[k]);
                            }
                            else
                            {
                                if (ABS(y[j]) > 1.e6) sprintf(s1, "%10.2e", (double)y[j]);
                                else sprintf(s1, "%10.*f", rpt->Field[j].Precision, (double)y[j]);
                            }
                            strcat(s, s1);
                        }
                    }

                    // Note if link is a pump or valve
                    if ((j = Link[i]->Type) > PIPE)
                    {
                        strcat(s, "  ");
                        strcat(s, LinkTxt[j]);
                    }

                    // Write results for link
                    epanet_shared::writeline(pr, s);
                }
            }
            epanet_shared::writeline(pr, (char*)" ");
        };
        static int writeresults(EN_Project pr)
            /*
            **--------------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: writes simulation results to report file
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Outfile* out = &pr->outfile;
            Report* rpt = &pr->report;
            Times* time = &pr->times;

            int j, m, n,
                np,           // Reporting period counter
                nnv,          // # node variables reported on
                nlv;          // # link variables reported on
            int errcode = 0;
            Pfloat* x;        // Array of pointers to floats (i.e., a 2-D array)
            FILE* outFile = out->OutFile;

            //-----------------------------------------------------------
            //  NOTE:  The OutFile contains results for 4 node variables
            //         (demand, head, pressure, & quality) and 8 link
            //         variables (flow, velocity, headloss, quality,
            //         status, setting, reaction rate & friction factor)
            //         at each reporting time.
            //-----------------------------------------------------------

                // Return if no nodes or links selected for reporting
                // or if no node or link report variables enabled
            if (!rpt->Nodeflag && !rpt->Linkflag)  return errcode;

            nnv = 0;
            for (j = ELEV; j <= QUALITY; j++) nnv += rpt->Field[j].Enabled;
            nlv = 0;
            for (j = LENGTH; j <= FRICTION; j++) nlv += rpt->Field[j].Enabled;
            if (nnv == 0 && nlv == 0) return errcode;

            // Return if no output file
            if (outFile == NULL) outFile = fopen(pr->outfile.OutFname, "rb");
            if (outFile == NULL) return 106;

            // Allocate memory for output variables:
            // m = larger of # node variables & # link variables
            // n = larger of # nodes & # links
            m = fMAX((QUALITY - DEMAND + 1), (FRICTION - FLOW + 1));
            n = fMAX((net->Nnodes + 1), (net->Nlinks + 1));
            x = (Pfloat*)calloc(m, sizeof(Pfloat));
            ERRCODE(MEMCHECK(x));
            if (errcode) return errcode;
            for (j = 0; j < m; j++)
            {
                x[j] = (REAL4*)calloc(n, sizeof(REAL4));
                if (x[j] == NULL) errcode = 101;
            }
            if (!errcode)
            {
                // Re-position output file & initialize report time
                fseek(outFile, out->OutOffset2, SEEK_SET);
                time->Htime = time->Rstart;

                // For each reporting time:
                for (np = 1; np <= rpt->Nperiods; np++)
                {
                    // Read in node results & write node table
                    // (Remember to offset x[j] by 1 because array is zero-based)
                    for (j = DEMAND; j <= QUALITY; j++)
                    {
                        fread((x[j - DEMAND]) + 1, sizeof(REAL4), net->Nnodes, outFile);
                    }
                    if (nnv > 0 && rpt->Nodeflag > 0) writenodetable(pr, x);

                    // Read in link results & write link table
                    for (j = FLOW; j <= FRICTION; j++)
                    {
                        fread((x[j - FLOW]) + 1, sizeof(REAL4), net->Nlinks, outFile);
                    }
                    if (nlv > 0 && rpt->Linkflag > 0) writelinktable(pr, x);
                    time->Htime += time->Rstep;
                }
            }

            // Free output file
            if (outFile != NULL)
            {
                fclose(outFile);
                outFile = NULL;
            }

            // Free allocated memory
            for (j = 0; j < m; j++) free(x[j]);
            free(x);
            return errcode;
        };
        static int writereport(EN_Project pr)
            /*
            **------------------------------------------------------
            **   Input:   none
            **   Output:  returns error code
            **   Purpose: writes formatted output report to file
            **------------------------------------------------------
            */
        {
            Report* rpt = &pr->report;
            Parser* parser = &pr->parser;

            int tflag;
            FILE* tfile;
            int errcode = 0;

            // If no secondary report file specified then
            // write formatted output to primary report file
            rpt->Fprinterr = FALSE;
            if (rpt->Rptflag && strlen(rpt->Rpt2Fname) == 0 && rpt->RptFile != NULL)
            {
                if (rpt->Energyflag) writeenergy(pr);
                errcode = writeresults(pr);
            }

            // A secondary report file was specified
            else if (strlen(rpt->Rpt2Fname) > 0)
            {
                // If secondary report file has same name as either input
                // or primary report file then use primary report file.
                if (strcomp(rpt->Rpt2Fname, parser->InpFname) ||
                    strcomp(rpt->Rpt2Fname, rpt->Rpt1Fname))
                {
                    if (rpt->Energyflag) writeenergy(pr);
                    errcode = writeresults(pr);
                }

                // Otherwise write report to secondary report file
                else
                {
                    // Try to open file
                    tfile = rpt->RptFile;
                    tflag = rpt->Rptflag;
                    if ((rpt->RptFile = fopen(rpt->Rpt2Fname, "wt")) == NULL)
                    {
                        rpt->RptFile = tfile;
                        rpt->Rptflag = tflag;
                        errcode = 303;
                    }

                    // Write full formatted report to file
                    else
                    {
                        rpt->Rptflag = 1;
                        //writelogo(pr);
                        if (rpt->Summaryflag) writesummary(pr);
                        if (rpt->Energyflag)  writeenergy(pr);
                        errcode = writeresults(pr);
                        fclose(rpt->RptFile);
                        rpt->RptFile = tfile;
                        rpt->Rptflag = tflag;
                    }
                }
            }

            // Special error handler for write-to-file error
            if (rpt->Fprinterr)  epanet_shared::errmsg(pr, 309);
            return errcode;
        };
    };
};