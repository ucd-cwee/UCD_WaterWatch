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
using Windows.UI.Xaml.Controls;
using System.ComponentModel;
using UWP_WaterWatch;
using System.Runtime.CompilerServices;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Media;
using Telerik.Charting;
using System.Collections.Generic;
using static UWP_WaterWatchLibrary.cweeXamlHelper;
using Telerik.UI.Xaml.Controls.Chart;
using UWP_WaterWatch.Pages;

namespace UWP_WaterWatchLibrary
{
    public static class Extensions
    {
        public static bool isNumeric(this string strToCheck)
        {
            System.Text.RegularExpressions.Regex rg = new System.Text.RegularExpressions.Regex(@"^[0-9_]+$");
            return rg.IsMatch(strToCheck);
        }
        public static bool isAlphaNumeric(this string strToCheck)
        {
            System.Text.RegularExpressions.Regex rg = new System.Text.RegularExpressions.Regex(@"^[a-zA-Z0-9_]+$");
            return rg.IsMatch(strToCheck);
        }
        public static bool isAlpha(this string strToCheck)
        {
            System.Text.RegularExpressions.Regex rg = new System.Text.RegularExpressions.Regex(@"^[a-zA-Z_]+$");
            return rg.IsMatch(strToCheck);
        }

        public static bool IsNumeric(this char v)
        {
            return $"{v}".isNumeric();
        }
        public static bool IsAlphaNumeric(this char v)
        {
            return $"{v}".isAlphaNumeric();
        }
        public static bool IsAlpha(this char v)
        {
            return $"{v}".isAlpha();
        }


        public static char ToSuperScript(this char v)
        {
            switch (v)
            {
                case '0': return '⁰';
                case '1': return '¹';
                case '2': return '²';
                case '3': return '³';
                case '4': return '⁴';
                case '5': return '⁵';
                case '6': return '⁶';
                case '7': return '⁷';
                case '8': return '⁸';
                case '9': return '⁹';
                case '-': return '⁻';
                default: return v;
            }
        }
        public static string ToSmallString(this double v) { 
            var s = v.ToString("G4", System.Globalization.CultureInfo.InvariantCulture);            
            s = s.Replace("E+0", "E+");
            s = s.Replace("E-0", "E-");
            s = s.Replace("E+", "E");
            s = s.Replace("E", "^");
            var splits = s.Split("^");
            if (splits.Length > 1)
            {
                s = splits[0];
                foreach (char x in splits[1])
                {
                    s = s + x.ToSuperScript();
                }
            }
            return s;
        }
        public static DateTime LocalDateTime(this cweeDateTime dt)
        {
            return new DateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.milliseconds);
        }

        public static string GetCurrentTabName(this Microsoft.UI.Xaml.Controls.TabView nv)
        {
            Microsoft.UI.Xaml.Controls.TabViewItem tab = nv.SelectedItem as Microsoft.UI.Xaml.Controls.TabViewItem;
            if (tab != null)
                return tab.GetTabName();
            return "";
        }
        public static string GetTabName(this Microsoft.UI.Xaml.Controls.TabViewItem tab)
        {
            if (tab != null)
            {
                return TryGetText(tab.Header);
            }
            return "";
        }
        public static string TryGetText<T>(T container) where T : class
        {
            if (container == null) { return null; }
            if (container is Border)
            {
                return TryGetText((container as Border).Child);
            }
            if (container is Panel)
            {
                foreach (var child in (container as Panel).Children)
                {
                    var o = TryGetText(child);
                    if (o != null) return o;
                }
                return null;
            }
            if (container is TextBlock)
            {
                return TryGetText((container as TextBlock).Text);
            }
            if (container is TextBox)
            {
                return TryGetText((container as TextBox).Text);
            }
            if (container is RichEditBox)
            {
                (container as RichEditBox).TextDocument.GetText(Windows.UI.Text.TextGetOptions.NoHidden, out string b);
                return TryGetText(b);
            }
            if (container is string)
            {
                return container as string;
            }

            return null;
        }

        public static cweeTask<Border> CreateTabHeader(string Label)
        {
            var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            return cdb.ContinueWith(()=> {
                var toReturn = new Border()
                {
                    Margin = new Windows.UI.Xaml.Thickness(0),
                    Padding = new Windows.UI.Xaml.Thickness(0),
                    MinWidth = 0,
                    MinHeight = 0
                };

                var tb = new TextBlock()
                {
                    Text = Label,
                    Margin = new Windows.UI.Xaml.Thickness(0),
                    Padding = new Windows.UI.Xaml.Thickness(0),
                    HorizontalAlignment = Windows.UI.Xaml.HorizontalAlignment.Center,
                    HorizontalTextAlignment = Windows.UI.Xaml.TextAlignment.Center,
                    VerticalAlignment = Windows.UI.Xaml.VerticalAlignment.Center,
                    Foreground = cdb.Result,
                    MinWidth = 0,
                    MinHeight = 0
                };

                toReturn.Child = tb;

                return toReturn;
            }, true);
        }
        public static cweeTask<Border> CreateEditableTabHeader(Binding Label) // ScriptingPageViewModel labelVM) // Binding Label
        {
            var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            return cdb.ContinueWith(() =>
            {
                var toReturn = new Border()
                {
                    Margin = new Windows.UI.Xaml.Thickness(0),
                    Padding = new Windows.UI.Xaml.Thickness(0),
                    MinWidth = 0,
                    MinHeight = 0
                };

                var reb = new RichEditBox()
                {
                    IsRightTapEnabled = false,
                    VerticalAlignment = VerticalAlignment.Center,
                    HorizontalAlignment = HorizontalAlignment.Center,
                    VerticalContentAlignment = VerticalAlignment.Center,
                    Foreground = cdb.Result,
                    Style = cweeXamlHelper.StaticStyleResource("cweeRichEditBox"),
                    FontSize = 12,
                    IsSpellCheckEnabled = false,
                    CornerRadius = new CornerRadius(0),
                    BorderBrush = new Windows.UI.Xaml.Media.SolidColorBrush(new Windows.UI.Color() { A = 0 }),
                    BorderThickness = new Thickness(1),
                    Margin = new Thickness(0),
                    Padding = new Thickness(0),
                    MinHeight = 0,
                    MinWidth = 0,
                    PlaceholderText = "Script Name",
                    IsTextPredictionEnabled = false,
                    AcceptsReturn = false,
                    TextWrapping = TextWrapping.NoWrap,
                    MaxLength = 32
                };

                ScrollViewer.SetVerticalScrollMode(reb, ScrollMode.Disabled);
                ScrollViewer.SetHorizontalScrollMode(reb, ScrollMode.Disabled);

                reb.SetBinding(UWP_WaterWatch.Custom_Controls.RichEditBoxExtension.PlainTextProperty, Label);
                toReturn.Child = reb;

                return toReturn;

            }, true);
        }

        //public static string UniqueObjectID(this UIElement obj)
        //{
        //    return obj.GetHashCode().ToString();
        //}
        
    }

    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        public ApplicationInfo AppInfo
        {
            get
            {
                return AppExtension.ApplicationInfo;
            }
            set
            {
                return; // do nothing
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        //cweeDequeue propertyChangedDeq = new cweeDequeue();
        //EdmsQueue<PropertyChangedEventArgs> propertyChangeEvents = new EdmsQueue<PropertyChangedEventArgs>();

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null, [System.Runtime.CompilerServices.CallerMemberName] string membername = "", [System.Runtime.CompilerServices.CallerFilePath] string filepath = "", [System.Runtime.CompilerServices.CallerLineNumber] int linenumber = 0)
        {
#if true
            if (PropertyChanged != null)
            {
                EdmsTasks.InsertJob(() => {
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
                }, EdmsTasks.Priority.Low, true, true, membername, filepath, linenumber);
            }
#else
            propertyChangeEvents.Enqueue(new PropertyChangedEventArgs(propertyName));
            propertyChangedDeq.Dequeue(DateTime.Now.AddSeconds(0.1), () =>
            {
                foreach (var k in propertyChangeEvents.DequeueAll())
                {
                    var j = k;
                    EdmsTasks.InsertJob(() =>
                    {
                        try
                        {
                            PropertyChanged?.Invoke(this, j);
                        }
                        catch (Exception)
                        {
                            EdmsTasks.InsertJob(() =>
                            {
                                PropertyChanged?.Invoke(this, j);
                            }, true);
                        }
                    }, false);
                }  
            }, false);
#endif
        }
    }

#if true

    public class TelerikChartData
    {
        public enum TelerikChartType { Area, Scatter, Line, Candlestick };
        public TelerikChartType type { get; set; } = TelerikChartType.Area;
        public object spline_data { get; set; } = null; // Binding or List<ChartItem>
        public SolidColorBrush fillColor { get; set; } = null; // color
        public SolidColorBrush strokeColor { get; set; } = null; // color
        public double? strokeThickness { get; set; } = null; // double
        public string ScatterPointTemplate { get; set; } = null;
    }
    public class TelerikChartDetails
    {
        public object y_axis_title = null; // binding OR string            
        public object y_axis_min = null; // binding OR double
        public object y_axis_max = null; // binding OR double
        public object y_axis_majorStep = null; // binding OR double
        public IContentFormatter y_axis_formatter = new ScadaAxisLabelFormatter(); // instance of a formatter
        public IContentFormatter x_axis_formatter = new DateTimeAxisLabelFormatter_ReducedFrequency(); // instance of a formatter
        public object x_axis_min = null; // binding OR DateTime
        public object x_axis_max = null; // binding OR DateTime
        public object X_axis_title = null; // binding OR string    
        public List<TelerikChartData> charts = new List<TelerikChartData>();
        //public event Chart.ClosestDataPointChangedEventHandler ClosestDataPointChanged;
        //public void OnClosestDataPointChanged(object sender, CategoricalDataPoint dataPoint)
        //{
        //if (ClosestDataPointChanged != null)
        //{
        //    ClosestDataPointChanged(sender, new Chart.Annotations.DataPointEventArgs(dataPoint));
        //}
        //}
        //public void ChartTrackBallBehavior_TrackInfoUpdated(object sender, Telerik.UI.Xaml.Controls.Chart.TrackBallInfoEventArgs e)
        //{
        //CategoricalDataPoint closestDataPoint = e.Context.ClosestDataPoint.DataPoint as CategoricalDataPoint;
        //OnClosestDataPointChanged(sender, closestDataPoint);
        //}
    }

    public class ChartItem
    {
        public float Value { get; set; }
        public float High;
        public float Low;
        public DateTime Date { get; set; }
        public int ColorData;
    }

    public class ScadaAxisLabelFormatter : IContentFormatter
    {
        public object Format(object owner, object content)
        {
            float value = float.Parse(content.ToString());
            if (value < 0)
            { // dealing with negative
                value = MathF.Abs(value);
            }

            if (value < 0.01)
            {
                return String.Format("{0:0.0000}", value);
            }
            else if (value >= 0.01 && value < 0.1)
            {
                return String.Format("{0:00.000}", value);
            }
            else if (value >= 0.1 && value < 1)
            {
                return String.Format("{0:000.00}", value);
            }
            else if (value >= 1.0 && value < 10)
            {
                return String.Format("{0:0000.0}", value);
            }
            else if (value >= 10.0 && value < 100)
            {
                return String.Format("{0:0000.0}", value);
            }
            else if (value >= 100.0 && value < 1000)
            {
                return String.Format("{0:0000.0}", value);
            }
            else if (value >= 1000.0 && value < 10000)
            {
                return String.Format("{0:00,000}", (int)value);
            }
            else if (value >= 10000.0 && value < 100000)
            {
                return String.Format("{0:00,000}", (int)value);
            }
            else if (value >= 100000.0 && value < 1000000)
            {
                return String.Format("{0:000.0k}", (value / 1000f));
            }
            else if (value >= 1000000.0 && value < 10000000)
            {
                return String.Format("{0:0,000k}", (int)(value / 1000f));
            }
            else if (value >= 10000000.0)
            {
                return String.Format("{0:0,000M}", (int)(value / 1000000f));
            }

            return String.Format("{0:000.00}", value);
        }
    }

    public class DirectionalScadaAxisLabelFormatter : IContentFormatter
    {
        public object Format(object owner, object content)
        {
            float value = float.Parse(content.ToString());

            if (value < 0)
            { // dealing with negative
                value = MathF.Abs(value);
                if (value < 0.01)
                {
                    return String.Format("{0:-0.0000}", value);
                }
                else if (value >= 0.01 && value < 0.1)
                {
                    return String.Format("{0:-00.000}", value);
                }
                else if (value >= 0.1 && value < 1)
                {
                    return String.Format("{0:-000.00}", value);
                }
                else if (value >= 1.0 && value < 10)
                {
                    return String.Format("{0:-0000.0}", value);
                }
                else if (value >= 10.0 && value < 100)
                {
                    return String.Format("{0:-0000.0}", value);
                }
                else if (value >= 100.0 && value < 1000)
                {
                    return String.Format("{0:-0000.0}", value);
                }
                else if (value >= 1000.0 && value < 10000)
                {
                    return String.Format("{0:-00,000}", (int)value);
                }
                else if (value >= 10000.0 && value < 100000)
                {
                    return String.Format("{0:-00,000}", (int)value);
                }
                else if (value >= 100000.0 && value < 1000000)
                {
                    return String.Format("{0:-000.0k}", (value / 1000f));
                }
                else if (value >= 1000000.0 && value < 10000000)
                {
                    return String.Format("{0:-0,000k}", (int)(value / 1000f));
                }
                else if (value >= 10000000.0)
                {
                    return String.Format("{0:-0,000M}", (int)(value / 1000000f));
                }
            }

            if (value < 0.01)
            {
                return String.Format("{0:+0.0000}", value);
            }
            else if (value >= 0.01 && value < 0.1)
            {
                return String.Format("{0:+00.000}", value);
            }
            else if (value >= 0.1 && value < 1)
            {
                return String.Format("{0:+000.00}", value);
            }
            else if (value >= 1.0 && value < 10)
            {
                return String.Format("{0:+0000.0}", value);
            }
            else if (value >= 10.0 && value < 100)
            {
                return String.Format("{0:+0000.0}", value);
            }
            else if (value >= 100.0 && value < 1000)
            {
                return String.Format("{0:+0000.0}", value);
            }
            else if (value >= 1000.0 && value < 10000)
            {
                return String.Format("{0:+00,000}", (int)value);
            }
            else if (value >= 10000.0 && value < 100000)
            {
                return String.Format("{0:+00,000}", (int)value);
            }
            else if (value >= 100000.0 && value < 1000000)
            {
                return String.Format("{0:+000.0k}", (value / 1000f));
            }
            else if (value >= 1000000.0 && value < 10000000)
            {
                return String.Format("{0:+0,000k}", (int)(value / 1000f));
            }
            else if (value >= 10000000.0)
            {
                return String.Format("{0:+0,000M}", (int)(value / 1000000f));
            }

            return String.Format("{0:?000.00}", value);
        }
    }
    public class TimeAxisLabelFormatter : IContentFormatter
    {
        public object Format(object owner, object content)
        {
            string subject = content.ToString();

            DateTime value = DateTime.Parse(subject);
            DateTime now = WaterWatch.getCurrentTime().LocalDateTime();

            if (((value.Hour - now.Hour) % 2.0) == 0.0 && value.Minute < 30)
            {
                return String.Format("{0,0:HH}", value);
            }
            else
            {
                return "";
            }
        }
    }

    public class DateTimeAxisLabelFormatter : IContentFormatter
    {
        public static int _countUp = 0;
        public static int _countDown = -1;
        public static DateTime lastDateTime = WaterWatch.getCurrentTime().LocalDateTime().AddDays(10000);
        public object Format(object owner, object content)
        {
            string subject = content.ToString();

            DateTime value = DateTime.Parse(subject);
            DateTime now = WaterWatch.getCurrentTime().LocalDateTime();

            if (lastDateTime > value)
            {
                _countUp = 0;
                _countDown = 6;
            }
            lastDateTime = value;

            float hour1 = now.Hour + 1;
            float hour2 = now.Hour + 13;
            hour1 =
                ((hour1 % 23) + 23) % 23;
            hour2 =
                ((hour2 % 23) + 23) % 23;

            hour1 = MathF.Round(hour1, 0);
            hour2 = MathF.Round(hour2, 0);

            if (value.DayOfYear == now.DayOfYear && now.Hour == value.Hour)
            {
                _countUp = 0;
                _countDown = 6;
                return String.Format("{0:ddd, MMM dd}" + Environment.NewLine + "{0,0:HH}", value);
            }

            if (_countDown >= 0)
            {
                _countDown--;
                _countUp++;
                return String.Format(" " + Environment.NewLine + "{0,0:HH}", value);
            }


            if (value.Hour == hour1 || value.Hour == hour2 || _countUp >= 12)
            {
                _countUp = 0;
                _countDown = 6;
                return String.Format("{0:ddd, MMM dd}" + Environment.NewLine + "{0,0:HH}", value);
            }
            else
            {
                _countUp++;
                return String.Format(" " + Environment.NewLine + "{0,0:HH}", value);
            }
        }
    }

    public class EmptyHorizontalLabelFormatter : IContentFormatter
    {
        public object Format(object owner, object content)
        {
            return "";
        }
    }

    public class LinearAxisLabelFormatter_Customized : IContentFormatter
    {
        public string FormatAdditive = "";
        public static int _countUp = 0;
        public static int _countDown = -1;
        public static DateTime lastDateTime = WaterWatch.getCurrentTime().LocalDateTime().AddDays(10000);
        
        public object Format(object owner, object content)
        {
            string subject = content.ToString();
            DateTime value = DateTime.Parse(subject);
            DateTime now = WaterWatch.getCurrentTime().LocalDateTime();

            float hour1 = now.Hour + 1;
            float hour2 = now.Hour + 13;
            hour1 =
                ((hour1 % 23) + 23) % 23;
            hour2 =
                ((hour2 % 23) + 23) % 23;

            hour1 = MathF.Round(hour1, 0);
            hour2 = MathF.Round(hour2, 0);

            if (lastDateTime > value)
            {
                _countUp = 0;
                _countDown = 6;
            }
            lastDateTime = value;

            if (_countDown >= 0)
            {
                _countDown--;
                _countUp++;
                if (_countUp % 3 == 0 && !showHour(value, now, 1) && !showHour(value, now, 2))
                    return " " + Environment.NewLine + value.ToUnixTimeSeconds().ToSmallString();
                else
                    return " " + Environment.NewLine + " ";

            }
            else if (value.Hour == hour1 || value.Hour == hour2 || _countUp >= 12)
            {
                _countUp = 0;
                _countDown = 6;
                if (_countUp % 3 == 0 && !showHour(value, now, 1) && !showHour(value, now, 2))
                    return FormatAdditive.Trim() + Environment.NewLine + value.ToUnixTimeSeconds().ToSmallString();
                else
                    return FormatAdditive.Trim() + Environment.NewLine + " ";
            }
            else
            {
                _countUp++;
                if (_countUp % 3 == 0 && !showHour(value, now, 1) && !showHour(value, now, 2))
                    return " " + Environment.NewLine + value.ToUnixTimeSeconds().ToSmallString();
                else
                    return " " + Environment.NewLine + " ";
            }
        }

        public bool showHour(DateTime value, DateTime now, int stepsBack)
        {
            float hour1 = now.Hour + 1;
            float hour2 = now.Hour + 13;
            hour1 =
                ((hour1 % 23) + 23) % 23;
            hour2 =
                ((hour2 % 23) + 23) % 23;

            hour1 = MathF.Round(hour1, 0);
            hour2 = MathF.Round(hour2, 0);

            if (_countDown >= 0)
            {
                if (Wrap(_countUp - stepsBack, 0, 12) % 3 == 0)
                    return true;
                else
                    return false;

            }
            else if (value.AddHours(-stepsBack).Hour == hour1 || value.AddHours(-stepsBack).Hour == hour2 || Wrap(_countUp - stepsBack, 0, 12) >= 12)
            {
                if (Wrap(_countUp - stepsBack, 0, 12) % 3 == 0)
                    return true;
                else
                    return false;
            }
            else
            {
                if (Wrap(_countUp - stepsBack, 0, 12) % 3 == 0)
                    return true;
                else
                    return false;
            }
        }

        public int Wrap(int kX, int kLowerBound, int kUpperBound)
        {
            int range_size = kUpperBound - kLowerBound + 1;

            if (kX < kLowerBound)
                kX += range_size * ((kLowerBound - kX) / range_size + 1);

            return kLowerBound + (kX - kLowerBound) % range_size;
        }
    }

    public class DateTimeAxisLabelFormatter_ReducedFrequency : IContentFormatter
    {
        public static int _countUp = 0;
        public static int _countDown = -1;
        public static DateTime lastDateTime = WaterWatch.getCurrentTime().LocalDateTime().AddDays(10000);
        public object Format(object owner, object content)
        {
            string subject = content.ToString();
            DateTime value = DateTime.Parse(subject);
            DateTime now = WaterWatch.getCurrentTime().LocalDateTime();

            float hour1 = now.Hour + 1;
            float hour2 = now.Hour + 13;
            hour1 =
                ((hour1 % 23) + 23) % 23;
            hour2 =
                ((hour2 % 23) + 23) % 23;

            hour1 = MathF.Round(hour1, 0);
            hour2 = MathF.Round(hour2, 0);

            if (lastDateTime > value)
            {
                _countUp = 0;
                _countDown = 6;
            }
            lastDateTime = value;

            if (_countDown >= 0)
            {
                _countDown--;
                _countUp++;
                if (_countUp % 3 == 0 && !showHour(value, now, 1) && !showHour(value, now, 2))
                    return String.Format(" " + Environment.NewLine + "{0,0:HH}", value);
                else
                    return String.Format(" " + Environment.NewLine + " ", value);

            }
            else if (value.Hour == hour1 || value.Hour == hour2 || _countUp >= 12)
            {
                _countUp = 0;
                _countDown = 6;
                if (_countUp % 3 == 0 && !showHour(value, now, 1) && !showHour(value, now, 2))
                    return String.Format("{0:ddd, MMM dd}" + Environment.NewLine + "{0,0:HH}", value);
                else
                    return String.Format("{0:ddd, MMM dd}" + Environment.NewLine + " ", value);
            }
            else
            {
                _countUp++;
                if (_countUp % 3 == 0 && !showHour(value, now, 1) && !showHour(value, now, 2))
                    return String.Format(" " + Environment.NewLine + "{0,0:HH}", value);
                else
                    return String.Format(" " + Environment.NewLine + " ", value);
            }
        }


        public bool showHour(DateTime value, DateTime now, int stepsBack)
        {
            float hour1 = now.Hour + 1;
            float hour2 = now.Hour + 13;
            hour1 =
                ((hour1 % 23) + 23) % 23;
            hour2 =
                ((hour2 % 23) + 23) % 23;

            hour1 = MathF.Round(hour1, 0);
            hour2 = MathF.Round(hour2, 0);

            if (_countDown >= 0)
            {
                if (Wrap(_countUp - stepsBack, 0, 12) % 3 == 0)
                    return true;
                else
                    return false;

            }
            else if (value.AddHours(-stepsBack).Hour == hour1 || value.AddHours(-stepsBack).Hour == hour2 || Wrap(_countUp - stepsBack, 0, 12) >= 12)
            {
                if (Wrap(_countUp - stepsBack, 0, 12) % 3 == 0)
                    return true;
                else
                    return false;
            }
            else
            {
                if (Wrap(_countUp - stepsBack, 0, 12) % 3 == 0)
                    return true;
                else
                    return false;
            }
        }

        public int Wrap(int kX, int kLowerBound, int kUpperBound)
        {
            int range_size = kUpperBound - kLowerBound + 1;

            if (kX < kLowerBound)
                kX += range_size * ((kLowerBound - kX) / range_size + 1);

            return kLowerBound + (kX - kLowerBound) % range_size;
        }
    }

    public class DateTimeYearAxisLabelFormatter : IContentFormatter
    {
        public static int _countUp = 0;
        public static int _countDown = -1;
        public static DateTime lastDateTime = WaterWatch.getCurrentTime().LocalDateTime().AddDays(10000);

        public object Format(object owner, object content)
        {
            string subject = content.ToString();

            DateTime value = DateTime.Parse(subject);
            DateTime now = WaterWatch.getCurrentTime().LocalDateTime();

            if (lastDateTime > value)
            {
                _countUp = 0;
                _countDown = 6;
            }
            lastDateTime = value;

            float hour1 = now.Hour + 1;
            float hour2 = now.Hour + 13;
            hour1 =
                ((hour1 % 23) + 23) % 23;
            hour2 =
                ((hour2 % 23) + 23) % 23;

            hour1 = MathF.Round(hour1, 0);
            hour2 = MathF.Round(hour2, 0);

            if (_countDown >= 0)
            {
                _countDown--;
                _countUp++;
                return String.Format(" " + Environment.NewLine + " " + Environment.NewLine + "{0,0:HH}", value);
            }
            if (value.Hour == hour1 || value.Hour == hour2 || _countUp >= 12)
            {
                _countUp = 0;
                _countDown = 6;
                return String.Format("{0:yyyy}" + Environment.NewLine + "{0:ddd, MMM dd}" + Environment.NewLine + "{0,0:HH}", value);
            }
            else
            {
                _countUp++;
                return String.Format(" " + Environment.NewLine + " " + Environment.NewLine + "{0,0:HH}", value);
            }
        }
    }

    public static class TelerikHelper
    {
        public static cweeTask<Grid> CreateTelerikChart(TelerikChartDetails chartData)
        {
            var cdb = ThemeColor("cweeDarkBlue");
            var cpbg = ThemeColor("cweePageBackground");
            return EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() { cdb, cpbg }).ContinueWith(()=> {
                return cdb.ContinueWith(() => {
                    Grid outerGrid = new Grid();
                    // outerGrid fromatting
                    {
                        outerGrid.HorizontalAlignment = HorizontalAlignment.Stretch;
                        outerGrid.VerticalAlignment = VerticalAlignment.Stretch;
                        outerGrid.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(20, GridUnitType.Pixel) });
                        outerGrid.ColumnDefinitions.Add(new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Star) });
                        outerGrid.Margin = new Thickness(0, 0, 0, 0);
                        outerGrid.Padding = new Thickness(0, 0, 0, 0);
                    }

                    // YaxisTitleBlock
                    {
                        Border YaxisTitleBlock = new Border();
                        // YaxisTitleBlock formatting
                        {
                            /* Margin="0,0,0,0" VerticalAlignment="Stretch" HorizontalAlignment="Stretch" */
                            YaxisTitleBlock.Margin = new Thickness(0);
                            YaxisTitleBlock.VerticalAlignment = VerticalAlignment.Stretch;
                            YaxisTitleBlock.HorizontalAlignment = HorizontalAlignment.Stretch;
                        }
                        // YaxisTitleBlock content
                        {
                            Microsoft.Toolkit.Uwp.UI.Controls.LayoutTransformControl rotator = new Microsoft.Toolkit.Uwp.UI.Controls.LayoutTransformControl();
                            {
                                rotator.VerticalAlignment = VerticalAlignment.Stretch;
                                rotator.HorizontalAlignment = HorizontalAlignment.Stretch;
                                {
                                    /*
                                            <controls:LayoutTransformControl.Transform>
                                                <TransformGroup>
                                                    <RotateTransform Angle="-90" />
                                                    <ScaleTransform ScaleX="1" ScaleY="1" />
                                                    <SkewTransform AngleX="0" AngleY="0" />
                                                </TransformGroup>
                                            </controls:LayoutTransformControl.Transform>                         
                                     */

                                    TransformGroup tg = new TransformGroup();
                                    tg.Children.Add(new RotateTransform() { Angle = -90 });
                                    tg.Children.Add(new ScaleTransform() { ScaleX = 1, ScaleY = 1 });
                                    tg.Children.Add(new SkewTransform() { AngleX = 0, AngleY = 0 });
                                    rotator.Transform = tg;
                                }
                            }
                            {
                                /*
                                            <Border SizeChanged="ForceChildrenSizesToMatch" VerticalAlignment="Stretch" HorizontalAlignment="Stretch">
                                                <TextBlock Text="{x:Bind vm.YAxisTitle,Mode=OneWay}"  Margin="0" Padding="0"
                                                    FontSize="20" Foreground="{ThemeResource cweeDarkBlue}" TextWrapping="WrapWholeWords"
                                                    HorizontalAlignment="Center" VerticalAlignment="Center" HorizontalTextAlignment="Center"
                                                    TextAlignment="Center"  SizeChanged="AutomaticTextSize"/>
                                            </Border>                     
                                 */
                                Border b = new Border();
                                {
                                    b.SizeChanged += ForceChildrenSizesToMatch;
                                    b.VerticalAlignment = VerticalAlignment.Stretch;
                                    b.HorizontalAlignment = HorizontalAlignment.Stretch;
                                    {
                                        TextBlock tb = new TextBlock();
                                        {
                                            if (chartData.y_axis_title != null)
                                            {
                                                if (chartData.y_axis_title is Binding)
                                                {
                                                    tb.SetBinding(TextBlock.TextProperty, chartData.y_axis_title as Binding);
                                                }
                                                else if (chartData.y_axis_title is string)
                                                {
                                                    tb.Text = chartData.y_axis_title as string;
                                                }
                                            }

                                            tb.Margin = new Thickness(0);
                                            tb.Padding = new Thickness(0);
                                            tb.FontSize = 20;
                                            tb.Foreground = cdb.Result;
                                            tb.TextWrapping = TextWrapping.WrapWholeWords;
                                            tb.HorizontalAlignment = HorizontalAlignment.Center;
                                            tb.VerticalAlignment = VerticalAlignment.Center;
                                            tb.HorizontalTextAlignment = TextAlignment.Center;
                                            tb.TextAlignment = TextAlignment.Center;
                                            tb.SizeChanged += AutomaticTextSize;
                                        }
                                        b.Child = tb;
                                    }
                                }
                                rotator.Child = b;
                            }
                            YaxisTitleBlock.Child = rotator;
                        }
                        outerGrid.Children.Add(YaxisTitleBlock);
                    }

                    // Chart
                    {
                        RadCartesianChart rcc = new RadCartesianChart();
                        { // rcc formatting 
                            rcc.Margin = new Thickness(0, 0, 0, 8); // -35, 8
                            rcc.ClipToBounds = true; // false;
                            rcc.EmptyContent = "No Data Points Found";
                            rcc.Zoom = new Windows.Foundation.Size(1, 1);
                            rcc.HorizontalAlignment = HorizontalAlignment.Stretch;
                            rcc.VerticalAlignment = VerticalAlignment.Stretch;
                        }

                        // rcc behaviors
                        {
                            var tbb = new ChartTrackBallBehavior() { ShowIntersectionPoints = true, LineStyle = StaticStyleResource("trackBallLineStyle") };
                            rcc.Behaviors.Add(tbb);
                        }

                        // rcc horizontal axis
                        if (chartData.X_axis_title is string && chartData.X_axis_title != null)
                        {
                            var ha = new DateTimeContinuousAxis()
                            {
                                VerticalAlignment = VerticalAlignment.Bottom,
                                HorizontalAlignment = HorizontalAlignment.Center,
                                VerticalLocation = AxisVerticalLocation.Top,
                                LabelFormat = "{}{0,0:HH}",
                                LabelStyle = StaticStyleResource("dateTimeAxisLabelStyle")
                            };
                            if (chartData.x_axis_min != null)
                            {
                                if (chartData.x_axis_min is Binding)
                                {
                                    ha.SetBinding(DateTimeContinuousAxis.MinimumProperty, chartData.x_axis_min as Binding);
                                }
                                else if (chartData.x_axis_min is double)
                                {
                                    ha.Minimum = DateTime.Now.FromUnixTimeSeconds((double)chartData.x_axis_min);
                                }
                            }
                            if (chartData.x_axis_max != null)
                            {
                                if (chartData.x_axis_max is Binding)
                                {
                                    ha.SetBinding(DateTimeContinuousAxis.MaximumProperty, chartData.x_axis_max as Binding);
                                }
                                else if (chartData.x_axis_max is double)
                                {
                                    ha.Maximum = DateTime.Now.FromUnixTimeSeconds((double)chartData.x_axis_max);
                                }
                            }
                            ha.Background = cdb.Result;
                            ha.Foreground = cdb.Result;
                            ha.BorderBrush = cdb.Result;
                            ha.BorderThickness = new Thickness(1);
                            if (string.IsNullOrEmpty(chartData.X_axis_title as string))
                            {
                                ha.LabelFormatter = new LinearAxisLabelFormatter_Customized() { FormatAdditive = "" };
                            }
                            else
                            {
                                ha.LabelFormatter = new LinearAxisLabelFormatter_Customized() { FormatAdditive = " " + (chartData.X_axis_title as string) };
                            }
                            rcc.HorizontalAxis = ha;
                        }
                        else
                        {
                            var ha = new DateTimeContinuousAxis()
                            {
                                VerticalAlignment = VerticalAlignment.Bottom,
                                HorizontalAlignment = HorizontalAlignment.Center,
                                VerticalLocation = AxisVerticalLocation.Top,
                                LabelFormat = "{}{0,0:HH}",
                                LabelStyle = StaticStyleResource("dateTimeAxisLabelStyle")
                            };
                            if (chartData.x_axis_min != null)
                            {
                                if (chartData.x_axis_min is Binding)
                                {
                                    ha.SetBinding(DateTimeContinuousAxis.MinimumProperty, chartData.x_axis_min as Binding);
                                }
                                else if (chartData.x_axis_min is double)
                                {
                                    ha.Minimum = (DateTime)chartData.x_axis_min;
                                }
                            }
                            if (chartData.x_axis_max != null)
                            {
                                if (chartData.x_axis_max is Binding)
                                {
                                    ha.SetBinding(DateTimeContinuousAxis.MaximumProperty, chartData.x_axis_max as Binding);
                                }
                                else if (chartData.x_axis_max is double)
                                {
                                    ha.Maximum = (DateTime)chartData.x_axis_max;
                                }
                            }
                            ha.Background = cdb.Result;
                            ha.Foreground = cdb.Result;
                            ha.BorderBrush = cdb.Result;
                            ha.BorderThickness = new Thickness(1);

                            ha.LabelFormatter = chartData.x_axis_formatter;
                            rcc.HorizontalAxis = ha;
                        }

                        // rcc vertical axis
                        {
                            /*
                                    <tc:RadCartesianChart.VerticalAxis>
                                        <tc:LinearAxis Minimum="{x:Bind vm.AssetChartMinimum, Mode=OneWay}" Maximum="{x:Bind vm.AssetChartMaximum, Mode=OneWay}">
                                            <tc:LinearAxis.LabelFormatter>
                                                <misc:DirectionalScadaAxisLabelFormatter/>
                                            </tc:LinearAxis.LabelFormatter>
                                        </tc:LinearAxis>
                                    </tc:RadCartesianChart.VerticalAxis>
                             */

                            var va = new LinearAxis();
                            {
                                if (chartData.y_axis_min != null)
                                {
                                    if (chartData.y_axis_min is Binding)
                                    {
                                        va.SetBinding(LinearAxis.MinimumProperty, chartData.y_axis_min as Binding);
                                    }
                                    else if (chartData.y_axis_min is double)
                                    {
                                        va.Minimum = (double)chartData.y_axis_min;
                                    }
                                }
                                if (chartData.y_axis_max != null)
                                {
                                    if (chartData.y_axis_max is Binding)
                                    {
                                        va.SetBinding(LinearAxis.MaximumProperty, chartData.y_axis_max as Binding);
                                    }
                                    else if (chartData.y_axis_max is double)
                                    {
                                        va.Maximum = (double)chartData.y_axis_max;
                                    }
                                }
                                if (chartData.y_axis_majorStep != null)
                                {
                                    if (chartData.y_axis_majorStep is Binding)
                                    {
                                        va.SetBinding(LinearAxis.MajorStepProperty, chartData.y_axis_majorStep as Binding);
                                    }
                                    else if (chartData.y_axis_majorStep is double)
                                    {
                                        va.MajorStep = (double)chartData.y_axis_majorStep;
                                    }
                                }
                                va.Background = cdb.Result;
                                va.Foreground = cdb.Result;
                                va.BorderBrush = cdb.Result;
                                va.BorderThickness = new Thickness(1);
                            }
                            va.LabelFormatter = chartData.y_axis_formatter;
                            rcc.VerticalAxis = va;
                        }

                        // rcc Grid
                        {
                            /*
                                    <tc:RadCartesianChart.Grid>
                                        <tc:CartesianChartGrid StripLinesVisibility="Y"/>
                                    </tc:RadCartesianChart.Grid>                     
                             */
                            rcc.Grid = new CartesianChartGrid() { StripLinesVisibility = GridLineVisibility.Y, Background = cpbg.Result, BorderBrush = cdb.Result, BorderThickness = new Thickness(0, 5, 0, 5) };

                        }

                        // rcc data
                        foreach (var chart in chartData.charts)
                        {
                            /*
                                <tc:SplineAreaSeries ItemsSource="{x:Bind vm.AssetChartData, Mode=OneWay}" 
                                                     Stroke="{ThemeResource cweeStorage}"
                                                     Fill="{ThemeResource cweeStorage}"
                                                     tc:ChartTrackBallBehavior.TrackInfoTemplate="{StaticResource trackInfoTemplate}"                                     
                                                     tc:ChartTrackBallBehavior.IntersectionTemplate="{StaticResource trackBallTemplate}" 
                                                     >
                                    <tc:SplineAreaSeries.CategoryBinding>
                                        <tc:PropertyNameDataPointBinding PropertyName="Date"/>
                                    </tc:SplineAreaSeries.CategoryBinding>
                                    <tc:SplineAreaSeries.ValueBinding>
                                        <tc:PropertyNameDataPointBinding PropertyName="Value"/>
                                    </tc:SplineAreaSeries.ValueBinding>
                                </tc:SplineAreaSeries>                         
                             */
                            var spline = new SplineAreaSeries();
                            {
                                if (chart.fillColor != null)
                                {
                                    spline.Fill = chart.fillColor;
                                }
                                if (chart.strokeColor != null)
                                {
                                    spline.Stroke = chart.strokeColor;
                                }
                                if (chart.strokeThickness != null)
                                {
                                    spline.StrokeThickness = chart.strokeThickness.Value;
                                }
                                if (chart.spline_data != null)
                                {
                                    if (chart.spline_data is Binding)
                                    {
                                        spline.SetBinding(SplineAreaSeries.ItemsSourceProperty, chart.spline_data as Binding);
                                    }
                                    else if (chart.spline_data is IEnumerable<ChartItem>)
                                    {
                                        spline.ItemsSource = (chart.spline_data as IEnumerable<ChartItem>);
                                    }
                                    spline.CategoryBinding = new PropertyNameDataPointBinding("Date");
                                    spline.ValueBinding = new PropertyNameDataPointBinding("Value");
                                }
                                //spline.Loaded += Spline_Loaded;
                            }
                            rcc.Series.Add(spline);
                        }

                        outerGrid.Children.Add(rcc);

                        Grid.SetColumn(rcc, 1);

                        foreach (var x in rcc.Series)
                        {
                            ChartTrackBallBehavior.SetIntersectionTemplate(x, cweeXamlHelper.StaticDataTemplateResource("trackInfoTemplate"));
                            if (chartData.X_axis_title is string && chartData.X_axis_title != null)
                            {
                                ChartTrackBallBehavior.SetTrackInfoTemplate(x, cweeXamlHelper.StaticDataTemplateResource("basic_trackBallTemplate_noBall"));
                            }
                            else
                            {
                                ChartTrackBallBehavior.SetTrackInfoTemplate(x, cweeXamlHelper.StaticDataTemplateResource("adjusted_trackBallTemplate_noBall"));
                            }
                        }
                    }
                    return outerGrid;
                }, true);
            }, true);
        }
    }

#endif

}