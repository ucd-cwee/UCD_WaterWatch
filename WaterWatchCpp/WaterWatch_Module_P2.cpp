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
#include "WaterWatch_Module_P2.h"

#include "Engineering.h"
#include "cweeScheduler.h"
#include "FileSystemH.h"
#include "Geocoding.h"
#include "Toasts.h" // queue for "toasts" or messages from anywhere in the app. Acts as a message repo. 
#include "BalancedPattern.h"
#include "cweeUnitedValue.h"
#include "enum.h"
#include "cweeUnitPattern.h"
#include "ExternData.h"
#include "AlgLibWrapper.h"
#include "odbc.h"
#include "RTree.h"

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_2() {
            auto lib = chaiscript::make_shared<Module>();

            // Units
            if (1) {
                AddUnit_t(scalar_t);
                AddUnit_t(inch_t);
                AddUnit_t(foot_t);
                AddUnit_t(meter_t);
                AddUnit_t(kilowatt_t);
                AddUnit_t(kilowatt_hour_t);
                AddUnit_t(square_foot_t);
                AddUnit_t(gallon_t);
                AddUnit_t(gallon_per_minute_t);
                AddUnit_t(gallon_per_year_t);                
                AddUnit_t(feet_per_second_t);
                AddUnit_t(feet_per_hour_t);
                AddUnit_t(pounds_per_square_inch_t);
                AddUnit_t(cubic_foot_t);
                AddUnit_t(cubic_foot_per_second_t);
                AddUnit_t(second_t);
                AddUnit_t(millisecond_t);
                AddUnit_t(hour_t);
                AddUnit_t(Dollar_t);
                AddUnit_t(acre_foot_per_year_t);
                AddUnit_t(mile_t);
                AddUnit_t(Dollar_per_gallon_t);

                AddUnit_t(kilowatt_hour_per_acre_foot_t);
                AddUnit_t(Dollar_per_mile_t);
                AddUnit_t(Dollar_per_ton_t);
                AddUnit_t(ton_per_kilowatt_hour_t);
                AddUnit_t(per_year_t);
                AddUnit_t(acre_foot_t);

                AddUnit_t(year_t);
            }

            // Engineering
            if (1) {
                using namespace cwee_units;
                lib->AddFunction(, CentrifugalPumpEnergyDemand, , return cweeEng::CentrifugalPumpEnergyDemand_kW(Flow_gpm, Head_feet, Efficiency_percent), gallon_per_minute_t Flow_gpm, foot_t Head_feet, scalar_t Efficiency_percent);
                lib->AddFunction(, SurfaceAreaCircle, , return cweeEng::SurfaceAreaCircle_ft2(Diameter_feet), foot_t Diameter_feet);
                lib->AddFunction(, VolumeCylinder, , return cweeEng::VolumeCylinder_gal(Diameter_feet, Height_feet), foot_t Diameter_feet, foot_t Height_feet);
                lib->AddFunction(, Cylinder_FlowRate_to_LevelRate, , return cweeEng::Cylinder_FlowRate_to_LevelRate_fph(FlowRate_gpm, Diameter_feet), gallon_per_minute_t FlowRate_gpm, foot_t Diameter_feet);

                lib->AddFunction(, Cylinder_Volume_to_Level, , return cweeEng::Cylinder_Volume_to_Level_f(volume, Diameter_feet), cubic_foot_t volume, foot_t Diameter_feet);
                lib->AddFunction(, Head_to_Pressure, , return cweeEng::Head_to_Pressure_psi(HydraulicHead_feet, BaseElevation_feet), foot_t HydraulicHead_feet, foot_t BaseElevation_feet);
                lib->AddFunction(, ReynoldsNumberInPipe, , return cweeEng::ReynoldsNumberInPipe(velocity_ftPerSec, diameter_inches), feet_per_second_t velocity_ftPerSec, inch_t diameter_inches);

                lib->AddFunction(, ReynoldsNumberInPipe, , return cweeEng::ReynoldsNumberInPipe(velocity_ftPerSec, diameter_inches, kinematicViscosity_ft2PerSec), feet_per_second_t velocity_ftPerSec, inch_t diameter_inches, float kinematicViscosity_ft2PerSec);
                lib->AddFunction(, EquivalentPipeRoughness, , return cweeEng::Cylinder_Volume_to_Level_f(volume, Diameter_feet), cubic_foot_t volume, foot_t Diameter_feet);
                
                lib->add(chaiscript::fun(&cweeEng::EquivalentPipeRoughness, 
                    { "desiredDiameter", "pipe1_length", "pipe2_length", "pipe1_diameter", "pipe2_diameter", "pipe1_roughness", "pipe2_roughness" }
                ), "EquivalentPipeRoughness");

            }

            // Vec2d
            if (1) {
                DEF_DECLARE_PAIR(double, double);
                lib->add(chaiscript::user_type<vec2d>(), "vec2d");
                lib->add(chaiscript::constructor<vec2d()>(), "vec2d");
                lib->add(chaiscript::constructor<vec2d(double const&, double const&)>(), "vec2d");
                lib->add(chaiscript::constructor<vec2d(const vec2d&)>(), "vec2d");
                lib->add(chaiscript::fun([](vec2d& a, const vec2d& b)->vec2d& { a = b; return a; }), "=");
                lib->AddFunction(, first, -> double&, return obj.x, vec2d& obj);
                lib->AddFunction(, second, -> double&, return obj.y, vec2d& obj);
                lib->AddFunction(, Pair, , SINGLE_ARG(return std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value>(var((double)(obj.x)), var((double)(obj.y)));), vec2d const& obj);
                lib->AddFunction(, vec2d, , return vec2d(chaiscript::boxed_cast<double>(obj.first), chaiscript::boxed_cast<double>(obj.second)), std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value> const& obj);
                lib->AddFunction(, [], -> double&, return obj[i], vec2d& obj, int i);
            }

            // Geocoding
            if (1) {
                lib->add(chaiscript::fun([](double X, double Y) { return cwee_units::length::foot_t(geocoding->GetElevation(vec2d(X, Y))); }, {"Longitude", "Latitude"}), "GetElevation");
                lib->add(chaiscript::fun([](cweeStr const& address) { auto v = geocoding->GetLongLat(address); return cweePair<double, double>(v.x, v.y); }, { "Address" }), "GetLongLat");
               
                lib->AddFunction(, GetLongLat, SINGLE_ARG(->cweePair<double, double>) , SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel, LatOrigin, FalseNorthing, FalseEasting); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel, double LatOrigin, double FalseNorthing, double FalseEasting);

                lib->AddFunction(, GetLongLat, SINGLE_ARG(->cweePair<double, double>), SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel, LatOrigin, FalseNorthing); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel, double LatOrigin, double FalseNorthing);

                lib->AddFunction(, GetLongLat, SINGLE_ARG(->cweePair<double, double>), SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel, LatOrigin); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel, double LatOrigin);

                lib->AddFunction(, GetLongLat, SINGLE_ARG(->cweePair<double, double>), SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel);

                lib->AddFunction(, GetLongLat, SINGLE_ARG(->cweePair<double, double>), SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel);

                lib->AddFunction(, GetLongLat, SINGLE_ARG(->cweePair<double, double>), SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian);

                lib->AddFunction(, GetLongLat, SINGLE_ARG(->cweePair<double, double>), SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing()); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing);

                lib->add(chaiscript::fun([](double X, double Y) { return geocoding->GetAddress(vec2d(X, Y)); }, { "Longitude", "Latitude" }), "GetAddress");
                lib->add(chaiscript::fun([](double X1, double Y1, double X2, double Y2) { return geocoding->Distance(vec2d(X1, Y1), vec2d(X2, Y2)); }, { "Longitude1", "Latitude1", "Longitude2", "Latitude2" }), "Distance");
            }

            // Patterns
            if (1) {
                using namespace cweeUnitValues;
                
                lib->add(chaiscript::user_type<cweeUnitPattern>(), "Pattern");
                lib->add(chaiscript::constructor<cweeUnitPattern()>(), "Pattern");
                lib->add(chaiscript::constructor<cweeUnitPattern(const cweeUnitPattern&)>(), "Pattern");
                lib->add(chaiscript::constructor<cweeUnitPattern(const unit_value&, const unit_value&)>(), "Pattern");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, cweeUnitPattern& b) { a = b; return a; }), "=");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const std::string& b) {
                    a.Clear();
                    cweeParser knots(b.c_str(), ", ", true);
                    cweeParser knotP;
                    for (auto& knot : knots) {
                        knotP.ParseFirstDelimiterOnly(knot, "|");
                        if (knotP.getNumVars() >= 2) {
                            a.AddValue((double)knotP[0], (double)knotP[1]);
                        }
                    }
                    return a;
                    }), "=");
                lib->add(chaiscript::fun([](std::string& a, const cweeUnitPattern& b) {
                    cweeStr out;
                    for (auto& x : b.GetKnotSeries()) {
                        out.AddToDelimiter(cweeStr::printf("%s|%s", cweeStr(x.first()).c_str(), cweeStr(x.second()).c_str()), ", ");
                    }
                    a = out.c_str();
                    return a;
                    }), "=");

                ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(interpolation_t, interpolation_t);
                lib->AddFunction(, SetInterpolationType, , a.SetInterpolationType(interpType); , cweeUnitPattern& a, const interpolation_t& interpType);
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetInterpolationType(); }), "GetInterpolationType");

                ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(boundary_t, boundary_t);
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const boundary_t& b) { a.SetBoundaryType(b); }), "SetBoundaryType");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetBoundaryType(); }), "GetBoundaryType");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.AddValue(b, c); }), "AddValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.AddUniqueValue(b, c); }), "AddUniqueValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { a.Clear(); }), "Clear");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { a.ClearData(); }), "ClearData");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { return a.GetCurrentValue(b); }), "GetCurrentValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { return a.GetCurrentFirstDerivative(b); }), "GetCurrentFirstDerivative");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { return a.GetCurrentSecondDerivative(b); }), "GetCurrentSecondDerivative");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetFirstDerivative(); }), "GetFirstDerivative");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetDistances(); }), "GetDistances");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, bool normalized) { return a.GetDistances(normalized); }), "GetDistances");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetRTree(); }), "RTree");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, bool normalized) { return a.GetRTree(normalized); }), "RTree");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMinValue(); }), "GetMinValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetAvgValue(); }), "GetAvgValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMaxValue(); }), "GetMaxValue");

                lib->add(chaiscript::fun([](cweeUnitPattern const& a) { return a.Ceiling(); }), "Ceiling");
                lib->add(chaiscript::fun([](cweeUnitPattern const& a) { return a.Floor(); }), "Floor");
                lib->add(chaiscript::fun([](cweeUnitPattern const& a) { return a.Abs(); }), "Abs");
                lib->add(chaiscript::fun([](cweeUnitPattern const& a, float roundToMagnitude) { return a.RoundNearest(roundToMagnitude); }), "RoundNearest");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.GetMinValue(b, c); }), "GetMinValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.GetAvgValue(b, c); }), "GetAvgValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.GetMaxValue(b, c); }), "GetMaxValue");

                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMinimumTimeStep(); }), "GetMinimumTimeStep");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { a.ShiftTime(b); return a; }), "ShiftTime");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { a.Translate(b); return a; }), "Translate");

                lib->add(chaiscript::fun([](cweeUnitPattern& a)->cweeUnitPattern& { a.RemoveUnnecessaryKnots(); return a; }), "RemoveUnnecessaryKnots");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& min, const unit_value& max) { a.ClampValues(min, max); }), "ClampValues");

                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ 0.01, 0.05, 0.5, 0.95, 0.99 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1, q2 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1, const double& q2);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1, q2, q3 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1, const double& q2, const double& q3);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1, q2, q3, q4 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1, const double& q2, const double& q3, const double& q4);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1, q2, q3, q4, q5 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1, const double& q2, const double& q3, const double& q4, const double& q5);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1, q2, q3, q4, q5, q6 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1, const double& q2, const double& q3, const double& q4, const double& q5, const double& q6);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1, q2, q3, q4, q5, q6, q7 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1, const double& q2, const double& q3, const double& q4, const double& q5, const double& q6, const double& q7);
                lib->AddFunction(, GetValueQuantiles, ->std::vector<chaiscript::Boxed_Value>, SINGLE_ARG(
                    std::vector<chaiscript::Boxed_Value> out; for (auto& x : a.GetValueQuantiles(std::vector<double>({ q1, q2, q3, q4, q5, q6, q7, q8 }))) { out.push_back(var(cweeUnitValues::unit_value(x))); } return out;
                ), cweeUnitPattern const& a, const double& q1, const double& q2, const double& q3, const double& q4, const double& q5, const double& q6, const double& q7, const double& q8);

                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMinTime(); }), "GetMinTime");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetAvgTime(); }), "GetAvgTime");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetAvgTimestep(); }), "GetAvgTimestep");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMaxTime(); }), "GetMaxTime");

                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetKnotSeries(); }), "GetKnotSeries"); // (std::vector<std::pair<u64, float>>)
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { return a.GetKnotSeries(b); }), "GetKnotSeries"); // (std::vector<std::pair<u64, float>>)
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.GetKnotSeries(b, c); }), "GetKnotSeries"); // (std::vector<std::pair<u64, float>>)
                //lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetValueKnotSeries(); }), "GetValueKnotSeries"); // (std::vector<scalar_t>)
                //lib->add(chaiscript::fun([](cweeUnitPattern& a, const u64& b) { return a.GetValueKnotSeries(b); }), "GetValueKnotSeries"); // (std::vector<scalar_t>)
                //lib->add(chaiscript::fun([](cweeUnitPattern& a, const u64& b, const u64& c) { return a.GetValueKnotSeries(b, c); }), "GetValueKnotSeries"); // (std::vector<scalar_t>)
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c, const unit_value& d) { return a.GetTimeSeries(b, c, d); }), "GetTimeSeries"); // (std::vector<std::pair<u64, float>>)
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c, const unit_value& d) { return a.GetValueTimeSeries(b, c, d); }), "GetValueTimeSeries"); // (std::vector<scalar_t>)

                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetNumValues(); }), "size");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetNumValues(); }), "GetNumValues");

                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.RombergIntegral(a.GetMinTime(), a.GetMaxTime()); }), "Integrate");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& from, const unit_value& to) { return a.RombergIntegral(from, to); }), "Integrate");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, unit_value b) { a += b; return a; }), "+=");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, unit_value b) { a -= b; return a; }), "-=");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, unit_value b) { a *= b; return a; }), "*=");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, unit_value b) { a /= b; return a; }), "/=");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, const cweeUnitPattern& b) { a += b; return a; }), "+=");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const cweeUnitPattern& b) { a -= b; return a; }), "-=");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const cweeUnitPattern& b) { a *= b; return a; }), "*=");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const cweeUnitPattern& b) { a /= b; return a; }), "/=");

                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b) { return a - b; }), "-");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b) { return a / b; }), "/");

                lib->add(chaiscript::fun([](const cweeUnitPattern& a, unit_value b) { return a + b; }), "+");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, unit_value b) { return a - b; }), "-");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, unit_value b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, unit_value b) { return a / b; }), "/");

                lib->add(chaiscript::fun([](const unit_value& b, const cweeUnitPattern& a) { return b + a; }), "+");
                lib->add(chaiscript::fun([](const unit_value& b, const cweeUnitPattern& a) { return b - a; }), "-");
                lib->add(chaiscript::fun([](const unit_value& b, const cweeUnitPattern& a) { return b * a; }), "*");
                lib->add(chaiscript::fun([](const unit_value& b, const cweeUnitPattern& a) { return b / a; }), "/");

                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const unit_value& b) { return a.pow(b); }), "^");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& from, const unit_value& to) { a.RemoveTimes(from, to); }), "RemoveTimes");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const cweeUnitPattern& mask) { a.RemoveWithMask(mask); }), "RemoveWithMask");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b) { return a.R_Squared(b); }), "R_Squared");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b) { return a.Collinear(b); }), "Collinear");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b, const unit_value& from, const unit_value& to) { return a.R_Squared(b, from, to); }), "R_Squared");
                lib->add(chaiscript::fun([](const cweeUnitPattern& a) { 
                    return (((a - a.GetAvgValue()).pow(2.0)).GetAvgValue()).pow(0.5);
                }), "StdDev");
                AUTO blur_pattern = [](cweeUnitPattern const& a, double desiredNumValues){
                    AUTO width = a.GetMaxTime() - a.GetMinTime();                    
                    if (width() <= 0 || desiredNumValues <= 0) {
                        return cweeUnitPattern(a);
                    }
                    AUTO out = cweeUnitPattern();

                    AUTO scale = width / desiredNumValues;
                    
                    out.AddValue(a.GetMinTime(), a.GetCurrentValue(a.GetMinTime()));
                    for (auto time = a.GetMinTime(); time < a.GetMaxTime(); time += scale) 
                        out.AddValue(time + scale / 2.0, a.GetAvgValue(time, time + scale));                    
                    out.AddValue(a.GetMaxTime(), a.GetCurrentValue(a.GetMaxTime()));
                    out.RemoveUnnecessaryKnots();
                    

                    { // Correct the avg/integral
                        AUTO t1 = a.RombergIntegral(a.GetMinTime(), a.GetMaxTime());
                        AUTO t2 = out.RombergIntegral(a.GetMinTime(), a.GetMaxTime());
                        if (t2 != 0) {
                            out *= t1 / t2;
                        }
                    }

                    return out;
                };
                
                lib->add(chaiscript::fun([=](const cweeUnitPattern& a) {
                    AUTO pat = blur_pattern(a, (a.GetNumValues() / 32) + 1);
                    pat.RemoveUnnecessaryKnots();
                    return pat;
                }), "Blur");
                lib->add(chaiscript::fun([=](const cweeUnitPattern& a, int desiredNumValues) {
                    AUTO pat = blur_pattern(a, desiredNumValues);
                    pat.RemoveUnnecessaryKnots();
                    return pat;
                }), "Blur");

                lib->add(chaiscript::fun([](cweeUnitPattern const& a) { return a.X_Type(); }), "X");
                lib->add(chaiscript::fun([](cweeUnitPattern const& a) { return a.Y_Type(); }), "Y");

                lib->add(chaiscript::fun([](cweeUnitPattern const& a, const unit_value& Y_type) { 
                    AUTO out = cweeUnitPattern(a.X_Type(), Y_type);
                    out = a;
                    return out;
                }), "Cast");
                lib->add(chaiscript::fun([](cweeUnitPattern const& a, const unit_value& X_type, const unit_value& Y_type) { 
                    AUTO out = cweeUnitPattern(X_type, Y_type);
                    out = a;
                    return out;
                }), "Cast");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, cweeList<cweeList<cweeStr>> const& data, int TimeColumn, int ValueColumn)->cweeUnitPattern& {
                    unit_value X_type = a.X_Type();
                    unit_value Y_type= a.Y_Type();
                    for (auto& row : data) {
                        a.AddUniqueValue(X_type = row[TimeColumn].ReturnNumeric(), Y_type = row[ValueColumn].ReturnNumeric());
                    }
                    a.RemoveUnnecessaryKnots();
                    return a;
                }), "AppendFromSQL");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, nanodbcResult& con, int TimeColumn, int ValueColumn)->cweeUnitPattern& {
                    unit_value X_type = a.X_Type();
                    unit_value Y_type = a.Y_Type();

                    cweeList<double> row;
                    while (odbc->GetNextRow(con, row)) {
                        a.AddUniqueValue(X_type = row[TimeColumn], Y_type = row[ValueColumn]);
                    }                    
                    a.RemoveUnnecessaryKnots();
                    return a;
                }), "AppendFromSQL");
            }

            // Boxed_Value Curve
            if (1) {
                lib->add(chaiscript::user_type<cweeBalancedCurve<Boxed_Value>>(), "Curve");
                lib->add(chaiscript::constructor<cweeBalancedCurve<Boxed_Value>()>(), "Curve");
                lib->add(chaiscript::constructor<cweeBalancedCurve<Boxed_Value>(const cweeBalancedCurve<Boxed_Value>&)>(), "Curve");
                lib->add(chaiscript::fun([](cweeBalancedCurve<Boxed_Value>& a, cweeBalancedCurve<Boxed_Value>& b) { a = b; return a; }), "=");
                lib->AddFunction(, AddUniqueValue, , return o.AddUniqueValue(t, val), cweeBalancedCurve<Boxed_Value>& o, double t, Boxed_Value const& val);
                lib->AddFunction(, AddValue, , return o.AddValue(t, val), cweeBalancedCurve<Boxed_Value>& o, double t, Boxed_Value const& val);
                lib->AddFunction(, RemoveTimes, , return o.RemoveTimes(greaterThan, lessThenEqualTo), cweeBalancedCurve<Boxed_Value>& o, const double& greaterThan, const double& lessThenEqualTo);
                lib->AddFunction(, Clear, , return o.Clear(), cweeBalancedCurve<Boxed_Value>& o);
                lib->AddFunction(, GetValueKnotSeries, ,
                    std::vector<Boxed_Value> out;
                    for (auto& x : o.GetValueKnotSeries()) {
                        out.push_back(x);
                    }
                    return out;
                , cweeBalancedCurve<Boxed_Value> const& o);
                lib->AddFunction(, GetValueKnotSeries, , 
                    std::vector<Boxed_Value> out;
                    for (auto& x : o.GetValueKnotSeries(t0)) {
                        out.push_back(x);
                    }
                    return out;
                , cweeBalancedCurve<Boxed_Value> const& o, double t0);
                lib->AddFunction(, GetValueKnotSeries, , 
                    std::vector<Boxed_Value> out;
                    for (auto& x : o.GetValueKnotSeries(t0, t1)) {
                        out.push_back(x);
                    }
                    return out;
                , cweeBalancedCurve<Boxed_Value> const& o, double t0, double t1);
                lib->AddFunction(, GetKnotSeries, , 
                    std::vector<Boxed_Value> out;
                    for (auto& x : o.GetKnotSeries()) {
                        out.push_back(var(std::pair<Boxed_Value, Boxed_Value>(var((double)x.first), x.second)));
                    }
                    return out;
                , cweeBalancedCurve<Boxed_Value> const& o);
                lib->AddFunction(, GetKnotSeries, , 
                    std::vector<Boxed_Value> out;
                    for (auto& x : o.GetKnotSeries(t0)) {
                        out.push_back(var(std::pair<Boxed_Value, Boxed_Value>(var((double)x.first), x.second)));
                    }
                    return out;
                , cweeBalancedCurve<Boxed_Value> const& o, double t0);
                lib->AddFunction(, GetKnotSeries, , 
                    std::vector<Boxed_Value> out;
                    for (auto& x : o.GetKnotSeries(t0,t1)) {
                        out.push_back(var(std::pair<Boxed_Value, Boxed_Value>(var((double)x.first), x.second)));
                    }
                    return out;
                , cweeBalancedCurve<Boxed_Value> const& o, double t0, double t1);
                lib->AddFunction(, GetCurrentValue, , return o.GetCurrentValue(t), cweeBalancedCurve<Boxed_Value> const& o, double t); 
                lib->AddFunction(, GetMaxTime, , return o.GetMaxTime(), cweeBalancedCurve<Boxed_Value> const& o);
                lib->AddFunction(, GetMinTime, , return o.GetMinTime(), cweeBalancedCurve<Boxed_Value> const& o);
                lib->AddFunction(, ShiftTime, , return o.ShiftTime(t), cweeBalancedCurve<Boxed_Value>& o, double t);
                lib->AddFunction(, Num, , return o.GetNumValues(), cweeBalancedCurve<Boxed_Value> const& o);
                lib->AddFunction(, size, , return o.GetNumValues(), cweeBalancedCurve<Boxed_Value> const& o);
                lib->AddFunction(, Reserve, , return o.Reserve(num), cweeBalancedCurve<Boxed_Value>& o, int num);
                lib->AddFunction(, [], , return o.GetCurrentValue(t), cweeBalancedCurve<Boxed_Value> const& o, double t);
                lib->AddFunction(, keys, ,
                    std::vector<Boxed_Value> out; 
                    for (auto& x : o.GetKnotSeries()) out.push_back(var((double)x.first));
                    return out;
                , cweeBalancedCurve<Boxed_Value> const& o);
                lib->eval("def to_string(Curve c){ return \"Curve\"; /* to_string(c.keys); */ }");

            }

            // Sparse Matrix
            if (1) {
                using SparseMatrixT = cweeInterpolatedMatrix<float>;

                lib->add(chaiscript::user_type<SparseMatrixT>(), "Matrix");
                lib->add(chaiscript::constructor<SparseMatrixT()>(), "Matrix");
                lib->add(chaiscript::constructor<SparseMatrixT(const SparseMatrixT&)>(), "Matrix");

                lib->AddFunction(, =, ->SparseMatrixT&, matrix1 = matrix2; return matrix1, SparseMatrixT& matrix1, SparseMatrixT const& matrix2);

                lib->AddFunction(, GetMinX, ->double , return matrix.GetMinX(), SparseMatrixT const& matrix);
                lib->AddFunction(, GetMaxX, ->double , return matrix.GetMaxX(), SparseMatrixT const& matrix);
                lib->AddFunction(, GetMinY, ->double , return matrix.GetMinY(), SparseMatrixT const& matrix);
                lib->AddFunction(, GetMaxY, ->double , return matrix.GetMaxY(), SparseMatrixT const& matrix);
                lib->AddFunction(, GetMinValue, , return matrix.GetMinValue(), SparseMatrixT const& matrix);
                lib->AddFunction(, GetMaxValue, , return matrix.GetMaxValue(), SparseMatrixT const& matrix);

                lib->AddFunction(, RemoveUnnecessaryKnots, , matrix.RemoveUnnecessaryKnots(), SparseMatrixT& matrix);

                lib->AddFunction(, GetValue, , return matrix.GetValue(X,Y), SparseMatrixT const& matrix, double X, double Y);
                lib->AddFunction(, GetCurrentValue, , return matrix.GetCurrentValue(X, Y), SparseMatrixT const& matrix, double X, double Y);

                lib->AddFunction(, InsertValue, , matrix.InsertValue(X, Y, V), SparseMatrixT& matrix, double X, double Y, float V);
                lib->AddFunction(, AddValue, , matrix.AddValue(X, Y, V), SparseMatrixT& matrix, double X, double Y, float V);

                lib->AddFunction(, Clear, , matrix.Clear(), SparseMatrixT& matrix);

                lib->AddFunction(, to_string, ->std::string , return matrix.ToString().c_str(), SparseMatrixT const& matrix);
                lib->AddFunction(, from_string, , return matrix.FromString(str), SparseMatrixT& matrix, cweeStr const& str);

                lib->AddFunction(, Num, , return matrix.Num(), SparseMatrixT& matrix);
                lib->AddFunction(, size, , return matrix.Num(), SparseMatrixT& matrix);

                lib->AddFunction(, MinHilbertPosition, ->double, return matrix.MinHilbertPosition(), SparseMatrixT const& matrix);
                lib->AddFunction(, MaxHilbertPosition, ->double, return matrix.MaxHilbertPosition(), SparseMatrixT const& matrix);
                lib->AddFunction(, HilbertPositionToValue, , return matrix.HilbertPositionToValue(position), SparseMatrixT const& matrix, double position);
                lib->AddFunction(, HilbertPositionToXY, , SINGLE_ARG(
                    AUTO x = matrix.HilbertPositionToXY(position);
                    return std::pair<Boxed_Value, Boxed_Value>(var((double)(x.first)), var((double)(x.second)));
                ), SparseMatrixT const& matrix, double position);
            }

            // Extern Data
            if(1) {
                lib->add(chaiscript::user_type<cweeData>(), "external_data");
                lib->add(chaiscript::fun([]()->cweeData* { return external_data.Get(); }), "external_data");
                                
                lib->AddFunction(, CreatePattern, , return dataOwner->CreatePattern(), cweeData* dataOwner);
                lib->AddFunction(, SetPattern, , dataOwner->SetPattern(index, pat); return index; , cweeData* dataOwner, int index, cweeUnitValues::cweeUnitPattern const& pat);
                lib->AddFunction(, GetPattern, , return dataOwner->GetPatternRef(index), cweeData* dataOwner, int index);
                lib->AddFunction(, DeletePattern, , return dataOwner->DeletePattern(index), cweeData* dataOwner, int index);

                lib->AddFunction(, CreateMatrix, , return dataOwner->CreateMatrix(), cweeData* dataOwner);
                lib->AddFunction(, SetMatrix, , dataOwner->SetMatrix(index, matrix); return index;, cweeData* dataOwner, int index, cweeInterpolatedMatrix<float> const& matrix);
                lib->AddFunction(, GetMatrix, , return dataOwner->GetMatrixRef(index), cweeData* dataOwner, int index);
                lib->AddFunction(, DeleteMatrix, , return dataOwner->DeleteMatrix(index), cweeData* dataOwner, int index);

                lib->AddFunction(, CreateString, , return dataOwner->CreateString(), cweeData* dataOwner);
                lib->AddFunction(, SetString, , dataOwner->GetStringRef(index)->operator=(str); return index;, cweeData* dataOwner, int index, cweeStr const& str);
                lib->AddFunction(, GetString, , return dataOwner->GetStringRef(index), cweeData* dataOwner, int index);
                lib->AddFunction(, DeleteString, , return dataOwner->DeleteString(index), cweeData* dataOwner, int index);
            }

            // File System
            if (1) {
                // IP Address Information
                {
                    AddBasicClassTemplate(IpAddressInformation);
                    AddBasicClassMember(IpAddressInformation, city);
                    AddBasicClassMember(IpAddressInformation, country_name);
                    AddBasicClassMember(IpAddressInformation, ip);
                    AddBasicClassMember(IpAddressInformation, latitude);
                    AddBasicClassMember(IpAddressInformation, longitude);
                    AddBasicClassMember(IpAddressInformation, region_name);
                    AddBasicClassMember(IpAddressInformation, time_zone);
                    AddBasicClassMember(IpAddressInformation, zip_code);
                    lib->add(chaiscript::fun([](IpAddressInformation& a) -> cwee_units::length::foot_t { return cwee_units::length::foot_t(a.coordinates.z); }), "elevation");
                    lib->add(chaiscript::fun([](IpAddressInformation const& a) -> std::string { 
                        return cweeStr::printf("%s, %s %s, %s", a.city.c_str(), a.region_name.c_str(), a.zip_code.c_str(), a.country_name.c_str()).c_str();
                    }), "to_string");
                }
                lib->add(chaiscript::fun([](const cweeStr& Directory) { return fileSystem->ensureDirectoryExists(Directory); }), "ensureDirectoryExists");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath, const cweeStr& newFilePath) { return fileSystem->renameFile(oldFilePath, newFilePath); }), "renameFile");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath, const cweeStr& newFilePath) { return fileSystem->copyFile(oldFilePath, newFilePath); }), "copyFile");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath) { return fileSystem->removeFile(oldFilePath); }), "removeFile");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath) { return fileSystem->checkFileExists(oldFilePath); }), "checkFileExists");
                lib->add(chaiscript::fun([]() { return fileSystem->getAppFolder(); }), "getAppFolder");
                lib->add(chaiscript::fun([]() { return fileSystem->getAppName(); }), "getAppName");
                lib->add(chaiscript::fun([]() { return fileSystem->getDataFolder(); }), "getDataFolder");
                lib->add(chaiscript::fun([](cweeStr const& in) { return fileSystem->setDataFolder(in); }), "setDataFolder");
                lib->add(chaiscript::fun([](cweeStr const& directory, cweeStr const& fileName, cweeStr const& fileType) { return fileSystem->createFilePath(directory, fileName, GetBetterEnum< fileType_t >(fileType)); }), "createFilePath");
                lib->add(chaiscript::fun([](cweeStr const& fileType) { return fileSystem->createRandomFilePath(GetBetterEnum< fileType_t >(fileType)); }), "createRandomFilePath");
                lib->add(chaiscript::fun([](cweeStr const& fileType) { return fileSystem->createRandomFile(GetBetterEnum< fileType_t >(fileType)); }), "createRandomFile");
                lib->add(chaiscript::fun([]() { return fileSystem->getCurrentTime(); }), "getCurrentTime");
                lib->add(chaiscript::fun([]() { return fileSystem->localtime(fileSystem->getCurrentTime()); }), "localtime");
                lib->add(chaiscript::fun([](u64 t) { return fileSystem->localtime(t); }), "localtime");
                lib->add(chaiscript::fun([]() { return fileSystem->GetIpAddress(); }), "GetIpAddress");
                lib->add(chaiscript::fun([]() { return fileSystem->GetAddress(); }), "GetAddress");
                {
                    lib->add(chaiscript::fun([](cweeStr const& filePath) { cweeList<chaiscript::Boxed_Value> bv; for (auto& s : fileSystem->readFileAsStrList(filePath)) { bv.Append(chaiscript::Boxed_Value(s)); } return bv; }), "readFileAsStrList");
                    lib->add(chaiscript::fun([](cweeStr const& filePath) { cweeStr out;  fileSystem->readFileAsCweeStr(out, filePath); return out; }), "readFileAsCweeStr");
                    lib->add(chaiscript::fun([](cweeStr const& filePath, cweeStr const& content) { fileSystem->writeFileFromCweeStr(filePath, content);  }), "writeFileFromCweeStr");
                    lib->add(chaiscript::fun([](cweeStr const& filePath, cweeThreadedList<cweeStr> const& content) { fileSystem->writeFileFromStrList(filePath, content);  }), "writeFileFromStrList");
                    lib->add(chaiscript::fun([](cweeStr const& url) { return fileSystem->DownloadCweeStrFromURL(url); }), "DownloadCweeStrFromURL");
                    lib->add(chaiscript::fun([](cweeStr const& url) {
                        AUTO filePath_dwnl = fileSystem->QueryHttpToFile(url, "");
                        return filePath_dwnl;
                    }), "DownloadFileFromURL");
                    lib->add(chaiscript::fun([](cweeStr const& url, cweeStr const& filePath) {
                        AUTO filePath_dwnl = fileSystem->QueryHttpToFile(url, "");
                        fileSystem->renameFile(filePath_dwnl, filePath);
                        return filePath;
                    }), "DownloadFileFromURL");
                    lib->add(chaiscript::fun([](cweeStr const& url) { return fileSystem->QueryHttp(url, ""); }), "QueryHTTP");
                    lib->add(chaiscript::fun([](cweeStr const& url, cweeStr const& params) { return fileSystem->QueryHttp(url, params); }), "QueryHTTP");
                    lib->add(chaiscript::fun([](cweeStr const& directory, cweeStr const& extension) { 
                        cweeList<chaiscript::Boxed_Value> bv; 
                        for (auto& s : fileSystem->listFilesWithExtension(directory, extension)) 
                        { 
                            bv.Append(chaiscript::Boxed_Value(s)); 
                        } 
                        return bv; 
                    }), "listFilesWithExtension");
                    lib->add(chaiscript::fun([](cweeStr const& directory, cweeStr const& extension) { 
                        cweeList<chaiscript::Boxed_Value> bv; 
                        for (auto& s : fileSystem->listFilesWithExtension(directory, "*")) 
                        { 
                            bv.Append(chaiscript::Boxed_Value(s)); 
                        } 
                        return bv; 
                    }), "listFiles");
                    lib->add(chaiscript::fun([](cweeStr const& directory) {
                        cweeList<chaiscript::Boxed_Value> bv;
                        for (auto& s : fileSystem->listDirectories(directory, true)) {
                            bv.Append(chaiscript::Boxed_Value(s));
                        }
                        return bv;
                    }), "listAllFiles");
                    lib->add(chaiscript::fun([](cweeStr const& directory) { 
                        cweeList<chaiscript::Boxed_Value> bv; 
                        for (auto& s : fileSystem->listDirectories(directory, false)) { 
                            bv.Append(chaiscript::Boxed_Value(s)); 
                        } 
                        return bv; 
                    }), "listDirectories");

                    lib->add(chaiscript::fun([](cweeStr const& title, cweeStr const& content) { fileSystem->submitToast(title, content); }), "submitToast");
                }
            }

            // RTree
            if (1) {
                class RTreeContainer {
                public:
                    static cweeBoundary const& GetCoordinates(RTreeContainer const& o) {
                        return o.boundary;
                    };
                    static cwee_units::foot_t GetDistance(RTreeContainer const& o, cweeBoundary const& b) {
                        return b.Distance(GetCoordinates(o));
                    };

                    chaiscript::Boxed_Value data;
                    cweeBoundary boundary;

                    RTreeContainer() : data(), boundary() {};
                    RTreeContainer(RTreeContainer const& o) : data(o.data), boundary(o.boundary) {};
                    RTreeContainer& operator=(RTreeContainer const& o) {
                        this->boundary = o.boundary;
                        this->data = o.data;
                        return *this;
                    };
                    bool operator==(RTreeContainer const& b) { 
                        return data.get_ptr() == b.data.get_ptr();
                    };
                    bool operator!=(RTreeContainer const& b) { return !operator==(b); };
                };
                
                using RTreeType = RTree< RTreeContainer, RTreeContainer::GetCoordinates, RTreeContainer::GetDistance>;

                class RTreeNode {
                public:
                    RTreeType::TreeNode* node{ nullptr };

                    RTreeNode() : node(nullptr) {};
                    RTreeNode(RTreeType::TreeNode* p) : node(p) {};
                    RTreeNode(RTreeNode const&) = default;
                    RTreeNode(RTreeNode&&) = default;
                    RTreeNode& operator=(RTreeNode const&) = default;
                    RTreeNode& operator=(RTreeNode&&) = default;
                    
                    cweeBoundary boundary() const {
                        cweeBoundary out;
                        if (node){
                            if (node->object) {
                                out = node->object->boundary;
                            } 
                            else {
                                out = node->bound;
                            }
                        }
                        return out;
                    };
                    Boxed_Value object() const {
                        Boxed_Value out;
                        if (node && node->object) {
                            out = node->object->data;
                        }
                        return out;
                    };
                    std::vector<Boxed_Value> children() const {
                        std::vector<Boxed_Value> out;
                        if (node) {
                            for (auto* x : node->children) {
                                out.push_back(var(RTreeNode(x)));
                            }                            
                        }
                        return out;
                    };
                    int num_children() const {
                        int out{ 0 };
                        if (node) {
                            out = node->children.Num();
                        }
                        return out;
                    };
                    RTreeNode parent() const {
                        if (node) return RTreeNode(node->parent);
                        return RTreeNode();
                    };
                    RTreeNode next() const {
                        return RTreeNode(RTreeType::GetNext(node));
                    };
                    RTreeNode next_leaf() const {
                        return RTreeNode(RTreeType::GetNextLeaf(node));
                    };
                };
                AUTO First_Node_From_Tree = [](RTreeType& obj) {
                    return RTreeNode(obj.GetRoot());
                };
                lib->add(chaiscript::user_type<RTreeNode>(), "RTreeNode");
                lib->add(chaiscript::constructor<RTreeNode()>(), "RTreeNode");
                lib->add(chaiscript::constructor<RTreeNode(const RTreeNode&)>(), "RTreeNode");
                lib->add(chaiscript::fun([](RTreeNode& a, const RTreeNode& b)->RTreeNode& { a = b; return a; }), "=");
                lib->AddFunction(, children, , return obj.children(), RTreeNode const& obj);
                lib->AddFunction(, object, , return obj.object(), RTreeNode const& obj);
                lib->AddFunction(, num_children, , return obj.num_children(), RTreeNode const& obj);
                lib->AddFunction(, parent, , return obj.parent(), RTreeNode const& obj);
                lib->AddFunction(, boundary, , return obj.boundary(), RTreeNode const& obj);
                lib->AddFunction(, next, , return obj.next(), RTreeNode const& obj);
                lib->AddFunction(, next_leaf, , return obj.next_leaf(), RTreeNode const& obj);

                lib->add(chaiscript::user_type<RTreeType>(), "RTree");
                lib->add(chaiscript::constructor<RTreeType()>(), "RTree");
                lib->add(chaiscript::constructor<RTreeType(const RTreeType&)>(), "RTree");
                lib->add(chaiscript::fun([](RTreeType& a, const RTreeType& b)->RTreeType& { a = b; return a; }), "=");                
                lib->AddFunction(, Root, , return RTreeNode(obj.GetRoot());, RTreeType& obj);
                AUTO Remove_From_RTree = [](RTreeType& obj, chaiscript::Boxed_Value const& t)-> void {
                    auto foundObj = obj.TryFindObject([&](RTreeContainer const& o)->bool {
                        return o.data.get_ptr() == t.get_ptr();
                    });
                    if (foundObj) {
                        obj.Remove(foundObj);
                    }                   
                };

                lib->AddFunction(, Add, ->void ,
                    RTreeContainer c; 
                    cweeBoundary bound;
                    bound.bottomLeft = vec2d(longitude, latitude);
                    bound.topRight = vec2d(longitude, latitude);
                    c.boundary = bound;
                    c.data = t; 
                    obj.Add(c);
                , RTreeType& obj, chaiscript::Boxed_Value const& t, double longitude, double latitude);
                lib->AddFunction(, Add, ->void,
                    RTreeContainer c;
                    cweeBoundary bound;
                    bound.bottomLeft = coordinate;
                    bound.topRight = coordinate;
                    c.boundary;
                    c.data = t;
                    obj.Add(c);
                , RTreeType& obj, chaiscript::Boxed_Value const& t, vec2d const& coordinate);
                lib->AddFunction(, Add, ->void,
                    RTreeContainer c;
                    c.boundary = coordinateFunction(t);                    
                    c.data = t;
                    obj.Add(c);
                , RTreeType& obj, chaiscript::Boxed_Value const& t, std::function<cweeBoundary(chaiscript::Boxed_Value const&)> const& coordinateFunction);
                lib->AddFunction(Remove_From_RTree, Remove,->void ,
                    Remove_From_RTree(obj, t);
                , RTreeType& obj, chaiscript::Boxed_Value const& t);
                lib->AddFunction(, Near, ,
                    chaiscript::Boxed_Value out;
                    auto ptr_list = obj.Near(t, 1);
                    if (ptr_list.Num() > 0 && ptr_list[0]->object) {
                        out = var(RTreeNode(ptr_list[0]));
                    }
                    return out;
                , RTreeType& obj, cweeBoundary const& t);
                lib->AddFunction(, Near, ,
                    std::vector<chaiscript::Boxed_Value> out;
                    for (auto& x : obj.Near(t, numNearest)) {
                        if (x && x->object) {
                            out.push_back(var(RTreeNode(x)));
                        }
                    }
                    return out;
                , RTreeType& obj, cweeBoundary const& t, int numNearest);
                
                AUTO Node_To_Layer = [](RTreeType::TreeNode* node)-> chaiscript::Boxed_Value {
                    chaiscript::Boxed_Value out;
                    if (node) {
                        UI_Color col(cweeRandomFloat(25, 230), cweeRandomFloat(25, 230), cweeRandomFloat(25, 230), 255);

                        cweeBoundary& obj = node->bound;
                        cweeSharedPtr< RTreeContainer>& p = node->object;

                        if (p) {
                            // object node
                            if (obj.bottomLeft != obj.topRight) {
                                UI_MapPolygon polygon;
                                polygon.thickness = cweeRandomFloat(1, 1);
                                polygon.fill = col;
                                polygon.stroke = col;
                                polygon.AddPoint(obj.bottomLeft.x, obj.topRight.y);
                                polygon.AddPoint(obj.topRight.x, obj.topRight.y);
                                polygon.AddPoint(obj.topRight.x, obj.bottomLeft.y);
                                polygon.AddPoint(obj.bottomLeft.x, obj.bottomLeft.y);
                                polygon.Tag = chaiscript::var(RTreeNode(node)); // p->data;
                                out = chaiscript::var(std::move(polygon));
                            } else {
                                UI_MapIcon icon;
                                icon.longitude = obj.bottomLeft.x;
                                icon.latitude = obj.bottomLeft.y;
                                icon.HideOnCollision = false;
                                icon.color = col;
                                icon.Tag = chaiscript::var(RTreeNode(node)); // p->data;
                                out = chaiscript::var(std::move(icon));
                            }
                        } 
                        else {
                            // tree with children
                            UI_MapPolygon polygon;
                            polygon.thickness = cweeRandomFloat(2, 5);
                            polygon.fill = UI_Color(0, 0, 0, 0);
                            polygon.stroke = col;
                            polygon.AddPoint(obj.bottomLeft.x, obj.topRight.y);
                            polygon.AddPoint(obj.topRight.x, obj.topRight.y);
                            polygon.AddPoint(obj.topRight.x, obj.bottomLeft.y);
                            polygon.AddPoint(obj.bottomLeft.x, obj.bottomLeft.y);
                            polygon.Tag = chaiscript::var(RTreeNode(node)); // cweeBoundary(obj)
                            out = chaiscript::var(std::move(polygon));
                        }
                    }
                    return out;
                };

                AUTO RTreeToMap = [Node_To_Layer](RTreeType& obj) {
                    UI_Map out; UI_MapLayer layer;
                    {
                        auto* root = obj.GetRoot();
                        while (root) {
                            layer.Children.push_back(Node_To_Layer(root));
                            root = RTreeType::GetNext(root);
                            if (root == obj.GetRoot()) break;
                        }
                    }
                    out.Layers.push_back(chaiscript::var((UI_MapLayer)layer));
                    return out;
                };

                lib->AddFunction(RTreeToMap, UI_Map, -> UI_Map, {
                    return RTreeToMap(obj);
                }, RTreeType& obj);

                lib->add(chaiscript::user_type<cweeBoundary>(), "Boundary");
                lib->add(chaiscript::constructor<cweeBoundary()>(), "Boundary");
                lib->add(chaiscript::constructor<cweeBoundary(const cweeBoundary&)>(), "Boundary");
                lib->add(chaiscript::fun([](cweeBoundary& a, const cweeBoundary& b)->cweeBoundary& { a = b; return a; }), "=");
                lib->add(chaiscript::fun([](cweeBoundary const& a) -> cweeStr { return cweeStr::printf("[<%f,%f>, <%f,%f>]", a.topRight.x, a.topRight.y, a.bottomLeft.x, a.bottomLeft.y); }), "to_string");
                lib->AddFunction(, topRight, ->vec2d& , return obj.topRight; , cweeBoundary& obj);
                lib->AddFunction(, bottomLeft, ->vec2d& , return obj.bottomLeft;, cweeBoundary& obj);
                lib->AddFunction(, geographic, ->bool&, return obj.geographic; , cweeBoundary& obj);
                lib->AddFunction(, distance, -> cweeUnitValues::unit_value, return cweeUnitValues::foot(cweeBoundary::Distance(obj1, obj2)());, cweeBoundary const& obj1, cweeBoundary const& obj2);
            }

            return lib;
        };
    };
}; // namespace chaiscript