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
using Microsoft.Toolkit.Uwp.UI;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    public class DroppableInputNodeViewModel : ViewModelBase
    {
        private string _VariableName = "";
        public string VariableName
        {
            get { 
                return _VariableName; 
            }
            set
            {
                if (value == _VariableName) return;

                if (ParentVM != null && ParentVM.ParentVM.engine.DoScript("__LOCK__{"+$"return {ParentVM.uniqueName}.inputs.contains(\"{value}\") ? 1 : 0;" + "};") == "1")
                {
                    VariableName = "_" + value;
                    return;
                }
                
                ParentVM.RenameInboundConnection(_VariableName, value);
                
                _VariableName = value; 
                OnPropertyChanged("VariableName");
            }
        }
        public string UniqueDroppableInputNodeID = $"Drop_{WaterWatch.RandomInt(100,10000)}_{WaterWatch.RandomInt(100, 10000)}_{WaterWatch.RandomInt(100, 10000)}";
        public string DataSourceUniqueName = "";
        public ScriptingNodeViewModel ParentVM = null; // which node this is child to... will be set and unset frequently
        public cweeEvent<DroppableInputNodeViewModel> ConnectedEvent = new cweeEvent<DroppableInputNodeViewModel>();

        ~DroppableInputNodeViewModel()
        {
            ParentVM = null;
            ConnectedEvent = null;
        }
    }

    public sealed partial class DroppableInputNode : UserControl
    {
        public Point PublicPagePosition = new Point(0, 0);
        public DroppableInputNodeViewModel vm = new DroppableInputNodeViewModel();

        public DroppableInputNode()
        {
            this.Tag = false;
            this.InitializeComponent();     
        }
        ~DroppableInputNode()
        {
            vm = null;
        }

    }
}
