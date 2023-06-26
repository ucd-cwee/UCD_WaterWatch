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

using Microsoft.Toolkit.Uwp.UI;
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
using Windows.UI.Xaml.Media.Animation;
using Windows.UI.Xaml.Navigation;
using static UWP_WaterWatchLibrary.cweeXamlHelper;

namespace UWP_WaterWatchLibrary
{
    namespace Floating
    {
        public enum FloatingBoundary
        {
            None,
            Parent,
            Window
        }
        public enum FloatingDirection
        {
            Any,
            Horizontal,
            Vertical
        }

        public enum NodeType
        {
            Start,
            End
        }
        public enum PortPosition
        {
            Left, Right
        }
        public enum PathMode
        {
            Temporary, Good, Bad, Loop
        }

        public static class BuildPath
        {
            public static PathGeometry BuildSmoothBezier(Point startPoint, Point endPoint)
            {
                var startGradient = ToGradient(PortPosition.Right);
                var endGradient = ToGradient(PortPosition.Left);

                return BuildSmoothBezier(startPoint, startGradient, endPoint, endGradient);
            }

            public static PathGeometry BuildSmoothBezier(Point startPoint, PortPosition startPosition, Point endPoint, PortPosition endPosition)
            {
                var startGradient = ToGradient(startPosition);
                var endGradient = ToGradient(endPosition);

                return BuildSmoothBezier(startPoint, startGradient, endPoint, endGradient);
            }

            public static PathGeometry BuildSmoothBezier(Point startPoint, PortPosition startPosition, Point endPoint)
            {
                var startGradient = ToGradient(startPosition);
                var endGradient = new List<double>() { -startGradient[0], -startGradient[1] };

                return BuildSmoothBezier(startPoint, startGradient, endPoint, endGradient);
            }

            public static PathGeometry BuildSmoothBezier(Point startPoint, Point endPoint, PortPosition endPosition)
            {
                var endGradient = ToGradient(endPosition);
                var startGradient = new List<double>() { -endGradient[0], -endGradient[1] };

                return BuildSmoothBezier(startPoint, startGradient, endPoint, endGradient);
            }

            private static List<double> ToGradient(PortPosition portPosition)
            {
                switch (portPosition)
                {
                    case PortPosition.Left:
                        return new List<double>() { -1.0, 0.0 };
                    case PortPosition.Right:
                        return new List<double>() { 1.0, 0.0 };
                    default:
                        throw new NotImplementedException();
                }
            }

            private const double MinGradient = 10;
            private const double WidthScaling = 5;

            private static PathGeometry BuildSmoothBezier(Point startPoint, List<double> startGradient, Point endPoint, List<double> endGradient)
            {
                double width = endPoint.X - startPoint.X;

                var gradientScale = Math.Sqrt(Math.Abs(width) * WidthScaling + MinGradient * MinGradient);

                Point startGradientPoint = new Point(startPoint.X + (startGradient[0] * gradientScale), startPoint.Y + (startGradient[1] * gradientScale));
                Point endGradientPoint = new Point(endPoint.X + (endGradient[0] * gradientScale), endPoint.Y + (endGradient[1] * gradientScale));

                Point midPoint = new Point((startGradientPoint.X + endGradientPoint.X) / 2d, (startPoint.Y + endPoint.Y) / 2d);

                PathFigure pathFigure = new PathFigure
                {
                    StartPoint = startPoint,
                    IsClosed = false,
                    Segments =
                {
                    new QuadraticBezierSegment(){ Point1 = startGradientPoint, Point2 = midPoint},
                    new QuadraticBezierSegment(){ Point1 = endGradientPoint, Point2 = endPoint}
                }
                };

                PathGeometry geom = new PathGeometry();
                geom.Figures.Add(pathFigure);

                return geom;
            }
        }






        /// <summary>
        /// A Content Control that can be dragged around.
        /// </summary>
        [TemplatePart(Name = BorderPartName, Type = typeof(Border))]
        [TemplatePart(Name = OverlayPartName, Type = typeof(UIElement))]
        public class FloatingContent : ContentControl
        {
            public event PropertyChangedEventHandler PropertyChanged;
            protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
            {
                EdmsTasks.InsertJob(() =>
                {
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
                }, true, true);
            }

            public static double border_max_opacity = 0.5;
            public static double border_min_opacity = 0.0;
            public static TimeSpan border_flashTime = TimeSpan.FromSeconds(0.25);

            private const string BorderPartName = "PART_Border";
            private const string OverlayPartName = "PART_Overlay";


            public double DragDelayMilliseconds = 100.0;
            public Border border;
            private bool LoadedAlready = false;
            private UIElement overlay;

            public static readonly DependencyProperty BoundaryProperty =
                DependencyProperty.Register(
                    "Boundary",
                    typeof(FloatingBoundary),
                    typeof(FloatingContent),
                    new PropertyMetadata(FloatingBoundary.None));

            public static readonly DependencyProperty DirectionProperty =
                DependencyProperty.Register(
                    "Direction",
                    typeof(FloatingDirection),
                    typeof(FloatingContent),
                    new PropertyMetadata(FloatingDirection.Any));

            public static readonly DependencyProperty InitialPositionLeftProperty =
                DependencyProperty.Register(
                    "InitialPositionLeft",
                    typeof(string),
                    typeof(FloatingContent),
                    new PropertyMetadata("0"));

            public static readonly DependencyProperty InitialPositionTopProperty =
                DependencyProperty.Register(
                    "InitialPositionTop",
                    typeof(string),
                    typeof(FloatingContent),
                    new PropertyMetadata("0"));

            /// <summary>
            /// Initializes a new instance of the <see cref="FloatingContent"/> class.
            /// </summary>
            public FloatingContent()
            {
                this.DefaultStyleKey = typeof(FloatingContent);
            }

            public FloatingBoundary Boundary
            {
                get { return (FloatingBoundary)GetValue(BoundaryProperty); }
                set { SetValue(BoundaryProperty, value); }
            }
            public FloatingDirection Direction
            {
                get { return (FloatingDirection)GetValue(DirectionProperty); }
                set { SetValue(DirectionProperty, value); }
            }

            public string InitialPositionLeft
            {
                get { return (string)GetValue(InitialPositionLeftProperty); }
                set
                {
                    SetValue(InitialPositionLeftProperty, value);
                }
            }
            public string InitialPositionTop
            {
                get { return (string)GetValue(InitialPositionTopProperty); }
                set
                {
                    SetValue(InitialPositionTopProperty, value);
                }
            }

            public string CurrentPositionLeft() {
                (double, double) widthHeight = (0, 0);
                var bound = GetParentsRect();
                widthHeight.Item1 = bound.Width;
                widthHeight.Item2 = bound.Height;


                double leftOffset = Canvas.GetLeft(this.border) / (bound.Width - this.border.ActualWidth);
                return $"{leftOffset}*";
            }
            public string CurrentPositionTop() {
                (double, double) widthHeight = (0, 0);
                var bound = GetParentsRect();
                widthHeight.Item1 = bound.Width;
                widthHeight.Item2 = bound.Height;


                double topOffset = Canvas.GetTop(this.border) / (bound.Height - this.border.ActualHeight);
                return $"{topOffset}*";
            }

            public static readonly DependencyProperty DragEnabledProperty =
        DependencyProperty.Register(
            "DragEnabled",
            typeof(bool),
            typeof(FloatingContent),
            new PropertyMetadata(true));
            public bool DragEnabled
            {
                get { return (bool)GetValue(DragEnabledProperty); }
                set { SetValue(DragEnabledProperty, value); }
            }



            /// <summary>
            /// Invoked whenever application code or internal processes (such as a rebuilding layout pass) call ApplyTemplate.
            /// In simplest terms, this means the method is called just before a UI element displays in your app.
            /// Override this method to influence the default post-template logic of a class.
            /// </summary>
            protected override void OnApplyTemplate()
            {
                // Border
                this.border = this.GetTemplateChild(BorderPartName) as Border;
                if (this.border != null)
                {
                    this.border.ManipulationStarted += Border_ManipulationStarted;
                    this.border.ManipulationDelta += Border_ManipulationDelta;
                    this.border.ManipulationCompleted += Border_ManipulationCompleted;
                    this.border.PointerEntered += Border_PointerEntered;
                    this.border.PointerExited += Border_PointerExited;
                    this.border.PointerPressed += Border_PointerPressed;
                    this.border.PointerMoved += Border_PointerMoved;

                    // Move Canvas properties from control to border.
                    Canvas.SetLeft(this.border, Canvas.GetLeft(this));
                    Canvas.SetLeft(this, 0);
                    Canvas.SetTop(this.border, Canvas.GetTop(this));
                    Canvas.SetTop(this, 0);

                    // Move Margin to border.
                    this.border.Padding = this.Margin;
                    this.Margin = new Thickness(0);
                    this.border.CornerRadius = this.CornerRadius;
                }
                else
                {
                    // Exception
                    throw new Exception("Floating Control Style has no Border.");
                }

                // Overlay
                this.overlay = GetTemplateChild(OverlayPartName) as UIElement;

                this.Loaded += Floating_Loaded;
            }


            cweeStopWatch cweeSW = new cweeStopWatch();
            private void Border_PointerMoved(object sender, PointerRoutedEventArgs e)
            {
                if (cweeSW != null)
                {
                    cweeSW.Stop();
                }
            }

            private void Border_PointerPressed(object sender, PointerRoutedEventArgs e)
            {
                cweeSW = new cweeStopWatch();
                cweeSW.Start();
            }

            private (double, bool) StringToComponents(string input)
            {
                bool hasStar = input.Contains("*");
                double v = 0;
                if (hasStar)
                {
                    var numericals = input.Split('*', StringSplitOptions.RemoveEmptyEntries);
                    if (numericals.Length > 0)
                    {
                        foreach (var potential in numericals)
                        {
                            if (potential.Length > 0)
                            {
                                if (double.TryParse(potential, out double a))
                                {
                                    v = a;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        v = 1;
                    }
                }
                else
                {
                    if (double.TryParse(input, out double a))
                        v = a;
                }

                return (v, hasStar);
            }

            private void Border_PointerEntered(object sender, PointerRoutedEventArgs e)
            {
                if (this.overlay != null)
                {
                    var ani = new DoubleAnimation()
                    {
                        From = border_min_opacity,
                        To = border_max_opacity,
                        Duration = new Duration(border_flashTime),
                        AutoReverse = false
                    };
                    var storyBoard = new Storyboard();
                    storyBoard.Children.Add(ani);
                    Storyboard.SetTarget(ani, overlay);
                    ani.SetValue(Storyboard.TargetPropertyProperty, "Opacity");
                    storyBoard.Begin();
                }
            }
            private void Border_PointerExited(object sender, PointerRoutedEventArgs e)
            {
                if (this.overlay != null)
                {
                    var ani = new DoubleAnimation()
                    {
                        From = border_max_opacity,
                        To = border_min_opacity,
                        Duration = new Duration(border_flashTime),
                        AutoReverse = false
                    };
                    var storyBoard = new Storyboard();
                    storyBoard.Children.Add(ani);
                    Storyboard.SetTarget(ani, overlay);
                    ani.SetValue(Storyboard.TargetPropertyProperty, "Opacity");
                    storyBoard.Begin();
                }
            }

            public class ManipulationStartedArgs
            {
                public Point startingPosition;
            }
            public cweeEvent<ManipulationStartedArgs> ManipulationStartedWWEvent = new cweeEvent<ManipulationStartedArgs>();
            public class ManipulationEndedArgs
            {

            }
            public cweeEvent<ManipulationEndedArgs> ManipulationEndedWWEvent = new cweeEvent<ManipulationEndedArgs>();
            public class ManipulationDeltaArgs
            {
                public Point newPosition;
            }
            public cweeEvent<ManipulationDeltaArgs> ManipulationDeltaWWEvent = new cweeEvent<ManipulationDeltaArgs>();

            private void Border_ManipulationStarted(object sender, ManipulationStartedRoutedEventArgs e)
            {
                if (!DragEnabled) // cweeSW.ElapsedMilliseconds < DragDelayMilliseconds || 
                {
                    e.Complete();
                    return;
                }

                ManipulationStartedWWEvent.InvokeEventAsync(this, new ManipulationStartedArgs() { startingPosition = e.Position });
            }

            private void Border_ManipulationCompleted(object sender, ManipulationCompletedRoutedEventArgs e)
            {
                ManipulationEndedWWEvent.InvokeEventAsync(this, new ManipulationEndedArgs() { });
            }


            public virtual void SetPosition()
            {
                if (this.border != null)
                {
                    if (this.InitialPositionLeft != "0" || this.InitialPositionTop != "0")
                    {
                        string positionleft = this.InitialPositionLeft;
                        string positionTop = this.InitialPositionTop;

                        // interpret the strings
                        var left_interp = StringToComponents(positionleft);
                        var top_interp = StringToComponents(positionTop);

                        // get the bounds
                        (double, double) widthHeight = (0, 0);
                        var bound = GetParentsRect();
                        widthHeight.Item1 = bound.Width;
                        widthHeight.Item2 = bound.Height;

                        // apply
                        if (left_interp.Item2)
                        {
                            double b = (bound.Width - this.border.ActualWidth) * left_interp.Item1;
                            Canvas.SetLeft(this.border, b);
                        }
                        else
                        {
                            Canvas.SetLeft(this.border, left_interp.Item1);
                        }

                        if (top_interp.Item2)
                        {
                            double b = top_interp.Item1 * (bound.Height - this.border.ActualHeight);
                            Canvas.SetTop(this.border, b);
                        }
                        else
                        {
                            Canvas.SetTop(this.border, top_interp.Item1);
                        }
                    }
                }
            }
            public virtual void SetPosition(string positionleft, string positionTop)
            {
                if (this.border != null)
                {
                    // interpret the strings
                    var left_interp = StringToComponents(positionleft);
                    var top_interp = StringToComponents(positionTop);

                    // get the bounds
                    (double, double) widthHeight = (0, 0);
                    var bound = GetParentsRect();
                    widthHeight.Item1 = bound.Width;
                    widthHeight.Item2 = bound.Height;

                    // apply
                    if (left_interp.Item2)
                    {
                        double b = (bound.Width - this.border.ActualWidth) * left_interp.Item1;
                        Canvas.SetLeft(this.border, b);
                    }
                    else
                    {
                        Canvas.SetLeft(this.border, left_interp.Item1);
                    }

                    if (top_interp.Item2)
                    {
                        double b = top_interp.Item1 * (bound.Height - this.border.ActualHeight);
                        Canvas.SetTop(this.border, b);
                    }
                    else
                    {
                        Canvas.SetTop(this.border, top_interp.Item1);
                    }
                }
            }


            private void Floating_Loaded(object sender, RoutedEventArgs e)
            {
                if (!LoadedAlready)
                {
                    LoadedAlready = true;

                    FrameworkElement el = GetClosestParentWithSize(this);
                    if (el == null)
                    {
                        return;
                    }

                    // initial offset
                    SetPosition();

                    el.SizeChanged += Floating_SizeChanged;
                }
            }

            private void Floating_SizeChanged(object sender, SizeChangedEventArgs e)
            {
                var left = Math.Max(0, Canvas.GetLeft(this.border));
                var top = Math.Max(0, Canvas.GetTop(this.border));

                Rect rect = new Rect(left, top, Math.Max(1, this.border.ActualWidth), Math.Max(1, this.border.ActualHeight));

                AdjustCanvasPosition(rect);
            }


            private double RestrictLeftMovement(double dt_x)
            {
                switch (Direction)
                {
                    case FloatingDirection.Any:
                        return dt_x;
                    case FloatingDirection.Horizontal:
                        return dt_x;
                    case FloatingDirection.Vertical:
                        return 0;
                    default:
                        throw(new Exception("Bad direction value"));
                }
            }
            private double RestrictTopMovement(double dt_y)
            {
                switch (Direction)
                {
                    case FloatingDirection.Any:
                        return dt_y;
                    case FloatingDirection.Horizontal:
                        return 0;
                    case FloatingDirection.Vertical:
                        return dt_y;
                    default:
                        throw (new Exception("Bad direction value"));
                }
            }
            private void Border_ManipulationDelta(object sender, ManipulationDeltaRoutedEventArgs e)
            {
                e.Handled = true;

                // find any DOWNSTREAM connecting nodes THAT ARE LOCKED / CONNECTED TO OTHERS 
                double leftMovement = RestrictLeftMovement(e.Delta.Translation.X);
                double topMovement = RestrictTopMovement(e.Delta.Translation.Y);

                var prevL = Canvas.GetLeft(this.border);
                var prevT = Canvas.GetTop(this.border);               
                var left = Canvas.GetLeft(this.border) + leftMovement;
                var top = Canvas.GetTop(this.border) + topMovement;

                Rect rect = new Rect(left, top, Math.Max(1, this.border.ActualWidth), Math.Max(1, this.border.ActualHeight));
                var moved = AdjustCanvasPosition(rect);

                ManipulationDeltaWWEvent.InvokeEventAsync(this, new ManipulationDeltaArgs() { newPosition = new Point(e.Position.X + e.Cumulative.Translation.X, e.Position.Y + e.Cumulative.Translation.Y) });
            }

            private Rect GetParentsRect()
            {
                FrameworkElement el = GetClosestParentWithSize(this);
                if (el != null && this.Boundary == FloatingBoundary.Parent)
                {
                    Rect parentRect = new Rect(0, 0, Math.Max(1, el.ActualWidth), Math.Max(1, el.ActualHeight));
                    return parentRect;
                }
                else if (el != null)
                {
                    var ttv = el.TransformToVisual(Window.Current.Content);
                    var topLeft = ttv.TransformPoint(new Point(0, 0));
                    Rect parentRect = new Rect(topLeft.X, topLeft.Y, Math.Max(1, Window.Current.Bounds.Width - topLeft.X), Math.Max(1, Window.Current.Bounds.Height - topLeft.Y));
                    return parentRect;
                }
                else
                {
                    var ttv = (this.Parent as UIElement).TransformToVisual(Window.Current.Content);
                    var topLeft = ttv.TransformPoint(new Point(0, 0));
                    Rect parentRect = new Rect(topLeft.X, topLeft.Y, Math.Max(1, Window.Current.Bounds.Width - topLeft.X), Math.Max(1, Window.Current.Bounds.Height - topLeft.Y));
                    return parentRect;
                }
            }

            /// <summary>
            /// Adjusts the canvas position according to the IsBoundBy* properties.
            /// </summary>
            public Point AdjustCanvasPosition(Rect rect)
            {
                // Free floating.
                if (this.Boundary == FloatingBoundary.None)
                {
                    Canvas.SetLeft(this.border, rect.Left);
                    Canvas.SetTop(this.border, rect.Top);

                    return new Point(rect.Left, rect.Top);
                }

                FrameworkElement el = GetClosestParentWithSize(this);

                // No parent
                if (el == null)
                {
                    Canvas.SetLeft(this.border, rect.Left);
                    Canvas.SetTop(this.border, rect.Top);
                    return new Point(rect.Left, rect.Top);
                }

                var position = new Point(rect.Left, rect.Top); ;

                if (this.Boundary == FloatingBoundary.Parent)
                {
                    Rect parentRect = new Rect(0, 0, Math.Max(1, el.ActualWidth), Math.Max(1, el.ActualHeight));
                    position = AdjustedPosition(rect, parentRect);
                }

                if (this.Boundary == FloatingBoundary.Window)
                {
                    var ttv = el.TransformToVisual(Window.Current.Content);
                    var topLeft = ttv.TransformPoint(new Point(0, 0));
                    Rect parentRect = new Rect(topLeft.X, topLeft.Y, Math.Max(1, Window.Current.Bounds.Width - topLeft.X), Math.Max(1, Window.Current.Bounds.Height - topLeft.Y));
                    position = AdjustedPosition(rect, parentRect);
                }

                // Set new position
                Canvas.SetLeft(this.border, position.X);
                Canvas.SetTop(this.border, position.Y);

                return position;
            }

            /// <summary>
            /// Returns the adjusted the topleft position of a rectangle so that is stays within a parent rectangle.
            /// </summary>
            /// <param name="rect">The rectangle.</param>
            /// <param name="parentRect">The parent rectangle.</param>
            /// <returns></returns>
            private Point AdjustedPosition(Rect rect, Rect parentRect)
            {
                var left = rect.Left;
                var top = rect.Top;

                if (left < -parentRect.Left)
                {
                    // Left boundary hit.
                    left = -parentRect.Left;
                }
                else if (left + rect.Width > parentRect.Width)
                {
                    // Right boundary hit.
                    left = parentRect.Width - (rect.Width + 1);
                }

                if (top < -parentRect.Top)
                {
                    // Top hit.
                    top = -parentRect.Top;
                }
                else if (top + rect.Height > parentRect.Height)
                {
                    // Bottom hit.
                    top = parentRect.Height - (rect.Height + 1);
                }

                return new Point(left, top);
            }

            /// <summary>
            /// Gets the closest parent with a real size.
            /// </summary>
            private FrameworkElement GetClosestParentWithSize(FrameworkElement element)
            {
                while (element != null && (element.ActualHeight == 0 || element.ActualWidth == 0))
                {
                    // Crawl up the Visual Tree.
                    element = element.Parent as FrameworkElement;
                }

                return element;
            }
        }

        /// <summary>
        /// A Content Control that can be stretched and resized.
        /// </summary>
        [TemplatePart(Name = GridName, Type = typeof(Grid))]
        [TemplatePart(Name = BottomRightName, Type = typeof(FloatingContent))]
        [TemplatePart(Name = BottomLeftName, Type = typeof(FloatingContent))]
        [TemplatePart(Name = TopRightName, Type = typeof(FloatingContent))]
        [TemplatePart(Name = TopLeftName, Type = typeof(FloatingContent))]
        public class StretchableContent : ContentControl
        {
            private const int cornerWidth = 10;

            private const string GridName = "PART_Grid";
            private const string BottomRightName = "PART_BottomRight";
            private const string BottomLeftName = "PART_BottomLeft";
            private const string TopRightName = "PART_TopRight";
            private const string TopLeftName = "PART_TopLeft";

            private bool LoadedAlready = false;
            private Grid grid;
            private FloatingContent bottomRight;
            private FloatingContent bottomLeft;
            private FloatingContent topRight;
            private FloatingContent topLeft;

            public cweeEvent<bool> StretchedEvent = new cweeEvent<bool>();

            /// <summary>
            /// Initializes a new instance of the <see cref="StretchableContent"/> class.
            /// </summary>
            public StretchableContent()
            {
                this.DefaultStyleKey = typeof(StretchableContent);
                base.ApplyTemplate();
                ApplyTemplate();
            }


            /// <summary>
            /// Invoked whenever application code or internal processes (such as a rebuilding layout pass) call ApplyTemplate.
            /// In simplest terms, this means the method is called just before a UI element displays in your app.
            /// Override this method to influence the default post-template logic of a class.
            /// </summary>
            protected override void OnApplyTemplate()
            {
                base.OnApplyTemplate();

                // Corners
                this.bottomRight = GetTemplateChild(BottomRightName) as FloatingContent;
                if (this.bottomRight != null)
                {
                    this.bottomRight.ManipulationCompleted += Corner_ManipulationCompleted;
                    this.bottomRight.ManipulationStarted += Corner_ManipulationStarted;
                    this.bottomRight.ManipulationDelta += Corner_ManipulationDelta;
                    this.bottomRight.PointerEntered += Corner_PointerEntered;
                    this.bottomRight.PointerExited += Corner_PointerExited;
                    this.bottomRight.DragDelayMilliseconds = 0;
                }

                this.bottomLeft = GetTemplateChild(BottomLeftName) as FloatingContent;
                if (this.bottomLeft != null)
                {
                    this.bottomLeft.ManipulationCompleted += Corner_ManipulationCompleted;
                    this.bottomLeft.ManipulationStarted += Corner_ManipulationStarted;
                    this.bottomLeft.ManipulationDelta += Corner_ManipulationDelta;
                    this.bottomLeft.PointerEntered += Corner_PointerEntered;
                    this.bottomLeft.PointerExited += Corner_PointerExited;
                    this.bottomLeft.DragDelayMilliseconds = 0;
                }

                this.topRight = GetTemplateChild(TopRightName) as FloatingContent;
                if (this.topRight != null)
                {
                    this.topRight.ManipulationCompleted += Corner_ManipulationCompleted;
                    this.topRight.ManipulationStarted += Corner_ManipulationStarted;
                    this.topRight.ManipulationDelta += Corner_ManipulationDelta;
                    this.topRight.PointerEntered += Corner_PointerEntered;
                    this.topRight.PointerExited += Corner_PointerExited;
                    this.topRight.DragDelayMilliseconds = 0;
                }

                this.topLeft = GetTemplateChild(TopLeftName) as FloatingContent;
                if (this.topLeft != null)
                {
                    this.topLeft.ManipulationCompleted += Corner_ManipulationCompleted;
                    this.topLeft.ManipulationStarted += Corner_ManipulationStarted;
                    this.topLeft.ManipulationDelta += Corner_ManipulationDelta;
                    this.topLeft.PointerEntered += Corner_PointerEntered;
                    this.topLeft.PointerExited += Corner_PointerExited;
                    this.topLeft.DragDelayMilliseconds = 0;
                }

                this.grid = GetTemplateChild(GridName) as Grid;
                if (this.grid != null)
                {
                    grid.CornerRadius = this.CornerRadius;
                }

                this.Loaded += Stretchable_Loaded;
                this.PointerEntered += This_PointerEntered;
                this.PointerExited += This_PointerExited;
            }
            private void Corner_ManipulationDelta(object sender, ManipulationDeltaRoutedEventArgs e)
            {
                e.Handled = true;
            }

            private void Corner_ManipulationStarted(object sender, ManipulationStartedRoutedEventArgs e)
            {
                e.Handled = true;
            }

            private void adjustFloatingContentCanvas(FloatingContent corner, double minWidth, double minHeight, double x, double y)
            {
                foreach (var parent in this.FindParents().OfType<FloatingContent>())
                {
                    switch (corner.Name)
                    {
                        case BottomRightName:
                            break;
                        case BottomLeftName:
                            {
                                var left = Canvas.GetLeft(parent.border) + (this.ActualWidth - Math.Max(minWidth, this.ActualWidth - x));
                                var top = Canvas.GetTop(parent.border);// + (this.ActualHeight - Math.Max(minHeight, this.ActualHeight + y));

                                Rect rect = new Rect(left, top, Math.Max(1, parent.border.ActualWidth), Math.Max(1, parent.border.ActualHeight));
                                parent.AdjustCanvasPosition(rect);
                            }
                            break;
                        case TopRightName:
                            {
                                var left = Canvas.GetLeft(parent.border);// + (this.ActualWidth - Math.Max(minWidth, this.ActualWidth + x));
                                var top = Canvas.GetTop(parent.border) + (this.ActualHeight - Math.Max(minHeight, this.ActualHeight - y));

                                Rect rect = new Rect(left, top, Math.Max(1, parent.border.ActualWidth), Math.Max(1, parent.border.ActualHeight));
                                parent.AdjustCanvasPosition(rect);
                            }
                            break;
                        case TopLeftName:
                            {
                                var left = Canvas.GetLeft(parent.border) + (this.ActualWidth - Math.Max(minWidth, this.ActualWidth - x));
                                var top = Canvas.GetTop(parent.border) + (this.ActualHeight - Math.Max(minHeight, this.ActualHeight - y));

                                Rect rect = new Rect(left, top, Math.Max(1, parent.border.ActualWidth), Math.Max(1, parent.border.ActualHeight));
                                parent.AdjustCanvasPosition(rect);
                            }
                            break;
                    }
                }
            }

            private void Corner_ManipulationCompleted(object sender, ManipulationCompletedRoutedEventArgs e)
            {
                if (e != null) e.Handled = true;

                double minHeight = 20;
                double minWidth = 20;

                if (sender is FloatingContent)
                {
                    double x = e == null ? 0 : e.Cumulative.Translation.X;
                    double y = e == null ? 0 : e.Cumulative.Translation.Y;

                    // determine if there are limits to our expansion 
                    {
                        FrameworkElement el = GetClosestParentWithSize(this.Parent as FrameworkElement);
                        if (el != null)
                        {
                            Rect parentRect = new Rect(0, 0, Math.Max(1, el.ActualWidth), Math.Max(1, el.ActualHeight));
                            // represents the total limits;
                            if (this.Parent is FloatingContent)
                            {
                                switch ((sender as FloatingContent).Name)
                                {
                                    case BottomRightName:
                                        {
                                            x = Math.Min(x, parentRect.Width - (Canvas.GetLeft((this.Parent as FloatingContent).border) + this.ActualWidth));
                                        }

                                        {
                                            y = Math.Min(y, parentRect.Height - (Canvas.GetTop((this.Parent as FloatingContent).border) + this.ActualHeight));
                                        }

                                        break;
                                    case BottomLeftName:
                                        {
                                            x = Math.Max(x, -Canvas.GetLeft((this.Parent as FloatingContent).border));
                                        }

                                        {
                                            y = Math.Min(y, parentRect.Height - (Canvas.GetTop((this.Parent as FloatingContent).border) + this.ActualHeight));
                                        }

                                        break;
                                    case TopRightName:
                                        {
                                            x = Math.Min(x, parentRect.Width - (Canvas.GetLeft((this.Parent as FloatingContent).border) + this.ActualWidth));
                                        }

                                        {
                                            y = Math.Max(y, -Canvas.GetTop((this.Parent as FloatingContent).border));
                                        }

                                        break;
                                    case TopLeftName:
                                        {
                                            x = Math.Max(x, -Canvas.GetLeft((this.Parent as FloatingContent).border));
                                        }

                                        {
                                            y = Math.Max(y, -Canvas.GetTop((this.Parent as FloatingContent).border));
                                        }

                                        break;
                                }
                            }
                            else
                            {
                                switch ((sender as FloatingContent).Name)
                                {
                                    case BottomRightName:
                                        {
                                            x = Math.Min(x, parentRect.Width - (this.ActualWidth));
                                        }

                                        {
                                            y = Math.Min(y, parentRect.Height - (this.ActualHeight));
                                        }

                                        break;
                                    case BottomLeftName:
                                        {
                                            x = Math.Max(x, 0);
                                        }

                                        {
                                            y = Math.Min(y, parentRect.Height - (this.ActualHeight));
                                        }

                                        break;
                                    case TopRightName:
                                        {
                                            x = Math.Min(x, parentRect.Width - (this.ActualWidth));
                                        }

                                        {
                                            y = Math.Max(y, 0);
                                        }

                                        break;
                                    case TopLeftName:
                                        {
                                            x = Math.Max(x, 0);
                                        }

                                        {
                                            y = Math.Max(y, 0);
                                        }

                                        break;
                                }
                            }
                        }
                    }

                    if (x == 0 && y == 0)
                    {
                        return;
                    }

                    double prevW = this.ActualWidth;
                    double prevH = this.ActualHeight;

                    switch ((sender as FloatingContent).Name)
                    {
                        case BottomRightName:
                            this.Width = Math.Max(minWidth, this.ActualWidth + x);
                            this.Height = Math.Max(minHeight, this.ActualHeight + y);

                            break;
                        case BottomLeftName:

                            this.Width = Math.Max(minWidth, this.ActualWidth - x);
                            this.Height = Math.Max(minHeight, this.ActualHeight + y);
                            adjustFloatingContentCanvas(sender as FloatingContent, minWidth, minHeight, x, y);

                            break;
                        case TopRightName:

                            this.Width = Math.Max(minWidth, this.ActualWidth + x);
                            this.Height = Math.Max(minHeight, this.ActualHeight - y);
                            adjustFloatingContentCanvas(sender as FloatingContent, minWidth, minHeight, x, y);

                            break;
                        case TopLeftName:

                            this.Width = Math.Max(minWidth, this.ActualWidth - x);
                            this.Height = Math.Max(minHeight, this.ActualHeight - y);
                            adjustFloatingContentCanvas(sender as FloatingContent, minWidth, minHeight, x, y);

                            break;
                    }

                    SetCorner(this.bottomRight);
                    SetCorner(this.bottomLeft);
                    SetCorner(this.topRight);
                    SetCorner(this.topLeft);

                    if (e != null) StretchedEvent.InvokeEventAsync(null, false);
                }
            }



            private void AnimateCornerOpacity(FloatingContent sender, double opacity)
            {
                var ani = new DoubleAnimation()
                {
                    From = sender.Opacity,
                    To = opacity,
                    Duration = new Duration(FloatingContent.border_flashTime),
                    AutoReverse = false
                };
                var storyBoard = new Storyboard();
                storyBoard.Children.Add(ani);
                Storyboard.SetTarget(ani, sender);
                ani.SetValue(Storyboard.TargetPropertyProperty, "Opacity");
                storyBoard.Begin();
            }


            private void ShowCorners()
            {
                AnimateCornerOpacity(this.bottomRight, FloatingContent.border_max_opacity);
                AnimateCornerOpacity(this.bottomLeft, FloatingContent.border_max_opacity);
                AnimateCornerOpacity(this.topRight, FloatingContent.border_max_opacity);
                AnimateCornerOpacity(this.topLeft, FloatingContent.border_max_opacity);
            }
            private void HideCorners()
            {
                AnimateCornerOpacity(this.bottomRight, FloatingContent.border_min_opacity);
                AnimateCornerOpacity(this.bottomLeft, FloatingContent.border_min_opacity);
                AnimateCornerOpacity(this.topRight, FloatingContent.border_min_opacity);
                AnimateCornerOpacity(this.topLeft, FloatingContent.border_min_opacity);
            }
            private void This_PointerEntered(object sender, PointerRoutedEventArgs e)
            {
                ShowCorners();
            }
            private void This_PointerExited(object sender, PointerRoutedEventArgs e)
            {
                HideCorners();
            }

            private void Corner_PointerEntered(object sender, PointerRoutedEventArgs e)
            {
                switch ((sender as FloatingContent).Name)
                {
                    case BottomRightName:
                        Windows.UI.Xaml.Window.Current.CoreWindow.PointerCursor = new Windows.UI.Core.CoreCursor(Windows.UI.Core.CoreCursorType.SizeNorthwestSoutheast, 1);

                        break;
                    case BottomLeftName:
                        Windows.UI.Xaml.Window.Current.CoreWindow.PointerCursor = new Windows.UI.Core.CoreCursor(Windows.UI.Core.CoreCursorType.SizeNortheastSouthwest, 1);

                        break;
                    case TopRightName:
                        Windows.UI.Xaml.Window.Current.CoreWindow.PointerCursor = new Windows.UI.Core.CoreCursor(Windows.UI.Core.CoreCursorType.SizeNortheastSouthwest, 1);

                        break;
                    case TopLeftName:
                        Windows.UI.Xaml.Window.Current.CoreWindow.PointerCursor = new Windows.UI.Core.CoreCursor(Windows.UI.Core.CoreCursorType.SizeNorthwestSoutheast, 1);
                        break;
                }
            }
            private void Corner_PointerExited(object sender, PointerRoutedEventArgs e)
            {
                Windows.UI.Xaml.Window.Current.CoreWindow.PointerCursor = new Windows.UI.Core.CoreCursor(Windows.UI.Core.CoreCursorType.Arrow, 1);
            }
            private void SetCorner(object sender)
            {
                double w;
                double h;

                if (Double.IsNaN(this.Width))
                {
                    w = this.ActualWidth;
                }
                else
                {
                    w = this.Width;
                }
                if (Double.IsNaN(this.Height))
                {
                    h = this.ActualHeight;
                }
                else
                {
                    h = this.Height;
                }

                if (!Double.IsNaN(this.MaxWidth))
                {
                    w = Math.Min(w, this.MaxWidth);
                }
                if (!Double.IsNaN(this.MaxHeight))
                {
                    h = Math.Min(h, this.MaxHeight);
                }

                if (!Double.IsNaN(this.MinWidth))
                {
                    w = Math.Max(w, this.MinWidth);
                }
                if (!Double.IsNaN(this.MinHeight))
                {
                    h = Math.Max(h, this.MinHeight);
                }

                switch ((sender as FloatingContent).Name)
                {
                    case BottomRightName:
                        Canvas.SetLeft((sender as FloatingContent).border, w - (cornerWidth + 1));
                        Canvas.SetTop((sender as FloatingContent).border, h - (cornerWidth + 1));
                        Canvas.SetLeft((sender as FloatingContent), 0);
                        Canvas.SetTop((sender as FloatingContent), 0);

                        break;
                    case BottomLeftName:
                        Canvas.SetLeft((sender as FloatingContent).border, 0);
                        Canvas.SetTop((sender as FloatingContent).border, h - (cornerWidth + 1));
                        Canvas.SetLeft((sender as FloatingContent), 0);
                        Canvas.SetTop((sender as FloatingContent), 0);

                        break;
                    case TopRightName:
                        Canvas.SetLeft((sender as FloatingContent).border, w - (cornerWidth + 1));
                        Canvas.SetTop((sender as FloatingContent).border, 0);
                        Canvas.SetLeft((sender as FloatingContent), 0);
                        Canvas.SetTop((sender as FloatingContent), 0);

                        break;
                    case TopLeftName:
                        Canvas.SetLeft((sender as FloatingContent).border, 0);
                        Canvas.SetTop((sender as FloatingContent).border, 0);
                        Canvas.SetLeft((sender as FloatingContent), 0);
                        Canvas.SetTop((sender as FloatingContent), 0);

                        break;
                }
            }

            private void Stretchable_Loaded(object sender, RoutedEventArgs e)
            {
                if (!LoadedAlready)
                {
                    LoadedAlready = true;

                    FrameworkElement el = GetClosestParentWithSize(this);
                    if (el == null)
                    {
                        return;
                    }

                    SetCorner(this.bottomRight);
                    SetCorner(this.bottomLeft);
                    SetCorner(this.topRight);
                    SetCorner(this.topLeft);

                    el.SizeChanged += Stretchable_SizeChanged;
                }
            }

            private void Stretchable_SizeChanged(object sender, SizeChangedEventArgs e)
            {
                Corner_ManipulationCompleted(this.bottomRight, null);
                Corner_ManipulationCompleted(this.bottomLeft, null);
                Corner_ManipulationCompleted(this.topRight, null);
                Corner_ManipulationCompleted(this.topLeft, null);

                SetCorner(this.bottomRight);
                SetCorner(this.bottomLeft);
                SetCorner(this.topRight);
                SetCorner(this.topLeft);
            }

            /// <summary>
            /// Returns the adjusted the topleft position of a rectangle so that is stays within a parent rectangle.
            /// </summary>
            /// <param name="rect">The rectangle.</param>
            /// <param name="parentRect">The parent rectangle.</param>
            /// <returns></returns>
            private Point AdjustedPosition(Rect rect, Rect parentRect)
            {
                var left = rect.Left;
                var top = rect.Top;

                if (left < -parentRect.Left)
                {
                    // Left boundary hit.
                    left = -parentRect.Left;
                }
                else if (left + rect.Width > parentRect.Width)
                {
                    // Right boundary hit.
                    left = parentRect.Width - rect.Width;
                }

                if (top < -parentRect.Top)
                {
                    // Top hit.
                    top = -parentRect.Top;
                }
                else if (top + rect.Height > parentRect.Height)
                {
                    // Bottom hit.
                    top = parentRect.Height - rect.Height;
                }

                return new Point(left, top);
            }

            /// <summary>
            /// Gets the closest parent with a real size.
            /// </summary>
            private FrameworkElement GetClosestParentWithSize(FrameworkElement element)
            {
                while (element != null && (element.ActualHeight == 0 || element.ActualWidth == 0))
                {
                    // Crawl up the Visual Tree.
                    element = element.Parent as FrameworkElement;
                }

                return element;
            }
        }








    }
}