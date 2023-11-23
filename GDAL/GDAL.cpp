// TODO: This is an example of a library function
#pragma once
#include "GDAL.h"
#include "include/gdal.h"
#include "include/gdal_priv.h"
#include "include/ogrsf_frmts.h"

template <typename T> INLINE cweeSharedPtr<void> ToVoidPtr(T& ptr) {
	return cweeSharedPtr<void>(make_cwee_shared<T>(ptr), [](void* p) { return p; });
};
template <typename T> INLINE cweeSharedPtr<T> FromVoidPtr(cweeSharedPtr<void> data) {
	return cweeSharedPtr<T>(data, [](void* p) { return static_cast<T*>(p); });
};

class GDAL_Data_Local final : public GDAL_Data {
public:
	GDAL_Data_Local() {
		GDALAllRegister(); // register all drivers
	};
	virtual ~GDAL_Data_Local() {};
	cweeStr TestGDAL(cweeStr filePath) { // cweeStr filePath = "point.shp";
		{
			// std::shared_ptr<GDALDataset> pDS = GDALDatasetUniquePtr((GDALDataset*)GDALOpenShared(filePath, GDALAccess::GA_ReadOnly));

			std::shared_ptr<GDALDataset> pDS = GDALDatasetUniquePtr((GDALDataset*)GDALOpenEx(
				filePath,
				GDAL_OF_READONLY, // GDAL_OF_ALL, // GDAL_OF_VECTOR
				NULL, NULL, NULL)
			);

			if (!pDS) {
				return "failure!";
			}
			else {
				OGRLayerUniquePtr pLayer;
				pLayer = OGRLayerUniquePtr(pDS->GetLayer(0));
				AUTO layers = pDS->GetLayers();
				for (auto layerP : layers) {
					std::cout << layerP->GetName() << std::endl;

					for (auto& pFeature : layerP)
					{
						//AUTO voidPtr = ToVoidPtr<OGRFeatureUniquePtr>(pFeature);
						//AUTO returned = FromVoidPtr<OGRFeatureUniquePtr>(voidPtr);

						//AUTO fieldSharedObj = cweeGeo::Field(ToVoidPtr<OGRFeatureUniquePtr>(pFeature));

						for (auto&& oField : *pFeature)
						{
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

						OGRGeometry* poGeometry;

						poGeometry = pFeature->GetGeometryRef();

						//sourceSR = layerP->GetSpatialRef();
						//targetSR = osr.SpatialReference();
						//targetSR.ImportFromEPSG(4326);// # WGS84
						//coordTrans = osr.CoordinateTransformation(sourceSR, targetSR);
						//geom.Transform(coordTrans);

						if (poGeometry != NULL) {
							printf("GeometryMode == %i, ", static_cast<int>(wkbFlatten(poGeometry->getGeometryType())));

							if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint) {
								OGRPoint* poPoint = poGeometry->toPoint();
								printf("%.3f,%3.f\n", poPoint->getX(), poPoint->getY());
							}
							else if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString) {
								OGRLineString* poPoint = poGeometry->toLineString();
								int numPoints = poPoint->getNumPoints();
								for (int i = 0; i < numPoints; i++) {
									printf("{%.3f,%3.f}", poPoint->getX(i), poPoint->getY(i));
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

			pDS = nullptr;
		}
		return "successful!";
	};
};
cweeSharedPtr<GDAL_Data> gdal_data = make_cwee_shared<GDAL_Data_Local>(new GDAL_Data_Local()).CastReference<GDAL_Data>();





