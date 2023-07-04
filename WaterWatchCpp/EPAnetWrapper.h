/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "EPANET_CPP.h"
#include "cweeThreadedMap.h"

class EPAnet_Local {
public:
	cweeUnion<int, epanet::EN_Project>   createNewProject(void);
	void deleteProject(int const& projectNum);
	void initBuild(int const& projectNum, cweeStr const& rptFile, cweeStr const& outFile, int const& unitsType, int const& headLossType);

	void loadINP(int const& projectNum, const char* filePathToInpFile, const char* filePathToRptFile = "");
	void closeINP(int const& projectNum);
	void saveINP(int const& projectNum, cweeStr const& filePathToInpFile);
	cweeStr getTitle(int const& projectNum);

	cweeStr getDescription(int const& projectNum, bool const& isNode, int const& epanetIndex);
	void setDescription(int const& projectNum, bool const& isNode, int const& epanetIndex, cweeStr const& description);
	int	getCount(int const& projectNum, int const& objectType);

	void setFlowUnits(int const& projectNum, int const& units);
	/*!
	EN_NONE = 0 // No quality analysis.
	EN_CHEM = 1 // Chemical fate and transport.
	EN_AGE = 2 // Water age analysis.
	EN_TRACE = 3 // Source tracing analysis.
	*/
	void setQualType(int const& projectNum, int const& qualType, cweeStr const& chemName, cweeStr const& chemUnits, cweeStr const& traceNode);
	/*!
  EN_DURATION     = 0,  //!< Total simulation duration
  EN_HYDSTEP      = 1,  //!< Hydraulic time step
  EN_QUALSTEP     = 2,  //!< Water quality time step
  EN_PATTERNSTEP  = 3,  //!< Time pattern period
  EN_PATTERNSTART = 4,  //!< Time when time patterns begin
  EN_REPORTSTEP   = 5,  //!< Reporting time step
  EN_REPORTSTART  = 6,  //!< Time when reporting starts
  EN_RULESTEP     = 7,  //!< Rule-based control evaluation time step
  EN_STATISTIC    = 8,  //!< Reporting statistic code (see @ref EN_StatisticType)
  EN_PERIODS      = 9,  //!< Number of reporting time periods (read only)
  EN_STARTTIME    = 10, //!< Simulation starting time of day
  EN_HTIME        = 11, //!< Elapsed time of current hydraulic solution (read only)
  EN_QTIME        = 12, //!< Elapsed time of current quality solution (read only)
  EN_HALTFLAG     = 13, //!< Flag indicating if the simulation was halted (read only)
  EN_NEXTEVENT    = 14, //!< Shortest time until a tank becomes empty or full (read only)
  EN_NEXTEVENTTANK = 15  //!< Index of tank with shortest time to become empty or full (read only)
*/
	void setTimeParam(int const& projectNum, int const& param, units::time::second_t const& value);
	/*!
	  EN_TRIALS         = 0,  //!< Maximum trials allowed for hydraulic convergence
	  EN_ACCURACY       = 1,  //!< Total normalized flow change for hydraulic convergence
	  EN_TOLERANCE      = 2,  //!< Water quality tolerance
	  EN_EMITEXPON      = 3,  //!< Exponent in emitter discharge formula
	  EN_DEMANDMULT     = 4,  //!< Global demand multiplier
	  EN_HEADERROR      = 5,  //!< Maximum head loss error for hydraulic convergence
	  EN_FLOWCHANGE     = 6,  //!< Maximum flow change for hydraulic convergence
	  EN_HEADLOSSFORM   = 7,  //!< Head loss formula (see @ref EN_HeadLossType)
	  EN_GLOBALEFFIC    = 8,  //!< Global pump efficiency (percent)
	  EN_GLOBALPRICE    = 9,  //!< Global energy price per KWH
	  EN_GLOBALPATTERN  = 10, //!< Index of a global energy price pattern
	  EN_DEMANDCHARGE   = 11,  //!< Energy charge per max. KW usage
	  EN_SP_GRAVITY     = 12, //!< Specific gravity
	  EN_SP_VISCOS      = 13, //!< Specific viscosity (relative to water at 20 deg C)
	  EN_UNBALANCED     = 14, //!< Extra trials allowed if hydraulics don't converge
	  EN_CHECKFREQ      = 15, //!< Frequency of hydraulic status checks
	  EN_MAXCHECK       = 16, //!< Maximum trials for status checking
	  EN_DAMPLIMIT      = 17, //!< Accuracy level where solution damping begins
	  EN_SP_DIFFUS      = 18, //!< Specific diffusivity (relative to chlorine at 20 deg C)
	  EN_BULKORDER      = 19, //!< Bulk water reaction order for pipes
	  EN_WALLORDER      = 20, //!< Wall reaction order for pipes (either 0 or 1)
	  EN_TANKORDER      = 21, //!< Bulk water reaction order for tanks
	  EN_CONCENLIMIT    = 22  //!< Limiting concentration for growth reactions*/
	void setOption(int const& projectNum, int const& option, units::dimensionless::scalar_t const& value);
	float getOption(int const& projectNum, int const& option);
	units::time::second_t getTimeParam(int const& projectNum, int const& param);
	int	 getFlowUnits(int const& projectNum);

	// returns in minutes. 
	units::time::second_t getCurrentSimTime(int const& projectNum);
	// returns in minutes. 
	units::time::second_t getCurrentSimTimestep(int const& projectNum);
	/*!
		Code	Description
		1	System hydraulically unbalanced - convergence to a hydraulic solution was not achieved in the allowed number of trials
		2	System may be hydraulically unstable - hydraulic convergence was only achieved after the status of all links was held fixed
		3	System disconnected - one or more nodes with positive demands were disconnected from all supply sources
		4	Pumps cannot deliver enough flow or head - one or more pumps were forced to either shut down (due to insufficient head) or operate beyond the maximum rated flow
		5	Valves cannot deliver enough flow - one or more flow control valves could not deliver the required flow even when fully open
		6	System has negative pressures - negative pressures occurred at one or more junctions with positive demand
		System Error 101: insufficient memory available.
		System Error 102: no network data available.
		System Error 103: hydraulics not initialized.
		System Error 104: no hydraulics for water quality analysis.
		System Error 105: water quality not initialized.
		System Error 106: no results saved to report on.
		System Error 107: hydraulics supplied from external file.
		System Error 108: cannot use external file while hydraulics solver is active.
		System Error 109: cannot change time parameter when solver is active.
		System Error 110: cannot solve network hydraulic equations.
		System Error 120: cannot solve water quality transport equations.
		Input Error 200: one or more errors in input file.
		Input Error 201: syntax error in following line of [%s] section:
		Input Error 202: %s %s contains illegal numeric value.
		Input Error 203: %s %s refers to undefined node.
		Input Error 204: %s %s refers to undefined link.
		Input Error 205: %s %s refers to undefined time pattern.
		Input Error 206: %s %s refers to undefined curve.
		Input Error 207: %s %s attempts to control a CV.
		Input Error 208: %s specified for undefined Node %s.
		Input Error 209: illegal %s value for Node %s.
		Input Error 210: %s specified for undefined Link %s.
		Input Error 211: illegal %s value for Link %s.
		Input Error 212: trace node %.0s %s is undefined.
		Input Error 213: illegal option value in [%s] section:
		Input Error 214: following line of [%s] section contains too many characters:
		Input Error 215: %s %s is a duplicate ID.
		Input Error 216: %s data specified for undefined Pump %s.
		Input Error 217: invalid %s data for Pump %s.
		Input Error 219: %s %s illegally connected to a tank.
		Input Error 220: %s %s illegally connected to another valve.
		Input Error 222: %s %s has same start and end nodes.
		Input Error 223: not enough nodes in network
		Input Error 224: no tanks or reservoirs in network.
		Input Error 225: invalid lower/upper levels for Tank %s.
		Input Error 226: no head curve supplied for Pump %s.
		Input Error 227: invalid head curve for Pump %s.
		Input Error 230: Curve %s has nonincreasing x-values.
		Input Error 233: Node %s is unconnected.
		Input Error 240: %s %s refers to undefined source.
		Input Error 241: %s %s refers to undefined control.
		Input Error 250: function call contains invalid format.
		Input Error 251: function call contains invalid parameter code.
		File Error 301: identical file names.
		File Error 302: cannot open input file.
		File Error 303: cannot open report file.
		File Error 304: cannot open binary output file.
		File Error 305: cannot open hydraulics file.
		File Error 306: hydraulics file does not match network data.
		File Error 307: cannot read hydraulics file.
		File Error 308: cannot save results to file.
		File Error 309: cannot save results to report file.
		Input Error 201: syntax error in following line of
		Input Error 202: illegal numeric value in following line of
		Input Error 203: undefined node in following line of
		Input Error 204: undefined link in following line of
		Input Error 207: attempt to control a CV in following line of
		Input Error 221: mis-placed clause in following line of
	*/
	int	 getCurrentError(int const& projectNum);

	// hydraulic simulation
	void fullHydraulicSim(int const& projectNum);
	void startHydraulicSim(int const& projectNum);
	void calcHydraulicsAtTime(int const& projectNum, int simQuality = ::epanet::HydraulicSimulationQuality::HIGHRES);
	void calcHydraulicTimestepToNextTime(int const& projectNum);
	void resetHydraulicSim(int const& projectNum);
	void endHydraulicSim(int const& projectNum);

	// quality simulation
	void fullQualitySim(int const& projectNum);
	void startQualitySim(int const& projectNum);
	void calcQualityAtTime(int const& projectNum);
	void calcQualityTimestepToNextTime(int const& projectNum);
	void resetQualitySim(int const& projectNum);
	void endQualitySim(int const& projectNum);

	int getNumControls(int const& projectNum);

	void deleteControl(int const& projectNum, int const& controlIndex);

	void deleteAllControls(int const& projectNum);

	/*!
		int controlType,	0 = EN_LOWLEVEL, 1 = EN_HILEVEL, 2 = EN_TIMER, 3 = EN_TIMEOFDAY
		cweeStr linkName,	Name of link to be controlled
		float setting,		Value of link if true
		cweeStr nodeName,	Node to look at
		float level,		Node setting to compare against */
	int addControl(int const& projectNum, int const& controlType, cweeStr const& linkName, float const& setting, cweeStr const& nodeName, float const& level);

	/*!
		easy way to add a control if you already typed the whole thing out. Returns an index to the control.
		*/
	int addControl(int const& projectNum, cweeStr const& entireControl);

	void getControl(int const& projectNum, int const& controlindex, int* type, int* linkIndex, units::dimensionless::scalar_t* setting, int* nodeIndex, units::dimensionless::scalar_t* level);

	cweeStr getControl(int const& projectNum, int const& controlindex);

	int getNumRules(int const& projectNum);

	int findRuleIndex(int const& projectNum, cweeStr const& ruleName);

	/*!
		IN:
		RULE  "name"
		IF	"refObjectType" "refObjectIndex" "refObjectVariable" "ComparisonOperator" "refObjectStatusOrValue"
		THEN	LINK	"linkIndex"  "linkValue" IS "ifTrueThenStatusOrSetting"
		PRIORITY	"priority"

		INTENDED TO BE A "SIMPLE" WAY OF IMPUTING A SINGLE-LINE RULE INTO EPANET.

		If confused, basically write it like a manually written rule in an EPAnet file.
		No technical limit to the complexity of the rule entered here, it could even be fully complete!
		Just becomes a pain to use the method. These are the minimum requirements to get it up and running, currently.

		OUT: index of new rule for other rule-modifying methods.
		*/
	int addRule(int const& projectNum, cweeStr const& name, cweeStr const& linkIndex, cweeStr const& linkValue, cweeStr const& ifTrueThenStatusOrSetting, cweeStr const& refObjectType, cweeStr const& refObjectIndex, cweeStr const& refObjectVariable, cweeStr const& ComparisonOperator, cweeStr const& refObjectStatusOrValue, units::dimensionless::scalar_t const& priority);

	/*!
		IN:
		cweeStr ruleName (NO SPACES!)
		cweeStr entireRUle:
			This must be the entire rule WITH \n NEW LINE EMBEDDED WITHIN. IT IS HIGHLY, HIGHLY RECOMMENDED
			THAT USERS CREATE THIS TYPE OF CWEESTR USING:
			cweeStr entireRule = cweeStr::printf("RULE %s \n IF %s %s %s %s \n THEN %s %s %s \n PRIORITY %s", cweeStr1.c_str(), cweeStr2.c_str(), etc...);
		If confused, basically write it like a manually written rule in an EPAnet file.
		No technical limit to the complexity of the rule entered here.

		OUT: index of new rule for other rule-modifying methods.
		*/
	int addRule(int const& projectNum, cweeStr const& ruleName, cweeStr entireRule, bool const& returnIndex);

	void deleteRule(int const& projectNum, int const& ruleIndex);

	void deleteAllRules(int const& projectNum);

	void getRuleSizes(int const& projectNum, int const& ruleIndex, int* nPremises, int* nThenActions, int* nElseActions, units::dimensionless::scalar_t* priority);

	/*
		int ruleIndex,				which rule
		int premiseIndex,			which premise level
		int premiseType,			IF=1, AND=2, OR=3
		int objectType,				NODE = 1/0, LINK = 2/1, SYSTEM = 3/2 // 6,7,8????
		int objectIndex,			NODE/LINK, use EPAnet function to get index. SYSTEM: 0?
		int objectVariable,			Demand = 0, Head = 1, Grade = 2, Level = 3, Pressure = 4, Flow = 5, Status = 6, Setting = 7, Power = 8, Time = 9, Clocktime = 10, Filltime = 11, Draintime = 12
		int comparisonOp,			Equal = 0, Not Equal = 1, <= 2, >= 3, < 4, > 5, Is Equal To = 6, Is Not Equal To = 7, Below = 8, Above = 9
		int comparisonStatus,		Open = 0, Closed = 1, Active = 2 // 1,2,3????
		float comparisonValue,
		*/
	void setRule_Premise(int const& projectNum, int const& ruleIndex, int const& premiseIndex, int const& premiseType, int const& objectType, int const& objectIndex, int const& objectVariable, int const& comparisonOp, int const& comparisonStatus, float const& comparisonValue);
	void setRule_ElseAction(int const& projectNum, int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting);
	void setRule_Priority(int const& projectNum, int const& index, units::dimensionless::scalar_t const& priority);
	void setRule_ThenAction(int const& projectNum, int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting);
	cweeStr getRuleID(int const& projectNum, int const& ruleIndex);

	/*!
		RULE		rule_name
		IF			PREMISE 1
		AND			PREMISE 2
		AND			PREMISE ...
		OR			PREMISE ...
		AND			PREMISE ...
		THEN		THEN_ACTION 1
		AND			THAN_ACTION 2
		AND			THAN_ACTION ...
		ELSE		ELSE_ACTION 1
		AND			ELSE_ACTION 2
		AND			ELSE_ACTION ...
		PRIORITY	priority_number
		*/
	cweeStr getRule(int const& projectNum, int const& ruleIndex);

	/*!
		Each "AND" && "ELSE" statement becomes its own, seperate rule.
		This is designed to simplify "assignment" to who owns / manages the rule in the future (instead of rules just floating about)
		*/
	cweeThreadedList<cweeStr> getRules(int const& projectNum);

	int getEPAnetIndexOfAsset(int projectNum, const cweeStr& name, bool isNode);
	bool isNode(asset_t const& type);
	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& projectNum, int const& EPAnetIndex, bool const& isNode, int const& prop);

	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& projectNum, int const& EPAnetIndex, value_t const& value);

	units::dimensionless::scalar_t getCurrentHydraulicValue(int projectNum, const cweeStr& name, const value_t& value, bool isNode);

	cweeThreadedList<cweeStr> getImpactedAssetsFromRule(int const& projectNum, int ruleIndex);
	cweeThreadedList<cweeStr> getImpactedAssetsFromRules(int const& projectNum);

	cweeThreadedList<cweeStr> getImpactedAssetsFromControls(int const& projectNum);

	int	 addNode(int const& projectNum, cweeStr const& name, int const& nodeType);
	void deleteNode(int const& projectNum, int const& nodeIndex, int const& actionCode);
	void setNodeID(int const& projectNum, int const& nodeIndex, cweeStr const& newid);
	/*!
		  EN_ELEVATION    = 0, //!< Elevation
		  EN_BASEDEMAND   = 1, //!< Primary demand baseline value
		  EN_PATTERN      = 2, //!< Primary demand time pattern index
		  EN_EMITTER      = 3, //!< Emitter flow coefficient
		  EN_INITQUAL     = 4, //!< Initial quality
		  EN_SOURCEQUAL   = 5, //!< Quality source strength
		  EN_SOURCEPAT    = 6, //!< Quality source pattern index
		  EN_SOURCETYPE   = 7, //!< Quality source type (see @ref EN_SourceType)
		  EN_TANKLEVEL    = 8, //!< Initial Tank Level (read only)
		  EN_DEMAND       = 9, //!< Current computed demand (read only)
		  EN_HEAD         = 10, //!< Current computed hydraulic head (read only)
		  EN_PRESSURE     = 11, //!< Current computed pressure (read only)
		  EN_QUALITY      = 12, //!< Current computed quality (read only)
		  EN_SOURCEMASS   = 13, //!< Current computed quality source mass inflow (read only)
		  EN_INITVOLUME   = 14, //!< Tank initial volume (read only)
		  EN_MIXMODEL     = 15, //!< Tank mixing model (see @ref EN_MixingModel)
		  EN_MIXZONEVOL   = 16, //!< Tank mixing zone volume (read only)
		  EN_TANKDIAM     = 17, //!< Tank diameter
		  EN_MINVOLUME    = 18, //!< Tank minimum volume
		  EN_VOLCURVE     = 19, //!< Tank volume curve index
		  EN_MINLEVEL     = 20, //!< Tank minimum level
		  EN_MAXLEVEL     = 21, //!< Tank maximum level
		  EN_MIXFRACTION  = 22, //!< Tank mixing fraction
		  EN_TANK_KBULK   = 23, //!< Tank bulk decay coefficient
		  EN_TANKVOLUME   = 24, //!< Current computed tank volume (read only)
		  EN_MAXVOLUME    = 25, //!< Tank maximum volume (read only)
		  EN_CANOVERFLOW  = 26, //!< Tank can overflow (= 1) or not (= 0)
		  EN_DEMANDDEFICIT = 27 //!< Amount that full demand is reduced under PDA (read only)
		*/
	void setNodeValue(int const& projectNum, int const& nodeIndex, int const& property, units::dimensionless::scalar_t const& value);
	void setJuncData(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& dmnd, cweeStr const& dmndpat);
	void setTankData(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& initlvl, units::dimensionless::scalar_t const& minlvl, units::dimensionless::scalar_t const& maxlvl, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& minvol, cweeStr const& volcurve);
	void setCoord(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y);
	int	 getNodeindex(int const& projectNum, cweeStr const& id);
	cweeStr getNodeid(int const& projectNum, int const& nodeIndex);
	int  getNodetype(int const& projectNum, int const& nodeIndex);

	float getNodevalue(int const& projectNum, int const& nodeIndex, int const& property);
	float getCoordX(int const& projectNum, int const& nodeIndex);
	float getCoordY(int const& projectNum, int const& nodeIndex);

	cweeList<::epanet::Plink> getConnectedLinks(int const& projectNum, int const& nodeIndex);

	void addDemand(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& baseDemand, cweeStr const& demandPattern, cweeStr const& demandName);
	void deleteDemand(int const& projectNum, int const& nodeIndex, int const& demandIndex);
	void setBaseDemand(int const& projectNum, int const& nodeIndex, int const& demandIndex, units::dimensionless::scalar_t const& baseDemand);
	void setDemandPattern(int const& projectNum, int const& nodeIndex, int const& demandIndex, int const& patIndex);
	void setDemandName(int const& projectNum, int const& nodeIndex, int const& demandIdx, cweeStr const& demandName);
	int  getDemandIndex(int const& projectNum, int const& nodeIndex, cweeStr const& demandName);
	int  getNumDemands(int const& projectNum, int const& nodeIndex);
	float getBaseDemand(int const& projectNum, int const& nodeIndex, int const& demandIndex);
	int  getDemandPattern(int const& projectNum, int const& nodeIndex, int const& demandIndex);
	cweeStr getDemandName(int const& projectNum, int const& nodeIndex, int const& demandIndex);

	void ParseNetwork(int const& projectNum);

	int addLink(int const& projectNum, cweeStr const& id, int const& linkType, cweeStr const& fromNode, cweeStr const& toNode);
	void deleteLink(int const& projectNum, int const& index, int const& actionCode);
	void setLinkValue(int const& projectNum, int const& index, int const& property, units::dimensionless::scalar_t const& value);
	void setPipeData(int const& projectNum, int const& index, units::dimensionless::scalar_t const& length, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& rough, units::dimensionless::scalar_t const& mloss);
	void setLinkID(int const& projectNum, int const& index, cweeStr const& newid);
	void setLinkType(int const& projectNum, int* inout_index, int const& linkType, int const& actionCode);
	void setLinkNodes(int const& projectNum, int const& index, int const& node1, int const& node2);
	int getlinkUpstreamNode(int const& projectNum, int const& index);
	int getlinkDownstreamNode(int const& projectNum, int const& index);
	/*!
		  EN_DIAMETER     = 0,  //!< Pipe/valve diameter
		  EN_LENGTH       = 1,  //!< Pipe length
		  EN_ROUGHNESS    = 2,  //!< Pipe roughness coefficient
		  EN_MINORLOSS    = 3,  //!< Pipe/valve minor loss coefficient
		  EN_INITSTATUS   = 4,  //!< Initial status (see @ref EN_LinkStatusType)
		  EN_INITSETTING  = 5,  //!< Initial pump speed or valve setting
		  EN_KBULK        = 6,  //!< Bulk chemical reaction coefficient
		  EN_KWALL        = 7,  //!< Pipe wall chemical reaction coefficient
		  EN_FLOW         = 8,  //!< Current computed flow rate (read only)
		  EN_VELOCITY     = 9,  //!< Current computed flow velocity (read only)
		  EN_HEADLOSS     = 10, //!< Current computed head loss (read only)
		  EN_STATUS       = 11, //!< Current link status (see @ref EN_LinkStatusType)
		  EN_SETTING      = 12, //!< Current link setting
		  EN_ENERGY       = 13, //!< Current computed pump energy usage (read only)
		  EN_LINKQUAL     = 14, //!< Current computed link quality (read only)
		  EN_LINKPATTERN  = 15, //!< Pump speed time pattern index
		  EN_PUMP_STATE   = 16, //!< Current computed pump state (read only) (see @ref EN_PumpStateType)
		  EN_PUMP_EFFIC   = 17, //!< Current computed pump efficiency (read only)
		  EN_PUMP_POWER   = 18, //!< Pump constant power rating
		  EN_PUMP_HCURVE  = 19, //!< Pump head v. flow curve index
		  EN_PUMP_ECURVE  = 20, //!< Pump efficiency v. flow curve index
		  EN_PUMP_ECOST   = 21, //!< Pump average energy price
		  EN_PUMP_EPAT    = 22  //!< Pump energy price time pattern index
		*/
	float getLinkValue(int const& projectNum, int const& index, int const& property);
	int getlinkIndex(int const& projectNum, cweeStr const& id);
	cweeStr getlinkID(int const& projectNum, int const& index);
	int getLinkType(int const& projectNum, int const& index);

	int getPumpType(int const& projectNum, int const& linkIndex);
	int getHeadCurveIndex(int const& projectNum, int const& linkIndex);
	void setHeadCurveIndex(int const& projectNum, int const& linkIndex, int const& curveIndex);

	void	addPattern(int const& projectNum, cweeStr const& id);
	void	deletePattern(int const& projectNum, int const& index);
	void	setPatternID(int const& projectNum, int const& index, cweeStr const& id);
	void	setPatternValue(int const& projectNum, int const& index, int const& period, units::dimensionless::scalar_t const& value);
	void	setPattern(int const& projectNum, int const& index, units::dimensionless::scalar_t* values, int const& len);
	int		getPatternIndex(int const& projectNum, cweeStr const& id);
	cweeStr getPatternID(int const& projectNum, int const& index);
	int		getPatternLen(int const& projectNum, int const& index);
	float getPatternValue(int const& projectNum, int const& index, int const& period);
	float	getAveragePatternValue(int const& projectNum, int const& index);
	int getNumPatterns(int const& projectNum);

	cweeThreadedList<float> getPattern(int const& projectNum, int const& index);

	void setPattern(int const& projectNum, int const& index, const cweeThreadedList<float>& data);

	void setPattern(int const& projectNum, int const& index, const cweeThreadedList<std::pair<u64, float>>& data);

	void setPattern(int const& projectNum, cweeStr const& id, const cweeThreadedList<float>& data);
	void setPattern(int const& projectNum, cweeStr const& id, const cweeThreadedList<std::pair<u64, float>>& data);

	void	addCurve(int const& projectNum, cweeStr const& id);
	void	deleteCurve(int const& projectNum, int const& index);
	void	setCurve(int const& projectNum, int const& index, units::dimensionless::scalar_t* xValues, units::dimensionless::scalar_t* yValues, int const& nPoints);
	void	setCurveID(int const& projectNum, int const& index, cweeStr const& id);
	void	setCurveValue(int const& projectNum, int const& curveIndex, int const& pointIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y);
	int		getCurveIndex(int const& projectNum, cweeStr const& id);
	cweeStr getCurveID(int const& projectNum, int const& index);
	std::pair<float, float> getCurveValue(int const& projectNum, int const& index, int const& period);
	int		getCurveLen(int const& projectNum, int const& index);
	int		getCurveType(int const& projectNum, int const& index);
	int getNumCurves(int const& projectNum);
	cweeThreadedList<std::pair<float, float>> getCurve(int const& projectNum, int const& index);
	int InsertPattern(int projectNum, Pattern* pat, const cweeStr& name, u64 start, u64 end, u64 stepSeconds);

	EPAnet_Local() : projectMap(), projectError(), projectTime(), projectTimestep(), numProjects(0), DefaultStr{0} {};
private:
	cweeThreadedMap<int, ::epanet::EN_Project> projectMap;
	// cweeInterlocked<std::map<int, epanet::EN_Project>> projectMap;
	cweeInterlocked < std::map<int, int>> projectError;
	cweeInterlocked < std::map<int, units::time::second_t>> projectTime;
	cweeInterlocked < std::map<int, units::time::second_t>> projectTimestep;
	cweeSysInterlockedInteger numProjects;
	char DefaultStr[epanet::MAXFNAME];

	static cweeStr createTimeFromMinutes(float minutes);
	bool findProject(int const& projectNum, epanet::EN_Project* out);
	bool findError(int const& projectNum, int* out);
	void setError(int const& projectNum, int err);
	bool findTime(int const& projectNum, units::time::second_t* out);
	bool findTimeStep(int const& projectNum, units::time::second_t* out);
};
extern cweeSharedPtr< EPAnet_Local > EPAnetLocal;

/* Individual EPAnet project. Allows for use of underlying member functions without carrying around the "project number". */
class EPAnetProject {
public:
	class EPAnetHydraulicSimulation {
	public:
		EPAnetHydraulicSimulation(EPAnetProject& p);
		~EPAnetHydraulicSimulation();
		void SetTimestep(float minutes);
		units::time::second_t GetCurrentSimTime();
		void DoSteadyState(::epanet::HydraulicSimulationQuality simQuality = ::epanet::HydraulicSimulationQuality::HIGHRES);
		bool ShouldContinueSimulation();
		float GetSimulationValue(cweeStr const& name, value_t const& value_type, bool isNode);

	private:
		EPAnetProject& parent;
	};
	cweeSharedPtr<EPAnetHydraulicSimulation> StartHydraulicSimulation();

	EPAnetProject();
	~EPAnetProject();

	::epanet::EN_Network GetNetwork();

	void initBuild(cweeStr const& rptFile, cweeStr const& outFile, int const& unitsType, int const& headLossType);

	void loadINP(const char* filePathToInpFile, const char* filePathToRptFile = "");
	void closeINP();
	void saveINP(cweeStr const& filePathToInpFile);
	cweeStr getTitle();

	cweeStr getDescription(bool const& isNode, int const& epanetIndex);
	void setDescription(bool const& isNode, int const& epanetIndex, cweeStr const& description);
	int	getCount(int const& objectType);

	void setFlowUnits(int const& units);
	void setQualType(int const& qualType, cweeStr const& chemName, cweeStr const& chemUnits, cweeStr const& traceNode);
	void setTimeParam(int const& param, units::time::second_t const& value);
	void setOption(int const& option, units::dimensionless::scalar_t const& value);
	float getOption(int const& option);
	units::time::second_t getTimeParam(int const& param);
	int	 getFlowUnits();

	units::time::second_t getCurrentSimTime();
	units::time::second_t getCurrentSimTimestep();

	int	 getCurrentError();

	void fullHydraulicSim();
	void startHydraulicSim();
	void calcHydraulicsAtTime(::epanet::HydraulicSimulationQuality simQuality = ::epanet::HydraulicSimulationQuality::HIGHRES);
	void calcHydraulicTimestepToNextTime();
	void resetHydraulicSim();
	void endHydraulicSim();

	void fullQualitySim();
	void startQualitySim();
	void calcQualityAtTime();
	void calcQualityTimestepToNextTime();
	void resetQualitySim();
	void endQualitySim();

	int	 addNode(cweeStr const& name, int const& nodeType);
	void deleteNode(int const& nodeIndex, int const& actionCode);
	void setNodeID(int const& nodeIndex, cweeStr const& newid);
	void setNodeValue(int const& nodeIndex, int const& property, units::dimensionless::scalar_t const& value);
	void setJuncData(int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& dmnd, cweeStr const& dmndpat);
	void setTankData(int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& initlvl, units::dimensionless::scalar_t const& minlvl, units::dimensionless::scalar_t const& maxlvl, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& minvol, cweeStr const& volcurve);
	void setCoord(int const& nodeIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y);
	int	 getNodeindex(cweeStr const& id);
	cweeStr getNodeid(int const& nodeIndex);
	int  getNodetype(int const& nodeIndex);
	float getNodevalue(int const& nodeIndex, int const& property);
	float getCoordX(int const& nodeIndex);
	float getCoordY(int const& nodeIndex);
	cweeList<::epanet::Plink> getConnectedLinks(int const& nodeIndex);

	void addDemand(int const& nodeIndex, units::dimensionless::scalar_t const& baseDemand, cweeStr const& demandPattern, cweeStr const& demandName);
	void deleteDemand(int const& nodeIndex, int const& demandIndex);
	void setBaseDemand(int const& nodeIndex, int const& demandIndex, units::dimensionless::scalar_t const& baseDemand);
	void setDemandPattern(int const& nodeIndex, int const& demandIndex, int const& patIndex);
	void setDemandName(int const& nodeIndex, int const& demandIdx, cweeStr const& demandName);
	int  getDemandIndex(int const& nodeIndex, cweeStr const& demandName);
	int  getNumDemands(int const& nodeIndex);
	float getBaseDemand(int const& nodeIndex, int const& demandIndex);
	int  getDemandPattern(int const& nodeIndex, int const& demandIndex);
	cweeStr getDemandName(int const& nodeIndex, int const& demandIndex);

	void ParseNetwork();
	bool RemoveDeadEnds();
	bool CombineBasicPipesInSeries();
	bool CollapseZone(::epanet::Pzone const& zone);

	int addLink(cweeStr const& id, int const& linkType, cweeStr const& fromNode, cweeStr const& toNode);
	void deleteLink(int const& index, int const& actionCode);
	void setLinkValue(int const& index, int const& property, units::dimensionless::scalar_t const& value);
	void setPipeData(int const& index, units::dimensionless::scalar_t const& length, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& rough, units::dimensionless::scalar_t const& mloss);
	void setLinkID(int const& index, cweeStr const& newid);
	void setLinkType(int* inout_index, int const& linkType, int const& actionCode);
	void setLinkNodes(int const& index, int const& node1, int const& node2);
	int getlinkUpstreamNode(int const& index);
	int getlinkDownstreamNode(int const& index);
	float getLinkValue(int const& index, int const& property);
	int getlinkIndex(cweeStr const& id);
	cweeStr getlinkID(int const& index);
	int getLinkType(int const& index);

	int getPumpType(int const& linkIndex);
	int getHeadCurveIndex(int const& linkIndex);
	void setHeadCurveIndex(int const& linkIndex, int const& curveIndex);

	void addPattern(cweeStr const& id);
	void deletePattern(int const& index);
	void setPatternID(int const& index, cweeStr const& id);
	void setPatternValue(int const& index, int const& period, units::dimensionless::scalar_t const& value);
	void setPattern(int const& index, units::dimensionless::scalar_t* values, int const& len);
	int getPatternIndex(cweeStr const& id);
	cweeStr getPatternID(int const& index);
	int getPatternLen(int const& index);
	float getPatternValue(int const& index, int const& period);
	float getAveragePatternValue(int const& index);
	int getNumPatterns();
	cweeThreadedList<float> getPattern(int const& index);
	void setPattern(int const& index, const cweeThreadedList<float>& data);
	void setPattern(int const& index, const cweeThreadedList<std::pair<u64, float>>& data);
	void setPattern(cweeStr const& id, const cweeThreadedList<float>& data);
	void setPattern(cweeStr const& id, const cweeThreadedList<std::pair<u64, float>>& data);

	void addCurve(cweeStr const& id);
	void deleteCurve(int const& index);
	void setCurve(int const& index, units::dimensionless::scalar_t* xValues, units::dimensionless::scalar_t* yValues, int const& nPoints);
	void setCurveID(int const& index, cweeStr const& id);
	void setCurveValue(int const& curveIndex, int const& pointIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y);
	int getCurveIndex(cweeStr const& id);
	cweeStr getCurveID(int const& index);
	std::pair<float, float> getCurveValue(int const& index, int const& period);
	int getCurveLen(int const& index);
	int getCurveType(int const& index);
	int getNumCurves();
	cweeThreadedList<std::pair<float, float>> getCurve(int const& index);

	int getNumControls();
	void deleteControl(int const& controlIndex);
	void deleteAllControls();
	int addControl(int const& controlType, cweeStr const& linkName, float const& setting, cweeStr const& nodeName, float const& level);
	int addControl(cweeStr const& entireControl);
	void getControl(int const& controlindex, int* type, int* linkIndex, units::dimensionless::scalar_t* setting, int* nodeIndex, units::dimensionless::scalar_t* level);
	cweeStr getControl(int const& controlindex);

	int getNumRules();
	int findRuleIndex(cweeStr const& ruleName);
	cweeStr getRuleID(int const& ruleIndex);
	int addRule(cweeStr const& name, cweeStr const& linkIndex, cweeStr const& linkValue, cweeStr const& ifTrueThenStatusOrSetting, cweeStr const& refObjectType, cweeStr const& refObjectIndex, cweeStr const& refObjectVariable, cweeStr const& ComparisonOperator, cweeStr const& refObjectStatusOrValue, units::dimensionless::scalar_t const& priority);
	int addRule(cweeStr const& ruleName, cweeStr entireRule, bool const& returnIndex = true);
	void deleteRule(int const& ruleIndex);
	void deleteAllRules();
	void getRuleSizes(int const& ruleIndex, int* nPremises = nullptr, int* nThenActions = nullptr, int* nElseActions = nullptr, units::dimensionless::scalar_t* priority = nullptr);
	void setRule_Premise(int const& ruleIndex, int const& premiseIndex, int const& premiseType, int const& objectType, int const& objectIndex, int const& objectVariable, int const& comparisonOp, int const& comparisonStatus, float const& comparisonValue);
	void setRule_ElseAction(int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting);
	void setRule_Priority(int const& index, units::dimensionless::scalar_t const& priority);
	void setRule_ThenAction(int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting);
	cweeStr getRule(int const& ruleIndex);
	cweeThreadedList<cweeStr> getRules();

	int getEPAnetIndexOfAsset(int projectNum, const cweeStr& name, bool isNode);
	static bool isNode(asset_t const& type);

	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& EPAnetIndex, bool const& isNode, int const& prop);
	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& EPAnetIndex, value_t const& value);
	units::dimensionless::scalar_t getCurrentHydraulicValue(const cweeStr& name, const value_t& value, bool isNode = true);
	cweeThreadedList<cweeStr> getImpactedAssetsFromRule(int ruleIndex);
	cweeThreadedList<cweeStr> getImpactedAssetsFromRules();
	cweeThreadedList<cweeStr> getImpactedAssetsFromControls();

	int InsertPattern(Pattern* pat, const cweeStr& name, u64 start, u64 end, u64 stepSeconds);

	epanet::EN_Project epanetProj;

private:
	int proj;
};

class EPAnet_Shared {
public:
	cweeSharedPtr<EPAnetProject>  createNewProject(void);
};
/* Manager for all simultaneous EPAnet projects. Allows creation of new projects. */
extern cweeSharedPtr< EPAnet_Shared > EPAnet; 
