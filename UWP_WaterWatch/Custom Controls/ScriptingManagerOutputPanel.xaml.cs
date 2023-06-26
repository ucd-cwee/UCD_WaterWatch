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
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using UWP_WaterWatchLibrary;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    public class ScriptingManagerOutputPanel_ViewModel : ViewModelBase
    {
        public ScriptingManagerOutputPanel_ViewModel() {
            StatusString = "Scripting page was initialized.";
        }

        ~ScriptingManagerOutputPanel_ViewModel() {
            timer?.Stop();
            timer = null;
        }

        public cweeTimer timer;
        // public ScriptEngine ScriptEngine;

        private string _StatusString;
        public string StatusString { get { return _StatusString; } set { _StatusString = value; OnPropertyChanged("StatusString"); } }

        private int _NumStatusReports = 0;
        public int NumStatusReports { 
            get { return _NumStatusReports; } 
            set { 
                _NumStatusReports = value; 
                OnPropertyChanged("NumStatusReports"); 
            } 
        }

        private string _PercentMemoryUsed = "0";
        public string PercentMemoryUsed { 
            get { return _PercentMemoryUsed; } 
            set { 
                _PercentMemoryUsed = value; 
                OnPropertyChanged("PercentMemoryUsed"); 
            } 
        }

        private string _PercentCPU = "0";
        public string PercentCPU { 
            get { return _PercentCPU; } 
            set { 
                _PercentCPU = value; 
                OnPropertyChanged("PercentCPU"); 
            } 
        }
    }

    public sealed partial class ScriptingManagerOutputPanel : UserControl
    {
        public ScriptingManagerOutputPanel_ViewModel vm = new ScriptingManagerOutputPanel_ViewModel();

        public ScriptingManagerOutputPanel()
        {
            this.InitializeComponent();
            this.Loaded += (object sender, RoutedEventArgs e)=> {
                (sender as ScriptingManagerOutputPanel).vm.timer = new cweeTimer(0.1, () => {
                    (sender as ScriptingManagerOutputPanel).vm.PercentMemoryUsed = Math.Min(WaterWatch.GetPercentMemoryUsedOfMachine().roundToNearest(1), 99).ToString();
                    (sender as ScriptingManagerOutputPanel).vm.PercentCPU = Math.Min(WaterWatch.GetPercentCpuUsedOfMachine().roundToNearest(1), 99).ToString();
                }, false);
            };
            this.Unloaded += (object sender, RoutedEventArgs e) => {
                (sender as ScriptingManagerOutputPanel).vm.timer?.Stop();
                (sender as ScriptingManagerOutputPanel).vm.timer = null;
            };
        }
        ~ScriptingManagerOutputPanel() {
            vm = null;
        }

    }
}
