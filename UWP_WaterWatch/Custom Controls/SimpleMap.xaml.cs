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

namespace UWP_WaterWatch.Custom_Controls
{
    public class cweeMap : Grid
    {
        public static class MapTools {
            public static MapStyleSheet GetDefaultStyleSheet()
            {
                string jsonText;
                {
                    jsonText =
                    "{" +
                    "  \"version\":\"1.*\"," +
                    "  \"settings\":" +
                    "  {" +
                    "    \"atmosphereVisible\": false," +
                    "    \"fogColor\": \"#FFFFFFFF\"," +
                    "    \"imageFamily\": \"Palette\"," +
                    "    \"landColor\": { \"value\":\"#FF000000\", \"ignoreTransform\":true }," +
                    "    \"logosVisible\": true," +
                    "    \"officialColorVisible\": true," +
                    "    \"shadedReliefVisible\": false," +
                    "    \"spaceColor\": \"#FFFFFFFF\"," +
                    "    \"useDefaultImageColors\": false" +
                    "  }," +
                    "  \"elements\":" +
                    "  {" +
                    "    \"mapElement\":" +
                    "    {" +
                    "      \"fontFamily\": \"SegoeBing\"," +
                    "      \"labelVisible\": true," +
                    "      \"visible\": true" +
                    "    }," +
                    "    \"area\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00D8D0C1\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": \"#FF666666\"," +
                    "      \"labelOutlineColor\": \"#66000000\"," +
                    "      \"labelVisible\": true" +
                    "    }," +
                    "    \"airport\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00E5E4E2\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00806C62\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00F7F4F2\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"areaOfInterest\":" +
                    "    {" +
                    "      \"labelVisible\": false," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"cemetery\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00D7E2DA\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00666666\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00FFFFFF\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"education\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00E7ECCB\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00806C62\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00F7F4F2\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"indigenousPeoplesReserve\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00EBE6DF\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#007D705D\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00FFFFFF\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"medical\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00F0E4DD\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00806C62\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00F7F4F2\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"military\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00ECECEC\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00737373\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00F7F4F2\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"nautical\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00F8F4F2\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00806C62\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00F7F4F2\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"neighborhood\":" +
                    "    {" +
                    "      \"labelColor\": { \"value\":\"#FF666666\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#66000000\", \"ignoreTransform\":true }," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"runway\":" +
                    "    {" +
                    "      \"fillColor\": \"#00D3C7C1\"," +
                    "      \"labelColor\": \"#FF806C62\"," +
                    "      \"labelOutlineColor\": \"#FFF7F4F2\"" +
                    "    }," +
                    "    \"sand\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00F7EBDA\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00756A56\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00FFFFFF\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"shoppingCenter\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00F8F4F2\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00806C62\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00F7F4F2\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"underground\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00E05A5D\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"vegetation\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00D5E2C2\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00486E46\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00FFFFFF\", \"ignoreTransform\":true }," +
                    "      \"labelVisible\": false," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"golfCourse\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00C3D9B8\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#003F633E\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00F7F4F2\", \"ignoreTransform\":true }," +
                    "      \"labelVisible\": false," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"park\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00D1E2B9\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00486E46\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00FFFFFF\", \"ignoreTransform\":true }," +
                    "      \"labelVisible\": false," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"reserve\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00D5E2C2\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#00486E46\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00FFFFFF\", \"ignoreTransform\":true }," +
                    "      \"labelVisible\": false," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"frozenWater\":" +
                    "    {" +
                    "      \"labelVisible\": false," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"point\":" +
                    "    {" +
                    "      \"fillColor\": \"#FFFFFFFF\"," +
                    "      \"scale\": 0," +
                    "      \"strokeColor\": \"#FFFFFFFF\"" +
                    "    }," +
                    "    \"peak\":" +
                    "    {" +
                    "      \"iconColor\": \"#FF737373\"," +
                    "      \"labelColor\": \"#FF5E5E5E\"," +
                    "      \"labelVisible\": false," +
                    "      \"scale\": 1," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"volcanicPeak\":" +
                    "    {" +
                    "      \"iconColor\": \"#FFFE5555\"," +
                    "      \"labelColor\": \"#FF804D4D\"" +
                    "    }," +
                    "    \"waterPoint\":" +
                    "    {" +
                    "      \"iconColor\": \"#FF4C6CBA\"," +
                    "      \"labelColor\": \"#FF4C6CBA\"," +
                    "      \"labelVisible\": false," +
                    "      \"scale\": 1," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"pointOfInterest\":" +
                    "    {" +
                    "      \"iconColor\": \"#FF9C9C9C\"," +
                    "      \"labelColor\": \"#FF787878\"," +
                    "      \"labelVisible\": false," +
                    "      \"scale\": 1," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"business\":" +
                    "    {" +
                    "      \"shadowVisible\": true," +
                    "      \"shape\": { \"background\": \"Circle\" }" +
                    "    }," +
                    "    \"populatedPlace\":" +
                    "    {" +
                    "      \"fillColor\": \"#FF000000\"," +
                    "      \"iconColor\": \"#FF666666\"," +
                    "      \"labelColor\": { \"value\":\"#FF666666\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#66000000\", \"ignoreTransform\":true }," +
                    "      \"scale\": 1," +
                    "      \"strokeColor\": \"#FF323232\"" +
                    "    }," +
                    "    \"capital\":" +
                    "    {" +
                    "      \"scale\": 1.5" +
                    "    }," +
                    "    \"adminDistrictCapital\":" +
                    "    {" +
                    "      \"scale\": 1," +
                    "      \"strokeColor\": \"#FF1B1B1B\"" +
                    "    }," +
                    "    \"countryRegionCapital\":" +
                    "    {" +
                    "      \"iconColor\": \"#FFA6A6A6\"," +
                    "      \"scale\": 1," +
                    "      \"strokeColor\": \"#FF1B1B1B\"" +
                    "    }," +
                    "    \"roadExit\":" +
                    "    {" +
                    "      \"fillColor\": \"#FFE7D8EE\"," +
                    "      \"iconColor\": \"#FF262626\"," +
                    "      \"labelColor\": \"#FF464646\"," +
                    "      \"labelOutlineColor\": \"#FFF7F4F2\"," +
                    "      \"labelVisible\": false," +
                    "      \"scale\": 1," +
                    "      \"strokeColor\": \"#FFA77D91\"," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"roadShield\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#FF333333\", \"ignoreTransform\":true }," +
                    "      \"iconColor\": \"#FFA6A6A6\"," +
                    "      \"labelColor\": \"#FF777777\"," +
                    "      \"labelOutlineColor\": \"#66000000\"," +
                    "      \"labelVisible\": false," +
                    "      \"scale\": 1," +
                    "      \"strokeColor\": \"#FF666666\"," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"transit\":" +
                    "    {" +
                    "      \"fillColor\": \"#FF333333\"," +
                    "      \"iconColor\": \"#FFDCDCDC\"," +
                    "      \"labelColor\": { \"value\":\"#FF8C8C8C\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00FFFFFF\", \"ignoreTransform\":true }," +
                    "      \"labelVisible\": false," +
                    "      \"scale\": 1," +
                    "      \"strokeColor\": \"#66FAF9F7\"," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"political\":" +
                    "    {" +
                    "      \"borderOutlineColor\": \"#FFFFFFFF\"," +
                    "      \"borderStrokeColor\": \"#FFFFFFFF\"," +
                    "      \"borderVisible\": true," +
                    "      \"borderWidthScale\": 0," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"adminDistrict\":" +
                    "    {" +
                    "      \"borderOutlineColor\": { \"value\":\"#07FFFFFF\", \"ignoreTransform\":true }," +
                    "      \"borderStrokeColor\": { \"value\":\"#FF333333\", \"ignoreTransform\":true }," +
                    "      \"borderWidthScale\": 1," +
                    "      \"labelColor\": { \"value\":\"#FF666666\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#66000000\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"countryRegion\":" +
                    "    {" +
                    "      \"borderOutlineColor\": { \"value\":\"#05FFFFFF\", \"ignoreTransform\":true }," +
                    "      \"borderStrokeColor\": { \"value\":\"#FF2C2C2C\", \"ignoreTransform\":true }," +
                    "      \"borderWidthScale\": 1," +
                    "      \"labelColor\": { \"value\":\"#FF777777\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#66000000\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"district\":" +
                    "    {" +
                    "      \"borderStrokeColor\": { \"value\":\"#00979799\", \"ignoreTransform\":true }," +
                    "      \"borderWidthScale\": 1," +
                    "      \"labelColor\": \"#FF666666\"," +
                    "      \"labelOutlineColor\": \"#66000000\"," +
                    "      \"labelVisible\": false" +
                    "    }," +
                    "    \"structure\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#00F8F4F2\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": \"#FF666666\"," +
                    "      \"labelOutlineColor\": \"#66000000\"," +
                    "      \"labelVisible\": false," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"building\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#FF1B1B1B\", \"ignoreTransform\":true }," +
                    "      \"visible\": false" +
                    "    }," +
                    "    \"transitBuilding\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#FF262626\", \"ignoreTransform\":true }," +
                    "      \"visible\": true" +
                    "    }," +
                    "    \"transportation\":" +
                    "    {" +
                    "      \"fillColor\": \"#FFFFFFFF\"," +
                    "      \"overwriteColor\": false," +
                    "      \"strokeWidthScale\": 0" +
                    "    }," +
                    "    \"railway\":" +
                    "    {" +
                    "      \"fillColor\": \"#FF000000\"," +
                    "      \"labelColor\": { \"value\":\"#FF505050\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#00EAE7E0\", \"ignoreTransform\":true }," +
                    "      \"strokeColor\": { \"value\":\"#FF282828\", \"ignoreTransform\":true }," +
                    "      \"strokeWidthScale\": 2" +
                    "    }," +
                    "    \"road\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#FF262626\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#FF444444\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#66000000\", \"ignoreTransform\":true }," +
                    "      \"strokeColor\": { \"value\":\"#FF000000\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"arterialRoad\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 0.5" +
                    "    }," +
                    "    \"controlledAccessHighway\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 0.8" +
                    "    }," +
                    "    \"highway\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 0.8" +
                    "    }," +
                    "    \"majorRoad\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 0.8" +
                    "    }," +
                    "    \"street\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 0.5" +
                    "    }," +
                    "    \"unpavedStreet\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 0.5" +
                    "    }," +
                    "    \"tollRoad\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 0.5" +
                    "    }," +
                    "    \"trail\":" +
                    "    {" +
                    "      \"labelColor\": \"#FF3F3F3F\"," +
                    "      \"labelOutlineColor\": \"#66000000\"," +
                    "      \"strokeColor\": \"#FF262626\"," +
                    "      \"strokeWidthScale\": 1" +
                    "    }," +
                    "    \"walkway\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#FF262626\", \"ignoreTransform\":true }" +
                    "    }," +
                    "    \"waterRoute\":" +
                    "    {" +
                    "      \"labelColor\": \"#FF666666\"," +
                    "      \"labelOutlineColor\": \"#66000000\"," +
                    "      \"labelVisible\": true," +
                    "      \"strokeColor\": \"#FF666666\"," +
                    "      \"strokeWidthScale\": 1" +
                    "    }," +
                    "    \"water\":" +
                    "    {" +
                    "      \"fillColor\": { \"value\":\"#FF333333\", \"ignoreTransform\":true }," +
                    "      \"labelColor\": { \"value\":\"#FF666666\", \"ignoreTransform\":true }," +
                    "      \"labelOutlineColor\": { \"value\":\"#66000000\", \"ignoreTransform\":true }," +
                    "      \"labelVisible\": true" +
                    "    }," +
                    "    \"river\":" +
                    "    {" +
                    "      \"strokeWidthScale\": 2" +
                    "    }," +
                    "    \"userMapElement\":" +
                    "    {" +
                    "      \"fillColor\": \"#FFFAF9F7\"," +
                    "      \"strokeColor\": \"#FFFF6000\"" +
                    "    }," +
                    "    \"userArea\":" +
                    "    {" +
                    "      \"fillColor\": \"#FF0000FF\"," +
                    "      \"strokeColor\": \"#FF0000FF\"" +
                    "    }," +
                    "    \"userBillboard\":" +
                    "    {" +
                    "      \"stemColor\": \"#FF888888\"," +
                    "      \"stemOutlineColor\": \"#66FFFFFF\"" +
                    "    }," +
                    "    \"userLine\":" +
                    "    {" +
                    "      \"fillColor\": \"#FF0000FF\"," +
                    "      \"strokeColor\": \"#FF0000FF\"" +
                    "    }," +
                    "    \"userPoint\":" +
                    "    {" +
                    "      \"stemColor\": \"#FF666666\"," +
                    "      \"stemOutlineColor\": \"#66FFFFFF\"," +
                    "      \"fillColor\": \"#FFF7630C\"," +
                    "      \"iconColor\": \"#FFFFFFFF\"," +
                    "      \"labelColor\": \"#FF444444\"," +
                    "      \"labelOutlineColor\": \"#FFFFFFFF\"," +
                    "      \"strokeColor\": \"#FFFFFFFF\"" +
                    "    }" +
                    "  }" +
                    "}";
                }
                return MapStyleSheet.Combine(new List<MapStyleSheet>
                    {
                        MapStyleSheet.ParseFromJson("{ \"version\": \"1.*\", \"settings\": { }, \"elements\": { \"userPoint\": { \"stemAnchorRadiusScale\": 0, \"stemHeightScale\": 0 } } }"),
                        MapStyleSheet.ParseFromJson(jsonText)
                    });
            }

            public static MapStyleSheet GetStyleSheet(int mapStyle)
            {
                MapStyleSheet toReturn;
                switch (mapStyle)
                {
                    case 0:
                        toReturn = MapStyleSheet.Aerial(); break;
                    case 1:
                        toReturn = MapStyleSheet.AerialWithOverlay(); break;
                    case 2:
                        toReturn = MapStyleSheet.RoadLight(); break;
                    case 3:
                        toReturn = MapStyleSheet.RoadDark(); break;
                    case 4:
                        //toReturn = MapStyleSheet.RoadLight(); break;
                    case 5:
                        //toReturn = MapStyleSheet.RoadDark(); break;
                    case 6:
                        toReturn = GetDefaultStyleSheet(); break;
                    case 7:
                        toReturn = MapStyleSheet.ParseFromJson("{ \"version\": \"1.*\", \"settings\": { }, \"elements\": { \"userPoint\": { \"stemAnchorRadiusScale\": 0, \"stemHeightScale\": 0 } } }");
                        break;
                    default:
                        toReturn = MapStyleSheet.AerialWithOverlay(); break;
                }

                MapStyleSheet custom = MapStyleSheet.ParseFromJson(
                    "{ \"version\": \"1.*\", \"settings\": { }, \"elements\": { \"userPoint\": { \"stemAnchorRadiusScale\": 0, \"stemHeightScale\": 0 } } }"
                );

                return MapStyleSheet.Combine(new List<MapStyleSheet>() { toReturn, custom });
            }




        }

        public MapControl map { get; set; }
        private ObservableCollection<MapElement> DynamicLayer { get; set; }

        private ObservableCollection<MapElement> PolylineLayer_0 { get; set; } // optimization for Polylines that should only be seen when heavily zoomed in
        private ObservableCollection<MapElement> PolylineLayer_1 { get; set; } // optimization for Polylines that should only be seen when heavily zoomed in
        private ObservableCollection<MapElement> PolylineLayer_2 { get; set; } // optimization for Polylines that should only be seen when heavily zoomed in
        private ObservableCollection<MapElement> PolylineLayer_3 { get; set; } // optimization for Polylines that should only be seen when heavily zoomed in
        private ObservableCollection<MapElement> PolylineLayer_4 { get; set; } // optimization for Polylines that should only be seen when heavily zoomed in
        private ObservableCollection<MapElement> PolylineLayer_5 { get; set; } // optimization for Polylines that should only be seen when heavily zoomed in


        public cweeMap()
        {
            this.HorizontalAlignment = HorizontalAlignment.Stretch;
            this.VerticalAlignment = VerticalAlignment.Stretch;

            var newMap = new MapControl();
            {
                newMap.Margin = new Thickness(0, 0, 0, 0);
                newMap.Padding = new Thickness(0, 0, 0, 0);
                newMap.MapServiceToken = "pwnqy6Z8qh4QUbDZgS2g~78ks7kQdYRvPl2RF-zA2eg~AjF-TNNVnDBR4FfaLMfMGehMdn2lnCrT94JcIjbNLF2Ys0lx33TsQEp69Zey52uB";
                newMap.RotateInteractionMode = MapInteractionMode.Auto;
                newMap.ZoomInteractionMode = MapInteractionMode.Auto;
                newMap.Style = MapStyle.None;
                newMap.StyleSheet = MapTools.GetStyleSheet(0);
            }
            this.map = newMap;

            this.PolylineLayer_0 = new ObservableCollection<MapElement>();
            this.PolylineLayer_1 = new ObservableCollection<MapElement>();
            this.PolylineLayer_2 = new ObservableCollection<MapElement>();
            this.PolylineLayer_3 = new ObservableCollection<MapElement>();
            this.PolylineLayer_4 = new ObservableCollection<MapElement>();
            this.DynamicLayer = new ObservableCollection<MapElement>();

            lock (this.map.Layers)
            {
                this.map.Layers = new ObservableCollection<MapLayer>() {
                    new MapElementsLayer() { MapElements = this.PolylineLayer_0 },
                    new MapElementsLayer() { MapElements = this.PolylineLayer_1 },
                    new MapElementsLayer() { MapElements = this.PolylineLayer_2 },
                    new MapElementsLayer() { MapElements = this.PolylineLayer_3 },
                    new MapElementsLayer() { MapElements = this.PolylineLayer_4 },
                    new MapElementsLayer() { MapElements = this.DynamicLayer }
                };
            }

            this.Name = "cweeMap_" + WaterWatch.RandomInt(1000, 100000).ToString();

            this.Children.Add(map);
            this.map.IsRightTapEnabled = true;
            this.map.MapRightTapped += Map_MapRightTapped;
            this.map.ZoomLevelChanged += Map_ZoomLevelChanged;            
        }
        ~cweeMap() {
            map = null;
            DynamicLayer = null;
            PolylineLayer_0 = null;
            PolylineLayer_1 = null;
            PolylineLayer_2 = null;
            PolylineLayer_3 = null;
            PolylineLayer_4 = null;
            PolylineLayer_5 = null;
        }


        private void Map_ZoomLevelChanged(MapControl sender, object args)
        {
            Dictionary<int, double> zoomLevel = new Dictionary<int, double>();
            zoomLevel[0] = 18.0;
            zoomLevel[1] = 17.0;
            zoomLevel[2] = 16.0;
            zoomLevel[3] = 14.0;
            zoomLevel[4] = 12.0;
            zoomLevel[5] = 10.0;

            foreach (var zoomPair in zoomLevel)
            {
                if ((sender.ZoomLevel >= zoomPair.Value) != this.map.Layers[zoomPair.Key].Visible)
                {
                    if (sender.ZoomLevel >= zoomPair.Value)
                    {
                        this.map.Layers[zoomPair.Key].Visible = !this.map.Layers[zoomPair.Key].Visible;
                    }
                    if (sender.ZoomLevel <= (zoomPair.Value - 0.05))
                    {
                        this.map.Layers[zoomPair.Key].Visible = !this.map.Layers[zoomPair.Key].Visible;
                    }
                }
            }
        }

        public void Map_CenterView(MapAnimationKind animationKind = MapAnimationKind.Default) {
            var centerPoint = new Windows.Devices.Geolocation.BasicGeoposition();
            double minX = double.MaxValue, maxX = -double.MaxValue, minY = double.MaxValue, maxY = -double.MaxValue, numSamples = 0;
            foreach (MapElementsLayer layer in map.Layers)
            {
                if (layer != null)
                {
                    foreach (MapElement child in layer.MapElements)
                    {
                        if (child is MapIcon)
                        {
                            numSamples++;

                            centerPoint.Longitude -= centerPoint.Longitude / numSamples;
                            centerPoint.Longitude += (child as MapIcon).Location.Position.Longitude / numSamples;

                            centerPoint.Latitude -= centerPoint.Latitude / numSamples;
                            centerPoint.Latitude += (child as MapIcon).Location.Position.Latitude / numSamples;

                            minX = Math.Min(minX, (child as MapIcon).Location.Position.Longitude);
                            minY = Math.Min(minY, (child as MapIcon).Location.Position.Latitude);
                            maxX = Math.Max(maxX, (child as MapIcon).Location.Position.Longitude);
                            maxY = Math.Max(maxY, (child as MapIcon).Location.Position.Latitude);
                        }
                    }
                }
            }

            map.TrySetSceneAsync(
                MapScene.CreateFromBoundingBox(
                    new Windows.Devices.Geolocation.GeoboundingBox(
                        new Windows.Devices.Geolocation.BasicGeoposition() { Longitude = minX, Latitude = maxY },
                        new Windows.Devices.Geolocation.BasicGeoposition() { Longitude = maxX, Latitude = minY }
                    )
                )
                , animationKind
            );
        }
        private void Map_MapRightTapped(MapControl sender, MapRightTappedEventArgs args)
        {
            MapControl mp = (sender as MapControl);
            if (mp != null)
            {
                StackPanel flyout = new StackPanel() { Orientation = Orientation.Vertical, HorizontalAlignment = HorizontalAlignment.Left };
                var point = args.Position;
                var _fly = mp.SetFlyout(flyout, mp, null, point);
                _fly.ShowMode = FlyoutShowMode.Transient;

                {
                    CommandBar panelA = new CommandBar() { HorizontalAlignment = HorizontalAlignment.Left, HorizontalContentAlignment = HorizontalAlignment.Left };
                    {
                        {
                            AppBarButton but = new AppBarButton() { Icon = new SymbolIcon(Symbol.World), HorizontalAlignment = HorizontalAlignment.Left };
                            but.Click += (object sender123, RoutedEventArgs e1234) => {
                                Map_CenterView();
                                _fly.Hide();
                            };
                            panelA.PrimaryCommands.Add(but);
                        }
                        if (args.Location != null)
                        {
                            {
                                AppBarSeparator but = new AppBarSeparator() { };
                                panelA.PrimaryCommands.Add(but);
                            }
                            {
                                AppBarButton but = new AppBarButton() { Icon = new SymbolIcon(Symbol.ZoomIn), HorizontalAlignment = HorizontalAlignment.Left };
                                but.Click += async (object sender123, RoutedEventArgs e1234) =>
                                {
                                    double a = 0.000001d;
                                    double b = 1.0d / (mp.MaxZoomLevel + 1.0d);
                                    double l = ((mp.MaxZoomLevel + 1.0d) - mp.ZoomLevel) / (mp.MaxZoomLevel - 1.0d); l *= l;
                                    double size = b * l + a * (1.0d - l);
                                    await mp.TrySetViewBoundsAsync(
                                        new GeoboundingBox(
                                            new BasicGeoposition()
                                            {
                                                Longitude = args.Location.Position.Longitude - size,
                                                Latitude = Math.Min(90d, args.Location.Position.Latitude + size / 2.0d)
                                            },
                                            new BasicGeoposition()
                                            {
                                                Longitude = args.Location.Position.Longitude + size,
                                                Latitude = Math.Max(-90d, args.Location.Position.Latitude - size / 2.0d)
                                            }
                                        ),
                                        new Thickness() { Bottom = 25d, Left = 25d, Right = 25d, Top = 25d },
                                        MapAnimationKind.Default
                                    );
                                };
                                panelA.PrimaryCommands.Add(but);
                            }
                        }
                        if (args.Location != null)
                        {
                            {
                                AppBarSeparator but = new AppBarSeparator() { };
                                panelA.PrimaryCommands.Add(but);
                            }
                            {
                                AppBarButton but = new AppBarButton() { Icon = new SymbolIcon(Symbol.ZoomOut), HorizontalAlignment = HorizontalAlignment.Left };
                                but.Click += async (object sender123, RoutedEventArgs e1234) =>
                                {
                                    double a = 0.000001d;
                                    double b = 1.0d / (mp.MaxZoomLevel + 1.0d);
                                    double l = ((mp.MaxZoomLevel + 1.0d) - mp.ZoomLevel) / (mp.MaxZoomLevel - 1.0d); l -= l * 0.9;
                                    double size = b * l + a * (1.0d - l);
                                    await mp.TrySetViewBoundsAsync(
                                        new GeoboundingBox(
                                            new BasicGeoposition()
                                            {
                                                Longitude = args.Location.Position.Longitude - size,
                                                Latitude = Math.Min(90d, args.Location.Position.Latitude + size / 2.0d)
                                            },
                                            new BasicGeoposition()
                                            {
                                                Longitude = args.Location.Position.Longitude + size,
                                                Latitude = Math.Max(-90d, args.Location.Position.Latitude - size / 2.0d)
                                            }
                                        ),
                                        new Thickness() { Bottom = 25d, Left = 25d, Right = 25d, Top = 25d },
                                        MapAnimationKind.Default
                                    );
                                };
                                panelA.PrimaryCommands.Add(but);
                            }
                        }
                        if (args.Location != null && mp.IsStreetsideSupported == true)
                        {
                            {
                                AppBarSeparator but = new AppBarSeparator() { };
                                panelA.PrimaryCommands.Add(but);
                            }
                            {
                                AppBarButton but = new AppBarButton() { Icon = new SymbolIcon(Symbol.Street), HorizontalAlignment = HorizontalAlignment.Left };
                                but.Click += async (object sender123, RoutedEventArgs e1234) =>
                                {
                                    StreetsidePanorama pan = await StreetsidePanorama.FindNearbyAsync(new Geopoint(args.Location.Position));
                                    if (pan != null)
                                    {
                                        // Create the Streetside view.
                                        StreetsideExperience ssView = new StreetsideExperience(pan);
                                        ssView.OverviewMapVisible = false;
                                        mp.CustomExperience = ssView;
                                    }
                                    _fly.Hide();
                                };
                                panelA.PrimaryCommands.Add(but);
                            }
                        }
                    }
                    flyout.Children.Add(panelA);
                }
                {
                    if (args.Location != null)
                    {
                        try
                        {
                            var longitude = args.Location.Position.Longitude;
                            var latitude = args.Location.Position.Latitude;
                            StackPanel PanelB = new StackPanel() { Orientation = Orientation.Vertical };
                            {
                                {
                                    var tb = cweeXamlHelper.SimpleTextBlock($"Longitude: {longitude}");
                                    tb.HorizontalAlignment = HorizontalAlignment.Left;
                                    tb.HorizontalTextAlignment = TextAlignment.Left;
                                    PanelB.Children.Add(tb);
                                }
                                {
                                    var tb = cweeXamlHelper.SimpleTextBlock($"Latitude:  {latitude}");
                                    tb.HorizontalAlignment = HorizontalAlignment.Left;
                                    tb.HorizontalTextAlignment = TextAlignment.Left;
                                    PanelB.Children.Add(tb);
                                }
                                EdmsTasks.InsertJob(() =>
                                {
                                    double elevation = WaterWatch.GeocodeElevation(longitude, latitude);
                                    return EdmsTasks.InsertJob(() =>
                                    {
                                        {
                                            var tb = cweeXamlHelper.SimpleTextBlock($"Elevation: {elevation} ft");
                                            tb.HorizontalAlignment = HorizontalAlignment.Left;
                                            tb.HorizontalTextAlignment = TextAlignment.Left;
                                            PanelB.Children.Add(tb);
                                        }
                                    }, true, true);
                                }, false, true).ContinueWith(() => {
                                    var nearby = WaterWatch.GeocodeAddress(longitude, latitude);
                                    return EdmsTasks.InsertJob(() =>
                                    {
                                        {
                                            var tb = cweeXamlHelper.SimpleTextBlock($"Approximate Address:\n  {nearby}");
                                            tb.HorizontalAlignment = HorizontalAlignment.Left;
                                            tb.HorizontalTextAlignment = TextAlignment.Left;
                                            PanelB.Children.Add(tb);
                                        }
                                    }, true, true);
                                }, false);
                            }
                            flyout.Children.Add(PanelB);
                        }
                        catch (Exception) { }
                    }
                }
            }
        }

        static private double PointInEllipse(float x, float y, float radius_x, float radius_y)
        {
            double C = (x - radius_x) * (x - radius_x) / (radius_x * radius_x);
            double D = (y - radius_y) * (y - radius_y) / (radius_y * radius_y);

            return ((C + D) - 1.0); // less than one means it is in the ellipse. 0 indicates it sits on te region. Larger indicates outside the ellipse.
        }
        static private cweeTask<byte[]> ImageBytesFromColorAsync(Windows.UI.Color c, int w, int h)
        {
            return EdmsTasks.InsertJob(() => {
                float radius_x = ((float)w) / 2.0f;
                float radius_y = ((float)h) / 2.0f;

                double d;
                byte[] imageBytes = new byte[4 * w * h];
                int k = 0;
                for (float i = 0; i < h; i++)
                {
                    for (float j = 0; j < w; j++)
                    {
                        if (imageBytes.Length > (k + 3))
                        {
                            imageBytes[k] = (byte)(c.B);
                            imageBytes[k + 1] = (byte)(c.G);
                            imageBytes[k + 2] = (byte)(c.R);
                            imageBytes[k + 3] = (byte)(c.A);

                            d = PointInEllipse(j + 0.5f, i + 0.5f, radius_x, radius_y); // add 0.5 to achieve the proper pixel center

                            if (d > 0)
                            {
                                imageBytes[k + 3] = 0;
                            }

                            k += 4;
                        }
                    }
                }
                return imageBytes;
            }, false, true);
        }

        static private byte[] ImageBytesFromColor(Windows.UI.Color c, int w, int h)
        {
            //string code = $"{w}_{h}_{c.R}_{c.G}_{c.B}_{c.A}";
            //lock (CachedImageBytes)
            //{
            //    if (CachedImageBytes.ContainsKey(code))
            //    {
            //        return CachedImageBytes[code];
            //    }
            //}

            float radius_x = ((float)w) / 2.0f;
            float radius_y = ((float)h) / 2.0f;

            double d;
            byte[] imageBytes = new byte[4 * w * h];
            int k = 0;
            for (float i = 0; i < h; i++)
            {
                for (float j = 0; j < w; j++)
                {
                    if (imageBytes.Length > (k + 3))
                    {
                        imageBytes[k] = (byte)(c.B);
                        imageBytes[k + 1] = (byte)(c.G);
                        imageBytes[k + 2] = (byte)(c.R);
                        imageBytes[k + 3] = (byte)(c.A);

                        d = PointInEllipse(j + 0.5f, i + 0.5f, radius_x, radius_y); // add 0.5 to achieve the proper pixel center

                        if (d > 0)
                        {
                            imageBytes[k + 3] = 0;
                        }

                        k += 4;
                    }
                }
            }

            //lock (CachedImageBytes)
            //{
            //    CachedImageBytes.Add(code, imageBytes);
            //}

            return imageBytes;
        }
        static private byte[] ImageBytesFromColor(Windows.UI.Color c, float w, float h, Microsoft.Graphics.Canvas.Geometry.CanvasGeometry geometry)
        {
            var bounds = geometry.ComputeBounds();
            double geoWidth = bounds.Width;
            double geoHeight = bounds.Height;

            float radius_x = ((float)w) / 2.0f;
            float radius_y = ((float)h) / 2.0f;

            byte[] imageBytes = new byte[4 * (int)w * (int)h];
            int k = 0;
            for (float i = 0; i < h; i++)
            {
                for (float j = 0; j < w; j++)
                {
                    if (imageBytes.Length > (k + 3))
                    {
                        imageBytes[k] = (byte)(c.B);
                        imageBytes[k + 1] = (byte)(c.G);
                        imageBytes[k + 2] = (byte)(c.R);
                        imageBytes[k + 3] = (byte)(c.A);

                        if (!geometry.FillContainsPoint(new System.Numerics.Vector2(
                            (float)((double)((j+0.5f) / w)* geoWidth)
                            ,
                            (float)((double)((i + 0.5f) / h) * geoHeight)
                        ))) {
                            imageBytes[k + 3] = 0;
                        }

                        k += 4;
                    }
                }
            }

            //lock (CachedImageBytes)
            //{
            //    CachedImageBytes.Add(code, imageBytes);
            //}

            return imageBytes;
        }
        static private cweeTask<RandomAccessStreamReference> ConvertWriteableBitmapToRandomAccessStreamAsync(cweeTask<byte[]> bytes, int w, int h)
        {
            return (EdmsTasks.cweeTask)bytes.ContinueWith(() => {
                byte[] pixels = bytes.Result;
                var streamJob = EdmsTasks.InsertJob(() => {
                    return new InMemoryRandomAccessStream();
                }, true, true);
                return streamJob.ContinueWith(() => {
                    InMemoryRandomAccessStream stream = streamJob.Result;

                    cweeTask<RandomAccessStreamReference> createStream = new EdmsTasks.cweeTask(() => {
                        return Windows.Storage.Streams.RandomAccessStreamReference.CreateFromStream(stream);
                    }, true, true);

                    System.Threading.Tasks.Task<Windows.Graphics.Imaging.BitmapEncoder> encoder_job = Windows.Graphics.Imaging.BitmapEncoder.CreateAsync(Windows.Graphics.Imaging.BitmapEncoder.PngEncoderId, stream).AsTask();
                    encoder_job.ContinueWith((System.Threading.Tasks.Task<Windows.Graphics.Imaging.BitmapEncoder> eJ) => {
                        EdmsTasks.InsertJob(() => {
                            try
                            {
                                Windows.Graphics.Imaging.BitmapEncoder encoder = eJ.Result;
                                encoder.SetPixelData(Windows.Graphics.Imaging.BitmapPixelFormat.Bgra8, Windows.Graphics.Imaging.BitmapAlphaMode.Straight, (uint)w, (uint)h, 96.0, 96.0, pixels);
                                encoder.FlushAsync().AsTask().ContinueWith((System.Threading.Tasks.Task fJ) =>
                                {
                                    EdmsTasks.InsertJob(createStream); // queue / perform the job
                                });
                            }
                            catch (Exception)
                            {
                                EdmsTasks.InsertJob(createStream); // queue / perform the job
                            }
                        }, true, true);
                    });

                    return createStream;
                }, false);
            }, false);
        }

        static private ConcurrentDictionary<string, RandomAccessStreamReference> CachedRandomAccessStreams = new ConcurrentDictionary<string, RandomAccessStreamReference>();
        static private cweeTask<RandomAccessStreamReference> ConvertWriteableBitmapToRandomAccessStream(Windows.UI.Color c, int w, int h) 
        {
            string code = $"{w}_{h}_{c.R}_{c.G}_{c.B}_{c.A}";
            //lock (CachedRandomAccessStreams)
            {
                if (CachedRandomAccessStreams.ContainsKey(code) && CachedRandomAccessStreams.TryGetValue(code, out RandomAccessStreamReference r))
                {
                    return EdmsTasks.cweeTask.CompletedTask(r);
                }
            }

            byte[] pixels = ImageBytesFromColor(c, w, h);

            EdmsTasks.cweeTask createStream = new EdmsTasks.cweeTask(/*() => { return Windows.Storage.Streams.RandomAccessStreamReference.CreateFromStream(stream); }, true, true*/);
            AppExtension.ApplicationInfo.MainWindowAction(async ()=> {
                try {
                    var stream = new InMemoryRandomAccessStream();
                    Windows.Graphics.Imaging.BitmapEncoder encoder = await Windows.Graphics.Imaging.BitmapEncoder.CreateAsync(Windows.Graphics.Imaging.BitmapEncoder.PngEncoderId, stream);
                    encoder.SetPixelData(Windows.Graphics.Imaging.BitmapPixelFormat.Bgra8, Windows.Graphics.Imaging.BitmapAlphaMode.Straight, (uint)w, (uint)h, 96.0, 96.0, pixels);
                    await encoder.FlushAsync();
                    var finalRef = Windows.Storage.Streams.RandomAccessStreamReference.CreateFromStream(stream);
                    //lock (CachedRandomAccessStreams)
                    {
                        CachedRandomAccessStreams.TryAdd(code, finalRef);
                    }
                    createStream.SetFinished(finalRef);
                }
                catch (Exception) {
                    createStream.SetFinished(null);
                }
                finally
                {
                    
                }
            });
            return createStream;
        }

        static private cweeTask<RandomAccessStreamReference> ConvertWriteableBitmapToRandomAccessStream(Windows.UI.Color c, int w, int h, string sData)
        {
            string code = $"{w}_{h}_{c.R}_{c.G}_{c.B}_{c.A}_{sData}";
            {
                if (CachedRandomAccessStreams.ContainsKey(code) && CachedRandomAccessStreams.TryGetValue(code, out RandomAccessStreamReference r))
                {
                    return EdmsTasks.cweeTask.CompletedTask(r);
                }
            }

            Microsoft.Graphics.Canvas.Geometry.CanvasGeometry geometry = Microsoft.Toolkit.Uwp.UI.Media.Geometry.CanvasPathGeometry.CreateGeometry(sData);

            byte[] pixels = ImageBytesFromColor(c, w, h, geometry);

            EdmsTasks.cweeTask createStream = new EdmsTasks.cweeTask(/*() => { return Windows.Storage.Streams.RandomAccessStreamReference.CreateFromStream(stream); }, true, true*/);
            AppExtension.ApplicationInfo.MainWindowAction(async () => {
                try
                {
                    var stream = new InMemoryRandomAccessStream();
                    Windows.Graphics.Imaging.BitmapEncoder encoder = await Windows.Graphics.Imaging.BitmapEncoder.CreateAsync(Windows.Graphics.Imaging.BitmapEncoder.PngEncoderId, stream);
                    encoder.SetPixelData(Windows.Graphics.Imaging.BitmapPixelFormat.Bgra8, Windows.Graphics.Imaging.BitmapAlphaMode.Straight, (uint)w, (uint)h, 96.0, 96.0, pixels);
                    await encoder.FlushAsync();
                    var finalRef = Windows.Storage.Streams.RandomAccessStreamReference.CreateFromStream(stream);
                    //lock (CachedRandomAccessStreams)
                    {
                        CachedRandomAccessStreams.TryAdd(code, finalRef);
                    }
                    createStream.SetFinished(finalRef);
                }
                catch (Exception)
                {
                    createStream.SetFinished(null);
                }
                finally
                {

                }
            });
            return createStream;
        }

        static public cweeTask<RandomAccessStreamReference> GetMapIconImage(Windows.UI.Color c, double _size, string _pathData = null)
        {
            if (string.IsNullOrEmpty(_pathData))
                return ConvertWriteableBitmapToRandomAccessStream(c, (int)_size, (int)_size);
            else
                return ConvertWriteableBitmapToRandomAccessStream(c, (int)_size, (int)_size, _pathData);
        }
        static public cweeTask<MapIcon> CreateMapIcon(double longitude, double latitude, Windows.UI.Color _col, double _size)
        {
            var iconTask = GetMapIconImage(_col, _size);
            return iconTask.ContinueWith(() => {
                try
                {
                    var obj = new MapIcon();
                    {
                        obj.Visible = true;
                        obj.Image = iconTask.Result;
                        obj.CollisionBehaviorDesired = MapElementCollisionBehavior.Hide;
                        obj.Location = new Windows.Devices.Geolocation.Geopoint(new Windows.Devices.Geolocation.BasicGeoposition() { Longitude = longitude, Latitude = latitude });
                    }
                    return obj;
                }
                catch (Exception e)
                {
                    e.EdmsHandle();

                    return null;
                }
            }, true);
        }
        static public cweeTask<MapIcon> CreateMapIcon(string _long, string _lat, Windows.UI.Color _col, double _size)
        {
            var iconTask = GetMapIconImage(_col, _size);
            return iconTask.ContinueWith(() =>
            {
                try
                {
                    var obj = new MapIcon();
                    {
                        obj.Visible = true;
                        obj.Image = iconTask.Result;
                        obj.CollisionBehaviorDesired = MapElementCollisionBehavior.Hide;
                        if (double.TryParse(_long, out double longitude))
                        {
                            if (double.TryParse(_lat, out double latitude))
                            {
                                obj.Location = new Windows.Devices.Geolocation.Geopoint(new Windows.Devices.Geolocation.BasicGeoposition() { Longitude = longitude, Latitude = latitude });
                            }
                        }
                    }
                    return obj;
                }
                catch (Exception e)
                {
                    e.EdmsHandle();

                    return null;
                }
            }, true);
        }
        public cweeTask<MapIcon> AddMapIcon(string _long, string _lat, Windows.UI.Color _col, double _size)
        {
            if (double.TryParse(_long, out double longitude))
            {
                if (double.TryParse(_lat, out double latitude))
                {
                    cweeTask<MapIcon> makeIt = CreateMapIcon(longitude, latitude, _col, _size);
                    return makeIt.ContinueWith(() => {
                        MapIcon icon = makeIt.Result;
                        if (icon != null)
                        {
                            lock (this.DynamicLayer)
                            {
                                lock (this.map)
                                {
                                    this.DynamicLayer.Add(icon);
                                }
                            }
                        }
                        return icon;
                    }, true);
                }
            }
            return EdmsTasks.cweeTask.CompletedTask(null);
        }
        public void AddMapElement(MapElement icon)
        {
            if (icon != null)
            {
                lock (this.DynamicLayer)
                {
                    lock (this.map)
                    {
                        if (!this.DynamicLayer.Contains(icon))
                        {
                            this.DynamicLayer.Add(icon);
                        }
                    }

                }
            }
        }
        public void AddMapLayer(MapElementsLayer layer)
        {
            map.Layers.Add(layer);
        }
        
        static Rect MakeBoundingBox(IReadOnlyList<BasicGeoposition> path)
        {
            try
            {
                var xmin = path.Min(x => x.Longitude);
                var xmax = path.Max(x => x.Longitude);
                var ymin = path.Min(x => x.Latitude);
                var ymax = path.Max(x => x.Latitude);
                return new Rect(new Point(xmin, ymax), new Point(xmax, ymin));
            }
            catch (Exception e)
            {
                e.EdmsHandle();
            }
            return new Rect(
                new Point(
                    path[0].Longitude < path[1].Longitude ? path[0].Longitude : path[1].Longitude,
                    path[0].Latitude > path[1].Latitude ? path[0].Latitude : path[1].Latitude
                ),
                new Point(
                    path[0].Longitude > path[1].Longitude ? path[0].Longitude : path[1].Longitude,
                    path[0].Latitude < path[1].Latitude ? path[0].Latitude : path[1].Latitude
                )
            );
        }

        static public cweeTask<MapPolyline> CreateMapPolyline(Windows.UI.Color _col, bool dashed, double thickness, List<Windows.Devices.Geolocation.BasicGeoposition> path)
        {
            return EdmsTasks.InsertJob(() => {
                try
                {
                    var obj = new MapPolyline();
                    {
                        obj.Visible = true;
                        obj.StrokeColor = _col;
                        obj.StrokeDashed = dashed;
                        obj.StrokeThickness = thickness;
                        if (path.Count > 1)
                        {
                            obj.Path = new Windows.Devices.Geolocation.Geopath(path);
                            //var bb = MakeBoundingBox(path);
                            //try
                            //{
                            //    var area = bb.Width * bb.Height;
                            //    if (area <= 0)
                            //    {

                            //    }
                            //} catch (Exception e) { }
                        }
                    }
                    return obj;
                }
                catch (Exception e)
                {
                    e.EdmsHandle();
                    return null;
                }
            }, true, true);
        }
        public cweeTask<MapPolyline> AddMapPolyline(Windows.UI.Color _col, bool dashed, double thickness, List<Windows.Devices.Geolocation.BasicGeoposition> path)
        {
            cweeTask<MapPolyline> makeIt = CreateMapPolyline(_col, dashed, thickness, path);
            return makeIt.ContinueWith(() => {
                MapPolyline icon = makeIt.Result;
                if (icon != null)
                {
                    try
                    {
                        lock (this.DynamicLayer)
                        {
                            lock (this.map)
                            {
                                this.DynamicLayer.Add(icon);
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        e.EdmsHandle();
                    }
                }
                return icon;
            }, true);
        }

        public class TemporaryMapTileDataSourceContainer
        {
            public MapBackground_Interop Source;
            public int SourceIndex = -1;
            ~TemporaryMapTileDataSourceContainer()
            {
                if (SourceIndex >= 0)
                    WaterWatch.DoScript($"external_data.EraseMatrix({SourceIndex});");
            }
        }

        public class CustomMapTileDataSourceWithTag : CustomMapTileDataSource
        {
            public static int pixelsPerPage = 256;
            public List<TemporaryMapTileDataSourceContainer> Sources;
            public int SourceIndex;
            public AtomicInt StopStreaming;
            public static void QueueBitMapStream(CustomMapTileDataSource sender, MapTileBitmapRequestedEventArgs args)
            {
                if ((sender as CustomMapTileDataSourceWithTag).StopStreaming.Get() != 0)
                {
                    return;
                }

                CreateBitmapAsStreamAsync(args, CustomMapTileDataSourceWithTag.pixelsPerPage, (sender as CustomMapTileDataSourceWithTag).Sources.ElementAt((sender as CustomMapTileDataSourceWithTag).SourceIndex));
            }

            // Create the custom tiles.
            private static void CreateBitmapAsStreamAsync(MapTileBitmapRequestedEventArgs args, int pixelSize, TemporaryMapTileDataSourceContainer cweeMapBackground)
            {
                MapTileBitmapRequestDeferral _deferral = args.Request.GetDeferral();
                try {
                    int x_pos = args.X;
                    int y_pos = args.Y;
                    int zoom = args.ZoomLevel;

                    int pixelHeight = pixelSize;
                    int pixelWidth = pixelSize;
                    int bpp = 4;
                    int byteIndex; int x;

                    byte[] bytes = new byte[pixelHeight * pixelWidth * bpp];

                    {
                        ExtensionMethods.TileSystem.TileXYToPixelXY(x_pos, y_pos, out x_pos, out y_pos);

                        double mapSize = ExtensionMethods.TileSystem.MapSize(zoom);
                        double mapX = 0, mapY = 0, v = 0;

                        mapY = 0.5 - (ExtensionMethods.TileSystem.Clip(y_pos, 0, mapSize - 1) / mapSize);
                        ExtensionMethods.TileSystem.PixelYToLat_Fast(ref mapY, out double latitude);

                        mapX = (ExtensionMethods.TileSystem.Clip(x_pos, 0, mapSize - 1) / mapSize) - 0.5;
                        ExtensionMethods.TileSystem.PixelXToLong_Fast(ref mapX, out double longitude);

                        mapY = 0.5 - (ExtensionMethods.TileSystem.Clip(y_pos + pixelHeight - 1, 0, mapSize - 1) / mapSize);
                        ExtensionMethods.TileSystem.PixelYToLat_Fast(ref mapY, out double latitudeBottom);

                        mapX = (ExtensionMethods.TileSystem.Clip(x_pos + pixelWidth - 1, 0, mapSize - 1) / mapSize) - 0.5;
                        ExtensionMethods.TileSystem.PixelXToLong_Fast(ref mapX, out double longitudeRight);

                        EdmsTasks.InsertJob(() =>
                        {
                            Array.Clear(bytes, 0, bytes.Length);

                            try
                            {
                                // for (int matrix_index = 0; matrix_index < cweeMapBackgrounds.Count; matrix_index++)
                                {
                                    // TemporaryMapTileDataSourceContainer cweeMapBackground = cweeMapBackgrounds[matrix_index];

                                    SharedMatrix shared_matrix = new SharedMatrix(cweeMapBackground.Source.matrix, false);

                                    Color_Interop minCol = cweeMapBackground.Source.min_color;
                                    Color_Interop maxCol = cweeMapBackground.Source.max_color;
                                    double minValue = cweeMapBackground.Source.minValue;
                                    double maxValue = cweeMapBackground.Source.maxValue;
                                    double alpha_foreground = 0;
                                    double alpha_background = 0;

                                    if (shared_matrix.GetNumValues() > 0 && maxValue > minValue)
                                    {
                                        {
                                            {
#if true
                                                if (minCol.A == maxCol.A && minCol.R == maxCol.R && minCol.G == maxCol.G && minCol.B == maxCol.B)
                                                {
                                                    // there is no gradient within this range -- are they using bounds clipping?
                                                    if (cweeMapBackground.Source.clipToBounds)
                                                    {
                                                        // got to do the analysis, but we can skip blending the colors
                                                        vector_double values;
                                                        if (cweeMapBackground.Source.highQuality)
                                                        {
                                                            values = shared_matrix.GetTimeSeries(longitude, latitude, longitudeRight, latitudeBottom, pixelWidth, pixelHeight);
                                                        }
                                                        else
                                                        {
                                                            values = shared_matrix.GetKnotSeries(longitude, latitude, longitudeRight, latitudeBottom, pixelWidth, pixelHeight);
                                                        }
                                                        int n = values.Count;
                                                        //alpha_foreground = minCol.A / 255.0;
                                                        for (int i = 0; i < n; i++)
                                                        {
                                                            v = values[i];
                                                            byteIndex = i * bpp;
                                                            if (values[i] >= minValue && values[i] <= maxValue)
                                                            {
                                                                //alpha_background = (double)bytes[byteIndex + 3] / 255.0;

                                                                bytes[byteIndex + 0] = (byte)minCol.R;// (alpha_foreground * minCol.R + alpha_background * (double)bytes[byteIndex + 0] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 1] = (byte)minCol.G;//(alpha_foreground * minCol.G + alpha_background * (double)bytes[byteIndex + 1] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 2] = (byte)minCol.B;//(alpha_foreground * minCol.B + alpha_background * (double)bytes[byteIndex + 2] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 3] = (byte)minCol.A;//((1.0 - (1.0 - alpha_foreground) * (1.0 - alpha_background)) * 255.0);
                                                            }
                                                            else
                                                            {
                                                                bytes[byteIndex + 0] = 0;
                                                                bytes[byteIndex + 1] = 0;
                                                                bytes[byteIndex + 2] = 0;
                                                                bytes[byteIndex + 3] = 0;
                                                            }
                                                        }

                                                    }
                                                    else
                                                    {
                                                        // no point in doing the analysis -- there is no "clip to bounds" and there will be no color transitions. 
                                                        // alpha_foreground = minCol.A / 255.0;

                                                        for (int y = 0; y < pixelHeight; y++)
                                                        {
                                                            for (x = 0; x < pixelWidth; x++)
                                                            {
                                                                byteIndex = (y * pixelWidth + x) * bpp;

                                                                // alpha_background = (double)bytes[byteIndex + 3] / 255.0;

                                                                bytes[byteIndex + 0] = (byte)minCol.R;//(alpha_foreground * (double)minCol.R + alpha_background * (double)bytes[byteIndex + 0] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 1] = (byte)minCol.G;//(alpha_foreground * (double)minCol.G + alpha_background * (double)bytes[byteIndex + 1] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 2] = (byte)minCol.B;//(alpha_foreground * (double)minCol.B + alpha_background * (double)bytes[byteIndex + 2] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 3] = (byte)minCol.A;//((1.0 - (1.0 - alpha_foreground) * (1.0 - alpha_background)) * 255.0);
                                                            }
                                                        }
                                                    }
                                                }
                                                else
#endif
                                                {
                                                    // traditional, full analysis
                                                    vector_double values;
                                                    if (cweeMapBackground.Source.highQuality)
                                                    {
                                                        values = shared_matrix.GetTimeSeries(longitude, latitude, longitudeRight, latitudeBottom, pixelWidth, pixelHeight);
                                                    }
                                                    else
                                                    {
                                                        values = shared_matrix.GetKnotSeries(longitude, latitude, longitudeRight, latitudeBottom, pixelWidth, pixelHeight);
                                                    }
                                                    int n = values.Count;
                                                    for (int i = 0; i < n; i++)
                                                    {
                                                        v = (values[i] - minValue) / (maxValue - minValue); // 0 - 1 between the min and max for this value
                                                        byteIndex = i * bpp;

                                                        if (v < 0)
                                                        {
                                                            if (!cweeMapBackground.Source.clipToBounds)
                                                            {
                                                                //alpha_foreground = minCol.A / 255.0;
                                                                //alpha_background = (double)bytes[byteIndex + 3] / 255.0;

                                                                bytes[byteIndex + 0] = (byte)minCol.R;//(alpha_foreground * minCol.R + alpha_background * (double)bytes[byteIndex + 0] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 1] = (byte)minCol.G;//(alpha_foreground * minCol.G + alpha_background * (double)bytes[byteIndex + 1] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 2] = (byte)minCol.B;//(alpha_foreground * minCol.B + alpha_background * (double)bytes[byteIndex + 2] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 3] = (byte)minCol.A;//((1.0 - (1.0 - alpha_foreground) * (1.0 - alpha_background)) * 255.0);
                                                            }
                                                        }
                                                        else if (v > 1)
                                                        {
                                                            if (!cweeMapBackground.Source.clipToBounds)
                                                            {
                                                                //alpha_foreground = maxCol.A / 255.0;
                                                                //alpha_background = (double)bytes[byteIndex + 3] / 255.0;

                                                                bytes[byteIndex + 0] = (byte)maxCol.R; // (alpha_foreground * maxCol.R + alpha_background * (double)bytes[byteIndex + 0] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 1] = (byte)maxCol.G; // (alpha_foreground * maxCol.G + alpha_background * (double)bytes[byteIndex + 1] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 2] = (byte)maxCol.B; // (alpha_foreground * maxCol.B + alpha_background * (double)bytes[byteIndex + 2] * (1.0 - alpha_foreground));
                                                                bytes[byteIndex + 3] = (byte)maxCol.A; // ((1.0 - (1.0 - alpha_foreground) * (1.0 - alpha_background)) * 255.0);
                                                            }
                                                        }
                                                        else
                                                        {
                                                            //alpha_foreground = minCol.A.Lerp(maxCol.A, v) / 255.0;
                                                            //alpha_background = (double)bytes[byteIndex + 3] / 255.0;

                                                            bytes[byteIndex + 0] = (byte)minCol.R.Lerp(maxCol.R, v); // (alpha_foreground * minCol.R.Lerp(maxCol.R, v) + alpha_background * (double)bytes[byteIndex + 0] * (1.0 - alpha_foreground));
                                                            bytes[byteIndex + 1] = (byte)minCol.G.Lerp(maxCol.G, v); // (alpha_foreground * minCol.G.Lerp(maxCol.G, v) + alpha_background * (double)bytes[byteIndex + 1] * (1.0 - alpha_foreground));
                                                            bytes[byteIndex + 2] = (byte)minCol.B.Lerp(maxCol.B, v); // (alpha_foreground * minCol.B.Lerp(maxCol.B, v) + alpha_background * (double)bytes[byteIndex + 2] * (1.0 - alpha_foreground));
                                                            bytes[byteIndex + 3] = (byte)minCol.A.Lerp(maxCol.A, v); // ((1.0 - (1.0 - alpha_foreground) * (1.0 - alpha_background)) * 255.0);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        alpha_foreground = minCol.A / 255.0;
                                        for (int y = 0; y < pixelHeight; y++)
                                        {
                                            for (x = 0; x < pixelWidth; x++)
                                            {
                                                byteIndex = (y * pixelWidth + x) * bpp;
                                                //alpha_background = (double)bytes[byteIndex + 3] / 255.0;

                                                bytes[byteIndex + 0] = (byte)minCol.R; //  (alpha_foreground * (double)minCol.R + alpha_background * (double)bytes[byteIndex + 0] * (1.0 - alpha_foreground));
                                                bytes[byteIndex + 1] = (byte)minCol.G; // (alpha_foreground * (double)minCol.G + alpha_background * (double)bytes[byteIndex + 1] * (1.0 - alpha_foreground));
                                                bytes[byteIndex + 2] = (byte)minCol.B; // (alpha_foreground * (double)minCol.B + alpha_background * (double)bytes[byteIndex + 2] * (1.0 - alpha_foreground));
                                                bytes[byteIndex + 3] = (byte)minCol.A; // ((1.0 - (1.0 - alpha_foreground) * (1.0 - alpha_background)) * 255.0);
                                            }
                                        }
                                    }
                                }
                            }
                            catch (Exception) { }
                        }, false).ContinueWith(()=> {
                            // Create RandomAccessStream from byte array.
                            InMemoryRandomAccessStream randomAccessStream = new InMemoryRandomAccessStream();
                            IOutputStream outputStream = randomAccessStream.GetOutputStreamAt(0);

                            var writer = new DataWriter(outputStream);
                            {
                                writer.WriteBytes(bytes);
                                args.Request.PixelData = RandomAccessStreamReference.CreateFromStream(randomAccessStream);
                                _deferral.Complete();
                                writer.StoreAsync().AsTask().ContinueWith((System.Threading.Tasks.Task<uint> a) =>
                                {
                                    writer.FlushAsync().AsTask().ContinueWith((System.Threading.Tasks.Task<bool> b) =>
                                    {
                                        try
                                        {
                                            writer.DetachStream();
                                            writer.Dispose();
                                        }
                                        catch (Exception)
                                        {

                                        }
                                        finally
                                        {
                                            // _deferral.Complete();
                                        }
                                    });
                                });
                            }
                        }, false);
                    }                    
                } catch (Exception e) {
                    e.EdmsHandle();
                    _deferral.Complete();
                } finally { }
            }
        }

        public EdmsTasks.cweeTask StreamBackground(List<MapBackground_Interop> matrixes, AtomicInt streamStopper)
        {
            AtomicInt locker = new AtomicInt();
            return EdmsTasks.InsertJob(()=> {
                if (this.IsLoaded) {
                    locker.Increment();
                }
                else {
                    this.Loaded += (object sender, RoutedEventArgs e)=> { locker.Increment(); };
                }
                return EdmsTasks.cweeTask.ContinueWhenTrue(() => { if (locker.Increment() > 1) { locker.Decrement(); return true; } else { locker.Decrement(); return false; } }).ContinueWith(() => {
                    // custom data source
                    List<TemporaryMapTileDataSourceContainer> sources = new List<TemporaryMapTileDataSourceContainer>();
                    int index = 0;
                    foreach (var matrix in matrixes)
                    {
                        double minX = -double.MaxValue, maxX = double.MaxValue, minY = -double.MaxValue, maxY = double.MaxValue;

                        sources.Add(new TemporaryMapTileDataSourceContainer() { Source = matrix, SourceIndex = matrix.matrix });
                        SharedMatrix shared_matrix = new SharedMatrix(matrix.matrix, false);

                        if (shared_matrix.GetNumValues() > 0)
                        {
                            minX = Math.Max(minX, shared_matrix.GetMinX());
                            maxX = Math.Min(maxX, shared_matrix.GetMaxX());
                            minY = Math.Max(minY, shared_matrix.GetMinY());
                            maxY = Math.Min(maxY, shared_matrix.GetMaxY());
                        }

                        CustomMapTileDataSourceWithTag customDataSource = new CustomMapTileDataSourceWithTag() { SourceIndex = index, Sources = sources, StopStreaming = streamStopper };

                        index++;

                        AtomicInt stopStreaming = customDataSource.StopStreaming;

                        map.Loaded += (object sender, RoutedEventArgs e) => {
                            stopStreaming.Set(0);
                        };
                        map.Unloaded += (object sender, RoutedEventArgs e) => {
                            stopStreaming.Set(1);
                        };

                        if (minX > -double.MaxValue && maxX < double.MaxValue && minY > -double.MaxValue && maxY < double.MaxValue)
                        {
                            try
                            {
                                customDataSource.BitmapRequested += CustomMapTileDataSourceWithTag.QueueBitMapStream;
                                map.TileSources.Add(new MapTileSource(customDataSource)
                                {
                                    TilePixelSize = CustomMapTileDataSourceWithTag.pixelsPerPage
                                    ,
                                    AllowOverstretch = true
                                    ,
                                    IsFadingEnabled = false
                                    ,
                                    Layer = MapTileLayer.AreaOverlay
                                    ,
                                    Bounds = new GeoboundingBox(
                                        new BasicGeoposition() { Longitude = minX, Latitude = maxY },
                                        new BasicGeoposition() { Longitude = maxX, Latitude = minY }
                                    )
                                });
                            }
                            catch (Exception) { }
                        }
                        else
                        {
                            try
                            {
                                customDataSource.BitmapRequested += CustomMapTileDataSourceWithTag.QueueBitMapStream;
                                map.TileSources.Add(new MapTileSource(customDataSource)
                                {
                                    TilePixelSize = CustomMapTileDataSourceWithTag.pixelsPerPage
                                    ,
                                    AllowOverstretch = true
                                    ,
                                    IsFadingEnabled = false
                                    ,
                                    Layer = MapTileLayer.AreaOverlay
                                });

                            }
                            catch (Exception) { }
                        }
                    }
                }, true);
            }, true);
        }
    }

    public class SimpleMapVM : ViewModelBase
    {
        public SimpleMapVM() { }
        ~SimpleMapVM() {
            map = null;
            container = null;
        }

        public void Reload()
        {
            for (int i = container.Children.Count-1; i >=0; i--)
            {
                if (container.Children[i] is cweeMap)
                {
                    container.Children.RemoveAt(i);
                }
            }
            container.Children.Add(map);

            map.map.MapProjection = MapProjection.Globe;
        }
        public cweeMap map { get; set; } = new cweeMap();
        public Grid container { get; set; }
        // public 
    }

    public sealed partial class SimpleMap : UserControl
    {
        public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;
        public void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string propertyName = null)
        {
            EdmsTasks.InsertJob(
                () => PropertyChanged?.Invoke(this, new System.ComponentModel.PropertyChangedEventArgs(propertyName)), true, true
            );
        }

        // --> DATA
        public SimpleMapVM vm = new SimpleMapVM();

        // --> Public Methods
        public SimpleMap()
        {
            this.InitializeComponent();
            this.Loaded += DoOnceLoaded;
        }
        ~SimpleMap() {
            vm = null;
        }

        // --> Private Methods
        private static void DoOnceLoaded(object sender, RoutedEventArgs e)
        {
            (sender as SimpleMap).Loaded -= DoOnceLoaded;

            // grab the template parts
            (sender as SimpleMap).vm.container = (sender as SimpleMap).PART_ContainerGrid as Grid; if ((sender as SimpleMap).vm.container == null) { throw (new Exception("SimpleMap's Container not found.")); };

            // Tell the VM to load
            (sender as SimpleMap).vm.Reload();
        }

        private void StyleSelect(object sender, RoutedEventArgs e)
        {
            int selection = 0;
            var selection_tag = (sender as MenuFlyoutItem).Tag;
            if (selection_tag is int)
            {
                selection = (int)selection_tag;
            }
            else if (selection_tag is string)
            {
                selection = int.Parse(selection_tag as string);
            }

            switch (selection)
            {
                case -1:
                    this.vm.map.map.Style = MapStyle.None;
                    break;
                default:
                    this.vm.map.map.Style = MapStyle.None;
                    this.vm.map.map.StyleSheet = cweeMap.MapTools.GetStyleSheet(selection);
                    break;
            }

            this.vm.map.map.MapProjection = MapProjection.Globe;

        }
    }
}
