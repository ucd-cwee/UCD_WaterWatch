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
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using UWP_WaterWatchLibrary;
using System.Xml.Serialization;
using Windows.ApplicationModel.ExtendedExecution;
using Windows.UI.Core.Preview;
using Windows.Storage;
using Windows.UI.ViewManagement;
using Windows.ApplicationModel.Core;
using System.Timers;
using System.Threading;
using Windows.ApplicationModel.DataTransfer;
using UWP_WaterWatch.Pages;
using System.Collections.ObjectModel;
using Microsoft.UI.Xaml.Controls;

namespace UWP_WaterWatch
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        System.Timers.Timer parallel_job_manager = new System.Timers.Timer() { Interval = 1, AutoReset = true, Enabled = true };
        System.Timers.Timer parallel_toast_manager = new System.Timers.Timer() { Interval = 100, AutoReset = true, Enabled = true };
        System.Timers.Timer AppLayerRequestProcessor = new System.Timers.Timer() { Interval = 100, AutoReset = true, Enabled = true };
        public static AppData Data = new AppData();
        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            WaterWatch.SetDataDirectory(Windows.Storage.ApplicationData.Current.LocalFolder.Path + @"\data");

            AppExtension.ApplicationInfo.Initialize(System.Threading.Thread.CurrentThread.ManagedThreadId, SynchronizationContext.Current, App.Current);

            ApplicationDataContainer localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
            if (!localSettings.Values.ContainsKey("lightModeSetting")) localSettings.Values["lightModeSetting"] = true;
            if ((bool)localSettings.Values["lightModeSetting"])
            {
                this.RequestedTheme = ApplicationTheme.Light;
            }
            else
            {
                this.RequestedTheme = ApplicationTheme.Dark;
            }

            parallel_toast_manager.Elapsed += AppSupport.Parallel_toast_manager_Elapsed;
            AppLayerRequestProcessor.Elapsed += AppSupport.AppLayerRequestProcessor_Elapsed;
            parallel_job_manager.Elapsed += AppSupport.AppLayerJobManager_Elapsed;

            this.InitializeComponent();
            this.Suspending += OnSuspending;
            this.UnhandledException += EdmsException;
        }

        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used such as when the application is launched to open a specific file.
        /// </summary>
        /// <param name="e">Details about the launch request and process.</param>
        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            AppExtension.ApplicationInfo._RootFrame = Window.Current.Content;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            if (AppExtension.ApplicationInfo._RootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
#if false
                var newPage = new Frame();

                newPage.NavigationFailed += OnNavigationFailed;

                if (e.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    //TODO: Load state from previously suspended application
                }

                newPage.Navigate(typeof(ScriptingPageManager), e.Arguments);

                AppExtension.ApplicationInfo._RootFrame = newPage;
#else
                var newPage = new ScriptingPageManager();
                AppExtension.ApplicationInfo._RootFrame = newPage;
                Window.Current.Closed += (object sender2, Windows.UI.Core.CoreWindowEventArgs e2)=> { 
                    newPage.VM = null; 
                    System.GC.Collect(); 
                };
#endif
                // Place the frame in the current Window
                Window.Current.Content = AppExtension.ApplicationInfo._RootFrame;
                AppExtension.ApplicationInfo._RootFrame.PointerMoved += RootFrame_PointerMoved;

                setTitleBarVisuals();
                setForegroundSession();
            }

            if (e.PrelaunchActivated == false)
            {
                //if (AppExtension.ApplicationInfo._RootFrame.Content == null)
                //{
                //    // When the navigation stack isn't restored navigate to the first page,
                //    // configuring the new page by passing required information as a navigation
                //    // parameter
                //    AppExtension.ApplicationInfo._RootFrame.Navigate(new ScriptingPageManager());
                //}
                // Ensure the current window is active
                Window.Current.Activate();
            }
        }

        private void RootFrame_PointerMoved(object sender, PointerRoutedEventArgs e)
        {
            AppExtension.ApplicationInfo.PointerMovedArgs = e;
        }

        protected override void OnActivated(IActivatedEventArgs e)
        {
            AppExtension.ApplicationInfo._RootFrame = Window.Current.Content;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            if (AppExtension.ApplicationInfo._RootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
#if false
                var newPage = new Frame();

                newPage.NavigationFailed += OnNavigationFailed;

                if (e.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    //TODO: Load state from previously suspended application
                }

                newPage.Navigate(typeof(ScriptingPageManager), null);

                AppExtension.ApplicationInfo._RootFrame = newPage;
#else
                var newPage = new ScriptingPageManager();
                AppExtension.ApplicationInfo._RootFrame = newPage;
                Window.Current.Closed += (object sender2, Windows.UI.Core.CoreWindowEventArgs e2) => {
                    newPage.VM = null; 
                    System.GC.Collect();
                };
#endif
                // Place the frame in the current Window
                Window.Current.Content = AppExtension.ApplicationInfo._RootFrame;
                AppExtension.ApplicationInfo._RootFrame.PointerMoved += RootFrame_PointerMoved;

                setTitleBarVisuals();
                setForegroundSession();
            }
        }

        void setTitleBarVisuals()
        {
            var appView = Windows.UI.ViewManagement.ApplicationView.GetForCurrentView();
            var titleBar = appView.TitleBar;
            setTitleBarVisuals(titleBar);
        }

        public static void setTitleBarVisuals(ApplicationViewTitleBar titleBar) {
            Windows.UI.Color color1;
            Windows.UI.Color color2;
            Windows.UI.Color color3;
            Windows.UI.Color color4;

            if ((bool)Windows.Storage.ApplicationData.Current.LocalSettings.Values["lightModeSetting"] == false)
            {
                //Windows.ApplicationModel.Core.CoreApplication.GetCurrentView().TitleBar.ExtendViewIntoTitleBar = true;
                color1 = new Windows.UI.Color() { R = 33, G = 33, B = 33, A = 255 };
                color2 = Windows.UI.Colors.White;
                color3 = new Windows.UI.Color() { R = 33, G = 33, B = 33, A = 255 };
                color4 = Windows.UI.Colors.White;
            }
            else
            {
                //Windows.ApplicationModel.Core.CoreApplication.GetCurrentView().TitleBar.ExtendViewIntoTitleBar = true;
                color1 = Windows.UI.Colors.White;
                color2 = Windows.UI.Colors.Black;
                color3 = Windows.UI.Colors.White;
                color4 = Windows.UI.Colors.Black;
            }

            titleBar.BackgroundColor = color1;
            titleBar.ForegroundColor = color2;

            titleBar.ButtonBackgroundColor = color3;
            titleBar.ButtonForegroundColor = color4;

            titleBar.InactiveBackgroundColor = color1;
            titleBar.InactiveForegroundColor = color2;

            titleBar.ButtonInactiveBackgroundColor = color3;
            titleBar.ButtonInactiveForegroundColor = color4;
        }
        public static void setTitleBarVisuals(Windows.UI.WindowManagement.AppWindowTitleBar titleBar)
        {
            Windows.UI.Color color1;
            Windows.UI.Color color2;
            Windows.UI.Color color3;
            Windows.UI.Color color4;

            if ((bool)Windows.Storage.ApplicationData.Current.LocalSettings.Values["lightModeSetting"] == false)
            {
                //Windows.ApplicationModel.Core.CoreApplication.GetCurrentView().TitleBar.ExtendViewIntoTitleBar = false;
                color1 = new Windows.UI.Color() { R = 33, G = 33, B = 33, A = 255 };
                color2 = Windows.UI.Colors.White;
                color3 = new Windows.UI.Color() { R = 33, G = 33, B = 33, A = 255 };
                color4 = Windows.UI.Colors.White;
            }
            else
            {
                //Windows.ApplicationModel.Core.CoreApplication.GetCurrentView().TitleBar.ExtendViewIntoTitleBar = false;
                color1 = Windows.UI.Colors.White;
                color2 = Windows.UI.Colors.Black;
                color3 = Windows.UI.Colors.White;
                color4 = Windows.UI.Colors.Black;
            }

            titleBar.BackgroundColor = color1;
            titleBar.ForegroundColor = color2;

            titleBar.ButtonBackgroundColor = color3;
            titleBar.ButtonForegroundColor = color4;

            titleBar.InactiveBackgroundColor = color1;
            titleBar.InactiveForegroundColor = color2;

            titleBar.ButtonInactiveBackgroundColor = color3;
            titleBar.ButtonInactiveForegroundColor = color4;
        }


        ExtendedExecutionSession session2;
        ExtendedExecutionResult foregroundPermission2;
        async void setForegroundSession()
        {
            session2 = new ExtendedExecutionSession();
            session2.Reason = ExtendedExecutionReason.Unspecified;
            session2.Description = "Long Running Processing";

            foregroundPermission2 = (ExtendedExecutionResult)await session2.RequestExtensionAsync();
            switch (foregroundPermission2)
            {
                case ExtendedExecutionResult.Allowed:
                    break;

                default:
                case ExtendedExecutionResult.Denied:
                    break;
            }

            SystemNavigationManagerPreview.GetForCurrentView().CloseRequested += CloseHandle;

            ApplicationDataContainer localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
            if (!localSettings.Values.ContainsKey("fullscreenModeSetting")) localSettings.Values["fullscreenModeSetting"] = false;
            if ((bool)localSettings.Values["fullscreenModeSetting"])
            {
                var view = ApplicationView.GetForCurrentView();
                if (!view.IsFullScreenMode)
                {
                    view.TryEnterFullScreenMode();
                }
            }
            else
            {
                var view = ApplicationView.GetForCurrentView();
                if (view.IsFullScreenMode)
                {
                    view.ExitFullScreenMode();
                }
            }
        }
        private void CloseHandle(object sender, SystemNavigationCloseRequestedPreviewEventArgs e)
        {
            e.Handled = true;            

            CoreApplication.Exit();
        }

        private void EdmsException(object sender, Windows.UI.Xaml.UnhandledExceptionEventArgs e)
        {
            var exc = e.Exception;

            AppExtension.ManageException(exc);

            e.Handled = true;
        }


        /// <summary>
        /// Invoked when Navigation to a certain page fails
        /// </summary>
        /// <param name="sender">The Frame which failed navigation</param>
        /// <param name="e">Details about the navigation failure</param>
        void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private void OnSuspending(object sender, SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();

            AppExtension.ApplicationInfo.Shutdown();

            //TODO: Save application state and stop any background activity
            deferral.Complete();
        }
    }

    internal class AppSupport
    {
        public static AtomicInt AppLayerJobManager_Atomic = new AtomicInt();
        public static void AppLayerJobManager_Elapsed(object sender, System.Timers.ElapsedEventArgs e) {
            if (AppLayerJobManager_Atomic.TryIncrementTo(1))
            {
                EdmsTasks.DoJob();
                AppLayerJobManager_Atomic.Decrement();
            }


            if (EdmsTasks.DoWriteAllJobTimes == true)
            {
                EdmsTasks.DoWriteAllJobTimes = false;
                string content = "";
                Dictionary<string, double> FuncToTime = new Dictionary<string, double>();
                List<string> keys = EdmsTasks.mainThreadTimers.Keys.ToList();
                foreach (var key in keys)
                {
                    if (EdmsTasks.mainThreadTimers.TryGetValue(key, out System.Diagnostics.Stopwatch v))
                    {
                        var seconds = v.ElapsedMilliseconds / 1000.0;
                        FuncToTime[key] = seconds;
                        content = content.AddToDelimiter($"{key}\t:\t{seconds}", "\n");
                    }
                }

                WaterWatch.AddToLog(WaterWatch.GetDataDirectory() + "\\SLOWLOG.txt", content + "\n\n");
            }

        }

        public static void AppLayerRequestProcessor_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            var req = WaterWatch.TryGetAppRequest();
            if (req.first >= 0)
            {
                string result = "Function Not Handled";
                switch (req.second.first)
                {
                    case "OS_SelectFile":
                        {
                            EdmsTasks.cweeTask task = AppExtension.ApplicationInfo.MainWindowAction(async () => {
                                try
                                {
                                    var savePicker = new Windows.Storage.Pickers.FileOpenPicker();
                                    savePicker.FileTypeFilter.Add("*");
                                    Windows.Storage.StorageFile file = await savePicker.PickSingleFileAsync();

                                    try
                                    {
                                        if (file != null)
                                        {
                                            var data_folder = await Windows.Storage.StorageFolder.GetFolderFromPathAsync(WaterWatch.GetDataDirectory());
                                            if (data_folder != null)
                                            {
                                                var folder = await data_folder.CreateFolderAsync("TempFiles", CreationCollisionOption.OpenIfExists);
                                                if (folder != null)
                                                {
                                                    Windows.Storage.StorageFile file2 = await file.CopyAsync(folder, file.Name, NameCollisionOption.ReplaceExisting);
                                                    if (file2 != null)
                                                    {
                                                        // full success
                                                        result = file2.Path;
                                                    }
                                                    else
                                                    {
                                                        // found data folder's tempFile directory but failed to do the copy
                                                        throw new Exception("Failed to copy file to new temporary data directory.");
                                                    }
                                                }
                                                else
                                                {
                                                    // could not make the tempFile directory?
                                                    throw new Exception("Failed to find or create the temporary file directory.");
                                                }
                                            }
                                            else
                                            {
                                                // could not find the data directory?
                                                throw new Exception("Failed to find the data directory.");
                                            }
                                        }
                                    }
                                    catch (Exception ele) { ele.EdmsHandle(); }
                                }
                                catch (Exception E) { E.EdmsHandle(); }
                            });
                            task.ContinueWith(() => {
                                WaterWatch.CompleteAppRequest(req.first, result);
                            }, false);
                            return;
                        }
                    case "OS_SelectFolder":
                        {
                            string toReturn = "";
                            EdmsTasks.cweeTask task = AppExtension.ApplicationInfo.MainWindowAction(async () => {
                                try
                                {
                                    var savePicker = new Windows.Storage.Pickers.FolderPicker();
                                    Windows.Storage.StorageFolder file = await savePicker.PickSingleFolderAsync();

                                    try
                                    {
                                        if (file != null)
                                        {
                                            toReturn = file.Path;
                                        }
                                    }
                                    catch (Exception ele) { ele.EdmsHandle(); }
                                }
                                catch (Exception E) { E.EdmsHandle(); }
                            });
                            task.ContinueWith(() => {
                                WaterWatch.CompleteAppRequest(req.first, result);
                            }, false);
                            return;
                        }
                    case "OS_SavePassword":
                        {
                            EdmsTasks.InsertJob(() =>
                            {
                                try
                                {
                                    if (req.second.second.Count >= 3)
                                    {
                                        var vault = new Windows.Security.Credentials.PasswordVault();
                                        vault.Add(new Windows.Security.Credentials.PasswordCredential(req.second.second[0], req.second.second[1], req.second.second[2]));
                                    }
                                }
                                catch (System.Exception)
                                {

                                }
                                WaterWatch.CompleteAppRequest(req.first, "");
                            }, true, true);
                        }
                        return;
                    case "OS_LoadPassword":
                        EdmsTasks.InsertJob(() => {
                            try
                            {
                                if (req.second.second.Count >= 2)
                                {
                                    var vault = new Windows.Security.Credentials.PasswordVault();
                                    Windows.Security.Credentials.PasswordCredential Credential = vault.Retrieve(req.second.second[0], req.second.second[1]);
                                    WaterWatch.CompleteAppRequest(req.first, Credential.Password);
                                    return;
                                }
                            }
                            catch (System.Exception) {}
                            WaterWatch.CompleteAppRequest(req.first, "");
                        }, true, true);
                        return;
                    case "OS_ThemeColor":
                        EdmsTasks.InsertJob(() =>
                        {
                            try
                            {
                                if (req.second.second.Count >= 1)
                                {
                                    var b = AppExtension.ApplicationInfo.Current.Resources[req.second.second[0]] as SolidColorBrush;
                                    WaterWatch.CompleteAppRequest(req.first, $"[{b.Color.R},{b.Color.G},{b.Color.B},{b.Color.A}]");
                                    return;
                                }
                            }
                            catch (System.Exception)
                            {

                            }
                            WaterWatch.CompleteAppRequest(req.first, "[0,0,0,255]");
                        }, true, true);
                        return;
                    case "OS_GetUserName":
                        {
                            EdmsTasks.InsertJob(() =>
                            {
                                WaterWatch.CompleteAppRequest(req.first, System.Security.Principal.WindowsIdentity.GetCurrent().Name);
                            }, true, true);
                        }
                        return;
                    case "OS_GetMousePosition":
                        {
                            EdmsTasks.InsertJob(() =>
                            {
                                var pointerPosition = Windows.UI.Core.CoreWindow.GetForCurrentThread().PointerPosition;
                                var x = pointerPosition.X - Window.Current.Bounds.X;
                                var y = pointerPosition.Y - Window.Current.Bounds.Y;
                                WaterWatch.CompleteAppRequest(req.first, $"[{x},{y}]");
                            }, true, true);
                        }
                        return;
                    case "OS_SetClipboard":
                        if (req.second.second.Count >= 1) Functions.CopyToClipboard(req.second.second[0]);
                        break;
                    case "OS_GetClipboard":
                        {
                            EdmsTasks.cweeTask task = AppExtension.ApplicationInfo.MainWindowAction(async () => { 
                                try { result = await Clipboard.GetContent().GetTextAsync(); } catch (Exception) { }
                            }).ContinueWith(()=> {
                                WaterWatch.CompleteAppRequest(req.first, result);
                            }, false);
                            return;
                        }
                    case "OS_SaveSetting":
                        if (req.second.second.Count >= 2) AppExtension.ApplicationInfo.SetApplicationParam(req.second.second[0], req.second.second[1]);
                        break;
                    case "OS_GetSetting":
                        if (req.second.second.Count >= 1)
                        {
                            var str = AppExtension.ApplicationInfo.GetApplicationParam(req.second.second[0]);
                            if (str is string && !string.IsNullOrEmpty(str as string))
                            {
                                result = (str as string);
                            }
                            result = "";
                        }
                        break;
                    default:
                        result = $"Function \"{req.second.first}\" Not Found";
                        break;
                }
                WaterWatch.CompleteAppRequest(req.first, result);
            }
        }

        public static void Parallel_toast_manager_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            while (true)
            {
                var toast = WaterWatch.TryGetToast();
                if (toast.first)
                {
                    EdmsToast newToast = new EdmsToast();
                    newToast.title = toast.second.first;
                    newToast.content = toast.second.second;
                    newToast.Call();
                }
                else
                {
                    break;
                }
            }
        }
    }

    public class AppData
    {
        private cweeTimer memoryCleanUpTimer = 
            new cweeTimer(15.0, ()=> { 
                System.GC.Collect();
            }, false);

        public ObservableCollection<TabView> ScriptTabCollection = new ObservableCollection<TabView>();

        public List<TabViewItem> GetAllOpenScriptingPages() {
            List<TabViewItem> o = new List<TabViewItem>();
            lock (ScriptTabCollection)
            {
                foreach (TabView tabview in ScriptTabCollection)
                {
                    foreach (TabViewItem tab in tabview.TabItems)
                    {
                        string tabName = tab.GetTabName();
                        if (!string.IsNullOrEmpty(tabName) && tabName != "Home")
                        {
                            o.Add(tab);
                        }
                    }
                }
            }
            return o;
        }

    }
}
