#include "../cweeLib/cnominatim.h"
#include "precompiled.h"
#pragma hdrstop

//The main class
class C_USGS
{
public:
    //Constructors / Destructors
    C_USGS() = default;
    C_USGS(const C_USGS&) = delete;
    C_USGS(C_USGS&&) = delete;
    virtual ~C_USGS()
    {
        ReleaseConnection();
        ReleaseSession();
    }

    //Methods
    C_USGS& operator=(const C_USGS&) = delete;
    C_USGS& operator=(C_USGS&&) = delete;

    HRESULT GetElevation(_In_ const vec2d& longLat, _Inout_ double& result) //Runs the search query
    {
        //Call the helper function to do the main work
        const HRESULT hr = _GetElevation(longLat, result);

        //Free up the connection handles before we return
        ReleaseConnection();

        return hr;
    }

    void AttachSession(_In_ HANDLE hSession) noexcept
    {
        m_Session.Attach(hSession);
    }

    HANDLE DetachSession() noexcept
    {
        return m_Session.Detach();
    }

protected:
    //Methods
    HRESULT _GetElevation(_In_ const vec2d& longLat, _Inout_ double& result)
    {
        //reset the result
        result = 0;

        auto y = cweeStr(longLat.y).c_str();
        auto x = cweeStr(longLat.x).c_str();
        constexpr auto units = "Feet";

        HRESULT hr;
        {
            //Create the internet session
            hr = CreateSession("EdmsApp");
            if (FAILED(hr))
                return hr;

            //Create the internet connection
            hr = CreateConnection("nationalmap.gov");
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
                ss << _T("epqs/pqs.php");
                ss << _T("?y=");
                ss << y;
                ss << _T("&x=");
                ss << x;
                ss << _T("&output=xml&units=");
                ss << units;

                sRequest = ss.str();
            }
                     
            //Create and issue the request
            hr = CreateRequest(sRequest.c_str());
            if (FAILED(hr))
                return hr;

            //Read the response
            std::string sFile;
            hr = ReadResponse(sFile);
            if (FAILED(hr))
                return hr;

            //Load up the XML file
            ATL::CComPtr<IXMLDOMDocument2> document;
            hr = LoadResponse(document, sFile.c_str());
            if (FAILED(hr))
                return hr;

            //Pull out the specific nodes from the DOM
            ATL::CComPtr< IXMLDOMNodeList> placeNodes;
            {               
                hr = document->selectNodes(ATL::CComBSTR(L"USGS_Elevation_Point_Query_Service/Elevation_Query/Elevation"), &placeNodes);
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
    }

    virtual HRESULT CreateSession(_In_z_ LPCTSTR pszUserAgent)
    {
        //Create the internet session
        if (m_Session == nullptr)
        {
#pragma warning(suppress: 26477)
            return m_Session.Initialize(ATL::CT2W(pszUserAgent), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        }

        return S_OK;
    }

    virtual HRESULT CreateConnection(_In_z_ LPCTSTR pszNominatimServer)
    {
        //Create the internet connection
#pragma warning(suppress: 26477)
        ATLASSERT(m_Connection == nullptr);

        return m_Connection.Initialize(m_Session, ATL::CT2W(pszNominatimServer), 80);
    }

    virtual HRESULT CreateRequest(_In_z_ LPCTSTR pszRequest)
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
        m_File.m_sFileToDownloadInto = ATL::CT2W(sFileName);

        return m_File.Initialize(m_Connection, ATL::CT2W(pszRequest));
    }

    virtual HRESULT ReadResponse(_Inout_ std::string& sFile)
    {
        //Update the output parameter
#ifdef _UNICODE
        sFile = m_File.m_sFileToDownloadInto;
#else
        sFile = ATL::CW2A(m_File.m_sFileToDownloadInto.c_str());
#endif //#ifdef _UNICODE

        //Do the download
        const HRESULT hr = m_File.SendRequestSync();
        m_File.ReleaseResources();
        return hr;
    }

    virtual HRESULT LoadResponse(_Inout_ ATL::CComPtr<IXMLDOMDocument2>& document, _In_z_ LPCTSTR pszFile)
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
    }

    virtual void ReleaseConnection() noexcept
    {
        m_File.Close();
        m_Connection.Close();
    }

    virtual void ReleaseSession() noexcept
    {
        m_Session.Close();
    }

    //Member variables
    WinHTTPWrappers::CSession m_Session;
    WinHTTPWrappers::CConnection m_Connection;
    WinHTTPWrappers::CSyncDownloader m_File;
};

class C_WINHTTP
{
public:
    //Constructors / Destructors
    C_WINHTTP() = default;
    C_WINHTTP(const C_WINHTTP&) = delete;
    C_WINHTTP(C_WINHTTP&&) = delete;
    virtual ~C_WINHTTP()
    {
        ReleaseConnection();
        ReleaseSession();
    }

    //Methods
    C_WINHTTP& operator=(const C_WINHTTP&) = delete;
    C_WINHTTP& operator=(C_WINHTTP&&) = delete;

    HRESULT Query(_Inout_ cweeStr& result, _In_ const cweeStr& mainAddress = "nationalmap.gov", _In_ const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", _In_ const cweeStr& UniqueAppName = "EdmsApp") //Runs the search query
    {
        //Call the helper function to do the main work
        const HRESULT hr = _Query(result, mainAddress, requestParameters, UniqueAppName);

        //Free up the connection handles before we return
        ReleaseConnection();

        return hr;
    }

protected:
    //Methods

    void AttachSession(_In_ HANDLE hSession) noexcept
    {
        m_Session.Attach(hSession);
    }

    HANDLE DetachSession() noexcept
    {
        return m_Session.Detach();
    }

    HRESULT _Query(_Inout_ cweeStr& result, _In_ const cweeStr& mainAddress, _In_ const cweeStr& requestParameters, _In_ const cweeStr& UniqueAppName)
    {
        //reset the result
        result = 0;

        HRESULT hr;
        {
            //Create the internet session
            hr = CreateSession(UniqueAppName.c_str());
            if (FAILED(hr))
                return hr;

            //Create the internet connection
            hr = CreateConnection(mainAddress.c_str());
            if (FAILED(hr))
                return hr;

            //Next form the request string we will be sending
            std::string sRequest = requestParameters.c_str();

            //Create and issue the request
            hr = CreateRequest(sRequest.c_str());
            if (FAILED(hr))
                return hr;

            //Read the response
            std::string sFile;
            hr = ReadResponse(sFile);
            if (FAILED(hr))
                return hr;

            result = sFile.c_str();
        }

        return hr;
    }

    virtual HRESULT CreateSession(_In_z_ LPCTSTR pszUserAgent)
    {
        //Create the internet session
        if (m_Session == nullptr)
        {
#pragma warning(suppress: 26477)
            return m_Session.Initialize(ATL::CT2W(pszUserAgent), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        }

        return S_OK;
    }

    virtual HRESULT CreateConnection(_In_z_ LPCTSTR pszNominatimServer)
    {
        //Create the internet connection
#pragma warning(suppress: 26477)
        ATLASSERT(m_Connection == nullptr);

        return m_Connection.Initialize(m_Session, ATL::CT2W(pszNominatimServer), 80);
    }

    virtual HRESULT CreateRequest(_In_z_ LPCTSTR pszRequest)
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
        m_File.m_sFileToDownloadInto = ATL::CT2W(sFileName);

        return m_File.Initialize(m_Connection, ATL::CT2W(pszRequest));
    }

    virtual HRESULT ReadResponse(_Inout_ std::string& sFile)
    {
        //Update the output parameter
#ifdef _UNICODE
        sFile = m_File.m_sFileToDownloadInto;
#else
        sFile = ATL::CW2A(m_File.m_sFileToDownloadInto.c_str());
#endif //#ifdef _UNICODE

        //Do the download
        const HRESULT hr = m_File.SendRequestSync();
        m_File.ReleaseResources();
        return hr;
    }

    virtual HRESULT LoadResponse(_Inout_ ATL::CComPtr<IXMLDOMDocument2>& document, _In_z_ LPCTSTR pszFile)
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
    }

    virtual void ReleaseConnection() noexcept
    {
        m_File.Close();
        m_Connection.Close();
    }

    virtual void ReleaseSession() noexcept
    {
        m_Session.Close();
    }

    //Member variables
    WinHTTPWrappers::CSession m_Session;
    WinHTTPWrappers::CConnection m_Connection;
    WinHTTPWrappers::CSyncDownloader m_File;
};

class cweeGeocodingLocal : public cweeGeocoding {
public:

    /*! 
    Initialize nominatim 
    */
    cweeGeocodingLocal() {
        hr = CoInitialize(nullptr);
    };

    /*! 
    Uninitialize nominatim 
    */
    virtual                 ~cweeGeocodingLocal() {
        CoUninitialize();
    };

    /*!
    Query "NationalMap.gov" to identify the elevation (feet) of the longitude/latitude. 
    */
    virtual double          GetElevation(const vec2d& LongLat) {
#if false
        std::string elevation;
        {
            WinHTTPWrappers::CSession m_Session;
            WinHTTPWrappers::CConnection m_Connection;
            WinHTTPWrappers::CSyncDownloader m_File;

            auto y = cweeStr(LongLat.y).c_str();
            auto x = cweeStr(LongLat.x).c_str();
            auto units = "Feet";

            {
                //Create the internet session
                {
                    //Create the internet session
                    if (m_Session == nullptr)
                    {
#pragma warning(suppress: 26477)
                        hr = m_Session.Initialize(ATL::CT2W((LPCTSTR)"EdmsApp"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
                    }
                    else {
                        hr = S_OK;
                    }
                }

                if (!Fail()) {

                    //Create the internet connection
                    {
                        //Create the internet connection
#pragma warning(suppress: 26477)
                        ATLASSERT(m_Connection == nullptr);
                        hr = m_Connection.Initialize(m_Session, ATL::CT2W((LPCTSTR)"nationalmap.gov"), 80);
                    }
                    if (!Fail()) {


                        //Next form the request string we will be sending
                        std::string sRequest;
#ifdef _UNICODE
                        std::wostringstream ss;
#else
                        std::ostringstream ss;
#endif //#ifdef _UNICODE
                        ss << _T("epqs/pqs.php");
                        ss << _T("?y=");
                        ss << y;
                        ss << _T("&x=");
                        ss << x;
                        ss << _T("&output=xml&units=");
                        ss << units;

                        sRequest = ss.str();


                        //Create and issue the request
                        {
                            //Get the temp path
                            ATL::CAtlString sTempPath;
                            const DWORD dwChars = GetTempPath(_MAX_PATH, sTempPath.GetBuffer(_MAX_PATH));
                            sTempPath.ReleaseBuffer();
                            if (dwChars == 0)
                                hr = ATL::AtlHresultFromLastError();
                            else {
                                //Generate a temp file to store the response into
                                ATL::CAtlString sFileName;
                                const UINT nChars = GetTempFileName(sTempPath, _T("CNM"), 0, sFileName.GetBuffer(_MAX_PATH));
                                sFileName.ReleaseBuffer();
                                if (nChars == 0)
                                    hr = ATL::AtlHresultFromLastError();
                                else {
                                    //Save the file to download into
                                    m_File.m_sFileToDownloadInto = ATL::CT2W(sFileName);

                                    hr = m_File.Initialize(m_Connection, ATL::CT2W((LPCTSTR)sRequest.c_str()));
                                }
                            }
                        }

                        if (!Fail()) {

                            //Read the response
                            std::string sFile;
                            {

                                //Update the output parameter
#ifdef _UNICODE
                                sFile = m_File.m_sFileToDownloadInto;
#else
                                sFile = ATL::CW2A(m_File.m_sFileToDownloadInto.c_str());
#endif //#ifdef _UNICODE

                                //Do the download
                                hr = m_File.SendRequestSync();
                                m_File.ReleaseResources();
                            }

                            if (!Fail()) {

                                //Load up the XML
                                ATL::CComPtr<IXMLDOMDocument2> document;
                                {
                                    //First create a DOM document
                                    HRESULT hr = document.CoCreateInstance(__uuidof(DOMDocument60));
                                    if (!Fail()) {
                                        //Now load up the XML from disk
                                        hr = document->put_async(VARIANT_FALSE);
                                        if (!Fail()) {
                                            VARIANT_BOOL vbSuccess = VARIANT_FALSE;
                                            hr = document->load(ATL::CComVariant((LPCTSTR)sFile.c_str()), &vbSuccess);
                                            if (!Fail())
                                            {
                                                //Delete the temp file now that we are finished with it
                                                DeleteFile((LPCTSTR)sFile.c_str());

                                                //Check we could load the xml response ok
                                                if (vbSuccess == VARIANT_FALSE)
                                                    hr = ATL::AtlHresultFromWin32(ERROR_FILE_CORRUPT);
                                                else
                                                    hr = S_OK;
                                            }
                                        }
                                    }
                                }

                                if (!Fail()) {
                                    ATL::CComPtr< IXMLDOMNodeList> placeNodes;

                                    hr = document->selectNodes(ATL::CComBSTR(L"USGS_Elevation_Point_Query_Service/Elevation_Query/Elevation"), &placeNodes);
                                    if (!Fail()) {


                                        if (placeNodes)
                                        {
                                            //Iterate across all the places found
                                            long nPlaces = 0;
                                            hr = placeNodes->get_length(&nPlaces);
                                            if (!Fail()) {

                                                for (long i = 0; i < nPlaces; i++)
                                                {
                                                    //Pull out the current place
                                                    ATL::CComPtr< IXMLDOMNode> placeNode;
                                                    hr = placeNodes->get_item(i, &placeNode);
                                                    if (!Fail() && placeNode) {
                                                        elevation = "";

                                                        ATL::CComBSTR bstrText;
                                                        hr = placeNode->get_text(&bstrText);
                                                        if (!Fail()) {
#ifdef _UNICODE
                                                            elevation = std::string(bstrText);
#else
                                                            elevation = std::string(ATL::CW2A(bstrText));
#endif
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        hr = document->get_childNodes(&placeNodes);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            //Free up the connection handles before we return
            m_File.Close();
            m_Connection.Close();
            m_Session.Close();
        }
        return (double)cweeStr(elevation.c_str());

#else
        double out;
        usgs.Lock();
        hr = usgs.UnsafeRead()->GetElevation(LongLat, out);
        usgs.Unlock();
        return out;
#endif
    };

    /*!
    Query "Open Street Map" to identify Longitude and Latitude (X,Y of return) of best-fit to address.
    */
    virtual vec2d           GetLongLat(const cweeStr& address) {
        vec2d out(-1.0f, -1.0f);
        {

            nominatim.Lock();
            Nominatim::CSearchQuery query; {
                query.m_sQueryString = _T(address.c_str());
                query.m_nLimit = -1;
                query.m_sUserAgent = _T("EDMSApp");
                query.m_Polygon = Nominatim::PolygonType::Old_Polygon;
                query.m_bDedupe = false;
            }
            std::vector<Nominatim::CSearchPlace> result;
            hr = nominatim.UnsafeRead()->Search(query, result);
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
            nominatim.Unlock();
            
        }
        return out;        
    };

    virtual vec2d           GetLongLat(double easting_feet, double northing_feet) {
        return cweeEng::CoordinateConversion_FeetToLongLat(vec2d(easting_feet, northing_feet));
    }; 

    virtual std::vector<std::map<std::string, cweeStr>> Geocode(const cweeStr& address) {
        std::vector<std::map<std::string, cweeStr>> out;

        {

            nominatim.Lock();
            Nominatim::CSearchQuery query; {
                query.m_sQueryString = _T(address.c_str());
                query.m_nLimit = -1;
                query.m_sUserAgent = _T("EDMSApp");
                query.m_Polygon = Nominatim::PolygonType::Old_Polygon;
                query.m_bDedupe = false;
            }
            std::vector<Nominatim::CSearchPlace> result;
            hr = nominatim.UnsafeRead()->Search(query, result);
            nominatim.Unlock();

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

    virtual std::map<std::string, cweeStr> GetAddressComponents(const vec2d& LongLat) {
        std::map<std::string, cweeStr> out;
        {
            nominatim.Lock();
            Nominatim::CReverseGeocodingQuery query; {
                query.m_fLongitude = LongLat.x;
                query.m_fLatitude = LongLat.y;
                query.m_sUserAgent = _T("EDMSApp");
                query.m_bExtraTags = true;
                query.m_bNameDetails = true;
            }
            Nominatim::CReverseGeocodingQueryResult result;
            hr = nominatim.UnsafeRead()->ReverseGeocoding(query, result);
            nominatim.Unlock();

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

    /*!
    Query "Open Street Map" to identify address that is best-fit to Longitude and Latitude (X,Y of input).
    */
    virtual cweeStr         GetAddress(const vec2& LongLat) {
        cweeStr out;
        {
            nominatim.Lock();
            Nominatim::CReverseGeocodingQuery query; {
                query.m_fLongitude = LongLat.x;
                query.m_fLatitude = LongLat.y;
                query.m_sUserAgent = _T("EDMSApp");
                query.m_bExtraTags = true;
                query.m_bNameDetails = true;
            }
            Nominatim::CReverseGeocodingQueryResult result;
            hr = nominatim.UnsafeRead()->ReverseGeocoding(query, result);
            out = result.m_sDisplayName.c_str();
            nominatim.Unlock();
            
        }
        return out;
    };	
    /*!
    Query "Open Street Map" to identify address that is best-fit to Longitude and Latitude (X,Y of input).
    */
    virtual cweeStr         GetAddress(const vec2d& LongLat) {
        cweeStr out;
        {
            nominatim.Lock();
            Nominatim::CReverseGeocodingQuery query; {
                query.m_fLongitude = LongLat.x;
                query.m_fLatitude = LongLat.y;
                query.m_sUserAgent = _T("EDMSApp");
                query.m_bExtraTags = true;
                query.m_bNameDetails = true;
            }
            Nominatim::CReverseGeocodingQueryResult result;
            hr = nominatim.UnsafeRead()->ReverseGeocoding(query, result);
            out = result.m_sDisplayName.c_str();
            nominatim.Unlock();            
        }
        return out;
    };

    /*!
    Calculate the mercator X and Y (X,Y of return, in meters) from the supplied Longitude and Latitude (X,Y of input).
    */
    virtual vec2		    GetMercatorXY(const vec2& LongLat) {
        constexpr auto EARTH_RADIUS = 6378137.0f; // meters
        return vec2(
            DEG2RAD(LongLat.x) * EARTH_RADIUS,
            log(tan(DEG2RAD(LongLat.y) / 2.0f + cweeMath::PI / 4.0f)) * EARTH_RADIUS
        );
    };
    /*!
    Calculate the mercator X and Y (X,Y of return, in meters) from the supplied Longitude and Latitude (X,Y of input).
    */
    virtual vec2d		    GetMercatorXY(const vec2d& LongLat) {
        constexpr auto EARTH_RADIUS = 6378137.0; // meters
        return vec2d(
            DEG2RAD(LongLat.x) * EARTH_RADIUS,
            log(tan(DEG2RAD(LongLat.y) / 2.0 + (double)cweeMath::PI / 4.0)) * EARTH_RADIUS
        );
    };

    /*!
    Calculate the Longitude and Latitude (X,Y of return) from the supplied mercator X and Y (X,Y of input, in meters).
    */
    virtual vec2		    GetLongLat(const vec2& MercatorXY) {
        constexpr auto EARTH_RADIUS = 6378137.0f;
        return vec2(
            RAD2DEG(MercatorXY.x / EARTH_RADIUS),
            RAD2DEG(2.0f * ::atan(::exp(MercatorXY.y / EARTH_RADIUS)) - cweeMath::PI / 2.0f)
        );
    };
    /*!
    Calculate the Longitude and Latitude (X,Y of return) from the supplied mercator X and Y (X,Y of input, in meters).
    */
    virtual vec2d		    GetLongLat(const vec2d& MercatorXY) {
        constexpr auto EARTH_RADIUS = 6378137.0;
        return vec2d(
            RAD2DEG(MercatorXY.x / EARTH_RADIUS),
            RAD2DEG(2.0 * ::atan(::exp(MercatorXY.y / EARTH_RADIUS)) - (double)cweeMath::PI / 2.0)
        );        
    };

    virtual cweeStr         GetWeatherData(const u64& start, const u64& end, const double& longitude, const double& latitude) {       
        return "";
    };

    virtual cweeStr         QueryHttpToFile(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "EdmsApp") {
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

        return out;
    };


private:

    virtual	bool		    testGeocoding(void) {
        //Do a search query using the example provided at http://wiki.openstreetmap.org/wiki/Nominatim
        Nominatim::CSearchQuery query;
#pragma warning(suppress: 26489)
        query.m_sQueryString = _T("135 pilkington avenue, birmingham");
        query.m_nLimit = 50;
#pragma warning(suppress: 26489)
        query.m_sUserAgent = _T("NominatimTestApp");
        query.m_Polygon = Nominatim::PolygonType::Old_Polygon;
        query.m_bDedupe = false;
        Nominatim::CNominatim nominatim;
        std::vector<Nominatim::CSearchPlace> result;
#pragma warning(suppress: 26486)
        hr = nominatim.Search(query, result);
#pragma warning(suppress: 26477)

        const auto nSize = result.size();

        auto var = GetLongLat(query.m_sQueryString.c_str());
        

#pragma warning(suppress: 26477)
        ATLVERIFY(nSize); //NOLINT(clang-diagnostic-unused-value)

        //Do reverse geocoding query using the details returned from the search above
        Nominatim::CReverseGeocodingQuery query2;
#pragma warning(suppress: 26446)
        query2.m_fLatitude = result[0].m_fLatitude;
#pragma warning(suppress: 26446)
        query2.m_fLongitude = result[0].m_fLongitude;
#pragma warning(suppress: 26489)
        query2.m_sUserAgent = _T("NominatimTestApp");
        query2.m_bExtraTags = true;
        query2.m_bNameDetails = true;
        Nominatim::CReverseGeocodingQueryResult result2;
#pragma warning(suppress: 26486)
        hr = nominatim.ReverseGeocoding(query2, result2);

        auto var2 = GetAddress(vec2(result[0].m_fLatitude, result[0].m_fLongitude));



#pragma warning(suppress: 26477)
        ATLVERIFY(!Fail());

        //Do the address lookup query using the example provided at http://wiki.openstreetmap.org/wiki/Nominatim
        Nominatim::CAddressLookupQuery query3;
#pragma warning(suppress: 26489)
        query3.m_sOSMIds = _T("R146656,W104393803,N240109189");
        query3.m_bExtraTags = true;
        query3.m_bNameDetails = true;
#pragma warning(suppress: 26489)
        query3.m_sUserAgent = _T("NominatimTestApp");
        std::vector<Nominatim::CLookupPlace> result3;
#pragma warning(suppress: 26486)
        hr = nominatim.AddressLookup(query3, result3);
#pragma warning(suppress: 26477)

        //Cleanup COM

        return true;
    };
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
    bool                    Fail() {        
        hr.Lock();
        if (*hr.UnsafeRead() < 0) {
            hr.Unlock();
            return true;
        }
        else {
            hr.Unlock();
            return false;
        }        
    }
    cweeUnpooledInterlocked < HRESULT >             hr;
    cweeUnpooledInterlocked< Nominatim::CNominatim >  nominatim;
    cweeUnpooledInterlocked< C_USGS >                 usgs;
};

/*!
geocoding and mapping services, should not be accessible outside of the cweeLib code.
*/
cweeGeocodingLocal	geocodingLocal;
/*!
geocoding and mapping services
*/
cweeGeocoding* geocoding = &geocodingLocal;