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

using Microsoft.UI.Xaml.Controls;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.WindowManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Hosting;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using UWP_WaterWatchLibrary;
using Windows.UI.Xaml.Media.Animation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace UWP_WaterWatch.Pages
{
    public class TabViewItemViewModel
    {
        public bool IsScriptPage;
        public bool IsTextScriptPage;
        public TabViewItemViewModel(bool IsScriptPage_p, bool IsTextScriptPage_p) {
            IsScriptPage = IsScriptPage_p;
            IsTextScriptPage = IsTextScriptPage_p;
        }
    }

    public class ScriptingPageManagerViewModelViewModel : ViewModelBase
    {
        private bool _ScriptPageIsInView = false;
        public bool ScriptPageIsInView { get { return _ScriptPageIsInView; } set { _ScriptPageIsInView = value; OnPropertyChanged("ScriptPageIsInView"); } }
        
        private bool _TextScriptPageIsInView = false;
        public bool TextScriptPageIsInView { get { return _TextScriptPageIsInView; } set { _TextScriptPageIsInView = value; OnPropertyChanged("TextScriptPageIsInView"); } }
    }

    public class ScriptingPageManagerViewModel : IDisposable
    {
        public static AtomicInt ObjCount = new AtomicInt();

        internal TabView tabView;
        public ScriptingPageManagerViewModelViewModel vm = new ScriptingPageManagerViewModelViewModel();

        public TabViewItem GetCurrentlySelectedTab()
        {
            if (tabView.SelectedItem == null || !(tabView.SelectedItem is TabViewItem))
            {
                return null;
            }
            else
            {
                return tabView.SelectedItem as TabViewItem;
            }
        }

        public ScriptingPageManagerViewModel(TabView tv) {
            ObjCount.Increment();

            tabView = tv;

            lock (App.Data.ScriptTabCollection)
            {
                App.Data.ScriptTabCollection.Add(tabView);
            }
        }
        ~ScriptingPageManagerViewModel()
        {
            ObjCount.Decrement();

            //Call Dispose from constructor
            Dispose(false);
        }

        public void Dispose()
        {
            //Call Dispose Explicitly
            Dispose(true);
            //Tell the GC not call our destructor, we already cleaned the object ourselves
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                lock (App.Data.ScriptTabCollection)
                {
                    App.Data.ScriptTabCollection.Remove(tabView);
                    tabView = null;
                    vm = null;
                }
            }
            //Clean UNMANAGED resources here
        }

    }

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class ScriptingPageManager : Page
    {
        public static AtomicInt ObjCount = new AtomicInt();

        public AutomaticDisposal<ScriptingPageManagerViewModel> VM { get; set; }

        public ScriptingPageManager()
        {
            ObjCount.Increment();

            this.InitializeComponent();
            VM = new AutomaticDisposal<ScriptingPageManagerViewModel>(new ScriptingPageManagerViewModel(ScriptTabs));

            //var coreTitleBar = Windows.ApplicationModel.Core.CoreApplication.GetCurrentView().TitleBar;
            //coreTitleBar.ExtendViewIntoTitleBar = true;
            //coreTitleBar.LayoutMetricsChanged += CoreTitleBar_LayoutMetricsChanged;
            //Window.Current.SetTitleBar(CustomDragRegion);

            var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            cdb.ContinueWith(()=> {
                TabViewItem homePage = new TabViewItem() { IsClosable = false, CanDrag = false };
                homePage.Content = new HomePage();
                homePage.IconSource = new Microsoft.UI.Xaml.Controls.SymbolIconSource() { Symbol = Windows.UI.Xaml.Controls.Symbol.Home, Foreground = cdb.Result };
                var header = Extensions.CreateTabHeader("Home");
                header.ContinueWith(()=> {
                    homePage.Header = header.Result;
                    homePage.Tag = new TabViewItemViewModel(false, false);
                    ScriptTabs.TabItems.Add(homePage);
                    ScriptTabs.SelectedItem = homePage;
                }, true);                
            }, true);

            this.PointerEntered += (object sender2, PointerRoutedEventArgs e) => { mouseOverDestination = ScriptTabs; };
            this.PointerExited += (object sender2, PointerRoutedEventArgs e) => { mouseOverDestination = null; };
        }
        public ScriptingPageManager(TabViewItem initialTab)
        {
            ObjCount.Increment();
            this.InitializeComponent();
            VM = new AutomaticDisposal<ScriptingPageManagerViewModel>(new ScriptingPageManagerViewModel(ScriptTabs));

            ScriptTabs.TabItems.Add(initialTab);

            this.PointerEntered += (object sender2, PointerRoutedEventArgs e) => { mouseOverDestination = ScriptTabs; };
            this.PointerExited += (object sender2, PointerRoutedEventArgs e) => { mouseOverDestination = null; };
        }
        ~ScriptingPageManager()
        {
            ObjCount.Decrement();
        }
        private async void TabView_TabDroppedOutside(Microsoft.UI.Xaml.Controls.TabView sender, Microsoft.UI.Xaml.Controls.TabViewTabDroppedOutsideEventArgs e)
        {
            if (being_dragged != null)
            {                
                // Create a new AppWindow
                AppWindow newWindow = await AppWindow.TryCreateAsync();

                // Create the content for the new window
                if (e.Tab != null)
                {
                    // Remove tab from existing list
                    ScriptTabs.TabItems.Remove(e.Tab);

                    var newPage = new ScriptingPageManager(e.Tab);

                    // Set the Window's content to the new page
                    ElementCompositionPreview.SetAppWindowContent(newWindow, newPage);
                    App.setTitleBarVisuals(newWindow.TitleBar);
                    newWindow.Closed += (AppWindow sender2, AppWindowClosedEventArgs args) => { newPage.VM = null; System.GC.Collect(); };

                    // newWindow.Title = e.Tab.GetTabName();

                    // Show the window
                    await newWindow.TryShowAsync();
                }                
            }

            being_dragged = null;
            possible_destination = null;
            drag_sender = null;
        }

        private void TabView_AddTabButtonClick(Microsoft.UI.Xaml.Controls.TabView sender = null, object args = null)
        {
            var scriptingPage = new ScriptingPage();
            scriptingPage.VM.ScriptName = "New Script";
            var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            cdb.ContinueWith(() =>
            {
                var tab = new TabViewItem()
                {
                    Content = scriptingPage,
                    IconSource = new Microsoft.UI.Xaml.Controls.SymbolIconSource() { Symbol = Windows.UI.Xaml.Controls.Symbol.Document, Foreground = cdb.Result }
                };
                tab.CloseRequested += (TabViewItem sender2, TabViewTabCloseRequestedEventArgs args2)=> {
                    sender2.Header = null;
                    sender2.Tag = null;
                };
                var header = Extensions.CreateEditableTabHeader(new Binding() { Source = scriptingPage.VM, Path = new PropertyPath("ScriptName"), Mode = BindingMode.TwoWay, UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged }); //  scriptingPage.VM);
                header.ContinueWith(()=> {
                    tab.Header = header.Result;
                    tab.Tag = new TabViewItemViewModel(true, false);
                    tab.PointerEntered += (object sender2, PointerRoutedEventArgs e) => { tab.IsClosable = true; };
                    tab.PointerExited += (object sender2, PointerRoutedEventArgs e) => { tab.IsClosable = false; };
                    ScriptTabs.TabItems.Add(tab);
                    ScriptTabs.SelectedItem = tab;
                }, true);
            }, true);
        }

        private void CheckSelectedPageIsScript(SelectionChangedEventArgs e)
        {
            foreach (TabViewItem obj in e.AddedItems)
            {
                if (obj != null)
                {
                    var vm = (obj.Tag as TabViewItemViewModel);
                    if (vm != null)
                    {
                        if (vm.IsScriptPage)
                        {
                            VM.obj.vm.TextScriptPageIsInView = false;
                            VM.obj.vm.ScriptPageIsInView = true;
                            return;
                        }
                        else if (vm.IsTextScriptPage)
                        {
                            VM.obj.vm.ScriptPageIsInView = false;
                            VM.obj.vm.TextScriptPageIsInView = true;
                            return;
                        }
                    }
                }
            }
            VM.obj.vm.ScriptPageIsInView = false;
            VM.obj.vm.TextScriptPageIsInView = false;
        }
        private void TabView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            CheckSelectedPageIsScript(e);            
        }
        private void TabView_TabCloseRequested(Microsoft.UI.Xaml.Controls.TabView sender, Microsoft.UI.Xaml.Controls.TabViewTabCloseRequestedEventArgs args)
        {
            args.Tab.Content = null;
            ScriptTabs.TabItems.Remove(args.Tab);
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

        public static TabViewItem being_dragged;
        public static TabView possible_destination;
        public static TabView drag_sender;
        public static TabView mouseOverDestination;
        private void ScriptTabs_TabStripDragOver(object senderO, DragEventArgs e)
        {
            if (senderO is TabView)
            {
                TabView sender = senderO as TabView;
                possible_destination = sender;
            }
        }
        private void ScriptTabs_TabDragStarting(TabView sender, TabViewTabDragStartingEventArgs args)
        {
            possible_destination = null;
            drag_sender = sender;
            being_dragged = args.Tab;            
        }
        private void ScriptTabs_DragOver(object senderO, DragEventArgs e)
        {
            if (senderO is TabView)
            {
                TabView sender = senderO as TabView;
                possible_destination = sender;
            }
        }
        private void ScriptTabs_TabDragCompleted(TabView sender, TabViewTabDragCompletedEventArgs args)
        {
            if (being_dragged != null)
            {
                if (mouseOverDestination == possible_destination && possible_destination != null)
                {
                    // no argument
                    drag_sender.TabItems.Remove(being_dragged);
                    mouseOverDestination.TabItems.Add(being_dragged);
                    mouseOverDestination.SelectedItem = being_dragged;

                    being_dragged = null;
                    possible_destination = null;
                    drag_sender = null;
                } 
                else if (mouseOverDestination == drag_sender && drag_sender != null) {
                    // no argument -- do nothing!

                    being_dragged = null;
                    possible_destination = null;
                    drag_sender = null;
                }
                else if (possible_destination != null && possible_destination != drag_sender)
                {
                    drag_sender.TabItems.Remove(being_dragged);
                    possible_destination.TabItems.Add(being_dragged);
                    possible_destination.SelectedItem = being_dragged;

                    being_dragged = null;
                    possible_destination = null;
                    drag_sender = null;
                }
            }   
        }
    }
}
