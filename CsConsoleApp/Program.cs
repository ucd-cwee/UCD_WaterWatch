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

using System;
using System.Runtime;
using System.Reflection;
using System.Threading;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Collections.Generic;
using WaterWatchTypeConversions;

namespace WaterWatchTypeConversions {
    public static class Extensions
    {
        public static DateTime to_DateTime(this cweeDateTime cdt){
            return new DateTime(cdt.year, cdt.month, cdt.day, cdt.hour, cdt.minute, cdt.second, cdt.milliseconds);
        }
        //public static Task AsTask(this Awaiter t)
        //{
        //    Task t2 = new Task(async () => { 
        //        t.
        //    });
        //}
    }

    public class AutomaticDisposal<T> where T : System.IDisposable
    {
        public AutomaticDisposal() { }
        public AutomaticDisposal(T d) { obj = d; }
        ~AutomaticDisposal() {
            if (obj != null)
               obj.Dispose();
        }

        public T obj;
    }
}



namespace CsConsoleApp
{
    class Program
    {
        static string GetHeaderString() { 
            string toRet = "Welcome to the WaterWatch Sample App.\n";
            toRet += "Data Directory = " + WaterWatch.GetDataDirectory() + "\n";
            toRet += "Begin Scripting:\n\n";

            return toRet;

        }
        static void Main(string[] args)
        {
            Console.WriteLine(GetHeaderString());

            System.Timers.Timer parallel_toast_manager = new System.Timers.Timer() { Interval = 100, AutoReset = true, Enabled = true };
            parallel_toast_manager.Elapsed += Parallel_toast_manager_Elapsed;

            System.Timers.Timer AppLayerRequestProcessor = new System.Timers.Timer() { Interval = 100, AutoReset = true, Enabled = true };
            AppLayerRequestProcessor.Elapsed += AppLayerRequestProcessor_Elapsed;

            string prevLine = "";
            string command = "";
            var engine = new ScriptEngine(); // new AutomaticDisposal<ScriptEngine>(
            while (true) {
                var str = Console.ReadLine();
                if (str == "Exit")
                {
                    Environment.Exit(0);
                }
                if (str == "Reset") { engine = new ScriptEngine(); System.GC.Collect(); command = ""; continue; } // only doing the GC collect because C# doesn't seem to "allerted" to the need for a collection except for the occassional collection every 1-4 minutes...
                if (str == "" && prevLine == "") {
                    Stopwatch sw = new();
                    sw.Start();

                    var waiter = engine.DoScript(command);                    

                    Console.WriteLine("Time Elapsed; " + sw.Elapsed.ToString());
                    Console.WriteLine("Current DateTime: " + WaterWatch.getCurrentTime().to_DateTime().ToString());

                    Console.WriteLine(waiter);

                    command = "";
                }

                command += str;
                command += "\n";
                prevLine = str;
            }
        }

        private static void AppLayerRequestProcessor_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            var req = WaterWatch.TryGetAppRequest();
            if (req.first >= 0) {
                string result = "Function Not Handled";
                switch (req.second.first) {
                    case "OS_SelectFile":
                        result = "TBD";
                        break;
                    case "OS_SelectFolder":
                        result = "TBD";
                        break;
                    case "OS_SavePassword":
                        result = "TBD";
                        break;
                    case "OS_LoadPassword":
                        result = "TBD";
                        break;
                    case "OS_ThemeColor":
                        result = "TBD";
                        break;
                    case "OS_GetUserName":
                        result = "TBD";
                        break;
                    case "OS_GetMousePosition":
                        {
                            (int left, int top) = Console.GetCursorPosition();
                            result = $"[{left},{top}]";
                        }
                        break;
                    case "OS_SetClipboard":
                        result = "TBD";
                        break;
                    case "OS_GetClipboard":
                        result = "TBD";
                        break;
                    case "OS_SaveSetting":
                        result = "TBD";
                        break;
                    case "OS_GetSetting":
                        result = "TBD";
                        break;
                    default:
                        result = $"Function \"{req.second.first}\" Not Found";
                        break;
                }
                WaterWatch.CompleteAppRequest(req.first, result);
            }
        }

        private static void Parallel_toast_manager_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            while (true)
            {
                var toast = WaterWatch.TryGetToast();
                if (toast.first)
                {
                    Console.WriteLine($"\n/* WaterWatch Toast: \t\"{toast.second.first}\": \t\"{toast.second.second}\" */\n\n");
                }
                else
                {
                    break;
                }
            }
        }
    }

}
