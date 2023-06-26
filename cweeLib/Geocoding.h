#ifndef	__GEOCODING_H__
#define __GEOCODING_H__

#pragma hdrstop





class cweeGeocoding {
public:
    virtual vec2d       GetLongLat(const cweeStr& address) = 0;
    virtual std::vector<std::map<std::string, cweeStr>> Geocode(const cweeStr& address) = 0;

    virtual cweeStr     GetAddress(const vec2& LongLat) = 0;
    virtual vec2		GetMercatorXY(const vec2& LongLat) = 0;
    virtual vec2		GetLongLat(const vec2& MercatorXY) = 0;

    virtual std::map<std::string, cweeStr> GetAddressComponents(const vec2d& LongLat) = 0;
    virtual cweeStr     GetAddress(const vec2d& LongLat) = 0;
    virtual vec2d		GetMercatorXY(const vec2d& LongLat) = 0;
    virtual vec2d		GetLongLat(const vec2d& MercatorXY) = 0;

    virtual vec2d       GetLongLat(double easting_feet, double northing_feet) = 0;


    virtual double      GetElevation(const vec2d& LongLat) = 0;

    virtual cweeStr     GetWeatherData(const u64& start, const u64& end, const double& longitude, const double& latitude) = 0;

    virtual cweeStr     QueryHttpToFile(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "EdmsApp") = 0;

private:
	virtual	bool		testGeocoding(void) = 0;
};
/*!
geocoding and mapping services
*/
extern cweeGeocoding* geocoding;

#endif // __GEOCODING_H__