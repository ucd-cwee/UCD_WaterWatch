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
using Newtonsoft.Json;
using static UWP_WaterWatchLibrary.ObjectExtensions;
using Microsoft.Toolkit.Uwp.UI;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    public class ScriptingNodeResult
    {
        public static AtomicInt ObjCount = new AtomicInt();
        public ScriptingNodeResult(ScriptingNodeViewModel VM, string error) { 
            ObjCount.Increment(); 
            vm = VM; 
            err = error;
            BasicCall = $"{vm.uniqueName}.result()";
        }
        public ScriptingNodeResult(ScriptingNodeViewModel VM, string error, string script)
        {
            ObjCount.Increment();
            vm = VM;
            err = error;
            BasicCall = script;
        }
        ~ScriptingNodeResult() {
            ObjCount.Decrement();
            vm = null;
        }
        private ScriptingNodeViewModel vm;
        private string err;
        private string BasicCall;

        public cweeTask<string> Query_String(string additional_params = "") {
            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.Cast_String($"{BasicCall}{additional_params};"); }, false);
        }
        public cweeTask<Color_Interop> Query_Color(string additional_params = "") {
            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.Cast_Color($"{BasicCall}{additional_params};"); }, false);
        }
        public cweeTask<MapIcon_Interop> Query_MapIcon(string additional_params = "") {
            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.Cast_MapIcon($"{BasicCall}{additional_params};"); }, false);
        }
        public cweeTask<MapPolyline_Interop> Query_MapPolyline(string additional_params = "") {
            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.Cast_MapPolyline($"{BasicCall}{additional_params};"); }, false);
        }
        public cweeTask<MapBackground_Interop> Query_MapBackground(string additional_params = "")
        {
            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.Cast_MapBackground($"{BasicCall}{additional_params};"); }, false);
        }
        public cweeTask<MapLayer_Interop> Query_MapLayer(string additional_params = "") {
            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.Cast_MapLayer($"{BasicCall}{additional_params};"); }, false);
        }
        public cweeTask<string> QueryResult(string additional_params = "")
        { // i.e. ".size()" or "[0]"
            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.DoScript($"{BasicCall}{additional_params};"); }, false);
        }
        public cweeTask<string> CustomizableQueryResult(string script, string replaceWithVariable, string additionalParams = "")
        {
            script = script.Replace(replaceWithVariable, $"{BasicCall}" + additionalParams);

            return EdmsTasks.InsertJob(() => { return vm.ParentVM?.engine?.DoScript(script); }, false, true);
        }
        public cweeTask<List<string>> CustomizableQueryResult_Cast_VectorString(string script, string replaceWithVariable, string additionalParams = "")
        {
            script = script.Replace(replaceWithVariable, $"{BasicCall}" + additionalParams);

            return EdmsTasks.InsertJob(() => { return (List<string>)vm.ParentVM?.engine?.DoScript_Cast_VectorStrings(script); }, false, true);
        }
        public cweeTask<List<float>> CustomizableQueryResult_Cast_VectorFloat(string script, string replaceWithVariable, string additionalParams = "")
        {
            script = script.Replace(replaceWithVariable, $"{BasicCall}" + additionalParams);

            return EdmsTasks.InsertJob(() => { return (List<float>)vm.ParentVM?.engine?.DoScript_Cast_VectorFloats(script); }, false, true);
        }

        public (ScriptEngine, string) AccessDirectly() {
            return (vm.ParentVM?.engine, $"{BasicCall}");
        }
        public string Error => err;
    }

    public class ScriptNodePathVM : ViewModelBase
    {
        public static AtomicInt ObjCount = new AtomicInt();
        public string Source;
        public string Destination;
        private string _Name = ""; public string Name
        {
            get { return _Name; }
            set
            {
                NameChangedEvent.InvokeEvent(this, (_Name, value)); // Async
                _Name = value;
                OnPropertyChanged("Name");
            }
        }
        public cweeEvent<(string, string)> NameChangedEvent = new cweeEvent<(string, string)>();

        public ScriptNodePathVM() { ObjCount.Increment(); }
        ~ScriptNodePathVM() {
            NameChangedEvent = null;
            ObjCount.Decrement();
        }
    }

    public class ScriptingNodeViewModel : ViewModelBase
    {
        public static AtomicInt ObjCount = new AtomicInt();
        public enum ScriptingNodeMode
        {
            Editing, 
            Visualizing
        }

        public string Script
        {
            get
            {
                return _Script.Get();
            }
            set
            {
                var x = value.Replace("\r", "\n");

                if (x == _Script.Get()) return;

                _Script.Set(x);
                string todo = "__LOCK__{ %s.script = \"{ \n${ external_data.GetString(%i) }\n }\"; }".Replace("%s", uniqueName).Replace("%i", _Script.Index().ToString());
                EdmsTasks.InsertJob(()=> {
                    ParentVM?.engine?.DoScript(todo);
                }, false, true);
                
                OnPropertyChanged("Script");
            }
        } public SharedString _Script = new SharedString();
        public string Label { get { 
                return ParentVM?.engine?.DoScript($"{uniqueName}.label;"); 
            } set {
                ParentVM?.engine?.DoScript($"{uniqueName}.label = \"{value}\";"); 
                OnPropertyChanged("Label");
            } 
        }
        public string uniqueName = "";
        public Pages.ScriptingPageViewModel ParentVM;
        public ScriptingManagerOutputPanel outputPanel;
        public List<Windows.UI.Xaml.Shapes.Path> availabe_paths;
        public ScriptingNodeMode mode = ScriptingNodeMode.Editing;
        public cweeEvent<ScriptNode> DeleteSelfEvent = new cweeEvent<ScriptNode>();

        public List<ScriptNodePathVM> inbound_connections = new List<ScriptNodePathVM>();
        
        public ScriptNodePathVM AddInboundConnection(ScriptingNodeViewModel IncomingNode, string initial_name)
        {
            var n = new ScriptNodePathVM() { Source = IncomingNode.uniqueName, Name = initial_name, Destination = uniqueName };

            ParentVM?.engine?.DoScript("try{ __LOCK__{" + $"{n.Destination}.inputs.erase(\"{n.Name}\");" + "} }; try{ __LOCK__{" + $"{n.Destination}.inputs[\"{n.Name}\"] := {n.Source};" + "} }; try{" + $"{n.Destination}.invalidate();" + "}");

            n.NameChangedEvent += (object r, (string, string) s) => {
                ScriptNodePathVM vv = r as ScriptNodePathVM;
                ParentVM?.engine?.DoScript(
                    "try{ __LOCK__{" + $"{vv.Destination}.inputs.erase(\"{s.Item1}\");" + "} };"
                    +
                    " try{ __LOCK__{" + $"{vv.Destination}.inputs.erase(\"{s.Item2}\");" + "} };"
                    +
                    " try{ __LOCK__{" + $"{vv.Destination}.inputs[\"{s.Item2}\"] := {vv.Source};" + "} };"
                    +
                    " try{" + $"{vv.Destination}.invalidate();" + "}"
                );
            };

            inbound_connections.Add(n);

            return n;
        }
        public void RenameInboundConnection(string old_name, string new_name)
        {
            bool rename_once = false;
            inbound_connections.FindAll((ScriptNodePathVM v) => {
                if (!rename_once)
                {
                    bool toR = false;

                    if (v.Name == old_name)
                    {
                        v.Name = new_name;
                        rename_once = true;
                    }

                    return toR;
                }
                return false;
            });
        }
        public void RemoveInboundConnection(string old_name)
        {
            inbound_connections.RemoveAll((ScriptNodePathVM n) => {
                if (n.Name == old_name)
                {
                    // do the removal
                    ParentVM?.engine?.DoScript("try{ __LOCK__{" + $"{n.Destination}.inputs.erase(\"{n.Name}\");" + "} }; try{" + $"{n.Destination}.invalidate();" + "}");

                    // what about the "available_paths" for the source node? They need to be sanitized and hidden where appropriate
                    // Funny thing... the available paths are likely in the same parent window as we are. 
                    if (availabe_paths != null)
                    {
                        foreach (var destination_path in availabe_paths)
                        { // Get the parent window of OUR paths...
                            FrameworkElement shared_parent = destination_path.Parent as FrameworkElement;
                            if (shared_parent != null)
                            {
                                foreach (ScriptNode child in shared_parent.FindChildren().Where((FrameworkElement child) => { if (child is ScriptNode) return true; else return false; }))
                                {
                                    if (child.vm.uniqueName == n.Source)
                                    {
                                        // found it, remove the connected path;
                                        foreach (Windows.UI.Xaml.Shapes.Path p in child.vm.availabe_paths)
                                        {
                                            if (p != null)
                                            {
                                                if (p?.Visibility == Visibility.Visible && p?.Tag != null)
                                                {
                                                    var tagObj = ((int, string, string))(p?.Tag);
                                                    DroppableInputNode droppable = SplineRedrawToConnectNodeLogic.FindDropNode(p, tagObj.Item2, tagObj.Item3);
                                                    if (droppable?.vm?.ParentVM?.uniqueName == n.Destination && droppable?.vm?.VariableName == n.Name)
                                                    {
                                                        droppable.vm.DataSourceUniqueName = "";
                                                        string prevName = droppable.vm.VariableName;
                                                        droppable.vm.VariableName = "";
                                                        droppable.vm.VariableName = prevName;

                                                        p.Visibility = Visibility.Collapsed;

                                                        var tagC = p.Tag;
                                                        if (tagC != null)
                                                        {
                                                            var data = ((int, string, string))tagC;
                                                            SplineRedrawToConnectNodeLogic.pathAdjusterForAll?.RemoveAction(data.Item1);
                                                            p.Tag = (-1, data.Item2, data.Item3);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                    return true;
                }
                else return false;
            });
        }
        public void RemoveOutboundConnections()
        {
            List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();
            if (availabe_paths != null)
            {
                foreach (var p in availabe_paths)
                {
                    if (p != null)
                    {
                        if (p?.Visibility == Visibility.Visible && p?.Tag != null)
                        {
                            var tagObj = ((int, string, string))(p?.Tag);
                            DroppableInputNode droppable = SplineRedrawToConnectNodeLogic.FindDropNode(p, tagObj.Item2, tagObj.Item3);
                            droppable.vm.ParentVM.RemoveInboundConnection(droppable.vm.VariableName);
                            droppable.vm.DataSourceUniqueName = "";
                            string prevName = droppable.vm.VariableName;
                            droppable.vm.VariableName = "";
                            droppable.vm.VariableName = prevName;

                            p.Visibility = Visibility.Collapsed;

                            var tagC = p.Tag;
                            if (tagC != null)
                            {
                                var data = ((int, string, string))tagC;
                                SplineRedrawToConnectNodeLogic.pathAdjusterForAll?.RemoveAction(data.Item1);
                                p.Tag = (-1, data.Item2, data.Item3);
                            }
                        }
                    }
                }
            }
        }
        public void RemoveInboundConnections()
        {
            List<string> keys = new List<string>();
            foreach (var o in inbound_connections) { keys.Add(o.Name); }
            foreach (var name in keys)
            {
                RemoveInboundConnection(name);
            }
        }
        public void DisconnectFromAll()
        {
            RemoveInboundConnections();
            RemoveOutboundConnections();
        }

        public cweeTask<ScriptingNodeResult> RunAsync()
        {
            return EdmsTasks.InsertJob(() => {
                string reply = ParentVM?.engine?.DoScript($"{uniqueName}.result(); return;");
                if (reply == "") reply = null;
                return new ScriptingNodeResult(this, reply);
            }, false, true);
        }

        public ScriptingNodeViewModel(Pages.ScriptingPageViewModel e, ScriptingManagerOutputPanel panel, List<Windows.UI.Xaml.Shapes.Path> paths, string UniqueName)
        {
            ObjCount.Increment();
            ParentVM = e;

            availabe_paths = paths;
            outputPanel = panel;
            uniqueName = UniqueName;

            var err = "DEFAULT";
            while (!string.IsNullOrEmpty(err))
            {
                err = ParentVM?.engine?.DoScript("try{ " + $"delete({uniqueName});" + " }" + $" global& {uniqueName} := ScriptNode(); {uniqueName}.uniqueID = \"{uniqueName}\"; {uniqueName}.label = \"\"; {uniqueName}.script = \"\"; {uniqueName}.inputs = Map(); {uniqueName}.version = 0; return;");
            }
        }

        public ScriptingNodeViewModel(Pages.ScriptingPage e, ScriptingManagerOutputPanel panel, List<Windows.UI.Xaml.Shapes.Path> paths, string UniqueName)
        {
            ObjCount.Increment();
            ParentVM = e?.VM;

            availabe_paths = paths;
            outputPanel = panel;
            uniqueName = UniqueName; 

            var err = "DEFAULT";
            while (!string.IsNullOrEmpty(err))
            {
                err = ParentVM?.engine?.DoScript("try{ " + $"delete({uniqueName});" + " }" + $" global& {uniqueName} := ScriptNode(); {uniqueName}.uniqueID = \"{uniqueName}\"; {uniqueName}.label = \"\"; {uniqueName}.script = \"\"; {uniqueName}.inputs = Map(); {uniqueName}.version = 0; return;");
            }
        }
        ~ScriptingNodeViewModel() {
            ObjCount.Decrement();

            ParentVM?.engine?.DoScript($"delete({uniqueName});");
            
            availabe_paths = null;
            outputPanel = null;
            inbound_connections = null;
            DeleteSelfEvent = null;
        }

        public class JSON_Serializer
        {
            public string script = "";
            public string label = "";
            public string uniqueName = "";
            public string mode = "";
            public string HorizontalOffset = "";
            public string VerticalOffset = "";
            public string Width = "";
            public string Height = "";
            public Dictionary<string, string> inbounds = new Dictionary<string, string>();
        }
        public JSON_Serializer ToJson()
        {
            var tosave = new JSON_Serializer();
            tosave.script = Script;
            tosave.label = Label;
            tosave.uniqueName = uniqueName;
            tosave.mode = mode.ToString();
            foreach (var x in inbound_connections)
            {
                tosave.inbounds[x.Name] = x.Source;
            }
            return tosave;
        }

        /// <summary>
        /// LOAD EVERYTHING EXCEPT THE INBOUND CONNECTIONS
        /// </summary>
        /// <param name="v"></param>
        public void FromJson_p1(JSON_Serializer toload)
        {
            Label = toload.label;
            Script = toload.script;
            mode = FindBestMatch<ScriptingNodeMode>(toload.mode);
        }

        public cweeEvent<bool> PositionChanged = new cweeEvent<bool>();
        public cweeEvent<bool> SizeChanged = new cweeEvent<bool>();
    }

    public sealed partial class ScriptNode : UserControl
    {
        public static AtomicInt ObjCount = new AtomicInt();
        public ScriptingNodeViewModel vm;
        public UWP_WaterWatchLibrary.Floating.FloatingContent floatingControl;
        public UWP_WaterWatchLibrary.Floating.StretchableContent stretchingControl;
        public StackPanel DroppableInputNodeList;
        public DraggableOutputNode dragNode;

        public ScriptNode() { ObjCount.Increment(); this.InitializeComponent(); }
        public ScriptNode(ScriptingManagerOutputPanel panel, List<Windows.UI.Xaml.Shapes.Path> paths, Pages.ScriptingPage parentPage, string UniqueName)
        {
            ObjCount.Increment();
            vm = new ScriptingNodeViewModel(parentPage, panel, paths, UniqueName);

            this.InitializeComponent();

            this.floatingControl = FloatingControl;
            floatingControl.ManipulationDeltaWWEvent += (object sender, UWP_WaterWatchLibrary.Floating.FloatingContent.ManipulationDeltaArgs args) => {
                // var floater = (sender as UWP_WaterWatchLibrary.Floating.FloatingContent);
                vm.PositionChanged.InvokeEventAsync(this, false);
            };
            this.stretchingControl = StretchableControl;
            stretchingControl.StretchedEvent += (object sender /*null*/, bool arg /*unused*/) => {
                vm.PositionChanged.InvokeEventAsync(this, false);
            };

            this.DroppableInputNodeList = InputNodeList;
            this.dragNode = OutputNode;

            vm.SizeChanged += (object sender, bool v) => {
                vm.PositionChanged.InvokeEventAsync(this, false);
            };

            vm.PositionChanged += (object sender, bool arg /*unused*/) => {
                var node = (sender as ScriptNode);
                {
                    var job1 = EdmsTasks.InsertJob(() =>
                    {
                        /*var newP = */
                        return node.dragNode.TransformToVisual(node.Parent as FrameworkElement).TransformPoint(new Point(9, 5.5));
                    }, true);
                    job1.ContinueWith(() =>
                    {
                        node.dragNode.PublicPagePosition = job1.Result;
                    }, false);
                }
                EdmsTasks.InsertJob(() =>
                {
                    foreach (var dropperItr in DroppableInputNodeList.Children)
                    {
                        if (dropperItr is DroppableInputNode)
                        {
                            var dropper = dropperItr as DroppableInputNode;
                            var job1 = EdmsTasks.InsertJob(() =>
                            {
                                /*var newP = */
                                return dropper.TransformToVisual(node.Parent as FrameworkElement).TransformPoint(new Point(1.5, 9.5));
                            }, true);
                            job1.ContinueWith(() =>
                            {
                                dropper.PublicPagePosition = job1.Result;
                            }, false);
                        }
                    }
                }, true);
            };

            vm.PositionChanged.InvokeEventAsync(this, false);

            if (vm.mode == ScriptingNodeViewModel.ScriptingNodeMode.Editing)
            {
                EditScript(null, null);
            }
            else
            {
                VisualizeResult(null, null);
            }
        }

        ~ScriptNode(){
            ObjCount.Decrement();
            vm = null;
            floatingControl = null;
            stretchingControl = null;
            DroppableInputNodeList = null;
        }

        public string ToJson()
        {
            var json = vm.ToJson();
            json.HorizontalOffset = floatingControl.CurrentPositionLeft();
            json.VerticalOffset = floatingControl.CurrentPositionTop();
            if (stretchingControl.Width == stretchingControl.ActualWidth)
            {
                json.Width = "0";
            }
            else
            {
                json.Width = $"{stretchingControl.Width}";
            }
            if (stretchingControl.Height == stretchingControl.ActualHeight)
            {
                json.Height = "0";
            }
            else
            {
                json.Height = $"{stretchingControl.Height}";
            }

            return JsonConvert.SerializeObject(json);
        }
        public void FromJson_p1(string v)
        {
            var toload = JsonConvert.DeserializeObject<ScriptingNodeViewModel.JSON_Serializer>(v);
            floatingControl.InitialPositionLeft = toload.HorizontalOffset;
            floatingControl.InitialPositionTop = toload.VerticalOffset;
            floatingControl.SetPosition(); // if it isn't loaded yet, it'll figure it out later then. Otherwise it'll move now

            if (double.TryParse(toload.Width, out double w) && w > 0)
            {
                stretchingControl.Width = w;
            }
            if (double.TryParse(toload.Height, out double h) && h > 0)
            {
                stretchingControl.Height = h;
            }
            vm.FromJson_p1(toload);
        }
        public void FromJson_p2(string v, List<ScriptingNodeViewModel> vms, List<ScriptNode> nodes)
        {
            var toload = JsonConvert.DeserializeObject<ScriptingNodeViewModel.JSON_Serializer>(v);

            foreach (var connection in toload.inbounds) // these ARE the number of required connections.
            {
                ScriptNode found_node = nodes.Find((ScriptNode V) => { return V.vm.uniqueName == connection.Value; });

                // get the last input node
                var dropNode = (InputNodeList.Children[InputNodeList.Children.Count - 1] as DroppableInputNode);

                Windows.UI.Xaml.Shapes.Path path = null;
                if (found_node.vm.availabe_paths != null)
                {
                    path = found_node.vm.availabe_paths.Find((Windows.UI.Xaml.Shapes.Path p) => { if (p.Visibility == Visibility.Collapsed) { p.Visibility = Visibility.Visible; return true; } return false; });
                }
                dropNode.vm.ParentVM = vm; // ensure these guys are ready
                found_node.OutputNode.vm.ParentVM = found_node.vm; // ensure these guys are ready
                found_node.OutputNode.vm.available_paths = found_node.vm.availabe_paths;

#if true
                dropNode.vm.VariableName = connection.Key;
                DraggableOutputNode.Make_Connection_Immediately(found_node.OutputNode, dropNode, path);

                DraggableOutputNode n = found_node.OutputNode as DraggableOutputNode;
                {
                    // if all of the incoming nodes are connected, add a new one!
                    bool anyEmpty = false;
                    foreach (DroppableInputNode x in InputNodeList.Children)
                    {
                        if (x != null)
                        {
                            if (string.IsNullOrEmpty(x.vm.DataSourceUniqueName)) //VariableName))
                            {
                                anyEmpty = true;
                            }
                        }
                    }

                    if (!anyEmpty)
                    {
                        var droppable = new DroppableInputNode() { HorizontalAlignment = HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Center };
                        droppable.Loaded += InputNodeLoaded;
                        droppable.Unloaded += InputNodeUnloaded;
                        InputNodeList.Children.Add(droppable);
                        vm.PositionChanged.InvokeEventAsync(this, false);
                    }
                }


#endif
            }

            if (this.IsLoaded)
            {
                switch (vm.mode)
                {
                    case ScriptingNodeViewModel.ScriptingNodeMode.Editing:
                        EditScript(null, null);
                        break;
                    case ScriptingNodeViewModel.ScriptingNodeMode.Visualizing:
                        VisualizeResult(null, null);
                        break;
                }
            }
            else
            {
                this.Loaded += SetVisualFormat;
            }
        }

        private void SetVisualFormat(object sender, RoutedEventArgs e)
        {
            switch (vm.mode)
            {
                case ScriptingNodeViewModel.ScriptingNodeMode.Editing:
                    EditScript(null, null);
                    break;
                case ScriptingNodeViewModel.ScriptingNodeMode.Visualizing:
                    EdmsTasks.InsertJob(()=> {
                        VisualizeResult(null, null);
                    }, true, true);
                    break;
            }
            this.Loaded -= SetVisualFormat;
        }

        public /*EdmsTasks.cweeTask*/ void DisconnectFromAll()
        {
            /*return */ vm.DisconnectFromAll(); //.ContinueWith(()=> { // each of the droppable input nodes need help
                //foreach (var child in DroppableInputNodeList.Children.ToList()) {
                //    DroppableInputNodeList.Children.Remove(child);
                //}
            //}, true);
        }

        ObjectExtensions.cweeFlyout GetLabelFlyout(UIElement sender, bool show = false)
        {
            StackPanel flyoutContent;
            {
                flyoutContent = new StackPanel() { Orientation = Orientation.Vertical };
                {
                    AppBarButton but = new AppBarButton();
                    but.Icon = new FontIcon() { FontFamily = new FontFamily("Segoe MDL2 Assets"), Glyph = "\xED62" };
                    but.Label = "Disconnect Node";
                    but.Click += (object sender2, RoutedEventArgs e2)=>
                    {
                        DisconnectFromAll();
                    };
                    flyoutContent.Children.Add(but);
                }
                {
                    AppBarButton but = new AppBarButton();
                    but.Icon = new FontIcon() { FontFamily = new FontFamily("Segoe MDL2 Assets"), Glyph = "\xE74D" };
                    but.Label = "Erase Node";
                    but.Click += (object sender2, RoutedEventArgs e2) =>
                    {
                        DisconnectFromAll();//.ContinueWith(()=> { 
                        
                        foreach (var child in Body.Children)
                        {
                            if (child is FrameworkElement)
                            {
                                if (Grid.GetColumn(child as FrameworkElement) == 1)
                                {
                                    Body.Children.Remove(child);
                                    break;
                                }
                            }
                        }

                        vm.DeleteSelfEvent.InvokeEvent(this, this);

                        //}, true);
                    };
                    flyoutContent.Children.Add(but);
                }
            }
            var flyout = sender.SetFlyout(flyoutContent, sender, null, null, show);
            return flyout;
        }

        private void LabelRightTapped(object sender, RightTappedRoutedEventArgs e)
        {
            if (sender is UIElement)
            {
                GetLabelFlyout(sender as UIElement, true);
            }
        }

        private void OutputNodeLoaded(object sender, RoutedEventArgs e)
        {
            (sender as DraggableOutputNode).vm.available_paths = this.vm.availabe_paths;
            (sender as DraggableOutputNode).vm.ParentVM = this.vm;
        }
        private void OutputNodeUnloaded(object sender, RoutedEventArgs e)
        {
            (sender as DraggableOutputNode).vm.ParentVM = null;
        }
        private void ResetNodePositions(object sender, SizeChangedEventArgs e)
        {
            OutputNode.ResetPosition();
        }

        private void InputNodeLoaded(object sender, RoutedEventArgs e)
        {
            (sender as DroppableInputNode).vm.ParentVM = this.vm;
            if (!(bool)((sender as DroppableInputNode).Tag))
            {
                (sender as DroppableInputNode).Tag = true;
                (sender as DroppableInputNode).vm.ConnectedEvent += (object draggableoutputnode, DroppableInputNodeViewModel e2) =>
                {
                    DraggableOutputNode n = draggableoutputnode as DraggableOutputNode;
                    EdmsTasks.InsertJob(() =>
                    {
                        // if all of the incoming nodes are connected, add a new one!
                        bool anyEmpty = false;
                        foreach (DroppableInputNode x in InputNodeList.Children)
                        {
                            if (x != null)
                            {
                                if (string.IsNullOrEmpty(x.vm.DataSourceUniqueName)) //VariableName))
                                {
                                    anyEmpty = true;
                                }
                            }
                        }

                        if (!anyEmpty)
                        {
                            var droppable = new DroppableInputNode() { HorizontalAlignment = HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Center };
                            droppable.Loaded += InputNodeLoaded;
                            droppable.Unloaded += InputNodeUnloaded;
                            InputNodeList.Children.Add(droppable);
                            vm.PositionChanged.InvokeEventAsync(this, false);
                        }
                    }, true);
                };
            }
        }
        private void InputNodeUnloaded(object sender, RoutedEventArgs e)
        {
            (sender as DroppableInputNode).vm.ParentVM = null;
        }

        private void EditScript(object sender, RoutedEventArgs e)
        {
            vm.mode = ScriptingNodeViewModel.ScriptingNodeMode.Editing;

            foreach (var child in Body.Children)
            {
                if (child is FrameworkElement)
                {
                    if (Grid.GetColumn(child as FrameworkElement) == 1) {
                        Body.Children.Remove(child);
                        break;
                    }
                }
            }

            // user wants to edit the script of this node
            {
                var editorS = new ScriptNode_Editor(vm); // vm.engine, vm
                editorS.Loaded += (object sender2, RoutedEventArgs e2) => {
                    (sender2 as ScriptNode_Editor).vm.ParentVM = vm;
                };
                editorS.Unloaded += (object sender2, RoutedEventArgs e2) => {
                    (sender2 as ScriptNode_Editor).vm.ParentVM = null;
                };

                editorS.SizeChanged += (object sender2, SizeChangedEventArgs args2) => {
                    vm.SizeChanged.InvokeEventAsync(null, false);
                };

                Body.Children.Add(editorS);
                Grid.SetColumn(editorS, 1);
            }
        }

        private void VisualizeResult(object sender, RoutedEventArgs e)
        {
            vm.mode = ScriptingNodeViewModel.ScriptingNodeMode.Visualizing;

            foreach (var child in Body.Children)
            {
                if (child is FrameworkElement)
                {
                    if (Grid.GetColumn(child as FrameworkElement) == 1)
                    {
                        Body.Children.Remove(child);
                        break;
                    }
                }
            }

            // user wants to visualize the result of this node
            {
                var editorS = new ScriptNode_Visualizer(vm);
                editorS.Loaded += (object sender2, RoutedEventArgs e2)=> {
                    (sender2 as ScriptNode_Visualizer).vm.ParentVM = vm;
                };
                editorS.Unloaded += (object sender2, RoutedEventArgs e2) => {
                    (sender2 as ScriptNode_Visualizer).vm.ParentVM = null;
                };

                editorS.SizeChanged += (object sender2, SizeChangedEventArgs args2) => {
                    vm.SizeChanged.InvokeEventAsync(null, false);
                };

                Body.Children.Add(editorS);
                Grid.SetColumn(editorS, 1);
            }
        }

        private void OnLabelLoaded(object sender, RoutedEventArgs e)
        {
            (sender as RichEditBox).SelectionFlyout = GetLabelFlyout((sender as RichEditBox), false);
            (sender as RichEditBox).IsSpellCheckEnabled = false;
            (sender as RichEditBox).ContextFlyout = GetLabelFlyout((sender as RichEditBox), false);
        }
    }
}
