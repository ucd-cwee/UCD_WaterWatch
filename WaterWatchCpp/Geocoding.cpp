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
#include "Units.h"
#include "../cNominitum/cNominitum.h"
#include "Geocoding.h"
#include "InterlockedValues.h"
#include "cweeInterlocked.h"
#include "DispatchTimer.h"
#include "cweeThreadedMap.h"
#include "Mutex.h"
#include "Engineering.h"

class cweeGeocodingLocal : public cweeGeocoding {
public:
    /*!
    Initialize nominatim
    */
    cweeGeocodingLocal() : cweeGeocoding(), nominatim(new C_GeocodingSupport()), usgs(new C_USGS()) {};

    /*!
    Uninitialize nominatim
    */
    ~cweeGeocodingLocal() {};

    /*!
    Query "NationalMap.gov" to identify the elevation (feet) of the longitude/latitude.
    */
    double          GetElevation(const vec2d& LongLat) {
        double out;
        AUTO g = usgs.Guard();
        usgs.UnsafeGet()->GetElevation(LongLat, out);
        return out;
    };

    /*!
    Query "Open Street Map" to identify Longitude and Latitude (X,Y of return) of best-fit to address.
    */
    vec2d           GetLongLat(const cweeStr& address) {
		return nominatim->GetLongLat(address);
    };

    vec2d           GetLongLat(double easting_feet, double northing_feet,
        double centralMeridian,
        double LatFirstStandardParallel,
        double LatSecondStandardParallel,
        double LatOrigin,
        double FalseNorthing,
        double FalseEasting
    ) {
        return cweeEng::CoordinateConversion_FeetToLongLat(vec2d(easting_feet, northing_feet), 
            centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel, LatOrigin, FalseNorthing, FalseEasting
        );
    };

    std::vector<std::map<std::string, cweeStr>> Geocode(const cweeStr& address) {
		return nominatim->Geocode(address);
    };

    std::map<std::string, cweeStr> GetAddressComponents(const vec2d& LongLat) {
		return nominatim->GetAddressComponents(LongLat);
    };

    /*!
    Query "Open Street Map" to identify address that is best-fit to Longitude and Latitude (X,Y of input).
    */
    cweeStr         GetAddress(const vec2& LongLat) {
		return nominatim->GetAddress(LongLat);
    };
    /*!
    Query "Open Street Map" to identify address that is best-fit to Longitude and Latitude (X,Y of input).
    */
    cweeStr         GetAddress(const vec2d& LongLat) {
		return nominatim->GetAddress(LongLat);
    };

    /*!
    Calculate the mercator X and Y (X,Y of return, in meters) from the supplied Longitude and Latitude (X,Y of input).
    */
    vec2		    GetMercatorXY(const vec2& LongLat) {
        constexpr auto EARTH_RADIUS = 6378137.0f; // meters
        return vec2(
            DEG2RAD(LongLat.x) * EARTH_RADIUS,
            log(tan(DEG2RAD(LongLat.y) / 2.0f + cweeMath::PI / 4.0f)) * EARTH_RADIUS
        );
    };
    /*!
    Calculate the mercator X and Y (X,Y of return, in meters) from the supplied Longitude and Latitude (X,Y of input).
    */
    vec2d		    GetMercatorXY(const vec2d& LongLat) {
        constexpr auto EARTH_RADIUS = 6378137.0; // meters
        return vec2d(
            DEG2RAD(LongLat.x) * EARTH_RADIUS,
            log(tan(DEG2RAD(LongLat.y) / 2.0 + (double)cweeMath::PI / 4.0)) * EARTH_RADIUS
        );
    };

    /*!
    Calculate the Longitude and Latitude (X,Y of return) from the supplied mercator X and Y (X,Y of input, in meters).
    */
    vec2		    GetLongLat(const vec2& MercatorXY) {
        constexpr auto EARTH_RADIUS = 6378137.0f;
        return vec2(
            RAD2DEG(MercatorXY.x / EARTH_RADIUS),
            RAD2DEG(2.0f * ::atan(::exp(MercatorXY.y / EARTH_RADIUS)) - cweeMath::PI / 2.0f)
        );
    };
    /*!
    Calculate the Longitude and Latitude (X,Y of return) from the supplied mercator X and Y (X,Y of input, in meters).
    */
    vec2d		    GetLongLat(const vec2d& MercatorXY) {
        constexpr auto EARTH_RADIUS = 6378137.0;
        return vec2d(
            RAD2DEG(MercatorXY.x / EARTH_RADIUS),
            RAD2DEG(2.0 * ::atan(::exp(MercatorXY.y / EARTH_RADIUS)) - (double)cweeMath::PI / 2.0)
        );
    };

    cweeStr         GetWeatherData(const u64& start, const u64& end, const double& longitude, const double& latitude) {
        return "";
    };

    cweeStr         QueryHttp(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "EdmsApp") {
        cweeStr filePath, out;

        C_WINHTTP http;

        http.Query(filePath, mainAddress, requestParameters, UniqueSessionName);

        {
            std::string get;
            std::ifstream file(filePath.c_str()); // ifstream is read only, ofstream is write only, fstream is read/write.
            out.Clear();
            while (std::getline(file, get)) {
                out.AddToDelimiter(get.c_str(), '\n');
            }
            file.close();
        }
        std::remove(filePath.c_str());

        return out;
    };

    units::length::meter_t          Distance(vec2d const& LongLat1, vec2d const& LongLat2)
    {
        double  lat_old = LongLat1.y * cweeMath::PI / 180.0;
        double  lat_new = LongLat2.y * cweeMath::PI / 180.0;
        double  lat_diff = (LongLat2.y - LongLat1.y) * cweeMath::PI / 180.0;
        double  lng_diff = (LongLat2.x - LongLat1.x) * cweeMath::PI / 180.0;

        double  a = std::sin(lat_diff / 2.0) * std::sin(lat_diff / 2.0) + std::cos(lat_new) * std::cos(lat_old) * std::sin(lng_diff / 2.0) * std::sin(lng_diff / 2.0);
        double  c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        units::length::meter_t  distance = 6372797.56085 * c;

        return distance;
    }

private:

    float                   DEG2RAD(float deg) {
        return deg / (180.0f / cweeMath::PI);
    };
    float                   RAD2DEG(float rad) {
        return rad * (180.0f / cweeMath::PI);
    };
    double                  DEG2RAD(const double& deg) {
        return deg / (180.0 / (double)cweeMath::PI);
    };
    double                  RAD2DEG(const double& rad) {
        return rad * (180.0 / (double)cweeMath::PI);
    };

    cweeSharedPtr< C_GeocodingSupport >		nominatim;
    cweeSharedPtr< C_USGS >                 usgs;
};
cweeSharedPtr<cweeGeocoding> geocoding = make_cwee_shared<cweeGeocodingLocal>(new cweeGeocodingLocal()).CastReference<cweeGeocoding>();