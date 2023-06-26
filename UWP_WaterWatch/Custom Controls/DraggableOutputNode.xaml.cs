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
using UWP_WaterWatchLibrary.Floating;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using static UWP_WaterWatchLibrary.Floating.FloatingContent;
using Microsoft.Toolkit.Uwp.UI;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    public class DraggableOutputNodeViewModel : ViewModelBase
    {
        public List<Windows.UI.Xaml.Shapes.Path> available_paths;
        public ScriptingNodeViewModel ParentVM = null;
        

        ~DraggableOutputNodeViewModel() {
            var job = EdmsTasks.InsertJob(()=> {
                if (available_paths != null)
                {
                    try
                    {
                        foreach (var j in available_paths)
                        {
                            if (j != null)
                            {
                                var tagC = j.Tag;
                                if (tagC != null)
                                {
                                    var data = ((int, string, string))tagC;
                                    SplineRedrawToConnectNodeLogic.pathAdjusterForAll?.RemoveAction(data.Item1);
                                    j.Tag = (-1, data.Item2, data.Item3);
                                }
                                else
                                {
                                    j.Tag = (-1, "", "");
                                }
                                Panel parent = (j.Parent as Panel);
                                if (parent != null)
                                {
                                    try
                                    {
                                        parent.Children.Remove(j);
                                    }
                                    catch (Exception) { }
                                }
                            }
                        }
                    }
                    catch (Exception) { }
                }
            }, true);

            while (!job.IsFinished)
            {
                EdmsTasks.DoJob();
            }

            available_paths = null;
            ParentVM = null;
        }
    }

    public sealed partial class DraggableOutputNode : UserControl
    {
        public Point PublicPagePosition = new Point(0,0);
        public DraggableOutputNodeViewModel vm = new DraggableOutputNodeViewModel();
        Windows.UI.Xaml.Shapes.Path currentPath = null;
        Point startPos;
        Point actualPos;
        Point actualEndPos;

        public DraggableOutputNode()
        {
            this.InitializeComponent();
        }
        ~DraggableOutputNode()
        {
            vm = null;
            currentPath = null;
        }

        public void ResetPosition() {
            floater.Boundary = FloatingBoundary.Parent;
            if (currentPath != null) currentPath.Visibility = Visibility.Collapsed;
            floater.SetPosition();
        }

        private Windows.UI.Xaml.Shapes.Path GetUnusedPath() {
            foreach (var p in vm.available_paths)
            {
                if (p.Visibility == Visibility.Collapsed)
                {
                    return p;
                }
            }
            return null;
        }
        private void DraggableNodeLoaded(object sender, RoutedEventArgs e)
        {
            (sender as FloatingContent).ManipulationStartedWWEvent += ManipulationStartedWWEvent;
            (sender as FloatingContent).ManipulationDeltaWWEvent += ManipulationDeltaWWEvent;
            (sender as FloatingContent).ManipulationEndedWWEvent += ManipulationEndedWWEvent;
        }
        private void DraggableNodeUnloaded(object sender, RoutedEventArgs e)
        {
            (sender as FloatingContent).ManipulationStartedWWEvent -= ManipulationStartedWWEvent;
            (sender as FloatingContent).ManipulationDeltaWWEvent -= ManipulationDeltaWWEvent;
            (sender as FloatingContent).ManipulationEndedWWEvent -= ManipulationEndedWWEvent;
        }
        private void ManipulationStartedWWEvent(object border, ManipulationStartedArgs e2)
        {
            startPos = e2.startingPosition;
            EdmsTasks.InsertJob(() => {
                floater.Boundary = FloatingBoundary.None;

                currentPath = GetUnusedPath();
                if (currentPath != null)
                {
                    var ttv = (border as FloatingContent).border.TransformToVisual(currentPath.Parent as UIElement);
                    actualPos = ttv.TransformPoint(startPos);

                    var pathGeo = BuildPath.BuildSmoothBezier(actualPos, actualPos);
                    currentPath.Data = pathGeo;
                    currentPath.Visibility = Visibility.Visible;
                    Canvas.SetZIndex(currentPath, 1000000);
                }
            }, true, true);
        }
        private void ManipulationDeltaWWEvent(object border, ManipulationDeltaArgs e2)
        {
            EdmsTasks.InsertJob(() =>
            {
                floater.Boundary = FloatingBoundary.None;
                if (currentPath != null)
                {
                    actualEndPos = new Point(actualPos.X + (e2.newPosition.X - startPos.X), actualPos.Y + (e2.newPosition.Y - startPos.Y));
                    var pathGeo = BuildPath.BuildSmoothBezier(actualPos, actualEndPos); //  startPos, e2.newPosition);
                    currentPath.Data = pathGeo;
                    currentPath.Visibility = Visibility.Visible;
                    Canvas.SetZIndex(currentPath, 1000000);
                }

            }, true, true);
        }
        private void ManipulationEndedWWEvent(object border, ManipulationEndedArgs e2)
        {
            EdmsTasks.InsertJob(() =>
            {
                floater.Boundary = FloatingBoundary.Parent;
                floater.SetPosition();
                if (currentPath != null)
                {
                    GeneralTransform gt = (currentPath.Parent as UIElement).TransformToVisual(Window.Current.Content);
                    Point pagePoint = gt.TransformPoint(actualEndPos);
                    var elements = Windows.UI.Xaml.Media.VisualTreeHelper.FindElementsInHostCoordinates(pagePoint, currentPath.Parent as UIElement);
                    foreach (var element in elements)
                    {
                        if (element is DroppableInputNode)
                        {
                            DroppableInputNode data_destination = element as DroppableInputNode;
                            if (data_destination?.vm?.ParentVM != null && this.vm.ParentVM != null)
                            {
                                Make_Connection(this, data_destination, currentPath);
                                currentPath = null;
                                return;
                            }
                        }
                    }
                    // no match found == hide the spline and allow it to be re-used later
                    currentPath.Visibility = Visibility.Collapsed;
                    currentPath = null;
                }
            }, true, true);
        }

        public static void Make_Connection(DraggableOutputNode node, DroppableInputNode data_destination, Windows.UI.Xaml.Shapes.Path path) {

            _ = new SplineRedrawToConnectNodeLogic(node, data_destination, path);
            data_destination.vm.ParentVM.AddInboundConnection(node.vm.ParentVM, data_destination.vm.VariableName);
            data_destination.vm.DataSourceUniqueName = node.vm.ParentVM.uniqueName;

            if (data_destination.IsLoaded)
            {
                data_destination.vm.ConnectedEvent.InvokeEvent(
                    node,
                    data_destination.vm
                );
            }
            else
            {
                data_destination.Loaded += node.Data_destination_Loaded;
            }
        }
        private void Data_destination_Loaded(object sender, RoutedEventArgs e)
        {
            (sender as DroppableInputNode).Loaded -= this.Data_destination_Loaded;
            (sender as DroppableInputNode).vm.ConnectedEvent.InvokeEvent(
                this,
                (sender as DroppableInputNode).vm
            );
        }

        /// <summary>
        /// Must be followed up by invoking the connection event.
        /// </summary>
        /// <param name="data_source"></param>
        /// <param name="data_destination"></param>
        /// <param name="path"></param>
        public static void Make_Connection_Immediately(DraggableOutputNode data_source, DroppableInputNode data_destination, Windows.UI.Xaml.Shapes.Path path)
        {
            //data_destination.vm.DataSourceUniqueName = data_source.vm.ParentVM.uniqueName;
            //data_destination.vm.ParentVM.AddInboundConnection(data_source.vm.ParentVM, data_destination.vm.VariableName);

            //path.Tag = ((cweeTimer, DroppableInputNode))(null, data_destination); 
            _ = new SplineRedrawToConnectNodeLogic(data_source, data_destination, path);
            data_destination.vm.DataSourceUniqueName = data_source.vm.ParentVM.uniqueName;
            data_destination.vm.ParentVM.AddInboundConnection(data_source.vm.ParentVM, data_destination.vm.VariableName);
        }
    }

    public class SplineRedrawToConnectNodeLogic
    {
        public static ScriptNode FindScriptNode(Windows.UI.Xaml.Shapes.Path path, string ownerNodeName)
        {
            if (path != null)
            {
                var panel = path.Parent as Panel;
                if (panel != null)
                {
                    try
                    {
                        foreach (ScriptNode x in panel.FindChildren().Where((FrameworkElement el) => { return (el != null) && (el is ScriptNode); }))
                        {
                            if (x != null && x.vm != null)
                            {
                                if (x.vm.uniqueName == ownerNodeName)
                                {
                                    return x;
                                }
                            }
                        }
                    }
                    catch (Exception) { }
                }
            }
            return null;
        }
        public static DraggableOutputNode FindDragNode(Windows.UI.Xaml.Shapes.Path path, string ownerNodeName)
        {
            return FindScriptNode(path, ownerNodeName)?.dragNode;
        }
        public static DroppableInputNode FindDropNode(Windows.UI.Xaml.Shapes.Path path, string ownerNodeName, string DropNodeUniqueID)
        {
            foreach (DroppableInputNode x in FindScriptNode(path, ownerNodeName)?.DroppableInputNodeList?.Children?.Where((UIElement x) => { return (x != null) && (x is DroppableInputNode) && (x as DroppableInputNode)?.vm?.UniqueDroppableInputNodeID == DropNodeUniqueID; })) {
                return x;
            }
            return null;
        }

        public static cweeAppendableTimer pathAdjusterForAll = new cweeAppendableTimer(0.25, false);
        public static AtomicInt pathAdjusterForAll_Counter = new AtomicInt(0);
        public SplineRedrawToConnectNodeLogic(DraggableOutputNode n1_orig, DroppableInputNode n2_orig, Windows.UI.Xaml.Shapes.Path path_orig)
        {
            path_orig.Visibility = Visibility.Visible;
            Point oldStartPos = new Point(0, 0);
            Point oldEndPos = new Point(0, 0);

            //System.Numerics.Vector3 startOffset = new System.Numerics.Vector3();
            //System.Numerics.Vector3 endOffset = new System.Numerics.Vector3();

            bool forceReload = false;
            UIElement pathParent = path_orig.Parent as UIElement;

            UIElement parentP = null;
            DraggableOutputNode n1 = null; // used as a local optimization cache
            DroppableInputNode n2 = null; // used as a local optimization cache

            string uniqueDragNodeParentName = n1_orig.vm.ParentVM.uniqueName;
            string uniqueDropNodeParentName = n2_orig.vm.ParentVM.uniqueName;
            string uniqueDropNodeID = n2_orig.vm.UniqueDroppableInputNodeID;

            path_orig.Loaded += (object sender, RoutedEventArgs e) =>
            {
                forceReload = true;
                AtomicInt localLock = new AtomicInt();

                int newInt = (int)pathAdjusterForAll_Counter.Increment();
                pathAdjusterForAll?.AddAction(() =>
                {
                    if (localLock.TryIncrementTo(1))
                    {
                        if (n1 == null)
                        {
                            EdmsTasks.InsertJob(() => {
                                n1 = FindDragNode((sender as Windows.UI.Xaml.Shapes.Path), uniqueDragNodeParentName);
                                localLock.Decrement();
                            }, true);
                            return;
                        }
                        if (n2 == null)
                        {
                            EdmsTasks.InsertJob(() => {
                                n2 = FindDropNode((sender as Windows.UI.Xaml.Shapes.Path), uniqueDropNodeParentName, uniqueDropNodeID);
                                localLock.Decrement();
                            }, true);
                            return;
                        }
                        //if (parentP == null)
                        //{
                        //    EdmsTasks.InsertJob(() => {
                        //        parentP = (sender as Windows.UI.Xaml.Shapes.Path).Parent as UIElement;
                        //        localLock.Decrement();
                        //    }, true);
                        //    return;
                        //}

                        //var offsetP1 = new Point(9, 5.5);
                        var startPos = n1.PublicPagePosition; /*EdmsTasks.InsertJob(() =>
                        {
                            try
                            {
                                if (parentP == null) return new Point(0, 0);
                                return n1.TransformToVisual(parentP).TransformPoint(offsetP1);
                            }
                            catch (Exception) { }
                            return new Point(0, 0);
                        }, true);*/
                        //var offsetP2 = new Point(1.5, 9.5);
                        var endPos = n2.PublicPagePosition; /* EdmsTasks.InsertJob(() =>
                        {
                            try
                            {
                                if (parentP == null) return new Point(0, 0);
                                return n2.TransformToVisual(parentP).TransformPoint(offsetP2);
                            }
                            catch (Exception) { }
                            return new Point(0, 0);
                        }, true);*/
                        //EdmsTasks.cweeTask.TrueWhenCompleted(new List<cweeTask<Point>>() { startPos, endPos }).ContinueWith(() =>
                        {
                            var newStart = startPos;//.Result;
                            var newEnd = endPos;//.Result;
                            if (newStart != oldStartPos || newEnd != oldEndPos || forceReload)
                            {
                                forceReload = false;
                                oldStartPos = newStart;
                                oldEndPos = newEnd;

                                /*return */EdmsTasks.InsertJob(() =>
                                {
                                    try
                                    {
                                        var pathGeo = BuildPath.BuildSmoothBezier(oldStartPos, oldEndPos);
                                        (sender as Windows.UI.Xaml.Shapes.Path).Data = pathGeo;
                                    }
                                    catch (Exception) { }
                                    finally
                                    {
                                        localLock.Decrement();
                                    }
                                }, true);
                            }
                            else
                            {
                                localLock.Decrement();
                                //return null;
                            }
                        }
                        //, false);

                    }
                }, newInt);

                (sender as Windows.UI.Xaml.Shapes.Path).Tag = (newInt, uniqueDropNodeParentName, uniqueDropNodeID);
            };
            path_orig.Unloaded += (object sender, RoutedEventArgs e) =>
            {
                if (sender != null && sender is Windows.UI.Xaml.Shapes.Path)
                {
                    var tagC = (sender as Windows.UI.Xaml.Shapes.Path).Tag;
                    if (tagC != null)
                    {
                        var data = ((int, string, string))tagC;
                        pathAdjusterForAll?.RemoveAction(data.Item1);
                        (sender as Windows.UI.Xaml.Shapes.Path).Tag = (-1, uniqueDropNodeParentName, uniqueDropNodeID);
                    }                    
                    parentP = null;
                    n1 = null;
                    n2 = null;
                }
            };
            if (path_orig.IsLoaded)
            {
                forceReload = true;
                AtomicInt localLock = new AtomicInt();

                int newInt = (int)pathAdjusterForAll_Counter.Increment();
                pathAdjusterForAll?.AddAction(() =>
                {
                    // if ((sender as Windows.UI.Xaml.Shapes.Path).IsLoaded || n1.IsLoaded || n2.IsLoaded)
                    if (localLock.TryIncrementTo(1))
                    {
                        if (n1 == null)
                        {
                            EdmsTasks.InsertJob(() => {
                                n1 = FindDragNode(path_orig, uniqueDragNodeParentName);
                                localLock.Decrement();
                            }, true);
                            return;
                        }
                        if (n2 == null)
                        {
                            EdmsTasks.InsertJob(() => {
                                n2 = FindDropNode(path_orig, uniqueDropNodeParentName, uniqueDropNodeID);
                                localLock.Decrement();
                            }, true);
                            return;
                        }
                        if (parentP == null)
                        {
                            EdmsTasks.InsertJob(() => {
                                parentP = path_orig.Parent as UIElement;
                                localLock.Decrement();
                            }, true);
                            return;
                        }

                        var offsetP1 = new Point(9, 5.5);
                        var startPos = EdmsTasks.InsertJob(() =>
                        {
                            try
                            {
                                if (parentP == null) return new Point(0, 0);
                                return n1.TransformToVisual(parentP).TransformPoint(offsetP1);
                            }
                            catch (Exception) { }
                            return new Point(0, 0);
                        }, true);
                        var offsetP2 = new Point(1.5, 9.5);
                        var endPos = EdmsTasks.InsertJob(() =>
                        {
                            try
                            {
                                if (parentP == null) return new Point(0, 0);
                                return n2.TransformToVisual(parentP).TransformPoint(offsetP2);
                            }
                            catch (Exception) { }
                            return new Point(0, 0);
                        }, true);
                        EdmsTasks.cweeTask.TrueWhenCompleted(new List<cweeTask<Point>>() { startPos, endPos }).ContinueWith(() =>
                        {
                            var newStart = startPos.Result;
                            var newEnd = endPos.Result;
                            if (newStart != oldStartPos || newEnd != oldEndPos || forceReload)
                            {
                                forceReload = false;
                                oldStartPos = newStart;
                                oldEndPos = newEnd;

                                return EdmsTasks.InsertJob(() =>
                                {
                                    try
                                    {
                                        var pathGeo = BuildPath.BuildSmoothBezier(oldStartPos, oldEndPos);
                                        path_orig.Data = pathGeo;
                                    }
                                    catch (Exception) { }
                                    finally
                                    {
                                        localLock.Decrement();
                                    }
                                }, true);
                            }
                            else
                            {
                                localLock.Decrement();
                                return null;
                            }
                        }, false);

                    }
                }, newInt);

                path_orig.Tag = (newInt, uniqueDropNodeParentName, uniqueDropNodeID);
            }
            else
            {
                path_orig.Tag = (-1, uniqueDropNodeParentName, uniqueDropNodeID);
            }
        }
    }
}
