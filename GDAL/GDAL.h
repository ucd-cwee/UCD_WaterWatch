#pragma once
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/Units.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/cweeTime.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/enum.h"
#include "../WaterWatchCpp/Iterator.h"
#include "../WaterWatchCpp/DelayedInstantiation.h"

#include "../WaterWatchCpp/chaiscript_wrapper.h"
#include "../WaterWatchCpp/WaterWatch_Module_Header.h"
#include "../WaterWatchCpp/Units.h"
#include "../WaterWatchCpp/Geocoding.h"
#include "../WaterWatchCpp/RTree.h"
#include "../WaterWatchCpp/InterpolatedMatrix.h"

class GDAL_Data {
public:
	virtual cweeStr TestGDAL(cweeStr filePath) = 0;
};
extern cweeSharedPtr<GDAL_Data> gdal_data;

namespace chaiscript {
	namespace WaterWatch_Lib {
		[[nodiscard]] ModulePtr GDAL_library();
	};
}; // namespace chaiscript



BETTER_ENUM(FieldType, uint8_t, Integer, IntegerList, Real, RealList, String, StringList, WideString, WideStringList, Binary, Date, Time, DateTime, Integer64, Integer64List, None);
BETTER_ENUM(GeometryType, uint8_t, Unknown, Point, Line, Polygon, Multipoint, Multiline, MultiPolygon);


namespace cweeGeo {
	class Raster;
	class Layer;
	class Feature;
	class Field;
	class Geometry;
	
	class Shapefile {
	protected:
		cweeSharedPtr<void> data;

	public:
		Shapefile() : data(nullptr) {};
		Shapefile(cweeStr const& filePath, bool readOnly = true);
		Shapefile(decltype(Shapefile::data) dataSource) : data(dataSource) {};
		Shapefile(Shapefile const& other) : data(other.data) {};
		Shapefile(Shapefile&& other) : data(std::forward<decltype(Shapefile::data)>(other.data)) {};
		Shapefile& operator=(Shapefile const& other) { data = other.data;  return *this; };
		Shapefile& operator=(Shapefile&& other) { data = other.data; return *this; };

		Raster GetRaster(int RasterId) const;
		int NumRasters() const;
		Layer GetLayer(int LayerId) const;
		Layer GetLayer(cweeStr const& name) const;
		int NumLayers() const;
		cweeStr DriverName() const;
		cweeStr Description() const;
		Layer CreateLayer(cweeStr const& name, GeometryType type);

	};

	class Raster {
	protected:
		cweeSharedPtr<void> data;
		cweeSharedPtr<void> transform;

	public:
		Raster() : data(nullptr), transform(nullptr) {};
		Raster(decltype(Raster::data) const& dataSource, decltype(Raster::transform) const& transformSource) : data(dataSource), transform(transformSource) {};
		Raster(Raster const& other) : data(other.data), transform(other.transform) {};
		Raster(Raster&& other) : data(other.data), transform(other.transform) {};
		Raster& operator=(Raster const& other) {
			data = other.data;
			transform = other.transform;
			return *this;
		};
		Raster& operator=(Raster&& other) {
			data = other.data;
			transform = other.transform;
			return *this;
		};

		int Fid() const;
		int SizeX() const;
		int SizeY() const;
		cweeStr Description() const;
		double Minimum() const;
		double Maximum() const;
		cweeInterpolatedMatrix<float> GetData() const;
	};

	class Layer {
	protected:
		cweeSharedPtr<void> data;
		cweeSharedPtr<void> transform;
		cweeSharedPtr<void> rTree;

	public:
		Layer() : data(nullptr), transform(nullptr), rTree(nullptr) {};
		Layer(decltype(Layer::data) const& dataSource);
		Layer(decltype(Layer::data) const& dataSource, decltype(Layer::transform) const& transformSource) : data(dataSource), transform(transformSource), rTree(nullptr) {};
		Layer(Layer const& other) : data(other.data), transform(other.transform), rTree(other.rTree) {};
		Layer(Layer&& other) : data(other.data), transform(other.transform), rTree(other.rTree) {};
		Layer& operator=(Layer const& other) {
			data = other.data;
			transform = other.transform;
			rTree = other.rTree;
			return *this;
		};
		Layer& operator=(Layer&& other) {
			data = other.data;
			transform = other.transform;
			rTree = other.rTree;
			return *this;
		};

		int NumFeatures() const;
		Feature GetFeature(int Fid) const;
		cweeStr Name() const;
		cweeBoundary Boundary();
		void Rename(cweeStr const& newName);
		// Feature CreateFeature();
		void AddField(cweeStr const& name, FieldType type);



		static cweeList<cweeList<cweePair<Feature, cwee_units::foot_t>>> Near(Layer& layer1, Layer& layer2, std::function<double(Geometry const&, Geometry const&)> DistanceFunction, int numNearest = 1, std::function<bool(Feature const&)> WhereFunction = [](Feature const&)->bool { return true; });
		static cweeList<cweePair<Feature, cwee_units::foot_t>> Near(Feature const& layer1, Layer& layer2, int numNearest = 1, std::function<bool(Feature const&)> WhereFunction = [](Feature const&)->bool { return true; });
		static cweeList<cweePair<Feature, cwee_units::foot_t>> Near(cweeBoundary const& layer1, Layer& layer2, int numNearest = 1, std::function<bool(Feature const&)> WhereFunction = [](Feature const&)->bool { return true; });

	};

	class Feature {
	protected:		
		cweeSharedPtr<void> data;
		Layer layer;

	public:
		Feature() : data(nullptr), layer(nullptr) {};
		Feature(decltype(Feature::data) const& dataSource, decltype(Feature::layer) const& layerSource) : data(dataSource), layer(layerSource) {};
		Feature(Feature const& other) : data(other.data), layer(other.layer) {};
		Feature(Feature&& other) : data(std::forward<decltype(Feature::data)>(other.data)), layer(other.layer) {};
		Feature& operator=(Feature const& other) { data = other.data; layer = other.layer; return *this; };
		Feature& operator=(Feature&& other) { data = other.data; layer = other.layer; return *this; };

		cweeSharedPtr<void> Data() const;
		int Fid() const;
		int NumFields() const;
		const Layer& GetLayer() const;
		Field GetField(int n) const;
		Field GetField(cweeStr const& name) const;
		bool HasField(cweeStr const& name) const;
		Geometry GetGeometry() const;
		/*! Returns the minimum (closest) distance between this and another feature. */
		cwee_units::foot_t Distance(Feature const& obj) const;
		cwee_units::foot_t Length() const;
		cweeStr Geocode() const;
		cwee_units::foot_t Elevation() const;
	};

	//class FeatureDefinition {
	//protected:
	//	cweeSharedPtr<void> data;
	//public:
	//	FeatureDefinition(decltype(FeatureDefinition::data) const& dataSource) : data(dataSource) {};
	//};

	//class FieldDefinition {
	//protected:
	//	cweeSharedPtr<void> data;
	//public:
	//	FieldDefinition(decltype(FieldDefinition::data) const& dataSource) : data(dataSource) {};
	//};

	class Field {
	public:
	protected:
		cweeGeo::Feature Feature;
		int fieldNumber;

	public:
		Field() : Feature(), fieldNumber(-1) {};
		Field(decltype(Field::Feature) const& dataSource, decltype(Field::fieldNumber) const& fieldN) : Feature(dataSource), fieldNumber(fieldN) {};
		Field(Field const& other) : Feature(other.Feature), fieldNumber(other.fieldNumber) {};
		Field(Field&& other) : Feature(other.Feature), fieldNumber(other.fieldNumber) {};
		Field& operator=(Field const& other) { Feature = other.Feature; fieldNumber = other.fieldNumber; return *this; };
		Field& operator=(Field&& other) { Feature = other.Feature; fieldNumber = other.fieldNumber; return *this; };

		const cweeGeo::Feature& GetFeature() const;
		cweeStr Name() const;
		FieldType Type() const;
		bool IsUnset() const;
		bool IsNull() const;
		void Set(nullptr_t);
		void Set(cweeStr const& V);
		void Set(double V);
		void Set(cweeTime const& V);
		chaiscript::Boxed_Value GetBoxed() const;
		cweeAny GetAny() const;
		cweeStr GetAsString() const;
		double GetAsDouble() const;
		cweeTime GetAsDateTime() const;
	};

	class Geometry {
	public:
	protected:
		cweeGeo::Feature Feature;
		cweeSharedPtr<void> geometry;
		
	public:
		Geometry() : Feature(), geometry(nullptr) {};
		Geometry(decltype(Geometry::Feature) const& dataSource, decltype(Geometry::geometry) const& geoSource) : Feature(dataSource), geometry(geoSource) {};
		Geometry(Geometry const& other) : Feature(other.Feature), geometry(other.geometry) {};
		Geometry(Geometry&& other) : Feature(other.Feature), geometry(other.geometry) {};
		Geometry& operator=(Geometry const& other) { Feature = other.Feature; geometry = other.geometry; return *this; };
		Geometry& operator=(Geometry&& other) { Feature = other.Feature; geometry = other.geometry;  return *this; };

		cweeSharedPtr<void> const& Data() const { return geometry; };
		const cweeGeo::Feature& GetFeature() const;
		GeometryType Type() const;
		int NumPoints() const;
		double Longitude(int index) const;
		double Latitude(int index) const;
		vec2d CenterOfMass() const;
		vec2d Coordinates(int index) const;
		cweeList<vec2d> AllCoordinates() const;
		cwee_units::foot_t Length() const;
		cwee_units::foot_t Elevation() const;
		cwee_units::foot_t Distance(Geometry const& obj) const;
		cwee_units::foot_t Distance(cweeBoundary const& obj) const;
		cweeStr Geocode() const;

	public:
		/*! Calculates the geodesic distance between two precise decimal coordinates */
		static cwee_units::foot_t Distance(vec2d const& point1, vec2d const& point2);
		/*! Calculates the closest point on a line to the input coordinate. */
		static vec2d ClosestPoint(vec2d const& pointCoord, cweeList<vec2d> const& lineCoords);
		/*! Calculates the minimum (closest) distance between a point and a series of straight lines. Looks for middle-of-line closeness as well. */
		static cwee_units::foot_t Distance(vec2d const& pointCoord, cweeList<vec2d> const& lineCoords);
		/*! Calculates the minimum (closest) distance between two lines. */
		static cwee_units::foot_t Distance(cweeList<vec2d> const& coords1, cweeList<vec2d> const& coords2);
		/*! Returns the approximate street address of the coordinate, it known. */
		static cweeStr Geocode(vec2d const& point1);
		/*! Returns the approximate elevation of the coordinate, it known. */
		static cwee_units::foot_t Elevation(vec2d const& point1);
		/*! Returns the minimum (closest) distance between two geometries. */
		static cwee_units::foot_t Distance(Geometry const& obj1, Geometry const& obj2);
		/*! Returns the minimum (closest) distance between two geometries. */
		static cwee_units::foot_t Distance(Geometry const& obj1, cweeBoundary const& obj2);
		/*! Returns the length of the geometry */
		static cwee_units::foot_t Length(Geometry const& obj);
		

		
		
	};

};