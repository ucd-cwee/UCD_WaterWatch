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
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using UWP_WaterWatch.Custom_Controls;
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

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace UWP_WaterWatch.Pages
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class TextScriptingPage : Page
    {
        public static AtomicInt ObjCount = new AtomicInt();

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            EdmsTasks.InsertJob(
                () => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName))
                , /*EdmsTasks.Priority.Low, */true, true
            );
        }

        public string ScriptName { get { return VM.ScriptName; } set { VM.ScriptName = value; OnPropertyChanged("ScriptName"); } }

        public ScriptingPage_Workspace thisWorkspace = new ScriptingPage_Workspace();
        public ScriptingNodeViewModel thisNodeVM;
        public ScriptingPageViewModel VM = new ScriptingPageViewModel();
        public TextScriptingPage()
        {
            ObjCount.Increment();
            this.InitializeComponent();
            VM.outputPanel = OutputPanel;
            VM.workspace = thisWorkspace;

            thisWorkspace.vm.canvas = DrawCanvas;

            thisNodeVM = new ScriptingNodeViewModel(VM, VM.outputPanel, null, ScriptingManagerCommandBar.NewScriptNodeName());

            var editor = new ScriptNode_Editor(thisNodeVM, 18);

            var visualizer = new ScriptNode_Visualizer(thisNodeVM);

            editor.RunClickedEvent += (object sender, ScriptingNodeResult res) => {
                visualizer.vm.Reload();
            };

            EditorBorder.Children.Add(editor);
            VisualizerBorder.Children.Add(visualizer);

            this.DataContext = VM;
        }
        ~TextScriptingPage() {
            VM = null;
            ObjCount.Decrement();
        }

        public string ToJson()
        {
            var json = thisNodeVM.ToJson();
            json.HorizontalOffset = "0";
            json.VerticalOffset = "0";
            json.Width = "0";
            json.Height = "0";
            return Newtonsoft.Json.JsonConvert.SerializeObject(json);
        }



    }
}
