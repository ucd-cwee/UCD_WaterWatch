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
//#define EdmsTaskCounter
#define blockCustomers

using System;
using UWP_WaterWatchLibrary.ArrayExtensions;
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
using Windows.Foundation;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI;
using UWP_WaterWatchLibrary;
using Windows.Storage;
using System.Runtime.CompilerServices;
using Windows.UI.Core;
using Windows.ApplicationModel.DataTransfer;
using Windows.Storage.Streams;
using Windows.Graphics.Imaging;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Media;
using Windows.UI.Notifications;
using Microsoft.Toolkit.Uwp.Notifications;
using System.Windows.Input;
using System.Collections;
using Windows.Data.Json;
using System.IO;
using Windows.UI.Xaml.Data;


namespace UWP_WaterWatchLibrary
{
    public class ApplicationInfo : INotifyPropertyChanged
    {
        public ApplicationInfo()
        {
            Initialize();
            _RootFrame = Window.Current == null ? null : Window.Current.Content == null ? null : Window.Current.Content;
        }

        [XmlIgnore]
        [NonSerialized]
        public SynchronizationContext mainThreadSyncContext;

        [XmlIgnore]
        [NonSerialized]
        public Application Current;

        [XmlIgnore]
        [NonSerialized]
        private double _appStartTime = 0;
        public double AppStartTime
        {
            get
            {
                return _appStartTime;
            }
            set
            {
                if (_appStartTime == value) return;
                _appStartTime = value;
                OnPropertyChanged("AppStartTime");
            }
        }

        [XmlIgnore]
        [NonSerialized]
        private double _appStartDateTime = DateTime.Now.ToUnixTimeSeconds();
        public double AppStartDateTime
        {
            get
            {
                return _appStartDateTime;
            }
            set
            {
                if (_appStartDateTime == value) return;
                _appStartDateTime = value;

                _appStartTime = WaterWatch.GetNanosecondsSinceStart();

                SetApplicationParam("AppStartDateTime", value);
                OnPropertyChanged("AppStartDateTime");
            }
        }

        [XmlIgnore]
        [NonSerialized]
        public PointerRoutedEventArgs PointerMovedArgs;

        public DateTimeOffset AppDateTime
        {
            get
            {
                return WaterWatch.getCurrentTime().to_DateTime();
            }
            set
            {

            }
        }

        //[XmlIgnore]
        //[NonSerialized]
        //public static int mainThreadId = 0;

        [XmlIgnore]
        [NonSerialized]
        private bool _inForeground = true;
        public bool InForeground
        {
            get
            {
                return _inForeground;
            }
            set
            {
                if (_inForeground == value) return;
                _inForeground = value;
                OnPropertyChanged("InForeground");
            }
        }

        [XmlIgnore]
        [NonSerialized]
        private static Dictionary<int, bool> IsInMainDispatchThread = new Dictionary<int, bool>();

        [XmlIgnore]
        [NonSerialized]
        public UIElement _RootFrame = null;

        public static bool IsMainDispatchThread()
        {
            lock (IsInMainDispatchThread)
            {
                if (IsInMainDispatchThread.TryGetValue(System.Threading.Thread.CurrentThread.ManagedThreadId, out bool isInMainThread) && isInMainThread)
                {
                    return true;
                }
                return false;
            }
        }
        public EdmsTasks.cweeTask MainWindowAction(Func<System.Threading.Tasks.Task> a)
        {
            if (_RootFrame != null)
            {
                EdmsTasks.cweeTask toReturn = new EdmsTasks.cweeTask();
                System.Threading.Tasks.Task.Run(async () =>
                {
                    int thisThreadID = System.Threading.Thread.CurrentThread.ManagedThreadId;
                    bool DoImmediate = false;

                    lock (IsInMainDispatchThread)
                    {
                        if (IsInMainDispatchThread.TryGetValue(thisThreadID, out bool isInMainThread) && isInMainThread)
                        {
                            DoImmediate = true;
                        }
                    }

                    if (DoImmediate)
                    {
                        await a();
                        return;
                    }
                    else
                    {
                        lock (IsInMainDispatchThread)
                        {
                            IsInMainDispatchThread[thisThreadID] = false;
                        }
                    }

                    //while (!EdmsTasks.mainThreadPostLock.TryIncrementTo(1)) { }
                    await _RootFrame.Dispatcher.RunAsync(
                        CoreDispatcherPriority.Normal,
                        async () =>
                        {
                            lock (IsInMainDispatchThread)
                            {
                                IsInMainDispatchThread[thisThreadID] = true;
                            }
                            await a();
                            toReturn.SetFinished(null);
                            lock (IsInMainDispatchThread)
                            {
                                IsInMainDispatchThread[thisThreadID] = false;
                            }
                        }
                    );
                    //EdmsTasks.mainThreadPostLock.Decrement();
                });
                return toReturn;
            }
            else
            {
                return EdmsTasks.InsertJob(() => { a(); }, true, true);
            }
        }
        public EdmsTasks.cweeTask MainWindowAction(Action a)
        {
            if (_RootFrame != null)
            {
                EdmsTasks.cweeTask toReturn = new EdmsTasks.cweeTask();
                System.Threading.Tasks.Task.Run(async () =>
                {
                    int thisThreadID = System.Threading.Thread.CurrentThread.ManagedThreadId;
                    bool DoImmediate = false;

                    lock (IsInMainDispatchThread)
                    {
                        if (IsInMainDispatchThread.TryGetValue(thisThreadID, out bool isInMainThread) && isInMainThread)
                        {
                            DoImmediate = true;
                        }
                    }

                    if (DoImmediate)
                    {
                        //while (!EdmsTasks.mainThreadPostLock.TryIncrementTo(1)) { }
                        a();
                        toReturn.SetFinished(null);
                        //EdmsTasks.mainThreadPostLock.Decrement();
                        return;
                    }
                    else
                    {
                        lock (IsInMainDispatchThread)
                        {
                            IsInMainDispatchThread[thisThreadID] = false;
                        }
                    }

                    while (!EdmsTasks.mainThreadPostLock.TryIncrementTo(1)) { }
                    await _RootFrame.Dispatcher.RunAsync(
                        CoreDispatcherPriority.Normal,
                        () =>
                        {
                            lock (IsInMainDispatchThread)
                            {
                                IsInMainDispatchThread[thisThreadID] = true;
                            }
                            a();
                            toReturn.SetFinished(null);
                            lock (IsInMainDispatchThread)
                            {
                                IsInMainDispatchThread[thisThreadID] = false;
                            }
                        }
                    );
                    EdmsTasks.mainThreadPostLock.Decrement();
                });
                return toReturn;
            }
            else
            {
                return EdmsTasks.InsertJob(() => { a(); }, true, true);
            }
        }

        public void Initialize()
        {
            // mainThreadId = System.Threading.Thread.CurrentThread.ManagedThreadId;
            EdmsTasks.mainThreadID.Set(System.Threading.Thread.CurrentThread.ManagedThreadId);
            mainThreadSyncContext = SynchronizationContext.Current;
        }

        public void Initialize(int mainThreadID_p, SynchronizationContext mainThreadContent_p, Application current_p)
        {
            // mainThreadId = mainThreadID_p;
            EdmsTasks.mainThreadID.Set(mainThreadID_p);
            mainThreadSyncContext = mainThreadContent_p;
            Current = current_p;

            LoadParamsFromInit();
        }

        public void Shutdown()
        {
            SaveParamsToInit();
        }

        private void SaveParamsToInit()
        {
            ApplicationDataContainer localSettings = ApplicationData.Current.LocalSettings;
            var se = new ScriptEngine();
            string fp = se.DoScript("return createRandomFilePath(\"TXT\");");
            var overallJSON = "";
            foreach (var x in localSettings.Values)
            {
                try
                {
                    //string jsonContent = "";
                    //if (x.Value is int)
                    //{
                    //    jsonContent = ((int)x.Value).ToString();
                    //}
                    //else if (x.Value is float)
                    //{
                    //    jsonContent = ((float)x.Value).ToString();
                    //}
                    //else if (x.Value is double)
                    //{
                    //    jsonContent = ((double)x.Value).ToString();
                    //}
                    //else if (x.Value is string)
                    //{
                    //    jsonContent = "\"" + (x.Value as string).ToString() + "\"";
                    //}
                    //if (jsonContent != "")
                    //{
                    //    jsonContent = "{\n" + $"\"{x.Key}\":" + $"{jsonContent.ToString()}" + "\n}";
                    //}

                    string jsonContent = "";
                    if (x.Value is double)
                    {
                        jsonContent = ((double)x.Value).ToString();
                    }
                    else
                    {
                        //using (FileStream fs = new FileStream(fp, FileMode.Create))
                        //{
                        //    var bf = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();
                        //    bf.Serialize(fs, x.Value);
                        //    jsonContent = "\"" + se.DoScript($"return readFileAsCweeStr(\"{ fp.EscapeCharactersAsLiterals() }\");") + "\"";
                        //}
                    }

                    if (jsonContent != "")
                    {
                        jsonContent = "{\n" + $"\"{x.Key}\":" + $"{jsonContent.ToString()}" + "\n}";
                        overallJSON = overallJSON.AddToDelimiter(jsonContent, ", ");
                    }
                }
                catch { }
            }
            overallJSON = "[" + overallJSON + "]";
            se.DoScript($"writeFileFromCweeStr(createFilePath(getDataFolder(), \"init\", \"TXT\"), \"{overallJSON.EscapeCharactersAsLiterals()}\");");
        }

        private void LoadParamsFromInit()
        {
            var se = new ScriptEngine();
            string fp = se.DoScript("if (checkFileExists(createFilePath(getDataFolder(), \"init\", \"TXT\"))){ return readFileAsCweeStr(createFilePath(getDataFolder(), \"init\", \"TXT\")); }else{  return \"\"; }");
            if (fp != ""){
                ApplicationDataContainer localSettings = ApplicationData.Current.LocalSettings;
                string fpTemp = se.DoScript("return createRandomFilePath(\"TXT\");");
                var jsonArr = JsonArray.Parse(fp);
                foreach (var jsonO in jsonArr)
                {
                    try
                    {
                        var jsonObj = jsonO.GetObject();
                        foreach (var key in jsonObj.Keys)
                        {
                            try
                            {
                                double val = jsonObj.GetNamedNumber(key);
                                localSettings.Values[key] = val;
                                continue;
                            }
                            catch { }
#if false
                            try
                            {
                                string gotStr = jsonObj.GetNamedString(key);
                                se.DoScript($"writeFileFromCweeStr(\"{fpTemp.EscapeCharactersAsLiterals()}\", \"{gotStr.EscapeCharactersAsLiterals()}\");");

                                using (FileStream fs = new FileStream(fpTemp, FileMode.Open))
                                {
                                    var bf = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();
                                    try
                                    {
                                        var obj = bf.Deserialize(fs);
                                        localSettings.Values[key] = obj;
                                        continue;
                                    }
                                    catch { }
                                }
                            }
                            catch { }
#endif
                        }
                    }
                    catch { }
                }
            }
        }


        public bool IsMainThread() => EdmsTasks.mainThreadID.Get() == System.Threading.Thread.CurrentThread.ManagedThreadId;


        public void SetApplicationParam(string name, object value)
        {
            ApplicationDataContainer localSettings = ApplicationData.Current.LocalSettings;
            localSettings.Values[name] = value;
        }
        public object GetApplicationParam(string name)
        {
            ApplicationDataContainer localSettings = ApplicationData.Current.LocalSettings;
            try
            {
                if (!localSettings.Values.ContainsKey(name))
                {
                    return null;
                }
                else
                {
                    object x;
                    if (localSettings.Values.TryGetValue(name, out x))
                    {
                        return (object)x;
                    }
                    else
                    {
                        return null;
                    }
                }
            }
            catch (Exception)
            {
                return null;
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            EdmsTasks.InsertJob(() =>
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }, true, true);
        }
    }

    public class AppExtension
    {
        [XmlIgnore]
        [NonSerialized]
        static public ApplicationInfo ApplicationInfo = new ApplicationInfo(); // main thread, hopefully

        public static void ManageException(Exception exc, string additionalMessage = null, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
            string report = $"{ExceptionToString(exc)}\n";
            
            if (!string.IsNullOrEmpty(additionalMessage))
            {
                report = additionalMessage + ". " + report;
            }

            //Log?.Info(report);
            WaterWatch.AddToLog(WaterWatch.GetDataDirectory() + @"\unhandledExceptions.csv", $"Exception @ {filepath}:{linenumber}" + ": " + report);
            WaterWatch.AddToLog(WaterWatch.GetDataDirectory() + @"\recentlyUnhandledExceptions.csv", $"Exception @ {filepath}:{linenumber}" + ": " + report);

            WaterWatch.SubmitToast($"Exception @ {filepath.Split("\\").Last()}:{linenumber}", report);
        }
        public static string ExceptionToString(Exception exc)
        {
            string report = "";

            if (exc.Message != null)
            {
                report = report.AddToDelimiter($"\t\"{exc.Message.Replace("\r", "\n").Replace("\n", "\n\t")}\"", "\n");
            }

            if (exc.StackTrace != null)
            {
                report = report.AddToDelimiter($"\t\"{exc.StackTrace.Replace("\r", "\n").Replace("\n", "\n\t")}\"", "\n");
            }

            if (exc.InnerException != null)
            {
                report = report.AddToDelimiter($"\t\"{ExceptionToString(exc.InnerException)}\"", "\n");
            }

            return report;
        }
    }

    public class Functions
    {
        public static void CopyToClipboard(string data)
        {
            var dataPackage = new DataPackage();
            dataPackage.RequestedOperation = DataPackageOperation.Copy;
            dataPackage.SetText(data);
            EdmsTasks.InsertJob(() =>
            {
                try
                {
                    Clipboard.SetContent(dataPackage);
                }
                catch (Exception) { }
            }, true, true);
        }
        public static void CopyToClipboard(StorageFile file)
        {
            DataPackage dp = new DataPackage();
            dp.SetStorageItems(new List<IStorageItem>() { file });
            EdmsTasks.InsertJob(() => {
                Clipboard.SetContent(dp);
            }, true);
        }
        public static void CopyToClipboard(Windows.UI.Xaml.Controls.Image r)
        {
            var filePath = cweeXamlHelper.UIElementToFile((UIElement)r, (int)r.ActualWidth, (int)r.ActualHeight);
            filePath.ContinueWith(()=> {
                string filePathFinal = WaterWatch.GetTemporaryFilePath(".png");
                StorageFile.GetFileFromPathAsync(filePathFinal).AsTask().ContinueWith((Task<StorageFile> file2) => {
                    StorageFile.GetFileFromPathAsync(filePath.Result).AsTask().ContinueWith((Task<StorageFile> file) => {
                        file.Result.CopyAndReplaceAsync(file2.Result).AsTask().ContinueWith((Task t) => {
                            DataPackage dp = new DataPackage();
                            dp.SetStorageItems(new List<IStorageItem>() { file2.Result });
                            EdmsTasks.InsertJob(() => {
                                Clipboard.SetContent(dp);
                            }, true);
                            File.Delete(filePath.Result);
                        });
                    });
                });
            }, true);
        }
        public static cweeTask<string> GetTextFromClipboard()
        {
            string toReturn = "";
            EdmsTasks.cweeTask toReturnTask = new EdmsTasks.cweeTask(() => { return toReturn; }, false, true);

            try
            {
                var content = Clipboard.GetContent();
                var contentStr = content.GetTextAsync();
                var task = contentStr.AsTask().ContinueWith((Task tj) =>
                {
                    toReturn = contentStr.GetResults();
                    toReturn = toReturn.Replace("\r", "\n").Replace("\n\n", "\n");
                    toReturnTask.QueueJob();
                });
            }
            catch (Exception)
            {
                toReturnTask.QueueJob();
            }

            return toReturnTask;
        }
        public static bool ListIsEqual<T>(List<T> lhs, List<T> rhs) where T : class
        {
            if (lhs == null && rhs == null) return true;
            if (lhs == null && rhs != null || lhs != null && rhs == null) return false;
            if (lhs.Count() != rhs.Count()) return false;

            for (int i = lhs.Count() - 1; i >= 0; i--)
            {
                T one = lhs[i];
                T two = rhs[i];

                if (one is string)
                {
                    if (!(one as string).Equals(two as string))
                        return false;
                }
                else
                {
                    if (one != two)
                        return false;
                }
            }

            return true;
        }

        public static bool ListIsEqual<T>(SynchronizedCollection<T> lhs, List<T> rhs) where T : class
        {
            if (lhs == null && rhs == null) return true;
            if (lhs == null && rhs != null || lhs != null && rhs == null) return false;
            if (lhs.Count() != rhs.Count()) return false;

            for (int i = lhs.Count() - 1; i >= 0; i--)
            {
                T one = lhs[i];
                T two = rhs[i];

                if (one is string)
                {
                    if (!(one as string).Equals(two as string))
                        return false;
                }
                else
                {
                    if (one != two)
                        return false;
                }
            }

            return true;
        }

        public static bool ListIsEqual<T>(SynchronizedCollection<T> lhs, SynchronizedCollection<T> rhs) where T : class
        {
            if (lhs == null && rhs == null) return true;
            if (lhs == null && rhs != null || lhs != null && rhs == null) return false;
            if (lhs.Count() != rhs.Count()) return false;

            for (int i = lhs.Count() - 1; i >= 0; i--)
            {
                T one = lhs[i];
                T two = rhs[i];

                if (one is string)
                {
                    if (!(one as string).Equals(two as string))
                        return false;
                }
                else
                {
                    if (one != two)
                        return false;
                }
            }

            return true;
        }

        public static bool ListIsEqual<T>(List<T> lhs, SynchronizedCollection<T> rhs) where T : class
        {
            if (lhs == null && rhs == null) return true;
            if (lhs == null && rhs != null || lhs != null && rhs == null) return false;
            if (lhs.Count() != rhs.Count()) return false;

            for (int i = lhs.Count() - 1; i >= 0; i--)
            {
                T one = lhs[i];
                T two = rhs[i];

                if (one is string)
                {
                    if (!(one as string).Equals(two as string))
                        return false;
                }
                else
                {
                    if (one != two)
                        return false;
                }
            }

            return true;
        }
    }

    public static class ObjectExtensions
    {
        public static string ToChar(this Windows.System.VirtualKey k)
        {
            string mappedChar = null;
            switch (k) {
                //
                // Summary:
                //     No virtual key value.
                case Windows.System.VirtualKey.None: break;
                //
                // Summary:
                //     The left mouse button.
                case Windows.System.VirtualKey.LeftButton: break;
                //
                // Summary:
                //     The right mouse button.
                case Windows.System.VirtualKey.RightButton: break;
                //
                // Summary:
                //     The cancel key or button
                case Windows.System.VirtualKey.Cancel: break;
                //
                // Summary:
                //     The middle mouse button.
                case Windows.System.VirtualKey.MiddleButton: break;
                //
                // Summary:
                //     An additional "extended" device key or button (for example, an additional mouse
                //     button).
                case Windows.System.VirtualKey.XButton1: break;
                //
                // Summary:
                //     An additional "extended" device key or button (for example, an additional mouse
                //     button).
                case Windows.System.VirtualKey.XButton2: break;
                //
                // Summary:
                //     The virtual back key or button.
                case Windows.System.VirtualKey.Back: break;
                //
                // Summary:
                //     The Tab key.
                case Windows.System.VirtualKey.Tab: break;
                //
                // Summary:
                //     The Clear key or button.
                case Windows.System.VirtualKey.Clear: break;
                //
                // Summary:
                //     The Enter key.
                case Windows.System.VirtualKey.Enter: break;
                //
                // Summary:
                //     The Shift key. This is the general Shift case, applicable to key layouts with
                //     only one Shift key or that do not need to differentiate between left Shift and
                //     right Shift keystrokes.
                case Windows.System.VirtualKey.Shift: break;
                //
                // Summary:
                //     The Ctrl key. This is the general Ctrl case, applicable to key layouts with only
                //     one Ctrl key or that do not need to differentiate between left Ctrl and right
                //     Ctrl keystrokes.
                case Windows.System.VirtualKey.Control: break;
                //
                // Summary:
                //     The menu key or button.
                case Windows.System.VirtualKey.Menu: break;
                //
                // Summary:
                //     The Pause key or button.
                case Windows.System.VirtualKey.Pause: break;
                //
                // Summary:
                //     The Caps Lock key or button.
                case Windows.System.VirtualKey.CapitalLock: break;
                //
                // Summary:
                //     The Kana symbol key-shift button
                case Windows.System.VirtualKey.Kana: break;
                //
                // Summary:
                //     The Hangul symbol key-shift button.
                // case Windows.System.VirtualKey.Hangul: break;
                case Windows.System.VirtualKey.ImeOn: break;
                //
                // Summary:
                //     The Junja symbol key-shift button.
                case Windows.System.VirtualKey.Junja: break;
                //
                // Summary:
                //     The Final symbol key-shift button.
                case Windows.System.VirtualKey.Final: break;
                //
                // Summary:
                //     The Hanja symbol key shift button.
                case Windows.System.VirtualKey.Hanja: break;
                //
                // Summary:
                //     The Kanji symbol key-shift button.
                // case Windows.System.VirtualKey.Kanji: break;
                case Windows.System.VirtualKey.ImeOff: break;
                //
                // Summary:
                //     The Esc key.
                case Windows.System.VirtualKey.Escape: break;
                //
                // Summary:
                //     The convert button or key.
                case Windows.System.VirtualKey.Convert: break;
                //
                // Summary:
                //     The nonconvert button or key.
                case Windows.System.VirtualKey.NonConvert: break;
                //
                // Summary:
                //     The accept button or key.
                case Windows.System.VirtualKey.Accept: break;
                //
                // Summary:
                //     The mode change key.
                case Windows.System.VirtualKey.ModeChange: break;
                //
                // Summary:
                //     The Spacebar key or button.
                case Windows.System.VirtualKey.Space: break;
                //
                // Summary:
                //     The Page Up key.
                case Windows.System.VirtualKey.PageUp: break;
                //
                // Summary:
                //     The Page Down key.
                case Windows.System.VirtualKey.PageDown: break;
                //
                // Summary:
                //     The End key.
                case Windows.System.VirtualKey.End: break;
                //
                // Summary:
                //     The Home key.
                case Windows.System.VirtualKey.Home: break;
                //
                // Summary:
                //     The Left Arrow key.
                case Windows.System.VirtualKey.Left: break;
                //
                // Summary:
                //     The Up Arrow key.
                case Windows.System.VirtualKey.Up: break;
                //
                // Summary:
                //     The Right Arrow key.
                case Windows.System.VirtualKey.Right: break;
                //
                // Summary:
                //     The Down Arrow key.
                case Windows.System.VirtualKey.Down: break;
                //
                // Summary:
                //     The Select key or button.
                case Windows.System.VirtualKey.Select: break;
                //
                // Summary:
                //     The Print key or button.
                case Windows.System.VirtualKey.Print: break;
                //
                // Summary:
                //     The execute key or button.
                case Windows.System.VirtualKey.Execute: break;
                //
                // Summary:
                //     The snapshot key or button.
                case Windows.System.VirtualKey.Snapshot: break;
                //
                // Summary:
                //     The Insert key.
                case Windows.System.VirtualKey.Insert: break;
                //
                // Summary:
                //     The Delete key.
                case Windows.System.VirtualKey.Delete: break;
                //
                // Summary:
                //     The Help key or button.
                case Windows.System.VirtualKey.Help: break;
                //
                // Summary:
                //     The number "0" key.
                case Windows.System.VirtualKey.Number0: break;
                //
                // Summary:
                //     The number "1" key.
                case Windows.System.VirtualKey.Number1: break;
                //
                // Summary:
                //     The number "2" key.
                case Windows.System.VirtualKey.Number2: break;
                //
                // Summary:
                //     The number "3" key.
                case Windows.System.VirtualKey.Number3: break;
                //
                // Summary:
                //     The number "4" key.
                case Windows.System.VirtualKey.Number4: break;
                //
                // Summary:
                //     The number "5" key.
                case Windows.System.VirtualKey.Number5: break;
                //
                // Summary:
                //     The number "6" key.
                case Windows.System.VirtualKey.Number6: break;
                //
                // Summary:
                //     The number "7" key.
                case Windows.System.VirtualKey.Number7: break;
                //
                // Summary:
                //     The number "8" key.
                case Windows.System.VirtualKey.Number8: break;
                //
                // Summary:
                //     The number "9" key.
                case Windows.System.VirtualKey.Number9: break;
                //
                // Summary:
                //     The letter "A" key.
                case Windows.System.VirtualKey.A: break;
                //
                // Summary:
                //     The letter "B" key.
                case Windows.System.VirtualKey.B: break;
                //
                // Summary:
                //     The letter "C" key.
                case Windows.System.VirtualKey.C: break;
                //
                // Summary:
                //     The letter "D" key.
                case Windows.System.VirtualKey.D: break;
                //
                // Summary:
                //     The letter "E" key.
                case Windows.System.VirtualKey.E: break;
                //
                // Summary:
                //     The letter "F" key.
                case Windows.System.VirtualKey.F: break;
                //
                // Summary:
                //     The letter "G" key.
                case Windows.System.VirtualKey.G: break;
                //
                // Summary:
                //     The letter "H" key.
                case Windows.System.VirtualKey.H: break;
                //
                // Summary:
                //     The letter "I" key.
                case Windows.System.VirtualKey.I: break;
                //
                // Summary:
                //     The letter "J" key.
                case Windows.System.VirtualKey.J: break;
                //
                // Summary:
                //     The letter "K" key.
                case Windows.System.VirtualKey.K: break;
                //
                // Summary:
                //     The letter "L" key.
                case Windows.System.VirtualKey.L: break;
                //
                // Summary:
                //     The letter "M" key.
                case Windows.System.VirtualKey.M: break;
                //
                // Summary:
                //     The letter "N" key.
                case Windows.System.VirtualKey.N: break;
                //
                // Summary:
                //     The letter "O" key.
                case Windows.System.VirtualKey.O: break;
                //
                // Summary:
                //     The letter "P" key.
                case Windows.System.VirtualKey.P: break;
                //
                // Summary:
                //     The letter "Q" key.
                case Windows.System.VirtualKey.Q: break;
                //
                // Summary:
                //     The letter "R" key.
                case Windows.System.VirtualKey.R: break;
                //
                // Summary:
                //     The letter "S" key.
                case Windows.System.VirtualKey.S: break;
                //
                // Summary:
                //     The letter "T" key.
                case Windows.System.VirtualKey.T: break;
                //
                // Summary:
                //     The letter "U" key.
                case Windows.System.VirtualKey.U: break;
                //
                // Summary:
                //     The letter "V" key.
                case Windows.System.VirtualKey.V: break;
                //
                // Summary:
                //     The letter "W" key.
                case Windows.System.VirtualKey.W: break;
                //
                // Summary:
                //     The letter "X" key.
                case Windows.System.VirtualKey.X: break;
                //
                // Summary:
                //     The letter "Y" key.
                case Windows.System.VirtualKey.Y: break;
                //
                // Summary:
                //     The letter "Z" key.
                case Windows.System.VirtualKey.Z: break;
                //
                // Summary:
                //     The left Windows key.
                case Windows.System.VirtualKey.LeftWindows: break;
                //
                // Summary:
                //     The right Windows key.
                case Windows.System.VirtualKey.RightWindows: break;
                //
                // Summary:
                //     The application key or button.
                case Windows.System.VirtualKey.Application: break;
                //
                // Summary:
                //     The sleep key or button.
                case Windows.System.VirtualKey.Sleep: break;
                //
                // Summary:
                //     The number "0" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad0: break;
                //
                // Summary:
                //     The number "1" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad1: break;
                //
                // Summary:
                //     The number "2" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad2: break;
                //
                // Summary:
                //     The number "3" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad3: break;
                //
                // Summary:
                //     The number "4" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad4: break;
                //
                // Summary:
                //     The number "5" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad5: break;
                //
                // Summary:
                //     The number "6" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad6: break;
                //
                // Summary:
                //     The number "7" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad7: break;
                //
                // Summary:
                //     The number "8" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad8: break;
                //
                // Summary:
                //     The number "9" key as located on a numeric pad.
                case Windows.System.VirtualKey.NumberPad9: break;
                //
                // Summary:
                //     The multiply (*) operation key as located on a numeric pad.
                case Windows.System.VirtualKey.Multiply: break;
                //
                // Summary:
                //     The add (+) operation key as located on a numeric pad.
                case Windows.System.VirtualKey.Add: break;
                //
                // Summary:
                //     The separator key as located on a numeric pad.
                case Windows.System.VirtualKey.Separator: break;
                //
                // Summary:
                //     The subtract (-) operation key as located on a numeric pad.
                case Windows.System.VirtualKey.Subtract: break;
                //
                // Summary:
                //     The decimal (.) key as located on a numeric pad.
                case Windows.System.VirtualKey.Decimal: break;
                //
                // Summary:
                //     The divide (/) operation key as located on a numeric pad.
                case Windows.System.VirtualKey.Divide: break;
                //
                // Summary:
                //     The F1 function key.
                case Windows.System.VirtualKey.F1: break;
                //
                // Summary:
                //     The F2 function key.
                case Windows.System.VirtualKey.F2: break;
                //
                // Summary:
                //     The F3 function key.
                case Windows.System.VirtualKey.F3: break;
                //
                // Summary:
                //     The F4 function key.
                case Windows.System.VirtualKey.F4: break;
                //
                // Summary:
                //     The F5 function key.
                case Windows.System.VirtualKey.F5: break;
                //
                // Summary:
                //     The F6 function key.
                case Windows.System.VirtualKey.F6: break;
                //
                // Summary:
                //     The F7 function key.
                case Windows.System.VirtualKey.F7: break;
                //
                // Summary:
                //     The F8 function key.
                case Windows.System.VirtualKey.F8: break;
                //
                // Summary:
                //     The F9 function key.
                case Windows.System.VirtualKey.F9: break;
                //
                // Summary:
                //     The F10 function key.
                case Windows.System.VirtualKey.F10: break;
                //
                // Summary:
                //     The F11 function key.
                case Windows.System.VirtualKey.F11: break;
                //
                // Summary:
                //     The F12 function key.
                case Windows.System.VirtualKey.F12: break;
                //
                // Summary:
                //     The F13 function key.
                case Windows.System.VirtualKey.F13: break;
                //
                // Summary:
                //     The F14 function key.
                case Windows.System.VirtualKey.F14: break;
                //
                // Summary:
                //     The F15 function key.
                case Windows.System.VirtualKey.F15: break;
                //
                // Summary:
                //     The F16 function key.
                case Windows.System.VirtualKey.F16: break;
                //
                // Summary:
                //     The F17 function key.
                case Windows.System.VirtualKey.F17: break;
                //
                // Summary:
                //     The F18 function key.
                case Windows.System.VirtualKey.F18: break;
                //
                // Summary:
                //     The F19 function key.
                case Windows.System.VirtualKey.F19: break;
                //
                // Summary:
                //     The F20 function key.
                case Windows.System.VirtualKey.F20: break;
                //
                // Summary:
                //     The F21 function key.
                case Windows.System.VirtualKey.F21: break;
                //
                // Summary:
                //     The F22 function key.
                case Windows.System.VirtualKey.F22: break;
                //
                // Summary:
                //     The F23 function key.
                case Windows.System.VirtualKey.F23: break;
                //
                // Summary:
                //     The F24 function key.
                case Windows.System.VirtualKey.F24: break;
                //
                // Summary:
                //     The navigation up button.
                case Windows.System.VirtualKey.NavigationView: break;
                //
                // Summary:
                //     The navigation menu button.
                case Windows.System.VirtualKey.NavigationMenu: break;
                //
                // Summary:
                //     The navigation up button.
                case Windows.System.VirtualKey.NavigationUp: break;
                //
                // Summary:
                //     The navigation down button.
                case Windows.System.VirtualKey.NavigationDown: break;
                //
                // Summary:
                //     The navigation left button.
                case Windows.System.VirtualKey.NavigationLeft: break;
                //
                // Summary:
                //     The navigation right button.
                case Windows.System.VirtualKey.NavigationRight: break;
                //
                // Summary:
                //     The navigation accept button.
                case Windows.System.VirtualKey.NavigationAccept: break;
                //
                // Summary:
                //     The navigation cancel button.
                case Windows.System.VirtualKey.NavigationCancel: break;
                //
                // Summary:
                //     The Num Lock key.
                case Windows.System.VirtualKey.NumberKeyLock: break;
                //
                // Summary:
                //     The Scroll Lock (ScrLk) key.
                case Windows.System.VirtualKey.Scroll: break;
                //
                // Summary:
                //     The left Shift key.
                case Windows.System.VirtualKey.LeftShift: break;
                //
                // Summary:
                //     The right Shift key.
                case Windows.System.VirtualKey.RightShift: break;
                //
                // Summary:
                //     The left Ctrl key.
                case Windows.System.VirtualKey.LeftControl: break;
                //
                // Summary:
                //     The right Ctrl key.
                case Windows.System.VirtualKey.RightControl: break;
                //
                // Summary:
                //     The left menu key.
                case Windows.System.VirtualKey.LeftMenu: break;
                //
                // Summary:
                //     The right menu key.
                case Windows.System.VirtualKey.RightMenu: break;
                //
                // Summary:
                //     The go back key.
                case Windows.System.VirtualKey.GoBack: break;
                //
                // Summary:
                //     The go forward key.
                case Windows.System.VirtualKey.GoForward: break;
                //
                // Summary:
                //     The refresh key.
                case Windows.System.VirtualKey.Refresh: break;
                //
                // Summary:
                //     The stop key.
                case Windows.System.VirtualKey.Stop: break;
                //
                // Summary:
                //     The search key.
                case Windows.System.VirtualKey.Search: break;
                //
                // Summary:
                //     The favorites key.
                case Windows.System.VirtualKey.Favorites: break;
                //
                // Summary:
                //     The go home key.
                case Windows.System.VirtualKey.GoHome: break;
                //
                // Summary:
                //     The gamepad A button.
                case Windows.System.VirtualKey.GamepadA: break;
                //
                // Summary:
                //     The gamepad B button.
                case Windows.System.VirtualKey.GamepadB: break;
                //
                // Summary:
                //     The gamepad X button.
                case Windows.System.VirtualKey.GamepadX: break;
                //
                // Summary:
                //     The gamepad Y button.
                case Windows.System.VirtualKey.GamepadY: break;
                //
                // Summary:
                //     The gamepad right shoulder.
                case Windows.System.VirtualKey.GamepadRightShoulder: break;
                //
                // Summary:
                //     The gamepad left shoulder.
                case Windows.System.VirtualKey.GamepadLeftShoulder: break;
                //
                // Summary:
                //     The gamepad left trigger.
                case Windows.System.VirtualKey.GamepadLeftTrigger: break;
                //
                // Summary:
                //     The gamepad right trigger.
                case Windows.System.VirtualKey.GamepadRightTrigger: break;
                //
                // Summary:
                //     The gamepad d-pad up.
                case Windows.System.VirtualKey.GamepadDPadUp: break;
                //
                // Summary:
                //     The gamepad d-pad down.
                case Windows.System.VirtualKey.GamepadDPadDown: break;
                //
                // Summary:
                //     The gamepad d-pad left.
                case Windows.System.VirtualKey.GamepadDPadLeft: break;
                //
                // Summary:
                //     The gamepad d-pad right.
                case Windows.System.VirtualKey.GamepadDPadRight: break;
                //
                // Summary:
                //     The gamepad menu button.
                case Windows.System.VirtualKey.GamepadMenu: break;
                //
                // Summary:
                //     The gamepad view button.
                case Windows.System.VirtualKey.GamepadView: break;
                //
                // Summary:
                //     The gamepad left thumbstick button.
                case Windows.System.VirtualKey.GamepadLeftThumbstickButton: break;
                //
                // Summary:
                //     The gamepad right thumbstick button.
                case Windows.System.VirtualKey.GamepadRightThumbstickButton: break;
                //
                // Summary:
                //     The gamepad left thumbstick up.
                case Windows.System.VirtualKey.GamepadLeftThumbstickUp: break;
                //
                // Summary:
                //     The gamepad left thumbstick down.
                case Windows.System.VirtualKey.GamepadLeftThumbstickDown: break;
                //
                // Summary:
                //     The gamepad left thumbstick right.
                case Windows.System.VirtualKey.GamepadLeftThumbstickRight: break;
                //
                // Summary:
                //     The gamepad left thumbstick left.
                case Windows.System.VirtualKey.GamepadLeftThumbstickLeft: break;
                //
                // Summary:
                //     The gamepad right thumbstick up.
                case Windows.System.VirtualKey.GamepadRightThumbstickUp: break;
                //
                // Summary:
                //     The gamepad right thumbstick down.
                case Windows.System.VirtualKey.GamepadRightThumbstickDown: break;
                //
                // Summary:
                //     The gamepad right thumbstick right.
                case Windows.System.VirtualKey.GamepadRightThumbstickRight: break;
                //
                // Summary:
                //     The gamepad right thumbstick left.
                case Windows.System.VirtualKey.GamepadRightThumbstickLeft: break;
            }

            return mappedChar;
        }
    
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

        public static childItem FindVisualChild<childItem>(this DependencyObject obj) where childItem : DependencyObject
        {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(obj); i++)
            {
                DependencyObject child = VisualTreeHelper.GetChild(obj, i);
                if (child != null && child is childItem)
                    return (childItem)child;
                else
                {
                    childItem childOfChild = FindVisualChild<childItem>(child);
                    if (childOfChild != null)
                        return childOfChild;
                }
            }
            return null;
        }
        public static int GetCharNumUnderCursor(this RichEditBox control, PointerRoutedEventArgs e)
        {
            try
            {
                var point = e.GetCurrentPoint(control);
                var position = point.Position;
                var range = control.Document.GetRangeFromPoint(position, Windows.UI.Text.PointOptions.ClientCoordinates);
                return range.StartPosition;
            }
            catch (Exception) { }
            return -1;
        }

        public static string GetWordUnderCursor(this RichEditBox control, PointerRoutedEventArgs e, List<string> TreatAsSpaces = null)
        {
            //var origStart = control.Document.Selection.StartPosition;
            //var origEnd = control.Document.Selection.EndPosition;

            // control.PointerMoved += Control_PointerMoved;
            control.Document.GetText(Windows.UI.Text.TextGetOptions.NoHidden, out string text);

            //check if there's any text entered
            if (string.IsNullOrWhiteSpace(text) || text.Length <= 0) return null;

            var point = e.GetCurrentPoint(control);
            var position = point.Position;

            var range = control.Document.GetRangeFromPoint(position, Windows.UI.Text.PointOptions.ClientCoordinates);

            string toReturn = null;

            if (range.StartPosition < text.Length)
            {
                if (range.EndPosition < text.Length && range.EndPosition >= range.StartPosition)
                {
                    // find the character we're over
                    int pos = range.StartPosition;

                    if (TreatAsSpaces != null)
                    {
                        foreach (var op in TreatAsSpaces)
                        {
                            text = text.Replace(op, " ");
                        }
                    }

                    var split = text.Split(' ');
                    int sPos = 0;
                    int ePos = 0;
                    foreach (var word in split)
                    {
                        sPos = ePos;
                        ePos += word.Length;

                        if (sPos <= pos && ePos >= pos)
                        {
                            toReturn = word;
                            break;
                        }

                        ePos++; // due to space
                    }
                }
            }

            // control.Document.Selection.StartPosition = origStart;
            // control.Document.Selection.EndPosition = origEnd;

            return toReturn;
        }

        public static Windows.UI.Color SnapColor(this Windows.UI.Color v)
        {
            //const float HeaderBrightnessThreshold = 35f;
            const float HeaderColorThreshold = 186f; // 224f; 
            try
            {
                if (((float)v.R * 0.299f + (float)v.G * 0.587f + (float)v.B * 0.114f) > HeaderColorThreshold)
                    return new Windows.UI.Color() { R = 255, G = 255, B = 255, A = 255 }; 
                else
                    return new Windows.UI.Color() { R = 0, G = 0, B = 0, A = 255 };
            }
            catch (Exception)
            {
                return new Windows.UI.Color();
            }
        }
        public static Windows.UI.Color GetOppositeColor(this Windows.UI.Color v)
        {
            return new Color() { R = (byte)(255 - (int)v.R), G = (byte)(255 - (int)v.G), B = (byte)(255 - (int)v.B), A = 255 };
        }
        public static Point Add(this Point p, Point p2)
        {
            Point pp = p;
            pp.X += p2.X;
            pp.Y += p2.Y;
            return pp;
        }
        public static Point Subtract(this Point p, Point p2)
        {
            Point pp = p;
            pp.X -= p2.X;
            pp.Y -= p2.Y;
            return pp;
        }
        public static Windows.UI.Color GetBestContrastingColor(this Windows.UI.Color col)
        {
            return col.SnapColor().GetOppositeColor();
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
            return firstFloat * (1.0f - by) + secondFloat * by;
        }
        public static double Lerp(this double firstFloat, double secondFloat, double by)
        {
            return firstFloat * (1.0 - by) + secondFloat * by;
        }
        public static byte Lerp(this byte first, byte second, float by)
        {
            float a = first;
            float b = second;
            return (byte)MathF.Floor(a.Lerp(b, by) + 0.5f);
        }
        public static byte Lerp(this byte first, byte second, double by)
        {
            float a = first;
            float b = second;
            return (byte)MathF.Floor(a.Lerp(b, (float)by) + 0.5f);
        }
        public static Windows.UI.Color Lerp(this Windows.UI.Color first, Windows.UI.Color second, float by)
        {
            return new Windows.UI.Color()
            {
                R = Lerp(first.R, second.R, by),
                G = Lerp(first.G, second.G, by),
                B = Lerp(first.B, second.B, by),
                A = Lerp(first.A, second.A, by)
            };
        }
        public static cweeTask<Windows.UI.Color> Lerp(this cweeTask<Windows.UI.Color> first, Windows.UI.Color second, float by)
        {
            return first.ContinueWith(()=> {
                return new Windows.UI.Color()
                {
                    R = Lerp(first.Result.R, second.R, by),
                    G = Lerp(first.Result.G, second.G, by),
                    B = Lerp(first.Result.B, second.B, by),
                    A = Lerp(first.Result.A, second.A, by)
                };
            }, false);
        }
        public static Windows.UI.Color LerpStratified(this Windows.UI.Color first, Windows.UI.Color second, float by)
        {
            float zeroToThree = by * 3;
            Windows.UI.Color toReturn = new Windows.UI.Color();
            if (zeroToThree <= 1)
            {
                toReturn.R = first.R.Lerp(second.R, zeroToThree);
                toReturn.G = first.G;
                toReturn.B = first.B;
            }
            else if (zeroToThree <= 2)
            {
                toReturn.R = second.R;
                toReturn.G = first.G.Lerp(second.G, zeroToThree - 1);
                toReturn.B = first.B;
            }
            else
            {
                toReturn.R = second.R;
                toReturn.G = second.G;
                toReturn.B = first.B.Lerp(second.B, zeroToThree - 2);
            }
            toReturn.A = first.A.Lerp(second.A, by);

            return toReturn;
        }

        public static cweeTask<SolidColorBrush> Blend(this SolidColorBrush a, SolidColorBrush b, float Lerp)
        {
            if (b == null) return EdmsTasks.cweeTask.CompletedTask(a);
            return EdmsTasks.InsertJob(()=> {
                (Color, Color) col = (a.Color, b.Color);
                Color r = new Color()
                {
                    R = (byte)((Lerp * (float)col.Item1.R) + ((1.0f - Lerp) * (float)col.Item2.R)),
                    G = (byte)((Lerp * (float)col.Item1.G) + ((1.0f - Lerp) * (float)col.Item2.G)),
                    B = (byte)((Lerp * (float)col.Item1.B) + ((1.0f - Lerp) * (float)col.Item2.B)),
                    A = (byte)((Lerp * (float)col.Item1.A) + ((1.0f - Lerp) * (float)col.Item2.A))
                };
                return new SolidColorBrush(r) { Opacity = (double)(((float)a.Opacity).Lerp((float)b.Opacity, Lerp)) };
            }, true);
        }
        public static cweeTask<SolidColorBrush> Blend(this cweeTask<SolidColorBrush> a, cweeTask<SolidColorBrush> b, float Lerp)
        {
            return a.ContinueWith(()=> {
                return b.ContinueWith(()=> {
                    (Color, Color) col = (a.Result.Color, b.Result.Color);
                    Color r = new Color()
                    {
                        R = (byte)((Lerp * (float)col.Item1.R) + ((1.0f - Lerp) * (float)col.Item2.R)),
                        G = (byte)((Lerp * (float)col.Item1.G) + ((1.0f - Lerp) * (float)col.Item2.G)),
                        B = (byte)((Lerp * (float)col.Item1.B) + ((1.0f - Lerp) * (float)col.Item2.B)),
                        A = (byte)((Lerp * (float)col.Item1.A) + ((1.0f - Lerp) * (float)col.Item2.A))
                    };
                    return new SolidColorBrush(r) { Opacity = (double)(((float)a.Result.Opacity).Lerp((float)b.Result.Opacity, Lerp)) };
                }, true);
            }, false);
        }
        public static string ToHex(this SolidColorBrush color)
        {
            return String.Format("#{0}{1}{2}{3}"
                , color.Color.A.ToString("X").Length == 1 ? String.Format("0{0}", color.Color.A.ToString("X")) : color.Color.A.ToString("X")
                , color.Color.R.ToString("X").Length == 1 ? String.Format("0{0}", color.Color.R.ToString("X")) : color.Color.R.ToString("X")
                , color.Color.G.ToString("X").Length == 1 ? String.Format("0{0}", color.Color.G.ToString("X")) : color.Color.G.ToString("X")
                , color.Color.B.ToString("X").Length == 1 ? String.Format("0{0}", color.Color.B.ToString("X")) : color.Color.B.ToString("X"));
        }
        public static int roundToNearest(this float i, int nearest)
        {
            return (int)(Math.Ceiling(i / (double)nearest) * nearest); // fixed
        }
        private static Windows.Foundation.Size CalculateSize(string text, double fontSize, TextBlock tb)
        {
            tb.Text = text;
            tb.FontSize = fontSize;
            tb.Measure(new Windows.Foundation.Size(Double.PositiveInfinity, Double.PositiveInfinity));
            //return new Windows.Foundation.Size(tb.ActualWidth, tb.ActualHeight);
            return tb.DesiredSize;
        }
        private static Windows.Foundation.Size CalculateSize(string text, double fontSize)
        {
            var tb = new TextBlock { Text = text, FontSize = fontSize };
            return CalculateSize(text, fontSize, tb);
        }
        public static double CalculateIdealFontSize(string text, double width, double height, TextBlock tb = null)
        {
            if (tb == null) tb = new TextBlock();

            int len = 0, mid = 0, offset = 0;
            Windows.Foundation.Size sample;
            bool res = false;
            // use binary search to find the index for the given time
            len = 100;
            mid = len;
            offset = 1; // normally 0, but I do not wish to explore x==0. 
            res = false;

            while (mid > 0)
            {
                mid = len >> 1;
                // OPTIMIZED ORDERING
                sample = CalculateSize(text, (double)(offset + mid), tb);
                if (sample.Width <= width && sample.Height <= height)
                {
                    offset += mid;
                    len -= mid;
                    res = true;
                    if (sample.Width == width && sample.Height == height) return offset;
                }
                else
                {
                    len -= mid;
                    res = false;
                }
            }
            len = offset + (res ? 1 : 0);
            return Math.Max((double)(len), 2.0);
        }
        public static void CalculateIdealFontSize(this TextBlock text, double width, double height, bool tryToCenter = true)
        {
            if (tryToCenter && text.Parent is Border)
            {
                (text.Parent as Border).VerticalAlignment = VerticalAlignment.Stretch;
                (text.Parent as Border).HorizontalAlignment = HorizontalAlignment.Stretch;

                text.LineStackingStrategy = LineStackingStrategy.BlockLineHeight;
                text.LineHeight = height;
                text.TextWrapping = TextWrapping.NoWrap;
                text.FontSize = CalculateIdealFontSize(text.Text, width, height);
                double a = Math.Max(0.0, (height - (text.FontSize + 5.0)));
                text.Margin = new Thickness(
                    0,
                   -Math.Max(0, (a) / 2.0),
                    0,
                  0
                );
                return;
            }
            else
            {
                text.FontSize = CalculateIdealFontSize(text.Text, width, height);
            }
        }
        public static Windows.Foundation.Size CalculateSize(this TextBlock text, double fontSize)
        {
            text.FontSize = fontSize;
            text.Measure(new Windows.Foundation.Size(Double.PositiveInfinity, Double.PositiveInfinity));
            return text.DesiredSize;
        }

        public class cweeFlyout : Flyout
        {
            public bool preventsClosing = false;
            public object Tag = null;
        }

        public static cweeFlyout SetFlyout(this UIElement obj, UIElement content, UIElement relativeTo = null, DependencyObject allowClickThrough = null, Point? position = null, bool showFlyout = true)
        {
            if (content == null)
            {
                if (obj.ContextFlyout != null && obj.ContextFlyout.IsOpen)
                {
                    obj.ContextFlyout.Hide();
                }
            }
            else
            {
                cweeFlyout toFly = null;
                if (obj.ContextFlyout != null && obj.ContextFlyout is cweeFlyout)
                {
                    toFly = obj.ContextFlyout as cweeFlyout;
                }
                if (toFly == null)
                {
                    toFly = new cweeFlyout();
                    toFly.Placement = Windows.UI.Xaml.Controls.Primitives.FlyoutPlacementMode.BottomEdgeAlignedLeft;
                    toFly.ShowMode = Windows.UI.Xaml.Controls.Primitives.FlyoutShowMode.TransientWithDismissOnPointerMoveAway;
                    toFly.LightDismissOverlayMode = LightDismissOverlayMode.Off;
                    toFly.AllowFocusWhenDisabled = false; // true? 
                    toFly.AllowFocusOnInteraction = false; // disable?
                    if (allowClickThrough != null)
                    {
                        toFly.OverlayInputPassThroughElement = allowClickThrough;
                    }
                    else
                    {
                        toFly.OverlayInputPassThroughElement = obj;
                    }

                    obj.ContextFlyout = toFly;
                }

                toFly.Content = content;

                if (relativeTo != null)
                {
                    var ttv = obj.TransformToVisual(Window.Current.Content);
                    Point screenCoords = ttv.TransformPoint(new Point(0, 0));
                    if (showFlyout)
                        toFly.ShowAt(relativeTo, new Windows.UI.Xaml.Controls.Primitives.FlyoutShowOptions()
                        {
                            ShowMode = Windows.UI.Xaml.Controls.Primitives.FlyoutShowMode.TransientWithDismissOnPointerMoveAway,
                            Placement = Windows.UI.Xaml.Controls.Primitives.FlyoutPlacementMode.BottomEdgeAlignedLeft,
                            Position = position
                        });
                }
                else
                {
                    if (showFlyout)
                        toFly.ShowAt(obj, new Windows.UI.Xaml.Controls.Primitives.FlyoutShowOptions()
                        {
                            ShowMode = Windows.UI.Xaml.Controls.Primitives.FlyoutShowMode.TransientWithDismissOnPointerMoveAway,
                            Placement = Windows.UI.Xaml.Controls.Primitives.FlyoutPlacementMode.BottomEdgeAlignedLeft,
                            Position = position
                        });
                }

            }

            return obj.ContextFlyout as cweeFlyout;
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

        public static bool ListMatches<T>(this SynchronizedCollection<T> original, List<T> other)
        {
            if ((original == null) == (other != null))
                return false;

            if ((original == null) && (other == null)) return true;

            if (original.Count != other.Count)
                return false;

            for (int i = original.Count - 1; i >= 0; i--)
            {
                if (!original[i].Equals(other[i]))
                {
                    return false;
                }
            }

            return true;
        }
        private static double minTime = System.DateTime.MinValue.ToUnixTimeSeconds();
        private static double maxTime = System.DateTime.MaxValue.ToUnixTimeSeconds();
        public static DateTime FromUnixTimeSeconds(this DateTime a, double unixTimeStamp)
        {
            // Unix timestamp is seconds past epoch
            if (unixTimeStamp == 0)
            {
                a = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToLocalTime();
            }
            else if (unixTimeStamp >= minTime && unixTimeStamp <= maxTime) {
                a = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).AddSeconds(unixTimeStamp).ToLocalTime();
            }
            else {
                a = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToLocalTime();
            }

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
            if ((startPosition + length) >= orig.Length)
            {
                return orig.Substring(startPosition, orig.Length - startPosition);
            }
            else
            {
                return orig.Substring(startPosition, length);
            }            
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
            AppExtension.ManageException(e, additionalMessage, membername, filepath, linenumber);
        }

        private static readonly MethodInfo CloneMethod = typeof(Object).GetMethod("MemberwiseClone", BindingFlags.NonPublic | BindingFlags.Instance);
        public static bool IsPrimitive(this Type type)
        {
            if (type == typeof(String)) return true;
            return (type.IsValueType & type.IsPrimitive);
        }
        public static Object Copy(this Object originalObject)
        {
            return InternalCopy(originalObject, new Dictionary<Object, Object>(new ReferenceEqualityComparer()));
        }
        private static Object InternalCopy(Object originalObject, IDictionary<Object, Object> visited)
        {
            if (originalObject == null) return null;
            var typeToReflect = originalObject.GetType();
            if (IsPrimitive(typeToReflect)) return originalObject;
            if (visited.ContainsKey(originalObject)) return visited[originalObject];
            if (typeof(Delegate).IsAssignableFrom(typeToReflect)) return null;
            var cloneObject = CloneMethod.Invoke(originalObject, null);
            if (typeToReflect.IsArray)
            {
                var arrayType = typeToReflect.GetElementType();
                if (IsPrimitive(arrayType) == false)
                {
                    Array clonedArray = (Array)cloneObject;
                    clonedArray.ForEach((array, indices) => array.SetValue(InternalCopy(clonedArray.GetValue(indices), visited), indices));
                }

            }
            visited.Add(originalObject, cloneObject);
            CopyFields(originalObject, visited, cloneObject, typeToReflect);
            RecursiveCopyBaseTypePrivateFields(originalObject, visited, cloneObject, typeToReflect);
            return cloneObject;
        }
        private static void RecursiveCopyBaseTypePrivateFields(object originalObject, IDictionary<object, object> visited, object cloneObject, Type typeToReflect)
        {
            if (typeToReflect.BaseType != null)
            {
                RecursiveCopyBaseTypePrivateFields(originalObject, visited, cloneObject, typeToReflect.BaseType);
                CopyFields(originalObject, visited, cloneObject, typeToReflect.BaseType, BindingFlags.Instance | BindingFlags.NonPublic, info => info.IsPrivate);
            }
        }
        private static void CopyFields(object originalObject, IDictionary<object, object> visited, object cloneObject, Type typeToReflect, BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy, Func<FieldInfo, bool> filter = null)
        {
            foreach (FieldInfo fieldInfo in typeToReflect.GetFields(bindingFlags))
            {
                if (filter != null && filter(fieldInfo) == false) continue;
                if (IsPrimitive(fieldInfo.FieldType)) continue;
                var originalFieldValue = fieldInfo.GetValue(originalObject);
                var clonedFieldValue = InternalCopy(originalFieldValue, visited);
                fieldInfo.SetValue(cloneObject, clonedFieldValue);
            }
        }
        public static T Copy<T>(this T original)
        {
            return (T)Copy((Object)original);
        }

        public static DateTime to_DateTime(this cweeDateTime cdt)
        {
            return new DateTime(cdt.year, cdt.month, cdt.day, cdt.hour, cdt.minute, cdt.second, cdt.milliseconds);
        }
    }
    public class ReferenceEqualityComparer : EqualityComparer<Object>
    {
        public override bool Equals(object x, object y)
        {
            return ReferenceEquals(x, y);
        }
        public override int GetHashCode(object obj)
        {
            if (obj == null) return 0;
            return obj.GetHashCode();
        }
    }
    namespace ArrayExtensions
    {
        public static class ArrayExtensions
        {
            public static void ForEach(this Array array, Action<Array, int[]> action)
            {
                if (array.LongLength == 0) return;
                ArrayTraverse walker = new ArrayTraverse(array);
                do action(array, walker.Position);
                while (walker.Step());
            }
        }

        internal class ArrayTraverse
        {
            public int[] Position;
            private int[] maxLengths;

            public ArrayTraverse(Array array)
            {
                maxLengths = new int[array.Rank];
                for (int i = 0; i < array.Rank; ++i)
                {
                    maxLengths[i] = array.GetLength(i) - 1;
                }
                Position = new int[array.Rank];
            }

            public bool Step()
            {
                for (int i = 0; i < Position.Length; ++i)
                {
                    if (Position[i] < maxLengths[i])
                    {
                        Position[i]++;
                        for (int j = 0; j < i; j++)
                        {
                            Position[j] = 0;
                        }
                        return true;
                    }
                }
                return false;
            }
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
                    catch (Exception e)
                    {
                        e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
                    }
                    return result;
                }
                else
                {
                    try
                    {
                        result = todo.Invoke();
                    }
                    catch (Exception e)
                    {
                        e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
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
                    catch (Exception e)
                    {
                        e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
                    }
                    return result;
                }
                else
                {
                    try
                    {
                        result = todo.Invoke();
                    }
                    catch (Exception e)
                    {
                        e.EdmsHandle(null, this.MemberName, this.FilePath, this.LineNumber);
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
        public class cweeAsyncOperation<T> : IAsyncOperation<T>
        {
            public cweeAsyncOperation() { }
            public cweeAsyncOperation(cweeTask a)
            {
                task = a;

                task.OnFinished += A_OnFinished;
            }


            private void A_OnFinished(object sender, cweeTask.FinishedArgs e)
            {

#if true
                InsertJob(() =>  {
                    whenCompleted(this, AsyncStatus.Completed);
                }, true, true);
#else
                   
                    EdmsTasks.InsertJob(() =>
                    {
                        whenCompleted(this, AsyncStatus.Completed);
                    }, Priority.Low, true, true);
#endif
            }

            public cweeTask task;

            T IAsyncOperation<T>.GetResults()
            {
                return (T)task.Result;
            }

            AsyncOperationCompletedHandler<T> whenCompleted = (IAsyncOperation<T> a, AsyncStatus b) => { };
            AsyncOperationCompletedHandler<T> IAsyncOperation<T>.Completed
            {
                get
                {
                    return whenCompleted;
                    // return (IAsyncOperation<T> a, AsyncStatus b) => { };
                }
                set
                {
                    whenCompleted = value;
                }
            }

            void IAsyncInfo.Cancel()
            {

            }

            void IAsyncInfo.Close()
            {

            }

            Exception IAsyncInfo.ErrorCode => null;

            uint IAsyncInfo.Id => (uint)task.GetHashCode();

            AsyncStatus IAsyncInfo.Status => task.IsFinished ? AsyncStatus.Completed : AsyncStatus.Started;
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

            public cweeAsyncOperation<T> AsAsyncOperation<T>()
            {
                return new cweeAsyncOperation<T>(this);
            }

            //public void Wait()
            //{
            //    while (!this.IsFinished)
            //    {
            //        EdmsTasks.DoJob();
            //    }
            //}

            public cweeTask()
            {
                todo = null;
                mainThreadOnly = false;
                canBeDeferred = true;
            }
            public cweeTask(cweeAction _todo, bool _mainThreadOnly, bool _canBeDeferred = true)
            {
                todo = _todo;
                mainThreadOnly = _mainThreadOnly;
                canBeDeferred = _canBeDeferred;
            }
            public cweeTask(Action _todo, bool _mainThreadOnly, bool _canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
            {
                todo = new cweeAction(_todo, membername, filepath, linenumber);
                mainThreadOnly = _mainThreadOnly;
                canBeDeferred = _canBeDeferred;
            }
            public cweeTask(Func<dynamic> _todo, bool _mainThreadOnly, bool _canBeDeferred = true, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
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
                    if (mainThreadOnly && !AppExtension.ApplicationInfo.IsMainThread())
                    {
                        throw new Exception("Threaded 'main UI' job should have already been submitted to the UI thread.");
                    }
                    else
                    {
                        Result = todo.Invoke(); // do the job
                    }
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
#if false
                if (!followUp.mainThreadOnly && !AppExtension.ApplicationInfo.IsMainThread())
                {
                    followUp.Resolve();
                }
                else
                {
                    EdmsTasks.InsertJob(followUp);
                }
#else
                if (followUp.mainThreadOnly == AppExtension.ApplicationInfo.IsMainThread())
                {
                    followUp.Resolve();
                }
                else
                {
                    EdmsTasks.InsertJob(followUp);
                }
#endif
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

        public static System.Collections.Concurrent.ConcurrentDictionary<string, Stopwatch> mainThreadTimers = new System.Collections.Concurrent.ConcurrentDictionary<string, Stopwatch>();

        public static bool DoWriteAllJobTimes = false;

        internal static int numCores = WaterWatch.GetNumLogicalCoresOnMachine() - 1;
        internal static AtomicInt numParallelJobs = new AtomicInt(0);
        internal static AtomicInt mainThreadLooping = new AtomicInt(0);
        public static string _MainThreadJobName = "";
        public static string MainThreadJobName { get { return _MainThreadJobName; } set { _MainThreadJobName = value; } }

        public static bool DoJob()
        {
            bool thisIsOnMainThread = AppExtension.ApplicationInfo.IsMainThread();
            bool isInForeground = AppExtension.ApplicationInfo.InForeground;
            bool toReturn = true;

            {
                {
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
                        foreach (var job in parallelToDo)
                        {
                            var jobCopy = job;
                            Task.Run(() =>
                            {
                                if (jobCopy.mainThreadOnly)
                                {
                                    main_actions.Enqueue(jobCopy);
                                }
                                else
                                {
                                    jobCopy.Resolve();
                                }
                            });
                        }
                    }
                }

                if (System.Threading.Thread.CurrentThread.ManagedThreadId == mainThreadID.Get()) 
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
                else
                {
                    if (mainThreadLooping.Increment() == 1 && main_actions.Count > 0)
                    {
                        Task.Run(async () =>
                        {
                            /*await */

                            bool submitted = await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.TryRunAsync(CoreDispatcherPriority.Normal, /*async*/ () =>
                            {
                                //deq = new cweeDequeue();
                                mainThreadID.Set(System.Threading.Thread.CurrentThread.ManagedThreadId);

                                var sw = new Stopwatch();
                                sw.Start();
                                while (main_actions.Dequeue(out cweeTask todo)){
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

                                mainThreadLooping.Decrement();
                            });
                            if (!submitted)
                            {
                                mainThreadLooping.Decrement();
                            }
                        });
                    }
                    else
                    {
                        mainThreadLooping.Decrement();

                        if (DoWriteAllJobTimes == true)
                        {
                            DoWriteAllJobTimes = false;
                            string content = "";
                            Dictionary<string, double> FuncToTime = new Dictionary<string, double>();
                            List<string> keys = mainThreadTimers.Keys.ToList();
                            foreach (var key in keys)
                            {
                                if (mainThreadTimers.TryGetValue(key, out Stopwatch v))
                                {
                                    var seconds = v.ElapsedMilliseconds / 1000.0;
                                    FuncToTime[key] = seconds;
                                    content = content.AddToDelimiter($"{key}\t:\t{seconds}", "\n");
                                }
                            }
                            WaterWatch.AddToLog(WaterWatch.GetDataDirectory() + "\\SLOWLOG.txt", content + "\n\n");
                        }

                    }
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

        public EdmsTasks.cweeAsyncOperation<Q> AsAsyncOperation<Q>()
        {
            return new EdmsTasks.cweeAsyncOperation<Q>(actual);
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
        public cweeTask() {
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
    public class TryLock : IDisposable
    {
        private object locked;

        public bool HasLock { get; private set; }

        public TryLock(object obj)
        {
            if (Monitor.TryEnter(obj))
            {
                HasLock = true;
                locked = obj;
            }
        }

        public void Dispose()
        {
            if (HasLock)
            {
                Monitor.Exit(locked);
                locked = null;
                HasLock = false;
            }
        }
    }
    public class ScopeGuard : IDisposable
    {
        private Action dispose_;
        private bool disposed_;

        public ScopeGuard(Action DoOnScopeEnd)
        {
            dispose_ = DoOnScopeEnd;
            disposed_ = false;
        }
        void IDisposable.Dispose()
        {
            // the "official" pattern is dumb boilerplate
            //   for this application
            if (!disposed_)
            {
                dispose_.Invoke();
            }
            disposed_ = true;
        }
        ~ScopeGuard()
        {
            // the "official" pattern is dumb boilerplate
            //   for this application
            if (!disposed_)
            {
                dispose_.Invoke();
            }
            disposed_ = true;
        }
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
                    newTask.ContinueWith(()=> {
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
                throw(new Exception("User must call dequeue at least once with a valid action before calling the basic time-based dequeue method."));
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
            _actions.TryAdd(key, new EdmsTasks.cweeAction(todo, membername, filepath, linenumber));
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
            public AtomicInt count = new AtomicInt(1);
            public EdmsTasks.cweeAction action = null;
        }

        private Mutex mut = new Mutex();
        private System.Collections.Concurrent.ConcurrentDictionary<int, CountedActions> _actions = new System.Collections.Concurrent.ConcurrentDictionary<int, CountedActions>();
        // private Dictionary<int, CountedActions> _actions = new Dictionary<int, CountedActions>();
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
            //mut.WaitOne();
            if (_actions.TryGetValue(key, out CountedActions v))
            {
                v.action = new EdmsTasks.cweeAction(todo, membername, filepath, linenumber);
                v.count.Increment();
            }
            else
            {
                _actions.TryAdd(key, new CountedActions() { count = new AtomicInt(1), action = new EdmsTasks.cweeAction(todo, membername, filepath, linenumber) });
            }
            //mut.ReleaseMutex();
        }

        public bool RemoveAction(int key)
        {
            bool toR = false;
            //mut.WaitOne();
            if (_actions.TryGetValue(key, out CountedActions v))
            {
                if (v.count.Decrement() == 0)
                {
                    _actions.TryRemove(key, out CountedActions v2);
                    toR = true;
                }
            }            
            //mut.ReleaseMutex();
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
                    // t.owner.mut.WaitOne();
                    // Dictionary<int, CountedActions> actions = new Dictionary<int, CountedActions>(t.owner._actions);
                    // t.owner.mut.ReleaseMutex();

                    List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();
                    foreach (var x in t.owner._actions)
                    {
                        tasks.Add(new EdmsTasks.cweeTask(x.Value.action, t._mainThread, true));
                    }
                    EdmsTasks.cweeTask.InsertListAsTask(tasks, false).ContinueWith(()=> {
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

    public class AutomaticDisposal<T> where T : System.IDisposable
    {
        public AutomaticDisposal() { }
        public AutomaticDisposal(T d) { obj = d; }
        ~AutomaticDisposal()
        {
            if (obj != null)
                obj.Dispose();
        }

        public T obj;
    }

    public static class cweeXamlHelper
    {
        public static string GetTextFromContainers(object obj)
        {
            if (obj is null)
            {
                return null;
            }
            if (obj is string)
            {
                return obj as string;
            }
            if (obj is TextBlock)
            {
                return (obj as TextBlock).Text;
            }
            if (obj is Panel)
            {
                foreach (var x in (obj as Panel).Children)
                {
                    var t = GetTextFromContainers(x);
                    if (t != null)
                    {
                        return t;
                    }
                }
            }
            if (obj is Border)
            {
                var t = GetTextFromContainers((obj as Border).Child);
                if (t != null)
                {
                    return t;
                }
            }

            return null;
        }
        public static TextBlock SimpleTextBlock(string text, HorizontalAlignment horizontalAlignment = HorizontalAlignment.Stretch)
        {
            var tb = new TextBlock()
            {
                Margin = new Thickness(0),
                Padding = new Thickness(0),
                Text = text,
                TextWrapping = TextWrapping.Wrap,
                HorizontalAlignment = horizontalAlignment,
                VerticalAlignment = VerticalAlignment.Stretch,
                HorizontalTextAlignment = ((horizontalAlignment == HorizontalAlignment.Stretch) || (horizontalAlignment == HorizontalAlignment.Center)) ? TextAlignment.Center : TextAlignment.Left,
                Style = cweeXamlHelper.StaticStyleResource("cweeTextBlock"),
                IsRightTapEnabled = true
            };
            tb.RightTapped += (object sender, RightTappedRoutedEventArgs e) =>
            {
                StackPanel tempGrid = new StackPanel() { Orientation = Orientation.Vertical, HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Margin = new Thickness(0), Padding = new Thickness(0) };
                {
                    {
                        var but = new Button()
                        {
                            HorizontalAlignment = HorizontalAlignment.Stretch,
                            VerticalAlignment = VerticalAlignment.Stretch,
                            Margin = new Thickness(0),
                            Padding = new Thickness(0)
                        };
                        but.Content = "Copy to Clipboard";
                        but.Click += (object sender2, RoutedEventArgs e2) =>
                        {
                            EdmsTasks.InsertJob(() =>
                            {
                                Functions.CopyToClipboard(text);
                            }, true);
                        };
                        tempGrid.Children.Add(but);
                    }
                }
                var flyout = tb.SetFlyout(tempGrid, tb, tb, e.GetPosition(tb));
                e.Handled = true;
            };
            return tb;
        }
        public static cweeTask<string> UIElementToFile(UIElement canvas, int width, int height)
        {
            return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() =>
            {
                Windows.UI.Xaml.Media.Imaging.RenderTargetBitmap bitmap = new Windows.UI.Xaml.Media.Imaging.RenderTargetBitmap();

                string toReturn = "";
                var toReturnJob = new EdmsTasks.cweeTask(() =>
                {
                    return toReturn;
                }, false, true);

                //Render a bitmap image
                EdmsTasks.InsertJob(async () =>
                {
                    await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, async () =>
                    {
                        await bitmap.RenderAsync(canvas, width, height);
                        {
                            var storageFolder = await StorageFolder.GetFolderFromPathAsync(WaterWatch.GetDataDirectory());
                            StorageFile file = await storageFolder.CreateFileAsync("prefabScreenshot.jpg", CreationCollisionOption.GenerateUniqueName);
                            using (var fileSaveStream = await file.OpenAsync(FileAccessMode.ReadWrite))
                            {
                                var encoder = await Windows.Graphics.Imaging.BitmapEncoder.CreateAsync(Windows.Graphics.Imaging.BitmapEncoder.PngEncoderId, fileSaveStream);
                                IBuffer pixelBuffer = await bitmap.GetPixelsAsync();

                                using (DataReader dataReader = DataReader.FromBuffer(pixelBuffer))
                                {
                                    byte[] bytes = new byte[pixelBuffer.Length];
                                    dataReader.ReadBytes(bytes);
                                    encoder.SetPixelData(
                                        Windows.Graphics.Imaging.BitmapPixelFormat.Bgra8,
                                        Windows.Graphics.Imaging.BitmapAlphaMode.Ignore,
                                        (uint)bitmap.PixelWidth,
                                        (uint)bitmap.PixelHeight,
                                        Windows.Graphics.Display.DisplayInformation.GetForCurrentView().LogicalDpi,
                                        Windows.Graphics.Display.DisplayInformation.GetForCurrentView().LogicalDpi,
                                        bytes
                                    );
                                }
                                await encoder.FlushAsync(); // The application hangs here
                            }
                            toReturn = toReturn + (WaterWatch.GetDataDirectory() + "\\prefabScreenshot.jpg");

                            toReturnJob.QueueJob(); // queue it
                        }
                    });
                }, EdmsTasks.Priority.Low, true);
                return toReturnJob;
            }, EdmsTasks.Priority.Low, true);
        }
        public static cweeTask<Windows.UI.Xaml.Controls.Image> UIElementToImage(UIElement canvas, int width, int height)
        {
            return EdmsTasks.InsertJob(() =>
            {
                Windows.UI.Xaml.Media.Imaging.RenderTargetBitmap bitmap = new Windows.UI.Xaml.Media.Imaging.RenderTargetBitmap();
                //Render a bitmap image
                return EdmsTasks.InsertJob(async () =>
                {
                    await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, async () =>
                    {
                        await bitmap.RenderAsync(canvas, width, height);
                        using (var inputImgStream = new InMemoryRandomAccessStream()) //this doesn't work
                        {
                            var encoder = await Windows.Graphics.Imaging.BitmapEncoder.CreateAsync(Windows.Graphics.Imaging.BitmapEncoder.PngEncoderId,
                                inputImgStream
                                    ); // I suspect passing the MemoryStream is the issue. While 'StorageFile' is used there are no issues.

                            IBuffer pixelBuffer = await bitmap.GetPixelsAsync();

                            using (DataReader dataReader = DataReader.FromBuffer(pixelBuffer))
                            {
                                byte[] bytes = new byte[pixelBuffer.Length];
                                dataReader.ReadBytes(bytes);
                                encoder.SetPixelData(
                                    Windows.Graphics.Imaging.BitmapPixelFormat.Bgra8,
                                    Windows.Graphics.Imaging.BitmapAlphaMode.Straight,
                                    (uint)bitmap.PixelWidth,
                                    (uint)bitmap.PixelHeight,
                                    Windows.Graphics.Display.DisplayInformation.GetForCurrentView().LogicalDpi,
                                    Windows.Graphics.Display.DisplayInformation.GetForCurrentView().LogicalDpi,
                                    bytes
                                );
                            }
                            await encoder.FlushAsync(); // The application hangs here
                        }
                    });
                }, true).ContinueWith(()=> {
                    Windows.UI.Xaml.Controls.Image newImage = new Windows.UI.Xaml.Controls.Image();
                    newImage.Width = width;
                    newImage.Height = height;
                    newImage.Source = bitmap;

                    return newImage;
                }, true);
            }, true);
        }
        public static cweeTask<Windows.UI.Xaml.Controls.Image> PixelsToImage(IBuffer pixelBuffer, int width, int height)
        {
            return EdmsTasks.InsertJob(() =>
            {
                Windows.UI.Xaml.Media.Imaging.RenderTargetBitmap bitmap = new Windows.UI.Xaml.Media.Imaging.RenderTargetBitmap();
                //Render a bitmap image
                return EdmsTasks.InsertJob(async () =>
                {
                    await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, async () =>
                    {
                        using (var inputImgStream = new InMemoryRandomAccessStream()) //this doesn't work
                        {
                            var encoder = await Windows.Graphics.Imaging.BitmapEncoder.CreateAsync(Windows.Graphics.Imaging.BitmapEncoder.PngEncoderId, inputImgStream);

                            using (DataReader dataReader = DataReader.FromBuffer(pixelBuffer))
                            {
                                byte[] bytes = new byte[pixelBuffer.Length];
                                dataReader.ReadBytes(bytes);
                                encoder.SetPixelData(
                                    Windows.Graphics.Imaging.BitmapPixelFormat.Bgra8,
                                    Windows.Graphics.Imaging.BitmapAlphaMode.Straight,
                                    (uint)bitmap.PixelWidth,
                                    (uint)bitmap.PixelHeight,
                                    Windows.Graphics.Display.DisplayInformation.GetForCurrentView().LogicalDpi,
                                    Windows.Graphics.Display.DisplayInformation.GetForCurrentView().LogicalDpi,
                                    bytes
                                );
                            }
                            await encoder.FlushAsync(); // The application hangs here
                        }
                    });
                }, true).ContinueWith(()=> {
                    Windows.UI.Xaml.Controls.Image newImage = new Windows.UI.Xaml.Controls.Image();
                    newImage.Width = width;
                    newImage.Height = height;
                    newImage.Source = bitmap;

                    return newImage;
                }, true);                
            }, true);
        }
        public static cweeTask<Windows.UI.Xaml.Controls.Image> PixelsToImage(byte[] bytes, int width, int height, Windows.UI.Xaml.Controls.Image newImage)
        {
            return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                // var newImage = new Windows.UI.Xaml.Controls.Image();
                var toReturn = new EdmsTasks.cweeTask(() => { return newImage; }, true, true);

                var t1 = EdmsTasks.InsertJob(() =>
                {
                    var stream = new InMemoryRandomAccessStream();
                    BitmapEncoder.CreateAsync(BitmapEncoder.PngEncoderId, stream).AsTask().ContinueWith((Task<BitmapEncoder> encoderJob, object d) =>
                    {
                        BitmapEncoder encoder = encoderJob.Result;
                        encoder.SetPixelData(BitmapPixelFormat.Rgba8, BitmapAlphaMode.Straight, (uint)width, (uint)height, 96.0, 96.0, bytes);
                        encoder.FlushAsync().AsTask().ContinueWith((Task flushJob, object d2) =>
                        {
                            EdmsTasks.InsertJob(() =>
                            {
                                WriteableBitmap image = new WriteableBitmap(width, height);
                                Windows.Storage.Streams.RandomAccessStreamReference.CreateFromStream(
                                    stream
                                ).OpenReadAsync().AsTask().ContinueWith((Task<IRandomAccessStreamWithContentType> newStreamTask, object d3) =>
                                {
                                    EdmsTasks.InsertJob(() =>
                                    {
                                        IRandomAccessStreamWithContentType newStream = newStreamTask.Result;
                                        image.SetSourceAsync(newStream).AsTask().ContinueWith((Task setSourceTask) =>
                                        {
                                            EdmsTasks.InsertJob(() =>
                                            {
                                                image.Invalidate();
                                                if (newImage.Width == double.NaN) newImage.Width = width;
                                                if (newImage.Height == double.NaN) newImage.Height = height;
                                                newImage.Source = image;

                                                toReturn.QueueJob();
                                            }, true);
                                        });
                                    }, true);
                                }, null);
                            }, true);
                        }, null);
                    }, null);
                }, true);

                return toReturn;
            }, true);
        }

        public static void RepeatUntilTrue(Func<bool> ifTrue, Action ThenDoThis)
        {
            if (ifTrue())
            {
                EdmsTasks.InsertJob(ThenDoThis);
            }
            else
            {
                EdmsTasks.InsertJob(() =>
                {
                    RepeatUntilTrue(ifTrue, ThenDoThis);
                });
            }
        }

        private static System.Collections.Concurrent.ConcurrentDictionary<string, SolidColorBrush> _cache_ThemeColor = new System.Collections.Concurrent.ConcurrentDictionary<string, SolidColorBrush>();        
        public static cweeTask<SolidColorBrush> ThemeColor(string name)
        {
            if (name == "random")
            {
                return EdmsTasks.InsertJob(() => {
                    return new SolidColorBrush(new Windows.UI.Color() { A = 255, R = (byte)WaterWatch.RandomInt(0, 254), G = (byte)WaterWatch.RandomInt(0, 254), B = (byte)WaterWatch.RandomInt(0, 254) });
                }, true);
            }
            else
            {
                if (_cache_ThemeColor.TryGetValue(name, out SolidColorBrush b))
                {
                    return EdmsTasks.cweeTask.CompletedTask(b);
                }
                else
                {
                    return EdmsTasks.InsertJob(() => {
                        var toR = (SolidColorBrush)AppExtension.ApplicationInfo.Current.Resources[name];
                        if (toR != null)
                        {
                            _cache_ThemeColor.TryAdd(name, toR);
                            return toR;
                        }
                        else
                        {
                            throw new Exception("Could not create the solid color brush.");
                        }
                    }, true);
                }
            }
        }
        public static cweeTask<Color> ThemeColorCol(string name)
        {
            var col = ThemeColor(name);
            return col.ContinueWith(()=> {
                return col.Result.Color;
            }, true);
        }
        public static DataTemplate StaticDataTemplateResource(string name)
        {
            return (DataTemplate)AppExtension.ApplicationInfo.Current.Resources[name];
        }

        public static Style StaticStyleResource(string name)
        {
            return (Style)AppExtension.ApplicationInfo.Current.Resources[name];
        }

        /// <summary>
        /// Class that helps the creation of control and data templates by using delegates.
        /// </summary>
        public static class TemplateGenerator
        {
            public static bool Create(out DataTemplate toReturn, string XamlContent = @"<Ellipse Height=""8"" Width=""8"" Stroke=""{ ThemeResource cweeWhite }""  StrokeThickness=""1"" Fill=""{ ThemeResource cweeDarkBlue}"" Margin=""0,0,0,0"" HorizontalAlignment=""Center"" VerticalAlignment=""Center"" />")
            {
                //StringReader stringReader = new StringReader(
                //    @"<DataTemplate xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""> 
                //        <Ellipse Height=""8"" Width=""8"" Stroke=""{ ThemeResource cweeWhite }""  StrokeThickness=""1"" Fill=""{ ThemeResource cweeDarkBlue}"" Margin=""0,0,0,0"" HorizontalAlignment=""Center"" VerticalAlignment=""Center"" />
                //    </DataTemplate>"
                //);
                //System.Xml.XmlReader xmlReader = System.Xml.XmlReader.Create(stringReader);
                // return System.Xml.XamlReader.Load(xmlReader) as DataTemplate;

                string command = @"<DataTemplate xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""> " + XamlContent + @" </DataTemplate>";

                toReturn = null;
                try
                {
                    toReturn = Windows.UI.Xaml.Markup.XamlReader.Load(command) as DataTemplate;
                    return true;
                }
                catch (Exception) { }
                return false;
            }
        }

#if true
 

#endif


#if false
        public class TelerikChartDetails
        {
            public object y_axis_title = null; // binding OR string            
            public object y_axis_min = null; // binding OR double
            public object y_axis_max = null; // binding OR double
            public object y_axis_majorStep = null; // binding OR double
            public IContentFormatter y_axis_formatter = new ScadaAxisLabelFormatter(); // instance of a formatter
            public IContentFormatter x_axis_formatter = new DateTimeAxisLabelFormatter_ReducedFrequency(); // instance of a formatter
            public object x_axis_min = null; // binding OR DateTime
            public object x_axis_max = null; // binding OR DateTime
            public List<TelerikChartData> charts = new List<TelerikChartData>();

            //public event Chart.ClosestDataPointChangedEventHandler ClosestDataPointChanged;
            //public void OnClosestDataPointChanged(object sender, CategoricalDataPoint dataPoint)
            //{
            //if (ClosestDataPointChanged != null)
            //{
            //    ClosestDataPointChanged(sender, new Chart.Annotations.DataPointEventArgs(dataPoint));
            //}
            //}
            //public void ChartTrackBallBehavior_TrackInfoUpdated(object sender, Telerik.UI.Xaml.Controls.Chart.TrackBallInfoEventArgs e)
            //{
            //CategoricalDataPoint closestDataPoint = e.Context.ClosestDataPoint.DataPoint as CategoricalDataPoint;
            //OnClosestDataPointChanged(sender, closestDataPoint);
            //}
        }
        public static Grid CreateTelerikChart(TelerikChartDetails chartData)
        {
            Grid outerGrid = new Grid();
            // outerGrid fromatting
            {
                /*                 
                        <Grid.ColumnDefinitions>
                            <ColumnDefinition Width="20" />
                            <ColumnDefinition Width="*" />
                        </Grid.ColumnDefinitions>                 
                 */
                outerGrid.HorizontalAlignment = HorizontalAlignment.Stretch;
                outerGrid.VerticalAlignment = VerticalAlignment.Stretch;
                outerGrid.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(20, GridUnitType.Pixel) });
                outerGrid.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Star) });
                outerGrid.Margin = new Thickness(0, 0, 0, 0);
                outerGrid.Padding = new Thickness(0, 0, 0, 0);
            }

            // YaxisTitleBlock
            {
                /*
                        <Border  >
                            <controls:LayoutTransformControl VerticalAlignment="Stretch" HorizontalAlignment="Stretch">
                                <controls:LayoutTransformControl.Transform>
                                    <TransformGroup>
                                        <RotateTransform Angle="-90" />
                                        <ScaleTransform ScaleX="1" ScaleY="1" />
                                        <SkewTransform AngleX="0" AngleY="0" />
                                    </TransformGroup>
                                </controls:LayoutTransformControl.Transform>
                                <Border SizeChanged="ForceChildrenSizesToMatch" VerticalAlignment="Stretch" HorizontalAlignment="Stretch">
                                    <TextBlock Text="{x:Bind vm.YAxisTitle,Mode=OneWay}"  Margin="0" Padding="0"
                                        FontSize="20" Foreground="{ThemeResource cweeDarkBlue}" TextWrapping="WrapWholeWords"
                                        HorizontalAlignment="Center" VerticalAlignment="Center" HorizontalTextAlignment="Center"
                                        TextAlignment="Center"  SizeChanged="AutomaticTextSize"/>
                                </Border>
                            </controls:LayoutTransformControl>
                        </Border>                 
                 */

                Border YaxisTitleBlock = new Border();
                // YaxisTitleBlock formatting
                {
                    /* Margin="0,0,0,0" VerticalAlignment="Stretch" HorizontalAlignment="Stretch" */
                    YaxisTitleBlock.Margin = new Thickness(0);
                    YaxisTitleBlock.VerticalAlignment = VerticalAlignment.Stretch;
                    YaxisTitleBlock.HorizontalAlignment = HorizontalAlignment.Stretch;
                }
                // YaxisTitleBlock content
                {
                    Microsoft.Toolkit.Uwp.UI.Controls.LayoutTransformControl rotator = new Microsoft.Toolkit.Uwp.UI.Controls.LayoutTransformControl();
                    {
                        rotator.VerticalAlignment = VerticalAlignment.Stretch;
                        rotator.HorizontalAlignment = HorizontalAlignment.Stretch;
                        {
                            /*
                                    <controls:LayoutTransformControl.Transform>
                                        <TransformGroup>
                                            <RotateTransform Angle="-90" />
                                            <ScaleTransform ScaleX="1" ScaleY="1" />
                                            <SkewTransform AngleX="0" AngleY="0" />
                                        </TransformGroup>
                                    </controls:LayoutTransformControl.Transform>                         
                             */

                            TransformGroup tg = new TransformGroup();
                            tg.Children.Add(new RotateTransform() { Angle = -90 });
                            tg.Children.Add(new ScaleTransform() { ScaleX = 1, ScaleY = 1 });
                            tg.Children.Add(new SkewTransform() { AngleX = 0, AngleY = 0 });
                            rotator.Transform = tg;
                        }
                    }
                    {
                        /*
                                    <Border SizeChanged="ForceChildrenSizesToMatch" VerticalAlignment="Stretch" HorizontalAlignment="Stretch">
                                        <TextBlock Text="{x:Bind vm.YAxisTitle,Mode=OneWay}"  Margin="0" Padding="0"
                                            FontSize="20" Foreground="{ThemeResource cweeDarkBlue}" TextWrapping="WrapWholeWords"
                                            HorizontalAlignment="Center" VerticalAlignment="Center" HorizontalTextAlignment="Center"
                                            TextAlignment="Center"  SizeChanged="AutomaticTextSize"/>
                                    </Border>                     
                         */
                        Border b = new Border();
                        {
                            b.SizeChanged += ForceChildrenSizesToMatch;
                            b.VerticalAlignment = VerticalAlignment.Stretch;
                            b.HorizontalAlignment = HorizontalAlignment.Stretch;
                            {
                                TextBlock tb = new TextBlock();
                                {
                                    /*
                                        <TextBlock Text="{x:Bind vm.YAxisTitle,Mode=OneWay}"  Margin="0" Padding="0"
                                            FontSize="20" Foreground="{ThemeResource cweeDarkBlue}" TextWrapping="WrapWholeWords"
                                            HorizontalAlignment="Center" VerticalAlignment="Center" HorizontalTextAlignment="Center"
                                            TextAlignment="Center"  SizeChanged="AutomaticTextSize"/>                                 
                                     */
                                    if (chartData.y_axis_title != null)
                                    {
                                        if (chartData.y_axis_title is Binding)
                                        {
                                            tb.SetBinding(TextBlock.TextProperty, chartData.y_axis_title as Binding);
                                        }
                                        else if (chartData.y_axis_title is string)
                                        {
                                            tb.Text = chartData.y_axis_title as string;
                                        }
                                    }

                                    tb.Margin = new Thickness(0);
                                    tb.Padding = new Thickness(0);
                                    tb.FontSize = 20;
                                    tb.Foreground = ThemeColor("cweeDarkBlue");
                                    tb.TextWrapping = TextWrapping.WrapWholeWords;
                                    tb.HorizontalAlignment = HorizontalAlignment.Center;
                                    tb.VerticalAlignment = VerticalAlignment.Center;
                                    tb.HorizontalTextAlignment = TextAlignment.Center;
                                    tb.TextAlignment = TextAlignment.Center;
                                    tb.SizeChanged += AutomaticTextSize;
                                }
                                b.Child = tb;
                            }
                        }
                        rotator.Child = b;
                    }
                    YaxisTitleBlock.Child = rotator;
                }
                outerGrid.Children.Add(YaxisTitleBlock);
            }

            // Chart
            {
                RadCartesianChart rcc = new RadCartesianChart();
                // rcc formatting 
                {
                    rcc.Margin = new Thickness(0, 0, -35, 8);
                    rcc.ClipToBounds = false;
                    rcc.EmptyContent = "No Data Available";
                    rcc.Zoom = new Windows.Foundation.Size(1, 1);
                    // rcc.Background = ThemeColor("cweePageBackground");
                    rcc.HorizontalAlignment = HorizontalAlignment.Stretch;
                    rcc.VerticalAlignment = VerticalAlignment.Stretch;
                    //rcc.Tag = chartData;
                }

                // rcc behaviors
                {
                    var tbb = new ChartTrackBallBehavior() { ShowIntersectionPoints = true, LineStyle = StaticStyleResource("trackBallLineStyle") };
                    rcc.Behaviors.Add(tbb);
                }

                // rcc horizontal axis
                {
                    var ha = new DateTimeContinuousAxis()
                    {
                        VerticalAlignment = VerticalAlignment.Bottom,
                        HorizontalAlignment = HorizontalAlignment.Center,
                        VerticalLocation = AxisVerticalLocation.Top,
                        LabelFormat = "{}{0,0:HH}",
                        LabelStyle = StaticStyleResource("dateTimeAxisLabelStyle")
                    };
                    if (chartData.x_axis_min != null)
                    {
                        if (chartData.x_axis_min is Binding)
                        {
                            ha.SetBinding(DateTimeContinuousAxis.MinimumProperty, chartData.x_axis_min as Binding);
                        }
                        else if (chartData.x_axis_min is double)
                        {
                            ha.Minimum = DateTime.Now.FromUnixTimeSeconds((double)chartData.x_axis_min); ;
                        }
                    }
                    if (chartData.x_axis_max != null)
                    {
                        if (chartData.x_axis_max is Binding)
                        {
                            ha.SetBinding(DateTimeContinuousAxis.MaximumProperty, chartData.x_axis_max as Binding);
                        }
                        else if (chartData.x_axis_max is double)
                        {

                            ha.Maximum = DateTime.Now.FromUnixTimeSeconds((double)chartData.x_axis_max);
                        }
                    }
                    ha.Background = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                    ha.Foreground = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                    ha.BorderBrush = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                    ha.BorderThickness = new Thickness(1);

                    ha.LabelFormatter = chartData.x_axis_formatter;
                    rcc.HorizontalAxis = ha;
                }

                // rcc vertical axis
                {
                    /*
                            <tc:RadCartesianChart.VerticalAxis>
                                <tc:LinearAxis Minimum="{x:Bind vm.AssetChartMinimum, Mode=OneWay}" Maximum="{x:Bind vm.AssetChartMaximum, Mode=OneWay}">
                                    <tc:LinearAxis.LabelFormatter>
                                        <misc:DirectionalScadaAxisLabelFormatter/>
                                    </tc:LinearAxis.LabelFormatter>
                                </tc:LinearAxis>
                            </tc:RadCartesianChart.VerticalAxis>
                     */

                    var va = new LinearAxis();
                    {
                        if (chartData.y_axis_min != null)
                        {
                            if (chartData.y_axis_min is Binding)
                            {
                                va.SetBinding(LinearAxis.MinimumProperty, chartData.y_axis_min as Binding);
                            }
                            else if (chartData.y_axis_min is double)
                            {
                                va.Minimum = (double)chartData.y_axis_min;
                            }
                        }
                        if (chartData.y_axis_max != null)
                        {
                            if (chartData.y_axis_max is Binding)
                            {
                                va.SetBinding(LinearAxis.MaximumProperty, chartData.y_axis_max as Binding);
                            }
                            else if (chartData.y_axis_max is double)
                            {
                                va.Maximum = (double)chartData.y_axis_max;
                            }
                        }
                        if (chartData.y_axis_majorStep != null)
                        {
                            if (chartData.y_axis_majorStep is Binding)
                            {
                                va.SetBinding(LinearAxis.MajorStepProperty, chartData.y_axis_majorStep as Binding);
                            }
                            else if (chartData.y_axis_majorStep is double)
                            {
                                va.MajorStep = (double)chartData.y_axis_majorStep;
                            }
                        }
                        va.Background = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                        va.Foreground = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                        va.BorderBrush = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                        va.BorderThickness = new Thickness(1);
                    }
                    va.LabelFormatter = chartData.y_axis_formatter;
                    rcc.VerticalAxis = va;
                }

                // rcc Grid
                {
                    /*
                            <tc:RadCartesianChart.Grid>
                                <tc:CartesianChartGrid StripLinesVisibility="Y"/>
                            </tc:RadCartesianChart.Grid>                     
                     */
                    rcc.Grid = new CartesianChartGrid() { StripLinesVisibility = GridLineVisibility.Y, Background = ThemeColor("cweePageBackground"), BorderBrush = ThemeColor("cweeDarkBlue"), BorderThickness = new Thickness(0, 5, 0, 5) };

                }

                // rcc data
                foreach (var chart in chartData.charts)
                {
                    /*
                        <tc:SplineAreaSeries ItemsSource="{x:Bind vm.AssetChartData, Mode=OneWay}" 
                                             Stroke="{ThemeResource cweeStorage}"
                                             Fill="{ThemeResource cweeStorage}"
                                             tc:ChartTrackBallBehavior.TrackInfoTemplate="{StaticResource trackInfoTemplate}"                                     
                                             tc:ChartTrackBallBehavior.IntersectionTemplate="{StaticResource trackBallTemplate}" 
                                             >
                            <tc:SplineAreaSeries.CategoryBinding>
                                <tc:PropertyNameDataPointBinding PropertyName="Date"/>
                            </tc:SplineAreaSeries.CategoryBinding>
                            <tc:SplineAreaSeries.ValueBinding>
                                <tc:PropertyNameDataPointBinding PropertyName="Value"/>
                            </tc:SplineAreaSeries.ValueBinding>
                        </tc:SplineAreaSeries>                         
                     */
                    switch (chart.type)
                    {
                        case TelerikChartData.TelerikChartType.Area:
                            {
                                var spline = new SplineAreaSeries();
                                {
                                    // spline.LegendTitle
                                    // spline.IsVisibleInLegend

                                    if (chart.fillColor != null)
                                    {
                                        spline.Fill = chart.fillColor;
                                    }
                                    if (chart.strokeColor != null)
                                    {
                                        spline.Stroke = chart.strokeColor;
                                    }
                                    if (chart.strokeThickness != null)
                                    {
                                        spline.StrokeThickness = chart.strokeThickness.Value;
                                    }
                                    if (chart.spline_data != null)
                                    {
                                        if (chart.spline_data is Binding)
                                        {
                                            spline.SetBinding(SplineAreaSeries.ItemsSourceProperty, chart.spline_data as Binding);
                                        }
                                        else if (chart.spline_data is IEnumerable<ChartItem>)
                                        {
                                            spline.ItemsSource = (chart.spline_data as IEnumerable<ChartItem>);
                                        }
                                        spline.CategoryBinding = new PropertyNameDataPointBinding("Date");
                                        spline.ValueBinding = new PropertyNameDataPointBinding("Value");
                                    }
                                }
                                rcc.Series.Add(spline);
                            }
                            break;
                        case TelerikChartData.TelerikChartType.Scatter:
                            {
                                var spline = new PointSeries();
                                {
                                    // spline.LegendTitle
                                    // spline.IsVisibleInLegend

                                    //                                if (!string.IsNullOrEmpty(chart.ScatterPointTemplate) && TemplateGenerator.Create(out DataTemplate toReturn,
                                    //@"<basicDynObj:BasicDynamicObject Script="""+ chart.ScatterPointTemplate + @"""/>"
                                    //                                )) {
                                    //                                    spline.PointTemplate = toReturn;
                                    //                                }
                                    //                                else 
                                    if (TemplateGenerator.Create(out DataTemplate toReturn2,
    @"<Ellipse 
    Height=""4"" Width=""4"" StrokeThickness=""1"" Margin=""0,0,0,0"" HorizontalAlignment=""Center"" VerticalAlignment=""Center"" >
    <Ellipse.Fill> <SolidColorBrush Color=""" + chart.fillColor.ToHex() + @"""/> </Ellipse.Fill>
    <Ellipse.Stroke> <SolidColorBrush Color=""" + chart.strokeColor.ToHex() + @""" /> </Ellipse.Stroke>
</Ellipse> "
                                    ))
                                    {
                                        spline.PointTemplate = toReturn2;
                                    }


                                    if (chart.spline_data != null)
                                    {
                                        if (chart.spline_data is Binding)
                                        {
                                            spline.SetBinding(SplineAreaSeries.ItemsSourceProperty, chart.spline_data as Binding);
                                        }
                                        else if (chart.spline_data is IEnumerable<ChartItem>)
                                        {
                                            spline.ItemsSource = (chart.spline_data as IEnumerable<ChartItem>);
                                        }
                                        spline.CategoryBinding = new PropertyNameDataPointBinding("Date");
                                        spline.ValueBinding = new PropertyNameDataPointBinding("Value");
                                    }
                                }
                                rcc.Series.Add(spline);
                            }
                            break;
                        case TelerikChartData.TelerikChartType.Line:
                            {
                                var spline = new LineSeries();
                                {
                                    // spline.LegendTitle
                                    // spline.IsVisibleInLegend

                                    if (chart.strokeColor != null)
                                    {
                                        spline.Stroke = chart.strokeColor;
                                    }
                                    if (chart.strokeThickness != null)
                                    {
                                        spline.StrokeThickness = chart.strokeThickness.Value;
                                    }
                                    if (chart.spline_data != null)
                                    {
                                        if (chart.spline_data is Binding)
                                        {
                                            spline.SetBinding(SplineAreaSeries.ItemsSourceProperty, chart.spline_data as Binding);
                                        }
                                        else if (chart.spline_data is IEnumerable<ChartItem>)
                                        {
                                            spline.ItemsSource = (chart.spline_data as IEnumerable<ChartItem>);
                                        }
                                        spline.CategoryBinding = new PropertyNameDataPointBinding("Date");
                                        spline.ValueBinding = new PropertyNameDataPointBinding("Value");
                                    }
                                }
                                rcc.Series.Add(spline);
                            }
                            break;
                        case TelerikChartData.TelerikChartType.Candlestick:
                            {
                                var spline = new CandlestickSeries();
                                {
                                    // spline.LegendTitle
                                    // spline.IsVisibleInLegend

                                    if (chart.spline_data != null)
                                    {
                                        if (chart.spline_data is Binding)
                                        {
                                            spline.SetBinding(SplineAreaSeries.ItemsSourceProperty, chart.spline_data as Binding);
                                        }
                                        else if (chart.spline_data is IEnumerable<ChartItem>)
                                        {
                                            spline.ItemsSource = (chart.spline_data as IEnumerable<ChartItem>);
                                        }

                                        spline.HighBinding = new PropertyNameDataPointBinding("High");
                                        spline.LowBinding = new PropertyNameDataPointBinding("Low");
                                        spline.OpenBinding = new PropertyNameDataPointBinding("Value");
                                        spline.CloseBinding = new PropertyNameDataPointBinding("Value");
                                        spline.CategoryBinding = new PropertyNameDataPointBinding("Date");
                                    }
                                }
                                rcc.Series.Add(spline);
                            }
                            break;
                    }
                }

                outerGrid.Children.Add(rcc);

                Grid.SetColumn(rcc, 1);

                bool Even = true;
                foreach (var x in rcc.Series)
                {
                    ChartTrackBallBehavior.SetIntersectionTemplate(x, cweeXamlHelper.StaticDataTemplateResource("trackInfoTemplate"));
                    if (Even)
                    {
                        ChartTrackBallBehavior.SetTrackInfoTemplate(x, cweeXamlHelper.StaticDataTemplateResource("adjusted_trackBallTemplate"));
                    }
                    else
                    {
                        ChartTrackBallBehavior.SetTrackInfoTemplate(x, cweeXamlHelper.StaticDataTemplateResource("trackBallTemplate"));
                    }
                    Even = !Even;

                }
            }

            return outerGrid;
        }

        public class TelerikMapDetails
        {
            public string SourceUriString;
            public string DataSourceUriString;
        }
        public static Grid CreateTelerikMap(TelerikMapDetails data)
        {
            Grid outerGrid = new Grid();

            var map = new Telerik.UI.Xaml.Controls.Map.RadMap();
            map.Behaviors.Add(new Telerik.UI.Xaml.Controls.Map.MapPanAndZoomBehavior());
            map.Behaviors.Add(new Telerik.UI.Xaml.Controls.Map.MapShapePointerOverBehavior());
            map.Behaviors.Add(new Telerik.UI.Xaml.Controls.Map.MapShapeSelectionBehavior());
            map.Behaviors.Add(new Telerik.UI.Xaml.Controls.Map.MapShapeToolTipBehavior());

            var layer = new Telerik.UI.Xaml.Controls.Map.MapShapeLayer()
            {
                Source = new Telerik.UI.Xaml.Controls.Map.ShapefileDataSource() { SourceUriString = data.SourceUriString, DataSourceUriString = data.DataSourceUriString }
            };
            map.Layers.Add(layer);

            return outerGrid;
        }
#endif

        //private static void Spline_Loaded(object sender, RoutedEventArgs e)
        //{
        //    ChartTrackBallBehavior.SetIntersectionTemplate(sender as SplineAreaSeries, cweeXamlHelper.StaticDataTemplateResource("trackInfoTemplate"));
        //    ChartTrackBallBehavior.SetTrackInfoTemplate(sender as SplineAreaSeries, cweeXamlHelper.StaticDataTemplateResource("adjusted_trackBallTemplate")); 
        //}

        public static void ForceChildrenSizesToMatch(object sender, SizeChangedEventArgs e)
        {
            if (sender is Border)
            {
                Border owner = sender as Border;
                if (owner.Child is TextBlock)
                {
                    TextBlock child = owner.Child as TextBlock;

                    if (child.HorizontalAlignment == HorizontalAlignment.Center && child.VerticalAlignment == VerticalAlignment.Center)
                    {
                        child.Width = e.NewSize.Width; // default = assume user prefers vertical alignment
                    }
                    else if (child.HorizontalAlignment == HorizontalAlignment.Center)
                    {
                        child.Height = e.NewSize.Height;
                    }
                    else if (child.VerticalAlignment == VerticalAlignment.Center)
                    {
                        child.Width = e.NewSize.Width;
                    }
                    else
                    {
                        child.Width = e.NewSize.Width;
                        child.Height = e.NewSize.Height;
                    }
                }
            }
            else
            {
                throw new Exception("Please add additional condition for the UIElement of type \"" + sender.GetType().ToString() + "\"");
            }
        }

        public static void AutomaticTextSize(object sender, SizeChangedEventArgs e)
        {
            if (sender is TextBlock)
            {
                TextBlock owner = sender as TextBlock;

                if (owner.Parent != null)
                {
                    if (owner.Parent is UIElement)
                    {
                        UIElement parent = owner.Parent as UIElement;
                        if (!double.IsNaN(parent.RenderSize.Width) && !double.IsNaN(parent.RenderSize.Height))
                        {
                            owner.CalculateIdealFontSize(parent.ActualSize.X, parent.ActualSize.Y);
                            return;
                        }
                    }
                }

                if (owner.Height == e.NewSize.Height && owner.Width == e.NewSize.Width) return;
                owner.FontSize = ObjectExtensions.CalculateIdealFontSize(owner.Text, e.NewSize.Width, e.NewSize.Height);
            }
        }

        public static void AutomaticTextPlacement(object sender, SizeChangedEventArgs e)
        {
            
            if (sender is TextBlock)
            {
                TextBlock owner = sender as TextBlock;
                double prevFontSize = owner.FontSize;
                if (owner.Parent != null)
                {
                    if (owner.Parent is UIElement)
                    {
                        UIElement parent = owner.Parent as UIElement;
                        if (!double.IsNaN(parent.RenderSize.Width) && !double.IsNaN(parent.RenderSize.Height))
                        {
                            if (prevFontSize > ObjectExtensions.CalculateIdealFontSize(owner.Text, parent.ActualSize.X, parent.ActualSize.Y))
                            {
                                owner.VerticalAlignment = VerticalAlignment.Top;
                            }
                            return;
                        }
                    }
                }

                if (owner.Height == e.NewSize.Height && owner.Width == e.NewSize.Width) return;
                if (prevFontSize > ObjectExtensions.CalculateIdealFontSize(owner.Text, e.NewSize.Width, e.NewSize.Height))
                {
                    owner.VerticalAlignment = VerticalAlignment.Top;
                }
            }
        }

    }

    public class EdmsToast
    {
        public DateTime date;
        public string title = "";
        public string content = "";
        //public string image = "ms-appx:///Assets/largeLogo.png";//  Headshots/ucd-cwee.png";
        public string logo = "ms-appx:///Assets/largeLogo.png";//  Headshots/ucd-cwee.png";
        private ToastNotification thisToast = null;

        private void Create()
        {


            // Construct the visuals of the toast
            ToastVisual visual = new Microsoft.Toolkit.Uwp.Notifications.ToastVisual()
            {
                BindingGeneric = new ToastBindingGeneric()
                {
                    Children =
                        {
                            new AdaptiveText()
                            {
                                Text = title
                            },

                            new AdaptiveText()
                            {
                                Text = content
                            }

                            //new AdaptiveImage()
                            //{
                            //    Source = image
                            //}
                        },

                    AppLogoOverride = new ToastGenericAppLogo()
                    {
                        Source = logo,
                        HintCrop = ToastGenericAppLogoCrop.None
                    }
                }
            };

            ToastActionsCustom actions = new ToastActionsCustom()
            {
                Buttons =
                    {
                        new ToastButton("Accept", "accept") {
                            ActivationType = ToastActivationType.Foreground,
                            TextBoxId = "tbReply" // Reference the text box's ID in order to place this button next to the text box
                        }
                    }
            };

            ToastContent cont = new ToastContent()
            {
                Visual = visual,
                Actions = actions
            };

            thisToast = new ToastNotification(cont.GetXml());
        }

        public void Call()
        {
            if (thisToast == null) Create();

            ToastNotificationManager.CreateToastNotifier().Show(thisToast);
        }
    }

    public static class WaterWatchExtensions
    {
        public static EdmsTasks.cweeTask FireAndForget(this ScriptEngine engine, string task)
        {
            return engine.AsTask(task, true);
        }
        public static cweeTask<string> AsTask(this ScriptEngine engine, string task, bool noConversion = false)
        {
            return EdmsTasks.InsertJob(() => { return engine.DoScript("fun[](){ try{ (fun[](){\n" + task + "\n})(); " + (noConversion ? "return;" : "") + " }catch(e){ return e.pretty_print(); } }();"); }, false, true);
        }
        public static string Immediate(this ScriptEngine engine, string task, bool noConversion = false)
        {
            return engine.DoScript("fun[](){ try{ (fun[](){\n" + task + "\n})(); " + (noConversion ? "return;" : "") + " }catch(e){ return e.pretty_print(); } }();");
        }
    }

    public class RelayCommand : ICommand
    {
        readonly Action _execute = null;
        readonly Func<bool> _canExecute = null;

        public event EventHandler CanExecuteChanged;

        public RelayCommand(Action execute) : this(execute, null)
        {

        }

        public RelayCommand(Action execute, Func<bool> canExecute)
        {
            if (execute == null)
                throw new ArgumentNullException("execute");

            _execute = execute;
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter)
        {
            return true;
        }

        public void Execute(object parameter)
        {
            _execute();
        }

        public void RaiseCanExecuteChanged()
        {
            var handler = CanExecuteChanged;
            if (handler != null)
            {
                handler(this, EventArgs.Empty);
            }
        }

    }

    namespace Converters
    {
        /// <summary>
        /// Visibility to bool or back
        /// </summary>
        public class VisBoolConverter : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                if (value is null)
                {
                    return null;
                }
                if (value is bool)
                {
                    return ((bool)value) ? Visibility.Visible : Visibility.Collapsed;
                }
                if (value is Visibility)
                {
                    return (Visibility)value == Visibility.Visible ? true : false;
                }
                return null;
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language)
            {
                return Convert(value, targetType, parameter, language);
            }

        }
        public class TrueWhenListIsEmptyConverter : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                if (value is null)
                {
                    return true;
                }
                if (value is ICollection)
                {
                    return (value as ICollection).Count == 0;
                }
                return true;
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language) => null;
        }
        public class TrueWhenListIsNotEmptyConverter : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                return !((bool)(new TrueWhenListIsEmptyConverter().Convert(value, targetType, parameter, language)));
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language) => null;
        }
        public class TrueWhenZero : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                if (value is null)
                {
                    return true;
                }
                if (value is bool)
                {
                    return !(bool)(value);
                }
                if (value is Visibility)
                {
                    return (Visibility)value == Visibility.Visible ? false : true;
                }
                return true;
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language) => null;
        }
        public class TrueWhenNotZero : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                return !((bool)(new TrueWhenZero().Convert(value, targetType, parameter, language)));
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language) => null;
        }

        public class VisibleWhenListIsNotEmptyConverter : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                return (bool)(new TrueWhenListIsNotEmptyConverter().Convert(value, targetType, parameter, language)) ? Visibility.Visible : Visibility.Collapsed;
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language) => null;
        }
        public class ListToCountConverter : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                if (value is null)
                {
                    return "0";
                }
                if (value is ICollection)
                {
                    return (value as ICollection).Count.ToString();
                }
                return "0";
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language) => null;
        }

        public class DoubleMultiplyByOneHalf : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                if (value is null)
                {
                    return 0;
                }
                if (value is double)
                {
                    return 0.5 * (double)(value);
                }
                if (value is float)
                {
                    return 0.5 * (float)(value);
                }
                if (value is int)
                {
                    return 0.5 * (int)(value);
                }
                return 0;
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language) => null;
        }


        public class DoubleStringConverter : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                if (value is null)
                {
                    return null;
                }
                if (value is double)
                {
                    return ((double)value).ToString();
                }
                if (value is string)
                {
                    if (double.TryParse(value as string, out double res))
                    {
                        return res;
                    }
                    return 0.0;
                }
                return null;
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language)
            {
                return Convert(value, targetType, parameter, language);
            }

        }
        public class DateTime_Double_Converter : Windows.UI.Xaml.Data.IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, string language)
            {
                if (value is null)
                {
                    return null;
                }
                if (value is DateTime)
                {
                    return ((DateTime)value).ToUnixTimeSeconds();
                }
                if (value is double)
                {
                    return DateTime.Now.FromUnixTimeSeconds((double)value);
                }
                return null;
            }
            public object ConvertBack(object value, Type targetType, object parameter, string language)
            {
                return Convert(value, targetType, parameter, language);
            }

        }
    }

}