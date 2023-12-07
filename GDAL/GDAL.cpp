#pragma once
#include "GDAL.h"
#include "include/gdal.h"
#include "include/gdal_priv.h"
#include "include/ogrsf_frmts.h"
#include "include/ogr_geos.h"
#include "../WaterWatchCpp/WaterWatch_Module_Header.h"
#include "../WaterWatchCpp/BalancedPattern.h"
#include "../WaterWatchCpp/Voronoi.h"

template <typename T> INLINE cweeSharedPtr<void> ToVoidPtr(T& ptr) {
	return cweeSharedPtr<void>(make_cwee_shared<T>(ptr), [](void* p) { return p; });
};
template <typename T> INLINE cweeSharedPtr<T> FromVoidPtr(cweeSharedPtr<void> data) {
	return cweeSharedPtr<T>(data, [](void* p) { return static_cast<T*>(p); });
};


namespace cweeGeo {
	Shapefile::Shapefile(cweeStr const& filePath) : data(nullptr) {
		auto* proj_ptr = (GDALDataset*)GDALOpenEx(
			filePath,
			GDAL_OF_READONLY, // GDAL_OF_ALL, // GDAL_OF_VECTOR
			NULL, NULL, NULL);

		if (proj_ptr) {
			data = cweeSharedPtr<void>(
					cweeSharedPtr< GDALDataset >(
						proj_ptr,
						[](GDALDataset* ptr) { GDALClose((GDALDatasetH)ptr); }
					)
				);
		}
	};
	Raster Shapefile::GetRaster(int RasterId) const {
		AUTO shapefile{ FromVoidPtr<GDALDataset>(data) };
		if (shapefile) {
			auto* rasterPtr = shapefile->GetRasterBand(RasterId);
			if (rasterPtr) {
				auto layerP{ cweeSharedPtr< GDALRasterBand >(rasterPtr, [shapefile](GDALRasterBand* p) { /* do nothing */ }) };



				// shapefile->GetGeoTransform();

				// auto transformPtr = 

				return cweeGeo::Raster(cweeSharedPtr<void>(layerP), nullptr);
			}
		}
		return Raster();
	};
	int Shapefile::NumRasters() const {
		AUTO shapefile{ FromVoidPtr<GDALDataset>(data) };
		if (shapefile) {
			return shapefile->GetRasterCount();
		}
		return 0;
	};
	Layer Shapefile::GetLayer(int LayerId) const {
		AUTO shapefile{ FromVoidPtr<GDALDataset>(data) };
		if (shapefile) {
			auto layerP{ cweeSharedPtr< OGRLayer >(shapefile->GetLayer(LayerId), [shapefile](OGRLayer* p) { /* do nothing */ }) };
			return cweeGeo::Layer(cweeSharedPtr<void>(layerP));
		}
		return Layer();
	};
	Layer Shapefile::GetLayer(cweeStr const& name) const {
		AUTO shapefile{ FromVoidPtr<GDALDataset>(data) };
		if (shapefile) {
			auto layerP{ cweeSharedPtr< OGRLayer >(shapefile->GetLayerByName(name), [shapefile](OGRLayer* p) { /* do nothing */ }) };
			return cweeGeo::Layer(cweeSharedPtr<void>(layerP));
		}
		return Layer();
	};
	int Shapefile::NumLayers() const {
		AUTO shapefile{ FromVoidPtr<GDALDataset>(data) };
		if (shapefile) {
			return shapefile->GetLayerCount();
		}
		return 0;
	};
	cweeStr Shapefile::DriverName() const {
		AUTO shapefile{ FromVoidPtr<GDALDataset>(data) };
		if (shapefile) {
			return shapefile->GetDriverName();
			return shapefile->GetDescription();
		}
		return "";
	};
	cweeStr Shapefile::Description() const {
		AUTO shapefile{ FromVoidPtr<GDALDataset>(data) };
		if (shapefile) {
			return shapefile->GetDescription();
		}
		return "";
	};

	Layer::Layer(decltype(Layer::data) const& dataSource) : data(dataSource), transform(nullptr) {
		AUTO layerP{ FromVoidPtr<OGRLayer>(data) };
		if (layerP) {
			AUTO WGS84SRS = cweeSharedPtr<OGRSpatialReference>(OGRSpatialReference::GetWGS84SRS(), [](OGRSpatialReference* p) {});
			auto sourceSR{ cweeSharedPtr< OGRSpatialReference >(layerP->GetSpatialRef(), [layerP, WGS84SRS](OGRSpatialReference* p) { /* do nothing */ }) };
			auto* transformPtr = OGRCreateCoordinateTransformation(sourceSR.get(), WGS84SRS.get());
			if (transformPtr) {
				this->transform = cweeSharedPtr<void>(cweeSharedPtr< OGRCoordinateTransformation >(transformPtr, [layerP, sourceSR](OGRCoordinateTransformation* p) { delete p; }));
			}
		}
	};
	int Layer::NumFeatures() const {
		AUTO ptr{ FromVoidPtr<OGRLayer>(data) };
		if (ptr) {
			return ptr->GetFeatureCount();
		}
		return 0;
	};
	Feature Layer::GetFeature(int Fid) const {
		AUTO ptr{ FromVoidPtr<OGRLayer>(data) };
		if (ptr && Fid < ptr->GetFeatureCount()) {
			auto* pFeatureActual = ptr->GetFeature(Fid);
			if (pFeatureActual) {
				auto pFeature{ cweeSharedPtr< OGRFeature >(pFeatureActual, [ptr](OGRFeature* p) { if (p) { delete p; } }) };
				if (pFeature) {
					auto poGeometry{ cweeSharedPtr< OGRGeometry >(pFeature->GetGeometryRef(), [pFeature](OGRGeometry* p) { /* do nothing */ }) };
					if (this->transform) {
						AUTO Transform{ FromVoidPtr<OGRCoordinateTransformation>(this->transform) };
						poGeometry->transform(Transform.get());
					}
				}
				return cweeGeo::Feature(cweeSharedPtr<void>(pFeature), *this);
			}
		}
		return cweeGeo::Feature(nullptr, *this);
	};
	cweeStr Layer::Name() const {
		AUTO ptr{ FromVoidPtr<OGRLayer>(data) };
		if (ptr) return ptr->GetName();
		else return "";
	};

	int Raster::Fid() const {
		AUTO ptr{ FromVoidPtr<GDALRasterBand>(data) };
		if (ptr) {
			return ptr->GetBand();
		}
		return -1;
	};
	int Raster::SizeX() const {
		AUTO ptr{ FromVoidPtr<GDALRasterBand>(data) };
		if (ptr) {
			
			return ptr->GetXSize();
		}
		return 0;
	};
	int Raster::SizeY() const {
		AUTO ptr{ FromVoidPtr<GDALRasterBand>(data) };
		if (ptr) {
			return ptr->GetYSize();
		}
		return 0;
	};
	cweeStr Raster::Description() const {
		AUTO ptr{ FromVoidPtr<GDALRasterBand>(data) };
		if (ptr) {
			return ptr->GetDescription();
		}
		return "";
	};
	double Raster::Minimum() const {
		AUTO ptr{ FromVoidPtr<GDALRasterBand>(data) };
		if (ptr) {
			return ptr->GetMinimum();
		}
		return 0;
	};
	double Raster::Maximum() const {
		AUTO ptr{ FromVoidPtr<GDALRasterBand>(data) };
		if (ptr) {
			return ptr->GetMaximum();
		}
		return 0;
	};
	cweeInterpolatedMatrix<float> Raster::GetData() const {
		cweeInterpolatedMatrix<float> out;
		AUTO ptr{ FromVoidPtr<GDALRasterBand>(data) };
		if (ptr) {
			//ulx, xres, xskew, uly, yskew, yres = src.GetGeoTransform()
			//lrx = ulx + (src.RasterXSize * xres)
			//lry = uly + (src.RasterYSize * yres)
			//ulx, uly is the upper left corner, lrx, lry is the lower right corner








			//auto dataType = ptr->GetRasterDataType(); // int, double, int64, etc.
			//int X = ptr->GetXSize();
			//int Y = ptr->GetYSize();

			//cweeList<float> matrix_as_row;
			//matrix_as_row.SetNum(X*Y);			
			//float* dataSrce = const_cast<float*>(matrix_as_row.data());

			//auto err = ptr->RasterIO(GDALRWFlag::GF_Read, 0, 0, X, Y, (void*)dataSrce, X, Y, GDALDataType::GDT_Float32, 0, 0);
			
		}
		return out;
	};

	int Feature::Fid() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(data) };
		if (!ptr) return 0;
		return ptr->GetFID();
	};
	int Feature::NumFields() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(data) };
		if (ptr) {
			return ptr->GetFieldCount();
		}
		return 0;
	};
	const Layer& Feature::GetLayer() const { 
		return this->layer;
	};
	cweeSharedPtr<void> Feature::Data() const {	return data; };
	Field Feature::GetField(int n) const { return Field(*this, n); };
	Field Feature::GetField(cweeStr const& name) const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(data) };
		if (!ptr) return Field(*this, -1);
		return Field(*this, ptr->GetFieldIndex(name));
	};
	bool Feature::HasField(cweeStr const& name) const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(data) };
		if (!ptr) return false;
		return (ptr->GetFieldIndex(name) >= 0);
	};
	Geometry Feature::GetGeometry() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(data) };
		if (ptr) {
			auto* geoPtr = ptr->GetGeometryRef();
			if (geoPtr) {
				auto poGeometry{ cweeSharedPtr< OGRGeometry >(geoPtr, [ptr](OGRGeometry* p) { /* do nothing */ }) };
				return Geometry(*this, cweeSharedPtr<void>(poGeometry));
			}
		}
		return Geometry(*this, nullptr);		
	};

	const cweeGeo::Feature& Field::GetFeature() const { return this->Feature; };
	cweeStr Field::Name() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (ptr) {
			AUTO p = ptr->GetFieldDefnRef(fieldNumber);
			if (p) {
				return p->GetNameRef();
			}
		}
		return "";
	};
	FieldType Field::Type() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (ptr) {
			AUTO p = ptr->GetFieldDefnRef(fieldNumber);
			if (p) {
				return FieldType::_from_integral(static_cast<int>(p->GetType()));
			}
		}
		return FieldType::None;
	};
	bool Field::IsUnset() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (this->fieldNumber >= 0 && ptr) {
			return !ptr->IsFieldSet(fieldNumber);
		}
		else return true;
	};
	bool Field::IsNull() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (this->fieldNumber >= 0 && ptr) {
			return ptr->IsFieldNull(fieldNumber);
		}
		else return true;
	};
	chaiscript::Boxed_Value Field::GetBoxed() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (!this->IsNull() && !this->IsUnset() && ptr) {
			switch (Type()) {
			case  FieldType::None:
				return chaiscript::Boxed_Value();
			case FieldType::Integer: // int
				return chaiscript::var(ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::Integer64: // big int
				return chaiscript::var((u64)ptr->GetFieldAsInteger64(fieldNumber));
			case FieldType::Real: // double
				return chaiscript::var(ptr->GetFieldAsDouble(fieldNumber));
			case FieldType::Binary: // bool
				return chaiscript::var((bool)ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::DateTime: { // date & time			
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				if (month <= 0 || day <= 0) {
					return chaiscript::var(cweeTime::Epoch());
				}
				return chaiscript::var(cweeTime::make_time(year, month, day, hour, minute, second));
			}
			case FieldType::Date: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				if (month <= 0 || day <= 0) {
					return chaiscript::var(cweeTime::Epoch());
				}
				return chaiscript::var(cweeTime::make_time(year, month, day, 0, 0, 0));
			}
			case FieldType::Time: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return chaiscript::var(cweeTime::make_time(1970, 1, 1, hour, minute, second));
			}
			default:
			case FieldType::String: // string
				return chaiscript::var((cweeStr)ptr->GetFieldAsString(fieldNumber));
			}
		}
		return chaiscript::Boxed_Value();
	};
	cweeAny Field::GetAny() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (!this->IsNull() && !this->IsUnset() && ptr) {
			switch (Type()) {
			case  FieldType::None:
				return cweeAny();
			case FieldType::Integer: // int
				return cweeAny(ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::Integer64: // big int
				return cweeAny((u64)ptr->GetFieldAsInteger64(fieldNumber));
			case FieldType::Real: // double
				return cweeAny(ptr->GetFieldAsDouble(fieldNumber));
			case FieldType::Binary: // bool
				return cweeAny((bool)ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::DateTime: { // date & time			
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				if (month <= 0 || day <= 0) {
					return cweeAny(cweeTime::Epoch());
				}
				return cweeAny(cweeTime::make_time(year, month, day, hour, minute, second));
			}
			case FieldType::Date: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				if (month <= 0 || day <= 0) {
					return cweeAny(cweeTime::Epoch());
				}
				return cweeAny(cweeTime::make_time(year, month, day, 0, 0, 0));
			}
			case FieldType::Time: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return cweeAny(cweeTime::make_time(1970, 1, 1, hour, minute, second));
			}
			default:
			case FieldType::String: // string
				return cweeAny((cweeStr)ptr->GetFieldAsString(fieldNumber));
			}
		}
		return cweeAny();
	};
	cweeStr Field::GetAsString() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (!this->IsNull() && !this->IsUnset() && ptr) {
			switch (Type()) {
			case  FieldType::None:
				return cweeStr();
			case FieldType::Integer: // int
				return cweeStr(ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::Integer64: // big int
				return cweeStr((u64)ptr->GetFieldAsInteger64(fieldNumber));
			case FieldType::Real: // double
				return cweeStr(ptr->GetFieldAsDouble(fieldNumber));
			case FieldType::Binary: // bool
				return cweeStr((bool)ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::DateTime: { // date & time	
				cweeTime t;
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				if (month <= 0 || day <= 0) {
					t = cweeTime::Epoch();
				}
				else {
					t = cweeTime::make_time(year, month, day, hour, minute, second);
				}
				return cweeStr(t.c_str());
			}
			case FieldType::Date: {
				cweeTime t; 
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				if (month <= 0 || day <= 0) {
					t = cweeTime::Epoch();
				}
				else {
					t = cweeTime::make_time(year, month, day, 0, 0, 0);
				}
				return cweeStr(t.c_str());
			}
			case FieldType::Time:
			default:
			case FieldType::String: // string
				return ptr->GetFieldAsString(fieldNumber);
			}
		}
		return cweeStr();
	};
	double Field::GetAsDouble() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (!this->IsNull() && !this->IsUnset() && ptr) {
			switch (Type()) {
			case  FieldType::None:
				return 0;
			case FieldType::Integer: // int
				return (ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::Integer64: // big int
				return ((u64)ptr->GetFieldAsInteger64(fieldNumber));
			case FieldType::Real: // double
				return (ptr->GetFieldAsDouble(fieldNumber));
			case FieldType::Binary: // bool
				return ((bool)ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::DateTime: { // date & time			
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return (u64)(cweeTime::make_time(year, month, day, hour, minute, second));
			}
			case FieldType::Date: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return (u64)(cweeTime::make_time(year, month, day, 0, 0, 0));
			}
			case FieldType::Time: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return (u64)(cweeTime::make_time(0, 0, 0, hour, minute, second));
			}
			default:
			case FieldType::String: // string
				return cweeStr((cweeStr)ptr->GetFieldAsString(fieldNumber)).ReturnNumericD();
			}
		}
		return 0;
	};
	cweeTime Field::GetAsDateTime() const {
		AUTO ptr{ FromVoidPtr<OGRFeature>(this->Feature.Data()) };
		if (!this->IsNull() && !this->IsUnset() && ptr) {
			switch (Type()) {
			case  FieldType::None:
				return cweeTime();
			case FieldType::Integer: // int
				return cweeTime(ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::Integer64: // big int
				return cweeTime((u64)ptr->GetFieldAsInteger64(fieldNumber));
			case FieldType::Real: // double
				return cweeTime(ptr->GetFieldAsDouble(fieldNumber));
			case FieldType::Binary: // bool
				return cweeTime((bool)ptr->GetFieldAsInteger(fieldNumber));
			case FieldType::DateTime: { // date & time			
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return (cweeTime::make_time(year, month, day, hour, minute, second));
			}
			case FieldType::Date: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return (cweeTime::make_time(year, month, day, 0, 0, 0));
			}
			case FieldType::Time: {
				int year, month, day, hour, minute, TZ; float second;
				ptr->GetFieldAsDateTime(fieldNumber, &year, &month, &day, &hour, &minute, &second, &TZ);
				return (cweeTime::make_time(0, 0, 0, hour, minute, second));
			}
			default:
			case FieldType::String: // string
				return cweeTime((cweeStr)ptr->GetFieldAsString(fieldNumber));
			}
		}
		return cweeTime();
	};

	const cweeGeo::Feature& Geometry::GetFeature() const { return this->Feature; };
	GeometryType Geometry::Type() const {
		if (this->geometry) {
			AUTO ptr{ FromVoidPtr<OGRGeometry>(this->geometry) };
			if (ptr) {
				return GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr->getGeometryType())));
			}
		}
		return GeometryType::Unknown;
	};
	int Geometry::NumPoints() const {
		if (this->geometry) {
			AUTO ptr{ FromVoidPtr<OGRGeometry>(this->geometry) };
			if (ptr) {
				switch (GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr->getGeometryType())))) {
				default:
				case GeometryType::Unknown:
					return -1;
				case GeometryType::Point:
					return 1;
				case GeometryType::Line: {
					auto* poPoint = ptr->toLineString();
					return poPoint->getNumPoints();
				}
				case GeometryType::Multiline: {
					auto* poPoints = ptr->toMultiLineString();
					int n = 0;
					for (int j = 0; j < poPoints->getNumGeometries(); ++j) {
						auto* poPoint = poPoints->getGeometryRef(j);
						n += poPoint->getNumPoints();
					}
					return n;
				}
				case GeometryType::Polygon: {
					auto* poPoint = ptr->toPolygon();
					return -1; // TODO
				}
				}
			}
		}
		return 0;
	};
	double Geometry::Longitude(int index) const {
		if (this->geometry) {
			AUTO ptr{ FromVoidPtr<OGRGeometry>(this->geometry) };
			if (ptr) {
				switch (GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr->getGeometryType())))) {
				default:
					return Coordinates(index).x;
				case GeometryType::Unknown:
					return 0;
				case GeometryType::Point: {
					auto* poPoint = ptr->toPoint();
					return poPoint->getX();
				}
				case GeometryType::Line: {
					auto* poPoint = ptr->toLineString();
					return poPoint->getX(index);
				}
				case GeometryType::Polygon: {
					auto* poPoint = ptr->toPolygon();
					return -1; // TODO
				}
				}
			}
		}
		return 0;
	};
	double Geometry::Latitude(int index) const {
		if (this->geometry) {
			AUTO ptr{ FromVoidPtr<OGRGeometry>(this->geometry) };
			if (ptr) {
				switch (GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr->getGeometryType())))) {
				default:
					return Coordinates(index).y;
				case GeometryType::Unknown:
					return 0;
				case GeometryType::Point: {
					auto* poPoint = ptr->toPoint();
					return poPoint->getY();
				}
				case GeometryType::Line: {
					auto* poPoint = ptr->toLineString();
					return poPoint->getY(index);
				}
				case GeometryType::Polygon: {
					auto* poPoint = ptr->toPolygon();
					return -1; // TODO
				}
				}
			}
		}
		return 0;
	};
	vec2d Geometry::Coordinates(int index) const {
		if (this->geometry) {
			AUTO ptr{ FromVoidPtr<OGRGeometry>(this->geometry) };
			if (ptr) {
				switch (GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr->getGeometryType())))) {
				default:
				case GeometryType::Unknown:
					return vec2d();
				case GeometryType::Point: {
					auto* poPoint = ptr->toPoint();
					return vec2d(poPoint->getX(), poPoint->getY());
				}
				case GeometryType::Line: {
					auto* poPoint = ptr->toLineString();
					return vec2d(poPoint->getX(index), poPoint->getY(index));
				}
				case GeometryType::Multiline: {
					auto* poPoints = ptr->toMultiLineString();			
					int progress = 0;
					for (int j = 0; j < poPoints->getNumGeometries(); ++j) {
						auto* poPoint = poPoints->getGeometryRef(j);
						int n = poPoint->getNumPoints();						
						if ((progress+n) > index) {
							return vec2d(poPoint->getX(index - progress), poPoint->getY(index - progress));
						}
						progress += n;
					}
					return vec2d();
				}
				case GeometryType::Polygon: {
					auto* poPoint = ptr->toPolygon();
					return vec2d(-1, -1); // TODO
				}
				}
			}
		}
		return vec2d();
	};
	cweeList<vec2d> Geometry::AllCoordinates() const {
		cweeList<vec2d> out;
		if (this->geometry) {
			AUTO ptr{ FromVoidPtr<OGRGeometry>(this->geometry) };
			if (ptr) {
				auto geoType = GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr->getGeometryType())));
				switch (geoType) {
				default:
				case GeometryType::Unknown:
					break;
				case GeometryType::Point: {
					auto* poPoint = ptr->toPoint();
					out.Append(vec2d(poPoint->getX(), poPoint->getY()));
					break;
				}
				case GeometryType::Line: {
					auto* poPoint = ptr->toLineString();
					int n = poPoint->getNumPoints();
					out.SetGranularity(n + 1);
					for (int i = 0; i < n; i++) {
						out.Append(vec2d(poPoint->getX(i), poPoint->getY(i)));
					}
					break;
				}
				case GeometryType::Multiline: {
					auto* poPoints = ptr->toMultiLineString();
					for (int j = 0; j < poPoints->getNumGeometries(); ++j) {
						auto* poPoint = poPoints->getGeometryRef(j);
						int n = poPoint->getNumPoints();
						// out.SetGranularity(n + 1);
						for (int i = 0; i < n; i++) {
							out.Append(vec2d(poPoint->getX(i), poPoint->getY(i)));
						}
					}
					break;
				}
				case GeometryType::Polygon: {
					auto* poPoint = ptr->toPolygon();
					// TODO
					break;
				}
				}
			}
		}
		return out; 
	};
	cwee_units::foot_t Geometry::Distance(vec2d const& point1, vec2d const& point2) {
		return geocoding->Distance(point1, point2);
	};
	vec2d Geometry::ClosestPoint(vec2d const& pointCoord, cweeList<vec2d> const& lineCoords) {
		if (lineCoords.Num() == 0) {
			return pointCoord;
		}
		else if (lineCoords.Num() == 1) {
			return lineCoords[0];
		}
		else {
			cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();
			vec2d toReturn = lineCoords[0];
			auto closestPointOnLineSegment = [](const vec2d& A, const vec2d& B, const vec2d& P)->vec2d {
				// Calculate the vector AB (direction of the line segment)
				double ABx = B[0] - A[0];
				double ABy = B[1] - A[1];

				// Calculate the vector AP (from point A to point P)
				double APx = P[0] - A[0];
				double APy = P[1] - A[1];

				// Calculate the dot product of AB and AP
				double dotProduct = ABx * APx + ABy * APy;

				// Calculate the squared length of AB
				double lengthABsq = ABx * ABx + ABy * ABy;

				// Calculate the parameter t (projection of AP onto AB)
				double t = dotProduct / lengthABsq;

				// If t is outside the segment [0, 1], find the distance to the closest endpoint
				if (t < 0.0) {
					return A; // Distance to point A
				}
				else if (t > 1.0) {
					return B; // Distance to point B
				}

				// Calculate the closest point on the line segment
				double closestX = A[0] + t * ABx;
				double closestY = A[1] + t * ABy;

				return vec2d(closestX, closestY);
			};
			cwee_units::foot_t dist_start;
			cwee_units::foot_t dist_end = Distance(pointCoord, lineCoords[0]);
			cwee_units::foot_t dist_perp;

			for (int i = 1; i < lineCoords.Num(); i += 1) {
				dist_start = dist_end;
				dist_end = Distance(pointCoord, lineCoords[i]);
				auto closest = closestPointOnLineSegment(lineCoords[i - 1], lineCoords[i], pointCoord);
				dist_perp = Distance(pointCoord, closest);

				if (out > dist_perp) {
					out = dist_perp;
					toReturn = closest;
				}
				if (out > dist_end) {
					out = dist_end;
					toReturn = lineCoords[i];
				}
				if (out > dist_start) {
					out = dist_start;
					toReturn = lineCoords[i-1];
				}
			}
			return toReturn;
		}
	};

	cwee_units::foot_t Distance_Point_Line(cweeSharedPtr< OGRGeometry> const& point, cweeSharedPtr< OGRGeometry> const& line) {
		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max(); 
		
		auto* p_ptr = point->toPoint();
		auto* l_ptr = line->toLineString();
		auto pointCoord{ vec2d(p_ptr->getX(), p_ptr->getY()) };
		auto numLinePoints = l_ptr->getNumPoints();
				
		if (numLinePoints == 0) {
			out = std::numeric_limits<decltype(out)>::max();
		}
		auto lineCoords0{ vec2d(l_ptr->getX(0), l_ptr->getY(0)) };

		if (numLinePoints == 1) {
			out = Geometry::Distance(pointCoord, lineCoords0);
		}
		else {
			auto closestPointOnLineSegment = [](const vec2d& A, const vec2d& B, const vec2d& P)->vec2d {
				// Calculate the vector AB (direction of the line segment)
				double ABx = B[0] - A[0];
				double ABy = B[1] - A[1];

				// Calculate the vector AP (from point A to point P)
				double APx = P[0] - A[0];
				double APy = P[1] - A[1];

				// Calculate the dot product of AB and AP
				double dotProduct = ABx * APx + ABy * APy;

				// Calculate the squared length of AB
				double lengthABsq = ABx * ABx + ABy * ABy;

				// Calculate the parameter t (projection of AP onto AB)
				double t = dotProduct / lengthABsq;

				// If t is outside the segment [0, 1], find the distance to the closest endpoint
				if (t < 0.0) {
					return A; // Distance to point A
				}
				else if (t > 1.0) {
					return B; // Distance to point B
				}

				// Calculate the closest point on the line segment
				double closestX = A[0] + t * ABx;
				double closestY = A[1] + t * ABy;

				return vec2d(closestX, closestY);
			};
			cwee_units::foot_t dist_start;
			cwee_units::foot_t dist_end = Geometry::Distance(pointCoord, lineCoords0);
			cwee_units::foot_t dist_perp;
			vec2d prevPoint = lineCoords0;
			vec2d currentPoint;
			for (int i = 1; i < numLinePoints; i += 1) {
				currentPoint = vec2d(l_ptr->getX(i), l_ptr->getY(i));
				dist_start = dist_end;
				dist_end = Geometry::Distance(pointCoord, currentPoint);
				dist_perp = Geometry::Distance(pointCoord, closestPointOnLineSegment(prevPoint, currentPoint, pointCoord));
				out = cwee_units::math::fmin(cwee_units::math::fmin(cwee_units::math::fmin(out, dist_perp), dist_end), dist_start);
				prevPoint = currentPoint;
			}
		}
		return out;
	};
	cwee_units::foot_t Distance_Point_MultiLine(cweeSharedPtr< OGRGeometry> const& point, cweeSharedPtr< OGRGeometry> const& line) {
		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();

		auto* p_ptr = point->toPoint();
		auto* l_ptr = line->toMultiLineString();
		for (int i = 0; i < l_ptr->getNumGeometries(); i++) {
			auto line2{ cweeSharedPtr< OGRGeometry>(l_ptr->getGeometryRef(i), [](OGRGeometry* p) { /* do nothing */ })};
			out = cwee_units::math::fmin(out, Distance_Point_Line(point, line2));
		}
		return out;
	};

	cwee_units::foot_t Geometry::Distance(vec2d const& pointCoord, cweeList<vec2d> const& lineCoords) {
		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();
		if (lineCoords.Num() == 0) {
			out = std::numeric_limits<decltype(out)>::max();
		}
		else if (lineCoords.Num() == 1) {
			out = Distance(pointCoord, lineCoords[0]);
		}
		else {
			auto closestPointOnLineSegment = [](const vec2d& A, const vec2d& B, const vec2d& P)->vec2d {
				// Calculate the vector AB (direction of the line segment)
				double ABx = B[0] - A[0];
				double ABy = B[1] - A[1];

				// Calculate the vector AP (from point A to point P)
				double APx = P[0] - A[0];
				double APy = P[1] - A[1];

				// Calculate the dot product of AB and AP
				double dotProduct = ABx * APx + ABy * APy;

				// Calculate the squared length of AB
				double lengthABsq = ABx * ABx + ABy * ABy;

				// Calculate the parameter t (projection of AP onto AB)
				double t = dotProduct / lengthABsq;

				// If t is outside the segment [0, 1], find the distance to the closest endpoint
				if (t < 0.0) {
					return A; // Distance to point A
				}
				else if (t > 1.0) {
					return B; // Distance to point B
				}

				// Calculate the closest point on the line segment
				double closestX = A[0] + t * ABx;
				double closestY = A[1] + t * ABy;

				return vec2d(closestX, closestY);
			};
			cwee_units::foot_t dist_start;
			cwee_units::foot_t dist_end = Distance(pointCoord, lineCoords[0]);
			cwee_units::foot_t dist_perp;

			for (int i = 1; i < lineCoords.Num(); i += 1) {
				dist_start = dist_end;
				dist_end = Distance(pointCoord, lineCoords[i]);				
				dist_perp = Distance(pointCoord, closestPointOnLineSegment(lineCoords[i - 1], lineCoords[i], pointCoord));
				out = cwee_units::math::fmin(cwee_units::math::fmin(cwee_units::math::fmin(out, dist_perp), dist_end), dist_start);
			}
		}
		return out;
	};
	cwee_units::foot_t Geometry::Distance(cweeList<vec2d> const& coords1, cweeList<vec2d> const& coords2) {
		using namespace cwee_units;

		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();

		if (coords1.Num() == 0 || coords2.Num() == 0) return out;
		if (coords1.Num() == 1) return Distance(coords1[0], coords2);		
		else if (coords2.Num() == 1) return Distance(coords2[0], coords1);

		// intersections of lines (Does not test if end-points overlap)
		//			.
		//	........*.....
		//			.
		//			.
		if (true) {
			// Return true if line segments AB and CD intersect
			auto ccw = [](vec2d const& A, vec2d const& B, vec2d const& C) {
				return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
			};
			auto intersect = [ccw](vec2d const& A, vec2d const& B, vec2d const& C, vec2d const& D) {
				return ccw(A, C, D) != ccw(B, C, D) and ccw(A, B, C) != ccw(A, B, D);
			};
			for (int i = 1; i < coords1.Num(); i++) {
				for (int j = 1; j < coords2.Num(); j++) {
					if (intersect(coords1[i - 1], coords1[i], coords2[j - 1], coords2[j])) {
						return 0_ft;
					}
				}
			}
		}

		// distance between end-points.
		//		*..........*
		//				
		//				*....*
		for (int i = 0; i < coords1.Num(); i++) {
			for (int j = 0; j < coords2.Num(); j++) {
				if (coords1[i] == coords2[j]) { return 0_ft; }
				out = cwee_units::math::fmin(out, Distance(coords1[i], coords2[j]));								
			}
		}

		// nearest perpendicular points for end-points
		//		.....*......
		//			  
		//			 *				
		//			.
		//		   .
		//        .
		for (int i = 0; i < coords1.Num(); i++) out = cwee_units::math::fmin(out, Distance(coords1[i], coords2));					
		for (int i = 0; i < coords2.Num(); i++) out = cwee_units::math::fmin(out, Distance(coords2[i], coords1));

		return out;
	};
	cweeStr Geometry::Geocode(vec2d const& point1) { return geocoding->GetAddress(point1); };
	cwee_units::foot_t Geometry::Elevation(vec2d const& point1) { return geocoding->GetElevation(point1); };
	cwee_units::foot_t Geometry::Distance(Geometry const& obj1, Geometry const& obj2) {
		using namespace cwee_units;
		cwee_units::foot_t out = std::numeric_limits<cwee_units::foot_t>::max();

		AUTO ptr1{ FromVoidPtr<OGRGeometry>(obj1.Data()) };
		AUTO ptr2{ FromVoidPtr<OGRGeometry>(obj2.Data()) };
		if (ptr1 && ptr2) {
			auto type1 = GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr1->getGeometryType())));
			auto type2 = GeometryType::_from_integral_unchecked(static_cast<int>(wkbFlatten(ptr2->getGeometryType())));

			if (type1 <= type2) {
				const Geometry& objA = obj1;
				const Geometry& objB = obj2;
				if (type1 == GeometryType::Point && type2 == GeometryType::Point) {// point / point
					out = Distance(objA.Coordinates(0), objB.Coordinates(0));
				}
				else if (type1 == GeometryType::Point && type2 == GeometryType::Line) {// point / Line
					out = Distance_Point_Line(ptr1, ptr2);
				}
				else if (type1 == GeometryType::Point && type2 == GeometryType::Multiline) {// point / Multiline
					out = Distance_Point_MultiLine(ptr1, ptr2);
				}
				else if (type1 == GeometryType::Line && type2 == GeometryType::Line) {// Line / Line
					out = Distance(objA.AllCoordinates(), objB.AllCoordinates());
				}
				else if (type1 == GeometryType::Line && type2 == GeometryType::Multiline) {// Line / Multiline
					out = Distance(objA.AllCoordinates(), objB.AllCoordinates());
				}
				else if (type1 == GeometryType::Multiline && type2 == GeometryType::Multiline) {// Multiline / Multiline
					out = Distance(objA.AllCoordinates(), objB.AllCoordinates());
				}
				else {
					out = std::numeric_limits<decltype(out)>::max();
				}
				return out;
			}
			else {
				const Geometry& objA = obj2;
				const Geometry& objB = obj1;
				if (type2 == GeometryType::Point && type1 == GeometryType::Point) {// point / point
					out = Distance(objA.Coordinates(0), objB.Coordinates(0));
				}
				else if (type2 == GeometryType::Point && type1 == GeometryType::Line) {// point / Line
					out = Distance_Point_Line(ptr2, ptr1);
				}
				else if (type2 == GeometryType::Point && type1 == GeometryType::Multiline) {// point / Multiline
					out = Distance_Point_MultiLine(ptr2, ptr1);
				}
				else if (type2 == GeometryType::Line && type1 == GeometryType::Line) {// Line / Line
					out = Distance(objA.AllCoordinates(), objB.AllCoordinates());
				}
				else if (type2 == GeometryType::Line && type1 == GeometryType::Multiline) {// Line / Multiline
					out = Distance(objA.AllCoordinates(), objB.AllCoordinates());
				}
				else if (type2 == GeometryType::Multiline && type1 == GeometryType::Multiline) {// Multiline / Multiline
					out = Distance(objA.AllCoordinates(), objB.AllCoordinates());
				}
				else {
					out = std::numeric_limits<decltype(out)>::max();
				}
				return out;
			}
		}
		return out;
	};
	cwee_units::foot_t Geometry::Distance(Geometry const& obj) const { return Distance(*this, obj);	};

	cwee_units::foot_t Geometry::Length(Geometry const& obj) {
		using namespace cwee_units;
		cwee_units::foot_t tot = 0_ft;
		AUTO coords = obj.AllCoordinates();
		for (int i = 1; i < coords.Num(); i++) {
			tot += Distance(coords[i - 1], coords[i]);
		}
		return tot;
	};
	cwee_units::foot_t Geometry::Length() const { return Length(*this); };
	cweeStr Geometry::Geocode() const { return Geocode(Coordinates(0)); };
	cwee_units::foot_t Geometry::Elevation() const { return Elevation(Coordinates(0)); };

	cwee_units::foot_t Feature::Distance(Feature const& obj) const{ return this->GetGeometry().Distance(obj.GetGeometry());	};
	cwee_units::foot_t Feature::Length() const { return this->GetGeometry().Length(); };
	cweeStr Feature::Geocode() const { return this->GetGeometry().Geocode(); };
	cwee_units::foot_t Feature::Elevation() const { return this->GetGeometry().Elevation(); };

	INLINE std::map<int, cweeList<cweePair<int, double>>> Near(std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, std::function<double(Geometry const&, Geometry const&)> DistanceFunction, int numNearest, std::function<bool(Feature const&)> WhereFunction) {
		std::map<int, cweeList<cweePair<int, double>>> out;
		cweeBalancedPattern<cwee_units::scalar_t, u64> distance_to_layer2_Fid; // x-axis is distance, y-axis is Layer2's Fid. 
		int i, Layer1Fid, Layer2Fid, NumFeatures_Layer2;
		cweeList<cweePair<int, double>> nearestList;

		std::map<int, Geometry> feat1_List;
		std::map<int, Geometry > feat2_List;

		for (Layer1Fid = 0; Layer1Fid < layer1.size(); Layer1Fid++) {
			auto& feat = layer1[Layer1Fid];
			const auto* featPtr = chaiscript::boxed_cast<const Feature*>(feat);
			if (featPtr && WhereFunction(*featPtr)) feat1_List[Layer1Fid] = featPtr->GetGeometry();
		}
		for (Layer2Fid = 0; Layer2Fid < layer2.size(); Layer2Fid++) {
			auto& feat = layer2[Layer2Fid];
			const auto* featPtr = chaiscript::boxed_cast<const Feature*>(feat);
			if (featPtr && WhereFunction(*featPtr)) feat2_List[Layer2Fid] = featPtr->GetGeometry();
		}
		for (auto& Layer1Iter : feat1_List) {
			distance_to_layer2_Fid.ClearData();
			for (auto& Layer2Iter : feat2_List) {
				distance_to_layer2_Fid.AddValue(DistanceFunction(Layer1Iter.second, Layer2Iter.second), Layer2Iter.first);
			}

			nearestList.Clear();
			distance_to_layer2_Fid.Lock();
			for (auto& ptr : distance_to_layer2_Fid.UnsafeGetValues()) {
				if (nearestList.Num() >= numNearest) break;
				if (ptr.object) {
					nearestList.Append(cweePair<int, double>(ptr.object->operator()(), ptr.key));
				}
			}
			distance_to_layer2_Fid.Unlock();

			out[Layer1Iter.first] = nearestList;
		}
		return out;
	};
	std::map<int, cweeList<cweePair<int, double>>> Layer::Near(Layer const& layer1, Layer const& layer2, std::function<double(Geometry const&, Geometry const&)> DistanceFunction, int numNearest, std::function<bool(Feature const&)> WhereFunction) {
		std::map<int, cweeList<cweePair<int, double>>> out;
		cweeBalancedPattern<cwee_units::scalar_t, u64> distance_to_layer2_Fid; // x-axis is distance, y-axis is Layer2's Fid. 
		int i, Layer1Fid, Layer2Fid, NumFeatures_Layer2;
		cweeList<cweePair<int, double>> nearestList;

		std::map<int, Geometry> feat1_List;
		std::map<int, Geometry > feat2_List;

		for (Layer1Fid = 0; Layer1Fid < layer1.NumFeatures(); Layer1Fid++) {
			auto feat = layer1.GetFeature(Layer1Fid);
			if (WhereFunction(feat)) feat1_List[Layer1Fid] = feat.GetGeometry();
		}
		for (Layer2Fid = 0; Layer2Fid < layer2.NumFeatures(); Layer2Fid++) {
			auto feat = layer2.GetFeature(Layer2Fid);
			if (WhereFunction(feat)) feat2_List[Layer2Fid] = feat.GetGeometry();
		}
		for (auto& Layer1Iter : feat1_List) {
			distance_to_layer2_Fid.ClearData();
			for (auto& Layer2Iter : feat2_List) {
				distance_to_layer2_Fid.AddValue(DistanceFunction(Layer1Iter.second, Layer2Iter.second), Layer2Iter.first);
			}

			nearestList.Clear();
			distance_to_layer2_Fid.Lock();
			for (auto& ptr : distance_to_layer2_Fid.UnsafeGetValues()) {
				if (nearestList.Num() >= numNearest) break;
				if (ptr.object) {
					nearestList.Append(cweePair<int, double>(ptr.object->operator()(), ptr.key));
				}
			}
			distance_to_layer2_Fid.Unlock();

			out[Layer1Iter.first] = nearestList;
		}
		return out;
	};
	cweeList<cweePair<int, double>> Layer::Near(Feature const& layer1, Layer const& layer2, std::function<double(Geometry const&, Geometry const&)> DistanceFunction, int numNearest, std::function<bool(Feature const&)> WhereFunction) {
		cweeBalancedPattern<cwee_units::scalar_t, u64> distance_to_layer2_Fid; // x-axis is distance, y-axis is Layer2's Fid. 
		int i, Layer1Fid, Layer2Fid, NumFeatures_Layer2;
		cweeList<cweePair<int, double>> nearestList;

		std::map<int, Geometry > feat2_List;

		auto feat1Geo = layer1.GetGeometry();
		for (Layer2Fid = 0; Layer2Fid < layer2.NumFeatures(); Layer2Fid++) {
			auto feat = layer2.GetFeature(Layer2Fid);
			if (WhereFunction(feat)) feat2_List[Layer2Fid] = feat.GetGeometry();
		}

		distance_to_layer2_Fid.ClearData();
		for (auto& Layer2Iter : feat2_List) {
			distance_to_layer2_Fid.AddValue(DistanceFunction(feat1Geo, Layer2Iter.second), Layer2Iter.first);
		}

		nearestList.Clear();
		distance_to_layer2_Fid.Lock();
		for (auto& ptr : distance_to_layer2_Fid.UnsafeGetValues()) {
			if (nearestList.Num() >= numNearest) break;
			if (ptr.object) {
				nearestList.Append(cweePair<int, double>(ptr.object->operator()(), ptr.key));
			}
		}
		distance_to_layer2_Fid.Unlock();

		return nearestList;
	};
}


class GDAL_Data_Local final : public GDAL_Data {
private:
	cweeSharedPtr<OGRSpatialReference> WGS84SRS;

public:
	GDAL_Data_Local() : WGS84SRS(OGRSpatialReference::GetWGS84SRS(), [](OGRSpatialReference* p) {}) {
		GDALAllRegister(); // register all drivers
		CPLPushErrorHandler(CPLQuietErrorHandler); // quiet the errors from GDAL
	};
	virtual ~GDAL_Data_Local() {
		GDALDestroy();
	};
	cweeStr TestGDAL(cweeStr filePath) { // cweeStr filePath = "point.shp";		
		cweeGeo::Field field; 
		cweeGeo::Layer layer; 
		cweeGeo::Feature feat; cweeGeo::Feature feat1;
		cweeGeo::Geometry geometry;
		cweeGeo::Shapefile shapefile{ filePath };
		
		for (int i = shapefile.NumLayers() - 1; i >= 0; i--) {
			layer = shapefile.GetLayer(i);

			feat1 = layer.GetFeature(13840);

			for (int Fid = layer.NumFeatures() - 1; Fid >= 0; Fid--) {
				feat = layer.GetFeature(Fid);
				
				printf("%d", (int)feat.Fid());
				for (int i = 0; i < feat.NumFields(); i++) {
					field = feat.GetField(i);
					if (field.IsUnset()) printf(", <unset>");
					else if (field.IsNull()) printf(", <null>");
					else {
						switch (field.Type()) {
						case FieldType::None:
							printf(", <UNKNOWN>"); break;
						case FieldType::Integer: // int
						case FieldType::Integer64: // big int
							printf(", %d", (int)field.GetAsDouble()); break;
						case FieldType::Real: // double
							printf(", %.3f", field.GetAsDouble()); break;
						case FieldType::Binary: // bool
							printf(", %d", (int)field.GetAsDouble()); break;
						case FieldType::DateTime: // date & time			
							printf(", %s", field.GetAsDateTime().c_str()); break;
						default:
						case FieldType::String: // string
							printf(", %s", field.GetAsString().c_str()); break;
						}
					}
				}

				printf(", %.3f ft", feat.Length().value());

				printf("\n\t");

				auto distance = feat.Distance(feat1);
				printf("Distance b/w Fid#%d and Fid#%d: %.3f %s", feat1.Fid(), feat.Fid(), distance.value(), distance.abbreviation());

				printf("\n\n");
			}
		}

		return "successful!";
	};
};
cweeSharedPtr<GDAL_Data> gdal_data = make_cwee_shared<GDAL_Data_Local>().CastReference<GDAL_Data>(); // DelayedInstantiation< GDAL_Data>([]()->GDAL_Data* { return new GDAL_Data_Local(); });







namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr GDAL_library() {
			using namespace cweeGeo;

            auto lib = chaiscript::make_shared<Module>();

			ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(FieldType, FieldType);
			ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(GeometryType, GeometryType);

			AddBasicClassTemplate(Geometry);
			lib->AddFunction(, GetFeature, -> Feature , return o.GetFeature(), Geometry const& o);
			AddBasicClassMember(Geometry, Type);
			AddBasicClassMember(Geometry, NumPoints);
			AddBasicClassMember(Geometry, Longitude);
			AddBasicClassMember(Geometry, Latitude);
			AddBasicClassMember(Geometry, Coordinates);
			AddBasicClassMember(Geometry, AllCoordinates); DEF_DECLARE_STD_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(vec2d);
			lib->AddFunction(, Distance, , return a.Distance(b), Geometry const& a, Geometry const& b);
			lib->AddFunction(, Length, , return o.Length(), Geometry const& o);
			lib->AddFunction(, Elevation, , return o.Elevation(), Geometry const& o);
			lib->AddFunction(, to_string, , 
				cweeStr out; 
			    for (auto& x : o.AllCoordinates()) 
					out.AddToDelimiter(cweeStr::printf("{%.4f, %.4f}", x.x, x.y), ", "); 
				return out, 
			Geometry const& o);

			AddBasicClassTemplate(Field);
			lib->AddFunction(, GetFeature, -> Feature, return o.GetFeature(), Field const& o);
			AddBasicClassMember(Field, Name);
			AddBasicClassMember(Field, Type);
			AddBasicClassMember(Field, IsUnset);
			AddBasicClassMember(Field, IsNull);
			lib->AddFunction(, Get, , return a.GetBoxed(), Field const& a);
			AddBasicClassMember(Field, GetAsString);
			AddBasicClassMember(Field, GetAsDouble);
			AddBasicClassMember(Field, GetAsDateTime);
			lib->AddFunction(, to_string, ,return o.GetAsString(), Field const& o);

			AddBasicClassTemplate(Feature);
			AddBasicClassMember(Feature, Fid);
			AddBasicClassMember(Feature, NumFields);
			lib->AddFunction(, GetLayer, -> Layer, return o.GetLayer(), Feature const& o);
			AddBasicClassMember(Feature, Geocode);
			lib->AddFunction(, GetField, , return a.GetField(index), Feature const& a, int index);
			lib->AddFunction(, GetField, , return a.GetField(name), Feature const& a, cweeStr const& name);
			lib->AddFunction(, GetGeometry, -> Geometry, return o.GetGeometry(), Feature const& o);
			lib->AddFunction(, Distance, , return a.Distance(b), Feature const& a, Feature const& b);
			lib->AddFunction(, Length, , return o.Length(), Feature const& o);
			lib->AddFunction(, Elevation, , return o.Elevation(), Feature const& o);
			lib->AddFunction(, to_string, ,
				cweeStr out = cweeStr::printf("[\"Fid\": %i]", o.Fid());
				for (int i = 0; i < o.NumFields(); i++) {
					auto field = o.GetField(i);
					if (field.IsNull()) out.AddToDelimiter(cweeStr::printf("[\"%s\": NULL]", field.Name().c_str()), ", ");
					else if (field.IsUnset()) out.AddToDelimiter(cweeStr::printf("[\"%s\": UNSET]", field.Name().c_str()), ", ");
					else out.AddToDelimiter(cweeStr::printf("[\"%s\": \"%s\"]", field.Name().c_str(), field.GetAsString().c_str()), ", ");				
				}
				return out;
			, Feature const& o);
			lib->AddFunction(, [], ->Field, return o.GetField(fieldName), Feature const& o, cweeStr const& fieldName);

			AddBasicClassTemplate(Layer);
			AddBasicClassMember(Layer, GetFeature);
			AddBasicClassMember(Layer, Name);
			AddBasicClassMember(Layer, NumFeatures);
			lib->AddFunction(, to_string, , return cweeStr::printf("%i Feature(s): \"%s\"", o.NumFeatures(), o.Name().c_str()), Layer const& o);
			lib->AddFunction(, [], ->Feature, return o.GetFeature(featureNum), Layer const& o, int featureNum);

			AddBasicClassTemplate(Raster);
			AddBasicClassMember(Raster, Fid); 
			AddBasicClassMember(Raster, SizeX);
			AddBasicClassMember(Raster, SizeY);
			AddBasicClassMember(Raster, Minimum);
			AddBasicClassMember(Raster, Maximum);
			AddBasicClassMember(Raster, GetData);
			lib->AddFunction(, to_string, , return cweeStr::printf("[%ix%i]", o.SizeX(), o.SizeY()), Raster const& o);

			AddBasicClassTemplate(Shapefile);
			lib->AddFunction(, Shapefile, , return Shapefile(fp), cweeStr const& fp);
			lib->AddFunction(, GetRaster, ->Raster, return a.GetRaster(index), Shapefile const& a, int index);
			lib->AddFunction(, GetLayer, ->Layer , return a.GetLayer(index), Shapefile const& a, int index);
			lib->AddFunction(, GetLayer, ->Layer , return a.GetLayer(name), Shapefile const& a, cweeStr const& name);
			AddBasicClassMember(Shapefile, NumLayers);
			AddBasicClassMember(Shapefile, NumRasters);
			AddBasicClassMember(Shapefile, DriverName);
			AddBasicClassMember(Shapefile, Description);
			lib->AddFunction(, to_string, , return cweeStr::printf("%i Layer(s), %i Raster(s): \"%s\"", o.NumLayers(), o.NumRasters(), o.Description().c_str()), Shapefile const& o);
			lib->AddFunction(, [], ->Layer, return o.GetLayer(layerNum), Shapefile const& o, int layerNum);
			lib->AddFunction(, [], ->Layer, return o.GetLayer(layerName), Shapefile const& o, cweeStr layerName);

			AUTO ToMapElementFromGeometry = [](Geometry const& geo) -> chaiscript::Boxed_Value {
				chaiscript::Boxed_Value out;
				switch (geo.Type()) {
				case GeometryType::Point: {
					UI_MapIcon icon; {
						icon.longitude = geo.Longitude(0);
						icon.latitude = geo.Latitude(0);
						icon.HideOnCollision = false;
					}
					out = var(std::move(icon));
					break;
				}
				case GeometryType::Multiline:
				case GeometryType::Line: {
					UI_MapPolyline line; {
						line.thickness = 2;
						line.color.A = 128;
						for (auto& coord : geo.AllCoordinates()) {
							line.AddPoint(coord.x, coord.y);
						}
					}
					out = var(std::move(line));
					break;
				}
				case GeometryType::Polygon: {
					// TBD
					break;
				}
				default:
					break;
				}
				return out;
			};
			AUTO ToMapElement = [](Feature const& feature) -> chaiscript::Boxed_Value {
				chaiscript::Boxed_Value out;
				auto geo = feature.GetGeometry();
				switch (geo.Type()) {
				case GeometryType::Point: {
					UI_MapIcon icon; {
						icon.longitude = geo.Longitude(0);
						icon.latitude = geo.Latitude(0);
						icon.HideOnCollision = false;
						icon.Tag = var(Feature(feature));
					}
					out = var(std::move(icon));
					break;
				}
				case GeometryType::Multiline:
				case GeometryType::Line: {
					UI_MapPolyline line; {
						line.thickness = 2;
						line.color.A = 128;
						for (auto& coord : geo.AllCoordinates()) {
							line.AddPoint(coord.x, coord.y);
						}
						line.Tag = var(Feature(feature));
					}
					out = var(std::move(line));
					break;
				}
				case GeometryType::Polygon: {
					// TBD
					break;
				}
				default:
					break;
				}
				return out;
			};
			AUTO ToMapLayer = [ToMapElement](Layer const& layer)-> UI_MapLayer {
				UI_MapLayer out;				
				for (int i = 0; i < layer.NumFeatures(); ++i) {
					out.Children.push_back(ToMapElement(layer.GetFeature(i)));
				}
				out.Tag = var(Layer(layer));
				return out;
			};
			AUTO ToMap = [ToMapLayer](Shapefile const& shapefile)-> UI_Map {
				UI_Map out;
				for (int i = 0; i < shapefile.NumLayers(); ++i) {
					out.Layers.push_back(var(ToMapLayer(shapefile.GetLayer(i))));
				}
				out.Tag = var(Shapefile(shapefile));
				return out;
			};

			lib->AddFunction(ToMap, UI_Map, , return ToMap(obj), Shapefile const& obj);
			lib->AddFunction(ToMapLayer, UI_MapLayer, , return ToMapLayer(obj), Layer const& obj);
			lib->AddFunction(ToMapElement, UI_MapIcon, , return ToMapElement(obj), Feature const& obj);
			lib->AddFunction(ToMapElement, UI_MapPolyline, , return ToMapElement(obj), Feature const& obj);
			lib->AddFunction(ToMapElementFromGeometry, UI_MapIcon, , return ToMapElementFromGeometry(obj), Geometry const& obj);
			lib->AddFunction(ToMapElementFromGeometry, UI_MapPolyline, , return ToMapElementFromGeometry(obj), Geometry const& obj);

			AUTO NearestMatchFeatureDistanceWhere = [](Feature const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> dist, std::function<bool(Feature const&)> Where) {
				AUTO nearList = Layer::Near(layer1, layer2, dist, numNearest, Where);
				std::vector<chaiscript::Boxed_Value> out;
				for (auto& pairing : nearList) {											
					std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value> match; {
						match.first = var(layer2.GetFeature(pairing.first));
						match.second = var(cwee_units::foot_t(pairing.second));
					}
					out.push_back(var(std::move(match)));
				}
				return out;
			};
			AUTO NearestMatchFeatureDistance = [NearestMatchFeatureDistanceWhere](Feature const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> dist) {
				return NearestMatchFeatureDistanceWhere(layer1, layer2, numNearest, dist, [](Feature const& a)->bool { return true; });
			};
			AUTO NearestMatchFeature = [NearestMatchFeatureDistance](Feature const& layer1, Layer const& layer2, int numNearest) {
				return NearestMatchFeatureDistance(layer1, layer2, numNearest, [](Geometry const& a, Geometry const& b)->double {
					return a.Distance(b)();
				});
			};

			AUTO NearestMatchDistanceWhere = [](Layer const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> dist, std::function<bool(Feature const&)> Where) {
				AUTO nearList = Layer::Near(layer1, layer2, dist, numNearest, Where);

				std::vector<chaiscript::Boxed_Value> out;
				for (auto& pairing : nearList) {
					std::vector<chaiscript::Boxed_Value> out2;
					for (auto& matched : pairing.second) {
						std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value> pair; {
							pair.first = var(layer1.GetFeature(pairing.first));
							{
								std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value> match; {
									match.first = var(layer2.GetFeature(matched.first));
									match.second = var(cwee_units::foot_t(matched.second));
								}
								pair.second = var(std::move(match));
							}
						}
						out2.push_back(var(std::move(pair)));
					}
					out.push_back(var(std::move(out2)));
				}
				return out;
			};
			AUTO NearestMatchDistance = [NearestMatchDistanceWhere](Layer const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> dist) {
				return NearestMatchDistanceWhere(layer1, layer2, numNearest, dist, [](Feature const& a)->bool { return true; });
			};
			AUTO NearestMatch = [NearestMatchDistance](Layer const& layer1, Layer const& layer2, int numNearest) {
				return NearestMatchDistance(layer1, layer2, numNearest, [](Geometry const& a, Geometry const& b)->double {
					return a.Distance(b)();
				});
			};

			AUTO NearestMatchListDistanceWhere = [](std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> dist, std::function<bool(Feature const&)> Where) {
				AUTO nearList = cweeGeo::Near(layer1, layer2, dist, numNearest, Where);

				std::vector<chaiscript::Boxed_Value> out;
				for (auto& pairing : nearList) {
					std::vector<chaiscript::Boxed_Value> out2;
					for (auto& matched : pairing.second) {
						std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value> pair; {
							pair.first = layer1[pairing.first]; {
								std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value> match; {
									match.first = layer2[matched.first];
									match.second = var(cwee_units::foot_t(matched.second));
								}
								pair.second = var(std::move(match));
							}
						}
						out2.push_back(var(std::move(pair)));
					}
					out.push_back(var(std::move(out2)));
				}
				return out;
			};
			AUTO NearestMatchListDistance = [NearestMatchListDistanceWhere](std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> dist) {
				return NearestMatchListDistanceWhere(layer1, layer2, numNearest, dist, [](Feature const& a)->bool { return true; });
			};
			AUTO NearestMatchList = [NearestMatchListDistance](std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, int numNearest) {
				return NearestMatchListDistance(layer1, layer2, numNearest, [](Geometry const& a, Geometry const& b)->double {
					return a.Distance(b)();
				});
			};

			lib->AddFunction(NearestMatch, Near, , return NearestMatch(layer1, layer2, 1), Layer const& layer1, Layer const& layer2);
			lib->AddFunction(NearestMatch, Near, , return NearestMatch(layer1, layer2, numNearest), Layer const& layer1, Layer const& layer2, int numNearest);
			lib->AddFunction(NearestMatchDistance, Near, , return NearestMatchDistance(layer1, layer2, numNearest, dist), Layer const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> const& dist);
			lib->AddFunction(NearestMatchDistanceWhere, Near, , return NearestMatchDistanceWhere(layer1, layer2, numNearest, [](Geometry const& a, Geometry const& b)->double { return a.Distance(b)(); }, Where), Layer const& layer1, Layer const& layer2, int numNearest, std::function<bool(Feature const&)> const& Where);
			lib->AddFunction(NearestMatchDistanceWhere, Near, , return NearestMatchDistanceWhere(layer1, layer2, numNearest, dist, Where), Layer const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> const& dist, std::function<bool(Feature const&)> const& Where);

			lib->AddFunction(NearestMatchFeature, Near, , return NearestMatchFeature(layer1, layer2, 1), Feature const& layer1, Layer const& layer2);
			lib->AddFunction(NearestMatchFeature, Near, , return NearestMatchFeature(layer1, layer2, numNearest), Feature const& layer1, Layer const& layer2, int numNearest);
			lib->AddFunction(NearestMatchFeatureDistance, Near, , return NearestMatchFeatureDistance(layer1, layer2, numNearest, dist), Feature const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> const& dist);
			lib->AddFunction(NearestMatchFeatureDistanceWhere, Near, , return NearestMatchFeatureDistanceWhere(layer1, layer2, numNearest, [](Geometry const& a, Geometry const& b)->double { return a.Distance(b)(); }, Where), Feature const& layer1, Layer const& layer2, int numNearest, std::function<bool(Feature const&)> const& Where);
			lib->AddFunction(NearestMatchFeatureDistanceWhere, Near, , return NearestMatchFeatureDistanceWhere(layer1, layer2, numNearest, dist, Where), Feature const& layer1, Layer const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> const& dist, std::function<bool(Feature const&)> const& Where);

			lib->AddFunction(NearestMatchList, Near, , return NearestMatchList(layer1, layer2, 1), std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2);
			lib->AddFunction(NearestMatchList, Near, , return NearestMatchList(layer1, layer2, numNearest), std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, int numNearest);
			lib->AddFunction(NearestMatchListDistance, Near, , return NearestMatchListDistance(layer1, layer2, numNearest, dist), std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> const& dist);
			lib->AddFunction(NearestMatchListDistanceWhere, Near, , return NearestMatchListDistanceWhere(layer1, layer2, numNearest, [](Geometry const& a, Geometry const& b)->double { return a.Distance(b)(); }, Where), std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, int numNearest, std::function<bool(Feature const&)> const& Where);
			lib->AddFunction(NearestMatchListDistanceWhere, Near, , return NearestMatchListDistanceWhere(layer1, layer2, numNearest, dist, Where), std::vector<chaiscript::Boxed_Value> const& layer1, std::vector<chaiscript::Boxed_Value> const& layer2, int numNearest, std::function<double(Geometry const&, Geometry const&)> const& dist, std::function<bool(Feature const&)> const& Where);

			AUTO kmeans = [](int k, int n)->std::vector<chaiscript::Boxed_Value> {
				int m = n * k * 10;
				std::vector<std::vector<double>> centers(k, std::vector<double>(n));
				std::vector<std::vector<double>> data(m, std::vector<double>(n));
				std::vector<int> labels(m);
				//random_device rd;
				//mt19937 gen(rd());
				//uniform_real_distribution<> dis(0.0, 1.0);
				for (int i = 0; i < m; ++i) {
					for (int j = 0; j < n; ++j) {
						data[i][j] = cweeRandomFloat(0, 1); // dis(gen);
					}
				}
				for (int i = 0; i < k; ++i) {
					centers[i] = data[i];
				}
				while (true) {
					std::vector<std::vector<double>> new_centers(k, std::vector<double>(n));
					std::vector<int> counts(k);
					for (int i = 0; i < m; ++i) {
						double min_dist = std::numeric_limits<double>::max();
						int label = -1;
						for (int j = 0; j < k; ++j) {
							double dist = 0;
							for (int l = 0; l < n; ++l) {
								dist += std::pow(data[i][l] - centers[j][l], 2);
							}
							if (dist < min_dist) {
								min_dist = dist;
								label = j;
							}
						}
						labels[i] = label;
						counts[label]++;
						for (int l = 0; l < n; ++l) {
							new_centers[label][l] += data[i][l];
						}
					}
					bool converged = true;
					for (int i = 0; i < k; ++i) {
						if (counts[i] == 0) {
							continue;
						}
						for (int l = 0; l < n; ++l) {
							new_centers[i][l] /= counts[i];
							if (new_centers[i][l] != centers[i][l]) {
								converged = false;
							}
							centers[i][l] = new_centers[i][l];
						}
					}
					if (converged) {
						break;
					}
				}

				std::vector<chaiscript::Boxed_Value> out;
				for (std::vector<double>& x : centers) {
					std::vector<chaiscript::Boxed_Value> row;
					for (auto& y : x) {
						row.push_back(var((double)y));
					}
					out.push_back(var(row));
				}
				return out;
			};
			AUTO kmeans2 = [](int k, std::vector<vec2d> const& data)->std::vector<chaiscript::Boxed_Value> {
				int m = data.size();
				int n = 2;
				int i, j, l;
				double min_dist, dist;
				bool converged;
				int label;
				std::vector<vec2d> centers(k, vec2d());
				std::vector<int> labels(m);
				std::vector<std::vector<double>> new_centers(k, std::vector<double>(n));
				std::vector<int> counts(k);

				for (i = 0; i < k; ++i) centers[i] = data[i];				
				while (true) {
					for (i = 0; i < k; ++i) for (j = 0; j < n; ++j) new_centers[i][j] = 0;										
					for (i = 0; i < k; ++i) counts[i] = 0;					
					for (i = 0; i < m; ++i) {
						min_dist = std::numeric_limits<double>::max();
						label = -1;
						for (j = 0; j < k; ++j) {
							dist = 0;
							for (l = 0; l < n; ++l) {
								dist += std::pow(data[i][l] - centers[j][l], 2);
							}
							if (dist < min_dist) {
								min_dist = dist;
								label = j;
							}
						}
						labels[i] = label;
						counts[label]++;
						for (l = 0; l < n; ++l) {
							new_centers[label][l] += data[i][l];
						}
					}
					converged = true;
					for (i = 0; i < k; ++i) {
						if (counts[i] == 0) {
							continue;
						}
						for (l = 0; l < n; ++l) {
							new_centers[i][l] /= counts[i];
							if (new_centers[i][l] != centers[i][l]) {
								converged = false;
							}
							centers[i][l] = new_centers[i][l];
						}
					}
					if (converged) {
						break;
					}
				}

				std::vector<chaiscript::Boxed_Value> out;
				for (auto& x : centers) {
					std::vector<chaiscript::Boxed_Value> row;
					for (i = 0; i < n; ++i) {
						row.push_back(var((double)x[i]));
					}
					out.push_back(var(row));
				}
				return out;
			};
			AUTO kmeans_cells = [](int k, std::vector<vec2d> const& data)->UI_MapLayer {
				UI_MapLayer out;

				int m = data.size();
				int n = 2;
				int i, j, l;
				double min_dist, dist;
				bool converged;
				int label;
				std::vector<vec2d> centers(k, vec2d());
				std::vector<int> labels(m);
				std::vector<std::vector<double>> new_centers(k, std::vector<double>(n));
				std::vector<int> counts(k);

				for (i = 0; i < k; ++i) centers[i] = data[i];
				while (true) {
					for (i = 0; i < k; ++i) for (j = 0; j < n; ++j) new_centers[i][j] = 0;
					for (i = 0; i < k; ++i) counts[i] = 0;
					for (i = 0; i < m; ++i) {
						min_dist = std::numeric_limits<double>::max();
						label = -1;
						for (j = 0; j < k; ++j) {
							dist = 0;
							for (l = 0; l < n; ++l) {
								dist += std::pow(data[i][l] - centers[j][l], 2);
							}
							if (dist < min_dist) {
								min_dist = dist;
								label = j;
							}
						}
						labels[i] = label;
						counts[label]++;
						for (l = 0; l < n; ++l) {
							new_centers[label][l] += data[i][l];
						}
					}
					converged = true;
					for (i = 0; i < k; ++i) {
						if (counts[i] == 0) {
							continue;
						}
						for (l = 0; l < n; ++l) {
							new_centers[i][l] /= counts[i];
							if (new_centers[i][l] != centers[i][l]) {
								converged = false;
							}
							centers[i][l] = new_centers[i][l];
						}
					}
					if (converged) {
						break;
					}
				}

				cweeList<vec2d> cweeCenters;
				cweeCenters = centers;

				auto voronoiParent { Voronoi(cweeCenters) };

				for (auto& cell : voronoiParent.GetCells()) {
					auto c = UI_Color(cweeRandomFloat(0, 245), cweeRandomFloat(0, 245), cweeRandomFloat(0, 245), 255);
					auto t = cweeRandomInt(1, 6);
					for (auto& edge : cell.edges) {
						UI_MapPolyline line;
						line.color = c;
						line.thickness = t;
						
						line.AddPoint(edge.first.x, edge.first.y);
						line.AddPoint(edge.second.x, edge.second.y);

						out.Children.push_back(var((UI_MapPolyline)line));
					}
				}

				return out;
			};
			lib->AddFunction(kmeans, kmeans, , return kmeans(numCenters, numDim), int numCenters, int numDim);
			lib->AddFunction(kmeans2, kmeans, , return kmeans2(numCenters, data), int numCenters, std::vector<vec2d> const& data);
			lib->AddFunction(kmeans_cells, draw_kmeans, , return kmeans_cells(numCenters, data), int numCenters, std::vector<vec2d> const& data);

            return lib;
        };
    };
}; // namespace chaiscript



