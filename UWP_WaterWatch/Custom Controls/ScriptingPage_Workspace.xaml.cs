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
using UWP_WaterWatchLibrary;
using UWP_WaterWatchLibrary.Floating;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.System;
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

    public class CanvasSizedChangedArgs
    {
        public double previousHeight;
        public double previousWidth;
        public double Height;
        public double Width;
    }
    public class ScriptingPage_Workspace_ViewModel : ViewModelBase
    {
        public static AtomicInt ObjCount = new AtomicInt();
        public ScriptingPage_Workspace_ViewModel() { ObjCount.Increment(); }
        ~ScriptingPage_Workspace_ViewModel() {
            ObjCount.Decrement();
            scrollViewer = null;
            canvas = null;
            cweeSizeChangedEvent = null;
            cweeSizeChangedDequeue?.Cancel();
            cweeSizeChangedDequeue = null;
        }

        public ScrollViewer scrollViewer;
        public Panel canvas;

        private double _canvasWidth = double.NaN;
        public double canvasWidth
        {
            get
            {
                if (!double.IsNaN(_canvasWidth))
                {
                    return _canvasWidth;
                }

                if (scrollViewer != null)
                {
                    return scrollViewer.ActualWidth;
                }

                return 1000;
            }
            set
            {
                var args = new CanvasSizedChangedArgs() { Height = _canvasHeight, Width = value, previousHeight = _canvasHeight, previousWidth = _canvasWidth };
                _canvasWidth = value;
                OnPropertyChanged("canvasWidth");
                cweeSizeChangedEvent.InvokeEvent(null, args); // .InvokeEvent
            }
        }

        private double _canvasHeight = double.NaN;
        public double canvasHeight
        {
            get
            {
                if (!double.IsNaN(_canvasHeight))
                {
                    return _canvasHeight;
                }

                if (scrollViewer != null)
                {
                    return scrollViewer.ActualHeight;
                }

                return 1000;
            }
            set
            {
                var args = new CanvasSizedChangedArgs() { Height = value, Width = _canvasWidth, previousHeight = _canvasHeight, previousWidth = _canvasWidth };
                _canvasHeight = value;
                OnPropertyChanged("canvasHeight");
                cweeSizeChangedEvent.InvokeEvent(null, args); // .InvokeEvent
            }
        }

        public bool PreviouslyLoaded => (!double.IsNaN(_canvasWidth) && !double.IsNaN(_canvasHeight));

        private bool _GridVisible = false;
        public bool GridVisible { get { return _GridVisible; } set { _GridVisible = value; OnPropertyChanged("GridVisible"); } }

        public cweeEvent<CanvasSizedChangedArgs> cweeSizeChangedEvent = new cweeEvent<CanvasSizedChangedArgs>();
        public cweeDequeue cweeSizeChangedDequeue;
    }

    public sealed partial class ScriptingPage_Workspace : UserControl
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

        public ScriptingPage_Workspace_ViewModel vm = new ScriptingPage_Workspace_ViewModel();

        public ScriptingPage_Workspace()
        {
            ObjCount.Increment();
            this.InitializeComponent();
            vm.scrollViewer = RenderScroll;
            vm.canvas = DrawCanvas;
            this.DataContext = vm;

            vm.cweeSizeChangedEvent += (object nothing, CanvasSizedChangedArgs arg) => {
                //EdmsTasks.InsertJob(()=> {
                    //if (vm.cweeSizeChangedDequeue == null)
                    //{
                        vm.cweeSizeChangedDequeue = new cweeDequeue(DateTime.Now.AddSeconds(0.1), () =>
                        {
                            double thickness = Math.Min(vm.canvasWidth / 100, vm.canvasHeight / 100);
                            double length = Math.Max(vm.canvasWidth / 20, vm.canvasHeight / 20);

                            (RightEdge.Content as FrameworkElement).Width = thickness;
                            (RightEdge.Content as FrameworkElement).Height = length;

                            (BottomEdge.Content as FrameworkElement).Width = length;
                            (BottomEdge.Content as FrameworkElement).Height = thickness;

                            RightEdge.SetPosition();
                            BottomEdge.SetPosition();
                        }, true);
                    //}
                    //else
                    //{
                    //    vm.cweeSizeChangedDequeue.Dequeue(DateTime.Now.AddSeconds(0.1));
                    //}
                //}, true);
            };
        }
        ~ScriptingPage_Workspace()
        {
            ObjCount.Decrement();
            vm = null;
        }

        public bool GridVisible { get { return vm.GridVisible; } set { vm.GridVisible = value; OnPropertyChanged("GridVisible"); } }

        public EdmsTasks.cweeTask ClearWorkspace()
        {
            return EdmsTasks.InsertJob(()=> {
                List<EdmsTasks.cweeTask> jobs = new List<EdmsTasks.cweeTask>();
                foreach (var child in vm.canvas.Children.ToList()) {
                    if (child is ScriptNode)
                    {
                        /*jobs.Add(*/(child as ScriptNode).DisconnectFromAll();// .ContinueWith(() => {
                            (child as ScriptNode).vm.DeleteSelfEvent.InvokeEvent((child as ScriptNode), (child as ScriptNode));
                        //}, true).ContinueWith(()=> {
                            vm.canvas.Children.Remove(child);
                        //}, true));                        
                    }
                }
                return EdmsTasks.cweeTask.TrueWhenCompleted(jobs);
            }, true);
        }
        public void ZoomIn() => ZoomIn(null, null);
        public void ZoomOut() => ZoomOut(null, null);
        public void ResetZoom() => ResetZoom(null, null);

        private void ResetZoom(object sender, RoutedEventArgs e)
        {
            ZoomTo(1.0f);
        }
        private void ZoomOut(object sender, RoutedEventArgs e)
        {
            var desiredZoom = vm.scrollViewer.ZoomFactor * 0.5f;
            ZoomTo(desiredZoom);
        }
        private void ZoomIn(object sender, RoutedEventArgs e)
        {
            var desiredZoom = vm.scrollViewer.ZoomFactor * 2.0f;
            ZoomTo(desiredZoom);
        }
        internal void ZoomTo(float setting)
        {
            var desiredZoom = setting;
            desiredZoom = Math.Min(desiredZoom, vm.scrollViewer.MaxZoomFactor);
            desiredZoom = Math.Max(desiredZoom, vm.scrollViewer.MinZoomFactor);

            var factor = desiredZoom / vm.scrollViewer.ZoomFactor;
            // 1 = not going to zoom;
            // > 1 = we are going to zoom in ... shift right/up
            // < 1 = we are going to zoom out ... shift left/down
            if (factor > 1)
            {
                var wOff_a = factor * vm.scrollViewer.HorizontalOffset;  // count of pixels scrolled to the right from the perspective of our new zoom
                var hOff_a = factor * vm.scrollViewer.VerticalOffset;  // count of pixels scrolled from the top from the perspective of our new zoom

                var w_a = factor * vm.scrollViewer.ActualWidth; // / vm.scrollViewer.ZoomFactor;
                var h_a = factor * vm.scrollViewer.ActualHeight; // / vm.scrollViewer.ZoomFactor;

                Point center = new Point(wOff_a + w_a / 2.0f, hOff_a + h_a / 2.0f);

                var w_b = vm.scrollViewer.ActualWidth;
                var h_b = vm.scrollViewer.ActualHeight;
                var wDelta = (w_a - w_b) / 2.0f;
                var hDelta = (h_a - h_b) / 2.0f;

                var wOff_b = wOff_a + wDelta;
                var hOff_b = hOff_a + hDelta;

                vm.scrollViewer.ChangeView(wOff_b, hOff_b, desiredZoom);
            }
            else if (factor < 1)
            {
                var wOff_a = factor * vm.scrollViewer.HorizontalOffset;  // count of pixels scrolled to the right from the perspective of our new zoom
                var hOff_a = factor * vm.scrollViewer.VerticalOffset;  // count of pixels scrolled from the top from the perspective of our new zoom

                var w_a = factor * vm.scrollViewer.ActualWidth; // / vm.scrollViewer.ZoomFactor;
                var h_a = factor * vm.scrollViewer.ActualHeight; // / vm.scrollViewer.ZoomFactor;

                Point center = new Point(wOff_a + w_a / 2.0f, hOff_a + h_a / 2.0f);

                var w_b = vm.scrollViewer.ActualWidth;
                var h_b = vm.scrollViewer.ActualHeight;
                var wDelta = (w_a - w_b) / 2.0f;
                var hDelta = (h_a - h_b) / 2.0f;

                var wOff_b = wOff_a + wDelta;
                var hOff_b = hOff_a + hDelta;

                vm.scrollViewer.ChangeView(wOff_b, hOff_b, desiredZoom);
            }
        }
        private void CanvasScroll(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {
            // exclusively happens when the viewer is zoomed "out" and there cannot be room to scroll -- only zoom. 
            //if (e.KeyModifiers != VirtualKeyModifiers.Control)
            //{
            //    var pointer = e.GetCurrentPoint(vm.scrollViewer);
            //    var scroll = pointer.Properties.MouseWheelDelta;
            //    if (scroll > 0)
            //    {
            //        ZoomTo(vm.scrollViewer.ZoomFactor + 0.2f);
            //    }
            //    else
            //    {
            //        ZoomTo(vm.scrollViewer.ZoomFactor - 0.2f);
            //    }
            //}
            //return;
            var pointer = e.GetCurrentPoint(vm.scrollViewer);
            var scroll = pointer.Properties.MouseWheelDelta;
            var shift = e.KeyModifiers == VirtualKeyModifiers.Shift;
            var canScrollHorizontal = vm.scrollViewer.ExtentWidth > vm.scrollViewer.ActualWidth;
            var canScrollVertical = vm.scrollViewer.ExtentHeight > vm.scrollViewer.ActualHeight;

            if (e.KeyModifiers != VirtualKeyModifiers.Control)
            {
                if (canScrollHorizontal && canScrollVertical)
                {
                    // could be either! 
                    if (shift)
                    {
                        // horizontal
                        if (scroll > 0)
                        {
                            if (vm.scrollViewer.HorizontalOffset > 0)
                            {
                                vm.scrollViewer.ChangeView(
                                    vm.scrollViewer.HorizontalOffset - vm.scrollViewer.ActualWidth / 10,
                                    null,
                                    null);
                                e.Handled = true;
                                return;
                            }
                        }
                        else
                        {
                            if (vm.scrollViewer.HorizontalOffset + vm.scrollViewer.ActualWidth < vm.scrollViewer.ExtentWidth)
                            {
                                vm.scrollViewer.ChangeView(
                                    vm.scrollViewer.HorizontalOffset + vm.scrollViewer.ActualWidth / 10,
                                    null,
                                    null);
                                e.Handled = true;
                                return;
                            }

                        }
                    }
                    else
                    {
                        // vertical 
                        if (scroll > 0)
                        {
                            if (vm.scrollViewer.VerticalOffset > 0)
                            {
                                vm.scrollViewer.ChangeView(
                                    null,
                                    vm.scrollViewer.VerticalOffset - vm.scrollViewer.ActualHeight / 10,
                                    null);
                                e.Handled = true;
                                return;
                            }
                        }
                        else
                        {
                            if (vm.scrollViewer.VerticalOffset + vm.scrollViewer.ActualHeight < vm.scrollViewer.ExtentHeight)
                            {
                                vm.scrollViewer.ChangeView(
                                    null,
                                    vm.scrollViewer.VerticalOffset + vm.scrollViewer.ActualHeight / 10,
                                    null);
                                e.Handled = true;
                                return;
                            }

                        }
                    }
                }

                if (canScrollHorizontal)
                {
                    // horizontal
                    if (scroll > 0)
                    {
                        if (vm.scrollViewer.HorizontalOffset > 0)
                        {
                            vm.scrollViewer.ChangeView(
                                vm.scrollViewer.HorizontalOffset - vm.scrollViewer.ActualWidth / 10,
                                null,
                                null);
                            e.Handled = true;
                            return;
                        }
                    }
                    else
                    {
                        if (vm.scrollViewer.HorizontalOffset + vm.scrollViewer.ActualWidth < vm.scrollViewer.ExtentWidth)
                        {
                            vm.scrollViewer.ChangeView(
                                vm.scrollViewer.HorizontalOffset + vm.scrollViewer.ActualWidth / 10,
                                null,
                                null);
                            e.Handled = true;
                            return;
                        }

                    }
                }
                if (canScrollVertical)
                {
                    // vertical 
                    if (scroll > 0)
                    {
                        if (vm.scrollViewer.VerticalOffset > 0)
                        {
                            vm.scrollViewer.ChangeView(
                                null,
                                vm.scrollViewer.VerticalOffset - vm.scrollViewer.ActualHeight / 10,
                                null);
                            e.Handled = true;
                            return;
                        }
                    }
                    else
                    {
                        if (vm.scrollViewer.VerticalOffset + vm.scrollViewer.ActualHeight < vm.scrollViewer.ExtentHeight)
                        {
                            vm.scrollViewer.ChangeView(
                                null,
                                vm.scrollViewer.VerticalOffset + vm.scrollViewer.ActualHeight / 10,
                                null);
                            e.Handled = true;
                            return;
                        }

                    }
                }
            }

            if (e.KeyModifiers == VirtualKeyModifiers.Control) return; // allow the actual scroller to handle this;

            if (!canScrollVertical && !canScrollHorizontal)
            {
                if (scroll > 0)
                {
                    ZoomTo(vm.scrollViewer.ZoomFactor + 0.2f);
                }
                else
                {
                    ZoomTo(vm.scrollViewer.ZoomFactor - 0.2f);
                }
                e.Handled = true;
            }

        }

        private void RenderScroll_Loaded(object sender, RoutedEventArgs e)
        {
            if (!vm.PreviouslyLoaded)
            {
                vm.canvasWidth = Math.Max(1, (sender as FrameworkElement).ActualWidth - 2);
                vm.canvasHeight = Math.Max(1, (sender as FrameworkElement).ActualHeight - 2);
            }
        }
        private void Canvas_DragOver(object sender, DragEventArgs e)
        {

        }

        private void Canvas_Drop(object sender, DragEventArgs e)
        {

        }

        private void Canvas_DropCompleted(UIElement sender, DropCompletedEventArgs args)
        {

        }

        private void DraggableRightEdgeLoaded(object sender, RoutedEventArgs e) {
            FloatingContent floater = (sender as FloatingContent);
            floater.SetPosition();
            floater.ManipulationStartedWWEvent += ManipulationStartedWWEvent;
            floater.ManipulationEndedWWEvent += ManipulationEndedWWEvent_Horizontal;
        }
        private void DraggableBottomEdgeLoaded(object sender, RoutedEventArgs e) {
            FloatingContent floater = (sender as FloatingContent);
            floater.SetPosition();
            floater.ManipulationStartedWWEvent += ManipulationStartedWWEvent;
            floater.ManipulationEndedWWEvent += ManipulationEndedWWEvent_Vertical;
        }
        private void DraggableRightEdgeUnloaded(object sender, RoutedEventArgs e)
        {
            FloatingContent floater = (sender as FloatingContent);
            floater.ManipulationStartedWWEvent -= ManipulationStartedWWEvent;
            floater.ManipulationEndedWWEvent -= ManipulationEndedWWEvent_Horizontal;
        }
        private void DraggableBottomEdgeUnloaded(object sender, RoutedEventArgs e)
        {
            FloatingContent floater = (sender as FloatingContent);
            floater.ManipulationStartedWWEvent -= ManipulationStartedWWEvent;
            floater.ManipulationEndedWWEvent -= ManipulationEndedWWEvent_Vertical;
        }

        private void ManipulationStartedWWEvent(object s2, FloatingContent.ManipulationStartedArgs args) {
            EdmsTasks.InsertJob(() => {
                (s2 as FloatingContent).Boundary = FloatingBoundary.None;
            }, true);
        }
        private void ManipulationEndedWWEvent_Horizontal(object s2, FloatingContent.ManipulationEndedArgs args)
        {
            EdmsTasks.InsertJob(() => {
                var ttv = (s2 as FloatingContent).border.TransformToVisual(DrawCanvas);
                var actualPos = ttv.TransformPoint(new Point(0, 0));
                vm.canvasWidth = Math.Max(actualPos.X, 100);
                (s2 as FloatingContent).Boundary = FloatingBoundary.Parent;
                (s2 as FloatingContent).SetPosition();
            }, true);
        }
        private void ManipulationEndedWWEvent_Vertical(object s2, FloatingContent.ManipulationEndedArgs args)
        {
            EdmsTasks.InsertJob(() => {
                var ttv = (s2 as FloatingContent).border.TransformToVisual(DrawCanvas);
                var actualPos = ttv.TransformPoint(new Point(0, 0));
                vm.canvasHeight = Math.Max(actualPos.Y, 100);
                (s2 as FloatingContent).Boundary = FloatingBoundary.Parent;
                (s2 as FloatingContent).SetPosition();
            }, true);
        }

    }
}
