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
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/enum.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/vec.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/cweeTime.h"

/* Explicit request to USGS for elevation data */
class C_USGS {
public:
    C_USGS();
    C_USGS(const C_USGS&) = delete;
    C_USGS(C_USGS&&) = delete;
    virtual ~C_USGS();
    C_USGS& operator=(const C_USGS&) = delete;
    C_USGS& operator=(C_USGS&&) = delete;

    HRESULT GetElevation(_In_ const vec2d& longLat, _Inout_ double& result);

    void AttachSession(_In_ HANDLE hSession) noexcept;

    HANDLE DetachSession() noexcept;

protected:
    cweeSharedPtr<void> m_Session;
    cweeSharedPtr<void> m_Connection;
    cweeSharedPtr<void> m_File;
};

/* Generic HTTPS Query tool */
class C_WINHTTP
{
public:    
    C_WINHTTP();
    C_WINHTTP(const C_WINHTTP&) = delete;
    C_WINHTTP(C_WINHTTP&&) = delete;
    virtual ~C_WINHTTP();
    C_WINHTTP& operator=(const C_WINHTTP&) = delete;
    C_WINHTTP& operator=(C_WINHTTP&&) = delete;

    HRESULT Query(_Inout_ cweeStr& result, _In_ const cweeStr& mainAddress = "nationalmap.gov", _In_ const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", _In_ const cweeStr& UniqueAppName = "EdmsApp");
    void AttachSession(_In_ HANDLE hSession) noexcept;
    HANDLE DetachSession() noexcept;

protected:
    cweeSharedPtr<void> m_Session;
    cweeSharedPtr<void> m_Connection;
    cweeSharedPtr<void> m_File;
};

/* Explicit call to Nominitum to get geocoding services */
class C_GeocodingSupport {
public:
    C_GeocodingSupport();
    C_GeocodingSupport(const C_GeocodingSupport&) = delete;
    C_GeocodingSupport(C_GeocodingSupport&&) = delete;
    ~C_GeocodingSupport();
    C_GeocodingSupport& operator=(const C_GeocodingSupport&) = delete;
    C_GeocodingSupport& operator=(C_GeocodingSupport&&) = delete;

    /* Geocode string address to best-matched Longitude/Latitude coordinate pair. */
    vec2d           GetLongLat(const cweeStr& address);
    /* Geocode string address to (0 to m) potential location matches with details. */
    std::vector<std::map<std::string, cweeStr>> Geocode(const cweeStr& address);
    /* Geocode string address to best-matched location with details. */
    std::map<std::string, cweeStr> GetAddressComponents(const vec2d& LongLat);  
    /* Reverse Geocode longitude/latitude pair to a single best-matched address */
    cweeStr         GetAddress(const vec2& LongLat);    
    /* Reverse Geocode longitude/latitude pair to a single best-matched address */
    cweeStr         GetAddress(const vec2d& LongLat);

protected:
    cweeSharedPtr< void >                   nominatim; /* Persistent Handle to Nominatum Server. */
    cweeSharedPtr < HRESULT >               hr; /* Error from last internal call. */
};