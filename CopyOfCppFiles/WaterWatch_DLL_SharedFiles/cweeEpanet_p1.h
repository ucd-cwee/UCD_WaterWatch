#pragma once
#include "Precompiled.h"

#if 0
namespace cweeAsset {
	using namespace cwee_units;

    // Input File Parser Wrapper
    class Parser {
    public:
        FILE* InFile = nullptr;

        cweeStr 
            DefPatID
            , InpFname
            , Comment
            , LineComment;

        int
            ErrTok,                // Index of error-producing token
            Unitsflag,// = ::epanet::UnitsType::US,             // Unit system flag
            Flowflag,// = ::epanet::FlowUnitsType::GPM,              // Flow units flag
            Pressflag; // = ::epanet::PressureUnitsType::PSI,             // Pressure units flag

        cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>>  DefPat;
        cweeList<cweeStr>   Tokens;
        cweeList<cweeList<cweeSharedPtr<::cweeAsset::RuleCondition>>>   sharedRuleCondition;
        cweeList< cweeUnion< cweeSharedPtr<::cweeAsset::cweeLink>, value_t, cweeSharedPtr<::cweeAsset::cweeControl_Logic> > >  sharedRules;
        ::epanet::Rulewords                                             sharedRuleState;
        int                                                             sharedRuleErrCode;
        cweeStr                                                         sharedRuleName;
    };

    class Times {
    public:
        Times() {
            StartOfCurrentDay = cweeTime::Now().ToStartOfDay();
        };

        cweeTime StartOfCurrentDay;
        cweeTime GetProjectStartTime() const {
            return StartOfCurrentDay + (u64)Tstart;
        };
        cweeTime GetPatternStartTime() const {
            return GetProjectStartTime() - (u64)((u64)Pstart * (u64)Pstep);
        };
        units::time::second_t
            Tstart,                // Starting time of day // seconds or minutes????
            Hstep,                 // Nominal hyd. time step
            Pstep,                 // Time pattern time step            
            Rstep,                 // Reporting time step
            Rstart,                // Time when reporting starts
            Rtime,                 // Next reporting time            
            Hydstep,               // Actual hydraulic time step
            Qstep,                 // Quality time step
            Qtime,                 // Current quality time
            Rulestep,              // Rule evaluation time step
            Dur;                   // Duration of simulation

        cweeTime Htime;            // Current hyd. time

        int 
            Pstart;                // Starting pattern time
    };
    
    class Hydraul {
    public:
        Hydraul() :
            smatrix(new Smatrix()),
            NodeHead(std::function<size_t(cweeUnion<cweeSharedPtr<cweeNode>, foot_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeNode>, foot_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            NodeDemand(std::function<size_t(cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            DemandFlow(std::function<size_t(cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            EmitterFlow(std::function<size_t(cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            LinkFlow(std::function<size_t(cweeUnion<cweeSharedPtr<cweeLink>, cubic_foot_per_second_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeLink>, cubic_foot_per_second_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            LinkSetting(std::function<size_t(cweeUnion<cweeSharedPtr<cweeLink>, scalar_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeLink>, scalar_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            P(std::function<size_t(cweeUnion<cweeSharedPtr<cweeLink>, cfs_p_ft_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeLink>, cfs_p_ft_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            Y(std::function<size_t(cweeUnion<cweeSharedPtr<cweeLink>, cubic_foot_per_second_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeLink>, cubic_foot_per_second_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            Xflow(std::function<size_t(cweeUnion<cweeSharedPtr<cweeAsset>, cubic_foot_per_second_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeAsset>, cubic_foot_per_second_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            LinkStatus(std::function<size_t(cweeUnion<cweeSharedPtr<cweeAsset>, scalar_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeAsset>, scalar_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; })),
            OldStatus(std::function<size_t(cweeUnion<cweeSharedPtr<cweeAsset>, scalar_t> const&)>([](const cweeUnion<cweeSharedPtr<cweeAsset>, scalar_t>& v)->size_t { if (v.get<0>()) { return v.get<0>()->Hash(); } return 0; }))
        {};

        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeNode>, foot_t>, size_t> NodeHead;
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t>, size_t> NodeDemand;
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t>, size_t> DemandFlow;
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeNode>, cubic_foot_per_second_t>, size_t> EmitterFlow;
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeLink>, cubic_foot_per_second_t>, size_t> LinkFlow;
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeLink>, scalar_t>, size_t> LinkSetting;
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeLink>, cfs_p_ft_t>, size_t> P; // Inverse of head loss derivatives
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeLink>, cubic_foot_per_second_t>, size_t> Y; // Flow correction factors
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeAsset>, cubic_foot_per_second_t>, size_t> Xflow;  // Inflow - outflow at each node
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeAsset>, scalar_t>, size_t> LinkStatus;  // Link status
        cweeThreadedSet<cweeUnion<cweeSharedPtr<cweeAsset>, scalar_t>, size_t> OldStatus;  // Previous link/tank status

        scalar_t
            Dmult;                 // Demand multiplier

        cubic_foot_per_second_t
            Dsystem;               // Total system demand

        typedef units::compound_unit<units::area::square_feet, units::inverse<units::time::second>> kin_viscocity;
        typedef units::unit_t<kin_viscocity> kin_viscocity_t;
        kin_viscocity_t Viscos; // Kin. viscosity (sq ft/sec)

        pounds_per_square_inch_t
            Pmin,                  // Pressure needed for any demand
            Preq;                  // Pressure needed for full demand           

        scalar_t
            RelativeError;         // Total flow change / total flow

        cubic_foot_per_second_t
            Hacc,                  // Relative flow change limit
            FlowChangeLimit,       // Absolute flow change limit
            Qtol,                  // Flow rate tolerance            
            MaxFlowChange;         // Max. change in link flow

        foot_t
            Htol;                  // Hydraulic head tolerance

        foot_t
            HeadErrorLimit;        // Hydraulic head error limit
        
        scalar_t 
            DemandReduction;       // % demand reduction at pressure deficient nodes

        foot_t
            MaxHeadError;          // Max. error for link head loss  

        double
            RQtol,                 // Flow resistance tolerance
            Hexp,                  // Exponent in headloss formula
            Qexp,                  // Exponent in emitter formula
            Pexp,                  // Exponent in demand formula
            DampLimit,             // Solution damping threshold             
            SpGrav,                // Specific gravity
            Epump,                 // Global pump efficiency            
            Ecost,                 // Base energy cost per kwh
            Dcost,                 // Energy demand charge/kw/day
            Emax,                  // Peak energy usage    
            RelaxFactor;           // Relaxation factor for flow updating

        int
            Epat,                  // Energy cost time pattern
            DemandModel,           // Fixed or pressure dependent
            Formflag,              // Head loss formula flag
            Iterations,            // Number of hydraulic trials taken
            MaxIter,               // Max. hydraulic trials allowed
            ExtraIter,             // Extra hydraulic trials
            CheckFreq,             // Hydraulic trials between status checks
            MaxCheck,              // Hydraulic trials limit on status checks
            OpenHflag,             // Hydraulic system opened flag
            Haltflag,              // Flag to halt simulation
            DeficientNodes;        // Number of pressure deficient nodes

        ::epanet::HeadLossType  headLossType = ::epanet::HW;
        ::epanet::UnitsType     unitsType = ::epanet::US;
        ::epanet::FlowUnitsType  flowUnitsType = ::epanet::GPM;
        ::epanet::PressureUnitsType pressureUnitsType = ::epanet::PSI;

        cweeSharedPtr<Smatrix> smatrix;         // Sparse matrix storage
    };



    class Project {
    public:
        Project() : 
            parser(new Parser())
            , network(new cweeHydraulicNetwork())
            , hydraul(new Hydraul())
            , times(new Times())
        {};

        cweeSharedPtr<Parser> parser;
        cweeSharedPtr<cweeHydraulicNetwork> network;
        cweeSharedPtr<Hydraul> hydraul;
        cweeSharedPtr<Times> times;

        cweeList<cweeStr> Title; // Project title
        cweeStr MapFname;        // Map file name
    };







    namespace epanet {
        namespace details {
            static cweeStr  errmsg(int errcode)
                /*----------------------------------------------------------------
                **  Input:   errcode = error code
                **  Output:  none
                **  Returns: pointer to string with error message
                **  Purpose: retrieves text of error message
                **----------------------------------------------------------------
                */
            {
                cweeStr Message = "                                                                                                                                                                                               ";
                char* msg = (char*)Message.c_str();

                switch (errcode) {
#define DAT(code,string) case code: ::strcpy(msg, string); break;
#include "EPANET_errors.dat"
#undef DAT
                default:
                    ::strcpy(msg, "");
                }
                return cweeStr(msg);
            };
            static void     handleErr(int errcode) { 
                if (errcode != 0) {
                    std::cout << errmsg(errcode) << std::endl;
                }                
            };
#pragma region Initialize
            static void     gettokens(cweeStr const& str, cweeThreadedList<cweeStr>& Tok, cweeStr& comment)
                /*
                 **--------------------------------------------------------------
                 **  Input:   *s = string to be tokenized
                 **  Output:  returns number of tokens in s
                 **  Purpose: scans string for tokens, saving pointers to them
                 **           in module global variable Tok[]
                 **
                 ** Tokens can be separated by the characters listed in SEPSTR
                 ** (spaces, tabs, newline, carriage return) which is defined
                 ** in TYPES.H. Text between quotes is treated as a single token.
                 **--------------------------------------------------------------
                 */
            {
                int  n;
                size_t len, m;
                char* c, * c2;

                // clear
                comment.Clear();
                Tok.Clear(); 
                n = 0;

                // Truncate s at start of comment
                cweeParser split1; split1.ParseFirstDelimiterOnly(str, ";");
                if (split1.getNumVars() > 1) {
                    comment = split1[1];
                }
                AUTO s = (char*)split1[0].c_str();

                len = split1[0].Length();

                // Scan s for tokens until nothing left
                while (len > 0 && n < ::epanet::MAXTOKS)
                {
                    m = (int)strcspn(s, ::epanet::SEPSTR);     // Find token length
                    if (m == len)                   // s is last token
                    {
                        Tok.Append(s);
                        n++;
                        break;
                    }
                    len -= m + 1;                     // Update length of s
                    if (m == 0) s++;                // No token found
                    else
                    {
                        if (*s == '"')              // Token begins with quote
                        {
                            s++;                           // Start token after quote
                            m = (int)strcspn(s, "\"\n\r");  // Find end quote (or EOL)
                        }
                        s[m] = '\0';                 // Null-terminate the token
                        Tok.Append(s);                  // Save pointer to token
                        n++;                         // Update token count
                        s += m + 1;                    // Begin next token
                    }
                }
            };
            static void     setdefaults(cweeSharedPtr<Project> pr) {
                AUTO parser = pr->parser;                
                AUTO hyd = pr->hydraul;
                //Quality* qual = &pr->quality;
                AUTO time = pr->times;                

                //strncpy(qual->ChemName, t_CHEMICAL, MAXID);
                //strncpy(qual->ChemUnits, u_MGperL, MAXID);

                parser->Unitsflag = ::epanet::US;     // US unit system
                parser->Flowflag = ::epanet::GPM;     // Flow units are gpm
                parser->Pressflag = ::epanet::PSI;    // Pressure units are psi
                parser->DefPat = nullptr;         // Default demand pattern

                hyd->unitsType = ::epanet::US;
                hyd->flowUnitsType = ::epanet::GPM;
                hyd->pressureUnitsType = ::epanet::PSI;
                hyd->Formflag = ::epanet::HW;         // Use Hazen-Williams formula
                hyd->Htol = ::epanet::HTOL;           // Default head tolerance
                hyd->Qtol = ::epanet::QTOL;           // Default flow tolerance
                hyd->Hacc = ::epanet::HACC;           // Default hydraulic accuracy
                hyd->FlowChangeLimit = 0.0; // Default flow change limit
                hyd->HeadErrorLimit = 0.0;  // Default head error limit
                hyd->DemandModel = ::epanet::DDA;     // Demand driven analysis
                hyd->Pmin = 0.0;            // Minimum demand pressure (ft)
                hyd->Preq = ::epanet::MINPDIFF;       // Required demand pressure (ft)
                hyd->Pexp = 0.5;            // Pressure function exponent
                hyd->MaxIter = ::epanet::MAXITER;     // Default max. hydraulic trials
                hyd->ExtraIter = -1;        // Stop if network unbalanced
                hyd->Viscos = ::epanet::MISSING;      // Temporary viscosity
                hyd->SpGrav = ::epanet::SPGRAV;       // Default specific gravity
                hyd->Epat = 0;              // No energy price pattern
                hyd->Ecost = 0.0;           // Zero unit energy cost
                hyd->Dcost = 0.0;           // Zero energy demand charge
                hyd->Epump = ::epanet::EPUMP;         // Default pump efficiency
                hyd->Emax = 0.0;            // Zero peak energy usage
                hyd->Qexp = 2.0;            // Flow exponent for emitters
                hyd->Dmult = 1.0;           // Demand multiplier
                hyd->RQtol = ::epanet::RQTOL;         // Default hydraulics parameters
                hyd->CheckFreq = ::epanet::CHECKFREQ;
                hyd->MaxCheck = ::epanet::MAXCHECK;
                hyd->DampLimit = ::epanet::DAMPLIMIT;

                //qual->Qualflag = ::epanet::NONE;      // No quality simulation
                //qual->Ctol = ::epanet::MISSING;       // No pre-set quality tolerance
                //qual->TraceNode = 0;        // No source tracing
                //qual->BulkOrder = 1.0;      // 1st-order bulk reaction rate
                //qual->WallOrder = 1.0;      // 1st-order wall reaction rate
                //qual->TankOrder = 1.0;      // 1st-order tank reaction rate
                //qual->Kbulk = 0.0;          // No global bulk reaction
                //qual->Kwall = 0.0;          // No global wall reaction
                //qual->Climit = 0.0;         // No limiting potential quality
                //qual->Diffus = MISSING;     // Temporary diffusivity
                //qual->Rfactor = 0.0;        // No roughness-reaction factor
                //qual->MassBalance.ratio = 0.0;

                time->Dur = 0;              // 0 sec duration (steady state)
                time->Tstart = 0;           // Starting time of day
                time->Pstart = 0;           // Starting pattern period
                time->Hstep = 3600;         // 1 hr hydraulic time step
                time->Qstep = 0;            // No pre-set quality time step
                time->Pstep = 3600;         // 1 hr time pattern period
                time->Rstep = 3600;         // 1 hr reporting period
                time->Rulestep = 0;         // No pre-set rule time step
                time->Rstart = 0;           // Start reporting at time 0
            };
            static void     inperrmsg(cweeSharedPtr<Project> pr, int err, int sect, cweeStr const& line) {
                AUTO parser = pr->parser;
                std::cout << cweeStr::printf("Error %i: %s %s in %s section: \n%s\n", err, errmsg(err).c_str(), parser->Tokens[parser->ErrTok].c_str(), ::epanet::SectTxt[sect], line.c_str());
            };            
            class ParseLine {
            public:
                static int setError(cweeSharedPtr<Parser> parser, int tokindex, int errcode)
                {
                    parser->ErrTok = tokindex;
                    return errcode;
                };

                template<typename desiredUnit> static desiredUnit convertToUnit(cweeSharedPtr<Project> pr, double v, bool pipe_diameter = false) {                                      
                    if constexpr (units::traits::is_length_unit< desiredUnit >()) {
                        if (pipe_diameter) { // dealing with a pipe diameter... 
                            if (pr->hydraul->unitsType == ::epanet::US) {
                                return (inch_t)v;
                            }
                            else { // SI units
                                return (millimeter_t)v;
                            }
                        }
                        else {
                            if (pr->hydraul->unitsType == ::epanet::US) {
                                return (foot_t)v;
                            }
                            else{ // SI units
                                return (meter_t)v;
                            }
                        }
                    }
                    else if constexpr (units::traits::is_energy_cost_rate_unit< desiredUnit >()) {
                        return (Dollar_per_kilowatt_hour_t)v;
                    }
                    else if constexpr (units::traits::is_power_unit< desiredUnit >()) {
                        if (pr->hydraul->unitsType == ::epanet::US) {
                            return (horsepower_t)v;
                        }
                        else { // SI units
                            return (kilowatt_t)v;
                        }                        
                    }
                    else if constexpr (units::traits::is_volume_unit< desiredUnit >()) {
                        if (pr->hydraul->unitsType == ::epanet::US) {
                            return (cubic_foot_t)v;
                        }
                        else { // SI units
                            return (cubic_meter_t)v;
                        }
                    }
                    else if constexpr (units::traits::is_flowrate_unit< desiredUnit >()) {
                        switch (pr->hydraul->flowUnitsType) {
                        case ::epanet::CFS: return (cubic_foot_per_second_t)v;          // cubic feet per second
                        case ::epanet::GPM: return (gallon_per_minute_t)v;          // gallons per minute
                        case ::epanet::MGD: return (million_gallon_per_minute_t)v;          // million gallons per day
                        case ::epanet::IMGD:return (imperial_million_gallon_per_day_t)v;          // imperial million gal. per day
                        case ::epanet::AFD: return (acre_foot_per_day_t)v;           // acre-feet per day
                        case ::epanet::LPS: return (liter_per_second_t)v;           // liters per second
                        case ::epanet::LPM: return (liter_per_minute_t)v;           // liters per minute
                        case ::epanet::MLD: return (megaliter_per_day_t)v;           // megaliters per day
                        case ::epanet::CMH: return (cubic_meter_per_hour_t)v;           // cubic meters per hour
                        case ::epanet::CMD: return (cubic_meter_per_day_t)v;            // cubic meters per day
                        default: return v;
                        }
                    }
                    else if constexpr (units::traits::is_pressure_unit< desiredUnit >()) {
                        if (pr->hydraul->unitsType == ::epanet::US) {
                            return (pounds_per_square_inch_t)v;
                        }
                        else { // SI units
                            return (head_t)(double)(meter_t)v;
                        }
                    }
                    else if constexpr (units::traits::is_dimensionless_unit< desiredUnit >()) {
                        return (scalar_t)v;
                    }
                    else if constexpr (units::traits::is_time_unit< desiredUnit >()) {
                        return (second_t)v;
                    }
                    else {
                        static_assert(false, "This unit type was not enumerated against");
                        return (desiredUnit)(v);
                    }
                };

                template<typename A> static AUTO FindOrAddPattern(cweeSharedPtr<Project> pr, cweeStr const& name) {
                    cweeSharedPtr<cweeBalancedPattern<A>> out;
                    AUTO patUnionPtr = pr->network->Patterns.Find(cweeStr::Hash(name));
                    if (!patUnionPtr) {
                        patUnionPtr = new cweeUnion< cweeStr, cweeAny >();
                        patUnionPtr->get<0>() = name;
                        patUnionPtr->get<1>() = new cweeBalancedPattern<A>();
                        patUnionPtr = pr->network->Patterns.AddOrReplace(patUnionPtr);
                    }
                    out = patUnionPtr->get<1>().cast< cweeSharedPtr<cweeBalancedPattern<A>>>();
                    return out;
                };
                template<typename A, typename B> static AUTO FindOrAddPattern(cweeSharedPtr<Project> pr, cweeStr const& name) {
                    cweeSharedPtr<cweeBalancedPattern<A, B>> out;
                    AUTO patUnionPtr = pr->network->Patterns.Find(cweeStr::Hash(name));
                    if (!patUnionPtr) {
                        patUnionPtr = new cweeUnion< cweeStr, cweeAny >();
                        patUnionPtr->get<0>() = name;
                        patUnionPtr->get<1>() = new cweeBalancedPattern<A, B>();
                        patUnionPtr = pr->network->Patterns.AddOrReplace(patUnionPtr);
                    }
                    out = patUnionPtr->get<1>().cast<cweeSharedPtr<cweeBalancedPattern<A, B>>>();
                    return out;
                };

                static int optionchoice(cweeSharedPtr<Project> pr, int n) {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    // AUTO qual = pr->quality;
                    
                    int choice;

                    // Check if 1st token matches a parameter name and
                    // process the input for the matched parameter
                    if (n < 0) return 201;

                    // Flow UNITS
                    if (::epanet::match(parser->Tokens[0], ::epanet::w_UNITS))
                    {
                        if (n < 1) return 0;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_CFS))  { parser->Flowflag = ::epanet::CFS; hyd->flowUnitsType = ::epanet::CFS; hyd->unitsType = ::epanet::US; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_GPM))  { parser->Flowflag = ::epanet::GPM; hyd->flowUnitsType = ::epanet::GPM; hyd->unitsType = ::epanet::US; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_AFD))  { parser->Flowflag = ::epanet::AFD; hyd->flowUnitsType = ::epanet::AFD; hyd->unitsType = ::epanet::US; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_MGD))  { parser->Flowflag = ::epanet::MGD; hyd->flowUnitsType = ::epanet::MGD; hyd->unitsType = ::epanet::US; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_IMGD)) { parser->Flowflag = ::epanet::IMGD; hyd->flowUnitsType = ::epanet::IMGD; hyd->unitsType = ::epanet::US; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_LPS)) { parser->Flowflag = ::epanet::LPS; hyd->flowUnitsType = ::epanet::LPS; hyd->unitsType = ::epanet::SI; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_LPM))  { parser->Flowflag = ::epanet::LPM; hyd->flowUnitsType = ::epanet::LPM; hyd->unitsType = ::epanet::SI; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_CMH))  { parser->Flowflag = ::epanet::CMH; hyd->flowUnitsType = ::epanet::CMH; hyd->unitsType = ::epanet::SI; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_CMD))  { parser->Flowflag = ::epanet::CMD; hyd->flowUnitsType = ::epanet::CMD; hyd->unitsType = ::epanet::SI; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_MLD))  { parser->Flowflag = ::epanet::MLD; hyd->flowUnitsType = ::epanet::MLD; hyd->unitsType = ::epanet::SI; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_SI))   { parser->Flowflag = ::epanet::LPS; hyd->flowUnitsType = ::epanet::LPS; hyd->unitsType = ::epanet::SI; }
                        else return setError(parser, 1, 213);
                    }

                    // PRESSURE units
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_PRESSURE))
                    {
                        if (n < 1) return 0;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_EXPONENT)) return -1;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_PSI)) { parser->Pressflag = ::epanet::PSI; hyd->pressureUnitsType = ::epanet::PSI; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_KPA)) { parser->Pressflag = ::epanet::KPA; hyd->pressureUnitsType = ::epanet::KPA; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_METERS)) { parser->Pressflag = ::epanet::METERS; hyd->pressureUnitsType = ::epanet::METERS; }
                        else return setError(parser, 1, 213);
                    }

                    // HEADLOSS formula
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_HEADLOSS))
                    {
                        if (n < 1)  return 0;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_HW)) { hyd->Formflag = ::epanet::HW; hyd->headLossType = ::epanet::HW; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_DW))   { hyd->Formflag = ::epanet::DW; hyd->headLossType = ::epanet::DW; }
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_CM))   { hyd->Formflag = ::epanet::CM; hyd->headLossType = ::epanet::CM; }
                        else return setError(parser, 1, 213);
                    }
                    
                    // Water QUALITY option
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_QUALITY))
                    {
                        return 0;
                        /*
                        if (n < 1) return 0;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_NONE))  qual->Qualflag = ::epanet::NONE;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_CHEM))  qual->Qualflag = ::epanet::CHEM;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_ENERGYINTENSITY))  qual->Qualflag = ::epanet::ENERGYINTENSITY;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_AGE))   qual->Qualflag = ::epanet::AGE;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_TRACE)) qual->Qualflag = ::epanet::TRACE_QUAL;
                        else
                        {
                            qual->Qualflag = ::epanet::CHEM;
                            strncpy(qual->ChemName, parser->Tokens[1], ::epanet::MAXID);
                            if (n >= 2) strncpy(qual->ChemUnits, parser->Tokens[2], MAXID);
                        }
                        if (qual->Qualflag == ::epanet::TRACE_QUAL)
                        {
                            // Copy Trace Node ID to parser->Tokens[0] for error reporting
                            ::strcpy(parser->Tokens[0], "");
                            if (n < 2) return 201;
                            ::strcpy(parser->Tokens[0], parser->Tokens[2]);
                            qual->TraceNode = epanet_shared::findnode(net, parser->Tokens[2]);
                            if (qual->TraceNode == 0) return setError(parser, 2, 212);
                            strncpy(qual->ChemName, ::epanet::u_PERCENT, ::epanet::MAXID);
                            strncpy(qual->ChemUnits, parser->Tokens[2], MAXID);
                        }
                        if (qual->Qualflag == ::epanet::AGE)
                        {
                            strncpy(qual->ChemName, ::epanet::w_AGE, ::epanet::MAXID);
                            strncpy(qual->ChemUnits, ::epanet::u_HOURS, ::epanet::MAXID);
                        }
                        */
                    }
                    
                    // Hydraulics UNBALANCED option
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_UNBALANCED))
                    {
                        if (n < 1) return 0;
                        if (::epanet::match(parser->Tokens[1], ::epanet::w_STOP)) hyd->ExtraIter = -1;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_CONTINUE))
                        {
                            if (n >= 2)  hyd->ExtraIter = atoi(parser->Tokens[2]);
                            else         hyd->ExtraIter = 0;
                        }
                        else return setError(parser, 1, 213);
                    }

                    // Default demand PATTERN
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_PATTERN))
                    {
                        if (n < 1) return 0;
                        parser->DefPatID = parser->Tokens[1];                       
                        parser->DefPat = FindOrAddPattern< DefaultUnits<value_t::_DEMAND_>::unit >(pr, "Pattern_" + parser->DefPatID);
                    }

                    // DEMAND model
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_DEMAND))
                    {
                        if (n < 2) return 0;
                        if (!::epanet::match(parser->Tokens[1], ::epanet::w_MODEL)) return -1;
                        choice = ::epanet::findmatch(parser->Tokens[2], ::epanet::DemandModelTxt);
                        if (choice < 0) return setError(parser, 2, 213);
                        hyd->DemandModel = choice;
                    }

                    // Return -1 if keyword did not match any option
                    else return -1;
                    return 0;
                };
                static int optionvalue(cweeSharedPtr<Project> pr, int n) {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    // AUTO qual = pr->quality;

                    int    nvalue = 1; // Index of token with numerical value
                    double y;
                    const char* tok0 = parser->Tokens[0];

                    // Check for deprecated SEGMENTS keyword
                    if (::epanet::match(tok0, ::epanet::w_SEGMENTS)) return 0;

                    // Check for missing value (which is permissible)
                    if (::epanet::match(tok0, ::epanet::w_SPECGRAV) || ::epanet::match(tok0, ::epanet::w_EMITTER) ||
                        ::epanet::match(tok0, ::epanet::w_DEMAND) || ::epanet::match(tok0, ::epanet::w_MINIMUM) ||
                        ::epanet::match(tok0, ::epanet::w_REQUIRED) || ::epanet::match(tok0, ::epanet::w_PRESSURE) ||
                        ::epanet::match(tok0, ::epanet::w_PRECISION)
                        ) nvalue = 2;
                    if (n < nvalue) return 0;

                    // Check for valid numerical input
                    if (!::epanet::getfloat(parser->Tokens[nvalue], &y)) return setError(parser, nvalue, 202);

                    // Quality tolerance option (which can be 0)
                    if (::epanet::match(tok0, ::epanet::w_TOLERANCE))
                    {
                        /*
                        if (y < 0.0) return setError(parser, nvalue, 213);
                        qual->Ctol = y;
                        */
                        return 0;                        
                    }

                    // Diffusivity
                    if (::epanet::match(tok0, ::epanet::w_DIFFUSIVITY))
                    {
                        /*
                        if (y < 0.0) return setError(parser, nvalue, 213);
                        qual->Diffus = y;
                        */
                        return 0;
                    }

                    // Hydraulic damping limit option */
                    if (::epanet::match(tok0, ::epanet::w_DAMPLIMIT))
                    {
                        hyd->DampLimit = y;
                        return 0;
                    }

                    // Flow change limit
                    else if (::epanet::match(tok0, ::epanet::w_FLOWCHANGE))
                    {
                        if (y < 0.0) return setError(parser, nvalue, 213);
                        hyd->FlowChangeLimit = y;
                        return 0;
                    }

                    // Head loss error limit
                    else if (::epanet::match(tok0, ::epanet::w_HEADERROR))
                    {
                        if (y < 0.0) return setError(parser, nvalue, 213);
                        hyd->HeadErrorLimit = y;
                        return 0;
                    }

                    // Pressure dependent demand parameters
                    else if (::epanet::match(tok0, ::epanet::w_MINIMUM))
                    {
                        if (y < 0.0) return setError(parser, nvalue, 213);
                        // Required pressure still at default value
                        AUTO typeSample = hyd->Preq;
                        if (hyd->Preq == (decltype(typeSample))::epanet::MINPDIFF)
                            hyd->Preq = y + ::epanet::MINPDIFF;
                        // Required pressure already entered
                        else if ((hyd->Preq - (decltype(typeSample))y) < (decltype(typeSample))::epanet::MINPDIFF)
                            return setError(parser, nvalue, 208);
                        hyd->Pmin = y;
                        return 0;
                    }
                    else if (::epanet::match(tok0, ::epanet::w_REQUIRED))
                    {
                        AUTO typeSample = hyd->Pmin;
                        if (y < 0.0) return setError(parser, nvalue, 213);
                        if (((decltype(typeSample))y - hyd->Pmin) < (decltype(typeSample))::epanet::MINPDIFF)
                            return setError(parser, nvalue, 208);
                        hyd->Preq = y;
                        return 0;
                    }
                    else if (::epanet::match(tok0, ::epanet::w_PRESSURE))
                    {
                        if (y < 0.0) return setError(parser, nvalue, 213);
                        hyd->Pexp = y;
                        return 0;
                    }

                    // All other options must be > 0
                    if (y <= 0.0) return setError(parser, nvalue, 213);

                    // Assign value to all other options
                    if (::epanet::match(tok0, ::epanet::w_VISCOSITY))     hyd->Viscos = y;
                    else if (::epanet::match(tok0, ::epanet::w_SPECGRAV)) hyd->SpGrav = y;
                    else if (::epanet::match(tok0, ::epanet::w_TRIALS))   hyd->MaxIter = (int)y;
                    else if (::epanet::match(tok0, ::epanet::w_ACCURACY))
                    {
                        y = ::epanet::fMAX(y, 1.e-5);
                        y = ::epanet::fMIN(y, 1.e-1);
                        hyd->Hacc = y;
                    }
                    else if (::epanet::match(tok0, ::epanet::w_HTOL))  hyd->Htol = y;
                    else if (::epanet::match(tok0, ::epanet::w_QTOL))  hyd->Qtol = y;
                    else if (::epanet::match(tok0, ::epanet::w_RQTOL))
                    {
                        if (y >= 1.0) return 213;
                        hyd->RQtol = y;
                    }
                    else if (::epanet::match(tok0, ::epanet::w_CHECKFREQ)) hyd->CheckFreq = (int)y;
                    else if (::epanet::match(tok0, ::epanet::w_MAXCHECK))  hyd->MaxCheck = (int)y;
                    else if (::epanet::match(tok0, ::epanet::w_EMITTER))   hyd->Qexp = 1.0 / y;
                    else if (::epanet::match(tok0, ::epanet::w_DEMAND))    hyd->Dmult = y;
                    else return 201;
                    return 0;
                };
                static int optiondata(cweeSharedPtr<Project> pr) {
                    AUTO parser = pr->parser;

                    int i, n;

                    // Option is a named choice
                    n = parser->Tokens.Num() - 1;
                    i = optionchoice(pr, n);
                    if (i >= 0) return i;

                    // Option is a numerical value
                    return (optionvalue(pr, n));
                };

                static int juncdata(cweeSharedPtr<Project> pr) {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;

                    int p = 0;                  // time pattern index
                    int n;                      // number of tokens
                    double el,                  // elevation
                        y = 0.0;                // base demand
                    cweeJunction node;
                    int err = 0;

                    // Add a new demand to it
                    AUTO demandP = node.Demands.GetExclusive()->Alloc();

                    // Add new junction to data base
                    n = parser->Tokens.Num();    

                    // Check for valid data
                    if (n < 2) return 201; // needs elevation
                    if (!::epanet::getfloat(parser->Tokens[1], &el)) return setError(parser, 1, 202);
                   
                    if (n >= 3 && !::epanet::getfloat(parser->Tokens[2], &y)) return setError(parser, 2, 202);
                    if (n >= 4)
                    {
                        AUTO patPtr = FindOrAddPattern< DefaultUnits<value_t::_DEMAND_>::unit >(pr, "Pattern_" + parser->Tokens[3]);

                        demandP.Pat = patPtr;
                        demandP.Name = parser->Tokens[3];
                    }
                    else {
                        // use the default demand pattern ID
                        AUTO patPtr = FindOrAddPattern< DefaultUnits<value_t::_DEMAND_>::unit >(pr, "Pattern_" + parser->DefPatID);
                        demandP.Pat = patPtr;
                        demandP.Name = parser->DefPatID;
                    }

                    // Save junction data
                    node.Name = parser->Tokens[0];
                    node.Nickname = node.Name;
                    node.Description = parser->Comment;
                    node.Coordinates = vec3d(::epanet::MISSING, ::epanet::MISSING, (double)convertToUnit<foot_t>(pr, el));
                    
                    node.Ke = 0.0;
                    demandP.Multiplier = y; // pat set earlier (hopefully)

                    net->Append(node);

                    return 0;
                };
                static int tankdata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;

                    int    i,               // Node index
                        n,               // # data items
                        overflow = FALSE;// Overflow indicator

                    double el = 0.0,        // Elevation
                        initlevel = 0.0, // Initial level
                        minlevel = 0.0,  // Minimum level
                        maxlevel = 0.0,  // Maximum level
                        minvol = 0.0,    // Minimum volume
                        diam = 0.0,      // Diameter
                        area;            // X-sect. area

                    cweeReservoir node;

                    int err = 0;

                    // Add new tank to data base
                    n = parser->Tokens.Num();                    

                    // Check for valid data
                    if (n < 2) return 201;
                    if (!::epanet::getfloat(parser->Tokens[1], &el)) return setError(parser, 1, 202);

                    // Tank is reservoir
                    if (n <= 3)
                    {
                        node.TerminalStorage = true;

                        if (n == 3) {
                            AUTO patPtr = FindOrAddPattern< DefaultUnits<value_t::_HEAD_>::unit >(pr, "Pattern_" + parser->Tokens[2]);
                            node.HeadPattern = patPtr;
                        }
                    }
                    else if (n < 6) return 201;
                    // Tank is a storage tank
                    else
                    {
                        node.TerminalStorage = false;

                        if (!::epanet::getfloat(parser->Tokens[2], &initlevel)) return setError(parser, 2, 202);
                        if (!::epanet::getfloat(parser->Tokens[3], &minlevel))  return setError(parser, 3, 202);
                        if (!::epanet::getfloat(parser->Tokens[4], &maxlevel))  return setError(parser, 4, 202);
                        if (!::epanet::getfloat(parser->Tokens[5], &diam))      return setError(parser, 5, 202);
                        if (n >= 7 && !::epanet::getfloat(parser->Tokens[6], &minvol)) return setError(parser, 6, 202);

                        // volume curve supplied
                        if (n >= 8)
                        {
                            AUTO patPtr = FindOrAddPattern< gallon_t, foot_t >(pr, "Curve_" + parser->Tokens[7]);
                            node.VolumeCurve = patPtr;
                        }

                        // Parse overflow indicator if present
                        if (n >= 9)
                        {
                            if (::epanet::match(parser->Tokens[8], ::epanet::w_YES)) overflow = TRUE;
                            else if (::epanet::match(parser->Tokens[8], ::epanet::w_NO)) overflow = FALSE;
                            else return setError(parser, 8, 213);
                        }

                        if (initlevel < minlevel) return setError(parser, 2, 209);
                        if (minlevel < 0.0) return setError(parser, 3, 209);
                        if (maxlevel < minlevel) return setError(parser, 4, 209);
                        if (diam < 0.0) return setError(parser, 5, 209);
                        if (minvol < 0.0) return setError(parser, 6, 209);
                    }

                    node.Name = parser->Tokens[0];
                    node.Nickname = parser->Tokens[0];
                    node.Description = parser->Comment;
                    node.Coordinates = vec3d(::epanet::MISSING, ::epanet::MISSING, (double)convertToUnit<foot_t>(pr, el));
                    node.Ke = 0.0;
                    AUTO lvl_ptr = node.GetValue<value_t::_HEAD_>();
                    if (lvl_ptr) { lvl_ptr->AddValue(0, convertToUnit<foot_t>(pr, initlevel) + convertToUnit<foot_t>(pr, el)); }
                    node.MinLevel = convertToUnit<foot_t>(pr, minlevel);
                    node.MaxLevel = convertToUnit<foot_t>(pr, maxlevel);
                    node.Diameter = convertToUnit<foot_t>(pr, diam);
                    node.CanOverflow = (bool)overflow;
                    node.MixModel = ::epanet::MIX1; // Completely mixed
                    node.MixCompartmentFraction = 1.0;

                    net->Append(node);

                    return 0;
                };
                static int pipedata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;

                    int      j1,               // Start-node index
                        j2,               // End-node index
                        n;                // # data items
                    double   length,           // Pipe length
                        diam,             // Pipe diameter
                        rcoeff,           // Roughness coeff.
                        lcoeff = 0.0;     // Minor loss coeff
                    ::epanet::LinkType type = ::epanet::PIPE;      // Link type
                    ::epanet::StatusType status = ::epanet::OPEN;  // Link status

                    cweePipe link;

                    int err = 0;

                    // Add new pipe to database
                    n = parser->Tokens.Num();


                    if (err) return setError(parser, 0, err);

                    // Check for valid data
                    if (n < 6) return 201;

                    AUTO startingNode = net->FindNode(parser->Tokens[1]); if (!startingNode) return setError(parser, 1, 203);
                    AUTO endingNode = net->FindNode(parser->Tokens[2]); if (!startingNode) return setError(parser, 2, 203);
                    if (startingNode == endingNode) return setError(parser, 0, 222); // cannot connect to itself

                    if (!::epanet::getfloat(parser->Tokens[3], &length)) return setError(parser, 3, 202);
                    if (length <= 0.0) return setError(parser, 3, 211);
                    if (!::epanet::getfloat(parser->Tokens[4], &diam)) return  setError(parser, 4, 202);
                    if (diam <= 0.0) return setError(parser, 4, 211);
                    if (!::epanet::getfloat(parser->Tokens[5], &rcoeff)) return setError(parser, 5, 202);
                    if (rcoeff <= 0.0) setError(parser, 5, 211);

                    // Either a loss coeff. or a status is supplied
                    if (n == 7)
                    {
                        if (::epanet::match(parser->Tokens[6], ::epanet::w_CV)) type = ::epanet::CVPIPE;
                        else if (::epanet::match(parser->Tokens[6], ::epanet::w_CLOSED)) status = ::epanet::CLOSED;
                        else if (::epanet::match(parser->Tokens[6], ::epanet::w_OPEN))   status = ::epanet::OPEN;
                        else if (!::epanet::getfloat(parser->Tokens[6], &lcoeff)) return setError(parser, 6, 202);
                    }

                    // Both a loss coeff. and a status is supplied
                    if (n == 8)
                    {
                        if (!::epanet::getfloat(parser->Tokens[6], &lcoeff)) return setError(parser, 6, 202);
                        if (::epanet::match(parser->Tokens[7], ::epanet::w_CV))  type = ::epanet::CVPIPE;
                        else if (::epanet::match(parser->Tokens[7], ::epanet::w_CLOSED)) status = ::epanet::CLOSED;
                        else if (::epanet::match(parser->Tokens[7], ::epanet::w_OPEN))   status = ::epanet::OPEN;
                        else return setError(parser, 7, 213);
                    }
                    if (lcoeff < 0.0) return setError(parser, 6, 211);

                    link.GetValue<value_t::_STATUS_>()->AddUniqueValue(0, static_cast<int>(status)); // set the initial value for the pipe to OPEN or CLOSED

                    // Save pipe data
                    link.Name = parser->Tokens[0];
                    link.Nickname = link.Name;
                    link.Description = parser->Comment;
                    link.StartingAsset = startingNode;
                    link.EndingAsset = endingNode;
                    link.Length = convertToUnit<foot_t>(pr, length);
                    link.Diameter = convertToUnit<foot_t>(pr, diam, true); // converting to diameter then inches as a confirmation that everythign works
                    link.Kc_Roughness = rcoeff;
                    link.Kb_BulkReactionCoeff = ::epanet::MISSING;
                    link.Km_MinorLoss = lcoeff;
                    link.Kw_WallReactionCoeff = ::epanet::MISSING;
                    link.CheckValve = type == ::epanet::CVPIPE ? true : false;

                    net->Append(link);

                    return 0;
                };
                static int pumpdata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;

                    int    j, m,  // Token array indexes
                        j1,    // Start-node index
                        j2,    // End-node index
                        n,     // # data items
                        c, p;  // Curve & Pattern indexes
                    double y;

                    int err = 0;

                    cweePump pump;

                    /* Add new pump to data base */
                    n = parser->Tokens.Num();

                    // Check for valid data
                    if (n < 3) return 201;

                    AUTO startingNode = net->FindNode(parser->Tokens[1]); if (!startingNode) return setError(parser, 1, 203);
                    AUTO endingNode = net->FindNode(parser->Tokens[2]); if (!startingNode) return setError(parser, 2, 203);
                    if (startingNode == endingNode) return setError(parser, 0, 222); // cannot connect to itself

                    cweeSharedPtr<cweePump> link = net->Append(pump);
                    link->Name = parser->Tokens[0];
                    link->Nickname = parser->Tokens[0];
                    link->Description = parser->Comment;
                    link->StartingAsset = startingNode;
                    link->EndingAsset = endingNode;
                    link->Diameter = (inch_t)0; // ????
                    link->Kc_Roughness = 1;
                    link->Kb_BulkReactionCoeff = 0;
                    link->Km_MinorLoss = 0;
                    link->Kw_WallReactionCoeff = 0;
                    link->HeadCurveMode = ::epanet::NOCURVE;

                    AUTO pat_ptr = link->GetValue<value_t::_STATUS_>();
                    if (pat_ptr) pat_ptr->AddValue(0, static_cast<int>(::epanet::OPEN)); 

                    if (n < 4) return 0;

                    // If 4-th token is a number then input follows Version 1.x format so retrieve pump curve parameters     
                    double temp(0);
                    if (::epanet::getfloat(parser->Tokens[3], &temp))
                    {
                        throw(std::exception("WaterWatch does not support the Version 1.x explicit pump curve format. Please transfer the headcurve into the CURVES section and reference it with the HEAD keyword."));
                        return 201;
                    }
                    else {
                        // Otherwise input follows Version 2 format so retrieve keyword/value pairs
                        m = 4;
                        while (m < n)
                        {
                            if (::epanet::match(parser->Tokens[m - 1], ::epanet::w_POWER)) // Const. HP curve
                            {
                                y = atof(parser->Tokens[m]);
                                if (y <= 0.0) return setError(parser, m, 202);
                                link->HeadCurveMode = ::epanet::CONST_HP;
                                link->Km_MinorLoss = (double)convertToUnit<horsepower_t>(pr, y);
                                link->Power = convertToUnit<horsepower_t>(pr, y);
                            }
                            else if (::epanet::match(parser->Tokens[m - 1], ::epanet::w_HEAD))  // Custom pump curve
                            {
                                link->HeadCurve = FindOrAddPattern<foot_t, gallon_per_minute_t>(pr, "Curve_" + parser->Tokens[m]);
                                link->HeadCurveMode = ::epanet::PumpType::CUSTOM;
                            }
                            else if (::epanet::match(parser->Tokens[m - 1], ::epanet::w_EFFIC))  // Efficiency Pump Curve
                            {
                                link->EfficiencyCurve = FindOrAddPattern<scalar_t, gallon_per_minute_t>(pr, "Curve_" + parser->Tokens[m]);
                                link->HeadCurveMode = ::epanet::PumpType::CUSTOM;
                            }
                            else if (::epanet::match(parser->Tokens[m - 1], ::epanet::w_PATTERN))  // Speed/status pattern
                            {
                                link->PreDeterminedStatusPattern = FindOrAddPattern<scalar_t>(pr, "Pattern_" + parser->Tokens[m]);
                            }
                            else if (::epanet::match(parser->Tokens[m - 1], ::epanet::w_SPEED))   // Speed setting? Initial Setting? Unclear
                            {
                                if (!::epanet::getfloat(parser->Tokens[m], &y)) return setError(parser, m, 202);
                                if (y < 0.0) return setError(parser, m, 211);
                                link->Kc_Roughness = y;                                
                            }
                            else return 201;
                            m = m + 2;  // Move to next keyword token
                        }
                    }

                    return 0;
                };
                static int valvecheck(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeValve> index, valveType_t type, cweeSharedPtr<cweeNode> j1, cweeSharedPtr<cweeNode> j2) {
                    AUTO net = pr->network;
                    if (type == valveType_t::PRV || type == valveType_t::PSV || type == valveType_t::FCV)
                    {
                        for (auto& x : net->valves.HashList()) {
                            AUTO valvePtr = net->valves.Find(x);
                            if (valvePtr && valvePtr != index) {
                                AUTO vj1 = valvePtr->StartingAsset;
                                AUTO vj2 = valvePtr->EndingAsset;
                                AUTO vtype = valvePtr->valveType;

                                if (vtype == valveType_t::PRV && type == valveType_t::PRV)
                                {
                                    if (vj2 == j2 || vj2 == j1 || vj1 == j2) return 220;
                                }

                                // Cannot have two PSVs sharing upstream nodes or in series
                                if (vtype == valveType_t::PSV && type == valveType_t::PSV)
                                {
                                    if (vj1 == j1 || vj1 == j2 || vj2 == j1) return 220;
                                }

                                // Cannot have PSV connected to downstream node of PRV
                                if (vtype == valveType_t::PSV && type == valveType_t::PRV && vj1 == j2) return 220;
                                if (vtype == valveType_t::PRV && type == valveType_t::PSV && vj2 == j1) return 220;

                                // Cannot have PSV connected to downstream node of FCV
                                // nor have PRV connected to upstream node of FCV
                                if (vtype == valveType_t::FCV && type == valveType_t::PSV && vj2 == j1) return 220;
                                if (vtype == valveType_t::FCV && type == valveType_t::PRV && vj1 == j2) return 220;
                                if (vtype == valveType_t::PSV && type == valveType_t::FCV && vj1 == j2) return 220;
                                if (vtype == valveType_t::PRV && type == valveType_t::FCV && vj2 == j1) return 220;
                            }
                        }
                    }
                    return 0;
                };
                static int valvedata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;

                    int c,                     // Curve index
                        j1,                    // Start-node index
                        j2,                    // End-node index
                        n;                     // # data items
                    char  status = ::epanet::ACTIVE,     // Valve status
                        type;                // Valve type
                    double diam = 0.0,         // Valve diameter
                        setting,            // Valve setting
                        lcoeff = 0.0;       // Minor loss coeff.
                    valveType_t ValveType;
                    int err = 0;

                    cweeSharedPtr<cweeValve> link = net->Append(cweeValve());

                    // Add new valve to data base
                    n = parser->Tokens.Num();

                    // Check for valid data
                    if (n < 6) return 201;
                    AUTO startingNode = net->FindNode(parser->Tokens[1]); if (!startingNode) return setError(parser, 1, 203);
                    AUTO endingNode = net->FindNode(parser->Tokens[2]); if (!startingNode) return setError(parser, 2, 203);
                    if (startingNode == endingNode) return setError(parser, 0, 222); // cannot connect to itself

                    if (::epanet::match(parser->Tokens[4], ::epanet::w_PRV)) ValveType = valveType_t::PRV;
                    else if (::epanet::match(parser->Tokens[4], ::epanet::w_PSV))  ValveType = valveType_t::PSV;
                    else if (::epanet::match(parser->Tokens[4], ::epanet::w_PBV))  ValveType = valveType_t::PBV;
                    else if (::epanet::match(parser->Tokens[4], ::epanet::w_FCV))  ValveType = valveType_t::FCV;
                    else if (::epanet::match(parser->Tokens[4], ::epanet::w_TCV))  ValveType = valveType_t::TCV;
                    else if (::epanet::match(parser->Tokens[4], ::epanet::w_GPV))  ValveType = valveType_t::GPV;
                    else return setError(parser, 4, 213);

                    if (!::epanet::getfloat(parser->Tokens[3], &diam)) return setError(parser, 3, 202);
                    if (diam <= 0.0) return setError(parser, 3, 211);

                    // Find headloss curve for GPV
                    if (ValveType == valveType_t::GPV)
                    {
                        link->HeadlossCurve = FindOrAddPattern<foot_t, gallon_per_minute_t>(pr, "Curve_" + parser->Tokens[4]);
                        setting = 0;
                        status = ::epanet::OPEN;
                    }
                    else if (!::epanet::getfloat(parser->Tokens[5], &setting)) return setError(parser, 5, 202);
                    if (n >= 7 && !::epanet::getfloat(parser->Tokens[6], &lcoeff)) return setError(parser, 6, 202);


                   
                    // Save valve data
                    link->Name = parser->Tokens[0];
                    link->Nickname = parser->Tokens[0];
                    link->Description = parser->Comment;
                    link->StartingAsset = startingNode;
                    link->EndingAsset = endingNode;

                    // Check for illegal connections
                    if (valvecheck(pr, link, ValveType, startingNode, endingNode)) { return setError(parser, -1, 220); }
                    link->Diameter = convertToUnit<foot_t>(pr, diam, true); // converting to feet then inches to confirm everything works
                    link->Length = (foot_t)0.0;
                    link->Kc_Roughness = setting; // for GPV, they tried to store the HEADLOSS CURVE into this! 
                    link->Km_MinorLoss = lcoeff;
                    link->Kb_BulkReactionCoeff = 0.0;
                    link->Kw_WallReactionCoeff = 0.0;
                    link->valveType = ValveType;
                    link->GetValue<value_t::_STATUS_>()->AddUniqueValue(0, static_cast<int>(status));

                    return 0;
                };

                /* 
                cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>> // pattern
                cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_HEAD_>::unit>> // pattern
                cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>> // curve
                cweeSharedPtr<cweeBalancedPattern<foot_t, gallon_per_minute_t>> // curve
                cweeSharedPtr<cweeBalancedPattern<scalar_t, gallon_per_minute_t>> // curve
                cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>> // curve
                cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>> // pattern
                */

                static int getNumItemsInPattern(cweeSharedPtr<Project> pr, cweeStr const& name) {
                    AUTO unionPtr = pr->network->Patterns.Find(cweeStr::Hash("Pattern_" + name));
                    if (unionPtr) {
                        cweeAny& patOrCurve = unionPtr->get<1>();
                        if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>>()) {
                            AUTO p = patOrCurve.cast<cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>>>();
                            return p->GetNumValues();
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<scalar_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<scalar_t>> >();
                            return p->GetNumValues();
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>> >();
                            return p->GetNumValues();
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_HEAD_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_HEAD_>::unit>>>();
                            return p->GetNumValues();
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<gallon_t, foot_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>>>();
                            return p->GetNumValues();
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<foot_t, gallon_per_minute_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<foot_t, gallon_per_minute_t>> >();
                            return p->GetNumValues();
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<scalar_t, gallon_per_minute_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<scalar_t, gallon_per_minute_t>> >();
                            return p->GetNumValues();
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<gallon_t, foot_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>> >();
                            return p->GetNumValues();
                        }
                    }
                    return -1;
                };
                static int addPatternValue(cweeSharedPtr<Project> pr, cweeStr const& name, u64 X, u64 Y) { 
                    AUTO unionPtr = pr->network->Patterns.Find(cweeStr::Hash("Pattern_" + name));
                    if (unionPtr) {
                        cweeAny& patOrCurve = unionPtr->get<1>();
                        if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>>()) {
                            AUTO p = patOrCurve.cast<cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>>>();
                            p->AddUniqueValue(X, convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<scalar_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<scalar_t>> >();
                            p->AddUniqueValue(X, Y);
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>> >();
                            p->AddUniqueValue(X, convertToUnit<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_HEAD_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_HEAD_>::unit>>>();
                            p->AddUniqueValue(X, convertToUnit<foot_t>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<gallon_t, foot_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>>>();
                            p->AddUniqueValue(convertToUnit<foot_t>(pr, X), convertToUnit<gallon_t>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<foot_t, DefaultUnits<value_t::_DEMAND_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<foot_t, DefaultUnits<value_t::_DEMAND_>::unit>> >();
                            p->AddUniqueValue(convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, X), convertToUnit<foot_t>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<scalar_t, DefaultUnits<value_t::_DEMAND_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<scalar_t, DefaultUnits<value_t::_DEMAND_>::unit>> >();
                            p->AddUniqueValue(convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, X), Y);
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<gallon_t, foot_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>> >();
                            p->AddUniqueValue(convertToUnit<foot_t>(pr, X), convertToUnit<gallon_t>(pr, Y));
                            return 0;
                        }
                    }
                    return 201;
                };
                static int addCurveValue(cweeSharedPtr<Project> pr, cweeStr const& name, u64 X, u64 Y) {
                    AUTO unionPtr = pr->network->Patterns.Find(cweeStr::Hash("Curve_" + name));
                    if (unionPtr) {
                        cweeAny& patOrCurve = unionPtr->get<1>();
                        if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>>()) {
                            AUTO p = patOrCurve.cast<cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>>>();
                            p->AddUniqueValue(X, convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<scalar_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<scalar_t>> >();
                            p->AddUniqueValue(X, Y);
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>> >();
                            p->AddUniqueValue(X, convertToUnit<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<DefaultUnits<value_t::_HEAD_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_HEAD_>::unit>>>();
                            p->AddUniqueValue(X, convertToUnit<foot_t>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<gallon_t, foot_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>>>();
                            p->AddUniqueValue(convertToUnit<foot_t>(pr, X), convertToUnit<gallon_t>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<foot_t, DefaultUnits<value_t::_DEMAND_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<foot_t, DefaultUnits<value_t::_DEMAND_>::unit>> >();
                            p->AddUniqueValue(convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, X), convertToUnit<foot_t>(pr, Y));
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<scalar_t, DefaultUnits<value_t::_DEMAND_>::unit>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<scalar_t, DefaultUnits<value_t::_DEMAND_>::unit>> >();
                            p->AddUniqueValue(convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, X), Y);
                            return 0;
                        }
                        else if (patOrCurve.IsTypeOf<cweeBalancedPattern<gallon_t, foot_t>>()) {
                            AUTO p = patOrCurve.cast < cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>> >();
                            p->AddUniqueValue(convertToUnit<foot_t>(pr, X), convertToUnit<gallon_t>(pr, Y));
                            return 0;
                        }
                    }
                    return 201;
                };
                
                static int patterndata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;

                    int i, j, n, n1;
                    double x;


                    // "n" is the number of pattern factors contained in the line
                    n = parser->Tokens.Num() - 1;
                    if (n < 1) return 201;

                    // Add parsed multipliers to the pattern
                    for (j = 1; j <= n; j++) {
                        n1 = getNumItemsInPattern(pr, parser->Tokens[0]);
                        if (n1 >= 0) {
                            if (!::epanet::getfloat(parser->Tokens[j], &x)) return setError(parser, j, 202);
                            if (addPatternValue(pr, parser->Tokens[0], (u64)pr->times->GetPatternStartTime() + (u64)((u64)pr->times->Pstep * (u64)n1), x)) { return setError(parser, 0, 205); }
                        }
                        else {
                            // nobody references this pattern? 
                            return 201; // can't instantiate this pattern because it isn't "used" by anyone! This is required because we know the units for the pattern only after it is referenced by someone.
                        }
                    }                    
                    return 0;
                };

                static int curvedata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;

                    int i;
                    double x, y;

                    // Check for valid data
                    if (parser->Tokens.Num() < 3) return 201;
                    if (!::epanet::getfloat(parser->Tokens[1], &x)) return setError(parser, 1, 202);
                    if (!::epanet::getfloat(parser->Tokens[2], &y)) return setError(parser, 2, 202);

                    return addCurveValue(pr, parser->Tokens[0], x, y);
                };
                static int demanddata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;

                    int n, p = 0;
                    double y;

                    // Extract data from tokens
                    n = parser->Tokens.Num();
                    if (n < 2) return 203;

                    if (!::epanet::getfloat(parser->Tokens[1], &y)) return setError(parser, 1, 202);

                    // If MULTIPLY command, save multiplier
                    if (::epanet::match(parser->Tokens[0], ::epanet::w_MULTIPLY))
                    {
                        if (y <= 0.0) return setError(parser, 1, 213);
                        else hyd->Dmult = y;
                        return 0;
                    }

                    AUTO junction = net->Find<asset_t::JUNCTION>(parser->Tokens[0]);
                    if (!junction) { return setError(parser, 1, 201); }

                    cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_DEMAND_>::unit>> DemandPattern;
                    // Otherwise find node (and pattern) being referenced
                    if (n >= 3)
                    {
                        DemandPattern = FindOrAddPattern<DefaultUnits<value_t::_DEMAND_>::unit>(pr, "Pattern_" + parser->Tokens[2]);
                    }

                    // originally, EPAnet overrides the demands from the junction data section if any demands exist for that junction in the demands section. I disagree and believe the user is "appending" demands rather then listing all of them.                      
                    AUTO demand = junction->Demands.GetExclusive()->Alloc();
                    demand.Multiplier = y;
                    demand.Pat = DemandPattern;
                    demand.Name = parser->Comment;

                    return 0;
                };
                static bool coord_is_longlat(vec2d const& xy) {
                    if (xy.x < 180 && xy.x > -180 && xy.y < 90 && xy.y > -90) {
                        return true;
                    }
                    else {
                        return false;
                    }
                };
                static vec2d convert_coord(double x, double y) {
                    vec2d* ref;
                    vec2d attempt0 = vec2d(x, y);
                    ref = &attempt0; if (coord_is_longlat(*ref)) { return *ref; }
                    vec2d attempt1 = vec2d(y, x);
                    ref = &attempt1; if (coord_is_longlat(*ref)) { return *ref; }
                    vec2d attempt2 = geocoding->GetLongLat(vec2d(x, y));
                    ref = &attempt2; if (coord_is_longlat(*ref)) { return *ref; }
                    vec2d attempt3 = geocoding->GetLongLat(vec2d(y, x));
                    ref = &attempt3; if (coord_is_longlat(*ref)) { return *ref; }
                    vec2d attempt4 = cweeEng::CoordinateConversion_FeetToLongLat(vec2d(x, y));
                    ref = &attempt4; if (coord_is_longlat(*ref)) { return *ref; }
                    vec2d attempt5 = cweeEng::CoordinateConversion_FeetToLongLat(vec2d(y, x));
                    ref = &attempt5; if (coord_is_longlat(*ref)) { return *ref; }
                    ref = &attempt0;
                    return *ref;
                };
                static int   coordata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;

                    int j;
                    double x, y;

                    // Check for valid node ID
                    if (parser->Tokens.Num() < 3) return 201;

                    AUTO node = net->FindNode(parser->Tokens[0]);
                    if (!node) return setError(parser, 0, 203);

                    // Check for valid data
                    if (!::epanet::getfloat(parser->Tokens[1], &x)) return setError(parser, 1, 202);
                    if (!::epanet::getfloat(parser->Tokens[2], &y)) return setError(parser, 2, 202);

                    // Save coord data                    
                    vec2d ref = convert_coord(x, y);
                    node->Coordinates.GetExclusive()->x = ref.x;
                    node->Coordinates.GetExclusive()->y = ref.y; 
                    return 0;
                };
                static int   vertexdata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;

                    int j;
                    double x, y;

                    // Check for valid link ID
                    if (parser->Tokens.Num() < 3) return 201;
                    AUTO link = net->Find<asset_t::PIPE>(parser->Tokens[0]);
                    if (!link) { return 0; /* Not worth crashing over this error - but note that the pipe was not found. Could have been referencing a pump or valve, of course. */ }

                    // Check for valid coordinate data
                    if (!::epanet::getfloat(parser->Tokens[1], &x)) return setError(parser, 1, 202);
                    if (!::epanet::getfloat(parser->Tokens[2], &y)) return setError(parser, 2, 202);

                    link->AppendMiddleVertex(convert_coord(x, y));
                };
                static int   energydata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;

                    int j, k, n, p, c;
                    double y;

                    // Check for sufficient data
                    n = parser->Tokens.Num();
                    if (n < 3) return 201;

                    // First keyword is DEMAND
                    if (::epanet::match(parser->Tokens[0], ::epanet::w_DMNDCHARGE))
                    {
                        if (!::epanet::getfloat(parser->Tokens[2], &y)) return setError(parser, 2, 202);
                        if (y < 0.0) return setError(parser, 2, 213);
                        hyd->Dcost = y;
                        return 0;
                    }
                    else {
                        if (::epanet::match(parser->Tokens[n - 2], ::epanet::w_PATTERN)) {
                            // setting a price pattern
                            AUTO patName = parser->Tokens[n - 1];
                            cweeSharedPtr<cweeBalancedPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>> costPat;
                            {
                                costPat = FindOrAddPattern<DefaultUnits<value_t::_ENERGY_PRICE_>::unit>(pr, "Pattern_" + patName);
                            }

                            if (::epanet::match(parser->Tokens[0], ::epanet::w_GLOBAL)) {
                                // for all pumps, set their cost pattern to this if their pattern was empty
                                for (auto& x : net->pumps.HashList()) {
                                    AUTO p = net->pumps.Find(x);
                                    if (p && !p->EnergyPrice) {
                                        p->EnergyPrice = costPat;
                                    }
                                }
                                return 0;
                            }
                            else if (::epanet::match(parser->Tokens[0], ::epanet::w_PUMP)) {
                                // for this pump, set its cost pattern to this
                                AUTO p = net->Find<asset_t::PUMP>(parser->Tokens[1]);
                                if (p) {
                                    p->EnergyPrice = costPat;
                                }
                                return 0;
                            }
                        }
                        else if (::epanet::match(parser->Tokens[n - 2], ::epanet::w_PRICE)) {
                            // setting a price multiplier
                            if (!::epanet::getfloat(parser->Tokens[n - 1], &y)) return setError(parser, n - 1, 202);
                            if (::epanet::match(parser->Tokens[0], ::epanet::w_GLOBAL)) {
                                for (auto& x : net->pumps.HashList()) {
                                    AUTO p = net->pumps.Find(x);
                                    if (p) {
                                        p->EnergyPriceMultiplier = y;
                                    }
                                }
                                return 0;
                            }
                            else if (::epanet::match(parser->Tokens[0], ::epanet::w_PUMP)) {
                                AUTO p = net->Find<asset_t::PUMP>(parser->Tokens[1]);
                                if (p) {
                                    p->EnergyPriceMultiplier = y;
                                }
                                return 0;
                            }
                        }
                        else if (::epanet::match(parser->Tokens[n - 2], ::epanet::w_EFFIC)) {
                            // setting an efficiency
                            if (::epanet::getfloat(parser->Tokens[n - 1], &y)) {
                                // setting a default percent efficiency
                                if (::epanet::match(parser->Tokens[0], ::epanet::w_GLOBAL)) {
                                    for (auto& x : net->pumps.HashList()) {
                                        AUTO p = net->pumps.Find(x);
                                        if (p) {
                                            p->AverageEfficiencyPercent = y;
                                        }
                                    }
                                    return 0;
                                }
                                else if (::epanet::match(parser->Tokens[0], ::epanet::w_PUMP)) {
                                    AUTO p = net->Find<asset_t::PUMP>(parser->Tokens[1]);
                                    if (p) {
                                        p->AverageEfficiencyPercent = y;
                                    }
                                    return 0;
                                }
                            }
                            else {
                                // setting an efficiency curve
                                AUTO patName = parser->Tokens[n - 1];
                                cweeSharedPtr<cweeBalancedPattern<scalar_t, DefaultUnits<value_t::_DEMAND_>::unit>> effPat;
                                {
                                    effPat = FindOrAddPattern<scalar_t, DefaultUnits<value_t::_DEMAND_>::unit>(pr, "Curve_" + patName);
                                }

                                if (::epanet::match(parser->Tokens[0], ::epanet::w_GLOBAL)) {
                                    // for all pumps, set their cost pattern to this if their pattern was empty
                                    for (auto& x : net->pumps.HashList()) {
                                        AUTO p = net->pumps.Find(x);
                                        if (p && !p->EfficiencyCurve) {
                                            p->EfficiencyCurve = effPat;
                                        }
                                    }
                                    return 0;
                                }
                                else if (::epanet::match(parser->Tokens[0], ::epanet::w_PUMP)) {
                                    // for this pump, set its cost pattern to this
                                    AUTO p = net->Find<asset_t::PUMP>(parser->Tokens[1]);
                                    if (p) {
                                        p->EfficiencyCurve = effPat;
                                    }
                                    return 0;
                                }
                            }
                        }                        
                    }
                    return 201; // could not parse
                };
                static int   timedata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int n;
                    long t;
                    double y;

                    n = parser->Tokens.Num() - 1;
                    if (n < 1) return 201;

                    // Check if setting report time statistic flag
                    if (::epanet::match(parser->Tokens[0], ::epanet::w_STATISTIC))
                    {
                        /*
                        if (::epanet::match(parser->Tokens[n], ::epanet::w_NONE))  rpt->Tstatflag = SERIES;
                        else if (::epanet::match(parser->Tokens[n], ::epanet::w_NO))    rpt->Tstatflag = SERIES;
                        else if (::epanet::match(parser->Tokens[n], ::epanet::w_AVG))   rpt->Tstatflag = AVG;
                        else if (::epanet::match(parser->Tokens[n], ::epanet::w_MIN))   rpt->Tstatflag = MIN;
                        else if (::epanet::match(parser->Tokens[n], ::epanet::w_MAX))   rpt->Tstatflag = MAX;
                        else if (::epanet::match(parser->Tokens[n], ::epanet::w_RANGE)) rpt->Tstatflag = RANGE;
                        else return setError(parser, n, 213);
                        */
                        return 0;
                    }

                    // Convert text time value to numerical value in seconds
                    // Examples:
                    //    5           = 5 * 3600 sec
                    //    5 MINUTES   = 5 * 60   sec
                    //    13:50       = 13*3600 + 50*60 sec
                    //    1:50 pm     = (12+1)*3600 + 50*60 sec
                    if (!::epanet::getfloat(parser->Tokens[n], &y))
                    {
                        if ((y = ::epanet::hour(parser->Tokens[n], (char*)"")) < 0.0)
                        {
                            if ((y = ::epanet::hour(parser->Tokens[n - 1], parser->Tokens[n])) < 0.0)
                            {
                                return setError(parser, n - 1, 213);
                            }
                        }
                    }
                    t = (long)(3600.0 * y + 0.5);

                    /// Process the value assigned to the matched parameter
                    if (::epanet::match(parser->Tokens[0], ::epanet::w_DURATION))  time->Dur = t;
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_HYDRAULIC)) time->Hstep = t;
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_QUALITY))  time->Qstep = t;
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_RULE))      time->Rulestep = t;
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_MINIMUM))   return 0; // Not used anymore
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_PATTERN))
                    {
                        if (::epanet::match(parser->Tokens[1], ::epanet::w_TIME))  time->Pstep = t;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_START)) time->Pstart = t;
                        else return setError(parser, 1, 213);
                    }
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_REPORT))
                    {
                        if (::epanet::match(parser->Tokens[1], ::epanet::w_TIME))  time->Rstep = t;
                        else if (::epanet::match(parser->Tokens[1], ::epanet::w_START)) time->Rstart = t;
                        else return setError(parser, 1, 213);
                    }
                    else if (::epanet::match(parser->Tokens[0], ::epanet::w_START)) time->Tstart = t % ::epanet::SECperDAY;
                    else return setError(parser, 0, 213);
                    return 0;
                };
                static int   mixingdata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int    i,     // Tank index
                        j,     // Node index
                        m,     // Type of mixing model
                        n;     // Number of data items
                    double v;     // Mixing zone volume fraction

                    // Check for valid data




                    if (net->nodes.Num() == 0) return setError(parser, 0, 203);
                    n = parser->Tokens.Num();
                    if (n < 2) return 0;

                    AUTO node = net->Find<asset_t::RESERVOIR>(parser->Tokens[0]);
                    if (!node) return setError(parser, 0, 203);

                    if ((m = ::epanet::findmatch(parser->Tokens[1], ::epanet::MixTxt)) < 0) return setError(parser, 1, 213);

                    // Find mixing zone volume fraction (which can't be 0)
                    v = 1.0;
                    if ((m == ::epanet::MIX2) && (n == 3) &&
                        (!::epanet::getfloat(parser->Tokens[2], &v))) return setError(parser, 2, 202);
                    if (v == 0.0) v = 1.0;

                    // Assign mixing data to tank (return if tank is a reservoir)
                    if (node->TerminalStorage.Read()) return 0;
                    node->MixCompartmentFraction = v;
                    node->MixModel = (::epanet::MixType)m;

                    return 0;
                };
                static void  changestatus(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<cweeLink> link, ::epanet::StatusType status, double y)
                {
                    AUTO pipe = net->Find<asset_t::PIPE>(link->Name.Read());
                    AUTO valve = net->Find<asset_t::VALVE>(link->Name.Read());
                    AUTO pump = net->Find<asset_t::PUMP>(link->Name.Read());
                    
                    if (pipe || (valve && valve->valveType == valveType_t::GPV))
                    {
                        if (status != ::epanet::ACTIVE) 
                            link->GetValue<value_t::_STATUS_>()->AddUniqueValue(0, static_cast<int>(status));
                    }
                    else if (pump)
                    {
                        if (status == ::epanet::ACTIVE)
                        {
                            link->Kc_Roughness = y;
                            status = ::epanet::OPEN;
                            if (y == 0.0) status = ::epanet::CLOSED;
                        }
                        else if (status == ::epanet::OPEN) link->Kc_Roughness = 1.0;
                        link->GetValue<value_t::_STATUS_>()->AddUniqueValue(0, static_cast<int>(status));
                    }
                    else if (valve) {

                        switch (valve->valveType.Read()) {
                        case valveType_t::PRV: 
                        case valveType_t::PSV:
                        case valveType_t::PBV:
                        case valveType_t::FCV:
                        case valveType_t::TCV:
                        case valveType_t::GPV:
                            link->Kc_Roughness = y;
                            link->GetValue<value_t::_STATUS_>()->AddUniqueValue(0, static_cast<int>(status));
                            if (status != ::epanet::ACTIVE) link->Kc_Roughness = ::epanet::MISSING;
                            break;
                        default: break;
                        }
                    }
                };
                static int   statusdata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int j, n;
                    long i, i1, i2;
                    double y = 0.0;
                    char status = ::epanet::ACTIVE;

                    if (net->links.Num() == 0) return setError(parser, 0, 204);
                    n = parser->Tokens.Num() - 1;
                    if (n < 1) return 201;

                    // Check for legal status or setting
                    if (::epanet::match(parser->Tokens[n], ::epanet::w_OPEN))  status = ::epanet::OPEN;
                    else if (::epanet::match(parser->Tokens[n], ::epanet::w_CLOSED)) status = ::epanet::CLOSED;
                    else
                    {
                        if (!::epanet::getfloat(parser->Tokens[n], &y)) return setError(parser, n, 202);
                        if (y < 0.0) return setError(parser, n, 211);
                    }
                   
                    if (n == 1)  // A single link ID was supplied
                    {
                        AUTO ptr = net->FindLink(parser->Tokens[0]);
                        AUTO pipe = net->Find<asset_t::PIPE>(parser->Tokens[0]);
                        AUTO valve = net->Find<asset_t::VALVE>(parser->Tokens[0]);
                        if (!ptr) return setError(parser, 0, 204);

                        // Cannot change status of a Check Valve
                        if (pipe && pipe->CheckValve.Read()) {
                            return setError(parser, 0, 207); 
                        }

                        // Cannot change setting for a GPV
                        if (valve && valve->valveType.Read() == valveType_t::GPV && status == ::epanet::ACTIVE) {
                            return setError(parser, 0, 207);
                        }

                        changestatus(net, ptr, (::epanet::StatusType)status, y);
                    }                   
                    else if ((i1 = atol(parser->Tokens[0])) > 0 && (i2 = atol(parser->Tokens[1])) > 0)  // A range of numerical link ID's was supplied
                    {
                        for (auto& x : net->links.HashList()) {
                            AUTO ptr = net->links.Find(x);
                            if (ptr && *ptr) {
                                i = atol(ptr->Get()->Name.Read());
                                if (i >= i1 && i <= i2) {
                                    changestatus(net, *ptr, (::epanet::StatusType)status, y);
                                }
                            }
                        }
                    }                    
                    else {
                        for (auto& x : net->links.HashList()) {
                            AUTO ptr = net->links.Find(x);
                            if (ptr && *ptr) {
                                AUTO nm = ptr->Get()->Name.Read();
                                if ((strcmp(parser->Tokens[0], nm) <= 0) && (strcmp(parser->Tokens[1], nm) >= 0)) {
                                    changestatus(net, *ptr, (::epanet::StatusType)status, y);
                                }
                            }
                        }
                    }
                    return 0;
                };
                static int   emitterdata(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int j,       // Node index
                        n;       // # data items
                    double k;    // Flow coeff.

                    // Check that node exists & is a junction
                    n = parser->Tokens.Num();
                    if (n < 2) return 201;

                    // Parse emitter flow coeff.
                    AUTO ptr = net->Find<asset_t::JUNCTION>(parser->Tokens[0]);
                    if (!ptr) { return setError(parser, 0, 204); }

                    if (!::epanet::getfloat(parser->Tokens[1], &k)) return setError(parser, 1, 202);
                    if (k < 0.0) return setError(parser, 1, 209);

                    ptr->Ke = k;

                    return 0;
                };
                static int   controldata(cweeSharedPtr<Project> pr) {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    cweeSharedPtr<cweeNode> i; // Node
                    cweeSharedPtr<cweeLink> k; // Link

                    int n;                     // # data items
                    double       setting = ::epanet::MISSING,    // Link setting
                        time_d = 0.0,            // Simulation time
                        level_d = 0.0;           // Pressure or tank level
                    ::epanet::StatusType   status = ::epanet::ACTIVE;      // Link status
                    ::epanet::ControlType  ctltype;              // Control type

                    // Check for sufficient number of input tokens
                    n = parser->Tokens.Num();
                    if (n < 6) return 201;

                    // Check that controlled link exists
                    k = net->FindLink(parser->Tokens[1]);
                    if (!k) return setError(parser, 1, 204);
                    AUTO pipe = net->pipes.Find(k->Hash());
                    AUTO valve = net->valves.Find(k->Hash());
                    AUTO pump = net->pumps.Find(k->Hash());
                   
                    // Cannot control a check valve
                    if (pipe && pipe->CheckValve.Read()) return setError(parser, 1, 207);

                    // Parse control setting into a status level or numerical setting
                    if (::epanet::match(parser->Tokens[2], ::epanet::w_OPEN))
                    {
                        status = ::epanet::OPEN;
                        if (pump) setting = 1.0;
                        if (valve && valve->valveType == valveType_t::GPV)  setting = k->Kc_Roughness.Read()();
                    }
                    else if (::epanet::match(parser->Tokens[2], ::epanet::w_CLOSED))
                    {
                        status = ::epanet::CLOSED;
                        if (pump) setting = 0.0;
                        if (valve && valve->valveType == valveType_t::GPV)  setting = k->Kc_Roughness.Read()();
                    }
                    else if (valve && valve->valveType == valveType_t::GPV) return setError(parser, 1, 207);
                    else if (!::epanet::getfloat(parser->Tokens[2], &setting)) return setError(parser, 2, 202);

                    // Set status for pump in case speed setting was supplied
                    // or for pipe if numerical setting was supplied
                    if (pump || pipe)
                    {
                        if (setting != ::epanet::MISSING)
                        {
                            if (setting < 0.0)       return setError(parser, 2, 211);
                            else if (setting == 0.0) status = ::epanet::CLOSED;
                            else                     status = ::epanet::OPEN;
                        }
                    }

                    // Determine type of control
                    if (::epanet::match(parser->Tokens[4], ::epanet::w_TIME))           ctltype = ::epanet::TIMER;
                    else if (::epanet::match(parser->Tokens[4], ::epanet::w_CLOCKTIME)) ctltype = ::epanet::TIMEOFDAY;
                    else
                    {
                        if (n < 8) return 201;
                        i = net->FindNode(parser->Tokens[5]);
                        if (!i) return setError(parser, 5, 203);
                        if (::epanet::match(parser->Tokens[6], ::epanet::w_BELOW))      ctltype = ::epanet::LOWLEVEL;
                        else if (::epanet::match(parser->Tokens[6], ::epanet::w_ABOVE)) ctltype = ::epanet::HILEVEL;
                        else return setError(parser, 6, 213);
                    }

                    // Parse control level or time
                    switch (ctltype)
                    {
                    case ::epanet::TIMER:
                    case ::epanet::TIMEOFDAY:
                        if (n == 6) time_d = ::epanet::hour(parser->Tokens[5], (char*)"");
                        if (n == 7) time_d = ::epanet::hour(parser->Tokens[5], parser->Tokens[6]);
                        if (time_d < 0.0) return setError(parser, 5, 213);
                        break;
                    case ::epanet::LOWLEVEL:
                    case ::epanet::HILEVEL:
                        if (!::epanet::getfloat(parser->Tokens[7], &level_d)) return setError(parser, 7, 202);
                        break;
                    }

                    cweeSharedPtr<::cweeAsset::cweeControl_Logic> statusControl = make_cwee_shared<::cweeAsset::cweeControl_Logic>();
                    cweeSharedPtr<::cweeAsset::cweeControl_Logic> settingControl = make_cwee_shared<::cweeAsset::cweeControl_Logic>();
                    switch (ctltype) {
                    case ::epanet::TIMER: // do once X seconds after simulation starts at a specific date-time
                    case ::epanet::TIMEOFDAY: // every day after X seconds from the start of the day
                        statusControl->Rule.MakeIntoBasicLogic<_TIME_>(nullptr, Optimization_Comparison::EqualsTo , (units::time::second_t)(time_d * 3600));
                        settingControl->Rule.MakeIntoBasicLogic<_TIME_>(nullptr, Optimization_Comparison::EqualsTo, (units::time::second_t)(time_d * 3600));
                        break;
                    case ::epanet::LOWLEVEL:
                    case ::epanet::HILEVEL:                        
                        // Fill in fields of control data structure
                        switch (i->Type) {
                        case asset_t::JUNCTION: 
                            {
                                AUTO junc = net->junctions.Find(i->Hash());
                                // observing the pressure --- converting to HEAD instead. 
                                statusControl->Rule.MakeIntoBasicLogic<_HEAD_>(junc.CastReference<cweeAsset>(), ::cweeAsset::cweeControl_Logic::cweeRule::GuessOperandFromString(parser->Tokens[6]), (foot_t)(convertToUnit<head_t>(pr, level_d)() + junc->Coordinates.GetExclusive()->z));
                                settingControl->Rule.MakeIntoBasicLogic<_HEAD_>(junc.CastReference<cweeAsset>(), ::cweeAsset::cweeControl_Logic::cweeRule::GuessOperandFromString(parser->Tokens[6]), (foot_t)(convertToUnit<head_t>(pr, level_d)() + junc->Coordinates.GetExclusive()->z));
                            }
                            break;
                        case asset_t::RESERVOIR: 
                            {
                                AUTO res = net->reservoirs.Find(i->Hash());
                                if (res) {
                                    if (res->TerminalStorage.Read()) { // infinite reservoir
                                        // observing the head?
                                        statusControl->Rule.MakeIntoBasicLogic<_HEAD_>(res.CastReference<cweeAsset>(), ::cweeAsset::cweeControl_Logic::cweeRule::GuessOperandFromString(parser->Tokens[6]), convertToUnit<foot_t>(pr, level_d));
                                        settingControl->Rule.MakeIntoBasicLogic<_HEAD_>(res.CastReference<cweeAsset>(), ::cweeAsset::cweeControl_Logic::cweeRule::GuessOperandFromString(parser->Tokens[6]), convertToUnit<foot_t>(pr, level_d));
                                    }
                                    else { // storage tank
                                        // observing the level?
                                        statusControl->Rule.MakeIntoBasicLogic<_HEAD_>(res.CastReference<cweeAsset>(), ::cweeAsset::cweeControl_Logic::cweeRule::GuessOperandFromString(parser->Tokens[6]), convertToUnit<foot_t>(pr, level_d + res->Coordinates.GetExclusive()->z));
                                        settingControl->Rule.MakeIntoBasicLogic<_HEAD_>(res.CastReference<cweeAsset>(), ::cweeAsset::cweeControl_Logic::cweeRule::GuessOperandFromString(parser->Tokens[6]), convertToUnit<foot_t>(pr, level_d + res->Coordinates.GetExclusive()->z));
                                    }
                                }
                            }
                            break;
                        default: return setError(parser, 5, 203);
                        }                                   
                        break;
                    }

                    statusControl->Rule.trueAnswer = (scalar_t)status;
                    statusControl->Rule.falseAnswer = nullptr;
                    settingControl->Rule.trueAnswer = (scalar_t)setting;
                    settingControl->Rule.falseAnswer = nullptr;
                   
                    AUTO controllerP = net->Controllers.FindOrMake(k->Hash());
                    controllerP->Parent = k;
                    controllerP->StatusControllers.GetExclusive()->Append(statusControl.CastReference<::cweeAsset::cweeControl>());
                    if (setting != ::epanet::MISSING) controllerP->SettingControllers.GetExclusive()->Append(settingControl.CastReference<::cweeAsset::cweeControl>()); // otherwise there is no point in adding it...

                    return 0;
                };
                static int   ruleerrmsg(cweeSharedPtr<Project> pr, int errCode) {     
                    if (pr->parser->sharedRuleErrCode != 0) {

                        AUTO net = pr->network;
                        AUTO parser = pr->parser;
                        AUTO hyd = pr->hydraul;
                        AUTO time = pr->times;

                        int i;
                        char label[::epanet::MAXMSG + 1];
                        char msg[::epanet::MAXLINE + 1];
                        auto& Tok = parser->Tokens;

                        // Get text of error message
                        switch (errCode)
                        {
                        case 201: strcpy(msg, ::epanet::R_ERR201); break;
                        case 202: strcpy(msg, ::epanet::R_ERR202); break;
                        case 203: strcpy(msg, ::epanet::R_ERR203); break;
                        case 204: strcpy(msg, ::epanet::R_ERR204); break;
                        case 207: strcpy(msg, ::epanet::R_ERR207); break;
                        case 221: strcpy(msg, ::epanet::R_ERR221); break;
                        default: return errCode;
                        }

                        // Get label of rule being parsed
                        if (Tok.Num() > 0)
                        {
                            strncpy(label, ::epanet::t_RULE, ::epanet::MAXMSG);
                            strncat(label, " ", ::epanet::MAXMSG);
                            strncat(label, parser->sharedRuleName, ::epanet::MAXMSG);
                        }
                        else strncpy(label, ::epanet::t_RULES_SECT, ::epanet::MAXMSG);

                        // Write rule label and error message to status report
                        _snprintf(msg, ::epanet::MAXMSG, "%s", msg);
                        strncat(msg, label, ::epanet::MAXMSG);
                        strncat(msg, ":", ::epanet::MAXMSG);
                        std::cout << msg << std::endl;

                        // Write text of rule clause being parsed to status report
                        strcpy(msg, Tok[0]);
                        for (i = 1; i < Tok.Num(); i++)
                        {
                            strncat(msg, " ", ::epanet::MAXLINE);
                            strncat(msg, Tok[i], ::epanet::MAXLINE);
                        }
                        std::cout << msg << std::endl;
                    }
                    return errCode;     
                };                
                template<::epanet::Rulewords logop> static int   newpremise(cweeSharedPtr<Project> pr) {
                    static_assert(logop == ::epanet::Rulewords::r_AND || logop == ::epanet::Rulewords::r_OR, "EPAnet Premise must be in an AND or OR mode.");

                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    auto& sharedRuleConditions = parser->sharedRuleCondition;
                    auto& sharedRules = parser->sharedRules;
                    auto& state = parser->sharedRuleState;
                    auto& errCode = parser->sharedRuleErrCode;
                    auto& sharedRuleName = parser->sharedRuleName;

                    cweeSharedPtr<cweeAsset> j;
                    ::epanet::Objects i;
                    Optimization_Comparison r;
                    int k, m;
                    // int Status;
                    ::epanet::Varwords v;
                    double x;
                    auto& Tok = parser->Tokens;
                    cweeSharedPtr<::cweeAsset::RuleCondition> premise;

                    // Check for correct number of tokens
                    if (Tok.Num() != 5 && Tok.Num() != 6) return 201;

                    // Find network object & id if present
                    i = (::epanet::Objects)::epanet::findmatch(Tok[1], ::epanet::Object);
                    if (i == ::epanet::r_SYSTEM)
                    {
                        j = nullptr;
                        v = (::epanet::Varwords)::epanet::findmatch(Tok[2], ::epanet::Varword);
                        if (v != ::epanet::r_DEMAND && v != ::epanet::r_TIME && v != ::epanet::r_CLOCKTIME) return 201;
                    }
                    else
                    {
                        v = (::epanet::Varwords)::epanet::findmatch(Tok[3], ::epanet::Varword);
                        if (v < 0) return (201);
                        switch (i)
                        {
                        case ::epanet::r_NODE:
                        case ::epanet::r_JUNC:
                        case ::epanet::r_RESERV:
                        case ::epanet::r_TANK:
                            k = ::epanet::r_NODE;
                            break;
                        case ::epanet::r_LINK:
                        case ::epanet::r_PIPE:
                        case ::epanet::r_PUMP:
                        case ::epanet::r_VALVE:
                            k = ::epanet::r_LINK;
                            break;
                        default:
                            return 201;
                        }
                        i = (::epanet::Objects)k;
                        if (i == ::epanet::r_NODE)
                        {
                            AUTO nodePtr = net->FindNode(Tok[2]);
                            if (!nodePtr) return 203;
                            if (nodePtr->Type == asset_t::RESERVOIR)
                                j = net->Find<asset_t::RESERVOIR>(nodePtr->Name.Read()).CastReference<cweeAsset>();
                            else if (nodePtr->Type == asset_t::JUNCTION)
                                j = net->Find<asset_t::JUNCTION>(nodePtr->Name.Read()).CastReference<cweeAsset>();

                            if (!j) return 203;

                            switch (v)
                            {
                            case ::epanet::r_DEMAND:
                            case ::epanet::r_HEAD:
                            case ::epanet::r_GRADE:
                            case ::epanet::r_LEVEL:
                            case ::epanet::r_PRESSURE:
                                break;
                            case ::epanet::r_FILLTIME:
                            case ::epanet::r_DRAINTIME:
                                if (j->Type != asset_t::RESERVOIR) return 201;
                                break;
                            default:
                                return 201;
                            }
                        }
                        else
                        {
                            AUTO nodePtr = net->FindLink(Tok[2]);
                            if (!nodePtr) return 203;
                            if (nodePtr->Type == asset_t::PIPE)
                                j = net->Find<asset_t::PIPE>(nodePtr->Name.Read()).CastReference<cweeAsset>();
                            else if (nodePtr->Type == asset_t::PUMP)
                                j = net->Find<asset_t::PUMP>(nodePtr->Name.Read()).CastReference<cweeAsset>();
                            else if (nodePtr->Type == asset_t::VALVE)
                                j = net->Find<asset_t::VALVE>(nodePtr->Name.Read()).CastReference<cweeAsset>();
                            if (!j) return 204;

                            switch (v)
                            {
                            case ::epanet::r_FLOW:
                            case ::epanet::r_STATUS:
                            case ::epanet::r_SETTING:
                                break;
                            default:
                                return 201;
                            }
                        }
                    }

                    // Parse relational operator (r) and check for synonyms
                    if (i == ::epanet::r_SYSTEM) m = 3;
                    else m = 4;

                    r = ::cweeAsset::cweeControl_Logic::cweeRule::GuessOperandFromString(Tok[m]);

                    // Parse for status (s) or numerical value (x)
                    // s = 0;
                    x = ::epanet::MISSING;
                    if (v == ::epanet::r_TIME || v == ::epanet::r_CLOCKTIME)
                    {
                        if (Tok.Num() == 6) x = ::epanet::hour(Tok[4], Tok[5]) * 3600.0;
                        else                      x = ::epanet::hour(Tok[4], (char*)"") * 3600.0;
                        if (x < 0.0) return 202;
                    }
                    else if ((k = ::epanet::findmatch(Tok[Tok.Num() - 1], ::epanet::Value)) > ::epanet::IS_NUMBER) x = k;
                    else
                    {
                        if (!::epanet::getfloat(Tok[Tok.Num() - 1], &x))
                            return (202);
                        if (v == ::epanet::r_FILLTIME || v == ::epanet::r_DRAINTIME) x = x * 3600.0;
                    }

                    cweeSharedPtr<cweeNode> n = j.CastReference<cweeNode>();

                    switch (v) {
#pragma region Node-Types
                        // this could also be related to the system!
                    case ::epanet::Varwords::r_DEMAND:      
                        if (i == ::epanet::r_SYSTEM)        premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_DEMAND_>(net->systemwide, r, convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, x));
                        else                                premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_DEMAND_>(j, r, convertToUnit<DefaultUnits<value_t::_DEMAND_>::unit>(pr, x));
                        break;
                    case ::epanet::Varwords::r_GRADE:
                    case ::epanet::Varwords::r_HEAD:        premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_HEAD_>(j, r, convertToUnit<DefaultUnits<value_t::_HEAD_>::unit>(pr, x));
                        break;
                    case ::epanet::Varwords::r_PRESSURE:    if (!n) return 201; premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_HEAD_>(j, r, (foot_t)(convertToUnit<head_t>(pr, x)() + n->Coordinates.GetExclusive()->z));
                        break;
#pragma endregion
#pragma region Tanks-Types
                    case ::epanet::Varwords::r_LEVEL:       if (!n) return 201; premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_HEAD_>(j, r, (foot_t)(convertToUnit<DefaultUnits<value_t::_HEAD_>::unit>(pr, x)() + n->Coordinates.GetExclusive()->z));
                        break;
                    case ::epanet::Varwords::r_FILLTIME:    return 201; premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_HEAD_>(j, r, convertToUnit<DefaultUnits<value_t::_HEAD_>::unit>(pr, x)); // ???
                        break;
                    case ::epanet::Varwords::r_DRAINTIME:   return 201; premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_HEAD_>(j, r, convertToUnit<DefaultUnits<value_t::_HEAD_>::unit>(pr, x)); // ???
                        break;
#pragma endregion
#pragma region Link-Types
                    case ::epanet::Varwords::r_FLOW:        premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_FLOW_>(j, r, convertToUnit<DefaultUnits<value_t::_FLOW_>::unit>(pr, x));
                        break;
                    case ::epanet::Varwords::r_STATUS:      premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_STATUS_>(j, r, convertToUnit<DefaultUnits<value_t::_STATUS_>::unit>(pr, x > 0 ? ::epanet::OPEN : ::epanet::CLOSED));
                        break;
                    case ::epanet::Varwords::r_SETTING:     premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_SETTING_>(j, r, convertToUnit<DefaultUnits<value_t::_SETTING_>::unit>(pr, x));
                        break;
                        // this could be related to the system!
                    case ::epanet::Varwords::r_POWER:       
                        if (i == ::epanet::r_SYSTEM)        premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_ENERGY_>(net->systemwide, r, convertToUnit<DefaultUnits<value_t::_ENERGY_>::unit>(pr, x));
                        else                                premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_ENERGY_>(j, r, 0);
                        break;
#pragma endregion
#pragma region System-Types
                    case ::epanet::Varwords::r_TIME:        premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_TIME_>(nullptr, r, convertToUnit<DefaultUnits<value_t::_TIME_>::unit>(pr, x));
                        break;
                    case ::epanet::Varwords::r_CLOCKTIME:   premise = ::cweeAsset::cweeControl_Logic::cweeRule::MakeCondition<value_t::_TIME_>(nullptr, r, convertToUnit<DefaultUnits<value_t::_TIME_>::unit>(pr, x));
                        break;
#pragma endregion
                    }

                    if (premise){
                        cweeList<cweeSharedPtr<::cweeAsset::RuleCondition>>* currentRuleRow = nullptr;
                        if (sharedRuleConditions.Num() == 0) { // first time
                            currentRuleRow = &sharedRuleConditions.Alloc();
                        }
                        else if (logop == ::epanet::Rulewords::r_OR) {
                            // end the current premise AND list and start a new row (OR = new AND list)
                            currentRuleRow = &sharedRuleConditions.Alloc();
                        }
                        else {
                            // we are still working on the last row
                            currentRuleRow = &sharedRuleConditions[sharedRuleConditions.Num() - 1];
                        }
                        if (!currentRuleRow) return 201;

                        // add this premise to the row
                        currentRuleRow->Append(premise);
                    }

                    return 0;
                };
                template<::epanet::Rulewords logop> static int   newaction(cweeSharedPtr<Project> pr) {
                    static_assert(logop == ::epanet::Rulewords::r_THEN || logop == ::epanet::Rulewords::r_ELSE, "EPAnet Action must be a THEN (if true) or ELSE (if false) mode.");

                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    auto& sharedRuleConditions = parser->sharedRuleCondition;
                    auto& sharedRules = parser->sharedRules;
                    auto& state = parser->sharedRuleState;
                    auto& errCode = parser->sharedRuleErrCode;
                    auto& sharedRuleName = parser->sharedRuleName;

                    cweeSharedPtr<cweeLink> j;
                    int k, s;
                    double x;
                    auto& Tok = parser->Tokens;

                    // Check for correct number of tokens
                    if (Tok.Num() != 6) return 201;

                    // Check that link exists
                    j = net->FindLink(Tok[2]);
                    if (!j) return 204;
                    AUTO pipe = net->Find<asset_t::PIPE>(Tok[2]);
                    AUTO valve = net->Find<asset_t::VALVE>(Tok[2]);

                    // Cannot control a CV
                    if (pipe && pipe->CheckValve.Read()) return 207;

                    // Find value for status or setting
                    s = -1;
                    x = ::epanet::MISSING;
                    if ((k = ::epanet::findmatch(Tok[5], ::epanet::Value)) > ::epanet::IS_NUMBER) {
                        switch ((::epanet::Values)k) {
                        case ::epanet::Values::IS_OPEN: s = (int)::epanet::StatusType::OPEN; break;
                        case ::epanet::Values::IS_CLOSED: s = (int)::epanet::StatusType::CLOSED; break;
                        case ::epanet::Values::IS_ACTIVE: s = (int)::epanet::StatusType::ACTIVE; break;
                        }
                    }
                    else
                    {
                        if (!::epanet::getfloat(Tok[5], &x)) return 202;
                        if (x < 0.0) return 202;
                    }

                    // Cannot change setting for a GPV
                    if (x != ::epanet::MISSING && valve && valve->valveType == valveType_t::GPV) return 202;

                    // Set status for pipe in case setting was specified
                    if (x != ::epanet::MISSING && pipe)
                    {
                        if (x == 0.0) s = ::epanet::CLOSED;
                        else          s = ::epanet::OPEN;
                        x = ::epanet::MISSING;
                    }

                    // Create a new action structure
                    if (s >= 0) {
                        auto& subject_and_valueT_and_rule = sharedRules.Alloc();
                        subject_and_valueT_and_rule.get<0>() = j;
                        subject_and_valueT_and_rule.get<1>() = value_t::_STATUS_;
                        subject_and_valueT_and_rule.get<2>() = make_cwee_shared<cweeControl_Logic>();
                        if constexpr (logop == ::epanet::Rulewords::r_THEN) subject_and_valueT_and_rule.get<2>()->Rule.trueAnswer = (scalar_t)s;
                        if constexpr (logop == ::epanet::Rulewords::r_ELSE) subject_and_valueT_and_rule.get<2>()->Rule.falseAnswer = (scalar_t)s;
                    }
                    if (x != ::epanet::MISSING) {
                        auto& subject_and_valueT_and_rule = sharedRules.Alloc();
                        subject_and_valueT_and_rule.get<0>() = j;
                        subject_and_valueT_and_rule.get<1>() = value_t::_SETTING_;
                        subject_and_valueT_and_rule.get<2>() = make_cwee_shared<cweeControl_Logic>();
                        if constexpr (logop == ::epanet::Rulewords::r_THEN) subject_and_valueT_and_rule.get<2>()->Rule.trueAnswer = (scalar_t)x;
                        if constexpr (logop == ::epanet::Rulewords::r_ELSE) subject_and_valueT_and_rule.get<2>()->Rule.falseAnswer = (scalar_t)x;
                    }

                    return 0;
                };
                static void  finishProcessingRules(cweeSharedPtr<Project> pr, double priority = -cweeMath::INF) {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    auto& sharedRuleConditions = parser->sharedRuleCondition;
                    auto& sharedRules = parser->sharedRules;
                    auto& sharedRuleName = parser->sharedRuleName;

                    for (auto& subject_and_valueT_and_rule : sharedRules) {
                        AUTO subject = subject_and_valueT_and_rule.get<0>();
                        AUTO val = subject_and_valueT_and_rule.get<1>();
                        AUTO rule = subject_and_valueT_and_rule.get<2>();

                        if (subject && rule) {
                            rule->name = sharedRuleName;
                            rule->priority = priority;
                            rule->Rule.RuleConditions = sharedRuleConditions;

                            // add this rule to the list based on its priority. 
                            AUTO controlPtr = net->Controllers.FindOrMake(subject->Hash());
                            if (controlPtr) {
                                controlPtr->Parent = subject;
                                AUTO statusControls = controlPtr->StatusControllers.GetExclusive();
                                AUTO settingControls = controlPtr->SettingControllers.GetExclusive();

                                switch (val) {
                                case value_t::_STATUS_: {
                                    auto& controls = statusControls;
                                    int desiredPosition = controls->Num();
                                    for (int i = desiredPosition - 1; i >= 0; i--) {
                                        if (controls->at(i) && controls->at(i)->priority.Read() > rule->priority.Read()) desiredPosition--;
                                        else break;                                        
                                    }
                                    controls->Insert(rule.CastReference<cweeControl>(), desiredPosition);
                                } break;
                                case value_t::_SETTING_: {
                                    auto& controls = settingControls;
                                    int desiredPosition = controls->Num();
                                    for (int i = desiredPosition - 1; i >= 0; i--) {
                                        if (controls->at(i) && controls->at(i)->priority.Read() > rule->priority.Read()) desiredPosition--;
                                        else break;
                                    }
                                    controls->Insert(rule.CastReference<cweeControl>(), desiredPosition);
                                } break;
                                default: break;
                                }
                            }
                        }
                    }

                    parser->sharedRuleCondition.Clear();
                    parser->sharedRules.Clear();
                    parser->sharedRuleErrCode = 0;
                    parser->sharedRuleName.Clear();
                };
                static int   ruledata(cweeSharedPtr<Project> pr) {
                    AUTO net = pr->network;
                    AUTO parser = pr->parser;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    auto& sharedRuleConditions = parser->sharedRuleCondition;
                    auto& sharedRules = parser->sharedRules;
                    auto& state = parser->sharedRuleState;
                    auto& errCode = parser->sharedRuleErrCode;
                    auto& sharedRuleName = parser->sharedRuleName;

                    int key,      // Keyword code
                        err;

                    auto& Tok = parser->Tokens;

                    // Exit if current rule has an error
                    // if (rules->RuleState == r_ERROR) return 0;

                    // Find the key word that begins the rule statement
                    err = 0;
                    key = ::epanet::findmatch(Tok[0], ::epanet::Ruleword);
                    switch (key)
                    {
                    case -1: err = 201; break;  // Unrecognized keyword
                    case ::epanet::r_RULE:                        
                        if (Tok.Num() != 2) { err = 201; break; } // Missing the rule label

                        finishProcessingRules(pr); // clears all and (if available) submits the controls

                        state = ::epanet::r_RULE;
                        errCode = 0;
                        for (int i = 1; i < Tok.Num(); i++) sharedRuleName.AddToDelimiter(Tok[i], " ");

                        break;

                    case ::epanet::r_IF:
                        if (state != ::epanet::r_RULE) { err = 221; break; }
                        state = ::epanet::r_IF;
                        err = newpremise<::epanet::r_AND>(pr);
                        break;

                    case ::epanet::r_AND:
                        if (state == ::epanet::r_IF) err = newpremise<::epanet::r_AND>(pr);
                        else if (state == ::epanet::r_THEN || state == ::epanet::r_ELSE)
                        {
                            if (state == ::epanet::r_THEN) {
                                err = newaction<::epanet::r_THEN>(pr);
                            }
                            else if (state == ::epanet::r_ELSE) {
                                err = newaction<::epanet::r_ELSE>(pr);
                            }
                        }
                        else err = 221;
                        break;

                    case ::epanet::r_OR:
                        if (state == ::epanet::r_IF) 
                            err = newpremise<::epanet::r_OR>(pr);
                        else err = 221;
                        break;

                    case ::epanet::r_THEN:
                        if (state != ::epanet::r_IF) { err = 221; break; }
                        state = ::epanet::r_THEN;
                        err = newaction<::epanet::r_THEN>(pr);
                        break;

                    case ::epanet::r_ELSE:
                        if (state != ::epanet::r_THEN) { err = 221; break; }
                        state = ::epanet::r_ELSE;
                        err = newaction<::epanet::r_ELSE>(pr);
                        break;

                    case ::epanet::r_PRIORITY: {
                        if (state != ::epanet::r_THEN && state != ::epanet::r_ELSE) { err = 221; break; }
                        state = ::epanet::r_PRIORITY;

                        // finished up! Time to add the rules and put them all together. 
                        double priority = 0;
                        if (!::epanet::getfloat(Tok[1], &priority)) return 202;
                        finishProcessingRules(pr, priority);

                        break;
                    }
                    default:
                        err = 201;
                    }

                    // Set RuleState to r_ERROR if errors found
                    if (err)
                    {
                        state = ::epanet::r_ERROR;
                        errCode = err;
                        err = 200;
                    }
                    return err;
                };

                static int   newline(cweeSharedPtr<Project> pr, int sect, cweeStr const& line)
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
                    AUTO parser = pr->parser;
                    int n;

                    switch (sect)
                    {                    
                    case ::epanet::_OPTIONS:     return optiondata(pr);
                    case ::epanet::_TIMES:       return timedata(pr);
                    case ::epanet::_TITLE:       pr->Title.Append(line); return 0;
                    case ::epanet::_JUNCTIONS:   return juncdata(pr);
                    case ::epanet::_RESERVOIRS:  return tankdata(pr);
                    case ::epanet::_TANKS:       return tankdata(pr);
                    case ::epanet::_PIPES:       return pipedata(pr);
                    case ::epanet::_PUMPS:       return pumpdata(pr);
                    case ::epanet::_VALVES:      return valvedata(pr);
                    case ::epanet::_PATTERNS:    return patterndata(pr);
                    case ::epanet::_CURVES:      return curvedata(pr);
                    case ::epanet::_DEMANDS:     return demanddata(pr);
                    case ::epanet::_COORDS:      return coordata(pr);
                    case ::epanet::_VERTICES:    return vertexdata(pr);
                    case ::epanet::_ENERGY:      return energydata(pr);
                    case ::epanet::_MIXING:      return mixingdata(pr);
                    case ::epanet::_STATUS:      return statusdata(pr);
                    case ::epanet::_ROUGHNESS:   return 0;
                    case ::epanet::_LABELS:      return 0;
                    case ::epanet::_TAGS:        return 0;
                    case ::epanet::_BACKDROP:    return 0;
                    case ::epanet::_EMITTERS:    return emitterdata(pr);
                    case ::epanet::_CONTROLS:    return controldata(pr);
                    case ::epanet::_RULES:       return ruleerrmsg(pr, ruledata(pr));

                        /*
                    case ::epanet::_SOURCES:     return sourcedata(pr);
                    case ::epanet::_QUALITY:     return qualdata(pr);                    
                    case ::epanet::_REACTIONS:   return reactdata(pr);
                    case ::epanet::_REPORT:      return reportdata(pr);                                       
                       */ 
                    default: return 0;                    
                    }
                   
                    return 201;
                };
                /*
                static int updatepumpparams(cweeSharedPtr<Project> pr, cweeSharedPtr<cweePump> pump)
                {
                    AUTO net = pr->network;

                    int m;
                    int curveindex;
                    int npts = 0;
                    int errcode = 0;
                    double a, b, c, h0 = 0.0, h1 = 0.0, h2 = 0.0, q1 = 0.0, q2 = 0.0;

                    if (pump->PumpType == ::epanet::CONST_HP)  // Constant Hp pump
                    {
                        pump->H0 = 0.0;
                        pump->R = -8.814 * net->Link[pump->Link].Km;
                        pump->N = -1.0;
                        pump->Hmax = BIG; // No head limit
                        pump->Qmax = BIG; // No flow limit
                        pump->Q0 = 1.0;   // Init. flow = 1 cfs
                        return errcode;
                    }

                    else if (pump->Ptype == NOCURVE) // Pump curve specified
                    {
                        curveindex = pump->Hcurve;
                        if (curveindex == 0) return 226;
                        curve = &net->Curve[curveindex];
                        curve->Type = PUMP_CURVE;
                        npts = curve->Npts;

                        // Generic power function curve
                        if (npts == 1)
                        {
                            pump->Ptype = POWER_FUNC;
                            q1 = curve->X[0];
                            h1 = curve->Y[0];
                            h0 = 1.33334 * h1;
                            q2 = 2.0 * q1;
                            h2 = 0.0;
                        }

                        // 3 point curve with shutoff head
                        else if (npts == 3 && curve->X[0] == 0.0)
                        {
                            pump->Ptype = POWER_FUNC;
                            h0 = curve->Y[0];
                            q1 = curve->X[1];
                            h1 = curve->Y[1];
                            q2 = curve->X[2];
                            h2 = curve->Y[2];
                        }

                        // Custom pump curve
                        else
                        {
                            pump->Ptype = CUSTOM;
                            for (m = 1; m < npts; m++)
                            {
                                if (curve->Y[m] >= curve->Y[m - 1]) return 227;
                            }
                            pump->Qmax = curve->X[npts - 1];
                            pump->Q0 = (curve->X[0] + pump->Qmax) / 2.0;
                            pump->Hmax = curve->Y[0];
                        }

                        // Compute shape factors & limits of power function curves
                        if (pump->Ptype == POWER_FUNC)
                        {
                            if (!powercurve(h0, h1, h2, q1, q2, &a, &b, &c)) return 227;
                            else
                            {
                                pump->H0 = -a;
                                pump->R = -b;
                                pump->N = c;
                                pump->Q0 = q1;
                                pump->Qmax = pow((-a / b), (1.0 / c));
                                pump->Hmax = h0;
                            }
                        }
                    }
                    return 0;
                };
                static int getpumpparams(cweeSharedPtr<Project> pr)
                {
                    AUTO net = pr->network;
                    int errcode = 0;
                    for (auto& pumpHash : net->pumps.HashList()) {
                        AUTO pump = net->pumps.Find(pumpHash);
                        errcode = updatepumpparams(pr, pump);
                        if (errcode)
                        {
                            std::cout << cweeStr::printf("Error %d: %s %s\n", errcode, errmsg(errcode).c_str(), pump->Name.Read().c_str()) << std::endl;
                            return 200;
                        }
                    }

                    return 0;
                };
                */
            };
            static int      readCategory(cweeSharedPtr<Project> pr, ::epanet::SectionType sectionType) {
                rewind(pr->parser->InFile); // re-start the read

                AUTO net = pr->network;
                AUTO parser = pr->parser;

                char line[::epanet::MAXLINE + 1],  // Line from input data file
                    wline[::epanet::MAXLINE + 1]; // Working copy of input line
                char errmsg[::epanet::MAXMSG + 1] = "";
                int  sect, newsect,      // Data sections
                    errcode = 0,        // Error code
                    inperr = 0, errsum;     // Error code & total error count

                // Initialize input data section and error count
                sect = -1;
                errsum = 0;

                // Read each line from input file
                while (fgets(line, ::epanet::MAXLINE, parser->InFile) != NULL)
                {
                    // Make copy of line and scan for tokens
                    ::strcpy(wline, line);
                    gettokens(wline, parser->Tokens, parser->Comment);

                    // Skip blank lines and those filled with a comment
                    parser->ErrTok = -1;
                    if (parser->Tokens.Num() == 0)
                    {
                        // Store full line comment for Patterns and Curves
                        if (sect == ::epanet::_PATTERNS || sect == ::epanet::_CURVES) { parser->LineComment = parser->Comment; }
                        continue;
                    }

                    // Apply full line comment for Patterns and Curves
                    if (sect == ::epanet::_PATTERNS || sect == ::epanet::_CURVES) { parser->Comment = parser->LineComment; }
                    parser->LineComment.Clear();

                    // Check if at start of a new input section
                    if (parser->Tokens[0][0] == '[')
                    {
                        newsect = ::epanet::findmatch(parser->Tokens[0], ::epanet::SectTxt);
                        if (newsect >= 0)
                        {
                            sect = newsect;
                            if (sect == ::epanet::_END) break;
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
                            inperr = 0;
                            if (sect == sectionType) {
                                inperr = ParseLine::newline(pr, sect, line);
                            }
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
                    if (errsum == ::epanet::MAXERRS)  break;
                }

                // Check for errors
                if (errsum > 0) errcode = 200;

                return errcode;
            };

#pragma endregion
#pragma region Parser?
            static int		openFile(cweeSharedPtr<Project> pr, cweeStr const& inpFile)  {
                pr->parser->InpFname = inpFile;
                // Attempt to open input and report files
                if (strlen(inpFile) > 0)
                {
                    if ((pr->parser->InFile = fopen(inpFile, "rt")) == NULL) return 302;
                }
                return 0;
            };


            static void      powercurve(foot_t h0, foot_t h1, foot_t h2, cubic_foot_per_second_t q1, cubic_foot_per_second_t q2, foot_t& a, foot_t& b, scalar_t& c)
                /*
                **---------------------------------------------------------
                **  Input:   h0 = shutoff head
                **           h1 = design head
                **           h2 = head at max. flow
                **           q1 = design flow
                **           q2 = max. flow
                **  Output:  *a, *b, *c = pump curve coeffs. (H = a-bQ^c),
                **           Returns 1 if sucessful, 0 otherwise.
                **  Purpose: computes coeffs. for pump curve
                **----------------------------------------------------------
                */
            {
                foot_t h4, h5;
                foot_t TINY_ft = ::epanet::TINY;
                cubic_foot_per_second_t TINY_cfs = ::epanet::TINY;
                if (h0 < TINY_ft || h0 - h1 < TINY_ft || h1 - h2 < TINY_ft || q1 < TINY_cfs || q2 - q1 < TINY_cfs) return;

                a = h0;
                h4 = h0 - h1;
                h5 = h0 - h2;
                c = units::math::log(h5 / h4) / units::math::log(q2 / q1);
                if (c > 0.0) {
                    b = h4 / std::pow((double)q1, (double)c);
                }
                else {
                    b = 0;
                }
            };

            static void     adjustdata(cweeSharedPtr<Project> pr) {
                AUTO net = pr->network;
                AUTO hyd = pr->hydraul;
                //AUTO qual = &pr->quality;
                AUTO time = pr->times;
                AUTO parser = pr->parser;

                int i;
                double ucf;     // Unit conversion factor

                // Use 1 hr pattern & report time step if none specified
                if (time->Pstep <= 0_s) time->Pstep = 3600_s;
                if (time->Rstep == 0_s) time->Rstep = time->Pstep;

                // Hydraulic time step cannot be greater than pattern or report time step
                if (time->Hstep <= 0_s) time->Hstep = 3600_s;
                if (time->Hstep > time->Pstep) time->Hstep = time->Pstep;
                if (time->Hstep > time->Rstep) time->Hstep = time->Rstep;

                // Report start time cannot be greater than simulation duration
                if (time->Rstart > time->Dur) time->Rstart = 0;

                // No water quality analysis for single period run
                // if (time->Dur == 0_s) qual->Qualflag = NONE;

                // If no quality timestep, then make it 1/10 of hydraulic timestep
                if (time->Qstep == 0_s) time->Qstep = time->Hstep / (scalar_t)10;

                // If no rule time step then make it 1/10 of hydraulic time step
                // but not greater than hydraulic time step
                if (time->Rulestep == 0_s) time->Rulestep = time->Hstep / (scalar_t)10;
                time->Rulestep = units::math::fmin(time->Rulestep, time->Hstep);

                // Quality timestep cannot exceed hydraulic timestep
                time->Qstep = units::math::fmin(time->Qstep, time->Hstep);

                // If no quality tolerance, then use default values
                //if (qual->Ctol == ::epanet::MISSING)
                //{
                //    if (qual->Qualflag == ::epanet::AGE) qual->Ctol = ::epanet::AGETOL;
                //    else qual->Ctol = ::epanet::CHEMTOL;
                //}

                // Determine units system based on flow units
                switch (parser->Flowflag)
                {
                case ::epanet::LPS: // Liters/sec
                case ::epanet::LPM: // Liters/min
                case ::epanet::MLD: // megaliters/day
                case ::epanet::CMH: // cubic meters/hr
                case ::epanet::CMD: // cubic meters/day
                    parser->Unitsflag = ::epanet::SI;
                    break;
                default:
                    parser->Unitsflag = ::epanet::US;
                }

                // Revise pressure units depending on flow units
                if (parser->Unitsflag != ::epanet::SI) parser->Pressflag = ::epanet::PSI;
                else if (parser->Pressflag == ::epanet::PSI) parser->Pressflag = ::epanet::METERS;

                // Store value of viscosity & diffusivity
                ucf = 1.0;
                if (parser->Unitsflag == ::epanet::SI) ucf = ::epanet::SQR(::epanet::MperFT);
                if (hyd->Viscos == (::cweeAsset::Hydraul::kin_viscocity_t)::epanet::MISSING)
                {
                    hyd->Viscos = ::epanet::VISCOS;               // No viscosity supplied
                }
                else if (hyd->Viscos > (::cweeAsset::Hydraul::kin_viscocity_t)(1.e-3))
                {
                    hyd->Viscos = hyd->Viscos * ::epanet::VISCOS; // Multiplier supplied
                }
                else hyd->Viscos = hyd->Viscos / ucf;   // Actual value supplied
                //if (qual->Diffus == ::epanet::MISSING)
                //{
                //    qual->Diffus = ::epanet::DIFFUS;              // No viscosity supplied
                //}
                //else if (qual->Diffus > 1.e-4)
                //{
                //    qual->Diffus = qual->Diffus * ::epanet::DIFFUS; // Multiplier supplied
                //}
                //else qual->Diffus = qual->Diffus / ucf;   //  Actual value supplied

                // Set exponent in head loss equation and adjust flow-resistance tolerance.
                if (hyd->Formflag == ::epanet::HW) hyd->Hexp = 1.852;
                else hyd->Hexp = 2.0;

                // See if default reaction coeffs. apply
                for (auto& link : net->links) {
                    if (link->Type != asset_t::PIPE) continue;
                    /*
                    if (link->Kb_BulkReactionCoeff.Read() == (scalar_t)::epanet::MISSING) link->Kb_BulkReactionCoeff = qual->Kbulk;   // Bulk coeff.
                    if (link->Kw_WallReactionCoeff.Read() == (scalar_t)::epanet::MISSING) { // Wall coeff.                        
                        if (qual->Rfactor == 0.0) link->Kw_WallReactionCoeff = qual->Kwall; else // Rfactor is the pipe roughness correlation factor                        
                        if ((link->Kc_Roughness.Read() > 0.0) && (link->Diameter.Read() > 0_in)) {
                            if (hyd->Formflag == ::epanet::HW) link->Kw_WallReactionCoeff = qual->Rfactor / link->Kc_Roughness;
                            if (hyd->Formflag == ::epanet::DW)
                            {
                                link->Kw_WallReactionCoeff = qual->Rfactor / ABS(std::log(link->Kc_Roughness / link->Diameter.Read()));
                            }
                            if (hyd->Formflag == ::epanet::CM) link->Kw_WallReactionCoeff = qual->Rfactor * link->Kc_Roughness;
                        }
                        else link->Kw_WallReactionCoeff = 0.0;
                    }
                    */
                }

                for (auto& tank : net->reservoirs) {
                    if (!tank.TerminalStorage.Read()) {
                        // if (tank.Kb == ::epanet::MISSING) tank->Kb = qual->Kbulk;
                    }
                }

                // Remove QUALITY as a reporting variable if no WQ analysis
                // if (qual->Qualflag == NONE) rpt->Field[QUALITY].Enabled = FALSE;

                // multi-sample the head curve for pumps to ensure they function correctly, including implimenting the EPAnet power func
                for (auto& pump : net->pumps) {
                    cubic_foot_per_second_t x_1 = 0;
                    cubic_foot_per_second_t x_2 = 0;
                    cubic_foot_per_second_t x_3 = 0;
                    foot_t y_1 = 0;
                    foot_t y_2 = 0;
                    foot_t y_3 = 0;

                    if (pump.HeadCurve) {
                        auto& curve = *pump.HeadCurve;
                        curve.SetGranularity(curve.GetGranularity() + 110);

                        if (curve.GetNumValues() == 0) continue;
                        else if (curve.GetNumValues() == 1) {
                            auto knotSeries = curve.GetKnotSeries();

                            x_1 = knotSeries[0].first;
                            y_1 = knotSeries[0].second;

                            curve.AddUniqueValue(0, y_1 * (scalar_t)1.33f); // epanet approach
                            curve.AddUniqueValue(x_1 * (scalar_t)2.0f, 0); // epanet approach
                        }
                        else if (curve.GetNumValues() == 2) {
                            auto knotSeries = curve.GetKnotSeries();

                            x_1 = knotSeries[0].first;
                            y_1 = knotSeries[0].second;

                            x_2 = knotSeries[1].first;
                            y_2 = knotSeries[1].second;

                            // make a central point             
                            cweeUnion< cubic_foot_per_second_t, foot_t> middle;
                            cweeUnion< cubic_foot_per_second_t, foot_t> topRight;

                            middle.get<0>() = units::math::avg(x_1, x_2);
                            middle.get<1>() = units::math::avg(y_1, y_2);

                            topRight.get<0>() = units::math::fmax(x_1, x_2);
                            topRight.get<1>() = units::math::fmax(y_1, y_2);
                            
                            curve.AddUniqueValue(
                                (scalar_t)0.8 * middle.get<0>() + (scalar_t)0.2 * topRight.get<0>(),
                                (scalar_t)0.8 * middle.get<1>() + (scalar_t)0.2 * topRight.get<1>()
                            );
                        }
                        if (curve.GetNumValues() == 3) {
                            auto knotSeries = curve.GetKnotSeries();

                            x_1 = knotSeries[0].first;
                            y_1 = knotSeries[0].second;

                            x_2 = knotSeries[1].first;
                            y_2 = knotSeries[1].second;
                            
                            x_3 = knotSeries[2].first;
                            y_3 = knotSeries[2].second;

                            // power curve this  (3 values only)
                            foot_t a; foot_t b; scalar_t c;
                            powercurve(y_1, y_2, y_3, x_2, x_3, a, b, c);

                            AUTO SampleThreePointCurve = [a, b, c](cubic_foot_per_second_t q) -> foot_t { return a - b * scalar_t(std::pow((double)q, (double)c)); };

                            // delete the previous values;
                            //curve.RemoveTimes(x_1 - 1_cfs, x_3 + 1_cfs);

                            // sub-sample and create new values. 
                            cubic_foot_per_second_t q0 = 0_cfs;
                            foot_t ft0 = 0_ft;
                            for (; ft0 >= 0_ft; q0 += (((x_3 - x_1) * scalar_t(0.01)) + 1_cfs)) {
                                ft0 = SampleThreePointCurve(q0);
                                curve.AddUniqueValue(q0, ft0);                                
                            }
                        }

                        // ensure we have at least one value in the negatives for the derivatives
                        curve.AddUniqueValue(-2, curve.GetCurrentValue(-1));
                        curve.AddUniqueValue(-1, curve.GetCurrentValue(-1));
                        curve.AddUniqueValue(curve.GetTimeForValue(-1), -1);
                        curve.AddUniqueValue(curve.GetTimeForValue(-2), -2);
                    }
                }


            };

            static int      readFile(cweeSharedPtr<Project> pr)
            {
                int errcode = 0;

                // Assign default data values & reporting options
                setdefaults(pr);
               
                handleErr(readCategory(pr, ::epanet::SectionType::_TITLE));                
                handleErr(readCategory(pr, ::epanet::SectionType::_OPTIONS));
                handleErr(readCategory(pr, ::epanet::SectionType::_TIMES));
                handleErr(readCategory(pr, ::epanet::SectionType::_JUNCTIONS));
                handleErr(readCategory(pr, ::epanet::SectionType::_RESERVOIRS));
                handleErr(readCategory(pr, ::epanet::SectionType::_TANKS));                
                handleErr(readCategory(pr, ::epanet::SectionType::_PIPES));
                handleErr(readCategory(pr, ::epanet::SectionType::_PUMPS));
                handleErr(readCategory(pr, ::epanet::SectionType::_VALVES));
                handleErr(readCategory(pr, ::epanet::SectionType::_MIXING));
                handleErr(readCategory(pr, ::epanet::SectionType::_DEMANDS));
                handleErr(readCategory(pr, ::epanet::SectionType::_COORDS));
                handleErr(readCategory(pr, ::epanet::SectionType::_VERTICES));
                handleErr(readCategory(pr, ::epanet::SectionType::_ENERGY));
                handleErr(readCategory(pr, ::epanet::SectionType::_STATUS));
                handleErr(readCategory(pr, ::epanet::SectionType::_EMITTERS));

                handleErr(readCategory(pr, ::epanet::SectionType::_PATTERNS));  // where possible, put these to last so that the other sections can instantiate the patterns and their units
                handleErr(readCategory(pr, ::epanet::SectionType::_CURVES));    // where possible, put these to last so that the other sections can instantiate the patterns and their units. Curves are special patterns where the X-axis is in units of something other than time.

                handleErr(readCategory(pr, ::epanet::SectionType::_CONTROLS));
                handleErr(readCategory(pr, ::epanet::SectionType::_RULES)); ParseLine::finishProcessingRules(pr); // if the rules section was finished but the last rule was not given a priority, ensure it was processed at all. 

                // Adjust data and convert it to the internal units
                if (!errcode)  adjustdata(pr);
                //if (!errcode)  ::epanet::epanet_shared::initunits(pr);
                //handleErr(epanet_shared::inittanks(pr));
                //if (!errcode)  ::epanet::epanet_shared::convertunits(pr);
                return errcode;
            };
            static int		closeFile(cweeSharedPtr<Project> p) {
                if (p->parser->InFile != NULL)
                {
                    fclose(p->parser->InFile);
                    p->parser->InFile = NULL;
                }
                return 0;
            };

#pragma endregion
        };

        static cweeSharedPtr<Project> createproject() {
            cweeSharedPtr<Project> project = new Project();
            return project;
        };




        static void openInpFile(cweeSharedPtr<Project> p, cweeStr const& inpFile)
        {
            // Open input & report files
            details::handleErr(details::openFile(p, inpFile));

            // Read input data
            details::handleErr(details::readFile(p));

            // Close input file
            details::handleErr(details::closeFile(p));
        };



    };
};
#endif