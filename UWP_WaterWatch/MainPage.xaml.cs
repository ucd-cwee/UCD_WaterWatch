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
using Windows.UI.WindowManagement;
using Microsoft.UI.Xaml.Controls;
using Windows.UI.Xaml.Hosting;
// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace UWP_WaterWatch
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public class MainPageViewModel {

            public bool IsDeveloperToolOpen { get; set; } = false;

            public string THINGY { get; set; } = "DEBUG";
        }

        public static MainPageViewModel VM;

        public MainPage()
        {
            this.InitializeComponent();

            var coreTitleBar = Windows.ApplicationModel.Core.CoreApplication.GetCurrentView().TitleBar;
            coreTitleBar.ExtendViewIntoTitleBar = true;
            coreTitleBar.LayoutMetricsChanged += CoreTitleBar_LayoutMetricsChanged; ;
            Window.Current.SetTitleBar(CustomDragRegion);

            VM = new MainPageViewModel();

            VM.THINGY = WaterWatch.GetDataDirectory();

            ScriptEngine engine = new ScriptEngine();
            var task = engine.AsTask("Async(fun[](){ return \"HELLO FROM DYNAMIC, REALTIME SCRIPTING WITH C++ STYLED SYNTAX. WHERE OPTIMIZED, MANY FUNCTIONS HAVE C++ PERFORMANCE BUILT-IN.\"; }).await();");

            task.ContinueWith(()=> {
                string thing = task.Result + " \n\n DATA DIRECTORY: " + WaterWatch.GetDataDirectory() + "\n\n NUM COMPUTER CORES: " + WaterWatch.GetNumMultithreadingCores().ToString();
                VM.THINGY = thing;                
            }, true);

            while (!task.IsFinished) { }

            this.DataContext = VM;
        }

        private void CoreTitleBar_LayoutMetricsChanged(Windows.ApplicationModel.Core.CoreApplicationViewTitleBar sender, object args)
        {
            if (FlowDirection == FlowDirection.LeftToRight)
            {
                CustomDragRegion.MinWidth = sender.SystemOverlayRightInset;
                ShellTitlebarInset.MinWidth = sender.SystemOverlayLeftInset;
            }
            else
            {
                CustomDragRegion.MinWidth = sender.SystemOverlayLeftInset;
                ShellTitlebarInset.MinWidth = sender.SystemOverlayRightInset;
            }
            CustomDragRegion.Height = ShellTitlebarInset.Height = sender.Height;
        }

        private void F12ButtonClicked(object sender, RoutedEventArgs e)
        {
            VM.IsDeveloperToolOpen = !VM.IsDeveloperToolOpen;
        }

        private void F5ButtonClicked(object sender, RoutedEventArgs e)
        {
            // should do the reload of the page from the script engine ... 




        }

        private void SaveProject_Keyboard(object sender, RoutedEventArgs e)
        {
            TabViewItem item = ScriptTabs.SelectedItem as TabViewItem;
            if (item != null)
            {
                


            }
        }

        private void QuitProject_Keyboard(object sender, RoutedEventArgs e)
        {

        }

        private void CloseWindow_Keyboard(object sender, RoutedEventArgs e)
        {

        }

        private void PrintScreen_Keyboard(object sender, RoutedEventArgs e)
        {

        }

        private void ContextHelp_Keyboard(object sender, RoutedEventArgs e)
        {

        }

        private void ScriptingEditActivate_Keyboard(object sender, RoutedEventArgs e)
        {

        }

        private async void TabView_TabDroppedOutside(Microsoft.UI.Xaml.Controls.TabView sender, Microsoft.UI.Xaml.Controls.TabViewTabDroppedOutsideEventArgs e)
        {
            // Create a new AppWindow
            AppWindow newWindow = await AppWindow.TryCreateAsync();

            // Create the content for the new window
            var newPage = new MainPage();

            // Remove tab from existing list
            ScriptTabs.TabItems.Remove(e.Tab);

            // Add tab to list of Tabs on new page
            //newPage.AddItemToTabs(e.Tab);

            // Set the Window's content to the new page
            ElementCompositionPreview.SetAppWindowContent(newWindow, newPage);

            // Show the window
            await newWindow.TryShowAsync();
        }

        private void TabView_AddTabButtonClick(Microsoft.UI.Xaml.Controls.TabView sender, object args)
        {
            ScriptTabs.TabItems.Add(new TabViewItem() { Content = "HELLO I AM NEW CONTENT" });
        }

        private void TabView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

        }

        private void TabView_TabCloseRequested(Microsoft.UI.Xaml.Controls.TabView sender, Microsoft.UI.Xaml.Controls.TabViewTabCloseRequestedEventArgs args)
        {
            ScriptTabs.TabItems.Remove(args.Tab);
        }


    }
}
