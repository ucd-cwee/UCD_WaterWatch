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
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
using Windows.Graphics.Imaging;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Maps;
using Windows.UI.Xaml.Media.Imaging;

namespace UWP_WaterWatchLibrary
{
    public static class ExtensionMethods
    {
        public static class TileSystem
        {
            private const double EarthRadius = 6378137;
            private const double MinLatitude = -85.05112878;
            private const double MaxLatitude = 85.05112878;
            private const double MinLongitude = -180;
            private const double MaxLongitude = 180;

            /// <summary>  
            /// Clips a number to the specified minimum and maximum values.  
            /// </summary>  
            /// <param name="n">The number to clip.</param>  
            /// <param name="minValue">Minimum allowable value.</param>  
            /// <param name="maxValue">Maximum allowable value.</param>  
            /// <returns>The clipped value.</returns>  
            public static double Clip(double n, double minValue, double maxValue)
            {
                return Math.Min(Math.Max(n, minValue), maxValue);
            }

            /// <summary>  
            /// Determines the map width and height (in pixels) at a specified level  
            /// of detail.  
            /// </summary>  
            /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)  
            /// to 23 (highest detail).</param>  
            /// <returns>The map width and height in pixels.</returns>  
            public static uint MapSize(int levelOfDetail)
            {
                return (uint)256 << levelOfDetail;
            }

            /// <summary>  
            /// Determines the ground resolution (in meters per pixel) at a specified  
            /// latitude and level of detail.  
            /// </summary>  
            /// <param name="latitude">Latitude (in degrees) at which to measure the  
            /// ground resolution.</param>  
            /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)  
            /// to 23 (highest detail).</param>  
            /// <returns>The ground resolution, in meters per pixel.</returns>  
            public static double GroundResolution(double latitude, int levelOfDetail)
            {
                latitude = Clip(latitude, MinLatitude, MaxLatitude);
                return Math.Cos(latitude * Math.PI / 180) * 2 * Math.PI * EarthRadius / MapSize(levelOfDetail);
            }

            /// <summary>  
            /// Determines the map scale at a specified latitude, level of detail,  
            /// and screen resolution.  
            /// </summary>  
            /// <param name="latitude">Latitude (in degrees) at which to measure the  
            /// map scale.</param>  
            /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)  
            /// to 23 (highest detail).</param>  
            /// <param name="screenDpi">Resolution of the screen, in dots per inch.</param>  
            /// <returns>The map scale, expressed as the denominator N of the ratio 1 : N.</returns>  
            public static double MapScale(double latitude, int levelOfDetail, int screenDpi)
            {
                return GroundResolution(latitude, levelOfDetail) * screenDpi / 0.0254;
            }

            /// <summary>  
            /// Converts a point from latitude/longitude WGS-84 coordinates (in degrees)  
            /// into pixel XY coordinates at a specified level of detail.  
            /// </summary>  
            /// <param name="latitude">Latitude of the point, in degrees.</param>  
            /// <param name="longitude">Longitude of the point, in degrees.</param>  
            /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)  
            /// to 23 (highest detail).</param>  
            /// <param name="pixelX">Output parameter receiving the X coordinate in pixels.</param>  
            /// <param name="pixelY">Output parameter receiving the Y coordinate in pixels.</param>  
            public static void LatLongToPixelXY(double latitude, double longitude, int levelOfDetail, out int pixelX, out int pixelY)
            {
                latitude = Clip(latitude, MinLatitude, MaxLatitude);
                longitude = Clip(longitude, MinLongitude, MaxLongitude);

                double x = (longitude + 180) / 360;
                double sinLatitude = Math.Sin(latitude * Math.PI / 180);
                double y = 0.5 - Math.Log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * Math.PI);

                uint mapSize = MapSize(levelOfDetail);
                pixelX = (int)Clip(x * mapSize + 0.5, 0, mapSize - 1);
                pixelY = (int)Clip(y * mapSize + 0.5, 0, mapSize - 1);
            }

            /// <summary>  
            /// Converts a pixel from pixel XY coordinates at a specified level of detail  
            /// into latitude/longitude WGS-84 coordinates (in degrees).  
            /// </summary>  
            /// <param name="pixelX">X coordinate of the point, in pixels.</param>  
            /// <param name="pixelY">Y coordinates of the point, in pixels.</param>  
            /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)  
            /// to 23 (highest detail).</param>  
            /// <param name="latitude">Output parameter receiving the latitude in degrees.</param>  
            /// <param name="longitude">Output parameter receiving the longitude in degrees.</param>  
            public static void PixelXYToLatLong(double pixelX, double pixelY, int levelOfDetail, out double latitude, out double longitude)
            {
                double mapSize = MapSize(levelOfDetail);
                double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;
                double y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);

                latitude = 90 - 360 * Math.Atan(Math.Exp(-y * 2 * Math.PI)) / Math.PI;
                longitude = 360 * x;
            }

            public static void PixelXToLong_Fast(ref double mapX, out double longitude)
            {
                longitude = 360 * mapX;
            }
            public static void PixelYToLat_Fast(ref double mapY, out double latitude)
            {
                latitude = 90 - 360 * Math.Atan(Math.Exp(-mapY * 2 * Math.PI)) / Math.PI;
            }

            /// <summary>  
            /// Converts pixel XY coordinates into tile XY coordinates of the tile containing  
            /// the specified pixel.  
            /// </summary>  
            /// <param name="pixelX">Pixel X coordinate.</param>  
            /// <param name="pixelY">Pixel Y coordinate.</param>  
            /// <param name="tileX">Output parameter receiving the tile X coordinate.</param>  
            /// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>  
            public static void PixelXYToTileXY(int pixelX, int pixelY, out int tileX, out int tileY)
            {
                tileX = pixelX / 256;
                tileY = pixelY / 256;
            }

            /// <summary>  
            /// Converts tile XY coordinates into pixel XY coordinates of the upper-left pixel  
            /// of the specified tile.  
            /// </summary>  
            /// <param name="tileX">Tile X coordinate.</param>  
            /// <param name="tileY">Tile Y coordinate.</param>  
            /// <param name="pixelX">Output parameter receiving the pixel X coordinate.</param>  
            /// <param name="pixelY">Output parameter receiving the pixel Y coordinate.</param>  
            public static void TileXYToPixelXY(int tileX, int tileY, out int pixelX, out int pixelY)
            {
                pixelX = tileX * 256;
                pixelY = tileY * 256;
            }

            /// <summary>  
            /// Converts tile XY coordinates into a QuadKey at a specified level of detail.  
            /// </summary>  
            /// <param name="tileX">Tile X coordinate.</param>  
            /// <param name="tileY">Tile Y coordinate.</param>  
            /// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)  
            /// to 23 (highest detail).</param>  
            /// <returns>A string containing the QuadKey.</returns>  
            public static string TileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
            {
                StringBuilder quadKey = new StringBuilder();
                for (int i = levelOfDetail; i > 0; i--)
                {
                    char digit = '0';
                    int mask = 1 << (i - 1);
                    if ((tileX & mask) != 0)
                    {
                        digit++;
                    }
                    if ((tileY & mask) != 0)
                    {
                        digit++;
                        digit++;
                    }
                    quadKey.Append(digit);
                }
                return quadKey.ToString();
            }

            /// <summary>  
            /// Converts a QuadKey into tile XY coordinates.  
            /// </summary>  
            /// <param name="quadKey">QuadKey of the tile.</param>  
            /// <param name="tileX">Output parameter receiving the tile X coordinate.</param>  
            /// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>  
            /// <param name="levelOfDetail">Output parameter receiving the level of detail.</param>  
            public static void QuadKeyToTileXY(string quadKey, out int tileX, out int tileY, out int levelOfDetail)
            {
                tileX = tileY = 0;
                levelOfDetail = quadKey.Length;
                for (int i = levelOfDetail; i > 0; i--)
                {
                    int mask = 1 << (i - 1);
                    switch (quadKey[levelOfDetail - i])
                    {
                        case '0':
                            break;

                        case '1':
                            tileX |= mask;
                            break;

                        case '2':
                            tileY |= mask;
                            break;

                        case '3':
                            tileX |= mask;
                            tileY |= mask;
                            break;

                        default:
                            throw new ArgumentException("Invalid QuadKey digit sequence.");
                    }
                }
            }
        }


        public static int AddLayer(this MapControl map)
        {
            if (map.Layers == null) map.Layers = new List<MapLayer>();
            if (map.Layers != null)
            {
                var Layer = new MapElementsLayer();
                map.Layers.Add(Layer);
                return map.Layers.Count - 1;
            }
            return -1;
        }
        public static int AddorReplaceLayer(this MapControl map, int layer)
        {
            if (map.Layers == null)
            {
                map.Layers = new List<MapLayer>();
            }
            if (map.Layers != null)
            {
                if (map.Layers.Count > layer)
                {
                    map.Layers[layer] = new MapElementsLayer();
                }
                else
                {
                    while (map.Layers.Count <= layer)
                        map.Layers.Add(new MapElementsLayer());
                }
            }

            return layer;
        }
        public static void InsertMapElement(this MapControl map, MapIcon obj, int whichLayer)
        {
            if (map.Layers != null && map.Layers.Count > whichLayer)
            {
                // var Layer = map.Layers[whichLayer] as MapElementsLayer;
                //Layer.MapElements.Add(obj);

                (map.Layers[whichLayer] as MapElementsLayer).MapElements.Add(obj);

                //return Layer.MapElements.Count - 1;
            }
            //return -1;
        }
        public static void InsertMapElement(this MapControl map, MapBillboard obj, int whichLayer)
        {
            if (map.Layers != null && map.Layers.Count > whichLayer)
            {
                //var Layer = map.Layers[whichLayer] as MapElementsLayer;
                //Layer.MapElements.Add(obj);

                (map.Layers[whichLayer] as MapElementsLayer).MapElements.Add(obj);

                //return Layer.MapElements.Count - 1;
            }
            //return -1;
        }
        public static void InsertMapElement(this MapControl map, MapPolyline obj, int whichLayer)
        {
            if (map.Layers != null && map.Layers.Count > whichLayer)
            {
                //var Layer = map.Layers[whichLayer] as MapElementsLayer;
                //Layer.MapElements.Add(obj);

                (map.Layers[whichLayer] as MapElementsLayer).MapElements.Add(obj);

                //return Layer.MapElements.Count - 1;
            }
            //return -1;
        }
        public static void InsertMapElement(this MapControl map, MapPolygon obj, int whichLayer)
        {
            if (map.Layers != null && map.Layers.Count > whichLayer)
            {
                //var Layer = map.Layers[whichLayer] as MapElementsLayer;
                //Layer.MapElements.Add(obj);

                (map.Layers[whichLayer] as MapElementsLayer).MapElements.Add(obj);

                //return Layer.MapElements.Count - 1;
            }
            //return -1;
        }


        public static void SetLayerVisibility(this MapControl map, int whichLayer, bool visibility)
        {
            if (map.Layers != null && map.Layers.Count > whichLayer)
            {
                var Layer = map.Layers[whichLayer] as MapElementsLayer;

                Layer.Visible = visibility;
            }
        }
        public static void SetLayerZ(this MapControl map, int whichLayer, int zIndex)
        {
            if (map.Layers != null && map.Layers.Count > whichLayer)
            {
                var Layer = map.Layers[whichLayer] as MapElementsLayer;
                Layer.ZIndex = zIndex;
            }
        }


        public static bool TryGetVisibleBounds(this MapControl map, out GeoboundingBox bb)
        {
            bb = new GeoboundingBox(new BasicGeoposition(), new BasicGeoposition());

            if (map.ActualWidth == 0 || map.ActualHeight == 0) return false;

            if (!map.TryGetLocationFromOffset(new Windows.Foundation.Point(0, 0), out Geopoint nw_sample)) return false;

            if (!map.TryGetLocationFromOffset(new Windows.Foundation.Point(map.ActualWidth, map.ActualHeight), out Geopoint se_sample)) return false;

            var nw = new Geopoint(new BasicGeoposition() { 
                Longitude = se_sample.Position.Longitude < nw_sample.Position.Longitude ? se_sample.Position.Longitude : nw_sample.Position.Longitude
                , 
                Latitude = se_sample.Position.Latitude >= nw_sample.Position.Latitude ? se_sample.Position.Latitude : nw_sample.Position.Latitude
            });
            var se = new Geopoint(new BasicGeoposition() { 
                Longitude = se_sample.Position.Longitude >= nw_sample.Position.Longitude ? se_sample.Position.Longitude : nw_sample.Position.Longitude
                , 
                Latitude = se_sample.Position.Latitude < nw_sample.Position.Latitude ? se_sample.Position.Latitude : nw_sample.Position.Latitude
            });

            bb = new GeoboundingBox(nw.Position, se.Position);
            return true;
        }

        public static bool IsPointInBoundingBox(this MapControl map, GeoboundingBox bb, BasicGeoposition p)
        {
            Windows.Foundation.Point returned = new Windows.Foundation.Point();
            map.GetOffsetFromLocation(new Geopoint(p), out returned);
            if (returned.X < 0 || returned.Y < 0 || returned.X > map.ActualWidth || returned.Y > map.ActualHeight)
            {
                return false;
            }
            else
            {
                return true;
            }

            //if (bb == null) return true;
            //if (bb.NorthwestCorner.Longitude <= p.Longitude && p.Longitude <= bb.SoutheastCorner.Longitude && bb.SoutheastCorner.Latitude <= p.Latitude && p.Latitude <= bb.NorthwestCorner.Latitude)
            //    return true;
            //else
            //    return false;
        }

        public static bool IsPointInBoundingBox(this MapControl map, BasicGeoposition p)
        {
            if (map != null)
            {
                Windows.Foundation.Point returned = new Windows.Foundation.Point();
                map.GetOffsetFromLocation(new Geopoint(p), out returned);
                if (returned.X < 0 || returned.Y < 0 || returned.X > map.ActualWidth || returned.Y > map.ActualHeight)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
            return false;
        }

        public static void UpdateMap(this MapControl map)
        {
            if (map != null)
            {
                map.Layers = map.Layers;
                map.UpdateLayout();
            }
        }
    }
}
