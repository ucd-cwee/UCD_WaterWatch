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

#include <string>
#include <map>

#pragma once
#pragma hdrstop

#include "Precompiled.h"
#include "vec.h"
#include "Strings.h"
#include "List.h"
#include "Parser.h"

BETTER_ENUM(asset_t, uint8_t, JUNCTION, RESERVOIR, PIPE, PUMP, VALVE, ANY, DMA, PUMPSTATION, NANTYPE, TIME);
enum value_t {
	_ANY_ = -1,		// ANY
	_FLOW_ = 0,		// LINK
	_VELOCITY_ = 1,		// LINK
	_HEADLOSS_ = 2,		// LINK
	_STATUS_ = 3,		// LINK
	_ENERGY_ = 4,		// LINK
	_ENERGY_PRICE_ = 5,		// PUMP STATION
	_WATER_PRICE_ = 6,		// LINK
	_COST_ = 7,		// LINK
	_SETTING_ = 8,		// LINK
	_HEAD_ = 9,		// NODE
	_DEMAND_ = 10,		// NODE
	_QUALITY_ = 11,		// NODE
	_MASS_FLOW_ = 12,		// NODE
	_TIME_ = 13,
	_LEVEL_ = 14,
	_EMISSION_INTENSITY_ = 15,
	_END_ = 16
};
enum measurement_t {
	// FLOW & DEMAND
	_cfs_ = 0,
	_gpm_ = 1,

	// VELOCITY
	_fps_ = 2,
	_mph_ = 3,

	// HEADLOSS & HEAD
	_feet_ = 4,
	_meters_ = 5,
	_psi_ = 6,

	// STATUS & SETTING & QUALITY & MASS_FLOW
	_unitless_ = 7,

	// ENERGY
	_kW_ = 8,
	_MW_ = 9,

	// ENERGY_PRICE
	_US_per_kWh_ = 10,

	// WATER_PRICE
	_US_per_gallon_ = 11,

	// COST
	_US_ = 12,

	// EMISSION_INTENSITY
	_GHG_per_MWh_ = 13,

	// ENERGY_PRICE
	_US_per_kW_ = 14,

	_end_ = 15
};

const static std::map<asset_t, const char*> StringMap_asset_t = {
	{asset_t::JUNCTION, "Junction"},
	{asset_t::RESERVOIR, "Reservoir"},
	{asset_t::PIPE, "Pipe"},
	{asset_t::PUMP, "Pump"},
	{asset_t::VALVE, "Valve"},
	{asset_t::ANY, "No Type"},
	{asset_t::DMA, "District Metered Area"},
	{asset_t::PUMPSTATION, "Pump Station"},
	{asset_t::NANTYPE, "NAN"},
	{asset_t::TIME, "Time"}
};
const static std::map<value_t, const char*> StringMap_value_t = {
	{_ANY_, "No Characteristic"},
	{_FLOW_, "Flow"},
	{_VELOCITY_, "Velocity"},
	{_HEADLOSS_, "Headloss"},
	{_STATUS_, "Status"},
	{_ENERGY_, "Energy"},
	{_ENERGY_PRICE_, "Energy Price"},
	{_WATER_PRICE_, "Water Price"},
	{_COST_, "Cost"},
	{_SETTING_, "Setting"},
	{_HEAD_, "Head"},
	{_DEMAND_, "Demand"},
	{_QUALITY_, "Quality"},
	{_MASS_FLOW_, "Mass Flow"},
	{_TIME_, "Time"},
	{_LEVEL_, "Level"},
	{_EMISSION_INTENSITY_, "Emission Intensity"},
	{_END_, "No Characteristic"}
};
const static std::map<measurement_t, const char*> StringMap_measurement_t = {
	{_cfs_, "Cubic Feet per Second"},
	{_gpm_, "Gallons per Minute"},
	{_fps_, "Feet per Second"},
	{_mph_, "Miles per Hour"},
	{_feet_, "Feet"},
	{_meters_, "Meters"},
	{_psi_, "Pounds per Square Inch"},
	{_unitless_, "No Unit"},
	{_kW_, "Kilowatts"},
	{_MW_, "Megawatts"},
	{_US_per_kWh_, "$US / kWh"},
	{_US_per_gallon_, "$US / Gallon"},
	{_US_, "$US"},
	{_GHG_per_MWh_, "Ton CO2e / MWh"},
	{_US_per_kW_, "$US / kW"},
	{_end_, "No Unit"}
};
const static std::map<value_t, measurement_t> defaultMeasurementForValueMap = {
	{_ANY_, _unitless_},
	{_FLOW_, _gpm_},
	{_VELOCITY_, _fps_},
	{_HEADLOSS_, _feet_},
	{_STATUS_, _unitless_},
	{_ENERGY_, _kW_},
	{_ENERGY_PRICE_, _US_per_kWh_},
	{_WATER_PRICE_, _US_per_gallon_},
	{_COST_, _US_},
	{_SETTING_, _unitless_},
	{_HEAD_, _feet_},
	{_DEMAND_, _gpm_},
	{_QUALITY_, _unitless_},
	{_MASS_FLOW_, _unitless_},
	{_TIME_, _unitless_},
	{_LEVEL_, _feet_},
	{_EMISSION_INTENSITY_, _GHG_per_MWh_},
	{_END_, _unitless_}
};

class cweeUnits {
public: // General engineering calculations
	static vec3 GetMadConversion(const measurement_t& from, const measurement_t& to) {
		switch (from) {
		case _cfs_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(7.4805f * 60.0f, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _gpm_: {
			switch (to) {
			case _cfs_: {
				return vec3(1.0f / (7.4805f * 60.0f), 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _fps_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(0.681818f, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _mph_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1.0f / 0.681818f, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _feet_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(0.3048f, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(2.307249f, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _meters_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1.0f / 0.3048f, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(2.307249f / 0.3048f, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _psi_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1.0f / 2.307249f, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(0.3048f / 2.307249f, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _unitless_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _kW_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(0.001f, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _MW_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1.0f / 0.001f, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _US_per_kWh_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _US_per_gallon_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _US_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _GHG_per_MWh_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _US_per_kW_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		case _end_: {
			switch (to) {
			case _cfs_: {
				return vec3(1, 0, 0);
				break;
			}
			case _gpm_: {
				return vec3(1, 0, 0);
				break;
			}
			case _fps_: {
				return vec3(1, 0, 0);
				break;
			}
			case _mph_: {
				return vec3(1, 0, 0);
				break;
			}
			case _feet_: {
				return vec3(1, 0, 0);
				break;
			}
			case _meters_: {
				return vec3(1, 0, 0);
				break;
			}
			case _psi_: {
				return vec3(1, 0, 0);
				break;
			}
			case _unitless_: {
				return vec3(1, 0, 0);
				break;
			}
			case _kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _MW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kW_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_gallon_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_: {
				return vec3(1, 0, 0);
				break;
			}
			case _GHG_per_MWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _US_per_kWh_: {
				return vec3(1, 0, 0);
				break;
			}
			case _end_: {
				return vec3(1, 0, 0);
				break;
			}
			}
			break; }
		}
		return vec3(1, 0, 0);
	};
	static std::vector < value_t >	GetValues(asset_t type = asset_t::ANY) {
		switch (type) {
		case asset_t::JUNCTION: {
			std::vector<value_t> out(JunctionValues.begin(), JunctionValues.end());
			return out;
			break;
		}
		case asset_t::RESERVOIR: {
			std::vector<value_t> out(ReservoirValues.begin(), ReservoirValues.end());
			return out;
			break;
		}
		case asset_t::PIPE: {
			std::vector<value_t> out(PipeValues.begin(), PipeValues.end());
			return out;
			break;
		}
		case asset_t::PUMP: {
			std::vector<value_t> out(PumpValues.begin(), PumpValues.end());
			return out;
			break;
		}
		case asset_t::VALVE: {
			std::vector<value_t> out(ValveValues.begin(), ValveValues.end());
			return out;
			break;
		}
		case asset_t::DMA: {
			std::vector<value_t> out(DmaValues.begin(), DmaValues.end());
			return out;
			break;
		}
		case asset_t::PUMPSTATION: {
			std::vector<value_t> out(PumpStationValues.begin(), PumpStationValues.end());
			return out;
			break;
		}
		default: {
			std::vector<value_t> out(AllValues.begin(), AllValues.end());
			return out;
			break;
		}
		}
	};
	static constexpr std::array < value_t, 14 > AllValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_STATUS_,
		_ENERGY_,
		_ENERGY_PRICE_,
		_WATER_PRICE_,
		_COST_,
		_SETTING_,
		_LEVEL_,
		_HEAD_,
		_DEMAND_,
		_QUALITY_,
		_MASS_FLOW_
	};
	static constexpr std::array < value_t, 4 > JunctionValues{
		_HEAD_,
		_DEMAND_,
		_QUALITY_,
		_MASS_FLOW_
	};
	static constexpr std::array < value_t, 5 > ReservoirValues{
		_LEVEL_,
		_HEAD_,
		_DEMAND_,
		_QUALITY_,
		_MASS_FLOW_
	};
	static constexpr std::array < value_t, 4 > PipeValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_STATUS_
	};
	static constexpr std::array < value_t, 6 > PumpValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_SETTING_,
		_STATUS_,
		_ENERGY_
	};
	static constexpr std::array < value_t, 5 > ValveValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_SETTING_,
		_STATUS_
	};
	static constexpr std::array < value_t, 3 > DmaValues{
		_DEMAND_,
		_HEAD_,
		_FLOW_
	};
	static constexpr std::array < value_t, 4 > PumpStationValues{
		_FLOW_,
		_ENERGY_,
		_ENERGY_PRICE_,
		_EMISSION_INTENSITY_
	};
};

static std::tuple<asset_t, value_t, measurement_t>	GuessScadaTagPathCharacteristics(const cweeStr& in) {

#if 0
	// Step 1. Assumption, remove special characters. 
	cweeStr tagpath = in;
	cweeThreadedList<cweeStr> temp({
		"~", "}", "|", "{", "`", "_", "^", "]", "[",
		"/", "\\", "@", "?", ">", "<", ",", ";", ":",
		".", "-", "+", "*", ")", "(", "'", "&", "%", "$", "#",
		"\"", "!", "  "
		});
	for (auto& x : temp) { tagpath.ReplaceInline(x, " "); }
	tagpath.ReplaceInline("\t", " ");
	tagpath.ReplaceInline("\r", " ");
	tagpath.ReplaceInline("\n", " ");

	cweeThreadedList<cweeStr> sources; int performance = cweeMath::INF;
	for (auto& a_t : cweeAsset::GetAssets()) {
		for (auto& v_t : cweeAsset::GetValues()) {
			for (auto& m_t : cweeAsset::GetMeasurements()) {
				sources.Append(GetString(a_t) + "+" + GetString(v_t) + "+" + GetString(m_t));
			}
		}
	}

	cweeStr bestFit = tagpath.BestMatch(sources);
	asset_t a_t2;	value_t v_t2;	measurement_t m_t2;
	cweeParser x(bestFit, "+", true);
	FromString(a_t2, x[0]); FromString(v_t2, x[1]); FromString(m_t2, x[2]);
	std::tuple<asset_t, value_t, measurement_t> out = std::make_tuple(a_t2, v_t2, m_t2);
	return out;

#else

	asset_t a_t;		value_t v_t;		measurement_t m_t;

	// value_t
	{
		if (in.Find("LEVEL", false, 0) != -1) v_t = _LEVEL_;
		else if (in.Find("RATE SCHEDULE", false, 0) != -1) v_t = _ENERGY_PRICE_;
		else if (in.Find("PUMP RUN", false, 0) != -1) v_t = _STATUS_;
		else if (in.Find("KILOWATT", false, 0) != -1) v_t = _ENERGY_;
		else if (in.Find("PRESSURE", false, 0) != -1) v_t = _HEAD_;
		else if (in.Find("VELOCITY", false, 0) != -1) v_t = _VELOCITY_;
		else if (in.Find("HEADLOSS", false, 0) != -1) v_t = _HEADLOSS_;
		else if (in.Find("RESIDUAL", false, 0) != -1) v_t = _QUALITY_;
		else if (in.Find("CHLORINE", false, 0) != -1) v_t = _QUALITY_;
		else if (in.Find("POSITION", false, 0) != -1) v_t = _SETTING_;
		else if (in.Find("AMMONIA", false, 0) != -1) v_t = _QUALITY_;
		else if (in.Find("PERCENT", false, 0) != -1) v_t = _SETTING_;
		else if (in.Find("STATUS", false, 0) != -1) v_t = _STATUS_;
		else if (in.Find("ALARM", false, 0) != -1) v_t = _STATUS_;
		else if (in.Find("SPEED", false, 0) != -1) v_t = _SETTING_;
		else if (in.Find("KWATT", false, 0) != -1) v_t = _ENERGY_;
		else if (in.Find("FLOW", false, 0) != -1) v_t = _FLOW_;
		else if (in.Find("HEAD", false, 0) != -1) v_t = _HEAD_;
		else if (in.Find("RATE", false, 0) != -1) v_t = _ENERGY_PRICE_;
		else if (in.Find("RUN", false, 0) != -1) v_t = _STATUS_;
		else if (in.Find("CFS", false, 0) != -1) v_t = _FLOW_;
		else if (in.Find("GPM", false, 0) != -1) v_t = _FLOW_;
		else if (in.Find("KW", false, 0) != -1) v_t = _ENERGY_;
		else v_t = _QUALITY_;
	}

	// measurement_t
	{
		if (v_t == _ANY_ || v_t == _STATUS_ || v_t == _SETTING_ || v_t == _QUALITY_ || v_t == _MASS_FLOW_) m_t = _unitless_;
		else if (v_t == _FLOW_ || v_t == _DEMAND_) {
			if (in.Find("cfs", false, 0) != -1) m_t = _cfs_;
			else m_t = _gpm_;
		}
		else if (v_t == _VELOCITY_) {
			if (in.Find("mph", false, 0) != -1) m_t = _mph_;
			else m_t = _fps_;
		}
		else if (v_t == _HEADLOSS_ || v_t == _HEAD_ || v_t == _LEVEL_) {
			if (in.Find("feet", false, 0) != -1) m_t = _feet_;
			else if (in.Find("meters", false, 0) != -1) m_t = _meters_;
			else if (in.Find("level", false, 0) != -1) m_t = _feet_;
			else if (in.Find("pressure", false, 0) != -1) m_t = _psi_;
			else m_t = _psi_;
		}
		else if (v_t == _ENERGY_) {
			if (in.Find("kw", false, 0) != -1) m_t = _kW_;
			else if (in.Find("mw", false, 0) != -1) m_t = _MW_;
			else m_t = _kW_;
		}
		else if (v_t == _ENERGY_PRICE_) {
			m_t = _US_per_kWh_;
		}
		else if (v_t == _WATER_PRICE_) {
			m_t = _US_per_gallon_;
		}
		else if (v_t == _COST_) {
			m_t = _US_;
		}
		else m_t = _unitless_;
	}

	// asset_t
	{
		if (in.Find("LEVEL", false, 0) != -1) a_t = asset_t::RESERVOIR;
		else if (in.Find("RATE SCHEDULE", false, 0) != -1) a_t = asset_t::PUMPSTATION;
		else if (in.Find("PUMP RUN", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("KILOWATT", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("PRESSURE", false, 0) != -1) a_t = asset_t::RESERVOIR;
		else if (in.Find("VELOCITY", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("HEADLOSS", false, 0) != -1) a_t = asset_t::VALVE;
		else if (in.Find("RESIDUAL", false, 0) != -1) a_t = asset_t::RESERVOIR;
		else if (in.Find("CHLORINE", false, 0) != -1) a_t = asset_t::RESERVOIR;
		else if (in.Find("POSITION", false, 0) != -1) a_t = asset_t::VALVE;
		else if (in.Find("AMMONIA", false, 0) != -1) a_t = asset_t::RESERVOIR;
		else if (in.Find("PERCENT", false, 0) != -1) a_t = asset_t::RESERVOIR;
		else if (in.Find("STATUS", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("ALARM", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("SPEED", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("KWATT", false, 0) != -1) a_t = asset_t::PUMPSTATION;
		else if (in.Find("FLOW", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("HEAD", false, 0) != -1) a_t = asset_t::RESERVOIR;
		else if (in.Find("RATE", false, 0) != -1) a_t = asset_t::PUMPSTATION;
		else if (in.Find("RUN", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("CFS", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("GPM", false, 0) != -1) a_t = asset_t::PUMP;
		else if (in.Find("KW", false, 0) != -1) a_t = asset_t::PUMPSTATION;
		else a_t = asset_t::RESERVOIR;
	}

	bool found = false;
	for (auto& x : cweeUnits::GetValues(a_t)) if (x == v_t) found = true;
	if (found == false) { v_t = _ANY_; m_t = measurement_t::_unitless_; }

	std::tuple<asset_t, value_t, measurement_t> out = std::make_tuple(a_t, v_t, m_t);
	return out;
#endif
};

enum class controlGenerationStatus {
	notStarted,
	started,
	finished,
	controlGenerationStatusEnd
};
enum class AssetValueControlType {
	Static,
	ManualPredicted,
	Deactivated,
	AutomatedWithoutOverride,
	AssetValueControlTypeEnd
};
enum class ControlGenerationMethod {
	UserInput,
	TrainedModel,
	Optimization,
	PID,
	ControlGenerationMethodEnd
};
enum class UserInput_InputType {
	Rule,
	Value, //, PID
	UserInput_InputTypeEnd
};
enum class TrainedModel_ErrorMetric {
	MicroAveraged_F1Score,
	R_Squared,
	TrainedModel_ErrorMetricEnd
};
enum class TrainedModel_Modifier {
	None,
	DayOfWeek,
	HourOfDay,
	Velocity,
	Acceleration,
	MovingAverage,
	Normalize,
	TrainedModel_ModifierEnd
};
enum class Optimization_SetPoint {
	KeepOriginal,
	Optimize,
	Exclude,
	Optimization_SetPointEnd
};
enum class Optimization_Value {
	SystemEnergy,
	EnergyCost,
	CarbonEmissions,
	WaterStorage,
	SystemPressure,
	PumpSwitches,
	AveragePercentOperationalStorage,
	WaterDemand,
	Optimization_ValueEnd
};
enum class Optimization_Aggregation {
	Minimum,
	Average,
	Maximum,
	Sum,
	Count,
	Optimization_AggregationEnd
};
enum class Optimization_Comparison {
	EqualsTo,
	NotEqual,
	GreaterThan,
	GreaterThanOrEqual,
	LessThan,
	LessThanOrEqual,
	Optimization_ComparisonEnd
};
enum class conversion_t {
	SITE,
	LONGITUDE,
	LATITUDE,
	ELEVATION,
	CONSTANT,
	TIMESERIES,
	MEASUREMENT,
	DESIGN,
	TYPE,
	NEW,
	DATA,
	SETTING,
	OPTION,
	SIMULATION,
	JUNCTION,
	RESERVOIR,
	PIPE,
	PUMP,
	VALVE,
	DMA,
	PUMPSTATION,
	OTHER,
	SNAPLEFT,
	SNAPRIGHT,
	LINEAR,
	SPLINE,
	CLAMP,
	LOOP,
	DEMAND_PATTERN,
	MIN_LEVEL,
	MAX_LEVEL,
	DIAMETER,
	TERMINAL_STORAGE,
	HEAD_PATTERN,
	VOLUME_CURVE,
	LEVEL,
	LENGTH,
	ROUGHNESS,
	MINORLOSS,
	CHECK_VALVE,
	STARTING_SITE,
	ENDING_SITE,
	PIPE_VERTEX,
	STATUS,
	HEAD_CURVE,
	EFFICIENCY_CURVE,
	AVG_PERCENT_EFFICIENCY,
	POWER,
	VALVE_TYPE,
	FORCE_OPEN,
	EMBEDDED_SITE,
	EMISSION_PATTERN,
	ENERGYPRICE_PATTERN,
	UPSTREAM_DMA,
	DOWNSTREAM_DMA,
	EMBEDDED_PUMP,
	UNITS,
	CALIBRATION_DATE,
	SIMULATION_DURATION,
	HEADLOSS,
	SPECIFIC_GRAVITY,
	VISCOSITY,
	TRIALS,
	ACCURACY,
	CHECKFREQ,
	MAXCHECK,
	DAMPLIMIT,
	UNBALANCED,
	PATTERN,
	DEMANDMULTIPLIER,
	EMITTEREXPONENT,
	QUALITY,
	DIFFUSIVITY,
	TOLERANCE,
	FLOW_WITHIN,
	FLOW_OUT,
	FLOW_IN,
	CONTROL,
	conversion_tEnd
};

const static std::map<conversion_t, const char*> StringMap_conversion_t = {
	{conversion_t::SITE, "SITE"},
	{conversion_t::LONGITUDE, "LONGITUDE"},
	{conversion_t::LATITUDE, "LATITUDE"},
	{conversion_t::ELEVATION, "ELEVATION"},
	{conversion_t::CONSTANT, "CONSTANT"},
	{conversion_t::TIMESERIES, "TIMESERIES"},
	{conversion_t::MEASUREMENT, "MEASUREMENT"},
	{conversion_t::DESIGN, "DESIGN"},
	{conversion_t::TYPE, "TYPE"},
	{conversion_t::NEW, "NEW"},
	{conversion_t::DATA, "DATA"},
	{conversion_t::SETTING, "SETTING"},
	{conversion_t::OPTION, "OPTION"},
	{conversion_t::SIMULATION, "SIMULATION"},
	{conversion_t::JUNCTION, "Junction"},
	{conversion_t::RESERVOIR, "Reservoir"},
	{conversion_t::PIPE, "Pipe"},
	{conversion_t::PUMP, "Pump"},
	{conversion_t::VALVE, "Valve"},
	{conversion_t::DMA, "District Metered Area"},
	{conversion_t::PUMPSTATION, "Pump Station"},
	{conversion_t::OTHER, "OTHER"},
	{conversion_t::SNAPLEFT, "SNAPLEFT"},
	{conversion_t::SNAPRIGHT, "SNAPRIGHT"},
	{conversion_t::LINEAR, "LINEAR"},
	{conversion_t::SPLINE, "SPLINE"},
	{conversion_t::CLAMP, "CLAMP"},
	{conversion_t::LOOP, "LOOP"},
	{conversion_t::DEMAND_PATTERN, "DEMAND_PATTERN"},
	{conversion_t::MIN_LEVEL, "MIN_LEVEL"},
	{conversion_t::MAX_LEVEL, "MAX_LEVEL"},
	{conversion_t::DIAMETER, "DIAMETER"},
	{conversion_t::TERMINAL_STORAGE, "TERMINAL_STORAGE"},
	{conversion_t::HEAD_PATTERN, "HEAD_PATTERN"},
	{conversion_t::VOLUME_CURVE, "VOLUME_CURVE"},
	{conversion_t::LEVEL, "LEVEL"},
	{conversion_t::LENGTH, "LENGTH"},
	{conversion_t::ROUGHNESS, "ROUGHNESS"},
	{conversion_t::MINORLOSS, "MINORLOSS"},
	{conversion_t::CHECK_VALVE, "CHECK_VALVE"},
	{conversion_t::STARTING_SITE, "STARTING_SITE"},
	{conversion_t::ENDING_SITE, "ENDING_SITE"},
	{conversion_t::PIPE_VERTEX, "PIPE_VERTEX"},
	{conversion_t::STATUS, "STATUS"},
	{conversion_t::HEAD_CURVE, "HEAD_CURVE"},
	{conversion_t::EFFICIENCY_CURVE, "EFFICIENCY_CURVE"},
	{conversion_t::AVG_PERCENT_EFFICIENCY, "AVG_PERCENT_EFFICIENCY"},
	{conversion_t::POWER, "POWER"},
	{conversion_t::VALVE_TYPE, "VALVE_TYPE"},
	{conversion_t::FORCE_OPEN, "FORCE_OPEN"},
	{conversion_t::EMBEDDED_SITE, "EMBEDDED_SITE"},
	{conversion_t::EMISSION_PATTERN, "EMISSION_PATTERN"},
	{conversion_t::ENERGYPRICE_PATTERN, "ENERGYPRICE_PATTERN"},
	{conversion_t::UPSTREAM_DMA, "UPSTREAM_DMA"},
	{conversion_t::DOWNSTREAM_DMA, "DOWNSTREAM_DMA"},
	{conversion_t::EMBEDDED_PUMP, "EMBEDDED_PUMP"},
	{conversion_t::UNITS, "UNITS"},
	{conversion_t::CALIBRATION_DATE, "CALIBRATION_DATE"},
	{conversion_t::SIMULATION_DURATION, "SIMULATION_DURATION"},
	{conversion_t::HEADLOSS, "HEADLOSS"},
	{conversion_t::SPECIFIC_GRAVITY, "SPECIFIC_GRAVITY"},
	{conversion_t::VISCOSITY, "VISCOSITY"},
	{conversion_t::TRIALS, "TRIALS"},
	{conversion_t::ACCURACY, "ACCURACY"},
	{conversion_t::CHECKFREQ, "CHECKFREQ"},
	{conversion_t::MAXCHECK, "MAXCHECK"},
	{conversion_t::DAMPLIMIT, "DAMPLIMIT"},
	{conversion_t::UNBALANCED, "UNBALANCED"},
	{conversion_t::PATTERN, "PATTERN"},
	{conversion_t::DEMANDMULTIPLIER, "DEMANDMULTIPLIER"},
	{conversion_t::EMITTEREXPONENT, "EMITTEREXPONENT"},
	{conversion_t::QUALITY, "QUALITY"},
	{conversion_t::DIFFUSIVITY, "DIFFUSIVITY"},
	{conversion_t::TOLERANCE, "TOLERANCE"},
	{conversion_t::FLOW_WITHIN, "FLOW_WITHIN"},
	{conversion_t::FLOW_OUT, "FLOW_OUT"},
	{conversion_t::FLOW_IN, "FLOW_IN"},
	{conversion_t::CONTROL, "CONTROL"}
};
const static std::map<controlGenerationStatus, const char*> StringMap_controlGenerationStatus = {
	{controlGenerationStatus::notStarted, "Not Started"},
	{controlGenerationStatus::started, "Started"},
	{controlGenerationStatus::finished, "Finished"}
};
const static std::map<AssetValueControlType, const char*> StringMap_AssetValueControlType = {
	{AssetValueControlType::Static, "Active"},
	{AssetValueControlType::ManualPredicted, "Manual Predicted"},
	{AssetValueControlType::Deactivated, "Deactivated"},
	{AssetValueControlType::AutomatedWithoutOverride, "Active without Overrides"}
};
const static std::map<ControlGenerationMethod, const char*> StringMap_ControlGenerationMethod = {
	{ControlGenerationMethod::UserInput, "PLC Rules"},
	{ControlGenerationMethod::TrainedModel, "Trained ML Model"},
	{ControlGenerationMethod::Optimization, "Optimized Rules"},
	{ControlGenerationMethod::PID, "PID Controller"}
};
const static std::map<UserInput_InputType, const char*> StringMap_UserInput_InputType = {
	{UserInput_InputType::Rule, "Rule"},
	{UserInput_InputType::Value, "Value"}
};
const static std::map<TrainedModel_ErrorMetric, const char*> StringMap_TrainedModel_ErrorMetric = {
	{TrainedModel_ErrorMetric::MicroAveraged_F1Score, "F1 Score"},
	{TrainedModel_ErrorMetric::R_Squared, "R² Fit"}
};
const static std::map<TrainedModel_Modifier, const char*> StringMap_TrainedModel_Modifier = {
	{TrainedModel_Modifier::None, "No Modification"},
	{TrainedModel_Modifier::DayOfWeek, "Day of the Week"},
	{TrainedModel_Modifier::HourOfDay, "Hour of the Day"},
	{TrainedModel_Modifier::Velocity, "Velocity"},
	{TrainedModel_Modifier::Acceleration, "Acceleration"},
	{TrainedModel_Modifier::MovingAverage, "Moving Average"},
	{TrainedModel_Modifier::Normalize, "Normalized (0 - 1)"}
};
const static std::map<Optimization_SetPoint, const char*> StringMap_Optimization_SetPoint = {
	{Optimization_SetPoint::KeepOriginal, "Keep Original"},
	{Optimization_SetPoint::Optimize, "Optimize"},
	{Optimization_SetPoint::Exclude, "Exclude"}
};
const static std::map<Optimization_Value, const char*> StringMap_Optimization_Value = {
	{Optimization_Value::WaterDemand, "Water Demand"},
	{Optimization_Value::AveragePercentOperationalStorage, "Normalized Water Storage"},
	{Optimization_Value::SystemEnergy, "System Energy"},
	{Optimization_Value::EnergyCost, "Energy Cost"},
	{Optimization_Value::CarbonEmissions, "Carbon Emissions"},
	{Optimization_Value::WaterStorage, "Water Storage"},
	{Optimization_Value::SystemPressure, "System Pressure"},
	{Optimization_Value::PumpSwitches, "Pump Switches"}
};
const static std::map<Optimization_Aggregation, const char*> StringMap_Optimization_Aggregation = {
	{Optimization_Aggregation::Minimum, "Minimum"},
	{Optimization_Aggregation::Average, "Average"},
	{Optimization_Aggregation::Maximum, "Maximum"},
	{Optimization_Aggregation::Sum, "Sum"},
	{Optimization_Aggregation::Count, "Count"}
};
const static std::map<Optimization_Comparison, const char*> StringMap_Optimization_Comparison = {
	{Optimization_Comparison::EqualsTo, "=="},
	{Optimization_Comparison::NotEqual, "!="},
	{Optimization_Comparison::GreaterThan, ">"},
	{Optimization_Comparison::GreaterThanOrEqual, ">="},
	{Optimization_Comparison::LessThan, "<"},
	{Optimization_Comparison::LessThanOrEqual, "<="}
};


template <typename enumType>
const static std::map<enumType, const char*>& StaticStringMap() {
	return StaticStringMap(typenames::identity<enumType>());
};

template <typename enumType>
const static std::map<enumType, const char*>& StaticStringMap(typenames::identity<enumType>) { throw std::runtime_error("User must provide an explicit (full) template specialization."); };

#define StaticStringMapDecl(enumType) const static std::map<enumType, const char*>& StaticStringMap(typenames::identity<enumType>) { return StringMap_##enumType; }
StaticStringMapDecl(conversion_t);
StaticStringMapDecl(controlGenerationStatus);
StaticStringMapDecl(AssetValueControlType);
StaticStringMapDecl(ControlGenerationMethod);
StaticStringMapDecl(UserInput_InputType);
StaticStringMapDecl(TrainedModel_ErrorMetric);
StaticStringMapDecl(TrainedModel_Modifier);
StaticStringMapDecl(Optimization_SetPoint);
StaticStringMapDecl(Optimization_Aggregation);
StaticStringMapDecl(Optimization_Comparison);
StaticStringMapDecl(asset_t);
StaticStringMapDecl(value_t);
StaticStringMapDecl(measurement_t);

template<typename enumType>
static cweeStr GetString(const enumType& object) {
	return StaticStringMap<enumType>().at(object);
};
template<typename enumType>
static void FromString(enumType& out, const cweeStr& source) {
	for (auto& it : StaticStringMap<enumType>()) {
		if (cweeStr(it.second) == source) {
			out = it.first;
			return;
		}
	}

	// not found. Ensure something is always found.
	cweeThreadedList<cweeStr> options;
	for (auto& it : StaticStringMap<enumType>()) options.Append(it.second);
	cweeStr bestMatch = source.BestMatch(options);
	for (auto& it : StaticStringMap<enumType>()) if (cweeStr(it.second) == bestMatch) out = it.first;
};
