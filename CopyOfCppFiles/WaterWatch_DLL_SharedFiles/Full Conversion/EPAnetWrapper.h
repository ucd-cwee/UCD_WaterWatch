#pragma once
#include "Precompiled.h"

#pragma warning(disable : 4996)
#include "EPANET_Precompiled.h"
#include "EPANET_Compiled.h"
#include "EPANET_Postcompiled.h"
#include "EPANET_CPP.h"

// the 'epanet' and 'epanet::epanet' namespaces have everything needed for epanet processing
class EPAnet_Local {
public:
	int  createNewProject(void) {
		int projectNum = numProjects.Increment();

		projectMap.GetExclusive()->operator[](projectNum) = epanet::epanet::EN_createproject();
		projectTime.GetExclusive()->operator[](projectNum) = 0;
		projectTimestep.GetExclusive()->operator[](projectNum) = 0;
		projectError.GetExclusive()->operator[](projectNum) = 0;

		return projectNum;
	};
	void deleteProject(int const& projectNum) {
		epanet::EN_Project toDelete;
		if (findProject(projectNum, &toDelete) == 1) {
			epanet::epanet::EN_clearproject(toDelete);
			projectMap.GetExclusive()->erase(projectNum);
			projectTime.GetExclusive()->erase(projectNum);
			projectTimestep.GetExclusive()->erase(projectNum);
			projectError.GetExclusive()->erase(projectNum);
		}
	};
	void initBuild(int const& projectNum, cweeStr const& rptFile, cweeStr const& outFile, int const& unitsType, int const& headLossType) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_init(toModify, (char*)rptFile.c_str(), (char*)outFile.c_str(), unitsType, headLossType);
		}
	};

	void loadINP(int const& projectNum, const char* filePathToInpFile, const char* filePathToRptFile = "") {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_open(toModify, filePathToInpFile, filePathToRptFile, "");
		}
	};
	void closeINP(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {

			epanet::epanet::EN_close(toModify);
		}
	};
	void saveINP(int const& projectNum, cweeStr const& filePathToInpFile) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_saveinpfile(toModify, filePathToInpFile.c_str());
		}
	};
	cweeStr getTitle(int const& projectNum) {
		// glitched and does not work. 
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {

			cweeStr outName1 = DefaultStr;
			cweeStr outName2 = DefaultStr;
			cweeStr outName3 = DefaultStr;
			char* out1 = (char*)outName1.c_str();
			char* out2 = (char*)outName2.c_str();
			char* out3 = (char*)outName3.c_str();
			epanet::epanet::EN_gettitle(toModify, out1, out2, out3);
			cweeStr out = cweeStr(cweeStr(cweeStr(out1) + cweeStr(out2)) + cweeStr(out3));
			out.ReplaceChar('\n', ' ');
			out.ReduceSpaces();
			return out;
		}
		return "No Name";
	};

	cweeStr getDescription(int const& projectNum, bool const& isNode, int const& epanetIndex) {

		epanet::EN_Project toModify;
		cweeStr outInit = DefaultStr;
		char* out = (char*)outInit.c_str();
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getcomment(toModify, isNode ? 0 : 1, epanetIndex, out) > 100) return "";
		}
		return cweeStr(out);
	};
	void setDescription(int const& projectNum, bool const& isNode, int const& epanetIndex, cweeStr const& description) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setcomment(toModify, isNode ? 0 : 1, epanetIndex, (char*)description.c_str());
		}
	};
	int	getCount(int const& projectNum, int const& objectType) {
		epanet::EN_Project toModify;
		int out = 0;
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getcount(toModify, objectType, &out) > 100) return -1;
		}
		return out;
	};

	void setFlowUnits(int const& projectNum, int const& units) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setflowunits(toModify, units);
		}
	};
	/*!
	EN_NONE = 0 // No quality analysis.
	EN_CHEM = 1 // Chemical fate and transport.
	EN_AGE = 2 // Water age analysis.
	EN_TRACE = 3 // Source tracing analysis.
	*/
	void setQualType(int const& projectNum, int const& qualType, cweeStr const& chemName, cweeStr const& chemUnits, cweeStr const& traceNode) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setqualtype(toModify, qualType, (char*)chemName.c_str(), (char*)chemUnits.c_str(), (char*)traceNode.c_str());
		}
	};
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
	void setTimeParam(int const& projectNum, int const& param, long const& value) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_settimeparam(toModify, param, value);
		}
	};
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
	void setOption(int const& projectNum, int const& option, units::dimensionless::scalar_t const& value) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setoption(toModify, option, value);
		}
	};
	float getOption(int const& projectNum, int const& option) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getoption(toModify, option, &out);
		}
		return out;
	};
	float getTimeParam(int const& projectNum, int const& param) {
		epanet::EN_Project toModify;
		long out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_gettimeparam(toModify, param, &out);
		}
		return out;
	};
	int	 getFlowUnits(int const& projectNum) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getflowunits(toModify, &out);
		}
		return out;
	};

	// returns in minutes. 
	long getCurrentSimTime(int const& projectNum) {
		long toModify(0);
		if (findTime(projectNum, &toModify) == 1)
			return toModify;
		return -1;
	};
	// returns in minutes. 
	long getCurrentSimTimestep(int const& projectNum) {
		long toModify;
		if (findTimeStep(projectNum, &toModify) == 1)
			return toModify;
		return -1;
	};
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
	int	 getCurrentError(int const& projectNum) {
		int toModify;
		if (findError(projectNum, &toModify) == 1)
			return toModify;
		return -1;
	};

	// hydraulic simulation
	void fullHydraulicSim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_solveH(toModify);
		}
	};
	void startHydraulicSim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_openH(toModify);
			projectTime.GetExclusive()->operator[](projectNum) = 0;
			projectTimestep.GetExclusive()->operator[](projectNum) = 0;
		}
	};
	void calcHydraulicsAtTime(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			long currentTime(-1); long currentTimeStep(-1);
			currentTime = getCurrentSimTime(projectNum);
			currentTimeStep = getCurrentSimTimestep(projectNum);
			if (currentTime == 0 && currentTimeStep == 0)
				epanet::epanet::EN_initH(toModify, 0);
			epanet::epanet::EN_runH(toModify, &currentTime);
			projectTime.GetExclusive()->operator[](projectNum) = currentTime;
		}
	};
	void calcHydraulicTimestepToNextTime(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			long currentTimeStep;
			findTimeStep(projectNum, &currentTimeStep);
			epanet::epanet::EN_nextH(toModify, &currentTimeStep);
			projectTimestep.GetExclusive()->operator[](projectNum) = currentTimeStep;
		}
	};
	void resetHydraulicSim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			projectTime.GetExclusive()->operator[](projectNum) = 0;
			projectTimestep.GetExclusive()->operator[](projectNum) = 0;
		}
	};
	void endHydraulicSim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			long currentTime(0);
			long currentTimeStep(0);
			findTime(projectNum, &currentTime);
			findTimeStep(projectNum, &currentTimeStep);
			epanet::epanet::EN_closeH(toModify);
			projectTime.GetExclusive()->operator[](projectNum) = currentTime;
			projectTimestep.GetExclusive()->operator[](projectNum) = currentTimeStep;
		}
	};

	// quality simulation
	void fullQualitySim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_solveQ(toModify);
		}
	};
	void startQualitySim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_openQ(toModify);
			projectTime.GetExclusive()->operator[](projectNum) = 0;
			projectTimestep.GetExclusive()->operator[](projectNum) = 0;
		}
	};
	void calcQualityAtTime(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			long currentTime(-1); long currentTimeStep(-1);
			currentTime = getCurrentSimTime(projectNum);
			currentTimeStep = getCurrentSimTimestep(projectNum);
			if (currentTime == 0 && currentTimeStep == 0)
				epanet::epanet::EN_initQ(toModify, 0);
			epanet::epanet::EN_runQ(toModify, &currentTime);
			projectTime.GetExclusive()->operator[](projectNum) = currentTime;
		}
	};
	void calcQualityTimestepToNextTime(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			long currentTimeStep;
			findTimeStep(projectNum, &currentTimeStep);
			epanet::epanet::EN_nextQ(toModify, &currentTimeStep);
			projectTimestep.GetExclusive()->operator[](projectNum) = currentTimeStep;
		}
	};
	void resetQualitySim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			projectTime.GetExclusive()->operator[](projectNum) = 0;
			projectTimestep.GetExclusive()->operator[](projectNum) = 0;
		}
	};
	void endQualitySim(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			long currentTime(0);
			long currentTimeStep(0);
			findTime(projectNum, &currentTime);
			findTimeStep(projectNum, &currentTimeStep);
			epanet::epanet::EN_closeQ(toModify);
			projectTime.GetExclusive()->operator[](projectNum) = currentTime;
			projectTimestep.GetExclusive()->operator[](projectNum) = currentTimeStep;
		}
	};

	int getNumControls(int const& projectNum) {
#define maxControls 100000
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			int index = 0;
			do {
				index++;
				int type(-1); int linkIndex(-1); units::dimensionless::scalar_t setting(-1); int nodeIndex(-1); units::dimensionless::scalar_t level(-1);
				epanet::epanet::EN_getcontrol(toModify, index, &type, &linkIndex, &setting, &nodeIndex, &level);
				if (setting == (units::dimensionless::scalar_t)-1 && level == (units::dimensionless::scalar_t)-1)
					return index - 1;
			} while (index < maxControls);
		}
		return -1;
#undef maxControls
	};

	void deleteControl(int const& projectNum, int const& controlIndex) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_deletecontrol(toModify, controlIndex);
		}
	};

	void deleteAllControls(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			int numControls = this->getNumControls(projectNum);
			for (int i = (numControls + 1); i-- > 0; ) {
				this->deleteControl(projectNum, i);
			}
		}
	};

	/*!
		int controlType,	0 = EN_LOWLEVEL, 1 = EN_HILEVEL, 2 = EN_TIMER, 3 = EN_TIMEOFDAY
		cweeStr linkName,	Name of link to be controlled
		float setting,		Value of link if true
		cweeStr nodeName,	Node to look at
		float level,		Node setting to compare against */
	int addControl(int const& projectNum, int const& controlType, cweeStr const& linkName, float const& setting, cweeStr const& nodeName, float const& level) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			int type(cweeMath::max(0, cweeMath::min(controlType, 3))); int linkIndex(-1); units::dimensionless::scalar_t Setting((units::dimensionless::scalar_t)setting); int nodeIndex(-1); units::dimensionless::scalar_t Level((units::dimensionless::scalar_t)level); int index(-1);
			linkIndex = getEPAnetIndexOfAsset(projectNum, linkName, false);
			nodeIndex = getEPAnetIndexOfAsset(projectNum, nodeName, true);
			if (type > 1) nodeIndex = 0;

			if (linkIndex != -1 && nodeIndex != -1)	epanet::epanet::EN_addcontrol(toModify, type, linkIndex, Setting, nodeIndex, Level, &index);
			return index;
		}
		return -1;
	};

	/*!
		easy way to add a control if you already typed the whole thing out. Returns an index to the control.
		*/
	int addControl(int const& projectNum, cweeStr const& entireControl) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			cweeStr control = entireControl;
			int controlType(-1);
			cweeStr linkName("");
			float setting(-1);
			cweeStr nodeName("");
			float level(-1);

			control.ReplaceChar('\t', ' ');
			control.ReduceSpaces();

			cweeParser words(control);
			if (words.getNumVars() > 4) {

				// CRITICAL NOTE. EPAnet 2.2 source code has an error. It cannot parse the following code through API: 
				// "LINK VALVE_FCV OPEN/CLOSED If Node ANY_NODE ABOVE/BELOW/TIME/CLOCKTIME etc.
				// It forces FCV type links to be "active" at all times - not OPEN, not CLOSED, only active. This will result in unnecessary warnings and bad simulations. 
				// It can, however, parse SETTING code for FCVs. It simply can't parse STATUS code for FCVs.
				// To get around this: 
				// When dealing with a valve STATUS, use rules instead of controls. May be worth using rules for all FCVs if dynamic changes are required (which is obviously the point here)

				bool findClockTime = (control.Find("clocktime", false) != -1);
				bool findTime = (control.Find("time", false) != -1);
				bool findAbove = (control.Find("above", false) != -1);
				bool findBelow = (control.Find("below", false) != -1);
				bool findPM = (control.Find(" PM ", false) != -1);
				if (!findPM) findPM = (control.Find(" PM", false) != -1);
				if (!findPM) findPM = (control.Find(" PM\n", false) != -1);
				if (!findPM) findPM = (control.Find(" PM;", false) != -1);

				setting = (float)words.getVar(2);

				if (!words.getVar(2).Icmp("closed")) setting = 0;
				if (!words.getVar(2).Icmp("open")) setting = 1;

				if (findClockTime) {
					cweeParser split(words[5], ":", true);
					int minuteTime;
					int hours = (int)split[0];
					int minutes = (int)split[1];
					if (findPM) hours += 12;
					minuteTime = minutes + hours * 60;
					return addControl(projectNum, 3, words.getVar(1), setting, "", minuteTime * 60);
				}
				else if (findTime) {
					if (words.getVar(5).Find(':') == -1)
						return addControl(projectNum, 2, words.getVar(1), setting, "", (float)words.getVar(5));
					else
					{
						cweeParser split(words[5], ":", true);
						int minuteTime;
						int hours = (int)split[0];
						int minutes = (int)split[1];
						if (findPM) hours += 12;
						minuteTime = minutes + hours * 60;
						return addControl(projectNum, 2, words.getVar(1), setting, "", minuteTime * 60);
					}
				}
				else if (findAbove) {
					return addControl(projectNum, 1, words.getVar(1), setting, words.getVar(5), (float)words.getVar(7));
				}
				else if (findBelow) {
					return addControl(projectNum, 0, words.getVar(1), setting, words.getVar(5), (float)words.getVar(7));
				}
			}
		}
		return -1;
	};

	void getControl(int const& projectNum, int const& controlindex, int* type, int* linkIndex, units::dimensionless::scalar_t* setting, int* nodeIndex, units::dimensionless::scalar_t* level) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getcontrol(toModify, controlindex, type, linkIndex, setting, nodeIndex, level) > 100) *type = -1;
		}
		return;
	};

	cweeStr getControl(int const& projectNum, int const& controlindex) {
		epanet::EN_Project toModify;
		cweeStr out;
		if (findProject(projectNum, &toModify) == 1) {
			int type(0);
			int linkIndex(0);
			units::dimensionless::scalar_t setting(0);
			int nodeIndex(0);
			units::dimensionless::scalar_t level(0);

			getControl(projectNum, controlindex, &type, &linkIndex, &setting, &nodeIndex, &level);

			if (type == -1) return "";

			cweeStr link_name = getlinkID(projectNum, linkIndex);
			cweeStr link_setting = cweeStr((float)setting);
			cweeStr node_name = getNodeid(projectNum, nodeIndex);
			cweeStr control_qualifier;
			cweeStr node_level = cweeStr((float)level);

			switch (type) {
			case 0: { // 0 = EN_LOWLEVEL
				control_qualifier = "BELOW";
				out = cweeStr::printf("LINK %s %s IF NODE %s %s %s", link_name.c_str(), link_setting.c_str(), node_name.c_str(), control_qualifier.c_str(), node_level.c_str());
				break;
			}
			case 1: { // 1 = EN_HILEVEL
				control_qualifier = "ABOVE";
				out = cweeStr::printf("LINK %s %s IF NODE %s %s %s", link_name.c_str(), link_setting.c_str(), node_name.c_str(), control_qualifier.c_str(), node_level.c_str());
				break;
			}
			case 2: { // 2 = EN_TIMER
				out = cweeStr::printf("LINK %s %s AT TIME %s", link_name.c_str(), link_setting.c_str(), createTimeFromMinutes((float)node_level / 60).c_str());
				break;
			}
			case 3: { // 3 = EN_TIMEOFDAY
				out = cweeStr::printf("LINK %s %s AT CLOCKTIME %s", link_name.c_str(), link_setting.c_str(), createTimeFromMinutes((float)node_level / 60).c_str());
				break;
			}
			}
		}
		return out;
	};

	int getNumRules(int const& projectNum) {
#define maxRules 100000
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			int ruleIndex = 0;
			cweeStr outName = DefaultStr;
			char* out = (char*)(const char*)outName;
			do {
				ruleIndex++;
				epanet::epanet::EN_getruleID(toModify, ruleIndex, out);
				outName = cweeStr(out);
				if (outName.IsEmpty())
					return ruleIndex - 1;
			} while (ruleIndex < maxRules);
		}
		return -1;
#undef maxRules
	};

	int findRuleIndex(int const& projectNum, cweeStr const& ruleName) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			int numRules = getNumRules(projectNum);
			for (int i = 1; i <= numRules; i++) {
				cweeStr outName = getRuleID(projectNum, i);
				if (outName == ruleName) return i;
			}
		}
		return -1;
	};

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
	int addRule(int const& projectNum, cweeStr const& name, cweeStr const& linkIndex, cweeStr const& linkValue,
		cweeStr const& ifTrueThenStatusOrSetting, cweeStr const& refObjectType, cweeStr const& refObjectIndex, cweeStr const& refObjectVariable, cweeStr const& ComparisonOperator,
		cweeStr const& refObjectStatusOrValue, units::dimensionless::scalar_t const& priority) {

		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			// if the rule already exists, do nothing. 
			if (findRuleIndex(projectNum, name) == -1) {
				cweeStr line1; cweeStr line2; cweeStr line3; cweeStr line4;
				line1 = "RULE " + name;
				line2 = cweeStr::printf("IF %s %s %s %s %s", refObjectType.c_str(), refObjectIndex.c_str(), refObjectVariable.c_str(), ComparisonOperator.c_str(), refObjectStatusOrValue.c_str());
				line3 = cweeStr::printf("THEN LINK %s %s IS %s", linkIndex.c_str(), linkValue.c_str(), ifTrueThenStatusOrSetting.c_str());
				line4 = "PRIORITY " + cweeStr((float)priority);

				cweeStr rule = cweeStr::printf("%s\n%s\n%s\n%s\n", line1.c_str(), line2.c_str(), line3.c_str(), line4.c_str()); // REQUIRED INTERFACE IS ONE LONG STRING WITH THE RULE. 
				//td::cout << rule << std::endl;

				char* ruleIn = (char*)(const char*)rule;
				std::strcpy(ruleIn, rule.c_str());
				epanet::epanet::EN_addrule(toModify, ruleIn);
				return findRuleIndex(projectNum, name);
			}
		}
		return -1;
	};

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
	int addRule(int const& projectNum, cweeStr const& ruleName, cweeStr entireRule, bool const& returnIndex) {
		epanet::EN_Project toModify;
		entireRule.Replace("\n", " \n ");
		if (findProject(projectNum, &toModify) == 1) {
			//if (findRuleIndex(projectNum, ruleName) == -1) {
			char* ruleIn = (char*)(const char*)entireRule;
			std::strcpy(ruleIn, entireRule.c_str());
			int err = epanet::epanet::EN_addrule(toModify, ruleIn);
			if (returnIndex) return findRuleIndex(projectNum, ruleName);
			//}
		}
		return -1;
	};

	void deleteRule(int const& projectNum, int const& ruleIndex) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_deleterule(toModify, ruleIndex);
		}
	};

	void deleteAllRules(int const& projectNum) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			int numRules = this->getNumRules(projectNum);
			for (int i = (numRules + 1); i-- > 0; ) {
				this->deleteRule(projectNum, i);
			}
		}
	};

	void getRuleSizes(int const& projectNum, int const& ruleIndex, int* nPremises, int* nThenActions, int* nElseActions, units::dimensionless::scalar_t* priority) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			int SnPremises(-1);
			int SnThenActions(-1);
			int SnElseActions(-1);
			units::dimensionless::scalar_t Spriority(-1);
			bool fail = false;
			if (epanet::epanet::EN_getrule(toModify, ruleIndex, &SnPremises, &SnThenActions, &SnElseActions, &Spriority) > 100) fail = true;
			if (nPremises != nullptr) *nPremises = SnPremises;
			if (nThenActions != nullptr) *nThenActions = SnThenActions;
			if (nElseActions != nullptr) *nElseActions = SnElseActions;
			if (priority != nullptr) *priority = Spriority;
			if (fail == true) *priority = -1;
		}
	};

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
	void setRule_Premise(int const& projectNum, int const& ruleIndex, int const& premiseIndex, int const& premiseType, int const& objectType, int const& objectIndex, int const& objectVariable, int const& comparisonOp, int const& comparisonStatus, float const& comparisonValue) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {

			epanet::epanet::EN_setpremise(toModify, ruleIndex, premiseIndex, premiseType, objectType, objectIndex, objectVariable, comparisonOp, comparisonStatus, comparisonValue);

		}
	};
	void setRule_ElseAction(int const& projectNum, int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setelseaction(toModify, ruleIndex, actionIndex, linkIndex, status, setting);
		}
	};
	void setRule_Priority(int const& projectNum, int const& index, units::dimensionless::scalar_t const& priority) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setrulepriority(toModify, index, priority);
		}
	};
	void setRule_ThenAction(int const& projectNum, int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setthenaction(toModify, ruleIndex, actionIndex, linkIndex, status, setting);
		}
	};
	cweeStr getRuleID(int const& projectNum, int const& ruleIndex) {
		epanet::EN_Project toModify;
		cweeStr outInit = " this this this this this this this";
		char* out = (char*)outInit.c_str();
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getruleID(toModify, ruleIndex, out) > 100) return "-1";
		}
		return cweeStr(out);
	};

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
	cweeStr getRule(int const& projectNum, int const& ruleIndex) {
		epanet::EN_Project toModify;
		cweeStr out;
		if (findProject(projectNum, &toModify) == 1) {
			int numPrem(0), numThen(0), numElse(0);
			units::dimensionless::scalar_t priority(0);
			getRuleSizes(projectNum, ruleIndex, &numPrem, &numThen, &numElse, &priority);

			cweeStr ruleName = getRuleID(projectNum, ruleIndex);
			cweeStr PREM;
			for (int i = 1; i <= numPrem; i++) {
				int logop(0), object(0), objIndex(0), variable(0), relop(0), status(0);
				units::dimensionless::scalar_t value(0);

				epanet::epanet::EN_getpremise(toModify, ruleIndex, i, &logop, &object, &objIndex, &variable, &relop, &status, &value);
				// logop (IF / AND / OR)
				{
					if (i == 1 || logop <= 1) PREM.AddToDelimiter("IF ", '\n');
					else if (logop == 2) PREM.AddToDelimiter("AND ", '\n');
					else PREM.AddToDelimiter("OR ", '\n');
				}
				// object, objIndex
				{
					switch (object) { // OBJECT IS 6 FOR A TANK?!
					case 6: {
						cweeStr objectName;
						PREM.AddToDelimiter("NODE", ' ');
						objectName = getNodeid(projectNum, objIndex);
						PREM.AddToDelimiter(objectName, ' ');
						break;
					}
					case 7: {
						cweeStr objectName;
						PREM.AddToDelimiter("LINK", ' ');
						objectName = getlinkID(projectNum, objIndex);
						PREM.AddToDelimiter(objectName, ' ');
						break;
					}
					case 8: {
						cweeStr objectName;
						PREM.AddToDelimiter("SYSTEM", ' ');
						break;
					}
					}
				}
				// variable
				{
					switch (variable) {
						// Demand = 0, Head = 1, Grade = 2, Level = 3, Pressure = 4, Flow = 5, Status = 6, Setting = 7, Power = 8, Time = 9, Clocktime = 10, Filltime = 11, Draintime = 12
					case 0: {
						PREM.AddToDelimiter("DEMAND", ' ');
						break;
					}
					case 1: {
						PREM.AddToDelimiter("HEAD", ' ');
						break;
					}
					case 2: {
						PREM.AddToDelimiter("GRADE", ' ');
						break;
					}
					case 3: {
						PREM.AddToDelimiter("LEVEL", ' ');
						break;
					}
					case 4: {
						PREM.AddToDelimiter("PRESSURE", ' ');
						break;
					}
					case 5: {
						PREM.AddToDelimiter("FLOW", ' ');
						break;
					}
					case 6: {
						PREM.AddToDelimiter("STATUS", ' ');
						break;
					}
					case 7: {
						PREM.AddToDelimiter("SETTING", ' ');
						break;
					}
					case 8: {
						PREM.AddToDelimiter("POWER", ' ');
						break;
					}
					case 9: {
						PREM.AddToDelimiter("TIME", ' ');
						break;
					}
					case 10: {
						PREM.AddToDelimiter("CLOCKTIME", ' ');
						break;
					}
					case 11: {
						PREM.AddToDelimiter("FILLTIME", ' ');
						break;
					}
					case 12: {
						PREM.AddToDelimiter("DRAINTIME", ' ');
						break;
					}
					}

				}
				// relop
				{
					// Equal = 0, Not Equal = 1, <= 2, >= 3, < 4, > 5, Is Equal To = 6, Is Not Equal To = 7, Below = 8, Above = 9
					switch (relop) {
					case 0: {
						PREM.AddToDelimiter("==", ' ');
						break;
					}
					case 1: {
						PREM.AddToDelimiter("!=", ' ');
						break;
					}
					case 2: {
						PREM.AddToDelimiter("<=", ' ');
						break;
					}
					case 3: {
						PREM.AddToDelimiter(">=", ' ');
						break;
					}
					case 4: {
						PREM.AddToDelimiter("<", ' ');
						break;
					}
					case 5: {
						PREM.AddToDelimiter(">", ' ');
						break;
					}
					case 6: {
						PREM.AddToDelimiter("Is", ' ');
						break;
					}
					case 7: {
						PREM.AddToDelimiter("Not", ' ');
						break;
					}
					case 8: {
						PREM.AddToDelimiter("Below", ' ');
						break;
					}
					case 9: {
						PREM.AddToDelimiter("Above", ' ');
						break;
					}
					}
				}
				// status and value
				{

					switch (object) {
					case 6: {
						if (value >= 0) PREM.AddToDelimiter((float)value, ' ');
						break;
					}
					case 7: {
						// Open = 1, Closed = 2, Active = 3
						switch (status) {
						case 1: {
							PREM.AddToDelimiter("OPEN", ' ');
							break;
						}
						case 2: {
							PREM.AddToDelimiter("CLOSED", ' ');
							break;
						}
						default: {
							if (value >= 0) PREM.AddToDelimiter((float)value, ' ');
							break;
						}
						}
						break;
					}
					case 8: {
						cweeStr timeComponent = createTimeFromMinutes(value / 60);

						if (value >= 0) PREM.AddToDelimiter(timeComponent, ' ');
						break;
					}
					default: {
						if (value >= 0) PREM.AddToDelimiter((float)value, ' ');
						break;
					}
					}
				}

			}
			cweeStr THEN;
			for (int i = 1; i <= numThen; i++) {
				int linkIndex(0), status(0);
				units::dimensionless::scalar_t setting(0);
				epanet::epanet::EN_getthenaction(toModify, ruleIndex, i, &linkIndex, &status, &setting);
				if (THEN.IsEmpty())
					THEN.AddToDelimiter("THEN ", '\n');
				else
					THEN.AddToDelimiter("AND ", '\n');
				// link name
				{
					THEN.AddToDelimiter("LINK", ' ');
					cweeStr assetName = getlinkID(projectNum, linkIndex);
					THEN.AddToDelimiter(assetName, ' ');
				}
				// status and value
				{
					// Open = 1, Closed = 2, Active = 3
					switch (status) {
					case 1: {
						THEN.AddToDelimiter("STATUS IS OPEN", ' ');
						break;
					}
					case 2: {
						THEN.AddToDelimiter("STATUS IS CLOSED", ' ');
						break;
					}
					default: {
						THEN.AddToDelimiter(cweeStr("SETTING IS " + cweeStr((float)setting)), ' ');
						break;
					}
					}
				}

			}
			cweeStr ELSE;
			for (int i = 1; i <= numElse; i++) {
				int linkIndex(0), status(0);
				units::dimensionless::scalar_t setting(0);
				epanet::epanet::EN_getelseaction(toModify, ruleIndex, i, &linkIndex, &status, &setting);
				if (ELSE.IsEmpty())
					ELSE.AddToDelimiter("ELSE ", '\n');
				else
					ELSE.AddToDelimiter("AND ", '\n');
				// link name
				{
					ELSE.AddToDelimiter("LINK", ' ');
					cweeStr assetName = getlinkID(projectNum, linkIndex);
					ELSE.AddToDelimiter(assetName, ' ');
				}
				// status and value
				{
					// Open = 1, Closed = 2, Active = 3
					switch (status) {
					case 1: {
						ELSE.AddToDelimiter("STATUS IS OPEN", ' ');
						break;
					}
					case 2: {
						ELSE.AddToDelimiter("STATUS IS CLOSED", ' ');
						break;
					}
					default: {
						ELSE.AddToDelimiter(cweeStr("SETTING IS " + cweeStr((float)setting)), ' ');
						break;
					}
					}
				}

			}

			out = cweeStr::printf("RULE %s\n%s\n%s", ruleName.c_str(), PREM.c_str(), THEN.c_str());
			if (!ELSE.IsEmpty()) out.AddToDelimiter(ELSE, "\n");
			out.AddToDelimiter(cweeStr("PRIORITY " + cweeStr((float)priority)), "\n");
		}
		return out;
	};

	/*!
		Each "AND" && "ELSE" statement becomes its own, seperate rule.
		This is designed to simplify "assignment" to who owns / manages the rule in the future (instead of rules just floating about)
		*/
	cweeThreadedList<cweeStr> getRules(int const& projectNum) {
		cweeThreadedList<cweeStr> out;

		for (int i = 1; i <= getNumRules(projectNum); i++) {
			cweeStr rule = getRule(projectNum, i);
			rule.Replace("\t", " ");
			rule.Replace("\r", "\n");
			rule.ReduceSpaces(true);

			cweeStr ruleName = "RULE edmsGen_" + cweeStr(cweeRandomInt(100, 100000));	// RULE HIGHLANDS790ON
			cweeThreadedList<cweeStr> ifs_ands_ors;							// IF TANK T04 LEVEL < 14.000000 \n OR SYSTEM CLOCKTIME < 1:00 PM
			cweeThreadedList<cweeStr> then_ands;							// THEN	PUMP	PS1301	STATUS	IS	OPEN \n AND PUMP	PS1301	STATUS	IS	OPEN
			cweeThreadedList<cweeStr> else_ands;							// ELSE PUMP	PS1301	STATUS	IS	CLOSED \n AND PUMP	PS1301	STATUS	IS	CLOSED
			cweeStr priority = "PRIORITY 5";								// PRIORITY 5

			{
				bool foundThen = false;
				bool foundElse = false;

				cweeParser parsedRule(rule, "\n", true);
				for (auto& x : parsedRule) {
					cweeParser y(x, " ", true);
					cweeStr Rstmt = y[0];

					if (Rstmt.Icmp("RULE") == 0) {
						ruleName = x;
					}
					else if (Rstmt.Icmp("THEN") == 0) {
						foundThen = true;
						then_ands.Append(x); // THEN AND
					}
					else if (Rstmt.Icmp("ELSE") == 0) {
						foundElse = true;

						y[0] = "THEN";
						x.Clear(); for (auto& z : y) x.AddToDelimiter(z, " ");

						else_ands.Append(x); // ELSE AND
					}
					else if (Rstmt.Icmp("PRIORITY") == 0) {
						priority = x;
					}
					else {
						// "IF", "OR", "AND"
						if (foundElse) {
							y[0] = "THEN";
							x.Clear(); for (auto& z : y) x.AddToDelimiter(z, " ");

							else_ands.Append(x); // ELSE'S AND
						}
						else if (foundThen) {
							y[0] = "THEN";
							x.Clear(); for (auto& z : y) x.AddToDelimiter(z, " ");

							then_ands.Append(x); // THEN'S AND
						}
						else {
							ifs_ands_ors.Append(x); // IF, AND, OR
						}
					}
				}
			}

			for (auto& then : then_ands) {
				cweeStr x = then;
				cweeStr newRule; {
					newRule = ruleName + cweeStr("_") + cweeStr(cweeRandomInt(1, 10000));
					for (cweeStr& IF : ifs_ands_ors) newRule.AddToDelimiter(IF, "\n");
					newRule.AddToDelimiter(x, "\n");
					newRule.AddToDelimiter(priority, "\n");
				}
				out.Append(newRule);
			}

			if (else_ands.Num() > 0) {
				// reverse the logic of the if statements
				cweeThreadedList< cweePair<cweeStr, cweeStr> > stringReplacementMap;
				{
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" = ", " !<> "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" == ", " !<> "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" <> ", " != "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" > ", " !<= "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" < ", " !>= "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" >= ", " !< "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" <= ", " !> "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" OR ", " !AND "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" or ", " !AND "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" Or ", " !AND "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" oR ", " !AND "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" AND ", " !OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" and ", " !OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" And ", " !OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" aNd ", " !OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" anD ", " !OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" ANd ", " !OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" AnD ", " !OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" aND ", " !OR "));
				}

				cweeThreadedList< cweeStr> olds;
				cweeThreadedList< cweeStr> news;
				for (cweePair<cweeStr, cweeStr>& x : stringReplacementMap) {
					olds.Append(x.first);
					news.Append(x.second);
				}

				for (cweeStr& x : ifs_ands_ors) {
					// FLIP THE RULE MEANING
					x = " " + x; // add space to start
					x.Replace(olds, news);
					x.StripLeadingOnce(" "); // remove starting space
				}
				// remove the "!" character
				stringReplacementMap.Clear();
				{
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" != ", " = "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !== ", " == "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !<> ", " <> "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !> ", " > "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !< ", " < "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !>= ", " >= "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !<= ", " <= "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !OR ", " OR "));
					stringReplacementMap.Append(cweePair<cweeStr, cweeStr>(" !AND ", " AND "));
				}
				olds.Clear();
				news.Clear();
				for (cweePair<cweeStr, cweeStr>& x : stringReplacementMap) {
					olds.Append(x.first);
					news.Append(x.second);
				}

				for (cweeStr& x : ifs_ands_ors) {
					// FLIP THE RULE MEANING
					x = " " + x; // add space to start
					x.Replace(olds, news);
					x.StripLeadingOnce(" "); // remove starting space
				}

				cweeStr newRule;
				for (cweeStr& ELSE : else_ands) {
					cweeStr x = ELSE;
					newRule.Clear();
					{
						newRule = ruleName + cweeStr("_") + cweeStr(cweeRandomInt(1, 10000));
						for (cweeStr& IF : ifs_ands_ors) newRule.AddToDelimiter(IF, "\n");
						newRule.AddToDelimiter(x, "\n");
						newRule.AddToDelimiter(priority, "\n");
					}
					out.Append(newRule);
				}
			}
		}

		return out;
	};

	int getEPAnetIndexOfAsset(int projectNum, const cweeStr& name, bool isNode) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			switch (isNode) {
			case true: {
				epanet::epanet::EN_getnodeindex(toModify, (char*)name.c_str(), &out);
				return out;
				break;
			}
			case false: {
				epanet::epanet::EN_getlinkindex(toModify, (char*)name.c_str(), &out);
				return out;
				break;
			}
			}
		}
		return -1;
	};
	bool isNode(asset_t const& type) {
		switch (type) {
		case asset_t::JUNCTION: {
			return true;
			break;
		}
		case asset_t::RESERVOIR: {
			return true;
			break;
		}
		case asset_t::PIPE: {
			return false;
			break;
		}
		case asset_t::PUMP: {
			return false;
			break;
		}
		case asset_t::VALVE: {
			return false;
			break;
		}
		}
		return false;
	};
	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& projectNum, int const& EPAnetIndex, bool const& isNode, int const& prop) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			// split based on Node or Link
			switch (isNode) {
			case true: {
				epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, prop, &out); // DEMAND, HEAD, PRESSURE, QUALITY, SOURCEMASS
				return out;
				break;
			}
			case false: {
				epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, prop, &out); // FLOW, VELOCITY, HEADLOSS, STATUS, SETTING
				return out;
				break;
			}
			}
		}
		return -(cweeMath::INF);
	};

	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& projectNum, int const& EPAnetIndex, value_t const& value) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			switch (value) {
			case _DEMAND_: {	epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 9, &out);	break; }
			case _HEAD_: {		epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 10, &out);	break; }
			case _LEVEL_: {
				epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 10, &out);
				units::dimensionless::scalar_t out2;
				epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 0, &out2);
				out -= out2;
				break; }
			case _QUALITY_: {	epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 12, &out);	break; }
			case _MASS_FLOW_: {	epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 13, &out);	break; }
			case _FLOW_: {		epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 8, &out);	break; }
			case _VELOCITY_: {	epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 9, &out);	break; }
			case _HEADLOSS_: {	epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 10, &out);	break; }
			case _STATUS_: {		epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 11, &out);	break; }
			case _SETTING_: {	epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 12, &out);	break; }
			case _ENERGY_: {		epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 13, &out);	break; }
			}
			return out;
		}
		return -(cweeMath::INF);
	};

	units::dimensionless::scalar_t getCurrentHydraulicValue(int projectNum, const cweeStr& name, const value_t& value, bool isNode) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			int EPAnetIndex = getEPAnetIndexOfAsset(projectNum, name, isNode);
			switch (value) {
			case _DEMAND_: {	epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 9, &out);	break; }
			case _HEAD_: {		epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 10, &out);	break; }
			case _LEVEL_: {
				epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 10, &out);
				units::dimensionless::scalar_t out2;
				epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 0, &out2);
				out -= out2;
				break; }
			case _QUALITY_: {	epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 12, &out);	break; }
			case _MASS_FLOW_: {	epanet::epanet::EN_getnodevalue(toModify, EPAnetIndex, 13, &out);	break; }
			case _FLOW_: {		epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 8, &out);	break; }
			case _VELOCITY_: {	epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 9, &out);	break; }
			case _HEADLOSS_: {	epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 10, &out);	break; }
			case _STATUS_: {		epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 11, &out);	break; }
			case _SETTING_: {	epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 12, &out);	break; }
			case _ENERGY_: {		epanet::epanet::EN_getlinkvalue(toModify, EPAnetIndex, 13, &out);	break; }
			}
			return out;
		}
		return -(cweeMath::INF);
	};

	cweeThreadedList<cweeStr> getImpactedAssetsFromRule(int const& projectNum, int ruleIndex) {
		cweeThreadedList<cweeStr> out;
		epanet::EN_Project toModify;

		if (findProject(projectNum, &toModify) == 1) {
			int numPrem(0), numThen(0), numElse(0);
			units::dimensionless::scalar_t priority(0);
			getRuleSizes(projectNum, ruleIndex, &numPrem, &numThen, &numElse, &priority);

			for (int i = 1; i <= numThen; i++) {
				int linkIndex(0), status(0); units::dimensionless::scalar_t setting(0);
				epanet::epanet::EN_getthenaction(toModify, ruleIndex, i, &linkIndex, &status, &setting);
				cweeStr assetName = getlinkID(projectNum, linkIndex);
				out.AddUnique(assetName);
			}
			for (int i = 1; i <= numElse; i++) {
				int linkIndex(0), status(0); units::dimensionless::scalar_t setting(0);
				epanet::epanet::EN_getelseaction(toModify, ruleIndex, i, &linkIndex, &status, &setting);
				cweeStr assetName = getlinkID(projectNum, linkIndex);
				out.AddUnique(assetName);
			}
		}

		return out;
	};
	cweeThreadedList<cweeStr> getImpactedAssetsFromRules(int const& projectNum) {
		cweeThreadedList<cweeStr> out;

		for (int i = 1; i <= getNumRules(projectNum); i++) {
			for (auto& x : getImpactedAssetsFromRule(projectNum, i)) {
				out.AddUnique(x);
			}
		}

		return out;
	};

	cweeThreadedList<cweeStr> getImpactedAssetsFromControls(int const& projectNum) {
		cweeThreadedList<cweeStr> out;

		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			for (int i = 1; i <= getNumControls(projectNum); i++) {
				int type(0);
				int linkIndex(0);
				units::dimensionless::scalar_t setting(0);
				int nodeIndex(0);
				units::dimensionless::scalar_t level(0);

				getControl(projectNum, i, &type, &linkIndex, &setting, &nodeIndex, &level);

				if (type == -1) continue;
				cweeStr link_name = getlinkID(projectNum, linkIndex);

				out.AddUnique(link_name);
			}
		}

		return out;
	};

	int	 addNode(int const& projectNum, cweeStr const& name, int const& nodeType) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_addnode(toModify, (char*)name.c_str(), nodeType, &out);
			return out;
		}
		return out;
	};
	void deleteNode(int const& projectNum, int const& nodeIndex, int const& actionCode) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_deletenode(toModify, nodeIndex, actionCode);
		}
	};
	void setNodeID(int const& projectNum, int const& nodeIndex, cweeStr const& newid) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setnodeid(toModify, nodeIndex, (char*)newid.c_str());
		}
	};
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
	void setNodeValue(int const& projectNum, int const& nodeIndex, int const& property, units::dimensionless::scalar_t const& value) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setnodevalue(toModify, nodeIndex, property, value);
		}
	};
	void setJuncData(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& dmnd, cweeStr const& dmndpat) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setjuncdata(toModify, nodeIndex, elev, dmnd, (char*)dmndpat.c_str());
		}
	};
	void setTankData(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& initlvl, units::dimensionless::scalar_t const& minlvl, units::dimensionless::scalar_t const& maxlvl, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& minvol, cweeStr const& volcurve) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_settankdata(toModify, nodeIndex, elev, initlvl, minlvl, maxlvl, diam, minvol, (char*)volcurve.c_str());
		}
	};
	void setCoord(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setcoord(toModify, nodeIndex, x, y);
		}
	};
	int	 getNodeindex(int const& projectNum, cweeStr const& id) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getnodeindex(toModify, (char*)id.c_str(), &out);
			return out;
		}
		return out;
	};
	cweeStr getNodeid(int const& projectNum, int const& nodeIndex) {
		epanet::EN_Project toModify;
		cweeStr outInit = " this this this this this this this";
		char* out = (char*)outInit.c_str();
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getnodeid(toModify, nodeIndex, out) > 100) return "-1";
			return cweeStr(out);
		}
		return cweeStr(out);
	};
	int  getNodetype(int const& projectNum, int const& nodeIndex) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getnodetype(toModify, nodeIndex, &out);
			return out;
		}
		return out;
	};

	float getNodevalue(int const& projectNum, int const& nodeIndex, int const& property) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getnodevalue(toModify, nodeIndex, property, &out);
			return out;
		}
		return out;
	};
	float getCoordX(int const& projectNum, int const& nodeIndex) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		units::dimensionless::scalar_t out2(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getcoord(toModify, nodeIndex, &out, &out2);
			return out;
		}
		return out;
	};
	float getCoordY(int const& projectNum, int const& nodeIndex) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		units::dimensionless::scalar_t out2(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getcoord(toModify, nodeIndex, &out2, &out);
			return out;
		}
		return out;
	};

	void addDemand(int const& projectNum, int const& nodeIndex, units::dimensionless::scalar_t const& baseDemand, cweeStr const& demandPattern, cweeStr const& demandName) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_adddemand(toModify, nodeIndex, baseDemand, (char*)demandPattern.c_str(), (char*)demandName.c_str());
		}
	};
	void deleteDemand(int const& projectNum, int const& nodeIndex, int const& demandIndex) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_deletedemand(toModify, nodeIndex, demandIndex);
		}
	};
	void setBaseDemand(int const& projectNum, int const& nodeIndex, int const& demandIndex, units::dimensionless::scalar_t const& baseDemand) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setbasedemand(toModify, nodeIndex, demandIndex, baseDemand);
		}
	};
	void setDemandPattern(int const& projectNum, int const& nodeIndex, int const& demandIndex, int const& patIndex) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setdemandpattern(toModify, nodeIndex, demandIndex, patIndex);
		}
	};
	void setDemandName(int const& projectNum, int const& nodeIndex, int const& demandIdx, cweeStr const& demandName) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setdemandname(toModify, nodeIndex, demandIdx, (char*)demandName.c_str());
		}
	};
	int  getDemandIndex(int const& projectNum, int const& nodeIndex, cweeStr const& demandName) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getdemandindex(toModify, nodeIndex, (char*)demandName.c_str(), &out);
		}
		return out;
	};
	int  getNumDemands(int const& projectNum, int const& nodeIndex) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getnumdemands(toModify, nodeIndex, &out);
		}
		return out;
	};
	float getBaseDemand(int const& projectNum, int const& nodeIndex, int const& demandIndex) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getbasedemand(toModify, nodeIndex, demandIndex, &out);
		}
		return out;
	};
	int  getDemandPattern(int const& projectNum, int const& nodeIndex, int const& demandIndex) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getdemandpattern(toModify, nodeIndex, demandIndex, &out);
		}
		return out;
	};
	cweeStr getDemandName(int const& projectNum, int const& nodeIndex, int const& demandIndex) {
		epanet::EN_Project toModify;
		cweeStr outInit = " this this this this this this this";
		char* out = (char*)outInit.c_str();
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getdemandname(toModify, nodeIndex, demandIndex, out);
		}
		return cweeStr(out);
	};

	int addLink(int const& projectNum, cweeStr const& id, int const& linkType, cweeStr const& fromNode, cweeStr const& toNode) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_addlink(toModify, (char*)id.c_str(), linkType, (char*)fromNode.c_str(), (char*)toNode.c_str(), &out);
		}
		return out;
	};
	void deleteLink(int const& projectNum, int const& index, int const& actionCode) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_deletelink(toModify, index, actionCode);
		}
	};
	void setLinkValue(int const& projectNum, int const& index, int const& property, units::dimensionless::scalar_t const& value) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setlinkvalue(toModify, index, property, value);
		}
	};
	void setPipeData(int const& projectNum, int const& index, units::dimensionless::scalar_t const& length, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& rough, units::dimensionless::scalar_t const& mloss) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setpipedata(toModify, index, length, diam, rough, mloss);
		}
	};
	void setLinkID(int const& projectNum, int const& index, cweeStr const& newid) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setlinkid(toModify, index, (char*)newid.c_str());
		}
	};
	void setLinkType(int const& projectNum, int* inout_index, int const& linkType, int const& actionCode) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setlinktype(toModify, inout_index, linkType, actionCode);
		}
	};
	void setLinkNodes(int const& projectNum, int const& index, int const& node1, int const& node2) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setlinknodes(toModify, index, node1, node2);
		}
	};
	int getlinkUpstreamNode(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		int out(-1);
		int out2(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getlinknodes(toModify, index, &out, &out2);
		}
		return out;
	};
	int getlinkDownstreamNode(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		int out(-1);
		int out2(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getlinknodes(toModify, index, &out2, &out);
		}
		return out;
	};
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
	float getLinkValue(int const& projectNum, int const& index, int const& property) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getlinkvalue(toModify, index, property, &out);
		}
		return out;
	};
	int getlinkIndex(int const& projectNum, cweeStr const& id) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getlinkindex(toModify, (char*)id.c_str(), &out);
		}
		return out;
	};
	cweeStr getlinkID(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		cweeStr outInit = " this this this this this this this";
		char* out = (char*)outInit.c_str();
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getlinkid(toModify, index, out) > 100) return "-1";
		}
		return cweeStr(out);
	};
	int getLinkType(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getlinktype(toModify, index, &out);
		}
		return out;
	};

	int getPumpType(int const& projectNum, int const& linkIndex) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getpumptype(toModify, linkIndex, &out);
		}
		return out;
	};
	int getHeadCurveIndex(int const& projectNum, int const& linkIndex) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getheadcurveindex(toModify, linkIndex, &out);
		}
		return out;
	};
	void setHeadCurveIndex(int const& projectNum, int const& linkIndex, int const& curveIndex) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setheadcurveindex(toModify, linkIndex, curveIndex);
		}
	};

	void	addPattern(int const& projectNum, cweeStr const& id) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_addpattern(toModify, (char*)id.c_str());
		}
	};
	void	deletePattern(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_deletepattern(toModify, index);
		}
	};
	void	setPatternID(int const& projectNum, int const& index, cweeStr const& id) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setpatternid(toModify, index, (char*)id.c_str());
		}
	};
	void	setPatternValue(int const& projectNum, int const& index, int const& period, units::dimensionless::scalar_t const& value) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setpatternvalue(toModify, index, period, value);
		}
	};
	void	setPattern(int const& projectNum, int const& index, units::dimensionless::scalar_t* values, int const& len) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setpattern(toModify, index, values, len);
		}
	};
	int		getPatternIndex(int const& projectNum, cweeStr const& id) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getpatternindex(toModify, (char*)id.c_str(), &out);
		}
		return out;
	};
	cweeStr getPatternID(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		cweeStr outInit = " this this this this this this this";
		char* out = (char*)outInit.c_str();
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getpatternid(toModify, index, out) > 100) return "-1";
		}
		return cweeStr(out);
	};
	int		getPatternLen(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getpatternlen(toModify, index, &out);
		}
		return out;
	};
	float getPatternValue(int const& projectNum, int const& index, int const& period) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getpatternvalue(toModify, index, period, &out);
		}
		return out;
	};
	float	getAveragePatternValue(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		units::dimensionless::scalar_t out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getaveragepatternvalue(toModify, index, &out);
		}
		return out;
	};
	int getNumPatterns(int const& projectNum) {
#define maxAssets 100000
		epanet::EN_Project toModify;
		int out(0);
		if (findProject(projectNum, &toModify) == 1) {
			int index = 0;
			int limit(5);
			do {
				index++;
				if (getPatternID(projectNum, index) != "-1") out++;
				else limit--;
			} while (index < maxAssets && limit > 0);
		}
		return out;
#undef maxAssets
	};

	cweeThreadedList<float> getPattern(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		cweeThreadedList<float> out;
		if (findProject(projectNum, &toModify) == 1) {
			int len = getPatternLen(projectNum, index);
			out.SetGranularity(len + 16);
			for (int j = 1; j <= len; j++) {
				float value = getPatternValue(projectNum, index, j);
				out.Append(value);
			}
		}
		return out;
	};

	void setPattern(int const& projectNum, int const& index, const cweeThreadedList<float>& data) {
		//if (getPatternLen(projectNum, index) >= data.Num()) {
		//	return; // cannot units::dimensionless::scalar_t-commit		
		//}

		cweeDynamicBlockAlloc<units::dimensionless::scalar_t, 1 << 18, sizeof(units::dimensionless::scalar_t)>	dataAllocator;
		dataAllocator.Init();
		units::dimensionless::scalar_t* values = dataAllocator.Alloc(data.Num());
		for (int j = 0; j < data.Num(); j++) {
			values[j] = (units::dimensionless::scalar_t)data[j];
		}
		setPattern(projectNum, index, values, data.Num());
		dataAllocator.Resize(values, 0);
		dataAllocator.Free(values);
		dataAllocator.Shutdown();
	};

	void setPattern(int const& projectNum, int const& index, const cweeThreadedList<std::pair<u64, float>>& data) {
		//if (getPatternLen(projectNum, index) >= data.Num()) {
		//	return; // cannot units::dimensionless::scalar_t-commit	
		//}

		cweeDynamicBlockAlloc<units::dimensionless::scalar_t, 1 << 18, sizeof(units::dimensionless::scalar_t)>	dataAllocator;
		dataAllocator.Init();
		units::dimensionless::scalar_t* values = dataAllocator.Alloc(data.Num());
		for (int j = 0; j < data.Num(); j++) {
			values[j] = (units::dimensionless::scalar_t)data[j].second;
		}
		setPattern(projectNum, index, values, data.Num());
		dataAllocator.Resize(values, 0);
		dataAllocator.Free(values);
		dataAllocator.Shutdown();
	};

	void setPattern(int const& projectNum, cweeStr const& id, const cweeThreadedList<float>& data) {
		int index = getPatternIndex(projectNum, id);
		if (index <= 0) {
			addPattern(projectNum, id);
			index = getPatternIndex(projectNum, id);
		}
		setPattern(projectNum, index, data);
	};
	void setPattern(int const& projectNum, cweeStr const& id, const cweeThreadedList<std::pair<u64, float>>& data) {
		int index = getPatternIndex(projectNum, id);
		if (index <= 0) {
			addPattern(projectNum, id);
			index = getPatternIndex(projectNum, id);
		}
		setPattern(projectNum, index, data);
	};

	void	addCurve(int const& projectNum, cweeStr const& id) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_addcurve(toModify, (char*)id.c_str());
		}
	};
	void	deleteCurve(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_deletecurve(toModify, index);
		}
	};
	void	setCurve(int const& projectNum, int const& index, units::dimensionless::scalar_t* xValues, units::dimensionless::scalar_t* yValues, int const& nPoints) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setcurve(toModify, index, xValues, yValues, nPoints);
		}
	};
	void	setCurveID(int const& projectNum, int const& index, cweeStr const& id) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setcurveid(toModify, index, (char*)id.c_str());
		}
	};
	void	setCurveValue(int const& projectNum, int const& curveIndex, int const& pointIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y) {
		epanet::EN_Project toModify;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_setcurvevalue(toModify, curveIndex, pointIndex, x, y);
		}
	};
	int		getCurveIndex(int const& projectNum, cweeStr const& id) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getcurveindex(toModify, (char*)id.c_str(), &out);
		}
		return out;
	};
	cweeStr getCurveID(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		cweeStr outInit = " this this this this this this this";
		char* out = (char*)outInit.c_str();
		if (findProject(projectNum, &toModify) == 1) {
			if (epanet::epanet::EN_getcurveid(toModify, index, out) > 100) return "-1";
		}
		return cweeStr(out);
	};
	std::pair<float, float> getCurveValue(int const& projectNum, int const& index, int const& period) {
		epanet::EN_Project toModify;
		std::pair<units::dimensionless::scalar_t, units::dimensionless::scalar_t> out;
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getcurvevalue(toModify, index, period, &out.first, &out.second);
		}
		return out;
	};
	int		getCurveLen(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getcurvelen(toModify, index, &out);
		}
		return out;
	};
	int		getCurveType(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		int out(-1);
		if (findProject(projectNum, &toModify) == 1) {
			epanet::epanet::EN_getcurvetype(toModify, index, &out);
		}
		return out;
	};
	int getNumCurves(int const& projectNum) {
#define maxAssets 100000
		epanet::EN_Project toModify;
		int out(0);
		if (findProject(projectNum, &toModify) == 1) {
			int index = 0;
			int limit(5);
			do {
				index++;
				if (getCurveID(projectNum, index) != "-1") out++;
				else limit--;
			} while (index < maxAssets && limit > 0);
		}
		return out;
#undef maxAssets
	};
	cweeThreadedList<std::pair<float, float>> getCurve(int const& projectNum, int const& index) {
		epanet::EN_Project toModify;
		cweeThreadedList<std::pair<float, float>> out;
		if (findProject(projectNum, &toModify) == 1) {
			int len = getCurveLen(projectNum, index);
			out.SetGranularity(len + 16);
			for (int j = 1; j <= len; j++) {
				std::pair<float, float> value = getCurveValue(projectNum, index, j);
				out.Append(value);
			}
		}
		return out;
	};
	int InsertPattern(int projectNum, Pattern* pat, const cweeStr& name, u64 start, u64 end, u64 stepSeconds) {
		int index = this->getPatternIndex(projectNum, name);
		if (index <= 0) {
			this->addPattern(projectNum, name);
			index = this->getPatternIndex(projectNum, name);
			cweeThreadedList<std::pair<u64, float>> timeSeries = pat->GetTimeSeries(start, end, stepSeconds);
			this->setPattern(projectNum, name, timeSeries);
		}
		return index;
	};




	EPAnet_Local() : projectMap(), projectError(), projectTime(), projectTimestep(), numProjects(0) {};

private:
	cweeInterlocked<std::map<int, epanet::EN_Project>> projectMap;
	cweeInterlocked < std::map<int, int>> projectError;
	cweeInterlocked < std::map<int, long>> projectTime;
	cweeInterlocked < std::map<int, long>> projectTimestep;
	cweeSysInterlockedInteger numProjects;
	static char DefaultStr[epanet::MAXFNAME];

	static cweeStr createTimeFromMinutes(float minutes) {
		cweeStr out;
		int simHours = cweeMath::Floor(minutes / 60);
		int simMinutes = cweeMath::Floor(cweeMath::Frac(minutes / 60) * 60);
		if (simHours < 10) out = "0" + cweeStr(simHours); else out = cweeStr(simHours);
		out += ":";
		if (simMinutes < 10) out += cweeStr("0" + cweeStr(simMinutes)); else out += cweeStr(simMinutes);
		return out;
	};
	bool findProject(int const& projectNum, epanet::EN_Project* out) {
		AUTO e = projectMap.GetExclusive();
		AUTO it = e->find(projectNum);
		if (it == e->end()) {
			return false;
		}
		else {
			*out = it->second;
			return true;
		}
	};
	bool findError(int const& projectNum, int* out) {
		AUTO e = projectError.GetExclusive();
		AUTO it = e->find(projectNum);
		if (it == e->end()) {
			return false;
		}
		else {
			*out = it->second;
			return true;
		}
	};
	bool findTime(int const& projectNum, long* out) {
		AUTO e = projectTime.GetExclusive();
		AUTO it = e->find(projectNum);
		if (it == e->end()) {
			return false;
		}
		else {
			*out = it->second;
			return true;
		}
	};
	bool findTimeStep(int const& projectNum, long* out) {
		AUTO e = projectTimestep.GetExclusive();
		AUTO it = e->find(projectNum);
		if (it == e->end()) {
			return false;
		}
		else {
			*out = it->second;
			return true;
		}
	};

};
static cweeSharedPtr< EPAnet_Local > EPAnetLocal = make_cwee_shared<EPAnet_Local>();

class EPAnetProject {
public:
	class EPAnetHydraulicSimulation {
	public:
		EPAnetHydraulicSimulation(EPAnetProject& p) : parent(p) { parent.startHydraulicSim(); };
		~EPAnetHydraulicSimulation() { parent.endHydraulicSim(); };
		void SetTimestep(float minutes) { parent.setTimeParam(5, 60.0f * minutes); };
		AUTO GetCurrentSimTime() { return parent.getCurrentSimTime(); };
		void DoSteadyState() { parent.calcHydraulicsAtTime(); };
		bool ShouldContinueSimulation() { 
			parent.calcHydraulicTimestepToNextTime();
			return parent.getCurrentSimTimestep() > 0; 
		};
		float GetSimulationValue(cweeStr const& name, value_t const& value_type, bool isNode) { return parent.getCurrentHydraulicValue(name, value_type, isNode); };

	private:
		EPAnetProject& parent;
	};
	cweeSharedPtr<EPAnetHydraulicSimulation> StartHydraulicSimulation() { return make_cwee_shared<EPAnetHydraulicSimulation>(new EPAnetHydraulicSimulation(*this)); };





	EPAnetProject() : proj(EPAnetLocal->createNewProject()) {};
	~EPAnetProject() {
		EPAnetLocal->deleteProject(proj);
	};






	void initBuild(cweeStr const& rptFile, cweeStr const& outFile, int const& unitsType, int const& headLossType) { return EPAnetLocal->initBuild(proj, rptFile, outFile, unitsType, headLossType); };

	void loadINP(const char* filePathToInpFile, const char* filePathToRptFile = "") { EPAnetLocal->loadINP(proj, filePathToInpFile, filePathToRptFile); };
	void closeINP() { EPAnetLocal->closeINP(proj); };
	void saveINP(cweeStr const& filePathToInpFile) { EPAnetLocal->saveINP(proj, filePathToInpFile); };
	cweeStr getTitle() { return EPAnetLocal->getTitle(proj); };

	cweeStr getDescription(bool const& isNode, int const& epanetIndex) { return EPAnetLocal->getDescription(proj, isNode, epanetIndex); };
	void setDescription(bool const& isNode, int const& epanetIndex, cweeStr const& description) { EPAnetLocal->setDescription(proj, isNode, epanetIndex, description); };
	int	getCount(int const& objectType) { return EPAnetLocal->getCount(proj, objectType); };

	void setFlowUnits(int const& units) { EPAnetLocal->setFlowUnits(proj, units); };
	void setQualType(int const& qualType, cweeStr const& chemName, cweeStr const& chemUnits, cweeStr const& traceNode) { EPAnetLocal->setQualType(proj, qualType, chemName, chemUnits, traceNode); };
	void setTimeParam(int const& param, long const& value) { EPAnetLocal->setTimeParam(proj, param, value); };
	void setOption(int const& option, units::dimensionless::scalar_t const& value) { EPAnetLocal->setOption(proj, option, value); };
	float getOption(int const& option) { return EPAnetLocal->getOption(proj, option); };
	float getTimeParam(int const& param) { return EPAnetLocal->getTimeParam(proj, param); };
	int	 getFlowUnits() { return EPAnetLocal->getFlowUnits(proj); };

	long getCurrentSimTime() { return EPAnetLocal->getCurrentSimTime(proj); };
	long getCurrentSimTimestep() { return EPAnetLocal->getCurrentSimTimestep(proj); };

	int	 getCurrentError() { return EPAnetLocal->getCurrentError(proj); };

	// hydraulic simulation
	void fullHydraulicSim() { EPAnetLocal->fullHydraulicSim(proj); };
	void startHydraulicSim() { EPAnetLocal->startHydraulicSim(proj); };
	void calcHydraulicsAtTime() { EPAnetLocal->calcHydraulicsAtTime(proj); };
	void calcHydraulicTimestepToNextTime() { EPAnetLocal->calcHydraulicTimestepToNextTime(proj); };
	void resetHydraulicSim() { EPAnetLocal->resetHydraulicSim(proj); };
	void endHydraulicSim() { EPAnetLocal->endHydraulicSim(proj); };

	// quality simulation
	void fullQualitySim() { EPAnetLocal->fullQualitySim(proj); };
	void startQualitySim() { EPAnetLocal->startQualitySim(proj); };
	void calcQualityAtTime() { EPAnetLocal->calcQualityAtTime(proj); };
	void calcQualityTimestepToNextTime() { EPAnetLocal->calcQualityTimestepToNextTime(proj); };
	void resetQualitySim() { EPAnetLocal->resetQualitySim(proj); };
	void endQualitySim() { EPAnetLocal->endQualitySim(proj); };

	// node management
	int	 addNode(cweeStr const& name, int const& nodeType) { return EPAnetLocal->addNode(proj, name, nodeType); };
	void deleteNode(int const& nodeIndex, int const& actionCode) { EPAnetLocal->deleteNode(proj, nodeIndex, actionCode); };
	void setNodeID(int const& nodeIndex, cweeStr const& newid) { EPAnetLocal->setNodeID(proj, nodeIndex, newid); };
	void setNodeValue(int const& nodeIndex, int const& property, units::dimensionless::scalar_t const& value) { EPAnetLocal->setNodeValue(proj, nodeIndex, property, value); };
	void setJuncData(int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& dmnd, cweeStr const& dmndpat) { EPAnetLocal->setJuncData(proj, nodeIndex, elev, dmnd, dmndpat); };
	void setTankData(int const& nodeIndex, units::dimensionless::scalar_t const& elev, units::dimensionless::scalar_t const& initlvl, units::dimensionless::scalar_t const& minlvl, units::dimensionless::scalar_t const& maxlvl, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& minvol, cweeStr const& volcurve) { 
		EPAnetLocal->setTankData(proj, nodeIndex, elev, initlvl, minlvl, maxlvl, diam, minvol, volcurve); 
	};
	void setCoord(int const& nodeIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y) { EPAnetLocal->setCoord(proj, nodeIndex, x, y); };
	int	 getNodeindex(cweeStr const& id) { return EPAnetLocal->getNodeindex(proj, id); };
	cweeStr getNodeid(int const& nodeIndex) { return EPAnetLocal->getNodeid(proj, nodeIndex); };
	int  getNodetype(int const& nodeIndex) { return EPAnetLocal->getNodetype(proj, nodeIndex); };
	float getNodevalue(int const& nodeIndex, int const& property) { return EPAnetLocal->getNodevalue(proj, nodeIndex, property); };
	float getCoordX(int const& nodeIndex) { return EPAnetLocal->getCoordX(proj, nodeIndex); };
	float getCoordY(int const& nodeIndex) { return EPAnetLocal->getCoordY(proj, nodeIndex); };

	// demand management
	void addDemand(int const& nodeIndex, units::dimensionless::scalar_t const& baseDemand, cweeStr const& demandPattern, cweeStr const& demandName) { EPAnetLocal->addDemand(proj, nodeIndex, baseDemand, demandPattern, demandName); };
	void deleteDemand(int const& nodeIndex, int const& demandIndex) { EPAnetLocal->deleteDemand(proj, nodeIndex, demandIndex); };
	void setBaseDemand(int const& nodeIndex, int const& demandIndex, units::dimensionless::scalar_t const& baseDemand) { EPAnetLocal->setBaseDemand(proj, nodeIndex, demandIndex, baseDemand); };
	void setDemandPattern(int const& nodeIndex, int const& demandIndex, int const& patIndex) { EPAnetLocal->setDemandPattern(proj, nodeIndex, demandIndex, patIndex); };
	void setDemandName(int const& nodeIndex, int const& demandIdx, cweeStr const& demandName) { EPAnetLocal->setDemandName(proj, nodeIndex, demandIdx, demandName); };
	int  getDemandIndex(int const& nodeIndex, cweeStr const& demandName) { return EPAnetLocal->getDemandIndex(proj, nodeIndex, demandName); };
	int  getNumDemands(int const& nodeIndex) { return EPAnetLocal->getNumDemands(proj, nodeIndex); };
	float getBaseDemand(int const& nodeIndex, int const& demandIndex) { return EPAnetLocal->getBaseDemand(proj, nodeIndex, demandIndex); };
	int  getDemandPattern(int const& nodeIndex, int const& demandIndex) { return EPAnetLocal->getDemandPattern(proj, nodeIndex, demandIndex); };
	cweeStr getDemandName(int const& nodeIndex, int const& demandIndex) { return EPAnetLocal->getDemandName(proj, nodeIndex, demandIndex); };

	// link management
	int addLink(cweeStr const& id, int const& linkType, cweeStr const& fromNode, cweeStr const& toNode) { return EPAnetLocal->addLink(proj, id, linkType, fromNode, toNode); };
	void deleteLink(int const& index, int const& actionCode) { EPAnetLocal->deleteLink(proj, index, actionCode); };
	void setLinkValue(int const& index, int const& property, units::dimensionless::scalar_t const& value) { EPAnetLocal->setLinkValue(proj, index, property, value); };
	void setPipeData(int const& index, units::dimensionless::scalar_t const& length, units::dimensionless::scalar_t const& diam, units::dimensionless::scalar_t const& rough, units::dimensionless::scalar_t const& mloss) { EPAnetLocal->setPipeData(proj, index, length, diam, rough, mloss); };
	void setLinkID(int const& index, cweeStr const& newid) { EPAnetLocal->setLinkID(proj, index, newid); };
	void setLinkType(int* inout_index, int const& linkType, int const& actionCode) { EPAnetLocal->setLinkType(proj, inout_index, linkType, actionCode); };
	void setLinkNodes(int const& index, int const& node1, int const& node2) { EPAnetLocal->setLinkNodes(proj, index, node1, node2); };
	int getlinkUpstreamNode(int const& index) { return EPAnetLocal->getlinkUpstreamNode(proj, index); };
	int getlinkDownstreamNode(int const& index) { return EPAnetLocal->getlinkDownstreamNode(proj, index); };
	float getLinkValue(int const& index, int const& property) { return EPAnetLocal->getLinkValue(proj, index, property); };
	int getlinkIndex(cweeStr const& id) { return EPAnetLocal->getlinkIndex(proj, id); };
	cweeStr getlinkID(int const& index) { return EPAnetLocal->getlinkID(proj, index); };
	int getLinkType(int const& index) { return EPAnetLocal->getLinkType(proj, index); };

	// pump supplemental management
	int getPumpType(int const& linkIndex) { return EPAnetLocal->getPumpType(proj, linkIndex); };
	int getHeadCurveIndex(int const& linkIndex) { return EPAnetLocal->getHeadCurveIndex(proj, linkIndex); };
	void setHeadCurveIndex(int const& linkIndex, int const& curveIndex) { EPAnetLocal->setHeadCurveIndex(proj, linkIndex, curveIndex); };

	// pattern and curve management
	void addPattern(cweeStr const& id) { EPAnetLocal->addPattern(proj, id); };
	void deletePattern(int const& index) { EPAnetLocal->deletePattern(proj, index); };
	void setPatternID(int const& index, cweeStr const& id) { EPAnetLocal->setPatternID(proj, index, id); };
	void setPatternValue(int const& index, int const& period, units::dimensionless::scalar_t const& value) { EPAnetLocal->setPatternValue(proj, index, period, value); };
	void setPattern(int const& index, units::dimensionless::scalar_t* values, int const& len) { EPAnetLocal->setPattern(proj, index, values, len); };
	int getPatternIndex(cweeStr const& id) { return EPAnetLocal->getPatternIndex(proj, id); };
	cweeStr getPatternID(int const& index) { return EPAnetLocal->getPatternID(proj, index); };
	int getPatternLen(int const& index) { return EPAnetLocal->getPatternLen(proj, index); };
	float getPatternValue(int const& index, int const& period) { return EPAnetLocal->getPatternValue(proj, index, period); };
	float getAveragePatternValue(int const& index) { return EPAnetLocal->getAveragePatternValue(proj, index); };
	int getNumPatterns() { return EPAnetLocal->getNumPatterns(proj); };
	cweeThreadedList<float> getPattern(int const& index) { return EPAnetLocal->getPattern(proj, index); };
	void setPattern(int const& index, const cweeThreadedList<float>& data) { EPAnetLocal->setPattern(proj, index, data); };
	void setPattern(int const& index, const cweeThreadedList<std::pair<u64, float>>& data) { EPAnetLocal->setPattern(proj, index, data); };
	void setPattern(cweeStr const& id, const cweeThreadedList<float>& data) { EPAnetLocal->setPattern(proj, id, data); };
	void setPattern(cweeStr const& id, const cweeThreadedList<std::pair<u64, float>>& data) { EPAnetLocal->setPattern(proj, id, data); };

	void addCurve(cweeStr const& id) { EPAnetLocal->addCurve(proj, id); };
	void deleteCurve(int const& index) { EPAnetLocal->deleteCurve(proj, index); };
	void setCurve(int const& index, units::dimensionless::scalar_t* xValues, units::dimensionless::scalar_t* yValues, int const& nPoints) { EPAnetLocal->setCurve(proj, index, xValues, yValues, nPoints); };
	void setCurveID(int const& index, cweeStr const& id) { EPAnetLocal->setCurveID(proj, index, id); };
	void setCurveValue(int const& curveIndex, int const& pointIndex, units::dimensionless::scalar_t const& x, units::dimensionless::scalar_t const& y) { EPAnetLocal->setCurveValue(proj, curveIndex, pointIndex, x, y); };
	int getCurveIndex(cweeStr const& id) { return EPAnetLocal->getCurveIndex(proj, id); };
	cweeStr getCurveID(int const& index) { return EPAnetLocal->getCurveID(proj, index); };
	std::pair<float, float> getCurveValue(int const& index, int const& period) { return EPAnetLocal->getCurveValue(proj, index, period); };
	int getCurveLen(int const& index) { return EPAnetLocal->getCurveLen(proj, index); };
	int getCurveType(int const& index) { return EPAnetLocal->getCurveType(proj, index); };
	int getNumCurves() { return EPAnetLocal->getNumCurves(proj); };
	cweeThreadedList<std::pair<float, float>> getCurve(int const& index) { return EPAnetLocal->getCurve(proj, index); };

	// controls and rule management
	int getNumControls() { return EPAnetLocal->getNumControls(proj); };
	void deleteControl(int const& controlIndex) { EPAnetLocal->deleteControl(proj, controlIndex); };
	void deleteAllControls() { EPAnetLocal->deleteAllControls(proj); };
	int addControl(int const& controlType, cweeStr const& linkName, float const& setting, cweeStr const& nodeName, float const& level) {
		return EPAnetLocal->addControl(proj, controlType, linkName, setting, nodeName, level);
	};
	int addControl(cweeStr const& entireControl) { return EPAnetLocal->addControl(proj, entireControl); };
	void getControl(int const& controlindex, int* type, int* linkIndex, units::dimensionless::scalar_t* setting, int* nodeIndex, units::dimensionless::scalar_t* level) { EPAnetLocal->getControl(proj, controlindex, type, linkIndex, setting, nodeIndex, level); };
	cweeStr getControl(int const& controlindex) { return EPAnetLocal->getControl(proj, controlindex); };

	int getNumRules() { return EPAnetLocal->getNumRules(proj); };
	int findRuleIndex(cweeStr const& ruleName) { return EPAnetLocal->findRuleIndex(proj, ruleName); };
	cweeStr getRuleID(int const& ruleIndex) { return EPAnetLocal->getRuleID(proj, ruleIndex); };
	int addRule(cweeStr const& name, cweeStr const& linkIndex, cweeStr const& linkValue, cweeStr const& ifTrueThenStatusOrSetting, cweeStr const& refObjectType, cweeStr const& refObjectIndex, cweeStr const& refObjectVariable, cweeStr const& ComparisonOperator, cweeStr const& refObjectStatusOrValue, units::dimensionless::scalar_t const& priority) { 
		return EPAnetLocal->addRule(proj, name, linkIndex, linkValue, ifTrueThenStatusOrSetting, refObjectType, refObjectIndex, refObjectVariable, ComparisonOperator,  refObjectStatusOrValue,  priority);
	};
	int addRule(cweeStr const& ruleName, cweeStr entireRule, bool const& returnIndex = true) { return EPAnetLocal->addRule(proj, ruleName, entireRule, returnIndex); };
	void deleteRule(int const& ruleIndex) { EPAnetLocal->deleteRule(proj, ruleIndex); };
	void deleteAllRules() { return EPAnetLocal->deleteAllRules(proj); };
	void getRuleSizes(int const& ruleIndex, int* nPremises = nullptr, int* nThenActions = nullptr, int* nElseActions = nullptr, units::dimensionless::scalar_t* priority = nullptr) { EPAnetLocal->getRuleSizes(proj, ruleIndex, nPremises, nThenActions, nElseActions, priority); };
	void setRule_Premise(int const& ruleIndex, int const& premiseIndex, int const& premiseType, int const& objectType, int const& objectIndex, int const& objectVariable, int const& comparisonOp, int const& comparisonStatus, float const& comparisonValue) { 
		EPAnetLocal->setRule_Premise(proj, ruleIndex, premiseIndex, premiseType, objectType, objectIndex, objectVariable, comparisonOp, comparisonStatus,comparisonValue);
	};
	void setRule_ElseAction(int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting) { EPAnetLocal->setRule_ElseAction(proj, ruleIndex, actionIndex, linkIndex, status, setting); };
	void setRule_Priority(int const& index, units::dimensionless::scalar_t const& priority) { EPAnetLocal->setRule_Priority(proj, index, priority); };
	void setRule_ThenAction(int const& ruleIndex, int const& actionIndex, int const& linkIndex, int const& status, units::dimensionless::scalar_t const& setting) { EPAnetLocal->setRule_ThenAction(proj, ruleIndex, actionIndex, linkIndex, status, setting); };
	cweeStr getRule(int const& ruleIndex) { return EPAnetLocal->getRule(proj, ruleIndex); };
	cweeThreadedList<cweeStr> getRules() { return EPAnetLocal->getRules(proj); };

	int getEPAnetIndexOfAsset(int projectNum, const cweeStr& name, bool isNode) { return EPAnetLocal->getEPAnetIndexOfAsset(proj, name, isNode); };
	static bool isNode(asset_t const& type) { return EPAnetLocal->isNode(type); };

	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& EPAnetIndex, bool const& isNode, int const& prop) { return EPAnetLocal->getCurrentHydraulicValue(proj, EPAnetIndex, isNode, prop); };
	units::dimensionless::scalar_t getCurrentHydraulicValue(int const& EPAnetIndex, value_t const& value) { return EPAnetLocal->getCurrentHydraulicValue(proj, EPAnetIndex, value); };
	units::dimensionless::scalar_t getCurrentHydraulicValue(const cweeStr& name, const value_t& value, bool isNode = true) { return EPAnetLocal->getCurrentHydraulicValue(proj, name, value, isNode); };
	cweeThreadedList<cweeStr> getImpactedAssetsFromRule(int ruleIndex) { return EPAnetLocal->getImpactedAssetsFromRule(proj, ruleIndex); };
	cweeThreadedList<cweeStr> getImpactedAssetsFromRules() { return EPAnetLocal->getImpactedAssetsFromRules(proj); };
	cweeThreadedList<cweeStr> getImpactedAssetsFromControls() { return EPAnetLocal->getImpactedAssetsFromControls(proj); };

	int InsertPattern(Pattern* pat, const cweeStr& name, u64 start, u64 end, u64 stepSeconds) {
		int index = this->getPatternIndex(name);
		if (index <= 0) {
			this->addPattern(name);
			index = this->getPatternIndex(name);
			cweeThreadedList<std::pair<u64, float>> timeSeries = pat->GetTimeSeries(start, end, stepSeconds);
			this->setPattern(name, timeSeries);
		}
		return index;
	};

private:
	int proj;
};
class EPAnet_Shared {
public:
	cweeSharedPtr<EPAnetProject>  createNewProject(void) { return make_cwee_shared<EPAnetProject>(); };
};
static cweeSharedPtr< EPAnet_Shared > EPAnet = make_cwee_shared<EPAnet_Shared>();
