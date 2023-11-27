// TODO: This is an example of a library function
#pragma once
#include "GDAL.h"
#include "include/gdal.h"
#include "include/gdal_priv.h"
#include "include/ogrsf_frmts.h"
#include "include/ogr_geos.h"

template <typename T> INLINE cweeSharedPtr<void> ToVoidPtr(T& ptr) {
	return cweeSharedPtr<void>(make_cwee_shared<T>(ptr), [](void* p) { return p; });
};
template <typename T> INLINE cweeSharedPtr<T> FromVoidPtr(cweeSharedPtr<void> data) {
	return cweeSharedPtr<T>(data, [](void* p) { return static_cast<T*>(p); });
};

class GDAL_Data_Local final : public GDAL_Data {
private:
	cweeSharedPtr<OGRSpatialReference> WGS84SRS;

public:
	GDAL_Data_Local() : WGS84SRS(OGRSpatialReference::GetWGS84SRS(), [](OGRSpatialReference* p) { /* do nothing */ }) {
		GDALAllRegister(); // register all drivers
		CPLPushErrorHandler(CPLQuietErrorHandler); // quiet the errors from GDAL
	};
	virtual ~GDAL_Data_Local() {};
	cweeStr TestGDAL(cweeStr filePath) { // cweeStr filePath = "point.shp";
		{
			auto pDS{ cweeSharedPtr< GDALDataset >(
				(GDALDataset*)GDALOpenEx(
					filePath,
					GDAL_OF_READONLY, // GDAL_OF_ALL, // GDAL_OF_VECTOR
					NULL, NULL, NULL)
				, [](GDALDataset* ptr) { GDALClose((GDALDatasetH)ptr); }
			)};

			if (!pDS) return "failure!";
			
			for (int i = pDS->GetLayerCount() - 1; i >= 0; i--) {
				auto layerP{ cweeSharedPtr< OGRLayer >(pDS->GetLayer(i), [pDS](OGRLayer* p) { /* do nothing */ })};
				if (layerP) {
					auto sourceSR{ cweeSharedPtr< OGRSpatialReference >(layerP->GetSpatialRef(), [layerP](OGRSpatialReference* p) { /* do nothing */ })};													
					auto transform{ cweeSharedPtr< OGRCoordinateTransformation >(OGRCreateCoordinateTransformation(sourceSR.get(), WGS84SRS.get())) };

					if (true) {
						std::cout << layerP->GetName() << std::endl;

						// foreach asset / drawn 'thing'		
						for (int j = layerP->GetFeatureCount() - 1; j >= 0; j--) {
							auto pFeature{ cweeSharedPtr< OGRFeature >(layerP->GetFeature(j)) }; // for whatever reason, these must be deleted. 
							if (pFeature) {
								// foreach column within this row of asset table
								for (AUTO oField : *pFeature) {
									if (oField.IsUnset()) {
										printf("(unset), ");
										continue;
									}
									if (oField.IsNull()) {
										printf("(null), ");
										continue;
									}
									switch (oField.GetType()) {
									case OFTInteger:
										printf("%d, ", oField.GetInteger());
										break;
									case OFTInteger64:
										printf(CPL_FRMT_GIB ",", oField.GetInteger64());
										break;
									case OFTReal:
										printf("%.3f, ", oField.GetDouble());
										break;
									case OFTString:
										// GetString() returns a C string
										printf("%s, ", oField.GetString());
										break;
									default:
										// Note: we use GetAsString() and not GetString(), since
										// the later assumes the field type to be OFTString while the
										// former will do a conversion from the original type to string.
										printf("%s, ", oField.GetAsString());
										break;
									}
								}

								auto poGeometry{ cweeSharedPtr< OGRGeometry >(pFeature->GetGeometryRef(), [pFeature](OGRGeometry* p) { /* do nothing */ }) };
								if (poGeometry) {
									poGeometry->transform(transform.get()); // > transformTo(newRef.get());

									printf("GeometryMode == %i, ", static_cast<int>(wkbFlatten(poGeometry->getGeometryType())));

									if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint) {
										OGRPoint* poPoint = poGeometry->toPoint();
										printf("%.7f,%.7f\n", poPoint->getX(), poPoint->getY());
									}
									else if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString) {
										OGRLineString* poPoint = poGeometry->toLineString();
										int numPoints = poPoint->getNumPoints();
										for (int i = 0; i < numPoints; i++) {
											printf("{%.7f,%.7f}", poPoint->getX(i), poPoint->getY(i));
										}
										printf("\n");
									}
									else {
										printf("geometry mode `%i` not handled\n", static_cast<int>(wkbFlatten(poGeometry->getGeometryType())));
									}
								}
								else {
									printf("no point geometry\n");
								}
							}
						}
					}
				}
			}
				
			
		}
		return "successful!";
	};
};
cweeSharedPtr<GDAL_Data> gdal_data = make_cwee_shared<GDAL_Data_Local>(new GDAL_Data_Local()).CastReference<GDAL_Data>();





