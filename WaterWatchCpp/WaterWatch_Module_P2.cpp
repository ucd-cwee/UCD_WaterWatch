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

#if 0
                using namespace cweeEng;

                AddBasicClassTemplate(LeakageModel2);

                AddBasicClassMember(LeakageModel2, TotalMileage);
                AddBasicClassMember(LeakageModel2, VariableProductionCost);
                AddBasicClassMember(LeakageModel2, NumberOfConnections);

                AddBasicClassMember(LeakageModel2, discount);
                AddBasicClassMember(LeakageModel2, T);
                AddBasicClassMember(LeakageModel2, M);
                AddBasicClassMember(LeakageModel2, L);
                AddBasicClassMember(LeakageModel2, Calr);
                AddBasicClassMember(LeakageModel2, Dldr);
                AddBasicClassMember(LeakageModel2, init);
                AddBasicClassMember(LeakageModel2, R1);

                AddBasicClassMember(LeakageModel2, Vol_Lost_Per_s);
                AddBasicClassMember(LeakageModel2, Vol_Lost_Total);
                AddBasicClassMember(LeakageModel2, Vol_Saved);
                AddBasicClassMember(LeakageModel2, LDR_Survey_Repair_Cost);
                AddBasicClassMember(LeakageModel2, Benefits);
                AddBasicClassMember(LeakageModel2, PR_Cost);
                AddBasicClassMember(LeakageModel2, TotalCost);
                AddBasicClassMember(LeakageModel2, NetBenefits);

                AddBasicClassMember(LeakageModel2, BestSurveyFrequency);
#endif

                //AddNamespacedClassTemplate(LeakageModel, FailureRecord);
                //AddNamespacedClassTemplate(LeakageModel, Inputs);
                //AddNamespacedClassTemplate(LeakageModel, PR_Costs);
                //AddNamespacedClassTemplate(LeakageModel, Benefits);
                //AddNamespacedClassTemplate(LeakageModel, LDR_Costs);
                //AddNamespacedClassTemplate(LeakageModel, TimeHorizon);

                //AddBasicClassMember(LeakageModel, Recalculate);
                //AddBasicClassMember(LeakageModel, inputs);
                //AddBasicClassMember(LeakageModel, pr_costs);
                //AddBasicClassMember(LeakageModel, benefits);
                //AddBasicClassMember(LeakageModel, ldr_Costs);
                //AddBasicClassMember(LeakageModel, timeHorizon);

                //AddNamespacedClassMember(LeakageModel, FailureRecord, FailureFlowrate);
                //AddNamespacedClassMember(LeakageModel, FailureRecord, ResponseTime);

                //AddNamespacedClassMember(LeakageModel, Inputs, CurrentAnnualRealLosses);
                //AddNamespacedClassMember(LeakageModel, Inputs, InfrastructureLeakageIndex);
                //AddNamespacedClassMember(LeakageModel, Inputs, TotalMileage);
                //AddNamespacedClassMember(LeakageModel, Inputs, UnavoidableAnnualRealLosses);
                //AddNamespacedClassMember(LeakageModel, Inputs, VariableProductionCost);
                //AddNamespacedClassMember(LeakageModel, Inputs, NumberOfConnections);
                //AddNamespacedClassMember(LeakageModel, Inputs, AverageOperatingPressure);
                //AddNamespacedClassMember(LeakageModel, Inputs, AverageLengthOfCustomerServiceLine);
                //AddNamespacedClassMember(LeakageModel, Inputs, InfrastructureConditionFactor);
                //AddNamespacedClassMember(LeakageModel, Inputs, PercentageOfPipesThatAreRigid);
                //AddNamespacedClassMember(LeakageModel, Inputs, MarginalEnergyIntensity);
                //AddNamespacedClassMember(LeakageModel, Inputs, AverageLeakRepairCost);
                //AddNamespacedClassMember(LeakageModel, Inputs, IndividualPressureModulatedPRVCost);
                //AddNamespacedClassMember(LeakageModel, Inputs, LeakDetectionSurveyCost);
                //AddNamespacedClassMember(LeakageModel, Inputs, HydraulicModelDevelopmentCost);
                //AddNamespacedClassMember(LeakageModel, Inputs, TimeHorizon);
                //AddNamespacedClassMember(LeakageModel, Inputs, DiscountRate);
                //AddNamespacedClassMember(LeakageModel, Inputs, SocialCostOfCarbon_GrowthRate);
                //AddNamespacedClassMember(LeakageModel, Inputs, WaterProductionCost_GrowthRate);
                //AddNamespacedClassMember(LeakageModel, Inputs, SocialCostOfCarbon);
                //AddNamespacedClassMember(LeakageModel, Inputs, CarbonEmissionRate);
                //AddNamespacedClassMember(LeakageModel, Inputs, AnnualRateOfRiseOfLeakage);
                //AddNamespacedClassMember(LeakageModel, Inputs, AverageFlowrateOfUnreportedDetectableLeaks);
                //AddNamespacedClassMember(LeakageModel, Inputs, StorageCapacity);
                //AddNamespacedClassMember(LeakageModel, Inputs, BackgroundLeakageRateForStorage);
                //AddNamespacedClassMember(LeakageModel, Inputs, MainFailureRate);

                //AddNamespacedClassMember(LeakageModel, PR_Costs, TotalRealLossesFromFailures);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, N1);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, HydraulicModelCosts);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, TotalLeaksFromStorage);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, ReducableRealBackgroundLosses);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, totalBackgroundLeakage);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, totalReportedLeakage);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, totalUnreportedLeakage);
                //AddNamespacedClassMember(LeakageModel, PR_Costs, total_UARL_Leakage);

                //AddNamespacedClassMember(LeakageModel, Benefits, NpvSccFactor);
                //AddNamespacedClassMember(LeakageModel, Benefits, ValueOfAvoidedCO2);
                //AddNamespacedClassMember(LeakageModel, Benefits, NpvOfCO2Savings);
                //AddNamespacedClassMember(LeakageModel, Benefits, NpvWaterProductionFactor);
                //AddNamespacedClassMember(LeakageModel, Benefits, NpvOfAvoidedWaterProduction);
                //AddNamespacedClassMember(LeakageModel, Benefits, NpvOfBenefits);

                //AddNamespacedClassMember(LeakageModel, LDR_Costs, AdjustedPipeRepairCost);
                //AddNamespacedClassMember(LeakageModel, LDR_Costs, LeakDetectionSurveyCost);
                //AddNamespacedClassMember(LeakageModel, LDR_Costs, AverageWaterLostPerYearPerLeak);
                //AddNamespacedClassMember(LeakageModel, LDR_Costs, BreakEven_SurveyFrequency);
                //AddNamespacedClassMember(LeakageModel, LDR_Costs, BreakEven_BenefitsUsed);
                //AddNamespacedClassMember(LeakageModel, LDR_Costs, BreakEven_TotalCost);
                //AddNamespacedClassMember(LeakageModel, LDR_Costs, BreakEven_EconomicPercentOfSystemPerYear);

                //AddNamespacedClassMember(LeakageModel, TimeHorizon, totalCostOfSurveysOverTimeHorizon);
                //AddNamespacedClassMember(LeakageModel, TimeHorizon, totalVolumeOverTimeHorizon);
            }

            // Vec2d
            if (1) {
                DEF_DECLARE_PAIR(double, double);
            }

            // Geocoding
            if (1) {
                lib->add(chaiscript::fun([](double X, double Y) { return cwee_units::length::foot_t(geocoding->GetElevation(vec2d(X, Y))); }, {"Longitude", "Latitude"}), "GetElevation");
                lib->add(chaiscript::fun([](cweeStr const& address) { auto v = geocoding->GetLongLat(address); return cweePair<double, double>(v.x, v.y); }, { "Address" }), "GetLongLat");
                // lib->add(chaiscript::fun([](double X, double Y) { auto v = geocoding->GetLongLat(X, Y); return cweePair<double, double>(v.x, v.y); }, { "Easting_ft", "Northing_ft" }), "GetLongLat");
               
                lib->AddFunction(, GetLongLat, , SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel, LatOrigin, FalseNorthing, FalseEasting); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel, double LatOrigin, double FalseNorthing, double FalseEasting);

                lib->AddFunction(, GetLongLat, , SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel, LatOrigin, FalseNorthing); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel, double LatOrigin, double FalseNorthing);

                lib->AddFunction(, GetLongLat, , SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel, LatOrigin); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel, double LatOrigin);

                lib->AddFunction(, GetLongLat, , SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel, LatSecondStandardParallel); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel, double LatSecondStandardParallel);

                lib->AddFunction(, GetLongLat, , SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian, LatFirstStandardParallel); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian, double LatFirstStandardParallel);

                lib->AddFunction(, GetLongLat, , SINGLE_ARG(if (true) {
                    auto v = geocoding->GetLongLat(easting(), northing(), centralMeridian); return cweePair<double, double>(v.x, v.y);
                }), cwee_units::length::foot_t easting, cwee_units::length::foot_t northing, double centralMeridian);

                lib->AddFunction(, GetLongLat, , SINGLE_ARG(if (true) {
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
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { return a.GetCurrentValue(b); }), "GetCurrentValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { return a.GetCurrentFirstDerivative(b); }), "GetCurrentFirstDerivative");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b) { return a.GetCurrentSecondDerivative(b); }), "GetCurrentSecondDerivative");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMinValue(); }), "GetMinValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetAvgValue(); }), "GetAvgValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMaxValue(); }), "GetMaxValue");

                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.GetMinValue(b, c); }), "GetMinValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.GetAvgValue(b, c); }), "GetAvgValue");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& b, const unit_value& c) { return a.GetMaxValue(b, c); }), "GetMaxValue");

                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMinimumTimeStep(); }), "GetMinimumTimeStep");

                //lib->add(chaiscript::fun([](cweeUnitPattern& a) { a.RemoveUnnecessaryKnots(); }), "RemoveUnnecessaryKnots");
                //lib->add(chaiscript::fun([](cweeUnitPattern& a, const float& percentToRemove) { a.ReduceMemory(percentToRemove); }), "ReduceMemory");
                lib->add(chaiscript::fun([](cweeUnitPattern& a, const unit_value& min, const unit_value& max) { a.ClampValues(min, max); }), "ClampValues");

                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetMinTime(); }), "GetMinTime");
                lib->add(chaiscript::fun([](cweeUnitPattern& a) { return a.GetAvgTime(); }), "GetAvgTime");
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

#if 1
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
#endif

                lib->add(chaiscript::fun([](const cweeUnitPattern& a, const cweeUnitPattern& b) { return a.R_Squared(b); }), "R_Squared");

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
            }

            // Extern Data
            if(1) {
                chaiscript::dispatch::Dynamic_Object o;
                o["CreatePattern"] = chaiscript::var(chaiscript::fun([]() { return external_data->CreatePattern(); }));
                o["SetPattern"] = chaiscript::var(chaiscript::fun([](int index, cweeUnitValues::cweeUnitPattern const& p) { external_data->SetPattern(index, p); }));
                o["GetPattern"] = chaiscript::var(chaiscript::fun([](int index) { return external_data->GetPatternRef(index); }));
                o["DeletePattern"]  = chaiscript::var(chaiscript::fun([](int index) { return external_data->DeletePattern(index); }));

                o["CreateString"] = chaiscript::var(chaiscript::fun([]() { return external_data->CreateString(); }));
                o["GetString"] = chaiscript::var(chaiscript::fun([](int index) { return external_data->GetStringRef(index); }));
                o["DeleteString"] = chaiscript::var(chaiscript::fun([](int index) { return external_data->DeleteString(index); }));

                lib->add_global_const(chaiscript::const_var(o), "external_data");
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
                }
                lib->add(chaiscript::fun([](const cweeStr& Directory) { return fileSystem->ensureDirectoryExists(Directory); }), "ensureDirectoryExists");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath, const cweeStr& newFilePath) { return fileSystem->renameFile(oldFilePath, newFilePath); }), "renameFile");
                lib->add(chaiscript::fun([](const cweeStr& oldFilePath, const cweeStr& newFilePath) { return fileSystem->copyFile(oldFilePath, newFilePath); }), "copyFile");
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
                    lib->add(chaiscript::fun([](cweeStr const& url) { return fileSystem->DownloadCweeStrFromURL(url); }), "DownloadCweeStrFromURL");
                    lib->add(chaiscript::fun([](cweeStr const& url) { return fileSystem->QueryHttp(url, ""); }), "QueryHTTP");
                    lib->add(chaiscript::fun([](cweeStr const& url, cweeStr const& params) { return fileSystem->QueryHttp(url, params); }), "QueryHTTP");
                    lib->add(chaiscript::fun([](cweeStr const& directory, cweeStr const& extension) { cweeList<chaiscript::Boxed_Value> bv; for (auto& s : fileSystem->listFilesWithExtension(directory, extension)) { bv.Append(chaiscript::Boxed_Value(s)); } return bv; }), "listFilesWithExtension");
                    lib->add(chaiscript::fun([](cweeStr const& title, cweeStr const& content) { fileSystem->submitToast(title, content); }), "submitToast");
                }
            }

            return lib;
        };
    };
}; // namespace chaiscript