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
#include "../WaterWatchCpp/Mutex.h"
#include <Python.h>
// #undef PyTuple_GET_SIZE

#define _Py_CAST(type, expr) ((type)(expr))

#define _PyTuple_CAST(op) \
    _Py_CAST(PyTupleObject*, (op))






#pragma region UTILITY CLASSES

enum uwp_color { RED, BLUE, GREEN };
enum uwp_patternInterpType { LEFT, RIGHT, LINEAR, SPLINE };

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

    // operator double() const { return unixTime; };
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
    Pair(std::pair<A, B> const& t) : first(t.first), second(t.second) {};

    A first;
    B second;

    //std::string __str__() {
    //    return std::to_string(first) + ", " + std::to_string(second);
    //};
    //std::string __repr__() {
    //    return std::string("Pair(") + __str__() + ")";
    //};
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
    float   GetAvgValue(double time1, double time2);
    std::vector<Pair<double, double>> GetTimeSeries();
    double  GetMinTime();
    double  GetMaxTime();
    int     GetNumValues();
    uwp_patternInterpType     GetInterpolationType();
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

class ScriptingNode {
public:
    ScriptingNode();
    ~ScriptingNode();

    std::string			text_get() const;
    int			        startLine_get() const;
    int			        startColumn_get() const;
    int			        endLine_get() const;
    int			        endColumn_get() const;
    WaterWatchEnums::ScriptNodeType	    type_get() const;
    std::string			typeHint_get() const;
    int			        depth_get() const;

    void			    text_set(std::string s);
    void		        startLine_set(int s);
    void		        startColumn_set(int s);
    void		        endLine_set(int s);
    void		        endColumn_set(int s);
    void        	    type_set(WaterWatchEnums::ScriptNodeType s);
    void	    		typeHint_set(std::string s);
    void		        depth_set(int s);

private:
    cweeSharedPtr<void> data;
};

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();
    std::vector<std::string> DoScript_Cast_VectorStrings(std::string command); /* Specialty function(s) to support casting to C# types instead of to a string */
    std::vector<float> DoScript_Cast_VectorFloats(std::string command); /* Specialty function(s) to support casting to C# types instead of to a string */
    std::string DoScript(std::string command);
    // ScriptObject GetObject(std::string command);
    Awaiter DoScriptAsync(std::string command);
    std::vector< ScriptingNode > PreParseScript(std::string command);
    std::vector<std::string> CompatibleFunctions(std::string TypeName);
    std::vector<std::string> FunctionsThatStartWith(std::string startsWith);
    void StopCurrentScript();

    std::string Cast_String(std::string command);
    std::vector<float> Cast_VectorFloats(std::string command);

private:
    cweeSharedPtr<void> ptr;
};

#pragma endregion 


class Geocoding {
public:
    static std::map<std::string, std::string> Geocode(std::string address);

};




class WaterWatch {
public:
    static std::vector<double> half1(const std::vector<double>& v);
    static std::vector<int> half2(const std::vector<int>& v);
    static std::vector<float> half3(const std::vector<float>& v);

    static std::vector<float> TestVector();
    static int TestVector2(std::vector<float> const& a);
    static int TestVector2(std::vector<double> const& a);
    static int TestVector2(std::vector<int> const& a);

    static void SubmitToast(std::string title, std::string content);
    static Pair<bool, Pair<std::string, std::string>> TryGetToast();
    static Pair<int, Pair<std::string, std::vector<std::string>>> TryGetAppRequest();
    static void CompleteAppRequest(int ID, std::string reply);

    static std::string GetDataDirectory();
    static void SetDataDirectory(std::string dir);
    static std::string GetTemporaryFilePath(std::string extension);

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
    static std::string DoScriptImmediately(std::string command);

    static void AddToLog(std::string filePath, std::string content);
    static double GetNanosecondsSinceStart();

};

