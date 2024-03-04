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
#include "framework.h"
#include "../WaterWatchCpp/Units.h"
#include "../WaterWatchCpp/cNominitum.h"
#include "cNominitum.h"
#include "../WaterWatchCpp/InterlockedValues.h"
#include "../WaterWatchCpp/cweeInterlocked.h"
// #include "../WaterWatchCpp/DispatchTimer.h"
#include "../WaterWatchCpp/cweeThreadedMap.h"
#include "../WaterWatchCpp/Mutex.h"
#include "../WaterWatchCpp/Engineering.h"

/* cweeSharedPtr<void> converter */
INLINE WinHTTPWrappers::CSession& Session(cweeSharedPtr<void> p) { return *static_cast<WinHTTPWrappers::CSession*>(p.Get()); };
/* cweeSharedPtr<void> converter */
INLINE WinHTTPWrappers::CConnection& Connection(cweeSharedPtr<void> p) { return *static_cast<WinHTTPWrappers::CConnection*>(p.Get()); };
/* cweeSharedPtr<void> converter */
INLINE WinHTTPWrappers::CSyncDownloader& File(cweeSharedPtr<void> p) { return *static_cast<WinHTTPWrappers::CSyncDownloader*>(p.Get()); };

INLINE HRESULT CreateSession(_In_z_ LPCTSTR pszUserAgent, cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File)
{
    //Create the internet session
    if (Session(m_Session) == nullptr)
    {
#pragma warning(suppress: 26477)
        return Session(m_Session).Initialize(ATL::CT2W(pszUserAgent), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    }

    return S_OK;
};
INLINE HRESULT CreateConnection(_In_z_ LPCTSTR pszNominatimServer, cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File)
{
    //Create the internet connection
#pragma warning(suppress: 26477)
    ATLASSERT(Connection(m_Connection) == nullptr);

    return Connection(m_Connection).Initialize(Session(m_Session), ATL::CT2W(pszNominatimServer), 80);
};
INLINE HRESULT CreateRequest(_In_z_ LPCTSTR pszRequest, cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File)
{
    //Get the temp path
    ATL::CAtlString sTempPath;
    const DWORD dwChars = GetTempPath(_MAX_PATH, sTempPath.GetBuffer(_MAX_PATH));
    sTempPath.ReleaseBuffer();
    if (dwChars == 0)
        return ATL::AtlHresultFromLastError();

    //Generate a temp file to store the response into
    ATL::CAtlString sFileName;
    const UINT nChars = GetTempFileName(sTempPath, _T("CNM"), 0, sFileName.GetBuffer(_MAX_PATH));
    sFileName.ReleaseBuffer();
    if (nChars == 0)
        return ATL::AtlHresultFromLastError();

    //Save the file to download into
    File(m_File).m_sFileToDownloadInto = ATL::CT2W(sFileName);

    return File(m_File).Initialize(Connection(m_Connection), ATL::CT2W(pszRequest));
};
INLINE HRESULT ReadResponse(_Inout_ std::string& sFile, cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File)
{
    //Update the output parameter
#ifdef _UNICODE
    sFile = File(m_File).m_sFileToDownloadInto;
#else
    sFile = ATL::CW2A(File(m_File).m_sFileToDownloadInto.c_str());
#endif //#ifdef _UNICODE

    //Do the download
    const HRESULT hr = File(m_File).SendRequestSync();
    File(m_File).ReleaseResources();
    return hr;
};
INLINE HRESULT LoadResponse(_Inout_ ATL::CComPtr<IXMLDOMDocument2>& document, _In_z_ LPCTSTR pszFile, cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File)
{
    //First create a DOM document
    HRESULT hr = document.CoCreateInstance(__uuidof(DOMDocument60));
    if (FAILED(hr))
        return hr;

    //Now load up the XML from disk
    hr = document->put_async(VARIANT_FALSE);
    if (FAILED(hr))
        return hr;
    VARIANT_BOOL vbSuccess = VARIANT_FALSE;
    hr = document->load(ATL::CComVariant(pszFile), &vbSuccess);
    if (FAILED(hr))
        return hr;

    //Delete the temp file now that we are finished with it
    DeleteFile(pszFile);

    //Check we could load the xml response ok
    if (vbSuccess == VARIANT_FALSE)
        return ATL::AtlHresultFromWin32(ERROR_FILE_CORRUPT);

    return S_OK;
};
INLINE HRESULT _GetElevation(_In_ const vec2d& longLat, _Inout_ double& result, cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File)
{
    //reset the result
    result = 0;

    auto y = cweeStr(longLat.y).c_str();
    auto x = cweeStr(longLat.x).c_str();
    constexpr auto units = "Feet";

    HRESULT hr;
    {
        //Create the internet session
        hr = CreateSession((LPCTSTR)std::string("EdmsApp").c_str(), m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Create the internet connection
        hr = CreateConnection((LPCTSTR)std::string("epqs.nationalmap.gov").c_str(), m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Next form the request string we will be sending
        std::string sRequest;
        {
#ifdef _UNICODE
            std::wostringstream ss;
#else
            std::ostringstream ss;
#endif //#ifdef _UNICODE
            ss << _T("v1/xml");
            ss << _T("?x=");
            ss << x;
            ss << _T("&y=");
            ss << y;
            ss << _T("&units=");
            ss << units;

            sRequest = std::string(ss.str().c_str());
        }

        //Create and issue the request
        hr = CreateRequest(sRequest.c_str(), m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Read the response
        std::string sFile;
        hr = ReadResponse(sFile, m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Load up the XML file
        ATL::CComPtr<IXMLDOMDocument2> document;
        hr = LoadResponse(document, sFile.c_str(), m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Pull out the specific nodes from the DOM
        ATL::CComPtr< IXMLDOMNodeList> placeNodes;
        {
            hr = document->selectNodes(ATL::CComBSTR(L"result/value"), &placeNodes); // USGS_Elevation_Point_Query_Service/Elevation_Query/Elevation
            if (FAILED(hr)) {
                return hr;
            }
        }

        // review the pulled nodes      
        cweeStr out;
        if (placeNodes)
        {
            //Iterate across all the places found
            long nPlaces = 0;
            hr = placeNodes->get_length(&nPlaces);
            if (FAILED(hr)) {
                return hr;
            }

            for (long i = 0; i < nPlaces; i++)
            {
                //Pull out the current place
                ATL::CComPtr< IXMLDOMNode> placeNode;
                hr = placeNodes->get_item(i, &placeNode);
                if (!FAILED(hr) && placeNode) {
                    ATL::CComBSTR bstrText;
                    hr = placeNode->get_text(&bstrText);
                    if (!FAILED(hr)) {
#ifdef _UNICODE
                        out = cweeStr(std::string(bstrText).c_str());
#else
                        out = cweeStr(std::string(ATL::CW2A(bstrText)).c_str());
#endif
                        break;
                    }
                }
            }
        }

        result = (double)out;
    }

    return hr;
};
INLINE void ReleaseConnection(cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File) noexcept
{
    File(m_File).Close();
    Connection(m_Connection).Close();
};
INLINE void ReleaseSession(cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File) noexcept
{
    Session(m_Session).Close();
};

C_USGS::C_USGS() :
    m_Session(cweeSharedPtr<void>(make_cwee_shared< WinHTTPWrappers::CSession>(new WinHTTPWrappers::CSession()), [](void* p) { return p; }))
    , m_Connection(cweeSharedPtr<void>(make_cwee_shared< WinHTTPWrappers::CConnection>(new WinHTTPWrappers::CConnection()), [](void* p) { return p; }))
    , m_File(cweeSharedPtr<void>(make_cwee_shared< WinHTTPWrappers::CSyncDownloader>(new WinHTTPWrappers::CSyncDownloader()), [](void* p) { return p; }))
{};
C_USGS::~C_USGS() {
    ReleaseConnection(m_Session, m_Connection, m_File);
    ReleaseSession(m_Session, m_Connection, m_File);
};
HRESULT C_USGS::GetElevation(_In_ const vec2d& longLat, _Inout_ double& result) //Runs the search query
{
    //Call the helper function to do the main work
    const HRESULT hr = _GetElevation(longLat, result, m_Session, m_Connection, m_File);

    //Free up the connection handles before we return
    ReleaseConnection(m_Session, m_Connection, m_File);

    return hr;
};
void  C_USGS::AttachSession(_In_ HANDLE hSession) noexcept
{
    Session(m_Session).Attach(hSession);
};
HANDLE  C_USGS::DetachSession() noexcept
{
    return Session(m_Session).Detach();
};

INLINE HRESULT _Query(_Inout_ cweeStr& result, _In_ const cweeStr& mainAddress, _In_ const cweeStr& requestParameters, _In_ const cweeStr& UniqueAppName, cweeSharedPtr<void> m_Session, cweeSharedPtr<void> m_Connection, cweeSharedPtr<void> m_File)
{
    //reset the result
    result = 0;

    HRESULT hr;
    {
        //Create the internet session
        hr = CreateSession(UniqueAppName.c_str(), m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Create the internet connection
        hr = CreateConnection(mainAddress.c_str(), m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Next form the request string we will be sending
        std::string sRequest = requestParameters.c_str();

        //Create and issue the request
        hr = CreateRequest(sRequest.c_str(), m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        //Read the response
        std::string sFile;
        hr = ReadResponse(sFile, m_Session, m_Connection, m_File);
        if (FAILED(hr))
            return hr;

        result = sFile.c_str();
    }

    return hr;
};

C_WINHTTP::C_WINHTTP() :
    m_Session(cweeSharedPtr<void>(make_cwee_shared< WinHTTPWrappers::CSession>(new WinHTTPWrappers::CSession()), [](void* p) { return p; }))
    , m_Connection(cweeSharedPtr<void>(make_cwee_shared< WinHTTPWrappers::CConnection>(new WinHTTPWrappers::CConnection()), [](void* p) { return p; }))
    , m_File(cweeSharedPtr<void>(make_cwee_shared< WinHTTPWrappers::CSyncDownloader>(new WinHTTPWrappers::CSyncDownloader()), [](void* p) { return p; }))
{};
C_WINHTTP::~C_WINHTTP()
{
    ReleaseConnection(m_Session, m_Connection, m_File);
    ReleaseSession(m_Session, m_Connection, m_File);
};
HRESULT C_WINHTTP::Query(_Inout_ cweeStr& result, _In_ const cweeStr& mainAddress, _In_ const cweeStr& requestParameters, _In_ const cweeStr& UniqueAppName) //Runs the search query
{
    //Call the helper function to do the main work
    const HRESULT hr = _Query(result, mainAddress, requestParameters, UniqueAppName, m_Session, m_Connection, m_File);

    //Free up the connection handles before we return
    ReleaseConnection(m_Session, m_Connection, m_File);

    return hr;
};
void C_WINHTTP::AttachSession(_In_ HANDLE hSession) noexcept
{
    Session(m_Session).Attach(hSession);
};
HANDLE C_WINHTTP::DetachSession() noexcept
{
    return Session(m_Session).Detach();
};

INLINE cweeSharedPtr<Nominatim::CNominatim>& NominatimPtr(cweeSharedPtr<void> p) { return *static_cast<cweeSharedPtr<Nominatim::CNominatim>*>(p.Get()); };

C_GeocodingSupport::C_GeocodingSupport() :
        nominatim(cweeSharedPtr<void>(make_cwee_shared< cweeSharedPtr<Nominatim::CNominatim>>(new cweeSharedPtr < Nominatim::CNominatim>(new Nominatim::CNominatim())), [](void* p) { return p; })),
        hr(CoInitialize(nullptr))
    {};
C_GeocodingSupport::~C_GeocodingSupport() {
    CoUninitialize();
};
vec2d           C_GeocodingSupport::GetLongLat(const cweeStr& address) {
    vec2d out(-1.0f, -1.0f);
    {

        AUTO g = NominatimPtr(nominatim).Guard();
        Nominatim::CSearchQuery query; {
            query.m_sQueryString = _T(address.c_str());
            query.m_nLimit = -1;
            query.m_sUserAgent = _T("EDMSApp");
            query.m_Polygon = Nominatim::PolygonType::Old_Polygon;
            query.m_bDedupe = false;
        }
        std::vector<Nominatim::CSearchPlace> result;
        hr = NominatimPtr(nominatim).UnsafeGet()->Search(query, result);
        // retrieves a collection of potential addresses. 
        std::vector<cweeStr> displayNames;
        for (auto& x : result) {
            displayNames.push_back(x.m_sDisplayName.c_str());
        }
        cweeStr bestMatch = address.BestMatch(displayNames);
        for (auto& x : result) {
            out.x = (double)x.m_fLongitude;
            out.y = (double)x.m_fLatitude;
            if (x.m_sDisplayName.c_str() == bestMatch) {
                break;
            }
        }

    }
    return out;
};
std::vector<std::map<std::string, cweeStr>> C_GeocodingSupport::Geocode(const cweeStr& address) {
    std::vector<std::map<std::string, cweeStr>> out;

    {
        AUTO g = NominatimPtr(nominatim).Guard();
        Nominatim::CSearchQuery query; {
            query.m_sQueryString = _T(address.c_str());
            query.m_nLimit = -1;
            query.m_sUserAgent = _T("EDMSApp");
            query.m_Polygon = Nominatim::PolygonType::Old_Polygon;
            query.m_bDedupe = false;
        }
        std::vector<Nominatim::CSearchPlace> result;
        hr = NominatimPtr(nominatim).UnsafeGet()->Search(query, result);

        for (auto& place : result) {
            std::map<std::string, cweeStr> out2;

            out2["Latitude"] = cweeStr(place.m_fLatitude);
            out2["Longitude"] = cweeStr(place.m_fLongitude);
            out2["City"] = place.m_sCity.c_str();
            out2["Country"] = place.m_sCountry.c_str();
            out2["County"] = place.m_sCounty.c_str();
            out2["Display Name"] = place.m_sDisplayName.c_str();
            out2["House Number"] = place.m_sHouseNumber.c_str();
            out2["Post Code"] = place.m_sPostcode.c_str();
            out2["Road"] = place.m_sRoad.c_str();
            out2["Town"] = place.m_sTown.c_str();
            out2["Village"] = place.m_sVillage.c_str();

            out.push_back(out2);
        }
    }

    return out;
};
std::map<std::string, cweeStr> C_GeocodingSupport::GetAddressComponents(const vec2d& LongLat) {
    std::map<std::string, cweeStr> out;
    {
        AUTO g = NominatimPtr(nominatim).Guard();
        Nominatim::CReverseGeocodingQuery query; {
            query.m_fLongitude = LongLat.x;
            query.m_fLatitude = LongLat.y;
            query.m_sUserAgent = _T("EDMSApp");
            query.m_bExtraTags = true;
            query.m_bNameDetails = true;
        }
        Nominatim::CReverseGeocodingQueryResult result;
        hr = NominatimPtr(nominatim).UnsafeGet()->ReverseGeocoding(query, result);

        /*
            String m_sPlaceId;
            OSMType m_OSMType;
            String m_sOSMId;
            String m_sDisplayName;
            String m_sHouseNumber;
            String m_sRoad;
            String m_sVillage;
            String m_sTown;
            String m_sCity;
            String m_sCounty;
            String m_sPostcode;
            String m_sCountry;
            String m_sCountryCode;
        */

        out["City"] = result.m_sCity.c_str();
        out["Country"] = result.m_sCountry.c_str();
        out["County"] = result.m_sCounty.c_str();
        out["House Number"] = result.m_sHouseNumber.c_str();
        out["Post Code"] = result.m_sPostcode.c_str();
        out["Road"] = result.m_sRoad.c_str();
        out["Town"] = result.m_sTown.c_str();
        out["Village"] = result.m_sVillage.c_str();
    }
    return out;
};  
cweeStr         C_GeocodingSupport::GetAddress(const vec2& LongLat) {
    cweeStr out;
    {
        AUTO g = NominatimPtr(nominatim).Guard();
        Nominatim::CReverseGeocodingQuery query; {
            query.m_fLongitude = LongLat.x;
            query.m_fLatitude = LongLat.y;
            query.m_sUserAgent = _T("EDMSApp");
            query.m_bExtraTags = true;
            query.m_bNameDetails = true;
        }
        Nominatim::CReverseGeocodingQueryResult result;
        hr = NominatimPtr(nominatim).UnsafeGet()->ReverseGeocoding(query, result);
        out = result.m_sDisplayName.c_str();

    }
    return out;
};  
cweeStr         C_GeocodingSupport::GetAddress(const vec2d& LongLat) {
    cweeStr out;
    {
        AUTO g = NominatimPtr(nominatim).Guard();
        Nominatim::CReverseGeocodingQuery query; {
            query.m_fLongitude = LongLat.x;
            query.m_fLatitude = LongLat.y;
            query.m_sUserAgent = _T("EDMSApp");
            query.m_bExtraTags = true;
            query.m_bNameDetails = true;
        }
        Nominatim::CReverseGeocodingQueryResult result;
        hr = NominatimPtr(nominatim).UnsafeGet()->ReverseGeocoding(query, result);
        out = result.m_sDisplayName.c_str();
    }
    return out;
};