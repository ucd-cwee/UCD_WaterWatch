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
#include "enum.h"
#include "Chaiscript_ConstexprString.h"
#include "List.h"
#include "Strings.h"
#include "SharedPtr.h"
#include "cwee_math.h"
#include "BalancedPattern.h"
#include "cweeSet.h"
#include "cweeUnitPattern.h"
#include "Engineering.h"

#pragma warning(disable : 4996)
namespace epanet {
    using namespace cwee_units;
    using squared_foot_per_second_t = units::unit_t<units::compound_unit<units::area::square_feet, units::inverse<units::time::second>>>;
    using squared_ft_p_cfs_t = units::unit_t<units::compound_unit<units::squared<units::length::feet>, units::inverse<units::flowrate::cubic_feet_per_second>>>;
    using cfs_p_ft_t = units::unit_t<units::compound_unit<units::flowrate::cubic_feet_per_second, units::inverse<units::length::feet>>>;
    using squared_cfs_p_ft_t = units::unit_t<units::squared<units::compound_unit<units::flowrate::cubic_feet_per_second, units::inverse<units::length::feet>>>>;
    using ft_per_cfs_t = units::unit_t<units::compound_unit<units::length::feet, units::inverse<units::flowrate::cubic_feet_per_second>>>;

#pragma region USING
    using SCALER = units::dimensionless::scalar_t;
    using REAL4 = float;
    using INT4 = int;
    using ceSS = chaiscript::utility::Static_String;
    using Pfloat = REAL4*;
#pragma endregion
#pragma region Constexpr
    static constexpr cubic_foot_per_second_t 
                            QZERO = 1.e-6;  // Equivalent to zero flow in cfs
    // Constants used for computing Darcy-Weisbach friction factor
    static constexpr SCALER PI = cweeMath::PI; 
    static constexpr SCALER A1 = 1000.0 * PI; 
    static constexpr SCALER A2 = 500.0 * PI; 
    static constexpr SCALER A3 = 16.0 * PI; 
    static constexpr SCALER A4 = 2.0 * PI; 
    static constexpr SCALER A8 = 4.61841319859066668690e+00;   // 5.74*(PI/4)^.9
    static constexpr SCALER A9 = -8.68588963806503655300e-01;  // -2/ln(10)
    static constexpr SCALER AA = -1.5634601348517065795e+00;   // -2*.9*2/ln(10)
    static constexpr SCALER AB = 3.28895476345399058690e-03;   // 5.74/(4000^.9)
    static constexpr SCALER AC = AA * AB; 
    static constexpr SCALER CSMALL = 1.e-6;
    static constexpr SCALER CBIG = 1.e8;
    static constexpr auto   MAXERRS = 10;      // Max. input errors reported
    static constexpr auto   MAXCOUNT = 10;   // Max. # of disconnected nodes listed
    static constexpr long   HASHTABLEMAXSIZE = 128000;
    static constexpr auto   ALLOC_BLOCK_SIZE = 64000;       /*(62*1024)*/
    static constexpr auto   NOTFOUND = 0;
    static constexpr auto   CODEVERSION = 20200;
    static constexpr auto   MAGICNUMBER = 516114521;
    static constexpr auto   ENGINE_VERSION = 201;   // Used for binary hydraulics file
    static constexpr auto   EOFMARK = 0x1A; // Use 0x04 for UNIX systems
    static constexpr auto   MAXTITLE = 3;       // Max. # title lines
    static constexpr auto   TITLELEN = 79;      // Max. # characters in a title line
    static constexpr auto   MAXID = 31;       // Max. # characters in ID name (this is very short! Want to fix, but would break current co-op with existing EPAnet files)
    static constexpr auto   MAXMSG = 255;      // Max. # characters in message text
    static constexpr auto   MAXLINE = 1024;     // Max. # characters read from input line
    static constexpr auto   MAXFNAME = 259;      // Max. # characters in file name
    static constexpr auto   MAXTOKS = 40;       // Max. items per line of input from INP files
#pragma push_macro("TRUE") // Exists as a macro already, so this is a MSVS-specific hack to allow us to make this variable and then restore the macro
#undef TRUE
    static constexpr auto   TRUE = 1;
#pragma pop_macro("TRUE")
#pragma push_macro("FALSE") // Exists as a macro already, so this is a MSVS-specific hack to allow us to make this variable and then restore the macro
#undef FALSE
    static constexpr auto   FALSE = 0;
#pragma pop_macro("FALSE")   
    static constexpr SCALER FULL = 2;
    static constexpr SCALER BIG = 1.E10;
    static constexpr SCALER TINY = 1.E-6;
    static constexpr SCALER MISSING = -1.E-7;     // Missing value indicator // was -1.E-10, but was too small for constexpr math
    static constexpr squared_foot_per_second_t
                            DIFFUS = 1.3E-8;     // Diffusivity of chlorine (sq ft/sec)
    static constexpr squared_foot_per_second_t 
                            VISCOS = 1.1E-5;     // Kinematic viscosity of water @ 20 deg C (sq ft/sec)
    static constexpr SCALER MINPDIFF = 0.1;        // PDA min. pressure difference (psi or m?)
    static constexpr ceSS   SEPSTR = ceSS(" \t\n\r");  // Token separator characters (space, tab, new line, carriage return)
    static constexpr SCALER GPMperCFS = ((SCALER)1.0) / ((gallon_per_minute_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER AFDperCFS = ((SCALER)1.0) / ((acre_foot_per_day_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER MGDperCFS = ((SCALER)1.0) / ((million_gallon_per_day_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER IMGDperCFS = ((SCALER)1.0) / ((imperial_million_gallon_per_day_t)(1) / (cubic_foot_per_second_t)(1)); // was 0.5382; // Disagreement between units??
    static constexpr SCALER LPSperCFS = ((SCALER)1.0) / ((liter_per_second_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER LPMperCFS = ((SCALER)1.0) / ((liter_per_minute_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER CMHperCFS = ((SCALER)1.0) / ((cubic_meter_per_hour_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER CMDperCFS = ((SCALER)1.0) / ((cubic_meter_per_day_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER MLDperCFS = ((SCALER)1.0) / ((megaliter_per_day_t)(1) / (cubic_foot_per_second_t)(1));
    static constexpr SCALER M3perFT3 = ((SCALER)1.0) / ((cubic_meter_t)(1) / (cubic_foot_t)(1));
    static constexpr SCALER LperFT3 = ((SCALER)1.0) / ((liter_t)(1) / (cubic_foot_t)(1));
    static constexpr SCALER MperFT = ((SCALER)1.0) / ((meter_t)(1) / (foot_t)(1));
    static constexpr SCALER PSIperFT = ((SCALER)1.0) / ((pounds_per_square_inch_t)(1) / (head_t)(1));
    static constexpr SCALER KPAperPSI = ((SCALER)1.0) / ((kilopascal_t)(1) / (pounds_per_square_inch_t)(1));
    static constexpr SCALER KWperHP = ((SCALER)1.0) / ((kilowatt_t)(1) / (horsepower_t)(1));
    static constexpr SCALER SECperDAY = ((SCALER)1.0) / ((second_t)(1) / (day_t)(1));

    static constexpr SCALER MAXITER = 200;  // Default max. # hydraulic iterations
    static constexpr SCALER HACC = 0.001;    // Default hydraulics convergence ratio
    static constexpr foot_t HTOL = 0.0005;   // Default hydraulic head tolerance (ft)
    static constexpr cubic_foot_per_second_t
                            QTOL = 0.0001;   // Default flow rate tolerance (cfs)
    static constexpr SCALER AGETOL = 0.01;   // Default water age tolerance (hrs)
    static constexpr SCALER CHEMTOL = 0.01;  // Default concentration tolerance
    static constexpr SCALER PAGESIZE = 0;    // Default uses no page breaks
    static constexpr SCALER SPGRAV = 1.0;    // Default specific gravity
    static constexpr SCALER EPUMP = 75;      // Default pump efficiency
    static constexpr ceSS   DEFPATID = ceSS("1");    // Default demand pattern ID
    static constexpr SCALER RQTOL = 1E-7;    // Default low flow resistance tolerance
    static constexpr SCALER CHECKFREQ = 2;   // Default status check frequency
    static constexpr SCALER MAXCHECK = 10;   // Default # iterations for status checks
    static constexpr SCALER DAMPLIMIT = 0;   // Default damping threshold
    static constexpr cubic_foot_per_second_t 
                            Q_STAGNANT = 0.005_gpm;     // 0.005 gpm = 1.114e-5 cfs
#pragma endregion
#pragma region DEFINES
#define ERRCODE(x) (errcode = ((errcode>100) ? (errcode) : (x)))
#pragma endregion
#pragma region inline templated functions
    template<typename T> INLINE AUTO MEMCHECK(T x) { return x == NULL ? 101 : 0; }; 
    template<typename T> INLINE void FREE(T x) { free(x); x = NULL; }; 
    template<typename T> INLINE AUTO INT(T x) { return (T)(int)(double)x; }; // integer portion of x
    template<typename T> INLINE AUTO FRAC(T x) { return x - INT(x); }; // // fractional part of x
    template<typename T> INLINE AUTO ABS(T x) { return x < (T)0 ? -x : x; }; // absolute value of x
    template<typename T, typename Z> INLINE T fMIN(T x, Z y) { return (T)(x < y ? (T)x : (T)y); }; // minimum of x and y
    template<typename T, typename Z> INLINE T fMAX(T x, Z y) { return (T)(x < y ? (T)y : (T)x); }; // maximum of x and y
    template<typename T> INLINE AUTO ROUND(T x) { return (T)cweeMath::Rint((double)x); }; 
    template<typename T, typename Z> INLINE AUTO MOD(T x, Z y) { return x % y; }; // x modulus y
    template<typename T> INLINE AUTO SQR(T x) { return x * x; }; // x-squared
    template<typename T> INLINE AUTO SGN(T x) { return x < (T)0 ? (SCALER)-1 : (SCALER)1; }; // sign of x
    template<typename T> INLINE AUTO UCHAR(T x) { return (x >= 'a' && x <= 'z') ? (x & ~32) : x; }; 
    template<typename T> INLINE AUTO log(T x) { return units::math::log((SCALER)(double)x); };
    template<typename T, typename Z> INLINE AUTO pow(T x, Z i) { return (T)cweeMath::Pow((double)x, (double)i); };
    template<typename T> INLINE AUTO sqrt(T x) { return units::math::sqrt(x); };
#pragma endregion
#pragma region Enums
    /// Size Limts
     /**
     Limits on the size of character arrays used to store ID names
     and text messages.
     */
    typedef enum {
        EN_MAXID = 31,     //!< Max. # characters in ID name
        EN_MAXMSG = 255     //!< Max. # characters in message text
    } EN_SizeLimits;

    /// Node properties
    /**
    These node properties are used with @ref EN_getnodevalue and @ref EN_setnodevalue.
    Those marked as read only are computed values that can only be retrieved.
    */
    typedef enum {
        EN_ELEVATION = 0, //!< Elevation
        EN_BASEDEMAND = 1, //!< Primary demand baseline value
        EN_PATTERN = 2, //!< Primary demand time pattern index
        EN_EMITTER = 3, //!< Emitter flow coefficient
        EN_INITQUAL = 4, //!< Initial quality
        EN_SOURCEQUAL = 5, //!< Quality source strength
        EN_SOURCEPAT = 6, //!< Quality source pattern index
        EN_SOURCETYPE = 7, //!< Quality source type (see @ref EN_SourceType)
        EN_TANKLEVEL = 8, //!< Current computed tank water level (read only)
        EN_DEMAND = 9, //!< Current computed demand (read only)
        EN_HEAD = 10, //!< Current computed hydraulic head (read only)
        EN_PRESSURE = 11, //!< Current computed pressure (read only)
        EN_QUALITY = 12, //!< Current computed quality (read only)
        EN_SOURCEMASS = 13, //!< Current computed quality source mass inflow (read only)
        EN_INITVOLUME = 14, //!< Tank initial volume (read only)
        EN_MIXMODEL = 15, //!< Tank mixing model (see @ref EN_MixingModel)
        EN_MIXZONEVOL = 16, //!< Tank mixing zone volume (read only)
        EN_TANKDIAM = 17, //!< Tank diameter
        EN_MINVOLUME = 18, //!< Tank minimum volume
        EN_VOLCURVE = 19, //!< Tank volume curve index
        EN_MINLEVEL = 20, //!< Tank minimum level
        EN_MAXLEVEL = 21, //!< Tank maximum level
        EN_MIXFRACTION = 22, //!< Tank mixing fraction
        EN_TANK_KBULK = 23, //!< Tank bulk decay coefficient
        EN_TANKVOLUME = 24, //!< Current computed tank volume (read only)
        EN_MAXVOLUME = 25, //!< Tank maximum volume (read only)
        EN_CANOVERFLOW = 26, //!< Tank can overflow (= 1) or not (= 0)
        EN_DEMANDDEFICIT = 27, //!< Amount that full demand is reduced under PDA (read only)
        EN_NODE_INCONTROL = 28  //!< Is present in any simple or rule-based control (= 1) or not (= 0)
    } EN_NodeProperty;

    /// Link properties
    /**
    These link properties are used with @ref EN_getlinkvalue and @ref EN_setlinkvalue.
    Those marked as read only are computed values that can only be retrieved.
    */
    typedef enum {
        EN_DIAMETER = 0,  //!< Pipe/valve diameter
        EN_LENGTH = 1,  //!< Pipe length
        EN_ROUGHNESS = 2,  //!< Pipe roughness coefficient
        EN_MINORLOSS = 3,  //!< Pipe/valve minor loss coefficient
        EN_INITSTATUS = 4,  //!< Initial status (see @ref EN_LinkStatusType)
        EN_INITSETTING = 5,  //!< Initial pump speed or valve setting
        EN_KBULK = 6,  //!< Bulk chemical reaction coefficient
        EN_KWALL = 7,  //!< Pipe wall chemical reaction coefficient
        EN_FLOW = 8,  //!< Current computed flow rate (read only)
        EN_VELOCITY = 9,  //!< Current computed flow velocity (read only)
        EN_HEADLOSS = 10, //!< Current computed head loss (read only)
        EN_STATUS = 11, //!< Current link status (see @ref EN_LinkStatusType)
        EN_SETTING = 12, //!< Current link setting
        EN_ENERGY = 13, //!< Current computed pump energy usage (read only)
        EN_LINKQUAL = 14, //!< Current computed link quality (read only)
        EN_LINKPATTERN = 15, //!< Pump speed time pattern index
        EN_PUMP_STATE = 16, //!< Current computed pump state (read only) (see @ref EN_PumpStateType)
        EN_PUMP_EFFIC = 17, //!< Current computed pump efficiency (read only)
        EN_PUMP_POWER = 18, //!< Pump constant power rating
        EN_PUMP_HCURVE = 19, //!< Pump head v. flow curve index
        EN_PUMP_ECURVE = 20, //!< Pump efficiency v. flow curve index
        EN_PUMP_ECOST = 21, //!< Pump average energy price
        EN_PUMP_EPAT = 22,  //!< Pump energy price time pattern index
        EN_LINK_INCONTROL = 23,  //!< Is present in any simple or rule-based control (= 1) or not (= 0)
        EN_GPV_CURVE = 24,  //!< GPV head loss v. flow curve index
        EN_VALVE_ELEC = 25  //!< Valve produces electricity from its flow/headloss
    } EN_LinkProperty;

    /// Time parameters
    /**
    These time-related options are used with @ref EN_gettimeparam and@ref EN_settimeparam.
    All times are expressed in seconds The parameters marked as read only are
    computed values that can only be retrieved.
    */
    typedef enum {
        EN_DURATION = 0,  //!< Total simulation duration
        EN_HYDSTEP = 1,  //!< Hydraulic time step
        EN_QUALSTEP = 2,  //!< Water quality time step
        EN_PATTERNSTEP = 3,  //!< Time pattern period
        EN_PATTERNSTART = 4,  //!< Time when time patterns begin
        EN_REPORTSTEP = 5,  //!< Reporting time step
        EN_REPORTSTART = 6,  //!< Time when reporting starts
        EN_RULESTEP = 7,  //!< Rule-based control evaluation time step
        EN_STATISTIC = 8,  //!< Reporting statistic code (see @ref EN_StatisticType)
        EN_PERIODS = 9,  //!< Number of reporting time periods (read only)
        EN_STARTTIME = 10, //!< Simulation starting time of day
        EN_HTIME = 11, //!< Elapsed time of current hydraulic solution (read only)
        EN_QTIME = 12, //!< Elapsed time of current quality solution (read only)
        EN_HALTFLAG = 13, //!< Flag indicating if the simulation was halted (read only)
        EN_NEXTEVENT = 14, //!< Shortest time until a tank becomes empty or full (read only)
        EN_NEXTEVENTTANK = 15  //!< Index of tank with shortest time to become empty or full (read only)
    } EN_TimeParameter;

    /// Analysis convergence statistics
    /**
    These statistics report the convergence criteria for the most current hydraulic analysis
    and the cumulative water quality mass balance error at the current simulation time. They
    can be retrieved with @ref EN_getstatistic.
    */
    typedef enum {
        EN_ITERATIONS = 0, //!< Number of hydraulic iterations taken
        EN_RELATIVEERROR = 1, //!< Sum of link flow changes / sum of link flows
        EN_MAXHEADERROR = 2, //!< Largest head loss error for links
        EN_MAXFLOWCHANGE = 3, //!< Largest flow change in links
        EN_MASSBALANCE = 4, //!< Cumulative water quality mass balance ratio
        EN_DEFICIENTNODES = 5, //!< Number of pressure deficient nodes
        EN_DEMANDREDUCTION = 6  //!< % demand reduction at pressure deficient nodes
    } EN_AnalysisStatistic;

    /// Types of network objects
    /**
    The types of objects that comprise a network model.
    */
    typedef enum {
        EN_NODE = 0,     //!< Nodes
        EN_LINK = 1,     //!< Links
        EN_TIMEPAT = 2,     //!< Time patterns
        EN_CURVE = 3,     //!< Data curves
        EN_CONTROL = 4,     //!< Simple controls
        EN_RULE = 5      //!< Control rules
    } EN_ObjectType;

    /// Types of objects to count
    /**
    These options tell @ref EN_getcount which type of object to count.
    */
    typedef enum {
        EN_NODECOUNT = 0,  //!< Number of nodes (junctions + tanks + reservoirs)
        EN_TANKCOUNT = 1,  //!< Number of tanks and reservoirs
        EN_LINKCOUNT = 2,  //!< Number of links (pipes + pumps + valves)
        EN_PATCOUNT = 3,  //!< Number of time patterns
        EN_CURVECOUNT = 4,  //!< Number of data curves
        EN_CONTROLCOUNT = 5,  //!< Number of simple controls
        EN_RULECOUNT = 6   //!< Number of rule-based controls
    } EN_CountType;

    /// Node Types
    /**
    These are the different types of nodes that can be returned by calling @ref EN_getnodetype.
    */
    typedef enum {
        EN_JUNCTION = 0,   //!< Junction node
        EN_RESERVOIR = 1,   //!< Reservoir node
        EN_TANK = 2    //!< Storage tank node
    } EN_NodeType;

    /// Link types
    /**
    These are the different types of links that can be returned by calling @ref EN_getlinktype.
    */
    typedef enum {
        EN_CVPIPE = 0,  //!< Pipe with check valve
        EN_PIPE = 1,  //!< Pipe
        EN_PUMP = 2,  //!< Pump
        EN_PRV = 3,  //!< Pressure reducing valve
        EN_PSV = 4,  //!< Pressure sustaining valve
        EN_PBV = 5,  //!< Pressure breaker valve
        EN_FCV = 6,  //!< Flow control valve
        EN_TCV = 7,  //!< Throttle control valve
        EN_GPV = 8   //!< General purpose valve
    } EN_LinkType;

    /// Link status
    /**
    One of these values is returned when @ref EN_getlinkvalue is used to retrieve a link's
    initial status ( \b EN_INITSTATUS ) or its current status ( \b EN_STATUS ). These options are
    also used with @ref EN_setlinkvalue to set values for these same properties.
    */
    typedef enum {
        EN_CLOSED = 0,
        EN_OPEN = 1
    } EN_LinkStatusType;

    /// Pump states
    /**
    One of these codes is returned when @ref EN_getlinkvalue is used to retrieve a pump's
    current operating state ( \b EN_PUMP_STATE ). \b EN_PUMP_XHEAD indicates that the pump has been
    shut down because it is being asked to deliver more than its shutoff head. \b EN_PUMP_XFLOW
    indicates that the pump is being asked to deliver more than its maximum flow.
    */
    typedef enum {
        EN_PUMP_XHEAD = 0,  //!< Pump closed - cannot supply head
        EN_PUMP_CLOSED = 2,  //!< Pump closed
        EN_PUMP_OPEN = 3,  //!< Pump open
        EN_PUMP_XFLOW = 5   //!< Pump open - cannot supply flow
    } EN_PumpStateType;

    /// Types of water quality analyses
    /**
    These are the different types of water quality analyses that EPANET can run. They
    are used with @ref EN_getqualinfo, @ref EN_getqualtype, and @ref EN_setqualtype.
    */
    typedef enum {
        EN_NONE = 0,   //!< No quality analysis
        EN_CHEM = 1,   //!< Chemical fate and transport
        EN_AGE = 2,   //!< Water age analysis
        EN_TRACE = 3,    //!< Source tracing analysis
        EN_ENERGYINTENSITY = 4    //!< Energy Intensity Model
    } EN_QualityType;

    /// Water quality source types
    /**
    These are the different types of external water quality sources that can be assigned
    to a node's \b EN_SOURCETYPE property as used by @ref EN_getnodevalue and @ref EN_setnodevalue.
    */
    typedef enum {
        EN_CONCEN = 0,   //!< Sets the concentration of external inflow entering a node
        EN_MASS = 1,   //!< Injects a given mass/minute into a node
        EN_SETPOINT = 2,   //!< Sets the concentration leaving a node to a given value
        EN_FLOWPACED = 3    //!< Adds a given value to the concentration leaving a node
    } EN_SourceType;

    /// Head loss formulas
    /**
    The available choices for the \b EN_HEADLOSSFORM option in @ref EN_getoption and
    @ref EN_setoption. They are also used for the head loss type argument in @ref EN_init.
    Each head loss formula uses a different type of roughness coefficient ( \b EN_ROUGHNESS )
    that can be set with @ref EN_setlinkvalue.
    */
    typedef enum {
        EN_HW = 0,   //!< Hazen-Williams
        EN_DW = 1,   //!< Darcy-Weisbach
        EN_CM = 2    //!< Chezy-Manning
    } EN_HeadLossType;

    /// Flow units
    /**
    These choices for flow units are used with @ref EN_getflowunits and @ref EN_setflowunits.
    They are also used for the flow units type argument in @ref EN_init. If flow units are
    expressed in US Customary units ( \b EN_CFS through \b EN_AFD ) then all other quantities are
    in US Customary units. Otherwise they are in metric units.
    */
    typedef enum {
        EN_CFS = 0,   //!< Cubic feet per second
        EN_GPM = 1,   //!< Gallons per minute
        EN_MGD = 2,   //!< Million gallons per day
        EN_IMGD = 3,   //!< Imperial million gallons per day
        EN_AFD = 4,   //!< Acre-feet per day
        EN_LPS = 5,   //!< Liters per second
        EN_LPM = 6,   //!< Liters per minute
        EN_MLD = 7,   //!< Million liters per day
        EN_CMH = 8,   //!< Cubic meters per hour
        EN_CMD = 9    //!< Cubic meters per day
    } EN_FlowUnits;

    /// Demand models
    /**
    These choices for modeling consumer demands are used with @ref EN_getdemandmodel
    and @ref EN_setdemandmodel.

    A demand driven analysis requires that a junction's full demand be supplied
    in each time period independent of how much pressure is available. A pressure
    driven analysis makes demand be a power function of pressure, up to the point
    where the full demand is met.
    */
    typedef enum {
        EN_DDA = 0,   //!< Demand driven analysis
        EN_PDA = 1    //!< Pressure driven analysis
    } EN_DemandModel;

    /// Simulation options
    /**
    These constants identify the hydraulic and water quality simulation options
    that are applied on a network-wide basis. They are accessed using the
    @ref EN_getoption and @ref EN_setoption functions.
    */
    typedef enum {
        EN_TRIALS = 0,  //!< Maximum trials allowed for hydraulic convergence
        EN_ACCURACY = 1,  //!< Total normalized flow change for hydraulic convergence
        EN_TOLERANCE = 2,  //!< Water quality tolerance
        EN_EMITEXPON = 3,  //!< Exponent in emitter discharge formula
        EN_DEMANDMULT = 4,  //!< Global demand multiplier
        EN_HEADERROR = 5,  //!< Maximum head loss error for hydraulic convergence
        EN_FLOWCHANGE = 6,  //!< Maximum flow change for hydraulic convergence
        EN_HEADLOSSFORM = 7,  //!< Head loss formula (see @ref EN_HeadLossType)
        EN_GLOBALEFFIC = 8,  //!< Global pump efficiency (percent)
        EN_GLOBALPRICE = 9,  //!< Global energy price per KWH
        EN_GLOBALPATTERN = 10, //!< Index of a global energy price pattern
        EN_DEMANDCHARGE = 11,  //!< Energy charge per max. KW usage
        EN_SP_GRAVITY = 12, //!< Specific gravity
        EN_SP_VISCOS = 13, //!< Specific viscosity (relative to water at 20 deg C)
        EN_UNBALANCED = 14, //!< Extra trials allowed if hydraulics don't converge
        EN_CHECKFREQ = 15, //!< Frequency of hydraulic status checks
        EN_MAXCHECK = 16, //!< Maximum trials for status checking
        EN_DAMPLIMIT = 17, //!< Accuracy level where solution damping begins
        EN_SP_DIFFUS = 18, //!< Specific diffusivity (relative to chlorine at 20 deg C)
        EN_BULKORDER = 19, //!< Bulk water reaction order for pipes
        EN_WALLORDER = 20, //!< Wall reaction order for pipes (either 0 or 1)
        EN_TANKORDER = 21, //!< Bulk water reaction order for tanks
        EN_CONCENLIMIT = 22  //!< Limiting concentration for growth reactions
    } EN_Option;

    /// Simple control types
    /**
    These are the different types of simple (single statement) controls that can be applied
    to network links. They are used as an argument to @ref EN_addcontrol,@ref EN_getcontrol,
    and @ref EN_setcontrol.
    */
    typedef enum {
        EN_LOWLEVEL = 0,   //!< Act when pressure or tank level drops below a setpoint
        EN_HILEVEL = 1,   //!< Act when pressure or tank level rises above a setpoint
        EN_TIMER = 2,   //!< Act at a prescribed elapsed amount of time
        EN_TIMEOFDAY = 3    //!< Act at a particular time of day
    } EN_ControlType;

    /// Reporting statistic choices
    /**
    These options determine what kind of statistical post-processing should be done on
    the time series of simulation results generated before they are reported using
    @ref EN_report. An option can be chosen by using \b STATISTIC _option_ as the argument
    to @ref EN_setreport.
    */
    typedef enum {
        EN_SERIES = 0,   //!< Report all time series points
        EN_AVERAGE = 1,   //!< Report average value over simulation period
        EN_MINIMUM = 2,   //!< Report minimum value over simulation period
        EN_MAXIMUM = 3,   //!< Report maximum value over simulation period
        EN_RANGE = 4    //!< Report maximum - minimum over simulation period
    } EN_StatisticType;

    /// Tank mixing models
    /**
    These are the different types of models that describe water quality mixing in storage tanks.
    The choice of model is accessed with the \b EN_MIXMODEL property of a Tank node using
    @ref EN_getnodevalue and @ref EN_setnodevalue.
    */
    typedef enum {
        EN_MIX1 = 0,   //!< Complete mix model
        EN_MIX2 = 1,   //!< 2-compartment model
        EN_FIFO = 2,   //!< First in, first out model
        EN_LIFO = 3    //!< Last in, first out model
    } EN_MixingModel;

    /// Hydraulic initialization options
    /**
    These options are used to initialize a new hydraulic analysis when @ref EN_initH is called.
    */
    typedef enum {
        EN_NOSAVE = 0,  //!< Don't save hydraulics; don't re-initialize flows
        EN_SAVE = 1,  //!< Save hydraulics to file, don't re-initialize flows
        EN_INITFLOW = 10, //!< Don't save hydraulics; re-initialize flows
        EN_SAVE_AND_INIT = 11  //!< Save hydraulics; re-initialize flows
    } EN_InitHydOption;

    /// Types of pump curves
    /**
    @ref EN_getpumptype returns one of these values when it is called.
    */
    typedef enum {
        EN_CONST_HP = 0,   //!< Constant horsepower
        EN_POWER_FUNC = 1,   //!< Power function
        EN_CUSTOM = 2,   //!< User-defined custom curve
        EN_NOCURVE = 3    //!< No curve
    } EN_PumpType;

    /// Types of data curves
    /**
    These are the different types of physical relationships that a data curve can
    represent as returned by calling @ref EN_getcurvetype.
    */
    typedef enum {
        EN_VOLUME_CURVE = 0,   //!< Tank volume v. depth curve
        EN_PUMP_CURVE = 1,   //!< Pump head v. flow curve
        EN_EFFIC_CURVE = 2,   //!< Pump efficiency v. flow curve
        EN_HLOSS_CURVE = 3,   //!< Valve head loss v. flow curve
        EN_GENERIC_CURVE = 4    //!< Generic curve
    } EN_CurveType;

    /// Deletion action codes
    /**
    These codes are used in @ref EN_deletenode and @ref EN_deletelink to indicate what action
    should be taken if the node or link being deleted appears in any simple or rule-based
    controls or if a deleted node has any links connected to it.
    */
    typedef enum {
        EN_UNCONDITIONAL = 0, //!< Delete all controls and connecing links
        EN_CONDITIONAL = 1  //!< Cancel object deletion if it appears in controls or has connecting links
    } EN_ActionCodeType;

    /// Status reporting levels
    /**
    These choices specify the level of status reporting written to a project's report
    file during a hydraulic analysis. The level is set using the @ref EN_setstatusreport function.
    */
    typedef enum {
        EN_NO_REPORT = 0,     //!< No status reporting
        EN_NORMAL_REPORT = 1, //!< Normal level of status reporting
        EN_FULL_REPORT = 2    //!< Full level of status reporting
    } EN_StatusReport;

    /// Network objects used in rule-based controls
    typedef enum {
        EN_R_NODE = 6,   //!< Clause refers to a node
        EN_R_LINK = 7,   //!< Clause refers to a link
        EN_R_SYSTEM = 8    //!< Clause refers to a system parameter (e.g., time)
    } EN_RuleObject;

    /// Object variables used in rule-based controls
    typedef enum {
        EN_R_DEMAND = 0,   //!< Nodal demand
        EN_R_HEAD = 1,   //!< Nodal hydraulic head
        EN_R_GRADE = 2,   //!< Nodal hydraulic grade
        EN_R_LEVEL = 3,   //!< Tank water level
        EN_R_PRESSURE = 4,   //!< Nodal pressure
        EN_R_FLOW = 5,   //!< Link flow rate
        EN_R_STATUS = 6,   //!< Link status
        EN_R_SETTING = 7,   //!< Link setting
        EN_R_POWER = 8,   //!< Pump power output
        EN_R_TIME = 9,   //!< Elapsed simulation time
        EN_R_CLOCKTIME = 10,  //!< Time of day
        EN_R_FILLTIME = 11,  //!< Time to fill a tank
        EN_R_DRAINTIME = 12   //!< Time to drain a tank
    } EN_RuleVariable;

    /// Comparison operators used in rule-based controls
    typedef enum {
        EN_R_EQ = 0,   //!< Equal to
        EN_R_NE = 1,   //!< Not equal
        EN_R_LE = 2,   //!< Less than or equal to
        EN_R_GE = 3,   //!< Greater than or equal to
        EN_R_LT = 4,   //!< Less than
        EN_R_GT = 5,   //!< Greater than
        EN_R_IS = 6,   //!< Is equal to
        EN_R_NOT = 7,   //!< Is not equal to
        EN_R_BELOW = 8,   //!< Is below
        EN_R_ABOVE = 9    //!< Is above
    } EN_RuleOperator;

    /// Link status codes used in rule-based controls
    typedef enum {
        EN_R_IS_OPEN = 1,   //!< Link is open
        EN_R_IS_CLOSED = 2,   //!< Link is closed
        EN_R_IS_ACTIVE = 3    //!< Control valve is active
    } EN_RuleStatus;

    typedef enum {
        NODE,
        LINK,
        TIMEPAT,
        CURVE,
        CONTROL,
        RULE
    } ObjectType;

    typedef enum {
        JUNCTION,
        RESERVOIR,
        TANK
    } NodeType;

    // BETTER_ENUM(LinkType, uint8_t, CVPIPE, PIPE, PUMP, PRV, PSV, PBV, FCV, TCV, GPV);

    typedef enum {
        CVPIPE,        // pipe with check valve
        PIPE,          // pipe
        PUMP,          // pump
        PRV,           // pressure reducing valve
        PSV,           // pressure sustaining valve
        PBV,           // pressure breaker valve
        FCV,           // flow control valve
        TCV,           // throttle control valve
        GPV            // general purpose valve
    } LinkType;

    typedef enum {
        USE,           // use hydraulics file from previous run
        SAVE,          // save hydraulics file after current run
        SCRATCH        // use temporary hydraulics file
    } HydFiletype;

    typedef enum {
        NONE,          // no quality analysis
        CHEM,          // analyze a chemical
        AGE,           // analyze water age
        TRACE_QUAL,         // trace % of flow from a source
        ENERGYINTENSITY
    } QualType;

    typedef enum {
        VOLUME_CURVE,  // volume curve
        PUMP_CURVE,    // pump curve
        EFFIC_CURVE,   // efficiency curve
        HLOSS_CURVE,   // head loss curve
        GENERIC_CURVE  // generic curve
    } CurveType;

    typedef enum {
        CONST_HP,      // constant horsepower
        POWER_FUNC,    // power function
        CUSTOM,        // user-defined custom curve
        NOCURVE
    } PumpType;

    typedef enum {
        CONCEN,        // inflow concentration
        MASS,          // mass inflow booster
        SETPOINT,      // setpoint booster
        FLOWPACED      // flow paced booster
    } SourceType;

    typedef enum {
        LOWLEVEL,      // act when grade below set level
        HILEVEL,       // act when grade above set level
        TIMER,         // act when set time reached
        TIMEOFDAY      // act when time of day occurs
    } ControlType;

    typedef enum {
        XHEAD,         // pump cannot deliver head (closed)
        TEMPCLOSED,    // temporarily closed
        CLOSED,        // closed
        OPEN,          // open
        ACTIVE,        // valve active (partially open)
        XFLOW,         // pump exceeds maximum flow
        XFCV,          // FCV cannot supply flow
        XPRESSURE,     // valve cannot supply pressure
        FILLING,       // tank filling
        EMPTYING,      // tank emptying
        OVERFLOWING    // tank overflowing
    } StatusType;

    typedef enum {
        HW,            // Hazen-Williams
        DW,            // Darcy-Weisbach
        CM             // Chezy-Manning
    } HeadLossType;

    typedef enum {
        US,            // US
        SI             // SI (metric)
    } UnitsType;

    typedef enum {
        CFS,           // cubic feet per second
        GPM,           // gallons per minute
        MGD,           // million gallons per day
        IMGD,          // imperial million gal. per day
        AFD,           // acre-feet per day
        LPS,           // liters per second
        LPM,           // liters per minute
        MLD,           // megaliters per day
        CMH,           // cubic meters per hour
        CMD            // cubic meters per day
    } FlowUnitsType;

    typedef enum {
        PSI,           // pounds per square inch
        KPA,           // kiloPascals
        METERS         // meters
    } PressureUnitsType;

    typedef enum {
        LOW,           // lower limit
        HI,            // upper limit
        PREC           // precision
    } RangeType;

    typedef enum {
        MIX1,          // complete mix model
        MIX2,          // 2-compartment model
        FIFO,          // first in, first out model
        LIFO           // last in, first out model
    } MixType;

    typedef enum {
        SERIES,        // point time series
        AVG,           // time-averages
        MIN,           // minimum values
        MAX,           // maximum values
        RANGE          // max - min values
    } StatisticType;

    typedef enum {
        ELEV = 0,      // nodal elevation
        DEMAND,        // nodal demand flow
        HEAD,          // nodal hydraulic head
        PRESSURE,      // nodal pressure
        QUALITY,       // nodal water quality

        LENGTH,        // link length
        DIAM,          // link diameter
        FLOW,          // link flow rate
        VELOCITY,      // link flow velocity
        HEADLOSS,      // link head loss
        LINKQUAL,      // avg. water quality in link
        STATUS,        // link status
        SETTING,       // pump/valve setting
        REACTRATE,     // avg. reaction rate in link
        FRICTION,      // link friction factor

        POWER,         // pump power output
        TIME,          // simulation time
        VOLUME,        // tank volume
        CLOCKTIME,     // simulation time of day
        FILLTIME,      // time to fill a tank
        DRAINTIME,     // time to drain a tank
        MAXVAR         // total number of variable fields
    } FieldType;

    typedef enum {
        _TITLE, _JUNCTIONS, _RESERVOIRS, _TANKS, _PIPES, _PUMPS,
        _VALVES, _CONTROLS, _RULES, _DEMANDS, _SOURCES, _EMITTERS,
        _PATTERNS, _CURVES, _QUALITY, _STATUS, _ROUGHNESS, _ENERGY,
        _REACTIONS, _MIXING, _REPORT, _TIMES, _OPTIONS,
        _COORDS, _VERTICES, _LABELS, _BACKDROP, _TAGS, _END
    } SectionType;

    typedef enum {
        STATHDR,       // hydraulic status header
        ENERHDR,       // energy usage header
        NODEHDR,       // node results header
        LINKHDR        // link results header
    } HdrType;

    typedef enum {
        NEGATIVE = -1,  // flow in reverse of pre-assigned direction
        ZERO_FLOW = 0,   // zero flow
        POSITIVE = 1    // flow in pre-assigned direction
    } FlowDirection;

    typedef enum {
        DDA,           // demand driven analysis
        PDA            // pressure driven analysis
    } DemandModelType;

    typedef enum  {
        r_RULE,
        r_IF,
        r_AND,
        r_OR,
        r_THEN,
        r_ELSE,
        r_PRIORITY,
        r_ERROR
    } Rulewords;

    typedef enum  {
        r_DEMAND,
        r_HEAD,
        r_GRADE,
        r_LEVEL,
        r_PRESSURE,
        r_FLOW,
        r_STATUS,
        r_SETTING,
        r_POWER,
        r_TIME,
        r_CLOCKTIME,
        r_FILLTIME,
        r_DRAINTIME
    } Varwords;

    typedef enum  {
        r_JUNC,
        r_RESERV,
        r_TANK,
        r_PIPE,
        r_PUMP,
        r_VALVE,
        r_NODE,
        r_LINK,
        r_SYSTEM
    } Objects;

    typedef enum  { EQ, NE, LE, GE, LT, GT, IS, NOT, BELOW, ABOVE } Operators;

    typedef enum { IS_NUMBER, IS_OPEN, IS_CLOSED, IS_ACTIVE } Values;

    typedef enum { HIGHESTRES, HIGHRES, LOWRES } HydraulicSimulationQuality;
#pragma endregion
#pragma region precompiled classes 
#pragma region MemPool
    struct MemBlock
    {
        struct MemBlock* next;   // Next block
        char* block,  // Start of block
            * free,   // Next free position in block
            * end;    // block + block size
    };
    struct Mempool
    {
        struct MemBlock* first;
        struct MemBlock* current;
    };
    class Mempool_t {
    public:
        static struct MemBlock* createMemBlock() {
            struct MemBlock* memBlock = (MemBlock*)malloc(sizeof(struct MemBlock));
            if (memBlock)
            {
                memBlock->block = (char*)malloc(ALLOC_BLOCK_SIZE * sizeof(char));
                if (memBlock->block == NULL)
                {
                    free(memBlock);
                    return NULL;
                }
                memBlock->free = memBlock->block;
                memBlock->next = NULL;
                memBlock->end = memBlock->block + ALLOC_BLOCK_SIZE;
            }
            return memBlock;
        };
        static void deleteMemBlock(struct MemBlock* memBlock) {
            free(memBlock->block);
            free(memBlock);
        };
        static struct Mempool* mempool_create() {
            struct Mempool* mempool;
            mempool = (struct Mempool*)malloc(sizeof(struct Mempool));
            if (mempool == NULL) return NULL;
            mempool->first = createMemBlock();
            mempool->current = mempool->first;
            if (mempool->first == NULL) return NULL;
            return mempool;
        };
        static void mempool_delete(struct Mempool* mempool) {
            if (mempool == NULL) return;
            while (mempool->first)
            {
                mempool->current = mempool->first->next;
                deleteMemBlock(mempool->first);
                mempool->first = mempool->current;
            }
            free(mempool);
            mempool = NULL;
        };
        static void mempool_reset(struct Mempool* mempool) {
            mempool->current = mempool->first;
            mempool->current->free = mempool->current->block;
        };
        static char* mempool_alloc(struct Mempool* mempool, size_t size) {
            char* ptr;

            /*
            **  Align to 4 byte boundary - should be ok for most machines.
            **  Change this if your machine has weird alignment requirements.
            */
            size = (size + 3) & 0xfffffffc;

            if (!mempool->current) return NULL;
            ptr = mempool->current->free;
            mempool->current->free += size;

            // Check if the current block is exhausted

            if (mempool->current->free >= mempool->current->end)
            {
                // Is the next block already allocated?

                if (mempool->current->next)
                {
                    // re-use block
                    mempool->current->next->free = mempool->current->next->block;
                    mempool->current = mempool->current->next;
                }
                else
                {
                    // extend the pool with a new block
                    mempool->current->next = createMemBlock();
                    if (!mempool->current->next) return NULL;
                    mempool->current = mempool->current->next;
                }

                // set ptr to the first location in the next block

                ptr = mempool->current->free;
                mempool->current->free += size;
            }

            // Return pointer to allocated memory

            return ptr;
        };
    };
#pragma endregion
#pragma region Hash
    typedef struct DataEntryStruct {
        char* key;
        int    data;
        struct DataEntryStruct* next;
    } DataEntry;
    using HashTable = struct DataEntryStruct*;
    class hashtable_t {
    public:
        // An entry in the hash table

        // Hash a string to an integer
        static unsigned int gethash(char* str)
        {
            unsigned int hash = 5381;
            unsigned int retHash;
            int c;
            while ((c = *str++))
            {
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
            }
            retHash = hash % HASHTABLEMAXSIZE;
            return retHash;
        };

        // Produce a duplicate string
        static char* dupstr(const char* s)
        {
            size_t size = strlen(s) + 1;
            char* p = (char*)malloc(size);
            if (p) memcpy(p, s, size);
            return p;
        };

        // Create a hash table
        static HashTable* hashtable_create()
        {
            int i;
            HashTable* ht = (HashTable*)calloc(HASHTABLEMAXSIZE, sizeof(HashTable));
            if (ht != NULL)
            {
                for (i = 0; i < HASHTABLEMAXSIZE; i++) ht[i] = NULL;
            }
            return ht;
        };

        // Insert an entry into the hash table
        static int hashtable_insert(HashTable* ht, char* key, int data)
        {
            unsigned int i = gethash(key);
            DataEntry* entry;
            if (i >= HASHTABLEMAXSIZE) return 0;
            entry = (DataEntry*)malloc(sizeof(DataEntry));
            if (entry == NULL) return(0);
            entry->key = dupstr(key);
            entry->data = data;
            entry->next = ht[i];
            ht[i] = entry;
            return 1;
        };

        // Change the hash table's data entry for a particular key
        static int hashtable_update(HashTable* ht, char* key, int new_data)
        {
            unsigned int i = gethash(key);
            DataEntry* entry;

            if (i >= HASHTABLEMAXSIZE) return NOTFOUND;
            entry = ht[i];
            while (entry != NULL)
            {
                if (strcmp(entry->key, key) == 0)
                {
                    entry->data = new_data;
                    return 1;
                }
                entry = entry->next;
            }
            return NOTFOUND;
        };

        // Delete an entry in the hash table
        static int hashtable_delete(HashTable* ht, char* key)
        {
            unsigned int i = gethash(key);
            DataEntry* entry, * preventry;

            if (i >= HASHTABLEMAXSIZE) return NOTFOUND;

            preventry = NULL;
            entry = ht[i];
            while (entry != NULL)
            {
                if (strcmp(entry->key, key) == 0)
                {
                    if (preventry == NULL) ht[i] = entry->next;
                    else preventry->next = entry->next;
                    free(entry->key);
                    free(entry);
                    return 1;
                }
                preventry = entry;
                entry = entry->next;
            }
            return NOTFOUND;
        };

        // Find the data for a particular key
        static int hashtable_find(HashTable* ht, char* key)
        {
            unsigned int i = gethash(key);
            DataEntry* entry;

            if (i >= HASHTABLEMAXSIZE) return NOTFOUND;
            entry = ht[i];
            while (entry != NULL)
            {
                if (strcmp(entry->key, key) == 0)
                {
                    return entry->data;
                }
                entry = entry->next;
            }
            return NOTFOUND;
        };

        // Find a particular key in the hash table
        static char* hashtable_findkey(HashTable* ht, char* key)
        {
            unsigned int i = gethash(key);
            DataEntry* entry;
            if (i >= HASHTABLEMAXSIZE) return NULL;
            entry = ht[i];
            while (entry != NULL)
            {
                if (strcmp(entry->key, key) == 0) return entry->key;
                entry = entry->next;
            }
            return NULL;
        };

        // Delete a hash table and free all of its memory
        static void hashtable_free(HashTable* ht)
        {
            DataEntry* entry, * nextentry;
            int i;

            for (i = 0; i < HASHTABLEMAXSIZE; i++)
            {
                entry = ht[i];
                while (entry != NULL)
                {
                    nextentry = entry->next;
                    free(entry->key);
                    free(entry);
                    entry = nextentry;
                }
                ht[i] = NULL;
            }
            free(ht);
        };
    };
#pragma endregion
#pragma region genmmd
    class genmmd_t {
    public:
        /* *************************************************************** */
        /* *************************************************************** */
        /* ****     GENMMD ..... MULTIPLE MINIMUM EXTERNAL DEGREE     **** */
        /* *************************************************************** */
        /* *************************************************************** */
        /*     AUTHOR - JOSEPH W.H. LIU */
        /*              DEPT OF COMPUTER SCIENCE, YORK UNIVERSITY. */
        /*     PURPOSE - THIS ROUTINE IMPLEMENTS THE MINIMUM DEGREE */
        /*        ALGORITHM.  IT MAKES USE OF THE IMPLICIT REPRESENTATION */
        /*        OF ELIMINATION GRAPHS BY QUOTIENT GRAPHS, AND THE */
        /*        NOTION OF INDISTINGUISHABLE NODES.  IT ALSO IMPLEMENTS */
        /*        THE MODIFICATIONS BY MULTIPLE ELIMINATION AND MINIMUM */
        /*        EXTERNAL DEGREE. */
        /*        --------------------------------------------- */
        /*        CAUTION - THE ADJACENCY VECTOR ADJNCY WILL BE */
        /*        DESTROYED. */
        /*        --------------------------------------------- */
        /*     INPUT PARAMETERS - */
        /*        NEQNS  - NUMBER OF EQUATIONS. */
        /*        (XADJ,ADJNCY) - THE ADJACENCY STRUCTURE. */
        /*        DELTA  - TOLERANCE VALUE FOR MULTIPLE ELIMINATION. */
        /*        MAXINT - MAXIMUM MACHINE REPRESENTABLE (SHORT) INTEGER */
        /*                 (ANY SMALLER ESTIMATE WILL DO) FOR MARKING */
        /*                 NODES. */
        /*     OUTPUT PARAMETERS - */
        /*        PERM   - THE MINIMUM DEGREE ORDERING. */
        /*        INVP   - THE INVERSE OF PERM. */
        /*        NOFSUB - AN UPPER BOUND ON THE NUMBER OF NONZERO */
        /*                 SUBSCRIPTS FOR THE COMPRESSED STORAGE SCHEME. */
        /*     WORKING PARAMETERS - */
        /*        DHEAD  - VECTOR FOR HEAD OF DEGREE LISTS. */
        /*        INVP   - USED TEMPORARILY FOR DEGREE FORWARD LINK. */
        /*        PERM   - USED TEMPORARILY FOR DEGREE BACKWARD LINK. */
        /*        QSIZE  - VECTOR FOR SIZE OF SUPERNODES. */
        /*        LLIST  - VECTOR FOR TEMPORARY LINKED LISTS. */
        /*        MARKER - A TEMPORARY MARKER VECTOR. */
        /*     PROGRAM SUBROUTINES - */
        /*        MMDELM, MMDINT, MMDNUM, MMDUPD. */
        /* *************************************************************** */
        static int genmmd(int* neqns, int* xadj, int* adjncy, int* invp, int* perm,
            int* delta, int* dhead, int* qsize, int* llist, int* marker,
            int* maxint, int* nofsub)
        {
            /* System generated locals */
            int i__1;

            /* Local variables */
            int mdeg = 0, ehead = 0, i = 0, mdlmt = 0, mdnode = 0;
            //extern /* Subroutine */ int mmdelm_(), mmdupd_(), mmdint_(), mmdnum_();
            int nextmd = 0, tag = 0, num = 0;


            /* *************************************************************** */

            /*         INTEGER*2  ADJNCY(1), DHEAD(1) , INVP(1)  , LLIST(1) , */
            /*     1              MARKER(1), PERM(1)  , QSIZE(1) */

            /* *************************************************************** */

                /* Parameter adjustments */
                //--marker; --llist; --qsize; --dhead; --perm; --invp; --adjncy; --xadj;

                /* Function Body */
            if (*neqns <= 0) {
                return 0;
            }

            /*        ------------------------------------------------ */
            /*        INITIALIZATION FOR THE MINIMUM DEGREE ALGORITHM. */
            /*        ------------------------------------------------ */
            *nofsub = 0;
            //mmdint_(neqns, &xadj[1], &adjncy[1], &dhead[1], &invp[1], &perm[1],
            //        &qsize[1], &llist[1], &marker[1]);
            mmdint_(neqns, xadj, adjncy, dhead, invp, perm, qsize, llist, marker);

            /*        ---------------------------------------------- */
            /*        NUM COUNTS THE NUMBER OF ORDERED NODES PLUS 1. */
            /*        ---------------------------------------------- */
            num = 1;

            /*        ----------------------------- */
            /*        ELIMINATE ALL ISOLATED NODES. */
            /*        ----------------------------- */
            nextmd = dhead[1];
        L100:
            if (nextmd <= 0) {
                goto L200;
            }
            mdnode = nextmd;
            nextmd = invp[mdnode];
            marker[mdnode] = *maxint;
            invp[mdnode] = -num;
            ++num;
            goto L100;

        L200:
            /*        ---------------------------------------- */
            /*        SEARCH FOR NODE OF THE MINIMUM DEGREE. */
            /*        MDEG IS THE CURRENT MINIMUM DEGREE; */
            /*        TAG IS USED TO FACILITATE MARKING NODES. */
            /*        ---------------------------------------- */
            if (num > *neqns) {
                goto L1000;
            }
            tag = 1;
            dhead[1] = 0;
            mdeg = 2;
        L300:
            if (dhead[mdeg] > 0) {
                goto L400;
            }
            ++mdeg;
            goto L300;
        L400:
            /*            ------------------------------------------------- */
            /*            USE VALUE OF DELTA TO SET UP MDLMT, WHICH GOVERNS */
            /*            WHEN A DEGREE UPDATE IS TO BE PERFORMED. */
            /*            ------------------------------------------------- */
            mdlmt = mdeg + *delta;
            ehead = 0;

        L500:
            mdnode = dhead[mdeg];
            if (mdnode > 0) {
                goto L600;
            }
            ++mdeg;
            if (mdeg > mdlmt) {
                goto L900;
            }
            goto L500;
        L600:
            /*                ---------------------------------------- */
            /*                REMOVE MDNODE FROM THE DEGREE STRUCTURE. */
            /*                ---------------------------------------- */
            nextmd = invp[mdnode];
            dhead[mdeg] = nextmd;
            if (nextmd > 0) {
                perm[nextmd] = -mdeg;
            }
            invp[mdnode] = -num;
            *nofsub = *nofsub + mdeg + qsize[mdnode] - 2;
            if (num + qsize[mdnode] > *neqns) {
                goto L1000;
            }
            /*                ---------------------------------------------- */
            /*                ELIMINATE MDNODE AND PERFORM QUOTIENT GRAPH */
            /*                TRANSFORMATION.  RESET TAG VALUE IF NECESSARY. */
            /*                ---------------------------------------------- */
            ++tag;
            if (tag < *maxint) {
                goto L800;
            }
            tag = 1;
            i__1 = *neqns;
            for (i = 1; i <= i__1; ++i) {
                if (marker[i] < *maxint) {
                    marker[i] = 0;
                }
                /* L700: */
            }
        L800:
            //mmdelm_(&mdnode, &xadj[1], &adjncy[1], &dhead[1], &invp[1], &perm[1],
            //        &qsize[1], &llist[1], &marker[1], maxint, &tag);
            mmdelm_(&mdnode, xadj, adjncy, dhead, invp, perm, qsize, llist,
                marker, maxint, &tag);
            num += qsize[mdnode];
            llist[mdnode] = ehead;
            ehead = mdnode;
            if (*delta >= 0) {
                goto L500;
            }
        L900:
            /*            ------------------------------------------- */
            /*            UPDATE DEGREES OF THE NODES INVOLVED IN THE */
            /*            MINIMUM DEGREE NODES ELIMINATION. */
            /*            ------------------------------------------- */
            if (num > *neqns) {
                goto L1000;
            }
            //mmdupd_(&ehead, neqns, &xadj[1], &adjncy[1], delta, &mdeg, &dhead[1],
            //        &invp[1], &perm[1], &qsize[1], &llist[1], &marker[1], maxint, &tag);
            mmdupd_(&ehead, neqns, xadj, adjncy, delta, &mdeg, dhead,
                invp, perm, qsize, llist, marker, maxint, &tag);
            goto L300;

        L1000:
            //mmdnum_(neqns, &perm[1], &invp[1], &qsize[1]);
            mmdnum_(neqns, perm, invp, qsize);

            //++marker; ++llist; ++qsize; ++dhead; ++perm; ++invp; ++adjncy; ++xadj;
            return 0;

        }; /* genmmd_ */


        static int mmdint_(int* neqns, int* xadj, int* adjncy, int* dhead, int* dforw,
            int* dbakw, int* qsize, int* llist, int* marker)
        {
            /* System generated locals */
            int i__1;

            /* Local variables */
            int ndeg = 0, node = 0, fnode = 0;


            /* *************************************************************** */

            /*         INTEGER*2  ADJNCY(1), DBAKW(1) , DFORW(1) , DHEAD(1) , */
            /*     1              LLIST(1) , MARKER(1), QSIZE(1) */

            /* *************************************************************** */

                /* Parameter adjustments */
                //--marker; --llist; --qsize; --dbakw; --dforw; --dhead; --adjncy; --xadj;

                /* Function Body */
            i__1 = *neqns;
            for (node = 1; node <= i__1; ++node) {
                dhead[node] = 0;
                qsize[node] = 1;
                marker[node] = 0;
                llist[node] = 0;
                /* L100: */
            }
            /*        ------------------------------------------ */
            /*        INITIALIZE THE DEGREE DOUBLY LINKED LISTS. */
            /*        ------------------------------------------ */
            i__1 = *neqns;
            for (node = 1; node <= i__1; ++node) {
                ndeg = xadj[node + 1] - xadj[node] + 1;
                fnode = dhead[ndeg];
                dforw[node] = fnode;
                dhead[ndeg] = node;
                if (fnode > 0) {
                    dbakw[fnode] = node;
                }
                dbakw[node] = -ndeg;
                /* L200: */
            }
            return 0;

        };

        /* *************************************************************** */
        /* *************************************************************** */
        /* **     MMDELM ..... MULTIPLE MINIMUM DEGREE ELIMINATION     *** */
        /* *************************************************************** */
        /* *************************************************************** */

        /*     AUTHOR - JOSEPH W.H. LIU */
        /*              DEPT OF COMPUTER SCIENCE, YORK UNIVERSITY. */

        /*     PURPOSE - THIS ROUTINE ELIMINATES THE NODE MDNODE OF */
        /*        MINIMUM DEGREE FROM THE ADJACENCY STRUCTURE, WHICH */
        /*        IS STORED IN THE QUOTIENT GRAPH FORMAT.  IT ALSO */
        /*        TRANSFORMS THE QUOTIENT GRAPH REPRESENTATION OF THE */
        /*        ELIMINATION GRAPH. */

        /*     INPUT PARAMETERS - */
        /*        MDNODE - NODE OF MINIMUM DEGREE. */
        /*        MAXINT - ESTIMATE OF MAXIMUM REPRESENTABLE (SHORT) */
        /*                 INTEGER. */
        /*        TAG    - TAG VALUE. */

        /*     UPDATED PARAMETERS - */
        /*        (XADJ,ADJNCY) - UPDATED ADJACENCY STRUCTURE. */
        /*        (DHEAD,DFORW,DBAKW) - DEGREE DOUBLY LINKED STRUCTURE. */
        /*        QSIZE  - SIZE OF SUPERNODE. */
        /*        MARKER - MARKER VECTOR. */
        /*        LLIST  - TEMPORARY LINKED LIST OF ELIMINATED NABORS. */

        /* *************************************************************** */
        static int mmdelm_(int* mdnode, int* xadj, int* adjncy, int* dhead, int* dforw,
            int* dbakw, int* qsize, int* llist, int* marker,
            int* maxint, int* tag)
        {
            /* System generated locals */
            int i__1, i__2;

            /* Local variables */
            int node = 0, link = 0, rloc = 0, rlmt = 0, i = 0, j = 0, nabor = 0, rnode = 0, elmnt = 0, xqnbr = 0,
                istop = 0, jstop = 0, istrt = 0, jstrt = 0, nxnode = 0, pvnode = 0, nqnbrs = 0, npv = 0;


            /* *************************************************************** */

            /*         INTEGER*2  ADJNCY(1), DBAKW(1) , DFORW(1) , DHEAD(1) , */
            /*     1              LLIST(1) , MARKER(1), QSIZE(1) */

            /* *************************************************************** */

            /*        ----------------------------------------------- */
            /*        FIND REACHABLE SET AND PLACE IN DATA STRUCTURE. */
            /*        ----------------------------------------------- */
                /* Parameter adjustments */
                //--marker; --llist; --qsize; --dbakw; --dforw; --dhead;
                //--adjncy; --xadj;

                /* Function Body */
            marker[*mdnode] = *tag;
            istrt = xadj[*mdnode];
            istop = xadj[*mdnode + 1] - 1;
            /*        ------------------------------------------------------- */
            /*        ELMNT POINTS TO THE BEGINNING OF THE LIST OF ELIMINATED */
            /*        NABORS OF MDNODE, AND RLOC GIVES THE STORAGE LOCATION */
            /*        FOR THE NEXT REACHABLE NODE. */
            /*        ------------------------------------------------------- */
            elmnt = 0;
            rloc = istrt;
            rlmt = istop;
            i__1 = istop;
            for (i = istrt; i <= i__1; ++i) {
                nabor = adjncy[i];
                if (nabor == 0) {
                    goto L300;
                }
                if (marker[nabor] >= *tag) {
                    goto L200;
                }
                marker[nabor] = *tag;
                if (dforw[nabor] < 0) {
                    goto L100;
                }
                adjncy[rloc] = nabor;
                ++rloc;
                goto L200;
            L100:
                llist[nabor] = elmnt;
                elmnt = nabor;
            L200:
                ;
            }
        L300:
            /*            ----------------------------------------------------- */
            /*            MERGE WITH REACHABLE NODES FROM GENERALIZED ELEMENTS. */
            /*            ----------------------------------------------------- */
            if (elmnt <= 0) {
                goto L1000;
            }
            adjncy[rlmt] = -elmnt;
            link = elmnt;
        L400:
            jstrt = xadj[link];
            jstop = xadj[link + 1] - 1;
            i__1 = jstop;
            for (j = jstrt; j <= i__1; ++j) {
                node = adjncy[j];
                link = -node;
                if (node < 0) {
                    goto L400;
                }
                else if (node == 0) {
                    goto L900;
                }
                else {
                    goto L500;
                }
            L500:
                if (marker[node] >= *tag || dforw[node] < 0) {
                    goto L800;
                }
                marker[node] = *tag;
                /*                            --------------------------------- */
                /*                            USE STORAGE FROM ELIMINATED NODES */
                /*                            IF NECESSARY. */
                /*                            --------------------------------- */
            L600:
                if (rloc < rlmt) {
                    goto L700;
                }
                link = -adjncy[rlmt];
                rloc = xadj[link];
                rlmt = xadj[link + 1] - 1;
                goto L600;
            L700:
                adjncy[rloc] = node;
                ++rloc;
            L800:
                ;
            }
        L900:
            elmnt = llist[elmnt];
            goto L300;
        L1000:
            if (rloc <= rlmt) {
                adjncy[rloc] = 0;
            }
            /*        -------------------------------------------------------- */
            /*        FOR EACH NODE IN THE REACHABLE SET, DO THE FOLLOWING ... */
            /*        -------------------------------------------------------- */
            link = *mdnode;
        L1100:
            istrt = xadj[link];
            istop = xadj[link + 1] - 1;
            i__1 = istop;
            for (i = istrt; i <= i__1; ++i) {
                rnode = adjncy[i];
                link = -rnode;
                if (rnode < 0) {
                    goto L1100;
                }
                else if (rnode == 0) {
                    goto L1800;
                }
                else {
                    goto L1200;
                }
            L1200:
                /*                -------------------------------------------- */
                /*                IF RNODE IS IN THE DEGREE LIST STRUCTURE ... */
                /*                -------------------------------------------- */
                pvnode = dbakw[rnode];
                if (pvnode == 0 || pvnode == -(*maxint)) {
                    goto L1300;
                }
                /*                    ------------------------------------- */
                /*                    THEN REMOVE RNODE FROM THE STRUCTURE. */
                /*                    ------------------------------------- */
                nxnode = dforw[rnode];
                if (nxnode > 0) {
                    dbakw[nxnode] = pvnode;
                }
                if (pvnode > 0) {
                    dforw[pvnode] = nxnode;
                }
                npv = -pvnode;
                if (pvnode < 0) {
                    dhead[npv] = nxnode;
                }
            L1300:
                /*                ---------------------------------------- */
                /*                PURGE INACTIVE QUOTIENT NABORS OF RNODE. */
                /*                ---------------------------------------- */
                jstrt = xadj[rnode];
                jstop = xadj[rnode + 1] - 1;
                xqnbr = jstrt;
                i__2 = jstop;
                for (j = jstrt; j <= i__2; ++j) {
                    nabor = adjncy[j];
                    if (nabor == 0) {
                        goto L1500;
                    }
                    if (marker[nabor] >= *tag) {
                        goto L1400;
                    }
                    adjncy[xqnbr] = nabor;
                    ++xqnbr;
                L1400:
                    ;
                }
            L1500:
                /*                ---------------------------------------- */
                /*                IF NO ACTIVE NABOR AFTER THE PURGING ... */
                /*                ---------------------------------------- */
                nqnbrs = xqnbr - jstrt;
                if (nqnbrs > 0) {
                    goto L1600;
                }
                /*                    ----------------------------- */
                /*                    THEN MERGE RNODE WITH MDNODE. */
                /*                    ----------------------------- */
                qsize[*mdnode] += qsize[rnode];
                qsize[rnode] = 0;
                marker[rnode] = *maxint;
                dforw[rnode] = -(*mdnode);
                dbakw[rnode] = -(*maxint);
                goto L1700;
            L1600:
                /*                -------------------------------------- */
                /*                ELSE FLAG RNODE FOR DEGREE UPDATE, AND */
                /*                ADD MDNODE AS A NABOR OF RNODE. */
                /*                -------------------------------------- */
                dforw[rnode] = nqnbrs + 1;
                dbakw[rnode] = 0;
                adjncy[xqnbr] = *mdnode;
                ++xqnbr;
                if (xqnbr <= jstop) {
                    adjncy[xqnbr] = 0;
                }

            L1700:
                ;
            }
        L1800:
            return 0;

        };

        /* *************************************************************** */
        /* *************************************************************** */
        /* *****     MMDUPD ..... MULTIPLE MINIMUM DEGREE UPDATE     ***** */
        /* *************************************************************** */
        /* *************************************************************** */
        /*     AUTHOR - JOSEPH W.H. LIU */
        /*              DEPT OF COMPUTER SCIENCE, YORK UNIVERSITY. */
        /*     PURPOSE - THIS ROUTINE UPDATES THE DEGREES OF NODES */
        /*        AFTER A MULTIPLE ELIMINATION STEP. */
        /*     INPUT PARAMETERS - */
        /*        EHEAD  - THE BEGINNING OF THE LIST OF ELIMINATED */
        /*                 NODES (I.E., NEWLY FORMED ELEMENTS). */
        /*        NEQNS  - NUMBER OF EQUATIONS. */
        /*        (XADJ,ADJNCY) - ADJACENCY STRUCTURE. */
        /*        DELTA  - TOLERANCE VALUE FOR MULTIPLE ELIMINATION. */
        /*        MAXINT - MAXIMUM MACHINE REPRESENTABLE (SHORT) */
        /*                 INTEGER. */
        /*     UPDATED PARAMETERS - */
        /*        MDEG   - NEW MINIMUM DEGREE AFTER DEGREE UPDATE. */
        /*        (DHEAD,DFORW,DBAKW) - DEGREE DOUBLY LINKED STRUCTURE. */
        /*        QSIZE  - SIZE OF SUPERNODE. */
        /*        LLIST  - WORKING LINKED LIST. */
        /*        MARKER - MARKER VECTOR FOR DEGREE UPDATE. */
        /*        TAG    - TAG VALUE. */
        /* *************************************************************** */
        static int mmdupd_(int* ehead, int* neqns, int* xadj, int* adjncy, int* delta,
            int* mdeg, int* dhead, int* dforw, int* dbakw, int* qsize,
            int* llist, int* marker, int* maxint, int* tag)
        {
            /* System generated locals */
            int i__1, i__2;

            /* Local variables */
            int node = 0, mtag = 0, link = 0, mdeg0 = 0, i = 0, j = 0, enode = 0, fnode = 0, nabor = 0, elmnt = 0,
                istop = 0, jstop = 0, q2head = 0, istrt = 0, jstrt = 0, qxhead = 0, iq2 = 0, deg = 0, deg0 = 0;


            /* *************************************************************** */

            /*         INTEGER*2  ADJNCY(1), DBAKW(1) , DFORW(1) , DHEAD(1) , */
            /*     1              LLIST(1) , MARKER(1), QSIZE(1) */

            /* *************************************************************** */

                /* Parameter adjustments */
                //--marker; --llist; --qsize; --dbakw; --dforw; --dhead;
                //--adjncy; --xadj;

                /* Function Body */
            mdeg0 = *mdeg + *delta;
            elmnt = *ehead;
        L100:
            /*            ------------------------------------------------------- */
            /*            FOR EACH OF THE NEWLY FORMED ELEMENT, DO THE FOLLOWING. */
            /*            (RESET TAG VALUE IF NECESSARY.)                         */
            /*            ------------------------------------------------------- */
            if (elmnt <= 0) {
                return 0;
            }
            mtag = *tag + mdeg0;
            if (mtag < *maxint) {
                goto L300;
            }
            *tag = 1;
            i__1 = *neqns;
            for (i = 1; i <= i__1; ++i) {
                if (marker[i] < *maxint) {
                    marker[i] = 0;
                }
                /* L200: */
            }
            mtag = *tag + mdeg0;
        L300:
            /*            --------------------------------------------- */
            /*            CREATE TWO LINKED LISTS FROM NODES ASSOCIATED */
            /*            WITH ELMNT: ONE WITH TWO NABORS (Q2HEAD) IN   */
            /*            ADJACENCY STRUCTURE, AND THE OTHER WITH MORE  */
            /*            THAN TWO NABORS (QXHEAD).  ALSO COMPUTE DEG0, */
            /*            NUMBER OF NODES IN THIS ELEMENT.              */
            /*            --------------------------------------------- */
            q2head = 0;
            qxhead = 0;
            deg0 = 0;
            link = elmnt;
        L400:
            istrt = xadj[link];
            istop = xadj[link + 1] - 1;
            i__1 = istop;
            for (i = istrt; i <= i__1; ++i) {
                enode = adjncy[i];
                link = -enode;
                if (enode < 0) {
                    goto L400;
                }
                else if (enode == 0) {
                    goto L800;
                }
                else {
                    goto L500;
                }

            L500:
                if (qsize[enode] == 0) {
                    goto L700;
                }
                deg0 += qsize[enode];
                marker[enode] = mtag;
                /*                        ---------------------------------- */
                /*                        IF ENODE REQUIRES A DEGREE UPDATE, */
                /*                        THEN DO THE FOLLOWING.             */
                /*                        ---------------------------------- */
                if (dbakw[enode] != 0) {
                    goto L700;
                }
                /*                            ---------------------------------------*/
                /*                            PLACE EITHER IN QXHEAD OR Q2HEAD LISTS.*/
                /*                            ---------------------------------------*/
                if (dforw[enode] == 2) {
                    goto L600;
                }
                llist[enode] = qxhead;
                qxhead = enode;
                goto L700;
            L600:
                llist[enode] = q2head;
                q2head = enode;
            L700:
                ;
            }
        L800:
            /*            -------------------------------------------- */
            /*            FOR EACH ENODE IN Q2 LIST, DO THE FOLLOWING. */
            /*            -------------------------------------------- */
            enode = q2head;
            iq2 = 1;
        L900:
            if (enode <= 0) {
                goto L1500;
            }
            if (dbakw[enode] != 0) {
                goto L2200;
            }
            ++(*tag);
            deg = deg0;
            /*                    ------------------------------------------ */
            /*                    IDENTIFY THE OTHER ADJACENT ELEMENT NABOR. */
            /*                    ------------------------------------------ */
            istrt = xadj[enode];
            nabor = adjncy[istrt];
            if (nabor == elmnt) {
                nabor = adjncy[istrt + 1];
            }
            /*                    ------------------------------------------------ */
            /*                    IF NABOR IS UNELIMINATED, INCREASE DEGREE COUNT. */
            /*                    ------------------------------------------------ */
            link = nabor;
            if (dforw[nabor] < 0) {
                goto L1000;
            }
            deg += qsize[nabor];
            goto L2100;
        L1000:
            /*                        -------------------------------------------- */
            /*                        OTHERWISE, FOR EACH NODE IN THE 2ND ELEMENT, */
            /*                        DO THE FOLLOWING.                            */
            /*                        -------------------------------------------- */
            istrt = xadj[link];
            istop = xadj[link + 1] - 1;
            i__1 = istop;
            for (i = istrt; i <= i__1; ++i) {
                node = adjncy[i];
                link = -node;
                if (node == enode) {
                    goto L1400;
                }
                if (node < 0) {
                    goto L1000;
                }
                else if (node == 0) {
                    goto L2100;
                }
                else {
                    goto L1100;
                }

            L1100:
                if (qsize[node] == 0) {
                    goto L1400;
                }
                if (marker[node] >= *tag) {
                    goto L1200;
                }
                /*                        ------------------------------------- */
                /*                        CASE WHEN NODE IS NOT YET CONSIDERED. */
                /*                        ------------------------------------- */
                marker[node] = *tag;
                deg += qsize[node];
                goto L1400;
            L1200:
                /*                        ---------------------------------------- */
                /*                        CASE WHEN NODE IS INDISTINGUISHABLE FROM */
                /*                        ENODE.  MERGE THEM INTO A NEW SUPERNODE. */
                /*                        ---------------------------------------- */
                if (dbakw[node] != 0) {
                    goto L1400;
                }
                if (dforw[node] != 2) {
                    goto L1300;
                }
                qsize[enode] += qsize[node];
                qsize[node] = 0;
                marker[node] = *maxint;
                dforw[node] = -enode;
                dbakw[node] = -(*maxint);
                goto L1400;
            L1300:
                /*                -------------------------------------- */
                /*                CASE WHEN NODE IS OUTMATCHED BY ENODE. */
                /*                -------------------------------------- */
                if (dbakw[node] == 0) {
                    dbakw[node] = -(*maxint);
                }
            L1400:
                ;
            }
            goto L2100;
        L1500:
            /*                ------------------------------------------------ */
            /*                FOR EACH ENODE IN THE QX LIST, DO THE FOLLOWING. */
            /*                ------------------------------------------------ */
            enode = qxhead;
            iq2 = 0;
        L1600:
            if (enode <= 0) {
                goto L2300;
            }
            if (dbakw[enode] != 0) {
                goto L2200;
            }
            ++(*tag);
            deg = deg0;
            /*                        --------------------------------- */
            /*                        FOR EACH UNMARKED NABOR OF ENODE, */
            /*                        DO THE FOLLOWING.                 */
            /*                        --------------------------------- */
            istrt = xadj[enode];
            istop = xadj[enode + 1] - 1;
            i__1 = istop;
            for (i = istrt; i <= i__1; ++i) {
                nabor = adjncy[i];
                if (nabor == 0) {
                    goto L2100;
                }
                if (marker[nabor] >= *tag) {
                    goto L2000;
                }
                marker[nabor] = *tag;
                link = nabor;
                /*                                ------------------------------ */
                /*                                IF UNELIMINATED, INCLUDE IT IN */
                /*                                DEG COUNT.                     */
                /*                                ------------------------------ */
                if (dforw[nabor] < 0) {
                    goto L1700;
                }
                deg += qsize[nabor];
                goto L2000;
            L1700:
                /*                                ------------------------------- */
                /*                                IF ELIMINATED, INCLUDE UNMARKED */
                /*                                NODES IN THIS ELEMENT INTO THE  */
                /*                                DEGREE COUNT.                   */
                /*                                ------------------------------- */
                jstrt = xadj[link];
                jstop = xadj[link + 1] - 1;
                i__2 = jstop;
                for (j = jstrt; j <= i__2; ++j) {
                    node = adjncy[j];
                    link = -node;
                    if (node < 0) {
                        goto L1700;
                    }
                    else if (node == 0) {
                        goto L2000;
                    }
                    else {
                        goto L1800;
                    }

                L1800:
                    if (marker[node] >= *tag) {
                        goto L1900;
                    }
                    marker[node] = *tag;
                    deg += qsize[node];
                L1900:
                    ;
                }
            L2000:
                ;
            }
        L2100:
            /*                    ------------------------------------------- */
            /*                    UPDATE EXTERNAL DEGREE OF ENODE IN DEGREE   */
            /*                    STRUCTURE, AND MDEG (MIN DEG) IF NECESSARY. */
            /*                    ------------------------------------------- */
            deg = deg - qsize[enode] + 1;
            fnode = dhead[deg];
            dforw[enode] = fnode;
            dbakw[enode] = -deg;
            if (fnode > 0) {
                dbakw[fnode] = enode;
            }
            dhead[deg] = enode;
            if (deg < *mdeg) {
                *mdeg = deg;
            }
        L2200:
            /*                    ---------------------------------- */
            /*                    GET NEXT ENODE IN CURRENT ELEMENT. */
            /*                    ---------------------------------- */
            enode = llist[enode];
            if (iq2 == 1) {
                goto L900;
            }
            goto L1600;
        L2300:
            /*            ----------------------------- */
            /*            GET NEXT ELEMENT IN THE LIST. */
            /*            ----------------------------- */
            *tag = mtag;
            elmnt = llist[elmnt];
            goto L100;

        };


        /* *************************************************************** */
        /* *************************************************************** */
        /* *****     MMDNUM ..... MULTI MINIMUM DEGREE NUMBERING     ***** */
        /* *************************************************************** */
        /* *************************************************************** */
        /*     AUTHOR - JOSEPH W.H. LIU */
        /*              DEPT OF COMPUTER SCIENCE, YORK UNIVERSITY. */
        /*     PURPOSE - THIS ROUTINE PERFORMS THE FINAL STEP IN */
        /*        PRODUCING THE PERMUTATION AND INVERSE PERMUTATION */
        /*        VECTORS IN THE MULTIPLE ELIMINATION VERSION OF THE */
        /*        MINIMUM DEGREE ORDERING ALGORITHM. */
        /*     INPUT PARAMETERS - */
        /*        NEQNS  - NUMBER OF EQUATIONS. */
        /*        QSIZE  - SIZE OF SUPERNODES AT ELIMINATION. */
        /*     UPDATED PARAMETERS - */
        /*        INVP   - INVERSE PERMUTATION VECTOR.  ON INPUT, */
        /*                 IF QSIZE(NODE)=0, THEN NODE HAS BEEN MERGED */
        /*                 INTO THE NODE -INVP(NODE); OTHERWISE, */
        /*                 -INVP(NODE) IS ITS INVERSE LABELLING. */
        /*     OUTPUT PARAMETERS - */
        /*        PERM   - THE PERMUTATION VECTOR. */
        /* *************************************************************** */
        static int mmdnum_(int* neqns, int* perm, int* invp, int* qsize) {
            /* System generated locals */
            int i__1;

            /* Local variables */
            int node = 0, root = 0, nextf = 0, father = 0, nqsize = 0, num = 0;


            /* *************************************************************** */

            /*         INTEGER*2  INVP(1)  , PERM(1)  , QSIZE(1) */

            /* *************************************************************** */

                /* Parameter adjustments */
                //--qsize; --invp; --perm;

                /* Function Body */
            i__1 = *neqns;
            for (node = 1; node <= i__1; ++node) {
                nqsize = qsize[node];
                if (nqsize <= 0) {
                    perm[node] = invp[node];
                }
                if (nqsize > 0) {
                    perm[node] = -invp[node];
                }
                /* L100: */
            }
            /*        ------------------------------------------------------ */
            /*        FOR EACH NODE WHICH HAS BEEN MERGED, DO THE FOLLOWING. */
            /*        ------------------------------------------------------ */
            i__1 = *neqns;
            for (node = 1; node <= i__1; ++node) {
                if (perm[node] > 0) {
                    goto L500;
                }
                /*                ----------------------------------------- */
                /*                TRACE THE MERGED TREE UNTIL ONE WHICH HAS */
                /*                NOT BEEN MERGED, CALL IT ROOT.            */
                /*                ----------------------------------------- */
                father = node;
            L200:
                if (perm[father] > 0) {
                    goto L300;
                }
                father = -perm[father];
                goto L200;
            L300:
                /*                ----------------------- */
                /*                NUMBER NODE AFTER ROOT. */
                /*                ----------------------- */
                root = father;
                num = perm[root] + 1;
                invp[node] = -num;
                perm[root] = num;
                /*                ------------------------ */
                /*                SHORTEN THE MERGED TREE. */
                /*                ------------------------ */
                father = node;
            L400:
                nextf = -perm[father];
                if (nextf <= 0) {
                    goto L500;
                }
                perm[father] = -root;
                father = nextf;
                goto L400;
            L500:
                ;
            }
            /*        ---------------------- */
            /*        READY TO COMPUTE PERM. */
            /*        ---------------------- */
            i__1 = *neqns;
            for (node = 1; node <= i__1; ++node) {
                num = -invp[node];
                invp[node] = num;
                perm[num] = node;
                /* L600: */
            }
            return 0;

        };
    };
#pragma endregion
#pragma endregion
#pragma region "Classes"
    class Spattern {             // Time Pattern Object
    public:
        Spattern() {
            Pat.SetBoundaryType(boundary_t::BT_LOOP);
            Pat.SetInterpolationType(interpolation_t::LEFT);
        };
        cweeStr ID;              // pattern ID
        cweeStr Comment;         // pattern comment
        cweeBalancedPattern<SCALER> Pat;
    };
    using Ppattern = cweeSharedPtr<Spattern>; // Pointer to a pattern

    class Scurve {                  // Curve Object    
    public:
        Scurve() {
            Curve.SetBoundaryType(boundary_t::BT_FREE); // can extend beyond bounds, following last available angle
            Curve.SetInterpolationType(interpolation_t::SPLINE); // linear interpolation or spline? 
        };

        char        ID[MAXID + 1];  // curve ID
        cweeStr     Comment;        // curve comment
        CurveType   Type;           // curve type
        cweeBalancedPattern<SCALER, SCALER> Curve;
    };
    using Pcurve = cweeSharedPtr<Scurve>; // Pointer to a curve

    class Sdemand {            // Demand List Item    
    public:
        cubic_foot_per_second_t Base;             // baseline demand
        int    Pat;              // pattern index
        Ppattern   TimePat;    // actual pattern (shared ptr)
        char* Name;            // demand category name
    };

    class Senergy             // Energy Usage Object
    {
    public:
        hour_t TimeOnLine;       // hours pump is online
        SCALER Efficiency;       // total time wtd. efficiency
        SCALER KwHrsPerFlow;     // total kw-hrs per unit of flow
        kilowatt_hour_t 
               KwHrs;            // total kw-hrs consumed
        kilowatt_t 
               MaxKwatts;        // max. kw consumed
        Dollar_t 
               TotalCost;        // total pumping cost
        kilowatt_t
               CurrentPower;     // current pump power (kw)
        SCALER CurrentEffic;     // current pump efficiency
    };

    class Ssource             // Water Quality Source Object
    {
    public:
        SCALER     Concentration;         // base concentration/mass
        int        Pat;        // pattern index
        Ppattern   TimePat;    // actual pattern (shared ptr)
        SCALER     Smass;      // actual mass flow rate
        SourceType Type;       // type of source
    };
    using Psource = cweeSharedPtr< Ssource >; // Pointer to source object

    class Svertices              // Coordinates of a link's vertices
    {
    public:
        cweeList<cweePair<SCALER, SCALER>> Array;
    };
    using Pvertices = cweeSharedPtr<Svertices>; // Pointer to a link's vertices

    template<value_t which> struct DefaultUnits {};
    template<> struct DefaultUnits<value_t::_ANY_> { using unit = scalar_t; };
    template<> struct DefaultUnits<value_t::_FLOW_> { using unit = cubic_foot_per_second_t; };
    template<> struct DefaultUnits<value_t::_VELOCITY_> { using unit = feet_per_second_t; };
    template<> struct DefaultUnits<value_t::_HEADLOSS_> { using unit = foot_t; };
    template<> struct DefaultUnits<value_t::_STATUS_> { using unit = scalar_t; };
    template<> struct DefaultUnits<value_t::_ENERGY_> { using unit = kilowatt_t; };
    template<> struct DefaultUnits<value_t::_ENERGY_PRICE_> { using unit = Dollar_per_kilowatt_hour_t; };
    template<> struct DefaultUnits<value_t::_WATER_PRICE_> { using unit = Dollar_per_gallon_t; };
    template<> struct DefaultUnits<value_t::_COST_> { using unit = Dollar_t; };
    template<> struct DefaultUnits<value_t::_SETTING_> { using unit = scalar_t; };
    template<> struct DefaultUnits<value_t::_HEAD_> { using unit = foot_t; };
    template<> struct DefaultUnits<value_t::_DEMAND_> { using unit = cubic_foot_per_second_t; };
    template<> struct DefaultUnits<value_t::_QUALITY_> { using unit = scalar_t; };
    template<> struct DefaultUnits<value_t::_MASS_FLOW_> { using unit = scalar_t; };
    template<> struct DefaultUnits<value_t::_TIME_> { using unit = second_t; };
    template<> struct DefaultUnits<value_t::_LEVEL_> { using unit = foot_t; };
    template<> struct DefaultUnits<value_t::_EMISSION_INTENSITY_> { using unit = metric_ton_per_day_t; };

#define AddValueType(v, INTERP) { cweeBalancedPattern<typename DefaultUnits<v>::unit> pat; pat.SetInterpolationType(interpolation_t::INTERP); pat.SetBoundaryType(boundary_t::BT_CLAMPED); out->Add(cweeUnion<int, cweeAny>(static_cast<int>(v), pat)); }
    using ValuesContainerType = cweeThreadedSet<cweeUnion<int, cweeAny>, int>; 
    template <int type> class cweeAssetValueCollection {
    public:
        cweeAssetValueCollection() = delete;
        static cweeSharedPtr<ValuesContainerType> Values() {
            cweeSharedPtr<ValuesContainerType> out = make_cwee_shared<ValuesContainerType>(std::function<int(const cweeUnion<int, cweeAny>&)>([](const cweeUnion<int, cweeAny>& v)->int { return v.get<0>(); }));
            
            if constexpr (type == asset_t::ANY) {
                // AddValueType(_DEMAND_);

                { 
                    cweeBalancedPattern<typename DefaultUnits<_DEMAND_>::unit> pat;
                    pat.SetInterpolationType(interpolation_t::LEFT); 
                    pat.SetBoundaryType(boundary_t::BT_CLAMPED); 
                    out->Add(cweeUnion<int, cweeAny>(static_cast<int>(_DEMAND_), pat));
                }


                AddValueType(_ENERGY_, LINEAR);
            }
            else {
                if constexpr (type == asset_t::DMA) {
                    AddValueType(_DEMAND_, LINEAR);
                    AddValueType(_HEAD_, LINEAR);
                    AddValueType(_FLOW_, LINEAR);
                }
                else if constexpr (type == asset_t::PUMPSTATION) {
                    AddValueType(_FLOW_, LINEAR);
                    AddValueType(_ENERGY_, LINEAR);
                    AddValueType(_ENERGY_PRICE_, LEFT);
                    AddValueType(_EMISSION_INTENSITY_, LEFT);
                }
                else if constexpr (type == asset_t::JUNCTION || type == asset_t::RESERVOIR) {
                    AddValueType(_HEAD_, LINEAR);
                    AddValueType(_DEMAND_, LINEAR);
                    AddValueType(_QUALITY_, LINEAR);
                    //AddValueType(_MASS_FLOW_);
                }
                else {
                    if constexpr (type == asset_t::PIPE || type == asset_t::PUMP || type == asset_t::VALVE) {
                        AddValueType(_FLOW_, LINEAR);
                        AddValueType(_VELOCITY_, LINEAR);
                        AddValueType(_HEADLOSS_, LINEAR);
                        AddValueType(_STATUS_, LEFT);
                    }
                    if constexpr (type == asset_t::PUMP || type == asset_t::VALVE) {
                        AddValueType(_SETTING_, LEFT);
                        AddValueType(_ENERGY_, LINEAR);
                    }
                }
            }
            return out;
        };
    };
#undef AddValueType

    class Project; // forward declaration

    class Sadjlist {           // Node Adjacency List Item
    public:
        int    node;           // index of connecting node
        int    link;           // index of connecting link
        cweeSharedPtr<Sadjlist> next; // next item in list
    };
    using Padjlist = cweeSharedPtr<Sadjlist>; // Pointer to adjacency list



    class Sasset {
    public: // construction or destruction
        Sasset() : 
            Type_p(asset_t::ANY), 
            Values_p( new ValuesContainerType(std::function<int(const cweeUnion<int, cweeAny>&)>([](const cweeUnion<int, cweeAny>& v)->int { return v.get<0>(); })) ),
            Name_p() 
        {};
        Sasset(asset_t type_m, cweeSharedPtr<ValuesContainerType> values) : Type_p(type_m), Values_p(values), Name_p() {};
        Sasset(asset_t type_m, cweeSharedPtr<ValuesContainerType> values, const cweeStr& name) : Type_p(type_m), Values_p(values), Name_p(name) {};
        virtual	~Sasset() {
            Values_p = nullptr;
        };

        // data members
    public:
        asset_t					        Type_p; // type of this asset
        cweeStr		                    Name_p; // name of the asset

        INLINE static size_t			Hash(const asset_t& type, cweeStr const& UniqueID) { return cweeStr::Hash(cweeStr::printf("%i %s", static_cast<int>(type), UniqueID.c_str())); };
        INLINE size_t					Hash() const { return Hash(Type_p, Name_p); };

        template<value_t v> typename DefaultUnits<v>::unit		GetCurrentValue(u64 t0) const {
            typename DefaultUnits<v>::unit out;
            AUTO p = GetValue<v>();
            if (p) {
                out = p->GetCurrentValue(t0);
            }
            return out;
        };
        template<value_t v> cweeSharedPtr<cweeBalancedPattern<typename DefaultUnits<v>::unit>>		GetValue() const {            
            AUTO anyP = Values_p->FindFast(static_cast<int>(v));
            if (anyP) {
                auto& p = anyP->get<1>();
                if (p && p.IsTypeOf<cweeBalancedPattern<typename DefaultUnits<v>::unit>>()) {
                    return p.cast<cweeSharedPtr<cweeBalancedPattern<typename DefaultUnits<v>::unit>>>();
                }
            }
            return nullptr;
        };
        void ReduceMemoryOfValues() {
#define ReduceMemoryFor(vt) if (auto p = GetValue<vt>()) { if (p){ p->RemoveUnnecessaryKnots(); } }
            ReduceMemoryFor(_FLOW_);
            ReduceMemoryFor(_VELOCITY_);
            ReduceMemoryFor(_HEADLOSS_);
            ReduceMemoryFor(_STATUS_);
            ReduceMemoryFor(_SETTING_);
            ReduceMemoryFor(_ENERGY_);
            ReduceMemoryFor(_ENERGY_PRICE_);
            ReduceMemoryFor(_WATER_PRICE_);
            ReduceMemoryFor(_COST_);
            ReduceMemoryFor(_SETTING_);
            ReduceMemoryFor(_HEAD_);
            ReduceMemoryFor(_DEMAND_);
            ReduceMemoryFor(_QUALITY_);
            ReduceMemoryFor(_MASS_FLOW_);
            ReduceMemoryFor(_TIME_);
            ReduceMemoryFor(_LEVEL_);
            ReduceMemoryFor(_EMISSION_INTENSITY_);
#undef ReduceMemoryFor
        };
        virtual cweeStr Icon() const noexcept { return "M 0 0 ZU 0 0 100 100 50 50"; };

    private:
        cweeSharedPtr<ValuesContainerType>	Values_p; // private access due to the type-erasure
    };
    using Passet = cweeSharedPtr<Sasset>;
}

namespace std {
    template <>
    struct hash<::epanet::Passet>
    {
        std::size_t operator()(::epanet::Passet const& k) const
        {
            static const size_t shift = (size_t)log2(1 + sizeof(::epanet::Sasset));
            return (size_t)(k.Get()) >> shift;
        }
    };
}

namespace epanet {

    class Szone;

    class Snode : public Sasset            // Node Object
    {
    public:
        Snode() : Sasset() {};
        Snode(asset_t type_m, cweeSharedPtr<ValuesContainerType> values) : Sasset(type_m, values) {};
        Snode(asset_t type_m, cweeSharedPtr<ValuesContainerType> values, const cweeStr& name) : Sasset(type_m, values, name) {};
        virtual ~Snode() {};
    private:
        SCALER   C0_internal;             // initial quality

    public:
        void     AddDemands(cweeList<Sdemand> const& demands) {
            for (auto& in_d : demands) {
                if (in_d.Base != 0_gpm) { // valid                    
                    AUTO matches = D.Select([&](Sdemand const& thisD)->bool {
                        return thisD.TimePat == in_d.TimePat;
                    });
                    if (matches.Num() > 0) {
                        matches[0]->Base += in_d.Base;
                    } else {
                        D.Append(in_d);
                    }
                }
            }
        };

        bool     HasWaterDemand() const {
            bool out = false;
            for (auto& demand : D) {
                if (demand.Base != 0_cfs) {
                    out = true;
                    break;
                }
            }
            return out;
        };
        pounds_per_square_inch_t GetMinPressure() const {
            AUTO head = this->GetValue<_HEAD_>();
            if (!head) return 0_psi;

            head_t headV = (head->GetMinValue() - this->El)();
            return headV;
        };
        pounds_per_square_inch_t GetAvgPressure() const {
            AUTO head = this->GetValue<_HEAD_>();
            if (!head) return 0_psi;

            head_t headV = (head->GetAvgValue() - this->El)();
            return headV;
        };
        pounds_per_square_inch_t GetMaxPressure() const {
            AUTO head = this->GetValue<_HEAD_>();
            if (!head) return 0_psi;

            head_t headV = (head->GetMaxValue() - this->El)();
            return headV;
        };

        char     ID[MAXID + 1];  // node ID
        SCALER   X;              // x-coordinate
        SCALER   Y;              // y-coordinate
        foot_t   El;             // elevation
        cweeList<Sdemand>  D;              // demand pointer list
        Psource  S;              // source pointer
        SCALER&  C0(cweeTime t) {
            AUTO ptr = this->GetValue<_QUALITY_>();
            ptr->AddUniqueValue((u64)t, ptr->GetCurrentValue((u64)t)); // ensure the value exists

            AUTO g = ptr->Guard();
            AUTO p = ptr->UnsafeGetValue((u64)t);
            if (p && p->object) return *p->object;
            else return C0_internal; 
        };             // initial quality
        SCALER   Ke;             // emitter coeff.
        int      Rpt = 0;            // reporting flag
        int      ResultIndex = 0;    // saved result index
        NodeType Type;           // node type
        char* Comment = NULL;           // node comment

        virtual cweeStr Icon() const noexcept override { return "M 0 0 ZU 0 0 100 100 50 50"; };

        cweeSharedPtr<Szone> Zone;
    };
    using Pnode = cweeSharedPtr<Snode>;

    class Slink : public Sasset            // Link Object
    {
    public:
        Slink() : Sasset() {};
        Slink(asset_t type_m, cweeSharedPtr<ValuesContainerType> values) : Sasset(type_m, values) {};
        Slink(asset_t type_m, cweeSharedPtr<ValuesContainerType> values, const cweeStr& name) : Sasset(type_m, values, name) {};
        virtual ~Slink() {};

    private:
        SCALER Status_internal;       // initial status
    public:
        char     ID[MAXID + 1];    // link ID

        Pnode    StartingNode;
        Pnode    EndingNode;

        int      N1 = 0;         // start node index
        int      N2 = 0;         // end node index
        foot_t   Diam;           // diameter
        foot_t   Len;            // length
        SCALER   Kc;             // roughness
        SCALER   Km;             // minor loss coeff.
        SCALER   Kb;             // bulk react. coeff.
        SCALER   Kw;             // wall react. coef.
        SCALER   R_FlowResistance;              // flow resistance
        SCALER   Rc;             // reaction coeff.
        LinkType Type;           // link type
        SCALER&  Status(cweeTime t) {
            AUTO ptr = this->GetValue<_STATUS_>();
            ptr->AddUniqueValue((u64)t, ptr->GetCurrentValue((u64)t)); // ensure the value exists

            AUTO g = ptr->Guard();
            AUTO p = ptr->UnsafeGetValue((u64)t);
            if (p && p->object) return *p->object;
            else return Status_internal;
        };       // initial status
        Pvertices  Vertices;     // internal vertex coordinates. Ptr because by being "null" it is effectively turned off. 
        int      Rpt = 0;            // reporting flag
        int      ResultIndex = 0;    // saved result index
        char* Comment = NULL;       // link comment

        square_foot_t Area() const {
            return PI * Diam * Diam / 4.0;
        };
        SCALER   X() const {
            SCALER out = 0;
            if (Vertices && Vertices->Array.Num() >= 1) {
                if (Vertices->Array.Num() >= 2) {
                    // the middle is in between the middlemost vertex points
                    int middlePoint = Vertices->Array.Num() / 2;
                    try {
                        out = Vertices->Array[middlePoint].first;
                        out += Vertices->Array[middlePoint - 1].first;
                        out /= 2.0;
                    }catch(...){}
                }
                else {
                    // the "middle" is the isolated vertex
                    out = Vertices->Array[0].first;
                }
            }
            else {
                // middle of start and end
                if (StartingNode) out = StartingNode->X; 
                if (EndingNode) out += EndingNode->X;
                out /= 2.0;
            }
            return out;
        };
        SCALER   Y() const {
            SCALER out = 0;
            if (Vertices && Vertices->Array.Num() >= 1) {
                if (Vertices->Array.Num() >= 2) {
                    // the middle is in between the middlemost vertex points
                    int middlePoint = Vertices->Array.Num() / 2;
                    try {
                        out = Vertices->Array[middlePoint].second;
                        out += Vertices->Array[middlePoint - 1].second;
                        out /= 2.0;
                    }
                    catch (...) {}
                }
                else {
                    // the "middle" is the isolated vertex
                    out = Vertices->Array[0].second;
                }
            }
            else {
                // middle of start and end
                if (StartingNode) out = StartingNode->Y;
                if (EndingNode) out += EndingNode->Y;
                out /= 2.0;
            }
            return out;
        };
        /* Get the list of zones that are referencing this link (either 1 or 2) */
        cweeList<cweeSharedPtr<Szone>> Zones() const {
            cweeList<cweeSharedPtr<Szone>> out;
            if (StartingNode && StartingNode->Zone) out.AddUnique(StartingNode->Zone);
            if (EndingNode && EndingNode->Zone) out.AddUnique(EndingNode->Zone);
            return out;
        };
        bool    IsBiDirectionalPipe() const {
            if (Type == LinkType::PIPE || Type == LinkType::TCV) {
                return true;
            }
            return false;
        };
        Pnode   GetDownstreamNode() const {
            if (IsBiDirectionalPipe()) {
                AUTO ptr = GetValue<_FLOW_>();
                if (ptr) {
                    if (ptr->GetAvgValue() < 0_cfs) {
                        return StartingNode;
                    }
                }
            }
            return EndingNode;
        };
        Pnode   GetUpstreamNode() const {
            if (IsBiDirectionalPipe()) {
                AUTO ptr = GetValue<_FLOW_>();
                if (ptr) {
                    if (ptr->GetAvgValue() < 0_cfs) {
                        return EndingNode;
                    }
                }
            }
            return StartingNode;
        };
        virtual cweeStr Icon() const noexcept override { 
            if (Type == LinkType::CVPIPE) {
                return "M 32.925818,54.016869 H 0.81084386 v 8.549385 H 32.597001 l 0.10962,44.939056 h 8.549378 V 41.302406 28.259118 24.751685 c 0,-0.40193 -0.115856,-1.037877 0.112664,-1.39169 0.256042,-0.396564 1.006415,-0.426812 1.421842,-0.550663 1.026466,-0.30581 2.031668,-0.680885 2.959391,-1.222788 0.396228,-0.231378 0.857034,-0.854936 1.315287,-0.903269 0.441949,-0.04658 0.98943,0.549681 1.315287,0.7941 1.128423,0.84628 2.269426,1.674588 3.397849,2.520972 4.442386,3.331742 8.885986,6.67115 13.372092,9.94381 12.054633,8.793692 24.014119,17.739311 35.951259,26.69205 3.74968,2.81263 7.53879,5.573315 11.28957,8.384964 1.81948,1.364393 3.82639,3.404081 5.91881,4.288614 -3.1556,4.638147 -6.91735,9.130079 -9.64547,14.029749 15.19158,4.279081 30.39085,8.532941 45.59671,12.760506 3.40113,0.9459 6.79238,1.92142 10.19348,2.86732 0.82975,0.23018 3.12274,0.45049 3.61376,1.18815 0.48446,0.7267 0.1129,2.70402 0.1129,3.57101 h 8.00135 V 73.965429 65.525655 c 0,-0.650194 -0.3069,-2.410379 0.17537,-2.893313 0.40993,-0.410038 1.69563,-0.175699 2.23598,-0.175699 h 6.24765 24.99047 v -8.549378 h -25.42891 -6.35724 c -0.54038,0 -1.82605,0.234339 -2.23598,-0.175699 -0.49434,-0.494768 -0.17537,-2.334645 -0.17537,-3.002916 V 42.069661 7.3240975 h -5.59 l -2.01676,0.1126764 -0.17537,1.4218272 0.10959,4.2746909 v 17.537195 l 0.21922,73.217773 h -0.10962 l -4.27468,-6.466839 -8.87821,-13.262503 -15.34176,-22.907952 -6.03168,-8.987814 c -0.98538,0.755526 -1.65617,2.078488 -2.39711,3.069012 -1.7625,2.354693 -3.44717,4.767815 -5.20637,7.124479 -0.62257,0.833568 -1.2265,1.680613 -1.8392,2.520971 -0.2006,0.274025 -0.49325,0.846176 -0.8703,0.889795 -0.45815,0.05294 -1.07195,-0.623341 -1.41503,-0.873238 -1.27801,-0.932545 -2.5681,-1.850947 -3.83627,-2.797732 -4.86655,-3.634582 -9.78026,-7.213704 -14.68739,-10.793597 C 91.824372,43.036067 80.414687,34.58709 68.986675,26.163865 64.928022,23.172349 60.854575,20.19936 56.820251,17.174844 55.021274,15.826234 53.007018,14.599513 51.346673,13.082435 50.569672,12.372397 50.865496,10.214556 50.635657,9.1874238 50.116336,6.8669238 48.883253,4.5462072 47.065183,2.9867105 40.408841,-2.723069 30.118128,1.0853511 28.25919,9.5162465 c -0.687241,3.1173455 0.0049,6.2945335 1.863748,8.8782075 0.638135,0.886938 2.621272,2.033436 2.78666,3.070645 0.177563,1.113506 0.01618,2.377829 0.01618,3.5058 v 7.124487 z";
            }
            else {
                AUTO ptr = this->GetValue<_STATUS_>();
                if (ptr) {
                    if (::epanet::StatusType::CLOSED == (::epanet::StatusType)(double)(ptr->GetMaxValue())) { // "max == closed" means never opened
                        return "m 9.8524295,167.27056 c -4.3777589,-4.38616 -7.9595557,-8.2314 -7.9595557,-8.54499 0,-0.31359 15.6449432,-16.21671 34.7665622,-35.34028 L 71.425977,88.615159 36.659436,53.852233 C 17.537817,34.732628 1.8928738,18.824248 1.8928738,18.500279 c 0,-0.323947 3.6518336,-4.233853 8.1151782,-8.6886566 L 18.123231,1.7119712 53.17651,36.761658 88.22979,71.811346 123.28305,36.761658 158.33633,1.7119712 166.4515,9.8116224 c 4.46335,4.4548036 8.11518,8.3647096 8.11518,8.6886566 0,0.323969 -15.64494,16.232349 -34.76656,35.351954 l -34.76656,34.762948 35.05915,35.062679 35.05915,35.06269 -8.40764,8.39263 -8.40761,8.39264 L 123.2832,140.476 88.22979,105.42618 53.316606,140.33579 c -19.202256,19.20026 -35.04625,34.90959 -35.208903,34.90959 -0.162718,0 -3.877515,-3.58867 -8.2552735,-7.97482 z";
                    }
                }
                return "";
            }                       
        };
    };
    using Plink = cweeSharedPtr<Slink>;

    class Stank final : public Snode   // Tank Object
    {
    public:
        Stank() : Snode(asset_t::RESERVOIR, cweeAssetValueCollection<asset_t::RESERVOIR>::Values()) {};
        Stank(const cweeStr& name) : Snode(asset_t::RESERVOIR, cweeAssetValueCollection<asset_t::RESERVOIR>::Values(), name) {};
        virtual ~Stank() {};

    private:
        foot_t  H0_internal;              // initial water elev
        cubic_foot_t  V0_internal;        // initial volume

    public:
        int     Node;            // node index of tank
        foot_t  Diameter;        // tank diameter
        square_foot_t  Area() const { return PI * Diameter * Diameter / 4.0; };        // tank area
        foot_t  Hmin;            // minimum water elev
        foot_t  Hmax;            // maximum water elev
        foot_t& InitialHead(cweeTime t) { 
            AUTO ptr = this->GetValue<_HEAD_>();
            ptr->AddUniqueValue((u64)t, ptr->GetCurrentValue((u64)t)); // ensure the value exists

            AUTO g = ptr->Guard();
            AUTO p = ptr->UnsafeGetValue((u64)t);
            if (p && p->object) return *p->object;
            else return H0_internal;
        };              // initial water elev

        cubic_foot_t Volume(cweeSharedPtr<Project> pr, foot_t head);
        cubic_foot_t  Vmin(cweeSharedPtr<Project> pr);      // minimum volume
        cubic_foot_t  Vmax(cweeSharedPtr<Project> pr);      // maximum volume

        SCALER  Kb;              // bulk reaction coeff.
        Ppattern   TimePat;      // actual fixed grade time pattern (shared ptr)
        int     Pat = 0;             // fixed grade time pattern
        int     Vcurve = 0;          // volume v. elev. curve index
        Pcurve  Vcurve_Actual;
        MixType MixModel;        // type of mixing model
        SCALER  V1frac;          // mixing compartment fraction
        int     CanOverflow = 0;     // tank can overflow or not

        cweeStr Icon() const noexcept override { 
            if (Diameter == 0.0_ft) {
                return "m 39.341154,101.23625 -3.78972,-0.24746 C 29.405532,100.58746 23.926794,96.923635 21.366022,91.502472 19.491674,87.534446 -0.18700597,3.461092 0.50076346,2.3596532 0.78884858,1.8982785 1.6481906,1.1679649 2.4104044,0.73672166 3.6439334,0.03882562 3.9522613,0.03991172 5.2157251,0.74706546 6.3796914,1.3981661 6.876452,2.386092 7.9755523,6.2356178 l 1.3403588,4.6945302 4.4162429,1.768614 c 7.760331,3.107846 12.614717,2.819699 21.165883,-1.256399 6.633261,-3.161881 10.172019,-4.100218 15.420214,-4.0888502 5.373293,0.011688 7.615249,0.5878274 14.252461,3.6628302 10.78163,4.995113 15.7992,5.039654 25.374506,0.225298 5.801164,-2.9167737 9.509859,-3.8977582 14.735812,-3.8977582 5.34963,0 8.68628,0.9117224 16.35677,4.4693982 8.9842,4.166999 14.55647,4.077949 23.39661,-0.373877 5.9133,-2.9778848 9.69095,-4.0346377 14.60941,-4.0868014 5.62344,-0.059684 8.82053,0.7277479 15.68158,3.8620824 3.01871,1.379046 6.42945,2.759953 7.57943,3.068695 4.79537,1.28741 11.13573,0.39023 16.52384,-2.338154 l 2.41041,-1.220568 0.93644,-4.4623015 c 0.54494,-2.5967573 1.31968,-4.8091303 1.85312,-5.29189603 1.42068,-1.28569269 4.21162,-0.7952829 5.16263,0.90715103 0.73767,1.3205104 0.19648,3.8980076 -9.3291,44.4311295 -6.43145,27.36697 -10.56341,43.926698 -11.3518,45.494735 -2.75473,5.478861 -8.78894,9.294544 -14.69973,9.295284 l -3.267,4.3e-4 c -22.89283,0.29804 -0.0378,-0.007 -22.8243,0.26135 l -42.61546,-0.13371 -42.615426,-0.13373 c -23.229751,0.21676 -10.742324,0.21864 -23.1473,0.14315 z M 82.261679,52.780804 c 1.900373,-0.402818 5.398253,-1.709344 7.773064,-2.90337 10.845577,-5.453053 18.546307,-5.456788 30.257307,-0.01469 10.18106,4.73114 14.50301,4.742673 24.56776,0.06558 9.33119,-4.336203 14.73341,-5.123987 22.14277,-3.228993 2.16933,0.554821 5.57295,1.82897 7.56359,2.831439 5.25056,2.644144 8.82695,3.551906 13.28109,3.371003 l 3.80419,-0.154495 1.45516,-5.749911 c 0.80034,-3.16245 1.46124,-5.873467 1.46865,-6.024476 0.007,-0.151019 -2.39757,-0.186685 -5.34441,-0.07923 -6.41583,0.233862 -9.7382,-0.510695 -17.15695,-3.844956 -9.39711,-4.223393 -15.32398,-4.119986 -24.30578,0.424075 -4.74154,2.398841 -9.35528,3.513354 -14.54413,3.513354 -5.8344,0 -9.20175,-0.830648 -16.00003,-3.946862 -9.40797,-4.312464 -14.09647,-4.30029 -23.637746,0.06134 -6.612831,3.022965 -10.085989,3.888338 -15.542984,3.872718 -5.868689,-0.01676 -8.044051,-0.563954 -14.918354,-3.752357 -9.911034,-4.596867 -14.506742,-4.569911 -24.981338,0.14654 -6.370735,2.868594 -10.896819,3.79785 -16.75446,3.439903 -4.551892,-0.278155 -4.618723,-0.264925 -4.360325,0.862859 0.144327,0.629948 0.779716,3.465121 1.41198,6.300397 l 1.149579,5.155049 5.040264,-0.216454 c 4.83855,-0.207786 5.291308,-0.334539 11.312898,-3.167323 11.434842,-5.379365 18.244816,-5.371576 29.606837,0.03393 5.183463,2.466034 8.460906,3.452501 12.210699,3.675265 0.574991,0.03424 2.600296,-0.26747 4.500669,-0.670288 z M 30.261957,32.877609 c 2.151479,-0.722225 5.032423,-1.881009 6.402129,-2.575089 5.64973,-2.862935 14.282302,-3.880538 20.449513,-2.410549 1.581236,0.376886 5.109588,1.693269 7.840795,2.925288 6.174689,2.785347 8.314563,3.370278 12.283902,3.357793 4.223301,-0.01324 6.999452,-0.714146 12.325091,-3.111569 6.379206,-2.871718 10.044186,-3.818817 14.856273,-3.839122 5.2205,-0.02203 8.50004,0.757776 15.23493,3.622564 2.91678,1.240707 6.40409,2.53302 7.74956,2.871811 3.70761,0.933589 9.91832,0.330091 13.91247,-1.351873 12.21097,-5.142129 15.79943,-5.932354 22.42935,-4.939215 3.66373,0.548821 6.23438,1.418579 14.56728,4.928737 2.07425,0.873761 4.73071,1.507205 7.05671,1.682698 3.56003,0.268597 8.67326,-0.416917 10.1915,-1.366354 0.59689,-0.373266 3.39421,-12.059338 2.98504,-12.470307 -0.0746,-0.07499 -1.17206,0.245738 -2.43875,0.712636 -1.2667,0.466888 -4.65683,0.968128 -7.5336,1.113872 -6.38197,0.323316 -9.85442,-0.457186 -17.12008,-3.848069 -9.41328,-4.393166 -14.43671,-4.340071 -24.32967,0.257188 -6.66752,3.098392 -9.41017,3.818651 -14.73926,3.870722 -5.19223,0.05079 -8.55476,-0.801717 -15.15885,-3.843 -6.56203,-3.02192 -10.05956,-3.852496 -14.48319,-3.439406 -3.210113,0.299763 -4.807195,0.856352 -13.482269,4.698577 -5.181178,2.294772 -10.58063,3.128347 -15.630979,2.413136 -3.533028,-0.500351 -5.216947,-1.101149 -13.755918,-4.907957 -7.608184,-3.391866 -12.989545,-3.102136 -22.113375,1.190551 -6.635515,3.121955 -10.227482,3.908114 -16.453218,3.601069 -2.874955,-0.141782 -6.226894,-0.626845 -7.448755,-1.077917 -1.827853,-0.674778 -2.221558,-0.693427 -2.221558,-0.105238 0,1.09004 2.466406,11.288332 2.814123,11.636049 0.705396,0.705385 5.525068,1.648078 8.593535,1.680836 2.222385,0.02369 4.587288,-0.395009 7.217271,-1.277862 z";
            }
            else {
                return "M 94.864078,1.5717254 C 74.44716,4.2524383 54.909705,5.0963801 35.086263,11.710559 25.378132,14.949699 15.289185,19.376913 7.9990444,26.764162 -1.1711268,36.056473 1.5418797,50.030719 1.5418797,62.027137 v 84.291013 c 0,11.41671 -2.9532274,26.52373 5.2353315,35.69468 9.0786578,10.16781 23.3000148,15.13669 36.0500658,18.79732 31.159292,8.94604 65.233503,9.79712 97.192703,5.59461 15.08121,-1.983 30.34692,-5.38301 44.2958,-11.55045 7.55995,-3.3424 15.15475,-7.52082 20.35498,-14.13165 6.88607,-8.75337 4.15822,-23.05876 4.15822,-33.54439 V 65.467587 c 0,-10.299416 2.99275,-25.998189 -3.19962,-34.834554 C 198.92177,21.060584 186.91976,15.589623 176.1447,11.997277 157.16718,5.670161 137.5807,2.6748631 117.65706,1.8023222 110.21924,1.4765967 102.29373,0.59622948 94.864078,1.5717254 M 97.01436,14.413677 c 6.2199,-0.73389 12.7116,0.01837 18.92247,0.29033 19.60154,0.858436 39.53851,3.107115 58.0576,10.091056 5.95239,2.244721 12.20068,4.976095 17.20052,8.971489 1.82086,1.455138 4.16423,3.437439 4.52247,5.897662 0.82012,5.626426 -8.67165,10.132554 -12.69181,12.147799 -14.75266,7.394385 -31.47754,10.670125 -47.73625,12.672896 C 104.71624,68.251341 71.001118,67.25189 41.537108,57.439728 34.102209,54.963895 26.348467,52.230887 20.034296,47.448231 17.925344,45.851003 14.748389,43.419894 14.580624,40.524327 14.183725,33.673574 27.462184,28.430284 32.505926,26.337459 53.49267,17.629423 74.916781,17.020936 97.01436,14.413677 m 8.17107,77.717396 h 32.25422 c 0,-3.183275 -0.7225,-7.522974 1.17319,-10.302856 2.55281,-3.74364 9.49607,-3.537213 11.29456,0.84334 1.0631,2.589368 0.43393,6.275379 0.43393,9.02946 v 18.922473 55.47726 c 0,7.63393 1.86,18.04344 -0.24642,25.37331 -1.325,4.61236 -8.10742,5.69309 -11.0847,2.05825 -2.27758,-2.78031 -1.57056,-7.32944 -1.57056,-10.65937 h -32.25422 c 0,3.17812 0.77496,7.55696 -1.16244,10.30243 -2.86504,4.05972 -9.905919,3.03577 -11.393484,-1.70131 -2.266396,-7.21677 -0.345765,-17.80992 -0.345765,-25.37331 V 110.62349 91.701017 c 0,-2.816869 -0.581866,-6.393645 0.587888,-9.02473 1.915899,-4.307873 8.515111,-4.601602 11.151361,-0.84807 1.93396,2.75365 1.16244,7.11915 1.16244,10.302856 m 0,12.901687 v 12.90169 h 32.25422 v -12.90169 h -32.25422 m 0,25.80337 v 12.90168 h 32.25422 v -12.90168 h -32.25422 m 0,25.80337 v 13.33176 h 32.25422 V 156.6395 Z";
            }
        };
    };
    using Ptank = cweeSharedPtr<Stank>;

    class Spump final : public Slink   // Pump Object
    {
    public:
        Spump() : Slink(asset_t::PUMP, cweeAssetValueCollection<asset_t::PUMP>::Values()) {};
        Spump(const cweeStr& name) : Slink(asset_t::PUMP, cweeAssetValueCollection<asset_t::PUMP>::Values(), name) {};
        virtual ~Spump() {};
    private:
        cubic_foot_per_second_t
            Q0_internal;              // initial flow

    public:
        int     Link;            // link index of pump
        int     Ptype;           // pump curve type

        cubic_foot_per_second_t& InitialFlow(cweeTime t) {
            AUTO ptr = this->GetValue<_FLOW_>();
            ptr->AddUniqueValue((u64)t, ptr->GetCurrentValue((u64)t)); // ensure the value exists

            AUTO g = ptr->Guard();
            AUTO p = ptr->UnsafeGetValue((u64)t);
            if (p && p->object) return *p->object;
            else return Q0_internal;
        };              // initial water elev

        cubic_foot_per_second_t
                Qmax;            // maximum flow
        foot_t  Hmax;            // maximum head
        foot_t  H0;              // shutoff head .. unused for custom head curve
        SCALER  R;               // flow coeffic. .. unused for custom head curve
        SCALER  N;               // flow exponent .. unused for custom head curve
        int     Hcurve = 0;          // head v. flow curve index
        int     Ecurve = 0;          // effic. v. flow curve index

        Ppattern   TimeUpat;      // actual
        int     Upat = 0;            // utilization pattern index

        Ppattern   TimeEpat;      // actual
        int     Epat = 0;            // energy cost pattern index

        Dollar_per_kilowatt_hour_t
                Ecost;           // unit energy cost
        Senergy Energy;          // energy usage statistics

        cweeStr Icon() const noexcept override { return "M 31.539821,15.32081 18.2742,16.218286 v 9.87226 h 43.481761 v -9.87226 H 49.22732 c 0,-3.669784 1.104728,-10.7831946 -0.8689,-13.8181093 -1.689893,-2.59864173 -15.052063,-2.86681095 -16.561395,0 C 30.421823,5.0119317 31.26124,12.281948 31.539821,15.32081 M 18.2742,29.680459 c 0,12.29724 4.380454,10.769731 13.265621,10.769731 l -1.181379,14.71595 -5.743718,3.519013 -0.444693,19.459118 C 16.849693,78.433259 12.402764,85.898486 12.378366,94.298875 H 1.3236835 c 0,4.361737 -1.81901234,17.722505 1.1816739,20.616865 2.1719483,2.09471 7.2458284,0.9226 9.8730086,0.9226 0.07569,7.80536 5.144555,15.54432 11.791665,16.15461 v 20.642 H 55.86013 v -73.5932 c 0,-9.034022 3.118895,-22.394769 -6.63281,-22.436954 V 40.45019 c 8.730986,0 12.528641,1.123647 12.528641,-10.769731 H 18.2742 m 84.0156,23.334426 h 55.27343 l -1.28381,-16.129472 -11.24483,-0.922607 h -29.47916 l -11.98181,0.922607 -1.28382,16.129472 m 74.1423,-7.154698 c -1.7341,1.672004 -1.18063,5.703474 -1.18137,8.052174 v 21.53947 59.233549 c 0,8.02346 -1.64125,19.09834 0.23436,26.79151 1.05167,4.31329 16.57023,4.45777 19.21966,1.90265 2.33624,-2.25355 0.52327,-7.39162 2.36055,-10.28329 13.97092,-21.9945 16.58203,-56.508806 8.05962,-83.029281 -1.87267,-5.828224 -5.20456,-8.852722 -8.05741,-13.717954 -1.70904,-2.915904 -0.4208,-6.349654 -1.62577,-9.33108 -1.43489,-3.549521 -7.95348,-2.080352 -10.61029,-2.080352 -2.30085,0 -6.46993,-0.938759 -8.39935,0.922604 M 86.813249,91.606435 h 37.585921 33.16406 c 3.79471,0 10.31623,1.527515 13.76825,-0.541174 2.16671,-1.298647 1.7039,-5.109342 1.70831,-7.536128 0.008,-5.234091 2.25074,-22.519513 -1.28381,-26.001728 -3.37758,-3.327848 -14.36815,-0.922609 -18.61462,-0.922609 h -47.90363 c -4.02907,0 -13.995981,-2.20959 -17.243105,0.922609 -1.857923,1.792265 -1.181376,6.434016 -1.181376,8.949651 v 25.129379 m -16.950517,2.69244 H 58.808045 v 21.539465 h 11.054687 c 0,14.18644 0.01767,27.82004 14.739577,27.82183 V 65.579579 C 69.512665,65.592118 69.862732,80.007427 69.862732,94.298875 M 51.438255,70.964448 v 2.69244 H 27.854927 v -2.69244 h 23.583328 m 0,8.974782 v 3.589903 c -6.755149,-1.958286 -16.0912,-1.958286 -22.846353,0 L 27.854927,80.836694 51.438255,79.93923 m 0,9.872246 v 2.692439 c -5.36226,0 -20.496126,3.107073 -23.583328,-2.692439 h 23.583328 m 35.374994,4.487399 v 8.974785 H 173.03979 V 94.298875 H 86.813249 M 51.438255,98.786257 V 101.4787 H 27.854927 v -2.692443 h 23.583328 m 35.374994,7.179843 v 8.97476 H 173.03979 V 105.9661 H 86.813249 m -35.374994,1.79494 v 2.69244 H 27.854927 c 3.087202,-5.7995 18.221068,-2.69244 23.583328,-2.69244 m 0,8.97478 v 3.58992 l -23.583328,-0.8975 v -1.79494 l 23.583328,-0.89748 m 35.374994,1.79496 v 34.10417 H 173.03979 V 118.53078 H 86.813249 m -35.374994,8.0773 v 2.69243 H 27.854927 v -2.69243 h 23.583328 m 0,8.97478 v 2.69243 H 27.854927 v -2.69243 h 23.583328 m -19.898434,20.64199 c -0.634538,8.57809 -5.50376,14.50414 -12.528641,15.25712 v 11.66722 c -8.023713,0 -12.2562545,3.55221 -16.950516,11.66721 H 204.72989 c -9.16434,-15.84228 -23.3534,-11.66721 -37.58593,-11.66721 0,-2.91681 0.73993,-7.78294 -0.75762,-10.26445 -4.12782,-6.83969 -10.95372,-5.60924 -11.77103,-16.65989 h -49.37758 c -0.81731,11.05065 -7.643221,9.8202 -11.771038,16.65989 -1.49754,2.48151 -0.757612,7.34764 -0.757612,10.26445 H 61.018982 C 63.9271,168.24298 49.704881,170.66796 48.490335,156.22485 H 31.539821 m -29.479157,43.07895 1.181669,10.7446 9.873012,0.9226 h 26.531244 122.338521 30.21613 l 11.34726,-0.9226 1.18139,-10.7446 z"; };
    };
    using Ppump = cweeSharedPtr<Spump>;

    class Svalve final : public Slink // Zone Object
    {
    public:
        Svalve() : Slink(asset_t::VALVE, cweeAssetValueCollection<asset_t::VALVE>::Values()) {};
        Svalve(const cweeStr& name) : Slink(asset_t::VALVE, cweeAssetValueCollection<asset_t::VALVE>::Values(), name) {};
        virtual ~Svalve() {};

        int Link = 0;                // link index of valve
        bool ProducesElectricity = false; // this valve can produce electricity from te flow and headloss as a pump-as-turbine
        Senergy Energy;          // energy usage statistics

        static constexpr cwee_units::power::kilowatt_t	energy_generation_potential(cwee_units::flowrate::gallon_per_minute_t const& flowrate, cwee_units::length::foot_t const& headloss) {
            return cweeEng::CentrifugalPumpEnergyDemand_kW(flowrate, headloss, 131); // assumes ~30% efficiency loss in pump-as-turbine operation, fitted from https://www.renewablesfirst.co.uk/hydropower/hydropower-learning-centre/head-and-flow-detailed-review/
        };

        cweeStr Icon() const noexcept override { return "m 96.362325,175.3006 c -7.947228,-1.3149 -14.270523,-3.91668 -19.990423,-8.22526 -1.531487,-1.1536 -5.025127,-4.22491 -7.763649,-6.82511 -2.738522,-2.60021 -6.170185,-5.81177 -7.625907,-7.13682 l -2.646773,-2.40915 h -4.592305 -4.592295 v 3.95676 c 0,5.40749 0.182132,5.27568 -7.29004,5.27568 -4.084653,0 -6.130208,-0.15911 -6.4747,-0.50359 -0.383801,-0.38381 -0.503586,-9.01467 -0.503586,-36.28552 0,-34.947045 0.01889,-35.792075 0.811725,-36.216355 0.446445,-0.238935 3.362849,-0.434423 6.480881,-0.434423 6.987783,0 6.97572,-0.0092 6.97572,5.548917 v 3.683533 h 4.152102 4.152112 l 7.28353,-6.777911 c 6.751579,-6.282877 13.33815,-11.686979 14.244201,-11.686979 0.210826,0 0.383318,-2.077299 0.383318,-4.616226 v -4.616215 h -4.176823 c -6.961727,0 -7.284567,-0.420615 -7.081482,-9.226024 0.130863,-5.674012 0.184586,-5.995855 1.128129,-6.758971 0.957824,-0.774665 2.186387,-0.801267 37.00421,-0.801267 h 36.01352 l 0.86977,1.105736 c 0.79159,1.006353 0.86977,1.668135 0.86977,7.363059 0,6.084041 -0.0285,6.285854 -1.03007,7.287401 -0.97963,0.979629 -1.27757,1.030066 -6.08501,1.030066 h -5.05495 l 8.3e-4,4.091645 8.4e-4,4.091654 1.98994,1.257278 c 3.08263,1.947637 6.45898,4.873239 13.66607,11.841608 l 6.63473,6.415146 h 3.7516 3.75159 v -3.683533 c 0,-5.558516 -0.0121,-5.548917 6.97572,-5.548917 3.11804,0 6.03443,0.195488 6.48088,0.434423 0.79278,0.42428 0.81172,1.26931 0.81172,36.216355 0,27.27085 -0.11978,35.90171 -0.50358,36.28552 -0.34449,0.34448 -2.39005,0.50359 -6.4747,0.50359 -7.47217,0 -7.29004,0.13181 -7.29004,-5.27568 v -3.95676 h -3.90157 -3.90156 l -1.65888,1.72462 c -0.91239,0.94856 -4.56944,4.46896 -8.12678,7.82314 -9.49875,8.95625 -15.16079,12.31245 -24.35009,14.43364 -4.69884,1.08463 -18.3094,1.44357 -23.317695,0.61494 z M 15.173558,159.12497 c -0.238935,-0.44644 -0.434424,-2.52374 -0.434424,-4.61622 v -3.80449 H 8.8448743 c -5.6985672,0 -5.9284679,-0.0342 -6.9243379,-1.03007 l -1.03006657,-1.03006 v -25.36181 c 0,-24.334746 0.0324411,-25.401915 0.80127467,-26.352521 0.7654047,-0.946367 1.0753199,-0.996794 6.9243276,-1.12659 l 6.1230619,-0.135872 v -3.925805 c 0,-5.345842 -0.13462,-5.24472 6.981901,-5.24472 3.114633,0 6.028244,0.195488 6.4747,0.434423 0.792784,0.42428 0.811725,1.26931 0.811725,36.216355 0,27.27085 -0.119785,35.90171 -0.503586,36.28552 -0.346206,0.34621 -2.440054,0.50359 -6.699745,0.50359 -5.696452,0 -6.231186,-0.0655 -6.630571,-0.81173 z M 182.266,159.43311 c -0.3838,-0.38381 -0.50359,-9.01467 -0.50359,-36.28552 0,-34.947045 0.0189,-35.792075 0.81173,-36.216355 0.44644,-0.238935 3.36006,-0.434423 6.4747,-0.434423 7.11652,0 6.9819,-0.101122 6.9819,5.24472 v 3.925805 l 6.12306,0.135872 c 5.84901,0.129796 6.15891,0.180223 6.92433,1.12659 0.76882,0.950606 0.80127,2.017775 0.80127,26.352521 v 25.36181 l -1.03006,1.03006 c -0.99587,0.99587 -1.22578,1.03007 -6.92434,1.03007 h -5.89426 v 3.80449 c 0,5.65842 0.29998,5.42795 -7.065,5.42795 -4.25969,0 -6.35354,-0.15738 -6.69974,-0.50359 z M 97.831127,37.606833 v -9.861928 h -5.608769 c -5.038802,0 -5.721143,-0.08839 -6.714508,-0.869772 -1.084118,-0.852771 -1.105738,-0.967598 -1.105738,-5.875191 V 15.994523 H 68.266 c -9.895666,0 -16.947376,-0.173985 -18.23381,-0.449872 -1.153733,-0.247434 -2.855854,-0.999089 -3.782509,-1.670355 -2.308462,-1.672242 -3.11796,-3.367088 -2.865094,-5.9986997 0.175839,-1.8300426 0.447441,-2.3364205 2.080984,-3.8798409 1.03368,-0.9766497 2.6154,-1.979631 3.516472,-2.2298135 1.137943,-0.3159507 18.47836,-0.4541796 56.822547,-0.4529569 l 55.18483,0.00175 2.13138,0.9924616 c 6.29958,2.9333574 6.05549,9.7920024 -0.45276,12.7218224 -1.51299,0.681102 -3.38672,0.768142 -18.98945,0.882088 l -17.31083,0.126426 v 5.014366 c 0,6.85634 0.18436,6.692993 -7.55382,6.692993 h -5.8752 v 9.861928 9.861928 h -7.5538 -7.553813 z"; };
    };
    using Pvalve = cweeSharedPtr<Svalve>;

    BETTER_ENUM(illDefined_t, uint8_t, Okay, TankWithoutInflow, IsolatedJunction, InactivePump, InactiveValve);
    BETTER_ENUM(direction_t, uint8_t, FLOW_IN_DMA, FLOW_OUT_DMA, FLOW_WITHIN_DMA);
    BETTER_ENUM(zoneType_t, uint8_t, Open, Draw, Closed, Reduced);

    class Szone final : public Sasset {   // Zone Object
    public:
        Szone() : Sasset(asset_t::DMA, cweeAssetValueCollection<asset_t::DMA>::Values()) {};
        Szone(const cweeStr& name) : Sasset(asset_t::DMA, cweeAssetValueCollection<asset_t::DMA>::Values(), name) {};
        virtual ~Szone() {};

        bool     HasWaterDemand() const {
            for (auto& node : Node) {
                if (node->HasWaterDemand()) return true;
            }
            return false;
        };

        zoneType_t Type = zoneType_t::Draw;
        illDefined_t IllDefined = illDefined_t::Okay;
        cweeThreadedList< Pnode > Node; // this contains all 'within' junctions or reservoirs/tanks
        cweeThreadedList<std::pair<Plink, direction_t>> Boundary_Link; // note that this only inludes the 'critical' links that determine inflow or outflow or can be controlled. 
        cweeThreadedList < Plink > Within_Link; // includes only the completely contained links
        
        Pnode FindMinPressureCustomer() const {
            Pnode out;
            pounds_per_square_inch_t avgPressure = std::numeric_limits<pounds_per_square_inch_t>::max(); int count = 0;
            for (auto& node : Node) {
                if (!node) continue;
                if (!node->HasWaterDemand()) continue;
                AUTO head = node->GetValue<_HEAD_>();
                if (!head) continue;
                head_t headV = (head->GetMinValue() - node->El)();
                pounds_per_square_inch_t pressure = headV;

                if (avgPressure > pressure) {
                    out = node;
                    avgPressure = pressure;
                }
            }
            return out;
        };
        Pnode FindMinPressureNode() const {
            Pnode out;
            pounds_per_square_inch_t avgPressure = std::numeric_limits<pounds_per_square_inch_t>::max(); int count = 0;
            for (auto& node : Node) {
                if (!node) continue;
                AUTO head = node->GetValue<_HEAD_>();
                if (!head) continue;
                head_t headV = (head->GetMinValue() - node->El)();
                pounds_per_square_inch_t pressure = headV;

                if (avgPressure > pressure) {
                    out = node;
                    avgPressure = pressure;
                }
            }
            return out;
        };

        pounds_per_square_inch_t MaximumNodePressure() const {
            pounds_per_square_inch_t avgPressure = -std::numeric_limits<pounds_per_square_inch_t>::max(); int count = 0;
            for (auto& node : Node) {
                if (!node) continue;
                AUTO head = node->GetValue<_HEAD_>();
                if (!head) continue;
                head_t headV = (head->GetMinValue() - node->El)();
                pounds_per_square_inch_t pressure = headV;

                avgPressure = Max< pounds_per_square_inch_t>(avgPressure, pressure);
            }
            return avgPressure;
        };
        pounds_per_square_inch_t AverageNodePressure() const {
            pounds_per_square_inch_t avgPressure = 0; int count = 0;
            for (auto& node : Node) {
                if (!node) continue;
                AUTO head = node->GetValue<_HEAD_>();
                if (!head) continue;
                head_t headV = (head->GetAvgValue() - node->El)();
                pounds_per_square_inch_t pressure = headV;

                cweeMath::rollingAverageRef< pounds_per_square_inch_t>(avgPressure, pressure, count);
            }
            return avgPressure;
        };
        pounds_per_square_inch_t MinimumNodePressure() const {
            pounds_per_square_inch_t avgPressure = std::numeric_limits<pounds_per_square_inch_t>::max(); int count = 0;
            for (auto& node : Node) {
                if (!node) continue;
                AUTO head = node->GetValue<_HEAD_>();
                if (!head) continue;
                head_t headV = (head->GetMinValue() - node->El)();
                pounds_per_square_inch_t pressure = headV;

                avgPressure = Min< pounds_per_square_inch_t>(avgPressure, pressure);
            }
            return avgPressure;
        };
        pounds_per_square_inch_t AverageCustomerPressure() const {
            pounds_per_square_inch_t avgPressure = 0; int count = 0;
            for (auto& node : Node) {
                if (!node) continue;
                if (!node->HasWaterDemand()) continue;
                AUTO head = node->GetValue<_HEAD_>();
                if (!head) continue;
                head_t headV = (head->GetAvgValue() - node->El)();
                pounds_per_square_inch_t pressure = headV;

                cweeMath::rollingAverageRef< pounds_per_square_inch_t>(avgPressure, pressure, count);
            }
            return avgPressure;
        };
        pounds_per_square_inch_t MinimumCustomerPressure() const {
            pounds_per_square_inch_t avgPressure = std::numeric_limits<pounds_per_square_inch_t>::max(); int count = 0;
            for (auto& node : Node) {
                if (!node) continue;
                if (!node->HasWaterDemand()) continue; 
                AUTO head = node->GetValue<_HEAD_>();
                if (!head) continue;
                head_t headV = (head->GetMinValue() - node->El)();
                pounds_per_square_inch_t pressure = headV;

                avgPressure = Min< pounds_per_square_inch_t>(avgPressure, pressure);
            }
            return avgPressure;
        };
        foot_t AverageElevation() const {
            foot_t El = 0; int n = 0;
            for (auto& node : Node) {
                if (node) {
                    cweeMath::rollingAverageRef(El, node->El, n);
                }
            }
            return El;
        };
        bool IsIllDefined() const {
            return IllDefined != illDefined_t::Okay;
        };
        int   NumberOfBoundaryLinksBetweenZones(cweeSharedPtr<Szone> const& zone) { return NumberOfBoundaryLinksBetweenZones_Impl(zone); };
        cweeStr Icon() const noexcept override { return ""; };

        /* Can we identify the best survey frequency right here? */
        cweeList< Pnode > FindDeadends(cweeSharedPtr<Project> pr);
        /* returns the list of assets that feed into or out of this zone. */
        cweeList< cweeUnion<Passet, direction_t> > GetMassBalanceAssets(cweeSharedPtr<Project> pr);
        cweeUnitValues::cweeUnitPattern TryCalibrateZone(cweeSharedPtr<Project> pr, std::map<std::string, cweeUnitValues::cweeUnitPattern> const& ScadaSources);
        std::map<std::string, cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>> LeakModelResults(cweeSharedPtr<Project> pr, year_t surveyFrequency, pounds_per_square_inch_t oldPressure) const;
        year_t SurveyFrequency(cweeSharedPtr<Project> pr, pounds_per_square_inch_t oldPressure) const;


        treeNS::tree< Passet > BuildAssetTree(Passet center, cweeList< Passet > const& exclusions) {
            using namespace treeNS;
            tree< Passet > data;
            {
                // build list of connections
                std::unordered_map < Passet, cweeThreadedList<Passet> > connections; {
                    for (auto& linkP : Within_Link) {
                        AUTO asset_ptr = linkP.CastReference<Sasset>();
                        if (!exclusions.Find(asset_ptr)) {
                            AUTO asset_ptr = linkP.CastReference<Sasset>();
                            cweeThreadedList<Passet> toAdd;
                            toAdd.Append(linkP->StartingNode.CastReference<Sasset>());
                            toAdd.Append(linkP->EndingNode.CastReference<Sasset>());
                            connections[asset_ptr] = toAdd;                            
                        }
                    }
                    for (auto& nodeP : Node) {
                        AUTO asset_ptr = nodeP.CastReference<Sasset>();
                        cweeThreadedList<Passet> toAdd;

                        auto stop = connections.end();
                        for (auto a_idS = connections.begin(); a_idS != stop; a_idS++) {
                            if (a_idS->first->Type_p != asset_t::JUNCTION && a_idS->first->Type_p != asset_t::RESERVOIR) {
                                if (a_idS->second.Find(asset_ptr)) {
                                    toAdd.AddUnique(a_idS->first);
                                }
                            }
                        }

                        connections[asset_ptr] = toAdd;
                    }
                }

                // build connection order
                cweeThreadedList< std::pair<Passet, Passet> > treeConstructionOrder; {
                    treeConstructionOrder.Clear();
                    treeConstructionOrder.SetGranularity(connections.size() + 1024);

                    std::pair<Passet, Passet> temp;
                    temp.second = center;
                    treeConstructionOrder.Append(temp);
                    bool unique;
                    int j;
                    std::pair<Passet, Passet> temp2;
                    for (int i = 0; i < treeConstructionOrder.Num(); i++) {
                        for (auto& x : connections[treeConstructionOrder[i].second]) {
                            temp2.first = treeConstructionOrder[i].second;
                            temp2.second = x;
                            unique = true;
                            for (j = treeConstructionOrder.Num() - 1; j >= 0; j--) {
                                if (treeConstructionOrder[j].second == x) {
                                    unique = false;
                                    break;
                                }
                            }
                            if (unique) treeConstructionOrder.Append(temp2);
                        }
                    }
                }

                // build tree
                decltype(data)::pre_order_iterator ptr, owner;
                {
                    ptr = data.insert(data.begin(), center);
                    for (int i = 1; i < treeConstructionOrder.Num(); i++) {
                        owner = std::find(data.begin(), data.end(), treeConstructionOrder[i].first);
                        ptr = data.append_child(owner, treeConstructionOrder[i].second);
                    }
                }
            }
            return data;
        };

        // Relatively fast algorithm that finds a path from the start of a pipe to the end of a pipe without using the pipe itself. (Looks for connections)
        cweeList<Passet>    findPathAroundLink(Plink link) {
            // ensure the link is "within" our zone;
            cweeList<Passet> out;
            if (Within_Link.Find(link)) {
                // Want to find the (ideally, shortest) path that connects the start and end node of this link WITHOUT using this link. 
                cweeList< Passet > exclusions; 
                
                exclusions.Append(link.CastReference<Sasset>());
                AUTO tree = BuildAssetTree(link->StartingNode.CastReference<Sasset>(), exclusions);
                
                Passet toFind = link->EndingNode.CastReference<Sasset>();
                for (auto itr = tree.begin(); itr != tree.end(); itr++) {
                    AUTO this_asset = itr.node->data;
                    if (toFind == this_asset) {
                        // navigate up the parents until we reach home? 
                        AUTO currentNode = itr.node;
                        while (currentNode) {
                            if (currentNode->data->Type_p != asset_t::JUNCTION && currentNode->data->Type_p != asset_t::RESERVOIR) {
                                out.Append(currentNode->data);
                            }
                            currentNode = currentNode->parent;                        
                        }
                        return out;
                    }
                }
            }
            return out;
        };

    private:

        template <int distance = 0>
        int   NumberOfBoundaryLinksBetweenZones_Impl(cweeSharedPtr<Szone> const& zone) {
            constexpr int maxDepth = 5;
            bool found = false;

            int minAnswer = maxDepth + 1;
            if (distance >= maxDepth) return -1;
            if (zone.Get() == this) { return distance; }

            for (auto& BL : Boundary_Link) {
                int reply = -1;
                switch (BL.second) {
                case direction_t::FLOW_IN_DMA:
                    reply = BL.first->StartingNode->Zone->NumberOfBoundaryLinksBetweenZones_Impl<distance + 1>(zone);
                    break;
                case direction_t::FLOW_OUT_DMA:
                    reply = BL.first->EndingNode->Zone->NumberOfBoundaryLinksBetweenZones_Impl<distance + 1>(zone);
                    break;
                case direction_t::FLOW_WITHIN_DMA:
                    reply = -1;
                    break;
                }
                if (reply != -1) {
                    // found it. Is this the shortest distance? Complete the search and keep the shortest answer.
                    found = true;
                    if (reply < minAnswer) minAnswer = reply;
                }
            }

            if (found)
                return minAnswer;
            else
                return -1;
        };
        template<> int   NumberOfBoundaryLinksBetweenZones_Impl<5>(cweeSharedPtr<Szone> const& zone) { return -1; };
    };
    using Pzone = cweeSharedPtr<Szone>;

    class Scontrol {             // Control Statement
    public:
        int         Link = 0;      // link index
        int         Node = 0;      // control node index
        units::time::second_t        Time;      // control time
        cweeTime    Time_Adjusted(cweeSharedPtr<Project> pr);
        SCALER      Grade;     // control grade
        SCALER      Setting;   // new link setting
        StatusType  Status;    // new link status
        ControlType Type;      // control type
    };
    using Pcontrol = cweeSharedPtr<Scontrol>;

    typedef struct             // Field Object of Report Table
    {
        char   Name[MAXID + 1];  // name of reported variable
        char   Units[MAXID + 1]; // units of reported variable
        int    Enabled;        // enabled if in table
        int    Precision;      // number of decimal places
        SCALER RptLim[2];      // lower/upper report limits
    } SField;



    struct  Sseg               // Pipe Segment List Item
    {
        SCALER  v;             // segment volume
        SCALER  c;             // segment water quality
        struct  Sseg* prev;    // previous segment in list
    };
    typedef struct Sseg* Pseg; // Pointer to pipe segment list

    typedef struct s_Premise       // Rule Premise Clause
    {
        int      logop;            // logical operator (IF, AND, OR)
        int      object;           // NODE or LINK
        int      index;            // object's index
        int      variable;         // pressure, flow, etc.
        int      relop;            // relational operator (=, >, <, etc.)
        int      status;           // variable's status (OPEN, CLOSED)
        SCALER   value;            // variable's value
        struct   s_Premise* next;  // next premise clause
    } Spremise;

    typedef struct s_Action        // Rule Action Clause
    {
        int     link;              // link index
        int     status;            // link's status
        SCALER  setting;           // link's setting
        struct  s_Action* next;
    } Saction;

    class Srule                    // Control Rule Structure
    {
    public:
        char     label[MAXID + 1]; // rule label
        SCALER   priority;         // priority level
        Spremise* Premises;        // linked list of premises
        Saction* ThenActions;      // linked list of THEN actions
        Saction* ElseActions;      // linked list of ELSE actions
    };
    using Prule = cweeSharedPtr<Srule>;

    typedef struct s_ActionItem    // Action List Item
    {
        int     ruleIndex;           // index of rule action belongs to
        Saction* action;             // an action clause
        struct  s_ActionItem* next;  // next action on the list
    } SactionList;

    class SmassBalance                 // Mass Balance Components
    {
    public:
        SCALER    initial;         // initial mass in system
        SCALER    inflow;          // mass inflow to system
        SCALER    outflow;         // mass outflow from system
        SCALER    reacted;         // mass reacted in system
        SCALER    final;           // final mass in system
        SCALER    ratio;           // ratio of mass added to mass lost
    };

    /*
    ------------------------------------------------------
      Wrapper Data Structures
    ------------------------------------------------------
    */

    // Input File Parser Wrapper
    typedef struct {
        FILE* InFile;            // Input file handle

        char
            DefPatID[MAXID + 1],     // Default demand pattern ID
            InpFname[MAXFNAME + 1],  // Input file name
            * Tok[MAXTOKS],           // Array of token strings
            Comment[MAXMSG + 1],     // Comment text
            LineComment[MAXMSG + 1]; // Full line comment
        
        UnitsType Unitsflag;             // Unit system flag
        FlowUnitsType Flowflag;          // Flow units flag
        PressureUnitsType Pressflag;     // Pressure units flag

        int
            MaxNodes,              // Node count   from input file */
            MaxLinks,              // Link count    "    "    "
            MaxJuncs,              // Junction count "   "    "
            MaxPipes,              // Pipe count    "    "    "
            MaxTanks,              // Tank count    "    "    "
            MaxPumps,              // Pump count    "    "    "
            MaxValves,             // Valve count   "    "    "
            MaxControls,           // Control count "   "     "
            MaxRules,              // Rule count    "   "     "
            MaxPats,               // Pattern count "   "     "
            MaxCurves,             // Curve count   "   "     "
            Ntokens,               // Number of tokens in line of input
            Ntitle,                // Number of title lines
            ErrTok,                // Index of error-producing token
            DefPat;                // Default demand pattern

        Ppattern PrevPat;       // Previous pattern processed
        Pcurve PrevCurve;     // Previous curve processed
        SCALER* X;               // Temporary array for curve data

    } Parser;

    class Times {
    private:
        cweeTime CalibrationDate;
        cweeTime SimulationStartTime;

    public:
        Times() { 
            CalibrationDate = cweeTime::Now().ToStartOfDay(); 
            SimulationStartTime = CalibrationDate;
        };

        void SetCalibrationDateTime(cweeTime t) {
            auto t2 = cweeTime(t).ToStartOfDay();
            Tstart = ((u64)(t) - (u64)(t2));
            CalibrationDate = t2;
        };
        cweeTime GetCalibrationDateTime() const {
            return CalibrationDate + (u64)Tstart;
        };

        cweeTime GetCurrentRealHtime() {
            return  GetSimStartTime() + (u64)Htime;
        };
        cweeTime GetCurrentRealQtime() {
            return  GetSimStartTime() + (u64)Qtime;
        };
        void SetSimStartTime(cweeTime t) {
            SimulationStartTime = t;
        };
        cweeTime GetSimStartTime() {
            return SimulationStartTime;
        };
        cweeTime GetPatternStartTime() const {
            return GetCalibrationDateTime() - (u64)((u64)Pstart * (u64)Pstep);
        };

        units::time::second_t
            Tstart,                // Starting time of day
            Hstep,                 // Nominal hyd. time step
            Pstep,                 // Time pattern time step            
            Rstep,                 // Reporting time step
            Rstep_JunctionsPipes,  // Reporting time step for junctions and pipes (usually 10 times less than Rstep)
            Rstart,                // Time when reporting starts
            Rtime,                 // Next reporting time        
            Rtime_JunctionsPipes,  // Next reporting time for junctions and pipes (usually 10 times less than Rstep)
            Hydstep,               // Actual hydraulic time step
            Qstep,                 // Quality time step
            Qtime,                 // Current quality time
            Rulestep,              // Rule evaluation time step
            Dur;                   // Duration of simulation

        units::time::second_t
            Htime;                  // Current hyd. time, as far as the EPAnet simulation goes

        int Pstart;                // Starting pattern position
    };

    // Reporting Wrapper
    typedef struct {
        FILE* RptFile;           // Report file handle

        int
            Nperiods,              // Number of reporting periods
            PageSize,              // Lines/page in output report/
            Rptflag,               // Report flag
            Tstatflag,             // Report time series statistic flag
            Summaryflag,           // Report summary flag
            Messageflag,           // Error/warning message flag
            Statflag,              // Status report flag
            Energyflag,            // Energy report flag
            Nodeflag,              // Node report flag
            Linkflag,              // Link report flag
            Fprinterr;             // File write error flag

        long
            LineNum,               // Current line number
            PageNum;               // Current page number

        char
            Atime[13],             // Clock time (hrs:min:sec)
            Rpt1Fname[MAXFNAME + 1], // Primary report file name
            Rpt2Fname[MAXFNAME + 1], // Secondary report file name
            DateStamp[26];         // Current date & time

        SField   Field[MAXVAR];  // Output reporting fields

    } Report;

    // Output File Wrapper
    typedef struct {
        char
            HydFname[MAXFNAME + 1],  // Hydraulics file name
            OutFname[MAXFNAME + 1];  // Binary output file name

        int
            Outflag,               // Output file flag
            Hydflag,               // Hydraulics flag
            SaveHflag,             // Hydraulic results saved flag
            SaveQflag,             // Quality results saved flag
            Saveflag;              // General purpose save flag

        long
            HydOffset,             // Hydraulics file byte offset
            OutOffset1,            // 1st output file byte offset
            OutOffset2;            // 2nd output file byte offset

        FILE
            * OutFile,              // Output file handle
            * HydFile,              // Hydraulics file handle
            * TmpOutFile;           // Temporary file handle

    } Outfile;

    // Rule-Based Controls Wrapper
    class Rules {
    public:
        SactionList* ActionList;     // Linked list of action items
        int         RuleState;       // State of rule interpreter
        int         Errcode;         // Rule parser error code
        units::time::second_t        Time1;           // Start of rule evaluation time interval
        cweeTime        Time_t;           // Start of rule evaluation time interval
        Spremise* LastPremise;    // Previous premise clause
        Saction* LastThenAction; // Previous THEN action
        Saction* LastElseAction; // Previous ELSE action
    };

    // Sparse Matrix Wrapper
    class Smatrix {
    public:
        cweeList<cfs_p_ft_t> // cfs_p_ft_t
            Aii;        // Diagonal matrix coeffs.
        cweeList<cfs_p_ft_t> // cfs_p_ft_t
            Aij;        // Non-zero, off-diagonal matrix coeffs.
        cweeList<cubic_foot_per_second_t> // units::flowrate::cubic_foot_per_second_t
            F;          // Right hand side vector
        cweeList<foot_t> // units::length::foot_t
            B_ft;       // Right hand side vector result
        cweeList<squared_cfs_p_ft_t> // squared_cfs_p_ft_t
            temp;       // Array used by linear eqn. solver

        int Ncoeffs;     // Number of non-zero matrix coeffs
        
        cweeList<int> 
            Order,      // Node-to-row of re-ordered matrix
            Row,        // Row-to-node of re-ordered matrix
            Ndx,        // Index of link's coeff. in Aij
            XLNZ,       // Start position of each column in NZSUB
            NZSUB,      // Row index of each coeff. in each column
            LNZ,        // Position of each coeff. in Aij array
            link,       // Array used by linear eqn. solver
            first;      // Array used by linear eqn. solver
    };

    // Hydraulics Solver Wrapper
    class Hydraul {
    public:
        cweeList<foot_t>
            NodeHead;             // Node hydraulic heads
        cweeList<cubic_foot_per_second_t>
            NodeDemand;           // Node demand + emitter flows
        cweeList<cubic_foot_per_second_t>
            DemandFlow;           // Work array of demand flows
        cweeList<cubic_foot_per_second_t>
            EmitterFlow;          // Emitter outflows
        cweeList<cubic_foot_t>
            TankVolume;           // Tank current volume
        cweeList<cubic_foot_per_second_t>
            LinkFlow;             // Link flows
        cweeList<SCALER>
            LinkSetting;          // Link settings
        foot_t
            Htol;                  // Hydraulic head tolerance
        cubic_foot_per_second_t
            Qtol;                  // Flow rate tolerance
        SCALER
            RQtol;                 // Flow resistance tolerance
        SCALER
            Hexp;                  // Exponent in headloss formula
        SCALER
            Qexp;                  // Exponent in emitter formula
        SCALER
            Pexp;                  // Exponent in demand formula
        SCALER
            Pmin;                  // Pressure needed for any demand... ft? head_t?
        SCALER
            Preq;                  // Pressure needed for full demand
        SCALER
            Dmult;                 // Demand multiplier
        SCALER
            Hacc;                  // Relative flow change limit
        cubic_foot_per_second_t
            FlowChangeLimit;       // Absolute flow change limit
        foot_t
            HeadErrorLimit;        // Hydraulic head error limit
        SCALER
            DampLimit;             // Solution damping threshold
        squared_foot_per_second_t
            Viscos;                // Kin. viscosity (sq ft/sec)
        SCALER
            SpGrav;                // Specific gravity
        SCALER
            Epump;                 // Global pump efficiency
        cubic_foot_per_second_t
            Dsystem;               // Total system demand
        Dollar_per_kilowatt_hour_t
            Ecost;                 // Base energy cost per kwh
        Dollar_per_kilowatt_t
            Dcost;                 // Energy demand charge/kw/day
        kilowatt_t
            Emax;                  // Peak energy usage
        kilowatt_t
            Etotal;                // Total energy usage
        SCALER
            RelativeError;         // Total flow change / total flow
        foot_t
            MaxHeadError;          // Max. error for link head loss
        cubic_foot_per_second_t
            MaxFlowChange;         // Max. change in link flow
        SCALER
            DemandReduction;       // % demand reduction at pressure deficient nodes
        SCALER
            RelaxFactor;           // Relaxation factor for flow updating
        cweeList<cfs_p_ft_t>
            P;                    // Inverse of head loss derivatives
        cweeList<cubic_foot_per_second_t>
            Y;                    // Flow correction factors
        cweeList<cubic_foot_per_second_t>
            Xflow;                // Inflow - outflow at each node

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

        cweeThreadedList< StatusType > LinkStatus;  // Link status
        cweeThreadedList< StatusType > OldStatus;  // Previous link/tank status

        Smatrix smatrix;         // Sparse matrix storage
    };

    // Water Quality Solver Wrapper
    class Quality {
    public:
        int
            Qualflag,              // Water quality analysis flag
            OpenQflag,             // Quality system opened flag
            Reactflag,             // Reaction indicator
            OutOfMemory;           // Out of memory indicator

        cweeList<int>
            SortedNodes;          // Topologically sorted node indexes

        cweeStr TraceNode;          // Source node for flow tracing

        char
            ChemName[MAXID + 1],   // Name of chemical
            ChemUnits[MAXID + 1];  // Units of chemical

        squared_foot_per_second_t
            Diffus;                // Diffusivity (sq ft/sec)

        SCALER
            Ctol;                  // Water quality tolerance        
        SCALER
            Wbulk;                 // Avg. bulk reaction rate
        SCALER
            Wwall;                 // Avg. wall reaction rate
        SCALER
            Wtank;                 // Avg. tank reaction rate
        SCALER
            Wsource;               // Avg. mass inflow
        SCALER
            Rfactor;               // Roughness-reaction factor
        SCALER
            Sc;                    // Schmidt Number
        SCALER
            Bucf;                  // Bulk reaction units conversion factor
        SCALER
            Tucf;                  // Tank reaction units conversion factor
        SCALER
            BulkOrder;             // Bulk flow reaction order
        SCALER
            WallOrder;             // Pipe wall reaction order
        SCALER
            TankOrder;             // Tank reaction order
        SCALER
            Kbulk;                 // Global bulk reaction coeff.
        SCALER
            Kwall;                 // Global wall reaction coeff.
        SCALER
            Climit;                // Limiting potential quality
        SCALER
            SourceQual;            // External source quality
           
        cweeList<SCALER>
            TankConcentration;           // Tank current volume
        cweeList<SCALER>
            NodeQual;              // Reported node quality state
        cweeList<SCALER>
            PipeRateCoeff;         // Pipe reaction rate coeffs.

        struct Mempool * SegPool;  // Memory pool for water quality segments

        Pseg
            FreeSeg,               // Pointer to unused segment
            * FirstSeg,            // First (downstream) segment in each pipe
            * LastSeg;             // Last (upstream) segment in each pipe

        cweeList<FlowDirection>
            FlowDir;               // Flow direction for each pipe

        SmassBalance
            MassBalance;           // Mass balance components
    };

    class LeakModel {
    public:
        // typedefs (new unit types)
        typedef unit_t< compound_unit<million_gallon, squared<inverse<month>>> > rateOfRise_t;
        typedef unit_t< compound_unit<million_gallon, inverse<month>> > million_gallon_per_month_t;

    private:
        // Static support functions
        static constexpr million_gallon_t Vol_Lost_At_t(month_t t, scalar_t gamma, rateOfRise_t R, million_gallon_per_month_t init, scalar_t eta) { //only under under one period, if do nothing, init = initial leakge rate
            return (gamma * R * t * t / 2.0) + (eta * init * t); // eta is zero for the case where the first surveyand repair eliminates all leaks
        };
        template <int months, typename T> static constexpr T Get_F_Given_P(T input, scalar_t rate) {
            return input * units::math::cpow<months>(1.0 + rate);
        };
        template <typename T> static T Get_F_Given_P(T input, scalar_t rate, month_t time) {
            return input * cweeMath::Pow(1.0 + rate, time());
        };
        template <int months, typename T> static T  Get_P_Given_F(T input, scalar_t rate) {
            return input * units::math::pow<-1 * months>(1.0 + rate);
        };
        template <typename T> static T Get_P_Given_F(T input, scalar_t rate, month_t time) {
            return input * cweeMath::Pow(1.0 + rate(), -1 * time());
        };
        static Dollar_t CostOfPRV(centimeter_t diam, scalar_t rate) {
            return Get_F_Given_P<1 * 12>(
                1474.8_USD * std::exp(0.0824 * diam())
                + 500_USD * std::exp(0.0658 * diam())
                + (100.5_USD * diam / 1_cm - 300_USD)
                + (412.2_USD * diam / 1_cm - 512_USD)
                , rate
                );

            //return Get_F_Given_P<1 * 12>(
            //    1474.8_USD * std::exp(0.2094 * diam())
            //    + 500_USD * std::exp(0.167 * diam())
            //    + (255.36_USD * diam / 1_in - 300_USD)
            //    + (1047_USD * diam / 1_in - 512_USD)
            //    , rate
            //    );
        };

    public:
        // internal 
        static constexpr scalar_t unitScalar = 1;
        static constexpr scalar_t dis = 0.04 / 12.0; // discount rate
        static constexpr scalar_t g = 0.025 / 12.0; //growth rate
        static constexpr scalar_t inflation = 0.025 / 12.0;
        static constexpr scalar_t leak_reduction_eta = 0; // 0 ONLY WE ASSUME ALL LEAKS FIXED AT TIME 0 SO NO LEAKAGE REMAINING
        static constexpr scalar_t do_nothing_eta = 1.0; // set to 1 to include the benefits of the initial survey and repair over the study period
        static constexpr scalar_t w_prv = 0.025 / 12.0;
        // static constexpr scalar_t N_PRV = 20.0;
        static constexpr scalar_t w_pat = 0.025 / 12;
        // static constexpr scalar_t N_PAT = 20.0;
        static constexpr scalar_t w_E = 0.025 / 12;
        static constexpr Dollar_per_kilowatt_hour_t Ce = 0.12; // $ / KWh
        static constexpr Dollar_per_gallon_t Cvp = 682.6732941_USD / 1_ac_ft; // 0.001215862; // $ / gallon
        static constexpr million_gallon_per_month_t connectionLeakRate = 4_gpd;
        static constexpr AUTO SystemLeakRatePerMile = connectionLeakRate * ((92950900.0 / 4.0) / 365.0) / 852.95_mi;
        static constexpr scalar_t PercRigid = 0.75;
        static constexpr scalar_t w_F = 0.025 / 12.0;
        static constexpr scalar_t w_M = 0.025 / 12.0;
        static constexpr scalar_t w_su = 0.025 / 12.0;
        static constexpr million_gallon_per_month_t  L = 500000.00_gal / 1_yr; // flowrate per leak average(gal / year) / months per year = gal / month
        static constexpr Dollar_t 	Calr = 5340.7; // cost per leak to repair
        static constexpr scalar_t 	Dldr = 1; // true positive rate for leak detection, cannot find for Marin in Amanda's data
        static constexpr Dollar_per_mile_t M = Get_F_Given_P<1 * 12>(605_USD_p_mi, w_M);
        static constexpr Dollar_t F = Get_F_Given_P<1 * 12>(Calr / Dldr, w_F); // adjust for one year, 2022 publication
    
    public:
        SCALER ICF = 1.3; // # infrastructure condition factor
        SCALER ILI = 1.1; // # infrastructure leakage index

        // evaluate the leak model on a network, assuming leak rates on pipe lengths, as a function of zone pressures and zone pipes.
        

        std::map<std::string, cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>> LeakModelResults(
            mile_t MilesMains,
            year_t surveyFrequency,
            cweeList<Pzone> new_zones, 
            pounds_per_square_inch_t avgPressure_old,
            pounds_per_square_inch_t avgPressure_new
        ) {
            // compare this zone to all of the zones, and determine what this zone's weighted responsibility is for the net leakage            
            scalar_t N1 = 1.5 - (1.0 - (2.0 / 3.0) * (ICF / ILI)) * PercRigid;
            
            AUTO LeakRatePerMile = connectionLeakRate * ((92950900.0 / 4.0) / 365.0) / 852.95_mi;
            rateOfRise_t R = LeakRatePerMile * MilesMains / 365_d;
            million_gallon_per_day_t totalWaterDemand = 0;
            mile_t totalSystemLength = 0;
            scalar_t numValves = 0;
            scalar_t numERTs = 0;
            scalar_t numJunctions = 0;
            scalar_t numCustomers = 0;
            inch_t avgPipeDiameter = 0; int pipeCount = 0;
            if (new_zones.Num () > 0) {
                pounds_per_square_inch_t avgSystemPressure = 0;                
                double totalSysRatio = 0;
                million_gallon_per_month_t totalSystemLeakage = 0;
                cweeList< pounds_per_square_inch_t > pressures;
                {
                    for (auto& zone : new_zones) {
                        if (zone) {
                            for (auto& link : zone->Within_Link) {
                                totalSystemLength += link->Len;
                                units::math::rolling_avg<inch_t>(avgPipeDiameter, link->Diam, pipeCount);
                            }
                            for (auto& link : zone->Boundary_Link) {
                                if (link.second == epanet::direction_t::FLOW_IN_DMA) {
                                    if (link.first->Type_p == asset_t::VALVE) {
                                        AUTO valve = link.first.CastReference< epanet::Svalve >();
                                        if (valve) {
                                            if (valve->ProducesElectricity) {
                                                numERTs++;
                                            }
                                            numValves++;
                                        }
                                    }
                                }                                
                            }
                            pressures.Append(zone->AverageNodePressure());
                            for (auto& node : zone->Node) {
                                if (node && node->Type_p == asset_t::JUNCTION) {
                                    numJunctions += 1;
                                    if (node->HasWaterDemand()) {
                                        totalWaterDemand += node->GetValue<_DEMAND_>()->GetAvgValue();
                                        numCustomers += 1;
                                    }
                                    
                                }
                            }
                        }
                    }
                    int Pcount = 0;
                    for (auto& pressure : pressures) cweeMath::rollingAverageRef<pounds_per_square_inch_t>(avgSystemPressure, pressure, Pcount);
                    totalSystemLeakage = totalSystemLength * SystemLeakRatePerMile;                    
                    Pcount = 0;
                    avgSystemPressure += 0.00001_psi;
                    for (auto& zone : new_zones) {
                        mile_t _MilesMains = 0;
                        for (auto& link : zone->Within_Link) _MilesMains += link->Len;
                        totalSysRatio += cweeMath::Pow((pressures[Pcount] / avgSystemPressure)(), N1()) * _MilesMains();
                        Pcount++;
                    }
                }
                if (totalSysRatio == 0) { totalSysRatio = 1; };
                auto thisRatio = cweeMath::Pow((avgPressure_new / avgSystemPressure)(), N1()) * MilesMains() / totalSysRatio; // this would sum to one across all zones
                R = (thisRatio * totalSystemLeakage) / 365_d;
            }

            // SYSTEM CHARACTERISTICS / ANALYSIS VARIABLES
            month_t surveyFrequency_Months = surveyFrequency; 
            surveyFrequency_Months = units::math::round(surveyFrequency_Months); // must be in increments of months to be valid
            surveyFrequency_Months = units::math::max(surveyFrequency_Months, (month_t)1);

            pounds_per_square_inch_t P0 = avgPressure_old/* + 0.00001_psi*/; // # initial pressure
            pounds_per_square_inch_t P1 = avgPressure_new; // # pressure after reduction
            mile_t d = MilesMains; // miles of mains

            // stepsize
            month_t stepSize = 1;

            // months
            cweeList<month_t> t;
            for (month_t i = 0; i <= 30_yr; i += stepSize) t.Append(i);

            // define the rate of rise of leakage           
            scalar_t gamma = (P0 == 0_psi) ? 1.0 : cweeMath::Pow((P1 / P0)(), N1()); // # fraction leakage rate remaining after pressure reduction, about 0.6 right now
            million_gallon_per_month_t init = R * 3.66424_yr; // # initial leakage rate, gallons / yr... solved by: (1 / (R/(1045.244408_ac_ft_y))). 1045_ac_ft_y came from 2022 water audit data for Marin.

            // calculate water lost volume and value
            cweeList<million_gallon_t> Water_Lost_Volume; Water_Lost_Volume.AssureSize(t.size(), 0_MG);
            cweeList<Dollar_t> Water_Lost_Value; Water_Lost_Value.AssureSize(t.size(), 0_USD); {
                bool firstPass = true;
                month_t zeroMonth = 0;
                int indexFollowingSurvey = 0;
                for (int t_ind = 0; t_ind < t.size(); t_ind++) { month_t idx = t[t_ind];                    
                    if (idx == zeroMonth || units::math::round(units::math::fmod(idx, surveyFrequency_Months)) == zeroMonth) {
                        if (firstPass) {
                            Water_Lost_Volume[t_ind] = 0;
                            firstPass = false;
                        }
                        else {
                            Water_Lost_Volume[t_ind] = // was 0
                                Vol_Lost_At_t(surveyFrequency_Months, gamma, R, init, leak_reduction_eta) -
                                Vol_Lost_At_t(surveyFrequency_Months - month_t(1), gamma, R, init, leak_reduction_eta);
                        }
                        indexFollowingSurvey = 0;
                    }
                    else {
                        Water_Lost_Volume[t_ind] = 
                            Vol_Lost_At_t(t[indexFollowingSurvey], gamma, R, init, leak_reduction_eta) -
                            Vol_Lost_At_t(t[indexFollowingSurvey-1], gamma, R, init, leak_reduction_eta);
                    }
                    indexFollowingSurvey++;                    
                }
                for (int t_ind = 0; t_ind < t.size(); t_ind++) { month_t idx = t[t_ind];
                    Water_Lost_Value[t_ind] = Water_Lost_Volume[t_ind] * Get_F_Given_P(Cvp, g, idx);
                }
            }

            // calculate the water lost under the do-nothing scenario
            cweeList<million_gallon_t> Water_Lost_Do_Nothing; Water_Lost_Do_Nothing.AssureSize(t.size(), 0_MG); {
                month_t idx_i = 0;
                bool firstPass = true;
                month_t zeroMonth = 0;
                for (int t_ind = 0; t_ind < t.size(); t_ind++) { month_t idx = t[t_ind];
                    if (firstPass) {
                        Water_Lost_Do_Nothing[t_ind] = 0;
                        firstPass = false;
                    }
                    else {
                        Water_Lost_Do_Nothing[t_ind] =
                            Vol_Lost_At_t(t[t_ind], 1, R, init, do_nothing_eta) -
                            Vol_Lost_At_t(t[t_ind - 1], 1, R, init, do_nothing_eta);
                    }
                }
            }

            // calculate the water saved volume and value for each time period
            cweeList<million_gallon_t> Water_Saved_Volume; Water_Saved_Volume.AssureSize(t.size(), 0_MG); {
                for (int i = 0; i < Water_Saved_Volume.size(); i++) {
                    Water_Saved_Volume[i] = Water_Lost_Do_Nothing[i] - Water_Lost_Volume[i];
                }
            }
            cweeList<Dollar_t> Water_Saved_Value; Water_Saved_Value.AssureSize(t.size(), 0_USD); {
                month_t idx_i = 0;
                for (int t_ind = 0; t_ind < t.size(); t_ind++) { month_t idx = t[t_ind];
                    Water_Saved_Value[t_ind] = Water_Saved_Volume[t_ind] * Get_F_Given_P(Cvp, g, idx);
                }
            }
            million_gallon_t TotalWaterSaved = 0_MG; for (auto& x : Water_Saved_Volume) TotalWaterSaved += x;

            // calculate costs of surveying over time period
            Dollar_t 	D = d * M; // D full leak detection survey cost

            // surveys after initial
            Dollar_t Cs_0 = (R * surveyFrequency_Months / L) * F + D; // cost of survey in year 0 dollars
            cweeList<Dollar_t> SurveyRep; SurveyRep.AssureSize(t.size(), 0_USD); {
                month_t idx_i = 0;
                month_t zeroMonth = 0;
                for (int t_ind = 0; t_ind < t.size(); t_ind++) { month_t idx = t[t_ind];
                    if (units::math::fmod(idx, surveyFrequency_Months) == zeroMonth) {
                        SurveyRep[t_ind] = Get_F_Given_P(Cs_0, w_su, idx);
                    }
                    else { // only take the periods the surveys are actually done
                        SurveyRep[t_ind] = 0_USD;
                    }
                }
            }

            // initial survey
            if (do_nothing_eta >= unitScalar) {
                SurveyRep[0] = (init / L) * F + D; // Initial survey cost
            }
            else {
                SurveyRep[0] = 0_USD;
            }

            // calculate net present values, smarter ways to do this, not worrying about it now
            Dollar_t NPV_costs = 0;
            {
                for (int t_ind = 0; t_ind < t.size(); t_ind++) { month_t idx = t[t_ind];
                    try {
                        NPV_costs += Get_P_Given_F(SurveyRep[t_ind], dis, idx);
                        NPV_costs += Get_P_Given_F(Water_Lost_Value[t_ind], dis, idx);
                    }
                    catch (...) {}
                }
            }

            Dollar_t NPV_benefits = 0;
            {
                month_t idx_i = 0;
                for (int t_ind = 0; t_ind < t.size(); t_ind++) { month_t idx = t[t_ind];
                    NPV_benefits += Get_P_Given_F(Water_Saved_Value[t_ind], dis, idx);
                }
            }

            std::map<std::string, cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>> results; {
                results["Total Costs"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::Dollar(NPV_costs), cweeUnitValues::cweeUnitPattern());
                results["Total Benefits"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::Dollar(NPV_benefits), cweeUnitValues::cweeUnitPattern());
                results["Net Benefits"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::Dollar(NPV_benefits - NPV_costs), cweeUnitValues::cweeUnitPattern());
                results["Total Water Saved"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::million_gallon(TotalWaterSaved), cweeUnitValues::cweeUnitPattern());
                results["Total Pipe Length"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::mile(MilesMains), cweeUnitValues::cweeUnitPattern());
                results["Initial Leak Rate"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::million_gallon_per_day((million_gallon_per_day_t)init), cweeUnitValues::cweeUnitPattern());
                {
                    AUTO pat = cweeUnitValues::cweeUnitPattern(cweeUnitValues::second(), cweeUnitValues::million_gallon());

                    for (int t_ind = 0; t_ind < t.size(); t_ind++) {
                        month_t idx = t[t_ind];
                        cweeTime t = cweeTime::Now().ToStartOfMonth() + second_t(idx)();

                        pat.AddValue(cweeUnitValues::second((u64)t), cweeUnitValues::million_gallon(Water_Lost_Do_Nothing[t_ind]));
                    }  

                    results["Water_Lost_Do_Nothing"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::unit_value(0), cweeUnitValues::cweeUnitPattern(pat));
                }
                {
                    AUTO pat = cweeUnitValues::cweeUnitPattern(cweeUnitValues::second(), cweeUnitValues::million_gallon());

                    for (int t_ind = 0; t_ind < t.size(); t_ind++) {
                        month_t idx = t[t_ind];
                        cweeTime t = cweeTime::Now().ToStartOfMonth() + second_t(idx)();

                        pat.AddValue(cweeUnitValues::second((u64)t), cweeUnitValues::million_gallon(Water_Lost_Volume[t_ind]));
                    }

                    results["Water_Lost_Volume"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::unit_value(0), cweeUnitValues::cweeUnitPattern(pat));
                }
                {
                    AUTO pat = cweeUnitValues::cweeUnitPattern(cweeUnitValues::second(), cweeUnitValues::million_gallon());

                    for (int t_ind = 0; t_ind < t.size(); t_ind++) {
                        month_t idx = t[t_ind];
                        cweeTime t = cweeTime::Now().ToStartOfMonth() + second_t(idx)();

                        pat.AddValue(cweeUnitValues::second((u64)t), cweeUnitValues::million_gallon(Water_Saved_Volume[t_ind]));
                    }

                    results["Water_Saved_Volume"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::unit_value(0), cweeUnitValues::cweeUnitPattern(pat));
                }
            }

            return results;
        };

        // evaluate the leak model on a network repeatably to find the most cost-effective survey frequency
        year_t BestSurveyFrequency(mile_t MilesMains, cweeList<Pzone> new_zones, pounds_per_square_inch_t avgPressure_old, pounds_per_square_inch_t avgPressure_new) {
            Dollar_t bestNetBenefit = -std::numeric_limits<Dollar_t>::max();
            month_t bestMonth = 1;
            Dollar_t thisV = 0;

            std::map< month_t, Dollar_t> cache;

            // test entire sampel space at low resolution
            for (month_t S = 1; S <= 30_yr; S += month_t(12)) {
                auto r = LeakModelResults(MilesMains, S, new_zones, avgPressure_old, avgPressure_new);
                cache[S] = r["Net Benefits"].get<0>()();
                if (cache[S] > bestNetBenefit) {
                    bestNetBenefit = cache[S];
                    bestMonth = S;
                }
            }

            // Dial-in to local 3-year period
            month_t bestMonthPrev = bestMonth;
            for (month_t S = units::math::max(month_t(1), (bestMonthPrev - month_t(18))); S <= units::math::min(30_yr, (bestMonthPrev + month_t(18))); S += month_t(2)) {                
                if (cache.count(S) > 0) { 
                    thisV = cache[S];
                } 
                else {
                    cache[S] = LeakModelResults(MilesMains, S, new_zones, avgPressure_old, avgPressure_new)["Net Benefits"].get<0>()();
                    thisV = cache[S];
                }

                if (thisV > bestNetBenefit) {
                    bestNetBenefit = thisV;
                    bestMonth = S;
                }
            }

            // Dial-in to specific month
            bestMonthPrev = bestMonth;
            for (month_t S = units::math::max(month_t(1), (bestMonthPrev - month_t(4))); S <= units::math::min(30_yr, (bestMonthPrev + month_t(4))); S += month_t(1)) {
                if (cache.count(S) > 0) {
                    thisV = cache[S];
                }
                else {
                    cache[S] = LeakModelResults(MilesMains, S, new_zones, avgPressure_old, avgPressure_new)["Net Benefits"].get<0>()();
                    thisV = cache[S];
                }

                if (thisV > bestNetBenefit) {
                    bestNetBenefit = thisV;
                    bestMonth = S;
                }
            }

            if (bestMonth < 1_yr) return 30_yr;
            else return bestMonth;
        };

        /*! evalMode: 0 = normal, 1 = force as PRV, 2 = force as ERT
        evaluate the net present 30 - year value of a PRV or ERT valve, including accounting for energy generation revenue.Should be combined with LeakModelResults to estimate benefits from pressure reduction, which is not done directly here.
        */
        Dollar_t NetPresentValueOfValve(Pvalve valve, int evalMode = 0) {
            // Bad Data Early Exit
            if (!valve) return 0_USD;

            // AVERAGE FLOW
            AUTO flowPat = valve->GetValue<_FLOW_>();
            cubic_meter_per_second_t avgFlow = units::math::fabs(flowPat->GetAvgValue());

            // AVERAGE HEADLOSS
            AUTO headPat = valve->GetValue<_HEADLOSS_>();
            meter_t avgHeadloss = units::math::fabs(headPat->GetAvgValue());

            // AVERAGE ENERGY PRODUCTION
            kilowatt_t avgEnergy = 0; // ISSUE IS HERE.
            if (valve->ProducesElectricity || evalMode == 2) {
                // build the energy generation (Building it here from flow/head instead of using the sim result because of evalMode)
                AUTO new_energy_pat = (*flowPat) * cwee_units::constants::d * cwee_units::constants::g * (*headPat) * (131.0 / 100.0);
                avgEnergy = cwee_units::math::fabs(new_energy_pat.GetAvgValue());
            }

            // stepsize
            month_t stepSize = 1;

            // months
            cweeList<month_t> t;
            for (month_t i = 0; i <= 30_yr; i += stepSize) t.Append(i);

            // Capital Costs, Operation & Maintenance
            cweeList<Dollar_t> Ci_T; Ci_T.AssureSize(t.size(), 0_USD);
            cweeList<Dollar_t> OM_T; OM_T.AssureSize(t.size(), 0_USD);
            cweeList<Dollar_t> EN_T; EN_T.AssureSize(t.size(), 0_USD);
            if (evalMode == 1 || (evalMode == 0 && !valve->ProducesElectricity)) {
                // PRV: Installation fees, no maintenance, no energy
                AUTO C_PRV = CostOfPRV(valve->Diam, w_prv);
                Ci_T[0] += /*N_PRV * */C_PRV;
            }
            else {
                // ERT: Installation, maintenance, and energy generation
                Dollar_t C_PAT;
                if ((avgFlow > 3_lps && avgHeadloss > 3_m && avgFlow < 320_lps && avgHeadloss < 352_m) || (avgFlow > 1_lps && avgHeadloss > 10_m && avgFlow > 62_lps && avgHeadloss > 311_m)) {
                    // good range
                    C_PAT = Get_F_Given_P<4 * 12>(11913.91_USD * avgFlow() * std::sqrt(avgHeadloss()), w_pat); // # average, inflated from 2019 to 2023 dollars
                }
                else {
                    // bad range
                    C_PAT = ::Max<Dollar_t>(
                        CostOfPRV(valve->Diam, w_pat),
                        Get_F_Given_P<4 * 12>(11913.91_USD * avgFlow() * std::sqrt(avgHeadloss()), w_pat)
                    ) * 10; // # average, inflated from 2019 to 2023 dollars, increased 10x for a hypothetical "custom" installation
                }
                                
                C_PAT += (1.0 - 0.26) * C_PAT / 0.26;
                Ci_T[0] += /*N_PAT * */C_PAT;

                for (int i = 0; i < OM_T.size(); i++) {
                    OM_T[i] += Get_F_Given_P((0.15 / 12) * C_PAT, w_pat, t[i]);
                };

                kilowatt_hour_t E_Production_Monthly = avgEnergy * month_t(1);
                for (int i = 0; i < EN_T.size(); i++) {
                    EN_T[i] += E_Production_Monthly * Get_F_Given_P(Ce, w_E, t[i]);
                };
            }
            OM_T[0] = 0_USD;

            // Costs
            Dollar_t NPV_costs = 0;
            for (int idx = 0; idx < t.size(); idx++) {
                try {
                    NPV_costs += Get_P_Given_F(Ci_T[idx], dis, t[idx]);
                    NPV_costs += Get_P_Given_F(OM_T[idx], dis, t[idx]);
                }
                catch (...) {}
            }            
            
            // Benefits
            Dollar_t NPV_benefits = 0;
            for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
                NPV_benefits += Get_P_Given_F(EN_T[idx()], dis, t[idx()]);
            }

            // return NPV
            return NPV_benefits - NPV_costs;
        };

    };

    // Pipe Network Wrapper
    class Network {
    public:
        Network() :
            Asset(std::function<size_t(cweeSharedPtr<Sasset> const&)>([](const cweeSharedPtr<Sasset>& v)->size_t { if (v) { return v->Hash(); } return 0; })), 
            System(make_cwee_shared<Sasset>(asset_t::ANY, cweeAssetValueCollection<asset_t::ANY>::Values(), "System"))
        {
            Asset.AddOrReplace(System);
        };

        int
            Nnodes,                // Number of network nodes
            Ntanks,                // Number of tanks
            Njuncs,                // Number of junction nodes
            Nlinks,                // Number of network links
            Npipes,                // Number of pipes
            Npumps,                // Number of pumps
            Nvalves,               // Number of valves
            Ncontrols,             // Number of simple controls
            Nrules,                // Number of control rules
            Npats,                 // Number of time patterns
            Ncurves;               // Number of data curves

        cweeSharedPtr<Sasset> System;
        cweeThreadedSet< cweeSharedPtr<Sasset>, size_t > Asset;
        cweeList<Pnode> Node;                       // Node array
        cweeList<Plink> Link;                       // Link array
        cweeList<Ptank> Tank;                       // Tank array
        cweeList<Ppump> Pump;                       // Pump array
        cweeList<Pvalve> Valve;                     // Valve array
        cweeList<Pzone> Zone;                       // Zone array
        cweeList<Ppattern> Pattern;                 // Time pattern array
        cweeList<Pcurve> Curve;                     // Data curve array
        cweeList<Pcontrol> Control;                 // Simple controls array
        cweeList<Prule> Rule;                       // Rule-based controls array
        HashTable
            * NodeHashTable,                        // Hash table for Node ID names
            * LinkHashTable;                        // Hash table for Link ID names
        cweeThreadedList<Padjlist> Adjlist;         // Node adjacency lists
        LeakModel       Leakage;

        cweeList<::epanet::Pzone>         getCalibrationOrder() const;
    };
    using EN_Network = cweeSharedPtr<Network>; // Network*;

    // Overall Project Wrapper
    class Project {
    public:
        Project() : network(make_cwee_shared<Network>()) {};
        ~Project() {};

        EN_Network network;                 // Pipe network wrapper
        Parser     parser;                  // Input file parser wrapper
        Times      times;                   // Time step wrapper
        Report     report;                  // Reporting wrapper
        Outfile    outfile;                 // Output file wrapper
        Rules      rules;                   // Rule-based controls wrapper
        Hydraul    hydraul;                 // Hydraulics solver wrapper
        Quality    quality;                 // Water quality solver wrapper

        SCALER Ucf[MAXVAR];                 // Unit conversion factors

        int
            Openflag,                       // Project open flag
            Warnflag;                       // Warning flag

        char
            Msg[MAXMSG + 1],                // General-purpose string: errors, messages
            Title[MAXTITLE][TITLELEN + 1],  // Project title
            MapFname[MAXFNAME + 1];         // Map file name

        cweeStr
            TmpHydFname,                    // Temporary hydraulics file name
            TmpOutFname,                    // Temporary output file name
            TmpStatFname;                   // Temporary statistic file name  

        void (*viewprog) (char*);           // Pointer to progress viewing function

        template<typename desiredUnit> desiredUnit convertToUnit(SCALER v, bool pipe_diameter = false) {
            if constexpr (units::traits::is_length_unit< desiredUnit >()) {
                if (pipe_diameter) { // dealing with a pipe diameter... 
                    if (parser.Unitsflag == ::epanet::US) {
                        return (inch_t)(double)v;
                    }
                    else { // SI units
                        return (millimeter_t)(double)v;
                    }
                }
                else {
                    if (parser.Unitsflag == ::epanet::US) {
                        return (foot_t)(double)v;
                    }
                    else { // SI units
                        return (meter_t)(double)v;
                    }
                }
            }
            else if constexpr (units::traits::is_area_unit< desiredUnit >()) {
                if (parser.Unitsflag == ::epanet::US) {
                    return (square_foot_t)(double)v;
                }
                else { // SI units
                    return (square_meter_t)(double)v;
                }                
            }
            else if constexpr (units::traits::is_energy_cost_rate_unit< desiredUnit >()) {
                return (Dollar_per_kilowatt_hour_t)(double)v;
            }
            else if constexpr (units::traits::is_power_unit< desiredUnit >()) {
                if (parser.Unitsflag == ::epanet::US) {
                    return (horsepower_t)(double)v;
                }
                else { // SI units
                    return (kilowatt_t)(double)v;
                }
            }
            else if constexpr (units::traits::is_volume_unit< desiredUnit >()) {
                if (parser.Unitsflag == ::epanet::US) {
                    return (cubic_foot_t)(double)v;
                }
                else { // SI units
                    return (cubic_meter_t)(double)v;
                }
            }
            else if constexpr (units::traits::is_flowrate_unit< desiredUnit >()) {
                switch (parser.Flowflag) {
                case ::epanet::CFS: return (cubic_foot_per_second_t)(double)v;          // cubic feet per second
                case ::epanet::GPM: return (gallon_per_minute_t)(double)v;          // gallons per minute
                case ::epanet::MGD: return (million_gallon_per_minute_t)(double)v;          // million gallons per day
                case ::epanet::IMGD:return (imperial_million_gallon_per_day_t)(double)v;          // imperial million gal. per day
                case ::epanet::AFD: return (acre_foot_per_day_t)(double)v;           // acre-feet per day
                case ::epanet::LPS: return (liter_per_second_t)(double)v;           // liters per second
                case ::epanet::LPM: return (liter_per_minute_t)(double)v;           // liters per minute
                case ::epanet::MLD: return (megaliter_per_day_t)(double)v;           // megaliters per day
                case ::epanet::CMH: return (cubic_meter_per_hour_t)(double)v;           // cubic meters per hour
                case ::epanet::CMD: return (cubic_meter_per_day_t)(double)v;            // cubic meters per day
                }
            }
            else if constexpr (units::traits::is_pressure_unit< desiredUnit >()) {
                switch (parser.Pressflag) {
                case PSI:
                    return ((pounds_per_square_inch_t)(double)v) / hydraul.SpGrav;
                    break;
                case KPA:
                    return ((kilopascal_t)(double)v) / hydraul.SpGrav;
                    break;
                case METERS:
                    return ((head_t)(double)(meter_t)(double)v) / hydraul.SpGrav;
                    break;
                }
            }
            else if constexpr (units::traits::is_dimensionless_unit< desiredUnit >()) {
                return (scalar_t)(double)v;
            }
            else if constexpr (units::traits::is_time_unit< desiredUnit >()) {
                return (second_t)(double)v;
            }
            else {
                static_assert(false, "This unit type was not enumerated against");
                return (desiredUnit)(double)v;
            }
        };
        bool		incontrols(int objType, int index)
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
            EN_Network net = this->network;

            int i, ruleObject;
            Spremise* premise;
            Saction* action;

            // Check simple controls
            for (i = 1; i <= net->Ncontrols; i++)
            {
                if (objType == NODE && net->Control[i]->Node == index) return true;
                if (objType == LINK && net->Control[i]->Link == index) return true;
            }

            // Check rule-based controls
            for (i = 1; i <= net->Nrules; i++)
            {
                // Convert objType to a rule object type
                if (objType == NODE) ruleObject = 6;
                else                 ruleObject = 7;

                // Check rule's premises
                premise = net->Rule[i]->Premises;
                while (premise != NULL)
                {
                    if (ruleObject == premise->object && premise->index == index) return true;
                    premise = premise->next;
                }

                // Rule actions only need to be checked for link objects
                if (objType == LINK)
                {
                    // Check rule's THEN actions
                    action = net->Rule[i]->ThenActions;
                    while (action != NULL)
                    {
                        if (action->link == index) return true;
                        action = action->next;
                    }

                    // Check rule's ELSE actions
                    action = net->Rule[i]->ElseActions;
                    while (action != NULL)
                    {
                        if (action->link == index) return true;
                        action = action->next;
                    }
                }
            }
            
            return false;
        };

        cweeList<::epanet::Plink>         getConnectedLinks(int node_index) {
            cweeList<::epanet::Plink> out;

            auto& links = network->Link;
            if (!Openflag) return out;
            if (node_index < 1 || node_index > network->Nnodes) return out;

            Padjlist adj = network->Adjlist[node_index];
            while (adj) {
                if (links[adj->link]) {
                    if (links[adj->link]->N1 == node_index) {
                        out.Append(links[adj->link]);
                    }
                    else if (links[adj->link]->N2 == node_index) {
                        out.Append(links[adj->link]);
                    }
                }
                adj = adj->next;
            }
            
            return out;
        };        
        cweeList<Pnode>    findDeadEnds();
    };
    using EN_Project = cweeSharedPtr<Project>;

    template<typename desiredUnit> static desiredUnit convertToUnit(EN_Project pr, double v, bool pipe_diameter = false) {
        return pr->convertToUnit<desiredUnit>(v, pipe_diameter);
    };

    INLINE cubic_foot_t Stank::Volume(cweeSharedPtr<Project> pr, foot_t head) {
        cubic_foot_t out = Area() * (Hmin - El);
        foot_t lvl = units::math::fmax(head - Hmin, 0.0_ft);
        if (Vcurve_Actual) {
            out = pr->convertToUnit<cubic_foot_t>(Vcurve_Actual->Curve.GetCurrentValue((double)(units::math::fmax(head - El, 0.0_ft) * pr->Ucf[ELEV])));
        }
        else {
            out += Area() * lvl;
        }
        return out;
    };
    INLINE cubic_foot_t  Stank::Vmin(cweeSharedPtr<Project> pr) {
        return Volume(pr, this->Hmin);
    };      // minimum volume
    INLINE cubic_foot_t  Stank::Vmax(cweeSharedPtr<Project> pr) {
        return Volume(pr, this->Hmax);
    };      // maximum volume
    INLINE cweeTime    Scontrol::Time_Adjusted(cweeSharedPtr<Project> pr) {
        return pr->times.GetCalibrationDateTime() + (u64)this->Time;
    };

    INLINE cweeList< Pnode > Szone::FindDeadends(cweeSharedPtr<Project> pr) {
        cweeList< Pnode > DeadEnds;

        for (auto& node : this->Node) { // contained nodes
            if (node && node->Type_p == asset_t::JUNCTION) {
                // potential candidate...
                AUTO node_index = hashtable_t::hashtable_find(pr->network->NodeHashTable, (char*)(node->Name_p.c_str()));
                AUTO connectedLinks = pr->getConnectedLinks(node_index);
                if (connectedLinks.size() <= 1) {  // dead-end or isolated
                    // is this link used by any rules?
                    if (connectedLinks.size() >= 1){
                        AUTO link_index = hashtable_t::hashtable_find(pr->network->LinkHashTable, (char*)(connectedLinks[0]->Name_p.c_str()));
                        if (pr->incontrols(LINK, link_index)) { // cannot use this
                            continue;
                        }
                    }
                    
                    // is this junction referenced by any rules?
                    if (pr->incontrols(NODE, node_index)) { // cannot use this
                        continue;
                    }

                    // isolated and unreferenced. Great candidate for deletion.
                    DeadEnds.Append(node);
                }
            }
        }

        return DeadEnds;
    };
    INLINE cweeList< cweeUnion<Passet, direction_t> > Szone::GetMassBalanceAssets(cweeSharedPtr<Project> pr) {
        cweeList< cweeUnion<Passet, direction_t> > out;

        for (auto& link_dir : this->Boundary_Link) {
            if (link_dir.first) {
                // is this link always closed? If so, exclude it.
                AUTO assetPtr = link_dir.first.CastReference<Sasset>();
                if (assetPtr) {
                    AUTO link_index = hashtable_t::hashtable_find(pr->network->LinkHashTable, (char*)(assetPtr->Name_p.c_str()));
                    if (pr->incontrols(LINK, link_index)) { // controlled == wonderful candidate
                        out.emplace_back(cweeUnion<Passet, direction_t>(assetPtr, link_dir.second));
                    }
                    else {
                        // not controlled.. is it open by default?
                        if ((::epanet::StatusType)(double)link_dir.first->Status(pr->times.GetSimStartTime()) == ::epanet::CLOSED) {
                            // bad option!
                            continue;
                        }
                        out.emplace_back(cweeUnion<Passet, direction_t>(assetPtr, link_dir.second));
                    }
                }
            }
        }

        for (auto& node : this->Node) {
            if (node && node->Type_p == asset_t::RESERVOIR) {
                AUTO tank = node.CastReference<Stank>();
                if (tank) {
                    AUTO assetPtr = node.CastReference<Sasset>();
                    if (assetPtr) {
                        out.emplace_back(cweeUnion<Passet, direction_t>(assetPtr, direction_t::FLOW_WITHIN_DMA));
                    }
                }
            }
        }

        return out;
    };

    INLINE cweeUnitValues::cweeUnitPattern Szone::TryCalibrateZone(cweeSharedPtr<Project> pr, std::map<std::string, cweeUnitValues::cweeUnitPattern> const& ScadaSources) {
        // if zone is ill-defined, then that must be addressed before calibration
        if (IsIllDefined()) throw(std::exception(cweeStr::printf("Zone %s is ill-defined (%s). Ill-defined zones cannot be calibrated until after they are declared \"Okay\".", this->Name_p.c_str(), this->IllDefined.ToString()).c_str()));

        cweeList< cweeUnion<Passet, direction_t> > massBalanceAssets = GetMassBalanceAssets(pr);
        // for each asset, get the scada source as well as the simulation values

        cweeList< cweeUnitValues::cweeUnitPattern > contributions;
        for (auto& asset_direction : massBalanceAssets) {
            Passet& asset = asset_direction.get<0>();
            direction_t& dir = asset_direction.get<1>();
            switch (asset->Type_p) {
            case asset_t::RESERVOIR:  // works for tanks or water sources
                if (ScadaSources.count(asset->Name_p.c_str()) > 0) {
                    AUTO resPtr = asset.CastReference<Stank>();
                    // check the units. If in GPM, then great. 
                    auto& pat = ScadaSources.at(asset->Name_p.c_str());
                    if (pat.Y_Type().AreConvertableTypes(1_gpm)) { // demand 
                        contributions.Append(pat * -1);
                        // sumDemands -= pat;
                    }
                    else if (pat.Y_Type().AreConvertableTypes(1_ft)) { // level
                        // convert level to demand.
                        // when level increases, positive demand. 
                        
                        AUTO pat1 = cweeUnitValues::cweeUnitPattern(pat);
                        pat1.ShiftTime(-1_hr);

                        
                        AUTO pat2 = cweeUnitValues::cweeUnitPattern(pat1.X_Type(), 1_gal);
                        pat2 = ((pat1 - cweeUnitValues::cweeUnitPattern(pat)) * (units::math::pow<2>(resPtr->Diameter)) * cweeMath::PI / 4.0); // correct conversion to gallons
                        
                        AUTO pat3 = cweeUnitValues::cweeUnitPattern(pat1.X_Type(), 1_gph); // force-cast to account for the 1-hour adjustment
                        pat3 = pat2;

                        AUTO pat4 = cweeUnitValues::cweeUnitPattern(pat1.X_Type(), 1_cfs); // 
                        pat4 = pat3;

                        pat4.ShiftTime(0.5_hr); // final adjustment for the "peaks"

                        contributions.Append(pat4 * -1);
                        // sumDemands -= pat4;
                    }
                }
                else {
                    // sumDemands -= cweeUnitValues::cweeUnitPattern(*asset->GetValue<_DEMAND_>());
                    contributions.Append(cweeUnitValues::cweeUnitPattern(*asset->GetValue<_DEMAND_>()) * -1);

                    // positive demand = filling. Negative tank demand = positive customer demand.
                }                
                break;
            case asset_t::PUMP:
            case asset_t::VALVE:
            case asset_t::PIPE:
                if (ScadaSources.count(asset->Name_p.c_str()) > 0) {
                    auto& pat = ScadaSources.at(asset->Name_p.c_str());
                    if (dir == direction_t::FLOW_IN_DMA) {
                        contributions.Append(pat);
                        // sumDemands += pat;
                    }
                    else {
                        contributions.Append(pat * -1);
                        // sumDemands -= pat;
                    }
                }
                else {
                    if (dir == direction_t::FLOW_IN_DMA) {
                        contributions.Append(cweeUnitValues::cweeUnitPattern(*asset->GetValue<_FLOW_>()));
                        // sumDemands += cweeUnitValues::cweeUnitPattern(*asset->GetValue<_FLOW_>());
                    }
                    else {
                        contributions.Append(cweeUnitValues::cweeUnitPattern(*asset->GetValue<_FLOW_>()) * -1);
                        // sumDemands -= cweeUnitValues::cweeUnitPattern(*asset->GetValue<_FLOW_>());
                    }
                }
                break;
            }
        }
       
        cweeUnitValues::cweeUnitPattern sumDemands = cweeUnitValues::cweeUnitPattern(1_s, 1_gpm);
        for (auto& pat : contributions) {
            sumDemands += pat;
        }

        if (sumDemands.GetMinValue() <= -1_gpm) {
            // we have a problem! What is the potential solution? 
            








        }

        return sumDemands;    
    };
    INLINE std::map<std::string, cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>> Szone::LeakModelResults(cweeSharedPtr<Project> pr, year_t surveyFrequency, pounds_per_square_inch_t oldPressure) const {
        mile_t MilesMains = 0;
        pounds_per_square_inch_t avgPressure = this->AverageNodePressure();

        for (auto& link : this->Within_Link) {
            MilesMains += link->Len;
        }

        AUTO results = pr->network->Leakage.LeakModelResults(MilesMains, surveyFrequency, pr->network->Zone, oldPressure, avgPressure);

        million_gallon_per_day_t totalWaterDemand = 0;
        scalar_t numValves = 0;
        scalar_t numERTs = 0;
        scalar_t numJunctions = 0;
        scalar_t numCustomers = 0;
        inch_t avgPipeDiameter = 0; int pipeCount = 0;

        {
            for (auto& link : this->Within_Link) {
                units::math::rolling_avg<inch_t>(avgPipeDiameter, link->Diam, pipeCount);
            }
            for (auto& link : this->Boundary_Link) {
                if (link.second == epanet::direction_t::FLOW_IN_DMA) {
                    if (link.first->Type_p == asset_t::VALVE) {
                        AUTO valve = link.first.CastReference< epanet::Svalve >();
                        if (valve) {
                            if (valve->ProducesElectricity) {
                                numERTs++;
                            }
                            numValves++;
                        }
                    }
                }
            }
            for (auto& node : this->Node) {
                if (node && node->Type_p == asset_t::JUNCTION) {
                    numJunctions += 1;
                    if (node->HasWaterDemand()) {
                        totalWaterDemand += node->GetValue<_DEMAND_>()->GetAvgValue();
                        numCustomers += 1;
                    }

                }
            }
        }

        results["Average Water Demand"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::million_gallon_per_day(totalWaterDemand), cweeUnitValues::cweeUnitPattern());
        results["Number of Valves"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::scalar(numValves), cweeUnitValues::cweeUnitPattern()); 
        results["Number of ERTs"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::scalar(numERTs), cweeUnitValues::cweeUnitPattern()); 
        results["Number of Junctions"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::scalar(numJunctions), cweeUnitValues::cweeUnitPattern()); 
        results["Number of Customer Nodes"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::scalar(numCustomers), cweeUnitValues::cweeUnitPattern());
        results["Average Pipe Diameter"] = cweeUnion<cweeUnitValues::unit_value, cweeUnitValues::cweeUnitPattern>(cweeUnitValues::inch(avgPipeDiameter), cweeUnitValues::cweeUnitPattern()); 

        return results;
    };
    INLINE year_t Szone::SurveyFrequency(cweeSharedPtr<Project> pr, pounds_per_square_inch_t oldPressure) const {
        mile_t MilesMains = 0;
        pounds_per_square_inch_t avgPressure = 0;

        for (auto& link : this->Within_Link) {
            MilesMains += link->Len;
        }
        for (auto& node : this->Node) {
            avgPressure += node->GetAvgPressure();
        } 
        if (this->Node.Num() > 0) avgPressure /= this->Node.Num();

        return pr->network->Leakage.BestSurveyFrequency(MilesMains, pr->network->Zone, oldPressure, avgPressure);
    };

#pragma region sparse matrix
    class smatrix_t {
    public:
        static int  createsparse(EN_Project pr)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code
            ** Purpose: creates sparse representation of coeff. matrix
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Smatrix* sm = &pr->hydraul.smatrix;

            int errcode = 0;

            // Allocate sparse matrix data structures
            errcode = allocsmatrix(sm, net->Nnodes, net->Nlinks);
            if (errcode) return errcode;

            // Build a local version of node-link adjacency lists
            // with parallel links removed
            errcode = localadjlists(net, sm);
            if (errcode) return errcode;

            // Re-order nodes to minimize number of non-zero coeffs.
            // in factorized solution matrix
            ERRCODE(reordernodes(pr));

            // Factorize solution matrix by updating adjacency lists
            // with non-zero connections due to fill-ins
            sm->Ncoeffs = net->Nlinks;
            ERRCODE(factorize(pr));

            // Allocate memory for sparse storage of positions of non-zero
            // coeffs. and store these positions in vector NZSUB
            ERRCODE(storesparse(pr, net->Njuncs));

            // Free memory used for local adjacency lists and sort
            // row indexes in NZSUB to optimize linsolve()
            freeadjlists(net);
            ERRCODE(sortsparse(sm, net->Njuncs));

            // Allocate memory used by linear eqn. solver
            ERRCODE(alloclinsolve(sm, net->Nnodes));

            // Re-build adjacency lists for future use
            ERRCODE(buildadjlists(net));
            return errcode;
        };
        static int	buildadjlists(EN_Network net)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code
            ** Purpose: builds linked list of links adjacent to each node
            **--------------------------------------------------------------
            */
        {
            int       i, j, k;
            int       errcode = 0;
            Padjlist  alink;

            // Create an array of adjacency lists
            freeadjlists(net);
            net->Adjlist.AssureSize(net->Nnodes + 1);
            // for (i = 0; i <= net->Nnodes; i++) net->Adjlist[i]->next = nullptr; // these will always initialize to nullptr

            // For each link, update adjacency lists of its end nodes
            for (k = 1; k <= net->Nlinks; k++)
            {
                i = net->Link[k]->N1;
                j = net->Link[k]->N2;

                // Include link in start node i's list
                alink = make_cwee_shared<Sadjlist>();
                alink->node = j;
                alink->link = k;
                alink->next = net->Adjlist[i];
                net->Adjlist[i] = alink;

                // Include link in end node j's list
                alink = make_cwee_shared<Sadjlist>();
                alink->node = i;
                alink->link = k;
                alink->next = net->Adjlist[j];
                net->Adjlist[j] = alink;
            }
            if (errcode) freeadjlists(net);
            return errcode;
        };
        static int  allocsmatrix(Smatrix* sm, int Nnodes, int Nlinks)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code
            ** Purpose: allocates memory for representing a sparse matrix
            **--------------------------------------------------------------
            */
        {
            // Memory for linear eqn. solver allocated in alloclinsolve().
            sm->Aij.Clear();
            sm->Aii.Clear();
            sm->F.Clear();
            sm->temp.Clear();
            sm->link.Clear();
            sm->first.Clear();

            // Memory for representing sparse matrix data structure
            sm->Order.ClearedResize(Nnodes + 1);
            sm->Row.ClearedResize(Nnodes + 1);
            sm->Ndx.ClearedResize(Nlinks + 1);

            return 0;
        };
        static int  alloclinsolve(Smatrix* sm, int n)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code
            ** Purpose: allocates memory used by linear eqn. solver.
            **--------------------------------------------------------------
            */
        {
            n = n + 1;    // All arrays are 1-based

            sm->Aij.ClearedResize(sm->Ncoeffs + 1);
            sm->Aii.ClearedResize(n);
            sm->F.ClearedResize(n);
            sm->temp.ClearedResize(n);
            sm->link.ClearedResize(n);
            sm->first.ClearedResize(n);

            return 0;
        };
        static void freesparse(EN_Project pr)
            /*
            **----------------------------------------------------------------
            ** Input:   None
            ** Output:  None
            ** Purpose: Frees memory used for sparse matrix storage
            **----------------------------------------------------------------
            */
        {
            Smatrix* sm = &pr->hydraul.smatrix;

            sm->Order.Clear();
            sm->Row.Clear();
            sm->Ndx.Clear();
            sm->XLNZ.Clear();
            sm->NZSUB.Clear();
            sm->LNZ.Clear();

            sm->Aij.Clear();
            sm->Aii.Clear();
            sm->F.Clear();
            sm->temp.Clear();
            sm->link.Clear();
            sm->first.Clear();
        };
        static int  localadjlists(EN_Network net, Smatrix* sm)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code
            ** Purpose: builds linked list of non-parallel links adjacent to each node
            **--------------------------------------------------------------
            */
        {
            int    i, j, k;
            int    pmark = 0;     // parallel link marker
            int    errcode = 0;
            Padjlist  alink;

            // Create an array of adjacency lists
            freeadjlists(net);
            net->Adjlist.AssureSize(net->Nnodes + 1);

            // For each link, update adjacency lists of its end nodes
            for (k = 1; k <= net->Nlinks; k++)
            {
                i = net->Link[k]->N1;
                j = net->Link[k]->N2;
                pmark = paralink(net, sm, i, j, k);  // Parallel link check

                // Include link in start node i's list
                alink = make_cwee_shared<Sadjlist>();
                if (!pmark) alink->node = j;
                else        alink->node = 0;         // Parallel link marker
                alink->link = k;
                alink->next = net->Adjlist[i];
                net->Adjlist[i] = alink;

                // Include link in end node j's list
                alink = make_cwee_shared<Sadjlist>();
                if (!pmark) alink->node = i;
                else        alink->node = 0;         // Parallel link marker
                alink->link = k;
                alink->next = net->Adjlist[j];
                net->Adjlist[j] = alink;
            }

            // Remove parallel links from adjacency lists
            xparalinks(net);
            return errcode;
        };
        static void	freeadjlists(EN_Network net)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  none
            ** Purpose: frees memory used for nodal adjacency lists
            **--------------------------------------------------------------
            */
        {
            int   i;
            Padjlist alink;

            if (net->Adjlist.size() == 0) return;
            for (i = 0; i <= net->Nnodes; i++)
            {
                for (alink = net->Adjlist[i]; alink != nullptr; alink = net->Adjlist[i])
                {
                    net->Adjlist[i] = alink->next;
                }
            }
            net->Adjlist = cweeThreadedList<Padjlist>();
        };
        static int  paralink(EN_Network net, Smatrix* sm, int i, int j, int k)
            /*
            **--------------------------------------------------------------
            ** Input:   i = index of start node of link
            **          j = index of end node of link
            **          k = link index
            ** Output:  returns 1 if link k parallels another link, else 0
            ** Purpose: checks for parallel links between nodes i and j
            **
            **--------------------------------------------------------------
            */
        {
            Padjlist alink;
            for (alink = net->Adjlist[i]; alink != nullptr; alink = alink->next)
            {
                // Link || to k (same end nodes)
                if (alink->node == j)
                {
                    // Assign Ndx entry to this link
                    sm->Ndx[k] = alink->link;
                    return(1);
                }
            }
            // Ndx entry if link not parallel
            sm->Ndx[k] = k;
            return(0);
        };
        static void xparalinks(EN_Network net)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  none
            ** Purpose: removes parallel links from nodal adjacency lists
            **--------------------------------------------------------------
            */
        {
            int    i;
            Padjlist    alink,       // Current item in adjacency list
                blink;       // Previous item in adjacency list

            // Scan adjacency list of each node
            for (i = 1; i <= net->Nnodes; i++)
            {
                alink = net->Adjlist[i];               // First item in list
                blink = nullptr;
                while (alink != nullptr)
                {
                    if (alink->node == 0)              // Parallel link marker found
                    {
                        if (blink == nullptr)             // This holds at start of list
                        {
                            net->Adjlist[i] = alink->next;              // Remove item from list
                            alink = net->Adjlist[i];
                        }
                        else                           // This holds for interior of list
                        {
                            blink->next = alink->next;       // Remove item from list
                            alink = blink->next;
                        }
                    }
                    else
                    {
                        blink = alink;                // Move to next item in list
                        alink = alink->next;
                    }
                }
            }
        };
        static int  reordernodes(EN_Project pr)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns 1 if successful, 0 if not
            ** Purpose: re-orders nodes to minimize # of non-zeros that
            **          will appear in factorized solution matrix
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Smatrix* sm = &pr->hydraul.smatrix;

            int k, knode, m, njuncs, nlinks;
            int delta = -1;
            int nofsub = 0;
            int maxint = INT_MAX;   //defined in limits.h
            int errcode;
            Padjlist alink;

            // Local versions of node adjacency lists
            cweeList<int> adjncy;
            cweeList<int> xadj;

            // Work arrays
            cweeList<int> dhead;
            cweeList<int> qsize;
            cweeList<int> llist;
            cweeList<int> marker;

            // Default ordering
            for (k = 1; k <= net->Nnodes; k++)
            {
                sm->Row[k] = k;
                sm->Order[k] = k;
            }
            njuncs = net->Njuncs;
            nlinks = net->Nlinks;

            // Allocate memory
            adjncy.AssureSize(2 * nlinks + 1);
            xadj.AssureSize(njuncs + 2);
            dhead.AssureSize(njuncs + 2);
            qsize.AssureSize(njuncs + 2);
            llist.AssureSize(njuncs + 2);
            marker.AssureSize(njuncs + 2);

            if (1)
            {
                // Create local versions of node adjacency lists
                xadj[1] = 1;
                m = 1;
                for (k = 1; k <= njuncs; k++)
                {
                    for (alink = net->Adjlist[k]; alink != nullptr; alink = alink->next)
                    {
                        knode = alink->node;
                        if (knode > 0 && knode <= njuncs)
                        {
                            adjncy[m] = knode;
                            m++;
                        }
                    }
                    xadj[k + 1] = m;
                }

                // Generate a multiple minimum degree node re-ordering
                genmmd_t::genmmd(&njuncs, xadj.Ptr(), adjncy.Ptr(), sm->Row.Ptr(), sm->Order.Ptr(), &delta, dhead.Ptr(), qsize.Ptr(), llist.Ptr(), marker.Ptr(), &maxint, &nofsub);
                errcode = 0;
            }
            else errcode = 101;  //insufficient memory

            return errcode;
        };
        static int  factorize(EN_Project pr)
            /*
            **--------------------------------------------------------------
            ** Input:   none
            ** Output:  returns error code
            ** Purpose: symbolically factorizes the solution matrix in
            **          terms of its adjacency lists
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Smatrix* sm = &pr->hydraul.smatrix;

            int k, knode;
            int errcode = 0;
            Padjlist alink;

            // Find degree of each junction node
            cweeList<int> degree;
            degree.AssureSize(net->Nnodes + 1);

            // NOTE: For purposes of node re-ordering, Tanks (nodes with indexes above Njuncs) have zero degree of adjacency.
            for (k = 1; k <= net->Njuncs; k++)
            {
                for (alink = net->Adjlist[k]; alink != nullptr; alink = alink->next)
                {
                    if (alink->node > 0) degree[k]++;
                }
            }

            // Augment each junction's adjacency list to account for
            // new connections created when solution matrix is solved.
            // NOTE: Only junctions (indexes <= Njuncs) appear in solution matrix.
            for (k = 1; k <= net->Njuncs; k++)          // Examine each junction
            {
                knode = sm->Order[k];                   // Re-ordered index
                if (!growlist(pr, knode, degree))               // Augment adjacency list
                {
                    errcode = 101;
                    break;
                }
                degree[knode] = 0;                  // In-activate node
            }

            return errcode;
        };
        static int  growlist(EN_Project pr, int knode, cweeList<int>& degree)
            /*
            **--------------------------------------------------------------
            ** Input:   knode = node index
            ** Output:  returns 1 if successful, 0 if not
            ** Purpose: creates new entries in knode's adjacency list for
            **          all unlinked pairs of active nodes that are
            **          adjacent to knode
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Smatrix* sm = &pr->hydraul.smatrix;

            int node;
            Padjlist alink;

            // Iterate through all nodes connected to knode
            for (alink = net->Adjlist[knode]; alink != nullptr; alink = alink->next)
            {
                node = alink->node;                   // End node of connecting link
                if (node > 0 && degree[node] > 0) // End node is active
                {
                    degree[node]--;           // Reduce degree of adjacency
                    if (!newlink(pr, alink, degree))      // Add to adjacency list
                    {
                        return 0;
                    }
                }
            }
            return 1;
        };
        static int  newlink(EN_Project pr, Padjlist alink, cweeList<int>& degree)
            /*
            **--------------------------------------------------------------
            ** Input:   alink = element of node's adjacency list
            ** Output:  returns 1 if successful, 0 if not
            ** Purpose: links end of current adjacent link to end nodes of
            **          all links that follow it on adjacency list
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Smatrix* sm = &pr->hydraul.smatrix;

            int inode, jnode;
            Padjlist blink;

            // Scan all entries in adjacency list that follow anode.
            inode = alink->node;             // End node of connection to anode
            for (blink = alink->next; blink != nullptr; blink = blink->next)
            {
                jnode = blink->node;          // End node of next connection

                // If jnode still active, and inode not connected to jnode, then add a new connection between inode and jnode.
                if (jnode > 0 && degree[jnode] > 0)  // jnode still active
                {
                    if (!linked(net, inode, jnode))      // inode not linked to jnode
                    {
                        // Since new connection represents a non-zero coeff. in the solution matrix, update the coeff. count.
                        sm->Ncoeffs++;

                        // Update adjacency lists for inode & jnode to reflect the new connection.
                        if (!addlink(net, inode, jnode, sm->Ncoeffs)) return 0;
                        if (!addlink(net, jnode, inode, sm->Ncoeffs)) return 0;
                        degree[inode]++;
                        degree[jnode]++;
                    }
                }
            }
            return 1;
        };
        static int  linked(EN_Network net, int i, int j)
            /*
            **--------------------------------------------------------------
            ** Input:   i = node index
            **          j = node index
            ** Output:  returns 1 if nodes i and j are linked, 0 if not
            ** Purpose: checks if nodes i and j are already linked.
            **--------------------------------------------------------------
            */
        {
            Padjlist alink;
            for (alink = net->Adjlist[i]; alink != nullptr; alink = alink->next)
            {
                if (alink->node == j) return 1;
            }
            return 0;
        };
        static int  addlink(EN_Network net, int i, int j, int n)
            /*
            **--------------------------------------------------------------
            ** Input:   i = node index
            **          j = node index
            **          n = link index
            ** Output:  returns 1 if successful, 0 if not
            ** Purpose: augments node i's adjacency list with node j
            **--------------------------------------------------------------
            */
        {
            Padjlist alink;
            alink = make_cwee_shared<Sadjlist>();
            if (alink == nullptr) return 0;
            alink->node = j;
            alink->link = n;
            alink->next = net->Adjlist[i];
            net->Adjlist[i] = alink;
            return 1;
        };
        static int  storesparse(EN_Project pr, int n)
            /*
            **--------------------------------------------------------------
            ** Input:   n = number of rows in solution matrix
            ** Output:  returns error code
            ** Purpose: stores row indexes of non-zeros of each column of
            **          lower triangular portion of factorized matrix
            **--------------------------------------------------------------
            */
        {
            EN_Network net = pr->network;
            Smatrix* sm = &pr->hydraul.smatrix;

            int i, ii, j, k, l, m;
            int errcode = 0;
            Padjlist alink;

            // Allocate sparse matrix storage
            sm->XLNZ.ClearedResize(n + 2);
            sm->NZSUB.ClearedResize(sm->Ncoeffs + 2);
            sm->LNZ.ClearedResize(sm->Ncoeffs + 2);

            if (errcode) return errcode;

            // Generate row index pointers for each column of matrix
            k = 0;
            sm->XLNZ[1] = 1;
            for (i = 1; i <= n; i++)            // column
            {
                m = 0;
                ii = sm->Order[i];
                for (alink = net->Adjlist[ii]; alink != nullptr; alink = alink->next)
                {
                    if (alink->node == 0) continue;
                    j = sm->Row[alink->node];    // row
                    l = alink->link;
                    if (j > i && j <= n)
                    {
                        m++;
                        k++;
                        sm->NZSUB[k] = j;
                        sm->LNZ[k] = l;
                    }
                }
                sm->XLNZ[i + 1] = sm->XLNZ[i] + m;
            }
            return errcode;
        };
        static int  sortsparse(Smatrix* sm, int n)
            /*
            **--------------------------------------------------------------
            ** Input:   n = number of rows in solution matrix
            ** Output:  returns eror code
            ** Purpose: puts row indexes in ascending order in NZSUB
            **--------------------------------------------------------------
            */
        {
            int  i, k;
            cweeList<int> xlnzt, nzsubt, lnzt, nzt;
            int  errcode = 0;

            auto& LNZ = sm->LNZ;
            auto& XLNZ = sm->XLNZ;
            auto& NZSUB = sm->NZSUB;

            xlnzt.AssureSize(n + 2);
            nzsubt.AssureSize(sm->Ncoeffs + 2);
            lnzt.AssureSize(sm->Ncoeffs + 2);
            nzt.AssureSize(n + 2);

            if (!errcode)
            {
                // Count # non-zeros in each row
                for (i = 1; i <= n; i++) nzt[i] = 0;
                for (i = 1; i <= n; i++)
                {
                    for (k = XLNZ[i]; k < XLNZ[i + 1]; k++) nzt[NZSUB[k]]++;
                }
                xlnzt[1] = 1;
                for (i = 1; i <= n; i++) xlnzt[i + 1] = xlnzt[i] + nzt[i];

                // Transpose matrix twice to order column indexes
                transpose(n, XLNZ, NZSUB, LNZ, xlnzt, nzsubt, lnzt, nzt);
                transpose(n, xlnzt, nzsubt, lnzt, XLNZ, NZSUB, LNZ, nzt);
            }

            // Reclaim memory
            return errcode;
        };
        static void transpose(int n, cweeList<int>& il, cweeList<int>& jl, cweeList<int>& xl, cweeList<int>& ilt, cweeList<int>& jlt, cweeList<int>& xlt, cweeList<int>& nzt)
            /*
            **---------------------------------------------------------------------
            ** Input:   n = matrix order
            **          il,jl,xl = sparse storage scheme for original matrix
            **          nzt = work array
            ** Output:  ilt,jlt,xlt = sparse storage scheme for transposed matrix
            ** Purpose: Determines sparse storage scheme for transpose of a matrix
            **---------------------------------------------------------------------
            */
        {
            int  i, j, k, kk;

            for (i = 1; i <= n; i++) nzt[i] = 0;
            for (i = 1; i <= n; i++)
            {
                for (k = il[i]; k < il[i + 1]; k++)
                {
                    j = jl[k];
                    kk = ilt[j] + nzt[j];
                    jlt[kk] = i;
                    xlt[kk] = xl[k];
                    nzt[j]++;
                }
            }
        };
        static int  linsolve(Smatrix* sm, int n)
            /*
            **--------------------------------------------------------------
            ** Input:   sm   = sparse matrix struct
                        n    = number of equations
            ** Output:  sm->F = solution values
            **          returns 0 if solution found, or index of
            **          equation causing system to be ill-conditioned
            ** Purpose: solves sparse symmetric system of linear
            **          equations using Cholesky factorization
            **
            ** NOTE:   This procedure assumes that the solution matrix has
            **         been symbolically factorized with the positions of
            **         the lower triangular, off-diagonal, non-zero coeffs.
            **         stored in the following integer arrays:
            **            XLNZ  (start position of each column in NZSUB)
            **            NZSUB (row index of each non-zero in each column)
            **            LNZ   (position of each NZSUB entry in Aij array)
            **
            **  This procedure has been adapted from subroutines GSFCT and
            **  GSSLV in the book "Computer Solution of Large Sparse
            **  Positive Definite Systems" by A. George and J. W-H Liu
            **  (Prentice-Hall, 1981).
            **--------------------------------------------------------------
            */
        {
            using namespace cwee_units;

            auto& Aii = sm->Aii;
            auto& Aij = sm->Aij;
            auto& B_cfs = sm->F;
            auto& temp = sm->temp;
            auto& B_ft = sm->B_ft;
            B_ft.ClearedResize(B_cfs.Num());

            auto& LNZ = sm->LNZ; // indexes
            auto& XLNZ = sm->XLNZ; // indexes
            auto& NZSUB = sm->NZSUB; // indexes
            auto& link = sm->link; // indexes
            auto& first = sm->first; // indexes

            int    i, istop, istrt, isub, j, k, kfirst, newk;

            for (auto& x : temp) x = 0;
            for (auto& x : link) x = 0;
            for (auto& x : first) x = 0;

            // Begin numerical factorization of matrix A into L ... Compute column L(*,j) for j = 1,...n
            for (j = 1; j <= n; j++) {
                // For each column L(*,k) that affects L(*,j):
                auto diagj = Aij[0] * Aij[0];
                diagj = 0;

                newk = link[j];
                k = newk;
                while (k != 0) {
                    // Outer product modification of L(*,j) by L(*,k) starting at first[k] of L(*,k)
                    newk = link[k];
                    kfirst = first[k];
                    auto& ljk = Aij[LNZ[kfirst]];

                    diagj += ljk * ljk;

                    istrt = kfirst + 1;
                    istop = XLNZ[k + 1] - 1;
                    if (istop >= istrt)
                    {
                        // Before modification, update vectors 'first' and 'link' for future modification steps
                        first[k] = istrt;
                        isub = NZSUB[istrt];
                        link[k] = link[isub];
                        link[isub] = k;

                        // The actual mod is saved in vector 'temp'
                        for (i = istrt; i <= istop; i++)
                        {
                            isub = NZSUB[i];
                            temp[isub] += Aij[LNZ[i]] * ljk;
                        }
                    }
                    k = newk;
                }

                // Apply the modifications accumulated in 'temp' to column L(*,j)
                if (diagj >=
                    Aii[j]
                    * (cfs_p_ft_t)1.0
                    ) { /* Check for ill - conditioning */
                    return j;
                }
                Aii[j] = ::epanet::sqrt((Aii[j]
                    * (cfs_p_ft_t)1.0
                    ) - diagj);
                istrt = XLNZ[j];
                istop = XLNZ[j + 1] - 1;
                if (istop >= istrt)
                {
                    first[j] = istrt;
                    isub = NZSUB[istrt];
                    link[j] = link[isub];
                    link[isub] = j;
                    for (i = istrt; i <= istop; i++)
                    {
                        isub = NZSUB[i];
                        Aij[LNZ[i]] = (
                            (Aij[LNZ[i]]
                                * (cfs_p_ft_t)1.0
                                )
                            - temp[isub]
                            ) / Aii[j];
                        temp[isub] = 0.0;
                    }
                }
            }      // next j

            // Foward substitution
            for (j = 1; j <= n; j++)
            {
                auto bj_ft = B_cfs[j] / Aii[j];
                B_ft[j] = bj_ft;

                istrt = XLNZ[j];
                istop = XLNZ[j + 1] - 1;
                if (istop >= istrt)
                {
                    for (i = istrt; i <= istop; i++)
                    {
                        isub = NZSUB[i];
                        B_cfs[isub] -= Aij[LNZ[i]] * bj_ft;
                    }
                }
            }

            // Backward substitution
            for (j = n; j >= 1; j--)
            {
                auto bj_ft = B_ft[j];

                istrt = XLNZ[j];
                istop = XLNZ[j + 1] - 1;
                if (istop >= istrt)
                {
                    for (i = istrt; i <= istop; i++)
                    {
                        isub = NZSUB[i];
                        bj_ft -=
                            (Aij[LNZ[i]] * B_ft[isub]) * ((ft_per_cfs_t)1.0); // RG ... accumulate, with units mult.
                    }
                }
                B_ft[j] = (bj_ft / Aii[j]) * (cfs_p_ft_t)1.0; // RG ... undo units mult.
            }

            return 0;
        };
    };
#pragma endregion

#if 0
    INLINE void    Project::parseNetwork() {
        AUTO pr = this;

        EN_Network net = pr->network;
        auto* time = &pr->times;

        int errcode = 0;

        // if there are any existing Zones, delete them. 
        for (int i = 0; i < net->Node.Num(); i++) {
            Pnode node = net->Node[i];
            if (!node) continue;
            node->Zone = nullptr;
        }
        net->Zone = cweeList<Pzone>();

        // fill the sparse matrix of N-links to M-nodes
        ERRCODE(smatrix_t::createsparse(pr));

        for (auto& pump : net->Pump) { // Determine approximate pump diameters (downstream > upstream > assume 1_ft)
            if (pump) {
                foot_t diam = 0;
                if (TryFindValidDiameter(pump.CastReference<Slink>(), pump->N2, diam)) {}
                else if (TryFindValidDiameter(pump.CastReference<Slink>(), pump->N1, diam)) {}
                else { diam = 1_ft; }
                pump->Diam = diam;
            }
        }

        { // Evaluate all links and nodes and generate a list of zones
            AUTO startTime = (u64)pr->times.GetSimStartTime();
            auto& links = net->Link; auto& controls = net->Control; auto& rules = net->Rule;
            cweeList<Pzone> zones; // Every node becomes its own private zone.
            cweeList<Pnode> nodes = net->Node;
            Padjlist adj; cweeStr name; illDefined_t illDefin; zoneType_t zT;

            // discover open pressure zones
            for (int i = 0; i < nodes.Num(); i++) {
                Pnode node = nodes[i];
                if (!node) continue;
                if (node->Type_p == asset_t::RESERVOIR) {
                    if (AUTO resP = node.CastReference<Stank>()) {
                        if (resP->Diameter != 0_ft) {
                            name = cweeStr::printf("Open Zone (Tank %s)", node->Name_p.c_str());
                            illDefin = illDefined_t::TankWithoutInflow;
                            zT = zoneType_t::Open;

                            Pzone zone = make_cwee_shared<Szone>(name);
                            parseZone_Basic(zone, nodes, i, startTime);
                            zone->IllDefined = illDefin;
                            zone->Type = zT;
                            zones.Append(zone);
                        }
                        else {
                            name = cweeStr::printf("Draw Zone (Reservoir %s)", node->Name_p.c_str());
                            illDefin = illDefined_t::Okay;
                            zT = zoneType_t::Draw;

                            Pzone zone = make_cwee_shared<Szone>(name);
                            parseZone_Basic(zone, nodes, i, startTime);
                            zone->IllDefined = illDefin;
                            zone->Type = zT;
                            zones.Append(zone);
                        }
                    }
                }
                else {
                    name = cweeStr::printf("Reduced Zone (Junction %s)", node->Name_p.c_str());
                    illDefin = illDefined_t::IsolatedJunction;
                    zT = zoneType_t::Reduced;

                    Pzone zone = make_cwee_shared<Szone>(name);
                    parseZone_Basic(zone, nodes, i, startTime);
                    zone->IllDefined = illDefin;
                    zone->Type = zT;
                    zones.Append(zone);
                }
            }

            // combine pressure zones where appropriate. 
            std::unordered_map<Slink*, bool> sharedLinksContolledMap;
            for (auto& link : links) if (link) sharedLinksContolledMap.emplace(link.Get(), IsLinkControlled(link));
            cweeList<Plink> sharedLinks;
            std::unordered_map<Slink*, bool> sharedLinksMap;
            for (auto& link : links) if (link) sharedLinksMap.emplace(link.Get(), false);
            std::unordered_map<Slink*, StatusType> sharedLinksStatusMap;
            for (auto& link : links) if (link) sharedLinksStatusMap.emplace(link.Get(), (StatusType)(double)link->Status(startTime));

            // combine zones recursively until you cannot anymore...
            while (combineZones(zones, sharedLinks, sharedLinksMap, sharedLinksStatusMap, sharedLinksContolledMap)) {}

            // try to craft a better name for the zone based on the boundary links (if any)
            for (auto& zone : zones) {
                if (zone->Type == zoneType_t::Draw || zone->Type == zoneType_t::Open) continue; // Open and draw zone names are always determined by their reservoirs

                bool done = false;
                for (auto& L : zone->Boundary_Link) {
                    if (!done && L.second == direction_t::FLOW_IN_DMA) {
                        if (L.first->Type_p == asset_t::PUMP) {
                            // Closed Zone
                            zone->Name_p = cweeStr::printf("Closed Zone (Pump %s)", L.first->Name_p.c_str());
                            zone->IllDefined = illDefined_t::InactivePump;
                            zone->Type = zoneType_t::Closed;
                            done = true;
                            break;
                        }
                    }
                }
                for (auto& L : zone->Boundary_Link) {
                    if (!done && L.second == direction_t::FLOW_IN_DMA) {
                        if (L.first->Type_p == asset_t::VALVE) {
                            // Reduced Zone
                            zone->Name_p = cweeStr::printf("Reduced Zone (Valve %s)", L.first->Name_p.c_str());
                            zone->IllDefined = illDefined_t::InactiveValve;
                            zone->Type = zoneType_t::Reduced;
                            done = true;
                            break;
                        }
                    }
                }
                for (auto& L : zone->Boundary_Link) {
                    if (!done && L.second == direction_t::FLOW_IN_DMA) {
                        if (L.first->Type_p == asset_t::PIPE) {
                            // Reduced Zone
                            zone->Name_p = cweeStr::printf("Reduced Zone (Pipe %s)", L.first->Name_p.c_str());
                            zone->IllDefined = illDefined_t::IsolatedJunction;
                            zone->Type = zoneType_t::Reduced;
                            done = true;
                            break;
                        }
                    }
                }
            }

            // Evaluate and (for the last time) rename the zone based on the observed characteristics
            for (auto& zone : zones) {
                if (isZoneWaterFlowPossible(zone, sharedLinksContolledMap, sharedLinksStatusMap)) {
                    zone->IllDefined = illDefined_t::Okay;
                }
                else {
                    if (!isZoneWaterFlowPossible(zone, sharedLinksContolledMap, sharedLinksStatusMap)) {
                        zone->Name_p = "Ill-Defined " + zone->Name_p;
                    }
                    switch (zone->IllDefined) {
                    case illDefined_t::Okay: zone->IllDefined = illDefined_t::IsolatedJunction; break;
                    case illDefined_t::IsolatedJunction:  break; // pump/valve faces wrong way or pipes are all closed
                    case illDefined_t::InactivePump:  break; // pump faces right way but is off
                    case illDefined_t::InactiveValve:  break; // valve faces right way but is off
                    case illDefined_t::TankWithoutInflow:  break; // tank has no possible fill
                    }
                }
            }

            // sort by zone type ...
            zones.Sort([](::epanet::Pzone const& a, ::epanet::Pzone const& b)->bool {
                return static_cast<int>(a->Type) < static_cast<int>(b->Type);
                });
            // ... then by ill-definition (if any) ...
            zones.Sort([](::epanet::Pzone const& a, ::epanet::Pzone const& b)->bool {
                return static_cast<int>(a->IllDefined) < static_cast<int>(b->IllDefined);
                });
            // ... then free the unused memory.
            zones.Condense();

            net->Zone = zones; // save the results
        }
    };
#endif

    INLINE cweeList< Pnode >    Project::findDeadEnds() {
        cweeList< Pnode > nodes;

        // find all dead-end nodes
        for (auto& zone : network->Zone) {
            if (zone) {
                nodes.Append(zone->FindDeadends(cweeSharedPtr<Project>(this, [](Project* p) { /* do not delete 'this' ptr */ })));
            }
        }

        return nodes;

#if 0

        // move node demands upstream
        for (auto& node : nodes) {
            if (node && node->HasWaterDemand()) {
                int node_index = hashtable_t::hashtable_find(network->NodeHashTable, (char*)(node->Name_p.c_str()));
                // get the connected node
                Pnode otherNode;
                {
                    auto links = getConnectedLinks(node_index);
                    if (links.size() > 0) {
                        if (links[0]->StartingNode == node) {
                            otherNode = links[0]->EndingNode;
                        }
                        else {
                            otherNode = links[0]->StartingNode;
                        }
                    }
                }

                if (otherNode) {
                    for (auto& demand : node->D) {
                        if (demand.Base != 0_cfs) {
                            otherNode->D.Append(demand);
                        }
                    }
                }
            }
        }


        // delete the dead-ends
        for (auto& node : nodes) {
            if (node) {
                deletenode(hashtable_t::hashtable_find(network->NodeHashTable, (char*)(node->Name_p.c_str())), EN_UNCONDITIONAL);
            }
        }

        // re-evaluate the zones
        this->parseNetwork();
#endif
    };



    INLINE cweeList<::epanet::Pzone>         Network::getCalibrationOrder() const {
        // order the zones according to which would be easiest to calibrate. 
        cweeList<::epanet::Pzone> out;

        cweeList<::epanet::Pzone> ZonesProcessing = this->Zone;
        ZonesProcessing.Sort([](::epanet::Pzone const& a, ::epanet::Pzone const& b)->bool { return a->Type >= b->Type; });

        int currentConnectivityDistance = 1;
        while (ZonesProcessing.Num() > 0) {
            bool success = false;

            // ill-defined zones are added first and must be resolved for obvious reasons
            if (!success) {
                for (auto& p : ZonesProcessing) {
                    if (p->IsIllDefined()) {
                        success = true;
                        out.Append(p);
                        ZonesProcessing.Remove(p);
                        break;
                    }
                }
            }

            // draw zones are simply dropped 
            if (!success) {
                for (auto& p : ZonesProcessing) {
                    if (p->Type == zoneType_t::Draw) {
                        success = true;
                        ZonesProcessing.Remove(p);
                        break;
                    }
                }
            }

            // add all reduced zones (end of the line)
            if (!success) {
                for (auto& p : ZonesProcessing) {
                    if (p->Type == zoneType_t::Reduced) {
                        success = true;
                        out.Append(p);
                        ZonesProcessing.Remove(p);
                        break;
                    }
                }
            }

            // add all closed zones (end of the line)
            if (!success) {
                for (auto& p : ZonesProcessing) {
                    if (p->Type == zoneType_t::Closed) {
                        success = true;
                        out.Append(p);
                        ZonesProcessing.Remove(p);
                        break;
                    }
                }
            }

            // Open zones are added last
            if (!success) {
                for (auto& p : ZonesProcessing) {
                    if (p->Type == zoneType_t::Open) {                        
                        success = true;
                        out.Append(p);
                        ZonesProcessing.Remove(p);
                        break;
                    }
                }
            }

            if (!success) {
                for (auto& p : ZonesProcessing) {
                    success = true;
                    out.Append(p);
                    ZonesProcessing.Remove(p);
                    break;
                }
            }
        }

        return out;
    };


    typedef struct {
        foot_t maxheaderror;
        SCALER maxflowerror;
        cubic_foot_per_second_t 
               maxflowchange;
        int    maxheadlink;
        int    maxflownode;
        int    maxflowlink;
    } Hydbalance;

#pragma endregion
#pragma region Enum Strings
    //------- Keyword Dictionary ------------------------------
    static constexpr ceSS   w_USE = ceSS("USE");
    static constexpr ceSS   w_SAVE = ceSS("SAVE");

    static constexpr ceSS   w_NONE = ceSS("NONE");
    static constexpr ceSS   w_ALL = ceSS("ALL");

    static constexpr ceSS   w_CHEM = ceSS("CHEM");
    static constexpr ceSS   w_AGE = ceSS("AGE");
    static constexpr ceSS   w_TRACE = ceSS("TRACE");
    static constexpr ceSS   w_ENERGYINTENSITY = ceSS("ENERGYINTENSITY");

    static constexpr ceSS   w_SYSTEM = ceSS("SYSTEM");
    static constexpr ceSS   w_JUNC = ceSS("Junc");
    static constexpr ceSS   w_RESERV = ceSS("Reser");
    static constexpr ceSS   w_TANK = ceSS("Tank");
    static constexpr ceSS   w_CV = ceSS("CV");
    static constexpr ceSS   w_PIPE = ceSS("Pipe");
    static constexpr ceSS   w_PUMP = ceSS("Pump");
    static constexpr ceSS   w_VALVE = ceSS("Valve");
    static constexpr ceSS   w_PRV = ceSS("PRV");
    static constexpr ceSS   w_PSV = ceSS("PSV");
    static constexpr ceSS   w_PBV = ceSS("PBV");
    static constexpr ceSS   w_FCV = ceSS("FCV");
    static constexpr ceSS   w_TCV = ceSS("TCV");
    static constexpr ceSS   w_GPV = ceSS("GPV");
    static constexpr ceSS   w_PAT = ceSS("PAT"); // pump as turbine
    
    static constexpr ceSS   w_OPEN = ceSS("OPEN");
    static constexpr ceSS   w_CLOSED = ceSS("CLOSED");
    static constexpr ceSS   w_ACTIVE = ceSS("ACTIVE");
    static constexpr ceSS   w_TIME = ceSS("TIME");
    static constexpr ceSS   w_ABOVE = ceSS("ABOVE");
    static constexpr ceSS   w_BELOW = ceSS("BELOW");
    static constexpr ceSS   w_PRECISION = ceSS("PREC");
    static constexpr ceSS   w_IS = ceSS("IS");
    static constexpr ceSS   w_NOT = ceSS("NOT");

    static constexpr ceSS   w_ADD = ceSS("ADD");
    static constexpr ceSS   w_MULTIPLY = ceSS("MULT");

    static constexpr ceSS   w_LIMITING = ceSS("LIMIT");
    static constexpr ceSS   w_ORDER = ceSS("ORDER");
    static constexpr ceSS   w_GLOBAL = ceSS("GLOB");
    static constexpr ceSS   w_BULK = ceSS("BULK");
    static constexpr ceSS   w_WALL = ceSS("WALL");

    static constexpr ceSS   w_PAGE = ceSS("PAGE");
    static constexpr ceSS   w_STATUS = ceSS("STATUS");
    static constexpr ceSS   w_SUMMARY = ceSS("SUMM");
    static constexpr ceSS   w_MESSAGES = ceSS("MESS");
    static constexpr ceSS   w_ENERGY = ceSS("ENER");
    static constexpr ceSS   w_NODE = ceSS("NODE");
    static constexpr ceSS   w_LINK = ceSS("LINK");
    static constexpr ceSS   w_FILE = ceSS("FILE");
    static constexpr ceSS   w_YES = ceSS("YES");
    static constexpr ceSS   w_NO = ceSS("NO");
    static constexpr ceSS   w_FULL = ceSS("FULL");

    static constexpr ceSS   w_HW = ceSS("H-W");
    static constexpr ceSS   w_DW = ceSS("D-W");
    static constexpr ceSS   w_CM = ceSS("C-M");

    static constexpr ceSS   w_CFS = ceSS("CFS");
    static constexpr ceSS   w_GPM = ceSS("GPM");
    static constexpr ceSS   w_MGD = ceSS("MGD");
    static constexpr ceSS   w_IMGD = ceSS("IMGD");
    static constexpr ceSS   w_AFD = ceSS("AFD");
    static constexpr ceSS   w_LPS = ceSS("LPS");
    static constexpr ceSS   w_LPM = ceSS("LPM");
    static constexpr ceSS   w_MLD = ceSS("MLD");
    static constexpr ceSS   w_CMH = ceSS("CMH");
    static constexpr ceSS   w_CMD = ceSS("CMD");
    static constexpr ceSS   w_SI = ceSS("SI");

    static constexpr ceSS   w_PSI = ceSS("PSI");
    static constexpr ceSS   w_KPA = ceSS("KPA");
    static constexpr ceSS   w_METERS = ceSS("METERS");

    static constexpr ceSS   w_ELEV = ceSS("ELEV");
    static constexpr ceSS   w_DEMAND = ceSS("DEMAND");
    static constexpr ceSS   w_HEAD = ceSS("HEAD");
    static constexpr ceSS   w_PRESSURE = ceSS("PRESSURE");
    static constexpr ceSS   w_QUALITY = ceSS("QUAL");

    static constexpr ceSS   w_DIAM = ceSS("DIAM");
    static constexpr ceSS   w_FLOW = ceSS("FLOW");
    static constexpr ceSS   w_ROUGHNESS = ceSS("ROUG");
    static constexpr ceSS   w_VELOCITY = ceSS("VELO");
    static constexpr ceSS   w_HEADLOSS = ceSS("HEADL");
    static constexpr ceSS   w_SETTING = ceSS("SETTING");
    static constexpr ceSS   w_POWER = ceSS("POWER");
    static constexpr ceSS   w_VOLUME = ceSS("VOLU");
    static constexpr ceSS   w_CLOCKTIME = ceSS("CLOCKTIME");
    static constexpr ceSS   w_FILLTIME = ceSS("FILLTIME");
    static constexpr ceSS   w_DRAINTIME = ceSS("DRAINTIME");
    static constexpr ceSS   w_GRADE = ceSS("GRADE");
    static constexpr ceSS   w_LEVEL = ceSS("LEVEL");

    static constexpr ceSS   w_DURATION = ceSS("DURA");
    static constexpr ceSS   w_HYDRAULIC = ceSS("HYDR");
    static constexpr ceSS   w_MINIMUM = ceSS("MINI");
    static constexpr ceSS   w_PATTERN = ceSS("PATT");
    static constexpr ceSS   w_REPORT = ceSS("REPO");
    static constexpr ceSS   w_START = ceSS("STAR");

    static constexpr ceSS   w_UNITS = ceSS("UNIT");
    static constexpr ceSS   w_MAP = ceSS("MAP");
    static constexpr ceSS   w_VERIFY = ceSS("VERI");
    static constexpr ceSS   w_VISCOSITY = ceSS("VISC");
    static constexpr ceSS   w_DIFFUSIVITY = ceSS("DIFF");
    static constexpr ceSS   w_SPECGRAV = ceSS("SPEC");
    static constexpr ceSS   w_TRIALS = ceSS("TRIAL");
    static constexpr ceSS   w_ACCURACY = ceSS("ACCU");
    static constexpr ceSS   w_SEGMENTS = ceSS("SEGM");
    static constexpr ceSS   w_TOLERANCE = ceSS("TOLER");
    static constexpr ceSS   w_EMITTER = ceSS("EMIT");

    static constexpr ceSS   w_PRICE = ceSS("PRICE");
    static constexpr ceSS   w_DMNDCHARGE = ceSS("DEMAN");

    static constexpr ceSS   w_HTOL = ceSS("HTOL");
    static constexpr ceSS   w_QTOL = ceSS("QTOL");
    static constexpr ceSS   w_RQTOL = ceSS("RQTOL");
    static constexpr ceSS   w_CHECKFREQ = ceSS("CHECKFREQ");
    static constexpr ceSS   w_MAXCHECK = ceSS("MAXCHECK");
    static constexpr ceSS   w_DAMPLIMIT = ceSS("DAMPLIMIT");

    static constexpr ceSS   w_FLOWCHANGE = ceSS("FLOWCHANGE");
    static constexpr ceSS   w_HEADERROR = ceSS("HEADERROR");

    static constexpr ceSS   w_MODEL = ceSS("MODEL");
    static constexpr ceSS   w_DDA = ceSS("DDA");
    static constexpr ceSS   w_PDA = ceSS("PDA");
    static constexpr ceSS   w_REQUIRED = ceSS("REQ");
    static constexpr ceSS   w_EXPONENT = ceSS("EXP");

    static constexpr ceSS   w_SECONDS = ceSS("SEC");
    static constexpr ceSS   w_MINUTES = ceSS("MIN");
    static constexpr ceSS   w_HOURS = ceSS("HOU");
    static constexpr ceSS   w_DAYS = ceSS("DAY");
    static constexpr ceSS   w_AM = ceSS("AM");
    static constexpr ceSS   w_PM = ceSS("PM");

    static constexpr ceSS   w_CONCEN = ceSS("CONCEN");
    static constexpr ceSS   w_MASS = ceSS("MASS");
    static constexpr ceSS   w_SETPOINT = ceSS("SETPOINT");
    static constexpr ceSS   w_FLOWPACED = ceSS("FLOWPACED");

    static constexpr ceSS   w_CURVE = ceSS("CURV");

    static constexpr ceSS   w_EFFIC = ceSS("EFFI");
    static constexpr ceSS   w_SPEED = ceSS("SPEE");

    static constexpr ceSS   w_MIXED = ceSS("MIXED");
    static constexpr ceSS   w_2COMP = ceSS("2COMP");
    static constexpr ceSS   w_FIFO = ceSS("FIFO");
    static constexpr ceSS   w_LIFO = ceSS("LIFO");

    static constexpr ceSS   w_STATISTIC = ceSS("STAT");
    static constexpr ceSS   w_AVG = ceSS("AVERAGE");
    static constexpr ceSS   w_MIN = ceSS("MINIMUM");
    static constexpr ceSS   w_MAX = ceSS("MAXIMUM");
    static constexpr ceSS   w_RANGE = ceSS("RANGE");

    static constexpr ceSS   w_UNBALANCED = ceSS("UNBA");
    static constexpr ceSS   w_STOP = ceSS("STOP");
    static constexpr ceSS   w_CONTINUE = ceSS("CONT");

    static constexpr ceSS   w_RULE = ceSS("RULE");
    static constexpr ceSS   w_IF = ceSS("IF");
    static constexpr ceSS   w_AND = ceSS("AND");
    static constexpr ceSS   w_OR = ceSS("OR");
    static constexpr ceSS   w_THEN = ceSS("THEN");
    static constexpr ceSS   w_ELSE = ceSS("ELSE");
    static constexpr ceSS   w_PRIORITY = ceSS("PRIO");

    // ------ Input File Section Names ------------------------
    static constexpr ceSS   s_TITLE = ceSS("[TITLE]");
    static constexpr ceSS   s_JUNCTIONS = ceSS("[JUNCTIONS]");
    static constexpr ceSS   s_RESERVOIRS = ceSS("[RESERVOIRS]");
    static constexpr ceSS   s_TANKS = ceSS("[TANKS]");
    static constexpr ceSS   s_PIPES = ceSS("[PIPES]");
    static constexpr ceSS   s_PUMPS = ceSS("[PUMPS]");
    static constexpr ceSS   s_VALVES = ceSS("[VALVES]");
    static constexpr ceSS   s_CONTROLS = ceSS("[CONTROLS]");
    static constexpr ceSS   s_RULES = ceSS("[RULES]");
    static constexpr ceSS   s_DEMANDS = ceSS("[DEMANDS]");
    static constexpr ceSS   s_SOURCES = ceSS("[SOURCES]");
    static constexpr ceSS   s_EMITTERS = ceSS("[EMITTERS]");
    static constexpr ceSS   s_PATTERNS = ceSS("[PATTERNS]");
    static constexpr ceSS   s_CURVES = ceSS("[CURVES]");
    static constexpr ceSS   s_QUALITY = ceSS("[QUALITY]");
    static constexpr ceSS   s_STATUS = ceSS("[STATUS]");
    static constexpr ceSS   s_ROUGHNESS = ceSS("[ROUGHNESS]");
    static constexpr ceSS   s_ENERGY = ceSS("[ENERGY]");
    static constexpr ceSS   s_REACTIONS = ceSS("[REACTIONS]");
    static constexpr ceSS   s_MIXING = ceSS("[MIXING]");
    static constexpr ceSS   s_REPORT = ceSS("[REPORT]");
    static constexpr ceSS   s_TIMES = ceSS("[TIMES]");
    static constexpr ceSS   s_OPTIONS = ceSS("[OPTIONS]");
    static constexpr ceSS   s_COORDS = ceSS("[COORDINATES]");
    static constexpr ceSS   s_VERTICES = ceSS("[VERTICES]");
    static constexpr ceSS   s_LABELS = ceSS("[LABELS]");
    static constexpr ceSS   s_BACKDROP = ceSS("[BACKDROP]");
    static constexpr ceSS   s_TAGS = ceSS("[TAGS]");
    static constexpr ceSS   s_END = ceSS("[END]");

    //------- Units -------------------------------------------
    static constexpr ceSS   u_CFS = ceSS("cfs");
    static constexpr ceSS   u_GPM = ceSS("gpm");
    static constexpr ceSS   u_AFD = ceSS("a-f/d");
    static constexpr ceSS   u_MGD = ceSS("mgd");
    static constexpr ceSS   u_IMGD = ceSS("Imgd");
    static constexpr ceSS   u_LPS = ceSS("L/s");
    static constexpr ceSS   u_LPM = ceSS("Lpm");
    static constexpr ceSS   u_CMH = ceSS("m3/h");
    static constexpr ceSS   u_CMD = ceSS("m3/d");
    static constexpr ceSS   u_MLD = ceSS("ML/d");
    static constexpr ceSS   u_MGperL = ceSS("mg/L");
    static constexpr ceSS   u_UGperL = ceSS("ug/L");
    static constexpr ceSS   u_HOURS = ceSS("hrs");
    static constexpr ceSS   u_MINUTES = ceSS("min");
    static constexpr ceSS   u_PERCENT = ceSS("% from");
    static constexpr ceSS   u_METERS = ceSS("m");
    static constexpr ceSS   u_MMETERS = ceSS("mm");
    static constexpr ceSS   u_MperSEC = ceSS("m/s");
    static constexpr ceSS   u_SQMperSEC = ceSS("sq m/sec");
    static constexpr ceSS   u_per1000M = ceSS("/1000m");
    static constexpr ceSS   u_KW = ceSS("kw");
    static constexpr ceSS   u_FEET = ceSS("ft");
    static constexpr ceSS   u_INCHES = ceSS("in");
    static constexpr ceSS   u_PSI = ceSS("psi");
    static constexpr ceSS   u_KPA = ceSS("kPa");
    static constexpr ceSS   u_FTperSEC = ceSS("fps");
    static constexpr ceSS   u_SQFTperSEC = ceSS("sq ft/sec");
    static constexpr ceSS   u_per1000FT = ceSS("/1000ft");
    static constexpr ceSS   u_HP = ceSS("hp");

    //------- Curve Types ------------------------------------- 
    static constexpr ceSS   c_HEADLOSS = ceSS("HEADLOSS");
    static constexpr ceSS   c_PUMP = ceSS("PUMP");
    static constexpr ceSS   c_EFFIC = ceSS("EFFIC");
    static constexpr ceSS   c_VOLUME = ceSS("VOLUME");

    //------- Text Phrases ------------------------------------
    static constexpr ceSS   t_ABOVE = ceSS("above");
    static constexpr ceSS   t_BELOW = ceSS("below");
    static constexpr ceSS   t_HW = ceSS("Hazen-Williams");
    static constexpr ceSS   t_DW = ceSS("Darcy-Weisbach");
    static constexpr ceSS   t_CM = ceSS("Chezy-Manning");
    static constexpr ceSS   t_CHEMICAL = ceSS("Chemical");
    static constexpr ceSS   t_XHEAD = ceSS("closed because cannot deliver head");
    static constexpr ceSS   t_TEMPCLOSED = ceSS("temporarily closed");
    static constexpr ceSS   t_CLOSED = ceSS("closed");
    static constexpr ceSS   t_OPEN = ceSS("open");
    static constexpr ceSS   t_ACTIVE = ceSS("active");
    static constexpr ceSS   t_XFLOW = ceSS("open but exceeds maximum flow");
    static constexpr ceSS   t_XFCV = ceSS("open but cannot deliver flow");
    static constexpr ceSS   t_XPRESSURE = ceSS("open but cannot deliver pressure");
    static constexpr ceSS   t_FILLING = ceSS("filling");
    static constexpr ceSS   t_EMPTYING = ceSS("emptying");
    static constexpr ceSS   t_OVERFLOWING = ceSS("overflowing");

    static constexpr ceSS   t_ELEV = ceSS("Elevation");
    static constexpr ceSS   t_DEMAND = ceSS("Demand");
    static constexpr ceSS   t_HEAD = ceSS("Head");
    static constexpr ceSS   t_PRESSURE = ceSS("Pressure");
    static constexpr ceSS   t_QUALITY = ceSS("Quality");
    static constexpr ceSS   t_LENGTH = ceSS("Length");
    static constexpr ceSS   t_DIAM = ceSS("Diameter");
    static constexpr ceSS   t_FLOW = ceSS("Flow");
    static constexpr ceSS   t_VELOCITY = ceSS("Velocity");
    static constexpr ceSS   t_HEADLOSS = ceSS("Headloss");
    static constexpr ceSS   t_LINKQUAL = ceSS("Quality");
    static constexpr ceSS   t_LINKSTATUS = ceSS("State");
    static constexpr ceSS   t_SETTING = ceSS("Setting");
    static constexpr ceSS   t_REACTRATE = ceSS("Reaction");
    static constexpr ceSS   t_FRICTION = ceSS("F-Factor");

    static constexpr ceSS   t_NODEID = ceSS("Node");
    static constexpr ceSS   t_LINKID = ceSS("Link");
    static constexpr ceSS   t_PERDAY = ceSS("/day");

    static constexpr ceSS   t_JUNCTION = ceSS("Junction");
    static constexpr ceSS   t_RESERVOIR = ceSS("Reservoir");
    static constexpr ceSS   t_TANK = ceSS("Tank");
    static constexpr ceSS   t_PIPE = ceSS("Pipe");
    static constexpr ceSS   t_PUMP = ceSS("Pump");
    static constexpr ceSS   t_VALVE = ceSS("Valve");
    static constexpr ceSS   t_CONTROL = ceSS("Control");
    static constexpr ceSS   t_RULE = ceSS("Rule");
    static constexpr ceSS   t_DEMANDFOR = ceSS("Demand for Node");
    static constexpr ceSS   t_SOURCE = ceSS("Source");
    static constexpr ceSS   t_EMITTER = ceSS("Emitter");
    static constexpr ceSS   t_PATTERN = ceSS("Pattern");
    static constexpr ceSS   t_CURVE = ceSS("Curve");
    static constexpr ceSS   t_STATUS = ceSS("Status");
    static constexpr ceSS   t_ROUGHNESS = ceSS("Roughness");
    static constexpr ceSS   t_ENERGY = ceSS("Energy");
    static constexpr ceSS   t_REACTION = ceSS("Reaction");
    static constexpr ceSS   t_MIXING = ceSS("Mixing");
    static constexpr ceSS   t_REPORT = ceSS("Report");
    static constexpr ceSS   t_TIME = ceSS("Times");
    static constexpr ceSS   t_OPTION = ceSS("Options");
    static constexpr ceSS   t_RULES_SECT = ceSS("[RULES] section");
    static constexpr ceSS   t_HALTED = ceSS(" EXECUTION HALTED.");
    static constexpr ceSS   t_FUNCCALL = ceSS("function call");
    static constexpr ceSS   t_CONTINUED = ceSS(" (continued)");
    static constexpr ceSS   t_perM3 = ceSS("  /m3");
    static constexpr ceSS   t_perMGAL = ceSS("/Mgal");
    static constexpr ceSS   t_DIFFER = ceSS("DIFFERENTIAL");
    static constexpr ceSS   t_FIXED = ceSS("Fixed Demands");
    static constexpr ceSS   t_POWER = ceSS("Power Function");
    static constexpr ceSS   t_ORIFICE = ceSS("Orifice Flow");


    //----- Summary Report Format Strings ---------------------
    static constexpr ceSS LOGO1 = ceSS("******************************************************************");
    static constexpr ceSS LOGO2 = ceSS("*                           E P A N E T                          *");
    static constexpr ceSS LOGO3 = ceSS("*                   Hydraulic and Water Quality                  *");
    static constexpr ceSS LOGO4 = ceSS("*                   Analysis for Pipe Networks                   *");
    static constexpr ceSS LOGO5 = ceSS("*                         Version %d.%d                          *");
    static constexpr ceSS LOGO6 = ceSS("******************************************************************");
    static constexpr ceSS FMT02 = ceSS("\n  o Retrieving network data");
    static constexpr ceSS FMT04 = ceSS("\n    Cannot use duplicate file names.");
    static constexpr ceSS FMT05 = ceSS("\n    Cannot open input file ");
    static constexpr ceSS FMT06 = ceSS("\n    Cannot open report file ");
    static constexpr ceSS FMT07 = ceSS("\n    Cannot open output file ");
    static constexpr ceSS FMT08 = ceSS("\n    Cannot open temporary output file");
    static constexpr ceSS FMT14 = ceSS("\n  o Computing hydraulics at hour ");
    static constexpr ceSS FMT15 = ceSS("\n  o Computing water quality at hour ");
    static constexpr ceSS FMT16 = ceSS("\n  o Transferring results to file");
    static constexpr ceSS FMT17 = ceSS("\n  o Writing output report to ");
    static constexpr ceSS FMT18 = ceSS("  Page 1                                    ");
    static constexpr ceSS FMT19 = ceSS("    Input Data File ................... %s");
    static constexpr ceSS FMT20 = ceSS("    Number of Junctions................ %-d");
    static constexpr ceSS FMT21a = ceSS("    Number of Reservoirs............... %-d");
    static constexpr ceSS FMT21b = ceSS("    Number of Tanks ................... %-d");
    static constexpr ceSS FMT22 = ceSS("    Number of Pipes ................... %-d");
    static constexpr ceSS FMT23 = ceSS("    Number of Pumps ................... %-d");
    static constexpr ceSS FMT24 = ceSS("    Number of Valves .................. %-d");
    static constexpr ceSS FMT25 = ceSS("    Headloss Formula .................. %s");
    static constexpr ceSS FMT25a = ceSS("    Nodal Demand Model ................ %s");
    static constexpr ceSS FMT26 = ceSS("    Hydraulic Timestep ................ %-.2f %s");
    static constexpr ceSS FMT27 = ceSS("    Hydraulic Accuracy ................ %-.6f");

    static constexpr ceSS FMT27a = ceSS("    Status Check Frequency ............ %-d");
    static constexpr ceSS FMT27b = ceSS("    Maximum Trials Checked ............ %-d");
    static constexpr ceSS FMT27c = ceSS("    Damping Limit Threshold ........... %-.6f");

    static constexpr ceSS FMT27d = ceSS("    Headloss Error Limit .............. %-.6f %s");
    static constexpr ceSS FMT27e = ceSS("    Flow Change Limit ................. %-.6f %s");

    static constexpr ceSS FMT28 = ceSS("    Maximum Trials .................... %-d");
    static constexpr ceSS FMT29 = ceSS("    Quality Analysis .................. None");
    static constexpr ceSS FMT30 = ceSS("    Quality Analysis .................. %s");
    static constexpr ceSS FMT31 = ceSS("    Quality Analysis .................. Trace From Node %s");
    static constexpr ceSS FMT32 = ceSS("    Quality Analysis .................. Age");
    static constexpr ceSS FMT33 = ceSS("    Water Quality Time Step ........... %-.2f min");
    static constexpr ceSS FMT34 = ceSS("    Water Quality Tolerance ........... %-.2f %s");
    static constexpr ceSS FMT36 = ceSS("    Specific Gravity .................. %-.2f");
    static constexpr ceSS FMT37a = ceSS("    Relative Kinematic Viscosity ...... %-.2f");
    static constexpr ceSS FMT37b = ceSS("    Relative Chemical Diffusivity ..... %-.2f");
    static constexpr ceSS FMT38 = ceSS("    Demand Multiplier ................. %-.2f");
    static constexpr ceSS FMT39 = ceSS("    Total Duration .................... %-.2f %s");
    static constexpr ceSS FMT40 = ceSS("    Reporting Criteria:");
    static constexpr ceSS FMT41 = ceSS("       No Nodes");
    static constexpr ceSS FMT42 = ceSS("       All Nodes");
    static constexpr ceSS FMT43 = ceSS("       Selected Nodes");
    static constexpr ceSS FMT44 = ceSS("       No Links");
    static constexpr ceSS FMT45 = ceSS("       All Links");
    static constexpr ceSS FMT46 = ceSS("       Selected Links");
    static constexpr ceSS FMT47 = ceSS("       with %s below %-.2f %s");
    static constexpr ceSS FMT48 = ceSS("       with %s above %-.2f %s");

    //----- Status Report Format Strings ----------------------
    static constexpr ceSS FMT49 = ceSS("Hydraulic Status:");
    static constexpr ceSS FMT50 = ceSS("%10s: Tank %s is %s at %-.2f %s");
    static constexpr ceSS FMT51 = ceSS("%10s: Reservoir %s is %s");
    static constexpr ceSS FMT52 = ceSS("%10s: %s %s %s");
    static constexpr ceSS FMT53 = ceSS("%10s: %s %s changed from %s to %s");
    static constexpr ceSS FMT54 = ceSS("%10s: %s %s changed by %s %s control");
    static constexpr ceSS FMT55 = ceSS("%10s: %s %s changed by timer control");
    static constexpr ceSS FMT56 = ceSS("            %s %s setting changed to %-.2f");
    static constexpr ceSS FMT57 = ceSS("            %s %s switched from %s to %s");
    static constexpr ceSS FMT58 = ceSS("%10s: Balanced after %-d trials");
    static constexpr ceSS FMT59 = ceSS("%10s: Unbalanced after %-d trials (flow change = %-.6f)");

    static constexpr ceSS FMT60a = ceSS("            Max. flow imbalance is %.4f %s at Node %s");
    static constexpr ceSS FMT60b = ceSS("            Max. head imbalance is %.4f %s at Link %s");

    static constexpr ceSS FMT61 = ceSS("%10s: Valve %s caused ill-conditioning");
    static constexpr ceSS FMT62 = ceSS("%10s: System ill-conditioned at node %s");
    static constexpr ceSS FMT63 = ceSS("%10s: %s %s changed by rule %s");
    static constexpr ceSS FMT64 = ceSS("%10s: Balancing the network:\n");
    static constexpr ceSS FMT65 = ceSS("            Trial %2d: relative flow change = %-.6f");
    static constexpr ceSS FMT66 = ceSS("                      maximum  flow change = %.4f for Link %s");
    static constexpr ceSS FMT67 = ceSS("                      maximum  flow change = %.4f for Node %s");
    static constexpr ceSS FMT68 = ceSS("                      maximum  head error  = %.4f for Link %s\n");
    static constexpr ceSS FMT69a = ceSS("            1 node had its demand reduced by a total of %.2f%%");
    static constexpr ceSS FMT69b = ceSS("            %-d nodes had demands reduced by a total of %.2f%%");

    //----- Energy Report Table -------------------------------
    static constexpr ceSS FMT71 = ceSS("Energy Usage:");
    static constexpr ceSS FMT72 = ceSS("           Usage   Avg.     Kw-hr      Avg.      Peak      Cost");
    static constexpr ceSS FMT73 = ceSS("Pump      Factor Effic.     %s        Kw        Kw      /day");
    static constexpr ceSS FMT74 = ceSS("%38s Demand Charge: %9.2f");
    static constexpr ceSS FMT75 = ceSS("%38s Total Cost:    %9.2f");

    //----- Node Report Table ---------------------------------
    static constexpr ceSS FMT76 = ceSS("%s Node Results:");
    static constexpr ceSS FMT77 = ceSS("Node Results:");
    static constexpr ceSS FMT78 = ceSS("Node Results at %s hrs:");

    //----- Link Report Table ---------------------------------
    static constexpr ceSS FMT79 = ceSS("%s Link Results:");
    static constexpr ceSS FMT80 = ceSS("Link Results:");
    static constexpr ceSS FMT81 = ceSS("Link Results at %s hrs:");
    static constexpr ceSS FMT82 = ceSS("\n\f\n  Page %-d    %60.60s\n");

    //----- Progress Messages ---------------------------------
    static constexpr ceSS FMT100 = ceSS("    Retrieving network data ...                   ");
    static constexpr ceSS FMT101 = ceSS("    Computing hydraulics at hour %-10s       ");
    static constexpr ceSS FMT102 = ceSS("    Computing water quality at hour %-10s    ");
    static constexpr ceSS FMT103 = ceSS("    Writing output report ...                     ");
    static constexpr ceSS FMT106 = ceSS("    Transferring results to file ...              ");
    static constexpr ceSS FMT104 = ceSS("Analysis begun %s");
    static constexpr ceSS FMT105 = ceSS("Analysis ended %s");

    //----- Rule Error Messages -------------------------------
    static constexpr ceSS R_ERR201 = ceSS("Input Error 201: syntax error in following line of ");
    static constexpr ceSS R_ERR202 = ceSS("Input Error 202: illegal numeric value in following line of ");
    static constexpr ceSS R_ERR203 = ceSS("Input Error 203: undefined node in following line of ");
    static constexpr ceSS R_ERR204 = ceSS("Input Error 204: undefined link in following line of ");
    static constexpr ceSS R_ERR207 = ceSS("Input Error 207: attempt to control a CV in following line of ");
    static constexpr ceSS R_ERR221 = ceSS("Input Error 221: mis-placed clause in following line of ");

    //----- Specific Warning Messages -------------------------
    static constexpr ceSS WARN01 = ceSS("WARNING: System unbalanced at %s hrs.");
    static constexpr ceSS WARN02 = ceSS("WARNING: Maximum trials exceeded at %s hrs. System may be unstable.");
    static constexpr ceSS WARN03a = ceSS("WARNING: Node %s disconnected at %s hrs");
    static constexpr ceSS WARN03b = ceSS("WARNING: %d additional nodes disconnected at %s hrs");
    static constexpr ceSS WARN03c = ceSS("WARNING: System disconnected because of Link %s");
    static constexpr ceSS WARN04 = ceSS("WARNING: Pump %s %s at %s hrs.");
    static constexpr ceSS WARN05 = ceSS("WARNING: %s %s %s at %s hrs.");
    static constexpr ceSS WARN06 = ceSS("WARNING: Negative pressures at %s hrs.");

    //----- General Warning Messages --------------------------
    static constexpr ceSS WARN1 = ceSS("WARNING: System hydraulically unbalanced.");
    static constexpr ceSS WARN2 = ceSS("WARNING: System may be hydraulically unstable.");
    static constexpr ceSS WARN3 = ceSS("WARNING: System disconnected.");
    static constexpr ceSS WARN4 = ceSS("WARNING: Pumps cannot deliver enough flow or head.");
    static constexpr ceSS WARN5 = ceSS("WARNING: Valves cannot deliver enough flow.");
    static constexpr ceSS WARN6 = ceSS("WARNING: System has negative pressures.");
    static constexpr ceSS WARN7 = ceSS("WARNING: Network asset could not be found, but network parsing continued.");

#pragma endregion
#pragma region Enum Text Collections
    static const char* NodeTxt[] = { t_JUNCTION.c_str(),
                               t_RESERVOIR.c_str(),
                               t_TANK.c_str() };
    static const char* LinkTxt[] = { w_CV.c_str(),
                               w_PIPE.c_str(),
                               w_PUMP.c_str(),
                               w_PRV.c_str(),
                               w_PSV.c_str(),
                               w_PBV.c_str(),
                               w_FCV.c_str(),
                               w_TCV.c_str(),
                               w_GPV.c_str() };
    static const char* StatTxt[] = { t_XHEAD.c_str(),
                               t_TEMPCLOSED.c_str(),
                               t_CLOSED.c_str(),
                               t_OPEN.c_str(),
                               t_ACTIVE.c_str(),
                               t_XFLOW.c_str(),
                               t_XFCV.c_str(),
                               t_XPRESSURE.c_str(),
                               t_FILLING.c_str(),
                               t_EMPTYING.c_str(),
                               t_OVERFLOWING.c_str() };
    static const char* FormTxt[] = { w_HW.c_str(),
                               w_DW.c_str(),
                               w_CM.c_str() };
    static const char* RptFormTxt[] = { t_HW.c_str(),
                               t_DW.c_str(),
                               t_CM.c_str() };
    static const char* RptFlowUnitsTxt[] = { u_CFS.c_str(),
                               u_GPM.c_str(),
                               u_MGD.c_str(),
                               u_IMGD.c_str(),
                               u_AFD.c_str(),
                               u_LPS.c_str(),
                               u_LPM.c_str(),
                               u_MLD.c_str(),
                               u_CMH.c_str(),
                               u_CMD.c_str() };
    static const char* FlowUnitsTxt[] = { w_CFS.c_str(),
                               w_GPM.c_str(),
                               w_MGD.c_str(),
                               w_IMGD.c_str(),
                               w_AFD.c_str(),
                               w_LPS.c_str(),
                               w_LPM.c_str(),
                               w_MLD.c_str(),
                               w_CMH.c_str(),
                               w_CMD.c_str() };
    static const char* PressUnitsTxt[] = { w_PSI.c_str(),
                               w_KPA.c_str(),
                               w_METERS.c_str() };
    static const char* DemandModelTxt[] = { w_DDA.c_str(),
                               w_PDA.c_str(),
                               NULL };
    static const char* QualTxt[] = { w_NONE.c_str(),
                               w_CHEM.c_str(),
                               w_AGE.c_str(),
                               w_TRACE.c_str(),
                               w_ENERGYINTENSITY.c_str() };
    static const char* SourceTxt[] = { w_CONCEN.c_str(),
                               w_MASS.c_str(),
                               w_SETPOINT.c_str(),
                               w_FLOWPACED.c_str() };
    static const char* ControlTxt[] = { w_BELOW.c_str(),
                               w_ABOVE.c_str(),
                               w_TIME.c_str(),
                               w_CLOCKTIME.c_str() };
    static const char* TstatTxt[] = { w_NONE.c_str(),
                               w_AVG.c_str(),
                               w_MIN.c_str(),
                               w_MAX.c_str(),
                               w_RANGE.c_str() };
    static const char* MixTxt[] = { w_MIXED.c_str(),
                               w_2COMP.c_str(),
                               w_FIFO.c_str(),
                               w_LIFO.c_str(),
                               NULL };
    static const char* RptFlagTxt[] = { w_NO.c_str(),
                               w_YES.c_str(),
                               w_FULL.c_str() };
    static const char* SectTxt[] = { s_TITLE.c_str(),      s_JUNCTIONS.c_str(),  s_RESERVOIRS.c_str(),
                               s_TANKS.c_str(),      s_PIPES.c_str(),      s_PUMPS.c_str(),
                               s_VALVES.c_str(),     s_CONTROLS.c_str(),   s_RULES.c_str(),
                               s_DEMANDS.c_str(),    s_SOURCES.c_str(),    s_EMITTERS.c_str(),
                               s_PATTERNS.c_str(),   s_CURVES.c_str(),     s_QUALITY.c_str(),
                               s_STATUS.c_str(),     s_ROUGHNESS.c_str(),  s_ENERGY.c_str(),
                               s_REACTIONS.c_str(),  s_MIXING.c_str(),     s_REPORT.c_str(),
                               s_TIMES.c_str(),      s_OPTIONS.c_str(),    s_COORDS.c_str(),
                               s_VERTICES.c_str(),   s_LABELS.c_str(),     s_BACKDROP.c_str(),
                               s_TAGS.c_str(),       s_END.c_str(),
                               NULL };
    static const char* Fldname[] = { t_ELEV.c_str(),       t_DEMAND.c_str(),     t_HEAD.c_str(),
                               t_PRESSURE.c_str(),   t_QUALITY.c_str(),    t_LENGTH.c_str(),
                               t_DIAM.c_str(),       t_FLOW.c_str(),       t_VELOCITY.c_str(),
                               t_HEADLOSS.c_str(),   t_LINKQUAL.c_str(),   t_LINKSTATUS.c_str(),
                               t_SETTING.c_str(),    t_REACTRATE.c_str(),  t_FRICTION.c_str(),
                               (const char*)"",  (const char*)"",  (const char*)"",  (const char*)"",  (const char*)"",  (const char*)"", NULL };
    static const char* Ruleword[] = { w_RULE.c_str(), w_IF.c_str(), w_AND.c_str(), w_OR.c_str(), w_THEN.c_str(), w_ELSE.c_str(), w_PRIORITY.c_str(), NULL };
    static const char* Varword[] = { w_DEMAND.c_str(), w_HEAD.c_str(), w_GRADE.c_str(), w_LEVEL.c_str(), w_PRESSURE.c_str(),
        w_FLOW.c_str(), w_STATUS.c_str(),   w_SETTING.c_str(),   w_POWER.c_str(), w_TIME.c_str(), w_CLOCKTIME.c_str(), w_FILLTIME.c_str(), w_DRAINTIME.c_str(), NULL };
    static const char* Object[] = { w_JUNC.c_str(),  w_RESERV.c_str(), w_TANK.c_str(), w_PIPE.c_str(), w_PUMP.c_str(), w_VALVE.c_str(), w_NODE.c_str(), w_LINK.c_str(), w_SYSTEM.c_str(), NULL };
    static const char* Operator[] = { (const char*)"=",  (const char*)"<>",  (const char*)"<=",    (const char*)">=",    (const char*)"<", (const char*)">", w_IS.c_str(), w_NOT.c_str(), w_BELOW.c_str(), w_ABOVE.c_str(), NULL };
    static const char* Value[] = { (const char*)"XXXX", w_OPEN.c_str(), w_CLOSED.c_str(), w_ACTIVE.c_str(), NULL };
#pragma endregion
#pragma region utility functions
    static size_t	f_save(REAL4* x, int n, FILE* file)
    {
        return fwrite(x + 1, sizeof(REAL4), n, file);
    };
    static size_t	f_read(REAL4* x, int n, FILE* file)
    {
        return fread(x + 1, sizeof(REAL4), n, file);
    };
    static int		strcomp(const char* s1, const char* s2)
        /*---------------------------------------------------------------
        **  Input:   s1 = character string
        **           s2 = character string
        **  Output:  none
        **  Returns: 1 if s1 is same as s2, 0 otherwise
        **  Purpose: case insensitive comparison of strings s1 & s2
        **---------------------------------------------------------------
        */
    {
        int i;
        for (i = 0; UCHAR(s1[i]) == UCHAR(s2[i]); i++)
        {
            if (!s1[i + 1] && !s2[i + 1]) return 1;
        }
        return 0;
    };
    static SCALER	interp(int n, SCALER x[], SCALER y[], SCALER xx)
        /*----------------------------------------------------------------
        **  Input:   n  = number of data pairs defining a curve
        **           x  = x-data values of curve
        **           y  = y-data values of curve
        **           xx = specified x-value
        **  Output:  none
        **  Returns: y-value on curve at x = xx
        **  Purpose: uses linear interpolation to find y-value on a
        **           data curve corresponding to specified x-value.
        **  NOTE:    does not extrapolate beyond endpoints of curve.
        **----------------------------------------------------------------
        */
    {
        int k, m;
        SCALER dx, dy;

        m = n - 1;                        // Highest data index
        if (xx <= x[0]) return (y[0]);    // xx off low end of curve
        for (k = 1; k <= m; k++)          // Bracket xx on curve
        {
            if (x[k] >= xx)               // Interp. over interval
            {
                dx = x[k] - x[k - 1];
                dy = y[k] - y[k - 1];
                if (ABS(dx) < TINY) return (y[k]);
                else return (y[k] - (x[k] - xx) * dy / dx);
            }
        }
        return (y[m]); // xx off high end of curve
    };
    static char*    geterrmsg(int errcode, char* msg)
        /*----------------------------------------------------------------
        **  Input:   errcode = error code
        **  Output:  none
        **  Returns: pointer to string with error message
        **  Purpose: retrieves text of error message
        **----------------------------------------------------------------
        */
    {
        switch (errcode) {
#define DAT(code,string) case code: ::strcpy(msg, string); break;
#include "EPANET_errors.dat"
#undef DAT
        default:
            ::strcpy(msg, "");
        }
        return (msg);
    };
    static void		writewin(void(*vp)(char*), char* s)
        /*----------------------------------------------------------------
        **  Input:   text string
        **  Output:  none
        **  Purpose: passes character string to viewprog() in
        **           application which calls the EPANET DLL
        **----------------------------------------------------------------
        */
    {
        char progmsg[MAXMSG + 1];
        if (vp != NULL)
        {
            ::strncpy(progmsg, s, MAXMSG);
            vp(progmsg);
        }
    };
    static char*    xstrcpy(char** s1, const char* s2, const size_t n)
        //----------------------------------------------------------------
        //  Input:   s1 = destination string
        //           s2 = source string
        //           n = maximum size of strings
        //  Output:  none
        //  Purpose: like strcpy except for dynamic strings.
        //  Note:    The calling program is responsible for ensuring that
        //           s1 points to a valid memory location or is NULL. E.g.,
        //           the following code will likely cause a segment fault:
        //             char *s;
        //             s = xstrcpy(s, "Some text");
        //           while this would work correctly:
        //             char *s = NULL;
        //             s = xstrcpy(s, "Some text");
        //----------------------------------------------------------------
    {
        size_t n1 = 0, n2 = 0;

        // Find size of source string
        if (s2) n2 = strlen(s2);
        if (n2 > n) n2 = n;

        // Source string is empty -- free destination string
        if (n2 == 0)
        {
            free(*s1);
            *s1 = NULL;
            return NULL;
        }

        // See if size of destination string needs to grow
        if (*s1) n1 = strlen(*s1);
        if (n2 > n1) *s1 = (char*)realloc(*s1, (n2 + 1) * sizeof(char));

        // Copy the source string into the destination string
        strncpy(*s1, s2, n2 + 1);
        return *s1;
    };
    static int		namevalid(const char* name)
        //----------------------------------------------------------------
        //  Input:   name = name used to ID an object
        //  Output:  returns TRUE if name is valid, FALSE if not
        //  Purpose: checks that an object's ID name is valid.
        //----------------------------------------------------------------
    {
        size_t n = strlen(name);
        if (n < 1 || n > MAXID || strpbrk(name, " ;") || name[0] == '"') return FALSE;
        return TRUE;
    };
    static void		getTmpName(char* fname)
        //----------------------------------------------------------------
        //  Input:   fname = file name string
        //  Output:  an unused file name
        //  Purpose: creates a temporary file name with an "en" prefix
        //           or a blank name if an error occurs.
        //----------------------------------------------------------------
    {
#ifdef _WIN32

        char* name = NULL;

        // --- use Windows _tempnam function to get a pointer to an
        //     unused file name that begins with "en"
        strcpy(fname, "");
        name = _tempnam(NULL, "en");
        if (name)
        {
            // --- copy the file name to fname
            if (strlen(name) < MAXFNAME) strncpy(fname, name, MAXFNAME);

            // --- free the pointer returned by _tempnam
            free(name);
        }
        // --- for non-Windows systems:
#else
        // --- use system function mkstemp() to create a temporary file name
    /*
        int f = -1;
        strcpy(fname, "enXXXXXX");
        f = mkstemp(fname);
        close(f);
        remove(fname);
    */
        strcpy(fname, "enXXXXXX");
        FILE* f = fdopen(mkstemp(fname), "r");
        if (f == NULL) strcpy(fname, "");
        else fclose(f);
        remove(fname);
#endif
    };
    static int      match(const char* str, const char* substr)
        /*
        **--------------------------------------------------------------
        **  Input:   *str    = string being searched
        **           *substr = substring being searched for
        **  Output:  returns 1 if substr found in str, 0 if not
        **  Purpose: sees if substr matches any part of str
        **
        **      (Not case sensitive)
        **--------------------------------------------------------------
        */
    {
        int i, j;

        // Fail if substring is empty
        if (!substr[0]) return 0;

        // Skip leading blanks of str
        for (i = 0; str[i]; i++)
        {
            if (str[i] != ' ') break;
        }

        // Check if substr matches remainder of str
        for (j = 0; substr[j]; i++, j++)
        {
            if (!str[i] || UCHAR(str[i]) != UCHAR(substr[j])) return 0;
        }
        return 1;
    };
    static int      findmatch(const char* line, const char* keyword[])
        /*
        **--------------------------------------------------------------
        **  Input:   *line      = line from input file
        **           *keyword[] = list of NULL terminated keywords
        **  Output:  returns index of matching keyword or
        **           -1 if no match found
        **  Purpose: determines which keyword appears on input line
        **--------------------------------------------------------------
        */
    {
        int i = 0;
        while (keyword[i] != NULL)
        {
            if (match(line, keyword[i])) return i;
            i++;
        }
        return -1;
    };
    static int      gettokens(char* s, char** Tok, int maxToks, char* comment)
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

        // clear comment
        comment[0] = '\0';

        // Begin with no tokens
        for (n = 0; n < maxToks; n++) Tok[n] = NULL;
        n = 0;

        // Truncate s at start of comment
        c = strchr(s, ';');
        if (c)
        {
            c2 = c + 1;
            if (c2)
            {
                // there is a comment here, after the semi-colon.
                len = strlen(c2);
                if (len > 0)
                {
                    len = strcspn(c2, "\n\r");
                    len = fMIN(len, MAXMSG);
                    strncpy(comment, c2, len);
                    comment[(size_t)fMIN(len, MAXMSG)] = '\0';
                }
            }
            *c = '\0';
        }
        len = (int)strlen(s);

        // Scan s for tokens until nothing left
        while (len > 0 && n < MAXTOKS)
        {
            m = (int)strcspn(s, SEPSTR.c_str());     // Find token length
            if (m == len)                   // s is last token
            {
                Tok[n] = s;
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
                Tok[n] = s;                  // Save pointer to token
                n++;                         // Update token count
                s += m + 1;                    // Begin next token
            }
        }
        return n;
    };
    static int      getfloat(const char* s, SCALER* y)
        /*
        **-----------------------------------------------------------
        **  Input:   *s = character string
        **  Output:  *y = floating point number
        **           returns 1 if conversion successful, 0 if not
        **  Purpose: converts string to floating point number
        **-----------------------------------------------------------
        */
    {
        char* endptr;
        *y = (SCALER)strtod(s, &endptr);
        if (*endptr > 0) return 0;
        return 1;
    };
    static SCALER   hour(const char* time, const char* units)
        /*
        **---------------------------------------------------------
        **  Input:   *time  = string containing a time value
        **           *units = string containing time units
        **  Output:  returns numerical value of time in hours,
        **           or -1 if an error occurs
        **  Purpose: converts time from units to hours
        **---------------------------------------------------------
        */
    {
        int n;
        SCALER y[3];
        char* s;

        // Separate clock time into hrs, min, sec
        for (n = 0; n < 3; n++) y[n] = 0.0;
        n = 0;
        cweeStr copy = time;
        s = strtok((char*)copy.c_str(), ":");
        while (s != NULL && n <= 3)
        {
            if (!getfloat(s, &y[n])) return -1.0;
            s = strtok(NULL, ":");
            n++;
        }

        // If decimal time with units attached then convert to hours
        if (n == 1)
        {
            if (strlen(units) == 0)      return (y[0]);
            if (match(units, w_SECONDS)) return (y[0] / 3600.0);
            if (match(units, w_MINUTES)) return (y[0] / 60.0);
            if (match(units, w_HOURS))   return (y[0]);
            if (match(units, w_DAYS))    return (y[0] * 24.0);
        }

        // Convert hh:mm:ss format to decimal hours
        if (n > 1) y[0] = y[0] + y[1] / 60.0 + y[2] / 3600.0;

        // If am/pm attached then adjust hour accordingly
        // (12 am is midnight, 12 pm is noon)
        if (units[0] == '\0') return y[0];
        if (match(units, w_AM))
        {
            if (y[0] >= 13.0) return -1.0;
            if (y[0] >= 12.0) return (y[0] - 12.0);
            else              return (y[0]);
        }
        if (match(units, w_PM))
        {
            if (y[0] >= 13.0) return -1.0;
            if (y[0] >= 12.0) return y[0];
            else              return (y[0] + 12.0);
        }
        return -1.0;
    };
    static int      powercurve(SCALER h0, SCALER h1, SCALER h2, SCALER q1, SCALER q2, SCALER* a, SCALER* b, SCALER* c)
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
        SCALER h4, h5;

        if (h0 < TINY || h0 - h1 < TINY || h1 - h2 < TINY ||
            q1 < TINY || q2 - q1 < TINY
            ) return 0;
        *a = h0;
        h4 = h0 - h1;
        h5 = h0 - h2;
        *c = ::epanet::log(h5 / h4) / ::epanet::log(q2 / q1);
        if (*c <= 0.0 || *c > 20.0) return 0;
        *b = -h4 / ::epanet::pow(q1, *c);
        if (*b >= 0.0) return 0;
        return 1;
    };
    static char*    clocktime(char* atime, units::time::second_t seconds)
        /*
        **--------------------------------------------------------------
        **   Input:   seconds = time in seconds
        **   Output:  atime = time in hrs:min
        **            (returns pointer to atime)
        **   Purpose: converts time in seconds to hours:minutes format
        **--------------------------------------------------------------
        */
    {
        units::time::hour_t h, fractional_h;
        units::time::minute_t m, fractional_m;
        units::dimensionless::scalar_t intPart;
        cweeTime t0, t1;


        t0 = cweeTime::Now().ToStartOfDay();
        t1 = t0 + (u64)seconds;

        seconds = (units::time::second_t)(u64)(t1 - t0);
        
        // extract the hour component(s)
        h = seconds;
        fractional_h = units::math::modf(scalar_t(h()), &intPart)();
        h = intPart();
        m = fractional_h;
        fractional_m = units::math::modf(scalar_t(m()), &intPart)();
        m = intPart();

        seconds = fractional_m;
        // second_t fractional_s = units::math::modf(scalar_t(((second_t)fractional_m)()), &intPart);
        // s = intPart();

        sprintf(atime, "%i:%02d:%02d", (int)(double)h, (int)(double)m, (int)(double)seconds);
        return atime;
    };
    static void     getobjtxt(int objtype, int subtype, char* objtxt)
        //-----------------------------------------------------------------------------
        //  Retrieve the text label for a specific type of object.
        //-----------------------------------------------------------------------------
    {
        if (objtype == r_NODE)
        {
            switch (subtype)
            {
            case JUNCTION:  strcpy(objtxt, "JUNCTION"); break;
            case RESERVOIR: strcpy(objtxt, "RESERVOIR"); break;
            case TANK:      strcpy(objtxt, "TANK"); break;
            default:        strcpy(objtxt, "NODE");
            }
        }
        else if (objtype == r_LINK)
        {
            switch (subtype)
            {
            case CVPIPE:
            case PIPE:  strcpy(objtxt, "PIPE"); break;
            case PUMP:  strcpy(objtxt, "PUMP"); break;
            default:    strcpy(objtxt, "VALVE");
            }
        }
        else strcpy(objtxt, "SYSTEM");
    };
    static void     gettimetxt(SCALER secs, char* timetxt)
        //-----------------------------------------------------------------------------
        //  Convert number of seconds to a text string in hrs:min:sec format.
        //-----------------------------------------------------------------------------
    {
        int hours = 0, minutes = 0, seconds = 0;
        hours = (int)secs / 3600;
        if (hours > 24 * 7) sprintf(timetxt, "%.4f", (double)(secs / 3600.0));
        else
        {
            minutes = (int)((secs - 3600 * hours) / 60);
            seconds = (int)(secs - 3600 * hours - minutes * 60);
            sprintf(timetxt, "%d:%02d:%02d", hours, minutes, seconds);
        }
    };
    static char*    fillstr(char* s, char ch, int n)
        /*
        **---------------------------------------------------------
        **  Fills n bytes of s to character ch.
        **  NOTE: does not check for overwriting s.
        **---------------------------------------------------------
        */
    {
        int i;
        for (i = 0; i <= n; i++) s[i] = ch;
        s[n + 1] = '\0';
        return (s);
    };
    static void		adjustcurve(int* curve, int index, Pcurve* p = nullptr)
        /*----------------------------------------------------------------
        ** Local function that modifies a reference to a deleted data curve
        **----------------------------------------------------------------
        */
    {
        if (*curve == index) {
            *curve = 0;
            if (p) *p = nullptr;
        }
        else if (*curve > index) (*curve)--;
    };
    static void		adjustpattern(int* pat, int index, Ppattern* p = nullptr)
        /*----------------------------------------------------------------
        ** Local function that modifies a reference to a deleted time pattern
        **----------------------------------------------------------------
        */
    {
        if (*pat == index) {
            *pat = 0;
            if (p) *p = nullptr;
        }
        else if (*pat > index) (*pat)--;
    };
    static SCALER   frictionFactor(SCALER q, SCALER e, SCALER s, SCALER* dfdq)
        /*
        **--------------------------------------------------------------
        **   Input:   q = |pipe flow|
        **            e = pipe roughness  / diameter
        **            s = viscosity * pipe diameter
        **   Output:  dfdq = derivative of friction factor w.r.t. flow
        **   Returns: pipe's friction factor
        **   Purpose: computes Darcy-Weisbach friction factor and its
        **            derivative as a function of Reynolds Number (Re).
        **--------------------------------------------------------------
        */
    {
        SCALER f;                // friction factor
        SCALER x1, x2, x3, x4,
            y1, y2, y3,
            fa, fb, r;
        SCALER w = q / s;        // Re*Pi/4

        //   For Re >= 4000 use Swamee & Jain approximation
        //   of the Colebrook-White Formula
        if (w >= A1)
        {
            y1 = A8 / ::epanet::pow(w, 0.9);
            y2 = e / 3.7 + y1;
            y3 = A9 * ::epanet::log(y2);
            f = 1.0 / (y3 * y3);
            *dfdq = 1.8 * f * y1 * A9 / y2 / y3 / q;
        }

        //   Use interpolating polynomials developed by
        //   E. Dunlop for transition flow from 2000 < Re < 4000.
        else
        {
            y2 = e / 3.7 + AB;
            y3 = A9 * ::epanet::log(y2);
            fa = 1.0 / (y3 * y3);
            fb = (2.0 + AC / (y2 * y3)) * fa;
            r = w / A2;
            x1 = 7.0 * fa - fb;
            x2 = 0.128 - 17.0 * fa + 2.5 * fb;
            x3 = -0.128 + 13.0 * fa - (fb + fb);
            x4 = 0.032 - 3.0 * fa + 0.5 * fb;
            f = x1 + r * (x2 + r * (x3 + r * x4));
            *dfdq = (x2 + r * (2.0 * x3 + r * 3.0 * x4)) / s / A2;
        }
        return f;
    };
    static SCALER   getucf(SCALER order)
        /*
        **--------------------------------------------------------------
        **   Input:   order = bulk reaction order
        **   Output:  returns a unit conversion factor
        **   Purpose: converts bulk reaction rates from per Liter to
        **            per FT3 basis
        **--------------------------------------------------------------
        */
    {
        if (order < 0.0)  order = 0.0;
        if (order == 1.0) return (1.0);
        else return (1. / ::epanet::pow(LperFT3, (order - 1.0)));
    };
#pragma endregion
};
