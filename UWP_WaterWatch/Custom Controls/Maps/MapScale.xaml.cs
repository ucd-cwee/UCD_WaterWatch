using Microsoft.Toolkit.Uwp.UI.Controls;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using UWP_WaterWatchLibrary;
using Windows.Devices.Geolocation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.Storage.Provider;
using Windows.Storage.Streams;
using Windows.System;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Popups;
using Windows.UI.Text;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Maps;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    public class MapScaleViewModel : ViewModelBase
    {
        private string _Text1 = "";
        private string _Text2 = "";
        private string _Text3 = "";
        private string _Text4 = "";
        private string _Text5 = "";
        private string _Text6 = "";
        public string Text1 { get => _Text1; set { _Text1 = value; OnPropertyChanged("Text1"); } }
        public string Text2 { get => _Text2; set { _Text2 = value; OnPropertyChanged("Text2"); } }
        public string Text3 { get => _Text3; set { _Text3 = value; OnPropertyChanged("Text3"); } }
        public string Text4 { get => _Text4; set { _Text4 = value; OnPropertyChanged("Text4"); } }
        public string Text5 { get => _Text5; set { _Text5 = value; OnPropertyChanged("Text5"); } }
        public string Text6 { get => _Text6; set { _Text6 = value; OnPropertyChanged("Text6"); } }

        public double ScaleWidth_Pixels { get; set; } = 0;

        private double _Meters_per_Pixel = 0;
        public double Meters_per_Pixel
        {
            get => _Meters_per_Pixel;
            set
            {
                _Meters_per_Pixel = value;
                double width_meters = ScaleWidth_Pixels * value;
                double scale = 1;
                while ((width_meters / 2.0) >= 10)
                {
                    width_meters /= 10.0;
                    scale *= 10.0;
                }
                while ((width_meters / 2.0) < 1)
                {
                    width_meters *= 10.0;
                    scale /= 10.0;
                }

#if false
                string abbrev = "km";
                var width_kilometers = width_meters / 1000.0;

                Text1 = (1.0 * width_kilometers / 10.0).ToString("0.###");
                Text2 = (2.0 * width_kilometers / 10.0).ToString("0.###");
                Text3 = (3.0 * width_kilometers / 10.0).ToString("0.###");
                Text4 = (4.0 * width_kilometers / 10.0).ToString("0.###");
                Text5 = (5.0 * width_kilometers / 10.0).ToString("0.###") + abbrev;
                Text6 = (width_kilometers).ToString("0.###") + abbrev;
#else
                string abbrev = "m";
                {
                    Dictionary<int, string> map = new Dictionary<int, string>() {
                    { 30, "Qm" }
                    , { 27, "Rm" }
                    , { 24, "Ym" }
                    , { 21, "Zm" }
                    , { 18, "Em" }
                    , { 15, "Pm" }
                    , { 12, "Tm" }
                    , { 9, "Gm" }
                    , { 6, "Mm" }
                    , { 3, "km" }
                    // , { 2, "hm" }
                    // , { 1, "dam" }
                    , { 0, "m" }
                    // , { -1, "dm" }
                    // , { -2, "cm" }
                    , { -3, "mm" }
                    , { -6, "μm" }
                    , { -9, "nm" }
                    , { -12, "pm" }
                    , { -15, "fm" }
                    , { -18, "am" }
                    , { -21, "zm" }
                    , { -24, "ym" }
                    , { -27, "rm" }
                    , { -30, "qm" }
                };
                    foreach (var option in map)
                    {
                        if (scale >= Math.Pow(10, option.Key))
                        {
                            while (scale > Math.Pow(10, option.Key))
                            {
                                width_meters *= 10.0;
                                scale /= 10.0;
                            }
                            abbrev = option.Value;
                            break;
                        }
                    }
                }

                Text1 = Math.Round((1.0 * width_meters / 10.0)).ToString("0.###"); //  (1.0 * width_meters / 10.0).ToString("0.###");
                Text2 = Math.Round((2.0 * width_meters / 10.0)).ToString("0.###"); // (2.0 * width_meters / 10.0).ToString("0.###");
                Text3 = Math.Round((3.0 * width_meters / 10.0)).ToString("0.###"); // (3.0 * width_meters / 10.0).ToString("0.###");
                Text4 = Math.Round((4.0 * width_meters / 10.0)).ToString("0.###"); // (4.0 * width_meters / 10.0).ToString("0.###");
                Text5 = Math.Round((5.0 * width_meters / 10.0)).ToString("0.###") + abbrev; // (5.0 * width_meters / 10.0).ToString("0.###") + abbrev;
                Text6 = Math.Round(width_meters).ToString("0.###") + abbrev;
#endif
            }
        }
    }

    public sealed partial class MapScale : UserControl
    {
        public static readonly DependencyProperty ParentViewModelProperty =
            DependencyProperty.Register(
                "ParentViewModel",
                typeof(SimpleMapVM),
                typeof(MapScale),
                new PropertyMetadata(null)
            );
        public SimpleMapVM ParentViewModel
        {
            get { return (SimpleMapVM)GetValue(ParentViewModelProperty); }
            set { SetValue(ParentViewModelProperty, value); }
        }
        public cweeTimer timer;
        public MapScaleViewModel vm;


        public MapScale()
        {
            this.vm = new MapScaleViewModel();
            this.Opacity = 0.001;
            this.Loaded += MapScale_Loaded;
            this.Unloaded += MapScale_Unloaded;
            this.SizeChanged += MapScale_SizeChanged;

            this.DataContext = vm;
            this.InitializeComponent();
        }

        private static void MapScale_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            (sender as MapScale).vm.ScaleWidth_Pixels = e.NewSize.Width;
        }

        private static void MapScale_Loaded(object sender, RoutedEventArgs e)
        {
            MapScale obj = sender as MapScale;
            SimpleMapVM vm = obj.ParentViewModel;
            var obj_vm = obj.vm;
            AtomicInt localLock = new AtomicInt();
            obj.timer = new cweeTimer(2, () => {
                if (localLock.TryIncrementTo(1))
                {
                    EdmsTasks.InsertJob(() => {
                        try
                        {
                            if (vm.map.map.Pitch <= 1 && vm.map.map.TryGetVisibleBounds(out GeoboundingBox bb))
                            {
                                var height_pixels = vm.map.ActualHeight;
                                var width_pixels = vm.map.ActualWidth;
                                var left_longitude = bb.NorthwestCorner.Longitude;
                                var right_longitude = bb.SoutheastCorner.Longitude;
                                var top_latitude = bb.NorthwestCorner.Latitude;
                                var bottom_latitude = bb.SoutheastCorner.Latitude;
                                EdmsTasks.InsertJob(() =>
                                {
                                    var diagonalDistance_pixels = Math.Sqrt(height_pixels * height_pixels + width_pixels * width_pixels);
                                    if (double.TryParse(WaterWatch.DoScriptImmediately($"Distance({left_longitude}, {top_latitude}, {right_longitude}, {bottom_latitude}).meter.double"), out double diagonalDistance_meters))
                                    {
                                        var Meters_per_Pixel = diagonalDistance_meters / diagonalDistance_pixels;
                                        EdmsTasks.InsertJob(() => {
                                            if (obj_vm.Meters_per_Pixel != Meters_per_Pixel) obj_vm.Meters_per_Pixel = Meters_per_Pixel;
                                            if (obj.Opacity != 1.0) obj.Opacity = 1.0;
                                        }, true).ContinueWith(() => {
                                            localLock.Decrement();
                                        }, false);
                                    }
                                    else
                                    {
                                        localLock.Decrement();
                                    }
                                }, false);
                            }
                            else
                            {
                                if (obj.Opacity == 1.0) obj.Opacity = 0.001;
                                localLock.Decrement();
                            }
                        }
                        catch (Exception) {
                            localLock.Decrement();
                        }
                    }, true);
                }
            }, false);
        }

        private static void MapScale_Unloaded(object sender, RoutedEventArgs e)
        {
            MapScale obj = sender as MapScale;
            SimpleMapVM vm = obj.ParentViewModel;
            obj.timer = null;
        }
    }
}
