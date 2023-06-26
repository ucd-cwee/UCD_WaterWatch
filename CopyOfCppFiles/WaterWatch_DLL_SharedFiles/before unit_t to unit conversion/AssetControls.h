#pragma once
#include "Precompiled.h"

#if 0
#pragma region "RG_Refactor"

struct UserInput_Input { 
	cweeStr userSpecification;
};
struct PID_Input {
	assetValueIdentifier simulationOutput; // i.e. pump flow
	float setpoint = 60; // i.e. desired pump flow. 
	float minSetting = 0; // i.e. 0%
	float maxSetting = 1; // i.e 100%
	float Ku = 1;
};







struct UserInput_Input {
	//bool useDefault = true;
	UserInput_InputType type;
	cweeStr userSpecification;

	cweeStr Serialize() {
		cweeStr delim = ":UserInput_Input:";
		cweeStr out;

		//out.AddToDelimiter(useDefault, delim);
		out.AddToDelimiter((int)enumClassToInt(type), delim);
		out.AddToDelimiter(userSpecification, delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {

		if (in == "") return;
		cweeParser obj(in, ":UserInput_Input:", true);

		//useDefault = (bool)(int)obj[0];
		type = static_cast<UserInput_InputType>((int)obj[0]);
		userSpecification = obj[1];
	};
};
struct PID_Input {
	assetValueIdentifier simulationOutput; // i.e. pump flow
	float setpoint = 60; // i.e. desired pump flow. 
	float minSetting = 0; // i.e. 0%
	float maxSetting = 1; // i.e 100%
	float Ku = 1;
};
struct TrainedModel_PredictionInput {
	bool isSimulation = false;
	bool isScada = false;
	bool isGlobal = false;
	int patternListIndex = -1;
	cweeStr patternName;
	float timeLag = 0;
	::TrainedModel_Modifier modifier = ::TrainedModel_Modifier::None;
	assetValueIdentifier simulation;

	cweeStr Serialize() {
		cweeStr delim = ":TrainedModel_PredictionInput:";
		cweeStr out;

		out.AddToDelimiter(isSimulation, delim);
		out.AddToDelimiter(isScada, delim);
		out.AddToDelimiter(isGlobal, delim);
		out.AddToDelimiter(patternListIndex, delim);
		out.AddToDelimiter(patternName, delim);
		out.AddToDelimiter(timeLag, delim);
		out.AddToDelimiter((int)enumClassToInt(modifier), delim);
		out.AddToDelimiter(simulation.Serialize(), delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {
		cweeParser obj(in, ":TrainedModel_PredictionInput:", true);

		isSimulation = (bool)(int)obj[0];
		isScada = (bool)(int)obj[1];
		isGlobal = (bool)(int)obj[2];
		patternListIndex = (int)obj[3];
		patternName = obj[4];
		timeLag = (float)obj[5];
		modifier = static_cast<TrainedModel_Modifier>((int)obj[6]);
		simulation.Deserialize(obj[7]);

	};
};
struct TrainedModel_Input {
	bool useDefault = true;
	cweeThreadedList<float> operatingSetPoints;
	time_t startDate = fileSystem->getCurrentTime() - 31536000;
	time_t endDate = fileSystem->getCurrentTime();
	cweeThreadedList<TrainedModel_PredictionInput> predictionInputs;
	::TrainedModel_ErrorMetric errMetric;

	cweeStr Serialize() {
		cweeStr delim = ":TrainedModel_Input:";
		cweeStr out;

		out.AddToDelimiter(useDefault, delim);
		cweeStr temp; for (auto& x : operatingSetPoints) temp.AddToDelimiter(x, ","); out.AddToDelimiter(temp, delim);
		out.AddToDelimiter((float)startDate, delim);
		out.AddToDelimiter((float)endDate, delim);
		temp.Clear(); for (auto& x : predictionInputs) temp.AddToDelimiter(x.Serialize(), ":TrainedModel_Input_predictionInputs:"); out.AddToDelimiter(temp, delim);
		out.AddToDelimiter((int)enumClassToInt(errMetric), delim);


		return out;
	};
	void Deserialize(const cweeStr& in) {
		if (in == "") return;

		cweeParser obj(in, ":TrainedModel_Input:", true);

		useDefault = (bool)(int)obj[0];
		operatingSetPoints.Clear();	cweeParser x(obj[1], ",", true); for (auto& y : x) { if (y.IsEmpty()) continue; operatingSetPoints.Append((int)y); }
		startDate = (time_t)(float)obj[2];
		endDate = (time_t)(float)obj[3];
		predictionInputs.Clear(); cweeParser z(obj[4], ":TrainedModel_Input_predictionInputs:", true); for (auto& w : z) { if (w.IsEmpty()) continue; TrainedModel_PredictionInput temp; temp.Deserialize(w); predictionInputs.Append(temp); }
		errMetric = static_cast<TrainedModel_ErrorMetric>((int)obj[5]);
	};

	void SwapAssetValueIdentifiers(const assetValueIdentifier& toFind, const assetValueIdentifier& replaceWith) {
		for (auto& x : predictionInputs) {
			if (x.simulation == toFind) x.simulation = replaceWith;
			if (x.isSimulation == true) {
				cweeStr temp = x.patternName;
				temp.Replace(":delim:", " ");

				cweeStr checkWith = GetString(toFind.first.first) + " " + toFind.assetName + " ";
				if (temp.Find(checkWith) >= 0) {
					if (x.patternName.Find(":delim:") >= 0) {
						x.patternName.Replace(GetString(toFind.first.first) + ":delim:" + toFind.assetName, GetString(toFind.first.first) + ":delim:" + replaceWith.assetName);
					}
					else {
						x.patternName.Replace(GetString(toFind.first.first) + " " + toFind.assetName, GetString(toFind.first.first) + " " + replaceWith.assetName);
					}
				}
			}
		}
	};

};
struct Optimization_Subject {
	assetValueIdentifier assetToOptimize;
	Optimization_SetPoint tankSetPoint;
	Optimization_SetPoint timeSetPoint;

	cweeStr Serialize() {
		cweeStr delim = ":Optimization_Subject:";
		cweeStr out;

		out.AddToDelimiter(assetToOptimize.Serialize(), delim);
		out.AddToDelimiter((int)enumClassToInt(tankSetPoint), delim);
		out.AddToDelimiter((int)enumClassToInt(timeSetPoint), delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {
		cweeParser obj(in, ":Optimization_Subject:", true);

		assetToOptimize.Deserialize(obj[0]);
		tankSetPoint = static_cast<Optimization_SetPoint>((int)obj[1]);
		timeSetPoint = static_cast<Optimization_SetPoint>((int)obj[2]);
	};
	bool is_equal(Optimization_Subject opt_subject_for_comparison) {
		if (assetToOptimize == opt_subject_for_comparison.assetToOptimize &&
			tankSetPoint == opt_subject_for_comparison.tankSetPoint &&
			timeSetPoint == opt_subject_for_comparison.timeSetPoint
			) {
			return true;
		}
		else { return false; }

	}
};
struct Optimization_ObjectiveValue {
	float weight;
	Optimization_Value value;
	Optimization_Aggregation aggregation;

	cweeStr Serialize() {
		cweeStr delim = ":Optimization_ObjectiveValue:";
		cweeStr out;

		out.AddToDelimiter(weight, delim);
		out.AddToDelimiter((int)enumClassToInt(value), delim);
		out.AddToDelimiter((int)enumClassToInt(aggregation), delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {
		cweeParser obj(in, ":Optimization_ObjectiveValue:", true);

		weight = (float)obj[0];
		value = static_cast<Optimization_Value>((int)obj[1]);
		aggregation = static_cast<Optimization_Aggregation>((int)obj[2]);
	};

};
struct Optimization_Constraint {
	Optimization_Value value;
	Optimization_Aggregation aggregation;
	Optimization_Comparison comparison;
	float threshold;

	cweeStr Serialize() {
		cweeStr delim = ":Optimization_Constraint:";
		cweeStr out;

		out.AddToDelimiter((int)enumClassToInt(value), delim);
		out.AddToDelimiter((int)enumClassToInt(aggregation), delim);
		out.AddToDelimiter((int)enumClassToInt(comparison), delim);
		out.AddToDelimiter(threshold, delim);
		return out;
	};
	void Deserialize(const cweeStr& in) {
		cweeParser obj(in, ":Optimization_Constraint:", true);

		value = static_cast<Optimization_Value>((int)obj[0]);
		aggregation = static_cast<Optimization_Aggregation>((int)obj[1]);
		comparison = static_cast<Optimization_Comparison>((int)obj[2]);
		threshold = (float)obj[3];
	};
};
struct Optimization_Input {
	time_t startDate;
	time_t endDate;
	cweeThreadedList<Optimization_Subject> optimizationSubjects;
	cweeThreadedList<Optimization_ObjectiveValue> optimizationObjectives;
	cweeThreadedList<Optimization_Constraint> optimizationConstraints;

	cweeStr Serialize() {
		cweeStr delim = ":Optimization_Input:";
		cweeStr out;

		out.AddToDelimiter((float)startDate, delim);
		out.AddToDelimiter((float)endDate, delim);
		cweeStr temp; for (auto& x : optimizationSubjects) temp.AddToDelimiter(x.Serialize(), ":Optimization_Input_optimizationSubjects:"); out.AddToDelimiter(temp, delim);
		temp.Clear(); for (auto& x : optimizationObjectives) temp.AddToDelimiter(x.Serialize(), ":Optimization_Input_optimizationObjectives:"); out.AddToDelimiter(temp, delim);
		temp.Clear(); for (auto& x : optimizationConstraints) temp.AddToDelimiter(x.Serialize(), ":Optimization_Input_optimizationConstraints:"); out.AddToDelimiter(temp, delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {
		if (in == "") return;

		cweeParser obj(in, ":Optimization_Input:", true);

		startDate = (time_t)(float)obj[0];
		endDate = (time_t)(float)obj[1];
		optimizationSubjects.Clear(); cweeParser z(obj[2], ":Optimization_Input_optimizationSubjects:", true); for (auto& w : z) { Optimization_Subject temp; temp.Deserialize(w); optimizationSubjects.Append(temp); }
		optimizationObjectives.Clear(); cweeParser a(obj[3], ":Optimization_Input_optimizationObjectives:", true); for (auto& w : a) { Optimization_ObjectiveValue temp; temp.Deserialize(w); optimizationObjectives.Append(temp); }
		optimizationConstraints.Clear(); cweeParser b(obj[4], ":Optimization_Input_optimizationConstraints:", true); for (auto& w : b) { Optimization_Constraint temp; temp.Deserialize(w); optimizationConstraints.Append(temp); }
	};
	void SwapAssetValueIdentifiers(const assetValueIdentifier& toFind, const assetValueIdentifier& replaceWith) {
		for (auto& x : optimizationSubjects)
			if (x.assetToOptimize == toFind) x.assetToOptimize = replaceWith;
	};
};

/*!
Class that contains the fundamental data to check a (say) reservoir's head >= 155
*/
struct RuleCondition {
	assetValueIdentifier subject;		// reservoir head, pump flow, pipe status, time, etc.
	Optimization_Comparison operand;	// >=, ==, !=, etc.
	float threshold;					// 155, 0.0, etc.

	cweeStr getEPAnetLogic() {
		// Form:
		/*
		assetType subject characteristic comparison threshold
		*/
		if (subject.first.first == asset_t::TIME)
			return cweeStr::printf("SYSTEM CLOCKTIME %s %s", GetString(operand).c_str(), fileSystem->createTimeFromMinutes(threshold / 60.0f).c_str());
		else {
			if (subject.assetName.IsEmpty()) return "";
			return cweeStr::printf("%s %s %s %s %f", GetString(subject.first.first).c_str(), subject.assetName.c_str(), GetString(subject.second).c_str(), GetString(operand).c_str(), threshold);
		}
	};
	cweeStr Serialize() {
		cweeStr delim = ":RuleCondition:";
		cweeStr out;

		out.AddToDelimiter(subject.Serialize(), delim);
		out.AddToDelimiter((int)enumClassToInt(operand), delim);
		out.AddToDelimiter(threshold, delim);
		return out;
	};
	void Deserialize(const cweeStr& in) {
		cweeParser obj(in, ":RuleCondition:", true);

		subject.Deserialize(obj[0]);
		operand = static_cast<Optimization_Comparison>((int)obj[1]);
		threshold = (float)obj[2];
	};
};

static constexpr u64 controls_timeBuffer = 5.0f * 60.0f; // exists to give flexibility to hydraulic simulation requirements.
static constexpr float controls_floatBuffer = 0.001f; // exists to give flexibility to hydraulic simulation requirements.

/*!
Class that contains the equivalent logic of one EPAnet rule or control, but executed utilizing generic logic.
Does not impliment the result - simply reports the result. (Can have no result if there is no "if false" condition)
Based on option selected, will utilize EPAnet or Simulations to retrieve inputs and evaluate logic independently.
*/
class cweeRule {
public: // Public Static Methods
	/*!
	Will help generate list of RuleConditions
	*/
	static void AppendSequenceOfAnds(const assetValueIdentifier& toObserve, const Optimization_Comparison& criteria, float threshold, cweeThreadedList<RuleCondition>& SequenceOfAnds) {
		SequenceOfAnds.Append(BuildCondition(toObserve, criteria, threshold));
	};
	static Optimization_Comparison GuessOperandFromString(const cweeStr& input_string) {
		Optimization_Comparison out = Optimization_Comparison::EqualsTo;

		cweeStr comp = input_string;
		comp.Strip(' ');
		cweeThreadedList<cweeStr> options({
			"=", "==", "<>", "!=", ">", "<", "<=",  ">=", "IS", "Is", "is", "BELOW", "Below", "below", "ABOVE", "Above", "above", "EQUALS", "Equals", "equals"
			});
		comp = comp.BestMatch(options);

		if (comp == "=" || comp == "==" || comp == "IS" || comp == "Is" || comp == "is") {
			out = Optimization_Comparison::EqualsTo;
		}
		else if (comp == "<>" || comp == "!=") {
			out = Optimization_Comparison::NotEqual;
		}
		else if (comp == ">" || comp == "ABOVE" || comp == "Above" || comp == "above") {
			out = Optimization_Comparison::GreaterThan;
		}
		else if (comp == "<" || comp == "BELOW" || comp == "Below" || comp == "below") {
			out = Optimization_Comparison::LessThan;
		}
		else if (comp == "<=") {
			out = Optimization_Comparison::LessThanOrEqual;
		}
		else if (comp == ">=") {
			out = Optimization_Comparison::GreaterThanOrEqual;
		}
		else out = Optimization_Comparison::EqualsTo;

		return out;
	};
	static RuleCondition BuildCondition(const assetValueIdentifier& toObserve, const Optimization_Comparison& criteria, float threshold) {
		RuleCondition statement;			// "IF, AND, OR"
		statement.subject = toObserve;		// "PUMP 1 FLOW, RESERVOIR 5 HEAD, VALVE 15 SETTING"
		statement.operand = criteria;		// ">=, ==, !=, <"
		statement.threshold = threshold;	// "15, 0.0, 1000000"
		return statement;
	};

public: // Public construction / destruction
	cweeRule() {};
	~cweeRule() {};

public: // Public methods
	/*!
	Generate equivalent EPAnet logic (may not translate correctly, depending on what assets are used. May require user to tweak.)
	*/
	cweeStr getEPAnetLogic(const assetValueIdentifier& toBeControlled, bool staticHeader = false) {
		// Form:
		/*
		RULE #RandomNumberGenerated
		IF observation characteristic comparison threshold
		AND ..
		OR...
		AND ..
		THEN ..
		ELSE ..
		PRIORITY 1 ... INF
		*/

#if 0
		cweeStr rule;
		// RULE ...
		{
			if (staticHeader)
				rule.AddToDelimiter("RULE edms_Generated", "\n");
			else
				rule.AddToDelimiter(cweeStr::printf("RULE edms_Gen_%i", cweeRandomInt(100, 1000000)), "\n");
		}
		// IF ... AND ... OR ... AND ...
		{
			bool IF = true;
			for (auto& IF_OR : RuleConditions) {
				bool AND = false;
				for (auto& AND_LIST : IF_OR) {
					auto AND_LIST_RULE = AND_LIST.getEPAnetLogic();
					if (AND_LIST_RULE.IsEmpty()) {
						return "";
					} // bad rule - something broke

					if (IF) {
						// use "IF ..."
						rule.AddToDelimiter(cweeStr::printf("IF %s", AND_LIST_RULE.c_str()), "\n");
					}
					else if (AND) {
						// use "AND ..."
						rule.AddToDelimiter(cweeStr::printf("AND %s", AND_LIST_RULE.c_str()), "\n");
					}
					else {
						// use "OR ..."
						rule.AddToDelimiter(cweeStr::printf("OR %s", AND_LIST_RULE.c_str()), "\n");
					}

					AND = true;
					IF = false;
				}
				IF = false;
			}
		}
		// THEN ... 
		{
			if (hasTrueAnswer)
				rule.AddToDelimiter(cweeStr::printf("THEN %s %s %s = %f",
					GetString(toBeControlled.first.first).c_str(), toBeControlled.assetName.c_str(), GetString(toBeControlled.second).c_str(), trueAnswer), "\n");
		}
		// ELSE ...
		{
			if (hasFalseAnswer)
				rule.AddToDelimiter(cweeStr::printf("ELSE %s %s %s = %f",
					GetString(toBeControlled.first.first).c_str(), toBeControlled.assetName.c_str(), GetString(toBeControlled.second).c_str(), falseAnswer), "\n");
		}
		// PRIORITY ...
		{
			rule.AddToDelimiter("PRIORITY 5", "\n");
		}
		return rule;
#else
		cweeStr rules;

		for (cweeThreadedList<RuleCondition>& Rule : RuleConditions) {
			// float trueAnswer = 1.0f;
			// bool hasTrueAnswer = true;
			// float falseAnswer = 0.0f;
			// bool hasFalseAnswer = false;

			// each of these will be submitted as their own rule. 

			cweeStr rule;
			// RULE ...
			{
				if (staticHeader)
					rule.AddToDelimiter("RULE edms_Generated", "\n");
				else
					rule.AddToDelimiter(cweeStr::printf("RULE edms_Gen_%i", cweeRandomInt(100, 1000000)), "\n");
			}
			// IF ... AND ... AND ... (LACK OF ANO OR STATEMENT IS INTENTIONAL)
			{
				bool IF = true;
				bool AND = false;
				for (auto& AND_LIST : Rule) {
					auto AND_LIST_RULE = AND_LIST.getEPAnetLogic();
					if (AND_LIST_RULE.IsEmpty()) {
						return "";
					} // bad rule - something broke

					if (IF) {
						// use "IF ..."
						rule.AddToDelimiter(cweeStr::printf("IF %s", AND_LIST_RULE.c_str()), "\n");
					}
					else if (AND) {
						// use "AND ..."
						rule.AddToDelimiter(cweeStr::printf("AND %s", AND_LIST_RULE.c_str()), "\n");
					}
					AND = true;
					IF = false;
				}
			}
			// THEN ... 
			{
				if (hasTrueAnswer)
					rule.AddToDelimiter(cweeStr::printf("THEN %s %s %s = %f",
						GetString(toBeControlled.first.first).c_str(), toBeControlled.assetName.c_str(), GetString(toBeControlled.second).c_str(), trueAnswer), "\n");
				else
					return ""; // something broke
			}
			// ELSE ...
			{
				if (hasFalseAnswer)
					rule.AddToDelimiter(cweeStr::printf("ELSE %s %s %s = %f",
						GetString(toBeControlled.first.first).c_str(), toBeControlled.assetName.c_str(), GetString(toBeControlled.second).c_str(), falseAnswer), "\n");
			}
			// PRIORITY ...
			{
				rule.AddToDelimiter("PRIORITY 5", "\n");
			}

			rules.AddToDelimiter(rule, "\n\n");
		}
		return rules;

#endif



	};

	/*!
	Generate the cweeRule automatically (I can only do this if I can access worldDef from this file...)
	*/
	void GenerateRule(const cweeStr& epanetControlOrRuleString, void* world);

	/*!
	Will append RuleConditions, but overrides the settingIfTrue. (Uncecessary if successful, above)
	*/
	void SetupWithEPAnetControl(float settingIfTrue, const assetValueIdentifier& toObserve, const Optimization_Comparison& criteria, float threshold) {
		RuleCondition statement;
		statement.operand = criteria;
		statement.subject = toObserve;
		statement.threshold = threshold;
		cweeThreadedList<RuleCondition> ListOfIfs; ListOfIfs.Append(statement);
		RuleConditions.Append(ListOfIfs);

		hasTrueAnswer = true;
		hasFalseAnswer = false;
		trueAnswer = settingIfTrue;
	};

	/*!
	Will append RuleConditions, but overrides the settingIfTrue. (Uncecessary if successful, above)
	*/
	void SetupWithEPAnetRule(const cweeThreadedList<RuleCondition>& SequenceOfAnds, float settingIfTrue = -cweeMath::INF, float settingIfFalse = -cweeMath::INF) {
		RuleConditions.Append(SequenceOfAnds);
		if (settingIfTrue != -cweeMath::INF) {
			hasTrueAnswer = true;
			trueAnswer = settingIfTrue;
		}
		if (settingIfFalse != -cweeMath::INF) {
			hasFalseAnswer = true;
			falseAnswer = settingIfFalse;
		}
	};

	/*!
	Returns true if this cweeRule was successfully evaulated, with result to 'out'. Returns false if not successfully evaluated - 'out' may not be safe to read then.
	*/
	bool Evaluate(const time_t& time, float& out, void* world);
	/*!
	Returns true if this cweeRule was successfully evaulated, with result to 'out'. Returns false if not successfully evaluated - 'out' may not be safe to read then.
	*/
	bool Evaluate(const time_t& time, float& out, void* world, int EPAnetProject);

	void SwapAssetValueIdentifiers(const assetValueIdentifier& toFind, const assetValueIdentifier& replaceWith) {
		for (auto& x : RuleConditions)
			for (auto& y : x)
				if (y.subject == toFind)
					y.subject = replaceWith;
	};

public: // data	
	/*!
	List(rows) < List(columns) > RuleConditions.
	All columns must be true for a row to be true.
	If any row is true, the entire rule returns true.
	If no row is true, the entire rule returns false.

	Example:
		IF RESERVOIR 5's HEAD >= 155			// Row 1, Column 1
		AND PUMP 33's FLOW <= 0					// Row 1, Column 2
		OR RESERVOIR 5's HEAD < 5				// Row 2, Column 1
		AND CLOCKTIME > 5:00					// Row 2, Column 2
		OR RESERVOIR 5's HEAD >= 20				// Row 3, Column 1
		AND CLOCKTIME < 23:00					// Row 3, Column 2
		AND CLOCKTIME >= 1:00					// Row 3, Column 3
		THEN (owner from cweeControl) is OPEN	// hasTrueAnswer = true, trueAnswer = 1
		ELSE (owner from cweeControl) is CLOSED // hasFalseAnswer = true, falseAnswer = 0
	*/
	cweeThreadedList<cweeThreadedList<RuleCondition>> RuleConditions;
	float trueAnswer = 1.0f;
	bool hasTrueAnswer = true;
	float falseAnswer = 0.0f;
	bool hasFalseAnswer = false;



	cweeStr Serialize() {
		cweeStr delim = ":cweeRule:";
		cweeStr out;

		out.AddToDelimiter((int)hasTrueAnswer, delim);
		out.AddToDelimiter((float)trueAnswer, delim);
		out.AddToDelimiter((int)hasFalseAnswer, delim);
		out.AddToDelimiter((float)falseAnswer, delim);
		cweeStr temp; for (auto& x : RuleConditions) {
			cweeStr temp2; for (auto& y : x) {
				temp2.AddToDelimiter(y.Serialize(), ":cweeRule_RuleConditions_In:");
			} temp.AddToDelimiter(temp2, ":cweeRule_RuleConditions:");
		} out.AddToDelimiter(temp, delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {
		cweeParser obj(in, ":cweeRule:", true);

		hasTrueAnswer = (bool)(int)obj[0];
		trueAnswer = (float)obj[1];
		hasFalseAnswer = (bool)(int)obj[2];
		falseAnswer = (float)obj[3];
		RuleConditions.Clear();
		cweeParser z(obj[4], ":cweeRule_RuleConditions:", true); for (auto& w : z) {
			cweeThreadedList<RuleCondition> temp;
			cweeParser a(w, ":cweeRule_RuleConditions_In:", true); for (auto& b : a) {
				RuleCondition temp2; temp2.Deserialize(b);
				temp.Append(temp2);
			}
			RuleConditions.Append(temp);
		}
	};

private: // rule evaluation tools
	bool evaluateRuleCondition(const RuleCondition& in, const time_t& time, void* world);
	bool evaluateRuleCondition(const cweeThreadedList<RuleCondition>& in, const time_t& time, void* world);
	bool evaluateRuleCondition(const cweeThreadedList < cweeThreadedList<RuleCondition> >& in, const time_t& time, void* world);
	bool evaluateRuleCondition(const time_t& time, void* world);

	bool evaluateRuleCondition(const RuleCondition& in, const time_t& time, void* world, int EPAnetProject) {
		float value = 0;

		if (in.subject.first.first == asset_t::TIME) {
			// threshold MUST represent the seconds in a day. i.e. 0 = start of day, 43200 = noon, 86400 = end of day
			cweeTime tmp = fileSystem->localtime(time);
			value = tmp.tm_sec() + tmp.tm_min() * 60 + tmp.tm_hour() * 60 * 60;
		}
		else {
			if (EPAnet->isNode(in.subject.first.first)) {
				value = EPAnet->getCurrentHydraulicValue(EPAnetProject, EPAnet->getNodeindex(EPAnetProject, in.subject.assetName), in.subject.second);
			}
			else {
				int index = EPAnet->getlinkIndex(EPAnetProject, in.subject.assetName);
				value = EPAnet->getCurrentHydraulicValue(EPAnetProject, index, in.subject.second);
			}
		}
		switch (in.operand) {
		case Optimization_Comparison::EqualsTo: {
			if (in.subject.first.first == asset_t::TIME) {
				return ((value > (in.threshold)) && (value < (in.threshold + controls_timeBuffer)));
				// return ((value > (in.threshold - controls_timeBuffer)) && (value < (in.threshold + controls_timeBuffer)));
			}
			else {
				return ((value > (in.threshold - controls_floatBuffer)) && (value < (in.threshold + controls_floatBuffer)));
			}

			break;
		}
		case Optimization_Comparison::GreaterThan: {
			return (value > in.threshold);
			break;
		}
		case Optimization_Comparison::GreaterThanOrEqual: {
			return (value >= in.threshold);
			break;
		}
		case Optimization_Comparison::LessThan: {
			return (value < in.threshold);
			break;
		}
		case Optimization_Comparison::LessThanOrEqual: {
			return (value <= in.threshold);
			break;
		}
		case Optimization_Comparison::NotEqual: {
			if (in.subject.first.first == asset_t::TIME) {
				return ((value < (in.threshold)) || (value > (in.threshold + controls_timeBuffer)));
			}
			else {
				return ((value < (in.threshold - controls_floatBuffer)) || (value > (in.threshold + controls_floatBuffer)));
			}
			break;
		}
		default: return false;
		}

	};;
	bool evaluateRuleCondition(const cweeThreadedList<RuleCondition>& in, const time_t& time, void* world, int EPAnetProject);
	bool evaluateRuleCondition(const cweeThreadedList < cweeThreadedList<RuleCondition> >& in, const time_t& time, void* world, int EPAnetProject);
	bool evaluateRuleCondition(const time_t& time, void* world, int EPAnetProject);

	/*!
	Rules engine. Recursive method of evaluating bools.
	*/
	class RulesEngine
	{
	public:
		RulesEngine() : Not(*this) {}

		void If(bool sufficientCondition) { sufficientConditions.push_back(sufficientCondition); }
		void NotIf(bool preventingCondition) { preventingConditions.push_back(preventingCondition); }

		class PreventingRulesEngine
		{
		public:
			explicit PreventingRulesEngine(RulesEngine& rulesEngine) : rulesEngine_(rulesEngine) {}
			void If(bool preventingCondition) { rulesEngine_.NotIf(preventingCondition); }
		private:
			RulesEngine& rulesEngine_;
		};
		PreventingRulesEngine Not;

		bool operator()() const
		{
			auto isTrue = [](bool b) { return b; };
			return std::any_of(begin(sufficientConditions), end(sufficientConditions), isTrue)
				&& std::none_of(begin(preventingConditions), end(preventingConditions), isTrue);
		}

	private:
		std::deque<bool> sufficientConditions;
		std::deque<bool> preventingConditions;
	};
};

struct UserInput_Output {
	cweeThreadedList<cweeRule> rules;
	::controlGenerationStatus status = ::controlGenerationStatus::notStarted;

	cweeStr Serialize() {
		if (status == controlGenerationStatus::notStarted) return "";

		cweeStr delim = ":UserInput_Output:";
		cweeStr out;

		out.AddToDelimiter(enumClassToInt(status), delim);
		cweeStr temp; for (auto& x : rules) temp.AddToDelimiter(x.Serialize(), ":UserInput_Output_rules:"); out.AddToDelimiter(temp, delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {
		if (in.IsEmpty()) return;

		cweeParser obj(in, ":UserInput_Output:", true);

		status = static_cast<controlGenerationStatus>((int)obj[0]);
		if (status == controlGenerationStatus::started) status = controlGenerationStatus::notStarted; // TO-DO; CAUSED DEADLOCK WHEN WE LOADED AFTER STARTING OPTIMIZATION, SAVED, AND LOADED. 
		rules.Clear(); cweeParser z(obj[1], ":UserInput_Output_rules:", true); for (auto& w : z) { cweeRule temp; temp.Deserialize(w); rules.Append(temp); }
	};
	void SwapAssetValueIdentifiers(const assetValueIdentifier& toFind, const assetValueIdentifier& replaceWith) {
		for (auto& x : rules)
			x.SwapAssetValueIdentifiers(toFind, replaceWith);
	};

};
struct PID_Output {
	cweeEng::pidLogic logic;
	assetValueIdentifier simulationOutput; // i.e. pump flow
	float setpoint = 60; // i.e. desired pump flow. 
	::controlGenerationStatus status = ::controlGenerationStatus::notStarted;

	void SwapAssetValueIdentifiers(const assetValueIdentifier& toFind, const assetValueIdentifier& replaceWith) {
		if (simulationOutput == toFind) simulationOutput = replaceWith;
	};
	cweeStr Serialize() {
		if (status == controlGenerationStatus::notStarted) return "";

		cweeStr delim = ":PID_Output:";
		cweeStr out;

		out.AddToDelimiter(logic.Serialize(), delim);
		out.AddToDelimiter(enumClassToInt(status), delim);
		out.AddToDelimiter(simulationOutput.Serialize(), delim);
		out.AddToDelimiter(setpoint, delim);

		return out;
	};
	void Deserialize(const cweeStr& in) {
		if (in.IsEmpty()) return;

		cweeParser obj(in, ":PID_Output:", true);

		logic.Deserialize(obj[0]);
		status = static_cast<controlGenerationStatus>((int)obj[1]); if (status == controlGenerationStatus::started) status = controlGenerationStatus::notStarted; // TO-DO; CAUSED DEADLOCK WHEN WE LOADED AFTER STARTING OPTIMIZATION, SAVED, AND LOADED. 
		simulationOutput.Deserialize(obj[2]);
		setpoint = (float)obj[3];
	};
};
struct TrainedModel_Output {
	Pattern trainedResult; // has the timeseries RESULT OF THE TRAINING, but does not have the 'machine logic' within.
	Pattern trainedPattern; // has the timeseries we trained ON, and the 'machine logic' within.
	vec2 err; // Training R^2, Testing R^2	
	controlGenerationStatus status = controlGenerationStatus::notStarted;

	cweeStr Serialize() {
		if (status == controlGenerationStatus::notStarted) return "";

		cweeStr delim = ":TrainedModel_Output:";
		cweeStr out;

		out.AddToDelimiter(trainedPattern.Serialize(), delim);
		out.AddToDelimiter(err.x, delim);
		out.AddToDelimiter(err.y, delim);
		out.AddToDelimiter(enumClassToInt(status), delim);
		out.AddToDelimiter(trainedResult.Serialize(), delim);
		return out;
	};
	void Deserialize(const cweeStr& in) {
		if (in.IsEmpty()) return;

		cweeParser obj(in, ":TrainedModel_Output:", true);

		trainedPattern.Deserialize(obj[0]);
		err = vec2((float)obj[1], (float)obj[2]);
		status = static_cast<controlGenerationStatus>((int)obj[3]);
		if (status == controlGenerationStatus::started) status = controlGenerationStatus::notStarted; // TO-DO; CAUSED DEADLOCK WHEN WE LOADED AFTER STARTING OPTIMIZATION, SAVED, AND LOADED. 
		trainedResult.Deserialize(obj[4]);
	};

};
struct Optimization_Output {
	cweeThreadedList<float> inputtedSettings;
	assetValueIdentifier inputtedReservoirHead;
	int optimizationHistoryID = -1;

	cweeThreadedList<cweeRule> optimizedRules;
	controlGenerationStatus status = controlGenerationStatus::notStarted;

	cweeStr Serialize() {
		if (status == controlGenerationStatus::notStarted) return "";

		cweeStr delim = ":UserInput_Output:";
		cweeStr out;

		out.AddToDelimiter(enumClassToInt(status), delim);
		cweeStr temp; for (auto& x : optimizedRules) temp.AddToDelimiter(x.Serialize(), ":UserInput_Output_optimizedRules:"); out.AddToDelimiter(temp, delim);
		cweeStr temp2; for (auto& x : inputtedSettings) temp2.AddToDelimiter(x, ":UserInput_Output_inputtedSettings:"); out.AddToDelimiter(temp2, delim);
		out.AddToDelimiter(inputtedReservoirHead.Serialize(), delim);
		out.AddToDelimiter(optimizationHistoryID, delim);
		return out;
	};
	void Deserialize(const cweeStr& in) {
		if (in.IsEmpty()) return;

		cweeParser obj(in, ":UserInput_Output:", true);

		status = static_cast<controlGenerationStatus>((int)obj[0]);
		if (status == controlGenerationStatus::started) status = controlGenerationStatus::notStarted; // TO-DO; CAUSED DEADLOCK WHEN WE LOADED AFTER STARTING OPTIMIZATION, SAVED, AND LOADED. 
		optimizedRules.Clear(); cweeParser z(obj[1], ":UserInput_Output_optimizedRules:", true); for (auto& w : z) { cweeRule temp; temp.Deserialize(w); optimizedRules.Append(temp); }
		inputtedSettings.Clear(); cweeParser r(obj[2], ":UserInput_Output_inputtedSettings:", true); for (auto& w : r) { inputtedSettings.Append((float)w); }
		inputtedReservoirHead.Deserialize(obj[3]);
		optimizationHistoryID = (int)obj[4];
	};
	void SwapAssetValueIdentifiers(const assetValueIdentifier& toFind, const assetValueIdentifier& replaceWith) {
		for (auto& x : optimizedRules)
			x.SwapAssetValueIdentifiers(toFind, replaceWith);

		if (inputtedReservoirHead == toFind) inputtedReservoirHead = replaceWith;
	};
};

/*!
Supports the execution of generic logic on hydraulic assets to determine how to control assets in EPAnet in real-time.
Optionally uses static controls as input, machine learned input, or optimized controls as input.
*/
class cweeControl_RG {
public: // Declerations
	cweeControl_RG() {
		user_Overrides.Lock();
		user_Overrides.UnsafeRead()->SetInterpolationType(interpolation_t::IT_LEFT_CLAMP);
		user_Overrides.UnsafeRead()->AddUniqueValue(0, -cweeMath::INF);
		user_Overrides.UnsafeRead()->AddUniqueValue(std::numeric_limits<u64>::max(), -cweeMath::INF);
		user_Overrides.Unlock();
	};
	~cweeControl_RG() {};

public: // Evaluate the cweeControl. Determine if evaluation gave a result, and if so, retrieve it. 
	/*!
	Evaluate cweeControl. Returns true and occupies 'out' if evaluated to an action. Queries the cweeSimulations for inputs.
	'world' must be a pointer or reference to a world. {  i.e. (void*)(&worldDef)  }
	*/
	bool evaluateControl(const time_t& time, void* world, float& out);
	/*!
	Evaluate cweeControl. Returns true and occupies 'out' if evaluated to an action. Queries the EPAnet project for inputs.
	'world' must be a pointer or reference to a world. {  i.e. (void*)(&worldDef)  }
	*/
	bool evaluateControl(const time_t& time, void* world, float& out, int EPAnetProject, bool checkUnecessaryAssignments = true);
private:
	bool evaluateControl_UserInput(const time_t& time, void* world, float& out, int EPAnetProject, bool finishedState = true, bool checkUnecessaryAssignments = true);
	bool evaluateControl_PID(const time_t& time, void* world, float& out, int EPAnetProject, bool finishedState = true, bool checkUnecessaryAssignments = true);
	bool evaluateControl_TrainedModel(const time_t& time, void* world, float& out, int EPAnetProject, bool finishedState = true, bool checkUnecessaryAssignments = true);
	bool evaluateControl_Optimization(const time_t& time, void* world, float& out, int EPAnetProject, bool finishedState = true, bool checkUnecessaryAssignments = true);

public: // Serialize && Deserialize
	cweeStr Serialize(int option = -1) {
		cweeStr delim = ":CWEECNTRL_in_DELIM:";
		cweeStr out;

		out.AddToDelimiter(owner->Serialize(), delim);
		out.AddToDelimiter(enumClassToInt(controlType.Read()), delim);
		out.AddToDelimiter(enumClassToInt(controlMethod.Read()), delim);

		out.AddToDelimiter(opt_Output.Read().status == controlGenerationStatus::finished ? opt_Input.Read().Serialize() : cweeStr(""), delim);
		out.AddToDelimiter(ml_Output.Read().status == controlGenerationStatus::finished ? ml_Input.Read().Serialize() : cweeStr(""), delim);
		out.AddToDelimiter(static_Output.Read().status == controlGenerationStatus::finished ? static_Input.Read().Serialize() : cweeStr(""), delim);

		out.AddToDelimiter(user_Overrides.Read().Serialize(), delim);

		out.AddToDelimiter(opt_Output.Read().Serialize(), delim);
		out.AddToDelimiter(ml_Output.Read().Serialize(), delim);
		out.AddToDelimiter(static_Output.Read().Serialize(), delim);

		out.AddToDelimiter(overrideFit.Read().x, delim);
		out.AddToDelimiter(overrideFit.Read().y, delim);
		out.AddToDelimiter(overrideFit.Read().z, delim);

		out.AddToDelimiter(pid_Output.Read().status == controlGenerationStatus::finished ? pid_Input.Read().Serialize() : cweeStr(""), delim);
		out.AddToDelimiter(pid_Output.Read().Serialize(), delim);

		return out;
	};
	void Deserialize(cweeStr& in) {
		cweeParser obj(in, ":CWEECNTRL_in_DELIM:", true);
		in.Clear();

		owner.Lock();  owner.UnsafeRead()->Deserialize(obj[0]); owner.Unlock();
		controlType = static_cast<AssetValueControlType>((int)obj[1]);
		controlMethod = static_cast<ControlGenerationMethod>((int)obj[2]);

		opt_Input.Lock(); opt_Input.UnsafeRead()->Deserialize(obj[3]); opt_Input.Unlock();
		ml_Input.Lock(); ml_Input.UnsafeRead()->Deserialize(obj[4]); ml_Input.Unlock();
		static_Input.Lock(); static_Input.UnsafeRead()->Deserialize(obj[5]); static_Input.Unlock();

		user_Overrides.Lock(); user_Overrides.UnsafeRead()->Deserialize(obj[6]); user_Overrides.Unlock();

		opt_Output.Lock(); opt_Output.UnsafeRead()->Deserialize(obj[7]); opt_Output.Unlock();
		ml_Output.Lock(); ml_Output.UnsafeRead()->Deserialize(obj[8]); ml_Output.Unlock();
		static_Output.Lock(); static_Output.UnsafeRead()->Deserialize(obj[9]); static_Output.Unlock();

		overrideFit = vec3((float)obj[10], (float)obj[11], (float)obj[12]);

		pid_Input.Lock(); pid_Input.UnsafeRead()->Deserialize(obj[13]); pid_Input.Unlock();
		pid_Output.Lock(); pid_Output.UnsafeRead()->Deserialize(obj[14]); pid_Output.Unlock();

		controlPotentiallyActive = (bool)((user_Overrides->GetNumValues() > 2) || (isControlActive()));

	};

public: // static method to determine the EPAnet control / rule to be generated or help in some other way.
	/*!
	Adds a Rule or Control to the EPAnet project directly to set the specified characteristic to the provided value ASAP.
	Only works for the status && setting of pipes && pumps && valves.
	*/
	static void addEPAnetLogic(int EPAnetProj, const assetValueIdentifier& toBeControlled, float value) {
		if (toBeControlled.second == _STATUS_) {
			EPAnet->setLinkValue(EPAnetProj, EPAnet->getlinkIndex(EPAnetProj, toBeControlled.assetName), EN_STATUS, value);
		}
		else { // Setting
			EPAnet->setLinkValue(EPAnetProj, EPAnet->getlinkIndex(EPAnetProj, toBeControlled.assetName), EN_SETTING, value);
		}
	};

	/*!
	Generate equivalent EPAnet logic (may not translate correctly, depending on what assets are used. May require user to tweak.)
	*/
	static cweeStr getEPAnetLogic(const assetValueIdentifier& toBeControlled, cweeThreadedList<cweeRule> rules) {
		// Form: (repeated for each cweeRule)
		/*
		RULE #RandomNumberGenerated
		IF observation characteristic comparison threshold
		AND ..
		OR...
		AND ..
		THEN ..
		ELSE ..
		PRIORITY 1 ... INF
		*/

		cweeStr out;
		for (auto& x : rules) {
			auto rule = x.getEPAnetLogic(toBeControlled);
			if (!rule.IsEmpty())
				out.AddToDelimiter(rule, "\n\n");
		}

		return out;
	};
	cweeStr getEPAnetLogic() {
		// Form: (repeated for each cweeRule)
		/*
		RULE #RandomNumberGenerated
		IF observation characteristic comparison threshold
		AND ..
		OR...
		AND ..
		THEN ..
		ELSE ..
		PRIORITY 1 ... INF
		*/

		cweeStr out;
		switch (this->controlMethod) {
		case ControlGenerationMethod::UserInput: {
			cweeThreadedList<cweeRule> rules;
			static_Output.Lock(); rules = static_Output.UnsafeRead()->rules; static_Output.Unlock();
			out = getEPAnetLogic(this->owner, rules);
			break;
		}
		case ControlGenerationMethod::TrainedModel: {
			// nada
			break;
		}
		case ControlGenerationMethod::Optimization: {
			cweeThreadedList<cweeRule> rules;
			opt_Output.Lock(); rules = opt_Output.UnsafeRead()->optimizedRules; opt_Output.Unlock();
			out = getEPAnetLogic(this->owner, rules);
			break;
		}
		case ControlGenerationMethod::PID: {
			// nada
			break;
		}
		}

		return out;
	};

	/*!
	Use the inputs to generate an Optimization_Output object that follows this logic: (Note, owner is set in actual cweeControl)
		<owner> <settingTankLow> If <tankHydraulicHead> Below <TankLowHydraulicHead>		// assetValueIdentifier must be for the reservoir head
		<owner> <settingTankHigh> If <tankHydraulicHead> Above <TankHighHydraulicHead>		// assetValueIdentifier must be for the reservoir head
		<owner> <settingTimeLow> AT CLOCKTIME <timeLowSeconds>								// timeLowSeconds must be between [0, 86400]
		<owner> <settingTimeHigh> AT CLOCKTIME <timeHighSeconds>							// timeHighSeconds must be between [0, 86400]
	*/
	static Optimization_Output generateOutputForOptimizedControl(
		float settingTimeLow, float settingTimeHigh, float timeLowSeconds, float timeHighSeconds,
		float settingTankLow, float settingTankHigh, float TankLowHydraulicHead, float TankHighHydraulicHead,
		bool skipReservoirRules = false, bool skipTimeRules = false, assetValueIdentifier tankHydraulicHead = assetValueIdentifier()
	) {
		Optimization_Output out;
		cweeThreadedList<cweeRule> rules;
		assetValueIdentifier timeID; timeID.first.first = asset_t::TIME;

		cweeRule rule1, rule2, rule3, rule4;
		// Rule1
		if (!skipReservoirRules && tankHydraulicHead.first.first == asset_t::RESERVOIR) {
			cweeThreadedList<RuleCondition> ruleList;
			cweeRule::AppendSequenceOfAnds(tankHydraulicHead, Optimization_Comparison::LessThan, TankLowHydraulicHead, ruleList);
			rule1.RuleConditions.Append(ruleList);
			rule1.hasTrueAnswer = true;
			rule1.trueAnswer = settingTankLow;
		}
		// Rule2
		if (!skipReservoirRules && tankHydraulicHead.first.first == asset_t::RESERVOIR) {
			cweeThreadedList<RuleCondition> ruleList;
			cweeRule::AppendSequenceOfAnds(tankHydraulicHead, Optimization_Comparison::GreaterThan, TankHighHydraulicHead, ruleList);
			rule2.RuleConditions.Append(ruleList);
			rule2.hasTrueAnswer = true;
			rule2.trueAnswer = settingTankHigh;
		}
		// Rule3
		if (!skipTimeRules && timeLowSeconds >= 0 && timeLowSeconds <= 60 * 60 * 24) {
			cweeThreadedList<RuleCondition> ruleList;
			cweeRule::AppendSequenceOfAnds(timeID, Optimization_Comparison::EqualsTo, timeLowSeconds, ruleList);
			rule3.RuleConditions.Append(ruleList);
			rule3.hasTrueAnswer = true;
			rule3.trueAnswer = settingTimeLow;
		}
		// Rule4
		if (!skipTimeRules && timeHighSeconds >= 0 && timeHighSeconds <= 60 * 60 * 24) {
			cweeThreadedList<RuleCondition> ruleList;
			cweeRule::AppendSequenceOfAnds(timeID, Optimization_Comparison::EqualsTo, timeHighSeconds, ruleList);
			rule4.RuleConditions.Append(ruleList);
			rule4.hasTrueAnswer = true;
			rule4.trueAnswer = settingTimeHigh;
		}
		if (!skipReservoirRules && tankHydraulicHead.first.first == asset_t::RESERVOIR) {
			out.optimizedRules.Append(rule1);
			out.optimizedRules.Append(rule2);
		}
		if (!skipTimeRules && timeLowSeconds >= 0 && timeLowSeconds <= 60 * 60 * 24) {
			out.optimizedRules.Append(rule3);
		}
		if (!skipTimeRules && timeHighSeconds >= 0 && timeHighSeconds <= 60 * 60 * 24) {
			out.optimizedRules.Append(rule4);
		}
		out.inputtedReservoirHead = tankHydraulicHead;
		out.inputtedSettings = { settingTimeLow, settingTimeHigh, timeLowSeconds, timeHighSeconds, settingTankLow, settingTankHigh, TankLowHydraulicHead, TankHighHydraulicHead };
		out.status = controlGenerationStatus::finished;

		return out;
	};

	static bool WasConstraintViolated(const Optimization_Constraint& x, void* world, const time_t& startDate = 0, const time_t& endDate = std::numeric_limits<time_t>::max());
	static Pattern GetObjectiveValues(const Optimization_ObjectiveValue& x, void* world, const time_t& startDate, const time_t& endDate);
	static float GetObjectiveValue(const Optimization_ObjectiveValue& x, void* world, const time_t& startDate = 0, const time_t& endDate = std::numeric_limits<time_t>::max());
	static float GetObjectiveValue(const Optimization_Aggregation& x, Pattern& pat, const time_t& startDate = 0, const time_t& endDate = std::numeric_limits<time_t>::max());

public: // public methods: cweeControl characteristics && settings
	void setAssociatedAssetValueIdentifier(const assetValueIdentifier& which) {
		owner = which;
	};
	const assetValueIdentifier& getAssociatedAssetValueIdentifier() {
		return *owner.UnsafeRead();
	};

	void setControlType(const AssetValueControlType& which) {
		controlType = which;
	};
	AssetValueControlType getControlType() {
		return controlType;
	};
	void setControlMethod(const ControlGenerationMethod& which) {
		controlMethod = which;
	};
	ControlGenerationMethod getControlMethod() {
		return controlMethod;
	};
	void setControlStatus(const ControlGenerationMethod& which, const controlGenerationStatus& status) {
		switch (which) {
		case ControlGenerationMethod::UserInput: {
			static_Output.Lock();  static_Output.UnsafeRead()->status = status; static_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::PID: {
			pid_Output.Lock();  pid_Output.UnsafeRead()->status = status; pid_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::TrainedModel: {
			ml_Output.Lock();  ml_Output.UnsafeRead()->status = status; ml_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::Optimization: {
			opt_Output.Lock();  opt_Output.UnsafeRead()->status = status; opt_Output.Unlock();
			break;
		}
		}
	};
	controlGenerationStatus getControlStatus(const ControlGenerationMethod& which) {
		controlGenerationStatus out = controlGenerationStatus::notStarted;
		switch (which) {
		case ControlGenerationMethod::UserInput: {
			static_Output.Lock();  out = static_Output.UnsafeRead()->status; static_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::PID: {
			pid_Output.Lock();   out = pid_Output.UnsafeRead()->status; pid_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::TrainedModel: {
			ml_Output.Lock();   out = ml_Output.UnsafeRead()->status; ml_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::Optimization: {
			opt_Output.Lock();   out = opt_Output.UnsafeRead()->status; opt_Output.Unlock();
			break;
		}
		}
		return out;
	};
	bool isControlActive() {
		bool out = true;

		if (out) { // otherwise nothing else matters 
			static_Output.Lock(); pid_Output.Lock(); ml_Output.Lock(); opt_Output.Lock();
			out = (static_Output.UnsafeRead()->status == controlGenerationStatus::finished
				|| pid_Output.UnsafeRead()->status == controlGenerationStatus::finished
				|| ml_Output.UnsafeRead()->status == controlGenerationStatus::finished
				|| opt_Output.UnsafeRead()->status == controlGenerationStatus::finished);

			static_Output.Unlock(); pid_Output.Unlock(); ml_Output.Unlock(); opt_Output.Unlock();
		}

		return out;
	};
	bool hasOverrides() {
		bool out = false;

		user_Overrides.Lock();   out = (user_Overrides.UnsafeRead()->GetNumValues() > 2); user_Overrides.Unlock();

		return out;
	};
	cweeThreadedList< assetValueIdentifier > getObservedAssets() const {
		cweeThreadedList< assetValueIdentifier > out;

		out.AddUnique(owner);

		switch (controlMethod) {
		case ControlGenerationMethod::UserInput: {
			static_Output.Lock();
			for (auto& x : static_Output.UnsafeRead()->rules) {
				for (auto& y : x.RuleConditions) {
					for (auto& z : y) {
						out.AddUnique(z.subject);
					}
				}
			}
			static_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::PID: {
			pid_Output.Lock();
			out.AddUnique(pid_Output.UnsafeRead()->simulationOutput);
			pid_Output.Unlock();
			break;
		}
		case ControlGenerationMethod::TrainedModel: {

			// to-do

			break;
		}
		case ControlGenerationMethod::Optimization: {

			// to-do

			break;
		}
		}

		return out;
	};

public: // public methods: input modifiers
	/*!
	Completely override the static input. Immediately && automatically creates an output based on input.
	*/
	void overrideWithInput(const UserInput_Input& in, void* world);
	void overrideWithInput(const UserInput_InputType& type, const cweeStr& userSpecification, void* world) {
		UserInput_Input temp;
		temp.type = type;
		temp.userSpecification = userSpecification;
		overrideWithInput(temp, world);
	};

	/*!
	Completely override the PID input. Immediately && automatically creates an output based on input.
	*/
	void overrideWithInput(const PID_Input& in, void* world);
	void overrideWithInput(const assetValueIdentifier& simulationOutput, float setpoint, float minSetting, float maxSetting, void* world) {
		PID_Input temp;
		temp.simulationOutput = simulationOutput;
		temp.setpoint = setpoint;
		temp.minSetting = minSetting;
		temp.maxSetting = maxSetting;
		overrideWithInput(temp, world);
	};

	/*!
	Completely override the machine learning input. Immediately && automatically creates an output based on input.
	*/
	void overrideWithInput(TrainedModel_Input& in, void* world);
	void overrideWithInput(bool useDefault, cweeThreadedList<float> operatingSetPoints, time_t startDate, time_t endDate, cweeThreadedList<TrainedModel_PredictionInput> predictionInputs, TrainedModel_ErrorMetric errMetric, void* world) {
		TrainedModel_Input temp;
		temp.useDefault = useDefault;
		temp.operatingSetPoints = operatingSetPoints;
		temp.startDate = startDate;
		temp.endDate = endDate;
		temp.predictionInputs = predictionInputs;
		temp.errMetric = errMetric;
		overrideWithInput(temp, world);
	};

	/*!
	Completely override the optimization input. DOES NOT CREATE AN OUTPUT AUTOMATICALLY.
	--- USER IS EXPECTED TO HANDLE OPTIMIZATION OUTPUTS SEPERATELY ---
	*/
	void overrideWithInput(const Optimization_Input& in);

	struct deferredOverrideControlPackage {
		ControlGenerationMethod		which;
		UserInput_Input				staticInput;
		PID_Input					pidInput;
		TrainedModel_Input			mlInput;
		Optimization_Input			optInput;

		cweeStr						scenarioName;
		assetValueIdentifier		whichAsset;
	};
	static void deferredOverride(deferredOverrideControlPackage* io);

	/*!
	As long as an 'owner' has already been established, this method will update the static user controls with a EPAnet compliant rule or control correctly.
	*/
	void appendUserInput(const cweeStr& in, void* worldDefPtr);

	void getControlInput(Optimization_Input& out) {
		out = opt_Input;
	};
	void getControlInput(TrainedModel_Input& out) {
		out = ml_Input;
	};
	void getControlInput(UserInput_Input& out) {
		out = static_Input;
	};
	void getControlInput(PID_Input& out) {
		out = pid_Input;
	};

public: // public methods: output modifiers
	/*!
	Required for optimizaiton - user must supply the optimization result.
	*/
	void setControlOutput(const Optimization_Output& in) {
		opt_Output = in;
		setControlStatus(ControlGenerationMethod::Optimization, controlGenerationStatus::finished);
		controlPotentiallyActive = true;
		SimulationNeedsUpdate = SimulationNeedsUpdate.Read() > 0 ? Min((u64)SimulationNeedsUpdate.Read() + cweeRandomFloat(-60, 60), (u64)(fileSystem->getCurrentTime() - (24 * 60 * 60))) : fileSystem->getCurrentTime() - (24 * 60 * 60);
	};
	/*!
	Allow for user-manipulation of the output results.
	*/
	void setControlOutput(const TrainedModel_Output& in) {
		ml_Output = in;
		setControlStatus(ControlGenerationMethod::TrainedModel, controlGenerationStatus::finished);
		controlPotentiallyActive = true;
		SimulationNeedsUpdate = SimulationNeedsUpdate.Read() > 0 ? Min((u64)SimulationNeedsUpdate.Read() + cweeRandomFloat(-60, 60), (u64)(fileSystem->getCurrentTime() - (24 * 60 * 60))) : fileSystem->getCurrentTime() - (24 * 60 * 60);
	};
	/*!
	Allow for user-manipulation of the output results.
	*/
	void setControlOutput(const UserInput_Output& in) {
		static_Output = in;
		setControlStatus(ControlGenerationMethod::UserInput, controlGenerationStatus::finished);
		controlPotentiallyActive = true;
		SimulationNeedsUpdate = SimulationNeedsUpdate.Read() > 0 ? Min((u64)SimulationNeedsUpdate.Read() + cweeRandomFloat(-60, 60), (u64)(fileSystem->getCurrentTime() - (24 * 60 * 60))) : fileSystem->getCurrentTime() - (24 * 60 * 60);
	};
	void setControlOutput(const PID_Output& in) {
		pid_Output = in;
		setControlStatus(ControlGenerationMethod::PID, controlGenerationStatus::finished);
		controlPotentiallyActive = true;
		SimulationNeedsUpdate = SimulationNeedsUpdate.Read() > 0 ? Min((u64)SimulationNeedsUpdate.Read() + cweeRandomFloat(-60, 60), (u64)(fileSystem->getCurrentTime() - (24 * 60 * 60))) : fileSystem->getCurrentTime() - (24 * 60 * 60);
	};

	void getControlOutput(Optimization_Output& out) {
		out = opt_Output;
	};
	void getControlOutput(TrainedModel_Output& out) {
		out = ml_Output;
	};
	void getControlOutput(UserInput_Output& out) {
		out = static_Output;
	};
	void getControlOutput(PID_Output& out) {
		out = pid_Output;
	};

	void SwapAssetValueIdentifiers(const assetValueIdentifier& toFind, const assetValueIdentifier& replaceWith) {
		if (owner.Read() == toFind) owner = replaceWith;

		opt_Input.Lock(); opt_Input.UnsafeRead()->SwapAssetValueIdentifiers(toFind, replaceWith); opt_Input.Unlock();
		ml_Input.Lock(); ml_Input.UnsafeRead()->SwapAssetValueIdentifiers(toFind, replaceWith); ml_Input.Unlock();
		pid_Input.Lock(); pid_Input.UnsafeRead()->SwapAssetValueIdentifiers(toFind, replaceWith); pid_Input.Unlock();

		opt_Output.Lock(); opt_Output.UnsafeRead()->SwapAssetValueIdentifiers(toFind, replaceWith); opt_Output.Unlock();
		static_Output.Lock();  static_Output.UnsafeRead()->SwapAssetValueIdentifiers(toFind, replaceWith); static_Output.Unlock();
		pid_Output.Lock();  pid_Output.UnsafeRead()->SwapAssetValueIdentifiers(toFind, replaceWith); pid_Output.Unlock();
	};

public: // public methods: user-specified control override
	/*!
	User may supply specification of what they'll be doing instead during a specified period. Assumes && requires a duration to that override.
	*/
	void AddUserOverride(const time_t& time, float value, const time_t& durationSeconds = 3600) {
		controlPotentiallyActive = true;
		SimulationNeedsUpdate = SimulationNeedsUpdate.Read() > 0 ? Min((u64)SimulationNeedsUpdate.Read() + cweeRandomFloat(-60, 60), (u64)(time)) : time;
		user_Overrides.Lock();
		user_Overrides.UnsafeRead()->AddUniqueValue(time + durationSeconds, user_Overrides.UnsafeRead()->GetCurrentValue(time + durationSeconds));
		user_Overrides.UnsafeRead()->AddUniqueValue(time - 1, user_Overrides.UnsafeRead()->GetCurrentValue(time - 1));
		user_Overrides.UnsafeRead()->RemoveTimes(time, time + durationSeconds - 1);
		user_Overrides.UnsafeRead()->AddUniqueValue(time, value);
		user_Overrides.Unlock();
	};

	/*!
	User may supply specification of what they'll be doing instead during a specified period. Assumes && requires a duration to that override.
	*/
	void AddUserOverride(const time_t& time, float value, void* worldDefPtr, const time_t& durationSeconds = 3600);

	/*!
	User may remove a previous specification of what they'll be doing instead during a specified period. Assumes && requires a duration to that override.
	*/
	void ClearUserOverride(const time_t& timeStart, const time_t& timeEnd) {
		SimulationNeedsUpdate = SimulationNeedsUpdate.Read() > 0 ? Min((u64)SimulationNeedsUpdate.Read() + cweeRandomFloat(-60, 60), (u64)(timeStart)) : timeStart;
		user_Overrides.Lock();
		user_Overrides.UnsafeRead()->RemoveTimes(timeStart, timeEnd);
		user_Overrides.UnsafeRead()->AddUniqueValue(timeStart, -cweeMath::INF);
		user_Overrides.UnsafeRead()->AddUniqueValue(timeEnd, -cweeMath::INF);
		user_Overrides.Unlock();
	};

	/*!
	Returns true if the user supplied an override during the period, supplies override in 'out'
	*/
	bool getUserOverride(const time_t& time, float& out);

	vec3 getUserOverrideBounds(void* worldDefPtr);

	/*!
	Returns true if the user supplied an override during the period, supplies override in 'out'
	*/
	const Pattern& getUserOverrides() {
		return *user_Overrides.UnsafeRead();
	};

protected: // private data: input values & input data. Deligated to private to force user to go through the interface - which has small interventions that must happen.
	/*!
	assetValueIdentifier associated to the object && characteristic this cweeControl will be controlling.
	 --- CAN ONLY CONTROL ONE ASSSETVALUEIDENTIFIER AT A TIME ---
	*/
	cweeUnpooledInterlocked<assetValueIdentifier>		owner;
	/*!
	User to specify how this control is utilized in real world operations.
	*/
	cweeUnpooledInterlocked < AssetValueControlType >	controlType = AssetValueControlType::Static;
	cweeUnpooledInterlocked<bool>						controlPotentiallyActive = false;
	/*!
	Determines which control method is automatically utilized.
	*/
	cweeUnpooledInterlocked < ControlGenerationMethod>	controlMethod = ControlGenerationMethod::UserInput;

	cweeUnpooledInterlocked < Optimization_Input>		opt_Input;
	cweeUnpooledInterlocked < TrainedModel_Input>		ml_Input;
	cweeUnpooledInterlocked < UserInput_Input>			static_Input;
	cweeUnpooledInterlocked < PID_Input>				pid_Input;

	cweeUnpooledInterlocked < Pattern>					user_Overrides; // user-provided value/timestamp pairs that specify when they will be doing something different than expected.
	cweeUnpooledInterlocked < vec3>						overrideFit = vec3(0.0f, 1.0f, 0.0001f); // min, max, decimal

protected: // private data: output results. Deligated to private to force user to go through the interface - which has small interventions that must happen.
	cweeUnpooledInterlocked < Optimization_Output>		opt_Output;
	cweeUnpooledInterlocked < TrainedModel_Output>		ml_Output;
	cweeUnpooledInterlocked < UserInput_Output>			static_Output;
	cweeUnpooledInterlocked < PID_Output>				pid_Output;
};

// cweeUnpooledInterlocked -> cweeInterlocked

#pragma endregion
#endif