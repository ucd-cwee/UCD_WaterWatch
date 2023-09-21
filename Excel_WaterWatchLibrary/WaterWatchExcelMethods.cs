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

using Microsoft.Office.Interop.Excel;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Excel = Microsoft.Office.Interop.Excel;

namespace Excel_WaterWatchLibrary
{
    public static class Extensions
    {
        public static bool Equals(this double a, string b)
        {
            return String.Equals(a.ToString(), b, StringComparison.InvariantCultureIgnoreCase);
        }
        public static bool Equals(this string a, string b)
        {
            return String.Equals(a, b, StringComparison.InvariantCultureIgnoreCase);
        }
        public static bool Equals(this string a, double b)
        {
            return String.Equals(a, b.ToString(), StringComparison.InvariantCultureIgnoreCase);
        }
        public static string EscapeCharactersAsLiterals(this string obj)
        {
            string copy = obj;

            List<(string, string)> replacements = new List<(string, string)>()
            {
                ( "\\",     "\\\\" ),
                ( "\'",     "\\\'" ),
                ( "\"",     "\\\"" ),
                ( "\0",     "\\0'" ),
                ( "\a",     "\\a" ),
                ( "\b",     "\\b" ),
                ( "\f",     "\\f" ),
                ( "\n",     "\\n" ),
                ( "\r",     "\\r" ),
                ( "\t",     "\\t" ),
                ( "\v",     "\\v" )
            };

            foreach (var x in replacements)
            {
                copy = copy.Replace(x.Item1, x.Item2);
            }

            return copy;
        }

        public static DateTime FromUnixTimeSeconds(this DateTime a, double unixTimeStamp)
        {
            // Unix timestamp is seconds past epoch
            DateTime dateTime = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
            dateTime = dateTime.AddSeconds(unixTimeStamp).ToLocalTime();
            a = dateTime;
            return a;
        }
        public static double ToUnixTimeSeconds(this DateTime a)
        {
            // Unix timestamp is seconds past epoch
            return (TimeZoneInfo.ConvertTimeToUtc(a) -
                   new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Utc)).TotalSeconds;
        }

        public static T Get<T>(this IList<T> a, int slot)
        {
            if ((a != null) && (slot < a.Count) && (slot >= 0))
            {
                return a[slot];
            }
            else
            {
                return default(T);
            }
        }

        public static class Lists
        {
            public static List<T> RepeatedDefault<T>(int count)
            {
                return Repeated(default(T), count);
            }

            public static List<T> Repeated<T>(T value, int count)
            {
                List<T> ret = new List<T>(count);
                ret.AddRange(Enumerable.Repeat(value, count));
                return ret;
            }
        }

        public static string GetVbaMethods(Type x, string FunctionNamePrefix)
        {
            string sCode =
                "\r\nDim managedObject As Object\r\n" +
                "Public Sub RegisterCallback(callback As Object)\r\n" +
                "    Set managedObject = callback\r\n" +
                "End Sub\r\n";

            foreach (var method in x.GetMethods())
            {
                string methodCode;

                string functionName = method.Name;

                if (functionName == "InitializeUserDefinedFunctions") continue; // skip self
                if (functionName == "Equals") continue; // skip self
                if (functionName == "GetHashCode") continue;
                if (functionName == "MemberwiseClone") continue;
                if (functionName == "ToString") continue;
                if (functionName == "GetType") continue;

                {
                    string methodParameterString = "";
                    string methodInputParameterString = "";

                    int numInputParam = (int)(method.GetParameters()?.Length);
                    var inputParams = method.GetParameters();
                    foreach (var inputParam in inputParams)
                    {
                        bool optional = inputParam.HasDefaultValue;

                        string inputParamName = inputParam.Name;
                        string inputParamType = inputParam.ParameterType.Name;

                        string paramHeaderString = "";
                        if (optional)
                        {
                            if (inputParam.DefaultValue != null)
                            {
                                paramHeaderString = "Optional " + inputParamName + " = \"" + inputParam.DefaultValue.ToString() + "\"";
                            }
                            else
                            {
                                paramHeaderString = "Optional " + inputParamName + " = Nothing";
                            }
                        }
                        else
                        {
                            paramHeaderString = inputParamName;
                        }

                        string paramInputString = inputParamName;
                        {
                            // cast from variant input (Excel) to actual type (C#)
                            switch (inputParamType)
                            {
                                case "Int32": paramInputString = "CInt(" + inputParamName + ")"; break;
                                case "Single": paramInputString = "CSng(" + inputParamName + ")"; break;
                                case "String": paramInputString = "CStr(" + inputParamName + ")"; break;
                                case "Double": paramInputString = "CDbl(" + inputParamName + ")"; break;
                                case "Int64": paramInputString = "CDbl(" + inputParamName + ")"; break;
                                case "Object": paramInputString = inputParamName; break;
                                case "UInt64": paramInputString = "CDbl(" + inputParamName + ")"; break;
                                case "DateTime": paramInputString = "CDate(" + inputParamName + ")"; break;
                                case "Boolean": paramInputString = "CBool(" + inputParamName + ")"; break;

                                default: paramInputString = inputParamName; break;
                            }
                        }

                        if (string.IsNullOrEmpty(methodInputParameterString)) methodInputParameterString += paramInputString;
                        else methodInputParameterString += ", " + paramInputString;

                        if (string.IsNullOrEmpty(methodParameterString)) methodParameterString += paramHeaderString;
                        else methodParameterString += ", " + paramHeaderString;
                    }

                    string returnParamType = method.ReturnParameter.ParameterType.Name;
                    if (returnParamType.Contains("Range"))
                    {
                        methodCode =
                             "Public Function " + FunctionNamePrefix + functionName + "(" + methodParameterString + ") As Range" + "\r\n";
                        methodCode += "     On Error GoTo ErrorHandler\r\n";
                        methodCode += "          Set " + FunctionNamePrefix + functionName + " = managedObject." + functionName + "(" + methodInputParameterString + ")\r\n";
                        methodCode += "     Exit Function\r\n";
                        methodCode += "ErrorHandler:\r\n";

                    }
                    else if (returnParamType == "Object")
                    {
                        methodCode =
                             "Public Function " + FunctionNamePrefix + functionName + "(" + methodParameterString + ") " + "\r\n";
                        methodCode += "     On Error GoTo ErrorHandler\r\n";
                        methodCode += "         " + FunctionNamePrefix + functionName + " = managedObject." + functionName + "(" + methodInputParameterString + ")\r\n";
                        methodCode += "     Exit Function\r\n";
                        methodCode += "ErrorHandler:\r\n";
                    }
                    else
                    {
                        methodCode =
                            "Public Function " + FunctionNamePrefix + functionName + "(" + methodParameterString + ") As " + returnParamType + "\r\n";
                        methodCode += "     On Error GoTo ErrorHandler\r\n";
                        methodCode += "         " + FunctionNamePrefix + functionName + " = managedObject." + functionName + "(" + methodInputParameterString + ")\r\n";
                        methodCode += "     Exit Function\r\n";
                        methodCode += "ErrorHandler:\r\n";
                    }

                    methodCode +=
                        "End Function\r\n";

                    // type name replacement
                    {
                        methodCode = methodCode.Replace("As Int32", "As Integer");
                        methodCode = methodCode.Replace("As Single", "As Single");
                        methodCode = methodCode.Replace("As Int64", "As Long");
                        methodCode = methodCode.Replace("As Double", "As Double");
                        methodCode = methodCode.Replace("As UInt64", "As Double");
                        methodCode = methodCode.Replace("As DateTime", "As Date");
                        methodCode = methodCode.Replace("As String", "As String");
                        methodCode = methodCode.Replace("As Boolean", "As Boolean");
                        methodCode = methodCode.Replace("As Object", "As Object");
                    }

                    sCode += methodCode;
                }
            }

            return sCode;

        }


        public static Excel.Range Range(this Excel.Worksheet ws, string options)
        {
            if (ws != null)
                return ws.get_Range(options);
            return null;
        }

        public static void SetValue(this Excel.Range cell, dynamic value)
        {
            try
            {
                if (cell.Value == null && value == null) return;
                if ((cell.Value == null && value != null) || (!cell.Value.Equals(value)))
                    cell.Value = value;
            }
            catch (Exception)
            {
                // Extensions.EdmsTasks.InsertJob(() => SetValue(cell, value), EdmsTasks.Priority.Low, true); // try again sometime later
            }
        }

        public static void SetRange<T>(this Excel.Range cell, List<T> obj)
        {
            int nCol = ((Excel.Range)cell).Columns.Count;
            int nRow = ((Excel.Range)cell).Rows.Count;

            var value = ListToMatrix(obj, nCol, nRow);

            try
            {
                if (cell.Value == null && value == null) return;

                if ((cell.Value == null && value != null))
                {
                    cell.Value = value;
                }
                else
                {
                    bool pass = false;
                    int num = obj.Count;
                    for (int row = 0; row < nRow; row++)
                    {
                        for (int col = 0; col < nCol; col++)
                        {
                            if (((cell.Value as object[,])[row + 1, col + 1] == null) && (value[row, col] == null)) continue;
                            if (
                                    (
                                        ((cell.Value as object[,])[row + 1, col + 1] == null) && (value[row, col] != null)
                                    )
                                ||
                                    (
                                        !(cell.Value as object[,])[row + 1, col + 1].Equals(value[row, col])
                                    )
                                )
                            {
                                pass = true;
                                break;
                            }
                        }
                    }
                    if (pass) cell.Value = value;
                }
            }
            catch (Exception)
            {
                // Extensions.EdmsTasks.InsertJob(() => SetRange(cell, obj), EdmsTasks.Priority.Low, true); // try again sometime later
            }
        }

        public static bool UserIsEditing(this Excel.Application app)
        {
            if (app.Interactive == false)
            {
                return false;
            }
            else
            {
                try
                {
                    app.Interactive = false;
                    app.Interactive = true;
                }
                catch (Exception)
                {
                    return true;
                }
            }
            return false;
        }

        public static object[,] ListToMatrix<T>(List<T> obj, int nCol, int nRow, bool numericConversion = true)
        {
            if (obj.Count == 0) return null;

            while ((nRow * nCol) >= ((obj.Count * 2) + 1) && (nRow > 1))
            {
                nRow /= 2;
            }

            while ((nRow * nCol) >= ((obj.Count * 2) + 1) && (nCol > 1))
            {
                nCol /= 2;
            }

            while ((nRow * (nCol - 1)) >= obj.Count)
            {
                nCol--;
            }

            while (((nRow - 1) * nCol) >= obj.Count)
            {
                nRow--;
            }

            nCol = Math.Max(1, nCol);
            nRow = Math.Max(1, nRow);

            var args = new object[nRow, nCol];

            {
                int num = obj.Count;
                int i = 0;
                double a = 0.0;
                for (int row = 0; row < nRow; row++)
                {
                    for (int col = 0; col < nCol; col++)
                    {
                        if (i < num)
                        {
                            if (numericConversion && obj[i] is string && !string.IsNullOrEmpty(obj[i] as string))
                            {
                                if (double.TryParse(obj[i] as string, out a))
                                {
                                    args[row, col] = a;
                                    i++;
                                    continue;
                                }
                            }

                            args[row, col] = obj[i];
                        }
                        else
                        {
                            args[row, col] = "";
                        }
                        i++;
                    }
                }
            }

            return args;
        }
        public static List<dynamic> RangeToList(Excel.Range obj)
        {
            List<dynamic> toReturn = new List<dynamic>();
            foreach (Excel.Range cell in obj.Cells)
            {
                toReturn.Add(cell.Value);
            }

            return toReturn;
        }
        public static object[,] ListToExcelMatrix<T>(List<T> obj, int nCol, int nRow)
        {
            if (obj.Count == 0) return null;

            while ((nRow * nCol) >= ((obj.Count * 2) + 1) && (nRow > 1))
            {
                nRow /= 2;
            }

            while ((nRow * nCol) >= ((obj.Count * 2) + 1) && (nCol > 1))
            {
                nCol /= 2;
            }

            while ((nRow * (nCol - 1)) >= obj.Count)
            {
                nCol--;
            }

            while (((nRow - 1) * nCol) >= obj.Count)
            {
                nRow--;
            }

            nCol = Math.Max(1, nCol);
            nRow = Math.Max(1, nRow);

            var args = NewExcelObjectArray(nRow, nCol);

            {
                int num = obj.Count;
                int i = 0;
                for (int row = 0; row < nRow; row++)
                {
                    for (int col = 0; col < nCol; col++)
                    {
                        if (i < num)
                        {
                            args[row + 1, col + 1] = obj[i];
                        }
                        else
                        {
                            args[row + 1, col + 1] = "";
                        }
                        i++;
                    }
                }
            }

            return args;
        }
        public static object[,] NewExcelObjectArray(int iRows, int iCols)
        {
            int[] aiLowerBounds = new int[] { 1, 1 };
            int[] aiLengths = new int[] { iRows, iCols };
            return (object[,])Array.CreateInstance(typeof(object), aiLengths, aiLowerBounds);
        }
    }

    public static class EvalMemory
    {
        public static Excel.Range _evalMemory = null;
        public static dynamic _lastGoodReply = null;
        public static List<dynamic> badReplies = new List<dynamic>() { -2146826273, -2146826259 };
    }


    [ComVisible(true)]
    public class UserDefinedFunctions
    {
        /// <summary>
        /// Required function to confirm the UDFs have been added.
        /// </summary>
        /// <returns></returns>
        [ComVisible(true)]
        public bool ConfirmCallback() { return true; }

        /// <summary>
        /// Perform a generic Excel function when presented as a string. Supports one level of recursion. Examples:
        /// =Eval("=A1 + 100")
        /// or (for fun!)
        /// =Eval("=Eval(""A1 + 100"")")
        /// </summary>
        /// <param name="commands"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public object Eval(string commands)
        {
            try
            {
                bool myResponsibility = false;
                if (EvalMemory._evalMemory is null)
                {
                    EvalMemory._evalMemory = Excel_WaterWatchLibrary.App.ThisCell;
                    myResponsibility = true;
                }

                var x = Excel_WaterWatchLibrary.App.Evaluate(commands);

                bool anyBadReplyMatches = false;
                foreach (var badReply in EvalMemory.badReplies)
                {
                    if (x.Equals(badReply))
                    {
                        anyBadReplyMatches = true;
                        break;
                    }
                }

                if (anyBadReplyMatches && EvalMemory._lastGoodReply != null)
                {
                    x = EvalMemory._lastGoodReply;
                }
                else
                {
                    EvalMemory._lastGoodReply = x;
                }

                if (myResponsibility)
                {
                    EvalMemory._evalMemory = null;
                    EvalMemory._lastGoodReply = null;
                }

                return x;
            }
            catch (Exception)
            {
                EvalMemory._evalMemory = null;
                EvalMemory._lastGoodReply = null;
                return "";
            }
        }

        /// <summary>
        /// Interpolate between a starting value and ending value with a (unsigned) step value. Spills the resulting interpolation in the direction and shape hinted at by the output range.
        /// </summary>
        /// <param name="StartVal"></param>
        /// <param name="EndVal"></param>
        /// <param name="StepByVal"></param>
        /// <param name="OutputRange"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public object Interpolate(double StartVal, double EndVal, double StepByVal, object OutputRange)
        {
            if (OutputRange is null || !(OutputRange is Excel.Range))
            {
                return -1;
            }

            List<double> data = new List<double>();

            if (StartVal < EndVal)
            {
                for (double a = StartVal; a <= EndVal; a += Math.Abs(StepByVal))
                {
                    data.Add(a);
                }
            }
            else
            {
                for (double a = StartVal; a >= EndVal; a -= Math.Abs(StepByVal))
                {
                    data.Add(a);
                }
            }

            int nCol = ((Excel.Range)OutputRange).Columns.Count;

            int nRow = ((Excel.Range)OutputRange).Rows.Count;

            var value = Extensions.ListToMatrix(data, nCol, nRow);

            return value;
        }

        /// <summary>
        /// Returns true if the value is within the max/min of the supplied range.
        /// </summary>
        /// <param name="val"></param>
        /// <param name="withinRange"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public bool Within(double val, object withinRange)
        {
            if (withinRange is null || !(withinRange is Excel.Range))
            {
                return false;
            }

            Microsoft.Office.Interop.Excel.WorksheetFunction wsf = Excel_WaterWatchLibrary.App.WorksheetFunction;
            double maximum = wsf.Max(withinRange);
            double minimum = wsf.Min(withinRange);

            if (minimum <= val && val <= maximum)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        /// <summary>
        /// Returns the resulting value located at the reference location, supplied as a string. Example:
        /// = ValueAt("A1")
        /// </summary>
        /// <param name="refLocation"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public object ValueAt(string refLocation)
        {
            return Eval("CELL(\"contents\"," + refLocation + ")");
        }

        /// <summary>
        /// Spills the supplied value across all cells, in the direction and shape hinted at by the output range.
        /// </summary>
        /// <param name="whatToLoop"></param>
        /// <param name="OutputRange"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public object LoopValueOver(object whatToLoop, object OutputRange)
        {
            if (OutputRange is null || !(OutputRange is Excel.Range))
            {
                return -1;
            }

            List<object> data = new List<object>();
            int n = ((Excel.Range)OutputRange).Cells.Count;
            for (int i = 0; i < n; i++)
            {
                data.Add(whatToLoop);
            }

            int nCol = ((Excel.Range)OutputRange).Columns.Count;

            int nRow = ((Excel.Range)OutputRange).Rows.Count;

            var value = Extensions.ListToMatrix(data, nCol, nRow);

            return value;
        }

        /// <summary>
        /// Spills the unique parameters found in the input range, in the direction and shape hinted at by the output range.
        /// </summary>
        /// <param name="InputRange"></param>
        /// <param name="OutputRange"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public object GetUniques(object InputRange, object OutputRange = null) // returns the string-array-matrix that (should) fit the size specified by the output range. 
        {
            if (InputRange is null || !(InputRange is Excel.Range))
            {
                return null;
            }
            else
            {
                int nCol, nRow;

                if (OutputRange is null || !(OutputRange is Excel.Range)) {
                    nCol = 1;
                    nRow = 1000000;
                }
                else
                {
                    nCol = ((Excel.Range)OutputRange).Columns.Count;
                    nRow = ((Excel.Range)OutputRange).Rows.Count;
                }

                List<dynamic> ListOut = new List<dynamic>();
                foreach (Excel.Range cell in ((Excel.Range)InputRange).Cells)
                {
                    if (cell.Value != null)
                    {
                        bool found = false;
                        foreach (var y in ListOut)
                        {
                            if (cell.Value.Equals(y))
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            ListOut.Add(cell.Value);
                        }
                    }
                }                

                var value = Extensions.ListToMatrix(ListOut, nCol, nRow, false);

                return value;
            }
        }

        /// <summary>
        /// returns the local string address of the (top-left) supplied range
        /// </summary>
        /// <param name="range"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public string GetAddress(object range)
        {
            if (range is null || !(range is Excel.Range))
            {
                return "";
            }

            return (range as Excel.Range).Address;
        }

        /// <summary>
        /// returns the range from the supplied string global or local address. (Identical to "Indirect")
        /// </summary>
        /// <param name="address"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public Excel.Range Range(string address)
        {
            Excel.Range x = Excel_WaterWatchLibrary.App.Range[address];
            return x;
        }

        /// <summary>
        /// Returns the first cell in the supplied range.
        /// </summary>
        /// <param name="rangeA"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range FirstInRange(object rangeA)
        {
            Excel.Range x = rangeA as Excel.Range;

            if ((x is null) || !(x is Excel.Range)) return null;

            foreach (Excel.Range y in x.Cells)
            {
                return y;
            }

            return x;
        }

        /// <summary>
        /// Returns the last cell in the supplied range.
        /// </summary>
        /// <param name="rangeA"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range LastInRange(object rangeA)
        {
            Excel.Range x = rangeA as Excel.Range;
            if ((x is null) || !(x is Excel.Range)) return null;
            Excel.Range z = null;
            foreach (Excel.Range y in x.Cells)
            {
                z = y;
            }
            return z;
        }

        /// <summary>
        /// Starting from the supplied cell, returns the last non-empty cell found when exploring in the desired direction ("Up", "Down", "Left", "Right")
        /// </summary>
        /// <param name="rangeA"></param>
        /// <param name="direction"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range SnapToRange(object rangeA, string direction)
        {
            Excel.Range a = rangeA as Excel.Range;
            if (a is null)
            {
                return null;
            }

            var options = new List<string>() { "Left", "Right", "Down", "Up" };

            string selection = WaterWatch.GetBestMatch(direction, options);

            switch (selection)
            {
                case "Left":
                    {
                        int i = 1;
                        while ((a.Column - i) > 0)
                        {
                            Excel.Range b = OffsetFrom(a, 0, -i);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            i++;
                        }
                        i--;
                        return OffsetFrom(a, 0, -i);
                    }
                case "Right":
                    {
                        int i = 1;
                        while ((a.Column + i) < a.Worksheet.Columns.Count)
                        {
                            Excel.Range b = OffsetFrom(a, 0, i);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            i++;
                        }
                        i--;
                        return OffsetFrom(a, 0, i);
                    }
                case "Up":
                    {
                        int i = 1;
                        while ((a.Row - i) > 0)
                        {
                            Excel.Range b = OffsetFrom(a, -i, 0);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            i++;
                        }
                        i--;
                        return OffsetFrom(a, -i, 0);
                    }
                case "Down":
                    {
                        int i = 1;
                        while ((a.Row + i) < a.Worksheet.Rows.Count)
                        {
                            Excel.Range b = OffsetFrom(a, i, 0);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            i++;
                        }
                        i--;
                        return OffsetFrom(a, i, 0);
                    }
            }

            return null;
        }

        /// <summary>
        /// Starting from the supplied cell, returns the last non-empty cell found when exploring in the desired direction ("Up", "Down", "Left", "Right"), or the first cell that meets the required condition. Example: 
        /// = SnapToRangeWhere(A1, "right", " != ""100""") // returns cell where value does not equal 100
        /// or
        /// = SnapToRangeWhere(A1, "right", " == """"") // returns cell that is equal to "" string
        /// </summary>
        /// <param name="rangeA"></param>
        /// <param name="direction"></param>
        /// <param name="whereCondition"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range SnapToRangeWhere(object rangeA, string direction, string whereCondition)
        {
            Excel.Range a = rangeA as Excel.Range;
            if (a is null)
            {
                return null;
            }

            var options = new List<string>() { "Left", "Right", "Down", "Up" };

            string selection = WaterWatch.GetBestMatch(direction, options);

            switch (selection)
            {
                case "Left":
                    {
                        int i = 0;
                        while ((a.Column - i) > 0)
                        {
                            i++;
                            Excel.Range b = OffsetFrom(a, 0, -i);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            else
                            {
                                // value is valid .. check the condition. 
                                string x = b.Value2.ToString();
                                string cmd;
                                if (double.TryParse(x, out double r))
                                {
                                    cmd = $"if ({x}" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                else
                                {
                                    cmd = $"if (cweeStr(\"{x}\")" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                var awaiter = WaterWatch.DoScript(cmd);
                                while (!awaiter.IsFinished()) { }
                                x = awaiter.Result();

                                if (x == "1")
                                {
                                    break;
                                }
                            }
                        }
                        return OffsetFrom(a, 0, -i);
                    }
                case "Right":
                    {
                        int i = 0;
                        while ((a.Column + i) < a.Worksheet.Columns.Count)
                        {
                            i++;
                            Excel.Range b = OffsetFrom(a, 0, i);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            else
                            {
                                // value is valid .. check the condition. 
                                string x = b.Value2.ToString();
                                string cmd;
                                if (double.TryParse(x, out double r)) {
                                    cmd = $"if ({x}" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                else
                                {
                                    cmd = $"if (cweeStr(\"{x}\")" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                var awaiter = WaterWatch.DoScript(cmd);
                                while (!awaiter.IsFinished()) { }
                                x = awaiter.Result();
                                if (x == "1")
                                {
                                    break;
                                }
                            }
                        }
                        return OffsetFrom(a, 0, i);
                    }
                case "Up":
                    {
                        int i = 0;
                        while ((a.Row - i) > 0)
                        {
                            i++;
                            Excel.Range b = OffsetFrom(a, -i, 0);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            else
                            {
                                // value is valid .. check the condition. 
                                string x = b.Value2.ToString();
                                string cmd;
                                if (double.TryParse(x, out double r))
                                {
                                    cmd = $"if ({x}" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                else
                                {
                                    cmd = $"if (cweeStr(\"{x}\")" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                var awaiter = WaterWatch.DoScript(cmd);
                                while (!awaiter.IsFinished()) { }
                                x = awaiter.Result();
                                if (x == "1")
                                {
                                    break;
                                }
                            }
                        }
                        return OffsetFrom(a, -i, 0);
                    }
                case "Down":
                    {
                        int i = 0;
                        while ((a.Row + i) < a.Worksheet.Rows.Count)
                        {
                            i++;
                            Excel.Range b = OffsetFrom(a, i, 0);
                            if (b is null || b.Value is null)
                            {
                                break;
                            }
                            else
                            {
                                // value is valid .. check the condition. 
                                string x = b.Value2.ToString();
                                string cmd;
                                if (double.TryParse(x, out double r))
                                {
                                    cmd = $"if ({x}" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                else
                                {
                                    cmd = $"if (cweeStr(\"{x}\")" + whereCondition + "){ return 1; }else{ return 0; }";
                                }
                                var awaiter = WaterWatch.DoScript(cmd);
                                while (!awaiter.IsFinished()) { }
                                x = awaiter.Result();
                                if (x == "1")
                                {
                                    break;
                                }
                            }
                        }
                        return OffsetFrom(a, i, 0);
                    }
            }

            return null;
        }

        /// <summary>
        /// Returns the entire range that connects two supplied cells within the same worksheet.
        /// </summary>
        /// <param name="rangeA"></param>
        /// <param name="rangeB"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range CreateRange(object rangeA, object rangeB)
        {
            Excel.Range a = rangeA as Excel.Range;
            Excel.Range b = rangeB as Excel.Range;

            if (a is null || b is null)
            {
                return null;
            }

            Excel.Range c = Excel_WaterWatchLibrary.App.Range[a, b];

            return c;
        }

        /// <summary>
        /// returns the entire range from the starting location to the last non-empty cell found when exploring in the desired direction ("Up", "Down", "Left", "Right")
        /// </summary>
        /// <param name="rangeA"></param>
        /// <param name="direction"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range CreateSnapRange(object rangeA, string direction)
        {
            var b = SnapToRange(rangeA, direction);
            return CreateRange(rangeA, b);
        }

        /// <summary>
        /// returns the entire range from the starting location to the last non-empty cell found when exploring in the desired direction ("Up", "Down", "Left", "Right"), or the first cell that meets the required condition. Example: 
        /// = CreateSnapRangeWhere(A1, "right", " != ""100""") // returns cell where value does not equal 100
        /// or
        /// = CreateSnapRangeWhere(A1, "right", " == """"") // returns cell that is equal to "" string
        /// </summary>
        /// <param name="rangeA"></param>
        /// <param name="direction"></param>
        /// <param name="whereCondition"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range CreateSnapRangeWhere(object rangeA, string direction, string whereCondition)
        {
            var b = SnapToRangeWhere(rangeA, direction, whereCondition);
            return CreateRange(rangeA, b);
        }

        [System.Runtime.InteropServices.ComVisible(true)]
        public int MatchFirst(object toMatch, object range, int unused = -1)
        {
            try
            {
                Excel.Range over = (Excel.Range)range;
                Excel.Range find = (Excel.Range)toMatch;

                var valToFind = find.Value2;


                int row = 1;
                foreach (Excel.Range cell in over)
                {
                    if (cell.Value is null)
                    {
                        row++;
                        continue;
                    }

                    if (cell.Value is string && valToFind is string)
                    {
                        if (String.Equals(cell.Value as string, valToFind as string, StringComparison.InvariantCultureIgnoreCase))
                        {
                            return row;
                        }

                    }
                    else if (cell.Value2.Equals(valToFind))
                    {
                        return row;
                    }

                    row++;
                }

                return -1;
            }
            catch (Exception)
            {
                return -1;
            }
        }

        [System.Runtime.InteropServices.ComVisible(true)]
        public int MatchLast(object toMatch, object range, int unused = -1)
        {
            try
            {
                Excel.Range over = (Excel.Range)range;
                Excel.Range find = (Excel.Range)toMatch;

                var valToFind = find.Value;

                var list = Extensions.RangeToList(over);
                list.Reverse();

                int row = list.Count;
                foreach (var cell in list)
                {
                    if (cell is null)
                    {
                        if (valToFind is null)
                            return row;

                        row++;
                        continue;
                    }

                    if (cell is string && valToFind is string)
                    {
                        if (String.Equals(cell as string, valToFind as string, StringComparison.InvariantCultureIgnoreCase))
                        {
                            return row;
                        }

                    }
                    else if (cell.Equals(valToFind))
                    {
                        return row;
                    }

                    row++;
                }

                return -1;
            }
            catch (Exception)
            {
                return -1;
            }
        }

        public object CopyRangeTo(object rangeA, object OutputRange)
        {
            // for each object in rangeA... 
            var srce = Extensions.RangeToList(rangeA as Excel.Range);
            int nCol = ((Excel.Range)OutputRange).Columns.Count;
            int nRow = ((Excel.Range)OutputRange).Rows.Count;
            return Extensions.ListToMatrix(srce, nCol, nRow);
        }

        /// <summary>
        /// Offset from a specified cell by some number of rows or columns. (Similar to "Offset")
        /// </summary>
        /// <param name="range"></param>
        /// <param name="row"></param>
        /// <param name="col"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range OffsetFrom(object range, int row, int col)
        {
            try
            {
                Excel.Range x = (Excel.Range)range;

                if (x is null)
                {
                    return null;
                }

                Excel.Range y = x.Offset[row, col];

                return y;
            }
            catch (Exception)
            {
                return null;
            }
        }

        /// <summary>
        /// Offset from the calling cell by some number of rows or columns. (Similar to "Offset" but without needing to reference this cell's location)
        /// </summary>
        /// <param name="row"></param>
        /// <param name="col"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range OffsetFromThis(int row, int col)
        {
            //var x = WaterWatchExcel.App.ThisCell; // may return "A1" when this is a dynamic evaluation... 

            //if (EvalMemory._evalMemory != null)
            //{
            //    x = EvalMemory._evalMemory;
            //}

            var x = GetThis();

            string add = GetAddress(x);

            Excel.Range y = (Range(add) as Excel.Range).Offset[row, col];

            return y;
        }

        /// <summary>
        /// Returns the range associated with the calling cell. Note: Calling this can cause a recursive formula if not done intentionally.
        /// </summary>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public Excel.Range GetThis()
        {
            var x = Excel_WaterWatchLibrary.App.ThisCell; // may return "A1" when this is a dynamic evaluation... 

            if (EvalMemory._evalMemory != null)
            {
                x = EvalMemory._evalMemory;
            }

            return x;
        }


















        /// <summary>
        /// Returns a string with every occurance of "find" replaced with "replace"
        /// </summary>
        /// <param name="InputString"></param>
        /// <param name="find"></param>
        /// <param name="replace"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public string ReplaceString(string InputString, string find, string replace)
        {
            return InputString.Replace(find, replace);
        }

        /// <summary>
        /// Spills a series of strings generated by splitting the supplied text by the provided delimiter, in the direction and shape hinted at by OutputRange. Example:
        /// = SplitString("split me"," ", A:A); // spills downward in a column
        /// or
        /// = SplitString("split me"," ", A1:D4); // spills into a rectangle with a limited size (4x4)
        /// </summary>
        /// <param name="InputString"></param>
        /// <param name="splitBy"></param>
        /// <param name="OutputRange"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public object SplitString(string InputString, string splitBy, object OutputRange = null)
        {
            int nCol,nRow;
            if (OutputRange is null || !(OutputRange is Excel.Range))
            {
                nCol = 10000;
                nRow = 1;
            }
            else
            {
                nCol = ((Excel.Range)OutputRange).Columns.Count;

                nRow = ((Excel.Range)OutputRange).Rows.Count;
            }

            var sepBy = new List<string>() { splitBy };

            var strings = InputString.Split(sepBy.ToArray(), StringSplitOptions.None);

            

            var value = Extensions.ListToMatrix(strings.ToList(), nCol, nRow);

            return value;
        } // returns the string-array-matrix that (should) fit the size specified by the output range. 

        /// <summary>
        /// Returns the Excel DateTime from a unix timestamp (seconds since January 1, 1970)
        /// </summary>
        /// <param name="val"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public DateTime DateFromUnix(double val)
        {
            DateTime toReturn = new DateTime();
            return toReturn.FromUnixTimeSeconds(val);
        }

        /// <summary>
        /// Returns the unix timestamp (seconds since January 1, 1970) from a Excel DateTime
        /// </summary>
        /// <param name="val"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public double UnixFromDate(DateTime val)
        {
            return val.ToUnixTimeSeconds();
        }

        [System.Runtime.InteropServices.ComVisible(true)]
        public object WW_SplitVector(string InputString, bool usesPairs, object OutputRange)
        {
            if (OutputRange is null || !(OutputRange is Excel.Range))
            {
                return -1;
            }

            if (InputString.Length >= 2)
                InputString = InputString.Substring(1, InputString.Length - 2); // [..] to ..

            if (usesPairs)
            {
                if (InputString.Length >= 2)
                    InputString = InputString.Substring(1, InputString.Length - 2); // <..,..>, <..,..> to ..,..>, <..,..

                var strings = InputString.Split((new List<string>() { ">, <" }).ToArray(), StringSplitOptions.None);

                int nCol = ((Excel.Range)OutputRange).Columns.Count;

                int nRow = ((Excel.Range)OutputRange).Rows.Count;

                var value = Extensions.ListToMatrix(strings.ToList(), nCol, nRow);

                return value;
            }
            else
            {
                var strings = InputString.Split((new List<string>() { ", " }).ToArray(), StringSplitOptions.None);

                int nCol = ((Excel.Range)OutputRange).Columns.Count;

                int nRow = ((Excel.Range)OutputRange).Rows.Count;

                var value = Extensions.ListToMatrix(strings.ToList(), nCol, nRow);

                return value;
            }

        } // returns the string-array-matrix that (should) fit the size specified by the output range. 


        /// <summary>
        /// loads a file from the folder path and splits it based on the loaded rows.
        /// </summary>
        /// <param name="filePath"></param>
        /// <param name="OutputRange"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public object ReadFile(string filePath, object OutputRange)
        {
            if (OutputRange is null || !(OutputRange is Excel.Range))
            {
                return -1;
            }

            var strings = WaterWatch.DoScript("return fileSystem.readFileAsStrList(\"" + filePath.EscapeCharactersAsLiterals() + "\")"); // list of rows from the loaded file. Typically a CSV

            while (!strings.IsFinished()) { }

            return WW_SplitVector(strings.Result(), false, OutputRange);
        }

        /// <summary>
        /// loads a file from the folder path and returns the content.
        /// </summary>
        /// <param name="filePath"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public string ReadFileAsStr(string filePath)
        {
            var awaiter = WaterWatch.DoScript("{ return fileSystem.readFileAsCweeStr(\"" + filePath.EscapeCharactersAsLiterals() + "\"); }"); // list of rows from the loaded file. Typically a CSV
            while (!awaiter.IsFinished()) { }
            return awaiter.Result();
        }

        /// <summary>
        /// Splits a chaiscript function return (vector, Map) into a suppiied matrix.
        /// </summary>
        /// <param name="strings"></param>
        /// <param name="OutputRange"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public object SplitToMatrix(string strings, object OutputRange)
        {
            return WW_SplitVector(strings, false, OutputRange);
        } // returns the string-array-matrix that (should) fit the size specified by the output range. 





        /// <summary>
        /// Returns true if the string is NULL or equals ""
        /// </summary>
        /// <param name="InputString"></param>
        /// <returns></returns>
        [ComVisible(true)]
        public bool IsEmpty(string InputString)
        {
            return string.IsNullOrEmpty(InputString);
        }

        /// <summary>
        /// Takes the content of "InputCell" and inserts it ahead of or appends behind of each cell in the InputRange.
        /// </summary>
        /// <param name="InputCell"></param>
        /// <param name="InputRange"></param>
        /// <param name="OutputRange"></param>
        /// <param name="options"></param>
        /// <returns></returns>
        [System.Runtime.InteropServices.ComVisible(true)]
        public object AppendToRange(string insertOrAppend, object InputRange, object OutputRange, string Selection)
        {
            var Options = new List<string>() { "Insert", "Append" };
            string selection = WaterWatch.GetBestMatch(Selection, Options);

            Excel.Range b = InputRange as Excel.Range;
            if (b is null)
            {
                return null;
            }

            if (OutputRange is null || !(OutputRange is Excel.Range))
            {
                return -1;
            }

            List<string> toReturn = new List<string>();
            foreach (var x in b.Cells)
            {
                switch (selection)
                {
                    case "Insert":
                        toReturn.Add($"{insertOrAppend}{(x as Excel.Range).Value2}");

                        break;
                    case "Append":
                        toReturn.Add($"{(x as Excel.Range).Value2}{insertOrAppend}");

                        break;
                }
            }

            int nCol = ((Excel.Range)OutputRange).Columns.Count;
            int nRow = ((Excel.Range)OutputRange).Rows.Count;
            return Extensions.ListToMatrix(toReturn, nCol, nRow);
        }



        [System.Runtime.InteropServices.ComVisible(true)]
        public double SumArray(object range)
        {
            if (range is null)// || inbound.Length == 0)
            {
                return 0;
            }
            else
            {
                double a = 0;
                if (range is Excel.Range)
                {
                    foreach (Excel.Range cell in ((Excel.Range)range).Cells)
                    {
                        double t; string attempt;
                        if (cell.Value != null)
                        {
                            attempt = cell.Value.ToString();
                            if (double.TryParse(attempt, out t)) a += t;
                        }
                    }
                }
                return a;
            }
        }

        [System.Runtime.InteropServices.ComVisible(true)]
        public DateTime GetCurrentTime()
        {
            var a = WaterWatch.getCurrentTime();
            
            DateTime toReturn = (new DateTime()).FromUnixTimeSeconds(a.unixTime);

            return toReturn;
        }

        [System.Runtime.InteropServices.ComVisible(true)]
        public string GetDataFolder()
        {
            return WaterWatch.GetDataDirectory();
        }

        [ComVisible(true)]
        public double ErtPotential_kW(double averageWaterDemand, double expectedNumberOfCascadesThroughErts = 1.719395)
        {
            double potential = averageWaterDemand * 0.007637 * expectedNumberOfCascadesThroughErts / 1.719395;
            return potential;
            //var await = WaterWatch.DoScript($"{potential}_kW");
            //while (!await.IsFinished()) { }
            //string reply = await.Result();
            //if (double.TryParse(reply, out double kilowatt))
            //{
            //    return kilowatt;
            //}
            //else
            //{
            //    return 0.0;
            //}
        }

        [ComVisible(true)]
        public double ErtAnalysis_kW(double AverageFlow_GPM, double AverageDownstreamPressure_PSI = 60.0, double Efficiency = 76.3359, double MinAvgPressure_PSI = 40.0, double expectedNumberOfCascadesThroughErts = 1)
        {
            if (AverageDownstreamPressure_PSI > MinAvgPressure_PSI)
            {
                var await = WaterWatch.DoScript($"CentrifugalPumpEnergyDemand({AverageFlow_GPM}_gpm, ({AverageDownstreamPressure_PSI - MinAvgPressure_PSI}_psi).head.double.foot, (10000.0 / {Efficiency}).scalar_t).double"); // 76.3359f
                while (!await.IsFinished()) { }
                string reply = await.Result();
                if (double.TryParse(reply, out double kilowatt))
                {
                    return expectedNumberOfCascadesThroughErts * kilowatt;
                }
                else
                {
                    return 0.0;
                }
            }
            else
            {
                return 0.0;
            }
        }

        [ComVisible(true)]
        public object ErtTimeseriesAnalysis_kW(object Flow_GPM, object DownstreamPressure_PSI, double Efficiency = 76.3359, double MinPressure_PSI = 40.0, double expectedNumberOfCascadesThroughErts = 1)
        {
            if (Flow_GPM is Excel.Range && DownstreamPressure_PSI is Excel.Range)
            {
                List<double> flows = new List<double>();
                List<double> pressures = new List<double>();

                List<double> values = new List<double>();

                int nCol = (Flow_GPM as Excel.Range).Columns.Count;
                int nRow = (Flow_GPM as Excel.Range).Rows.Count;

                int progress = 0;

                foreach (Excel.Range cell in (Flow_GPM as Excel.Range).Cells)
                {
                    if (cell.Value != null)
                    {
                        if (double.TryParse(cell.Value.ToString(), out double v))
                        {
                            flows.Add(v);
                        }
                    }
                }
                foreach (Excel.Range cell in (DownstreamPressure_PSI as Excel.Range).Cells)
                {
                    if (cell.Value != null)
                    {
                        if (double.TryParse(cell.Value.ToString(), out double v))
                        {
                            pressures.Add(v);
                        }
                    }
                }

                double minPressure = double.MaxValue;
                for (int i = 0; i < pressures.Count && i < flows.Count; i++)
                {
                    minPressure = Math.Min(minPressure, pressures[i]);
                }

                if (minPressure > MinPressure_PSI)
                {
                    double pressureRemoval = minPressure - MinPressure_PSI;
                    for (int i = 0; i < pressures.Count && i < flows.Count; i++)                    
                        values.Add(expectedNumberOfCascadesThroughErts * ErtAnalysis_kW(flows[i], pressures[i], Efficiency, pressures[i] - pressureRemoval));                    
                }
                else
                {
                    for (int i = 0; i < pressures.Count && i < flows.Count; i++)                    
                        values.Add(0.0);                    
                }

                var matrix = Extensions.ListToMatrix(values.ToList(), nCol, nRow);

                return matrix;
            }
            else
            {
                return ErtAnalysis_kW((double)Flow_GPM, (double)DownstreamPressure_PSI, Efficiency, MinPressure_PSI);
            }
        }

        [ComVisible(true)]
        public string ExcelWaterWatch(string request)
        {
            var await = WaterWatch.DoScript(request);
            while (!await.IsFinished()) { }
            return await.Result();
        }

        [ComVisible(true)]
        public string WW_Command(string request)
        {
            var await = WaterWatch.DoScript(request);
            while (!await.IsFinished()) { }
            return await.Result();
        }

        [ComVisible(true)]
        public string WW_Commands(object range)
        {
            if (range is null)
            {
                return "";
            }
            else
            {
                string command = "";
                if (range is Excel.Range)
                {
                    foreach (Excel.Range cell in ((Excel.Range)range).Cells)
                    {
                        string attempt;
                        if (cell.Value != null)
                        {
                            attempt = cell.Value.ToString();
                            if (command.Length > 0)
                            {
                                command += "\n";
                                command += attempt;
                            }
                            else
                            {
                                command = attempt;
                            }
                        }
                    }
                    var await = WaterWatch.DoScript(command);
                    while (!await.IsFinished()) { }
                    return await.Result();
                }
                return "";
            }
        }

    }

    [ComVisible(true)]
    class WaterWatchExcelMethods
    {
        private static Excel.Application App => Excel_WaterWatchLibrary.App;
        private static ConcurrentDictionary<string, bool> cache = new ConcurrentDictionary<string, bool>();
        public static void EnsureUserDefinedFunctionsAreEnabled()
        {
            if (App != null && App.VBE != null && App.VBE.ActiveVBProject != null)
            {
                string buildFileName = "";

                try {
                    buildFileName = App.VBE.ActiveVBProject.BuildFileName;
                } catch (Exception) { }

                bool valueExits = false;
                bool foundValue = false;
                try {
                    foundValue = cache.TryGetValue(buildFileName, out valueExits);
                } catch (Exception) { }

                if (!foundValue || !valueExits)
                {
                    // each VB project should have these functions enabled
                    try
                    {
                        if (App != null && App.VBE != null)
                        {
                            foreach (Microsoft.Vbe.Interop.VBProject VBProject in App?.VBE?.VBProjects)
                            {
                                if (VBProject != null)
                                {
                                    bool previouslyStarted = false;
                                    foreach (Microsoft.Vbe.Interop.VBComponent component in VBProject.VBComponents)
                                    {
                                        Microsoft.Vbe.Interop.CodeModule cm = component?.CodeModule;
                                        previouslyStarted = cm.Find("RegisterCallback", 0, 0, 0, 0) && cm.Find("Dim managedObject As Object", 0, 0, 0, 0);
                                        if (previouslyStarted)
                                        {
                                            break;
                                        }
                                    }

                                    if (!previouslyStarted)
                                    {
                                        var wwModule = VBProject?.VBComponents?.Add(Microsoft.Vbe.Interop.vbext_ComponentType.vbext_ct_StdModule);
                                        string sCode = Extensions.GetVbaMethods(typeof(UserDefinedFunctions), "");
                                        wwModule?.CodeModule?.AddFromString(sCode);

                                        //wwModule?.VBE?.ActiveWindow?.Close();
                                        //wwModule?.VBE?.MainWindow?.Close();

                                        var codepane = wwModule?.CodeModule?.CodePane;
                                        if (codepane != null) // actual VBE editor window
                                        {
                                            var window = wwModule?.CodeModule?.CodePane?.Window; //  individual sub-window for editing a code module's text
                                            if (window != null)
                                            {
                                                bool visible = window.Visible;
                                                if (!visible) window.Visible = true;
                                                if (window.Visible) window.Visible = false;
                                                window?.Close();
                                            }
                                        }

                                        App.Run("RegisterCallback", new UserDefinedFunctions());
                                        cache[VBProject.BuildFileName] = true;
                                    }                                    
                                }
                            }
                        }
                    }
                    catch (Exception) { }

#if false
                try
                {
                    if (App != null && App.VBE != null && App.VBE.ActiveVBProject != null)
                    {
                        bool previouslyStarted = false;
                        foreach (Microsoft.Vbe.Interop.VBComponent x in App.VBE.ActiveVBProject.VBComponents)
                        {
                            Microsoft.Vbe.Interop.CodeModule cm = x?.CodeModule;
                            previouslyStarted = cm.Find("RegisterCallback", 0, 0, 0, 0) && cm.Find("Dim managedObject As Object", 0, 0, 0, 0);
                            if (previouslyStarted)
                            {
                                break;
                            }
                        }

                        if (!previouslyStarted)
                        {
                            var wwModule = App?.VBE?.ActiveVBProject?.VBComponents?.Add(Microsoft.Vbe.Interop.vbext_ComponentType.vbext_ct_StdModule);
                            string sCode = Extensions.GetVbaMethods(typeof(UserDefinedFunctions), "");
                            wwModule?.CodeModule?.AddFromString(sCode);

                            wwModule?.VBE?.ActiveWindow?.Close();
                            wwModule?.VBE?.MainWindow?.Close();

                            var codepane = wwModule?.CodeModule?.CodePane;
                            if (codepane != null) // actual VBE editor window
                            {
                                var window = wwModule?.CodeModule?.CodePane?.Window; //  individual sub-window for editing a code module's text
                                if (window != null)
                                {
                                    bool visible = window.Visible;
                                    if (!visible) window.Visible = true;
                                    if (window.Visible) window.Visible = false;
                                    window?.Close();
                                }
                            }
                        }

                        App.Run("RegisterCallback", new UserDefinedFunctions());

                        {
                            Excel.Worksheet sh = App?.ActiveSheet;
                            if (sh != null)
                            {
                                sh.Activate();
                                sh.Visible = XlSheetVisibility.xlSheetVisible;
                            }
                        }

                        Excel_WaterWatchLibrary.state.UDF_Enabled = true;
                    }
                    return;
                }
                catch (Exception)
                {
                    // WaterWatchExcel.state.UDF_Enabled = false;
                    return;
                }
#endif
                }
                else
                {
                    if (App != null && App.VBE != null && App.VBE.ActiveVBProject != null)
                    {
                        // activated. But what if the user accidentally changes the macro and this needs to be reset?
                        bool reply = false;
                        try
                        {
                            reply = App?._Run2("ConfirmCallback");
                        }
                        catch (Exception)
                        {
                            cache[App.VBE.ActiveVBProject.BuildFileName] = false;
                            return;
                        }

                        if (!reply)
                        {
                            cache[App.VBE.ActiveVBProject.BuildFileName] = false;
                            return;
                        }
                        else
                        {
                            cache[App.VBE.ActiveVBProject.BuildFileName] = true;
                            return;
                        }
                    }
                }

            }
        }
    }
}
