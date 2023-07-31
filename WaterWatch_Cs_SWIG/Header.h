/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include "../WaterWatchCpp/SharedPtr.h"
#pragma region UTILITY CLASSES

enum uwp_color { RED, BLUE, GREEN };

class cweeDateTime {
public:
    cweeDateTime();
    cweeDateTime(double x);
    double unixTime;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int milliseconds;
};

class Awaiter {
public:
    Awaiter() : data_m(new std::string("")), isFinished_m(new bool(false)) {};
    bool IsFinished();
    std::string Result();

    std::shared_ptr<std::string> data_m;
    std::shared_ptr<bool> isFinished_m;
};

template <typename A, typename B> 
class Pair {
public:
    Pair() : first(), second() {};
    Pair(A const& t1, B const& t2) : first(t1), second(t2) {};
    Pair(A&& t1, B&& t2) : first(std::forward<A>(t1)), second(std::forward<B>(t2)) {};
    Pair(std::pair<A,B> const& t) : first(t.first), second(t.second) {};    

    A first;
    B second;
};

class SharedMatrix {
public:
    SharedMatrix();
    SharedMatrix(int index, bool deleteDataWhenScopeEnds = false);
    SharedMatrix(SharedMatrix const& other);
    ~SharedMatrix();

    void    Clear();
    void    AppendData(double X, double Y, float value);
    double  GetValue(double X, double Y);
    std::vector<double> GetKnotSeries(double Left, double Top, double Right, double Bottom, int numColumns, int numRows);
    std::vector<double> GetTimeSeries(double Left, double Top, double Right, double Bottom, int numColumns, int numRows);
    double  GetMinX();
    double  GetMaxX();
    double  GetMinY();
    double  GetMaxY();
    double  GetMinValue();
    double  GetMaxValue();
    int     GetNumValues();
    int     Index();

private:
    int     index_p;
    cweeSharedPtr<void> ptr;
};


class SharedTimeSeriesPattern {
public:
    SharedTimeSeriesPattern();
    SharedTimeSeriesPattern(int index, bool deleteDataWhenScopeEnds = false);
    ~SharedTimeSeriesPattern();

    void    Clear();
    void    AppendData(double time, float value);
    float   GetValue(double time);
    std::vector<Pair<double, double>> GetTimeSeries();
    double  GetMinTime();
    double  GetMaxTime();
    int     GetNumValues();
    std::string X_Units();
    std::string Y_Units();
    int     Index();

private:
    int     index_p;
    bool deleteDataWhenScopeEnds_p;
};

class SharedString {
public:
    SharedString();
    SharedString(int index, bool deleteDataWhenScopeEnds = false);
    ~SharedString();

    void    Set(std::string v);
    std::string   Get();
    int     Index();

private:
    int     index_p;
    bool deleteDataWhenScopeEnds_p;
};

class WaterWatchEnums {
public:
    WaterWatchEnums() {};
    ~WaterWatchEnums() {};

    enum ScriptNodeType {
        Id = 0,
        Fun_Call = 1,
        Unused_Return_Fun_Call = 2,
        Arg_List = 3,
        Equation = 4,
        Var_Decl = 5,
        Assign_Decl = 6,
        Array_Call = 7,
        Dot_Access = 8,
        Lambda = 9,
        Block = 10,
        Scopeless_Block = 11,
        Def = 12,
        While = 13,
        If = 14,
        For = 15,
        Ranged_For = 16,
        Inline_Array = 17,
        Inline_Map = 18,
        Return = 19,
        File = 20,
        Prefix = 21,
        Break = 22,
        Continue = 23,
        Map_Pair = 24,
        Value_Range = 25,
        Inline_Range = 26,
        Do = 27, 
        Try = 28,
        Catch = 29,
        Finally = 30,
        Method = 31,
        Attr_Decl = 32,
        Logical_And = 33,
        Logical_Or = 34,
        Reference = 35,
        Switch = 36,
        Case = 37,
        Default = 38,
        Noop = 39,
        Class = 40,
        Binary = 41,
        Arg = 42,
        Global_Decl = 43,
        Constant = 44,
        Compiled = 45,
        ControlBlock = 46,
        Postfix = 47,
        Assign_Retroactively = 48,
        Error = 49
    };
};

class ScriptingNode
{
public:
    ScriptingNode() : startLine(0), startColumn(0), endLine(0), endColumn(0), text(""), type(WaterWatchEnums::ScriptNodeType::Noop), typeHint(""), depth(0) {};
    ~ScriptingNode() {};

    std::string			text;
    int			        startLine;
    int			        startColumn;
    int			        endLine;
    int			        endColumn;
    WaterWatchEnums::ScriptNodeType	    type;
    std::string			typeHint;
    int			        depth;
};

class Color_Interop {
public:
    Color_Interop() = default;
    Color_Interop(Color_Interop const&) = default;
    Color_Interop(double _R, double _G, double _B, double _A) : R(_R), G(_G), B(_B), A(_A) {};

    double R,G,B,A;
};
class MapIcon_Interop {
public:
    Color_Interop color;
    double size, longitude, latitude;
    bool HideOnCollision;
    std::string IconPathGeometry;
    std::string Label;
};
class MapPolyline_Interop {
public:
    Color_Interop color;
    double thickness;
    bool dashed;
    std::vector<Pair<double, double>> coordinates;
};
class MapBackground_Interop {
public:
    bool highQuality;
    bool clipToBounds;
    double minValue;
    double maxValue;
    int matrix; // SharedMatrix 
    Color_Interop min_color;
    Color_Interop max_color;
};
class MapLayer_Interop {
public:
    std::map<int, MapPolyline_Interop> polylines; // childIndex : lineDef
    std::map<int, MapIcon_Interop> icons; // childIndex : iconDef
};

class ScriptObject {
public:
    ScriptObject(cweeSharedPtr<void> p);
    ~ScriptObject();

    std::string Cast_String();
    Color_Interop Cast_Color();
    MapIcon_Interop Cast_MapIcon();
    MapPolyline_Interop Cast_MapPolyline();
    MapLayer_Interop Cast_MapLayer();

private:
    cweeSharedPtr<void> boxedvalue; 
};

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();    
    std::vector<std::string> DoScript_Cast_VectorStrings(std::string command); /* Specialty function(s) to support casting to C# types instead of to a string */
    std::string DoScript(std::string command);
    // ScriptObject GetObject(std::string command);
    Awaiter DoScriptAsync(std::string command);
    std::vector< ScriptingNode > PreParseScript(std::string command);
    std::vector<std::string> CompatibleFunctions(std::string TypeName);
    std::vector<std::string> FunctionsThatStartWith(std::string startsWith);
    void StopCurrentScript();

    std::string Cast_String(std::string command);
    Color_Interop Cast_Color(std::string command);
    MapIcon_Interop Cast_MapIcon(std::string command);
    MapPolyline_Interop Cast_MapPolyline(std::string command);
    MapBackground_Interop Cast_MapBackground(std::string command);
    MapLayer_Interop Cast_MapLayer(std::string command);

private:
    cweeSharedPtr<void> ptr;
};

#pragma endregion 

class WaterWatch {
public:


    static void SubmitToast(std::string title, std::string content);
    static Pair<bool, Pair<std::string, std::string>> TryGetToast();
    static Pair<int, Pair<std::string, std::vector<std::string>>> TryGetAppRequest();
    static void CompleteAppRequest(int ID, std::string reply);

    static std::string GetDataDirectory();
    static void SetDataDirectory(std::string dir);
    static Pair<double, double> GeocodeAddress(std::string address);
    static std::string GeocodeAddress(double longitude, double latitude);
    static double GeocodeElevation(double longitude, double latitude);
    static Pair<double, double> ValidateCoordinates(double longitude, double latitude);
    static double Encode_2D_to_1D(double x, double y, double H);
    static Pair<double, double> Decode_1D_to_2D(double l, double H);

    static std::vector<float> PredictNext(std::vector<double> const& data, int nextN);
    static cweeDateTime getCurrentTime();

    static int RandomInt(int min, int max);
    static float RandomFloat(float min, float max);
    static std::string GetBestMatch(std::string const& input, std::vector<std::string> const& options);

    static int GetNumMultithreadingCores();
    static int GetNumLogicalCoresOnMachine();
    static int GetNumPhysicalCoresOnMachine();
    static float GetPercentMemoryUsedOfMachine();
    static float GetPercentCpuUsedOfMachine();

    static Awaiter DoScript(std::string command);

    static void AddToLog(std::string filePath, std::string content);
    static double GetNanosecondsSinceStart();




    //static int Data_CreatePattern();
    //static void Data_DeletePattern(int index);
    //static void Data_ClearPattern(int index);
    //static void Data_AppendData(int index, double time, float value);
    //static float Data_GetValue(int index, double time);

    //static int Data_CreateMatrix();
    //static void Data_DeleteMatrix(int index);
    //static void Data_ClearMatrix(int index);
    //static void Data_AppendData(int index, double column, double row, float value);
    //static float Data_GetValue(int index, double column, double row);
    //static std::vector <float> Data_GetMatrix(int index, double left, double top, double right, double bottom, int nCol, int nRow);
    //static int Data_CreateSpatialAsset();
    //static void Data_DeleteSpatialAsset(int index);
    //static void Data_ClearSpatialAsset(int index);
    //static void Data_AppendData(int index, std::string const& name, std::string const& value);
    //static void Data_AppendData(int index, std::string const& name, cweeDateTime const& time, std::string const& value);
    //static void Data_AppendData(int index, std::string const& name, cweeDateTime const& time, float value);
    //static std::vector<Pair<cweeDateTime, float>>	Data_GetPattern(int index);
    //static std::vector<Pair<cweeDateTime, float>>	Data_GetPattern(int index, double from, double till);
    //static std::vector<Pair<cweeDateTime, float>>	Data_GetPattern(int index, double from, double till, double step);

#if 0
    static std::string GetDataFolder();
    static void SetDataFolder(std::string const& in);
    static void WriteSetting(std::string const& name, std::string const& value);
    static std::string ReadSetting(std::string const& name);
    static void WriteSetting(std::string const& name, std::string const& value, cweeDateTime const& timeOfSetting);
    static std::string ReadSetting(std::string const& name, cweeDateTime const& timeToRetrieve);
    static void SaveSettingsToFile();
    static int CopySettingsToExternalData();
    static std::string GetAppFolder();
    static bool CheckFileExists(std::string const& filePath);
    static void DeleteFileFromOS(std::string const& folderPath, std::string const& fileNameWithExtension);
    static std::vector<std::string> GetListOfFilesWithExtension(std::string const& folderPath, std::string const& extension);
    static void SaveFileFromStrList(std::vector<std::string> const& data, std::string const& fileName);
    static void	WriteFileFromStr(std::string const& filePath, std::string const& fileContent);
    static void AddToLog(std::string const& filepath, std::string const& commentToLog);
    static float GetLoadingStatus();
    static std::string GetLoadingMessage();
    static void CreateProject(std::string const& ProjectName, std::string const& ProjectDescription, std::string const& INPFilePath, cweeDateTime const& CalibrationDate, std::string const& PhysicalLocation, std::string const& ScadaServer, std::string const& ScadaDatabaseName, std::string const& ScadaUsername, std::string const& ScadaPassword, int ScadaPort, std::string const& CsvScadaMapFilePath);
    static cweeDateTime GetProjectCalibrationDate();
    static void SaveProject(std::string const& folderPath, std::string const& fileName, int numDaysToRecord);
    static void SaveProject(int numDaysToRecord);
    static bool IsSaveDone();
    static bool IsStreamingDone();
    static bool IsSimulationDone();
    static bool IsOptimizationDone();
    static void LoadProject(std::string const& filePath);
    static std::string GetCurrentProjectName();
    static void SetCurrentProjectName(std::string const& in);
    static std::string GetCurrentProjectSaveDir();
    static void SetCurrentProjectSaveDir(std::string const& in);
    static bool CheckProjectReady();
    static void ClearProject();
    static std::string GetProjectNameFromFile(std::string const& filePath);
    static std::string GetProjectDescriptionFromFile(std::string const& filePath);
    static cweeDateTime GetProjectSaveDateFromFile(std::string const& filePath);
    static std::vector<Pair<cweeDateTime, float>> GetGlobalPatternTimeSeries(std::string const& patternName, cweeDateTime const& start, cweeDateTime const& end, int numSamples);
    // static EDMS_WRC::MeasurementType GetGlobalPatternUnits(std::string const& patternName);
    static void SetCustomerBillingDatabaseDesign(std::vector<std::string> const& design);
    static void SetScadaDatabaseDesign(std::vector<std::string> const& design);
    static std::vector<std::string> GetCustomerBillingDatabaseDesign();
    static std::vector<std::string> GetScadaDatabaseDesign();
    static void ConnectToWaterSystemData(std::string const& SCADAserver, std::string const& SCADAusername, std::string const& SCADApassword, std::string const& BILLINGserver, std::string const& BILLINGusername, std::string const& BILLINGpassword, std::string const& SCADAdriver, std::string const& BILLINGdriver);
    static std::vector<std::string> GetSqlDrivers();
    static bool IsConnectedToBillingData();
    static bool IsConnectedToScadaData();
    static std::vector<std::string> GetScadaConnectionDetails();
    static std::vector<std::string> GetBillingConnectionDetails();
    static std::vector<Pair<double, double>> GetWaterSystemCustomerLongLats();
    static void AssociateScenarioToScada(std::string const& scenarioName);
    static bool TestDllConnect(std::string const& destination, std::string const& username, std::string const& password, std::string const& dbName, int port);
    static std::vector<int> GetListOfScadaIDs();
    static std::vector<std::string> GetListOfScadaTagPaths();
    static std::string GetTagPathOfScadaID(int TagID);
    static int GetTagIdOfScadaTagPath(std::string const& TagPath);
    // static EDMS_WRC::AssetType GetAssetTypeOfTagPath(std::string const& TagPath);
    // static EDMS_WRC::CharacteristicType GetCharacteristicTypeOfTagPath(std::string const& TagPath);
    // static EDMS_WRC::MeasurementType GetMeasurementTypeOfTagPath(std::string const& TagPath);
    static void SetTagPathCharacteristics(std::string const& tagPath, EDMS_WRC::AssetType const& asset, EDMS_WRC::CharacteristicType const& value, EDMS_WRC::MeasurementType const& meas);
    // static EDMS_WRC::ASSET_PAIR GetAssociatedAssetToTagPath(std::string const& scenarioName, std::string const& tagPath);
    static std::string GetAssociatedTagPath(std::string const& scenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& value);
    static void SetAssociatedTagPath(std::string const& scenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& value, std::string const& tagPath);
    static std::vector<Pair<cweeDateTime, float>> GetTimeSeriesOfTagPath(std::string const& TagPath);
    static std::vector<Pair<cweeDateTime, float>> GetTimeSeriesOfTagPath(std::string const& TagPath, EDMS_WRC::MeasurementType const& desiredUnit);
    static std::vector<Pair<cweeDateTime, float>> GetTimeSeriesOfTagPath(std::string const& TagPath, cweeDateTime const& start, cweeDateTime const& end, int numSamples);
    static std::vector<Pair<cweeDateTime, float>> GetTimeSeriesOfTagPath(std::string const& TagPath, cweeDateTime const& start, cweeDateTime const& end, int numSamples, EDMS_WRC::MeasurementType const& desiredUnit);
    // static EDMS_WRC::MeasurementType GetUnitsOfTagPath(std::string const& TagPath);
    static std::vector<int> GetCustomerAccountIds();
    static std::vector<Pair<cweeDateTime, float>> GetTimeSeriesOfCustomer(int accountNumber);
    static std::vector<Pair<cweeDateTime, float>> GetTimeSeriesOfCustomer(int accountNumber, cweeDateTime const& start, cweeDateTime const& end, int numSamples);
    // static EDMS_WRC::MeasurementType GetUnitsOfCustomer(int accountNumber);
    static std::string GetCustomerName(int accountNumber);
    static std::vector<EDMS_WRC::CWEE_ENERGY_RATE> GetEnergyRates();
    static Pair<double, double> EvaluateEnergyRate(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, std::string const& rateID, cweeDateTime const& start, cweeDateTime const& end);
    static std::vector<Pair<cweeDateTime, float>> GetEnergyRate_kWh(std::string const& rateID, cweeDateTime const& start, cweeDateTime const& end);
    static std::vector<Pair<cweeDateTime, float>> GetEnergyRate_kW(std::string const& rateID, cweeDateTime const& start, cweeDateTime const& end);
    static void ResetEnergyRate(std::string const& rateID);
    static int GetNumScenarios();
    static int GetNumScenariosThatFitInMemory();
    static std::vector<std::string> GetScenariosInColdStorage();
    static void MoveScenarioToColdStorage(std::string const& scenarioName);
    static void RemoveScenarioFromColdStorage(std::string const& scenarioName);
    static std::vector<std::string> GetScenarios();
    static std::vector<std::string> GetScenarioDescriptions();
    static void CopyScenario(std::string const& NewScenarioName, std::string const& OldScenarioName);
    static void EraseScenario(std::string const& scenarioName);
    static void SetScenarioAsBaseline(std::string const& scenarioName);
    static void SetScenarioColor(std::string const& scenarioName, float R, float G, float B);
    static void SetScenarioName(std::string const& oldScenarioName, std::string const& newScenarioName);
    static void SetScenarioDescription(std::string const& scenarioName, std::string const& newDescription);
    // static EDMS_WRC::CWEE_COLOR GetScenarioColor(std::string const& scenarioName);
    static std::string GetScenarioDescription(std::string const& scenarioName);
    static bool GetScenarioIsBaseline(std::string const& scenarioName);
    static void SetActiveScenario(std::string const& scenarioName);
    static void SetScenarioStar(std::string const& scenarioName, bool setting);
    static std::vector<std::string> GetStarredScenarios();
    static std::string GetActiveScenario();
    static std::string GetBaselineScenario();
    static void SetScenarioIconNum(std::string const& scenarioName, int newNum);
    static int GetScenarioIconNum(std::string const& scenarioName);
    static void	QueueSimulateScenario(std::string const& scenarioName, cweeDateTime const& startDate, int durationMinutes);
    static void SimulateScenario(std::string const& scenarioName, cweeDateTime const& startDate, int durationMinutes);
    static void SimulateScenario_CompleteRefresh(std::string const& scenarioName);
    static void SkeletonizeScenario(std::string const& scenarioName, bool skeletonizeDeadEnds, bool skeletonizePipes);
    static void SkeletonizeScenario(std::string const& scenarioName, bool skeletonizeDeadEnds, bool skeletonizePipes, bool sameHydraulics);
    static std::string GenerateEPAnetProjectFromScenario(std::string const& scenarioName);
    static void SetScenarioAsReality(std::string const& scenarioName);
    static std::string GetRealityScenario();
    static bool GetScenarioIsReality(std::string const& scenarioName);
    static float EvaluateObjectiveFunction(std::string const& ScenarioName, EDMS_WRC::Optimization_Value const& value, EDMS_WRC::Optimization_Aggregation const& agg, cweeDateTime const& start, cweeDateTime const& end);
    static std::vector<Pair<cweeDateTime, float>> EvaluateObjectiveFunction(std::string const& ScenarioName, EDMS_WRC::Optimization_Value const& value, EDMS_WRC::Optimization_Aggregation const& agg, cweeDateTime const& start, cweeDateTime const& end, int numSamples);
    static std::vector<std::string> GetSimulationPatternNames(std::string const& ScenarioName);
    static std::vector<std::string> GetScadaPatternNames();
    static std::vector<std::string> GetGlobalPatternNames();
    static void SetCustomerPatternValue(int accountNumber, cweeDateTime const& time, float value);
    static void SetScadaPatternValue(std::string const& tagpath, cweeDateTime const& time, float value);
    static void SetGlobalPatternValue(std::string const& patternName, cweeDateTime const& time, float value);
    static void LearnGlobalPattern(std::string const& patternName);
    static void LearnScadaPattern(std::string const& tagpath);
    static void LearnCustomerPattern(int accountNumber);
    static void AddNewGlobalPattern(std::string const& patternName, cweeDateTime const& start, cweeDateTime const& end, float initialValue);
    static void RotateCoordinates(std::string const& ScenarioName, double clockwiseRotationDegrees);
    static void ScaleCoordinates(std::string const& ScenarioName, double xScale, double yScale, double xAdj, double yAdj, double prev_xScale, double prev_yScale);
    static void ScaleCoordinatesLongLat(std::string const& ScenarioName, double xScale, double yScale, double xAdj, double yAdj);
    static std::vector<double> GetLongLat(std::string const& address);
    static std::vector<double> GetAssetCoordinates(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName);
    static std::string GetAssetDescription(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName);
    static void SetAssetDescription(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, std::string const& description);
    static std::vector<EDMS_WRC::CWEE_GEOSPATIAL_POSITION> GetPipeVertices(std::string const& ScenarioName, std::string const& pipeName);
    static std::vector<std::vector<Pair<double, double>>> GetPolygonsFromAsset(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName);
    static std::vector<std::vector<Pair<double, double>>> GetPolygonsFromAsset(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, float neighborScaling);
    static void RenameScenarioAsset(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& oldAssetName, std::string const& newAssetName);
    static void SetAssetCoordinates(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, double Long, double Lat);
    static std::vector<EDMS_WRC::ASSET_PAIR> GetConnectedAssets(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName);
    // static EDMS_WRC::ASSET_PAIR GetConnectedAsset(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, bool upstream);
    static void SetConnectedAsset(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, bool upstream, EDMS_WRC::ASSET_PAIR const& newAsset);
    // static EDMS_WRC::CWEE_CONTROL_PARAM GetAssetCharacteristicControls(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static void SetAssetCharacteristicControls(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, EDMS_WRC::CWEE_CONTROL_PARAM const& input);
    // static EDMS_WRC::controlGenerationStatus GetAssetCharacteristic_Control_Status(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, EDMS_WRC::ControlGenerationMethod const& whichControl);
    static Pair<double, double> GetAssetCharacteristic_TrainedModelControl_Error(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static std::vector<EDMS_WRC::TIMESERIES_COUPLED_PAIR> GetAssetCharacteristic_TrainedModelControl_Plots(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static void SetAssetCharacteristic_TrainedModelControl(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, array_view<float const> operatingSetPoints, cweeDateTime const& startDate, cweeDateTime const& endDate, array_view<EDMS_WRC::CWEE_TRAINED_MODEL_INPUT const> predictionInputs, EDMS_WRC::TrainedModel_ErrorMetric const& errMetric);
    static std::vector<float> GetAssetCharacteristic_TrainedModelControl_operatingSetPoints(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static std::vector<EDMS_WRC::CWEE_TRAINED_MODEL_INPUT> GetAssetCharacteristic_TrainedModelControl_predictionInputs(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static cweeDateTime GetAssetCharacteristic_TrainedModelControl_startDate(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static cweeDateTime GetAssetCharacteristic_TrainedModelControl_endDate(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    // static EDMS_WRC::TrainedModel_ErrorMetric GetAssetCharacteristic_TrainedModelControl_errMetric(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    // static EDMS_WRC::UserInput_InputType GetAssetCharacteristic_UserInputControl_Type(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static std::string GetAssetCharacteristic_UserInputControl_Value(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static void SetAssetCharacteristic_UserInputControl(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, EDMS_WRC::UserInput_InputType const& inputtype, std::string const& value);
    // static EDMS_WRC::CWEE_PID_CONTROL_INPUT GetAssetCharacteristic_PIDControl(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static void SetAssetCharacteristic_PIDControl(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, EDMS_WRC::ASSET_PAIR const& observedAsset, EDMS_WRC::CharacteristicType const& observedAssetCharacteristic, float setPoint, float minSetting, float maxSetting, float Ku);
    static std::vector<EDMS_WRC::CWEE_OPTIMIZATION_SUBJECT> GetAssetCharacteristic_OptimizationControl_Subjects(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static std::vector<EDMS_WRC::CWEE_OPTIMIZATION_OBJECTIVE> GetAssetCharacteristic_OptimizationControl_Objectives(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static std::vector<EDMS_WRC::CWEE_OPTIMIZATION_CONSTRAINT> GetAssetCharacteristic_OptimizationControl_Constraints(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static std::vector<cweeDateTime> GetAssetCharacteristic_OptimizationControl_DateRange(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static void SubmitScenarioOptimization(std::string const& ScenarioName, EDMS_WRC::CWEE_OPTIMIZATION_PARAMETERS const& params, std::vector<EDMS_WRC::CWEE_OPTIMIZATION_SUBJECT> const& subjects, std::vector<EDMS_WRC::CWEE_OPTIMIZATION_OBJECTIVE> const& objectives, std::vector<EDMS_WRC::CWEE_OPTIMIZATION_CONSTRAINT> const& constraints, cweeDateTime const& startDate, cweeDateTime const& endDate);
    static std::vector<EDMS_WRC::CWEE_OPTIMIZATION_RECORD> GetOptimizationRecords();
    // static EDMS_WRC::CWEE_OPTIMIZATION_RECORD GetOptimizationRecord(int num);
    static void ReviewOptimizationRecordForRemoval(int num);
    static std::vector<EDMS_WRC::TIMESERIES_COUPLED_PAIR> GetOptimizationRecord_TimeSeries(int num);
    static std::vector<EDMS_WRC::CWEE_OPTIMIZATION_SUBJECT> GetOptimizationRecord_Subjects(int num);
    static std::vector<EDMS_WRC::CWEE_OPTIMIZATION_OBJECTIVE> GetOptimizationRecord_Objectives(int num);
    static std::vector<EDMS_WRC::CWEE_OPTIMIZATION_CONSTRAINT> GetOptimizationRecord_Constraints(int num);
    static std::vector<cweeDateTime> GetOptimizationRecord_DateRange(int num);
    static std::vector<std::string> GetOptimizationRecord_Details(int num);
    static std::vector<std::string> GetOptimizationRecord_Results(int num, EDMS_WRC::CWEE_OPTIMIZATION_SUBJECT const& subject);
    static bool OptimizationRecord_EarlyExit(int num, bool setAs);
    static bool OptimizationRecord_EarlyExit(int num);
    static std::string GetOptimizationRecord_RelatedScenario(int num);
    // static EDMS_WRC::PATTERN_NAME_SOURCE_PAIR GetPumpStationEnergyPricePattern(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName);
    static void SetPumpStationEnergyPricePattern(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, int mode, std::string const& patName);
    // static EDMS_WRC::PATTERN_NAME_SOURCE_PAIR GetPumpStationEmissionIntensityPattern(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName);
    static void SetPumpStationEmissionIntensityPattern(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, int mode, std::string const& patName);
    static std::string GetAssetCharacteristic_OptimizationControl_Value(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static void AddUserOverrideToAssetCharacteristicControl(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, cweeDateTime const& time, float value);
    static float GetAssetCharacteristic_Control_Override(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, cweeDateTime const& time, float originalValue);
    static void RemoveUserOverrideFromAssetCharacteristicControl(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, cweeDateTime const& timeStart, cweeDateTime const& timeEnd);
    static bool GetIfAssetCharacteristic_Control_Override(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, cweeDateTime const& time);
    static int GetNumAssets(std::string const& ScenarioName, EDMS_WRC::AssetType const& type);
    static std::vector<std::string> GetAssets(std::string const& ScenarioName, EDMS_WRC::AssetType const& type);
    static std::vector<EDMS_WRC::ASSET_PAIR> GetAllAssetsInHydraulicZone(std::string const& ScenarioName, std::string const& ZoneName);
    static std::vector<EDMS_WRC::ASSET_PAIR> GetCriticalAssetsInHydraulicZone(std::string const& ScenarioName, std::string const& ZoneName);
    static std::vector<EDMS_WRC::ASSET_PAIR> GetHighestAndLowestElevationJunctionInHydraulicZone(std::string const& ScenarioName, std::string const& ZoneName);
    static std::vector<float> GetCurrentAssetData(std::string const& ScenarioName, std::string const& HydraulicZoneName, EDMS_WRC::AssetType const& type, std::string const& assetName, cweeDateTime const& time);
    static int GetNumAssetProperties(std::string const& ScenarioName, EDMS_WRC::AssetType const& type);
    static std::string GetAssetPropertyName(std::string const& ScenarioName, int whichProperty, EDMS_WRC::AssetType const& type);
    static std::vector<std::string> GetAssetPropertyOptions(std::string const& ScenarioName, int whichProperty, EDMS_WRC::AssetType const& type);
    static std::string GetAssetProperty(std::string const& ScenarioName, int whichProperty, EDMS_WRC::AssetType const& type, std::string const& assetName);
    static void SetAssetProperty(std::string const& ScenarioName, int whichProperty, EDMS_WRC::AssetType const& type, std::string const& assetName, std::string const& val);
    static std::vector<Pair<cweeDateTime, float>> GetSystemStorage(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static std::vector<Pair<cweeDateTime, float>> GetWaterDemand(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static std::vector<Pair<cweeDateTime, float>> GetEnergyDemand(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static float GetOperatingCost(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static float GetReliability(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static float GetRevenue(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static float GetGHG(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static float GetEnergyUsed(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static float GetPeakLoad(std::string const& ScenarioName, cweeDateTime const& Start, cweeDateTime const& End);
    static float GetCharacteristicValue(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, cweeDateTime const& time);
    static float GetCharacteristicAggregation(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, cweeDateTime const& Start, cweeDateTime const& End, EDMS_WRC::Optimization_Aggregation const& agg);
    // static EDMS_WRC::CWEE_OVERRIDE_BOUNDS GetCharacteristicValueBounds(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static std::vector<Pair<cweeDateTime, float>> GetCharacteristicTimeSeries(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic, cweeDateTime const& start, cweeDateTime const& end, int numSamples);
    // static EDMS_WRC::MeasurementType GetCharacteristicUnits(std::string const& ScenarioName, EDMS_WRC::AssetType const& type, std::string const& assetName, EDMS_WRC::CharacteristicType const& characteristic);
    static int GetRandomInt(int min, int max);
    static float GetRandomFloat(float min, float max);
    static float IntegrateTimeSeries(std::vector<Pair<cweeDateTime, float>> const& data, cweeDateTime const& start, cweeDateTime const& end);
    static cweeDateTime GetCurrentTime();
    static void	SetCurrentTime(cweeDateTime const& time, bool mode);
    static int GetIndexOfNearestTime(std::vector<Pair<cweeDateTime, float>> const& data, cweeDateTime const& target);
    static std::vector<float> GetMaxMinYaxisValues(std::vector<Pair<cweeDateTime, float>> const& data);
    static void DebugDllMethod();
    static uint64_t GetNanosecondsSinceStart();
    static int submitToast(std::string const& title, std::string const& content);
    static std::vector<EDMS_WRC::CWEE_TOAST> getToasts();
    static void removeToast(int id);
    static std::string GetFileExtension(std::string const& filePath);
    static std::string SaveGlobalPatternToFile(std::string const& patternName);
    static void LoadGlobalPatternsFromFile(std::string const& filePath);
    static std::vector<std::string> ParseSentance(std::string const& In, std::string const& delim);
    static Pair<double, double> GetMercadorFromLongLat(double Long, double Lat);
    static Pair<double, double> GetLongLatFromMercador(double X, double Y);
    static Pair<double, double> GetCustomerLongLat(int customerAccount);
    static Pair<double, double> GetMapTopRight();
    static Pair<double, double> GetMapBottomLeft();
    static std::string GetBestMatch(std::string const& input, std::vector<std::string> const& options);
    static int Hash(int a, int b);



    static std::string Geocode_GetAddress(double longitude, double latitude);
    static Pair<double, double> Geocode_GetLongLat(std::string const& address);
    static double Geocode_GetElevation(double longitude, double latitude);



    static void		StartRouterServer();
    static std::string	QueryRouterServer(std::string const& request, std::string const& ipaddress, float timeoutSeconds);

    static void		StartDebugServer();
    static std::string	TestDebugServer();
    static std::string	PackageServerMessage(std::string const& srce);
    static std::string	UnpackageServerMessage(std::string const& srce);

    static std::vector <std::string> GetLargeScriptVariableImmediate(std::string const& variable, bool noConversion) noexcept;
    //static Windows::Foundation::IAsyncOperation < std::vector <std::string> >	GetLargeScriptVariable(std::string const& variable) noexcept;
    //static Windows::Foundation::IAsyncOperation < std::vector <std::string> >	GetLargeScriptVariable(std::string const& variable, bool noConversion) noexcept;
    //static Windows::Foundation::IAsyncOperation < std::string >	DoScript(std::string const& command) noexcept;
    //static Windows::Foundation::IAsyncOperation < std::string >	DoScript(std::string const& command, bool noConversion) noexcept;
    //static void  CancelScript();
    //static Windows::Foundation::IAsyncOperation < std::vector < EDMS_WRC::CWEE_SCRIPT_NODE>>	PreParseScript(std::string const& command) noexcept;
    //static std::vector <std::string> PreParseScriptJSON(std::string const& command);

    static std::vector<std::string> GetRegisteredScriptFunctions();
    static void		ResetScripting();

#endif

};

