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

#define UseFrameworkUpdater

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using UWP_WaterWatchLibrary;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.System;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Text;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using UWP_WaterWatch;
using static UWP_WaterWatchLibrary.ObjectExtensions;
using Windows.UI.Xaml.Shapes;
using Windows.UI.Xaml.Controls.Maps;
// using Microsoft.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    public class ScriptNode_VisualizerVM : ViewModelBase
    {
        public static AtomicInt ObjCount = new AtomicInt();

        public ScriptingNodeViewModel ParentVM = null;
        public Border ResultContainer;
        public ScriptingNodeResult NodeResult { get { return _NodeResult; } set { if (_NodeResult == value) return; _NodeResult = value; OnPropertyChanged("ScriptingNodeResult"); } }
        private ScriptingNodeResult _NodeResult = null;

        public ScriptNode_VisualizerVM() {
            ObjCount.Increment();
        }
        ~ScriptNode_VisualizerVM() {
            ObjCount.Decrement();
            ParentVM = null;
            ResultContainer = null;
            _NodeResult = null;
        }

        public void Reload()
        {
            cweeTask<ScriptingNodeResult> t = ParentVM.RunAsync();
            t.ContinueWith(() => {
                NodeResult = t.Result;

                if (!string.IsNullOrEmpty(NodeResult.Error))
                {
                    var getErrText = StringContent(NodeResult.Error, true);
                    getErrText.ContinueWith(() => {
                        if (ResultContainer != null && getErrText.Result != null)
                        {
                            ResultContainer.Child = getErrText.Result;
                        }
                    }, true);
                }
                else
                {
                    // result is valid.
                    var getContent = GetNodeContent(new SharedNodeResult() { additionalParams = "", result = NodeResult });
                    getContent.ContinueWith(() => {
                        ResultContainer.Child = getContent.Result;
                    }, true);
                }

            }, false);
        }

        public enum ValueTypeModes
        {
            String, cweeStr, Vector, Map, Pattern, Pair, Number, Void, Other, Error, Any, Function, DynamicObject, VectorMapItems, Matrix, MatX, FrameworkElement, Curve
        }

        public static void ToReturn_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (sender is Panel)
            {

                foreach (var child in (sender as Panel).Children)
                {
                    if (child is FrameworkElement)
                    {
                        (child as FrameworkElement).Width = (sender as Panel).ActualWidth;
                        (child as FrameworkElement).Height = (sender as Panel).ActualHeight;
                    }
                }
            }
            else if (sender is Border)
            {
                var child = (sender as Border).Child;
                if (child is FrameworkElement)
                {
                    (child as FrameworkElement).Width = (sender as Border).ActualWidth;
                    (child as FrameworkElement).Height = (sender as Border).ActualHeight;
                }
            }
        }

        public class SharedNodeResult
        {
            public ScriptingNodeResult result;
            public string additionalParams;
        }

        public static cweeTask<FrameworkElement> StringContent(string content, bool resizeText = false)
        {
            return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() =>
            {
                var container = new Border();
                var tb = new TextBlock()
                {
                    Margin = new Thickness(0),
                    Padding = new Thickness(0),
                    Text = content,
                    TextWrapping = TextWrapping.Wrap,
                    HorizontalAlignment = HorizontalAlignment.Center,
                    VerticalAlignment = VerticalAlignment.Center,
                    HorizontalTextAlignment = TextAlignment.Center,
                    Style = cweeXamlHelper.StaticStyleResource("cweeTextBlock")
                };
                if (resizeText)
                {
                    tb.SizeChanged += cweeXamlHelper.AutomaticTextPlacement; //  AutomaticTextSize; // beware looping resize actions crashing UWP
                }
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
                                    Functions.CopyToClipboard(content);
                                }, true, true);
                            };
                            tempGrid.Children.Add(but);
                        }
                    }
                    var flyout = tb.SetFlyout(tempGrid, tb, tempGrid, e.GetPosition(tb));
                    e.Handled = true;
                    tb.ContextFlyout.LightDismissOverlayMode = LightDismissOverlayMode.On;
                };

                container.Child = tb;
                container.HorizontalAlignment = HorizontalAlignment.Stretch;
                container.VerticalAlignment = VerticalAlignment.Stretch;

                return container;
            }, true, true);
        }

        public static cweeTask<FrameworkElement> GetDefaultContent(SharedNodeResult res)
        {
            var contentAsString = res.result.QueryResult(res.additionalParams + ".to_string();");
            return contentAsString.ContinueWith(() => {
                string content_string = contentAsString.Result;
                if (content_string.Find("Error: ") == 0)
                {
                    // we have an error -- simply return the type.
                    var contentAsString2 = res.result.QueryResult(res.additionalParams + ".type_name()");
                    return contentAsString2.ContinueWith(() => {
                        string content_string2 = contentAsString2.Result;
                        return StringContent(content_string2);
                    }, false);
                }
                return StringContent(content_string);
            }, false);
        }

        public static cweeTask<FrameworkElement> GetVoidContent()
        {
            return StringContent("\"Void\"");
        }

        public static cweeTask<ValueTypeModes> ValueTypeMode(SharedNodeResult res)
        {
            if (!string.IsNullOrEmpty(res.result.Error))
            {
                return EdmsTasks.cweeTask.CompletedTask(ValueTypeModes.Error);
            }
            else
            {
                var typeNameJob = res.result.QueryResult(res.additionalParams + ".type_name()");
                return typeNameJob.ContinueWith(() =>
                {
                    string vt = typeNameJob.Result;

                    if (string.IsNullOrEmpty(vt)) {
                        return ValueTypeModes.Void;
                    }

                    bool isNull = (vt == "void");
                    bool _container = vt.Contains("_", StringComparison.OrdinalIgnoreCase) || vt.Contains("<", StringComparison.OrdinalIgnoreCase);
                    bool _isMap = (vt == "Map");
                    bool _vector = vt.Contains("Vector", StringComparison.OrdinalIgnoreCase);
                    bool _dynamic_object = vt.Contains("Dynamic_Object", StringComparison.OrdinalIgnoreCase);
                    bool _cweeGeopoint = vt.Contains("cweeGeopoint", StringComparison.OrdinalIgnoreCase); // other
                    bool _cweeMapIcon = vt.Contains("cweeMapIcon", StringComparison.OrdinalIgnoreCase); // other 
                    bool _pattern = vt.Contains("Pattern", StringComparison.OrdinalIgnoreCase);
                    bool _matrix = vt.Contains("Matrix", StringComparison.OrdinalIgnoreCase);
                    bool _curve = vt.Contains("Curve", StringComparison.OrdinalIgnoreCase);
                    bool _MatX = vt.Contains("MatX", StringComparison.OrdinalIgnoreCase);
                    bool _string = vt.Contains("string", StringComparison.OrdinalIgnoreCase);
                    bool _cweestring = vt.Contains("cweeStr", StringComparison.OrdinalIgnoreCase);
                    bool _function = vt.Contains("Function", StringComparison.OrdinalIgnoreCase);
                    int _int = vt.Contains("int", StringComparison.OrdinalIgnoreCase) ? 1 : 0;
                    bool _pair = vt.Contains("Pair", StringComparison.OrdinalIgnoreCase);
                    int _float = vt.Contains("float", StringComparison.OrdinalIgnoreCase) ? 1 : 0;
                    bool _void = isNull || vt.Contains("void", StringComparison.OrdinalIgnoreCase);
                    int _double = vt.Contains("double", StringComparison.OrdinalIgnoreCase) ? 1 : 0;
                    int _u64 = vt.Contains("int64_t", StringComparison.OrdinalIgnoreCase) ? 1 : 0;

                    if (isNull)
                    {
                        return ValueTypeModes.Void;
                    }
                    else if (_vector || _pair)
                    {
                        ValueTypeModes containerType = ValueTypeModes.Other;

                        // it's a container
                        // can we extract the outermost container? 
                        if (_container && vt.Contains("_", StringComparison.OrdinalIgnoreCase))
                        {
                            string bestMatch = WaterWatch.GetBestMatch(vt.Split("_")[0], new List<string>() { "Vector", "Pair" });
                            switch (bestMatch)
                            {
                                case "Vector":
                                    {
                                        if (_vector)
                                        {
                                            containerType = ValueTypeModes.Vector;
                                        }
                                        break;
                                    }
                                case "Pair":
                                    {
                                        if (_pair)
                                        {
                                            containerType = ValueTypeModes.Pair;
                                        }
                                        break;
                                    }
                            }
                        }
                        else if (_container && vt.Contains("<", StringComparison.OrdinalIgnoreCase))
                        {
                            string bestMatch = WaterWatch.GetBestMatch(vt.Split("<")[0], new List<string>() { "Vector", "Pair" });
                            switch (bestMatch)
                            {
                                case "Vector":
                                    {
                                        if (_vector)
                                        {
                                            containerType = ValueTypeModes.Vector;
                                        }
                                        break;
                                    }
                                case "Pair":
                                    {
                                        if (_pair)
                                        {
                                            containerType = ValueTypeModes.Pair;
                                        }
                                        break;
                                    }
                            }
                        }
                        else
                        {
                            if (_vector)
                            {
                                containerType = ValueTypeModes.Vector;
                            }
                            if (_pair)
                            {
                                containerType = ValueTypeModes.Pair;
                            }
                        }

                        switch (containerType)
                        {
                            case ValueTypeModes.Vector:
                                {
                                    // check if all of the items in the vector are of a specific type, else, just return Vector
                                    var subTypeJob = res.result.QueryResult(res.additionalParams + "[0].type_name()");
                                    return subTypeJob.ContinueWith(() =>
                                    {
                                        string subType = subTypeJob.Result;
                                        if (subType.Find("Error: ") == 0)
                                        {
                                            return ValueTypeModes.Vector;
                                        }
                                        else if (subType == "")
                                        {
                                            return ValueTypeModes.Vector;
                                        }
                                        //else if (subType == "cweeMapIcon")
                                        //{
                                        //    return ValueTypeModes.VectorMapItems;
                                        //}
                                        //else if (subType == "cweeMapPolyline")
                                        //{
                                        //    return ValueTypeModes.VectorMapItems;
                                        //}
                                        //else if (subType == "cweeMapBackground")
                                        //{
                                        //    return ValueTypeModes.VectorMapItems;
                                        //}
                                        else
                                        {
                                            return ValueTypeModes.Vector;
                                        }
                                    }, false);
                                }
                            case ValueTypeModes.Map:
                                return containerType;
                            case ValueTypeModes.Pair:
                                return containerType;
                        }
                    }
                    else if (_isMap)
                    {
                        return ValueTypeModes.Map;
                    }
                    else if (_curve)
                    {
                        return ValueTypeModes.Curve;
                    }
                    else if ((_int + _float + _double + _u64) == 1)
                    {
                        return ValueTypeModes.Number;
                    }
                    else if (_dynamic_object)
                    {
                        return ValueTypeModes.DynamicObject;
                    }
                    else if (_string)
                    {
                        return ValueTypeModes.String;
                    }
                    else if (_cweestring)
                    {
                        return ValueTypeModes.cweeStr;
                    }
                    else if (_function)
                    {
                        return ValueTypeModes.Function;
                    }
                    else if (_pattern)
                    {
                        return ValueTypeModes.Pattern;
                    }
                    else if (_matrix)
                    {
                        return ValueTypeModes.Matrix;
                    }
                    else if (_MatX)
                    {
                        return ValueTypeModes.MatX;
                    }
                    else if (_void)
                    {
                        return ValueTypeModes.Void;
                    }

                    //var InteractiveJob2 = res.result.QueryResult(res.additionalParams + ".is_type(\"UI_MapElement\") ? 1 : 0"); // checks for polymorphic Map UI elements
                    //return InteractiveJob2.ContinueWith(() =>
                    //{
                    //    string is_map_element = InteractiveJob2.Result;
                    //    if (int.TryParse(is_map_element, out int result2) && result2 == 1)
                    //    {
                    //        return ValueTypeModes.Map;
                    //    }
                    //    else
                    //    {
                            var InteractiveJob = res.result.QueryResult(res.additionalParams + ".is_type(\"UI_FrameworkElement\") ? 1 : 0"); // checks for polymorphic UI elements
                            return InteractiveJob.ContinueWith(() =>
                            {
                                string is_interactive = InteractiveJob.Result;
                                if (int.TryParse(is_interactive, out int result) && result == 1)
                                {
                                    return ValueTypeModes.FrameworkElement;
                                }
                                else
                                {
                                    return ValueTypeModes.Other;
                                }
                            }, false);
                    //    }
                    //}, false);

                }, false);
            }
        }
        public static Microsoft.UI.Xaml.Controls.ProgressRing GenericLoadingRing()
        {
            var toReturn = new Microsoft.UI.Xaml.Controls.ProgressRing()
            {
                Width = 24,
                Height = 24,
                MaxWidth = 24,
                MaxHeight = 24,
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
                HorizontalContentAlignment = HorizontalAlignment.Center,
                VerticalContentAlignment = VerticalAlignment.Center,
                Padding = new Thickness(0),
                Margin = new Thickness(0),
                Foreground = (SolidColorBrush)App.Current.Resources["cweeDarkBlue"],
                IsIndeterminate = true
            };
            return toReturn;
        }
        public static Microsoft.UI.Xaml.Controls.ProgressRing GenericLoadingRing(AtomicInt progress)
        {
            AtomicInt prev_progress = new AtomicInt(0);
            var toReturn = GenericLoadingRing();
            toReturn.IsIndeterminate = false;
            toReturn.Value = 0; 

            toReturn.Loaded += (object sender, RoutedEventArgs e)=> {
                var PR = sender as Microsoft.UI.Xaml.Controls.ProgressRing;
                PR.Tag = new cweeTimer(20.0 / 60.0, ()=> {
                    if (prev_progress.Get() != progress.Get())
                    {
                        prev_progress.Set(progress.Get());
                        EdmsTasks.InsertJob(()=> {
                            PR.Value = (double)prev_progress.Get();
                        }, true);
                    }
                }, false);               
            };
            toReturn.Unloaded += (object sender, RoutedEventArgs e) =>
            {
                var PR = sender as Microsoft.UI.Xaml.Controls.ProgressRing;
                PR.Tag = null;
            };
            return toReturn;
        }

        public static cweeTask<FrameworkElement> GetNodeContent(SharedNodeResult res)
        {
            if (!string.IsNullOrEmpty(res.result.Error))
            {
                return StringContent(res.result.Error, true);
            }
            else
            {
                var getVT = ValueTypeMode(res);
                return getVT.ContinueWith(() =>
                {
                    switch (getVT.Result)
                    {
                        case ValueTypeModes.Function:
                            return GetNodeContent_Function(res);
                        case ValueTypeModes.Map:
                            return GetNodeContent_Map(res);
                        case ValueTypeModes.Curve:
                            return GetNodeContent_Curve(res);
                        case ValueTypeModes.Vector:
                            return GetNodeContent_Vector(res);
                        case ValueTypeModes.Void:
                            return GetVoidContent();
                        case ValueTypeModes.DynamicObject:
                            return GetNodeContent_DynamicObject(res);
                        case ValueTypeModes.Pattern:
                            return GetNodeContent_Pattern(res);
                        case ValueTypeModes.FrameworkElement:
                            return GetNodeContent_FrameworkElement(res);
                        case ValueTypeModes.Pair:
                        case ValueTypeModes.Other:
                        case ValueTypeModes.String:
                        case ValueTypeModes.cweeStr:
                            return GetNodeContent_String(res);

                        case ValueTypeModes.Number:
                        default:
                            return GetDefaultContent(res);
                    }
                }, false);
            }
        }

        private static cweeTask<FrameworkElement> GetNodeContent_Function(SharedNodeResult res)
        {
            cweeTask<string> task1 = res.result.QueryResult(res.additionalParams + ".get().size();");
            return task1.ContinueWith(() => {
                if (int.TryParse(task1.Result, out int n))
                {
                    var collection = new cweeDeferredIncrementalLoader<MapStreamingSource, FrameworkElement>(5);
                    collection.Tag = (new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".get()" }, n);

                    return EdmsTasks.InsertJob(() =>
                    {
                        var L = new ListView();
                        L.ItemsSource = collection;
                        return L;
                    }, /*EdmsTasks.Priority.Low, */true, true);
                }
                else
                {
                    return GetDefaultContent(res);
                }
            }, false);
        }
        private static cweeTask<FrameworkElement> GetNodeContent_Map(SharedNodeResult res)
        {
            cweeTask<string> task1 = res.result.QueryResult(res.additionalParams + ".size();");
            return task1.ContinueWith(() => {
                if (int.TryParse(task1.Result, out int n))
                {
                    var collection = new cweeDeferredIncrementalLoader<MapStreamingSource, FrameworkElement>(5);
                    collection.Tag = (new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams }, n);

                    return EdmsTasks.InsertJob(() =>
                    {
                        var L = new ListView();
                        L.ItemsSource = collection;
                        return L;
                    }, true, true);
                }
                else
                {
                    return GetDefaultContent(res);
                }
            }, false);
        }
        private static cweeTask<FrameworkElement> GetNodeContent_Curve(SharedNodeResult res)
        {
            cweeTask<string> task1 = res.result.QueryResult(res.additionalParams + ".size();");
            return task1.ContinueWith(() => {
                if (int.TryParse(task1.Result, out int n))
                {
                    var collection = new cweeDeferredIncrementalLoader<CurveStreamingSource, FrameworkElement>(5);
                    collection.Tag = (new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams }, n);

                    return EdmsTasks.InsertJob(() =>
                    {
                        var L = new ListView();
                        L.ItemsSource = collection;
                        return L;
                    }, true, true);
                }
                else
                {
                    return GetDefaultContent(res);
                }
            }, false);
        }

        private static cweeTask<FrameworkElement> GetNodeContent_Vector(SharedNodeResult res)
        {
            cweeTask<string> task1 = res.result.QueryResult(res.additionalParams + ".size()");
            return task1.ContinueWith(() => {
                if (int.TryParse(task1.Result, out int n))
                {
                    var collection = new cweeDeferredIncrementalLoader<VectorStreamingSource, FrameworkElement>(5);
                    collection.Tag = (new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams }, n);
                    return EdmsTasks.InsertJob(() =>
                    {
                        var L = new ListView()
                        {
                            Margin = new Thickness(0),
                            Padding = new Thickness(0),
                            HorizontalAlignment = HorizontalAlignment.Stretch,
                            VerticalAlignment = VerticalAlignment.Stretch,
                            HorizontalContentAlignment = HorizontalAlignment.Stretch,
                            VerticalContentAlignment = VerticalAlignment.Stretch,
                            ItemContainerStyle = cweeXamlHelper.StaticStyleResource("cweeListViewSimpleItemStyle"),
                            ItemsSource = collection
                        };
                        return L;
                    }, /*EdmsTasks.Priority.Low, */true, true);
                }
                else
                {
                    return GetDefaultContent(res);
                }
            }, false);
        }
        private static cweeTask<FrameworkElement> GetNodeContent_String(SharedNodeResult res)
        {
            cweeTask<string> task1 = res.result.QueryResult(res.additionalParams + ".to_string()");
            return task1.ContinueWith(() => {
                if (task1.Result.StartsWith("https://"))
                {
                    return EdmsTasks.InsertJob(() => {
                        try
                        {
                            var sp1 = new Grid() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch };

                            var wv = new WebView() { Source = new Uri(task1.Result), HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch };

                            sp1.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(1, GridUnitType.Star) });
                            sp1.Children.Add(wv);
                            Grid.SetRow(wv, 0);
                            return sp1;
                        }
                        catch (Exception)
                        {
                            return StringContent(task1.Result);
                        }
                    }, true);
                }
                else
                {
                    return StringContent(task1.Result);
                }
            }, false);
        }
        private static cweeTask<FrameworkElement> GetNodeContent_DynamicObject(SharedNodeResult res)
        {
            cweeTask<string> task1 = res.result.QueryResult(res.additionalParams + ".get_attrs().size()");
            return task1.ContinueWith(() => {
                if (int.TryParse(task1.Result, out int n))
                {
                    var collection = new cweeDeferredIncrementalLoader<MapStreamingSource, FrameworkElement>(5);
                    collection.Tag = (new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".get_attrs()" }, n);

                    return EdmsTasks.InsertJob(() =>
                    {
                        var L = new ListView();
                        L.ItemsSource = collection;
                        return L;
                    }, /*EdmsTasks.Priority.Low, */true, true);
                }
                else
                {
                    return GetDefaultContent(res);
                }
            }, false);
        }
        private static cweeTask<FrameworkElement> GetNodeContent_Pattern(SharedNodeResult res)
        {
            return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                AtomicInt loadingProgress = new AtomicInt(0);
                var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing(loadingProgress) };
                r.SizeChanged += ToReturn_SizeChanged;
                double screenWidth = Window.Current.CoreWindow.Bounds.Width;
                EdmsTasks.InsertJob(() => {
                    var sharedPattern = new SharedTimeSeriesPattern(); // will self-delete some time after scope ends (required GC) -- including the C++ objects. 
                    {
                        loadingProgress.Set(10);
                        var setupTask = res.result.CustomizableQueryResult($"external_data.SetPattern({sharedPattern.Index()}, %s)", "%s", res.additionalParams); // .Blur({screenWidth}).RemoveUnnecessaryKnots();
                        return setupTask.ContinueWith(() => {
                            var numValues = sharedPattern.GetNumValues();
#if false
                            int progress = 0;
                            loadingProgress.Set(20);
                            List<ChartItem> actualDataList = new List<ChartItem>();
                            foreach (var point in sharedPattern.GetTimeSeries()) {                                
                                actualDataList.Add(new ChartItem() { 
                                    Date = DateTime.Now.FromUnixTimeSeconds(point.first), 
                                    Value = (float)(point.second) 
                                });
                                loadingProgress.Set((int)(20.0 + ((double)(progress++) / (double)(numValues))*80.0));
                            }
                            List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();
#else
                            loadingProgress.Set(20);

                            int totalProgress = 2;
                            
                            double minTime = sharedPattern.GetMinTime();
                            double maxTime = sharedPattern.GetMaxTime();
                            double step = ((maxTime - minTime) / (screenWidth + 1)) + 1;
                            for (double timeV = minTime; timeV <= maxTime; timeV += step) totalProgress++;

                            var arr = new ChartItem[totalProgress];
                            for (int i = 0; i < arr.Length; i++) arr[i] = new ChartItem();                            
                            List<ChartItem> actualDataList = arr.ToList();
                            
                            actualDataList[0].Date = DateTime.Now.FromUnixTimeSeconds(minTime);
                            actualDataList[0].Value = sharedPattern.GetValue(minTime);
                            actualDataList[totalProgress - 1].Date = DateTime.Now.FromUnixTimeSeconds(maxTime);
                            actualDataList[totalProgress - 1].Value = sharedPattern.GetValue(maxTime);

                            List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();

                            AtomicInt progress = new AtomicInt(2);
                            int capStep = Math.Max(100, (int)((double)totalProgress / 10.0));
                            for (int start = 1; start < (totalProgress - 1); start += capStep) {
                                int start_actual = start;
                                int end_actual = Math.Min(start + capStep, (totalProgress - 1));
                                tasks.Add(new EdmsTasks.cweeTask(()=> {
                                    for (int i = start_actual; i < end_actual; i++)
                                    {
                                        double timeV = minTime + step * (double)(i - 1);                                 
                                        actualDataList[i].Date = DateTime.Now.FromUnixTimeSeconds(timeV + (step / 2.0));
                                        actualDataList[i].Value = sharedPattern.GetAvgValue(timeV, timeV + step);

                                        loadingProgress.Set((long)(20.0 + 80.0 * ((double)progress.Increment() / (double)totalProgress)));                          
                                    }
                                }, false, true));
                            }

                            //for (int j = 1; j<(totalProgress - 1); j++){
                            //    int i = j;
                            //    double timeV = minTime + step * (double)(i - 1);
                            //    //tasks.Add(new EdmsTasks.cweeTask(()=> {                                    
                            //        actualDataList[i].Date = DateTime.Now.FromUnixTimeSeconds(timeV + (step / 2.0));
                            //        actualDataList[i].Value = sharedPattern.GetAvgValue(timeV, timeV + step);

                            //        loadingProgress.Set((long)(20.0 + 80.0 * ((double)progress.Increment() / (double)totalProgress)));                                   
                            //    //}, false, true));                                
                            //}
#endif
                            return EdmsTasks.cweeTask.InsertListAsTask(tasks, true).ContinueWith(()=> {
                                loadingProgress.Set(100);
                                (string, string) units = (sharedPattern.X_Units(), sharedPattern.Y_Units());
                                List<EdmsTasks.cweeTask> colorTask = new List<EdmsTasks.cweeTask>() { cweeXamlHelper.ThemeColor("cweeDarkBlue"), cweeXamlHelper.ThemeColor("cweeLightBlue") };
                                return EdmsTasks.cweeTask.TrueWhenCompleted(colorTask).ContinueWith(() => {
                                    var chart = TelerikHelper.CreateTelerikChart(new TelerikChartDetails() {
                                        X_axis_title = units.Item1 == "s" ? null : units.Item1,
                                        y_axis_title = units.Item2,
                                        charts = new List<TelerikChartData>() {
                                            new TelerikChartData()
                                            {
                                                fillColor = colorTask[0].Result,
                                                spline_data = actualDataList,
                                                strokeColor = colorTask[1].Result,
                                                strokeThickness = 0.25
                                            }
                                        }
                                    });
                                    return chart.ContinueWith(() => {
                                        //chart.Result.HorizontalAlignment = HorizontalAlignment.Left;
                                        //chart.Result.VerticalAlignment = VerticalAlignment.Top;
                                        if (r.IsLoaded)
                                        {
                                            chart.Result.Width = r.ActualWidth;
                                            chart.Result.Height = r.ActualHeight;
                                        }

                                        chart.Result.MinHeight = 160;
                                        chart.Result.MinWidth = 220;

                                        r.Child = chart.Result;

                                        return chart;
                                    }, true);
                                }, true);
                            }, false);
                        }, false);
                    }
                }, false, true);

                return r;
            }, true, true);
        }
        private static cweeTask<FrameworkElement> GetNodeContent_FrameworkElement(SharedNodeResult res)
        {
            var typeNameJob = res.result.QueryResult(res.additionalParams + ".type_name()");
            return (EdmsTasks.cweeTask)typeNameJob.ContinueWith(() => {
                string typeName = typeNameJob.Result;
                switch (typeName)
                {
                    case "UI_Map": return ScriptedFrameworkElements.GetFrameworkElement_Map(res);
                    case "UI_MapLayer": return ScriptedFrameworkElements.GetFrameworkElement_Map(res);
                    case "UI_MapPolyline": return ScriptedFrameworkElements.GetFrameworkElement_Map(res);
                    case "UI_MapIcon": return ScriptedFrameworkElements.GetFrameworkElement_Map(res);
                    case "UI_ProgressRing": return ScriptedFrameworkElements.GetFrameworkElement_ProgressRing(res);
                    case "UI_Rectangle": return ScriptedFrameworkElements.GetFrameworkElement_Rectangle(res);
                    case "UI_TextBlock": return ScriptedFrameworkElements.GetFrameworkElement_TextBlock(res);
                    case "UI_Image": return ScriptedFrameworkElements.GetFrameworkElement_Image(res);
                    case "UI_WebView": return ScriptedFrameworkElements.GetFrameworkElement_WebView(res);
                    case "UI_Grid": return ScriptedFrameworkElements.GetFrameworkElement_Grid(res);
                    case "UI_StackPanel": return ScriptedFrameworkElements.GetFrameworkElement_StackPanel(res);
                    case "UI_Button": return ScriptedFrameworkElements.GetFrameworkElement_Button(res);
                    case "UI_CheckBox": return ScriptedFrameworkElements.GetFrameworkElement_CheckBox(res);
                    case "UI_Slider": return ScriptedFrameworkElements.GetFrameworkElement_Slider(res);
                    case "UI_ToggleSwitch": return ScriptedFrameworkElements.GetFrameworkElement_ToggleSwitch(res);
                    case "UI_ListView": return ScriptedFrameworkElements.GetFrameworkElement_ListView(res);
                    case "UI_TabView": return ScriptedFrameworkElements.GetFrameworkElement_TabView(res);
                    case "UI_Expander": return ScriptedFrameworkElements.GetFrameworkElement_Expander(res);
                    case "UI_TextBox": return ScriptedFrameworkElements.GetFrameworkElement_TextBox(res);
                    case "UI_Plot": return ScriptedFrameworkElements.GetFrameworkElement_Plot(res);
                    default: return GetDefaultContent(res);
                }
            }, false);
        }
        public static class ScriptedFrameworkElements
        {
#if UseFrameworkUpdater
            public static class Framework_Updater
            {
                internal static cweeMultiAppendableTimer timer = new cweeMultiAppendableTimer(1.0 / 60.0, false);
                internal class Framework_Updater_Impl
                {
                    public int currentVersion = -1;
                    public cweeEvent<List<string>> tasks = new cweeEvent<List<string>>();
                }

                internal static System.Collections.Concurrent.ConcurrentDictionary<string, Framework_Updater_Impl> ManagedBackendObjects = new System.Collections.Concurrent.ConcurrentDictionary<string, Framework_Updater_Impl>();
              
                public static void Subscribe(FrameworkElement el, SharedNodeResult res, Action<object, List<string>> DoUpdateSubscriber)
                {
                    var shared_key = res.result.QueryResult(res.additionalParams + ".UniqueName"); // represents the C++ back-end object
                    shared_key.ContinueWith(()=> { 
                        if (el.IsLoaded)
                        {
                            Subscriber_Loaded(shared_key.Result, res, DoUpdateSubscriber);
                        }
                        el.Loaded += (object sender, RoutedEventArgs e) => { Subscriber_Loaded(shared_key.Result, res, DoUpdateSubscriber); };
                        el.Unloaded += (object sender, RoutedEventArgs e) => { Subscriber_Unloaded(shared_key.Result, DoUpdateSubscriber); };
                    }, true);
                }

                private static void Subscriber_Loaded(string shared_key, SharedNodeResult res, Action<object, List<string>> DoUpdateSubscriber)
                {
                    EdmsTasks.InsertJob(() => {
                        if (!ManagedBackendObjects.ContainsKey(shared_key)) { ManagedBackendObjects[shared_key] = new Framework_Updater_Impl(); }
                        ManagedBackendObjects[shared_key].tasks += DoUpdateSubscriber;
                        AtomicInt localLock = new AtomicInt();
                        timer.AddAction(() => {
                            if (localLock.TryIncrementTo(1))
                            {
                                var versionJob = res.result.QueryResult(res.additionalParams + ".Version");
                                versionJob.ContinueWith(() =>
                                {
                                    if (int.TryParse(versionJob.Result, out int version))
                                    {
                                        int prevVersion = 0;

                                        if (ManagedBackendObjects.ContainsKey(shared_key))
                                        {
                                            prevVersion = ManagedBackendObjects[shared_key].currentVersion;
                                            ManagedBackendObjects[shared_key].currentVersion = version;
                                        }

                                        if (prevVersion != version)
                                        {
                                            cweeTask<List<string>> tasks = res.result.CustomizableQueryResult_Cast_VectorString("%s", "%s", res.additionalParams + ".GetTasks()");
                                            tasks.ContinueWith(() =>
                                            {
                                                if (tasks.Result.Count > 0 && ManagedBackendObjects.ContainsKey(shared_key))
                                                {
                                                    ManagedBackendObjects[shared_key].tasks.InvokeEventAsync(res, tasks.Result);
                                                    localLock.Decrement();
                                                }
                                                else
                                                {
                                                    localLock.Decrement();
                                                }                                                    
                                            }, false);
                                        }
                                        else
                                        {
                                            localLock.Decrement();
                                        }
                                    }
                                    else
                                    {
                                        localLock.Decrement();
                                    }
                                }, false);
                            }
                        }, int.Parse(shared_key));
                    }, false);
                }
                private static void Subscriber_Unloaded(string shared_key, Action<object, List<string>> DoUpdateSubscriber)
                {
                    EdmsTasks.InsertJob(() =>
                    {
                        //lock (ManagedBackendObjects)
                        {
                            if (timer.RemoveAction(int.Parse(shared_key)))
                            {
                                if (ManagedBackendObjects.ContainsKey(shared_key))
                                {
                                    ManagedBackendObjects[shared_key].tasks -= DoUpdateSubscriber;
                                    ManagedBackendObjects.Remove(shared_key, out Framework_Updater_Impl j);
                                }
                            }
                            else
                            {
                                if (ManagedBackendObjects.ContainsKey(shared_key))
                                {
                                    ManagedBackendObjects[shared_key].tasks -= DoUpdateSubscriber;
                                }
                            }
                        }
                    }, false, true);
                }
            }
#endif
            public static bool TryStringToGridLength(string v, out GridLength R) {
                if (v.IndexOf("auto", StringComparison.CurrentCultureIgnoreCase) >= 0)
                {
                    R = new GridLength(1, GridUnitType.Auto);
                    return true;
                }
                bool star = false;
                if (v.IndexOf("*", StringComparison.CurrentCultureIgnoreCase) >= 0)
                {
                    star = true;
                }
                v = v.Replace("*", "").Trim();
                if (v == "")
                {
                    R = new GridLength(1, star ? GridUnitType.Star : GridUnitType.Pixel);
                    return true;
                }
                if (double.TryParse(v, out double rr)) {
                    R = new GridLength(rr, star ? GridUnitType.Star : GridUnitType.Pixel);
                    return true;
                }
                return false;
            }
            public static bool TryStringToThickness(string v, out Thickness R)
            {
                var Paddings = v.Split(",");
                List<double> paddings = new List<double>();
                foreach (var pad in Paddings)
                {
                    if (double.TryParse(pad.Trim(), out double w))
                    {
                        paddings.Add(w);
                    }
                }

                switch (paddings.Count)
                {
                    case 0:
                        // do nothing
                        return false;
                    case 1:
                        R = new Thickness(paddings[0]);
                        return true;
                    case 2:
                        R = new Thickness(paddings[0], paddings[1], 0, 0);
                        return true;
                    case 3:
                        R = new Thickness(paddings[0], paddings[1], paddings[2], 0);
                        return true;
                    case 4:
                    default:
                        R = new Thickness(paddings[0], paddings[1], paddings[2], paddings[3]);
                        return true;
                }
            }
            public static cweeTask<Color> GetColor(SharedNodeResult res)
            {
                var job1 = res.result.CustomizableQueryResult_Cast_VectorString("return [%s.R.to_string(), %s.G.to_string(), %s.B.to_string(), %s.A.to_string()]", "%s", res.additionalParams);
                return job1.ContinueWith(()=> {
                    List<string> colors = job1.Result;
                    try
                    {
                        if (double.TryParse(colors[0], out double R) && double.TryParse(colors[1], out double G) && double.TryParse(colors[2], out double B) && double.TryParse(colors[3], out double A))
                        {
                            return Color.FromArgb((byte)A, (byte)R, (byte)G, (byte)B);
                        }
                    } catch (Exception) { }
                    return new Color();
                }, false);
            }
            private static cweeTask<T> SetFrameworkElement<T>(SharedNodeResult res, T R) where T : FrameworkElement
            {
                var FrameworkJob = res.result.CustomizableQueryResult_Cast_VectorString(
                    "return [" +
                    "\"${ %s.Opacity }\"" +
                    ", \"${ %s.Width }\"" +
                    ", \"${ %s.Height }\"" +
                    ", \"${ %s.VerticalAlignment }\"" +
                    ", \"${ %s.HorizontalAlignment }\"" +
                    ", \"${ %s.Name }\"" +
                    ", \"${ %s.MinWidth }\"" +
                    ", \"${ %s.MinHeight }\"" +
                    ", \"${ %s.MaxWidth }\"" +
                    ", \"${ %s.MaxHeight }\"" +
                    ", \"${ %s.Margin }\"" +
                    "]"
                    , "%s", res.additionalParams);
                
                R.Loaded += (object sender, RoutedEventArgs e) => {
                    res.result.CustomizableQueryResult("%s.OnLoaded();", "%s", res.additionalParams);
                };
                R.Unloaded += (object sender, RoutedEventArgs e) => {
                    res.result.CustomizableQueryResult("%s.OnUnloaded();", "%s", res.additionalParams);
                };

                return FrameworkJob.ContinueWith(() => {
                    List<string> frameworkParms = FrameworkJob.Result;
                    { if (double.TryParse(frameworkParms[0], out double w) && w != -1.0) { R.Opacity = w; } }
                    { if (double.TryParse(frameworkParms[1], out double w) && w != -1.0) { R.Width = w; } }
                    { if (double.TryParse(frameworkParms[2], out double w) && w != -1.0) { R.Height = w; } }
                    { R.VerticalAlignment = FindBestMatch<VerticalAlignment>(frameworkParms[3]); }
                    { R.HorizontalAlignment = FindBestMatch<HorizontalAlignment>(frameworkParms[4]); }
                    { R.Name = frameworkParms[5]; }
                    { if (double.TryParse(frameworkParms[6], out double w) && w != -1.0) { R.MinWidth = w; } }
                    { if (double.TryParse(frameworkParms[7], out double w) && w != -1.0) { R.MinHeight = w; } }
                    { if (double.TryParse(frameworkParms[8], out double w) && w != -1.0) { R.MaxWidth = w; } }
                    { if (double.TryParse(frameworkParms[9], out double w) && w != -1.0) { R.MaxHeight = w; } }
                    { if (TryStringToThickness(frameworkParms[10], out Thickness t)) { R.Margin = t; } }
                    return R;
                }, true);
            }

            private static cweeTask<T> SetControlElement<T>(SharedNodeResult res, T R) where T : ContentControl
            {
                return SetFrameworkElement<T>(res, R).ContinueWith(()=> {
                    var FrameworkJob = res.result.CustomizableQueryResult_Cast_VectorString(
                    "return [" +
                    "\"${ %s.IsEnabled ? 1.0 : 0.0 }\"" +
                    ", \"${ %s.Padding }\"" +
                    "]"
                    , "%s", res.additionalParams);

                    return FrameworkJob.ContinueWith(() => {
                        List<string> frameworkParms = FrameworkJob.Result;
                        { if (double.TryParse(frameworkParms[0], out double w) && w != -1.0) { R.IsEnabled = (w != 0.0); } }
                        { if (TryStringToThickness(frameworkParms[1], out Thickness t)) { R.Padding = t; } }
                        return R;
                    }, true);
                }, false);
            }

            private static void FrameworkElement_FirstLoaded(object sender, RoutedEventArgs e)
            {
                FrameworkElement frameworkElement = sender as FrameworkElement;

                (frameworkElement.Tag as cweeDequeue).Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0));

                frameworkElement.Loaded -= FrameworkElement_FirstLoaded;
            }
            public static cweeTask<FrameworkElement> GetFrameworkElement_ProgressRing(SharedNodeResult res)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return GenericLoadingRing(); }, true);
                        actualContainer.ContinueWith(() => {
                            var frameworkJob = SetFrameworkElement(res, actualContainer.Result as Microsoft.UI.Xaml.Controls.ProgressRing);
                            frameworkJob.ContinueWith(() => {
                                Microsoft.UI.Xaml.Controls.ProgressRing R = frameworkJob.Result;
                                {
                                    var fill = GetColor(new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".Foreground" });
                                    fill.ContinueWith(() => {
                                        R.Foreground = new SolidColorBrush(fill.Result);
                                        r.Child = R; // DONE
                                    }, true);
                                }
                            }, false);
                        }, false);

                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;

                        foreach (var task in tasks)
                        {
                            switch (task)
                            {
                                case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                            }
                        }
                    });
#endif
                    r.Loaded += FrameworkElement_FirstLoaded;

                    return r;
                }, true, true);
            }
            public static cweeTask<FrameworkElement> GetFrameworkElement_Rectangle(SharedNodeResult res) {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return new Rectangle(); }, true);
                        actualContainer.ContinueWith(()=> {
                            var frameworkJob = SetFrameworkElement(res, actualContainer.Result as Rectangle);
                            var customJob = res.result.CustomizableQueryResult_Cast_VectorString(
                            "return [" +
                                "\"${ Min(%s.GradientOffsets.size, %s.GradientColors.size) }\"" +
                                ", \"${ %s.GradientStart }\"" +
                                ", \"${ %s.GradientEnd }\"" +
                            "]"
                            , "%s", res.additionalParams);
                            var fill = GetColor(new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".Fill" });
                            EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() {
                                    frameworkJob,
                                    customJob,
                                    fill
                            }).ContinueWith(() => { 
                                Rectangle R = frameworkJob.Result;
                                {
                                    int numGradients = 0;
                                    { if (double.TryParse(customJob.Result[0], out double w)) {
                                            numGradients = (int)w; 
                                    } }
                                    if (numGradients > 0)
                                    {
                                        var startP = new Point();
                                        var endP = new Point();
                                        { if (TryStringToThickness(customJob.Result[1], out Thickness w)) { startP.X = w.Left; startP.Y = w.Top; } }
                                        { if (TryStringToThickness(customJob.Result[2], out Thickness w)) { endP.X = w.Left; endP.Y = w.Top; } }
                                        
                                        var gradientJobs = new List<EdmsTasks.cweeTask>();
                                        for (int i = 0; i < numGradients; i++) {
                                            gradientJobs.Add(res.result.CustomizableQueryResult_Cast_VectorFloat(
                                                $"return [%s.GradientOffsets[{i}].float, %s.GradientColors[{i}].R.float, %s.GradientColors[{i}].G.float, %s.GradientColors[{i}].B.float, %s.GradientColors[{i}].A.float]"
                                            , "%s", res.additionalParams));
                                        }
                                        EdmsTasks.cweeTask.TrueWhenCompleted(gradientJobs).ContinueWith(()=> {
                                            var gsc = new GradientStopCollection();
                                            foreach (var job in gradientJobs) {
                                                List<float> f = (job.Result as List<float>);
                                                if (f.Count >= 5) {
                                                    gsc.Add(new GradientStop() { Color = new Color() { R= (byte)f[1], G = (byte)f[2], B = (byte)f[3], A = (byte)f[4] }, Offset = (double)(f[0]) });
                                                }
                                            }
                                            var brush = new LinearGradientBrush() { StartPoint = startP, EndPoint = endP, GradientStops = gsc };
                                            R.Fill = brush;
                                            r.Child = R;
                                        }, true);
                                    }
                                    else
                                    {
                                        R.Fill = new SolidColorBrush(fill.Result);
                                        r.Child = R; // DONE
                                    }
                                }
                            }, true);
                        }, false);

                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;

                        foreach (var task in tasks)
                        {
                            switch (task)
                            {
                                case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                            }
                        }
                    });
#endif
                    r.Loaded += FrameworkElement_FirstLoaded;

                    return r;
                }, true, true);
            }
            public static cweeTask<FrameworkElement> GetFrameworkElement_TextBlock(SharedNodeResult res)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return new TextBlock(); }, true);
                        actualContainer.ContinueWith(() =>
                        {
                            var frameworkJob = SetFrameworkElement(res, actualContainer.Result as TextBlock);
                            frameworkJob.ContinueWith(() =>
                            {
                                TextBlock R = frameworkJob.Result;
                                {
                                    var customJob = res.result.CustomizableQueryResult_Cast_VectorString(
                                        "return [" +
                                        "\"${ %s.TextWrapping }\"" +
                                        ", \"${ %s.TextTrimming }\"" +
                                        ", \"${ %s.TextAlignment }\"" +
                                        ", \"${ %s.Text }\"" +
                                        ", \"${ %s.Padding }\"" +
                                        ", \"${ %s.FontSize }\"" +
                                        ", \"${ %s.HorizontalTextAlignment }\"" +
                                        "]"
                                    , "%s", res.additionalParams);

                                    var foreground = GetColor(new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".Foreground" });

                                    EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() {
                                    foreground,
                                    customJob
                                }).ContinueWith(() =>
                                {
                                    R.Foreground = new SolidColorBrush(foreground.Result);
                                    { if (Enum.TryParse<TextWrapping>(customJob.Result[0], out TextWrapping w)) { R.TextWrapping = w; } }
                                    { if (Enum.TryParse<TextTrimming>(customJob.Result[1], out TextTrimming w)) { R.TextTrimming = w; } }
                                    { if (Enum.TryParse<TextAlignment>(customJob.Result[2], out TextAlignment w)) { R.TextAlignment = w; } }
                                    R.Text = customJob.Result[3];
                                    { if (TryStringToThickness(customJob.Result[4], out Thickness w)) { R.Padding = w; } }
                                    { if (double.TryParse(customJob.Result[5], out double w) && w != -1.0) { R.FontSize = w; } }
                                    { if (Enum.TryParse<TextAlignment>(customJob.Result[6], out TextAlignment w)) { R.HorizontalTextAlignment = w; } }

                                    r.Child = R; // DONE
                                }, true);
                                }
                            }, false);
                        }, false);
                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
                    r.Loaded += FrameworkElement_FirstLoaded;
#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;

                        foreach (var task in tasks)
                        {
                            switch (task)
                            {
                                case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                            }
                        }
                    });
#endif
                    return r;
                }, true, true);
            }
            public static cweeTask<FrameworkElement> GetFrameworkElement_Image(SharedNodeResult res)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return new Image(); }, true);
                        actualContainer.ContinueWith(() =>
                        {
                            var frameworkJob = SetFrameworkElement(res, actualContainer.Result as Image);
                            frameworkJob.ContinueWith(() =>
                            {
                                Image R = frameworkJob.Result;
                                {
                                    var customJob = res.result.CustomizableQueryResult_Cast_VectorString(
                                        "return [" +
                                        "\"${ %s.ImagePath }\"" +
                                        ", \"${ %s.Stretch }\"" +
                                        ", \"${ %s.ImagePixels.size() }\"" +
                                        ", \"${ %s.ImagePixelsWidth }\"" +
                                        ", \"${ %s.ImagePixelsHeight }\"" +
                                        "]"
                                    , "%s", res.additionalParams);

                                    EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() {
                                    customJob
                                }).ContinueWith(() =>
                                {
                                    R.RightTapped += (object sender, RightTappedRoutedEventArgs e) =>
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
                                                        Functions.CopyToClipboard((sender as Image));
                                                    }, true, true);
                                                };
                                                tempGrid.Children.Add(but);
                                            }
                                        }
                                        var flyout = (sender as Image).SetFlyout(tempGrid, (sender as Image), tempGrid, e.GetPosition((sender as Image)));
                                        e.Handled = true;
                                        (sender as Image).ContextFlyout.LightDismissOverlayMode = LightDismissOverlayMode.On;
                                    };









                                    { if (Enum.TryParse<Stretch>(customJob.Result[1], out Stretch w)) { R.Stretch = w; } }
                                    if (customJob.Result[0].Length > 0)
                                    {
                                        R.Source = new Windows.UI.Xaml.Media.Imaging.BitmapImage(new Uri(customJob.Result[0]));
                                        r.Child = R; // DONE
                                    }
                                    else
                                    {
                                        if (
                                            int.TryParse(customJob.Result[2], out int w) && w > 0
                                            && int.TryParse(customJob.Result[3], out int pixelsWidth) && pixelsWidth > 0
                                            && int.TryParse(customJob.Result[4], out int pixelsHeight) && pixelsHeight > 0
                                            ) {
                                            var customJob2 = res.result.CustomizableQueryResult_Cast_VectorFloat(
                                                "%s.ImagePixels"
                                            , "%s", res.additionalParams);
                                            customJob2.ContinueWith(() => {
                                                byte[] bytes = new byte[customJob2.Result.Count];
                                                for (int i = 0; i < bytes.Length; i++)
                                                {
                                                    bytes[i] = (byte)(MathF.Min(255, MathF.Max(0, customJob2.Result[i])));
                                                }
                                                var imageTask = cweeXamlHelper.PixelsToImage(bytes, pixelsWidth, pixelsHeight, R);
                                                imageTask.ContinueWith(()=> {                                                    
                                                    r.Child = imageTask.Result; // DONE
                                                }, true);                                                
                                            }, false);                                            
                                        }
                                        else
                                        {
                                            r.Child = R; // DONE
                                        }
                                    }
                                }, true);
                                }
                            }, false);
                        }, false);
                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
                    r.Loaded += FrameworkElement_FirstLoaded;
#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;

                        foreach (var task in tasks)
                        {
                            switch (task)
                            {
                                case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                            }
                        }
                    });
#endif
                    return r;
                }, true, true);
            }
            public static cweeTask<FrameworkElement> GetFrameworkElement_WebView(SharedNodeResult res)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return new WebView(); }, true);
                        actualContainer.ContinueWith(() =>
                        {
                            var frameworkJob = SetFrameworkElement(res, actualContainer.Result as WebView);
                            frameworkJob.ContinueWith(() =>
                            {
                                WebView R = frameworkJob.Result;
                                {
                                    var customJob = res.result.CustomizableQueryResult_Cast_VectorString(
                                        "return [" +
                                        "\"${ %s.Source }\"" +
                                        "]"
                                    , "%s", res.additionalParams);

                                    EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() {
                                    customJob
                                }).ContinueWith(() =>
                                {
                                    R.Source = new Uri((customJob.Result[0]) as string);
                                    R.NavigationCompleted += (WebView sender, WebViewNavigationCompletedEventArgs args) =>
                                    {
                                        res.result.QueryResult(res.additionalParams + $".Source = \"{args.Uri.AbsoluteUri}\"");
                                        res.result.QueryResult(res.additionalParams + ".NavigationCompleted()");
                                    };

                                    r.Child = R; // DONE
                                }, true);
                                }
                            }, false);
                        }, false);
                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
                    r.Loaded += FrameworkElement_FirstLoaded;
#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;

                        foreach (var task in tasks)
                        {
                            var task_params = task.SplitNum(" ", 1);
                            if (task_params.Count > 0)
                            {                                
                                switch (task_params[0])
                                {
                                    case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                    case "Navigate": {
                                            var R = EdmsTasks.InsertJob(() => { return r.Child as WebView; }, true);
                                            R.ContinueWith(()=> {
                                                (R.Result as WebView).Navigate(new Uri((task_params[1]) as string));
                                            }, true);
                                            break; }
                                    case "InvokeScript": {
                                            var R = EdmsTasks.InsertJob(() => { return r.Child as WebView; }, true);
                                            R.ContinueWith(() => {
                                                var invokeParams = task_params[1].SplitNum(" ", 1);
                                                if (invokeParams.Count > 0)
                                                {
                                                    var invokeJob = EdmsTasks.InsertJob(async () =>
                                                    {
                                                        string result;
                                                        try
                                                        {
                                                            result = await (R.Result as WebView).InvokeScriptAsync("eval", new string[] { invokeParams[1] });
                                                        }
                                                        catch (Exception e)
                                                        {
                                                            result = e.Message;
                                                        }
                                                        return result;
                                                    }, true, true);
                                                    invokeJob.ContinueWith(() => {
                                                        res.result.QueryResult(res.additionalParams + $".ResultQueue[\"{invokeParams[0]}\"](\"{invokeJob.Result}\")"); // try call the return func with the result
                                                        res.result.QueryResult(res.additionalParams + $".ResultQueue[\"{invokeParams[0]}\"]()"); // try call the return func without the result
                                                        res.result.QueryResult(res.additionalParams + $".ResultQueue.erase(\"{invokeParams[0]}\")"); // erase the return location
                                                    }, false);
                                                }
                                            }, false);
                                            break; }
                                    default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                                }
                            }
                        }
                    });
#endif
                    return r;
                }, true, true);
            }

            private static cweeTask<T> SetPanelElement<T>(SharedNodeResult res, T R) where T : Panel // Grids and Stackpanels all have "padding"
            {
                var FrameworkJob = res.result.CustomizableQueryResult_Cast_VectorString(
                    "return [" +
                    "\"${ %s.Padding }\"" +
                    ", \"${ %s.Children.size() }\"" +
                    ", \"${ %s.BorderThickness }\"" +
                    "]"
                    , "%s", res.additionalParams);
                var bg = GetColor(new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".Background" });
                var borderBrushColor = GetColor(new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".BorderBrush" });

                return EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() {
                    FrameworkJob
                    , bg, borderBrushColor
                }).ContinueWith(() => {
                    List<string> frameworkParms = FrameworkJob.Result;
                    { if (TryStringToThickness(frameworkParms[0], out Thickness t)) { if (R is StackPanel) (R as StackPanel).Padding = t; if (R is Grid) (R as Grid).Padding = t; } }
                    R.Background = new SolidColorBrush(bg.Result);
                    if (R is Grid)
                    {
                        (R as Grid).BorderBrush = new SolidColorBrush(borderBrushColor.Result);
                        if (TryStringToThickness(frameworkParms[2], out Thickness t)) (R as Grid).BorderThickness = t;
                    }
                    if (R is StackPanel)
                    {
                        (R as StackPanel).BorderBrush = new SolidColorBrush(borderBrushColor.Result);
                        if (TryStringToThickness(frameworkParms[2], out Thickness t)) (R as StackPanel).BorderThickness = t;
                    }

                    { 
                        if (double.TryParse(frameworkParms[1], out double w) && w != -1.0) { 
                            if (w > 0) {
                                var childrenJobs = new List<EdmsTasks.cweeTask>();                                
                                for (int i = 0; i < w; i++)
                                {
                                    // load the children...
                                    childrenJobs.Add(GetNodeContent(new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + $".Children[{i}]" }));
                                }
                                return EdmsTasks.cweeTask.TrueWhenCompleted(childrenJobs).ContinueWith(()=> {
                                    foreach (var childJob in childrenJobs)
                                    {
                                        R.Children.Add(childJob.Result);
                                    }
                                    return R;
                                }, true);
                            }                        
                        } 
                    }
                    return R;
                }, true);
            }

            public static cweeTask<FrameworkElement> GetFrameworkElement_Grid(SharedNodeResult res)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return new Grid(); }, true);
                        actualContainer.ContinueWith(() =>
                        {
                            var frameworkJob = SetFrameworkElement(res, actualContainer.Result as Grid);
                            frameworkJob.ContinueWith(() =>
                            {
                                Grid R = frameworkJob.Result;
                                var panelJob = SetPanelElement(res, R);
                                panelJob.ContinueWith(() =>
                                {
                                    // loaded the children by now as well.
                                    var ColumnDefinitions_Job = res.result.CustomizableQueryResult_Cast_VectorString("return %s.ColumnDefinitions", "%s", res.additionalParams);
                                    var RowDefinitions_Job = res.result.CustomizableQueryResult_Cast_VectorString("return %s.RowDefinitions", "%s", res.additionalParams);
                                    var ChildPositions_Job = res.result.CustomizableQueryResult_Cast_VectorString("return %s.ChildPositions", "%s", res.additionalParams);
                                    var customJob = res.result.CustomizableQueryResult_Cast_VectorString(
                                        "return [" +
                                        "\"${ %s.ColumnSpacing }\"" +
                                        ", \"${ %s.RowSpacing }\"" +
                                        "]"
                                    , "%s", res.additionalParams);

                                    EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() {
                                    ColumnDefinitions_Job
                                    , RowDefinitions_Job
                                    , ChildPositions_Job
                                    , customJob
                                }).ContinueWith(() =>
                                {
                                    List<string> ColumnDefinitions = ColumnDefinitions_Job.Result; // i.e. 1* or *5 or Auto
                                    List<string> RowDefinitions = RowDefinitions_Job.Result;
                                    List<string> ChildPositions = ChildPositions_Job.Result;

                                    for (int i = 0; i < ColumnDefinitions.Count; i++)
                                    {
                                        if (TryStringToGridLength(ColumnDefinitions[i], out GridLength rr))
                                        {

                                            R.ColumnDefinitions.Add(new ColumnDefinition() { Width = rr });

                                        }
                                    }
                                    for (int i = 0; i < RowDefinitions.Count; i++)
                                    {
                                        if (TryStringToGridLength(RowDefinitions[i], out GridLength rr))
                                        {
                                            R.RowDefinitions.Add(new RowDefinition() { Height = rr });

                                        }
                                    }

                                    int minIndexes = Math.Min(R.Children.Count, ChildPositions.Count);
                                    for (int i = 0; i < minIndexes; i++)
                                    {
                                        var row_col_rowSpan_colSpan = ChildPositions[i].Split(",");
                                        if (row_col_rowSpan_colSpan.Length >= 1) Grid.SetRow(R.Children[i] as FrameworkElement, int.Parse(row_col_rowSpan_colSpan[0]));
                                        if (row_col_rowSpan_colSpan.Length >= 2) Grid.SetColumn(R.Children[i] as FrameworkElement, int.Parse(row_col_rowSpan_colSpan[1]));
                                        if (row_col_rowSpan_colSpan.Length >= 3) Grid.SetRowSpan(R.Children[i] as FrameworkElement, Math.Max(1, int.Parse(row_col_rowSpan_colSpan[2])));
                                        if (row_col_rowSpan_colSpan.Length >= 4) Grid.SetColumnSpan(R.Children[i] as FrameworkElement, Math.Max(1, int.Parse(row_col_rowSpan_colSpan[3])));
                                    }

                                    { if (double.TryParse(customJob.Result[0], out double w) && w != -1.0) { R.ColumnSpacing = w; } }
                                    { if (double.TryParse(customJob.Result[1], out double w) && w != -1.0) { R.RowSpacing = w; } }

                                    r.Child = R;
                                }, true);
                                }, false);
                            }, false);
                        }, false);
                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;

                        foreach (var task in tasks)
                        {
                            switch (task)
                            {
                                case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                            }
                        }
                    });
#endif
                    r.Loaded += FrameworkElement_FirstLoaded;

                    return r;
                }, true, true);
            }
            public static cweeTask<FrameworkElement> GetFrameworkElement_StackPanel(SharedNodeResult res)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return new StackPanel(); }, true);
                        actualContainer.ContinueWith(() =>
                        {
                            var frameworkJob = SetFrameworkElement(res, actualContainer.Result as StackPanel);
                            frameworkJob.ContinueWith(() =>
                            {
                                StackPanel R = frameworkJob.Result;
                                var panelJob = SetPanelElement(res, R);
                                panelJob.ContinueWith(() =>
                                {
                                    // loaded the children by now as well.
                                    var customJob = res.result.CustomizableQueryResult_Cast_VectorString(
                                        "return [" +
                                        "\"${ %s.Orientation }\"" +
                                        ", \"${ %s.Spacing }\"" +
                                        "]"
                                    , "%s", res.additionalParams);

                                    EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() {
                                    customJob
                                }).ContinueWith(() =>
                                {
                                    { if (Enum.TryParse<Orientation>(customJob.Result[0], out Orientation w)) { R.Orientation = w; } }
                                    { if (double.TryParse(customJob.Result[1], out double w) && w != -1.0) { R.Spacing = w; } }
                                    r.Child = R;
                                }, true);
                                }, false);
                            }, false);
                        }, false);
                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;

                        foreach (var task in tasks)
                        {
                            switch (task)
                            {
                                case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                            }
                        }
                    });
#endif
                    r.Loaded += FrameworkElement_FirstLoaded;

                    return r;
                }, true, true);
            }
            public static cweeTask<FrameworkElement> GetFrameworkElement_Map(SharedNodeResult res)
            {
                cweeTask<List<string>> typeCheck = res.result.CustomizableQueryResult_Cast_VectorString("return [" +
                    "\"${%s.is_type(\"UI_Map\") ? 1 : 0}\"" +
                    ", \"${%s.is_type(\"UI_MapLayer\") ? 1 : 0}\"" +
                "];", "%s", res.additionalParams);
                return (EdmsTasks.cweeTask)typeCheck.ContinueWith(() => {
                    List<string> result = typeCheck.Result;
                    for (int j = 0; j < result.Count; j++)
                    {
                        var tc = result[j];
                        if (int.TryParse(tc, out int isValid) && isValid == 1)
                        {
                            switch (j)
                            {
                                case 0: // UI_Map
                                    {
                                        return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() =>
                                        {
                                            var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };
                                            cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                                            {
                                                var queryTask = res.result.QueryResult(res.additionalParams + ".Layers.size()");
                                                queryTask.ContinueWith(() => {
                                                    if (double.TryParse(queryTask.Result, out double numChildren))
                                                    {
                                                        var queryTask2 = res.result.QueryResult(res.additionalParams + ".Backgrounds.size()");
                                                        return queryTask2.ContinueWith(() =>
                                                        {
                                                            if (int.TryParse(queryTask2.Result, out int numBackgrounds))
                                                            {
                                                                cweeTask<SimpleMap> obj = EdmsTasks.InsertJob(() => {
                                                                    var JJ = new SimpleMap();
                                                                    JJ.Loaded += HandleMapLoaded;
                                                                    return JJ;
                                                                }, true, true);
                                                                return obj.ContinueWith(() => {
                                                                    var tasks = new List<EdmsTasks.cweeTask>((int)(numChildren + 1));
                                                                    for (int childN = 0; childN < numChildren; childN++)
                                                                    {
                                                                        var toQuery = new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".Layers[" + childN.ToString() + "]" };
                                                                        tasks.Add(new EdmsTasks.cweeTask(() => {
                                                                            cweeTask<MapElementsLayer> newLayer = (EdmsTasks.cweeTask)GetNodeContent(toQuery);
                                                                            return newLayer.ContinueWith(() => {
#if UseFrameworkUpdater
                                                                                Framework_Updater.Subscribe(obj.Result as SimpleMap, toQuery, (object Res, List<string> tasks2) => {
                                                                                    SharedNodeResult query = Res as SharedNodeResult;
                                                                                    foreach (var task in tasks2)
                                                                                    {
                                                                                        var task_params = task.SplitNum(" ", 1);
                                                                                        if (task_params.Count > 0)
                                                                                        {
                                                                                            switch (task_params[0])
                                                                                            {
                                                                                                case "Update": { break; }
                                                                                                case "SetVisibility":
                                                                                                    {
                                                                                                        if (int.TryParse(task_params[1].Trim(), out int V))
                                                                                                        {
                                                                                                            if (V > 0)
                                                                                                            {
                                                                                                                EdmsTasks.InsertJob(() => {
                                                                                                                    newLayer.Result.Visible = true;
                                                                                                                }, true, true);
                                                                                                            }
                                                                                                            else
                                                                                                            {
                                                                                                                EdmsTasks.InsertJob(() => {
                                                                                                                    newLayer.Result.Visible = false;
                                                                                                                }, true, true);
                                                                                                            }
                                                                                                        }
                                                                                                        break;
                                                                                                    }
                                                                                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                });
#endif
                                                                                return newLayer.Result;
                                                                            }, true);
                                                                        }, false, true));
                                                                    }

                                                                    if (numBackgrounds > 0) {
                                                                        // the backgrounds should be inserted in-order.
                                                                        tasks.Add(new EdmsTasks.cweeTask(() =>
                                                                        {
                                                                            List<EdmsTasks.cweeTask> results = new List<EdmsTasks.cweeTask>();
                                                                            for (int bgN = 0; bgN < numBackgrounds; bgN++)
                                                                            {
                                                                                var toQuery = new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".Backgrounds[" + bgN.ToString() + "]" };
                                                                                results.Add((EdmsTasks.cweeTask)(toQuery.result.Query_MapBackground(toQuery.additionalParams)));
                                                                            }
                                                                            return EdmsTasks.cweeTask.TrueWhenCompleted(results, results);
                                                                        }, false, true));
                                                                    }

                                                                    return EdmsTasks.cweeTask.InsertListAsTask(tasks, true).ContinueWith(() => {
                                                                        var x = obj.Result.vm.map;
                                                                        foreach (var lay in tasks)
                                                                        {
                                                                            if (lay.Result is MapElementsLayer)
                                                                            {
                                                                                x.AddMapLayer(lay.Result as MapElementsLayer);
                                                                            }
                                                                            else if (lay.Result is List<EdmsTasks.cweeTask>)
                                                                            {
                                                                                var streamStopper = new AtomicInt(0);

                                                                                obj.Result.Loaded += (object sender, RoutedEventArgs e) => {
                                                                                    streamStopper.Set(0);
                                                                                };
                                                                                obj.Result.Unloaded += (object sender, RoutedEventArgs e) => {
                                                                                    streamStopper.Set(1);
                                                                                };

                                                                                List<MapBackground_Interop> backgrounds = new List<MapBackground_Interop>();
                                                                                
                                                                                foreach (EdmsTasks.cweeTask tsk in (lay.Result as List<EdmsTasks.cweeTask>)) {
                                                                                    backgrounds.Add(tsk.Result);
                                                                                }

                                                                                x.StreamBackground(backgrounds, streamStopper);
                                                                            }                                                                            
                                                                        }

                                                                        obj.Result.vm.map.Map_CenterView(MapAnimationKind.None);
                                                                        obj.Result.vm.map.map.MapProjection = MapProjection.Globe;

                                                                        r.Child = obj.Result;
                                                                    }, true);
                                                                }, false);
                                                            }
                                                            return null;
                                                        }, false);


                                                    }
                                                    return null;
                                                }, false);


                                            }, false); // expect this to be called shortly and to have its queue revised
                                            r.Tag = updateDequeue;
#if UseFrameworkUpdater
                                            Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks2) =>
                                            {
                                                SharedNodeResult query = Res as SharedNodeResult;
                                                foreach (var task in tasks2)
                                                {
                                                    switch (task)
                                                    {
                                                        case "Update": { 
                                                                updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); 
                                                                break; 
                                                            }
                                                        default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                                                    }
                                                }
                                            });
#endif
                                            r.Loaded += FrameworkElement_FirstLoaded;
                                            return r;
                                        }, true, true);
                                    }
                                case 1: // UI_MapLayer
                                    {
                                        var Layer = res.result.Query_MapLayer(res.additionalParams);
                                        return Layer.ContinueWith(()=> {
                                            MapLayer_Interop layer = Layer.Result;
                                            var tasks = new List<EdmsTasks.cweeTask>(layer.icons.Count + layer.polylines.Count + 1);
                                            cweeTask<MapElementsLayer> LAYER = EdmsTasks.InsertJob(() => {
                                                var L = new MapElementsLayer();
                                                return L;
                                            }, true, true);
                                            return LAYER.ContinueWith(()=> {
                                                foreach (var key in layer.polygons.Keys)
                                                {
                                                    if (layer.polygons.TryGetValue(key, out MapPolygon_Interop polygon))
                                                    {
                                                        var queryTag = new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + $".Children[{key}].Tag" };

                                                        Color fill = new Color() { R = (Byte)polygon.fill.R, G = (Byte)polygon.fill.G, B = (Byte)polygon.fill.B, A = (Byte)polygon.fill.A };
                                                        Color stroke = new Color() { R = (Byte)polygon.stroke.R, G = (Byte)polygon.stroke.G, B = (Byte)polygon.stroke.B, A = (Byte)polygon.stroke.A };
                                                        List<Windows.Devices.Geolocation.BasicGeoposition> path = new List<Windows.Devices.Geolocation.BasicGeoposition>();
                                                        bool StrokeDashed = polygon.dashed;
                                                        double StrokeThickness = polygon.thickness;
                                                        foreach (var coord in polygon.coordinates)
                                                        {
                                                            var coords = WaterWatch.ValidateCoordinates(coord.first, coord.second);
                                                            path.Add(new Windows.Devices.Geolocation.BasicGeoposition()
                                                            {
                                                                Longitude = coords.first,
                                                                Latitude = coords.second
                                                            });
                                                        }

                                                        tasks.Add(EdmsTasks.InsertJob(() => {
                                                            var obj = new MapPolygon();
                                                            {
                                                                obj.Visible = true;
                                                                obj.FillColor = fill;
                                                                obj.StrokeColor = stroke;
                                                                obj.StrokeDashed = StrokeDashed;
                                                                obj.StrokeThickness = StrokeThickness;
                                                                if (path.Count > 1) {
                                                                    obj.Path = new Windows.Devices.Geolocation.Geopath(path);
                                                                }
                                                                obj.Tag = queryTag;
                                                            }
                                                            return obj;
                                                        }, true, true));
                                                    }
                                                }
                                                foreach (var key in layer.polylines.Keys)
                                                {
                                                    if (layer.polylines.TryGetValue(key, out MapPolyline_Interop polyline))
                                                    {
                                                        var queryTag = new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + $".Children[{key}].Tag" };

                                                        Color c = new Color() { R = (Byte)polyline.color.R, G = (Byte)polyline.color.G, B = (Byte)polyline.color.B, A = (Byte)polyline.color.A };
                                                        List<Windows.Devices.Geolocation.BasicGeoposition> path = new List<Windows.Devices.Geolocation.BasicGeoposition>();
                                                        bool StrokeDashed = polyline.dashed;
                                                        double StrokeThickness = polyline.thickness;
                                                        foreach (var coord in polyline.coordinates)
                                                        {
                                                            var coords = WaterWatch.ValidateCoordinates(coord.first, coord.second);
                                                            path.Add(new Windows.Devices.Geolocation.BasicGeoposition() {
                                                                Longitude = coords.first , Latitude = coords.second
                                                            });
                                                        }

                                                        tasks.Add(EdmsTasks.InsertJob(() => {
                                                            var obj = new MapPolyline();
                                                            {
                                                                obj.Visible = true;
                                                                obj.StrokeColor = c;
                                                                obj.StrokeDashed = StrokeDashed;
                                                                obj.StrokeThickness = StrokeThickness;
                                                                if (path.Count > 1) { obj.Path = new Windows.Devices.Geolocation.Geopath(path); }
                                                                obj.Tag = queryTag;
                                                            }
                                                            return obj;
                                                        }, true, true));
                                                    }
                                                }
                                                foreach (var key in layer.icons.Keys)
                                                {
                                                    if (layer.icons.TryGetValue(key, out MapIcon_Interop icon))
                                                    {
                                                        var queryTag = new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + $".Children[{key}].Tag" };

                                                        Color c = new Color() { R = (Byte)icon.color.R, G = (Byte)icon.color.G, B = (Byte)icon.color.B, A = (Byte)icon.color.A };
                                                        KeyValuePair<double, double> longLat = new KeyValuePair<double, double>(icon.longitude, icon.latitude);
                                                        KeyValuePair<double, string> size_path = new KeyValuePair<double, string>(icon.size, icon.IconPathGeometry);
                                                        string Label = icon.Label;
                                                        tasks.Add(new EdmsTasks.cweeTask(()=> {
                                                            cweeTask<Windows.Storage.Streams.RandomAccessStreamReference> iconTask;
                                                            if (string.IsNullOrEmpty(size_path.Value)) {
                                                                iconTask = cweeMap.GetMapIconImage(c, size_path.Key);
                                                            } else {
                                                                iconTask = cweeMap.GetMapIconImage(c, size_path.Key, size_path.Value);
                                                            }
                                                            Windows.Devices.Geolocation.Geopoint geop;
                                                            try
                                                            {
                                                                var coords = WaterWatch.ValidateCoordinates(longLat.Key, longLat.Value);
                                                                geop = new Windows.Devices.Geolocation.Geopoint(new Windows.Devices.Geolocation.BasicGeoposition() { 
                                                                    Longitude = coords.first, Latitude = coords.second 
                                                                });
                                                            }
                                                            catch (Exception)
                                                            {
                                                                try
                                                                {
                                                                    geop = new Windows.Devices.Geolocation.Geopoint(new Windows.Devices.Geolocation.BasicGeoposition() {
                                                                        Longitude = longLat.Key,
                                                                        Latitude = longLat.Value
                                                                    });
                                                                }
                                                                catch (Exception) {
                                                                    geop = new Windows.Devices.Geolocation.Geopoint(new Windows.Devices.Geolocation.BasicGeoposition() {
                                                                        Longitude = 0,
                                                                        Latitude = 0
                                                                    });
                                                                }
                                                            }
                                                            return iconTask.ContinueWith(() => {
                                                                var mapIcon = new MapIcon() {
                                                                    Image = iconTask.Result,
                                                                    CollisionBehaviorDesired = icon.HideOnCollision ? MapElementCollisionBehavior.Hide : MapElementCollisionBehavior.RemainVisible,
                                                                    Location = geop,
                                                                    Tag = queryTag                                                                    
                                                                };
                                                                if (!string.IsNullOrEmpty(Label)) {
                                                                    mapIcon.Title = Label;
                                                                }
                                                                return mapIcon;
                                                            }, true);
                                                        }, false, true));
                                                    }
                                                }
                                                return EdmsTasks.cweeTask.InsertListAsTask(tasks).ContinueWith(() => {
                                                    foreach (var elem in tasks) {
                                                        LAYER.Result.MapElements.Add(elem.Result as MapElement);
                                                    }
                                                    return LAYER.Result;
                                                }, true);
                                            }, false);
                                        }, false);
                                    }
                            }
                        }
                    }
                    return null;
                }, false);
            }

            private static void HandleMapClick(Windows.UI.Xaml.Controls.Maps.MapControl mapSender, Windows.UI.Xaml.Controls.Maps.MapElementClickEventArgs mapArgs)
            {
                foreach (var margObj in mapArgs.MapElements)
                {
                    if (margObj.Tag != null && margObj.Tag is SharedNodeResult)
                    {
                        var objQueriable = margObj.Tag as SharedNodeResult;
                        var getContent = GetNodeContent(objQueriable);
                        getContent.ContinueWith(() =>
                        {
                            FrameworkElement toPresent = getContent.Result;
                            var flyout = mapSender.SetFlyout(toPresent, mapSender, mapSender, mapArgs.Position);
                        }, true);
                        break;
                    }
                }
            }
            private static void HandleMapLoaded(object sender, RoutedEventArgs e) {
                (sender as SimpleMap).Loaded -= HandleMapLoaded;
                (sender as SimpleMap).vm.map.map.MapElementClick += HandleMapClick;
            }

            public static cweeTask<FrameworkElement> GetFrameworkElement_Button(SharedNodeResult res) {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() => {
                    var r = new Border() { HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, Child = GenericLoadingRing() };

                    cweeDequeue updateDequeue = new cweeDequeue(DateTime.Now.AddSeconds(60), () =>
                    {
                        var actualContainer = EdmsTasks.InsertJob(() => { return new Button(); }, true);
                        actualContainer.ContinueWith(() =>
                        {
                            var frameworkJob = SetControlElement(res, actualContainer.Result as Button);
                            frameworkJob.ContinueWith(() =>
                            {
                                Button R = frameworkJob.Result;
                                R.Content = GenericLoadingRing();
                                R.Click += (object sender, RoutedEventArgs e)=> {
                                    res.result.CustomizableQueryResult("%s.Clicked()", "%s", res.additionalParams);
                                };
                                r.Child = R;

                                var childJob = GetNodeContent(new SharedNodeResult() { result = res.result, additionalParams = res.additionalParams + ".Content" });
                                childJob.ContinueWith(()=> {
                                    R.Content = childJob.Result;
                                }, true);
                            }, true);
                        }, false);
                    }, false); // expect this to be called shortly and to have its queue revised
                    r.Tag = updateDequeue;
                    r.Loaded += FrameworkElement_FirstLoaded;
                    

#if UseFrameworkUpdater
                    Framework_Updater.Subscribe(r, res, (object Res, List<string> tasks) => {
                        SharedNodeResult query = Res as SharedNodeResult;



                        foreach (var task in tasks)
                        {
                            switch (task)
                            {
                                case "Update": { updateDequeue.Dequeue(DateTime.Now.AddSeconds(1.0 / 60.0)); break; }
                                //case "Clicked": { 
                                //    // the button was clicked.. call it
                                //    var clicked_job = res.result.CustomizableQueryResult("%s.Clicked()", "%s", res.additionalParams);
                                //    break; 
                                //}
                                default: WaterWatch.SubmitToast("Failed to Parse UI Task at " + System.Reflection.MethodBase.GetCurrentMethod().Name, task); break;
                            }
                        }
                    });
#endif
                    return r;
                }, true, true);
            }

            public static cweeTask<FrameworkElement> GetFrameworkElement_CheckBox(SharedNodeResult res) { return GetDefaultContent(res); }
            public static cweeTask<FrameworkElement> GetFrameworkElement_Slider(SharedNodeResult res) { return GetDefaultContent(res); }
            public static cweeTask<FrameworkElement> GetFrameworkElement_ToggleSwitch(SharedNodeResult res) { return GetDefaultContent(res); }
            public static cweeTask<FrameworkElement> GetFrameworkElement_ListView(SharedNodeResult res) { return GetDefaultContent(res); }
            public static cweeTask<FrameworkElement> GetFrameworkElement_TabView(SharedNodeResult res) { return GetDefaultContent(res); }
            public static cweeTask<FrameworkElement> GetFrameworkElement_Expander(SharedNodeResult res) { return GetDefaultContent(res); }
            public static cweeTask<FrameworkElement> GetFrameworkElement_TextBox(SharedNodeResult res) { return GetDefaultContent(res); }
            public static cweeTask<FrameworkElement> GetFrameworkElement_Plot(SharedNodeResult res) { return GetDefaultContent(res); }


        }

        public class CurveStreamingSource : cweeDeferredIIncrementalSource<FrameworkElement>
        {
            // internal static ListView viewer;
            internal List<FrameworkElement> toReturn = new List<FrameworkElement>();
            internal List<string> keys = new List<string>();

            public CurveStreamingSource()
            {

            }

            public cweeTask<IEnumerable<FrameworkElement>> GetPagedItemsAsync(int pageIndex, int pageSize, object Tag = null)
            {
                return GetPagedItems(pageIndex, pageSize, Tag);
            }

            internal cweeTask<IEnumerable<FrameworkElement>> GetPagedItems(int pageIndex, int pageSize, object Tag = null)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() =>
                {
                    (SharedNodeResult, int) v = ((SharedNodeResult, int))Tag;
                    for (int i = toReturn.Count; i < v.Item2; i++)
                    {
                        toReturn.Add(null);
                        keys.Add("");
                    }

                    int minIndex = pageIndex * pageSize;
                    int maxIndex = Math.Min(v.Item2, (pageIndex + 1) * pageSize);

                    List<EdmsTasks.cweeTask> tasksToDo = new List<EdmsTasks.cweeTask>();
                    for (int i = minIndex; i < maxIndex && i < toReturn.Count; i++)
                    {
                        string j = i.ToString();
                        tasksToDo.Add(new EdmsTasks.cweeTask(() => {
                            var t1 = v.Item1.result.CustomizableQueryResult($"__RESULT__.keys[{j}]", "__RESULT__", v.Item1.additionalParams);
                            return t1.ContinueWith(() => {
                                keys[int.Parse(j)] = t1.Result;
                            }, true);
                        }, false, true));
                    }

                    return EdmsTasks.cweeTask.InsertListAsTask(tasksToDo).ContinueWith(() => {
                        var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                        return cdb.ContinueWith(() => {
                            var waitingList = new List<EdmsTasks.cweeTask>();

                            for (int i = minIndex; i < maxIndex && i < toReturn.Count; i++)
                            {
                                var key = keys[i];

                                var newC = new SharedNodeResult() { result = v.Item1.result, additionalParams = v.Item1.additionalParams + $".GetValueKnotSeries()[{i}]" };
                                var index = $"{i}";

                                var secondTask = new EdmsTasks.cweeTask(() => {
                                    Border border = new Border() { Padding = new Thickness(0), Margin = new Thickness(0), BorderBrush = cdb.Result, BorderThickness = new Thickness(0, 1, 0, 1), HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, MinWidth = 200, MinHeight = 40, MaxHeight = 200 };
                                    border.Child = new Microsoft.UI.Xaml.Controls.ProgressRing() { Width = 24, Height = 24, Padding = new Thickness(0), Margin = new Thickness(0), Foreground = cdb.Result, IsIndeterminate = true };

                                    Grid container = new Grid() { Padding = new Thickness(0), Margin = new Thickness(0), HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch };
                                    {
                                        container.ColumnSpacing = 5;

                                        container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(80, GridUnitType.Pixel) });
                                        container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Pixel) });
                                        container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Auto) });

                                        container.Children.Add(new TextBlock() { Margin = new Thickness(0), Padding = new Thickness(0), Text = $"{ key }", TextWrapping = TextWrapping.Wrap, HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center, HorizontalTextAlignment = TextAlignment.Center, Style = cweeXamlHelper.StaticStyleResource("cweeTextBlock") });

                                        var g = new Grid() { /*Background = cweeXamlHelper.ThemeColor("cweePageBackground")*/ };
                                        container.Children.Add(g);
                                        Grid.SetColumn(g, 1);

                                        container.Children.Add(border);
                                        Grid.SetColumn(border, 2);
                                    }
                                    toReturn[int.Parse(index)] = container;
                                    return border;
                                }, true, true);

                                secondTask.Tag = newC;

                                waitingList.Add(secondTask);
                            }

                            var waiter = EdmsTasks.cweeTask.InsertListAsTask(waitingList);
                            var lastTask = waiter.ContinueWith(() => {
                                var result = (from p in toReturn select p).Skip(pageIndex * pageSize).Take(pageSize);
                                return result;
                            }, true);

                            lastTask.ContinueWith(() => {
                                foreach (var tsk in waitingList)
                                {
                                    var Tsk = tsk;
                                    var mainTsk = GetNodeContent(Tsk.Tag as SharedNodeResult);
                                    mainTsk.ContinueWith(() => {
                                        try
                                        {
                                            (Tsk.Result as Border).Child = (mainTsk.Result as UIElement);
                                        }
                                        catch (Exception) { }
                                    }, true);
                                }
                            }, false);

                            return lastTask;
                        }, false);
                    }, false);
                });
            }
        }
        public class MapStreamingSource : cweeDeferredIIncrementalSource<FrameworkElement>
        {
            // internal static ListView viewer;
            internal List<FrameworkElement> toReturn = new List<FrameworkElement>();
            internal List<string> keys = new List<string>();

            public MapStreamingSource()
            {

            }

            public cweeTask<IEnumerable<FrameworkElement>> GetPagedItemsAsync(int pageIndex, int pageSize, object Tag = null)
            {
                return GetPagedItems(pageIndex, pageSize, Tag);
            }

            internal cweeTask<IEnumerable<FrameworkElement>> GetPagedItems(int pageIndex, int pageSize, object Tag = null)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() =>
                {
                    (SharedNodeResult, int) v = ((SharedNodeResult, int))Tag;
                    for (int i = toReturn.Count; i < v.Item2; i++)
                    {
                        toReturn.Add(null);
                        keys.Add("");
                    }

                    int minIndex = pageIndex * pageSize;
                    int maxIndex = Math.Min(v.Item2, (pageIndex + 1) * pageSize);

                    List<EdmsTasks.cweeTask> tasksToDo = new List<EdmsTasks.cweeTask>();
                    for (int i = minIndex; i < maxIndex && i < toReturn.Count; i++)
                    {
                        //for (int i = toReturn.Count; i < v.Item2; i++)
                        //{
                        string j = i.ToString();
                        tasksToDo.Add(new EdmsTasks.cweeTask(() => {
                            var t1 = v.Item1.result.CustomizableQueryResult(
                                "{" +
                                $"var j = 0;\n for (keyValuePair : __RESULT__)\n" +
                                "{" +
                                $"if (j == {j})\n" +
                                "{ return keyValuePair.first.to_string(); }\n " +
                                "\n++j;\n}" +
                                "}",
                                "__RESULT__", v.Item1.additionalParams
                            );
                            return t1.ContinueWith(() => {
                                keys[int.Parse(j)] = t1.Result;
                            }, true);
                        }, false, true));
                    }

                    return EdmsTasks.cweeTask.InsertListAsTask(tasksToDo).ContinueWith(() => {
                        var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                        return cdb.ContinueWith(()=> {
                            var waitingList = new List<EdmsTasks.cweeTask>();

                            for (int i = minIndex; i < maxIndex && i < toReturn.Count; i++)
                            {
                                var key = keys[i];

                                var newC = new SharedNodeResult() { result = v.Item1.result, additionalParams = v.Item1.additionalParams + $"[\"{key}\"]" };
                                var index = $"{i}";

                                var secondTask = new EdmsTasks.cweeTask(() => {
                                    Border border = new Border() { Padding = new Thickness(0), Margin = new Thickness(0), BorderBrush = cdb.Result, BorderThickness = new Thickness(0, 1, 0, 1), HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, MinWidth = 200, MinHeight = 40, MaxHeight = 200 };
                                    border.Child = new Microsoft.UI.Xaml.Controls.ProgressRing() { Width = 24, Height = 24, Padding = new Thickness(0), Margin = new Thickness(0), Foreground = cdb.Result, IsIndeterminate = true };

                                    Grid container = new Grid() { Padding = new Thickness(0), Margin = new Thickness(0), HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch };
                                    {
                                        container.ColumnSpacing = 5;

                                        container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(80, GridUnitType.Pixel) });
                                        container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Pixel) });
                                        container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Auto) });

                                        container.Children.Add(new TextBlock() { Margin = new Thickness(0), Padding = new Thickness(0), Text = $"{ key }", TextWrapping = TextWrapping.Wrap, HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center, HorizontalTextAlignment = TextAlignment.Center, Style = cweeXamlHelper.StaticStyleResource("cweeTextBlock") });

                                        var g = new Grid() { /*Background = cweeXamlHelper.ThemeColor("cweePageBackground")*/ };
                                        container.Children.Add(g);
                                        Grid.SetColumn(g, 1);

                                        container.Children.Add(border);
                                        Grid.SetColumn(border, 2);
                                    }
                                    toReturn[int.Parse(index)] = container;
                                    return border;
                                }, true, true);

                                secondTask.Tag = newC;

                                waitingList.Add(secondTask);
                            }

                            var waiter = EdmsTasks.cweeTask.InsertListAsTask(waitingList);
                            var lastTask = waiter.ContinueWith(() => {
                                var result = (from p in toReturn select p).Skip(pageIndex * pageSize).Take(pageSize);
                                return result;
                            }, true);

                            lastTask.ContinueWith(() => {
                                foreach (var tsk in waitingList)
                                {
                                    var Tsk = tsk;
                                    var mainTsk = GetNodeContent(Tsk.Tag as SharedNodeResult);
                                    mainTsk.ContinueWith(() => {
                                        try
                                        {
                                            (Tsk.Result as Border).Child = (mainTsk.Result as UIElement);
                                        }
                                        catch (Exception) { }
                                    }, true);
                                }
                            }, false);

                            return lastTask;
                        }, false);
                    }, false);
                });
            }
        }
        public class VectorStreamingSource : cweeDeferredIIncrementalSource<FrameworkElement>
        {
            // internal static ListView viewer;
            List<FrameworkElement> toReturn = new List<FrameworkElement>();
            public VectorStreamingSource()
            {

            }

            public cweeTask<IEnumerable<FrameworkElement>> GetPagedItemsAsync(int pageIndex, int pageSize, object Tag = null)
            {
                return GetPagedItems(pageIndex, pageSize, Tag);
            }

            internal cweeTask<IEnumerable<FrameworkElement>> GetPagedItems(int pageIndex, int pageSize, object Tag = null)
            {
                return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(() =>
                {
                    var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                    return cdb.ContinueWith(() =>
                    {
                        (SharedNodeResult, int) v = ((SharedNodeResult, int))Tag;

                        for (int i = toReturn.Count; i < v.Item2; i++)
                        {
                            toReturn.Add(null);
                        }

                        int minIndex = pageIndex * pageSize;
                        int maxIndex = Math.Min(v.Item2, (pageIndex + 1) * pageSize);


                        var waitingList = new List<EdmsTasks.cweeTask>();

                        for (int i = minIndex; i < maxIndex && i < toReturn.Count; i++)
                        {
                            var key = $"{i}";
                            var newC = new SharedNodeResult() { result = v.Item1.result, additionalParams = v.Item1.additionalParams + $"[{key}]" };
                            var index = $"{i}";

                            var secondTask = new EdmsTasks.cweeTask(() =>
                            {
                                Border border = new Border() { Padding = new Thickness(0), Margin = new Thickness(0), BorderBrush = cdb.Result, BorderThickness = new Thickness(0, 1, 0, 1), HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch, MinWidth = 200, MinHeight = 40, MaxHeight = 200 };
                                border.Child = new Microsoft.UI.Xaml.Controls.ProgressRing() { Width = 24, Height = 24, Padding = new Thickness(0), Margin = new Thickness(0), Foreground = cdb.Result, IsIndeterminate = true };

                                Grid container = new Grid() { Padding = new Thickness(0), Margin = new Thickness(0), HorizontalAlignment = HorizontalAlignment.Stretch, VerticalAlignment = VerticalAlignment.Stretch };
                                {
                                    container.ColumnSpacing = 5;

                                    container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(80, GridUnitType.Pixel) });
                                    container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Pixel) });
                                    container.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Auto) });

                                    container.Children.Add(new TextBlock() { Margin = new Thickness(0), Padding = new Thickness(0), Text = $"[{ key }]", TextWrapping = TextWrapping.Wrap, HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center, HorizontalTextAlignment = TextAlignment.Center, Style = cweeXamlHelper.StaticStyleResource("cweeTextBlock") });

                                    var g = new Grid() { /*Background = cweeXamlHelper.ThemeColor("cweePageBackground")*/ };
                                    container.Children.Add(g);
                                    Grid.SetColumn(g, 1);

                                    container.Children.Add(border);
                                    Grid.SetColumn(border, 2);
                                }
                                toReturn[int.Parse(index)] = container;
                                return border;
                            }, true, true);

                            secondTask.Tag = newC;

                            waitingList.Add(secondTask);
                        }

                        var waiter = EdmsTasks.cweeTask.InsertListAsTask(waitingList);
                        var lastTask = waiter.ContinueWith(() =>
                        {
                            var result = (from p in toReturn select p).Skip(pageIndex * pageSize).Take(pageSize);
                            return result;
                        }, true);

                        lastTask.ContinueWith(() =>
                        {
                            foreach (var tsk in waitingList)
                            {
                                var Tsk = tsk;
                                var mainTsk = GetNodeContent(Tsk.Tag as SharedNodeResult);
                                mainTsk.ContinueWith(() =>
                                {
                                    try
                                    {
                                        (Tsk.Result as Border).Child = (mainTsk.Result as UIElement);
                                    }
                                    catch (Exception) { }
                                }, true);
                            }
                        }, false);

                        return lastTask;
                    }, false);
                }, false, true);
            }
        }
    }

    public sealed partial class ScriptNode_Visualizer : UserControl
    {
        public static AtomicInt ObjCount = new AtomicInt();
        public ScriptNode_VisualizerVM vm = new ScriptNode_VisualizerVM();

        public ScriptNode_Visualizer(ScriptingNodeViewModel parentVM)
        {
            ObjCount.Increment();
            vm.ParentVM = parentVM;
            this.InitializeComponent();
            vm.ResultContainer = ResultContainer;
            vm.Reload();
        }
        ~ScriptNode_Visualizer() {
            ObjCount.Decrement();
            vm = null;
        }

    }
}