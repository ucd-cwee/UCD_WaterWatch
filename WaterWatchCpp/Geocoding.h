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

#pragma once
#include "Precompiled.h"
#include "enum.h"
#include "Strings.h"
#include "vec.h"
#include "SharedPtr.h"
#include "List.h"
#include "cweeTime.h"

class cweeGeocoding {
public:
    /*!
    Initialize nominatim
    */
    cweeGeocoding() {};

    /*!
    Uninitialize nominatim
    */
    virtual ~cweeGeocoding() {};

    /*!
    Query "NationalMap.gov" to identify the elevation (feet) of the longitude/latitude.
    */
    virtual double          GetElevation(const vec2d& LongLat) = 0;

    /*!
    Query "Open Street Map" to identify Longitude and Latitude (X,Y of return) of best-fit to address.
    */
    virtual vec2d           GetLongLat(const cweeStr& address) = 0;

    virtual vec2d           GetLongLat(double easting_feet, double northing_feet,
        double centralMeridian = -120.5000000000000000,
        double LatFirstStandardParallel = 37.0666666666666666667,
        double LatSecondStandardParallel = 38.433333333333333333,
        double LatOrigin = 36.500000000000000000,
        double FalseNorthing = 1640416.6666666666666,
        double FalseEasting = 6561666.6666666666666
        ) = 0;

    virtual std::vector<std::map<std::string, cweeStr>> Geocode(const cweeStr& address) = 0;

    virtual std::map<std::string, cweeStr> GetAddressComponents(const vec2d& LongLat) = 0;

    /*!
    Query "Open Street Map" to identify address that is best-fit to Longitude and Latitude (X,Y of input).
    */
    virtual cweeStr         GetAddress(const vec2& LongLat) = 0;
    
    /*!
    Query "Open Street Map" to identify address that is best-fit to Longitude and Latitude (X,Y of input).
    */
    virtual cweeStr         GetAddress(const vec2d& LongLat) = 0;

    /*!
    Calculate the mercator X and Y (X,Y of return, in meters) from the supplied Longitude and Latitude (X,Y of input).
    */
    virtual vec2		    GetMercatorXY(const vec2& LongLat) = 0;
    
    /*!
    Calculate the mercator X and Y (X,Y of return, in meters) from the supplied Longitude and Latitude (X,Y of input).
    */
    virtual vec2d		    GetMercatorXY(const vec2d& LongLat) = 0;

    /*!
    Calculate the Longitude and Latitude (X,Y of return) from the supplied mercator X and Y (X,Y of input, in meters).
    */
    virtual vec2		    GetLongLat(const vec2& MercatorXY) = 0;
    
    /*!
    Calculate the Longitude and Latitude (X,Y of return) from the supplied mercator X and Y (X,Y of input, in meters).
    */
    virtual vec2d		    GetLongLat(const vec2d& MercatorXY) = 0;

    virtual cweeStr         GetWeatherData(const u64& start, const u64& end, const double& longitude, const double& latitude) = 0;

    virtual cweeStr         QueryHttp(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "EdmsApp") = 0;

    virtual units::length::meter_t 
                            Distance(vec2d const& LongLat1, vec2d const& LongLat2) = 0;
};

extern cweeSharedPtr<cweeGeocoding> geocoding;