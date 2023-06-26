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

#define EdmsMultitask

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Xml.Serialization;
using System.Runtime.CompilerServices;
using System.Windows.Input;
using System.Collections;
using System.IO;


namespace Foundation
{
    public static class ObjectExtensions
    {
        static string GetSourceCode(this Action action,
        [System.Runtime.CompilerServices.CallerMemberName] string membername = "",
        [System.Runtime.CompilerServices.CallerFilePath] string filepath = "",
        [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            Match _match = Match.Empty;
            string[] _fileLines;
            string[] _fileSubLines;
            string _fileSubText;
            string _out;

            _fileLines = File.ReadLines(filepath).ToArray();

            _fileSubLines =
                _fileLines.Skip(linenumber - 1)
                .ToArray();

            _fileSubText = string.Join(
                separator: "\n",
                value: _fileSubLines);

            try
            {
                _match = (new Regex(@"(\{(\n.*)*\})\);")).Match(_fileSubText);

                _out =
                    (_match.Success)
                    ? _match.Groups[1].Value.ToString()
                    : string.Empty;

            }
            catch (Exception)
            {
                Debugger.Break();
                _out = string.Empty;

            }

            return ""
                + $"Caller Member Name: {membername}\n"
                + $"Caller File Path: {filepath}\n"
                + $"Caller Line Number: {linenumber}\n"
                + $"Action Body: {_out}"
                ;
        }


        public static List<TEnum> GetEnumList<TEnum>() where TEnum : Enum => ((TEnum[])Enum.GetValues(typeof(TEnum))).ToList();

        public static T FindBestMatch<T>(string input) where T : Enum
        {
            List<string> options = new List<string>();
            var array = Enum.GetValues(typeof(T));
            foreach (T e in array)
            {
                options.Add(e.ToString());
            }
            string bestMatch = WaterWatch.GetBestMatch(input, options);
            return (T)array.GetValue(options.IndexOf(bestMatch));
        }

        public static string Copy(this string obj)
        {
            return new string(obj.ToCharArray());
        }
        public static bool Copy(this bool obj)
        {
            bool t = false;
            if (obj) t = true;
            return t;
        }
        public static float Lerp(this float firstFloat, float secondFloat, float by)
        {
            return firstFloat * (1 - by) + secondFloat * by;
        }
        public static byte Lerp(this byte first, byte second, float by)
        {
            float a = first;
            float b = second;
            return (byte)Math.Floor(a.Lerp(b, by) + 0.5f);
        }
        public static byte Lerp(this byte first, byte second, double by)
        {
            float a = first;
            float b = second;
            return (byte)Math.Floor(a.Lerp(b, (float)by) + 0.5f);
        }
        public static int roundToNearest(this float i, int nearest)
        {
            return (int)(Math.Ceiling(i / (double)nearest) * nearest); // fixed
        }
        public static bool IsType(this object objectToCheck, Type typeToCheck)
        {
            if (objectToCheck is null) return false;
            if (objectToCheck.HasMethod("GetType"))
            {
                Type typeToCompare = objectToCheck.GetType();
                try
                {
                    if (typeToCompare == typeToCheck)
                    {
                        return true;
                    }
                    else if (typeToCompare.IsGenericType && typeToCompare.GetGenericTypeDefinition() == typeToCheck)
                    {
                        return true;
                    }
                }
                catch (Exception)
                {
                    return false;
                }
            }
            return false;
        }

        public static bool HasMethod(this object objectToCheck, string methodName)
        {
            var type = objectToCheck.GetType();
            return type.GetMethod(methodName) != null;
        }

        public static bool HasProperty(this object objectToCheck, string propertyName)
        {
            var type = objectToCheck.GetType();
            return type.GetProperty(propertyName) != null;
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
        public static int FindLast(this string value, string toFind)
        {
            return value.LastIndexOf(toFind);
        }
        public static int Find(this string value, string toFind)
        {
            return value.IndexOf(toFind);
        }
        public static int Find(this string value, string toFind, int startIndex)
        {
            return value.IndexOf(toFind, startIndex);
        }
        public static string RightOf(this string value, string toFind)
        {
            var start = value.Find(toFind);
            if (start >= 0)
            {
                start += toFind.Length;
                return value.Mid(start, value.Length - start);
            }
            else
            {
                return value;
            }
        }
        public static string LeftOf(this string value, string toFind)
        {
            var start = value.Find(toFind);
            if (start >= 0)
            {
                start += toFind.Length;
                return value.Mid(0, start);
            }
            else
            {
                return value;
            }
        }

        internal static Regex alphanumericregex = new Regex("^[a-zA-Z0-9]*$");
        public static bool IsAlphaNumeric(this string value)
        {
            return alphanumericregex.IsMatch(value);
        }
        internal static Regex numericregex = new Regex("^[0-9]*$");
        public static bool IsNumeric(this string value)
        {
            return numericregex.IsMatch(value);
        }
        public static string Left(this string value, int maxLength)
        {
            if (string.IsNullOrEmpty(value)) return value;
            maxLength = Math.Abs(maxLength);

            return (value.Length <= maxLength
                   ? value
                   : value.Substring(0, maxLength)
                   );
        }
        public static string Mid(this string orig, int startPosition, int length)
        {
            return orig.Substring(startPosition, length);
        }
        public static string Right(this string value, int maxLength)
        {
            if (string.IsNullOrEmpty(value)) return value;
            maxLength = Math.Abs(maxLength);

            return (value.Length <= maxLength
                   ? value
                   : value.Substring(value.Length - maxLength, maxLength)
                   );
        }
        public static int Count(this string value, char C, int upto)
        {
            int n = 0;
            if (upto >= 0)
            {
                for (int i = 0; i < upto && i < value.Length; i++)
                {
                    if (C == value[i])
                    {
                        ++n;
                    }
                }
            }
            else
            {
                for (int i = 0; i < value.Length; i++)
                {
                    if (C == value[i])
                    {
                        ++n;
                    }
                }
            }

            return n;
        }
        public static string[] Split(this string v, string delim)
        {
            var x = new List<string>() { delim };
            return v.Split(x.ToArray(), StringSplitOptions.None);
        }

        public static List<string> SplitNum(this string input, string delim, int numSplits)
        {
            List<string> result = new List<string>();
            if (numSplits <= 0)
            {
                result.Add(input);
                return result;
            }

            string[] reply = input.Split(delim);
            if (reply.Length > 0)
            {
                result.Add(reply[0]);
                int i;
                for (i = 1; i < numSplits; i++)
                {
                    if (reply.Length > i)
                    {
                        result.Add(reply[i]);
                    }
                }
                if (reply.Length > numSplits)
                {
                    string f = "";
                    for (; i < reply.Length; i++)
                    {
                        f = f.AddToDelimiter(reply[i], delim);
                    }
                    result.Add(f);
                }
            }
            return result;
        }
        public static string ToLiteral(this string input)
        {
            System.Text.StringBuilder literal = new System.Text.StringBuilder(input.Length + 2);
            foreach (var c in input)
            {
                switch (c)
                {
                    case '\"': literal.Append("\\\""); break;
                    case '\\': literal.Append(@"\\"); break;
                    case '\0': literal.Append(@"\0"); break;
                    case '\a': literal.Append(@"\a"); break;
                    case '\b': literal.Append(@"\b"); break;
                    case '\f': literal.Append(@"\f"); break;
                    case '\n': literal.Append(@"\n"); break;
                    case '\r': literal.Append(@"\r"); break;
                    case '\t': literal.Append(@"\t"); break;
                    case '\v': literal.Append(@"\v"); break;
                    default:
                        // ASCII printable character
                        if (c >= 0x20 && c <= 0x7e)
                        {
                            literal.Append(c);
                            // As UTF16 escaped character
                        }
                        else
                        {
                            literal.Append(@"\u");
                            literal.Append(((int)c).ToString("x4"));
                        }
                        break;
                }
            }
            return literal.ToString();
        }
        public static string AddToDelimiter(this string orig, string newContent, string delim)
        {
            if (orig.Length > 0) orig += delim;

            orig += newContent;

            return orig;
        }
        public static string AddLine(this string orig, string newContent)
        {
            return AddToDelimiter(orig, newContent, "\n");
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

        public static void EdmsHandle(this Exception e, string additionalMessage = null, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            // AppExtension.ManageException(e, additionalMessage, membername, filepath, linenumber);
        }

        public static DateTime to_DateTime(this cweeDateTime cdt)
        {
            return new DateTime(cdt.year, cdt.month, cdt.day, cdt.hour, cdt.minute, cdt.second, cdt.milliseconds);
        }
    }







    public class cweeMutex
    {
        //private long count = 0;
        private Mutex mut = new Mutex();
        public void Lock()
        {
            mut.WaitOne();
            return;// Interlocked.Increment(ref count);
        }
        public void Unlock()
        {
            mut.ReleaseMutex();
            return;// Interlocked.Decrement(ref count);
        }
    }

    public class cweeStopWatch
    {
        private ulong start = 0;
        private ulong prev = 0;
        bool active = false;

        public void Start()
        {
            start = (ulong)WaterWatch.GetNanosecondsSinceStart();
            active = true;
        }

        public void Stop()
        {
            prev = (ulong)WaterWatch.GetNanosecondsSinceStart();
            active = false;
        }

        public void Restart()
        {
            start = (ulong)WaterWatch.GetNanosecondsSinceStart();
            active = true;
        }

        public bool IsActive => active;

        public double ElapsedMilliseconds
        {
            get
            {
                double toReturn;

                if (active) prev = (ulong)WaterWatch.GetNanosecondsSinceStart();

                toReturn = (prev - start);
                toReturn /= 1000000.0; // total milliseconds between start and stop

                return toReturn;
            }
        }
    }
    public class AtomicInt
    {
        private long data = 0;
        public AtomicInt() { }
        public AtomicInt(long n) { data = n; }

        public void Set(long n)
        {
            data = n;
        }
        public long Get()
        {
            return Interlocked.Read(ref data);
        }
        public long Increment()
        {
            return Interlocked.Increment(ref data);
        }

        public long Decrement()
        {
            return Interlocked.Decrement(ref data);
        }

        public bool TryIncrementTo(long equalsTo)
        {
            if (equalsTo == Increment())
            {
                return true;
            }
            else
            {
                Decrement();
                return false;
            }
        }

    }


    public class cweeEvent<T>// where T : new()
    {
        public cweeEvent()
        {
        }

        public void InvokeEventAsync(object source, T args)
        {
            lock (Registrees)
            {
                if (Registrees.Count != 0)
                {
                    foreach (var x in Registrees)
                    {
                        var y = x.task;
                        EdmsTasks.InsertJob(() =>
                        {
                            y.Invoke(source, args);
                        }, EdmsTasks.Priority.Low, false, true, x.MemberName, x.FilePath, x.LineNumber);
                    }
                }
            }
        }
        public EdmsTasks.cweeTask InvokeEvent(object source, T args)
        {
            List<EdmsTasks.cweeTask> tasks = null;
            lock (Registrees)
            {
                if (Registrees.Count != 0)
                {
                    tasks = new List<EdmsTasks.cweeTask>();
                    foreach (var x in Registrees)
                    {
                        var y = x.task;
                        tasks.Add(new EdmsTasks.cweeTask(() =>
                        {
                            y.Invoke(source, args);
                        }, false, true, x.MemberName, x.FilePath, x.LineNumber));
                    }
                }
            }
            if (tasks != null && tasks.Count > 0)
            {
                if (tasks.Count > 0)
                {
                    for (int i = 0; i < (tasks.Count - 1); i++)
                    {
                        tasks[i].ContinueWith(tasks[i + 1]);
                    }
                    EdmsTasks.InsertJob(tasks[0]);
                    return tasks[tasks.Count - 1];
                }
                else
                {
                    return EdmsTasks.cweeTask.CompletedTask(null);
                }
                // return EdmsTasks.cweeTask.InsertListAsTask(tasks);
            }
            else
                return EdmsTasks.cweeTask.CompletedTask(null);
        }
        public static cweeEvent<T> operator +(cweeEvent<T> ev, Action<object, T> t)
        {
            lock (ev.Registrees)
            {
                ev.Registrees.Add(new cweeEventHandle(t));
            }

            return ev;
        }
        public static cweeEvent<T> operator -(cweeEvent<T> ev, Action<object, T> t)
        {
            lock (ev.Registrees)
            {
                int index = ev.Registrees.FindIndex(x => x.task.Method == t.Method);
                if (index >= 0)
                {
                    ev.Registrees.RemoveAt(index);
                }
            }
            return ev;
        }
        internal List<cweeEventHandle> Registrees = new List<cweeEventHandle>();
        internal class cweeEventHandle
        {
            public cweeEventHandle(Action<object, T> t, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                task = t;
                MemberName = membername;
                FilePath = filepath;
                LineNumber = linenumber;
            }
            public Action<object, T> task;
            public string MemberName;
            public string FilePath;
            public int LineNumber;
        }
    }

    public class EdmsQueue<T>
    {
        private LinkedList<T> front = new LinkedList<T>();
        private Queue<T> back = new Queue<T>(10000);
        private Mutex mut = new Mutex();

        public void Enqueue(T obj)
        {
            mut.WaitOne();
            back.Enqueue(obj);
            mut.ReleaseMutex();
        }

        public void EnqueueFront(T obj)
        {
            mut.WaitOne();
            front.AddFirst(obj);
            mut.ReleaseMutex();
        }

        public bool Dequeue(out T obj)
        {
            obj = default(T);
            mut.WaitOne();
            if (front.Count > 0)
            {
                obj = front.First.Value;
                front.RemoveFirst();
                mut.ReleaseMutex();
                return true;
            }
            else if (back.Count > 0)
            {
                obj = back.Dequeue();
                mut.ReleaseMutex();
                return true;
            }
            mut.ReleaseMutex();
            return false;
        }

        public bool Peek(out T obj)
        {
            obj = default(T);
            mut.WaitOne();
            if (front.Count > 0)
            {
                obj = front.First.Value;
                mut.ReleaseMutex();
                return true;
            }
            else if (back.Count > 0)
            {
                obj = back.Peek();
                mut.ReleaseMutex();
                return true;
            }
            mut.ReleaseMutex();
            return false;
        }

        public List<T> DequeueAll()
        {
            List<T> toReturn = new List<T>();
            mut.WaitOne();

            toReturn.AddRange(front.ToList());
            front = new LinkedList<T>();
            toReturn.AddRange(back.ToList());
            back = new Queue<T>();

            mut.ReleaseMutex();
            return toReturn;
        }

        public int Count
        {
            get
            {
                int n = 0;

                mut.WaitOne();
                n = front.Count + back.Count;
                mut.ReleaseMutex();

                return n;
            }

        }

        public LinkedList<T> UnsafeGetFront()
        {
            return front;
        }
        public bool UnsafeDequeue(out T obj)
        {
            obj = default(T);
            if (front.Count > 0)
            {
                obj = front.First.Value;
                front.RemoveFirst();
                return true;
            }
            else if (back.Count > 0)
            {
                obj = back.Dequeue();
                return true;
            }
            return false;
        }
        public Queue<T> UnsafeGetBack()
        {
            return back;
        }
        public void Lock()
        {
            mut.WaitOne();
        }
        public bool TryLock()
        {
            return mut.WaitOne(1);
        }
        public void Unlock()
        {
            mut.ReleaseMutex();
        }

    }
    public class EdmsTasks
    {
        public class cweeAction
        {
            public cweeAction()
            {
                todo = null;
            }

            public cweeAction(Action ac, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                todo = ac;
                MemberName = membername;
                FilePath = filepath;
                LineNumber = linenumber;
            }

            public cweeAction(Func<dynamic> ac, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                todo = ac;
                MemberName = membername;
                FilePath = filepath;
                LineNumber = linenumber;
            }

            public dynamic Invoke()
            {
                dynamic result = null;
                if (todo is null)
                {
                    return result;
                }
                else if (todo is Action)
                {
                    try
                    {
                        todo.Invoke();
                    }
                    catch (Exception)
                    {
                        //e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
                    }
                    return result;
                }
                else
                {
                    try
                    {
                        result = todo.Invoke();
                    }
                    catch (Exception)
                    {
                        //e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
                    }
                    return result;
                }
            }
            public dynamic Invoke(bool _)
            {
                dynamic result = null;
                if (todo is null)
                {
                    return result;
                }
                else if (todo is Action)
                {
                    try
                    {
                        todo.Invoke();
                    }
                    catch (Exception)
                    {
                        //e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
                    }
                    return result;
                }
                else
                {
                    try
                    {
                        result = todo.Invoke();
                    }
                    catch (Exception)
                    {
                        //e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
                    }
                    return result;
                }
            }

            dynamic todo = null;

            public string LambdaSource => string.IsNullOrEmpty(FilePath) || string.IsNullOrEmpty(MemberName) ? "UNKNOWN" : $"{MemberName}:{FilePath.Split("\\").Last()}:{LineNumber}";

            public string MemberName;
            public string FilePath;
            public int LineNumber;

            public MethodInfo Method
            {
                get
                {
                    if (todo is null)
                    {
                        return null;
                    }
                    else if (todo is Action)
                    {
                        return todo.Method;
                    }
                    else
                    {
                        return todo.Method;
                    }
                }
            }

            public object Target
            {
                get
                {
                    if (todo is null)
                    {
                        return null;
                    }
                    else if (todo is Action)
                    {
                        return todo.Target;
                    }
                    else
                    {
                        return todo.Target;
                    }
                }
            }
        }
        public class cweeTask
        {
            public object Tag;
            public string LambdaSource => (todo != null) ? todo.LambdaSource : "";
            public static cweeTask ContinueWhenTrue(Func<bool> whenTrue, dynamic returnWhenFinished = null)
            {
                return InsertJob(() =>
                {
                    if (whenTrue.Invoke())
                    {
                        return returnWhenFinished;
                    }
                    else
                    {
                        return ContinueWhenTrue(whenTrue, returnWhenFinished);
                    }
                });
            }
            public static cweeTask<T> ContinueWhenTrue<T>(Func<bool> whenTrue, T returnWhenFinished)
            {
                return InsertJob(() =>
                {
                    if (whenTrue.Invoke())
                    {
                        return returnWhenFinished;
                    }
                    else
                    {
                        return ContinueWhenTrue(whenTrue, returnWhenFinished);
                    }
                });
            }

            public static cweeTask TrueWhenCompleted<T>(List<cweeTask<T>> afterWhom, dynamic returnWhenFinished = null)
            {
                if (afterWhom != null && afterWhom.Count > 0)
                {
                    var counter = new AtomicInt(afterWhom.Count);
                    cweeTask t = new cweeTask() { Result = returnWhenFinished, Tag = counter, mainThreadOnly = false, canBeDeferred = true };
                    foreach (cweeTask<T> x in afterWhom)
                    {
                        if (x.IsFinished)
                        {
                            if ((t.Tag as AtomicInt).Decrement() == 0)
                            {
                                // we're done
                                t.IsFinished = true;
                            }
                        }
                        else
                        {
                            x.ContinueWith(() =>
                            {
                                if ((t.Tag as AtomicInt).Decrement() == 0)
                                {
                                    // we're done
                                    t.IsFinished = true;
                                }
                            }, false);
                        }
                    }
                    return t;
                }
                else
                {
                    return cweeTask.CompletedTask(returnWhenFinished);
                }
            }

            public static cweeTask TrueWhenCompleted(List<cweeTask> afterWhom, dynamic returnWhenFinished = null)
            {
                if (afterWhom != null && afterWhom.Count > 0)
                {
                    var counter = new AtomicInt(afterWhom.Count);
                    cweeTask t = new cweeTask() { Result = returnWhenFinished, Tag = counter, mainThreadOnly = false, canBeDeferred = true };
                    foreach (var x in afterWhom)
                    {
#if true
                        if (x.IsFinished)
                        {
                            if ((t.Tag as AtomicInt).Decrement() == 0)
                            {
                                // we're done
                                t.IsFinished = true;
                            }
                        }
                        else
                        {
                            x.ContinueWith(() =>
                            {
                                if ((t.Tag as AtomicInt).Decrement() == 0)
                                {
                                    // we're done
                                    t.IsFinished = true;
                                }
                            }, false);
                        }
#else
                        x.OnFinished += (object sender, FinishedArgs e) =>
                        {
                            if ((t.Tag as AtomicInt).Decrement() == 0)
                            {
                                // we're done
                                t.IsFinished = true;
                            }
                        };
#endif
                    }
                    return t;
                }
                else
                {
                    return cweeTask.CompletedTask(returnWhenFinished);
                }
            }
            public static cweeTask<T> TrueWhenCompleted<T>(List<cweeTask> afterWhom, T returnWhenFinished)
            {
                if (afterWhom != null && afterWhom.Count > 0)
                {
                    var counter = new AtomicInt(afterWhom.Count);
                    cweeTask t = new cweeTask() { Result = returnWhenFinished, Tag = counter, mainThreadOnly = false, canBeDeferred = true };
                    foreach (var x in afterWhom)
                    {
#if true
                        if (x.IsFinished)
                        {
                            if ((t.Tag as AtomicInt).Decrement() == 0)
                            {
                                // we're done
                                t.IsFinished = true;
                            }
                        }
                        else
                        {
                            x.ContinueWith(() =>
                            {
                                if ((t.Tag as AtomicInt).Decrement() == 0)
                                {
                                    // we're done
                                    t.IsFinished = true;
                                }
                            }, false);
                        }
#else
                        x.OnFinished += (object sender, FinishedArgs e) =>
                        {
                            if ((t.Tag as AtomicInt).Decrement() == 0)
                            {
                                // we're done
                                t.IsFinished = true;
                            }
                        };
#endif
                    }
                    return t;
                }
                else
                {
                    return cweeTask.CompletedTask(returnWhenFinished);
                }
            }

            public static cweeTask InsertListAsTask(List<cweeTask> afterWhom, bool UnorderedSubmissions = true)
            {
                if (UnorderedSubmissions)
                {
#if true
                    var toReturn = TrueWhenCompleted(afterWhom);
                    foreach (var job in afterWhom)
                    {
                        EdmsTasks.InsertJob(job);
                    }
                    return toReturn;
#else
                    int nT = EdmsTasks.NumThreads - 1;
                    if (nT <= 1)//  || System.Diagnostics.Debugger.IsAttached)
                    {
                        if (afterWhom.Count > 0)
                        {
                            for (int i = 0; i < (afterWhom.Count - 1); i++)
                            {
                                afterWhom[i].ContinueWith(afterWhom[i + 1]);
                            }
                            EdmsTasks.InsertJob(afterWhom[0]);
                            return afterWhom[afterWhom.Count - 1];
                        }
                        else
                        {
                            return cweeTask.CompletedTask(null);
                        }
                    }
                    else if (afterWhom.Count <= nT || afterWhom.Count <= 1)
                    {
                        foreach (var x in afterWhom)
                        {
                            EdmsTasks.InsertJob(x);
                        }
                        return TrueWhenCompleted(afterWhom);
                    }
                    else
                    {
                        List<List<cweeTask>> jobs_into_threads = new List<List<cweeTask>>();
                        for (int C = 0; C < nT; C++)
                        {
                            jobs_into_threads.Add(new List<cweeTask>());
                        }

                        var toReturn = TrueWhenCompleted(afterWhom);
                        int n = 0;

                        while (n < afterWhom.Count)
                        {
                            for (int C = 0; (C < nT) && (n < afterWhom.Count); C++)
                            {
                                jobs_into_threads[C].Add(afterWhom[n]);
                                n++;
                            }
                        }

                        foreach (var list in jobs_into_threads)
                        {
                            if (list.Count > 0)
                            {
                                for (int i = list.Count - 1; i >= 1; i--)
                                {
                                    list[i - 1].ContinueWith(list[i]);
                                }
                                EdmsTasks.InsertJob(list[0]);
                            }
                        }

                        return toReturn;
                    }

#endif
                }
                else
                {
                    if (afterWhom.Count > 0)
                    {
                        var toReturn = TrueWhenCompleted(afterWhom);
                        for (int i = 0; i < (afterWhom.Count - 1); i++)
                        {
                            afterWhom[i].ContinueWith(afterWhom[i + 1]);
                        }
                        EdmsTasks.InsertJob(afterWhom[0]);
                        return toReturn;
                    }
                    else
                    {
                        return cweeTask.CompletedTask(null);
                    }
                }
            }

            public cweeTask()
            {
                todo = null;
                mainThreadOnly = false;
                canBeDeferred = true;
            }
            public cweeTask(cweeAction _todo, bool _mainThreadOnly, bool _canBeDeferred)
            {
                todo = _todo;
                mainThreadOnly = _mainThreadOnly;
                canBeDeferred = _canBeDeferred;
            }
            public cweeTask(Action _todo, bool _mainThreadOnly, bool _canBeDeferred, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                todo = new cweeAction(_todo, membername, filepath, linenumber);
                mainThreadOnly = _mainThreadOnly;
                canBeDeferred = _canBeDeferred;
            }
            public cweeTask(Func<dynamic> _todo, bool _mainThreadOnly, bool _canBeDeferred, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                todo = new cweeAction(_todo, membername, filepath, linenumber);
                mainThreadOnly = _mainThreadOnly;
                canBeDeferred = _canBeDeferred;
            }

            public void QueueJob()
            {
                EdmsTasks.InsertJob(this);
            }

            public static cweeTask CompletedTask(dynamic r)
            {
                var x = new cweeTask();
                x.Result = r;
                if (Interlocked.Increment(ref x._complete) == 1)
                {
                    x.startedFinish = true;
                }
                x.todo = null;
                x.mainThreadOnly = false;
                x.canBeDeferred = true;

                return x;
            }
            public static cweeTask<T> CompletedTask<T>(T r)
            {
                var x = new cweeTask();
                x.Result = r;
                if (Interlocked.Increment(ref x._complete) == 1)
                {
                    x.startedFinish = true;
                }
                x.todo = null;
                x.mainThreadOnly = false;
                x.canBeDeferred = true;

                return x;
            }
            public void SetFinished(dynamic r)
            {
                this.Result = r;
                this.todo = null;
                this.mainThreadOnly = false;
                this.canBeDeferred = true;
                this.IsFinished = true;
            }

            public cweeAction todo { get; set; }
            public bool mainThreadOnly { get; set; }
            public bool canBeDeferred { get; set; }
            public Priority priority { get; set; } = Priority.Low;

            private long _complete = 0;
            public bool IsFinished
            {
                get
                {
                    return (Interlocked.Read(ref this._complete) > 0.0);
                }
                set
                {
                    if (value)
                    {
                        if (Interlocked.Increment(ref this._complete) == 1)
                        {
                            startedFinish = true;
                            Finished();
                        }
                        else
                        {
                            Interlocked.Decrement(ref this._complete);
                        }
                    }
                }
            }



            public class FinishedArgs
            {
                public FinishedArgs() { }

                public cweeTask whoChanged;
            }

            public cweeEvent<FinishedArgs> OnFinished = new cweeEvent<FinishedArgs>();
            internal void Finished()
            {
                // OnFinished.InvokeEventAsync(this, new FinishedArgs() { whoChanged = this });
                OnFinished.InvokeEvent(this, new FinishedArgs() { whoChanged = this });
            }


            private dynamic _result = null;
            public dynamic Result
            {
                get { return _result; }
                set { _result = value; }
            }

            private long _startedFinish = 0;
            internal bool startedFinish
            {
                get
                {
                    return (Interlocked.Read(ref this._startedFinish) > 0.0);
                }
                set
                {
                    if (value)
                    {
                        if (Interlocked.Increment(ref this._startedFinish) != 1)
                        {
                            Interlocked.Decrement(ref this._startedFinish);
                        }
                    }
                }
            }

            public void Resolve()
            {
                if (!IsFinished)
                {
                    Result = todo.Invoke(); // do the job
                    _resolve();
                }
            }

            public void Resolve(bool _)
            {
                if (!IsFinished)
                {
                    Result = todo.Invoke(_); // do the job
                    _resolve(_);
                }
            }

            private void _resolve(/*int n = 0*/)
            {
                //if (n > 100)
                //{
                //    throw (new Exception(Environment.StackTrace));
                //}

                if (Result is null)
                {
                    this.IsFinished = true;
                }
                else if (Result is cweeTask)
                {
                    Result.ContinueWith((Action)(() =>
                    {
                        try
                        {
                            Result = Result.Result;
                        }
                        catch (Exception e)
                        {
                            e.EdmsHandle(null, this.todo.MemberName, this.todo.FilePath, this.todo.LineNumber);
                        }
                        _resolve(/*n + 1*/);
                    }), false);
                }
                else if ((Result is object) && (Result as object).IsType(typeof(cweeTask<>)))
                {
                    Result.ContinueWith((Action)(() =>
                    {
                        try
                        {
                            Result = ((cweeTask)Result).Result;
                        }
                        catch (Exception e)
                        {
                            e.EdmsHandle(null, this.todo.MemberName, this.todo.FilePath, this.todo.LineNumber);
                        }
                        _resolve(/*n + 1*/);
                    }), false);
                }
                else
                {
                    this.IsFinished = true;
                }
            }
            private void _resolve(bool _ /*int n = 0*/)
            {
                //if (n > 100)
                //{
                //    throw (new Exception(Environment.StackTrace));
                //}

                if (Result is null)
                {
                    this.IsFinished = true;
                }
                else if (Result is cweeTask)
                {
                    Result.ContinueWith((Action)(() =>
                    {
                        try
                        {
                            Result = Result.Result;
                        }
                        catch (Exception e)
                        {
                            e.EdmsHandle(null, this.todo.MemberName, this.todo.FilePath, this.todo.LineNumber);
                        }
                        _resolve(_ /*n + 1*/);
                    }), false);
                }
                else if ((Result is object) && (Result as object).IsType(typeof(cweeTask<>)))
                {
                    Result.ContinueWith((Action)(() =>
                    {
                        try
                        {
                            Result = ((cweeTask)Result).Result;
                        }
                        catch (Exception e)
                        {
                            e.EdmsHandle(null, this.todo.MemberName, this.todo.FilePath, this.todo.LineNumber);
                        }
                        _resolve(_ /*n + 1*/);
                    }), false);
                }
                else
                {
                    this.IsFinished = true;
                }
            }
            public cweeTask ContinueWith(cweeTask _todo)
            {
                cweeTask newTask = _todo;

                if (startedFinish)
                {
                    Impl_ContinueWithOnFinished(newTask);
                }
                else
                {
                    OnFinished += (object sender, FinishedArgs e) =>
                    {
                        Impl_ContinueWithOnFinished(newTask);
                    };
                }

                return newTask;
            }
            public cweeTask ContinueWith(Action _todo, bool mainThread, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                cweeTask newTask = new cweeTask(_todo, mainThread, true, membername, filepath, linenumber);

                if (startedFinish)
                {
                    Impl_ContinueWithOnFinished(newTask);
                }
                else
                {
                    OnFinished += (object sender, FinishedArgs e) =>
                    {
                        Impl_ContinueWithOnFinished(newTask);
                    };
                }

                return newTask;
            }
            public cweeTask ContinueWith(Func<dynamic> _todo, bool mainThread, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                cweeTask<dynamic> newTask = new cweeTask<dynamic>(_todo, mainThread, true, membername, filepath, linenumber);

                if (startedFinish)
                {
                    Impl_ContinueWithOnFinished(newTask);
                }
                else
                {
                    OnFinished += (object sender, FinishedArgs e) =>
                    {
                        Impl_ContinueWithOnFinished(newTask);
                    };
                }

                return newTask;
            }
            public cweeTask<T> ContinueWith<T>(Func<T> _todo, bool mainThread, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                cweeTask<T> newTask = new cweeTask<T>(_todo, mainThread, true, membername, filepath, linenumber);

                if (startedFinish)
                {
                    Impl_ContinueWithOnFinished(newTask);
                }
                else
                {
                    OnFinished += (object sender, FinishedArgs e) =>
                    {
                        Impl_ContinueWithOnFinished(newTask);
                    };
                }

                return newTask;
            }


            private static void Impl_ContinueWithOnFinished(cweeTask followUp)
            {
                EdmsTasks.InsertJob(followUp);
            }
        }





        public static bool GuiTaskCheck()
        {
            if (numJobs() < 10) return true;
            return true;
        }

        public static int numJobs()
        {
            return parallel_actions.Count + main_actions.Count;
        }
        public enum Priority
        {
            Low,
            High
        }
        private static EdmsQueue<cweeTask> parallel_actions = new EdmsQueue<cweeTask>();
        private static EdmsQueue<cweeTask> main_actions = new EdmsQueue<cweeTask>();

        public static void SetMaxNumThreads(int num)
        {
#if EdmsMultitask
            NumThreads = num;
#endif
        }

#if EdmsMultitask
        static int maxNumThreads = WaterWatch.GetNumLogicalCoresOnMachine() - 1; // 16; // was const
        public const double millisecondsPerFrame = (double)((1000.0 / 60.0) / 3.0); // the last /3.0 is meant to encourage smoother framerates 
        private static int _NumThreads = maxNumThreads;
        public static int NumThreads { get { return Volatile.Read(ref _NumThreads); } set { if (value > maxNumThreads) Interlocked.Exchange(ref _NumThreads, maxNumThreads); else Interlocked.Exchange(ref _NumThreads, value); } }

        private static long _waiting = 0;
        public static bool Waiting
        {
            get
            {
                bool waiting = (Interlocked.Read(ref _waiting) > 0.0); // 1 indicates waiting
                return waiting;
            }
            set
            {
                if (value)
                    Interlocked.Increment(ref _waiting);
                else
                    Interlocked.Decrement(ref _waiting);
            }
        }
        private static Mutex mut = new Mutex();
        private static Queue<cweeTask> mainThreadQueue = new Queue<cweeTask>(10000);
        //private static int mainThreadSyncFlag = 0;
#if EdmsTaskCounter
        public static cweeStopWatch QueuedMainThreadTaskStopwatch = new cweeStopWatch();
#endif
#else
        private static Task task = null;
#endif

#if EdmsTaskCounter
        public static Dictionary<string, double> ActionToTimeSpan = new Dictionary<string, double>();
#endif




        /// <summary>
        ///  Force the provided action into the Queue, regardless of whether a similar job already exists. 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="priority"></param>
        public static cweeTask InsertJob(Action action, Priority priority = Priority.Low, bool mainThreadOnly = false, bool canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            cweeTask toEnqueue = new cweeTask(action, mainThreadOnly, canBeDeferred, membername, filepath, linenumber);

            if (priority == Priority.Low)
            {
                if (mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.Enqueue(toEnqueue);
                }
                else
                {
                    parallel_actions.Enqueue(toEnqueue);
                }
            }
            else
            {
                if (mainThreadOnly)
                {
                    main_actions.EnqueueFront(toEnqueue);
                }
                else
                {
                    parallel_actions.EnqueueFront(toEnqueue);
                }
            }
            return toEnqueue;
        }
        public static cweeTask InsertJob(Action action, bool mainThreadOnly, bool canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            cweeTask toEnqueue = new cweeTask(action, mainThreadOnly, canBeDeferred, membername, filepath, linenumber);
            {
                if (mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.Enqueue(toEnqueue);
                }
                else
                {
                    parallel_actions.Enqueue(toEnqueue);
                }
            }
            return toEnqueue;
        }

        /// <summary>
        ///  Force the provided action into the Queue, regardless of whether a similar job already exists. 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="priority"></param>
        public static cweeTask<T> InsertJob<T>(Func<T> action, Priority priority = Priority.Low, bool mainThreadOnly = false, bool canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            cweeTask<T> toEnqueue = new cweeTask<T>(action, mainThreadOnly, canBeDeferred, membername, filepath, linenumber);

            if (priority == Priority.Low)
            {
                if (mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.Enqueue(toEnqueue);
                }
                else
                {
                    parallel_actions.Enqueue(toEnqueue);
                }
            }
            else
            {
                if (mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.EnqueueFront(toEnqueue);
                }
                else
                {
                    parallel_actions.EnqueueFront(toEnqueue);
                }
            }
            return toEnqueue;
        }
        /// <summary>
        ///  Force the provided action into the Queue, regardless of whether a similar job already exists. 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="priority"></param>
        public static cweeTask<T> InsertJob<T>(Func<T> action, bool mainThreadOnly, bool canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            cweeTask<T> toEnqueue = new cweeTask<T>(action, mainThreadOnly, canBeDeferred, membername, filepath, linenumber);

            if (mainThreadOnly)
            {
                // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                main_actions.Enqueue(toEnqueue);
            }
            else
            {
                parallel_actions.Enqueue(toEnqueue);
            }

            return toEnqueue;
        }

        /// <summary>
        ///  Force the provided action into the Queue, regardless of whether a similar job already exists. 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="priority"></param>
        public static cweeTask InsertJob(Func<dynamic> action, Priority priority = Priority.Low, bool mainThreadOnly = false, bool canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            cweeTask toEnqueue = new cweeTask(action, mainThreadOnly, canBeDeferred, membername, filepath, linenumber);

            if (priority == Priority.Low)
            {
                if (mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.Enqueue(toEnqueue);
                }
                else
                {
                    parallel_actions.Enqueue(toEnqueue);
                }
            }
            else
            {
                if (mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.EnqueueFront(toEnqueue);
                }
                else
                {
                    parallel_actions.EnqueueFront(toEnqueue);
                }
            }
            return toEnqueue;
        }
        /// <summary>
        ///  Force the provided action into the Queue, regardless of whether a similar job already exists. 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="priority"></param>
        public static cweeTask InsertJob(Func<dynamic> action, bool mainThreadOnly, bool canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            cweeTask toEnqueue = new cweeTask(action, mainThreadOnly, canBeDeferred, membername, filepath, linenumber);

            if (mainThreadOnly)
            {
                // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                // main_actions.Enqueue(toEnqueue);
                main_actions.Enqueue(toEnqueue);
            }
            else
            {
                parallel_actions.Enqueue(toEnqueue);
            }

            return toEnqueue;
        }

        /// <summary>
        ///  Force the provided action into the Queue, regardless of whether a similar job already exists. 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="priority"></param>                       
        public static cweeTask InsertJob(cweeTask toEnqueue)
        {
            if (toEnqueue.priority == Priority.Low)
            {
                if (toEnqueue.mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.Enqueue(toEnqueue);
                }
                else
                {
                    parallel_actions.Enqueue(toEnqueue);
                }
            }
            else if (toEnqueue.mainThreadOnly)
            {
                // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                main_actions.EnqueueFront(toEnqueue);
            }
            else
            {
                parallel_actions.EnqueueFront(toEnqueue);
            }
            return toEnqueue;
        }
        /// <summary>
        ///  Force the provided action into the Queue, regardless of whether a similar job already exists. 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="priority"></param>
        public static cweeTask<T> InsertJob<T>(cweeTask<T> toEnqueue)
        {
            if (toEnqueue.priority == Priority.Low)
            {
                if (toEnqueue.mainThreadOnly)
                {
                    // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                    main_actions.Enqueue(toEnqueue);
                }
                else
                {
                    parallel_actions.Enqueue(toEnqueue);
                }
            }
            else if (toEnqueue.mainThreadOnly)
            {
                // specific case: simply do the job on the main thread immediately. No need to pop the list, etc. 
                main_actions.EnqueueFront(toEnqueue);
            }
            else
            {
                parallel_actions.EnqueueFront(toEnqueue);
            }
            return toEnqueue;
        }

        // private static AtomicInt mainThreadPostLock = new AtomicInt();
        public static AtomicInt mainThreadPostLock = new AtomicInt();
        public static AtomicInt mainThreadID = new AtomicInt(-1);
        public static bool IsMainThread() {
            return System.Threading.Thread.CurrentThread.ManagedThreadId == mainThreadID.Get();
        }
        public static System.Collections.Concurrent.ConcurrentDictionary<string, Stopwatch> mainThreadTimers = new System.Collections.Concurrent.ConcurrentDictionary<string, Stopwatch>();

        public static bool DoWriteAllJobTimes = false;

        internal static int numCores = WaterWatch.GetNumLogicalCoresOnMachine() - 1;
        internal static AtomicInt numParallelJobs = new AtomicInt(0);
        internal static AtomicInt mainThreadLooping = new AtomicInt(0);
        public static string _MainThreadJobName = "";
        public static string MainThreadJobName { get { return _MainThreadJobName; } set { _MainThreadJobName = value; } }

        public static bool DoJob()
        {
            bool thisIsOnMainThread = IsMainThread();
            bool toReturn = true;

            {
                //if (isInForeground)
                {
                    // WORKS BUT GETS OVERWHELMED WHEN LOADING A LARGE SCRIPT... IDK WHY
#if false
                    var parallelToDo = parallel_actions.DequeueAll();
                    foreach (var t in parallelToDo)
                    {
                        Task.Run(()=> {
                            if (t.mainThreadOnly)
                            {
                                main_actions.Enqueue(t);
                            }
                            else
                            {
                                t.Resolve();
                            }
                        });
                    }
#else
                    var parallelToDo = parallel_actions.DequeueAll();
                    if (parallelToDo.Count == 1)
                    {
                        Task.Run(() =>
                        {
                            parallelToDo[0].Resolve();
                        });
                    }
                    else if (parallelToDo.Count >= 2)
                    {
                        Parallel.ForEach(parallelToDo, (cweeTask t) =>
                        {
                            if (t.mainThreadOnly)
                            {
                                main_actions.Enqueue(t);
                            }
                            else
                            {
                                t.Resolve();
                            }
                        });
                    }
                    ////if (parallelToDo.Count > 0) {
                    //Parallel.ForEach(parallelToDo, (cweeTask t) =>
                    //    {
                    //        if (t.mainThreadOnly)
                    //        {
                    //            main_actions.Enqueue(t);
                    //        }
                    //        else
                    //        {
                    //            t.Resolve();
                    //        }
                    //    });
                    ////}
#endif
                }

                {
                    var sw = new Stopwatch();
                    sw.Start();
                    while (main_actions.Dequeue(out cweeTask todo))
                    {
                        var sww = mainThreadTimers.GetOrAdd(todo.LambdaSource, new Stopwatch());
                        bool wasRunning = sww.IsRunning;
                        if (!wasRunning) sww.Start();

                        todo.Resolve(true);

                        if (!wasRunning) sww.Stop();

                        if (sw.Elapsed.TotalSeconds > (1.0 / 60))
                        {
                            break;
                        }
                    }
                    sw.Stop();
                }
            }

            return toReturn;
        }
    }
    public class cweeTask<T>
    {
        public object Tag
        {
            get
            {
                return actual.Tag;
            }
            set
            {
                actual.Tag = value;
            }
        }

        //public EdmsTasks.cweeAsyncOperation<T> AsAsyncOperation()
        //{
        //    return new EdmsTasks.cweeAsyncOperation<T>(actual);
        //}
#if false
        public EdmsTasks.cweeAsyncOperation<D> AsAsyncOperation<D>()
        {
            return new EdmsTasks.cweeAsyncOperation<D>(actual);
        }
#endif
        public void Resolve()
        {
            actual.Resolve();
        }

        public string LambdaSource => (todo != null) ? todo.LambdaSource : "";
        public cweeTask()
        {
            actual = new EdmsTasks.cweeTask();
        }
        public cweeTask(EdmsTasks.cweeTask other)
        {
            actual = other;
        }
        public cweeTask(Func<T> _todo, bool _mainThreadOnly, bool _canBeDeferred, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            actual = new EdmsTasks.cweeTask(new EdmsTasks.cweeAction(() => { return _todo.Invoke(); }, membername, filepath, linenumber), _mainThreadOnly, _canBeDeferred);
        }

        public EdmsTasks.cweeAction todo => actual.todo;
        public bool mainThreadOnly => actual.mainThreadOnly;
        public bool canBeDeferred => actual.canBeDeferred;
        public EdmsTasks.Priority priority => actual.priority;
        public bool IsFinished
        {
            get
            {
                return actual.IsFinished;
            }
            set
            {
                actual.IsFinished = value;
            }
        }
        //public dynamic Result
        //{
        //    get
        //    {
        //        return actual.Result;
        //    }
        //    set
        //    {
        //        actual.Result = value;
        //    }
        //}
        public T Result
        {
            get
            {
                return actual.Result;
            }
            set
            {
                actual.Result = value;
            }
        }

        public EdmsTasks.cweeTask ContinueWith(EdmsTasks.cweeTask _todo) => actual.ContinueWith(_todo);
        public EdmsTasks.cweeTask ContinueWith(Action _todo, bool mainThread, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0) => actual.ContinueWith(_todo, mainThread, membername, filepath, linenumber);
        public EdmsTasks.cweeTask ContinueWith(Func<dynamic> _todo, bool mainThread, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0) => actual.ContinueWith(_todo, mainThread, membername, filepath, linenumber);
        public cweeTask<Q> ContinueWith<Q>(Func<Q> _todo, bool mainThread, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0) => actual.ContinueWith(_todo, mainThread, membername, filepath, linenumber);

        public static implicit operator EdmsTasks.cweeTask(cweeTask<T> g) => g.actual;
        public static implicit operator cweeTask<T>(EdmsTasks.cweeTask a) => new cweeTask<T>(a);

        public static implicit operator cweeTask<cweeTask<T>>(cweeTask<T> g) => g.actual;
        public static implicit operator cweeTask<T>(cweeTask<cweeTask<T>> a) => new cweeTask<T>(a);

        private EdmsTasks.cweeTask actual;
    }

    public class cweeTimer
    {
        private TimerState _state;
        private double _seconds_interval;
        public cweeTimer(double Interval_seconds, Action action, bool mainThread = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            _seconds_interval = Interval_seconds;

            _state = new TimerState()
            {
                _mainThread = mainThread,
                _action = new EdmsTasks.cweeAction(action, membername, filepath, linenumber),
                _seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds),
                _init_seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds)
            };
            _state._timer = new Timer(
                    new TimerCallback(TimerTask),
                   _state,
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(Interval_seconds), (int)(1000.0 * (Interval_seconds - Math.Floor(Interval_seconds)))
                   ),
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(Interval_seconds), (int)(1000.0 * (Interval_seconds - Math.Floor(Interval_seconds)))
                   )
            );
        }
        ~cweeTimer()
        {
            Stop();
        }

        public class TimerState
        {
            public bool _mainThread;
            public EdmsTasks.cweeAction _action;
            public double _seconds_interval;
            public double _init_seconds_interval;
            public System.Threading.Timer _timer;

            internal int _count_misses = 0;
            internal int _working = 0;
        }

        private static void TimerTask(object timerState)
        {
            TimerState t = (timerState as TimerState);

            if (Interlocked.Increment(ref t._working) == 1)
            {
                if (t._mainThread)
                {
                    var newTask = new EdmsTasks.cweeTask(t._action, true, true);
                    newTask.ContinueWith(() => {
                        Interlocked.Decrement(ref t._working);
                    }, false);
                    EdmsTasks.InsertJob(newTask);
                }
                else
                {
                    var newTask = new EdmsTasks.cweeTask(t._action, false, true);
                    newTask.ContinueWith(() => {
                        Interlocked.Decrement(ref t._working);
                    }, false);
                    EdmsTasks.InsertJob(newTask);
                }
            }
            else
            {
                Interlocked.Decrement(ref t._working);
            }
        }

        public void SetInterval(double Interval_seconds)
        {
            _seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds);
            _state._seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds);
            _state._timer.Change((int)(1000.0 * _seconds_interval), (int)(1000.0 * _seconds_interval));
        }

        public bool IsActive()
        {
            return (_state._timer != null);
        }

        public void Stop()
        {
            if (IsActive())
            {
                try
                {
                    _state._timer.Change(Timeout.Infinite, Timeout.Infinite);
                    _state._timer.Dispose();
                    _state._timer = null;
                }
                catch (Exception) { }
            }
        }

        public void Restart()
        {
            Stop();
            _state._timer = new Timer(
                    new TimerCallback(TimerTask),
                   _state,
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(_seconds_interval), (int)(1000.0 * (_seconds_interval - Math.Floor(_seconds_interval)))
                   ),
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(_seconds_interval), (int)(1000.0 * (_seconds_interval - Math.Floor(_seconds_interval)))
                   )
            );
        }
    }
    public class cweeDequeue // : DependencyObject
    {
        private cweeTimer doTimer = null;
        private EdmsTasks.cweeAction work = null;
        private bool mainthread = false;
        private DateTime realtarget;
        private bool doneWork = true;
        private bool doingWork = false;

        public cweeDequeue() { }
        public bool Dequeue(DateTime realTarget, Action action, bool mainThread = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            if (doingWork) return false;

            if (doTimer == null) // finished or haven't started
            {
                work = new EdmsTasks.cweeAction(action, membername, filepath, linenumber);
                mainthread = mainThread;
                realtarget = realTarget;
                doneWork = false;
                doingWork = false;
                this.IsFinished = this.doneWork;

                if (DateTime.Now > realtarget)
                {
                    DoWork();
                    return true;
                }
                else
                {
                    TimeSpan ts = DateTime.Now - realtarget;
                    var interval = Math.Abs(ts.TotalSeconds);// / 10.0;
                    if (doTimer != null) doTimer.Stop();
                    doTimer = new cweeTimer(interval, () => { this.CheckDoWork(); }, false);
                }
            }
            else // waiting for next queue
            {
                work = new EdmsTasks.cweeAction(action, membername, filepath, linenumber);
                ChangeTarget(realTarget);
            }

            return true;
        }
        public cweeDequeue(DateTime realTarget, Action action, bool mainThread = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            Dequeue(realTarget, action, mainThread, membername, filepath, linenumber);
        }
        public DateTime GetTarget()
        {
            return realtarget;
        }
        public bool IsSameTarget(DateTime _Target)
        {
            return realtarget <= _Target;
        }
        public void ChangeTarget(DateTime _realTarget)
        {
            realtarget = _realTarget;

            TimeSpan ts = DateTime.Now - realtarget;
            var interval = Math.Abs(ts.TotalSeconds);// / 10.0;
            if (doTimer != null) doTimer.Stop();
            doTimer = new cweeTimer(interval, () => { this.CheckDoWork(); }, false);
        }
        private void CheckDoWork()
        {
            if (!doingWork && DateTime.Now > this.realtarget)
            {
                if (doTimer != null && doTimer.IsActive())
                {
                    doTimer.Stop(); // not null, just stopped
                    DoWork();
                }

            }
        }
        private void DoWork()
        {
            if (!this.doingWork)
            {
                this.doingWork = true;
                if (!this.doneWork)
                {
                    EdmsTasks.InsertJob(new EdmsTasks.cweeTask(work, mainthread, true)).ContinueWith(() =>
                    {
                        this.doneWork = true;
                        this.doingWork = false;
                        this.doTimer = null;
                        this.IsFinished = this.doneWork;
                    }, false); // true);
                }
                else
                {
                    this.doneWork = true;
                    this.doingWork = false;
                    this.doTimer = null;
                    this.IsFinished = this.doneWork;
                }
            }

        }

        public bool IsFinished = false;

        public void Cancel()
        {
            if (this.doTimer != null && this.doTimer.IsActive())
            {
                this.doTimer.Stop();
            }

            this.doneWork = true;
            this.doingWork = false;
            this.doTimer = null;
            this.IsFinished = this.doneWork;
        }

        public bool Dequeue(DateTime realTarget)
        {
            if (doingWork) return false;

            if (work == null)
            {
                // can't support this
                throw (new Exception("User must call dequeue at least once with a valid action before calling the basic time-based dequeue method."));
            }

            if (doTimer == null) // finished or haven't started
            {
                realtarget = realTarget;
                doneWork = false;
                doingWork = false;
                this.IsFinished = this.doneWork;


                if (DateTime.Now > realtarget)
                {
                    DoWork();
                    return true;
                }
                else
                {
                    TimeSpan ts = DateTime.Now - realtarget;
                    var interval = Math.Abs(ts.TotalSeconds);// / 10.0;
                    if (doTimer != null) doTimer.Stop();
                    doTimer = new cweeTimer(interval, () => { this.CheckDoWork(); }, false);
                }
            }
            else // waiting for next queue
            {
                ChangeTarget(realTarget);
            }
            return true;
        }
    }

    public class cweeAppendableTimer
    {
        private Mutex mut = new Mutex();
        private Dictionary<int, EdmsTasks.cweeAction> _actions = new Dictionary<int, EdmsTasks.cweeAction>();
        private TimerState _state;
        private double _seconds_interval = 0.017;

        public cweeAppendableTimer(double Interval_seconds, bool mainThread = true)
        {
            _seconds_interval = Interval_seconds;

            _state = new TimerState()
            {
                owner = this,
                _mainThread = mainThread,
                _seconds_interval = Math.Max(1.0 / 1000.0, _seconds_interval),
                _init_seconds_interval = Math.Max(1.0 / 1000.0, _seconds_interval)
            };
            _state._timer = new Timer(
                    new TimerCallback(TimerTask),
                   _state,
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(Interval_seconds), (int)(1000.0 * (Interval_seconds - Math.Floor(Interval_seconds)))
                   ),
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(Interval_seconds), (int)(1000.0 * (Interval_seconds - Math.Floor(Interval_seconds)))
                   )
            );

        }

        public void AddAction(Action todo, int key, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            mut.WaitOne();
            _actions.Remove(key);
            _actions.Add(key, new EdmsTasks.cweeAction(todo, membername, filepath, linenumber));
            mut.ReleaseMutex();
        }

        public void RemoveAction(int key)
        {
            mut.WaitOne();
            _actions.Remove(key);
            mut.ReleaseMutex();
        }

        public class TimerState
        {
            public bool _mainThread = true;
            public double _seconds_interval = 0.017;
            public double _init_seconds_interval = 0.017;
            public System.Threading.Timer _timer = null;
            public cweeAppendableTimer owner = null;
            public AtomicInt Working = new AtomicInt(0);
        }

        private static void TimerTask(object timerState)
        {
            TimerState t = (timerState as TimerState);
            if (t != null)
            {
                if (t.Working.Increment() == 1)
                {
                    t.owner.mut.WaitOne();
                    Dictionary<int, EdmsTasks.cweeAction> actions = new Dictionary<int, EdmsTasks.cweeAction>(t.owner._actions);
                    t.owner.mut.ReleaseMutex();

                    List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();
                    foreach (var x in actions)
                    {
                        tasks.Add(new EdmsTasks.cweeTask(x.Value, t._mainThread, true));
                    }
                    EdmsTasks.cweeTask.InsertListAsTask(tasks, false).ContinueWith(() => {
                        t.Working.Decrement();
                    }, false);
                }
                else
                {
                    t.Working.Decrement();
                }
            }
        }

        public void SetInterval(double Interval_seconds)
        {
            _seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds);
            _state._seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds);
            _state._timer.Change((int)(1000.0 * _seconds_interval), (int)(1000.0 * _seconds_interval));
        }

        public bool IsActive()
        {
            return (_state._timer != null);
        }

        public void Stop()
        {
            if (_state._timer != null)
            {
                _state._timer.Change(Timeout.Infinite, Timeout.Infinite);
                _state._timer.Dispose();
                _state._timer = null;
            }
        }
    }
    public class cweeMultiAppendableTimer
    {
        internal class CountedActions
        {
            public int count = 1;
            public EdmsTasks.cweeAction action = null;
        }

        private Mutex mut = new Mutex();
        private Dictionary<int, CountedActions> _actions = new Dictionary<int, CountedActions>();
        private TimerState _state;
        private double _seconds_interval = 0.017;

        public cweeMultiAppendableTimer(double Interval_seconds, bool mainThread = true)
        {
            _seconds_interval = Interval_seconds;

            _state = new TimerState()
            {
                owner = this,
                _mainThread = mainThread,
                _seconds_interval = Math.Max(1.0 / 1000.0, _seconds_interval),
                _init_seconds_interval = Math.Max(1.0 / 1000.0, _seconds_interval)
            };
            _state._timer = new Timer(
                    new TimerCallback(TimerTask),
                   _state,
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(Interval_seconds), (int)(1000.0 * (Interval_seconds - Math.Floor(Interval_seconds)))
                   ),
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor(Interval_seconds), (int)(1000.0 * (Interval_seconds - Math.Floor(Interval_seconds)))
                   )
            );

        }

        public void AddAction(Action todo, int key, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            mut.WaitOne();
            if (_actions.TryGetValue(key, out CountedActions v))
            {
                v.action = new EdmsTasks.cweeAction(todo, membername, filepath, linenumber);
                v.count++;
            }
            else
            {
                _actions.Add(key, new CountedActions() { count = 1, action = new EdmsTasks.cweeAction(todo, membername, filepath, linenumber) });
            }
            mut.ReleaseMutex();
        }

        public bool RemoveAction(int key)
        {
            bool toR = false;
            mut.WaitOne();
            if (_actions.TryGetValue(key, out CountedActions v))
            {
                v.count--;
                if (v.count <= 0)
                {
                    _actions.Remove(key);
                    toR = true;
                }
            }
            mut.ReleaseMutex();
            return toR;
        }

        public class TimerState
        {
            public bool _mainThread = true;
            public double _seconds_interval = 0.017;
            public double _init_seconds_interval = 0.017;
            public System.Threading.Timer _timer = null;
            public cweeMultiAppendableTimer owner = null;
            public AtomicInt Working = new AtomicInt(0);
        }

        private static void TimerTask(object timerState)
        {
            TimerState t = (timerState as TimerState);
            if (t != null)
            {
                if (t.Working.Increment() == 1)
                {
                    t.owner.mut.WaitOne();
                    Dictionary<int, CountedActions> actions = new Dictionary<int, CountedActions>(t.owner._actions);
                    t.owner.mut.ReleaseMutex();

                    List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();
                    foreach (var x in actions)
                    {
                        tasks.Add(new EdmsTasks.cweeTask(x.Value.action, t._mainThread, true));
                    }
                    EdmsTasks.cweeTask.InsertListAsTask(tasks, false).ContinueWith(() => {
                        t.Working.Decrement();
                    }, false);
                }
                else
                {
                    t.Working.Decrement();
                }
            }
        }

        public void SetInterval(double Interval_seconds)
        {
            _seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds);
            _state._seconds_interval = Math.Max(1.0 / 1000.0, Interval_seconds);
            _state._timer.Change((int)(1000.0 * _seconds_interval), (int)(1000.0 * _seconds_interval));
        }

        public bool IsActive()
        {
            return (_state._timer != null);
        }

        public void Stop()
        {
            if (_state._timer != null)
            {
                _state._timer.Change(Timeout.Infinite, Timeout.Infinite);
                _state._timer.Dispose();
                _state._timer = null;
            }
        }
    }








}